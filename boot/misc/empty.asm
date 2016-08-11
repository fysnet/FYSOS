comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: empty.asm                                                          *
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
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*   EQUates for lean.asm                                                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm empty<enter>                                *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* This bootsector is written for an empty disk.  i.e.: just the simple     *
*  message and freeze, and the sig at the end.                             *
*                                                                          *
***************************************************************************|

.model tiny                        ;
.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector

outfile 'empty.bin'                ; out file

           org  00h                ; 07C0:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
           cli                     ; don't allow interrupts
           mov  ax,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,ax              ;
           mov  ss,ax              ; start of stack segment (07C0h)
           mov  sp,4000h           ; first push at 07C0:3FFEh
                                   ; (16k size in bytes)
                                   ; (07C0:4000h = 0BC0:0000h which is less
                                   ;      than 0C00:0000h where ROOT resides)
           sti                     ; allow interrupts again


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  print a simple message and freeze (halt)
           mov  si,offset message
           call display_string

@@:        hlt
           jmp  short @b


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
;
display_string proc near
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
display_loop:                      ; ds:si = asciiz message
           cld
           lodsb
           or   al,al
           jz   short end_string
           int  10h                ; output the character
           jmp  short display_loop
end_string: ret
display_string endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

message    db 13,10,7,'I am an empty boot sector.  I will just halt here.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of code. Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-2)
           dw  0AA55h

.end

