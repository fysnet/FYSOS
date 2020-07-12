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
