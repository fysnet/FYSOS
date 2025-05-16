comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: exfat_f.asm                                                        *
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
*   exfat boot code                                                        *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm exfat<enter>                                *
*                                                                          *
* Last Updated: 17 Sept 2018                                               *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include ..\boot.inc                ;
include exfat.inc                  ;

outfile 'exfat_f.bin'              ; target filename

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

           jmp  short start        ; There should always be 3 bytes of code
           nop                     ;  followed by the start of the data.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following figures are specific to a 1.44M 3 1/2 inch floppy disk
; Our formatter (mexfat.exe) will fill all of these in for us.
;  However, we need to have their offsets so that we can access them
;  here in this code.
;
OEM_Name     db  'EXFAT   '    ; 8 bytes for OEM Name and Version

; we need all the room we can get in the first sector
; therefore, we use the 53 reserved bytes (in memory, not disk) for
;  the boot_data area.
; So instead of:
;            dup 53,0          ; must be zeros
; we are going to use:
boot_data    st  S_BOOT_DATA  ; booted data to pass to loader.sys
             dup (53-sizeof(S_BOOT_DATA)),0
             
PartOffset   dq  ?             ; LBA to partition
VolumeLen    dq  ?             ; Sectors in this volume
FATOffset    dd  ?             ; LBA to 1st FAT
FATLength    dd  ?             ; Sectors in the FAT
ClustHeapOff dd  ?             ; Cluster Heap Offset
ClusterCount dd  ?             ; Number of Clusters in Heap
RootCluster  dd  ?             ; Cluster of Root Cluster
SerNum       dd  ?             ; Volume Serial Number
FSVersion    dw  ?             ; Version
VolumeFlags  dw  ?             ; Volume Flags
logBytesSect db  ?             ; bytes per sector (1<<9 = 512)
logSectClust db  ?             ; sectors per cluser (1<<1 = 512)
NumFATs      db  ?             ; Number of FATs
DriveNumber  db  ?             ; used by the BIOS
HeapUse      db  ?             ; percentage of heap in use
             dup 7,0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code
.ifne $ 0x78
  %error 1, 'Offset of start code does not equal 0x78'
.endif
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
           ; store the drive number
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
           popf                    ; restore the interrupt bit
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
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; read in 8 more sectors from base + 1
           mov  ebx,07E00h      ; physical address 0x07E00
           xor  eax,eax         ; eax = 1  (smaller than  'mov  eax,1')
           inc  ax              ;
           mov  cx,8            ; 
           call read_sectors
           
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

include ..\services\lba2chs.inc    ; include the LBA to CHS routine
include ..\services\read_chs.inc   ; include the read disk using CHS routine
include ..\services\conio.inc      ; include the display_char and display_string functions

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%print (200h-$-4-8-2)           ; 3 bytes free in this area

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The Serial Number required by all FYS bootable bootsectors.
;   It is a dword sized random number seeded by the time of day and date.
;   This value should be created at format time
;  The Base LBA of this boot code.
;  The formatter will update this data for us.
           org (200h-2-8-4)
boot_sig   dd  ?
base_lba   dq  ?

           org ((1 * 200h)-2)  ; End of LSN 0
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Start of remaining sectors.  None of this code can be used until
;  these sectors are loaded from the first sector code.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the fat and root directory into memory
;
remaining_code:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  first the FAT
           mov  ecx,FATLength      ; ecx = logical sector of start of
           cmp  ecx,FATSEG_SIZE    ; if sectors > FATSEG_SIZE, error
           jbe  short @f           ;
           mov  si,offset fat_size_err ; loading message
           jmp  BootErr            ;           
@@:        mov  ebx,(FAT_FATSEG << 4) ; physical address to read it to
           mov  eax,FATOffset      ; LSN of FAT
           call read_sectors       ; read in the first FAT
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  next the ROOT
           push FAT_FATSEG         ; fs = segment of fat
           pop  fs                 ;
           mov  cl,logSectClust    ; calculate SPC
           mov  eax,1              ;
           shl  eax,cl             ;
           mov  ecx,eax            ; ecx = sectors per cluster
           mov  ebx,(FAT_ROOTSEG << 4) ; -> to ROOT memory area
           mov  eax,RootCluster    ;
@@:        push eax                ;
           sub  eax,2              ; clusters are 2 based
           mul  ecx                ; make it sector based
           add  eax,ClustHeapOff   ; from start of heap
           call read_sectors       ; read in the first cluster
           mov  eax,512            ; point to next position
           mul  ecx                ;
           add  ebx,eax            ;
           pop  eax                ;
           mov  eax,fs:[eax*4]     ; get next cluster number
           cmp  eax,0FFFFFFF8h     ; is it the last?
           jb   short @b           ;
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           push FAT_ROOTSEG        ; es = segment of root
           pop  es                 ;
           xor  di,di              ; es:di-> root table
s_loader:  mov  al,es:[di+S_EXFAT_ROOT->entry_type]
           cmp  al,EXFAT_DIR_EOD   ; if no more directory entries, be done
           jz   short not_found    ;
           mov  cx,1               ; for calculation of next below
           cmp  al,EXFAT_DIR_ENTRY ;
           jne  short @f
           movzx cx,byte es:[di+S_EXFAT_DIR->sec_count] ; get count of remaining entries in this set
           dec  cx                 ; cx = count of name ext entries
           add  di,sizeof(S_EXFAT_ROOT) ; move to stream extension
           lea  si,[di+32]         ; esi is first name ext entry (nbasm doesn't (yet) allow sizeof(S_EXFAT_ROOT) in []'s
           call extract_name       ; saves all registers
           push di                 ; save di
           push es                 ; save es
           push ds                 ; es = ds
           pop  es                 ;
           mov  si,offset loadname ; 8.3 formatted loader file name
           mov  di,offset buffer
           call stricmp            ; returns zero flag set if equal
           pop  es                 ; restore es
           pop  di                 ; restore di
           jz   short f_loader     ; jump if file entry found
           
           ; not this entry.  Move to next set
@@:        mov  ax,sizeof(S_EXFAT_ROOT)
           mul  cx                 ; cx = count of secondary entries (-1)
           add  di,ax              ; di = stream_ext + count of secondary - 1
           jmp  short s_loader

not_found: mov  si,offset loader_error
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
           ;  Walk the FAT chain reading all 'clusters' of
           ;   the loader file.
           ; es:di->S_EXFAT_STRM entry of file name found
           ; fs: still points to fat segment
           mov  cl,logSectClust    ; calculate SPC
           mov  eax,1              ;
           shl  eax,cl             ;
           mov  ecx,eax            ; sectors per cluster
           mov  eax,es:[di+S_EXFAT_STRM->first_clust]
@@:        push eax                ;
           sub  eax,2              ; clusters are 2 based
           mul  ecx                ; make it sector based
           add  eax,ClustHeapOff   ; from start of heap
           call read_sectors       ; read in the first cluster
           mov  eax,512            ; point to next position
           mul  ecx                ;
           add  ebx,eax            ;
           pop  eax                ;
           mov  eax,fs:[eax*4]     ; get next cluster number
           cmp  eax,0FFFFFFF8h     ; is it the last?
           jb   short @b           ;
           
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
           mov  byte boot_data.file_system,ExFAT
           
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
           mov  ebx,(07C00h + boot_data) ; we pass the physical address of BOOT_DATA
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           retf

os_load_str   db  13,10,'Starting FYSOS...',0
loader_error  db  13,10,07,'Did not find Loader.sys file.',0
fat_size_err  db  13,10,'FAT is larger than FATSEG_SIZE sectors...',0
exe_error     db  13,10,07,'Error with Loader.sys format.',0
loadname      db  'loader.sys',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; new interrupt vector for INT 21h
;  this simply (i)returns
int32vector:
           iret


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
%print ((2 * 200h)-$-4)          ; 18 bytes free in this area
           org ((2 * 200h)-4)  ; End of LSN 1
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; extract 16-bit UTF name from cx count of name_ext entries
; and place ascii in 'buffer'
; on entry:
; ds:si-> first name_ext entry
;    cx = count of name_ext entries to use
extract_name proc near uses all ds
           mov  ax,07C0h
           mov  ds,ax
           mov  di,offset buffer
           
extractit: push cx
           push si
           add  si,S_EXFAT_NAME->name  ; (add 2 to si)
           mov  cx,15
@@:        mov  ax,es:[si]
           add  si,2
           mov  [di],al
           inc  di
           loop @b
           pop  si
           add  si,sizeof(S_EXFAT_NAME)
           pop  cx
           loop extractit
           
           ret         
extract_name endp

include ..\services\address.inc    ; include the "choosing" of the loader.sys base address code
include ..\services\stricmp.inc    ; include the stricmp function

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
%print ((3 * 200h)-$-4)        ; 361 bytes free in this area
           org ((3 * 200h)-4)  ; End of LSN 2
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((4 * 200h)-$-4)          ; 508 bytes free in this area
           org ((4 * 200h)-4)  ; End of LSN 3
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((5 * 200h)-$-4)          ; 508 bytes free in this area
           org ((5 * 200h)-4)  ; End of LSN 4
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((6 * 200h)-$-4)          ; 508 bytes free in this area
           org ((6 * 200h)-4)  ; End of LSN 5
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((7 * 200h)-$-4)          ; 508 bytes free in this area
           org ((7 * 200h)-4)  ; End of LSN 6
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((8 * 200h)-$-4)          ; 508 bytes free in this area
           org ((8 * 200h)-4)  ; End of LSN 7
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; at the end of every sector, we must place a sig of 0xAA550000
;%print ((9 * 200h)-$-4)          ; 508 bytes free in this area
           org ((9 * 200h)-4)  ; End of LSN 8
           db  0x00, 0x00, 0x55, 0xAA

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of code.
.ifne $ (9*512)
%error 1, 'Binary is not 9 sectors in length'
.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; .bss area
buffer  dup 256,?

.end
