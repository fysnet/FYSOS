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
; The following routines are simple STDLIB/STDIO/etc routines.
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strcpy(dest, src)
; copies a string from src to dest
; On entry:  (dest, src)
; On return: nothing
strcpy     proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           push esi
           
           mov  edi,PARAM0 ; dest
           mov  esi,PARAM1 ; src
           cld
@@:        lodsb
           stosb
           or   al,al
           jnz  short @b
           
           pop  esi
           pop  edi
           
           pop  ebp
           ret  8
strcpy     endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; strncpyz()
; copies a string from src to dest, stops at len - 1, asciiz's it
; On entry:  (dest, src, len)
; On return: nothing
strncpyz   proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           push esi
           
           mov  edi,PARAM0 ; dest
           mov  esi,PARAM1 ; src
           mov  ecx,PARAM2 ; len
           .adsize
           jcxz short strncpyz_done
           
           cld
@@:        lodsb
           stosb
           or   al,al
           jz   short @f
           loop @b
           
@@:        mov  byte [edi-1],0
           
strncpyz_done:
           pop  esi
           pop  edi
           
           pop  ebp
           ret  12
strncpyz   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; memcpy()
; copies a buffer from src to dest of len in length
; On entry:  (dest, src, len)
; On return: nothing
memcpy     proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           push esi
           
           mov  edi,PARAM0  ; dest
           mov  esi,PARAM1  ; src
           mov  ecx,PARAM2  ; len
           cld
           rep
            movsb
           
           pop  esi
           pop  edi
           
           pop  ebp
           ret  12
memcpy     endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; memset()
; sets a buffer's contents to a given value
; On entry:  (dest, val, len)
; On return: nothing
memset     proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           
           mov  edi,PARAM0  ; dest
           mov  eax,PARAM1  ; val
           mov  ecx,PARAM2  ; len
           cld
           rep
            stosb
           
           pop  edi
           
           pop  ebp
           ret  12
memset     endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; outpb(addr, byte)
; sends the byte to addr
; On entry:  (addr, byte)
; On return: byte
outpb      proc near
           push ebp
           mov  ebp,esp           
           
           push eax
           push edx
           
           mov  edx,PARAM0
           mov  eax,PARAM1
           out  dx,al
           
           pop  edx
           pop  eax
           
           pop  ebp
           ret  8
outpb      endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; iodelay()
; delays a short time for the IO to happen
; On entry:  nothing
; On return: nothing
iodelay    proc near
           push ebp
           mov  ebp,esp
           
           ; at this time we do nothing since we
           ;  assume a 486 will delay for us.
           
           ; however, older computers needed a small delay
           ; between I/O accesses.  If we will have a
           ; 486+, we can skip this function and remove
           ; it from the code.
           
           pop  ebp
           ret
iodelay    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; mul64_32()
; multiplies edx:eax by ecx
; On entry:  edx:eax = value to multiply
;            ecx     = multiplier
; On return: edx:eax = product
mul64_32   proc near
           push ebp
           mov  ebp,esp
           
           or   edx,edx         ; test for edx as zero.
           jnz  short long_way  ; if so, just 'mul ecx' and return
           
           mul  ecx
           pop  ebp
           ret

; long way
;      ebx  = edx * ecx (ignore the overflow into edx)
;  edx:eax  = (orginal) eax * ecx
;  edx:eax += ebx:zero
long_way:  push ebx
           
           mov  ebx,eax         ; save original eax in ebx
           mov  eax,edx         ; eax = edx
           mul  ecx             ;  = edx * ecx
           xchg ebx,eax         ; save result in ebx, restoring eax
           mul  ecx             ;  = eax * ecx
           add  edx,ebx         ;
           
           pop  ebx
           pop  ebp
           ret
mul64_32   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; min()
; returns the minimum value of eax and ecx
; On entry:  eax and ecx (signed values)
; On return: eax = the lesser of eax and ecx
min        proc near
           push ebp
           mov  ebp,esp
           
           cmp  eax,ecx
           jle  short @f
           xchg eax,ecx
           
@@:        pop  ebp
           ret
min        endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; cmp64bitregs()
; compare two 64-bit values
; On entry:  ecx:ebx is compared to edx:eax
; On return: carry set if ecx:ebx < edx:eax
;            zero  set if both are equ
cmp64bitregs proc near
           cmp  ecx,edx
           jne  short @f
           cmp  ebx,eax
@@:        ret
cmp64bitregs endp


.end
