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

comment |*******************************************************************
*                            FYS OS version 2.0                            *
* FILE: a20.inc                                                            *
*                                                                          *
* Loader code for the FYS OS version 2.0 operating system.                 *
*                                                                          *
*  Built with:  NBASM ver 00.26.52                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update:  23 June 2013                                               *
*                                                                          *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* This is the A20 activate code for the Loader.sys file.                   *
*                                                                          *
* It first checks to see if the A20 line is already activated.  If so, it  *
*  simply returns.                                                         *
* If not, it trys numerous ways to activate it.                            *
*                                                                          *
* The UHCI controller wants a NULL (0xFF) command sent to the keyboard     *
*  after the keyboard a20 sequence.                                        *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; tests and activates the a20 line.
;  returns in al, the number of the technique used.
;  if can not set the a20 line, this functions prints an error and freezes.
set_a20_line proc near uses dx si
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure no interrupts bother us
           cli

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; first check to see if it is already active
           mov  al,0       ; return 0 = already set
           call test_a20
           jnz  set_a20_done

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; it was not already set, so try
           ; Method 1: keyboard controller
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_2  ; returns carry if timed out
           mov  al,0D0h
           out  64h,al
           call wait_kbd_status1  ; waits while keyboard status bit 0 = 0
           jc   short a20_try_method_2  ; returns carry if timed out
           in   al,60h
           mov  ah,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_2  ; returns carry if timed out
           mov  al,0D1h
           out  64h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_2  ; returns carry if timed out
           mov  al,ah
           or   al,02
           out  60h,al
           
           ; Wait for the A20 line to settle down (up to 20usecs)
           mov  al,0FFh     ; Send FFh (Pulse Output Port NULL)
           out  64h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_2  ; returns carry if timed out
           
           ; now test the a20 line
           mov  al,1       ; return 1 = keyboard method
           call test_a20
           jnz  short set_a20_done


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Method 2: fast a20 (port 0x92)
a20_try_method_2:
           mov  dx,92h
           in   al,dx
           and  al,0FEh    ; make sure we don't do a reset
           or   al,02h
           out  dx,al

           ; now test the a20 line
           mov  al,2       ; return 2 = fast port 92h
           call test_a20
           jnz  short set_a20_done


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Method 3: Keyboard controller: Command DFh
a20_try_method_3:
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_4  ; returns carry if timed out
           mov  al,0DFh                 ; Send command DFh
           out  60h,al
           
           ; read in the acknowledge
           call wait_kbd_status1  ; waits while keyboard status bit 0 = 0
           jc   short a20_try_method_4  ; returns carry if timed out
           in   al,60h
           cmp  al,0FAh  ; if not ACK, then error
           jne  short a20_try_method_4

           ; Wait for the A20 line to settle down (up to 20usecs)
           ; Some UHCI controllers when using legacy mode, need the FF (null command)
           ;  sent after the above DF command.
           mov  al,0FFh     ; Send FFh (Pulse Output Port NULL)
           out  64h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_4  ; returns carry if timed out
           
           ; now test the a20 line
           mov  al,3       ; return 3 = keyboard method: command DFh
           call test_a20
           jnz  short set_a20_done


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Method 4: Brute force of Method 1
a20_try_method_4:
           mov  al,0D1h
           out  64h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_5  ; returns carry if timed out
           mov  al,0DFh
           out  60h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_5  ; returns carry if timed out
           mov  al,0FFh
           out  64h,al
           call wait_kbd_status2  ; waits while keyboard status bit 1 = 1
           jc   short a20_try_method_5  ; returns carry if timed out
           
           ; now test the a20 line
           mov  al,4       ; return 4 = keyboard method: brute force of #1
           call test_a20
           jnz  short set_a20_done


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Method 5: BIOS a20 enable service (later PS/2's)
a20_try_method_5:
           mov  ax,2401h
           int  15h
           jc   short a20_try_method_6  ; returns carry if not supported
           cmp  ah,00
           jne  short a20_try_method_6  ; returns 00h if not supported

           ; now test the a20 line
           mov  al,5       ; return 5 = keyboard method: BIOS a20 service 2401h
           call test_a20
           jnz  short set_a20_done

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Method 6: 
a20_try_method_6:
           
           ; this is where we would try another method
           
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we make it here, then we didn't set the a20 line,
           ;  say so and freeze.
           mov  si,offset no_a20_line
           call display_string
a20_hang:  jmp  short a20_hang
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; allow interrupts and
           ; return with al = method used
set_a20_done:
           sti
           ret
set_a20_line endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; tests whether the a20 line is set.
;  we do this by writing a value to 0x00100900, and then check the value
;  at 0x00000900 to see if they are the same.  i.e.: 0x00100900 will wrap
;  around to 0x00000900 if the a20 line is not active.
; we use 0x00100900 since the point of wrap around, 0x00000900, does not
;  contain anything valid aready and can be overwritten.
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

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; waits while keyboard status bit 1 = 1
; returns carry if timedout
; (used in a20 above)
wait_kbd_status2 proc near uses ax cx

           ; we will try 65536 times before a time out is sent
           xor  cx,cx

@@:        in   al,64h
           test al,00000010b
           jz   short @f
           loop @b

           ; timed out
           stc
           ret

@@:        clc
           ret
wait_kbd_status2 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; waits while keyboard status bit 0 = 0
; returns carry if timedout
; (used in a20 above)
wait_kbd_status1 proc near uses ax cx

           ; we will try 65536 times before a time out is sent
           xor  cx,cx

@@:        in   al,64h
           test al,00000001b
           jnz  short @f
           loop @b

           ; timed out
           stc
           ret

@@:        clc
           ret
wait_kbd_status1 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; a slight delay on older machines
io_delay   proc near uses ax
           xor  al,al
           out  80h,al
           out  80h,al
           ret
io_delay   endp


no_a20_line   db  13,10,'Unable to set the a20 line.  Halting...'
              db  13,10,'Please report this to me at fys@fysnet.net'
              db  13,10,'Also include as much information about your computer'
              db  13,10,' along with brand and version of BIOS.'
              db  13,10
              db  13,10,'Thank you.',0

.end
