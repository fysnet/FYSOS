comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: mbr.asm                                                            *
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
*   EQUates for mbr.asm                                                    *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.74                                         *
*          Command line: nbasm mbr<enter>                                  *
*                                                                          *
* Last Updated: 27 Sept 2017                                               *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* MBR Boot.  This is the boot sector that gets placed in the               *
*   first sector of the actual, physical disk.                             *
* This boot code does the following:                                       *
*  - moves itself to 0:7C00h+200h (just passed this sector)                *
*  - Finds active partition, will search extended partitions               *
*  - Loads and executes active partition                                   *
*  - If this code does not find an active partition, it checks             *
*    the next BIOS disk number for a valid disk, then loads that           *
*    MBR and jumps to it (if a valid sig of 0xAA55 or 0x55AA is found).    *
*                                                                          *
* Assumptions:                                                             *
*  - none at this time                                                     *
*                                                                          *
* On entry to this boot code:                                              *
*     cs:ip = usually 0000:7C00h or 07C0:0000h but can be different        *
*        dl = boot drive number                                            *
*     es:di = pnp address (if pnp is supported)                            *
*                                                                          *
***************************************************************************|

outfile 'mbr.bin'    ; name the file to create

.model tiny

USE_LARGE_DISK  equ  1  ; 1 = use the extended read service (64-bit LBA's)
                        ; 0 = use the standard BIOS read service (24-bit LBA's)

PART_ENTRY struct
  boot_id    byte       ; boot id
  s_head     byte       ; starting head number
  s_sector   byte       ; hi 2 bits = hi 2 bits of cyl, low 6 bits = sector
  s_cylinder byte       ; low 8 bits of 10 bit cyl
  sys_id     byte
  e_head     byte       ; ending head number
  e_sector   byte       ; hi 2 bits = hi 2 bits of cyl, low 6 bits = sector
  e_cylinder byte       ; low 8 bits of 10 bit cyl
  start_lba  dword
  size       dword
PART_ENTRY ends

.code
.186
           ; seg = 07C0h and offset 0000h, we can read in 127 MBR's on our stack.
           org  00h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set up the stack and seg registers (real mode)
           mov  ax,07C0h                ; set our stack, DS, ES
           mov  ds,ax                   ;
           xor  si,si                   ; (we need si to be zero below)
           mov  ss,si                   ; first push at 0x07BFE
           mov  sp,7C00h                ;

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the BIOS is PnP aware, it will pass the PnP address
           ;  in es:di.  We don't use it in this MBR, but we pass
           ;  it on to the boot partition incase it does.
           mov  [pnp_address+0],di
           mov  [pnp_address+2],es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           push 0F000h             ; if bits 15:14 are still set
           popf                    ;  after pushing/poping to/from
           pushf                   ;  the flags register then we have
           pop  ax                 ;  a 386+
           popf                    ; restore the interrupt bit
           and  ax,0F000h          ;
           jnz  short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  (display_string - 200h)  ; since we haven't moved yet, we are 200h bytes before the assembler thinks we are

not386str  db  13,10,'Processor is not a 386 compatible processor.'
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;          
.386P   ; allow processor specific code for the 386
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; BIOS sets dl to drive id when calling bootsector.
           ; also, reset the drive service
@@:        mov  physical_drive,dl       ; make sure we don't mess up DL above
           xor  ah,ah                   ;
           int  13h                     ; reset disk system
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we are using the standard BIOS read service, we will need to
           ; get the disk's parameters
.if (!USE_LARGE_DISK)
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; this function is called to get the parameters of the disk for the lba_to_chs()
           ;  function, specificially the sectors/track and heads parameters
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; use the BIOS to get the SPT and HEADS parameters of the disk
           xor  di,di                   ; es:di -> 0
           mov  es,di                   ;  "to guard against BIOS bugs"
           mov  ah,08h                  ; get disk parameters
         ; mov  dl,physical_drive       ;
           int  13h                     ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; the above function returns:
           ;  ch = low eight bits of maximum cylinder number
	         ;  cl = maximum sector number (bits 5-0)
           ;     = high two bits of maximum cylinder number (bits 7-6)
           ;  dh = maximum head number
           mov  secpertrack,cl          ; sectors per track
           inc  dh                      ; heads is zero based
           mov  heads,dh                ; heads
.endif     
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move out of the way, so later we can load
           ;  the partition's boot code to here
           push ds                      ; set es = ds from above
           pop  es                      ; (have to do this here, after we save es above)
           cld
           mov  cx,0200h
           mov  di,cx
           rep                          ; si = 0 from start of code above
            movsb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now jump to newly moved code.  We need cs:ip updated so that
           ;  we can push/ret a valid offset at 'push offset part_walk_done' below
           push  es
           push  offset moved
           retf
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now update the offsets within our code to the newly moved location
           orgnf ($ + 200h)
moved:           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we are using the extended BIOS read services, we will need to
           ; detect that the BIOS supports them
.if (USE_LARGE_DISK)
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; We now check for the BIOS disk extentions.
           ; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
         ; mov  dl,physical_drive       ; dl still = drive_num
           int  13h
           jc   short disk_err
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jnz  short disk_err
.endif     
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Now we need to find an active partition, load it, and jump to it
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; push the offset of the 'didnt_find_active' code so that the
           ;  'ret' on the last recursion will find where to go
           push offset part_walk_done
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; our buffer 'stack' will start at 0x007E00,
           ;    where the first partition already exists
           mov  bx,200h                 ;
           xor  eax,eax                 ; edx:eax = base of disk drive (start with zero)
.if (USE_LARGE_DISK)
           cdq                          ;
.endif
walk_partitions:
           mov  bp,04                   ; four entries
           lea  si,[bx+446]             ; si -> 1st part entry
           call chk_sig                 ; check that the sig at bx+510 == 0xAA55 or 0x55AA
           jnz  short part_done
part_good: 
           movzx cx,byte [si+PART_ENTRY->sys_id]
           jcxz short part_walk_next    ; if SYS_ID = 0, then is an empty partition entry
           cmp  cl,15                   ; if SYS_ID = 15, then is an extendend partition entry, so recurse
           je   short is_extended       ;
           cmp  cl,5                    ; if SYS_ID = 5, then is an extendend partition entry, so recurse
           jne  short not_extended
is_extended:
           ; we need to save:
           ;push bx                      ; save current 512 byte boot block pointer
           ;push bp                      ; save current partition entry pointer
           ;push si                      ; 
           ;push eax                     ; save current lba base
           ;push edx                     ;
           ; however, we also need to save space, so a single pushad will save space
           pushad                       ; save our parameters
           add  bh,02h                  ; move to next position in the 'stack' (add bh,02h = add bx,0200h)
           add  eax,[si+PART_ENTRY->start_lba]  ; add starting sector (LBA) to current base
.if (USE_LARGE_DISK)
           adc  edx,0                   ;
.endif
           call read_sector             ; read it in to es:bx
           jc   short disk_err          ;
           call walk_partitions         ; recurse
           popad                        ; we must have not found an active on yet, 
           jmp  short part_walk_next    ; so go back to last buffer and partition
not_extended:
           test  byte [si+PART_ENTRY->boot_id],80h  ; if it is active and not empty, then we found it
           jnz   short found_active     ; move it to 0x07C00 and be done
part_walk_next:                         ;
           add  si,16                   ; else move to next partition entry and try again
           dec  bp                      ; if we have tried all the partition entries in this boot block 
           jnz  short part_good         ;  go to previous or if this is the original, then quit
part_done: ret                          ; else, return (from recurse) or if last one, 'jump' to next code below

           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; didn't find an active partition, so see if we can read from the next
;  drive number, check the sig, and jump to it.
part_walk_done:
           inc  byte physical_drive     ; move to next drive id (i.e.: if now 80h, try 81h)
           xor  eax,eax                 ; lba 0
.if (USE_LARGE_DISK)
           cdq                          ;
.endif
           xor  bx,bx                   ; 07C0:0000
           call read_sector             ; read it in
           jc   short no_active
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure it has a valid SIG word.
           call chk_sig                 ; check that the sig at bx+510 == 0xAA55 or 0x55AA
           je   short next_good
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; didn't find an active partition
no_active: mov  si,offset s_no_active_part

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; display a string to the screen using the BIOS
           ; On entry:
           ;   ds:si -> ascii string to display (not asciiz)
           ;   direction flag is forward
           ;   uses '.' to indicate end of string
display_string:  
           xor  bx,bx
           mov  ah,0Eh             ; print char service
@@:        lodsb
           push ax
           int  10h                ; output the character
           pop  ax
           cmp  al,'.'
           jne  short @b
           
@@:        hlt
           jmp  short @b
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print disk error string
disk_err:  mov  si,offset diskerrorS
           jmp  short display_string

; found the active partition
; si points to part entry
; edx:eax is base lba
; read it to 0x007C00, and jump to it
found_active:
           add  eax,[si+PART_ENTRY->start_lba] ; starting sector (LBA)
.if (USE_LARGE_DISK)
           adc  edx,0
.endif
           xor  bx,bx                   ; 07C0:0000
           call read_sector             ; read it in
           jc   short disk_err
           
next_good: push es                      ; for retf below
           push bx
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; TPM support
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; check to see if the BIOS supports TPM, and if so, 
           ; have it create a hash for the loaded boot sector
           mov  ax,0BB00h
           int  1Ah
           or   eax,eax
           jnz  short execute_vbr
           cmp  ebx,41504354h          ; 'TCPA'
           jne  short execute_vbr
           cmp  cx,0102h
           jb   short execute_vbr
           
           mov  ax,0BB07h              ; CompactHashLogExtendEvent
           mov  ecx,0200h              ; 512 bytes
           mov  edx,8                  ; PCR = 8
           xor  esi,esi                ; event field = 0
           xor  edi,edi                ; es:edi -> 0x07C00 (es = 7C00h, edi = 0)
           int  1Ah                    ; ebx = 41504354h from above
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
execute_vbr:
           ; make sure dl = disk booted from before we destroy 07C0:0000h
           mov  dl,physical_drive      ; from booted drive
           
           ; restore es:di for PnP aware BIOS'
           les  di,[pnp_address]
           
           ; now jump to the newly loaded code
           retf

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; check that the sig at [bx+510] is either 0xAA55 or 0x55AA
; on entry: bx = start of "sector" to check
;  on exit: zero flag set if found 0xAA55 or 0x55AA
chk_sig    proc near
           cmp  word [bx+510],0AA55h
           je   short @f
           cmp  word [bx+510],055AAh
@@:        ret
chk_sig    endp
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Use the IBM-MS INT 13 Extensions - EXTENDED READ service which allows
;   64-bit LBA's to be used
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
.if (USE_LARGE_DISK)
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in 1 sector using the bios interrupt 13h extended
;  read service. 
; On entry:
;    edx:eax = starting sector in LBA format
;      es:bx = offset to store data, es:offset
; On return:
;    carry = clear if successful
read_sector proc near uses all ds   ; need to save at least bp ax dx si ds
           mov  bp,sp      ; save sp
           
           push edx        ; offset 12 (high dword of lba)
           push eax        ; offset  8 (low dword of lba)
           push es         ; offset  6 (segment address)
           push bx         ; offset  4 (offset address)
           push 01h        ; offset  2 (1 sector to read)
           push 10h        ; offset  0 (16 byte buffer)
           mov  si,sp      ; si -> dap
           mov  ah,42h                  ; extended read
           mov  dl,physical_drive       ; dl = drive
           
           push ss         ; need ds to point to our stack
           pop  ds         ;
           
           int  13h
           
           mov  sp,bp
           ret
read_sector endp

.else    ; (!USE_LARGE_DISK)
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Use the standard BIOS read service which only allows 24-bit LBA's to be used
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in 1 sector using the bios interrupt 13h service
; On entry:
;  edx:eax = starting sector in LBA format (edx ignored)
;  es:bx = offset to read to
; On return:
;    carry = clear if successful
read_sector proc near uses alld  ; save at least eax bx edx
           and  eax,00FFFFFFh
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Note: if edx:eax was more than 00FC000h, we would
           ;  have a problem here.  The BIOS only allows:
           ;    3Fh * 100h * 400h sectors (00FC0000h)
           ; However, since the drive doesn't support the extended
           ;  read service (42h), it most likely doesn't have more
           ;  than that many sectors anyway.  How would you read them?
           ; Also, some, if not most BIOS's will only use the
           ;  lower 4 bits of the head value.  Therefore, will only
           ;  allow:
           ;    3Fh * 10h * 400h sectors (000FC000h)
           ; Some BIOS's allow 12 bit cylinder values by placing 
           ;  bits 11:10 in bits 7:6 of DH.
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine converts LBA (EAX) to CHS
; Sector   = (LBA mod SPT)+1
; Head     = (LBA  /  SPT) mod Heads
; Cylinder = (LBA  /  SPT)  /  Heads
;    (SPT = Sectors per Track)
; on entry:
;  eax = LBA
; on return
;  DX and CX = BIOS formatted CHS values
;   cl = sector (1 based) (lower 6 bits)
;   ch = cyl (high 2 bits of cl also)
;   dh = head
; assumptions:
;   eax <= 0x7FFFFFFF (i.e.: bit 31 = clear)
;   (0x7FFFFFFF = 2,147,483,647)
;   If we are having to use this service, LBA will definately
;    be less than this.
           cdq
           ; edx:eax = LBA
           
           movzx ecx,byte secpertrack ; sectors per track
           div  ecx                ; eax = edx:eax / ecx  with edx = remdr.
           ; eax = (LBA / SPT)
           ; edx = remainder (zero based sector)
           
           push dx                 ; save zero based sector
           
           cdq
           ; edx:eax = (LBA / SPT)
           
           mov  cl,heads           ; heads per cylinder
           div  ecx                ; eax = edx:eax / ecx  with edx = remdr.
           ; eax = cylinder
           ; edx = head
           
           mov  dh,dl              ; save head number in dh
           ; dh = head
           
           pop  cx                 ; cx = zero based sector
           inc  cx                 ; sectors are one (1) based
           ; cx = 1 based sector
           
           ; eax = cylinder
           mov  ch,al
           shl  ah,6
           or   cl,ah
           ; cx = LLLLLLLLHHSSSSSS  (L = low cyl 8 bits, H = high 2 bits, S = 1 based sector number)
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Some BIOS's may not return the carry set if an error
           ;  occured.  However, it will clear it if no error was
           ;  found.  Therefore, make sure the carry is set before-hand.
           stc
           
           mov  dl,physical_drive       ; dl = drive
           mov  ax,0201h                ; read 1 sector
           int  13h                     ; do the read/write
           
           ret
read_sector endp

.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string data
diskerrorS        db 13,10,'Error reading from the disk.'
s_no_active_part  db 13,10,'Did not find active partition.'

; at assembly time, this will print how many free bytes we have remaining.
%PRINT (400h-$-4-2-(4*16)-2)  ; 12/1 byte(s) free in this area

; disk identifier and partition entry(s)
           org (400h-4-2-(4*16)-2)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; modern OS's write a 32-bit signature here to help
           ; identify the media device.  The 16-bits that follow
           ; might be used on some systems to indicate copy-protection.
           dd  ?              ; identifier written at FDISK time
           dw  0              ; reserved
           
           dup (16*4),0       ; four 16-byte partitions
           
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; since these items do not need to be pre-initialized and we need
;  all of the space we can get, we place them at the end of the
;  code 
pnp_address     dup 4,?    ; save pnp address (needs to remain = 0 for code below to work)
physical_drive  dup 1,?    ; physical disk booted from
.if (!USE_LARGE_DISK)
heads           dup 1,?    ; heads per cylinder
secpertrack     dup 1,?    ; sectors per track
.endif

.end
