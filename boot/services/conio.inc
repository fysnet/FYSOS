comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: conio.inc                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*   Read Sectors Long                                                      *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 10 Apr 2017                                                *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we have included this after the first sector, we are in error...
.ifndef SKIP_200h_CHECK
.ifa $, 200h
%error 1, 'This must be in the first sector of the code...'
.endif
.endif

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

.end
