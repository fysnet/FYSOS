 ;
 ;                             Copyright (c) 1984-2020
 ;                              Benjamin David Lunt
 ;                             Forever Young Software
 ;                            fys [at] fysnet [dot] net
 ;                              All rights reserved
 ; 
 ; Redistribution and use in source or resulting in  compiled binary forms with or
 ; without modification, are permitted provided that the  following conditions are
 ; met.  Redistribution in printed form must first acquire written permission from
 ; copyright holder.
 ; 
 ; 1. Redistributions of source  code must retain the above copyright notice, this
 ;    list of conditions and the following disclaimer.
 ; 2. Redistributions in printed form must retain the above copyright notice, this
 ;    list of conditions and the following disclaimer.
 ; 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 ;    this list of  conditions and the following  disclaimer in the  documentation
 ;    and/or other materials provided with the distribution.
 ; 
 ; THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 ; AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 ; ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 ; WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 ; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 ; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 ; ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 ; PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 ; USES AS THEIR OWN RISK.
 ; 
 ; Any inaccuracy in source code, code comments, documentation, or other expressed
 ; form within Product,  is unintentional and corresponding hardware specification
 ; takes precedence.
 ; 
 ; Let it be known that  the purpose of this Product is to be used as supplemental
 ; product for one or more of the following mentioned books.
 ; 
 ;   FYSOS: Operating System Design
 ;    Volume 1:  The System Core
 ;    Volume 2:  The Virtual File System
 ;    Volume 3:  Media Storage Devices
 ;    Volume 4:  Input and Output Devices
 ;    Volume 5:  ** Not yet published **
 ;    Volume 6:  The Graphical User Interface
 ;    Volume 7:  ** Not yet published **
 ;    Volume 8:  USB: The Universal Serial Bus
 ; 
 ; This Product is  included as a companion  to one or more of these  books and is
 ; not intended to be self-sufficient.  Each item within this distribution is part
 ; of a discussion within one or more of the books mentioned above.
 ; 
 ; For more information, please visit:
 ;             http://www.fysnet.net/osdesign_book_series.htm
 
 ;
 ;  fysfs.inc
 ;
 ;  Last updated: 19 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;

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
