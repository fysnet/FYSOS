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

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set_screen(x_min, x_max, y_min, y_max)
; sets the coordinates of the screen
; On entry:  (x_min, x_max, y_min, y_max)
; On return: nothing
set_screen proc near
           
           push ebp
           mov  ebp,esp
           
           ; make sure that the new settings are within our range
           mov  eax,PARAM0
           mov  ecx,scrn_width
           call min
           mov  scrn_width_min,eax
           
           mov  eax,PARAM1
           mov  ecx,scrn_width
           call min
           mov  scrn_width_max,eax
           
           mov  eax,PARAM2
           mov  ecx,scrn_height
           call min
           mov  scrn_height_min,eax
           
           mov  eax,PARAM3
           mov  ecx,scrn_height
           call min
           mov  scrn_height_max,eax
           
           mov  eax,scrn_width_min
           cmp  eax,curx
           jb   short @f
           mov  curx,eax
           
@@:        mov  eax,scrn_width_max
           cmp  eax,curx
           ja   short @f
           mov  curx,eax

@@:        mov  eax,scrn_height_min
           cmp  eax,cury
           jb   short @f
           mov  cury,eax
           
@@:        mov  eax,scrn_height_max
           cmp  eax,cury
           ja   short @f
           mov  cury,eax

@@:        pop  ebp
           ret  16
set_screen endp

; these are default values, it will be up to you
;  to update them within your kernel code
screen_mem       dd 000B8000h
           
scrn_width       dd 80
scrn_height      dd 25

scrn_width_min   dd  0
scrn_width_max   dd 80
scrn_height_min  dd  0
scrn_height_max  dd 25

curx             dd 0
cury             dd 0

FORCOL             equ  7
BCKCOL             equ  0
DEFAULT_TAB_SPACES equ  2

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; putch(char)
; prints a char to the screen at curx,cury and
;  advances the cursor, scrolling the screen if
;  needed.
; On entry:  (char)
; On return: char
putch      proc near
           
           push ebp
           mov  ebp,esp
           
           cmp  dword PARAM0,13   ; '\r'
           jne  short putch_not_13
           mov  ecx,scrn_width_min
           mov  curx,ecx
           jmp  putch_done
           
putch_not_13:           
           cmp  dword PARAM0,10   ; '\n'
           jne  putch_not_10
           
           inc  dword cury
           mov  ecx,scrn_height_max
           cmp  cury,ecx
           jb   putch_done
           
           ; we need to scroll the viewable screen contents up
           
           ; saved perserved registers
           push edi
           push esi
           push ebx
           
           ; point to top left corner of viewable screen
           ; targ_ptr = (screen_mem + (scrn_height_min * scrn_width * 2) + (scrn_width_min * 2))
           mov  edi,screen_mem
           mov  eax,scrn_height_min
           mov  ecx,scrn_width
           mul  ecx
           shl  eax,1   ; * 2
           add  edi,eax
           mov  eax,scrn_width_min
           shl  eax,1   ; * 2
           add  edi,eax
           
           ; point to next line of viewable screen
           ; src_ptr = (targ + (scrn_width * 2))
           mov  esi,edi
           mov  eax,scrn_width
           shl  eax,1   ; * 2
           add  esi,eax
           
           ; y=scrn_height_min; y<scrn_height_max-1; y++
           mov  ecx,scrn_height_min
           mov  edx,scrn_height_max
           dec  edx
putch_0:   cmp  ecx,edx
           jnb  short putch_3
           push ecx
           push edx
           ; i = 0
           xor  ebx,ebx
           ; x=scrn_width_min; x<scrn_width_max; x++
           mov  ecx,scrn_width_min
           mov  edx,scrn_width_max
putch_1:   cmp  ecx,edx
           jnb  short putch_2
           ; targ[i] = src[i]
           mov  ax,[esi+ebx]
           mov  [edi+ebx],ax
           ; i++
           add  ebx,2  ; words
           inc  ecx
           jmp  short putch_1
putch_2:   pop  edx
           pop  ecx
           
           ; src += (scrn_width * 2)
           ; targ += (scrn_width * 2)
           mov  eax,scrn_width
           shl  eax,4
           add  esi,eax
           add  edi,eax
           
           inc  ecx
           jmp  short putch_0
           
           ; x=scrn_width_min; x<scrn_width_max; x++
putch_3:   mov  ecx,scrn_width_min
           mov  edx,scrn_width_max
putch_4:   cmp  ecx,edx
           jnb  short putch_5
           mov  ax,(20h | (FORCOL | (BCKCOL << 4)) << 8)
           mov  [edi],ax
           add  edi,2  ; words
           inc  ecx
           jmp  short putch_4
           
           ; cury = scrn_height_max - 1
putch_5:   mov  eax,scrn_height_max
           dec  eax
           mov  cury,eax
           
           ; restore preserved registers
           pop  ebx
           pop  esi
           pop  edi
           jmp  putch_done
           
putch_not_10:
           cmp  dword PARAM0,08  ; backspace key
           jne  short putch_not_8
           
           ; if (curx > scrn_width_min) curx--
           mov  ecx,scrn_width_min
           cmp  curx,ecx
           jna  short @f
           dec  dword curx

@@:        ; targ_ptr = (screen_mem + (cury * scrn_width * 2) + (curx * 2))
           mov  edi,screen_mem
           mov  eax,cury
           mov  ecx,scrn_width
           mul  ecx
           shl  eax,1   ; * 2
           add  edi,eax
           mov  eax,curx
           shl  eax,1   ; * 2
           add  edi,eax
           
           ; put a space to overwrite last char
           mov  ax,(20h | (FORCOL | (BCKCOL << 4)) << 8)
           mov  [edi],ax
           jmp  short putch_done
           
putch_not_8:
           cmp  dword PARAM0,09  ; tab key
           jne  short putch_not_9
           
           mov  ecx,DEFAULT_TAB_SPACES
@@:        push 32
           call putch            ; recurse
           loop @b
           jmp  short putch_done           
           
putch_not_9:
           ; targ_ptr = (screen_mem + (cury * scrn_width * 2) + (curx * 2))
           mov  edi,screen_mem
           mov  eax,cury
           mov  ecx,scrn_width
           mul  ecx
           shl  eax,1   ; * 2
           add  edi,eax
           mov  eax,curx
           shl  eax,1   ; * 2
           add  edi,eax
           
           ; put the char
           mov  eax,PARAM0
           or   eax,((FORCOL | (BCKCOL << 4)) << 8)
           mov  [edi],ax
           
           ; curx++
           inc  dword curx
           
           ; if (curx >= scrn_width_max)
           mov  ecx,scrn_width_max
           cmp  curx,ecx
           jnae short putch_done
           
           ; curx = scrn_width_min
           mov  ecx,scrn_width_min
           mov  curx,ecx
           
           ; putch(LF);  // recurse for a LF
           push 10
           call putch
           
putch_done:
           ; set hardware cursor position
           push dword cury
           push dword curx
           call set_cursor_pos
           
           ; return char
           mov  eax,PARAM0
           
           pop  ebp
           ret  4
putch      endp           



; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; putstr()
; prints a string to the screen
; On entry:  (str)
; On return: nothing
putstr     proc near
           
           push ebp
           mov  ebp,esp
           push esi
           
           xor  eax,eax
           mov  esi,PARAM0
@@:        lodsb
           or   al,al
           jz   short @f
           push eax
           call putch
           jmp  short @b           
           
@@:        pop  esi
           pop  ebp
           ret  4
putstr     endp

; CRT Controll Registers
; we assume color screen, not mono
VID_CNTRL_ADDR  equ  (0304h | 03D0h)  ; CRTC contrller address register
VID_CNTRL_DATA  equ  (0305h | 03D0h)  ; CRTC contrller data register

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set_cursor_pos(x, y)
; sets the hardware cursor position
; On entry:  (x, y)
; On return: nothing
set_cursor_pos proc near
           
           push ebp
           mov  ebp,esp
           
           ; pos = (y * scrn_width) + x
           mov  eax,scrn_width
           mov  ecx,PARAM1
           mul  ecx
           add  eax,PARAM0
           
           push 0Eh
           push VID_CNTRL_ADDR
           call outpb
           
           ; high byte first
           push eax
           shr  eax,8
           push eax
           push VID_CNTRL_DATA
           call outpb
           pop  eax
           
           push 0Fh
           push VID_CNTRL_ADDR
           call outpb
           
           ; then the low byte
           push eax
           push VID_CNTRL_DATA
           call outpb
           
           pop  ebp
           ret  8
set_cursor_pos endp

.end
