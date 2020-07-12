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

comment /******************************************************************\
*                            FYS OS version 2.0                            *
* FILE: rdtsc.asm                                                          *
*                                                                          *
*  Built with:  NBASM ver 00.26.44                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update: 20 May 2013                                                 *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*   none                                                                   *
*                                                                          *
*                                                                          *
****************************************************************************
*                                                                          *
* If you have any modifications, improvements, or comments, please let me  *
*  know by posting to alt.os.development or emailing me at                 *
*    fys@fysnet.net                                                        *
*                                                                          *
\**************************************************************************/

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
     
