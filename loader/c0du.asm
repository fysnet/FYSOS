;
;  Copyright (c) 2016, Alexey Frunze
;  Copyright (c) 2017, Benjamin David Lunt (modifications only)
;  2-clause BSD license.
;
; Notes from Ben:
;  assemble with: 
;    nasm -f elf c0du.asm -o c0du.o
;  or include c0du.asm in your resource file and let smlrcc do it for you
;
;  I removed the IRQ5 exception code, added the call to main(), and added
;   the code to preserve the register values from boot loader to 2nd stage main().
;
;  This assumes the boot loader has patched the INT 21h vector to be a single
;    iret
;  instruction.  This way we have a stub at the beginning of this loader incase
;  this .exe is executed at the (DOS) command line.
;
bits 16

    extern _main
    extern __start__relot, __stop__relot
    extern __start__relod, __stop__relod
    extern __start__bss
    extern __stop__bss

section .text

    global __start
__start:
    
    ; we need to save the passed general registers so that we can
    ;  pass them on to main().  We preserve: eax, ebx, ecx, edx, esi, edi, ebp, eflags, ds, and es
    push    es
    push    ds
    pushfd
    push    ebp
    push    edi
    push    esi
    push    edx
    push    ecx
    push    ebx
    push    eax
    
    ; Perform code and data relocations.
    ; Do this without using unreal (AKA big real) mode to make sure that
    ; it doesn't get disabled in the process by some driver or ISR.
    call    .labnext
.labnext:
    xor     ebx, ebx
    mov     bx, cs
    shl     ebx, 4
    xor     eax, eax
    pop     ax
    add     ebx, eax
    sub     ebx, .labnext   ; ebx now equals base physical address
    
    ; this is a sub so that if the .exe file is ever executed in DOS, it will
    ; simply print and exit.  this assumes the boot loader patched INT21h vector
    ; to simply iret.
    mov     eax,ebx
    shr     eax,4
    mov     ds,ax
    mov     ah,09h
    mov     dx,.stub_string
    int     21h
    mov     ah,4Ch
    int     21h
    
    ; Patch addresses recorded in .relod (.relod is generated by the linker).
    ; [Note that the .relot section (handled later) contains some of the same
    ; addresses contained in the .relod section and .relod must be processed
    ; before .relot. .relot is used to transform flat 32-bit addresses into
    ; far addresses of a form segment:offset and this can only be done when
    ; the address relocation (using .relod) has been done.]
    ; Note that the following loop patches addresses in both code and data,
    ; including the addresses in the two following instructions!
    mov     esi, __start__relod
    mov     ebp, __stop__relod
.relo_data_loop:
    cmp     esi, ebp
    jae     .relo_data_done

    lea     edi, [ebx + esi] ; edi = physical address of a relocation table element

    ror     edi, 4
    mov     ds, di
    shr     edi, 28

    mov     edi, [di]
    add     edi, ebx ; edi = physical address of a dword to which to add ebx

    ror     edi, 4
    mov     ds, di
    shr     edi, 28

    add     [di], ebx ; actual relocation

    add     esi, 4
    jmp     .relo_data_loop
.relo_data_done:

    ; Patch direct calls recorded in .relot (.relot is generated by the compiler
    ; and/or written by the programmer).
    mov     esi, __start__relot
    mov     ebp, __stop__relot
.relo_text_loop:
    cmp     esi, ebp
    jae     .relo_text_done

    mov     edi, esi ; edi = physical address of a relocation table element
    ror     edi, 4
    mov     ds, di
    shr     edi, 28

    mov     edi, [di] ; edi = address of an address which to transform into seg:ofs far address

    ror     edi, 4
    mov     ds, di
    shr     edi, 28

    mov     eax, [di]
    shl     eax, 12
    rol     ax, 4
    mov     [di], eax ; actual transformation
    
    add     esi, 4
    jmp     .relo_text_loop
.relo_text_done:

    ; Init .bss

    mov     edi, __start__bss
    mov     ebx, __stop__bss
    sub     ebx, edi
    ror     edi, 4
    mov     es, di
    shr     edi, 28
    xor     al, al
    cld

.bss1:
    mov     ecx, 32768
    cmp     ebx, ecx
    jc      .bss2

    sub     ebx, ecx
    rep     stosb
    and     di, 15
    mov     si, es
    add     si, 2048
    mov     es, si
    jmp     .bss1

.bss2:
    mov     cx, bx
    rep     stosb

    ; Call __setup_unreal()
    db      0x9A
.patch_setup_unreal_addr:
    dd      ___setup_unreal

    ; We can now use flat 32-bit addresses with zero loaded into ds, es, fs, gs
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    
    ; push the address of the registers on to the stack so that
    ;  we can use 'int main(struct REGS *regs)'
    mov     ebx, registers
    pop     dword [ebx]    ; eax
    pop     dword [ebx+4]  ; ebx
    pop     dword [ebx+8]  ; ecx
    pop     dword [ebx+12] ; edx
    pop     dword [ebx+16] ; esi
    pop     dword [ebx+20] ; edi
    pop     dword [ebx+24] ; ebp
    pop     dword [ebx+28] ; eflags
    pop     dword [ebx+32] ; ds, es
    push    ebx ; push the address of the reg structure
    
    ; Call far _main()
    db      0x9A
.patch_start_addr:
    dd      _main       ; main is fixed up by code
    ; _main() shouldn't return
    

; this needs to remain in the 'text' section so that it is in the base segment
;
.stub_string  db  'This loader file should not be executed from the command line.',13,10,'$'
    

    global ___setup_unreal
___setup_unreal: ; far
    ; Set 4GB segment limits for ds, es, fs, gs
    pushfd                          ; we will preserve EFLAGS.IF
    push    eax
    push    ebx
    push    ds
    push    es
    push    fs
    push    gs

    ; Prepare a temporary GDT on the stack
    ; 32-bit 4GB data segment descriptor (selector 0x10)
    push    dword 0x00cf9200
    push    dword 0x0000ffff
    ; 16-bit 64KB code segment (starting at .prot) descriptor (selector 0x08)
    mov     eax, .prot
    rol     eax, 16
    push    word 0
    or      ah, 0x9a
    push    ax
    mov     ax, 0xffff
    push    eax
    ; NULL descriptor (selector 0x00)
    push    dword 0
    push    dword 0

    ; Load GDTR
    xor     eax, eax
    mov     ax, ss
    shl     eax, 4
    movzx   ebx, sp
    add     eax, ebx                ; eax = GDT address
    push    eax
    push    word 3*8-1              ; GDT size - 1
    lgdt    [ss:bx-6]               ; load the GDTR

    ; Enter protected mode
    cli                             ; disable interrupts
    mov     ebx, cr0
    inc     ebx
    mov     cr0, ebx
    ; jmp far 0x08:.prot
    jmp     0x08:0
.prot:

    ; Reload the segment registers to activate the new segment limits.
    ; We don't need ss to have a 4GB limit as the stack is still restricted to 64KB.
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    ; Leave protected mode
    dec     ebx
    mov     cr0, ebx
    ; jmp far .real_addr
    db      0xEA
.patch_real_addr:
    dd      .real_addr
.real_addr:
    add     sp, 3*8+6               ; remove GDT and GDTR from the stack

    ; Reload the segment registers to match the base address and the selector
    pop     gs
    pop     fs
    pop     es
    pop     ds
    pop     ebx
    pop     eax
    popfd                           ; restore interrupt "enabledness"
    retf ; far

; this is the space we save the initial registers to.
;  we pass the address of this block to _main as a parameter.
; 
;   int main(struct REGS *regs) {
;
section .data
registers:
  times 8 dd 0  ; 8 registers (eax, ebx, ecx, edx, esi, edi, ebp, eflags in that order)
  times 2 dw 0  ; ds, es


section .relot ; .relot must exist for __start__relot and __stop__relot to also exist
    dd      __start.patch_setup_unreal_addr ; patch the far call to __setup_unreal()
    dd      __start.patch_start_addr ; patch the far call to __start__()
    dd      ___setup_unreal.patch_real_addr ; patch the far jump to switch to real mode

;section .relod ; .relod must exist for __start__relod and __stop__relod to also exist
;               ; the linker will generate .relod for us

section .bss ; .bss must exist for __start__bss and __stop__bss to also exist
;    global ___GpCnt
;___GpCnt resd 1
    global ___pOldInt0xdIsr
___pOldInt0xdIsr resd 1
Int0xdIsrDepth resd 1
