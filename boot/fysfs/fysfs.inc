comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs.inc                                                          *
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
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm exfat<enter>                                *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

; A buffer relative to 0x07C00
FYSFS_BUFFER     equ  2200h

; rootseg  128k  This allows for the maximum of 256 sectors (65535 entries)
FYSFS_ROOTSEG    equ  1000h          ; segment to load ROOT to (0x10000 = 64k)

; size in sectors
FYSFS_ROOTSEG_SIZE  equ  128

; the super block is at 2000h (17th sector)
FYSFS_SUPER_BLOCK   equ 2000h

S_FYSFS_SUPER struct
           sig0         dword  ; signature   "FYSF" "SUPR"
           sig1         dword  ;
           ver          word   ; version (0x0150)
           sect_clust   word   ; sectors per cluster
           resv0        byte   ;
           bitmaps      byte   ; 1 or 2
           bitmap_flag  word   ; flags describing the 2 bitmaps (see note below)
           root_entries dword  ; count of root entries (size of root)
           base_lba     qword  ; LBA of the first sector of this volume
           root         qword  ; LSN pointer to root
           data         qword  ; LSN pointer to data
           data_sectors qword  ; count of sectors in data area
           sectors      qword  ; total sectors in partition
           bitmap       qword  ; LSN of list of bitmap sectors
           bitmapspare  qword  ; LSN of list of bitmap sectors (second copy)
           chkdsk       dword  ; Seconds since 0:00 1-1-1970 when last time chkdsk was ran
           lastopt      dword  ; Seconds since 0:00 1-1-1970 when last optimized
           flags        dword  ;
           crc          dword  ; 
           guid         dup 16 ;
           vol_label    dup 64 ; asciiz volume label
           filler       dup 340 ; filler
S_FYSFS_SUPER ends

S_FYSFS_ROOT_NEW    equ    534C4F54h  ; 'SLOT'
S_FYSFS_CONT_NAME   equ    4E414D45h  ; 'NAME'
S_FYSFS_CONT_FAT    equ    46415420h  ; 'FAT '

FYSFS_LARGE_FAT     equ    01h

S_FYSFS_ROOT struct
           sig          dword ; 
           attribute    dword ; file attributes
           resv         dup 5 ;
           fat_entries  byte  ; entries in this directory entry
           crc          byte  ; crc of this slot.  8-bit sum
           scratch      byte  ; 
           created      dword ; Secs since 00:00 1-1-80
           lastaccess   dword ; Secs since 00:00 1-1-80  (last time it was closed)
           fsize        qword ; file size
           fat_continue dword ; next root entry that continues with the fat entries (0 if none)
           name_continue dword ; next root entry that continues with the fat entries (0 if none)
           flags        word  ; file flags (encryption, etc)
           namelen      byte  ; length of name in this slot
           resv1        byte  ; crc of this slot.  8-bit sum
           slot         dword ; used in sub directories
           name_fat     dup 80 ;
S_FYSFS_ROOT ends

S_FYSFS_CONT struct
           sig          dword   ; 'NAME' or 'FAT'
           previous     dword   ;
           next         dword   ;
           count        byte    ;
           flags        byte    ;
           crc          byte    ; 8-bit sum
           scratch      byte    ;
           name_fat     dup 112 ;
S_FYSFS_CONT ends

.end
