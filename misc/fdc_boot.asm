comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fdc_boot.asm                                                       *
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
*   To see if I can read remaining sectors from a booted floppy disk       *
*    without using the BIOS.  I.e.: Directly program the FDC to read       *
*    from the disk.                                                        *
*   This must be all coded in a single sector since we cannot load any     *
*    addition sectors using the BIOS.  All read code, FDC, DMA (if used),  *
*    and other code must fit in the first sector already loaded for us     *
*    by the BIOS.                                                          *
*   This is simply an experiment to see if it can be done somewhat         *
*    reliably.  i.e.: very few assumptions, and has error checking.        *
*   I just wanted to do it for fun.                                        *
*   It only reads in the next sector and jumps to it.  However, it should  *
*    be quite simple to add additional sectors, different starting sector, *
*    etc.                                                                  *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.76                                         *
*          Command line: nbasm fdc_boot<enter>                             *
*                                                                          *
* Last Updated: 7 Dec 2019                                                 *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*  Assuptions:                                                             *
*   - the disk is a 1.44meg, 3 1/2" standard formatted floppy disk.        *
*   - this will be the first drive of the FDC.  i.e.: Drv = DRIVE          *
*     (we could get the DL value from the POST and use that to select      *
*      the drive.  i.e.:  DL = 00h = first drive, DL = 01h = second drive) *
*   - this will be at either 0x3F0 or 0x370. i.e.: controler = CNTRLR      *
*   - the disk is already inserted.  It pretty much has to be since the    *
*      BIOS just read this sector.                                         *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 186 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
* This code uses some size optimizations here and there to get it to fit   *
*   with in the 512 byte sector(s).  Be careful when assuming the          *
*   optimizations work if you modify the code.  i.e.: previous register    *
*   values, etc.                                                           *
*                                                                          *
***************************************************************************|

.model tiny

include 'fdc_boot.inc'

outfile 'fdc_boot.img'             ; target filename

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code
.code
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector

           org  00h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
           cli
           mov  ax,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,ax
           mov  es,ax
           mov  ss,ax
           mov  sp,STACK_OFFSET    ; first push at 07C0:STACK_OFFSET - 2

           ; set up our interrupt handler
           call Init_ISR
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; reset the controller
           ; the minimum duration of the reset should be 500ns.
           ;  A write duration to an ISA I/O port takes longer than that.
           ;  Therefore, if we write it twice, we will be sure to be long enough.
           mov  dx,FDC_DOR
           xor  al,al
           out  dx,al             ; reset
           out  dx,al             ; twice
           
           ; clear the reset
           mov  al,4              ; normal operation
           out  dx,al
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; wait for the controller to be ready for input (20ms typical)
           mov  ch,(FDC_INT_WAIT>>8)  ; we will try many times
           mov  dx,FDC_MSR
@@:        in   al,dx
           cmp  al,80h
           loopne @b
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the controller isn't ready for input after the delay, halt
           mov  ax,ERROR_NOT_READY    ; if error, return error code
           jcxz short do_error

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; we have successfully reset the controller
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; wait for the interrupt, then sense an interrupt on all four drives
           ; Page 41 of the FDC 82077AA specs
           call fdc_wait_int
           mov  ax,ERROR_NO_INTERRUPT  ; if error, return error code
           jc   short do_error

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; received the interrupt so send the FDC_SENSE_INT command to all four drives
           xor  cx,cx
           mov  si,offset FDC_CMD_SENSE_INT_str

@@:        call fdc_command
           
           mov  ax,2
           call fdc_return
           
           cmp  al,2             ; if we didn't return two bytes, error
           mov  ax,ERROR_NOT_TWO ; if error, return error code
           jne  short do_error
           
           mov  al,[di+0]        ; if drive numbers don't match, error
           and  al,3
           cmp  al,cl
           mov  ax,ERROR_NO_MATCH ; if error, return error code
           jne  short do_error

           mov  al,[di+1]        ; if drive numbers don't match, error
           or   al,al
           mov  ax,ERROR_NO_MATCH ; if error, return error code
           jz   short @f

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we get here, there was an error
; print the string and halt
; (we place this in the middle of the code so that it
;  is a short jump either way)
do_error:  mov  si,offset error_string
           mov  [si+7],ax        ; ax = ascii error code
           call display_string
           cli
           hlt
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
@@:        ; loop through all four
           inc  cx
           cmp  cx,4
           jb   short @b

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; now try to read the sector
; we will need to turn on the motor, setup the DMA, etc.

           ; turn motor on the motor
           mov  dx,FDC_DOR
           mov  al,((0x10 << DRIVE) | 0x0C | DRIVE)
           out  dx,al
           
           ; TODO: must wait 2 seconds for the disk to actually spin up

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; setup the DMA for transfering data
           mov  bx,0x7E00              ; address
           mov  cx,0x200               ; count of bytes to read
           call dma_init_dma           ; call the DMA initialization
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; send the specify command
@@:        mov  si,offset FDC_CMD_SPECIFY_str
           call fdc_command
           mov  ax,ERROR_SPECIFY  ; if error, return error code
           jc   short do_error
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; seek to cylinder
           ; (really, the head should still be at cylinder 0 since it just
           ;  read the first sector.  However, let's make sure, and with
           ;  this code, we can change to any sector we wish to read)
           mov  si,offset FDC_CMD_SEEK_str
           call fdc_command_int
           mov  ax,ERROR_SEEK     ; if error, return error code
           jc   short do_error

           ;  TODO: wait for the head to settle
           ; a 'mdelay(FDC_SLT_AFTER_SEEK)' function call or something like this
           
           ; make sure motor is on (again)
           mov  dx,FDC_DOR
           mov  al,((0x10 << DRIVE) | 0x0C | DRIVE)
           out  dx,al

           mov  si,offset FDC_CMD_READ_str
           call fdc_command
           mov  ax,ERROR_READ     ; if error, return error code
           jc   short do_error

           ; wait for interrupt
           call fdc_wait_int
           mov  ax,ERROR_NO_INTERRUPT ; if error, return error code
           jc   short do_error
           
           mov  ax,7
           call fdc_return

           cmp  al,7             ; if we didn't return seven bytes, error
           mov  ax,ERROR_NOT_SEVEN ; if error, return error code
           jne  short do_error
           
           mov  al,[di+0]        ; if not 00xxxxxxb, error
           and  al,0xC0
           mov  ax,ERROR_NOT_READY ; if error, return error code
           jnz  short do_error

           ; else, we did a good read, so jump to that sector and see what happens
           jmp  remaining

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; our ISR
FDC_ISR    proc near

           push ax
           
           ;push ds
           ;mov  ax,0x07C0
           ;mov  ds,ax
           ;mov  byte fdc_drv_stats,1
           ;pop  ds
           
           ; this line assumes we set the interrupt vector table entry to 0x07C0:FDC_ISR,
           ;  as opposed to 0x0000:(0x7C00 + FDC_ISR).
           ; the above lines do not assume, though they take more space.
           mov  byte cs:fdc_drv_stats,1

           ; end of interrupt on controller
.if (IRQ_NUM >= 8)
           mov  al,((0<<3) | (1<<5))
           out  0xA0,al
.endif
           mov  al,((0<<3) | (1<<5))
           out  0x20,al
           
           pop  ax

           iret

FDC_ISR    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; initialize our ISR
; Replace BIOS ISR with ours
Init_ISR   proc near uses ax dx es
           
           cli
           
           xor  ax,ax
           mov  es,ax

           mov  ax,offset FDC_ISR
           mov  es:[((IRQ_NUM * 4) + 0)],ax
           mov  ax,0x07C0
           mov  es:[((IRQ_NUM * 4) + 2)],ax

           ; need to unmask the PIC
           ; this assumes that the IRQ's are already mapped, which they 
           ;  should since the BIOS just left it to us.
           ; (in theroy, the IRQ should already be unmasked since the BIOS
           ;  used it to read this first sector in.)
.if (IRQ_NUM < 8)
           mov  dx,0x21
           in   al,dx
           or   al,(1<<IRQ_NUM)
           out  dx,al
.else
           mov  dx,0xA1
           in   al,dx
           or   al,(1<<(IRQ_NUM-8))
           out  dx,al
.endif
           ; make sure interrupts are active before we leave
           sti

           ret
Init_ISR   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; wait for completion interrupt
;  returns carry set if takes more than given time
; not preserved:  ax
fdc_wait_int proc near uses dx
           
           ; delay count
           mov  ch,(FDC_INT_WAIT>>8)

wait_int_loop:
           ; has the ISR set the flag?
           cmp  byte fdc_drv_stats,0
           je   short @f

           ; yes, so clear it and return
           mov  byte fdc_drv_stats,0
           clc
           ret
           
@@:        ; a read on the ISA takes about 250ns
           ; reading it 100 times will take about 25000ns, or 25ms
           ; this is a poor man's delay, but it works for our sake
           mov  dx,FDC_MSR
           push cx
           mov  cx,100
@@:        in   al,dx
           loop @b
           pop  cx

           loop wait_int_loop
           
           stc
           ret
fdc_wait_int endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Send a command to the controller
; on entry:
;  ds:si -> command to send (first byte is length of command)
; on return:
;  carry clear if successful
; not preserved:  ax, si
fdc_command proc near uses bx cx dx
           ; clear the interrupt flag
           mov  byte fdc_drv_stats,0
           
           mov  cx,FDC_TRY_COUNT   ; try count of times before we error
           
           ; put the length of the command in bl
           ; and point to the first byte of the command
           lodsb
           mov  bl,al
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; main loop
           ;  try to send a count of 'BL' byte(s) to the controller
           ;  if the controller says to read instead, we read and discard it
command_loop:
           mov  dx,FDC_MSR
           
           ; is it busy
           in   al,dx
           test al,80h
           jz   short command_loop_it

           ; direction?
           in   al,dx
           test al,40h
           jz   short command_loop_out
           
           ; read in the byte and try again
           mov  dx,FDC_CSR
           in   al,dx
           jmp  short command_loop_it

command_loop_out:
           lodsb    ; next command byte
           mov  dx,FDC_CSR
           out  dx,al
           
           ; was that the last one
           dec  bl
           jz   short command_done
           
command_loop_it:
           loop command_loop
           
           ; if cx == 0, we tried 128 times, so return FALSE
           stc
           ret
           
command_done:
           clc
           ret
fdc_command endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Send a command to the controller
; on entry:
;  ds:si -> command to send (first byte is length of command)
; on return:
;  carry clear if successful
;  ax = count of bytes returned
;  di -> ret_buffer filled
; not preserved:  si
fdc_command_int proc near
           
           call fdc_command
           jc   short @f
           
           ; wait for interrupt
           call fdc_wait_int
           jc   short @f
           
           mov  si,offset FDC_CMD_SENSE_INT_str
           call fdc_command
           jc   short @f
           
           mov  al,2
           call fdc_return

@@:        ret
fdc_command_int endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Read a return buffer from the controller
; on entry:
;  es:di -> buffer to hold data
;  al = requested count to return
; on return:
;  carry clear if successful
;   di -> buffer of bytes read
;   al = count of bytes read
;  carry set if error
fdc_return  proc near uses bx cx dx
           
           mov  cx,FDC_TRY_COUNT   ; try count of times before we error
           mov  di,offset ret_buffer
           
           mov  ah,al  ; save requested count in ah
           xor  bx,bx  ; count

return_loop:
           mov  dx,FDC_MSR
           in   al,dx
           and  al,0C0h
           cmp  al,0C0h
           jne  short return_loop_next

           ; read in the byte
           mov  dx,FDC_CSR
           in   al,dx
           mov  [bx+di],al
           
           ; increment the count and
           ;  don't do more than requested
           inc  bx
           cmp  bl,ah
           jae  short return_done  ; (i.e.: carry = clear)
           ; (optimization: The carry is clear via the
           ;  test above, so no need to clear it again
           ;  when jumping to the return)

return_loop_next:
           ; delay 500ns and try again
           push cx
           mov  ch,(FDC_INT_WAIT>>8)
           mov  dx,FDC_MSR
@@:        in   al,dx
           loop @b
           pop  cx

           loop return_loop

           stc

return_done:
           mov  al,bl
           ret
fdc_return  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
; On entry:
;   ds:si -> asciiz string to display
;
; (nothing is preserved)
display_string proc near
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; bug in some BIOSes if bx != 0
;           cld
@@:        lodsb
           or   al,al
           jz   short @f
           int  10h                ; output the character
           jmp  short @b
@@:        ret
display_string endp

; setup the dma for a specified channel
; on entry
;   bx = physical address (0x7E00)
;   cx = size in bytes (0x200)
; (nothing is preserved)
dma_init_dma proc near
           
           ; mask this channel for no interruptions
           mov  dx,DMA_MASK_REG
           mov  al,((1<<2) | 2)		 ; DMA-1 Mask Register Bit Port (8-bit)
           out  dx,al

           mov  dx,DMA_MODE_REG    ; Set DMA-1 Mode Register port
           mov  al,(DMA_CMD_WRITE | 2) ; write to memory
           out  dx,al
           
           mov  dx,DMA_FLIP_FLOP
           out  dx,al              ; DMA-1 Clear Byte Flip-Flop (write anything)

	         ; size is always 1 less from sent size
	         dec  cx
           
           mov  dx,0x81
           xor  al,al              ; DMA-1 Channel 2 Output (page value)
           out  dx,al
           
           mov  dx,0x04
           mov  al,bl              ; DMA-1 Channel 2 Output (low byte)
           out  dx,al
           mov  al,bh              ; DMA-1 Channel 2 Output (high byte)
           out  dx,al
           
           inc  dx                 ; dx = 5
           mov  al,cl              ; DMA-1 Channel 2 Output size (low byte)
           out  dx,al
           mov  al,ch              ; DMA-1 Channel 2 Output size (high byte)
           out  dx,al
           
           ; select channel
           mov  dx,DMA_MASK_REG
           mov  al,2
           out  dx,al              ; DMA-1 Mask Register Bit Port (8-bit)
           
           ret
dma_init_dma endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strings to display
error_string           db  'Error: xxh',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; commands
FDC_CMD_SPECIFY_str    db  3      ; length of this command (in bytes)
                       db  FDC_CMD_SPECIFY
                       db  0xAF
                       db  0x1E

FDC_CMD_SENSE_INT_str  db  1      ; length of this command (in bytes)
                       db  FDC_CMD_SENSE_INT

FDC_CMD_SEEK_str       db  3      ; length of this command (in bytes)
                       db  FDC_CMD_SEEK
                       db  ((0 << 2) | DRIVE)  ; (head << 2) | drive
                       db  0                   ; cyl << stepping

FDC_CMD_READ_str       db  9      ; length of this command (in bytes)
                       db  (FDC_CMD_READ | FDC_MFM | FDC_SKIP)
                       db  ((0 << 2) | DRIVE)    ; (head << 2) | drive
                       db  0                     ; cyl
                       db  0                     ; head map
                       db  2                     ; sector
                       db  2                     ; sector len
                       db  18                    ; sectors per track
                       db  27                    ; gap length
                       db  255                   ; data length

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Pad out to fill 512 bytes, including final word 0xAA55

%PRINT (200h-$-2)               ; 8 bytes free in this area
           
           org (200h-2)
           dw  0AA55h

;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is the second sector loaded by the code above
remaining:

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we simply print a string and halt

           mov  si,offset we_made_it_str
           call display_string
           cli
           hlt


we_made_it_str  db  'We made it!!!!!',0



;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; bss
           org 400h

ret_buffer           dup  16,?   ; return buffer
fdc_drv_stats        dup  1,?    ; interrupt flag



;=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; end of 1.44Meg Image file
.pmode  ; must indicate pmode now to get past 64k limit
           org (2880 * 512)

.end
