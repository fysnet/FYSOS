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
 
 ;  Last updated: 19 July 2020

;                ----index---- L PR  ; L = set = LDT, clear = GDT
FLATZERO    equ  0000000000000_0_00b ; PR = protection level (0-3)
FLATCODE    equ  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
FLATDATA    equ  0000000000010_0_00b ;  0 = ring 0 (highest priv.)

FLATCODE16  equ  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
FLATDATA16  equ  0000000000010_0_00b ;  0 = ring 0 (highest priv.)

; size of the data we transfer to the kernel
DATA_SIZE   equ 1000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Our loader/kernel memory layout is as follows
;
               ; 00000000h   ; BIOS/SYSTEM
               ; 00100000h   ;  free
TASKS_BUFF  equ  005D0000h   ; task buffer (1025 * 80)
               ; 005E4050h   ;  free
LSTACK_LOC  equ  005E8000h   ; Local stack (used in INT 20h handler)
LSTACK_SIZE equ    00008000h ; Local stack size
IDT_ADDRESS equ  005F0000h   ; physical address of our IDT
GDTFLGS_LOC equ  005F0800h   ; the GDT Flags buffer (256 bytes)
               ; 005F0900h   ;  free
KERNEL_BASE equ  00600000h   ; Kernel Base
STACK_BASE  equ  01000000h   ; base of stack (bottom)
STACK_SIZE  equ    00400000h ; size of stack
KERN_RAM    equ  01400000h   ; kernel RAM
END_RAM     equ  10000000h   ; end of ram at meg (256 meg)
; the above is were our memory starts and ends
; for the purpose of this book, it assumes you have at least 256 meg.

; standard true and false
TRUE   equ  1
FALSE  equ  0


S_FLOPPY1E struct
           spec0     byte
           spec1     byte
           offdelay  byte
           sect_size byte
           spt       byte
           gaplen    byte
           datalen   byte
           gaplen_f  byte
           filler    byte
           settle    byte
           start     byte
S_FLOPPY1E ends

S_TIME     struct
           year      word
           month     byte
           day       byte
           hour      byte
           min       byte
           sec       byte
           jiffy     byte
           msec      word
           d_savings byte
           weekday   byte
           yearday   word
S_TIME     ends

S_APM      struct
           present      byte
           initialized  byte
           maj_ver      byte
           min_ver      byte
           flags        word
           batteries    byte
           cap_flags    word
           error_code   word
           resv0        dup 5
           cs_segment32 word
           entry_point  dword
           cs_segment   word
           cs16_gdt_idx dword
           cs32_gdt_idx dword
           ds_gdt_idx   dword
           ds_segment   word
           cs_length32  word
           cs_length16  word
           ds_length    word
S_APM      ends

S_MEMORY   struct
           typeused     word    ; (0 = not used, 1 = E820h, 2 = 0E801h, 3 = 88h, 4 = cmos)
           size         qword   ; size of memory in bytes
           blocks       word    ; count of bases returned (usually 2)
           block        dup 128 ; nbasm currently doesn't allow nested structs
S_MEMORY   ends

HALT_ON_ERROR  equ (1<<0)
IS_KERNEL      equ (1<<1)

S_LDR_HDR  struct
           id           dword   ; 0x46595332 = 'FYS2'
           location     dword   ; location to store the file
           comp_type     byte   ; compression type (0=none, 1 = bz2)
           flags         byte   ; halt loader if error loading it, is kernel, etc
           hdr_crc       byte   ; byte sum check sum of hdr
           file_crc     dword   ; uncompressed/moved files crc
           resv         dup 17  ; reserved
S_LDR_HDR  ends

S_DRV_PARAMS struct
           drv_num       byte   ; 80h, 81h, etc. (0 = not used)
           extended_info byte   ; was service 48h used?
           ; start of BIOS data
           ret_size      word   ; size of buffer returned (1Ah = v1.x, 1Eh = v2.x, 42h = v3.x)
           info_flags    word   ; see RBIL table 00274
           cylinders    dword   ; number of physical cylinders
           heads        dword   ; number of physical heads
           spt          dword   ; number of physical sectors per track
           tot_sectors  qword   ; total sectors on drive
           bytes_per_sector word ; bytes per sector
           EDD_config_ptr dword  ; see RBIL table 00278
           v3_sig        word   ; will be BEDD if this info is valid
           v3_len        byte   ; length of this data (includes sig and length) (24h)
           resv0         dup 3  ;
           host_name     dup 4  ; ASCIIZ host ("ISA" or "PCI")
           interface_name dup 8 ; ASCIIZ interface type ("ATA", "ATAPI", "SCSI", "USB", "1394", "FIBRE"
           interface_path dup 8 ; see RBIL table 00275
           device_path    dup 8 ; see RBIL table 00276
           resv1         byte   ;
           v3crc         byte   ; bytes 1Eh-40h = zero  (two's complement)
           ; our reserved to pad to 80 bytes
           padding       dup 12 ;
           
           dpte          dup 16 ; parameters from address EDD_config_ptr above
S_DRV_PARAMS ends


.end
