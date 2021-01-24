comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
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
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm fat32<enter>                                *
*                                                                          *
* Last Updated: 24 Jan 2021                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   *** Assumes this is for a hard drive ***                               *
*                                                                          *
*   *** This assumes that the FAT is less than 1,110 sectors. ***          *
*   *** Any image with more than this will overwrite the BIOS area.        *
*   *** This needs to be fixed.                                            *
*                                                                          *
* This bootsector is written for a FAT32 hard drive.                       *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the \ (root), \BOOT,    *
*   or \SYSTEM\BOOT directory.                                             *
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

include ../boot.inc                ;
include fat.inc                    ;

RESVD_SECTS  equ  32

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
nSecRes      dw  RESVD_SECTS   ; Sectors reserved for Boot Record, info sect, etc.
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
; since we need all the room we can get, *and* 'boot_data' does not
;  need to be pre initialized, we use the area where our code first
;  starts since once it is exectued, we don't need it anymore.
; however, we must make sure that we do not access any members of
;  S_BOOT_DATA until sizeof(S_BOOT_DATA) bytes have been executed.
; which means we also have to tell 'services/read_ext.inc' we are
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
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,STACK_OFFSET    ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h which is just
                                   ;    under 0C00:0000h where ROOT resides)
           
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
           int  13h                     ; dl still = drive_num
           jc   short @f
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jz   short read_remaining
@@:        mov  si,offset no_extentions_found
         ; jmp  short BootErr

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;
BootErr:   call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the rest of the boot code
;
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
           mov  di,offset boot_data ; this assumes es = 0x07C0, which it should
           mov  si,offset boot_sig
           movsd       ; signature
           movsd       ; lba low dword
           movsd       ; lba high dword
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
           mov  ebx,07E00h     ; physical address 0x07E00
           xor  eax,eax        ; (smaller than 'mov eax,1')
           inc  ax             ; eax = 1
           cdq
           mov  cx,nSecRes     ; doesn't matter that it reads 31 as long
         ; dec  cx             ;   as it reads the 3 we have here
           call read_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; jump over info block
           jmp  skip_info_block

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Procedures used with in code follow:
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

include ..\services\read_ext.inc   ; include the read_sectors_long function
include ..\services\conio.inc      ; include the display_char and display_string functions

loadname      db  'LOADER  SYS'
bootname      db  'BOOT       '
sys_name      db  'SYSTEM     '

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data that must be in the first sector
no_extentions_found db  'Require int 13h extented read service',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%PRINT (200h-$-4-8-2)           ; 13 byte(s) free in this area

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The Serial Number required by all FYS bootable bootsectors.
;   It is a dword sized random number seeded by the time of day and date.
;   This value should be created at format time
;  The Base LBA of this boot code.
;  The formatter will update this data for us.
           org (200h-2-8-4)
boot_sig   dd  ?
base_lba   dq  ?

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
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory and the FAT into memory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  first, the FAT
           movzx eax,word nSecRes  ; eax = logical sector of start of
           cdq                     ; edx = 0
           mov  cx,nSecPerFat      ; sectors per FAT (only need to load one fat)
           mov  ebx,(FATSEG << 4)
           call read_sectors_long  ; read in the first FAT
           
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
           mov  ebx,(ROOTSEG << 4) ; segment to read to ROOT memory area
           mov  eax,nRootClust     ;
           mov  cx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           call load_it            ; returns ax = sectors read from above
           shl  ax,4               ; shl ax,9 / shr ax,5
           mov  nRootEnts,ax       ;
           
           push ROOTSEG            ;
           pop  es                 ;
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
           mov  si,offset loadname ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           call search_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in \boot directory
           mov  si,offset bootname ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           call search_name
           jnz  short try_system
           
           ; if we found the boot directory, we need to load the dir data
           mov  cx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           movzx eax,word es:[di+1Ah]  ; starting cluster number
           call load_it            ; read in the \BOOT directory
           ; then search for the loader name again
           mov  si,offset loadname ; 8.3 formatted loader file name
           shl  ax,4               ; ax = count of entries to search
           call search_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in \system\boot directory
try_system:
           ; read the root directory again
           mov  eax,nRootClust     ;
           mov  cx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           call load_it            ; returns ax = sectors read from above
           
           mov  si,offset sys_name ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           call search_name
           jnz  short not_find_it
           
           ; if we found the \system directory, we need to load the dir data
           mov  cx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           movzx eax,word es:[di+1Ah]  ; starting cluster number
           call load_it            ; read in the \SYSTEM directory
           ; now search for the \system\boot directory
           mov  si,offset bootname ; 8.3 formatted loader file name
           shl  ax,4               ; count of entries to search
           call search_name
           jnz  short not_find_it
           
           ; if we found the \system\boot directory, we need to load the dir data
           mov  cx,ROOTSEG_SIZE    ; we only allow ROOTSEG_SIZE sectors
           movzx eax,word es:[di+1Ah]  ; starting cluster number
           call load_it            ; read in the \SYSTEM\BOOT directory
           ; then search for the loader name again
           mov  si,offset loadname ; 8.3 formatted loader file name
           shl  ax,4               ; ax = count of entries to search
           call search_name
           jz   short f_loader
           
           ; else there was an error
not_find_it:
           mov  si,offset not_found_str ; loading message
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
           mov  [boot_data+S_BOOT_DATA->loader_base],ebx ; save to our boot_data block too
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; es:di->found root entry block
           ; set up first read
   xchg bx,bx  ; ben
           mov  ax,es:[di+14h]     ; starting cluster number (hi word)
           shl  eax,16             ; move to hi word of eax
           mov  ax,es:[di+1Ah]     ; starting cluster number (low word)
           mov  cx,256             ; set a hi limit (256 ?)
           call load_it            ; returns ax = sectors read from above
           
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
           mov  byte [boot_data+S_BOOT_DATA->file_system],FAT32 ; this assumes ds = 0x07C0, which it should
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ebx-> boot data (includes  'booted from drive') (physical address)
           mov  eax,[boot_data+S_BOOT_DATA->loader_base] ; this assumes ds = 0x07C0, which it should
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
           mov  ebx,(07C00h + boot_data) ; we pass the physical address of BOOT_DATA
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           retf

include ..\services\address.inc    ; include the "choosing" of the loader.sys base address code

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; new interrupt vector for INT 21h
;  this simply (i)returns
int32vector:
           iret

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; loads sectors until fat entry = no more, or have loaded the limit (BX)
; On entry:
;    EAX = starting cluster in LBA format
;    EBX = address to read to
;     CX = Max count of sectors to read
; On return:
;     AX = count of sectors read
load_it    proc near uses bx cx dx bp di es
           mov  di,cx              ; max count
           
           movzx ecx,byte nSecPerClust  ; makes sure high word of ecx is clear
           xor  bp,bp              ; amount of sectors read
           
load_next: cmp  cx,di              ;
           jle  short @f           ;
           mov  cx,di              ;
@@:        sub  di,cx
           
           add  bp,cx              ; amount of sectors read
           
           push eax                ; save cluster number
           sub  eax,2              ; the cluster numbers are 2 based
           ;xor  edx,edx
           mul  ecx
           add  eax,data_start     ; add the base of the data area
           adc  edx,0
           call read_sectors_long  ; read in the ROOT directory/file
           
           mov  eax,200h           ; calculate offset for next read.
           mul  ecx                ;
           add  ebx,eax            ;
           
           pop  eax                ; restore cluster number

           ; get next cluster number
           push ds                 ;
           push FATSEG             ;
           pop  ds                 ;
           shl  eax,2              ; 
           mov  eax,[eax]          ; this assumes all cluster numbers are < 0x3F80
           pop  ds                 ; 
           
           cmp  di,0
           jle  short @f
           
           cmp  eax,0FFF_FFF8h     ; if eax >= 0FFF_FFF8h, done
           jb   short load_next    ;

@@:        xchg bp,ax              ; return ax = sectors read
           ret
load_it    endp
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; search for name given
; on entry:
;  ds:si-> name to search for (FAT SFN formatted)
;  es:di-> buffer to search
;     ax = count of entries to search in
; on exit:
;  zero flag set if found
;  es:di-> entry found
search_name proc near
           xor  di,di              ; es:di-> root table
           mov  cx,11              ; file name + extension
@@:        pusha                   ; save si, di, and cx
           repe                    ; check if root entry is our file
           cmpsb                   ;
           popa                    ; restore si, di, and cx
           je   short @f           ; jump if file entry found
           add  di,32              ; else set up for next root entry
           dec  ax                 ; if no more root entries to search
           jnz  short @b           ; and search again
           or   al,1               ; clear the zero flag
@@:        ret
search_name endp           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 2nd data area.
;  Any data here must not be used until we load this sector to memory

data_start    dd  0
nRootEnts     dw  0

os_load_str   db  13,10,'Starting FYSOS...',0
exe_error     db  13,10,07,'Error with Loader.sys format.',0
not_found_str db  13,10,'Did not find the LOADER.SYS file.',0

%PRINT (800h-$)  ;  426 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of 4th sector. Pad out to fill 512 bytes
           org 800h

.end
