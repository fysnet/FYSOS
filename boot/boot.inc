comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: boot.inc                                                           *
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
*   EQUates for various boot files                                         *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

; FS types
LEAN    equ   1
EXT2    equ   2

FAT12   equ  12
FAT16   equ  16
FAT32   equ  32

FYSFS   equ  22


; loadseg  128k  This is the max size of our loader file
LOADSEG    equ  3000h          ; start of loader load area   (0x30000 = 192k)
;LOADSEG_SIZE  equ  256

; initial stack location
STACK_OFFSET  equ 4400h        ; first push at 07C0:43FEh

; save location of the boot sector for the kernel to find the booted from drive
SAVE_SEG      equ 07A0h

S_BOOT_DATA struct
           SecPerFat    dword  ;  FAT: Sectors per FAT
           FATs         byte   ;  FAT: Number of FATs
           SecPerClust  word   ;  FAT: Sectors per Cluster
           SecRes       word   ;  FAT: Sectors reserved for Boot Record
           SecPerTrack  word   ;  FAT: Sectors per Track
           Heads        word   ;  FAT: Number of Heads
           root_entries dword  ;  FAT: Root entries
           base_lba     qword  ;  ALL: base lba of partition
           file_system  byte   ;  ALL: filesystem number
           drive        byte   ;  ALL: BIOS drive number
           sect_size    word   ;  ALL: Sector size: 512, 1024, etc.
           root_loc     dword  ;  ALL: Far pointer (seg:off) to current root location (loaded by the boot code)
           other_loc    dword  ; SOME: Far pointer (seg:off) to current super/fat/etc location
           misc0        word   ; SOME: Misc word used buy some file systems
           misc1        word   ; SOME: Misc word used buy some file systems
           misc2        word   ; SOME: Misc word used buy some file systems
           reserved     dup 5  ; padding/reserved
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

S_INT1E_PARMS struct
           first_spec    byte  ; first specify byte
           second_spec   byte  ; second specify byte
           delay         byte  ; delay until motor turned off
           sect_size     byte  ; bytes per sector (00h = 128, 01h = 256, 02h = 512, 03h = 1024)
           sect_trck     byte  ; sectors per track
           gap_len       byte  ; length of gap between sectors (2Ah for 5.25", 1Bh for 3.5")
           data_len      byte  ; data length (ignored if bytes-per-sector field nonzero)
           gap_len_f     byte  ; gap length when formatting (50h for 5.25", 6Ch for 3.5")
           format_fill   byte  ; format filler byte (default F6h)
           head_settle   byte  ; head settle time in milliseconds
           motor_start   byte  ; motor start time in 1/8 seconds
S_INT1E_PARMS ends

.end
