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
; This is the memory allocation/initialization code
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MEM_MAGIC_USED  equ  0DDDDDDDDh
MEM_MAGIC_FREE  equ  0FFFFFFFFh

SPLIT_SIZE      equ  1024  ; split size

; memory control block
S_MCB        struct
  magic      dword
  tot_size   dword
  prev       dword
  next       dword
  available  byte
  resv       dup 31  
S_MCB        ends

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine simply sets up a simple linked list
;  of memory blocks.  The allocator then simply needs
;  to find a free block that meets its needs, mark it
;  as used, and return.
; On entry:  nothing
; On return: nothing
mem_initalize proc near
           
           push ebp
           mov  ebp,esp
           
           push edi
           
           ; pointer to our RAM
           mov  edi,KERN_RAM
           
           ; set the first MCB in the list to available
           mov  byte [edi+S_MCB->available],TRUE
           mov  dword [edi+S_MCB->magic],MEM_MAGIC_FREE
           
           mov  eax,(END_RAM - sizeof(S_MCB))
           mov  [edi+S_MCB->next],eax
           mov  dword [edi+S_MCB->prev],NULL
           
           mov  eax,(END_RAM - KERN_RAM - sizeof(S_MCB))
           mov  [edi+S_MCB->tot_size],eax
           
           ; set the last MCB in the list to used.
           mov  edi,[edi+S_MCB->next]
           mov  byte [edi+S_MCB->available],FALSE
           mov  dword [edi+S_MCB->magic],MEM_MAGIC_USED
           
           mov  dword [edi+S_MCB->prev],NULL
           mov  dword [edi+S_MCB->next],NULL
           mov  dword [edi+S_MCB->tot_size],sizeof(S_MCB)
           
           pop  edi
           
           pop  ebp
           ret
mem_initalize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; malloc()
; allocates system memory
; On entry:  (size)
; On return: eax = physical address or NULL
malloc     proc near
           
           push ebp
           mov  ebp,esp           
           
           push edi
           push esi
           mov  esi,KERN_RAM
           
           ; don't allow multitasking
           push TRUE
           call task_lock_set
           
           ; calculate the min size that we can allocate given 'size'
           mov  ecx,PARAM0
           add  ecx,15
           and  ecx,(~15)
           ; add size of control block
           add  ecx,sizeof(S_MCB)
           
malloc_loop:
           or   esi,esi
           jz   malloc_none
           
           cmp  byte [esi+S_MCB->available],FALSE
           je   malloc_not_avail
           cmp  ecx,[esi+S_MCB->tot_size]
           ja   malloc_not_avail
           cmp  dword [esi+S_MCB->magic],MEM_MAGIC_FREE
           jne  malloc_not_avail
                      
           ; found available block
           ; if size requested is some-what smaller than this blocks size,
           ;  split the block into two (1 used, 1 free).
           mov  edx,[esi+S_MCB->tot_size]
           sub  edx,ecx
           cmp  edx,SPLIT_SIZE
           jb   short malloc_dont_split
           
           ; split the block into 2 blocks
           ; clear the info for the emtpy one
           lea  edi,[esi+ecx]
           
           ; set the info
           mov  byte [edi + S_MCB->available],TRUE
           mov  dword [edi + S_MCB->magic],MEM_MAGIC_FREE
           mov  [edi + S_MCB->tot_size],edx
           mov  [edi + S_MCB->prev],esi
           mov  eax,[esi + S_MCB->next]
           mov  [edi + S_MCB->next],eax
           mov  [esi + S_MCB->next],edi
           mov  [esi + S_MCB->tot_size],ecx
           
malloc_dont_split:
           mov  byte [esi + S_MCB->available],FALSE
           mov  dword [esi + S_MCB->magic],MEM_MAGIC_USED
           
           ; allow multitasking again
           push FALSE
           call task_lock_set
           
           ; return the address of the memory (not MCB)
           mov  eax,esi
           add  eax,sizeof(S_MCB)
           jmp  short malloc_done
           
malloc_not_avail:
           ; point to next one and try again
           mov  esi,[esi + S_MCB->next]
           jmp  short malloc_loop

malloc_none:
           ; if we don't have any memory.
           ;  (Print a debug error to the screen)
	         push offset memory404_str
           call putstr
           
           push FALSE
           call task_lock_set
           
           ; return NULL
           xor  eax,eax
           
malloc_done:           
           pop  esi
           pop  edi
           pop  ebp
           ret  4
malloc     endp

memory404_str  db  13,10,' .....OUT of MEMORY.....',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; mfree()
; frees allocated memory
; On entry:  (address)
; On return: nothing
mfree      proc near
           
           push ebp
           mov  ebp,esp
           
           push esi
           push edi
           
           ; first check to see if NULL was passed
           mov  eax,PARAM0
           or   eax,eax
           jz   short mfree_done
           
           ; don't allow multitasking
           push TRUE
           call task_lock_set
           
           ; backup from the given pointer to find the mcb
           sub  eax,sizeof(S_MCB)
           mov  esi,eax
           
           ; for now, a simple check
           cmp  dword [esi + S_MCB->magic],MEM_MAGIC_USED
           jne  short mfree_done
           
           ; mark the block as being available
           mov  byte [esi + S_MCB->available],TRUE
           mov  dword [esi + S_MCB->magic],MEM_MAGIC_FREE
           
           ; consolidate all consecutive free blocks before and after this one
           ; making sure to do the next before the previous
           mov  edi,[esi + S_MCB->next]
           or   edi,edi
           jz   short mfree_next_done
           
           cmp  byte [edi + S_MCB->available],TRUE
           jne  short mfree_next_done
           cmp  dword [edi + S_MCB->magic],MEM_MAGIC_FREE
           jne  short mfree_next_done
           
           ; block after is free, so consolidate sizes and change next member
           mov  eax,[edi + S_MCB->tot_size]
           add  [esi + S_MCB->tot_size],eax
           mov  eax,[edi + S_MCB->next]
           mov  [esi + S_MCB->next],eax
           
mfree_next_done:
           mov  edi,[esi + S_MCB->prev]
           or   edi,edi
           jz   short mfree_done
           
           cmp  byte [edi + S_MCB->available],TRUE
           jne  short mfree_done
           cmp  dword [edi + S_MCB->magic],MEM_MAGIC_FREE
           jne  short mfree_done
           
           ; block before is free, so consolidate sizes and change next member
           mov  eax,[esi + S_MCB->tot_size]
           add  [edi + S_MCB->tot_size],eax
           mov  eax,[esi + S_MCB->next]
           mov  [edi + S_MCB->next],eax
           
mfree_done:
           ; allow multitasking again
           push FALSE
           call task_lock_set
           
           pop  edi
           pop  esi
           pop  ebp
           ret  4
mfree      endp

.end
