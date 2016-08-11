comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: lean_hd.asm                                                        *
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
*   EQUates for lean.asm                                                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm lean_hd<enter>                              *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   This bootsector is written for a LEAN_FS floppy disk.                  *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*  Even though the LEAN FS uses 64-bit sectors, since this is for a        *
*   floppy disk, we only need to worry about the 32-bit part.              *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 386 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include boot.inc                   ;
include lean.inc                   ; equates and structures

outfile 'lean_hd.bin'              ; out file

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; to save space in the first sector, and since this doesn't have to be
;  initialized, we reuse this code space for the 26 byte read buffer
read_buffer:

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
           mov  ax,7000h           ; if bits 12,13,14 are still set
           push ax                 ; after pushing/poping to/from
           popf                    ; the flags register then we have
           pushf                   ; a 386+
           pop  ax                 ;
           and  ax,7000h           ;
           cmp  ax,7000h           ;
           je   short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  BootErr

not386str   db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now check for the BIOS disk extentions.
; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive_num
           jc   short @f
           cmp  bx,0AA55h
           jne  short @f
           test cl,1
           jnz  short read_remaining
@@:        mov  si,offset no_extentions_found
           jmp  BootErr

no_extentions_found db  'Requires int 13h extented read service.',0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to call the BIOS to get the parameters of the current disk.
;  so we can pass it on to loader.sys
read_remaining:
           mov  ah,48h
           mov  dl,boot_data.drive
           mov  si,offset read_buffer
           mov  word [si],26
           int  13h
           mov  ax,[si+S_INT13_PARMS->sect_size]
           mov  boot_data.sect_size,ax
           mov  ax,[si+S_INT13_PARMS->heads]
           mov  boot_data.Heads,ax
           mov  ax,[si+S_INT13_PARMS->sect_trck]
           mov  boot_data.SecPerTrack,ax

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to load the next 32 sectors into memory before we continue.
;  The super will be at lba 1, 2, 3, ..., or 32.
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           mov  bx,0200h       ; address 0x07E00
           mov  eax,1          ;
           cdq
           mov  cx,32
           call read_sectors_long

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the root directory into memory.  No need to load the bitmap.
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  find the super block.  It will be at lba 1, 2, 3, ..., 32
           call find_super
           or   ax,ax
           jnz  short @f
           mov  si,offset NoSuperS
           jmp  short BootErr

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  the ROOT
@@:        mov  si,ax  ; ax = offset to super from find_super() above
           mov  eax,[si+S_LEAN_SUPER->root_start + 0]
           mov  edx,[si+S_LEAN_SUPER->root_start + 4]
           mov  cx,LEAN_ROOTSEG
           mov  es,cx
           call load_file

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           mov  si,offset loadname ; lean_fs formatted loader file name
           mov  ebx,es:[S_LEAN_INODE->file_size]    ; all of our .sys files are < 32-bits in size
           mov  di,LEAN_INODE_SIZE
           test dword es:[S_LEAN_INODE->attributes],(1<<19)
           jz   short s_loader
           mov  di,boot_data.sect_size
s_loader:  cmp  byte es:[di+S_LEAN_DIRENTRY->type],1
           jne  short s_cont
           mov  cx,es:[di+S_LEAN_DIRENTRY->name_len]
           push di
           add  di,12
           call stricmp
           pop  di
           or   al,al
           jz   f_loader
           
s_cont:    movzx eax,byte es:[di+S_LEAN_DIRENTRY->rec_len]
           shl  eax,4
           add  di,ax
           sub  ebx,eax
           jg   short s_loader
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the loader.sys file was not found, display error
           mov  si,offset noloaderS
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;                        
BootErr:   call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The Serial Number required by all FYS bootable bootsectors.
;  It is a dword sized random number seeded by the time of day and date.
;  This value should be created at format time, but this address is not
;  the same depending on the code above.
           dd  12345678h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following procedures up until the super block *must* remain in the
;  first sector since they are loaded by the BIOS.
;

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;  edx:eax = starting sector in LBA format
;       cx = count of sectors to read (doesn't check for > 7Fh)
;    es:bx = offset to store data, es:offset
; NOTE: This should get called with <= 7Fh sectors to read
read_sectors_long proc near uses eax ebx ecx edx si di

           mov  si,offset read_buffer
           mov  word [si],0010h
           mov  [si+2],cx
           mov  [si+04h],bx
           mov  [si+06h],es
           add  eax,[boot_data.base_lba+0]     ; base lba
           adc  edx,[boot_data.base_lba+4]
           mov  [si+08h],eax
           mov  [si+0Ch],edx
           
           mov  ah,42h             ; read
           mov  dl,boot_data.drive ; dl = drive
           int  13h
           mov  si,offset diskerrorS
           jc   short BootErr
           
           ret
read_sectors_long endp

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
;  These data items must be in the first sector already loaded by the BIOS

boot_data      st S_BOOT_DATA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55

%PRINT (510-$)               ; 58 bytes free in this area

           org (200h-2)
           dw  0AA55h


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;
f_loader:  mov  si,offset os_load_str  ; loading message
           call display_string         ; saves all registers

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now load the loader file to LOADSEG.
           ; (es:di->found root entry)
           mov  eax,es:[di+S_LEAN_DIRENTRY->inode+0]
           mov  edx,es:[di+S_LEAN_DIRENTRY->inode+4]
           mov  cx,LOADSEG
           mov  es,cx
           call load_file  ; returns sector count in ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move the file back to erase the inode
           ;  ax = sectors read
           mov  cx,ax
           shl  cx,8  ; ax = words to move

           ; move past INODE and ea's
           mov  si,LEAN_INODE_SIZE
           test dword es:[S_LEAN_INODE->attributes],(1<<19)
           jz   short @f
           mov  si,boot_data.sect_size

@@:        xor  di,di
           push ds
           push es
           pop  ds
           rep
           movsw
           pop  ds

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')

           push 07C0h              ; set up some data for LOADER.BIN
           pop  ds                 ; ds = 07C0h
           mov  si,offset boot_data
           
           ; fill the boot data struct
           mov  byte boot_data.file_system,LEAN
           
           mov  eax,((LEAN_ROOTSEG << 16) | 0)
           mov  boot_data.root_loc,eax
           xor  eax,eax
           mov  boot_data.other_loc,eax
           mov  boot_data.misc0,ax
           mov  boot_data.misc1,ax
           mov  boot_data.misc2,ax
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine compares (case insensitive) two strings
; ds:si->string0 (asciiz)
; es:di->string1
; cx = length of string1
; returns  al -1 if string0 < string1 (or) string0[cx] != '\0'
;          al  0 if string0 = string1
;          al  1 if string0 > string1
stricmp    proc near uses si di

stricmp_loop:
           mov  al,es:[di]
           inc  di
           call uppercase
           mov  ah,al

           lodsb
           call uppercase

           cmp  al,ah
           jb   short stricmp_less
           ja   short stricmp_above
           loop stricmp_loop

           ; make sure ds:si-> is an asciiz string at this length
           cmp  byte [si],0
           jne  short stricmp_less

           xor  al,al
           ret

stricmp_less:
           mov  al,-1
           ret

stricmp_above:
           mov  al,1
           ret

stricmp    endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; upper case the letter in al returning it in al
uppercase  proc near
           cmp  al,'a'
           jb   short uppercase_done
           cmp  al,'z'
           ja   short uppercase_done
           sub  al,32
uppercase_done:
           ret
uppercase  endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Load a file to given memory
; on entry:
;       es = segment to start (remember that the inode struct is included)
;  edx:eax = sector number to inode (first sector)
; on return:
;       ax = sectors read
load_file proc near uses ebx ecx edx ebp si di es

           ; we must start by loading at least one sector to get the inode
           xor  bx,bx
           mov  ecx,1
           call read_sectors_long

           ; create a list of sectors from the direct extent list
           ;  and indirect extent list(s)
           ; this works as long as there is 1365 or less extents total
           xor  di,di

           ; extent count in the inode
           movzx cx,byte es:[di+S_LEAN_INODE->extent_count]

           ; get next indirect sector number
           mov  eax,es:[di+S_LEAN_INODE->first_indirect+0]
           mov  edx,es:[di+S_LEAN_INODE->first_indirect+4]

           lea  si,[di+S_LEAN_INODE->extent_start]
           lea  bx,[di+S_LEAN_INODE->extent_size]
           mov  di,INDIRECT_BUFF
           xor  ebp,ebp  ; loop count

           ; save segment to load file to
           push es

ind_loop:  push eax  ; save the next indirect sector

           ; move the sectors start and count of sectors fields
@@:        mov  eax,es:[si]    ; start
           mov  [di],eax
           mov  eax,es:[si+4]  ;
           mov  [di+4],eax
           mov  eax,es:[bx]    ; size
           mov  [di+8],eax
           add  si,8
           add  bx,4
           add  di,12
           inc  ebp
           loop @b

           pop  eax

           ; if next indirect sector is zero, no more.
           or   eax,eax
           jnz  short @f
           or   edx,edx
           jz   short ind_done

           ; read in 1 sector of indirect extents
@@:        push (SECT_BUFFER >> 4) ; this works since our boot is at 0:7C00h
           pop  es
           mov  ecx,1
           xor  bx,bx
           call read_sectors_long

           ; store the next indirect sector
           mov  eax,es:[S_LEAN_INDIRECT->next_indirect+0]
           mov  edx,es:[S_LEAN_INDIRECT->next_indirect+4]

           ; get the count (at most 38)
           mov  ecx,es:[S_LEAN_INDIRECT->extent_count]

           ; point to our buffer
           mov  si,sizeof(S_LEAN_INDIRECT)
           mov  bx,(sizeof(S_LEAN_INDIRECT) + 8)
           jmp  short ind_loop

ind_done:  pop  es

           ; ebp now equals extents to read
           ; so save in ecx
           mov  ecx,ebp

           ; total sectors read counter
           xor  ebp,ebp

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now do the loading of the file, an extent at a time
           mov  si,INDIRECT_BUFF
           xor  ebx,ebx         ; es:ebx = start of read
           
@@:        push ecx
           mov  eax,[si]
           mov  edx,[si+4]
           mov  ecx,[si+8]
           add  si,12
           call read_sectors_long
           add  ebp,ecx         ; increment sector count
           
           ; calculate the segment for next read
           ; This assumes that loader.sys will be less than 64k
           ; If loader >= 64k, add to ES and leave bx = 0 instead
           xor  edx,edx
           movzx eax,word boot_data.sect_size
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

           mov  si,boot_data.sect_size

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

find_next: add  si,boot_data.sect_size
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
diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press any key',0
loadname      db  'loader.sys',0

;%PRINT (600h-$)               ; 457 bytes free in this area

; fill remaining with zeros
              dup  (600h-$),0

.end
