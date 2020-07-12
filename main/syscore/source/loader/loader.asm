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
* FILE: loader.asm                                                         *
*                                                                          *
* Loader code for the FYS OS version 2.0 operating system.                 *
*                                                                          *
*  Built with:  NBASM ver 00.26.52                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update:  23 June 2013                                               *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* This loader is for the FYSFS file system only.                           *
*                                                                          *
* Loading of files:                                                        *
*  The kernel file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*   (Same with any other pre-kernel files)                                 *
*                                                                          *
* This loader program has been loaded via the FYSFS.BIN file (boot sector) *
*  and resides at 3000:0000h (0x30000).  It can use any memory above       *
*  this position.                                                          *
*                                                                          *
* The bootsector passed the address of a data block in the DS:SI register  *
*  pair.  Each FS boot, if needed, set these values for use.  See the      *
*  S_BOOT_DATA structure for more information.                             *
*  We need to save the respective values for later.                        *
*  ROOT of current disk is at:  ROOTSEG                                    *
*   FAT of current disk is at:  FATSEG                                     *
*   (see boot.inc)                                                         *
*                                                                          *
* Boot has also left us a stack at ss=07C0h which is 16k in size.          *
*                                                                          *
* With this loader, we do the following in the following order:            *
*  Check for 386 code making sure not to destroy es:di                     *
*  Load system files                                                       *
*  Sets our GDT and IDT                                                    *
*  Move to PMODE                                                           *
*  Jump to kernel                                                          *
*                                                                          *
****************************************************************************
*                                                                          *
* Assumptions:                                                             *
*  - the boot code has already checked that this disk supports the BIOS    *
*    Extended Read Service.  No need to check again.                       *
*                                                                          *
***************************************************************************/

; offset within LOADSEG of our buffer
; needs to be 512 * clusters/sector long, and not cross a 64k boundary
; If our loader.sys code > BUFFER, then this will overwrite it.
BUFFER_SIZE_IN_SECTORS equ 40
BUFFER            equ  08000h
BUFFER1           equ  0D000h
BUFFER2           equ  0E000h
BUFFER3           equ  0F000h

.optoff                        ; we don't want anything changed unknowingly
.model tiny                    ; make it a flat binary

include 'loader.inc'           ; include file(s)
include '..\include\boot.inc'  ;
include '..\include\fysfs.inc' ;

outfile 'loader.sys'           ; name of our target file

.code                          ;
.rmode                         ; bios starts with (un)real mode
.8086                          ; only allow 80x86 code at start
                               ;
           org  00h            ; flat binary
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our loader code
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the seg registers (real mode)
           push di
           push ax
           mov  ax,LOADSEG         ; set ds and es to LOADSEG
           mov  es,ax              ;
           push ax                 ; save ax for ds later
           push cx
           mov  al,cl              ; cl = fs type
           mov  di,offset boot_data ; ds:si = 07C0:xxxxh
           mov  cx,sizeof(S_BOOT_DATA)
           cld
           rep                     ; store BOOT BPB from passed BOOT
           movsb                   ;
           pop  cx
           pop  ds                 ; ds now = LOADSEG from above
           pop  ax
           pop  di
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  BOOT has already set a 16k stack at 07C0h
           ;   (current ss:sp should point to 07C0:4000h)
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set to screen 03h
           mov  ax,0003h           ; make sure we are in screen mode 03h
           int  10h                ; 80x25
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; turn off the cursor.
           ; FYSOS turns it back on after keyboard initialization.
           mov  ax,0103h           ; al = current screen mode (bug on some BIOSs)
           mov  ch,00100000b       ; bits 6:5 = 01
           int  10h

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           ;   Returns processor type in AX
           mov  si,offset chk_486_str
           call display_string
           call chk_486            ; check to see if 486+ with RDTSC
           jnc  short is_486       ; carry set = no 486+
           mov  si,offset not486str
           call display_string
           mov  si,offset not486_off
           add  si,ax
           add  si,ax
           mov  si,[si]
           call display_string
           mov  si,offset not486bios
           call display_string
           xor  ah,ah              ; Wait for keypress
           int  16h                ;
           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.486       ; allow processor specific code for the 486
is_486:

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; A20 line

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now we need to make sure the a20 line is active
           ;  and save the technique number used
           call set_a20_line
           mov  a20_tech,al

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Setup of Un-Real mode and point fs:edi to KERNEL_BASE
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; load a temp GDT
           cli
           lgdtf [gdtoff_temp]
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Switch to protected mode
           mov  eax,CR0
           or   al,01h
           mov  CR0,eax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Here is the jump.  Intel recommends we do a jump.
           db  0EAh
           dw  offset unreal_mode     ; not in pmode yet, so word sized
           dw  FLATCODE16

unreal_mode:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; load segment with a temp base of 0x00000000
           mov  eax,FLATDATA16     ; Selector for 4Gb data seg
           mov  fs,ax              ; 
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Switch back to real mode
           mov  eax,CR0
           and  eax,7FFFFFFEh
           mov  CR0,eax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Here is the jump.  Intel recommends we do a jump.
           db  0EAh
           dw  offset real_mode     ; in rmode, so word sized
           dw  LOADSEG
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
real_mode:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; reload fs with a 16-bit segment of 0x0000
           ; the limit is still set at 4gig.
           xor  ax,ax
           mov  fs,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; allow interrupts again
           sti

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; get the memory size from the bios (int 15h/e820/e801/88 or cmos)
           push LOADSEG
           pop  es

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; first, clear it out.
           mov  di,offset memory
           xor  al,al
           mov  cx,sizeof(S_MEMORY)
           cld
           rep
           stosb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now try the most recent/best service
           xor  ebx,ebx
get_mem_loop:
           mov  eax,0000E820h
           mov  edx,534D4150h  ; 'SMAP'
           mov  ecx,00000020h
           mov  di,BUFFER
           int  15h
           jnc  short get_mem_e820_good

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if carry and we haven't gotten any blocks yet,
           ; this service is not allowed, or a different error
           cmp  word memory.blocks,0
           jz   short get_mem_e801

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if memory.blocks > 0 and carry set, then we are done
           jmp  get_mem_done

get_mem_e820_good:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we have a valid block returned in es:di of length cx
           mov  si,offset memory.block
           mov  ax,memory.blocks
           shl  ax,4
           add  si,ax
           mov  eax,[di]
           mov  [si],eax
           mov  eax,[di+4]
           mov  [si+4],eax
           mov  eax,[di+8]
           mov  [si+8],eax
           add  [memory.size],eax
           mov  eax,[di+12]
           mov  [si+12],eax
           adc  [memory.size+4],eax
           inc  word memory.blocks
           mov  word memory.typeused,1
           or   ebx,ebx
           jnz  short get_mem_loop
           jmp  get_mem_done

get_mem_e801:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if service E820 didn't work, try service E801
           ; first, clear it out (again).
           mov  di,offset memory
           xor  al,al
           mov  cx,sizeof(S_MEMORY)
           cld
           rep
           stosb

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now do the service
           mov  ax,0E801h
           int  15h
           jc   short get_mem_88
           or   ax,ax
           jnz  short @f
           or   bx,bx
           jnz  short @f
           mov  ax,cx
           mov  bx,dx
@@:        xor  edx,edx
           and  eax,0000FFFFh
           shl  eax,10
           shl  ebx,16
           add  eax,ebx
           adc  edx,0
           mov  [memory.size],eax
           mov  [memory.size+4],edx
           mov  word memory.typeused,2
           jmp  short get_mem_done

get_mem_88:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now try service 88h
           ;  service 88h only returns up to 15 meg.
           ;  we need to try service C7h for the rest.
           mov  ah,88h
           int  15h
           jc   short get_mem_cmos

           xor  edx,edx
           and  eax,0000FFFFh
           shl  eax,10
           mov  [memory.size],eax
           mov  [memory.size+4],edx

           ; now see if service C7h is available
           ; on return:
           ;  carry clear and ah = 0
           ;   es:bx->rom table
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
           
           ; serive C7h is supported, so get the rest
           mov  ah,0C7h
           mov  si,BUFFER    ; at least 42 byte buffer
           int  15h
           jc   short service_88_done
           
           ; dword at 0Eh = system memory between 16M and 4G, in 1K blocks
           xor  edx,edx
           mov  eax,[si+0Eh]
           shl  eax,10
           add  [memory.size],eax
           adc  [memory.size+4],edx
           
service_88_done:
           mov  word memory.typeused,3
           jmp  short get_mem_done

get_mem_cmos:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now try the cmos value
           ;  reg 17h = low byte
           ;  reg 18h = high byte  (in kb's)
           mov  al,18h
           out  70h,al
           in   al,71h
           mov  dl,al
           mov  al,17h
           out  70h,al
           in   al,71h
           mov  ah,dl
           and  eax,0000FFFFh
           shl  eax,10
           mov  [memory.size],eax
           xor  eax,eax
           mov  [memory.size+4],eax
           mov  word memory.typeused,4
get_mem_done:


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now get the vesa video info from the BIOS.

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; clear the buffer first
           mov  ax,LOADSEG
           mov  es,ax
           mov  di,offset vesa_video
           xor  ax,ax
           mov  cx,128
           rep
           stosw

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; call the BIOS, first setting the first 4 bytes to 'VBE2'
           ; Since it returns 512 bytes, we need to use a spare buffer.
           mov  di,BUFFER
           mov  dword [di],'2EBV'
           mov  ax,4F00h
           int  10h
           cmp  ax,004Fh           ; was it successful?
           je   short @f
           mov  dword [di],00000000h
@@:        mov  si,BUFFER
           mov  di,offset vesa_video
           mov  cx,128
           rep
           movsw

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now get the supported modes.  We have to do this incase
           ;  the BIOS builds the list on the fly (Bochs BIOS does this)
           push ds
           lds  si,[BUFFER+0Eh]
           mov  di,offset vesa_modes
           mov  cx,63
           rep
           movsw
           mov  ax,0FFFFh
           stosw
           pop  ds

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now get the oem name.  We have to do this incase the BIOS
           ; builds the list on the fly.
           push ds
           lds  si,[BUFFER+06h]
           mov  di,offset vesa_oem_name
           mov  cx,32
           rep
           movsb
           xor  al,al
           stosb
           pop  ds

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; since we got the modes, let's zero out the mode list ptr
           mov  dword [vesa_video+0Eh],00000000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now load the system file(s)
;  Search for the files in root directory

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the loading string
@@:        mov  si,offset krnl_load_str
           call display_string

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  initialize the crc check
           call crc32_initialize

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  set gs: to the loaders root segment we got in the boot code
           push FYSFS_ROOTSEG      ; gs -> to ROOT memory area
           pop  gs                 ;
           
           ; si-> first system file to load
           ; si is saved through out the process below
           ; it should be preserved when ever used
           mov  si,offset system_files
more_files:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure that ds and es are correct
           mov  ax,LOADSEG         ; 
           mov  ds,ax              ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the status string
           push si
           mov  si,offset progress_str
           call display_string
           pop  si
           call display_string

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure ds = LOADSEG
           mov  ax,LOADSEG
           mov  ds,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; find the entry in the root
           xor  ebx,ebx            ; current slot number to look in.
           mov  edi,BUFFER         ; buffer for found name
           mov  ecx,ss:[SUPER_BLOCK + S_FYSFS_SUPER->root_entries] ; count of entries to search
ss_loader: call get_name
           jc   short @f
           call stricmp
           or   al,al
           jz   short fnd_loader
@@:        inc  ebx                ; next slot number
           loop ss_loader
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we get here, the file wasn't found
           mov  si,offset filenotfoundS

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;
KernErr:   push LOADSEG            ; make sure ds = LOADSEG
           pop  ds
           call display_string     ;
KernErr1:  mov  si,offset anykeyrebootS
           call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           xor  ax,ax              ; No warm boot
           mov  es,ax              ;
           mov  si,472h            ;
           mov  es:[si],ax         ;

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Walk the (FYS)FAT chain reading all 'clusters' of the file.
           ; ebx = first dir entry of file
fnd_loader:                        
           mov  edi,BUFFER1         ; buffer for found 64-bit fat entries
           call get_fat_entries    ; get all fat entries, returns ax = entries
           jnc  short @f
           mov  si,offset fat_slot_errorS
           jmp  short KernErr

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; get the file size
@@:        shl  ebx,7              ; multiply by 128
           mov  ebx,gs:[ebx + S_FYSFS_ROOT->fsize]  ; ebx = files size in bytes

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; move to the next filename for next time.
           push eax            ; save count of fat entries found
@@:        lodsb               ;
           or   al,al          ;
           jnz  short @b       ;
           pop  eax            ; restore count of fat entries found
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  We now have the count of entries in EAX and
           ;  the list of entries in ds:edi
           movzx ecx,word ss:[SUPER_BLOCK + S_FYSFS_SUPER->sect_clust]  ; number of sectors to read (per cluster)
           push LOADSEG            ; es-> loader seg
           pop  es                 ;
           
           push ebx
           push si
           mov  ebx,eax            ; count of fat entries found
           xor  bp,bp              ; flag so we use the header only once
           
           mov  esi,BUFFER1
fys_fat_l: mov  eax,[esi+0]
           mov  edx,[esi+4]
           add  esi,8
           mul  ecx   ; cluster based
           add  eax,ss:[SUPER_BLOCK + S_FYSFS_SUPER->data + 0]
           adc  edx,ss:[SUPER_BLOCK + S_FYSFS_SUPER->data + 4]
           call read_sectors_long   ; read in clusters (saves all registers used)

           push esi                 ; save the esi (offset to files to load)
           push cx                  ; save cluster size
           mov  esi,BUFFER          ; location of read file data
           shl  cx,7                ; cx * 512 / 4 = (shl 7)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we only need to check for the header stuff once per file
           or   bp,bp
           jnz  short not_hdr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; here is were we would check the header of the
           ;  first cluster read to make sure it is a valid
           ;  system file.  
           ; Get the header and check it's info.
           mov  eax,[BUFFER+S_LDR_HDR->id]
           cmp  eax,'FYS2'
           je   short hdr_id_ok
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print bad header and move to next file
bad_hdr:   mov  si,offset bad_file_hdr_str
           call display_string
           pop  cx              ; clear the stack of cx & esi
           pop  esi
           jmp  short next_file
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; here is were we would check the Header CRC, File CRC32,
           ;  halt on error, and compression type.  
           ; For the purpose of this book I am leaving that for the reader.
hdr_id_ok:
                      
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; move the read 'base' into edi
           mov  edi,[BUFFER+S_LDR_HDR->location]
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the IS_KERNEL flag is set, we need to update
           ;  the kernel location settings.
           mov  al,[BUFFER+S_LDR_HDR->flags]
           test al,IS_KERNEL
           jz   short not_kernel

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set the kernel base and jump address
           mov  kern_base,edi
           mov  jmp_off,edi
           add  dword jmp_off,DATA_SIZE  ; size of our data transferred
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make that the kernel was loaded
           mov  byte kernel_fnd,TRUE

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
not_kernel:; skip over header and decrement bytes read by size of header
           add  esi,sizeof(S_LDR_HDR)
           sub  cx,(sizeof(S_LDR_HDR) >> 2)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set flag to indicate next loops are not header loops
           inc  bp
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now move the data from BUFFER to read base (edi)
not_hdr:   lodsd
           mov  fs:[edi],eax
           add  edi,4
           loop not_hdr
           pop  cx
           pop  esi
           
           dec  ebx
           jnz  fys_fat_l

next_file: pop  si
           pop  ebx

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; move to the next file specification and see if there are more
           ; if there is a null char, then we are done
           cmp  byte [si],0        ; if no filename
           jne  more_files         ;  then there are more files to load

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we didn't load the kernel, no need to jump to it!
           cmp  byte kernel_fnd,TRUE
           je   short @f
           mov  si,offset no_kernel_fnd_str
           call display_string
freeze:    hlt
           jmp  short freeze

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Kernel file is loaded to KERNEL_BASE
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure ds = LOADSEG
@@:        mov  ax,LOADSEG
           mov  ds,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; We need to move the current INT 1Eh table to RAM
           ;  and set the Max SPT to 36.  Many BIOS's default
           ;  disk parameter tables will not recognize multi-sector
           ;  reads beyond the maximum sector number specified in
           ;  the default diskette parameter tables.  This may
           ;  mean 7 sectors in some cases.
           ; We need to also save the original INT 1Eh table
           ;  address so that we can restore it on (warm) reboot.
           push ds
           xor  ax,ax
           mov  es,ax
           mov  ax,es:[(1Eh*4)+2]
           mov  si,es:[(1Eh*4)]
           mov  [org_int1e+2],ax
           mov  [org_int1e+0],si
           mov  ds,ax
           mov  di,0522h
           mov  es:[(1Eh*4)+2],es
           mov  es:[(1Eh*4)+0],di
           mov  cx,sizeof(S_FLOPPY1E)
           cld
           rep
           movsb
           mov  byte es:[di-7],36  ; set max spt to 36 (the most a 2.88 would have)
           xor  ax,ax
           mov  ds,ax
           mov  si,0522h
           mov  ax,LOADSEG
           mov  es,ax
           mov  di,offset floppy_1e
           mov  cx,sizeof(S_FLOPPY1E)
           rep
           movsb
           pop  ds
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Reset the disk drive so it loads the new int 1Eh table values
           xor  ah,ah
           mov  dl,0   ; drive doesn't matter
           int  13h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Get the bios equipment list 16-bit word value
           ; Get the keyboard status bits at 0040:0017
           push es
           mov  ax,0040h
           mov  es,ax
           mov  ax,es:[0010h]
           mov  bios_equip,ax
           mov  al,es:[0017h]
           mov  kbd_bits,al
           pop  es

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We run through all of the disk drives with a BIOS drive number of 80h
;  or higher and store the (extended) disk parameters.
           call get_drv_parameters

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; one of the last things we need to do before we jump,
           ; is get the time from the bios.
           mov  ah,04h
           int  1Ah
           push dx
           xor  ah,ah
           mov  al,ch           ; century
           call bcd2dec         ;
           mov  bx,100          ;
           mul  bx              ;
           xor  ch,ch           ;
           xchg cx,ax           ; save century * 100 and place year in al
           call bcd2dec         ;
           xchg cx,ax           ; restore centry * 100 in ax and place year in cl
           xor  ch,ch           ;
           add  ax,cx           ;
           mov  time.year,ax    ;
           pop  dx              ;
           mov  al,dh           ; month
           call bcd2dec         ;
           mov  time.month,al   ;
           mov  al,dl           ; day
           call bcd2dec         ;
           mov  time.day,al     ;
           
           mov  ah,02h
           int  1Ah
           mov  al,ch           ; hours
           call bcd2dec
           mov  time.hour,al
           mov  al,cl           ; minutes
           call bcd2dec
           mov  time.min,al
           mov  al,dh           ; seconds
           call bcd2dec
           mov  time.sec,al
           mov  time.d_savings,dl  ; daylight savings
           mov  time.jiffy,dl
           mov  byte time.weekday,0
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; see if we have and APM BIOS
           mov  di,offset apm     ; clear out the APM struct
           mov  cx,sizeof(S_APM)  ;  this will set present = FALSE
           xor  al,al             ;        and initialized = FALSE
           rep                    ;
           stosb                  ;
    
           mov  ax,5300h          ; apm installation check
           xor  bx,bx             ;
           int  15h               ;
           jc   short no_apm_bios ; if carry set, no APM installed
           cmp  bx,'PM'           ; make sure BH = 'P', BL = 'M'
           jne  short no_apm_bios ;
    
           ; found APM bios, so store info
           mov  byte apm.present,1
           mov  apm.maj_ver,ah
           mov  apm.min_ver,al
           mov  apm.flags,cx

           ; get capabilities
           mov  ax,5310h
           xor  bx,bx
           int  15h
           jc   short bad_apm_bios
           mov  apm.batteries,bl
           mov  apm.cap_flags,cx

           ; now connect to the 32bit interface
           mov  ax,5303h
           xor  bx,bx
           int  15h
           jc   short bad_apm_bios
           mov  apm.cs_segment32,ax
           mov  apm.entry_point,ebx
           mov  apm.cs_segment,cx
           mov  apm.ds_segment,dx
           mov  eax,esi
           mov  apm.cs_length32,ax
           shr  eax,16
           mov  apm.cs_length16,ax
           mov  apm.ds_length,di
           jmp  short no_apm_bios
bad_apm_bios:
           mov  byte apm.present,0
           mov  apm.error_code,ax   ; ah = error_code, al = function
no_apm_bios:
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; move the actual GDT and IDT and the DATA_SIZE byte block
           ; of data we have created through out this loader process
           mov  si,offset gdtoff
           mov  edi,kern_base
           mov  cx,(DATA_SIZE>>2)
gdt_mov_it: lodsd
           mov  fs:[edi],eax
           add  edi,4
           loop gdt_mov_it

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure the floppy disk drive motor is turned off.
           ;  put a 1 at 0x00440 so that the next time the BIOS
           ;  decrements it, it will turn off the motor.
           push es
           mov  ax,40h
           mov  es,ax

           ; first check to see that [0x00440] > 0, if not, don't
           ;  do check this code.  It seems that the Compaq BIOS 
           ;  does not do the "decrement" if the drive is not turning.
           ;  It may only hook the timer tick interrupt when the drive
           ;  is turning, and when it counts to 0, it unhooks itself.
           cmp  byte es:[40h],0
           je   short drive_no_wait

           mov  byte es:[40h],1
           ; we need to wait for this to actually happen since
           ;  the cli below won't allow the timer tick interrupt to fire
           ; make sure the interrupt flag is set or we will wait forever
           sti
tick_wait: cmp  byte es:[40h],0
           jne  short tick_wait

drive_no_wait:
           pop  es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; clear the screen
           mov  ax,0003h           ; make sure we are in screen mode 03h
           int  10h                ; 80x25

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; turn off the cursor.
           ; FYSOS turns it back on after keyboard initialization.
           mov  ax,0103h           ; al = current screen mode (bug on some BIOSs)
           mov  ch,00100000b       ; bits 6:5 = 01
           int  10h

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; load the actual GDT and IDT
           ; we don't have a valid IDT yet, but since we turn off interrupts
           ;  right here, and don't turn them back on until after the kernel
           ;  initializes a valid IDT, we can get away with this.
           cli
           lgdtf [gdtoff]
           lidtf [idtoff]  

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Switch to protected mode
           mov  eax,CR0

           and  eax,(~60000000h)  ; clear CD and NW bits

           or   al,21h     ; and set the NE bit
           mov  CR0,eax
          
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Here is the jump.  Intel recommends we do a jump.
           db  66h
           db  0EAh
           dd  (prot_mode + 30000h)
           dw  FLATCODE

still_in_16bit:
           mov  si,offset didnt_make_it
           call display_string
@@:        jmp  short @b

didnt_make_it db 13,10,"We didn't make it into pmode...",0


.pmode
prot_mode: ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; let's make sure we made it here
           ; if still in 16bit mode,
           ;  the below will look like this -> ; xor  ax,ax
           xor eax,eax                         ; xor  ax,0000h
           xor eax,0C08B0000h                  ; mov  ax,ax
           jz  short still_in_16bit            ; jz   short still_in_16bit

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we need to make sure that CR4.TSD = 0 (bit 2)
           ; i.e.: allows the RDTSC instruction
           ; we need to make sure that CR4.VME = 0 (bit 1)
           mov  eax,cr4
           and  al,(~3)
           mov  cr4,eax
          
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  set up the segment descriptors
           ;  ds, es, fs, and ss have a base of 00000000h
           mov  ax,FLATDATA        ; Selector for 4Gb data seg
           mov  ds,ax              ;  ds
           mov  es,ax              ;  es
           mov  fs,ax              ;  fs
           mov  gs,ax              ;  gs
           mov  ss,ax              ;  ss
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  set up a stack at STACK_BASE (physical) of 4 meg size
           mov  esp,((STACK_BASE + STACK_SIZE) - 4)
            
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now have PMODE setup and all our segment selectors correct.
; CS              = 0x00000000
; SS & remaining  = 0x00000000

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Here is the jump.
           ; We will jump to physical address KERNEL_BASE + DATA_SIZE
           db  0EAh
jmp_off    dd  0     ; *in* pmode, so *dword* sized
           dw  FLATCODE

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Kernel file should have taken over from here
           ;   but to be sure
           ; TODO: print something 'we didn't make it dude'
           jmp  short $
           
           
.rmode
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Checks for a 486+ machine with the CPUID and RDTSC instructions.
;  on entry: nothing
;  on exit:
;    carry set = NOT a 486+, carry clear = 486+ with CPUID and RDTSC
;    ax = 0 = 8086, 1 = 186, 2 = 286, 3 = 386,
;         4 = 486+ without CPUID, 5 = 486+ with CPUID but not RDTSC,
;         5 = 486+ with CPUID and RDTSC
chk_486    proc near uses bx cx dx
           pushf                   ; save the original flags value

           mov  ax,00h             ; Assume an 8086
           mov  cx,0121h           ; If CH can be shifted by 21h,
           shl  ch,cl              ; then it's an 8086, because
           jz   short @f           ; a 186+ limits shift counts.
           push sp                 ; If SP is pushed as its
           pop  ax                 ; original value, then
           cmp  ax,sp              ; it's a 286+.
           mov  ax,01h             ; is 186
           jne  short @f           ;
           mov  ax,7000h           ; if bits 12,13,14 are still set
           push ax                 ; after pushing/poping to/from
           popf                    ; the flags register then we have
           pushf                   ; a 386+
           pop  ax                 ;
           and  ax,7000h           ;
           cmp  ax,7000h           ;
           mov  ax,02h             ; is 286
           jne  short @f           ; it's a 386

           ; =-=-=-=-=- test for .486
           ; if we can toggle bit 18 in EFLAGS (AC bit) we have a
           ;  486+.  The 386 doesn't have the AC bit.
           cli
           pushfd
           pop  eax
           mov  ebx,eax
           xor  eax,00040000h      ; bit 18
           push eax
           popfd
           pushfd
           pop  eax
           push ebx
           popfd
           sti            
           xor  eax,ebx
           mov  ax,03              ; is 386
           jz   short @f           ; else it's a 486+

           ; =-=-=-=-=- test for CPUID
           ; if we can toggle bit 21 in EFLAGS (ID bit) we have a
           ;  486+ with the CPUID instruction
           cli
           pushfd
           pop  eax
           mov  ebx,eax
           xor  eax,00200000h      ; bit 21
           push eax
           popfd
           pushfd
           pop  eax
           push ebx
           popfd
           sti            
           xor  eax,ebx
           mov  ax,04              ; is 486+ without CPUID
           jz   short @f           ; else it's a 486+ with CPUID

           ; =-=-=-=-=- test for RDTSC
           ; do a CPUID with function 1.  If bit 4 of EDX on return,
           ;  we have the RDTSC instruciton
           mov  eax,1
           cpuid
           test edx,10h
           mov  ax,05              ; is 486+ with CPUID but without RDTSC
           jz   short @f

           ; =-=-=-=-=- We got a 486+ with the CPUID and RDTSC instructions
           popf                    ; restore the original flags value
           mov  ax,06              ; is 486+ with CPUID and RDTSC
           clc                     ; so clear the carry and 
           ret                     ;  return

@@:        popf                    ; restore the original flags value
           stc                     ; not a 386+, so set the carry and
           ret                     ;  return
chk_486    endp

not486str   db  13,10,' A 486+ compatible processor with the CPUID and RDTSC instructions is required.'
            db  13,10,' Processor returned is a ',0
not486_off  dw  offset not486_8086
            dw  offset not486_186
            dw  offset not486_286
            dw  offset not486_386
            dw  offset not486_486_cpuid
            dw  offset not486_486_rdtsc
not486_8086 db  '8086 or compatible.',0
not486_186  db  '80186 or compatible.',0
not486_286  db  '80286 or compatible.',0
not486_386  db  '80386 or compatible.',0
not486_486_cpuid  db  '80486 or compatible without CPUID.',0
not486_486_rdtsc  db  '80486 or compatible with CPUID but without RDTSC.',0
not486bios  db  13,10,'Press a key to continue.',0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine scans through all drives 80h or higher, using the byte at
;  0040:0075 as a count of installed hard drives.  If a drive is found at
;  that drive number, it stores the drives parameters.  It checks to see
;  if the extended services are available for that drive.  If so, it gets
;  the extended parameters.  If not, it gets the standard parameters.
; Some BIOS' incorrectly return type in AX instead of AH, so we test for
;  this too.
; On entry: nothing
;  On exit: nothing
;
get_drv_parameters proc near uses alld es

           ; make sure es = ds
           push ds
           pop  es

           ; clear out the struct first
           mov  di,offset drive_params
           mov  cx,(10 * sizeof(S_DRV_PARAMS))
           xor  al,al
           rep
            stosb

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; check to see if there is a disk present at drive number
get_param_loop: 
           mov  dl,cur_drive
           mov  ax,15FFh        ; set al to FFh for bug test below
           int  13h
           jc   get_param_next_drv
           cmp  ax,0003h        ; some BIOS' will set ax = 3 instead of ah = 3
           je   short get_param_good_drv
           cmp  ah,03h
           jne  get_param_next_drv

get_param_good_drv:
           ; start to store some information
           mov  di,cur_param_ptr        ; current table offset in tables

           ; store the drive number
           mov  dl,cur_drive
           mov  [di+S_DRV_PARAMS->drv_num],dl

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; check to see if the BIOS supports large disks for this drive
           mov  ax,4100h                ; BIOS extensions installation check
           mov  bx,55AAh                ; (dl = drive above)
           push di                      ; make sure we save the pointer
           int  13h                     ;
           pop  di                      ;
           jc   short get_param_do_standard
           cmp  bx,0AA55h               ;
           jne  short get_param_do_standard

           ; flag that we did use extended parameters
           mov  byte [di+S_DRV_PARAMS->extended_info],TRUE

           ; get the extended parameters.
           mov  ah,48h
           mov  dl,cur_drive
           lea  si,[di+S_DRV_PARAMS->ret_size]
           mov  word [si],0042h
           mov  word [si+2],0     ; some DELL BIOS' error if this isn't zero
           push di
           int  13h
           pop  di
           jc   short get_param_next_drv

           ; since the parameters at EDD_config_ptr may be at a temp buffer,
           ;  we need to copy them now
           cmp  word [di+S_DRV_PARAMS->ret_size],1Eh
           jb   short get_param_next_drv0
           cmp  dword [di+S_DRV_PARAMS->EDD_config_ptr],0FFFFFFFFh
           je   short get_param_next_drv0

           push ds
           push di
           mov  si,[di+S_DRV_PARAMS->EDD_config_ptr+0]
           mov  ax,[di+S_DRV_PARAMS->EDD_config_ptr+2]
           mov  ds,ax
           lea  di,[di+S_DRV_PARAMS->dpte]
           mov  cx,16
           rep
            movsb
           pop  di
           pop  ds

           jmp  short get_param_next_drv0

get_param_do_standard:
           ; get the standard parameters.
           mov  ah,08h
           mov  dl,cur_drive
           push di
           int  13h
           pop  di
           jc   short get_param_next_drv

           xor  eax,eax
           mov  al,ch
           mov  ah,cl
           shr  ah,6
           mov  [di+S_DRV_PARAMS->cylinders],eax

           mov  al,cl
           and  eax,3Fh
           inc  eax
           mov  [di+S_DRV_PARAMS->spt],eax

           mov  al,dh
           mov  [di+S_DRV_PARAMS->heads],eax

get_param_next_drv0:
           add  word cur_param_ptr,sizeof(S_DRV_PARAMS)

get_param_next_drv:
           ; reset the bus after an ah=15h call
           mov  ah,1
           mov  dl,cur_drive
           int  13h

           ; increment the drive number
           inc  byte cur_drive

           ; check to see if we have done all drives
           inc  byte cur_count
           push es
           xor  ax,ax
           mov  es,ax
           mov  al,cur_count
           cmp  al,es:[0475h]
           pop  es
           jb   get_param_loop

           ret
get_drv_parameters endp

cur_drive     db 80h
cur_count     db 00h
cur_param_ptr dw offset drive_params

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;  edx:eax = starting sector in LBA format
;      ecx = count of sectors to read
read_sectors_long proc near uses eax ebx ecx edx si
           
           mov  si,offset long_packet
           mov  word [si],0010h
           and  cx,7Fh             ; make sure not more than 7Fh
           mov  [si+2],cx         
           mov  word [si+04h],BUFFER
           mov  [si+06h],es
           add  eax,ss:[SUPER_BLOCK + S_FYSFS_SUPER->base_lba + 0]       ; base lba
           adc  edx,ss:[SUPER_BLOCK + S_FYSFS_SUPER->base_lba + 4]
           mov  [si+08h],eax
           mov  [si+0Ch],edx
           mov  ah,42h
           mov  dl,boot_data.drive ; dl = drive
           int  13h
           jnc  short @f
           
           mov  si,offset diskerrorS ; loading message
           jmp  KernErr            ; do error display and reboot

@@:        ret
read_sectors_long endp

long_packet dup 16,0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a char to the screen using the BIOS
;
display_char proc near uses ax bx
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           int  10h                ; output the character
           ret
display_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
;
display_string proc near uses ax si
           cld
display_loop:                      ; ds:si = asciiz message
           lodsb
           or   al,al
           jz   short end_string
           call display_char
           jmp  short display_loop
end_string: ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a 32-bit decimal number to the screen using the BIOS
;
PrtDec     proc near uses eax ebx ecx edx

           or   eax,eax
           jns  short pnot_neg
           push eax
           mov  al,'-'
           mov  ah,0Eh
           xor  bx,bx
           int  10h
           pop  eax
           neg  eax

pnot_neg:  mov  cx,0FFFFh               ; Ending flag
           push cx
           mov  ecx,10
PD1:       xor  edx,edx
           div  ecx                     ; Divide by 10
           add  dl,30h                  ; Convert to ASCII
           push dx                      ; Store remainder
           or   eax,eax                 ; Are we done?
           jnz  short PD1               ; No, so continue
PD2:       pop  ax                      ; Character is now in DL
           cmp  ax,0FFFFh               ; Is it the ending flag?
           je   short PD3               ; Yes, so continue
           mov  ah,0Eh
           xor  bx,bx
           int  10h
           jmp  short PD2               ; Keep doing it
PD3:       ret
PrtDec     endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; converts the bcd numeral in al to a decimal number and returns it in al
;
bcd2dec    proc near uses cx
           mov  cl,al
           shr  al,4
           mov  ch,10
           mul  ch
           and  cl,0Fh
           add  al,cl
           ret
bcd2dec    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; the a20 code is in the a20.inc file
include ..\include\a20.inc

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the name of the current chain in ds:si
;  on entry:
;   ds:edi-> 256 byte buffer to place asciiz name
;   ebx = current slot to start in
;  on return
;   if this slot was a SLOT, and no errors
;     ds:edi-> is filled with asciiz name.
;     carry clear
;   else
;     carry set
get_name   proc near uses eax ebx ecx edx esi edi

           ; ds:edi-> passed buffer
           mov  esi,edi            ;

           shl  ebx,7              ; multiply by 128

           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  get_name_error

           cmp  dword gs:[ebx + S_FYSFS_ROOT->sig],S_FYSFS_ROOT_NEW
           jne  short get_name_error
           
           ; is a SLOT, so start the name
           movzx cx,byte gs:[ebx + S_FYSFS_ROOT->namelen]
           jcxz short next_slot

           xor  edi,edi
@@:        mov  al,gs:[ebx + edi + S_FYSFS_ROOT->name_fat]
           mov  [esi],al
           inc  esi
           inc  edi
           loop @b

next_slot: mov  ebx,gs:[ebx + S_FYSFS_ROOT->name_continue]
           or   ebx,ebx
           jz   short get_name_done

next_cont: shl  ebx,7              ; multiply by 128

           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_name_error

           cmp  dword gs:[ebx + S_FYSFS_CONT->sig],S_FYSFS_CONT_NAME
           jne  short get_name_error

           movzx cx,byte gs:[ebx + S_FYSFS_CONT->count]
           jcxz short get_name_done

           xor  edi,edi
@@:        mov  al,gs:[ebx + edi + S_FYSFS_CONT->name_fat]
           mov  [esi],al
           inc  esi
           inc  edi
           loop @b

           mov  ebx,gs:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont

get_name_done:
           mov  byte [esi],0   ; asciiz it
           clc
           ret

get_name_error:
           stc
           ret
get_name   endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the fat entries of the current chain in ds:si
;  on entry:
;   ds:edi-> buffer to place entries
;   ebx = current slot to start in
;  on return
;   if this slot was a SLOT, all found entries are 64-bit, and no errors
;     ds:edi-> 64-bit fat entries
;     eax = count of entries found
;     carry clear
;   else
;     carry set
get_fat_entries proc near uses ebx ecx edx esi edi ebp

           ; starting count
           xor  ebp,ebp

           ; ds:edi-> passed buffer
           mov  esi,edi            ;

           shl  ebx,7              ; multiply by 128

           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  get_fat_error

           cmp  dword gs:[ebx + S_FYSFS_ROOT->sig],S_FYSFS_ROOT_NEW
           jne  get_fat_error

           ; is a SLOT, so start the fat
           movzx ecx,byte gs:[bx + S_FYSFS_ROOT->fat_entries]
           .adsize
           jcxz short next_slot_fat

           add  ebp,ecx

           ; calculate start of entries.
           lea  edi,gs:[ebx + S_FYSFS_ROOT->name_fat]
           movzx eax,byte gs:[ebx + S_FYSFS_ROOT->namelen]
           add  al,3
           and  al,(~3)
           add  edi,eax
@@:        mov  eax,gs:[edi]
           mov  [esi],eax
           add  esi,4
           xor  eax,eax
           mov  [esi],eax
           add  esi,4
           add  edi,4
           .adsize
           loop @b

next_slot_fat:
           mov  ebx,gs:[ebx + S_FYSFS_ROOT->fat_continue]
           or   ebx,ebx
           jz   short get_fat_done

next_cont_fat:
           shl  ebx,7              ; multiply by 128

           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_fat_error

           cmp  dword gs:[ebx + S_FYSFS_CONT->sig],S_FYSFS_CONT_FAT
           jne  short get_fat_error

           movzx ecx,byte gs:[ebx + S_FYSFS_CONT->count]
           .adsize
           jcxz short get_fat_done

           add  ebp,ecx

           mov  edi,S_FYSFS_CONT->name_fat
get_cont_fats:
           mov  eax,gs:[ebx+edi]
           mov  [esi],eax
           add  edi,4
           add  esi,4
           xor  eax,eax
           test byte gs:[ebx + S_FYSFS_CONT->flags],01
           jz   short @f
           mov  eax,gs:[ebx+edi]
           add  edi,4
@@:        mov  [esi],eax
           add  esi,4
           loop get_cont_fats

           mov  ebx,gs:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont_fat

get_fat_done:
           mov  eax,ebp
           clc
           ret

get_fat_error:
           stc
           ret
get_fat_entries endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Initialize the crc32 code
crc32_initialize proc near uses alld
           
           ; 256 values representing ASCII character codes.
           xor  ecx,ecx
           mov  edi,offset crc32_table
           
init_loop: mov  ebx,8
           call crc32_reflect
           shl  eax,24
           mov  [edi+ecx*4],eax
           
poly_loop: mov  eax,[edi+ecx*4]
           mov  edx,eax
           shl  edx,1
           mov  edx,04C11DB7h
           jc   short @f
           xor  edx,edx
@@:        shl  eax,1
           xor  eax,edx
           mov  [edi+ecx*4],eax
           dec  ebx
           jnz  short poly_loop
           
           push ecx
           mov  ecx,[edi+ecx*4]
           mov  ebx,32
           call crc32_reflect
           pop  ecx
           mov  [edi+ecx*4],eax
           
           inc  ecx
           cmp  ecx,256
           jb   short init_loop
           
           ret
crc32_initialize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Reflection is a requirement for the official CRC-32 standard.
;  You can create CRCs without it, but they won't conform to the standard.
; on entry:
;  ebx = char
;  ecx = reflect
; on exit
;  eax = result
crc32_reflect proc near uses ebx ecx edx

           mov  edx,ecx   ; edx = reflect
           mov  ecx,ebx   ; ecx = char
           mov  ebx,1
           
           xor  eax,eax
           
           ; swap bit 0 for bit 7 bit 1 For bit 6, etc....
reflect_l: shr  edx,1
           jnc  short @f
           
           push ecx
           push edx
           sub  ecx,ebx
           mov  edx,1
           shl  edx,cl
           or   eax,edx
           pop  edx
           pop  ecx
           
@@:        inc  ebx
           cmp  ebx,ecx
           jbe  short reflect_l
           
           ret
crc32_reflect endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; check the CRC of the slot
; on entry
;  gs:ebx-> slot
;  edi = offset of crc field
; on exit
;  zero flag set if crc is okay
chk_crc    proc near uses eax ebx ecx
           
           mov  dl,gs:[ebx+edi]
           mov  byte gs:[ebx+edi],0
           
           mov  eax,0FFFFFFFFh
           mov  ecx,128
           call crc32_partial
           xor  eax,0FFFFFFFFh
           
           ; restore the original crc value
           mov  gs:[ebx+edi],dl
           cmp  al,dl
           
           ret
chk_crc    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; on entry:
;  eax = crc
;  gs:ebx -> buffer (slot)
;  ecx = length of buffer
; on exit
;  eax = new crc
crc32_partial proc near uses ebx edx edi
           
           mov  edi,offset crc32_table
           
@@:        mov  edx,eax
           and  edx,0FFh
           xor  dl,gs:[ebx]
           inc  ebx
           
           shr  eax,8
           xor  eax,[edi+edx*4]
           
           .adsize
           loop @b
           
           ret
crc32_partial endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string compare case insensitive
; on entry:
;  ds:esi-> string0
;  ds:edi-> string1
; on return:
;  al = 0 if equal, -1 if string0[x] < string1[x], 1 if string0[x] > string1[x]
stricmp    proc near uses esi edi

@@:        lodsb
           call to_upper
           mov  ah,al
           mov  al,[edi]
           inc  edi
           call to_upper

           cmp  al,ah
           jne  short stricmp_done
           or   al,al
           jnz  short @b
           ret

stricmp_done:
           jb   short @f
           mov  al,1
           ret

@@:        mov  al,-1
           ret
stricmp    endp

to_upper   proc near
           cmp  al,'a'
           jb   short @f
           cmp  al,'z'
           ja   short @f
           sub  al,32
@@:        ret
to_upper   endp

crc32_table      dup (256*sizeof(dword)),0   ; CRC lookup table array.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data
out_of_memS0  db  13,10,07,'*** Not enough memory ***'
              db  13,10,'       Size in megabytes found: ',0
out_of_memS1  db  'Size of extended memory needed: ',0
diskerrorS    db  13,10,07,'Error reading disk or Non-System Disk',0
filenotfoundS db  13,10,07,'Did not find file...',0
fat_slot_errorS db 13,10,'Found error when parsing FAT entries...',0
anykeyrebootS db  13,10,'Press a key to reboot',0
chk_486_str   db  13,10,'Checking for a 486+ processor with the RDTSC instruction...',0
krnl_load_str db  13,10,'Loading system files...',0
progress_str  db  13,10,'Loading: ',0
bad_file_hdr_str db 13,10,'Found bad header in system file, moving to next file.',0

; this is the list of files we need to load via this loader
; All filenames must be in asciiz format and contain a 32-bit physical address
;  after the name for the place to load the file to
system_files  db  'kernel.sys',0       ; filename
              db  0                    ; denote no more files

kernel_fnd    db  0  ; set to TRUE if one of the files loaded was the kernel
no_kernel_fnd_str db 13,10,'None of the files loaded had the IS_KERNEL flag set. Halting...',0
              
.para
gdtoff_temp dw ((8*3)-1)
           dd  ((LOADSEG<<4) + gdtoff_temp)
           dw  0   ; filler

           ; code16 desc
           dw  0FFFFh     ; -------------> limit 0xFFFF + (byte below)
           dw   0000h     ; ______/------> base at LOADSEG
           db     03h     ; /----/
           db     9Ah     ; |   Code(E/R), S=1, Priv = 00b, present = Yes
           db     00h     ; |   0 (limit), avl = 0, 0, 16-bit, gran = 0
           db     00h     ; /

           ; data16 desc
           dw  0FFFFh     ; -------------> limit 4gig + (byte below)
           dw   0000h     ; ______/------> base at 0x00000000
           db     00h     ; /----/
           db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes
           db     8Fh     ; |   F (limit), avl = 0, 0, 16-bit, gran = 1
           db     00h     ;/


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is the DATA_SIZE byte block that we will move to KERNEL_BASE for
;  the kernel.sys file *after we load the kernel.sys file*
.para
gdtoff     dw  ((256*8)-1)   ; we can make this block the first of the
kern_base  dd  0             ;  gdt since the CPU ignores the first entry
           dw  0

           ; code desc
           dw  0FFFFh     ; -------------> limit 4gig + (byte below)
           dw   0000h     ; ______/------> base at 0x00000000
           db     00h     ; /----/
           db     9Ah     ; |   Code(E/R), S=1, Priv = 00b, present = Yes
           db    0CFh     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1
           db     00h     ; /

           ; data desc (matches cs)
           dw  0FFFFh     ; -------------> limit 4gig + (byte below)
           dw   0000h     ; ______/------> base at 0x00000000
           db     00h     ; /----/
           db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes
           db    0CFh     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1
           db     00h     ;/

           ; tss desc
           dw       ?     ; -------------> limit 4gig + (byte below)
           dw       ?     ; ______/------> base at 0x00000000
           db       ?     ; /----/
           db       ?     ; |   Data(R/W), S=1, Priv = 00b, present = Yes
           db       ?     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1
           db       ?     ;/

           dup ((256-4)*8),0   ; for a total of 256 entries (2048 bytes)

idtoff     dw  ((8*256)-1)  ; 256 = number of interrupts we allow
           dd  IDT_ADDRESS  ; 
           
boot_data  st  S_BOOT_DATA  ; booted data
org_int1e  dd  0            ; original INT1Eh address
floppy_1e  st  S_FLOPPY1E   ; floppies status
time       st  S_TIME       ; current time passed to kernel
apm        st  S_APM        ; Advanced Power Management
bios_equip dw  0            ; bios equipment list at INT 11h (or 0040:0010h)
kbd_bits   db  0            ; bits at 0x00417
memory     st  S_MEMORY     ; memory blocks returned by INT 15h memory services
a20_tech   db  0FFh         ; the technique number used to enable the a20 line
vesa_video dup 256,0        ; the vesa video informtion from int 10h/4f00h
vesa_modes dup 128,0        ; 63 16-bit mode values + ending 0FFFFh
vesa_oem_name dup 33,0      ; 32 + null vesa oem name string
drive_params dup (96*10),0  ; up to 10 hard drive parameter tables
           dup (DATA_SIZE - ($ - gdtoff)),0  ; reserved (padding to DATA_SIZE bytes)
           
.ifne ($ - gdtoff) DATA_SIZE
  %error 1 'transfer block not 4096 bytes in size'
  %print ($ - gdtoff)
.endif

.end
