 ;
 ;                             Copyright (c) 1984-2021
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
 ;  USBBOOT.BIN
 ;   This code is written as a 1.44Meg floppy disk image or a small
 ;    hard drive image (IS_HARDDRIVE).  It then is placed onto a USB 
 ;    Thumb Drive and booted.
 ;   
 ;   The BIOS Emulation will try to detect this image to see if it is
 ;    a floppy image or a hard drive image.  We place both types within
 ;    this image to see what the Emulation does.
 ;  
 ;   Even though the first sector makes it look like a FAT 12/16 volume,
 ;    there is no file system, FAT or other, on this volume.  This
 ;    code is simply to see if it will boot our USB thumb drive.
 ;  
 ;   Hardware Requirements:
 ;    This code uses instructions that are valid for a 386 or later 
 ;    Intel x86 or compatible CPU.
 ;
 ;  Last updated: 17 June 2021
 ;
 ;  Assembled using (NBASM v00.26.80) (http://www.fysnet/newbasic.htm)
 ;   nbasm usbboot
 ;

.model tiny                        ;

outfile 'usbboot.bin'              ; target filename

EXTRA_SECTS      equ  1            ; we load one additional sector

IS_HARDDRIVE     equ  0            ; 0 = use 1.44 floppy emulation, 1 = use hard drive emulation

.if IS_HARDDRIVE
  MEDIA_DESC       equ  0F8h
  SECT_PER_FAT     equ  64
  SECT_PER_TRACK   equ  63
  SECT_PER_CLUST   equ  4
  ROOT_ENTRIES     equ  512
  TOT_HEADS        equ  255
  TOT_SECTS        equ  65500
  FAT_SIZE         equ  'FAT16   '
.else
  MEDIA_DESC       equ  0F0h
  SECT_PER_FAT     equ  9
  SECT_PER_TRACK   equ  18
  SECT_PER_CLUST   equ  1
  ROOT_ENTRIES     equ  224
  TOT_HEADS        equ  2
  TOT_SECTS        equ  2880
  FAT_SIZE         equ  'FAT12   '
.endif

.code                              ;
.rmode                             ; bios starts with (un)real mode
.386P                              ; allow processor specific code for the 386
           org  00h                ; 07C0:0000h

           jmp  short start        ; There should always be 3 bytes of code
           nop                     ;  followed by the start of the data.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;
OEM_Name     db  'FYSOS2.0'    ; 8 bytes for OEM Name and Version
nBytesPerSec dw  0200h         ; 512 bytes per Sector
nSecPerClust db  SECT_PER_CLUST ; Sectors per Cluster
nSecRes      dw  1             ; Sectors reserved for Boot Record
nFATs        db  2             ; Number of FATs
nRootEnts    dw  ROOT_ENTRIES  ; Max Root Directory Entries allowed
nSecs        dw  TOT_SECTS     ; Number of Logical Sectors (0B40h)
                               ;   00h when > 65,535 sectors
mDesc        db  MEDIA_DESC    ; Medium Descriptor Byte
nSecPerFat   dw  SECT_PER_FAT  ; Sectors per FAT
nSecPerTrack dw  SECT_PER_TRACK ; Sectors per Track
nHeads       dw  TOT_HEADS     ; Number of Heads
nSecHidden   dd  00h           ; Number of Hidden Sectors
nSecsExt     dd  00h           ; This value used when there are more
                               ;   than 65,535 sectors on a disc
                               ;   (ie disc size >= 32M)
DriveNum     db  00h           ; Physical drive number
nResByte     db  00h           ; Reserved
             db  29h           ; Signature for Extended Boot Record
SerNum       dd  12345678h     ; Volume Serial Number
VolName      db  'NO NAME    ' ; Volume Label
FSType       db  FAT_SIZE      ; File system type

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
           ;  (with our read_long_sectors code, our SS must be equal to DS)
start:     cli                     ; don't allow interrupts
           mov  ax,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,ax              ;
           mov  es,ax              ;
           mov  ss,ax              ; start of stack segment (0000h)
           mov  sp,0FFFEh          ; first push at 07C0:FFFEh (-2)
           sti                     ; allow interrupts again
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; save the drive value the BIOS sent us
           mov  drive,dl
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print a string to the screen so we know we made it here.
           mov  si,offset made_it_str
           call display_string
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; now let's see if the BIOS emulates the BIOS Extended Disk Read Service(s)
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive number sent
           jc   short @f
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jnz  short @f
           mov  byte extensions,1       ; it does allow the extended read service
@@:        
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; next we need to load the remaining sectors to memory
; either use service 02h or 42h, depending on 'extensions' above
           cmp  byte extensions,1
           je   short use_big_read
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else use service 02h
           mov  ax,(0200h | EXTRA_SECTS)
           mov  cx,0002h       ; first cyl, second sector of the "disk"
           mov  dh,00h         ; first head
           mov  dl,drive       ;
           mov  bx,200h        ;
           int  13h            ;
           jnc  short done_read
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; there was an error reading from the device
disk_read_error:
           mov  si,offset read_error_str
           call display_string
           jmp  short freeze
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else use service 42h
           ; (this assumes SS = DS)
use_big_read:
           push dword 0    ; offset 12
           push dword 1    ; offset 8
           
           push es         ; offset 4
           push 200h
           
           push EXTRA_SECTS ; offset 2
           
           push 10h        ; offset 0
           mov  si,sp
           mov  ah,42h     ; extended read
           mov  dl,drive   ;
           int  13h
           jc   short disk_read_error
           
           add  sp,16      ; remove the items from the stack
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
done_read:
           
           ; there are about 70 bytes free right here to add
           ; more code or other strings.  If this is not enough,
           ; we can always add it below and increment the EXTRA_SECTS
           ; equate.
           
           jmp  do_remaining

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a char to the screen using the BIOS
; On entry:
;       al = char to display
;
display_char proc near uses ax bx
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           int  10h                ; output the character
           ret
display_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
; On entry:
;   ds:si -> asciiz string to display
;
display_string proc near uses ax si
           cld
@@:        lodsb
           or   al,al
           jz   short @f
           call display_char
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display the CR and LF chars, in turn moving to the next line on the screen
display_CRLF proc near uses si
           mov  si,offset crlf_str
           call display_string
           ret
display_CRLF endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; simply halt the processor, allowing interrupts
freeze     proc near
           sti
@@:        hlt
           jmp  short @b
           .noret         ; we don't return, so keep the assembler happy
freeze     endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print a value in hex form
;  "As an additional example, I am still using essentially the same routine
;   which I wrote almost 25 years ago, albeit updated for 32-bit registers."
;  - (the late) Charles A. Crayne
; eax/ax/al = value to be printed
hexdd:  push    eax
        shr     eax,16          ; do high word first
        call    hexdw
        pop     eax
hexdw:  push    eax
        shr     eax,8           ; do high byte first
        call    hexdb
        pop     eax
hexdb:  push    eax
        shr     eax,4           ; do high nibble first
        call    hexdn
        pop     eax
hexdn:  and     al,0Fh          ; isolate nibble
        add     al,'0'          ; convert to ascii
        cmp     al,'9'          ; valid digit?
        jbe     short @f        ; yes
        add     al,7            ; use alpha range
@@:     call    display_char
        ret

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; These must remain in the first sector since we use them before we
;  successfully read the remaining sectors.
made_it_str     db  "We made it so let's print some information: "
crlf_str        db  13,10,0
read_error_str  db  'The called INT 13h service returned an error.  Halting...',13,10,0
extensions      db  0   ; assume we do not allow the extended read services
drive           db  0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%PRINT (200h-$-2-(4*16))               ; ~70 bytes free in this area
           org (200h-2-(4*16))

.if IS_HARDDRIVE
           db  0  ; boot id
             db  0 ; start head
             db  1 ; start sector
             db  0 ; start cyl 
           db  4 ; sys id   4 = FAT16 < 32M (6 = FAT16 >= 32M)
             db  0FFh ; end head
             db  0FFh ; end sector
             db  0FFh ; end cyl 
           dd      0  ; start lba
           dd  65500  ; size in sectors
.else
           db  0  ; boot id
             db  0 ; start head
             db  1 ; start sector
             db  0 ; start cyl 
           db  1 ; sys id   1 = FAT12
             db  1 ; end head
             db 18 ; end sector
             db 79 ; end cyl 
           dd     0  ; start lba
           dd  2880  ; size in sectors
.endif
           dup 16,0
           dup 16,0
           dup 16,0
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-2)
           dw  0AA55h


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of second sector
do_remaining:
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the drive number the BIOS sent us
           mov  si,offset drv_number_str
           call display_string
           mov  al,drive
           call hexdb
           call display_CRLF

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the status of the extension flag
           mov  si,offset extensions_str
           call display_string
           mov  al,extensions
           call hexdn
           call display_CRLF
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; now get the drive parameters either using service 08h or 48h
           cmp  byte extensions,1
           je   short use_big_params
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else use service 08h
           push es
           mov  ah,08h
           mov  dl,drive
           xor  di,di
           mov  es,di
           int  13h
           pop  es
           jc   disk_read_error
           
           ; print the information
           mov  si,offset small_params_type
           call display_string
           mov  al,bl       ; bl = type
           call hexdb
           call display_CRLF
           
           mov  si,offset small_params_cyls
           call display_string
           mov  al,ch
           mov  ah,cl
           shr  ah,6
           call hexdw
           call display_CRLF

           mov  si,offset small_params_heads
           call display_string
           mov  al,dh
           call hexdb
           call display_CRLF

           mov  si,offset small_params_spt
           call display_string
           mov  al,cl
           call hexdb
           call display_CRLF
           
           jmp  short done_params
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; use the extended service
use_big_params:           
           mov  ah,48h
           mov  dl,drive
           mov  si,offset temp_buff
           mov  word [si],42h
           int  13h
           jc   disk_read_error
           
           mov  di,offset temp_buff
           mov  si,offset small_params_cyls
           call display_string
           mov  eax,[di+04h]
           call hexdd
           call display_CRLF
           
           mov  si,offset small_params_heads
           call display_string
           mov  eax,[di+08h]
           call hexdd
           call display_CRLF
           
           mov  si,offset small_params_spt
           call display_string
           mov  eax,[di+0Ch]
           call hexdd
           call display_CRLF
           
           mov  si,offset small_params_total
           call display_string
           mov  eax,[di+10h]
           call hexdd
           call display_CRLF

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;
done_params:
           jmp  freeze

drv_number_str     db  'The BIOS gives us a drive value of: ',0
extensions_str     db  'Extensions supported: ',0
small_params_type  db  ' Type: ',0
small_params_cyls  db  ' Cylinders: ',0
small_params_heads db  ' Heads: ',0
small_params_spt   db  ' SPT: ',0
small_params_total db  ' Total Sectors: ',0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
%PRINT (400h-$)           ; ~200 bytes free in this area before we need
                          ;  another sector added to EXTRA_SECTS
.if ((400h-$) < 0)
%error 1, 'Ben: increment EXTRA_SECTS'
.endif


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; remaining sectors to fill a 1.44Meg disk

; indicate pmode code so we can pad with a value greater than 64k
.pmode

temp_buff  dup ((2880 * 512) - $),00h

.end
