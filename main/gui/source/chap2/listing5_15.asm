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
; Listing 5-15 from Volume 1, "The System Core"
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Get Video Mode Info service
; On entry: 
;   es:edi-> segmented address to store the information
;      ebx   16-bit mode number
; On return:
;   Carry clear if service supported
;
get_mode_info proc near uses eax ebx ecx edi

    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; older versions of the VESA standard did not clear
    ;   out the unused portions of the returned data.
    push edi
    xor  al,al
    mov  ecx,256
    rep
      stosb
    pop  edi

    ; get the mode information
    mov  eax,00004F01h
    mov  ecx,ebx
    int  10h
    
    ; ah = zero if function succeeded, ah = 1 if error
    shr  ah,1        ; the shift shifts bit 0 to the carry
    ret              ; flag. ah = 0 = NC, ah = 1 = carry
get_mode_info endp
