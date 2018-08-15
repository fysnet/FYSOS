;/*             Author: Benjamin David Lunt
; *                     Forever Young Software
; *                     Copyright (c) 1984-2018
; *  
; *  This code is donated to the Freeware communitee.  You have the
; *   right to use it for learning purposes only.  You may not modify it
; *   for redistribution for any other purpose unless you have written
; *   permission from the author.
; *
; *  You may modify and use it in your own projects as long as they are
; *   for non-profit only and not distributed.  Any project for profit that 
; *   uses this code must have written permission from the author.
; *
; *  For more information:
; *    http://www.fysnet.net/osdesign_book_series.htm
; *  Contact:
; *    fys [at] fysnet [dot] net
; *
; * Last update:  10 Aug 2018
; *
; * compile using SmallerC  (https://github.com/fysnet/SmallerC)
; *  smlrcc @make.txt
; */

bits 16

extern _sys_block
extern _kernel_base

section .text

    global _finish
_finish:

    cli
    
    ; Load GDTR
    mov     eax,_sys_block
    add     eax,4       ; skip over magic0
    lgdt    [eax]
    add     eax,6
    lidt    [eax]
    
    mov     eax,CR0      ; get the CR0 value
    push    eax          ; save it
    mov     eax,1        ; if we support SSE, we need to...
    cpuid 
    test    edx,(1<<25)
    jz      short .noSSE
    pop     eax
    and     ax,0xFFFB    ; ...clear coprocessor emulation CR0.EM
    or      ax,2         ; ...set coprocessor monitoring  CR0.MP
    push    eax          ; save the cr0 value
    mov     eax,cr4      ; 
    or      ax,(3<<9)    ; ...set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov     cr4,eax
.noSSE:
    pop     eax          ; restore the cr0 value
    and     eax,~60000000h  ; clear CR0.CD and CR0.NW bits (Cache Disable, Non-Write through)
    or      al,21h       ; and set the NE and PE bits
    mov     CR0,eax
    
    ; Here is the jump.  Intel recommends we do a jump.
    jmp     0x08:dword .prot_mode
    
.still_in_16bit:
    hlt
    jmp short .still_in_16bit
    

bits 32
  ; let's make sure we made it here.
  ; if still in 16bit mode,
  ;  the below will look like this ---->
.prot_mode:                          ; xor  ax,ax     \n"
    xor     eax,eax                  ; xor  ax,0000h  \n"
    xor     eax,0C08B0000h           ; mov  ax,ax     \n"
    jz      short .still_in_16bit     ; jz   short still_in_16bit \n"
  
    ; we need to make sure that CR4.TSD = 0 (bit 2)
    ; i.e.: allows the RDTSC instruction
    ; we need to make sure that CR4.VME = 0 (bit 0)
    mov     eax,cr4
    and     al,~5
    mov     cr4,eax
    
    ; set up the segment descriptors
    ; ds, es, fs, and ss have a base of 00000000h
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    
    ; set up a stack at STACK_BASE (physical) of 4 meg size
    mov     esp,((0x01000000 + 00400000h) - 4)
            
    ; We now have PMODE setup and all our segment selectors correct.
    ; CS              = 0x00000000
    ; SS & remaining  = 0x00000000
    
    ; We will jump to physical address 'kernel_base' + 400h
    mov     eax,[dword _kernel_base]
    add     eax,400h
    mov     [dword .kernel_addr],eax
    
             db  0EAh
.kernel_addr dd  0      ; *in* pmode, so *dword* sized
             dw  0x08
    
    ; Kernel file should have taken over from here
.halt:
    hlt
    jmp short .halt
