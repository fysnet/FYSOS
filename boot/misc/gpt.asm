comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: gpt.asm                                                            *
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
*   Find and boot an EFI Partition                                         *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm gpt<enter>                                  *
*                                                                          *
* Last Updated: 23 Dec 2018                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* GPT Boot.  This is the boot sector that gets placed in the               *
*   first sector of the actual, physical disk.                             *
*                                                                          *
* A GPT Partitioned disk is usually booted from an EFI BIOS.  However,     *
*  we write this so that we can boot from it from a Legacy type BIOS.      *
* Normally, we would have a standard MBR in the first sector, allowing     *
*  us to boot one of the Partition Entries.  However, this only allows     *
*  us to see up to four entries.  This code assumes the MBR is a Protected *
*  MBR and has the System ID of 0xEE.  Then all partitions are seen.       *
*                                                                          *
*                                                                          *
* This boot code does the following:                                       *
*  - moves itself to 0:7C00h+200h (just passed this sector)                *
*  - reads in the GPT header at LBA 1                                      *
*     (which also reads in the remaining code we store after this header)  *
*  - Displays all partitions found                                         *
*  - Waits for a keypress from the user, then                              *
*     loads and executes that partition                                    *
*                                                                          *
* Supports:                                                                *
*  - 64-bit LBAs                                                           *
*  - Entry's on any starting LBA (not the normal LBA 2)                    *
*  - Checks that the Header CRC is correct.                                *
*    Does not check the Entries CRC since there might be more than         *
*     the number we support.                                               *
*                                                                          *
* This boot code uses two sectors of code and data.  This is okay          *
*  since the entries must be no less than LBA 2 *and* the header at        *
*  LBA 1 only uses the first 92 bytes of the sector.  This allows us       *
*  to use the remaining 420 bytes for code and data.                       *
* If more code or data is needed, we could use more, but then we would     *
*  have to make sure the GPT partitioned device had the entries start      *
*  at an LBA greater than 2, which is allowed, but not normal.             *
*                                                                          *
* Assumptions:                                                             *
*  - Assumes that the IBM-MS INT 13 Extensions - EXTENDED READ function is *
*      available.                                                          *
*  - Assumes a 32-bit 80x386 compatible processor starting in real mode.   *
*                                                                          *
* On entry to this boot code:                                              *
*     cs:ip = usually 0000:7C00h or 07C0:0000h but can be different        *
*        dl = boot drive number                                            *
*                                                                          *
***************************************************************************|

outfile 'gpt.bin'    ; name the file to create

.model tiny

; to save room for code, we re-use the memory area at 0x7C00 (now at 0x7E00)
;  for data area since we have already executed that code.
PHYSICAL_DRIVE  equ  [0200h]   ; byte
;  a byte is available here
PNP_ADDRESS_DI  equ  [0202h]   ; word
PNP_ADDRESS_ES  equ  [0204h]   ; word

MAX_ENTRIES    equ  10   ; we allow up to this many entries (because we only allow single digit selections, '0' -> '9')

HEADER_OFFSET  equ  0400h   ; 0x7C00 + 0x400 = 0x8000
ENTRY_OFFSET   equ  0600h   ; 0x7C00 + 0x600 = 0x8200

S_GPT_HDR struct
  sig           dup 8  ; signature
  version       dword  ; version
  hdr_size      dword  ; size of header (92)
  crc32         dword  ; crc of header  ; only bytes 0 -> hdr_size are checked
  resv          dword  ; reserved
  primary_lba   qword  ; this LBA
  backup_lba    qword  ; other header's LBA
  first_usable  qword  ; first usable LBA
  last_usable   qword  ; last usable LBA
  guid          dup 16 ; GUID
  entry_offset  qword  ; LBA of start of entries
  entries       dword  ; count of entries
  entry_size    dword  ; entry size (128)
  crc32_entries dword  ; crc of entries
S_GPT_HDR ends

ATTRIB_REQUIRED  equ  (1     )  ; (1 << 0) nbasm gives a warning on shift by zero
ATTRIB_NO_BLOCK  equ  (1 << 1)
ATTRIB_LEGACY    equ  (1 << 2)

S_GPT_ENTRY struct
  guid_type     dup 16 ; GUID type
  guid          dup 16 ; GUID
  first_lba     qword  ; starting LBA of partition
  last_lba      qword  ; last LBA of partition
  attribute     qword  ; attribute bitmap of partition
  name          dup (36*2) ;  16-bit UTF name
S_GPT_ENTRY ends

.code
.386
           ; seg = 07C0h and offset 0000h, we can read in 127 MBR's on our stack.
           org  00h

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set up the stack and seg registers (real mode)
           mov  ax,07C0h                ; set our stack, DS, ES
           mov  ds,ax                   ;
           mov  ss,ax                   ;
           xor  sp,sp                   ; first push at 07C0:FFFEh
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the BIOS is PnP aware, it will pass the PnP address
           ;  in es:di.  We don't use it in this MBR, but we pass
           ;  it on to the boot partition incase it does.
           mov  PNP_ADDRESS_DI,di
           mov  PNP_ADDRESS_ES,es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move out of the way, so later we can load
           ;  the partition's boot code to here
           mov  es,ax                   ; set es = ds from above
           cld
           xor  si,si
           mov  cx,0200h
           mov  di,cx
           rep
            movsb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now jump to newly moved code.
           jmp   ($ + 200h + 3)   ; 200h from here + size of this instruction
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now update the offsets within our code to the newly moved location
           orgnf ($ + 200h)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; BIOS sets dl to drive id when calling bootsector
moved:     mov  PHYSICAL_DRIVE,dl       ; make sure we don't mess up DL above
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Now we need to find an active partition, load it, and jump to it
; To do that, we load the header (LBA 1) and the entries (LBA x), check
;  the GPT header and get the count of entries.  We then check the
;  entries for the EFI System Partition Flag and boot the first one
;  we find.
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Read in the header
           ;  (which will read in the remaining code/data as well)
           xor  eax,eax                 ; edx:eax = base of disk drive
           cdq                          ;
           inc  eax                     ; LBA 1
           mov  cx,ax                   ; 1 header
           mov  bx,HEADER_OFFSET        ; es:HEADER_OFFSET is just after this code
           call read_sectors            ; read it in to es:bx
           jc   disk_err                ;
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; now we can use any code and data that is in the second sector as well
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Check the header
           mov  si,offset gpt_header_error  ; si-> error string before hand
           call gpt_check_header
           jc   printit
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; And now read in the max allowed entries we support
           mov  eax,[bx+S_GPT_HDR->entry_offset + 0]
           mov  edx,[bx+S_GPT_HDR->entry_offset + 4]
           mov  cx,((MAX_ENTRIES + 3) / 4) ; x sectors. ex: 16 entries = (4 per sector * 4 = 16)
           mov  bx,ENTRY_OFFSET         ; es:ENTRY_OFFSET is just after the header above
           call read_sectors            ; read it in to es:bx
           jc   disk_err                ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; clr the screen and print main string
           mov  ax,0003h
           int  10h
           mov  si,offset main_string
           call display_string
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; scroll through the entries printing the information
           mov  si,HEADER_OFFSET        ; point to the header
           mov  ecx,[si+S_GPT_HDR->entries]  ; is little endian, so will read in the correct part
           cmp  ecx,MAX_ENTRIES         ; we only allow up to MAX_ENTRIES entries
           jbe  short @f
           mov  cx,MAX_ENTRIES
@@:        ;mov  bx,ENTRY_OFFSET
           xor  dx,dx  ; current partition count
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; loop through the entries
part_loop: mov  eax,[bx+S_GPT_ENTRY->first_lba + 0]   ; low dword
           mov  edi,[bx+S_GPT_ENTRY->first_lba + 4]   ; high dword
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; is it outside the range?
           ; is the low end before the start of the GPT allowed?
           cmp  edi,[si+S_GPT_HDR->first_usable + 4]
           jb   short next_part
           ja   short @f
           cmp  eax,[si+S_GPT_HDR->first_usable + 0]
           jb   short next_part
           
           ; is the high end past the end of the GPT allowed?
@@:        cmp  edi,[si+S_GPT_HDR->last_usable + 4]
           ja   short next_part
           jb   short disp_part
           cmp  eax,[si+S_GPT_HDR->last_usable + 0]
           ja   short next_part
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; display the number and the name
disp_part: pusha
           add  dl,'0'
           mov  line_index,dl
           mov  si,offset line_string
           call display_string
           
           ; display the attribute(s)
           mov  ecx,[bx+S_GPT_ENTRY->attribute + 0]
           ;mov  edx,[bx+S_GPT_ENTRY->attribute + 4]
           mov  al,' '
           test ecx,ATTRIB_REQUIRED
           jz   short @f
           mov  al,'R'
@@:        call display_char
           mov  al,' '
           test ecx,ATTRIB_NO_BLOCK
           jz   short @f
           mov  al,'H'
@@:        call display_char
           mov  al,' '
           test ecx,ATTRIB_LEGACY
           jz   short @f
           mov  al,'L'
@@:        call display_char
           mov  si,offset endattrb_str
           call display_string
           
           ; display the name
           lea  si,[bx+S_GPT_ENTRY->name]
           call display_wstring
           
           popa
           inc  dx       ; count of entries
           
next_part: add  bx,sizeof(S_GPT_ENTRY)
           loop part_loop
           
           mov  si,offset attribs_string
           call display_string
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; allow the user to choose an entry
chooseit:  mov  si,offset choose_string
           call display_string
           
           ; get a key from the (BIOS) keyboard
           xor  ah,ah
           int  16h
           
           ; convert to zero-based integer index
           ; this is a quick trick but only allows 0 -> 9 (a max of 10 entries)
           and  ax,000Fh                ; '0' = 0x0B30 & 0x000F = 0x0000
           
           ; is it within range?
           cmp  ax,dx                   ; '0' = 0x0B30 & 0x000F = 0x0000,
           jae  short chooseit          ; '1' = 0x0B31 & 0x000F = 0x0001, etc
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; convert index to point to indexed entry
           ;  and then read in the first sector of that partition
           shl  ax,7
           add  ax,ENTRY_OFFSET
           mov  bx,ax
           mov  eax,[bx+S_GPT_ENTRY->first_lba + 0]
           mov  edx,[bx+S_GPT_ENTRY->first_lba + 4]
           xor  bx,bx                   ; 07C0:0000
           mov  cx,1
           call read_sectors            ; read it in
           jc   short disk_err
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure it has a valid SIG word.
           cmp  word [bx+510],0AA55h
           je   short part_good
           cmp  word [bx+510],055AAh
           je   short part_good
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; was not a bootable partition
           mov  si,offset bad_partitionS
           jmp  short printit
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; is a bootable partition
part_good: push es   ; for retf below
           push bx
           
           ; make sure dl = disk booted from before we destroy 07C0:0000h
           mov  dl,PHYSICAL_DRIVE  ; from booted drive
           
           ; restore es:di for PnP aware BIOS'
           les  di,PNP_ADDRESS_DI
           
           ; now jump to the newly loaded code
           retf
           
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print disk error string
disk_err:  mov  si,offset diskerrorS
printit:   call display_string
@@:        sti
           hlt
           jmp  short @b
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in cx sector(s) using the bios interrupt 13h extended
;  read service.
; On entry:
;    edx:eax = starting sector in LBA format
;         cx = count of sectors to read (must be 0x7F (127d) or less)
;      es:bx = offset to store data, es:offset
; On return:
;    carry = clear if successful
; *** must stay in first sector of code ***
read_sectors proc near uses alld
           push edx        ; offset 12
           push eax        ; offset 8
           push es         ; offset 6
           push bx         ; offset 4
           push cx         ; offset 2
           push 10h        ; offset 0
           mov  si,sp      ; this assumes ds==ss (which it should)
           mov  ah,42h                  ; extended read
           mov  dl,PHYSICAL_DRIVE       ; dl = drive
           int  13h
           pushf           ; save the carry flag
           add  sp,16      ; remove the items from the stack
           popf            ;  since the add above will modify it
           ret
read_sectors endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a char to the screen using the BIOS
; *** must stay in first sector of code ***
display_char proc near uses bx  ; ax
           mov  ah,0Eh
           xor  bx,bx
           int  10h
           ret
display_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen
; *** must stay in first sector of code ***
display_string proc near uses ax
           cld
@@:        lodsb               ; ds:si = w_char message
           or   al,al
           jz   short @f
           call display_char   ; output the character
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string data
; the following must stay in the first sector of this code
diskerrorS        db 13,10,'Error reading from the disk.',0

; the remaining may be here or in the second sector
bad_partitionS    db 13,10,"Didn't find 0xAA55 sig.",0

; at assembly time, this will print how many free bytes we have remaining.
%PRINT (HEADER_OFFSET-$-(4*16)-2-4-2)  ; 3 bytes free in this area

; disk identifier and partition entry(s)
           org (HEADER_OFFSET-2-(4*16)-2-4)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; modern OS's write a 32-bit signature here to help
           ; identify the media device.  The following 16-bits
           ; might be used on some systems to indicate copy-protection.
           dd  0              ; identifier written at FDISK time
           dw  0              ; reserved
           
           ; first partition in an Protected GPT MBR is used
           db  0     ; Boot ID: 0x00
           db  0     ; Start: Head: 0x00
           db  1     ; Sector: 0x01
           db  0     ; Cylinder: 0x00
           db  0EEh  ; System ID: 0xEE  = EFI GPT
           db  0FEh  ; End: Head: 0xFE
           db  0FFh  ; Sector: 0xFF
           db  0FFh  ; Cylinder: 0xFF
           dd  1     ; Start LBA: 0x00000001 (1)
           dd  0     ; Size of disk in sectors
           
           ; remaining are zero'd
           dup (16*3),0       ; 3 more 16-byte partitions
           
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this should be at es:HEADER_OFFSET (0x07C00 + HEADER_OFFSET)
.ifne $ HEADER_OFFSET
  %error 1 'We should be at 0x08000 right here...'
.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is where the GPT header is loaded to, as well as stored on the disk
;  (LBA 1)
; since it is only 92 bytes, we use the remaining 420 bytes for code and data.

gpt_header   st  S_GPT_HDR

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  now we can use 420 bytes of code and data as long as any of this code
;   and data is not accessed until *after* we read in the header above.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a wide_char string to the screen
display_wstring proc near uses ax
           cld
@@:        lodsw               ; ds:si = w_char message
           or   ax,ax
           jz   short @f
           call display_char   ; output the character (assumes ascii in al)
           jmp  short @b
@@:        ret
display_wstring endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; check the GPT header to make sure it is valid
;  return carry clear if valid header
gpt_check_header proc near uses alld es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; check the signature
           mov  esi,HEADER_OFFSET        ; es:HEADER_OFFSET is just after this code
           cmp  dword [esi + S_GPT_HDR->sig + 0],20494645h
           jne  short bad_hdr
           cmp  dword [esi + S_GPT_HDR->sig + 4],54524150h
           jne  short bad_hdr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; initialize the CRC table
           ; we need 256 dwords (1024 bytes) in space for the table
           ; we should be find using physical address 0x07000 (0700:0000)
           mov  ax,0700h
           mov  es,ax
           call crc32_initialize
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the crc
           mov  edx,[esi + S_GPT_HDR->crc32]
           mov  dword [esi + S_GPT_HDR->crc32],0
           mov  ecx,[esi + S_GPT_HDR->hdr_size]
           call crc32
           cmp  edx,eax
           jne  short bad_hdr
           
           clc
           ret
           
bad_hdr:   stc           
           ret
gpt_check_header endp


CRC32_POLYNOMIAL  equ  04C11DB7h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; initialize 256 dwords to our CRC table
; passed address in es:0000
crc32_initialize proc near
  
           xor  ecx,ecx
           xor  edi,edi
init_loop: push ecx
           mov  eax,8           ;
           xchg ecx,eax         ;
           call crc32_reflect   ; crc32_table[i] = crc32_reflect(i, 8) << 24;
           shl  eax,24          ;
           
           mov  cx,8
poly_loop: shl  eax,1           ;  crc32_table[i] = (crc32_table[i] << 1) ^ 
           jnc  short @f        ;    ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
           xor  eax,CRC32_POLYNOMIAL
@@:        loop poly_loop       ;
           
           mov  ecx,32          ;
           call crc32_reflect   ; crc32_table[i] = crc32_reflect(crc32_table[i], 32);
           
           stosd                ; crc32_table[i] = eax
           
           pop  ecx             ;
           inc  ecx             ; 256 times
           cmp  ecx,256         ;
           jb   short init_loop
           
           ret
crc32_initialize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;   Swap bit 0 for bit 7 bit 1 For bit 6, etc....
; passed reflect value in eax
; passed ch value in ecx
; returns reflected value in eax
crc32_reflect proc near uses ebx ecx edx
           mov  edx,eax                 ; edx = reflect value
           xor  eax,eax                 ; eax = ret value (0)
           mov  ebx,1                   ; create bitmap bit
           dec  ecx                     ; zero-base the shift
           shl  ebx,cl
reflect_loop:
           shr  edx,1
           jnc  short @f
           or   eax,ebx
@@:        shr  ebx,1
           jnc  reflect_loop            ; go until the shift bit "falls off"
           
           ret
crc32_reflect endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; passed byte count to check in ecx
; passed pointer to data to check in ds:esi
; passed seg value of table in es:0000
; returns crc in eax
crc32      proc near
           mov  eax,0FFFFFFFFh
           call crc32_partial
           xor  eax,0FFFFFFFFh           
           ret
crc32      endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; passed byte count to check in ecx
; passed seg value of table in es:0000
; passed pointer to data to check in ds:esi
; passed current crc in eax
; returns crc in eax
crc32_partial proc near uses edx esi edi
           
@@:        movzx edx,byte [esi] ;
           inc  esi             ;
           mov  edi,eax         ;    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
           and  edi,0FFh        ;
           xor  edi,edx         ;
           mov  edx,es:[edi*4]  ;
           shr  eax,8           ;
           xor  eax,edx         ;
           ;.adsize              ; loop using ecx
           loop @b              ;  (not really needed since we are in real mode and can't have
                                ;    more than 65536 bytes, but just because...)
           ret
crc32_partial endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string data
gpt_header_error  db 13,10,"Didn't find valid GPT Header.",0

main_string       db '-=-=-= UEFI GPT Boot =-=-=-=-',0
line_string       db 13,10,' '
line_index        db '?: [',0
endattrb_str      db '] ',0
attribs_string    db 13,10,'R = Required, H = Hidden, L = Legacy Bootable',0
choose_string     db 13,10,'Please choose a partition: ',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
%PRINT (ENTRY_OFFSET-$)  ; 3 bytes free in this area
           org ENTRY_OFFSET

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this should be at es:ENTRY_OFFSET (0x07C00 + ENTRY_OFFSET)
.ifne $ ENTRY_OFFSET
  %error 1 'We should be at 0x08200 right here...'
.endif

.end
