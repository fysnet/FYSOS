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
 ;  boot.inc
 ;
 ;  Last updated: 19 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;

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
