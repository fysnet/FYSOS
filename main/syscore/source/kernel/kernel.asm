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

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Please note that all of the functions here use the STDCALL convention except
;  that the callee (the procedure itself) cleans the stack in most cases.
;  printf() and others would allow the caller to clean the stack.
;
; Parameters are pushed onto the stack in right-to-left order
; Registers EAX, ECX, and EDX are designated for use within the function.
; Return values are stored in the EAX register.
; All parameters are passed as 32-bit values
;
; Also note that if you use this convention, you can not use the 'uses' keyword
;  on the proc near line unless you update the EBP register too.
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

.model tiny

outfile 'kernel.sys'

; these two includes are for the first DATA_SIZE bytes in this kernel file
include '..\include\boot.inc'
include '..\loader\loader.inc'

include 'kernel.inc'

.code
.486
.pmode

           org   00h          ; NBASM needs a zero first or it will default to 100h
           orgnf KERNEL_BASE  ; remember, if you adjust the base in kernel.sys's header
                              ;  you need to adjust it here too.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This is the DATA_SIZE bytes that gets filled and transfered from the loader code.
; We include it here so that we may access it with NBASM assembly opcodes
; The assembler will assign zeros to this DATA_SIZE bytes, but the loader will
;  assign them to the correct values when it copies them over.
;
gdtoff        dup (256*8),0   ; for a total of 256 entries (2048 bytes)
idtoff        dw  0   ; set in loader.asm
              dd  0   ; set in loader.asm
boot_data     st  S_BOOT_DATA  ; booted data
org_int1e     dd  0            ; original INT1Eh address
floppy_1e     st  S_FLOPPY1E   ; floppies status
time          st  S_TIME       ; current time passed to kernel
apm           st  S_APM        ; Advanced Power Management
bios_equip    dw  0            ; bios equipment list at INT 11h (or 0040:0010h)
kbd_bits      db  0            ; bits at 0x00417
memory        st  S_MEMORY     ; memory blocks returned by INT 15h memory services
a20_tech      db  0            ; the technique number used to enable the a20 line
vesa_video    dup 256,0        ; the vesa video informtion from int 10h/4f00h
vesa_modes    dup 128,0        ; 63 16-bit mode values + ending 0FFFFh
vesa_oem_name dup 33,0         ; 32 + null vesa oem name string
drive_params  dup (96*10),0    ; up to 10 hard drive parameter tables
              dup (DATA_SIZE - ($ - gdtoff)),0  ; reserved (padding to DATA_SIZE bytes)

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This is were our code actually begins and where the Loader jumped to.
;
; At this point, ds=es=fs=gs=ss=0x00000000 or a flat address space
; the stack is at ss:esp where esp = ((STACK_BASE + STACK_SIZE)-4)


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Initialize our memory allocator
           call mem_initalize

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Re-map the PIC
           push SLAVE_DEFAULT
           push MASTER_DEFAULT
           call remap_pic
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Speed up the timer to 100hz
           ;  (fires every 10ms)
           push 100
           call set_timer
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set our internal clock value
           push offset time
           call set_clock
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set the conio values for our screen
           push 25    ; y max (height)
           push 0     ; y min
           push 80    ; x max (width)
           push 0     ; x min
           call set_screen  ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Create the IDT and GDT
           ; must be after the a20 set code (in loader.sys)
           push 3                ; unused, code, data = three used GDT entries
           call modIDTtable
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print startup string
           ; (We can't call INT 80h yet, we haven't set the
           ;  interrupts on yet)
	         push offset startup_str
           call putstr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set the starting IRQ handlers
           call irq_initialize
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; This is where our Multitasking should start
           call task_init
           
           ; make_thread(k_main, esp, 8, ebp, 0, TASK_FLAG_USED | TASK_TYPE_KERNEL, FALSE)
           mov  eax,esp   ; have to get esp now instead of after the first push'es
           push FALSE     ; don't modify the interrupt flag (i.e: don't use cli and sti)
           push (TASK_FLAG_USED | TASK_TYPE_KERNEL)  ; user flags
           push 0         ; not used.  Left for future enhancements
           push 0         ; push esp + 0 = ebp to use
           push 0         ; use esp as is
           push eax       ; saved esp from above
           push offset k_main  ; offest to code to begin with
           call make_thread
           
           ; returns this id in eax
           mov  main_thread_id,eax
           
           ; if eax == 0, then there was an error
           or   eax,eax
           jnz  short main_wait
           push offset main_thread_error_str
           call putstr
@@:        jmp  short @b
           
           ; we should be ready for interrupts to happen
main_wait: push FALSE           ; unlock the scheduler
           call task_lock_set   ;
           
           sti                  ; finally allow interrupts
@@:        jmp  short @b        ; do nothing until the first IRQ0 fires.
           
             
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; some data for this area
startup_str     db  13,10,' *Konan*  aka FYS-OS'
                db  13,10,' Forever Young Software'
                db  13,10,' Build: ', _DATE_ ,13,10,0
main_thread_id  dd 0
main_thread_error_str  db 13,10,'We had an error starting k_main.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the memory stuff
include 'memory.inc'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the IDT and handlers
include 'handlers.inc'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the timer stuff
include 'timer.inc'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the conio stuff
include 'conio.inc'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; include the stdlib stuff
include 'stdlib.inc'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This is were our kernel is ready to start detecting hardware and other items
;
.para
k_main:    
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; This is were you would continue your kernel code.
           ; You currently have a running Round-Robin style task scheduler,
           ;  ready for you to use make_thread() to add tasks to it.
           ; You have a valid and ready to use interrupt system.
           ; Now would be the time to start enumerating devices and
           ;  loading drivers for them.
           ; 
           ; See Chapter 11 for more information on what to do now.
           ; 
           ; For now, I will print a string to the screen and then
           ;  leave the rest to you.
           ;
           ; Happy coding.
           ;
           mov  eax,1
           mov  edx,offset this_string
           int  80h
           
done:      jmp  short done
         

this_string db  13,10,10,'   We are here.....',0



.end
