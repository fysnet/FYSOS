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

S_IDT         struct
	handlerlow  word        ; low word of handler offset
	selector    word        ; selector value
	zero        byte        ; zero
	type        byte        ; type
	handlerhigh word        ; high word of handler offset
S_IDT         ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following are the PIC includes.
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PIC_MASTER      equ  20h
MASTER_DEFAULT  equ  0F8h    ; IRQ7  - IRQ0
PIC_SLAVE       equ  0A0h  
SLAVE_DEFAULT   equ  0FFh    ; IRQ15 - IRQ8
EOI             equ  20h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following are simple STDLIB/STDIO/etc includes.
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
NULL           equ 0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; stack contents on procedure call
; caller pushes from right to left
;
; proc(param0, param1, param2, param3)
;
;  /------------\
;  |   param3   | +20 4th parameter
;   ------------
;  |   param2   | +16 3rd parameter
;   ------------
;  |   param1   | +12 2nd parameter
;   ------------
;  |   param0   | +08 1st parameter
;   ------------
;  |   return   | +04 return address
;   ------------
;  |  org ebp   | +00 <- offset from ebp
;  \------------/
;

PARAMEBP       equ [ebp+ 0]
PARAMRET       equ [ebp+ 4]
PARAM0         equ [ebp+ 8]
PARAM1         equ [ebp+12]
PARAM2         equ [ebp+16]
PARAM3         equ [ebp+20]
PARAM4         equ [ebp+24]
PARAM5         equ [ebp+28]
PARAM6         equ [ebp+32]
PARAM7         equ [ebp+36]
PARAM8         equ [ebp+40]

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Local stack space parameters
LOCAL0         equ [ebp- 4]
LOCAL1         equ [ebp- 8]
LOCAL2         equ [ebp-12]
LOCAL3         equ [ebp-16]
LOCAL4         equ [ebp-20]
LOCAL5         equ [ebp-24]
LOCAL6         equ [ebp-28]
LOCAL7         equ [ebp-32]
LOCAL8         equ [ebp-36]
LOCAL9         equ [ebp-40]


.end
