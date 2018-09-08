comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: stricmp.inc                                                        *
*                                                                          *
* DESCRIPTION:                                                             *
*   Compares two strings (case insensitive)                                *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 26 June 2017                                               *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this routine compares (case insensitive) two strings
; ds:si->string0 (asciiz)
; es:di->string1 (asciiz)
; returns  zero flag set if equal
;          carry flag set if string0 < string1
;          carry flag clear if string0 > string1
;  preserves: si, di
;   destroys: ax
stricmp    proc near uses si di
           
@@:        mov  al,es:[di]
           inc  di
           call upper_case
           mov  ah,al
           
           lodsb
           call upper_case
           
           ; if both strings are equal and we are at
           ;  the null terminator, we need to exit now
           or   ax,ax
           jz   short @f
           
           ; 'sub' will set/clr the zero flag
           ;   and the carry flag for us
           sub  al,ah
           jz   short @b
           
@@:        ret
stricmp    endp

upper_case proc near
           cmp  al,'a'
           jb   short @f
           cmp  al,'z'
           ja   short @f
           sub  al,('a'-'A')
@@:        ret
upper_case endp

.end
