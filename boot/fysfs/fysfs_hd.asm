comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs_hd.asm                                                       *
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
*   EQUates for fysfs.asm                                                  *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm fysfs_hd<enter>                             *
*                                                                          *
* Last Updated: 28 Sept 2017                                               *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*   This bootsector is written for a FYSFS hard disk.                      *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the \ (root), \BOOT,    *
*   or \SYSTEM\BOOT directory.                                             *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include ..\boot.inc                ;
include fysfs.inc

outfile 'fysfs_hd.bin'             ; out filename (using quotes will keep it lowercase)

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
; which means we also have to tell 'services/read_ext.inc' we are
;  doing this:
BOOT_DATA_IS_PTR  equ  1
boot_data:

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
           cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,STACK_OFFSET    ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h 
           
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
; We now check for the BIOS disk extensions.
; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive_num
           jc   short @f
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jz   short read_remaining
@@:        mov  si,offset no_extentions_found
           jmp  short BootErr

no_extentions_found db  'Requires int 13h extented read service.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the rest of the boot code
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
           ; read in 16 more sectors from base + 1
           mov  ebx,07E00h     ; address 0x07E00
           xor  eax,eax        ; (smaller than 'mov eax,1')
           inc  ax             ; eax = 1
           cdq
           mov  cx,16          ; make sure we get the super_block too
           call read_sectors_long

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; jump to the rest of the code we just loaded
           jmp  remaining_code

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
; The following code and data must remain in the first sector

include ..\services\read_ext.inc   ; include the read_sectors_long function
include ..\services\conio.inc      ; include the display_char and display_string functions

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data
loadname      db  'loader.sys',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%print (200h-$-4-8-2)           ; 124 bytes free in this area

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
;  Start of remaining sectors.  None of this code can be used until
;  these sectors are loaded from the first sector code.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory into memory
;
remaining_code:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  the ROOT (which contains the fat too.)
           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           mov  ebx,(FYSFS_ROOTSEG << 4)
           mov  eax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 0]
           mov  edx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 4]
           mov  ecx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root_entries]   ; max root entries allowed
           shr  ecx,2               ; div by 4
           cmp  ecx,FYSFS_ROOTSEG_SIZE ; limit the read to FYSFS_ROOTSEG_SIZE
           jbe  short @f           ;
           mov  ecx,FYSFS_ROOTSEG_SIZE
@@:        call read_sectors_long  ; read in the ROOT directory

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  initialize the crc check
;
           call crc32_initialize

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           push ds                 ; make es -> 0x07C00
           pop  es                 ;
           xor  ebx,ebx            ; current slot number to look in.
           mov  si,offset loadname ; loader file name
           mov  di,FYSFS_BUFFER    ; buffer for found name
           mov  ecx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root_entries]  ; count of entries to search
s_loader:  call get_name
           jc   short @f
           call stricmp            ; returns zero flag set if equal
           jz   f_loader
@@:        inc  ebx                ; next slot number
           .adsize                 ; use ECX as the counter
           loop s_loader
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we get here, the loader file wasn't found
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
           ;  Walk the FAT chain reading all 'clusters' of
           ;   the loader file.
           ; ebx = first dir entry of file
           mov  edi,FYSFS_BUFFER   ; buffer for found 32-bit fat entries
           call get_fat_entries    ; get all fat entries, returns eax = entries
           jnc  short @f
           mov  si,offset fat_slot_errorS
           jmp  BootErr

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; "choose" loader.sys base address
           ;  this returns the address is ebx
@@:        call set_up_address     ; so that we can loader a loader.sys file
                                   ;  > 64k, we pass a physical address in ebx.
           mov  [boot_data+S_BOOT_DATA->loader_base],ebx ; save to our boot_data block too
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  We now have the count of entries in EAX and
           ;  the list of entries in ds:edi
           movzx ecx,word [FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->sect_clust]   ; number of sectors to read (per cluster)
           mov  ebp,eax            ; count of clusters to read
           mov  si,FYSFS_BUFFER
@@:        mov  eax,[si+0]
           mov  edx,[si+4]
           add  si,sizeof(qword)
           mul  ecx                ; cluster based
           add  eax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 0]
           adc  edx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 4]
           call read_sectors_long  ; read in cluster (saves all registers used)
           
           ; calculate next position
           mov  eax,200h
           mul  ecx
           add  ebx,eax
           
           dec  ebp
           jnz  short @b
           
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
           mov  byte [boot_data+S_BOOT_DATA->file_system],FYSFS ; this assumes ds = 0x07C0, which it should
           
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
include ..\services\stricmp.inc    ; include the stricmp function

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; new interrupt vector for INT 21h
;  this simply (i)returns
int32vector:
           iret

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; included the common items

include fysfs_common.inc

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  data

os_load_str   db  13,10,'Starting FYSOS...',0

fat_slot_errorS  db 13,10,'Found error when parsing FAT entries...',0
noloaderS        db 13,10,'Did not find loader file...',0
exe_error        db  13,10,07,'Error with Loader.sys format.',0

%print (2000h-$)  ; 5441 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is were the super block is.  As soon as we load
; the remaining 16 sectors, these fields will be filled
; with there corresponding values.
           org 2000h  ; make sure we are at this position

.end
