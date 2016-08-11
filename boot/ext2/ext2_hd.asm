comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: ext2_hd.asm                                                        *
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
*   EQUates for ext2.asm                                                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm ext2<enter>                                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*   This bootsector is written for a Ext2 partition.                       *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 386 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
* This assumes that:                                                       *
*  -  the loader file will not be more than 65536 bytes.                   *
*  -  the inodes *must* use blocks for directorys, but can use blocks or   *
*      extents for files.                                                  *
*                                                                          *
* This code also must fit within 1024 bytes.  Therefore, it uses some      *
*  size coding optimizations to squeeze a few more bytes in it.            *
*                                                                          *
***************************************************************************|

.model tiny                        ;

outfile 'ext2_hd.bin'              ; the target file name

include ..\boot.inc                ;
include ext2.inc                   ; equates and structures

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
           mov  sp,EXT2_STACK_TOP  ; first push at 07C0:(EXT2_STACK_TOP - 2)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           mov  boot_data.drive,dl ; Store drive number
                                   ; (suplied by BIOS startup code)
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

not386str   db  13,10,'Not a 386 compatible',0

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

no_extentions_found db  'Requires int 13h extentions',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to load the next 8 blocks.  Since we don't know the blocks
;   size until after we load the next block, let's load at least 8 more
;   sectors.  This will make sure we have 9 sectors loaded, and the first
;   group descriptor will be within these 9 sectors.
read_remaining:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           mov  bx,0200h       ; address 0x07E00
           xor  eax,eax        ; eax = 1 (we need to shave bytes, so use these two lines instead)
           inc  ax             ;
           mov  cx,8           ; second sector of boot and super block
           call read_sectors_long

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; get the size of a sector.  i.e.: bytes per sector
           mov  ah,48h
           mov  dl,boot_data.drive
           xor  si,si          ; 07C0:0000 is free to use now
           int  13h
           mov  ax,[si+S_INT13_PARMS->sect_size]
           mov  boot_data.sect_size,ax

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Calculate where the first group is.

           mov  si,offset super_block
           mov  cl,[si+S_EXT2_SUPER->log_block_size]  ; this is a dword, but only a byte is used
           mov  ax,1024
           shl  ax,cl
           mov  [EXT2_BLOCK_SIZE],ax
           cmp  ax,1024        ; if block size is 1024
           ja   short @f       ;  we need to use the third group offset
           add  ax,ax          ;  -> first group
@@:        mov  [EXT2_FIRST_GRP],ax ; save it for later
           xchg di,ax          ; di = group (ignore new value of ax to save a byte)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now read in one block.  The second inode will be in the first block
           mov  eax,[di+S_EXT2_GROUP_DESC->inode_table]
           movzx ecx,word [EXT2_BLOCK_SIZE]
           shr  cx,9           ; block size / 512 = sectors to read
           mul  ecx
           mov  bx,TEMP_BUFFER  ; temp offset
           call read_sectors_long
           
           mov  ax,128          ; assume 128 byte inodes
           cmp  dword [si+S_EXT2_SUPER->rev_level],0
           jz   short @f
           mov  ax,[si+S_EXT2_SUPER->inode_size]
@@:        add  bx,ax           ; move to second inode
           mov  [EXT2_INODE_SIZE],ax

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the root directory into memory.  No need to load the bitmap.
;
           mov  si,bx   ; -> inode
           mov  ax,[si+S_EXT2_INODE->mode]
           and  ax,EXT2_S_IFMT
           cmp  ax,EXT2_S_IFDIR
           je   short @f
           mov  si,offset norootS
           jmp  short BootErr
@@:        mov  bx,EXT2_ROOTOFF ; root address
           call load_file

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;  (we assume the root size will be less than 65536)
           mov  eax,[si+S_EXT2_INODE->size]
           mov  [EXT2_ROOT_SIZE],eax   ; store it for the loader file
           mov  bp,EXT2_ROOTOFF
           mov  si,bp
           add  bp,ax

find_loader:
           mov  di,TEMP_BUFFER
           movzx cx,byte [si+S_EXT2_DIR->name_len]
           push si
           add  si,sizeof(S_EXT2_DIR)
           pusha                   ; saves cx and di (saves two bytes using pusha/popa)
           rep
           movsb
           mov  [di],cl            ; asciiz it
           popa
           mov  si,offset loadname
           call stricmp
           pop  si
           or   al,al
           jz   f_loader
           mov  cx,[si+S_EXT2_DIR->rec_len]
           jcxz did_not_find_loader  ; saves a few bytes using cx instead of ax
           add  si,cx
           cmp  si,bp
           jb   short find_loader

did_not_find_loader:
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
; The following procedures up until the super block *must* remain in the
;  first sector since they are loaded by the BIOS.
;

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;      eax = starting sector in LBA format
;       cx = count of sectors to read (doesn't check for > 7Fh)
;    es:bx = offset to store data, es:offset
; NOTE: This should get called with <= 7Fh sectors to read
read_sectors_long proc near uses edx si
           
           xor  si,si              ; we need to save bytes (07C0:0000 is free to use anyway)
           mov  word [si],0010h
           mov  [si+02h],cx
           mov  [si+04h],bx
           mov  [si+06h],es
           xor  edx,edx
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
; display a string to the screen using the BIOS
;
display_string proc near
display_loop:                      ; ds:si = asciiz message
           lodsb
           or   al,al
           jz   short end_string
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           int  10h                ; output the character
           jmp  short display_loop
end_string: ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  These data items must be in the first sector already loaded by the BIOS

boot_data     st  S_BOOT_DATA  ; booted data to pass to loader.sys

loadname      db  'loader.sys',0

diskerrorS    db  13,10,'Error reading disk/non-system disk'
              db  13,10,'Press a key',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55

%PRINT (510-$)               ; 25 byte(s) free in this area

           org (200h-2)
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;
f_loader:  push si
           mov  si,offset os_load_str  ; loading message
           call display_string     ; saves all registers
           pop  si

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  get inode table again
           mov  di,[EXT2_FIRST_GRP]
           mov  eax,[di+S_EXT2_GROUP_DESC->inode_table]
           movzx ecx,word [EXT2_BLOCK_SIZE]
           shr  ecx,9          ; block size / 512 = sectors to read
           mul  ecx
           push eax
           ; eax = sector of inode table

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  calculate offset within table to read
           xor  edx,edx
           movzx ebx,word [super_block+S_EXT2_SUPER->inode_size]
           mov  eax,[si+S_EXT2_DIR->inode]
           dec  eax     ; inodes are 1 based
           mul  ebx
           mov  bx,512  ; high word of ebx already clear
           div  ebx
           ; edx = offset within sector
           ; eax = sector number
           pop  ecx     ; sector of inode table
           add  eax,ecx

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  read in the inode
           mov  bx,TEMP_BUFFER
           mov  cx,1
           call read_sectors_long

           mov  esi,TEMP_BUFFER
           add  esi,edx       ; add inode offset within sector

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  make sure the mode == regular file
           mov  ax,[si+S_EXT2_INODE->mode]
           and  ax,EXT2_S_IFMT
           cmp  ax,EXT2_S_IFREG
           jne  did_not_find_loader

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  load loader.sys
           ; ds:si-> loader->inode
           ; es:bx-> where to load it to
           push LOADSEG
           pop  es
           xor  bx,bx
           call load_file

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')

           push 07C0h              ; set up some data for LOADER.BIN
           pop  ds                 ; ds = 07C0h
           mov  si,offset boot_data

           ; fill the boot data struct
           xor  eax,eax
           mov  boot_data.SecPerFat,eax
           mov  boot_data.FATs,al
           mov  boot_data.SecPerClust,ax
           mov  boot_data.SecRes,ax
           ;mov  boot_data.SecPerTrack,ax
           ;mov  boot_data.Heads,ax
           ;mov  boot_data.root_entries,eax
           mov  byte boot_data.file_system,EXT2
           mov  eax,((007C0h << 16) | EXT2_ROOTOFF)
           mov  boot_data.root_loc,eax
           mov  ax,EXT2_SUPER
           mov  boot_data.other_loc,eax
           mov  ax,[EXT2_ROOT_SIZE]
           mov  boot_data.misc0,ax
           mov  ax,[EXT2_FIRST_GRP]
           mov  boot_data.misc1,ax
           mov  ax,[EXT2_BLOCK_SIZE]
           mov  boot_data.misc2,ax
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h



; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Load the file
;  ds:si -> inode
;  es:bx -> buffer to load to
; We don't have to worry about double indirect blocks since the loader
;  file will not be more than 65536 bytes.
load_file  proc near uses esi

           mov  eax,[si+S_EXT2_INODE->flags]
           test eax,EXT4_EXTENTS_FL
           jz   short is_blocks

           lea  di,[si+S_EXT2_INODE->block_array]
           mov  cx,[di+EXT3_EXTENT_HEADER->entries]
           add  di,sizeof(EXT3_EXTENT_HEADER)
load_extents:
           push cx
           push bx
           mov  eax,[di+S_EXT3_EXTENT->block]
           movzx ecx,word [EXT2_BLOCK_SIZE]
           mul  ecx
           add  bx,ax
           ;movzx edx,word [di+S_EXT3_EXTENT->start_hi]
           mov  eax,[di+S_EXT3_EXTENT->start]
           shr  ecx,9
           mul  ecx
           push eax
           movzx eax,word [di+S_EXT3_EXTENT->len]
           mul  ecx
           xchg ecx,eax
           pop  eax
           call read_sectors_long
           pop  bx
           pop  cx
           add  di,sizeof(S_EXT3_EXTENT)
           loop load_extents
           jmp  done_load_file

is_blocks: xor  edx,edx
           mov  eax,[si+S_EXT2_INODE->size]
           movzx ecx,word [EXT2_BLOCK_SIZE]
           dec  ecx
           add  eax,ecx
           inc  ecx
           div  ecx
           mov  ecx,eax
           ; ecx = blocks to load
           
           ; direct blocks
           lea  si,[si+S_EXT2_INODE->block_array]
           mov  di,12  ; up to 12 before we have to do an indirect block
@@:        push ecx
           mov  eax,[si]
           add  si,4
           movzx ecx,word [EXT2_BLOCK_SIZE]
           shr  ecx,9
           mul  ecx
           call read_sectors_long
           add  bx,[EXT2_BLOCK_SIZE]
           pop  ecx
           dec  ecx
           jz   short done_load_file
           dec  di           
           jnz  short @b

           ; indirect blocks
           mov  eax,[si]       ; get indirect block number
           push es
           push bx
           push ecx
           mov  bx,ds
           mov  es,bx
           mov  bx,TEMP_BUFFER
           movzx ecx,word [EXT2_BLOCK_SIZE]
           shr  ecx,9
           mul  ecx
           call read_sectors_long
           pop  ecx
           pop  bx
           pop  es

           mov  si,TEMP_BUFFER
@@:        push ecx
           mov  eax,[si]
           add  si,4
           movzx ecx,word [EXT2_BLOCK_SIZE]
           shr  ecx,9
           mul  ecx
           call read_sectors_long
           add  bx,[EXT2_BLOCK_SIZE]
           pop  ecx
           dec  ecx
           jnz  short @b

done_load_file:
           ret
load_file  endp

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
;  start of our data
os_load_str   db  13,10,'Starting FYSOS...',0

norootS       db  13,10,'Didn',39,'t find root inode',0
noloaderS     db  13,10,'Didn',39,'t find loader.sys',0

%PRINT (400h-$)               ; 1 bytes free in this area

; fill remaining with zeros
              dup  (400h-$),0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The super block is at offset 0x400

super_block   dup (1000h-400h),?  ; make sure we have the whole block

.end
