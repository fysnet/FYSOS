comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: lean_f.asm                                                         *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*   Boot sector code for a leanfs file system.                             *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm lean_f<enter>                               *
*                                                                          *
* Last Updated: 27 Sept 2017                                               *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   This bootsector is written for a LEAN_FS floppy disk.                  *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the \ (root), \BOOT,    *
*   or \SYSTEM\BOOT directory.                                             *
*  Even though the LEAN FS uses 64-bit sectors, since this is for a        *
*   floppy disk, we only need to worry about the 32-bit part.              *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 386 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include ..\boot.inc                ;
include lean.inc                   ; equates and structures

outfile 'lean_f.bin'               ; out file

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ;
           mov  sp,STACK_OFFSET    ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h which is just
                                   ;    under 0C00:0000h where ROOT resides)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           mov  boot_data.drive,dl ; Store drive number
                                   ; (supplied by BIOS startup code)
           sti                     ; allow interrupts again

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           push 0F000h             ; if bits 15:14 are still set
           popf                    ;  after pushing/poping to/from
           pushf                   ;  the flags register then we have
           pop  ax                 ;  a 386+
           and  ax,0F000h          ;
           jnz  short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  short BootErr

not386str  db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf                   ; restore the interrupt bit

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to call the BIOS to get the parameters of the current disk.
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; reset the disk services to be sure we are ready
           xor  ax,ax
           mov  dl,boot_data.drive
           int  13h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; save the Signature and Base LBA to boot_data
           ; (this assumes that the two items are in the same order
           ;  and at the first of boot_data for this to work.
           ;  we do this so that we can save room in the boot code.)
           mov  di,offset boot_data
           mov  si,offset boot_sig
           movsd       ; signature
           movsd       ; lba low dword
           movsd       ; lba high dword
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           xor  ax,ax           ; this service destroys es:di
           mov  es,ax           ;  and has a bug (?) if es:di != 0000h:0000h
           xor  di,di           ;
           mov  ah,08h          ;
           mov  dl,boot_data.drive
           int  13h             ;
           xor  ah,ah           ;
           mov  al,dh           ;
           inc  ax              ;
           mov  Heads,ax        ; number of heads
           mov  al,cl           ;
           and  ax,003Fh        ;
           mov  SecPerTrack,ax  ; sectors per track
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to load the next 32 sectors into memory before we continue.
;  The super will be at lba 1, 2, 3, ..., 32.
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           mov  ebx,0x07E00
           xor  eax,eax         ; eax = 1  (smaller than  'mov  eax,1')
           inc  ax              ;
           mov  cx,32
           call read_sectors

           jmp  remaining_code
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;                        
BootErr:   call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

include ..\services\lba2chs.inc    ; include the LBA to CHS routine
include ..\services\read_chs.inc   ; include the read disk using CHS routine
include ..\services\conio.inc      ; include the display_char and display_string functions

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  These data items must be in the first sector already loaded by the BIOS
boot_data      st S_BOOT_DATA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55

%PRINT (200h-$-4-8-2)               ; 75 bytes free in this area

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The Serial Number required by all FYS bootable bootsectors.
;   It is a dword sized random number seeded by the time of day and date.
;   This value should be created at format time
;  The Base LBA of this boot code.
;  The formatter will update this data for us.
           org (200h-2-8-4)
boot_sig   dd  ?
base_lba   dq  ?

           org (200h-2)
           dw  0AA55h
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Root and search for loader file in root directory
remaining_code:           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  find the super block.  It will be at lba 1, 2, 3, ..., 32
           call find_super
           or   ax,ax
           jnz  short @f
           mov  si,offset NoSuperS
           jmp  BootErr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  the ROOT
@@:        mov  si,ax  ; ax = offset to super from find_super() above
           mov  eax,[si+S_LEAN_SUPER->root_start]
           mov  root_start,eax
           
           ; load the root directory to LEAN_ROOTSEG
           mov  ebx,(LEAN_ROOTSEG << 4)
           call load_file
           
           ; point es to this data
           push LEAN_ROOTSEG
           pop  es
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  the loader file might be in the root directory
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; search the root directory for the loader file
           mov  si,offset loadname ; lean_fs formatted loader file name
           mov  dl,S_DIRENTRY_TYPE_FILE
           call find_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  if not in the root directory, the loader file might be in the 
;  \boot directory
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if not in the root directory, find the 'boot' directory,
           ; then search within it for the loader file
           mov  si,offset bootname ; lean_fs formatted boot directory filename
           mov  dl,S_DIRENTRY_TYPE_DIR
           call find_name
           jnz  short f_try_system
           
           ; found the boot\ directory, so load it and search for the 
           ;  loader file within it
           mov  eax,es:[di+S_LEAN_DIRENTRY->inode]
           mov  ebx,(LEAN_ROOTSEG << 4)
           call load_file
           
           mov  si,offset loadname ; lean_fs formatted loader file name
           mov  dl,S_DIRENTRY_TYPE_FILE
           call find_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  if not in the \boot directory, the loader file might be in the 
;  \system\boot directory
f_try_system:           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if not in the boot directory, find the 'system\boot' directory,
           ; then search within it for the loader file
           ; (need to reload the root directory)
           mov  eax,root_start
           mov  ebx,(LEAN_ROOTSEG << 4)
           call load_file
           
           mov  si,offset systemname ; lean_fs formatted system directory filename
           mov  dl,S_DIRENTRY_TYPE_DIR
           call find_name
           jnz  short f_not_found
           
           ; found the system\ directory, so load it and search for the 
           ;  boot\ director within it
           mov  eax,es:[di+S_LEAN_DIRENTRY->inode]
           mov  ebx,(LEAN_ROOTSEG << 4)
           call load_file
           
           mov  si,offset bootname ; lean_fs formatted boot directory filename
           mov  dl,S_DIRENTRY_TYPE_DIR
           call find_name
           jnz  short f_not_found
           
           ; found the system\boot\ directory, so load it and search for the 
           ;  loader file within it
           mov  eax,es:[di+S_LEAN_DIRENTRY->inode]
           mov  ebx,(LEAN_ROOTSEG << 4)
           call load_file
           
           mov  si,offset loadname ; lean_fs formatted loader file name
           mov  dl,S_DIRENTRY_TYPE_FILE
           call find_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; was not in any of the three directories listed
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the loader.sys file was not found, display error
f_not_found:
           mov  si,offset noloaderS
           jmp  BootErr
                      
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;          
f_loader:  mov  si,offset os_load_str  ; loading message
           call display_string     ; saves all registers
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;          
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; "choose" loader.sys base address
           ;  this returns the address is ebx
           call set_up_address     ; so that we can loader a loader.sys file
                                   ;  > 64k, we pass a physical address in ebx.
           mov  boot_data.loader_base,ebx ; save to our boot_data block too
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now load the loader file to LOADSEG.
           ; (es:di->found root entry)
           mov  eax,es:[di+S_LEAN_DIRENTRY->inode]
           call load_file  ; returns sector count in ax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move the file back to erase the inode
           ;  ax = sectors read
           mov  cx,ax
           shl  cx,5  ; cx = paragraphs to move (<< 9 for bytes then >> 4 for paragraphs == << 5)
           
           ; move past INODE and ea's
           ; since the loader file can be > 64k, we have to allow for roll-over
           mov  si,sizeof(S_LEAN_INODE)
           test dword es:[S_LEAN_INODE->attributes],LEAN_ATTR_INLINEXTATTR
           jz   short @f
           mov  si,512
@@:        shr  ebx,4      ; ebx = loader address from above
@@:        mov  ds,bx
           mov  es,bx
           xor  di,di
           push si
           movsd
           movsd
           movsd
           movsd
           pop  si
           inc  bx
           loop @b
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  So that we can have a DOS INT 21h stub on the front part of
;   the loader, we need to "patch" the 21h vector
           xor  ax,ax
           mov  ds,ax
           mov  eax,(07C00000h + int32vector)
           mov  [(21h * 4)],eax
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  update the Boot Data block
           mov  ax,07C0h
           mov  ds,ax
           mov  byte boot_data.file_system,LEAN
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ebx-> boot data (includes  'booted from drive') (physical address)
           mov  eax,boot_data.loader_base
           shr  eax,4
           mov  ds,ax
           
           cmp  word [0],5A4Dh
           je   short is_dos_exe
           
           push 07C0h              ; 
           pop  ds                 ; ds = 07C0h again
           mov  si,offset exe_error
           jmp  BootErr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; the loader.sys file is a DOS MZ type .exe file
is_dos_exe:
           add  ax,[08h]           ; skip over .exe header
           mov  bx,ax
           add  bx,[0Eh]
           mov  ss,bx              ; ss for EXE
           mov  sp,[10h]           ; sp for EXE
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set up cs:ip
           add  ax,[16h]           ; cs
           push ax
           push word [14h]         ; ip
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;   ebx -> boot data (includes  'booted from drive')
           mov  ebx,07C00h           ; we pass the physical address of BOOT_DATA
           add  ebx,offset boot_data ;  to the loader in the EBX register
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           retf

include ..\services\address.inc    ; include the "choosing" of the loader.sys base address code
include ..\services\stricmp.inc    ; include the stricmp function

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; new interrupt vector for INT 21h
;  this simply (i)returns
int32vector:
           iret


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; load a directory, then search for a name
; on entry:
;  es = segment where to load the directory (LEAN_ROOTSEG)
;  ds:si-> filename to find
;   dl = type of file to find (1 = file, 2 = dir)
; on return:
;  zero flag set if found
;  es:di-> directory entry
;  
find_name  proc near uses eax
           
           mov  ebx,es:[S_LEAN_INODE->file_size]
           mov  di,LEAN_INODE_SIZE
           test dword es:[S_LEAN_INODE->attributes],LEAN_ATTR_INLINEXTATTR
           jz   short s_loader
           mov  di,512
s_loader:  cmp  es:[di+S_LEAN_DIRENTRY->type],dl
           jne  short n_loader
           mov  cx,es:[di+S_LEAN_DIRENTRY->name_len]
           push di
           add  di,12
           
           ; we need to extract the filename and place it
           ;  into a buffer since it isn't guarenteed to be
           ;  null terminated
           push si
           mov  si,offset buffer
@@:        mov  al,es:[di]
           inc  di
           mov  [si],al
           inc  si
           loop @b
           xor  al,al
           mov  [si],al
           pop  si
           
           push es
           push ds
           pop  es
           mov  di,offset buffer
           call stricmp            ; returns zero flag set if equal
           pop  es
           pop  di
           jz   short find_done

n_loader:  movzx ax,byte es:[di+S_LEAN_DIRENTRY->rec_len]
           shl  ax,4
           add  di,ax
           sub  bx,ax
           jnz  short s_loader
           
           or   al,1  ; clear the zero flag
           
find_done: ret
find_name  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Load a file to given memory
; on entry:
;  ebx = physical address to start (remember that the inode struct is included)
;  eax = sector number to inode (first sector)
; on return:
;  eax = sectors read
load_file proc near uses ebx ecx edx ebp si di es
           
           ; we must start by loading at least one sector to get the inode
           mov  cx,1
           call read_sectors       ; read in the first sector of the file
           
           ; get segment of address where the inode was loaded
           push ebx
           shr  ebx,4
           mov  es,bx
           pop  ebx
           xor  di,di
           ; es:di -> inode
           
           ; create a list of sectors from the direct extent list
           ;  and indirect extent list(s)
           ; this works as long as there is 2048 or less extents total
           
           ; extent count in the inode
           movzx cx,byte es:[di+S_LEAN_INODE->extent_count]
           
           ; get next indirect sector number
           mov  edx,es:[di+S_LEAN_INODE->first_indirect]
           
           ; save address to load file to
           ; we don't add 512 to the address since we re-read the inode again
           ;  (it is part of the first extent)
           push ebx
           
           lea  si,[di+S_LEAN_INODE->extent_start]
           lea  bx,[di+S_LEAN_INODE->extent_size]
           mov  di,INDIRECT_BUFF
           
           xor  ebp,ebp  ; loop counter
           
           push eax  ; save the next indirect sector
           
           ; move the sectors start and count of sectors fields
@@:        mov  eax,es:[si]    ; start
           mov  [di],eax
           mov  eax,es:[bx]    ; size
           mov  [di+4],eax
           add  si,8
           add  bx,4
           add  di,8
           inc  ebp
           loop @b
           
           pop  eax
           
           ; if next indirect sector is zero, no more.
           or   edx,edx
           jz   short @f
           
           ; read in 1 sector of indirect extents
           push ax
           mov  ax,(SECT_BUFFER >> 4) ; this works since our boot is at 0:7C00h
           mov  es,ax
           pop  ax
           mov  ebx,SECT_BUFFER
           mov  cx,1
           call read_sectors
           
           ; store the next indirect sector
           mov  edx,es:[S_LEAN_INDIRECT->next_indirect]
           
           ; get the count (at most 38)
           mov  ecx,es:[S_LEAN_INDIRECT->extent_count]
           
           ; point to our buffer
           mov  si,sizeof(S_LEAN_INDIRECT)
           mov  bx,(sizeof(S_LEAN_INDIRECT) + 8)
           jmp  short @b
           
           ; restore original loader address
@@:        pop  ebx
           
           ; ebp now equals extents to read
           ; so save in ecx
           mov  ecx,ebp
           
           ; total sectors read counter
           xor  ebp,ebp
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now do the loading of the file, an extent at a time
           mov  si,INDIRECT_BUFF
@@:        push ecx
           mov  eax,[si]
           mov  ecx,[si+4]
           add  si,8
           call read_sectors
           add  ebp,ecx         ; increment sector count
           
           ; calculate the offset for next read
           mov  eax,512
           mul  ecx
           add  ebx,eax
           
           pop  ecx
           .adsize     ; use ECX as the counter
           loop @b
           
           mov  eax,ebp   ; return the sector count read
           ret
load_file  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Find the super block.  It will reside on a paragraph boundary from
;  0x07C00 to (0x07C00 + (32 * 200h))
; on entry:
;  nothing
; on return:
;  ax = address relative to 0x07C00 of found super.
;     = 0 if error
find_super proc near uses cx dx si

           mov  si,200h

           mov  cx,32
find_loop: push cx
           cmp  dword [si + S_LEAN_SUPER->magic + 0],4E41454Ch
           jne  short find_next

           push si
           add  si,4
           mov  cx,127
           xor  edx,edx
@@:        lodsd
           ror  edx,1
           add  edx,eax
           loop @b
           pop  si

           cmp  edx,[si + S_LEAN_SUPER->checksum]
           jne  short find_next

           pop  cx
           mov  ax,si
           ret

find_next: add  si,512
           pop  cx
           loop find_loop

           xor  ax,ax
           ret
find_super endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data

os_load_str   db  13,10,'Starting FYSOS...',0

noloaderS     db  13,10,'Could not find loader.sys.',0
NoSuperS      db  13,10,'Could not find the Super Block.',0
exe_error     db  13,10,07,'Error with Loader.sys format.',0
loadname      db  'loader.sys',0
systemname    db  'system',0        ; \system directory
bootname      db  'boot',0          ; \boot directory

%PRINT (600h-$)               ; 120 bytes free in this area

; fill remaining with zeros
              dup  (600h-$),0

; data and a temp buffer that does not occupy the binary file
root_start    dup 4,?
buffer        dup 100,?

.end
