comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: address.inc                                                        *
*                                                                          *
* DESCRIPTION:                                                             *
*   "Chooses" an address to load the loader.sys file to.                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 05 May 2017                                                *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine "chooses" an address to load the loader to.
; We can load it anywhere from 0x0C000 to EBDA (0x9FC00) - 0x30000
;  on paragraph boundaries. (- 0x30000 for size we use for the loader.sys)
; We do this, first, because we can, second for the fun of it, and
;  third, it has some security benefits of not allowing a malacious
;  act of assuming the loader file is at a certain place.
; On entry:
;  nothing
; On return:
;  ebx = physical address
;  
set_up_address proc near uses ds ax cx
           
           mov  ebx,00030000h  ;;; for some reason, the below code fails on occasion
           ret

           xor  ebx,ebx
           mov  ds,bx
           
           mov  ax,16    ; try 16 times before using fixed address
address_loop:
           ; "random" value from timer tick
           mov  bx,[046Ch + 0]
           mov  cl,[046Ch + 1]
           xor  bx,[046Ch + 2]
           ror  bx,cl
           
           ; so that we don't cross a 64k boundary when reading from
           ;  the disk, we need to make sure we are on a sector (200h) boundary
           
           ; this makes it a larger number *and* clears bits 8:0 for us.
           shl  ebx,9
           ; (also, mask off the hi bits)
           and  ebx,0007FE00h
           
           ; 0x0C000 <= ebx <= (0x9FC00-0x30000)
           cmp  ebx,0x0C000
           jb   short address_try_again
           cmp  ebx,(0x9FC00-0x30000)
           jbe  short good_address
           
address_try_again:
           ; we need to wait for just a little while, or we will
           ; get the same response again
           mov  bx,[046Ch]
@@:        cmp  bx,[046Ch]
           je   short @b
           
           dec  ax
           jnz  short address_loop
           
           ; after 16 tries, just use fixed address
           ; (this keeps it from stalling)
           mov  ebx,00030000h
           
good_address:           
           ret
set_up_address endp

.end
