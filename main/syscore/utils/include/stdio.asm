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
 ;  stdio.asm
 ;   This is the common stdio code that is included for most .asm utilities.
 ;   Make sure you don't include it at the first of you source or it will be
 ;    assembled as the start of your code.
 ;
 ;  Last updated: 19 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;   nbasm embr
 ;

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine prints a char to the screen (stdout)
; On entry: 
;   al = char
; On return:
;   nothing
print_char proc near uses eax edx
           mov  dl,al
           mov  ah,6
           int  21h
           ret
print_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine prints an asciiz string to the screen (stdout)
; On entry: 
;   ds:esi-> segmented address to string
; On return:
;   nothing
print_str  proc near uses eax esi

@@:        lodsb
           or   al,al
           jz   short @f
           call print_char
           jmp  short @b

@@:        ret
print_str  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a decimal value to the screen using a temp buffer to right align it.
; It also will put comma's at every third character
; On entry: 
;   eax = value to print
;   ebx = TRUE = right align
; On return:
;   nothing
prt_dec32  proc near uses eax ecx edx edi esi
           
           ; clear out the print buffer
           push eax
           mov  edi,offset _temp_buff
           mov  al,20h
           mov  ecx,14
           rep
             stosb
           xor  al,al                   ; put a null as the last character
           stosb                        ;
           dec  edi                     ; point to the position just before the '$'
           dec  edi                     ;
           pop  eax                     ; restore the value to print
           
           push ebx
           mov  ecx,3                   ; 3 characters per thousand
           mov  ebx,10
@@:        xor  edx,edx
           div  ebx                     ; Divide by 10
           add  dl,'0'                  ; Convert to ASCII
           mov  [edi],dl 
           dec  edi
           or   eax,eax                 ; Are we done?
           jz   short @f                ; 
           loop @b
           mov  byte [edi],','          ; comma every 3rd character
           dec  edi
           mov  ecx,3
           jmp  short @b
           
@@:        pop  ebx 
           mov  esi,offset _temp_buff   ; pointer to buffer
           or   ebx,ebx                 ; if bx != FALSE, print it
           jnz  short @f                ; 
           lea  esi,[edi+1]             ; else point to the left most character
@@:        call print_str               ; print it
           ret
prt_dec32  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a hexidecimal value to the screen
; On entry: 
;   eax = value to print
; On return:
;   nothing
prt_hex32  proc near uses eax ecx
           mov  ecx,8
@@:        rol  eax,4
           push eax
           and  al,0Fh
           daa
           add  al,0F0h
           adc  al,40h
           call print_char
           pop  eax
           loop @b
           ret
prt_hex32  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine prints a hexidecimal value to the screen
; On entry: 
;   ax = value to print
; On return:
;   nothing
prt_hex16  proc near uses eax ecx
           mov  cx,4
@@:        rol  ax,4
           push ax
           and  al,0Fh
           daa
           add  al,0F0h
           adc  al,40h
           call print_char
           pop  ax
           loop @b
           ret
prt_hex16  endp

_temp_buff dup 16,0

.end
  