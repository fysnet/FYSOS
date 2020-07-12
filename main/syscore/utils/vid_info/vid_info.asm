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
* FILE: vid_info.asm                                                       *
*                                                                          *
*  Built with:  NBASM ver 00.26.44                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update: 5 May 2013                                                  *
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

TOT_MODES  equ  128

include 'vid_info.inc'             ; our include file
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
; call the service to get the info
           mov  esi,offset modes
           mov  edi,offset our_vesa_info
           mov  ebx,offset oem_name
           call vesa_service  ; returns carry set if not supported
           jnc  short print_info

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print error
           mov  esi,offset error_str
           call print_str
           shr  ax,4
           call prt_hex16
           .exit

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the information to the screen
print_info:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the oem name string
           mov  esi,offset oem_name_str
           call print_str
           mov  esi,offset oem_name
           call print_str
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the version returned
           mov  ebx,FALSE  ; no right align
           mov  esi,offset version_str
           call print_str
           movzx eax,word [our_vesa_info + 4]
           push eax
           shr  eax,8
           call prt_dec32
           mov  al,'.'
           call print_char
           pop  eax
           and  eax,0FFh
           call prt_dec32
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the modes supported
           mov  esi,offset modes_str
           call print_str
           mov  esi,offset modes
           call dump_words
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; The S3 specific video mode list can follow the modes above
           ;  If so, it too will end in 0xFFFF
           push esi
           mov  esi,offset crlf
           call print_str
           pop  esi
           
           call dump_words           
           
           .exit


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Get VESA Info service
; On entry: 
;   es:edi-> segmented address to store the information
;   es:esi-> segmented address to store the supported modes
;   es:ebx-> segmented address to store the OEM Name
; On return:
;   Carry clear if memory service supported
;
vesa_service proc near uses eax ebx ecx edx esi edi

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; first, clear it out.
           push edi
           xor  al,al
           mov  ecx,512
           rep
             stosb
           pop  edi
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; If we want only the VESA info, we put 'VESA' in the
           ;  first dword of the buffer.  If we want VBE2, then
           ;  we put 'VBE2'.
           mov  eax,00004F00h
           mov  dword es:[edi],'2EBV'
           int  10h
           jc   short vesa_done
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; on return, if the carry is clear, but ax != 0x004F
           ;  then this wasn't a good call.
           cmp  ax,004Fh
           jne  short vesa_done
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else it was a good call, so we have a valid buffer.
           ; let's extract the supported modes
           ; (we just extract 128 words. The print routine stops at 0xFFFF)
           xchg edi,esi  ; es:edi -> passed pointer to video mode buffer
                         ; es:esi -> vesa info buffer
           push ds
           push esi
           lds  si,es:[esi + 0Eh]
           mov  cx,TOT_MODES
           rep
           movsw
           pop  esi
           pop  ds
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now get the oem name.  We have to do this incase the BIOS
           ; builds the list on the fly.
           mov  edi,ebx         ; es:ebx -> passed as buffer to hold OEM name
           push ds
           lds  si,es:[esi + 06h]
           mov  cx,32
           rep
           movsb
           xor  al,al
           stosb
           pop  ds
           
vesa_done: clc
           ret
vesa_service endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print 16-bit hex values until 0xFFFF reached
; On entry: 
;   es:esi-> segmented address to of the buffer
; On return:
;   es:esi-> to word after 0xFFFF
;
dump_words proc near uses eax ecx edx
           mov  ecx,10       ; 10 per line
@@:        lodsw
           cmp  ax,0FFFFh
           je   short @f
           call prt_hex16
           mov  al,','
           call print_char
           mov  al,' '
           call print_char
           loop @b
           push esi
           mov  esi,offset crlf
           call print_str
           pop  esi
@@:        ret
dump_words endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the print routines and other items
include '..\include\stdio.asm'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strings and other data
hdr_copy_str     db  13,10,'VID_INFO  v1.00.00   Copyright 1984-2013   Forever Young Software',13,10,0

error_str        db  13,10,'Service returned an error: ',0
version_str      db  13,10,' Version returned: ',0
oem_name_str     db  13,10,'OEM Name returned: ',0
modes_str        db  13,10,'  Modes Supported:',13,10,0
crlf             db  13,10,0

; the block of memory we use to store the information
our_vesa_info    dup 512,?
modes            dup (TOT_MODES * sizeof(word)),?
oem_name         dup 33,?

.end

                                                                                                                                
                  
