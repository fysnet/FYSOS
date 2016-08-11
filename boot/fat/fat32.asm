comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat32.asm                                                          *
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
*   fat32 code                                                             *
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
*   *** Assumes this is for a hard drive ***                               *
*                                                                          *
* This bootsector is written for a FAT32 hard drive.                       *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
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
* This code assumes that the FAT entries for LOADER.SYS are within the     *
*  first 128 sectors of the FAT.                                           *
*                                                                          *
* This code assumes we will not be a floppy drive.                         *
*                                                                          *
* This boot sector uses three sectors.                                     *
*                                                                          *
***************************************************************************|

.model tiny                        ;

outfile 'fat32.bin'                ; target filename

include boot.inc                   ;
include fat.inc                    ;

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
nSecRes      dw  32            ; Sectors reserved for Boot Record, info sect, etc.
nFats        db  NUM_FATS      ; Number of FATs
             dw  0             ; reserved
             dw  0             ; reserved
mDesc        db  0F0h          ; Medium Descriptor Byte
             dw  0             ; reserved
nSecPerTrack dw  18            ; Sectors per Track
nHeads       dw  02            ; Number of Heads
nSecHidden   dd  00h           ; Number of Hidden Sectors
nSecsExt     dd  2880          ; total number of sectors in volume
nSecPerFat   dd  23            ; sectors per fat
ExtFlags     dw  00000000b     ; flags
nFSVer       dw  00h           ; version 0.0
nRootClust   dd  02            ; Root Cluster number
nFSInfo      dw  01            ; sector number of the FSINFO struct (0 based)
nBKBootSect  dw  06            ; sector number of backup boot block (0 based)
             dup 12,0          ; reserved
DriveNum     db  00h           ; Physical drive number
nResByte     db  00h           ; Reserved
             db  29h           ; Signature for Extended Boot Record
SerNum       dd  12345678h     ; Volume Serial Number
VolName      db  'NO NAME    ' ; Volume Label
FSType       db  'FAT32   '    ; File system type

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
;  The next task is to check for a 386
           pushf                   ; save the interrupt bit
           push 7000h              ; if bits 12,13,14 are still set
           popf                    ; after pushing/poping to/from
           pushf                   ; the flags register then we have
           pop  ax                 ; a 386+
           and  ax,7000h           ;
           cmp  ax,7000h           ;
           je   short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  short BootErr

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
           jmp  short BootErr

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the rest of the boot code
;
read_remaining:
           push 07C0h
           pop  es
           mov  bx,0200h       ; address 0x07E00
           mov  eax,1          ;
           mov  cx,nSecRes     ; doesn't matter that it reads 31 as long
         ; dec  cx             ;   as it reads the 3 we have here
           call read_sectors_long

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; get the size of a sector.  i.e.: bytes per sector
           mov  ah,48h
           mov  dl,boot_data.drive
           mov  si,offset start
           int  13h
           mov  ax,[si+S_INT13_PARMS->sect_size]
           mov  boot_data.sect_size,ax

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory and the FAT into memory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  first, the FAT
           push FATSEG             ;
           pop  es                 ; es-> FAT table
           movzx eax,word nSecRes  ; eax = logical sector of start of
           mov  cx,nSecPerFat      ; sectors per FAT (only need to load one fat)
           cmp  cx,FATSEG_SIZE     ; if sectors > FATSEG_SIZE, error
           jbe  short @f           ;
           mov  cx,FATSEG_SIZE     ; only load FATSEG_SIZE sectors
@@:        xor  bx,bx              ; es:0000h
           call read_sectors_long  ; read in the first FAT

           jmp  skip_info_block


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;
BootErr:   call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Procedures used with in code follow:
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;      eax = starting sector in LBA format
;       cx = count of sectors to read (doesn't check for > 7Fh)
;    es:bx = offset to store data, es:offset
; NOTE: This should get called with <= 7Fh sectors to read
read_sectors_long proc near uses eax ebx ecx edx si di

           mov  si,offset start    ; okay to use 0x07C00 (0x07C0:xxxx)
           mov  word [si],0010h    ; [si+0] = 10h, [si+1] = 0
           mov  [si+02h],cx        ; size ( < 7F) (byte0 = count, byte1 = 0)
           mov  [si+04h],bx        ; es:bx
           mov  [si+06h],es        ;
           xor  edx,edx            ;
           add  eax,[boot_data.base_lba]   ; base lba
           adc  edx,[boot_data.base_lba+4]
           mov  [si+08h],eax       ;
           mov  [si+0Ch],edx       ;
           
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
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; ds:si = asciiz message
@@:        lodsb
           or   al,al
           jz   short @f
           int  10h                ; output the character
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data that must be in the first sector

boot_data     st  S_BOOT_DATA  ; booted data to pass to loader.sys

no_extentions_found db  'Require int 13h extented read service',0
not386str     db  13,10,'Not 386 compatible processor',0
diskerrorS    db  13,10,'Error reading disk or non-system disk'
              db  13,10,'Press a key..',0

%print (200h-$-2)  ; 6 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of 1st sector. Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-2)
           dw  0AA55h


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Start of 2nd Sector.
;  This is the Info Sector
           dup 512,0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Start of 3nd Sector.
;  Any code here must not be used until we load this sector to memory
skip_info_block:

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the first sector of the data area
           movzx eax,byte nFats    ; 
           movzx ecx,word nSecPerFat
           mul  ecx                ;
           movzx ecx,word nSecRes  ;
           add  eax,ecx            ; add in the boot and info sectors
           mov  data_start,eax     ; save in data_start

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  the ROOT
           mov  cx,ROOTSEG         ; segment to read to ROOT memory area
           mov  eax,nRootClust     ;
           mov  bx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           call load_it            ; returns ax = sectors read from above
           mov  root_sectors,ax    ; save it for the loader.sys

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           mov  es,cx              ; cx = ROOTSEG from above
           xor  di,di              ; es:di-> root table
           mov  si,offset loadname ; 8.3 formatted loader file name
                                   ; ax = sectors read from above
           shl  ax,4               ; mul by 512, then divide by 32
           mov  cx,11              ; file name + extension
s_loader:  pusha                   ; save si, di, and cx
           repe                    ; check if root entry is our file
           cmpsb                   ;
           popa                    ; restore si, di, and cx
           je   short f_loader     ; jump if file entry found
           add  di,32              ; else set up for next root entry
           dec  ax                 ; if no more root entries to search
           jnz  short s_loader     ; and search again
           mov  si,offset diskerrorS ; loading message
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
           ; es:di->found root entry block
           ; set up first read
           mov  ax,es:[di+14h]     ; starting cluster number (hi word)
           shl  eax,16             ; move to hi word of eax
           mov  ax,es:[di+1Ah]     ; starting cluster number (low word)
           mov  cx,LOADSEG         ; read to loader seg
           mov  bx,cx              ; set a hi limit (3000h)
           call load_it            ; returns ax = sectors read from above

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')

           push 07C0h              ; set up some data for LOADER.BIN
           pop  ds                 ; ds = 07C0h
           mov  si,offset boot_data

           ; fill the boot data struct
           mov  eax,nSecPerFat
           mov  [boot_data.SecPerFat],eax
           mov  al,nFATs
           mov  boot_data.FATs,al
           xor  ah,ah
           mov  al,nSecPerClust
           mov  boot_data.SecPerClust,ax
           mov  ax,nSecRes
           mov  boot_data.SecRes,ax
           mov  ax,nSecPerTrack
           mov  boot_data.SecPerTrack,ax
           mov  ax,nHeads
           mov  boot_data.Heads,ax
           xor  eax,eax
           mov  boot_data.root_entries,eax
           mov  byte boot_data.file_system,FAT32
           mov  eax,(ROOTSEG << 16)
           mov  boot_data.root_loc,eax
           mov  eax,(FATSEG << 16)
           mov  boot_data.other_loc,eax
           xor  ax,ax
           mov  boot_data.misc0,ax
           mov  boot_data.misc1,ax
           mov  boot_data.misc2,ax
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; loads sectors until fat entry = no more, or have loaded the limit (BX)
; On entry:
;    EAX = starting cluster in LBA format
;     CX = segment to read to (cx:0000h)
;     BX = Max count of sectors to read
; On return:
;     AX = count of sectors read
load_it    proc near uses bx cx dx bp di es

           mov  di,bx              ; max count

           mov  es,cx              ; es:di = es:0000
           xor  bx,bx

           movzx cx,byte nSecPerClust
           xor  bp,bp              ; amount of sectors read

load_next: cmp  cx,di              ;
           jle  short @f           ;
           mov  cx,di              ;
@@:        sub  di,cx

           add  bp,cx              ; amount of sectors read

           push eax                ; save cluster number
           sub  eax,2              ; the cluster numbers are 2 based
           mul  ecx
           add  eax,data_start     ; add the base of the data area
           call read_sectors_long  ; read in the ROOT directory
           
           mov  ax,200h            ; calculate offset for next read.
           mul  cx                 ;
           add  bx,ax              ;
           
           pop  eax                ; restore cluster number

           ; get next cluster number
           push ds                 ;
           push FATSEG             ;
           pop  ds                 ;
           shl  eax,2              ; 
           mov  eax,[eax]          ; this assumes all clusters are < 03FFh
           pop  ds                 ; 

           cmp  di,0
           jle  short @f
           
           cmp  eax,0FFF_FFF8h     ; if eax >= 0FFF_FFF8h, done
           jb   short load_next    ;

@@:        xchg bp,ax              ; return ax = sectors read
           ret
load_it    endp


               

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 2nd data area.
;  Any data here must not be used until we load this sector to memory

data_start    dd  0
root_sectors  dw  0  ; sectors read for root.

os_load_str   db  13,10,'Starting FYSOS...',0
loadname      db  'LOADER  SYS'

;%print (600h-$)  ;  237 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of 3nd sector. Pad out to fill 512 bytes
           org 600h

.end
