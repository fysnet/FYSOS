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
 
 ;  Last updated: 19 July 2020

EFLAGS_CARRY                equ  00000001h
EFLAGS_PARITY               equ  00000004h
EFLAGS_AUXILIARY            equ  00000010h
EFLAGS_ZERO                 equ  00000040h
EFLAGS_SIGN                 equ  00000080h
EFLAGS_TRAP                 equ  00000100h
EFLAGS_INT_ENABLE           equ  00000200h
EFLAGS_DIRECTION            equ  00000400h
EFLAGS_OVERFLOW             equ  00000800h
EFLAGS_IOPL                 equ  00003000h
EFLAGS_NESTED               equ  00004000h
EFLAGS_RESUME               equ  00010000h
EFLAGS_V86                  equ  00020000h
EFLAGS_ALIGNMENT            equ  00040000h
EFLAGS_VIRTUAL_INT          equ  00080000h
EFLAGS_VIRTUAL_INT_PENDING  equ  00100000h
EFLAGS_ID                   equ  00200000h

S_REGS     struct
reg_eax    dword  ; 0x00
reg_ebx    dword  ; 0x04
reg_ecx    dword  ; 0x08
reg_edx    dword  ; 0x0C
reg_esi    dword  ; 0x10
reg_edi    dword  ; 0x14
reg_ebp    dword  ; 0x18
reg_eip    dword  ; 0x1C
reg_esp    dword  ; 0x20
reg_cs      word  ; 0x24
reg_ds      word  ; 0x26
reg_es      word  ; 0x28
reg_fs      word  ; 0x2A
reg_gs      word  ; 0x2C
reg_ss      word  ; 0x2E
reg_eflags dword  ; 0x30
reg_cr3    dword  ; 0x34
S_REGS     ends

S_TASK     struct
flags      dword    ;
id         dword    ;
stack      dword    ; pointer to the stack allocated at make time. (so kill_thread can unallocated it)
mem_start  dword    ; pointer to address space of task (if user task)
timer      qword    ; value of (64-bit) timer tick when this task should be executed again.
reg_eax    dword  ; 0x00  ; unfortunetly, NBASM doesn't support nested structures at the moment, so
reg_ebx    dword  ; 0x04  ; we have to include it again here.
reg_ecx    dword  ; 0x08
reg_edx    dword  ; 0x0C
reg_esi    dword  ; 0x10
reg_edi    dword  ; 0x14
reg_ebp    dword  ; 0x18
reg_eip    dword  ; 0x1C
reg_esp    dword  ; 0x20
reg_cs      word  ; 0x24
reg_ds      word  ; 0x26
reg_es      word  ; 0x28
reg_fs      word  ; 0x2A
reg_gs      word  ; 0x2C
reg_ss      word  ; 0x2E
reg_eflags dword  ; 0x30
reg_cr3    dword  ; 0x34
S_TASK     ends

S_GDT      struct
limitlow   word   ; low word of limit
baselow    word   ; low word of base
basemid    byte   ; mid byte of base
flags0     byte   ;
flags1     byte   ;
basehigh   byte   ; high byte of base
S_GDT      ends

TASK_FLAG_USED    equ  00000001h
TASK_FLAG_ASLEEP  equ  00000002h
TASK_TYPE_USER    equ  00008000h
TASK_TYPE_KERNEL  equ  00010000h
MAX_TASKS         equ  1024

task_lock   dd  0
cur_task    dd  MAX_TASKS

exception_regs st S_REGS
user_regs      st S_REGS

; 16 IRQ handler pointers
irq_handlers  dup  (16 * sizeof(dword)),0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; set the starting IRQ handlers to all point
;  to illega_irq
irq_initialize proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           
           mov  eax,offset idt_handlerXX
           mov  ecx,16
           mov  edi,offset irq_handlers
           cld
           rep
             stosd
           
           pop  edi
           
           pop  ebp
           ret
irq_initialize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; remap_pic(master, slave)
; remaps the PIC
;  remap PICs from 08h to 20h and 70h 28h
; On entry:  (master, slave)
; On return: nothing
remap_pic  proc near
           
           push ebp
           mov  ebp,esp
           
           ; send ICW1
           push 11h              ; (initialization + ICW4)
           push PIC_MASTER
           call outpb
           call iodelay
           
           push 11h              ; (initialization + ICW4)
           push PIC_SLAVE
           call outpb
           call iodelay
           
           ; send ICW2
           push 20h              ; set PIC_MASTER to be at 20h
           push (PIC_MASTER+1)
           call outpb
           call iodelay
           
           push 28h              ; set PIC_SLAVE to be at 28h
           push (PIC_SLAVE+1)
           call outpb
           call iodelay
           
           ; send ICW3
           push 04h              ; IRQ2 is connection to slave
           push (PIC_MASTER+1)
           call outpb
           call iodelay
           
           push 02h              ; IRQ9 is connect to master
           push (PIC_SLAVE+1)
           call outpb
           call iodelay
           
           ; send ICW4
           push 01h              ; 80x86/88 mode
           push (PIC_MASTER+1)
           call outpb
           call iodelay
           
           push 01h              ; 80x86/88 mode
           push (PIC_SLAVE+1)
           call outpb
           call iodelay
           
           ; send mask to master
           push dword PARAM0     ; default mask for master
           push (PIC_MASTER+1)
           call outpb
           call iodelay
           
           ; send mask to slave
           push dword PARAM1     ; default mask for slave
           push (PIC_SLAVE+1)
           call outpb
           call iodelay
           
           pop  ebp
           ret  8
remap_pic  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; irq_set()
; assign a handler to an irq below
; On entry:  (irq_num, address)
; On return: TRUE if successful
irq_set    proc near
           
           push ebp
           mov  ebp,esp
           
           ; the 'ja' instruction is unsiged.
           ; therefore, it will jump if eax
           ;  is < 0 or > 15.
           mov  eax,PARAM0
           cmp  eax,15
           ja   short irq_set_err
           
           ; this routine does not error checking.
           ; it would be a good thing to see if this
           ; irq is already assigned and return FALSE
           ; if so.
           mov  ecx,PARAM1
           lea  eax,[irq_handlers+eax*4]
           mov  [eax],ecx
           
           mov  eax,TRUE
           jmp  short irq_set_done
           
irq_set_err:
           xor  eax,eax
           
irq_set_done:
           pop  ebp
           ret  8
irq_set    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_init()
; Initialize the task table
; On entry:  nothing
; On return:    error: NULL
;            no error: address to task
task_init  proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           
           ; make sure the tasks buffer is empty
           mov  edi,TASKS_BUFF
           xor  al,al
           mov  ecx,((MAX_TASKS+1) * sizeof(S_TASK))
           rep
             stosb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make the idle task (so that we have at least one task.
           ; we need to allocate some memory for its stack.
           push 1024  ; 1024 bytes
           call malloc      ; returns address in eax
           or   eax,eax     ; if returned NULL
           jz   short task_done ; return NULL if error
           
           ; make_thread(task_address, stack_address, stack_size, 0, 0, flags, FALSE)
           push FALSE
           push (TASK_FLAG_USED | TASK_TYPE_KERNEL)
           push 0
           push 0
           push 1024   ; stack size
           push eax
           push offset task_idle
           call make_thread
           ; returns NULL if it did not make it, else task ID
           
task_done: pop  edi
           
           pop  ebp
           ret
task_init  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; make_thread()
; Create a thread in our round robin task table
; On entry:  (task_address, stack_address, stack_size, frame_offset, mem_start, flags, mod_int_flag)
; On return:    error: NULL
;            no error: task ID
make_thread proc near
           
           push ebp
           mov  ebp,esp
           sub  esp,(1 * sizeof(dword)) ; local stack space
           
           ; save preserved registers
           push esi
           
           ; get the current IF bit
           pushfd
           pop  eax
           mov  LOCAL0,eax         ; save the flags register
           
           ; Don't allow the scheduler to switch tasks while we are doing this
           ; if the mod_int_flag is set, turn off interrupts
           cmp  dword PARAM6,0
           je   short @f
           cli
           
           ; let's see if we can find an open slot
           ; (i=0; i<MAX_TASKS; i++)
@@:        xor  ecx,ecx
make_thread_loop:
           cmp  ecx,MAX_TASKS
           jnb  make_thread_none
           
           ; if not tasks[ecx].flags & TASK_FLAG_USED, then we found one
           mov  esi,TASKS_BUFF
           mov  eax,sizeof(S_TASK)
           mul  ecx
           add  esi,eax
           test dword [esi+S_TASK->flags],TASK_FLAG_USED
           jnz  make_thread_cont
           
           mov  eax,PARAM5   ; flags
           mov  [esi+S_TASK->flags],eax
           call task_unique_id
           mov  [esi+S_TASK->id],eax
           mov  dword [esi+S_TASK->timer],0
           mov  eax,PARAM1   ; stack address
           mov  [esi+S_TASK->stack],eax
           mov  eax,PARAM4   ; mem start
           mov  [esi+S_TASK->mem_start],eax
           
           mov  word [esi+S_TASK->reg_cs],08h
           mov  eax,PARAM0   ; task address
           mov  [esi+S_TASK->reg_eip],eax
           mov  word [esi+S_TASK->reg_ds],10h
           mov  word [esi+S_TASK->reg_es],10h
           mov  word [esi+S_TASK->reg_fs],10h
           mov  word [esi+S_TASK->reg_gs],10h
           mov  word [esi+S_TASK->reg_ss],10h
           xor  eax,eax
           mov  [esi+S_TASK->reg_eax],eax
           mov  [esi+S_TASK->reg_ebx],eax
           mov  [esi+S_TASK->reg_ecx],eax
           mov  [esi+S_TASK->reg_edx],eax
           mov  [esi+S_TASK->reg_esi],eax
           mov  [esi+S_TASK->reg_edi],eax
           mov  [esi+S_TASK->reg_eax],eax
           mov  eax,PARAM1    ; stack address
           add  eax,PARAM2    ; stack size
           mov  [esi+S_TASK->reg_esp],eax
           add  eax,PARAM3    ; frame offset
           mov  [esi+S_TASK->reg_ebp],eax
           mov  dword [esi+S_TASK->reg_eflags],(EFLAGS_INT_ENABLE | 2)
           
           ; now return the ID number in eax
           mov  eax,[esi+S_TASK->id]
           jmp  short make_thread_ret
           
make_thread_cont:
           inc  ecx
           jmp  make_thread_loop
           
           ; didn't find an empty slot           
make_thread_none:
           
           ;  DEBUG("\n Did not find an empty slot for new task")
           ; return zero for task not created
           xor  eax,eax           
           
make_thread_ret:           
           ; if we cleared the IF bit, and it was set to begin with,
           ;  set it back
           cmp  dword PARAM6,0
           je   short @f
           test dword LOCAL0,EFLAGS_INT_ENABLE
           jz   short @f
           sti  
           
           ; restore preserved registers
@@:        pop  esi
           
           mov  esp,ebp
           pop  ebp
           ret  (7 * sizeof(dword))
make_thread endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_unique_id()
; Return an unique number for task ID
; On entry:  nothing
; On return: task ID
task_unique_id proc near
           
           push ebp
           mov  ebp,esp
           
           inc  dword unique_id
           mov  eax,unique_id
           
           pop  ebp           
           ret
task_unique_id endp

unique_id   dd 0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_lock_set()
; Lock the task scheduler from switching tasks
; On entry:  (lock_it)
; On return: nothing
task_lock_set proc near
           
           push ebp
           mov  ebp,esp
           
           ; get the current IF bit
           pushfd
           pop  eax                  ; save the flags register in eax
           
           cli
           
           ; if lock_it == TRUE, increment the counter
           cmp  dword PARAM0,0
           je   short @f
           inc  dword task_lock
           jmp  short task_lock_done
           
           ; else, decrement the counter (only if it is greater than zero)
@@:        cmp  dword task_lock,0
           je   short task_lock_done
           dec  dword task_lock

task_lock_done:
           test eax,EFLAGS_INT_ENABLE
           jz   short @f
           sti
           
@@:        pop  ebp
           ret  4
task_lock_set endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_idle()
; The Idle task
; On entry:  nothing
; On return: TRUE/FALSE
task_idle  proc near
           
@@:        call task_giveup_timeslice
           jmp  short @b
           
           .noret           
task_idle  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_giveup_timeslice()
; gives up the current task's timeslice
; on entry:  nothing
; on return: nothing
task_giveup_timeslice proc near
           pop  eax         ; get the return address
           pushfd           ; push the flags register on the stack
           cli              ; allow interrupts
           mov  byte not_an_interrupt,1
           push 08h
           push eax
           jmp  idt_handler20
           .noret
task_giveup_timeslice endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_sys_delay()
; pauses for a specific time, giving up the time slice
; on entry:  (delay)
; on return: nothing
task_sys_delay proc near

           push ebp
           mov  ebp,esp
           
           push esi
           push ebx
           
           mov  esi,TASKS_BUFF
           mov  eax,cur_task
           mov  ecx,sizeof(S_TASK)
           mul  eax
           add  esi,eax
           
           ; (((delay + 9) / 10) + 10)  ; + 10 to be more than--instead of less than
           xor  edx,edx
           mov  eax,PARAM0
           add  eax,9
           mov  ebx,10
           div  ebx
           add  eax,10
           mov  ebx,eax
           
           ; get the current timer tick and add ebx to it
           call get_timer_tick64
           add  eax,ebx
           adc  edx,0
           mov  [esi+S_TASK->timer + 0],eax
           mov  [esi+S_TASK->timer + 4],edx
           
           call task_giveup_timeslice
           
           pop  ebx
           pop  esi
           
           pop  ebp
           ret  4
task_sys_delay endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; task_scheduler()
; The Task scheduler
;   (interrupts are off)
; On entry:  (pRegs)
; On return: nothing
task_scheduler proc near

           push ebp
           mov  ebp,esp
           
           push esi
           push ebx
           mov  ebx,cur_task
           
           ; if the task_lock flag is not zero,
           ;  don't task switch, return to same task
           cmp  dword task_lock,0
           jnz  task_sch_done
           
           ; On the first time we call the task scheduler, we 
           ;  will still be in single tasking mode.
           ; The saving of the registers will not matter, 
           ;  but we do it anyway
           
           ; save the current tasks registers
           ; memcpy(&tasks[cur_task].regs, regs, sizeof(REGS))
           push sizeof(S_REGS)
           push dword PARAM0
           mov  eax,sizeof(S_TASK)
           mul  ebx                        ; current task index
           add  eax,TASKS_BUFF
           lea  eax,[eax + S_TASK->reg_eax]
           push eax
           call memcpy
           
           ; Find a task that may need to be executed before a regular cycle
           ;  would find it.  If we didn't find one, then execute next task.
           cmp  ebx,MAX_TASKS  ; the first time this scheduler gets called, cur_task == MAX_TASKS
           jnb  short task_sch_notfrst
           
           mov  ecx,ebx      ; ecx = temp current task index
task_sch_loop:
           inc  ecx
           ; roll it over to zero if at max
           cmp  ecx,MAX_TASKS
           jb   short @f
           xor  ecx,ecx
           
@@:        mov  eax,sizeof(S_TASK)
           mul  ecx
           mov  esi,eax   
           add  esi,TASKS_BUFF  ; esi = tasks[ecx]
           
           ; if the flags.used is set and the flags.asleep is clear
           ; check to see if the current task's timer is less than current timer tick
           mov  eax,[esi + S_TASK->flags]
           and  eax,(TASK_FLAG_USED | TASK_FLAG_ASLEEP)
           cmp  eax,TASK_FLAG_USED
           jne  short task_sch_cont
           
           ; if the current task's timer value == 0, then skip the check
           mov  eax,[esi + S_TASK->timer + 0]
           mov  edx,[esi + S_TASK->timer + 4]
           or   eax,eax
           jnz  short @f
           or   edx,edx
           jz   short task_sch_cont
@@:        push ebx
           push ecx
           mov  ebx,eax     ; ecx:ebx = tasks[ebx].timer value
           mov  ecx,edx
           call get_timer_tick64
           call cmp64bitregs  ; cmp ecx:ebx,edx:eax
           ja   short task_sch_cont
           mov  dword [esi + S_TASK->timer + 0],0
           mov  dword [esi + S_TASK->timer + 4],0
           mov  cur_task,ecx   ; set cur_task to new task index
           mov  ebx,ecx        ; set it to ebx also for copy below
           jmp  short task_sch_copy  ; copy it and return
                      
task_sch_cont:
           cmp  ecx,cur_task
           jne  short task_sch_loop
           
task_sch_notfrst:
           ; increment to next task
           ; This assumes there will always be at least one task present
           ; This also assumes that the idle task will always have a value of 0 in the .timer member
task_nxt_loop:
           inc  ebx
           ; roll it over to zero if at max
           cmp  ebx,MAX_TASKS
           jb   short @f
           xor  ebx,ebx
           
@@:        mov  eax,sizeof(S_TASK)
           mul  ebx
           add  eax,TASKS_BUFF
           mov  esi,eax            ; esi = &tasks[ebx]
           
           ; if the flags.used is set and the flags.asleep is clear
           ; check to see if the current task's timer is less than current timer tick
           mov  eax,[esi + S_TASK->flags]
           and  eax,(TASK_FLAG_USED | TASK_FLAG_ASLEEP)
           cmp  eax,TASK_FLAG_USED
           jne  short task_nxt_loop
           
           ; if the current task's timer value == 0, then skip the check
           mov  eax,[esi + S_TASK->timer + 0]
           mov  edx,[esi + S_TASK->timer + 4]
           or   eax,eax
           jnz  short task_nxt_loop
           or   edx,edx
           jnz  short task_nxt_loop
           
           ; memcpy(regs, &tasks[cur_task].regs, sizeof(REGS))
task_sch_copy:
           mov  cur_task,ebx
           push sizeof(S_REGS)
           ; esi-> current task
           lea  esi,[esi+S_TASK->reg_eax]  ; reg_eax is the first of the S_REGS struct
           push esi
           push dword PARAM0
           call memcpy    
           
task_sch_done:
           pop  ebx
           pop  esi
           pop  ebp
           ret  4
task_scheduler endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; modIDTtable()
; set the IDT table
; On entry:  (count)
; On return: nothing
modIDTtable proc near
           
           push ebp
           mov  ebp,esp
           
           ; make sure the flag bytes are all zeros
           mov  edi,GDTFLGS_LOC
           mov  ecx,256
           xor  al,al
           rep
             stosb
           
           ; set the first 'count' as used.
           mov  edi,GDTFLGS_LOC
           mov  ecx,PARAM0
           mov  al,1
           rep
             stosb
           
           ; point all IDT's to their cooresponding handler
           mov  edi,IDT_ADDRESS
           mov  ecx,256
           xor  ebx,ebx            ; 1st handler (INT 0)
@@:        mov  eax,[idthandlers+ebx*4]
           mov  [edi+S_IDT->handlerlow],ax
           mov  word [edi+S_IDT->selector],0008h
           mov  byte [edi+S_IDT->zero],00h
           mov  byte [edi+S_IDT->type],8Eh  ; type (32-bit, Ring 0) (P,DPL,0,D,1,1,0)
           shr  eax,16
           mov  [edi+S_IDT->handlerhigh],ax
           inc  ebx                ; move to next handler
           add  edi,sizeof(S_IDT)
           loop @b
           
           pop  ebp
           ret  4
modIDTtable endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; get_timer_tick64()
; return the 64-bit timer tick
; On entry:  nothing
; On return: 64-bit timer tick
get_timer_tick64 proc near
           mov  eax,[time_stamp + 0]
           mov  edx,[time_stamp + 4]
           ret
get_timer_tick64 endp

time_stamp  dq 0



idthandlers   dd idt_handler00,
              dd idt_handler01,
              dd idt_handler02,
              dd idt_handler03,
              dd idt_handler04,
              dd idt_handler05,
              dd idt_handler06,
              dd idt_handler07,
              dd idt_handler08,
              dd idt_handler09,
              dd idt_handler0A,
              dd idt_handler0B,
              dd idt_handler0C,
              dd idt_handler0D,
              dd idt_handler0E,
              dd idt_handler0F,
              dd idt_handler10,
              dd idt_handler11,
              dd idt_handler12,
              dd idt_handler13,
              dd idt_handler14,
              dd idt_handler15,
              dd idt_handler16,
              dd idt_handler17,
              dd idt_handler18,
              dd idt_handler19,
              dd idt_handler1A,
              dd idt_handler1B,
              dd idt_handler1C,
              dd idt_handler1D,
              dd idt_handler1E,
              dd idt_handler1F,
              dd idt_handler20,
              dd idt_handler21,
              dd idt_handler22,
              dd idt_handler23,
              dd idt_handler24,
              dd idt_handler25,
              dd idt_handler26,
              dd idt_handler27,
              dd idt_handler28,
              dd idt_handler29,
              dd idt_handler2A,
              dd idt_handler2B,
              dd idt_handler2C,
              dd idt_handler2D,
              dd idt_handler2E,
              dd idt_handler2F,
              ; 30h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 40h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 50h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 60h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 70h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 80h
              dd idt_handler80, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; 90h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; A0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; B0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; C0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; D0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ; E0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              ;  F0h
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX,
              dd idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX, idt_handlerXX


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; On the first 6 handlers, we simply print
;  an 'INT0x' to the top corner of the screen.
;  and freeze.  We can't use putstr since
;  it might have been the culprit of the error.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x00: Divide Error Exception
;
idt_handler00 proc near uses alld
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '0')
@@:        jmp  short @b
           
           ret
idt_handler00 endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x01: Debug exception
;
idt_handler01 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '1')
@@:        jmp  short @b
           
           ret
idt_handler01 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x02: NMI Interrupt
;
idt_handler02 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '2')
@@:        jmp  short @b
           
           ret
idt_handler02 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x03: Break Point (INT 3h instruction)
;
idt_handler03 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '3')
@@:        jmp  short @b
           
           ret
idt_handler03 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x04: Overflow Exception
;
idt_handler04 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '4')
@@:        jmp  short @b
           
           ret
idt_handler04 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x05: Bound Range Exceeded Exception
;
idt_handler05 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '5')
@@:        jmp  short @b
           
           ret
idt_handler05 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x06: Invalid Opcode
;
idt_handler06 proc near
           
           pushad
           mov  ax,10h
           mov  ds,ax           
           
           mov  ebx,[esp+32]  ; address to eip
           mov  ecx,[esp+36]  ; address to cs
           
           mov  ss,ax
           mov  ebp,esp
           
           ;  now we need to find the physical address of the cs:eip, using the gdt.
           ;  we can't assume that the cs selector above is readable, it may be execute only.
           push ecx
           shr  ecx,3
           mov  eax,sizeof(S_GDT)
           mul  ecx
           pop  ecx
           mov  esi,eax
           add  esi,offset gdtoff
           
           ; calcuate into eax, the physical address of the offending instruction
           mov  al,[esi + S_GDT->basehigh]
           shl  eax,8
           mov  al,[esi + S_GDT->basemid]
           shl  eax,16
           mov  ax,[esi + S_GDT->baselow]
           add  eax,ebx
           
           ; this is where you would print a string indicating that the processor
           ; found an invalid instruction.  You could also show all the register
           ; values just before the instruction. These values are on the stack
           ; due to the pushad instruction above. (except for ds,es,etc.)
           ;
           ; eax is the physical address of this instruction
           ; ecx:ebx is the cs:eip register pair of that instruction
           ;
           ; if you are going to return to the caller, you will need to be sure
           ; to modify the return address.  You don't want to jump back to the
           ; offending instruction.  However, this may cause undesired results.
           ; the best thing to do is delete the task from the scheduler and move
           ; to the next task
           
           ; for now, we will simply print a string as with the others
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '6')
@@:        jmp  short @b
           
           ; return to the caller
           popad
           
           ret
idt_handler06 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x07: Device not available exception
;
idt_handler07 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '7')
@@:        jmp  short @b
           
           ret
idt_handler07 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x08: Double Fault
;
idt_handler08 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '8')
@@:        jmp  short @b
           
           ret
idt_handler08 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x09: Coprocessor Segment Overrun
;
idt_handler09 proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | '9')
@@:        jmp  short @b
           
           ret
idt_handler09 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0A: Invalid TSS Exception
;
idt_handler0A proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'A')
@@:        jmp  short @b
           
           ret
idt_handler0A endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0B: Segment not present
;
idt_handler0B proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'B')
@@:        jmp  short @b
           
           ret
idt_handler0B endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0C: Stack Fault Exception
;
idt_handler0C proc near

           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'C')
@@:        jmp  short @b
           
           ret
idt_handler0C endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0D: General Protection Fault
;
idt_handler0D proc near

           ; save the register values for our exception handler(s)
           push ds
           push eax
           mov  ax,10h
           mov  ds,ax
           pop  eax
           mov  exception_regs.reg_eax,eax
           mov  exception_regs.reg_ebx,ebx
           mov  exception_regs.reg_ecx,ecx
           mov  exception_regs.reg_edx,edx
           mov  exception_regs.reg_esi,esi
           mov  exception_regs.reg_edi,edi
           mov  exception_regs.reg_ebp,ebp
           pop  eax
           mov  exception_regs.reg_esp,esp
           mov  exception_regs.reg_ds,ax
           mov  exception_regs.reg_es,es
           mov  exception_regs.reg_fs,fs
           mov  exception_regs.reg_gs,gs
           mov  exception_regs.reg_ss,ss
           
           mov  eax,cr3
           mov  exception_regs.reg_cr3,eax
           
           ; move to our kernel selectors
           mov  ax,10h
           mov  es,ax
           mov  fs,ax
           mov  gs,ax
           mov  ss,ax
           
           ; get the eflags register
           pushfd
           pop  eax
           mov  exception_regs.reg_eflags,eax
           
           ; allow interrupts again
           sti
           
           ; this is where you would print a string indicating that the processor
           ; found an general protection fault.  You could also show all the register
           ; values just before the instruction.
           ;
           ; this is also where you would catch Virtual Real Mode GPF's, handle them,
           ; then jump back to the code.  Virtual Real Mode GPF's are when you have
           ; a TSS and are running in a virtual real mode environment and the code
           ; issued a protected instruction, like PUSHF or INT.
           
           ; for now, we will simply print a string as with the others
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'D')
@@:        jmp  short @b
           
           ; return to the caller
           ; place your 'return to caller' code here
           
           ret
idt_handler0D endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0E: Page Fault Exception
;
idt_handler0E proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'E')
@@:        jmp  short @b
           
           ret
idt_handler0E endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x0F: 
;
idt_handler0F proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '0')
           mov  word [edi+8],(0700h | 'F')
@@:        jmp  short @b
           
           ret
idt_handler0F endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x10: x87 Floating Point Error
;
idt_handler10 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '0')
@@:        jmp  short @b
           
           ret
idt_handler10 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x11: Alignment Check Exception
;
idt_handler11 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '1')
@@:        jmp  short @b
           
           ret
idt_handler11 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x12: Machine Check Exception
;
idt_handler12 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '2')
@@:        jmp  short @b
           
           ret
idt_handler12 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x13: SIMD Floating Point Exception
;
idt_handler13 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '3')
@@:        jmp  short @b
           
           ret
idt_handler13 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x14: Undefined
;
idt_handler14 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '4')
@@:        jmp  short @b
           
           ret
idt_handler14 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x15:  Undefined
;
idt_handler15 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '5')
@@:        jmp  short @b
           
           ret
idt_handler15 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x16:  Undefined
;
idt_handler16 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '6')
@@:        jmp  short @b
           
           ret
idt_handler16 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x17:  Undefined
;
idt_handler17 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '7')
@@:        jmp  short @b
           
           ret
idt_handler17 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x18:  Undefined
;
idt_handler18 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '8')
@@:        jmp  short @b
           
           ret
idt_handler18 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x19:  Undefined
;
idt_handler19 proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | '9')
@@:        jmp  short @b
           
           ret
idt_handler19 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1A:  Undefined
;
idt_handler1A proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'A')
@@:        jmp  short @b
           
           ret
idt_handler1A endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1B:  Undefined
;
idt_handler1B proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'B')
@@:        jmp  short @b
           
           ret
idt_handler1B endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1C:   Undefined
;
idt_handler1C proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'C')
@@:        jmp  short @b
           
           ret
idt_handler1C endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1D:  Undefined
;
idt_handler1D proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'D')
@@:        jmp  short @b
           
           ret
idt_handler1D endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1E:  Undefined
;
idt_handler1E proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'E')
@@:        jmp  short @b
           
           ret
idt_handler1E endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; 0x1F:  Undefined
;
idt_handler1F proc near
           
           mov  edi,0B8000h
           mov  word [edi+0],(0700h | 'I')
           mov  word [edi+2],(0700h | 'N')
           mov  word [edi+4],(0700h | 'T')
           mov  word [edi+6],(0700h | '1')
           mov  word [edi+8],(0700h | 'F')
@@:        jmp  short @b
           
           ret
idt_handler1F endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Interrupt 20h - 2Fh are reserved for IRQ handling
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Priority level:
;  IRQ0   20h - Timer
;  IRQ1   21h - Keyboard
;  IRQ2   22h - IRQ2 - IRQ's from slave 8259A's
;  IRQ8   28h -   CMOS RTC
;  IRQ9   29h -   Software redirectied to IRQ2
;  IRQ10  2Ah -   Free
;  IRQ11  2Bh -   Free
;  IRQ12  2Ch -   Free or PS2 mouse
;  IRQ13  2Dh -   FPU
;  IRQ14  2Eh -   HDC1
;  IRQ15  2Fh -   HDC2 or Free if absent
;  IRQ3   23h - COM2/4
;  IRQ4   24h - COM1/3
;  IRQ5   25h - Data requested from LPT2 or Free if absent
;  IRQ6   26h - FDC interrupt
;  IRQ7   27h - Data requested from LPT1 or Free if absent

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 20h *TIMER*
;   The system timer tick (IRQ 0)
;   This handler gets called each time by the system timer tick (IRQ 0)
;   We have stepped-up the freqency of this tick to 100hz. (1 jiffy) (100 jiffy's = 1 second)
;    i.e.:  Each time this gets called, 10mS has elapsed.
;           10mS = 10,000 uS
;           10mS * 100hz = 1 second (1 second = 1,000,000 uS)
;   ** No need to clear interrupts (cli), the master 8259A will delay
;        all other interrupts until EOI is issued. (use sti to allow them)
; make it global to all kernel code
;
not_an_interrupt db  FALSE
saved_regs       st  S_REGS

idt_handler20 proc near
           
           push ds
           push eax
           mov  ax,10h
           mov  ds,ax
           pop  eax
           mov  saved_regs.reg_eax,eax
           mov  saved_regs.reg_ebx,ebx
           mov  saved_regs.reg_ecx,ecx
           mov  saved_regs.reg_edx,edx
           mov  saved_regs.reg_esi,esi
           mov  saved_regs.reg_edi,edi
           mov  saved_regs.reg_ebp,ebp
           pop  eax
           add  esp,12  ; remove the PUSHs from the INT (must be after the pushes/pops above)
           mov  saved_regs.reg_esp,esp
           mov  saved_regs.reg_ds,ax
           mov  saved_regs.reg_es,es
           mov  saved_regs.reg_fs,fs
           mov  saved_regs.reg_gs,gs
           mov  saved_regs.reg_ss,ss
           mov  eax,[esp-4]              ; get the eflags off the stack
           mov  saved_regs.reg_eflags,eax
           mov  eax,[esp-8]              ; save the cs value from the stack
           mov  saved_regs.reg_cs,ax
           mov  eax,[esp-12]             ; save the eip value from the stack
           mov  saved_regs.reg_eip,eax
           
           ; setup a local stack
           mov  esp,LSTACK_LOC
           add  esp,LSTACK_SIZE
           
           ; setup the kernels DS and SS
           mov  ax,10h   ; we are in the kernel
           mov  ss,ax
           mov  es,ax
           mov  fs,ax
           mov  gs,ax
           
           cmp  byte not_an_interrupt,0
           jne  short @f
           
           ; the optimizer will change the add x,1 to increment.
           ; we can't have it do that because of the adc.
         .optoff
           add  dword [time_stamp + 0],1   ; increment the timer tick
           adc  dword [time_stamp + 4],0
         .opton
           
           ; do the task scheduler
@@:        push offset saved_regs
           call task_scheduler
           
           ; interrupt done
           cmp  byte not_an_interrupt,0
           jne  short @f
           push EOI
           push PIC_MASTER
           call outpb                      ; end of interrupt on controller
           
           ; make sure it is clear for next interrupt
@@:        mov  byte not_an_interrupt,0
           
           ; put the new register values back in the registers for the next task
           mov  ss,saved_regs.reg_ss
           mov  esp,saved_regs.reg_esp
           mov  eax,saved_regs.reg_eflags
           push eax    ; new eflags value
           movzx eax,word saved_regs.reg_cs
           push eax    ; new cs value
           mov  eax,saved_regs.reg_eip
           push eax    ; new eip value
           
           mov  eax,saved_regs.reg_eax
           mov  ebx,saved_regs.reg_ebx
           mov  ecx,saved_regs.reg_ecx
           mov  edx,saved_regs.reg_edx
           mov  esi,saved_regs.reg_esi
           mov  edi,saved_regs.reg_edi
           mov  ebp,saved_regs.reg_ebp
           mov  es,saved_regs.reg_es
           mov  fs,saved_regs.reg_fs
           mov  gs,saved_regs.reg_gs
           mov  ds,saved_regs.reg_ds
           
           iretd
idt_handler20 endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 21h *Keyboard*
;   The keyboard interrupt (IRQ 1)
;   This handler gets called each time by the keyboards hardware interrupts
;   ** No need to clear interrupts (cli), the master 8259A will only
;       let INT 20h above fire.
;       If INT 20h does something with the keyboard, then we need cli
idt_handler21 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(01h * 4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler21 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 22h **
;
idt_handler22 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(02h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler22 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 23h ** COM2/4
;
idt_handler23 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(03h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler23 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 24h ** COM1/3
;
idt_handler24 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(04h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler24 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 25h ** Bus Mouse and others
;
idt_handler25 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(05h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler25 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 26h ** FDC
;
idt_handler26 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(06h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler26 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 27h ** Parallel Port
;
idt_handler27 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(07h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler27 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 28h ** RTC
;
idt_handler28 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(08h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler28 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 29h **
;
idt_handler29 proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(09h*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler29 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Ah **
;
idt_handler2A proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Ah*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2A endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Bh **
;
idt_handler2B proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Bh*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2B endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Ch ** PS2 Mouse
;
idt_handler2C proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Ch*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2C endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Dh **
;
idt_handler2D proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Dh*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2D endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Eh **
;
idt_handler2E proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Eh*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2E endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 2Fh **
;
idt_handler2F proc near

           sti
           pushad
           push ds
           mov  ax,10h 
           mov  ds,ax
           mov  es,ax
           
           ; call the assigned handler.  If we haven't assigned one
           ; to this handler yet, it will call a dummy handler which
           ; simply returns.
           ; you would typically assign a handler to this pointer when
           ; you detect the hardware that uses this handler.
           mov  eax,(irq_handlers+(0Fh*4))
           call eax
           
           ; interrupt done
           push EOI
           push PIC_SLAVE
           call outpb
           
           push EOI
           push PIC_MASTER
           call outpb
           
           pop  ds
           popad 
           iretd
idt_handler2F endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler 80h ** Our Software Interrupt Handler
; this is our software interrupt called by tasks to perform
;  functions like printing a string, etc.
; this is *not* reenterant
idt_handler80 proc near
           
           cli
           push ds       ; save the caller's DS selector
           push eax      ; save eax
           mov  ax,10h   ; change to our code selector
           mov  ds,ax    ;
           pop  eax      ; restore the callers eax value
           
           ; save the registers passed to us
           mov  user_regs.reg_eax,eax
           mov  user_regs.reg_ebx,ebx
           mov  user_regs.reg_ecx,ecx
           mov  user_regs.reg_edx,edx
           mov  user_regs.reg_esi,esi
           mov  user_regs.reg_edi,edi
           mov  user_regs.reg_ebp,ebp
           
           ; restore the passed DS selector value
           pop  eax
           mov  user_regs.reg_esp,esp
           mov  user_regs.reg_ds,ax
           mov  user_regs.reg_es,es
           mov  user_regs.reg_fs,fs
           mov  user_regs.reg_gs,gs
           mov  user_regs.reg_ss,ss
           
           ; and the flags too
           pushfd
           pop  eax
           mov  user_regs.reg_eflags,eax
           
           ; point everything to our selectors
           mov  ax,10h
           mov  es,ax
           mov  fs,ax
           mov  gs,ax
           
           ; we need to create a new temp stack that has a base selector of 10h
           push 00100000h      ; 1 meg
           call malloc         ; allocate the memory
           mov  ebx,eax        ; save this address
           add  eax,00100000h  ; point esp to the top of it
           mov  esp,eax        ;
           mov  ax,10h         ;
           mov  ss,ax          ; set ss to the zero base
           push ebx            ; save the address of the stack on this new stack
           
           sti
           
           ; call the handler
           call task_int80h
           
           ; restore the register values from our handler
           cli
           
           pop  ebx            ; restore the address of the stack before we free it
           
           ; restore the users stack
           mov  ax,user_regs.reg_ss
           mov  ecx,user_regs.reg_esp
           mov  ss,ax
           mov  esp,ecx
           
           ; free the new stack above
           ; (saving the flags register)
           pushfd
           push ebx
           call mfree
           
           ; restore the flags into edx
           pop  edx
           mov  eax,user_regs.reg_eflags
           and  eax, 000000FFh
           and  edx,0FFFFFF00h
           or   edx,eax
           or   edx,EFLAGS_INT_ENABLE
           mov  [esp+8],edx
           
           mov  eax,user_regs.reg_eax
           mov  ebx,user_regs.reg_ebx
           mov  ecx,user_regs.reg_ecx
           mov  edx,user_regs.reg_edx
           mov  esi,user_regs.reg_esi
           mov  edi,user_regs.reg_edi
           mov  ebp,user_regs.reg_ebp
           mov  gs,user_regs.reg_gs
           mov  fs,user_regs.reg_fs
           mov  es,user_regs.reg_es
           
           iretd  ; return to the code
idt_handler80 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; INT handler XXh ** All other unknown handlers point here
;
; this is a dummy irq handler that gets called
;  from an irq that we haven't assigned a driver
;  for yet.
idt_handlerXX proc near
              iret
idt_handlerXX endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This is called by the idt_handler80 above.
; it is not reenterant
; it uses the values in user_regs
;
task_int80h   proc near
           
           ; switch(user_regs.reg_eax)
           ;
           ; case 1:
           cmp  dword user_regs.reg_eax,1
           jne  short not_print
           
           ; service 1
           ;  eax = 1
           ;  edx = kernel_ds:offset string
           push dword user_regs.reg_edx
           call putstr
           ret
           
           ; default
not_print: 
           
           ret
task_int80h   endp



.end                                                                                                                            
