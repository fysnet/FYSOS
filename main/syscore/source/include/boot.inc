; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;    Copyright (c) 1984-2015    Forever Young Software  Benjamin David Lunt
;
; This code is intended for use with the book it accompanies.
; You may use this code for that purpose only.  You may modify it,
;  and/or include it within your own code as long as you do not
;  distribute it.
; You may not distribute this code to anyone with out permission
;  from the author.
;
;             -- All rights reserved -- Use at your own risk -- 
; 
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

comment /******************************************************************\
*                            FYS OS version 2.0                            *
* FILE: boot.inc                                                           *
*                                                                          *
* EQUates for all boot sectors and loader.asm                              *
*                                                                          *
*  Built with:  NBASM ver 00.26.52                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update:  23 June 2013                                               *
*                                                                          *
***************************************************************************/

; loadseg  128k  This is the max size of our loader file
LOADSEG    equ  3000h          ; start of loader load area   (0x30000 = 192k)
LOADSEG_SIZE  equ  256

; initial stack location
STACK_OFFSET  equ 4400h        ; first push at 07C0:43FEh

; needs to remain 32 bytes
S_BOOT_DATA struct
           SecPerFat    dword  ; FAT: Sectors per FAT
           FATs         byte   ; FAT: Number of FATs
           SecPerClust  byte   ; FAT: Sectors per Cluster
           SecRes       word   ; FAT: Sectors reserved for Boot Record
           SecPerTrack  word   ; FAT: Sectors per Track
           Heads        word   ; FAT: Number of Heads
           root_entries word   ; FAT: Root entries
           base_lba     qword  ; ALL: base lba of partition
           file_system  byte   ; ALL: filesystem number
           drive        byte   ; ALL: BIOS drive number
           sect_size    word   ; ALL: Sector size: 512, 1024, etc.
           reserved     dup 6  ; padding/reserved
S_BOOT_DATA ends

S_INT13_PARMS struct
           buf_size     word   ; caller: size of buffer (1Ah)
           flags        word   ; info flags
           cyls         dword  ; number of physical cyls on disk
           heads        dword  ; number of physical heads on disk
           sect_trck    dword  ; number of physical sectors per track on disk
           tot_sects    qword  ; total number of sectors
           sect_size    word   ; bytes per sectors
S_INT13_PARMS ends

.end
