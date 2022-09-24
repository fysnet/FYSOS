 ;
 ;                             Copyright (c) 1984-2022
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
 ;  mbr.asm
 ; Notes:
 ; MBR Boot.  This is the boot sector that gets placed in the
 ;   first sector of the actual, physical disk.
 ; This boot code does the following:
 ;  - moves itself to 0:7C00h+200h (just passed this sector)
 ;  - Finds active partition, will search extended partitions
 ;  - Loads and executes active partition
 ;  - If this code does not find an active partition, it checks
 ;    the next BIOS disk number for a valid disk, then loads that
 ;    MBR and jumps to it (if a valid sig of 0xAA55 or 0x55AA is found).
 ;
 ; Assumptions:
 ;  - When USE_LARGE_DISK is set, this code will assume that the
 ;    IBM-MS INT 13 Extensions - EXTENDED READ function is available.
 ;    Since the utility that will write this MBR to the disk, should know
 ;    whether it does or not, there is no reason to check for it.  The
 ;    only draw-back would be if the user pulls this disk from one machine
 ;    and places it into another that does not support this function.
 ;    Make sure to clear USE_LARGE_DISK when you only want the standard
 ;    BIOS disk read service (02h) to be used.
 ;  - This code assumes a 32-bit 80x386 compatible processor starting in
 ;    real mode. Most of the time, MBR's should not assume 32-bit code.
 ;    However, without the 32-bit code, the 32-bit LBA values would be a
 ;    lot more work and code.  It will be up to you if you assume 32-bit
 ;    or not.
 ;
 ; On entry to this boot code:
 ;     cs:ip = usually 0000:7C00h or 07C0:0000h but can be different
 ;        dl = boot drive number
 ;
 ; This code also assumes you will not be on a floppy disk.
 ;
 ;  Last updated: 24 Sept 2022
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;   nbasm mbr
 ;

outfile 'mbr.bin'    ; name of the file to create

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
.386
           ; seg = 07C0h and offset 0000h, we can read in 126 MBR's on our stack.
           org  00h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; our memory setup is as such:
           ; 0x00600 - 0x07DFF: program stack (SS:SP)
           ; 0x07C00 - 0x07DFF: current MBR code
           ; 0x07E00 - 0x07FFF: current MBR after move out of the way
           ; 0x08000 - 0x17BFF: 126 512-byte partition boot sectors (our partition stack)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set up the stack and seg registers (real mode)
           mov  ax,07C0h                ; set our stack, DS, ES
           mov  ds,ax                   ;
           xor  ax,ax                   ;
           mov  ss,ax                   ; first push at 0x07BFE
           mov  sp,7C00h                ; ss = 0x0000, sp = 0x7C00
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the BIOS is PnP aware, it will pass the PnP address
           ;  in es:di.  We don't use it in this MBR, but we pass
           ;  it on to the boot partition incase it does.
           mov  [pnp_address+0],di
           mov  [pnp_address+2],es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move out of the way, so later we can load
           ;  the partition's boot code to here
           push ds
           pop  es                      ; set es = ds
           cld
           xor  si,si
           mov  cx,200h
           mov  di,cx
           rep
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
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Check to see if the Ctrl Key is pressed.  If so,
           ; change dl to 80h
moved:     push es
           xor  ax,ax
           mov  es,ax
           mov  al,es:[0417h]
           pop  es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if bit 2 in al is set, the ctrl key is pressed
           test al,(1 << 2)
           jz   short @f
           mov  dl,80h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; BIOS sets dl to drive id when calling bootsector
           ; also, reset the drive service
@@:        mov  physical_drive,dl       ; make sure we don't mess up DL above
           xor  ah,ah                   ;
           int  13h                     ; reset disk system
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we are using the standard BIOS read service, we will need to
           ; get the disk's parameters
.if (!USE_LARGE_DISK)
           call get_parameters          ; get the disk's parameters
.endif
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Now we need to find an active partition, load it, and jump to it
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; push the offset of the 'didnt_find_active' code so that the
           ;  'ret' on the last recursion will find where to go
           push offset part_walk_done
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; our buffer 'stack' will start at 0x007E00,
           ;    where the first partition table already exists
           mov  bx,200h                 ;
           xor  eax,eax                 ; edx:eax = base of disk drive (start with zero)
           xor  edx,edx                 ;
walk_partitions:
           xor  bp,bp                   ; bp = 1st, 2nd, 3rd, or 4th part entry (0, 16, 32, or 48 respectively)
           lea  si,[bx+446]             ; si -> 1st part entry
           cmp  word [bx+510],0AA55h    ; check the signature word of this partition table
           je   short part_good         ;
           cmp  word [bx+510],055AAh    ;
           jne  short part_done         ;
part_good: cmp  byte ds:[si+bp+PART_ENTRY->sys_id],0 ; if SYS_ID = 0, then is an empty partition entry
           je   short part_walk_next    ;
.if (USE_LARGE_DISK)
           cmp  byte ds:[si+bp+PART_ENTRY->sys_id],15 ; if SYS_ID = 15, then is an extendend partition entry, so recurse
.else
           cmp  byte ds:[si+bp+PART_ENTRY->sys_id],5 ; if SYS_ID = 5, then is an extendend partition entry, so recurse
.endif
           jne  short not_extended
           push bx                      ; save current 512 byte boot block pointer
           push bp                      ; save current partition entry pointer
           push si                      ; 
           add  bx,200h                 ; move to next position in the 'stack'
           push eax                     ; save current lba base
           push edx                     ;
           add  eax,ds:[si+bp+PART_ENTRY->start_lba]  ; add starting sector (LBA) to current base
           adc  edx,0                   ;
           call read_sector             ; read it in to es:bx
           jc   short disk_err          ;
           call walk_partitions         ; recurse
           pop  edx                     ; restore current lba base
           pop  eax                     ;
           pop  si                      ;
           pop  bp                      ; we must have not found an active one yet, 
           pop  bx                      ;  so go back to last buffer and partition
           jmp  short part_walk_next    ;
not_extended:
           test  byte ds:[si+bp+PART_ENTRY->boot_id],80h  ; if it is active and not empty, then we found it
           jnz   short found_active     ; move it to 0x07C00 and be done
part_walk_next:                         ;
           add  bp,16                   ; else move to next partition entry and try again
           cmp  bp,(16*4)               ; if we have tried all the partition entries in this boot block
           jb   short part_good         ;  go to previous or if this is the original, then quit
part_done: ret                          ; else, return (from recurse) or if last one, 'jump' to next code below
                                        ; 

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; didn't find an active partition, so see if we can read from the next
;  drive number, check the sig, and jump to it.
part_walk_done:
           inc  byte physical_drive     ; move to next drive id (i.e.: if now 80h, try 81h)
           xor  eax,eax                 ; lba 0
           xor  edx,edx                 ;
           xor  bx,bx                   ; 07C0:0000
           call read_sector             ; read it in
           jc   short no_active
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure it has a valid SIG word.
           cmp  word [bx+510],0AA55h
           je   short next_good
           cmp  word [bx+510],055AAh
           je   short next_good

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; didn't find an active partition
no_active: mov  si,offset s_no_active_part
print_it:  call display_string
hang:      jmp  short hang

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print disk error string
disk_err:  mov  si,offset diskerrorS
           jmp  short print_it

; found the active partition
; si+bp points to part entry
; edx:eax is base lba
; read it to 0x007C00, and jump to it
found_active:
           add  eax,ds:[si+bp+PART_ENTRY->start_lba] ; starting sector (LBA)
           adc  edx,0
           xor  bx,bx                   ; 07C0:0000
           call read_sector             ; read it in
           jc   short disk_err
           
next_good: push es                      ; for retf below
           push bx
           
           ; make sure dl = disk booted from before we destroy 07C0:0000h
           mov  dl,physical_drive  ; from booted drive
           
           ; restore es:di for PnP aware BIOS'
           mov  di,[pnp_address+0]
           mov  es,[pnp_address+2]
           
           ; now jump to the newly loaded code
           retf
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Use the IBM-MS INT 13 Extensions - EXTENDED READ service which allows
;   64-bit LBA's to be used
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
.if USE_LARGE_DISK

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in 1 sector using the bios interrupt 13h extended
;  read service. 
; On entry:
;    edx:eax = starting sector in LBA format
;      es:bx = offset to store data, es:offset
; On return:
;    carry = clear if successful
read_sector proc near uses eax bx edx si

           mov  si,offset read_packet
           mov  word [si],0010h         ; [si+0] = 10h, [si+1] = 0
           mov  word [si+02h],1         ; size ( < 7F) (byte0 = count, byte1 = 0)
           mov  [si+04h],bx             ; es:bx
           mov  [si+06h],es             ;
           mov  [si+08h],eax            ; lba to read
           mov  [si+0Ch],edx            ;
           
           mov  ah,42h                  ; extended read
           mov  dl,physical_drive       ; dl = drive
           int  13h
           
           ret
read_sector endp

.else
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
read_sector proc near uses eax bx edx
           xor  edx,edx                 ; this service only allows 24-bit LBA's
           and  eax,00FFFFFFh           ;

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

           call lba_to_chs              ; edx:eax(lba) -> (ax/cx/dx)(chs)
           
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

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this function is called to get the parameters of the disk for the lba_to_chs()
;  below, specificially the sectors/track and heads parameters
get_parameters proc near uses es di     ; this service destroys es:di
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; use the BIOS to get the SPT and HEADS parameters of the disk
           xor  di,di                   ; es:di -> 0
           mov  es,di                   ;  "to guard against BIOS bugs"
           mov  ah,08h                  ; get disk parameters
           mov  dl,physical_drive       ;
           int  13h                     ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; the above function returns:
           ;  ch = low eight bits of maximum cylinder number
	         ;  cl = maximum sector number (bits 5-0)
           ;     = high two bits of maximum cylinder number (bits 7-6)
           ;  dh = maximum head number
           and  cl,3Fh                  ; 
           mov  num_sect_track,cl       ; sectors per track
           inc  dh                      ; heads is zero based
           mov  num_heads,dh            ; heads
           
           ret
get_parameters endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
num_sect_track  db 0
num_heads       db 0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine converts LBA (edx:eax) to CHS (edx assumed zero)
; Sector   = (LBA mod SPT)+1
; Head     = (LBA  /  SPT) mod Heads
; Cylinder = (LBA  /  SPT)  /  Heads
;    (SPT = Sectors per Track)
lba_to_chs proc near
           ; (LBA / SPT)
           movzx ecx,byte num_sect_track ; sectors per track 
           div  ecx                     ; edx:eax = edx:eax / ecx with edx = remdr.
           push dx                      ; save sector

           ; (result / Heads)           ;
           xor  edx,edx                 ; 
           mov  cl,num_heads            ; heads per cylinder (high part of ecx = zero from above)
           div  ecx                     ; edx:eax = edx:eax / ecx with ecx = remdr.

           ; restore sector from stack
           pop  cx                      ; cx = sector
           inc  cx                      ; sectors are one (1) based
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; current register values:
           ; cx = sector (1 based)
           ; dx = head
           ; ax = cyl
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we need:
           ;  ch = low eight bits of cylinder number
           ;  cl = sector number 1-63 (bits 0-5)
           ;     = high two bits of cylinder (bits 6-7, hard disk only)
           ;  dh = head number
           mov  dh,dl                   ; dh = head
           mov  ch,al                   ; ch = low 8 bits of cyl
           shl  ah,6                    ; set hi 2 bits of cyl in cl
           or   cl,ah                   ;
           
           ret
lba_to_chs endp

.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
; (destroys all registers used)
display_string proc near
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           cld
display_loop:                      ; ds:si = asciiz message
           lodsb
           or   al,al
           jz   short end_string
           int  10h                ; output the character
           jmp  short display_loop
end_string: ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string data
diskerrorS        db 13,10,'There was an error reading from the disk.',0
s_no_active_part  db 13,10,'Did not find an active partition.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; data
physical_drive  db 0                    ; physical disk booted from
pnp_address     dw 0,0                  ; di then es

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we start to run out of room here, we can point 'read_packet' to
;  any mem value below 0x07C00.  However, if we point it to 0x07C00
;  we can leave DS: as is.
.if USE_LARGE_DISK
read_packet     dup 32,0                ; read packet
.endif

; at assembly time, this will print how many free bytes we have remaining.
;%PRINT (400h-$-(4*16)-2-4-2)  ; 23 bytes free in this area (USE_LARGE_DISK == 1)
;                              ;  2 bytes free in this area (USE_LARGE_DISK == 0)

; disk identifier and partition entry(s)
           org (400h-2-(4*16)-2-4)
           
           dd  0xDEADBEEF     ; identifier written at FDISK time
           dw  0              ; reserved
           
           ; 1st partition entry
           ; This assumes you have a hard disk (image) that is 204624 sectors
           ;  in size (100meg) and that the first 'partition' starts at LBA 1, the
           ;  second sector of the first cylinder on the first head.
           ; This ready for an eMBR partition explained in Chapter 3
           db  80h            ; identifier (80h = active)
           db  0,2,0          ; chs (first Cyl, first Head, second Sector) (LBA 1)
           db  0E0h           ; partition type (FYSOS eMBR)
           db  0FFh,0FFh,0FEh ; ending CHS
           dd  1              ; starting LBA
           dd  (204624-1)     ; size in sectors
           
           ; remaining 3 partition entry's are empty
           dup (16*3),0       ; three 16-byte partitions
           
           dw  0AA55h
.end
