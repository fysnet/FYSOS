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
; Listing 5-14 from Volume 1, "The System Core"
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Get VESA Info service
; On entry: 
;   es:edi-> segmented address to store the information
;   es:esi-> segmented address to store the supported modes
;   es:ebx-> segmented address to store the OEM Name
; On return:
;   Carry clear if service supported
;
vesa_service proc near uses eax ebx ecx edx esi edi

    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; first, clear it out.
    push edi
    xor  al,al
    mov  ecx,512
    rep
      stosb
    pop  edi
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; If we want only the VESA info, we put 'VESA' in the
    ;  first dword of the buffer.  If we want VBE2+ (VBE3),
    ;  then we put 'VBE2'.
    mov  eax,00004F00h
    mov  dword es:[edi],'2EBV'
    int  10h
    jc   short vesa_done
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; on return, if the carry is clear, but ax != 0x004F
    ;  then this wasn't a good call.
    cmp  ax,004Fh
    jne  short vesa_done
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; else it was a good call, so we have a valid buffer.
    ; let's extract the supported modes. (we just 
    ; extract 128 words. The print routine stops at 0xFFFF)
    xchg edi,esi  ; es:edi -> passed pointer to buffer
                  ; es:esi -> vesa info buffer
    push ds
    push esi
    lds  si,es:[esi + 0Eh]
    mov  cx,TOT_MODES
    rep
    movsw
    pop  esi
    pop  ds
    
    ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ; get the oem name.  We have to do this incase the BIOS
    ; builds the list on the fly.
    mov  edi,ebx      ; es:ebx -> buffer to hold OEM name
    push ds
    lds  si,es:[esi + 06h]
    mov  cx,32
    rep
    movsb
    xor  al,al
    stosb
    pop  ds
    
vesa_done:
    clc
    ret
vesa_service endp
