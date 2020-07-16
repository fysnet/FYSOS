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

 ;  Last updated: 15 July 2020
 
; initial stack location
STACK_OFFSET  equ 0FFFEh        ; first push at 07C0:FFFE

; count of entries in window at one time, must be at least 2
TOTAL_DISPLAY equ  7

; 'El Torito' Boot Descriptor
S_BRVD       struct
  id              byte   ; 0 = boot
  ident           dup 5  ; 'CD001'
  version         byte   ; 1
  bsident         dup 32 ;
  resv0           dup 32 ;
  boot_cat_lba   dword   ; LBA of Boot Catalog
  resv1           dup 1973
S_BRVD       ends

; Boot Catalog Validation Entry
S_VAL_ENTRY  struct
  id              byte   ; 1
  platform        byte   ; 0, 1, 2
  resv0           word   ;
  ident           dup 24 ;
  crc             word   ; crc
  key55           byte   ;
  keyAA           byte   ;
S_VAL_ENTRY  ends

S_SECT_HDR   struct
  id              byte   ; 0x90 = no more entries, 0x91 = more entries follow
  platform        byte   ; platform values for entries in this section
  num             word   ; number of entries in this section
  ident           dup 28 ;
S_SECT_HDR   ends

S_SECT_ENTRY struct
  bootable        byte   ;
  media           byte   ;
  load_seg        word   ;
  sys_type        byte   ;
  resv0           byte   ;
  load_cnt        word   ;
  load_rba       dword   ;
  criteria        byte   ;
  vendor_spec     dup 19 ;
S_SECT_ENTRY ends

S_SECT_EXTRA struct
  indicator       byte   ;
  next            byte   ;
  vendor_spec     dup 30 ;
S_SECT_ENTRY ends

; allowed total entries
; must be a multiple of 64
ALLOWED_ENTRIES equ 128

; allowed extension entries per entry
ALLOWED_EXTRAS  equ 6

S_OUR_ENTRY  struct
  bootable        byte   ;
  media           byte   ;
  load_seg        word   ;
  sys_type        byte   ;
  platform        byte   ;
  load_cnt        word   ;
  load_rba       dword   ;
  criteria        byte   ;
  vendor_spec     dup (19 + (ALLOWED_EXTRAS * 30))
S_OUR_ENTRY  ends

VEND_SPEC_LINES equ 8
VEND_SPEC_CHARS equ 16  ; must be a multiple of 2 (even number)

.end
