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
* FILE: fysfs.inc                                                          *
*                                                                          *
* EQUates for fysfs.asm                                                    *
*                                                                          *
*  Built with:  NBASM ver 00.26.52                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update:  23 June 2013                                               *
*                                                                          *
***************************************************************************/

; A buffer relative to 0x07C00
FYSFS_BUFFER     equ  2200h

; rootseg  128k  This allows for the maximum of 256 sectors (65535 entries)
FYSFS_ROOTSEG    equ  1000h          ; segment to load ROOT to (0x10000 = 64k)

; size in sectors
FYSFS_ROOTSEG_SIZE  equ  128

; the last 16 bytes of the first 512 byte sector
S_BOOT_SIG struct
           sig         dword   ; signature "FYSB"
           base_lba    qword   ; LBA of the first sector of this volume
           resv         word   ; reserved
           boot_sig     word   ; 0xAA55
S_BOOT_SIG ends

; the super block is at 2000h (17th sector)
SUPER_BLOCK   equ 2000h

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
