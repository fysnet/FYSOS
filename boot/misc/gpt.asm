comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
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
* Last Updated: 12 Oct 2016                                                *
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
*  - Displays all partitions found                                         *
*  - Waits for a keypress from the user, then                              *
*     loads and executes that partition                                    *
*                                                                          *
* Assumptions:                                                             *
*  - Assumes that the IBM-MS INT 13 Extensions - EXTENDED READ function is *
*      available.                                                          *
*  - Assumes a 32-bit 80x386 compatible processor starting in real mode.   *
*  - Assumes that the entries follow the header, header at                 *
*    LBA 1, with the entries start at LBA 2                                *
*  - Assumes all CRCs are correct.  i.e.: Header and Entries are valid.    *
*  - Assumes all LBAs are 32-bit.  i.e.: No 64-bit addresses...            *
*                                                                          *
* On entry to this boot code:                                              *
*     cs:ip = usually 0000:7C00h or 07C0:0000h but can be different        *
*        dl = boot drive number                                            *
*                                                                          *
***************************************************************************|

outfile 'gpt.bin'    ; name the file to create

.model tiny

MAX_ENTRIES    equ  16   ; we allow up to 16 entries

HEADER_OFFSET  equ  0400h
ENTRY_OFFSET   equ  0600h

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
           xor  ax,ax                   ;
           mov  ss,ax                   ; first push at 0x07BFE
           mov  sp,7C00h                ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if the BIOS is PnP aware, it will pass the PnP address
           ;  in es:di.  We don't use it in this MBR, but we pass
           ;  it on to the boot partition incase it does.
           mov  [pnp_address+0],di
           mov  [pnp_address+2],es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now move out of the way, so later we can load
           ;  the partition's boot code to here
           mov  ax,ds
           mov  es,ax                   ; set es = ds from above
           cld
           xor  si,si
           mov  cx,200h
           mov  di,cx
           rep
            movsb
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Now jump to newly moved code.
           push  es
           push  offset moved
           retf
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now update the offsets within our code to the newly moved location
           orgnf ($ + 200h)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; BIOS sets dl to drive id when calling bootsector
moved:     mov  physical_drive,dl       ; make sure we don't mess up DL above
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Now we need to find an active partition, load it, and jump to it
; To do that, we load the first 33 sectors, starting at LBA 1, check
;  the GPT header and get the count of entries.  We then check the
;  entries for the EFI System Partition Flag and boot the first one
;  we find.
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Read in the header and a max of 128 entries
           xor  eax,eax                 ; edx:eax = base of disk drive
           cdq                          ;
           inc  eax                     ; LBA 1
           mov  cx,33                   ; 1 header and 32 for 128 entries
           mov  bx,HEADER_OFFSET        ; es:HEADER_OFFSET is just after this code
           call read_sectors            ; read it in to es:bx
           jc   disk_err                ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; clr the screen and print main string
           mov  ax,0003h
           int  10h
           mov  si,offset main_string
           call display_string
           
; scroll through the entries printing the information
           mov  si,bx  ; save so we can use si for the header access
           mov  cx,[si+S_GPT_HDR->entries]  ; is little endian, so will read in the correct part
           cmp  cx,MAX_ENTRIES     ; we only allow up to MAX_ENTRIES entries
           jbe  short @f
           mov  cx,MAX_ENTRIES
@@:        mov  bx,ENTRY_OFFSET
           xor  dx,dx  ; current partition count
part_loop: mov  eax,[bx+S_GPT_ENTRY->first_lba]
           
           ; is it outside the range?
           cmp  eax,[si+S_GPT_HDR->first_usable]
           jb   short next_part
           cmp  eax,[si+S_GPT_HDR->last_usable]
           ja   short next_part
           
           ; display the letter and the name
           pusha
           add  dl,'A'
           mov  line_index,dl
           mov  si,offset line_string
           call display_string
           lea  si,[bx+S_GPT_ENTRY->name]
           call display_wstring
           popa
           inc  dx
           
next_part: add  bx,sizeof(S_GPT_ENTRY)
           loop part_loop
           
chooseit:  mov  si,offset choose_string
           call display_string
           
           xor  ah,ah
           int  16h
           
           and  ax,1Fh   ; 'A' = 0x1E61 & 0x001F = 0x0001
           dec  ax       ; zero based
           cmp  ax,dx
           jnb  short chooseit
           
           shl  ax,7
           add  ax,ENTRY_OFFSET
           mov  bx,ax
           mov  eax,[bx+S_GPT_ENTRY->first_lba]
           xor  edx,edx
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
           mov  dl,physical_drive  ; from booted drive
           
           ; restore es:di for PnP aware BIOS'
           mov  di,[pnp_address+0]
           mov  es,[pnp_address+2]
           
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
read_sectors proc near uses alld
           xor  si,si                   ; may use 0x07C00 for the offset
           mov  word [si],0010h         ; [si+0] = 10h, [si+1] = 0
           mov  [si+02h],cx             ; count of sectors to read
           mov  [si+04h],bx             ; es:bx
           mov  [si+06h],es             ;
           mov  [si+08h],eax            ; lba to read
           mov  [si+0Ch],edx            ;
           mov  ah,42h                  ; extended read
           mov  dl,physical_drive       ; dl = drive
           int  13h
           ret
read_sectors endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a char to the screen using the BIOS
display_char proc near uses ax bx
           mov  ah,0Eh
           xor  bx,bx
           int  10h
           ret
display_char endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen
display_string proc near uses allf
           cld
@@:        lodsb               ; ds:si = asciiz message
           or   al,al
           jz   short @f
           call display_char   ; output the character
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a wide_char string to the screen
display_wstring proc near uses allf
           cld
@@:        lodsw               ; ds:si = w_char message
           or   ax,ax
           jz   short @f
           call display_char   ; output the character (assumes ascii in al)
           jmp  short @b
@@:        ret
display_wstring endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string data
diskerrorS        db 13,10,'Error reading from the disk.',0
bad_partitionS    db 13,10,"Didn't find 0xAA55 sig.",0
main_string       db '-=-=-= UEFI GPT Boot =-=-=-=-',0
line_string       db 13,10,' '
line_index        db 'A'
                  db ': ',0
choose_string     db 13,10,'Please choose a partition: ',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; data
physical_drive  db 0                    ; physical disk booted from
pnp_address     dw 0,0                  ; di then es

; at assembly time, this will print how many free bytes we have remaining.
%PRINT (HEADER_OFFSET-$-(4*16)-2-4-2)  ; 7 bytes free in this area

; disk identifier and partition entry(s)
           org (HEADER_OFFSET-2-(4*16)-2-4)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; modern OS's write a 32-bit signature here to help
           ; identify the media device.  The following 16-bits
           ; might be used on some systems to indicate copy-protection.
           dd  ?              ; identifier written at FDISK time
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
           dd  0E4FF77Fh ; Size of disk in sectors: 0x0E4FF77F (240,121,727)
           
           ; remaining are zero'd
           dup (16*3),0       ; four 16-byte partitions
           
           dw  0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this should be at es:HEADER_OFFSET (0x07C00 + HEADER_OFFSET)
.ifne $ HEADER_OFFSET
  %error 1 'We should be at 0x08000 right here...'
.endif

.end
