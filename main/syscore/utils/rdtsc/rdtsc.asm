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
 ;  rdtsc.asm
 ;
 ;  Last updated: 19 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;   nbasm embr
 ;

.model tiny                        ;

include '..\include\stdio.inc'

; start of our code
.code                              ;
.stack  1024                       ; use a stack size of 1024 bytes
.rmode                             ; bios starts with real mode
.486                               ; we assume 486

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; DOS .COM files start at 100h
           org  100h
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this free's up any unused memory and sets up a stack for us
           .start
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; make sure segment registers are good
           mov  ax,cs
           mov  ds,ax
           mov  es,ax
      
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; print the header and copywrite strings.
           mov  esi,offset hdr_copy_str
           call print_str

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; call the check
           call chk_486
           
           ; calculate string offset
           mov  si,offset found_str
           shl  ax,1
           add  si,ax
           mov  si,[si]
           call print_str
           .exit


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Checks for a 486+ machine with the CPUID and RDTSC
;  instructions.
; on entry: nothing
; on exit:
;  carry set = NOT a 486+
;  carry clear = 486+ with CPUID and RDTSC
;    ax = 0 = 8086
;         1 = 186
;         2 = 286
;         3 = 386
;         4 = 486+ without CPUID
;         5 = 486+ with CPUID but not RDTSC,
;         6 = 486+ with CPUID and RDTSC
chk_486    proc near uses bx cx dx
           pushf             ; save the original flags value
    
           mov  ax,00h       ; Assume an 8086
           mov  cx,0121h     ; If CH can be shifted by 21h,
           shl  ch,cl        ; then it's an 8086, because
           jz   short @f     ; a 186+ limits shift counts.
           push sp           ; If SP is pushed as its
           pop  ax           ; original value, then
           cmp  ax,sp        ; it's a 286+.
           mov  ax,01h       ; is 186
           jne  short @f     ;
           mov  ax,7000h     ; if bits 12,13,14 are still set
           push ax           ; after pushing/poping to/from
           popf              ; the flags register then we have
           pushf             ; a 386+
           pop  ax           ;
           and  ax,7000h     ;
           cmp  ax,7000h     ;
           mov  ax,02h       ; is 286
           jne  short @f     ; it's a 386

           ; =-=-=-=-=- test for .486
           ; if we can toggle bit 18 in EFLAGS (AC bit) we have a
           ;  486+.  The 386 doesn't have the AC bit.
           cli
           pushfd
           pop  eax
           mov  ebx,eax
           xor  eax,00040000h ; bit 18
           push eax
           popfd
           pushfd
           pop  eax
           push ebx
           popfd
           sti            
           xor  eax,ebx
           mov  ax,03        ; is 386
           jz   short @f     ; else it's a 486+
        
           ; =-=-=-=-=- test for CPUID
           ; if we can toggle bit 21 in EFLAGS (ID bit) we have a
           ;  486+ with the CPUID instruction
           cli
           pushfd
           pop  eax
           mov  ebx,eax
           xor  eax,00200000h  ; bit 21
           push eax
           popfd
           pushfd
           pop  eax
           push ebx
           popfd
           sti            
           xor  eax,ebx
           mov  ax,04       ; is 486+ without CPUID
           jz   short @f    ; else it's a 486+ with CPUID

           ; =-=-=-=-=- test for RDTSC
           ; do CPUID with function 1.  If bit 4 of EDX on return,
           ;  we have the RDTSC instruction
           mov  eax,1
           cpuid
           test edx,10h
           mov  ax,05      ; is 486+ with CPUID but without RDTSC
           jz   short @f
        
           ; =-=-=-=-=- We got a 486+ with the CPUID
           ;  and RDTSC instructions
           popf            ; restore the original flags value
           mov  ax,06      ; is 486+ with CPUID and RDTSC
           clc             ; so clear the carry and 
           ret             ;  return
       
@@:        popf            ; restore the original flags value
           stc             ; not a 386+, so set the carry and
           ret             ;  return
chk_486    endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the print routines and other items
include '..\include\stdio.asm'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strings and other data
hdr_copy_str     db  13,10,'RDTSC Test  v1.00.00   Copyright 1984-2013   Forever Young Software',13,10,0

found_str        dw  offset fnd_8086_str
                 dw  offset fnd_186_str
                 dw  offset fnd_286_str
                 dw  offset fnd_386_str
                 dw  offset fnd_486_noCPUID_str
                 dw  offset fnd_486_noRDTSC_str
                 dw  offset fnd_486_plus_str

fnd_8086_str        db  13,10,'Found an 8086 processor.',0
fnd_186_str         db  13,10,'Found an 186 processor.',0
fnd_286_str         db  13,10,'Found an 286 processor.',0
fnd_386_str         db  13,10,'Found an 386 processor.',0
fnd_486_noCPUID_str db  13,10,'Found an 486 processor, but no CPUID or RDTSC.',0
fnd_486_noRDTSC_str db  13,10,'Found an 486 processor with CPUID, but not RDTSC.',0
fnd_486_plus_str    db  13,10,'Found an 486 or better processor with CPUID and RDTSC.',0
                 
.end
