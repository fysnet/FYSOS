; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;    Copyright (c) 1984-2015    Forever Young Software  Benjamin David Lunt
;
; This code is intended for use with the book it accompanies.
; You may use this code for that purpose only.  You may modify it,
;  and/or include it within your own code as long as you do not
;  distribute it.
; You may not distribute this code to anyone with out permission
;  from the author.
;
;             -- All rights reserved -- Use at your own risk --
;
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

comment /******************************************************************\
*                            FYS OS version 2.0                            *
* FILE: mem_info.asm                                                       *
*                                                                          *
*  Built with:  NBASM ver 00.26.44                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update: 11 May 2015                                                 *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
* This code must be assembled with NBASM version 00.26.31 or later.  Any   *
*  version before this will not correctly assemble the 64-bit immediate    *
*  values with the DQ declaration.  Versions before also had a bug in the  *
*  IMUL reg32,reg3,immed instruction.                                      *
*                                                                          *
* This code assumes a 32-bit machine with out checking first.              *
*                                                                          *
*                                                                          *
****************************************************************************
*                                                                          *
* If you have any modifications, improvements, or comments, please let me  *
*  know by posting to alt.os.development or emailing me at                 *
*    fys@fysnet.net                                                        *
*                                                                          *
\**************************************************************************/

.model tiny                        ;

include 'mem_info.inc'             ; our include file
include '..\include\stdio.inc'

; start of our code
.code                              ;
.stack  1024                       ; use a stack size of 1024 bytes
.rmode                             ; bios starts with real mode
.386                               ; we assume 386

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; DOS .COM files start at 100h
           org  100h
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this free's up any unused memory and sets up a stack for us
           .start
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; make sure segment registers are good
           mov  ax,cs
           mov  ds,ax
           mov  es,ax
      
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the header and copywrite strings.
           mov  esi,offset hdr_copy_str
           call print_str

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print the "trying service E820" string and try that service
           mov  esi,offset service_E820_str
           call print_str
           
           mov  edi,offset our_memory_info
           mov  esi,offset temp_buff
           call try_e820_service  ; returns carry set if not supported
           jnc  short print_info
          
    
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print the "trying service E8x1" string and try that service
           mov  esi,offset service_E8x1_str
           call print_str
           
           mov  edi,offset our_memory_info
           call try_e8x1_service  ; returns carry set if not supported
           jnc  short print_info
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print the "trying service 88" string and try that service
           mov  esi,offset service_88_str
           call print_str
           
           mov  edi,offset our_memory_info
           call try_88_service  ; returns carry set if not supported
           jnc  short print_info
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print the "using the CMOS" string and use that tech
           mov  esi,offset using_cmos_str
           call print_str
           
           mov  edi,offset our_memory_info
           call get_cmos_tech  ; returns carry set if not supported
           jnc  short print_info


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we got here, none of the services worked!
           mov  esi,offset no_services
           call print_str
           
           .exit


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the information to the screen
print_info:
           mov  esi,offset blocks_found_str
           call print_str
           
           mov  edi,offset our_memory_info
           movzx eax,word [edi+S_MEMORY->blocks]
           mov  ebx,FALSE
           call prt_dec32
           
           mov  esi,offset crlf
           call print_str
           
           xor  ecx,ecx
           mov  [edi+S_MEMORY->size],ecx
           lea  esi,[edi+S_MEMORY->block]
print_loop:
           mov  eax,ecx
           inc  eax
           mov  ebx,FALSE
           call prt_dec32
           
           push esi
           mov  esi,offset base_str
           call print_str
           pop  esi
           
           mov  eax,[esi+S_MEMORY_ENTRY->base]
           call prt_hex32
           
           push esi
           mov  esi,offset size_str
           call print_str
           pop  esi
           
           mov  eax,[esi+S_MEMORY_ENTRY->size]
           add  [edi+S_MEMORY->size],eax        ; add to total size
           call prt_hex32
           push eax
           mov  al,' '
           call print_char
           mov  al,'('
           call print_char
           pop  eax
           mov  ebx,TRUE
           call prt_dec32
           mov  al,')'
           call print_char
           
           push esi
           mov  esi,offset type_str
           call print_str
           pop  esi
           
           mov  eax,[si+S_MEMORY_ENTRY->type]
           mov  ebx,FALSE
           call prt_dec32

           push esi
           mov  esi,offset crlf
           call print_str
           pop  esi
           
           add  esi,sizeof(S_MEMORY_ENTRY)
           
           inc  ecx
           movzx eax,word [edi+S_MEMORY->blocks]
           cmp  ecx,eax
           jb   print_loop

           mov  esi,offset total_str
           call print_str
           
           mov  eax,[edi+S_MEMORY->size]   ; total size
           mov  ebx,TRUE
           call prt_dec32
           
           push eax
           mov  al,')'
           call print_char
           mov  al,' '
           call print_char
           pop  eax
           
           shr  eax,20
           mov  ebx,FALSE
           call prt_dec32

           mov  al,'M'
           call print_char
           mov  al,'e'
           call print_char
           mov  al,'g'
           call print_char
           
           mov  esi,offset crlf
           call print_str
           
           .exit


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Try the E820 service
; On entry:
;   es:edi-> segmented address to store the information
;   es:esi-> segmented address of a spare buffer at least 32 bytes in size
; On return:
;   Carry clear if memory service supported
;
try_e820_service proc near uses eax ebx ecx edx esi edi

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; first, clear it out.
; we clear it out each try just in case the last try added
;  something, but then errored
           push edi
           xor  al,al
           mov  ecx,sizeof(S_MEMORY)
           rep
             stosb
           pop  edi
        
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; since this service uses es:edi, we will swap
           ; buffer pointers.  Our buffer to store to will now
           ; be in es:esi and the spare buffer in es:edi
           xchg esi,edi
            
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           xor  ebx,ebx
get_mem_loop:
           mov  eax,0000E820h
           mov  edx,534D4150h  ; 'SMAP'
           mov  ecx,00000020h
           int  15h
           jnc  short get_mem_e820_good
            
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if carry and we haven't gotten any blocks yet, or
           ; this service is not allowed, or a different error.
           ; if memory.blocks > 0 and carry set, then we are done
           movzx eax,word es:[esi+S_MEMORY->blocks]
           sub  eax,1            ; if blocks == 0, this will set
           ret                   ;                 the carry flag
            
get_mem_e820_good:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we have a valid block returned in es:edi of length cx
           push ebx
           lea  ebx,[esi+S_MEMORY->block] ; start of memory entries
           movzx eax,word es:[esi+S_MEMORY->blocks]
           mov  ecx,sizeof(S_MEMORY_ENTRY)
           mul  ecx                     ; find next entry
           add  ebx,eax                 ;  and put it in es:bx
           mov  eax,es:[edi + 0]         ; base address (lo)
           mov  es:[ebx+S_MEMORY_ENTRY->base + 0],eax
           mov  eax,es:[edi + 4]         ; base address (hi)
           mov  es:[ebx+S_MEMORY_ENTRY->base + 4],eax
           mov  eax,es:[edi + 8]         ; size in bytes (lo)
           mov  es:[ebx+S_MEMORY_ENTRY->size + 0],eax
           add  es:[esi+S_MEMORY->size + 0],eax ; add to total size
           mov  eax,es:[edi + 12]        ; size in bytes (hi)
           mov  es:[ebx+S_MEMORY_ENTRY->size + 4],eax
           adc  es:[esi+S_MEMORY->size + 4],eax ; add to total size
           mov  eax,es:[edi + 16]        ; type of memory
           mov  es:[ebx+S_MEMORY_ENTRY->type],eax
           inc  word es:[esi+S_MEMORY->blocks]
           pop  ebx
           or   ebx,ebx
           jnz  get_mem_loop
           
           clc
           ret
try_e820_service endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Try the E881 (32-bit) and E801 (16-bit) services
; On entry:
;   es:edi-> segmented address to store the information
; On return:
;   Carry clear if memory service supported
;
try_e8x1_service proc near uses eax ebx ecx edx esi edi

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; first, clear it out.
; we clear it out each try just in case the last try added
;  something, but then errored
           push edi
           xor  al,al
           mov  ecx,sizeof(S_MEMORY)
           rep
             stosb
           pop  edi

           ; the 32-bit version is identical to the 16-bit
           ;  version except that it returns in 32-bit registers
           ;  instead of the 16-bit registers
           mov  eax,0E881h  ; (32-bit)
           int  15h
           jnc  short e881_good
           
           ; the 32-bit version didn't work, so try the 16-bit version
           mov  eax,0E801h
           int  15h
           jc   e801_error
           
           ; if it worked, clear out the high word of each regsiter
           ;  returned since the code below uses 32-bit registers
           movzx eax,ax
           movzx ebx,bx
           movzx ecx,cx
           movzx edx,dx

e881_good:           
           mov  word es:[edi+S_MEMORY->blocks],3
           lea  esi,[edi+S_MEMORY->block] ; start of memory entries
           
           ; this service does not include the first meg, so lets add it.
           mov  dword es:[esi+S_MEMORY_ENTRY->base],00000000h
           mov  dword es:[esi+S_MEMORY_ENTRY->size],00100000h
           mov  dword es:[esi+S_MEMORY_ENTRY->type],1
           add  esi,sizeof(S_MEMORY_ENTRY)
           
           ; if eax and ebx == 0, then use ecx and edx
           or   eax,eax
           jnz  short @f
           or   ebx,ebx
           jnz  short @f
           mov  eax,ecx
           mov  ebx,edx
           
@@:        shl  eax,10     ; * 1k
           mov  dword es:[esi+S_MEMORY_ENTRY->base],00100000h
           mov  es:[esi+S_MEMORY_ENTRY->size],eax
           mov  dword es:[esi+S_MEMORY_ENTRY->type],1
           add  esi,sizeof(S_MEMORY_ENTRY)
           
           shl  ebx,16     ; * 64k
           mov  dword es:[esi+S_MEMORY_ENTRY->base],01000000h
           mov  es:[esi+S_MEMORY_ENTRY->size],ebx
           mov  dword es:[esi+S_MEMORY_ENTRY->type],1
           
           clc
e801_error:
           ret
try_e8x1_service endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Try the 88 service
; On entry:
;   es:edi-> segmented address to store the information
;   ds:esi-> segmented address of a spare buffer at least 42 bytes in size
; On return:
;   Carry clear if memory service supported
;
try_88_service proc near uses eax ebx ecx edx esi edi

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; first, clear it out.
; we clear it out each try just in case the last try added
;  something, but then errored
           push edi
           xor  al,al
           mov  ecx,sizeof(S_MEMORY)
           rep
             stosb
           pop  edi
           
           mov  ah,88h
           int  15h
           jc   e88_error
           
           mov  word es:[edi+S_MEMORY->blocks],2
           lea  ebx,[edi+S_MEMORY->block] ; start of memory entries

           ; this service does not include the first meg, so lets add it.
           mov  dword es:[ebx+S_MEMORY_ENTRY->base],00000000h
           mov  dword es:[ebx+S_MEMORY_ENTRY->size],00100000h
           mov  dword es:[ebx+S_MEMORY_ENTRY->type],1
           add  ebx,sizeof(S_MEMORY_ENTRY)
           
           movzx eax,ax
           shl  eax,10     ; * 1k
           mov  dword es:[ebx+S_MEMORY_ENTRY->base],00100000h
           mov  es:[ebx+S_MEMORY_ENTRY->size],eax
           mov  dword es:[ebx+S_MEMORY_ENTRY->type],1
           
           ; now see if service C7h is available
           ; on return:
           ;  carry clear and ah = 0
           ;   es:bx->rom table
           push es
           push ebx
           mov  ah,0C0h
           int  15h
           jc   short service_88_done
           or   ah,ah
           jnz  short service_88_done
           
           ; service C7h is available if bit 4 in feature_byte 2
           ; (offset 06h) is set.
           mov  al,es:[bx+6]
           and  al,00010000b
           jz   short service_88_done
           pop  ebx
           pop  es
           
           ; service C7h is supported, so get the rest
           ; passed ds:si -> temp buffer
           mov  ah,0C7h
           int  15h
           jc   short service_88_done
           
           ; dword at 0Ah = system memory between 1M and 16M, in 1K blocks
           ; dword at 0Eh = system memory between 16M and 4G, in 1K blocks
           mov  eax,[si+0Ah]
           add  eax,[si+0Eh]
           shl  eax,10
           mov  es:[ebx+S_MEMORY_ENTRY->size],eax
           push es
           push ebx
           
           ; since the first service, 88h, passed, we return carry clear
           ;  even if the rest of them didn't pass.
service_88_done:
           pop  ebx
           pop  es
           clc
e88_error:
           ret
try_88_service endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Use the CMOS technique
; On entry:
;   es:edi-> segmented address to store the information
; On return:
;   Carry clear if memory service supported
;
get_cmos_tech proc near uses eax ebx ecx edx esi edi

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; first, clear it out.
; we clear it out each try just in case the last try added
;  something, but then errored
           push edi
           xor  al,al
           mov  ecx,sizeof(S_MEMORY)
           rep
             stosb
           pop  edi
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now try the cmos value
           ;  reg 17h = low byte
           ;  reg 18h = high byte  (in kb's)
           mov  al,18h
           out  70h,al
           in   al,71h
           mov  ah,al
           mov  al,17h
           out  70h,al
           in   al,71h
           
           mov  word es:[edi+S_MEMORY->blocks],2
           lea  ebx,[edi+S_MEMORY->block] ; start of memory entries
           
           ; this service does not include the first meg, so lets add it.
           mov  dword es:[ebx+S_MEMORY_ENTRY->base],00000000h
           mov  dword es:[ebx+S_MEMORY_ENTRY->size],00100000h
           mov  dword es:[ebx+S_MEMORY_ENTRY->type],1
           add  ebx,sizeof(S_MEMORY_ENTRY)
           
           movzx eax,ax
           shl  eax,10     ; * 1k
           mov  dword es:[ebx+S_MEMORY_ENTRY->base],00100000h
           mov  es:[ebx+S_MEMORY_ENTRY->size],eax
           mov  dword es:[ebx+S_MEMORY_ENTRY->type],1
           
           clc
           ret
get_cmos_tech endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the print routines and other items
include '..\include\stdio.asm'


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strings and other data
hdr_copy_str     db  13,10,'MEM_INFO   v1.00.00    Forever Young Software 1984-2013',0

service_E820_str db 13,10,10,'Trying Service E820',0
service_E8x1_str db 13,10,10,'Trying Service E881/E801',0
service_88_str   db 13,10,10,'Trying Service 88',0
using_cmos_str   db 13,10,10,'Using the values in the CMOS',0
no_services      db 13,10,10,'None of the services worked!',0

blocks_found_str db 13,10,10,'Total Blocks Found: ',0
base_str         db '.  Base: 0x',0
size_str         db '   Size: 0x',0
type_str         db '   Type: ',0
total_str        db '                            Total Size: (',0
crlf             db 13,10,0

; the block of memory we use to store the information
our_memory_info  st S_MEMORY

; temp buffer
temp_buff        dup 42,0

.end

                                                                                                                 
