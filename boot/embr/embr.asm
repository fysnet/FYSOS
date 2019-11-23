comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: embr.asm                                                           *
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
*   embr boot code                                                         *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm embr -d<enter>                              *
*                                                                          *
* Last Updated: 22 Oct 2018                                                *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* This code and the eMBR technique is copyrighted.  You may use this       *
*  and the eMBR technique as you would like as long as you do not change   *
*  the overall function of the use and you must also include the following *
*  copyright line in all code and documentation pertaining to the use      *
*  of this technique.                                                      *
*                                                                          *
* This code must be assembled with NBASM version 00.26.31 or later.  Any   *
*  version before this will not correctly assemble the 64-bit immediate    *
*  values with the DQ declaration.  Versions before also had a bug in the  *
*  IMUL reg32,reg3,immed instruction.                                      *
*                                                                          *
* This code reads in any remaining code and all of the EMBR entries. Once  *
*  a partition is selected, it is read in to a buffer location, then       *
*  copied to 0x07C00.  We use this technique so that we don't have to move *
*  out of the way.                                                         *
*                                                                          *
* This code assumes that it is on LBA 1.  You should have a valid MBR at   *
*  LBA 0. If you modify this code and re-build it, make sure you place     *
*  a valid MBR at LBA 0 and this code at LBA 1.                            *
*                                                                          *
* This code also assumes you will not be on a floppy disk.                 *
*                                                                          *
***************************************************************************|

.model tiny                        ;
.diag 0

include 'embr.inc'                 ; our include file
outfile 'embr.bin'                 ; declare the out filename

; sector size
; SECT_SIZE   equ  512               ; we are assembling for 512 bytes sectors

; total count of sectors the code and entries occupy
; we will allocate 31 (32 including the MBR) even though we don't
;  use all of them.  This is for future additions
; OCCUPY         equ 32 ; MBR = 1, EMBR = 31

; count of entries in menu window at one time, must be at least 2
; used to know how many entries to display in a given window
TOTAL_DISPLAY  equ  7

; start of our code
.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for now
           org  00h                ; 07C0:0000h (which doesn't mean cs:ip = 07C0:0000h)

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     mov  ax,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,ax              ;
           mov  es,ax              ;
           mov  ax,0600h           ; place stack below our boot code
           mov  ss,ax              ; start of stack segment (0600h)
           mov  sp,1A00h           ; first push at 0600:19FEh (0x079FE)

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  save the dl value
           mov  drive,dl           ; Store drive number
                                   ; (supplied by BIOS startup code)

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           mov  ax,7000h           ; if bits 12,13,14 are still set
           push ax                 ; after pushing/poping to/from
           popf                    ; the flags register then we have
           pushf                   ; a 386+
           pop  ax                 ;
           and  ax,7000h           ;
           cmp  ax,7000h           ;
           je   short @f           ; it's a 386+
           mov  si,offset not_386
           jmp  short display_error

.386       ; we assume we will not be on a floppy disk.
@@:        ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we need to check for the BIOS DISK Extensions.
           mov  ah,41h
           mov  bx,55AAh
           mov  dl,drive
           int  13h
           jc   short @f
           shr  cx,1                    ; carry = bit 0 of cl
           adc  bx,55AAh                ; AA55 + 55AA + carry = 0 if supported
           jz   short read_remaining
@@:        mov  si,offset no_extentions_found
           jmp  short display_error

read_remaining:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  get the size of a sector.  i.e.: bytes per sector
           mov  ah,48h
           mov  dl,drive
           mov  si,offset packet
           mov  byte [si],1Ah
           int  13h
           mov  ax,[si+S_INT13_PARMS->sector_size]
           mov  sector_size,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Now read in the remaining sectors
           xor  edx,edx        ;
           mov  eax,2          ; always at LBA 2
           mov  cx,remaining   ;
           mov  ebx,07E00h     ; physical address 0x07E00
           mov  si,4201h       ; read service
           call transfer_sectors_long
           
           jmp  remaining_code
           
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; display read error
display_error:
           mov  word row,23
           mov  word col 10
           call display_string
           jmp  short $
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;  edx:eax = starting sector in LBA format
;       cx = count of sectors to read/write (doesn't check for > 7Fh)
;      ebx = physical address to store data
; NOTE: This should get called with <= 7Fh sectors to read
transfer_sectors_long proc near uses eax ebx ecx edx si di
           
           push si         ; save service
           mov  si,offset packet
           mov  word [si],0010h
           mov  [si+2],cx

           ; ebx = physical address.  We need
           ;  to convert to a segmented address.
           push bx
           and  bx,0Fh
           mov  [si+04h],bx  ; offset
           pop  bx
           shr  ebx,4
           mov  [si+06h],bx  ; segment
           
           mov  [si+08h],eax
           mov  [si+0Ch],edx
           pop  ax         ; restore service number
           
           mov  dl,drive           ; dl = drive
           int  13h
           mov  si,offset diskerrorS
           jc   short display_error
           
           ret
transfer_sectors_long endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen
display_string proc near uses ax si
           cld                    ; ds:si = asciiz message
@@:        lodsb
           or   al,al
           jz   short @f
           call display_char
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a char to the screen using direct memory access
; note: we don't check for screen scroll
display_char proc near uses ax di
           
           cmp  al,13
           jne  short @f
           mov  word col,0
           ret
           
@@:        cmp  al,10
           jne  short @f
           inc  word row
           ret
           
@@:        imul di,row,80
           add  di,col
           shl  di,1
           
           push es
           push 0B800h
           pop  es
           mov  ah,color
           mov  es:[di],ax
           pop  es
           
           inc  word col
           
           cmp  word col,80
           jb   short @f
           mov  word col,0
           inc  word row
           
@@:        ret
display_char endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; data that must remain in the first sector

color         db 07h
row           dw 0
col           dw 0

drive         db 0

packet        dup 26,0  ; needs to be at least 26 bytes in length

sector_size   dw 0

diskerrorS    db 'Error Reading from or Writing to disk.',0
no_extentions_found db  'Requires int 13h extented read service.',0
not_386       db 'Requires at least a 80x386 processor.',0
bad_crc32     db 'Found Bad crc32: ',0
bad_crc32_0   db '  Should be: ',0


;%print ((200h-8-2-2-2)-$)  ; 40 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  at (0x200-header size)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Pad out to fill 512 bytes, including final word 0xAA55
           org (200h-8-2-2-2)
           
signature  db 'EmbrrbmE'
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; the offset from LBA 0 where entries start
hdr_offset dw  0  ; ((entry_hdr/SECT_SIZE) + 1)  ; hard coded / written to at EMBR build time
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; the count of sectors remaining which include the
           ; code *and* all sectors used for EMBR entries.
remaining  dw  0  ; (OCCUPY-2)      ; hard coded / written to at EMBR build time
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; for compatibility, we still have the AA55h sig at the
           ; end of the sector
           dw  0AA55h
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; remaining code that gets loaded by the above code.  The amount to load
; is in the traditional struct above (remaining)

remaining_code:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure we are in text mode 3
           mov  ax,0003h
           int  10h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; turn off cursor
           mov  ah,01h                  ;
           mov  ch,00100000b            ; bit number 5
           int  10h                     ;

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure the 'blink' bit is off
           ; this is so we can print bright colors instead of the
           ;  vga using bit 7 as the blink bit.
           ; http://scatcat.fhsu.edu/~jplang2/asm-blink.html
           mov  dx,3DAh             ; reset the flip-flop
           in   al,dx
           
           mov  dx,3C0h             ; index 0x10  (20h + 10h)?
           mov  al,30h
           out  dx,al
           
           inc  dx                  ;  clear bit 3 to disable blink
           in   al,dx
           and  al,(~(1<<3))
           dec  dx
           out  dx,al
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the crc32 and see if it is correct
           ; no need to restore the original CRC32 since we will
           ;  calculate a new one when we update the date
           call crc32_initialize

           call get_entry_hdr_offset
           mov  ebx,[di + S_EMBR->crc]
           mov  dword [di + S_EMBR->crc],0

           mov  si,di
           mov  ax,[di + S_EMBR->entry_count]
           mov  cx,sizeof(S_EMBR_ENTRY)
           mul  cx
           mov  cx,ax
           add  cx,sizeof(S_EMBR)
           call crc32
           
           cmp  ebx,eax
           je   short @f
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if CRC was not correct, print error, current CRC value,
           ; and the calculated CRC value.
           mov  word row,23
           mov  word col 10
           mov  si,offset bad_crc32
           call display_string

           push eax

           mov  eax,ebx
           push eax
           shr  eax,16
           call display_hex16
           dec  word col
           pop  eax
           call display_hex16

           mov  si,offset bad_crc32_0
           call display_string

           pop  eax

           push eax
           shr  eax,16
           call display_hex16
           dec  word col
           pop  eax
           call display_hex16
           
           cli
           hlt
           jmp  short $

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the menu text and start getting valid entrys
@@:        mov  si,offset menu_start
           call display_string
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now get the entries.
           mov  word cur_start,0
           call get_entry_hdr_offset
           mov  ax,[di + S_EMBR->entry_count]
           mov  tot_entries,ax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate last booted entry
           mov  word last_boot,0
           xor  ebp,ebp  ; ebp:esi is largest date
           xor  esi,esi
           xor  bx,bx
           add  di,sizeof(S_EMBR)
get_last_boot1:
           mov  eax,[di + S_EMBR_ENTRY->date_last_booted+0]
           mov  edx,[di + S_EMBR_ENTRY->date_last_booted+4]
           cmp  edx,ebp
           jb   short get_last_next
           ja   short update_last_boot
           cmp  eax,esi
           jb   short get_last_next
update_last_boot:
           mov  last_boot,bx
           mov  ebp,edx
           mov  esi,eax
get_last_next:
           inc  bx
           cmp  bx,tot_entries
           je   short get_last_boot_done
           add  di,sizeof(S_EMBR_ENTRY)
           jmp  short get_last_boot1

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the entry to be displayed as selected
get_last_boot_done:
           ; call get_entry_hdr_offset
           mov  ax,last_boot
           mov  cur_selected,ax
           sub  ax,(TOTAL_DISPLAY>>1)
           jns  short @f
           mov  word cur_start,0
           jmp  short again
@@:        mov  bx,tot_entries
           sub  bx,TOTAL_DISPLAY
           cmp  ax,bx
           jbe  short again
           mov  ax,bx
           mov  cur_start,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; our display loop
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; update the scroll bar
again:     call update_scroll_bar
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; display the entries
           mov  word row,3
           mov  word col,7
           mov  word cur_entry,0
           mov  word cur_display,0
           call get_entry_hdr_offset
           add  di,sizeof(S_EMBR)
           
first:     mov  ax,cur_entry
           cmp  ax,cur_start
           jb   next_entry
           
           mov  byte color,07h
           mov  ax,cur_selected
           sub  ax,cur_start
           cmp  ax,cur_display
           jne  short @f
           mov  byte color,70h
           
@@:        ; check to see if we are past the count of entries
           mov  ax,cur_entry
           cmp  ax,tot_entries
           jae  short empty_line
           
           test dword [di + S_EMBR_ENTRY->flags],ENTRY_VALID
           jz   short not_valid
           
           ; check the signature field too
           cmp  dword [di + S_EMBR_ENTRY->signature],52424D65h    ; 'RBMe'
           jne  short not_valid
           
           ; is a valid entry
           mov  si,offset blank_line
           call display_string

           mov  word col,7
           lea  si,[di + S_EMBR_ENTRY->description]
           call display_string
           
           inc  word row         ; move to next row
           mov  word col,7
           
           mov  si,offset blank_line
           call display_string
           mov  word col,7
           mov  si,offset base_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->starting_sector+0]
           mov  edx,[di + S_EMBR_ENTRY->starting_sector+4]
           call display_hex64
           
           mov  si,offset size_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->sector_count+0]
           mov  edx,[di + S_EMBR_ENTRY->sector_count+4]
           call display_hex64
           
           jmp  short next

; no more entries to display, so print an empty line/entry
empty_line:
           mov  word col,7
           mov  si,offset blank_line
           call display_string
           inc  word row
           mov  word col,7
           call display_string
           jmp  short next
           
not_valid: mov  word col,7
           mov  si,offset blank_line
           call display_string
           mov  word col,7
           mov  si,offset not_valid_s
           call display_string
           mov  word col,21
           xor  eax,eax
           mov  ax,cur_entry
           inc  ax
           call display_dec
           inc  word row
           mov  word col,7
           mov  si,offset blank_line
           call display_string

next:      inc  word row   ; move to next row
           mov  word col,7
           inc  word cur_display

next_entry:
           add  di,sizeof(S_EMBR_ENTRY)
           inc  word cur_entry
           cmp  word cur_display,TOTAL_DISPLAY
           jae  short get_user_input
           
           jmp  first


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now wait for user input, counting down the seconds
           ; until we boot last_booted.  If key press, then stop
           ; counting down.
get_user_input:
           mov  byte color,07h
           push edi
           call get_entry_hdr_offset
           movzx ecx,byte [di + S_EMBR->boot_delay]
           pop  edi
wait_loop: mov  word row,19
           mov  word col,45
           
           cmp  byte key_pressed,1
           je   short get_key
           
           mov  ah,01h
           int  16h
           jz   no_key_pressed
           
           mov  byte key_pressed,1
           mov  word row,18
           mov  word col,0
           mov  si,offset legend
           call display_string
           
get_key:   xor  ah,ah
           int  16h

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; down key
           cmp  ax,5000h
           jne  short not_down
           mov  ax,cur_selected
           sub  ax,cur_start
           ; check to see if at last one
           mov  bx,tot_entries
           dec  bx
           cmp  ax,bx
           je   do_again
           ; else see if we can scroll the screen
           cmp  ax,(TOTAL_DISPLAY-1)
           jae  short @f
           inc  word cur_selected
           jmp  do_again
@@:        mov  ax,cur_selected
           inc  ax
           cmp  ax,tot_entries
           jae  do_again
           inc  word cur_start
           inc  word cur_selected
           jmp  do_again

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; up key
not_down:  cmp  ax,4800h
           jne  short not_up
           mov  ax,cur_selected
           sub  ax,cur_start
           jbe  short @f
           dec  word cur_selected
           jmp  do_again
@@:        cmp  word cur_start,0
           je   do_again
           dec  word cur_start
           dec  word cur_selected
           jmp  do_again

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Enter Key
not_up:    cmp  ax,1C0Dh  ; enter key
           jne  short not_enter
           mov  ax,cur_selected
           mov  last_boot,ax
           jmp  boot_it

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; 'I' key (for info)
not_enter: cmp  ax,1769h  ; 'I'
           je   short @f
           cmp  ax,1749h  ; 'i'
           jne  short not_i
@@:        mov  ax,cur_selected
           call display_info
           jmp  do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; 'T' key (change seconds to wait for boot)
not_i:     cmp  ax,1474h  ; 'T'
           je   short @f
           cmp  ax,1454h  ; 't'
           jne  short not_t
@@:        mov  word col,5
           mov  word row,23
           mov  si,offset enter_dec_number
           call display_string
           call get_dec_number
           mov  word col,5
           mov  word row,23
           mov  si,offset blank_line
           call display_string
           cmp  eax,-1
           je   short do_again
           cmp  eax,01
           jb   short @f
           cmp  eax,255
           jbe  short save_delay_count
@@:        ; TODO: print error
           jmp  short do_again

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now write delay back to the disk
save_delay_count:
           push edi
           call get_entry_hdr_offset
           mov  [di + S_EMBR->boot_delay],ax
           mov  cx,1
           xor  ebx,ebx
           mov  bx,es
           shl  ebx,4
           add  ebx,edi
           mov  eax,edi
           pop  edi
           shr  eax,9
           cdq
           mov  si,4301h       ; write service
           call transfer_sectors_long
           jmp  short do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; 'R' key (Reboot)
not_t:     cmp  ax,1372h  ; 'R'
           je   short @f
           cmp  ax,1352h  ; 'r'
           jne  short not_r
@@:        int  18h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; 
not_r:     
           ; another key would go here.


do_again:  jmp  again


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; this gets called if no key was pressed.
           ; once a key is pressed, a flag gets set and
           ; this code isn't called again.
no_key_pressed:
           ; wait for a second to lapse
           call wait_sec_lapse

           mov  word row,22
           mov  word col,21
           movzx eax,word last_boot
           call display_dec

           ; clear out the last number
           mov  word row,22
           mov  word col,26
           cmp  ecx,9
           ja   short @f
           mov  al,' '
           call display_char
@@:        mov  eax,ecx
           call display_dec

           dec  ecx
           jnz  wait_loop

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the entry offset
boot_it:   call get_entry_hdr_offset
           mov  cx,last_boot
           xor  bx,bx
           add  di,sizeof(S_EMBR)
boot_it0:  jcxz short boot_it1
           add  di,sizeof(S_EMBR_ENTRY)
           dec  cx
           inc  bx
           jmp  short boot_it0

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure this entry is valid
boot_it1:  test byte [di + S_EMBR_ENTRY->flags],ENTRY_VALID
           jnz  short @f
           mov  ax,last_boot
           call display_info
           jmp  short do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; set booted time on entry
@@:        call date_to_secs
           mov  [di + S_EMBR_ENTRY->date_last_booted+0],eax
           mov  [di + S_EMBR_ENTRY->date_last_booted+4],edx
           
           mov  ax,07C0h
           mov  es,ax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate new crc
           push edi
           call get_entry_hdr_offset
           mov  si,di
           pop  edi
           mov  dword [si + S_EMBR->crc],0
           mov  ax,[si + S_EMBR->entry_count]
           mov  cx,sizeof(S_EMBR_ENTRY)
           mul  cx
           mov  cx,ax
           add  cx,sizeof(S_EMBR)
           call crc32
           mov  [si + S_EMBR->crc],eax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now write the entries back to disk
           xor  dx,dx
           mov  ax,tot_entries
           mov  cx,sizeof(S_EMBR_ENTRY)
           mul  cx
           mov  cx,sector_size
           div  cx
           or   dx,dx
           jz   short @f
           inc  ax
@@:        push ax                   ; ax = sectors to write

           xor  ebx,ebx
           mov  bx,es
           shl  ebx,4
           push edi
           call get_entry_hdr_offset
           add  ebx,edi
           
           xor  edx,edx
           mov  eax,edi
           pop  edi
           movzx ecx,word sector_size
           div  ecx
           
           xor  edx,edx
           inc  eax            ; eMBR is at LBA 1
           
           pop  cx             ; sectors to write
           mov  si,4301h       ; write service
           call transfer_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; clear the screen
           mov  ax,0003h
           int  10h
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; get the starting from sector and read it to 0x07A00
           ;  (We can't read it to 0x07C00 since our transfer_sectors_long
           ;   routine is in that sector.  Read it into memory at 0x07A00
           ;   then move it to 0x07C00, then at the last thing jump to it
           mov  cx,1
           mov  ebx,07A00h     ; physical address 0x07A00
           mov  eax,[di + S_EMBR_ENTRY->starting_sector + 0] ; edx:eax = starting sector (LBA)
           mov  edx,[di + S_EMBR_ENTRY->starting_sector + 4]
           mov  si,4201h       ; read service
           call transfer_sectors_long
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure dl = disk booted from
           ; (we need to do this before the move below, or the
           ;  move will overwrite the value in 'drive')
           mov  dl,drive                ; from booted drive
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; move memory from 0x07A00 to 0x07C00
           push ds
           push es
           xor  ax,ax
           mov  ds,ax
           mov  es,ax
           mov  si,7A00h
           mov  di,7C00h
           ; mov  cx,(SECT_SIZE>>2)
           mov  cx,(512>>2)
           rep
             movsd
           pop es
           pop ds
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now jump to the newly loaded code
           jmp far 0000h,07C0h   ; (NBASM order->) offset,segment

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; So that we can update the S_EMBR struct independently of this code,
;  we need to retrieve the address instead of hard coding it.
; Since we load the code and the header/entries consecutively in memory,
;  the offset to the S_EMBR struct is (0x0000 + (S_EMBR.offset * 512) - 512)
get_entry_hdr_offset proc near
           movzx edi,word [hdr_offset]
           dec  edi
           shl  edi,9
           ret
get_entry_hdr_offset endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a 32-bit decimal number to the screen
;
display_dec proc near uses eax ecx edx
           ; preserve the color and then print all integers in white
           movzx cx,byte color
           push cx
           and  byte color,(~0Fh)
           or   byte color,0Fh
           
           or   eax,eax
           jns  short @f
           push ax
           mov  al,'-'
           call display_char
           pop  ax
           neg  eax
           
@@:        mov  cx,0FFFFh               ; Ending flag
           push cx
           mov  ecx,10
@@:        xor  edx,edx
           div  ecx                     ; Divide by 10
           add  dl,30h                  ; Convert to ASCII
           push dx                      ; Store remainder
           or   eax,eax                 ; Are we done?
           jnz  short @b                ; No, so continue
@@:        pop  ax                      ; Character is now in DL
           cmp  ax,0FFFFh               ; Is it the ending flag?
           je   short @f                ; Yes, so continue
           call display_char
           jmp  short @b                ; Keep doing it
           
@@:        ; restore the color
           pop  cx
           mov  color,cl
           ret
display_dec endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a 64-bit hex number to the screen
;
display_hex64 proc near uses eax ebx ecx
           
           ; preserve the color and then print all integers in white
           movzx cx,byte color
           push cx
           and  byte color,(~0Fh)
           or   byte color,0Fh
           
           mov  si,offset hex_str
           push eax
           push edx
           
           mov  cx,2
hex_loop:  pop  eax
           push cx
           mov  cx,08
@@:        rol  eax,04         ;
           mov  bx,ax
           and  bx,000Fh
           push ax
           mov  al,[bx+si]
           call display_char
           pop  ax
           loop @b
           pop  cx
           loop hex_loop
           
           mov  al,'h'
           call display_char
           
           ; restore the color
           pop  cx
           mov  color,cl
           
           ret
display_hex64 endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a 16-bit hex number to the screen
;
display_hex16 proc near uses eax ebx

           ; preserve the color and then print all integers in white
           movzx cx,byte color
           push cx
           and  byte color,(~0Fh)
           or   byte color,0Fh
           
           mov  si,offset hex_str
           rol  eax,16
           mov  cx,4
@@:        rol  eax,04         ;
           mov  bx,ax
           and  bx,000Fh
           push ax
           mov  al,[bx+si]
           call display_char
           pop  ax
           loop @b
           
           mov  al,'h'
           call display_char
           
           ; restore the color
           pop  cx
           mov  color,cl
           
           ret
display_hex16 endp

hex_str    db '0123456789ABCDEF'

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; update the scroll bar
;  on entry
;   nothing
;  on return
;   nothing
update_scroll_bar proc near uses ax bx cx dx
           
           ; if there is only 1 entry, then just return.
           ; (if only 1 entry, the cur_selected = 0, and
           ;  a zero based tot_entries = 0, and 0 div 0 = -err-
           cmp  word tot_entries,1
           ja   short @f
           ret
           
           ; update scroll bar position
           ; we multiply by 100, then divide by 100 so
           ;  that we don't have to use floating point math.
           ; percent = ((cur_selected * 100) / (tot_entries - 1))
@@:        mov  ax,cur_selected
           mov  cx,100
           mul  cx
           mov  cx,tot_entries
           dec  cx    ; zero based
           xor  dx,dx
           div  cx
           ; ax now == percent
           
           ; clear the scroll bar
           push ax
           mov  byte color,07h
           mov  word row,4
           mov  al,176
           mov  cx,12
@@:        mov  word col,76
           call display_char
           inc  word row
           loop @b
           pop  ax
           
           ; calculate the size of the bar
           push ax
           mov  ax,12
           mov  cx,tot_entries
           xor  dx,dx
           div  cx
           or   ax,ax
           jnz  short @f
           inc  ax
@@:        cmp  ax,12
           jb   short @f
           mov  ax,11
@@:        mov  bx,ax
           pop  ax
           
           ; now set the bar.
           ; ax = cur_select percentage of total entries
           ; ax = 0 is first one, 100 is last one
           ; calcuate position
           ; relative_pos = (((max_size_of_bar - cur_size) * percentage_from_above) / 100)
           mov  cx,12
           sub  cx,bx
           mul  cx
           mov  cx,100
           xor  dx,dx
           div  cx
           
           ; ax = which row (of the 12) to set.
           add  ax,4
           mov  row,ax
           mov  al,178
           mov  cx,bx
@@:        mov  word col,76
           call display_char
           inc  word row
           loop  @b

           ret
update_scroll_bar endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; waits for a second to lapse or a key pressed
wait_sec_lapse proc near uses ax dx cx si es
           xor  ax,ax
           mov  es,ax
           mov  cx,18
           mov  si,046Ch
wait0:     mov  dx,es:[si]
wait1:     mov  ah,01h
           int  16h
           jnz  short @f
           mov  ax,es:[si]
           cmp  ax,dx
           je   short wait1
           loop wait0
@@:        ret
wait_sec_lapse endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this converts a given date and time to a count of seconds since 1 Jan 1980
; on entry: nothing
; on return edx:eax = seconds since 1 Jan 1980
date_to_secs proc near uses ebx ecx esi edi

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; get the date
           mov  ah,04h
           int  1Ah

           xor  ax,ax
           mov  al,ch
           call bcd2dec
           mov  bl,100
           mul  bl
           mov  bx,ax

           xor  ah,ah
           mov  al,cl
           call bcd2dec
           add  bx,ax
           mov  year,bx

           xor  ax,ax
           mov  al,dh
           call bcd2dec
           mov  months,ax

           mov  al,dl
           call bcd2dec
           mov  day,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; get the time
           mov  ah,02
           int  1Ah

           xor  ax,ax
           mov  al,ch
           call bcd2dec
           mov  hour,ax

           mov  al,cl
           call bcd2dec
           mov  mins,ax

           mov  al,dh
           call bcd2dec
           mov  secs,ax

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now start to calculate the secs
           xor  edx,edx
           movzx ebx,word year
           sub  ebx,1980
           mov  edi,ebx  ; save the year
           imul eax,ebx,365  ; eax = days

           shr  ebx,2    ; (year / 4) + 1 /*1980*/ - 1 /*2000*/ ; leap days
           add  eax,ebx  ; eax = days including leap days
           
           mov  cx,months
           dec  cx
           jz   short skip_months
           mov  si,offset month_days
           xor  bx,bx
month_loop:
           add  eax,[si+bx]
           cmp  bx,4          ; february?
           jne  short @f
           and  di,03h        ; is this a leap year (not checking for year 2400)
           jnz  short @f
           inc  eax
@@:        add  bx,4
           loop month_loop

skip_months:
           movzx ebx,word day
           add  eax,ebx

           mov  ebx,86400     ; seconds in a day
           mul  ebx           ; edx:eax = seconds until this morning

           movzx ebx,word hour
           imul ebx,ebx,3600
           add  eax,ebx
           adc  edx,0

           movzx ebx,word mins
           imul ebx,ebx,60
           add  eax,ebx
           adc  edx,0

           movzx ebx,word secs
           add  eax,ebx
           adc  edx,0

           ret
date_to_secs endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; converts the bcd numeral in al to a decimal number and returns it in al
;
bcd2dec    proc near uses cx
           mov  cl,al
           shr  al,4
           mov  ch,10
           mul  ch
           and  cl,0Fh
           add  al,cl
           ret
bcd2dec    endp

year    dw  0
months  dw  0
day     dw  0
hour    dw  0
mins    dw  0
secs    dw  0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; displays information about the currently selected entry
; on entry:
;  ax = index
display_info proc near uses all

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the entry offset
disp_it:   call get_entry_hdr_offset
           mov  cx,ax
           xor  bx,bx
           add  di,sizeof(S_EMBR)
disp_it0:  jcxz short disp_it1
           add  di,sizeof(S_EMBR_ENTRY)
           dec  cx
           inc  bx
           jmp  short disp_it0

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
disp_it1:  ; di -> entry
           ; clear the display box
           mov  cx,(TOTAL_DISPLAY<<1)
           mov  word row,3
           mov  byte color,07h
@@:        mov  word col,7
           mov  si,offset blank_line
           call display_string
           inc  word row
           loop @b

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print info
           cmp  byte [di + S_EMBR_ENTRY->flags],0
           jne  short @f
                      
           mov  word row,6
           mov  word col,20
           mov  si,offset not_valid_s
           call display_string
           mov  word row,6
           mov  word col,34
           xor  eax,eax
           mov  ax,bx
           inc  ax
           call display_dec
           jmp  info_not_valid

@@:        mov  word row,4
           mov  word col,7
           mov  byte color,0Fh
           lea  si,[di + S_EMBR_ENTRY->description]
           call display_string

           mov  byte color,07h
           mov  word row,5
           mov  word col,7
           mov  si,offset start_sect_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->starting_sector+0]
           mov  edx,[di + S_EMBR_ENTRY->starting_sector+4]
           call display_hex64
           push ax
           mov  al,' '
           call display_char
           mov  al,'('
           call display_char
           pop  ax
           call display_dec  ; we assume edx:eax <= 32-bits
           mov  al,')'
           call display_char
           
           mov  word row,6
           mov  word col,7
           mov  si,offset sect_count_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->sector_count+0]
           mov  edx,[di + S_EMBR_ENTRY->sector_count+4]
           call display_hex64
           push ax
           mov  al,' '
           call display_char
           mov  al,'('
           call display_char
           pop  ax
           call display_dec  ; we assume edx:eax <= 32-bits
           mov  al,')'
           call display_char
           
           mov  word row,7
           mov  word col,7
           mov  si,offset date_created_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->date_created+0]
           mov  edx,[di + S_EMBR_ENTRY->date_created+4]
           call display_date

           mov  word row,8
           mov  word col,7
           mov  si,offset last_booted_str
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->date_last_booted+0]
           mov  edx,[di + S_EMBR_ENTRY->date_last_booted+4]
           call display_date

           mov  word row,9
           mov  word col,7
           mov  si,offset OS_sig_str0
           call display_string
           mov  eax,[di + S_EMBR_ENTRY->OS_signature+0]
           mov  edx,[di + S_EMBR_ENTRY->OS_signature+4]
           call display_OS_Sig

info_not_valid:
           mov  byte color,0Fh
           mov  word row,14
           mov  word col,20
           mov  si,offset press_a_key_str
           call display_string
           mov  byte color,07h
           
           xor  ah,ah
           int  16h

           ret
display_info endp

month_days  dd 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
month_name  db 'Jan',0, 'Feb',0, 'Mar',0, 'Apr',0, 'May',0
            db 'Jun',0, 'Jul',0, 'Aug',0, 'Sep',0, 'Oct',0
            db 'Nov',0, 'Dec',0


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; displays a given date
; on entry:
;   edx:eax = seconds since 1 Jan 1980
display_date proc near uses eax ebx ecx edx esi edi
           
           ; preserve the color and then print date in white
           movzx cx,byte color
           push cx
           and  byte color,(~0Fh)
           or   byte color,0Fh

           ; save in esi:edi
           mov  esi,edx
           mov  edi,eax
           
           ; calculate the year
           mov  ebx,31536000
           div  ebx
           push eax
           mov  ecx,eax         ; save the year
           add  eax,1980
           call display_dec
           mov  al,' '
           call display_char
           pop  eax
           
           ; remaining = esi:edi - (year * 31536000)
           mul  ebx
           sub  edi,eax
           sbb  esi,edx  
           
           ; calculate the month
           mov  edx,esi                     ; 18924483
           mov  eax,edi
           mov  ebx,86400
           div  ebx
           push eax              ; save days
           mul  ebx
           sub  edi,eax          ; edi = remaining seconds
           pop  eax
           shr  ecx,2                       ; leap days
           sub  eax,ecx                     ; 213 (209)

           xor  bx,bx
           mov  si,offset month_days
@@:        cmp  eax,[si+bx]
           jb   short @f
           sub  eax,[si+bx]
           add  bx,4
           jmp  short @b

@@:        mov  si,offset month_name
           add  si,bx
           call display_string
           push ax
           mov  al,' '
           call display_char
           pop  ax

           ; eax should now have days left over
           cmp  eax,9
           ja   short @f
           push ax
           mov  al,'0'
           call display_char
           pop  ax
@@:        call display_dec
           mov  al,' '
           call display_char
           call display_char
           
           ; edi should now have seconds left
           mov  cl,'a'       ; assume am
           mov  eax,edi
           mov  ebx,3600
           div  ebx
           push eax
           cmp  eax,12
           jb   short @f
           mov  cl,'p'
@@:        jbe  short @f
           sub  eax,12
@@:        or   eax,eax       ; if midnight, print 12 instead of 00.
           jnz  short @f
           mov  eax,12
@@:        cmp  eax,9
           ja   short @f
           push ax
           mov  al,'0'
           call display_char
           pop  ax
@@:        call display_dec   ; hours
           pop  eax
           mul  ebx
           sub  edi,eax
           mov  al,':'
           call display_char
           
           mov  eax,edi
           mov  ebx,60
           div  ebx
           cmp  eax,9
           ja   short @f
           push ax
           mov  al,'0'
           call display_char
           pop  ax
@@:        call display_dec  ; minutes
           mul  ebx
           sub  edi,eax
           mov  al,':'
           call display_char
           
           mov  eax,edi
           cmp  eax,9
           ja   short @f
           push ax
           mov  al,'0'
           call display_char
           pop  ax
@@:        call display_dec
           
           mov  al,cl
           call display_char
           
           ; restore the color
           pop  cx
           mov  color,cl
           
           ret
display_date endp


start_sect_str   db ' Starting Sector: ',0
sect_count_str   db '   Total Sectors: ',0
date_created_str db '   Creation Date: ',0
last_booted_str  db 'Last Booted Date: ',0
OS_sig_str0      db '    OS signature:  OS sig: ',0
OS_sig_str1      db '                   FS sig: ',0
OS_sig_str2      db '                    Usage: ',0
OS_sig_str3      db '              OS specific: ',0
press_a_key_str  db ' Press a key to return...',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; displays information about the given OS signature
; on entry:
;   edx:eax = sig
display_OS_Sig proc near uses eax edx esi

           push eax
           mov  word row,9
           mov  word col,7
           mov  si,offset OS_sig_str0
           call display_string
           mov  eax,edx
           shr  eax,16
           call display_hex16

           mov  word row,10
           mov  word col,7
           mov  si,offset OS_sig_str1
           call display_string
           mov  eax,edx
           and  eax,0FFFFh
           call display_hex16
           pop  eax

           mov  word row,11
           mov  word col,7
           mov  si,offset OS_sig_str2
           call display_string
           push eax
           shr  eax,16
           call display_hex16
           pop  eax

           mov  word row,12
           mov  word col,7
           mov  si,offset OS_sig_str3
           call display_string
           and  eax,0FFFFh
           call display_hex16

           ret
display_OS_Sig endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; gets a decimal number from the user using the bios
; on entry:
;   nothing
; on exit:
;   eax = value  -1 if ESC pressed
get_dec_number proc near uses ebx ecx edx edi

           mov  di,offset key_buffer
           xor  bx,bx
           
get_dec_0: xor  ah,ah
           mov  [di+bx],ah
           int  16h
           cmp  ax,1C0Dh
           je   short get_dec_done0
           cmp  ax,011Bh
           je   short get_dec_doneESC
           cmp  ax,0E08h
           jne  short @f
           or   bx,bx
           jz   short get_dec_0
           dec  word col
           mov  al,' '
           call display_char
           dec  word col
           dec  bx
           jmp  short get_dec_0
@@:        cmp  al,'0'
           jb   short get_dec_0
           cmp  al,'9'
           ja   short get_dec_0
           mov  [di+bx],al
           call display_char
           inc  bx
           cmp  bx,31   ; size of buffer - 1
           jb   short get_dec_0
           mov  byte [di+bx],0

get_dec_done0:
           xor  bx,bx
           xor  eax,eax
           cdq
           xor  ecx,ecx
           mov  esi,10
@@:        mov  cl,[di+bx]
           or   cl,cl
           jz   short get_dec_done1
           mul  esi
           sub  cl,'0'
           add  eax,ecx
           inc  bx
           jmp  short @b

get_dec_done1:
           ret

get_dec_doneESC:
           mov  eax,-1
           ret
get_dec_number endp



; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; crc32 code and data

CRC32_POLYNOMIAL equ 04C11DB7h

crc32_table  dup (256 * sizeof(dword)),0   ; CRC lookup table array.

crc32_initialize proc near uses eax ecx edx di

           mov  di,offset crc32_table

           xor  ecx,ecx
outer_loop:
           mov  dl,8
           mov  eax,ecx
           call crc32_reflect
           shl  eax,24

           push ecx
           xor  ecx,ecx
inner_loop:
           mov  edx,eax
           shl  eax,1
           test edx,(1<<31)
           jz   short @f
           xor  eax,CRC32_POLYNOMIAL

@@:        inc  ecx
           cmp  ecx,8
           jb   short inner_loop
           pop  ecx

           mov  dl,32
           call crc32_reflect
           stosd

           inc  ecx
           cmp  ecx,256
           jb   short outer_loop

           ret
crc32_initialize endp

; on entry:
;  eax = reflect
;   dl = char (bit count)
; on return:
;  eax = return value
crc32_reflect proc near uses ebx ecx edx ebp

           xor  ebx,ebx

           mov  cl,1

loop_it:   test al,1
           jz   short @f
           push dx
           push cx
           sub  dl,cl
           mov  cl,dl
           mov  ebp,1
           shl  ebp,cl
           or   ebx,ebp
           pop  cx
           pop  dx

@@:        shr  eax,1

           inc  cl
           cmp  cl,dl
           jbe  short loop_it

           mov  eax,ebx
           ret
crc32_reflect endp

; on entry
;  eax = current crc (should be 0xFFFFFFFF to start with)
;  ds:si-> data to check
;  cx = length of data
; on return
;  eax = crc
crc32_partial proc near uses bx si

@@:        movzx bx,byte al
           xor  bl,[si]
           inc  si
           shl  bx,2  ; dword pointer
           shr  eax,8
           xor  eax,[bx+crc32_table]
           loop @b

           ret
crc32_partial endp

; on entry
;  ds:si-> data to check
;  cx = length of data
; on return
;  eax = crc
crc32      proc near
           mov  eax,0FFFFFFFFh
           call crc32_partial
           xor  eax,0FFFFFFFFh
           ret
crc32      endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; remaining data needed

key_buffer   dup 32,0

enter_dec_number  db 'Please enter a decimal number from 1 to 255: ',0

key_pressed  db  0    ; was a key pressed yet?
last_boot    dw  0

cur_entry    dw  0    ; current entry selected
cur_start    dw  0    ; starting entry to display as first in list
cur_display  dw  0    ; current entry being displayed
cur_selected dw  0    ; current entry selected
tot_entries  dw  0    ; total entries


menu_start  db        'ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ  FYS OS (aka Konan) Multi-boot EMBR v0.94.10  ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸'
            db  13,10,'³                (C)opyright Forever Young Software 1984-2018                 ³'
            db  13,10,'³    ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ ³'
            db  13,10,'³    ³                                                                      ',24,' ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ° ³'
            db  13,10,'³    ³                                                                      ',25,' ³'
            db  13,10,'³    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'³    Will boot entry   in    seconds.  Press any key to stop.                 ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾',0

blank_line  dup 69,' '
            db  0

not_valid_s db  ' Empty entry #',0
base_str    db  '    Base LBA: ',0
size_str    db  ' Sectors: ',0


legend      db        '³     ENTER = Boot current selected partition entry.                          ³'
            db  13,10,'³     (I)   = Info about entry.                                               ³'
            db  13,10,'³     (T)   = Change boot delay time.                                         ³'
            db  13,10,'³                                                                             ³'
            db  13,10,'³     (R)   = Reboot.                                                         ³'
            db  13,10,'³                                                                             ³',0


; the remaining is if we make a single image from this .bin file and run it to test it.
;  since we now have ultimate.exe, we can update any embr image with it

comment |  ;;;; start of comment

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of EMBR entries
; the rest of this should actually be written to the disk via a FDISK
;  utility or something of the sort.  The eMBR code ends here.
; We include this here to make it easy to test our code.  These are
;  simply dummy entries.
;
; *Make sure that it starts on a sector boundary*

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; must start on a sector boundary
           org (($ + (SECT_SIZE-1)) & ~(SECT_SIZE-1))

DEMO_THIS   equ 0  ; change '1' to '0' when not demonstrating 10 entries...

entry_hdr   db  'EMBR'        ; sig0
.if DEMO_THIS
            dd  59828E59h     ; crc32
            dw  10            ; total entries in table
.else
            dd  9F03423Fh     ; crc32
            dw  1             ; total entries in table
.endif
boot_delay  db  20            ; boot delay
            db  25h           ; version
            dq  0
            dup 8,0           ; reserved
            db  'RBME'        ; sig1
            
            ; entry 1
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  63            ; starting sector (MBR + eMBR + blank to next track) (63)
            dq  (100800-63)   ; sector count (100 cylinders)
            db  'FYSOS / LEAN Filesystem used with the FYSOS:SYSCORE book.      ',0
            dq  465D6740h     ; date created  (29 May 2017, noon)
            dq  465D6E48h     ; last booted   (29 May 2017, noon thirty)
            dq  1111222233334444h ; OS signature
            dup 16,0          ; reserved
            
.if DEMO_THIS
            ; entry 2
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  24567         ; starting sector
            dq  25184954      ; sector count
            db  'Entry #2 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46516740h     ; date created
            dq  46546E48h     ; last booted
            dq  4568751657532121h ; OS signature
            dup 16,0          ; reserved
            
            ; entry 3
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  124567        ; starting sector
            dq  125184954     ; sector count
            db  'Entry #3 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46416740h     ; date created
            dq  46446E48h     ; last booted
            dq  4231876854489457h ; OS signature
            dup 16,0          ; reserved

            ; entry 4
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  2845          ; starting sector
            dq  2545844       ; sector count
            db  'Entry #4 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46416040h     ; date created
            dq  46446E08h     ; last booted
            dq  3135487646878746h ; OS signature
            dup 16,0          ; reserved

            ; entry 5
            dd  0             ; flags *Not Valid* *Empty slot*
            db  'eMBR'        ; signature
            dq  111121111     ; starting sector
            dq  22544         ; sector count
            db  'Entry #5 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46415040h     ; date created
            dq  46445E08h     ; last booted
            dq  46546A456D746446h ; OS signature
            dup 16,0          ; reserved

            ; entry 6
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  1AAAAAAAAh    ; starting sector
            dq  22548855      ; sector count
            db  'Entry #6 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46413040h     ; date created
            dq  46443E08h     ; last booted
            dq  46546A46456D5446h ; OS signature
            dup 16,0          ; reserved

            ; entry 7
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  1111111111    ; starting sector
            dq  2222222222    ; sector count
            db  'Entry #7 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46403040h     ; date created
            dq  46433E08h     ; last booted
            dq  4673A1213AA1A34Dh ; OS signature
            dup 16,0          ; reserved

            ; entry 8
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  333333333     ; starting sector
            dq  444444444     ; sector count
            db  'Entry #8 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  46401040h     ; date created
            dq  46431E08h     ; last booted
            dq  47799A4654D444AAh ; OS signature
            dup 16,0          ; reserved

            ; entry 9
            dd  0             ; flags *Not Valid* *Empty slot*
            db  'eMBR'        ; signature
            dq  2255225522    ; starting sector
            dq  5522552255    ; sector count
            db  'Entry #9 Demo entry.  Will not have anything.  Do not boot...  ',0
            dq  36401040h     ; date created
            dq  36431E08h     ; last booted
            dq  9999A99AAA4A45AAh ; OS signature
            dup 16,0          ; reserved

            ; entry 10
            dd  ENTRY_VALID   ; flags
            db  'eMBR'        ; signature
            dq  0DEADBEEFh    ; starting sector
            dq  0A5A5A5A5A5h  ; sector count
            db  'Entry #10 Demo entry.  Will not have anything.  Do not boot... ',0
            dq  26401040h     ; date created
            dq  26431E08h     ; last booted
            dq  44DDDDDDDDAAAAAAh ; OS signature
            dup 16,0          ; reserved
.endif
            
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  fill the remaining with 90h's

;%print ((OCCUPY * SECT_SIZE)-$)  ; 8544 bytes here

            org (OCCUPY * SECT_SIZE)

|  ;;; end of comment


.end

;TODO:
; - the calculated seconds from date is a day ahead.  Leap year or something.
; - scroll bar
