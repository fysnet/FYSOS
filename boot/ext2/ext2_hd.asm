comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
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
*   ext2_hd.asm source code                                                *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm ext2_hd<enter>                              *
*                                                                          *
* Last Updated: 31 May 2017                                                *
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
*   or compatible CPU.                                                     *
*                                                                          *
* This assumes that:                                                       *
*  -  the inodes *must* use blocks for directorys, but can use blocks or   *
*      extents for files.                                                  *
*     (Ben, I don't think this matters anymore)                            *
*                                                                          *
* This code also must fit within 1024 bytes.  Therefore, it uses some      *
*  size coding optimizations to squeeze a few more bytes into it.          *
* At a quick glance, it would be really easy to simply shorten the         *
*  display (error) strings to get our size down.  However, I want to       *
*  keep all error strings the same, across file system boot sectors,       *
*  so that there is no confusion to what error has happened.               *
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
; since we need all the room we can get, *and* 'boot_data' does not
;  need to be pre initialized, we use the area where our code first
;  starts since once it is exectued, we don't need it anymore.
; however, we must make sure that we do not access any members of
;  S_BOOT_DATA until sizeof(S_BOOT_DATA) bytes have been executed.
; which means we now have to tell 'services/read_ext.inc' we are
;  doing this:
BOOT_DATA_IS_PTR  equ  1
boot_data:

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
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           push 0F000h             ; if bits 15:14 are still set
           popf                    ;  after pushing/poping to/from
           pushf                   ;  the flags register then we have
           pop  ax                 ;  a 386+
           and  ax,0F000h          ;
           jnz  short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  short BootErr0     ; to save a byte, we short jump to another jump
           
not386str  db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf                   ; restore the interrupt bit

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we have moved this down here (as opposed to our
           ;  other boot sectors) since we cannot access
           ;  S_BOOT_DATA until after sizeof(S_BOOT_DATA)
           ;  bytes above.
           ; this also assumes ds = 0x07C0, which it should
           mov  [boot_data+S_BOOT_DATA->drive],dl ; Store drive number
                                   ; (suplied by BIOS startup code)
           sti                     ; allow interrupts again

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now check for the BIOS disk extentions.
; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive_num from above
           jc   short @f
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jz   short read_remaining
@@:        mov  si,offset no_extentions_found
BootErr0:  jmp  BootErr

no_extentions_found db  'Requires int 13h extentions',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to load the next 8 blocks.  Since we don't know the blocks
;   size until after we load the next block, let's load at least 8 more
;   sectors.  This will make sure we have 9 sectors loaded, and the first
;   group descriptor will be within these 9 sectors.
read_remaining:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; reset the disk services to be sure we are ready
           xor  ax,ax
           mov  dl,[boot_data+S_BOOT_DATA->drive] ; this assumes ds = 0x07C0, which it should
           int  13h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; save the Signature and Base LBA to boot_data
           ; (this assumes that the two items are in the same order
           ;  and at the first of boot_data for this to work.
           ;  we do this so that we can save room in the boot code.)
           xor  di,di  ; this assumes es = 0x07C0 and offset boot_data = 0, which it should
           mov  si,offset boot_sig
           mov  cx,12  ; signature (dword) + base lba (qword)
           rep
            movsb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           xor  ebx,ebx        ; address 0x07E00
           mov  bh,7Eh         ; (smaller than 'mov  ebx,07E00h');
           xor  eax,eax        ; (smaller than 'mov eax,1')
           inc  ax             ; eax = 1
           cdq                 ; edx = 0
           mov  cl,8           ; read at most 8 more sectors (ch = 0 from 'rep' above)
           call read_sectors_long
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Calculate where the first group is.
           mov  si,offset super_block
           mov  cl,[si+S_EXT2_SUPER->log_block_size]  ; this is a dword, but only a byte is used
           mov  ax,1024
           shl  ax,cl
           mov  [EXT2_BLOCK_SIZE],eax
           push eax            ; high word of eax = 0 from above. low word (ax) = block size in bytes
           cmp  ax,1024        ; if block size is 1024
           ja   short @f       ;  we need to use the third group offset
           add  ax,ax          ;  -> first group
@@:        mov  [EXT2_FIRST_GRP],ax ; save it for later (ax = offset to group (ds:ax -> S_EXT2_GROUP_DESC))
           xchg di,ax          ; di = group (ignore new value of ax to save a byte)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now read in one block.  The second inode will be in the first block
           ; di is offset from 0x7C00 where we have loaded the first sectors
           ;  of the disk.
           mov  eax,[di+S_EXT2_GROUP_DESC->inode_table]
           pop  ecx            ; restore block_size in ecx (in bytes)
           shr  cx,9           ; block size / 512 = sectors to read (shr cx only since won't be > 65535)
           mul  ecx
           mov  bx,(0x7C00 + TEMP_BUFFER) ; inode table buffer (physical address) (high word of ebx == zero from above)
           mov  [EXT2_INODE_TBLE],eax  ; save LBA of inode table
           call read_sectors_long
           
           mov  ebp,128         ; assume 128 byte inodes
           cmp  byte [si+S_EXT2_SUPER->rev_level],0  ; rev_level won't be anything more than a byte (through ext4 anyway)
           jz   short @f
           mov  bp,[si+S_EXT2_SUPER->inode_size]
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the root directory into memory.  No need to load the bitmap.
;          
@@:        lea  si,[bp+TEMP_BUFFER]   ; ds:si -> inode
           mov  al,[si+S_EXT2_INODE->mode + 1]  ; get the high byte
           and  al,(EXT2_S_IFMT >> 8)
           cmp  al,(EXT2_S_IFDIR >> 8)
           je   short @f
           mov  si,offset norootS
           jmp  short BootErr
@@:        mov  bx,(0x7C00 + EXT2_ROOTOFF) ; root address (high word of ebx == zero from above)
           call load_file
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;  (we assume the root size will be less than 65536)
           mov  dx,[si+S_EXT2_INODE->size]
           mov  si,EXT2_ROOTOFF
           add  dx,si  ; dx -> end of root directory
           
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
           call stricmp            ; returns zero flag set if equal
           pop  si
           jz   f_loader
           mov  cx,[si+S_EXT2_DIR->rec_len]
           jcxz did_not_find_loader  ; saves a few bytes using cx instead of ax
           add  si,cx
           cmp  si,dx
           jb   short find_loader

did_not_find_loader:
           mov  si,offset norootS

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
include ..\services\read_ext.inc   ; include the read_sectors_long function
include ..\services\conio.inc      ; include the display_char and display_string functions

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  These data items must be in the first sector already loaded by the BIOS
loadname      db  'loader.sys',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%print (200h-$-4-8-2)           ; 4 byte(s) free in this area

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The Serial Number required by all FYS bootable bootsectors.
;   It is a dword sized random number seeded by the time of day and date.
;   This value should be created at format time
;  The Base LBA of this boot code.
;  The formatter will update this data for us.
           org (200h-2-8-4)
boot_sig   dd  0xDEADBEAF    ; usually the format utility will give us a sig
base_lba   dq  0             ; and a base

           org (200h-2)
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;
f_loader:  push si   ; si-> directory entry
           mov  si,offset os_load_str  ; loading message
           call display_string     ; saves all registers
           pop  si

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  calculate offset within table to read
           mov  eax,[si+S_EXT2_DIR->inode]
           dec  eax       ; inodes are 1 based
         ; cdq            ; we assume eax < 32-bits
           mul  ebp       ; ebp = inode size
           mov  bx,512    ; high word of ebx already clear
           div  ebx
           ; edx = offset within sector
           ; eax = sector number
           add  eax,[EXT2_INODE_TBLE]  ; sector of inode table
           
           ; add offset and point to (not yet read) inode
           mov  si,TEMP_BUFFER
           add  si,dx         ; add inode offset within sector
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  read in the inode (read in the sector holding the inode)
           cdq          ; we assume eax < 32-bits
           mov  bx,(0x7C00 + TEMP_BUFFER)  ; high word of ebx is clear
           mov  cx,1
           call read_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  make sure the mode == regular file
           mov  al,[si+S_EXT2_INODE->mode + 1]  ; get the high byte
           and  al,(EXT2_S_IFMT >> 8)
           cmp  al,(EXT2_S_IFREG >> 8)
           jne  did_not_find_loader
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; "choose" loader.sys base address
           ;  this returns the address is ebx
           call set_up_address     ; so that we can loader a loader.sys file
                                   ;  > 64k, we pass a physical address in ebx.
           mov  [boot_data+S_BOOT_DATA->loader_base],ebx ; save to our boot_data block too
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  load loader.sys
           ; ds:si-> loader->inode
           ;   ebx-> where to load it to
           call load_file
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  So that we can have a DOS INT 21h stub on the front part of
;   the loader, we need to "patch" the 21h vector
           xor  ax,ax
           mov  es,ax
           mov  eax,(07C00000h + int32vector)
           mov  es:[(21h * 4)],eax
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  update the Boot Data block
           mov  byte [boot_data+S_BOOT_DATA->file_system],EXT2 ; this assumes ds = 0x07C0, which it should
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ebx-> boot data (includes  'booted from drive') (physical address)
           mov  eax,[boot_data+S_BOOT_DATA->loader_base] ; this assumes ds = 0x07C0, which it should
           shr  eax,4
           mov  ds,ax
           
           cmp  word [0],5A4Dh
           je   short is_dos_exe
           
           push es                 ; es = 07C0h from before
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
           mov  ebx,(07C00h + boot_data) ; we pass the physical address of BOOT_DATA
           
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
; Load the file
;  ds:si -> inode
;   ebx -> physical buffer to load to
; We don't have to worry about double indirect blocks since the loader
;  file will not be more than 65536 bytes.
load_file  proc near uses si
           
           mov  al,[si+S_EXT2_INODE->flags + 2] ; get low byte of high word (+2)
           test al,(EXT4_EXTENTS_FL >> 16)
           jz   short is_blocks
           
           lea  di,[si+S_EXT2_INODE->block_array]
           mov  cx,[di+EXT3_EXTENT_HEADER->entries]
           add  di,sizeof(EXT3_EXTENT_HEADER)
load_extents:
           push ecx
           push ebx
           mov  eax,[di+S_EXT3_EXTENT->block]
           mov  ecx,[EXT2_BLOCK_SIZE]
           mul  ecx
           add  ebx,eax
         ; movzx edx,word [di+S_EXT3_EXTENT->start_hi]  ; (the 'mul ecx' below destroys edx anyway)
           mov  eax,[di+S_EXT3_EXTENT->start]
           shr  cx,9          ; we assume block size < 65536
           mul  ecx
           push eax
           movzx eax,word [di+S_EXT3_EXTENT->len]
           mul  ecx
           xchg ecx,eax
           pop  eax
           call read_sectors_long
           pop  ebx
           
           ; add to the buffer
           add  ebx,[EXT2_BLOCK_SIZE]  ; block size is in bytes
           
           pop  ecx
           add  di,sizeof(S_EXT3_EXTENT)
           loop load_extents
           jmp  short done_load_file
           
is_blocks: xor  edx,edx
           mov  eax,[si+S_EXT2_INODE->size]
           mov  ecx,[EXT2_BLOCK_SIZE]
           add  eax,ecx
           dec  eax
           div  ecx
           mov  ecx,eax
           ; ecx = blocks to load
           
           ; direct blocks
           lea  si,[si+S_EXT2_INODE->block_array]
           mov  di,12  ; up to 12 before we have to do an indirect block
@@:        push ecx
           lodsd
           mov  ecx,[EXT2_BLOCK_SIZE]
           shr  cx,9          ; we assume block size < 65536
           mul  ecx
           call read_sectors_long
           add  ebx,[EXT2_BLOCK_SIZE]
           pop  ecx
           dec  ecx
           jz   short done_load_file
           dec  di           
           jnz  short @b
           
           ; indirect blocks
           ; ???? what does si point to ????
;           lodsd            ; get indirect block number
;           push ebx
;           push ecx
;           mov  ebx,(0x7C00 + TEMP_BUFFER)
;           mov  ecx,[EXT2_BLOCK_SIZE]
;           shr  cx,9          ; we assume block size < 65536
;           mul  ecx
;           call read_sectors_long
;           pop  ecx
;           pop  ebx
           
           mov  si,TEMP_BUFFER
@@:        push ecx
           lodsd
           mov  ecx,[EXT2_BLOCK_SIZE]
           shr  cx,9          ; we assume block size < 65536
           mul  ecx
           call read_sectors_long
           add  ebx,[EXT2_BLOCK_SIZE]
           pop  ecx
           .adsize
           loop @b
           
done_load_file:
           ret
load_file  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data
os_load_str   db  13,10,'Starting FYSOS...',0
exe_error     db  13,10,07,'Error with Loader.sys format.',0
norootS       db  13,10,'Didn',39,'t find root inode or loader.sys',0

%print (400h-$)               ; 0 byte(s) free in this area

; fill remaining with zeros
              dup  (400h-$),0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The super block is at offset 0x400

super_block   dup (1000h-400h),?  ; make sure we have the whole block

.end
