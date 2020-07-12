;             Author: Benjamin David Lunt
;                     Forever Young Software
;                     Copyright (c) 1984-2016
;  
;  This code is included on the disc that is included with the book
;   FYSOS: The Graphical User Interface, and is for that purpose only.  You
;   have the right to use it for learning purposes only.  You may not modify
;   it for redistribution for any other purpose unless you have written
;   permission from the author.
;
;  You may modify and use it in your own projects as long as they are
;   for non-profit only and not distributed.  Any project for profit that 
;   uses this code must have written permission from the author.
;
;  assemble using NBASM
;    http://www.fysnet.net/newbasic.htm
;    

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Listing 5-16 from Volume 1, "The System Core"
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Set Video Mode service
; On entry: 
;      ebx   16-bit mode number
; On return:
;   Carry clear if service supported
;
set_vid_mode proc near uses eax ebx ecx

    ; set the mode
    mov  eax,00004F02h
    int  10h

    ; al = 4Fh if function supported.
    ; ah = 00h if succeeded.
    cmp  ax,004Fh
    je   short @f     ; if equal, carry = clear
    stc               ; else, set the carry flag
@@: ret          
set_vid_mode endp
