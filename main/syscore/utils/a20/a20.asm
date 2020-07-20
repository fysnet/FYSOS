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
 ;  A20.ASM
 ; Notes:
 ; This code must be assembled with NBASM version 00.26.31 or later.  Any
 ;  version before this will not correctly assemble the 64-bit immediate
 ;  values with the DQ declaration.  Versions before also had a bug in the
 ;  IMUL reg32,reg3,immed instruction.
 ;
 ; This code assumes a 32-bit machine with out checking first.
 ;
 ;  Last updated: 19 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;   nbasm embr

.model tiny                        ;

include '..\include\stdio.inc'

; start of our code
.code                              ;
.stack  1024                       ; use a stack size of 1024 bytes
.rmode                             ; bios starts with real mode
.386                               ; we assume 386

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
; test to see if the A20 line is on
           mov  edi,offset list
main_loop: mov  esi,[edi]
           add  edi,4
           or   esi,esi
           jz   short main_done
           
           ; first deactivate it, then
           ; make sure that the deactivation worked
           call deactivate
           call test_a20
           jz   short deactive_ok
           mov  esi,offset deactive_err_str
           call print_str
           jmp  short main_done

deactive_ok:           
           call print_str
           mov  eax,[edi]
           add  edi,4
           call eax
           jnc  short method_ok
method_err:
           mov  esi,offset method_err_str
           call print_str
           jmp  short main_loop

method_ok:
           call test_a20
           jz   short method_err
           mov  esi,offset method_str
           call print_str
           jmp  short main_loop
           
main_done:
           .exit


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Deactivate A20
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
deactivate proc near uses ax
    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short deactivate_err  ; return carry if timed out
    mov  al,0D0h
    out  64h,al

    ; wait while keyboard status bit 0 = 0
    call wait_kbd_status1
    jc   short deactivate_err  ; return carry if timed out
    in   al,60h
    mov  ah,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short deactivate_err  ; return carry if timed out
    mov  al,0D1h
    out  64h,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short deactivate_err  ; return carry if timed out
    mov  al,ah
    and  al,(~02)
    out  60h,al
    
    ; Wait for the A20 line to settle down (up to 20usecs)
    mov  al,0FFh     ; Send FFh (Pulse Output Port NULL)
    out  64h,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short deactivate_err  ; return carry if timed out
    
    ; else, successfully set bit 1
    
deactivate_err:
    ; we are going to do this method also just to make sure
    in   al,92h     ; read in the port
    and  al,0FCh    ; clear bit 1 and make sure we don't do a reset
    out  92h,al     ; write it back
    clc             ; let the test_a20 determine if it was successful
    ret
deactivate endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 1: keyboard controller
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
method_one proc near uses ax
    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_one_done  ; return carry if timed out
    mov  al,0D0h
    out  64h,al

    ; wait while keyboard status bit 0 = 0
    call wait_kbd_status1
    jc   short method_one_done  ; return carry if timed out
    in   al,60h
    mov  ah,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_one_done  ; return carry if timed out
    mov  al,0D1h
    out  64h,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_one_done  ; return carry if timed out
    mov  al,ah
    or   al,02
    out  60h,al
    
    ; Wait for the A20 line to settle down (up to 20usecs)
    mov  al,0FFh     ; Send FFh (Pulse Output Port NULL)
    out  64h,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_one_done  ; return carry if timed out
    
    ; else, successfully set bit 1
    clc
method_one_done:
    ret
method_one endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 2: Fast A20, Port 0x92
; on entry:
;  nothing
; on exit:
;  nothing
method_two proc near uses ax
    in   al,92h     ; read in the port
    test al,(1<<1)  ; is the bit already set?
    jnz  short @f   ;
    or   al,(1<<1)  ; else, set bit 1
    and  al,0FEh    ; make sure we don't do a reset
    out  92h,al     ; write it back
@@: ret             ; return to caller
method_two endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 3: keyboard controller, 0xDF
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
method_three proc near uses ax
    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_thr_done  ; return carry if timed out
    mov  al,0DFh
    out  60h,al

    ; read in the ACKnowledge
    ; wait while keyboard status bit 0 = 0
    call wait_kbd_status1
    jc   short method_thr_done  ; return carry if timed out
    in   al,60h
    cmp  al,0FAh   ; was it the ACKnowledge byte?
    mov  ah,al
    jne  short method_thr_done  ; return if not

    ; Wait for the A20 line to settle down (up to 20usecs)
    mov  al,0FFh     ; Send FFh (Pulse Output Port NULL)
    out  64h,al

    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_thr_done  ; return carry if timed out
    
    ; else, successful
    clc
method_thr_done:
    ret
method_three endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 4: Brute Force of Method 1
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
method_four proc near uses ax
    mov  al,0D1h
    out  64h,al
    
    ; wait while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_for_done  ; return carry if timed out

    mov  al,0DFh
    out  60h,al
    
    ; waits while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_for_done  ; return carry if timed out

    mov  al,0FFh
    out  64h,al
    
    ; waits while keyboard status bit 1 = 1
    call wait_kbd_status2
    jc   short method_for_done  ; return carry if timed out
    
    ; else, successful
    clc
method_for_done:
    ret
method_four endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 5: BIOS a20 enable service (later PS/2's)
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
method_five proc near uses ax
    mov  ax,2401h
    int  15h
    jc   short method_fiv_done  ; return carry not supp?ted
    cmp  ah,00
    jne  short method_fiv_done ; returns 0 if not supp?ted
    
    ; else, successful
    clc
    ret
method_fiv_done:
    stc
    ret
method_five endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Method 6: Port 0xEE
; on entry:
;  nothing
; on exit:
;  carry clear if successful
;  (successful only means the bit was set,
;     not that the A20 line is on)
method_six proc near uses ax dx
    mov  dx,0EEh
    in   al,dx
    clc
    ret
method_six endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; waits while keyboard status bit 1 = 1
; returns carry if timed out
wait_kbd_status2 proc near uses ax cx
    
    ; we will try 65536 times before a time out is sent
    xor  cx,cx
    
@@: in   al,64h
    test al,00000010b
    jz   short @f
    loop @b
    
    ; timed out
    stc
    ret
    
@@: clc
    ret
wait_kbd_status2 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; waits while keyboard status bit 0 = 0
; returns carry if timed out
wait_kbd_status1 proc near uses ax cx
    
    ; we will try 65536 times before a time out is sent
    xor  cx,cx
    
@@: in   al,64h
    test al,00000001b
    jnz  short @f
    loop @b
    
    ; timed out
    stc
    ret
    
@@: clc
    ret
wait_kbd_status1 endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; tests whether the a20 line is set.
;  we do this by writing a value to 0x00100900, 
;  and then check the value at 0x00000900 to see if 
;  they are the same.  i.e.: 0x00100900 will wrap
;  around to 0x00000900 if the a20 line is not active.
; we use 0x00100900 since the point of wrap around,
;  0x00000900, does not contain anything valid already
;  and can be overwritten.
; on entry:
;  nothing
; on exit:
;  zero flag clear if active
test_a20   proc near uses eax cx si di ds es

           mov  ax,0FF00h  ; FF00:1900h = 0x00100900
           mov  ds,ax
           mov  si,1900h
            
           xor  ax,ax      ; 0000:0900h = 0x00000900
           mov  es,ax
           mov  di,0900h
            
           ; read and store the original dword at ds:[si]
           mov  eax,[si]
           push eax        ; save the current value
            
           ; loop 32 times to make sure
           mov  cx,32
@@:        add  dword [si],01010101h
           mov  eax,[si]
           call io_delay
           cmp  eax,es:[di]
           loope @b
                   
           ; if the values are still the same, the two pointers
           ;  point to the same location in memory.
           ; i.e.: the a20 line is not set
            
           pop  eax        ; restore the original value
           mov  [si],eax
           ret
test_a20   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; a slight delay on older machines
io_delay   proc near uses ax
           xor  al,al
           out  80h,al
           out  80h,al
           ret
io_delay   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the print routines and other items
include '..\include\stdio.asm'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strings and other data
hdr_copy_str     db  13,10,'A20 Test v1.00.00'
                 db  13,10,'Forever Young Software -- (c) Copyright 1984-2015',13,10,0

inactive_str     db  13,10,'Not Active',0
active_str       db  13,10,'Is Active',0

deactive_err_str db  13,10,'Error Deactivating A20 line.',0

method_err_str   db  'Unsuccessful.',0
method_str       db  'Successful.',0

method_one_str   db  13,10,'Trying Method 1: ',0
method_two_str   db  13,10,'Trying Method 2: ',0
method_three_str db  13,10,'Trying Method 3: ',0
method_four_str  db  13,10,'Trying Method 4: ',0
method_five_str  db  13,10,'Trying Method 5: ',0
method_six_str   db  13,10,'Trying Method 6: ',0

list             dd  offset method_one_str
                 dd  offset method_one
                 dd  offset method_two_str
                 dd  offset method_two
                 dd  offset method_three_str
                 dd  offset method_three
                 dd  offset method_four_str
                 dd  offset method_four
                 dd  offset method_five_str
                 dd  offset method_five
                 dd  offset method_six_str
                 dd  offset method_six
                 
                 dd  0
.end
