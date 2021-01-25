comment |*******************************************************************
*  Copyright (c) 1984-2021    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat_hd.asm                                                         *
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
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm fat1x_hd<enter>                             *
*                                                                          *
* Last Updated: 24 Jan 2021                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   *** Assumes this is for a hard drive ***                               *
*                                                                          *
* This bootsector is written for a FAT12\16 hard disk.                     *
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
*   within the 512 byte sector(s).  Be careful when assuming the           *
*   optimizations work if you modify the code.  i.e.: previous register    *
*   values, etc.                                                           *
*                                                                          *
* This source file is used to build a FAT12 or FAT16 boot for a hard-      *
*  drive depending on the setting of FAT12_CODE or FAT16_CODE equates      *
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
outfile 'fat12_hd.bin'             ; target filename
  SECT_PER_FAT    equ  9
  FS_TYPE_STR     equ  'FAT12   '
  LAST_FAT_ENTRY  equ  0FF8h
  LOADER_FAT_TYPE equ  FAT12
.else
outfile 'fat16_hd.bin'             ; target filename
  SECT_PER_FAT    equ  12
  FS_TYPE_STR     equ  'FAT16   '
  LAST_FAT_ENTRY  equ  0FFF8h
  LOADER_FAT_TYPE equ  FAT16
.endif
RESVD_SECTS  equ  3

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
nSecRes      dw  RESVD_SECTS   ; Sectors reserved for Boot Record

nFATs        db  2 ;;; NUM_FATS      ; Number of FATs
nRootEnts    dw  512 ;;; 224           ; Max Root Directory Entries allowed
nSecs        dw  8064 ;;; 2880          ; Number of Logical Sectors (0B40h)
                               ;   00h when > 65,535 sectors
mDesc        db  0F8h ;;; 0F0h          ; Medium Descriptor Byte
nSecPerFat   dw  32 ;;; SECT_PER_FAT  ; Sectors per FAT
nSecPerTrack dw  63 ;;; 18            ; Sectors per Track
nHeads       dw  16 ;;; 02            ; Number of Heads
nSecHidden   dd  1024 ;;; 00h           ; Number of Hidden Sectors
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
           jmp  BootErr

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
           jmp  BootErr

no_extentions_found db  'Requires int 13h extented read service.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Save the base lba of the partition
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
           
;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the second sector of the boot code.
;          
           mov  ebx,07E00h     ; physical address 0x07E00
           xor  eax,eax        ; (smaller than 'mov eax,1')
           inc  ax             ; eax = 1
           cdq
           mov  cx,(RESVD_SECTS - 1)
           call read_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  if we get here, we have successfully loaded the
           ;  second sector.  So let's jump over all of the
           ;  code and data that must remain in the first
           ;  sector before the loading of the second sector.
           jmp  remaining_code
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following code and data must remain in the first sector

os_load_str   db  13,10,'Starting FYSOS...',0
loadname      db  'LOADER  SYS'
bootname      db  'BOOT       '
sys_name      db  'SYSTEM     '

include ..\services\read_ext.inc   ; include the read_sectors_long function
include ..\services\conio.inc      ; include the display_char and display_string functions

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;                        
BootErr:   call display_string
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%print (200h-$-4-8-2)           ; 15 bytes free in this area

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

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory into memory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  read the ROOT
remaining_code:
           movzx eax,byte nFATs    ; number of FATs
           movzx ecx,word nSecPerFat  ; sectors per FAT
           mul  ecx
           movzx ebx,word nSecRes
           add  eax,ebx            ; edx:eax = logical sector of start of
           adc  edx,0              ;  root directory (0 based)
           movzx ecx,word nRootEnts ; max root entries allowed
           shr  ecx,4              ; div by 16 = ((x * 32) / 512)
           mov  ebp,eax            ; set up ebp for the 'loader' below
           add  ebp,ecx            ; ebp -> (first sector of data area relative to boot_data.base_lba)
           mov  root_lba,eax       ; save the location and size of the root
           mov  root_size,ecx      ;
           mov  ebx,(ROOTSEG << 4) ;
           call read_sectors_long  ; read in the ROOT directory

           push ROOTSEG            ;
           pop  es                 ;
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
           mov  si,offset loadname ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           call search_name
           jz   short f_loader
           
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
           call load_file          ; read in the \BOOT directory
           ; then search for the loader name again
           mov  si,offset loadname ; 8.3 formatted loader file name
           shr  eax,5              ; ax = count of entries to search
           call search_name
           jz   short f_loader
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in \system\boot directory
try_system:
           ; read the root directory again
           mov  eax,root_lba       ;
           mov  ecx,root_size      ;
           call read_sectors_long  ; read in the ROOT directory
           
           mov  si,offset sys_name ; 8.3 formatted loader file name
           mov  ax,nRootEnts       ; count of entries to search
           call search_name
           jnz  short not_find_it
           
           ; if we found the \system directory, we need to load the dir data
           call load_file          ; read in the \SYSTEM directory
           ; now search for the \system\boot directory
           mov  si,offset bootname ; 8.3 formatted loader file name
           shr  eax,5              ; count of entries to search
           call search_name
           jnz  short not_find_it
           
           ; if we found the \system\boot directory, we need to load the dir data
           call load_file          ; read in the \SYSTEM\BOOT directory
           ; then search for the loader name again
           mov  si,offset loadname ; 8.3 formatted loader file name
           shr  eax,5              ; ax = count of entries to search
           call search_name
           jz   short f_loader
           
           ; else there was an error
not_find_it:
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
           ; "choose" loader.sys base address
           ;  this returns the address is ebx
           call set_up_address     ; so that we can loader a loader.sys file
                                   ;  > 64k, we pass a physical address in ebx.
           mov  [boot_data+S_BOOT_DATA->loader_base],ebx ; save to our boot_data block too
           call load_file
           
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
           mov  byte [boot_data+S_BOOT_DATA->file_system],LOADER_FAT_TYPE ; this assumes ds = 0x07C0, which it should
           
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
; load a file
; on entry:
;  es:di->dir entry of file/sub
;  ebx = physical address to load it to
; on exit
;  eax = bytes read
load_file  proc near uses ebx
           xor  edx,edx
           ; set up first read
           movzx eax,word es:[di+1Ah]  ; starting cluster number
           movzx ecx,byte nSecPerClust ; number of sectors to read (per cluster)
           ; since FAT12 should never have a FAT larger than 64k,
           ;  we don't have to worry about 64k wrapping with our addressing
walk_fat:  push eax                ; save cluster number
           dec  eax
           dec  eax
           mul  ecx                ; make it cluster based, not sector based
           add  eax,ebp            ; add offset of first data sector
           call read_sectors_long  ; read in cluster (saves all registers used)
           
           mov  eax,200h           ; calculate offset for next read.
           mul  ecx                ;
           add  ebx,eax            ;
           add  edx,eax
           
           pop  eax                ; restore cluster number
           call get_next_fat_entry ; get next cluster number
           
           cmp  eax,LAST_FAT_ENTRY ; if last one, then we are done.
           jb   short walk_fat     ; 
           
           mov  eax,edx
           ret
load_file  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; get the next FAT entry.
; this will load a sector from the FAT if that sector is not already loaded
; On entry:
;    EAX = current FAT entry
; On return:
;    EAX = next FAT entry
get_next_fat_entry proc near uses ebx ecx edx ebp

           ; calculate which sector of the fat we need
           xor  edx,edx
           movzx ecx,word nBytesPerSec
.if FAT12_CODE
           ; LSN from starting of FAT = (((EAX * 3) / 2) / bpb->bytes_per_sector);
           mov  ebp,eax
           mov  ebx,3
           mul  ebx
           shr  eax,1
.else
           ; in a FAT16 FAT, we have 256 entries per 512-byte sector
           ; LSN from starting of FAT = (EAX / (bpb->bytes_per_sector / sizeof(single_entry)));
           shr  ecx,1              ; divide by 2
.endif
           div  ecx                ;
           push edx                ; save the modulor for below
           
           movzx ecx,word nSecRes  ; ecx = logical sector of start of FAT
           add  eax,ecx            ; 
           cmp  eax,cur_fat_sect   ; do we already have this sector loaded?
           je   short @f
           
           ; else, load the sector
           cdq                     ; edx = 0
           mov  cx,1               ; need just one sector
           mov  ebx,(FATSEG << 4)
           call read_sectors_long  ; read in the FAT sector
           mov  cur_fat_sect,eax   ; save the current sector
           
@@:        pop  ebx                ; entry within this sector
           push ds                 ;
           push FATSEG             ;
           pop  ds                 ;
.if FAT12_CODE
           movzx eax,word [ebx]    ;
           test ebp,1              ; was it odd or even?
           jz   short is_even
           shr  eax,4
is_even:   and  ah,0Fh             ; if n is even, get low 12 bits
.else
           shl  ebx,1              ; 
           movzx eax,word [ebx]    ;
.endif
           pop  ds                 ; 

           ret
get_next_fat_entry endp

include ..\services\address.inc    ; include the "choosing" of the loader.sys base address code
         
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
exe_error     db  13,10,07,'Error with Loader.sys format.',0
fat_size_err  db  13,10,'FAT is larger than FATSEG_SIZE sectors...',0
loader_error  db  13,10,07,'Did not find Loader.sys file.',0

root_lba      dd  0
root_size     dd  0
cur_fat_sect  dd  0   ; used to indicate which sector we currently have loaded

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; new interrupt vector for INT 21h
;  this simply (i)returns
int32vector:
           iret

%print ((RESVD_SECTS * 512) - $)  ; 412/426 bytes here (fat12/fat16)

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of code. Pad out to fill sector
           org (RESVD_SECTS * 512)

.end
