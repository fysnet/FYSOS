comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: usbboot.asm                                                        *
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
* DESCRIPTION:                                                             *
*   test image to see if a machine will boot the USB drive                 *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm usbboot<enter>                              *
*                                                                          *
* Last Updated: 21 Oct 2017                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* This code is written as a 1.44Meg floppy disk image.  It then is         *
*  placed onto a USB Thumb Drive and (try) booted.                         *
*                                                                          *
* The BIOS Emulation will try to detect this image to see if it is         *
*  a floppy image or a hard drive image.  We place both types within       *
*  this image to see what the Emulation does.                              *
*                                                                          *
* Even though the first sector makes it look like a FAT 12 volume,         *
*  there is no file system, FAT 12 or other, on this volume.  This         *
*  code is simply to see if it will boot our USB thumb drive.              *
*                                                                          *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 386 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
***************************************************************************|

.model tiny                        ;

outfile 'usbboot.bin'              ; target filename

EXTRA_SECTS      equ   1           ; count of extra sectors we need loaded
USE_BPB          equ   1           ; include the BPB in our code
USE_FLOPPY       equ   1           ; use values that are compatible with a 1.44 meg floppy
USE_SUPER_FLPY   equ   0           ; use values that are compatible with a "super floppy"
USE_PART_TBLE    equ   0           ; give a partition table


.if USE_SUPER_FLPY
  SECT_PER_TRACK   equ  63
  TOT_HEADS        equ  255
  TOT_SECTS        equ  65500
;.elif USE_FLOPPY
;  SECT_PER_TRACK   equ  18
;  TOT_HEADS        equ   2
;  TOT_SECTS        equ  2880
.else
  SECT_PER_TRACK   equ  63
  TOT_HEADS        equ  16
  TOT_SECTS        equ  65500
.endif

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

           jmp  short start        ; There should always be 3 bytes of code
           nop                     ;  followed by the start of the data.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following figures are specific to a 1.44M 3 1/2 inch floppy disk
;
.if USE_BPB
OEM_Name     db  'FYSOS2.0'    ; 8 bytes for OEM Name and Version
nBytesPerSec dw  0200h         ; 512 bytes per Sector
nSecPerClust db  1             ; Sectors per Cluster
nSecRes      dw  1             ; Sectors reserved for Boot Record
nFATs        db  2             ; Number of FATs
nRootEnts    dw  224           ; Max Root Directory Entries allowed
nSecs        dw  TOT_SECTS     ; Number of Logical Sectors (0B40h)
                               ;   00h when > 65,535 sectors
mDesc        db  0F0h          ; Medium Descriptor Byte
nSecPerFat   dw  9             ; Sectors per FAT
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
FSType       db  'FAT12   '    ; File system type
.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,4400h           ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h which is just
                                   ;    under 0C00:0000h where ROOT resides)

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           push 0F000h             ; if bits 15:14 are still set
           popf                    ;  after pushing/poping to/from
           pushf                   ;  the flags register then we have
           pop  ax                 ;  a 386+
           and  ax,0F000h          ;
           jnz  short @f           ; it's a 386+
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else it is not a 386+ machine, so print a string
           ;  and halt
           mov  si,offset not386str
           call display_string
           jmp  freeze
           
not386str  db  13,10,'Processor is not a 386 compatible processor.',0
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf                   ; restore the interrupt bit

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
           jmp  short done_read
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; else use service 42h
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
           add  sp,16      ; remove the items from the stack
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
done_read:
           
           
           ; there is about 70 bytes free right here to add
           ; more checks or other strings.  If 70 bytes is
           ; not enough, we can always add it below and increment
           ;  the EXTRA_SECTS equate.
           
           
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


freeze     proc near
           sti
@@:        hlt
           jmp  short @b
           .noret         ; we don't return, so keep the assembler happy
freeze     endp


prtcrlf    proc near uses si
           mov  si,offset crlf_str
           call display_string
           ret
prtcrlf    endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print a value in hex form
;  (the late) Charles A. Crayne
;  As an additional example, I am still using essentially the same routine
;  which I wrote almost 25 years ago, albeit updated for 32-bit registers:
; eax = value to be converted
hexdd:  push    eax
        shr     eax,16          ;do high word first
        call    hexdw
        pop     eax
hexdw:  push    eax
        shr     eax,8           ;do high byte first
        call    hexdb
        pop     eax
hexdb:  push    eax
        shr     eax,4           ;do high nibble first
        call    hexdn
        pop     eax
hexdn:  and     eax,0fh         ;isolate nibble
        add     al,'0'          ;convert to ascii
        cmp     al,'9'          ;valid digit?
        jbe     hexdn1          ;yes
        add     al,7            ;use alpha range
hexdn1: call    display_char
        ret

made_it_str   db  "We made it so let's print some information:",13,10,0
extensions    db  0   ; assume we do not allow the extended read services
drive         db  0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55
%PRINT (200h-$-2-(4*16))               ; ~130 bytes free in this area
           org (200h-2-(4*16))

.if USE_PART_TBLE
.if USE_FLOPPY
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
.else
           db  0  ; boot id
             db  0 ; start head
             db  1 ; start sector
             db  0 ; start cyl 
           db  4 ; sys id   4 = FAT16 < 32M (6 = FAT16 >= 32M)
             db  0FFh ; end head
             db  0FFh ; end sector
             db  0FFh ; end cyl 
           dd      0  ; start lba
           dd  65524  ; size in sectors
.endif
           dup 16,0
           dup 16,0
           dup 16,0
.else
           dup (16*4),90h
.endif
           
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
           call prtcrlf
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the status of the extension flag
           mov  si,offset extensions_str
           call display_string
           mov  al,extensions
           call hexdn
           call prtcrlf
           
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
           
           ; print the information
           mov  si,offset small_params_type
           call display_string
           mov  al,bl       ; bl = type
           call hexdb
           ;call prtcrlf
           
           mov  si,offset small_params_cyls
           call display_string
           mov  al,ch
           mov  ah,cl
           shr  ah,6
           call hexdw
           ;call prtcrlf

           mov  si,offset small_params_heads
           call display_string
           mov  al,dh
           call hexdb
           ;call prtcrlf

           mov  si,offset small_params_spt
           call display_string
           mov  al,cl
           call hexdb
           call prtcrlf
           
           jmp  short done_params
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; use the extended service
use_big_params:           
           mov  ah,48h
           mov  dl,drive
           mov  si,offset temp_buff
           mov  word [si],1Ah
           int  13h
           
           mov  di,offset temp_buff
           mov  si,offset small_params_cyls
           call display_string
           mov  eax,[di+04h]
           call hexdd
           ;call prtcrlf

           mov  si,offset small_params_heads
           call display_string
           mov  eax,[di+08h]
           call hexdd
           ;call prtcrlf

           mov  si,offset small_params_spt
           call display_string
           mov  eax,[di+0Ch]
           call hexdd
           ;call prtcrlf

           mov  si,offset small_params_total
           call display_string
           mov  eax,[di+10h]
           call hexdd
           call prtcrlf

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;
done_params:
           jmp  freeze


crlf_str           db  13,10,0
drv_number_str     db  'The BIOS gives us a drive value of: ',0
extensions_str     db  'Extensions supported: ',0
small_params_type  db  ' Type: ',0
small_params_cyls  db  ' Cylinders: ',0
small_params_heads db  ' Heads: ',0
small_params_spt   db  ' SPT: ',0
small_params_total db  ' Total Sectors: ',0



; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
%PRINT (400h-$)           ; ~210 bytes free in this area before we need
                          ;  another sector added to EXTRA_SECTS
.if ((400h-$) < 0)
%error 1, 'Ben: increment EXTRA_SECTS'
.endif


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; remaining sectors to fill a 1.44Meg disk

; indicate pmode code so we can pad with a value greater than 64k
.pmode

temp_buff  dup ((2880 * 512) - $),0

.end
