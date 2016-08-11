comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat_f.asm                                                          *
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
*   fat1x code                                                             *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm exfat<enter>                                *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   *** Assumes this is for a floppy drive ***                             *
*                                                                          *
* This bootsector is written for a FAT12\16 floppy disk.                   *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*                                                                          *
* Boot record:                                                             *
*  The boot parameter block is hard-coded with values suitable for a       *
*   3.5" High Density (ie 1.44M) FAT12 floppy disk.                        *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 186 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
* This code uses some size optimizations here and there to get it to fit   *
*   with in the 512 byte sector(s).  Be careful when assuming the          *
*   optimizations work if you modify the code.  i.e.: previous register    *
*   values, etc.                                                           *
*                                                                          *
* This source file is used to build a FAT12 or FAT16 boot for a floppy     *
*  disk depending on the setting of FAT12_CODE or FAT16_CODE equates       *
*  below.                                                                  *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include ..\boot.inc                ;
include fat.inc                    ;

; which fat size are we building for
FAT12_CODE   equ  0
FAT16_CODE   equ  1

; catch that we must have one or the other, not both, or none
.if ((FAT12_CODE && FAT16_CODE) || (!FAT12_CODE && !FAT16_CODE))
  %error "Must define one or the other, not both or neither"
.endif

; the different items depending on FAT12 or FAT16
.if FAT12_CODE
outfile 'fat12_f.bin'              ; target filename
  SECT_PER_FAT    equ  9
  FS_TYPE_STR     equ  'FAT12   '
  LAST_FAT_ENTRY  equ  0FF8h
  LOADER_FAT_TYPE equ  FAT12
.else
outfile 'fat16_f.bin'              ; target filename
  SECT_PER_FAT    equ  12
  FS_TYPE_STR     equ  'FAT16   '
  LAST_FAT_ENTRY  equ  0FFF8h
  LOADER_FAT_TYPE equ  FAT16
.endif

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

           jmp  short start        ; There should always be 3 bytes of code
           nop                     ;  followed by the start of the data.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following figures are specific to a 1.44M 3 1/2 inch floppy disk
;
OEM_Name     db  'FYSOS2.0'    ; 8 bytes for OEM Name and Version
nBytesPerSec dw  0200h         ; 512 bytes per Sector
nSecPerClust db  01            ; Sectors per Cluster
nSecRes      dw  02            ; Sectors reserved for Boot Record
nFATs        db  NUM_FATS      ; Number of FATs
nRootEnts    dw  224           ; Max Root Directory Entries allowed
nSecs        dw  2880          ; Number of Logical Sectors (0B40h)
                               ;   00h when > 65,535 sectors
mDesc        db  0F0h          ; Medium Descriptor Byte
nSecPerFat   dw  SECT_PER_FAT  ; Sectors per FAT
nSecPerTrack dw  18            ; Sectors per Track
nHeads       dw  02            ; Number of Heads
nSecHidden   dd  00h           ; Number of Hidden Sectors
nSecsExt     dd  00h           ; This value used when there are more
                               ;   than 65,535 sectors on a disc
                               ;   (ie disc size >= 32M)
DriveNum     db  00h           ; Physical drive number
nResByte     db  00h           ; Reserved
             db  29h           ; Signature for Extended Boot Record
SerNum       dd  12345678h     ; Volume Serial Number
VolName      db  'NO NAME    ' ; Volume Label
FSType       db  FS_TYPE_STR   ; File system type

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,STACK_OFFSET    ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h which is just
                                   ;    under 0C00:0000h where ROOT resides)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           mov  boot_data.drive,dl ; Store drive number
                                   ; (supplied by BIOS startup code)
           sti                     ; allow interrupts again

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the second sector of the boot code.
;
           mov  ax,1               ; lba 1
           cwd                     ;
           mov  cx,1               ; 1 sector
           mov  bx,0200h           ; at cs:0200h
           call read_sectors       ; read in the ROOT directory
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  if we get here, we have successfully loaded the
           ;  second sector.  So let's jump over all of the
           ;  code and data that must remain in the first
           ;  sector before the loading of the second sector.
           jmp  remaining
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following code and data must remain in the first sector

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h service
; Since we don't know if the current bios can span heads and cylinders,
;  with multiple counts in CX, we will read one sector at a time, manually
;  updating DH, and CX as we go.
; On entry:
;  DX:AX = starting sector in LBA format
;  ES:BX ->segment:offset to read to
;     CX = count of sectors to read
read_sectors proc near uses ax bx cx dx
           add  ax,[boot_data.base_lba+0]  ; add base lba
           adc  dx,[boot_data.base_lba+2]  ; 
read_loop: push dx
           push ax
           push cx
           push bx
           call lba_to_chs         ; dx:ax(lba) -> (ax/cx/dx)(chs)
           mov  ax,0003h           ; three try's
chck_loop: push ax
           mov  dl,boot_data.drive ; dl = drive
           mov  ax,0201h
           int  13h                ; do the read/write
           pop  ax
           jnc  short int13_no_error
           pusha                   ; save cx & dx
           xor  ah,ah
           int  13h                ; reset disk
           popa
           dec  ax
           jnz  short chck_loop    ; try again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  we have an error reading from the disk
           mov  si,offset diskerrorS ; loading message
           jmp  short BootErr      ; do error display and reboot

int13_no_error:
           pop  bx
           add  bh,2               ; point to next position
           pop  cx                 ; restore sector count
           pop  ax                 ; restore current LBA
           pop  dx                 ; (dx:ax)
           add  ax,1               ; inc to next LBA sector
           adc  dx,0               ;
           loop read_loop          ; decrement counter, read another one

           ret
read_sectors endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine converts LBA (DX:AX) to CHS
; Sector   = (LBA mod SPT)+1
; Head     = (LBA  /  SPT) mod Heads
; Cylinder = (LBA  /  SPT)  /  Heads
;    (SPT = Sectors per Track)
lba_to_chs proc near               ; dx:ax = LBA
           mov  cx,nSecPerTrack    ; sectors per track 
           call div32_16           ; dx:ax = dx:ax / cx  with cx = remdr.
           push cx                 ; save sectors
           mov  cx,nHeads          ; heads per cylinder
           call div32_16           ; dx:ax = dx:ax / cx  with cx = remdr.
           pop  dx                 ; dx = sector
           inc  dx                 ; sectors are one (1) based
           mov  dh,cl
           xchg ah,al
           xchg cx,ax
           ror  cl,2
           and  cl,11000000b
           and  dl,00111111b
           or   cl,dl
           ret
lba_to_chs endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; dx:ax = dx:ax / cx  with cx = remainder
div32_16   proc near uses bx
           mov  bx,dx              ;
           xchg bx,ax              ;
           xor  dx,dx              ;
           div  cx                 ; divide high order first
           xchg bx,ax              ;
           div  cx                 ; remainder will be in dx
           mov  cx,dx              ;  save remainder
           mov  dx,bx              ; full 32-bit answer
           ret                     ;
div32_16   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
;
display_string proc near
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           cld
@@:        lodsb                   ; ds:si = asciiz message
           or   al,al
           jz   short @f
           int  10h                ; output the character
           jmp  short @b
@@:        ret
display_string endp

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;                        
BootErr:   call display_string
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; booted data to pass to loader.sys
boot_data  st  S_BOOT_DATA uses \
    0,   \ ;    SecPerFat    dword  ; FAT: Sectors per FAT
    0,   \ ;    FATs         byte   ; FAT: Number of FATs
    0,   \ ;    SecPerClust  word   ; FAT: Sectors per Cluster
    0,   \ ;    SecRes       word   ; FAT: Sectors reserved for Boot Record
    0,   \ ;    SecPerTrack  word   ; FAT: Sectors per Track
    0,   \ ;    Heads        word   ; FAT: Number of Heads
    0,   \ ;    root_entries dword  ; FAT: Root entries
    0,   \ ;    base_lba     qword  ; ALL: base lba of partition
    0,   \ ;    file_system  byte   ; ALL: filesystem number
    0,   \ ;    drive        byte   ; ALL: BIOS drive number
    512, \ ;    sect_size    word   ; ALL: Sector size: 512, 1024, etc.
    0      ;    reserved     dup 6  ; padding/reserved

os_load_str   db  13,10,'Starting FYSOS...',0
loader_error  db  13,10,07,'Did not find Loader.sys file.',0
diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press a key...',0
fat_size_err  db  13,10,'FAT is larger than FATSEG_SIZE sectors...',0
loadname      db  'LOADER  SYS'

;%print (512-$-2)  ; ? bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-2)
           dw  0AA55h

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is the second sector loaded by the code above


;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory and the FAT into memory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  first the ROOT
remaining: push ROOTSEG            ; es -> to ROOT memory area
           pop  es                 ;
           xor  bx,bx              ;
           mov  al,nFATs           ; number of FATs
           cbw                     ;  (al < 80h)
           mul  word nSecPerFat    ; sectors per FAT
           add  ax,nSecRes         ; dx:ax = logical sector of start of
           adc  dx,00              ;  root directory (0 based)
           mov  cx,nRootEnts       ; max root entries allowed
           shr  cx,4               ; div by 16
           mov  bp,ax              ; set up bp for the 'loader' below
           add  bp,cx              ; bp -> (first sector of data area relative to boot_data.base_lba)
           call read_sectors       ; read in the ROOT directory

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  now the FAT
           push FATSEG             ; 
           pop  es                 ; es-> FAT table
           xor  bx,bx              ;
           mov  ax,nSecRes         ; dx:ax = logical sector of start of
           cwd                     ;  FAT (0 based)
           mov  cx,nSecPerFat      ; sectors per FAT (only need to load one fat)
           cmp  cx,FATSEG_SIZE     ; if sectors > FATSEG_SIZE, error
           jbe  short @f           ;
           mov  si,offset fat_size_err ; loading message
           jmp  BootErr            ;           
@@:        call read_sectors       ; read in the first FAT

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           push ROOTSEG            ;
           pop  es                 ;
           xor  di,di              ; es:di-> root table
           mov  si,offset loadname ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           mov  cx,11              ; file name + extension
s_loader:  pusha                   ; save si, di, and cx
           repe                    ; check if root entry is our file
           cmpsb                   ;
           popa                    ; restore si, di, and cx
           je   short f_loader     ; jump if file entry found
           add  di,32              ; else set up for next root entry
           dec  ax                 ; if no more root entries to search
           jnz  short s_loader     ; and search again
           
           ; else there was an error
           mov  si,offset loader_error ; loading message
           jmp  BootErr            ; else, error

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;
f_loader:  mov  si,offset os_load_str  ; loading message
           call display_string     ; saves all registers

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Walk the FAT chain reading all 'clusters' of
           ;   the loader file.
           ; es:di->found root entry block
           ; bp = first sector of data area (relative to base lba)

           ; set up first read
           mov  ax,es:[di+1Ah]     ; starting cluster number
           xor  ch,ch              ;
           mov  cl,nSecPerClust    ; number of sectors to read (per cluster)
           push LOADSEG            ; es-> loader seg
           pop  es                 ;
           xor  bx,bx              ;

           ; since FAT12 should never have a FAT larger than 64k,
           ;  we don't have to worry about 64k wrapping with our addressing
walk_fat:  xor  dx,dx              ; cluster number always < 0FFF8h
           push ax                 ; save cluster number
           dec  ax
           dec  ax
           mul  cx                 ; make it cluster based, not sector based
           add  ax,bp              ; add offset of first data sector
           adc  dx,00h             ;
           call read_sectors       ; read in cluster (saves all registers used)
           pop  ax                 ; restore cluster number

.if FAT12_CODE
           push ds                 ; save data segment register
           mov  si,ax              ;
           push FATSEG             ; es:si -> FATSEG
           pop  ds                 ;
           shr  si,1               ; offset = n + (n \ 2)
           add  si,ax              ; 
           test al,1               ; was it odd or even?
           lodsw                   ;
           jz   short is_even      ; 
           shr  ax,4               ; if n is odd, get high 12 bits
is_even:   and  ah,0Fh             ; if n is even, get low 12 bits
.else
           push ds                 ; save data segment register
           mov  si,ax              ;
           push FATSEG             ; es:si -> FATSEG
           pop  ds                 ;
           add  si,si              ; words
           lodsw                   ;
.endif

done_fat:  pop  ds                 ; restore data segment register
           
           ; calculate next position
           push ax
           push dx
           xor  dx,dx
           mov  ax,boot_data.sect_size
           mul  cx
           add  bx,ax
           pop  dx
           pop  ax
           
           cmp  ax,LAST_FAT_ENTRY  ; if last one, then we are done.
           jb   short walk_fat     ; 

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')

           push 07C0h              ; set up some data for loader.sys file
           pop  ds                 ; ds = 07C0h
           mov  si,offset boot_data
           
           ; fill the boot data struct
           mov  word [boot_data.SecPerFat+2],0
           mov  ax,nSecPerFat
           mov  boot_data.SecPerFat,ax
           mov  al,nFATs
           mov  boot_data.FATs,al
           mov  al,nSecPerClust
           xor  ah,ah
           mov  boot_data.SecPerClust,ax
           mov  ax,nSecRes
           mov  boot_data.SecRes,ax
           mov  ax,nSecPerTrack
           mov  boot_data.SecPerTrack,ax
           mov  ax,nHeads
           mov  boot_data.Heads,ax
           mov  ax,nRootEnts
           mov  [boot_data.root_entries + 0],ax
           xor  ax,ax
           mov  [boot_data.root_entries + 2],ax
           mov  byte boot_data.file_system,LOADER_FAT_TYPE
           mov  bx,ROOTSEG
           mov  [boot_data.root_loc + 0],ax  ; ax = 0 from above
           mov  [boot_data.root_loc + 2],bx
           mov  bx,FATSEG
           mov  [boot_data.other_loc + 0],ax
           mov  [boot_data.other_loc + 2],bx
           mov  boot_data.misc0,ax
           mov  boot_data.misc1,ax
           mov  boot_data.misc2,ax
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h

%print (400h-$)  ; ? bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of code. Pad out to fill sector
           org 400h

.end
