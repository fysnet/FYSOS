comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
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
* Last Updated: 10 Apr 2017                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

; FS types
LEAN    equ   1
EXT2    equ   2
SFS     equ   3

ExFAT   equ  11
FAT12   equ  12
FAT16   equ  16
FAT32   equ  32

FYSFS   equ  22

; initial stack location
STACK_OFFSET  equ 4400h        ; first push at 07C0:4400h (0x0C000) minus size of pushed value

; first two items must remain at top and in that order
S_BOOT_DATA struct
           signature    dword  ; signature used for finding booted from partition
           base_lba     qword  ; base lba of partition
           loader_base  dword  ; physical address of loader.sys
           file_system  byte   ; filesystem number
           drive        byte   ; BIOS drive number
           reserved     dup 30 ; padding/reserved
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
