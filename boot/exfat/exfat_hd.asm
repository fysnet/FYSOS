comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: exfat_hd.asm                                                       *
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
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   *** Assumes this is for a hard drive ***                               *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include boot.inc                   ;
include exfat.inc                  ;

outfile 'exfat_hd.bin'             ; target filename

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

           jmp  short start        ; There should always be 3 bytes of code
           nop                     ;  followed by the start of the data.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following figures are specific to a 1.44M 3 1/2 inch floppy disk
;
OEM_Name     db  'EXFAT   '    ; 8 bytes for OEM Name and Version
             dup 53,0          ; must be zeros
PartOffset   dq  01            ; LBA to partition
VolumeLen    dq  0             ; Sectors in this volume
FATOffset    dd  0             ; LBA to 1st FAT
FATLength    dd  0             ; Sectors in the FAT
ClustHeapOff dd  0             ; Cluster Heap Offset
ClusterCount dd  0             ; Number of Clusters in Heap
RootCluster  dd  0             ; Cluster of Root Cluster
SerNum       dd  12345678h     ; Volume Serial Number
FSVersion    dw  0100h         ; Version
VolumeFlags  dw  0             ; Volume Flags
logBytesSect db  9             ; bytes per sector (1<<9 = 512)
logSectClust db  1             ; sectors per cluser (1<<1 = 512)
NumFATs      db  1             ; Number of FATs
DriveNumber  db  80h           ; used by the BIOS
HeapUse      db  0             ; percentage of heap in use
             dup 7,0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; to save space in the first sector, and since this doesn't have to be
;  initialized, we reuse this code space for the 26 byte read buffer
read_buffer:

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,4400h           ; first push at 07C0:43FEh
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

not386str  db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now check for the BIOS disk extentions.
; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive_num
           jc   short @f
           cmp  bx,0AA55h
           jne  short @f
           test cx,1
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
           mov  ax,[si+S_INT13_PARMS->sect_size]
           mov  boot_data.sect_size,ax

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the remianing 11 sectors of the boot code.
;
           mov  bx,0200h       ; address 0x07E00
           mov  eax,1          ;
           cdq                 ;
           mov  cx,11          ; 11 more sectors
           call read_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  if we get here, we have successfully loaded the
           ;  second sector.  So let's jump over all of the
           ;  code and data that must remain in the first
           ;  sector before the loading of the second sector.
           jmp  remaining_code
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following code and data must remain in the first sector

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
boot_data     st  S_BOOT_DATA

diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press a key...',0

%print (512-$-2)  ; ~1 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-2)
           dw  0AA55h

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is the second sector loaded by the code above


;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory and FAT into memory
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  first the ROOT
remaining_code:
           push EXFAT_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           xor  edx,edx
           mov  eax,RootCluster
           
           ; root can grow, so we need to read in the cluster, loop until no more clusters.
           
           add  eax,ebx            ; edx:eax = logical sector of start of
           adc  edx,0              ;  root directory (0 based)
           movzx ecx,word nRootEnts ; max root entries allowed
           shr  ecx,4              ; div by 16 = ((x * 32) / 512)
           mov  ebp,eax            ; set up ebp for the 'loader' below
           add  ebp,ecx            ; ebp -> (first sector of data area relative to boot_data.base_lba)
           xor  bx,bx              ;
           call read_sectors_long  ; read in the ROOT directory
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  now the FAT
           push EXFAT_FATSEG       ; 
           pop  es                 ; es-> FAT table
           xor  bx,bx              ;
           movzx eax,word nSecRes  ; edx:eax = logical sector of start of
           xor  edx,edx            ; FAT (0 based)
           movzx ecx,word nSecPerFat  ; sectors per FAT (only need to load one fat)
           cmp  ecx,FATSEG_SIZE    ; if sectors > FATSEG_SIZE, error
           jbe  short @f           ;
           mov  si,offset fat_size_err ; loading message
           jmp  BootErr            ;           
@@:        call read_sectors_long  ; read in the first FAT

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           push EXFAT_ROOTSEG      ;
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
           movzx eax,word es:[di+1Ah]  ; starting cluster number
           movzx ecx,byte nSecPerClust ; number of sectors to read (per cluster)
           push LOADSEG            ; es-> loader seg
           pop  es                 ;
           xor  ebx,ebx            ;
           
           ; since FAT12 should never have a FAT larger than 64k,
           ;  we don't have to worry about 64k wrapping with our addressing
walk_fat:  xor  edx,edx            ; cluster number always < 0FFF8h
           push eax                ; save cluster number
           dec  eax
           dec  eax
           mul  ecx                ; make it cluster based, not sector based
           add  eax,ebp            ; add offset of first data sector
           adc  edx,0              ;
           call read_sectors_long  ; read in cluster (saves all registers used)
           pop  eax                ; restore cluster number
           
.if FAT12_CODE
           push ds                 ; save data segment register
           mov  esi,eax            ;
           push EXFAT_FATSEG       ; es:si -> EXFAT_FATSEG
           pop  ds                 ;
           shr  esi,1              ; offset = n + (n \ 2)
           add  esi,eax             ; 
           test al,1               ; was it odd or even?
           lodsw                   ;
           jz   short is_even      ; 
           shr  ax,4               ; if n is odd, get high 12 bits
is_even:   and  ah,0Fh             ; if n is even, get low 12 bits
.else
           push ds                 ; save data segment register
           mov  esi,eax            ;
           push EXFAT_FATSEG       ; es:si -> EXFAT_FATSEG
           pop  ds                 ;
           add  esi,esi            ; words
           lodsw                   ;
.endif

done_fat:  pop  ds                 ; restore data segment register
           
           ; calculate next position
           push eax
           push edx
           xor  edx,edx
           movzx eax,word boot_data.sect_size
           mul  ecx
           add  ebx,eax
           pop  edx
           pop  eax
           
           cmp  ax,LAST_FAT_ENTRY  ; if last one, then we are done.
           jb   short walk_fat     ; 

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')

           push 07C0h              ; set up some data for LOADER.BIN
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
           xor  eax,eax
           mov  ax,nRootEnts
           mov  boot_data.root_entries,eax
           mov  byte boot_data.file_system,LOADER_FAT_TYPE

           mov  eax,((EXFAT_ROOTSEG << 16) | 0)
           mov  boot_data.root_loc,eax
           mov  eax,((EXFAT_FATSEG << 16) | 0)
           mov  boot_data.other_loc,eax
           xor  ax,ax
           mov  boot_data.misc0,ax
           mov  boot_data.misc1,ax
           mov  boot_data.misc2,ax
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h


os_load_str   db  13,10,'Starting FYSOS...',0
loader_error  db  13,10,07,'Did not find Loader.sys file.',0
fat_size_err  db  13,10,'FAT is larger than FATSEG_SIZE sectors...',0
loadname      db  'LOADER  SYS'


;%print (400h-$)  ; ~110 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of code. Pad out to fill sector
           org 400h

.end
