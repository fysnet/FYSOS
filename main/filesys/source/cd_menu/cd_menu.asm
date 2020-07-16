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
 

 ;
 ;  CD_MENU.ASM
 ;   Multi-boot CD-ROM menu system for an 'El Torito' formatted bootable
 ;   CD-ROM.  This code gets placed at 0x07C00 by the BIOS.  It loads all
 ;   the sectors of the file for us.
 ;  
 ;  Hardware Requirements:
 ;    This code uses instructions that are valid for a 386 or later 
 ;    Intel x86 or compatible CPU.
 ;
 ;  Last updated: 15 July 2020
 ;
 ;  Assembled using (NBASM v00.26.74) (http://www.fysnet/newbasic.htm)
 ;   nbasm usbboot
 ;
 ;  Notes:
 ;   This 'El Torito' BIOS loads us to 0x07C0:0000, all sectors of the file.
 ;   Once the BIOS jumps to 0x07C0:0000, the program can retrieve its boot
 ;   information by issuing INT 13, Function 4B, AL=01. After the boot
 ;   process has been initiated the INT 13 Extensions (functions 41-48)
 ;   will access the CD using 0800h byte sectors and the LBA address
 ;   provided to INT 13 is an absolute sector number.

.model tiny                        ;

include 'cd_menu.inc'              ;

outfile 'cd_menu.img'              ; target filename

.code                              ;
.rmode                             ; bios starts with (un)real mode
.386                               ; we assume 80x386 for this code
           org  00h                ; 0x07C0:0000h
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our cd_menu code
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
start:     cli                     ; don't allow interrupts
           mov  ax,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,ax              ;
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           mov  drive,dl           ; Store drive number
                                   ; (supplied by BIOS startup code)

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Our first task is to move out of the way so that we can load another
;  image to 0x07C00
; We move to 0x70000, then we limit the amount of an 'No emulation' load
;  so that we don't overwrite ourselves.
           xor  si,si
           xor  di,di
           mov  ax,7000h
           mov  es,ax
           mov  cx,(end_of_code >> 2)
           rep
             movsd
           
           push ax                ; push segment
           push offset new_start  ; push offset
           retf                   ; jump to it
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; setup new seg registers
new_start: mov  ax,7000h
           mov  ds,ax
           mov  es,ax
           mov  ss,ax
           mov  sp,STACK_OFFSET
           
           sti                     ; allow interrupts again
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Print that we are here
           mov  ax,0003h
           int  10h
           mov  si,offset start_str
           call display_string
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Our next task is to retrieve the Bootable CD-ROM Specification Packet
;  so that we can update it later.
           mov  ax,4B01h
           mov  dl,drive
           mov  si,offset boot_packet
           int  13h
           jnc  short check_bios_serv
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if bad call, display message, wait for keypress, reboot
           mov  si,offset bad_params
reboot:    call display_string
           mov  si,offset pressakey
           call display_string
           xor  ah,ah
           int  16h
           int  18h

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; we need to check for the BIOS DISK Extensions.
check_bios_serv:
           mov  ah,41h
           mov  bx,55AAh
           mov  dl,drive
           int  13h
           jc   short @f
           cmp  bx,0AA55h
           jne  short @f
           test cx,1
           jnz  short read_boot_desc
@@:        mov  si,offset no_extentions
           jmp  short reboot
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Next task is to read sector 17 (11h) of the cd to get the Boot Descriptor
read_boot_desc:
           xor  edx,edx
           mov  eax,17
           mov  ecx,1
           mov  ebx,offset buffer
           call read_sectors_long
           mov  si,offset bad_read
           jc   short reboot
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure we have the correct sector
           mov  si,offset buffer
           mov  di,offset brvd_contents
           mov  cx,32
           call mem_compare
           mov  si,offset bad_brvd_str
           jne  short reboot
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The BRVD passed our checks. Get the boot catalog lba and read it in.
; We read in 1 sector.  This code assumes there will be no more sectors
;  used.  one sectors = ~64 entries.  More than any bootable CD should have. :-)
           mov  si,offset buffer
           xor  edx,edx
           mov  eax,[si + S_BRVD->boot_cat_lba]
           mov  ecx,(ALLOWED_ENTRIES / 64)  ; (64 32-byte entries per sector)
           mov  ebx,offset buffer
           call read_sectors_long
           mov  si,offset bad_read
           jc   reboot
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now have the boot catalog at 'buffer' (1 sector for now)
; We can now read the entries and store them in our structure.
; We skip the Default/Initial entry since we are currently running
;  that entry.
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; We first check the Validation Entry to verify
           ; that this is a valid Boot Catalog.
           mov  si,offset buffer
           
           ; check a few items
           cmp  byte [si + S_VAL_ENTRY->id],1
           jne  short bad_validation_entry
           cmp  byte [si + S_VAL_ENTRY->platform],2
           ja   short bad_validation_entry
           cmp  word [si + S_VAL_ENTRY->key55],0AA55h
           jne  short bad_validation_entry
           
           ; check the crc
           call check_crc
           jz   short good_validation_entry
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Validation entry did not check out, give error.
bad_validation_entry:
           mov  si,offset bad_val_entry
           jmp  reboot

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; validation entry checks okay.
good_validation_entry:
           add  si,(sizeof(S_SECT_ENTRY) * 2) ; skip to the default/initial entry
           
           mov  di,offset menu_data
           mov  cx,(sizeof(S_OUR_ENTRY) * ALLOWED_ENTRIES)
           call clear_memory
           
           mov  word tot_entries,0
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; ds:si -> first section header
           ; es:di -> our buffer
           ; see if this section is the last
section_loop:
           cmp  byte [si + S_SECT_HDR->id],91h
           jne  short no_more_sections
           
           ; number of entries in this section
           mov  cx,[si + S_SECT_HDR->num]
           add  tot_entries,cx
           
           ; get platform id
           mov  dl,[si + S_SECT_HDR->platform]
           
           ; skip to section entries
           add  si,sizeof(S_SECT_HDR)
entry_loop:
           push cx
           mov  cx,32
           call mem_copy
           pop  cx
           
           ; store the platform id
           mov  [di + S_OUR_ENTRY->platform],dl
           
           ; if bit 5 set of media, then there are extended entries to follow.
           mov  al,[si + S_SECT_ENTRY->media]
           test al,(1<<5)
           jz   short get_next_entry
           
           ; else, loop extended entries
           xor  bx,bx
           push cx
           push di
           add  di,sizeof(S_SECT_ENTRY)
get_next_extra:
           add  si,sizeof(S_SECT_ENTRY)
           ; don't allow it to over flow.
           inc  bx
           cmp  bx,ALLOWED_EXTRAS
           jae  short @f
           ; skip to vendor specific area and copy it
           push si
           add  si,S_SECT_EXTRA->vendor_spec
           mov  cx,30
           call mem_copy
           pop  si
           add  di,30
@@:        cmp  byte [si + S_SECT_EXTRA->next],0
           jne  short get_next_extra
           pop  di
           pop  cx
           
get_next_entry:
           add  si,sizeof(S_SECT_ENTRY)
           add  di,sizeof(S_OUR_ENTRY)
           loop entry_loop
           
           ; try the next section
           jmp  short section_loop
           
no_more_sections:
           ; check to see that we found at least 1 entry
           cmp  word tot_entries,0
           ja   short @f
           
           ; did not find any entries, so give error and reboot
           mov  si,offset no_entries_s
           jmp  reboot
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; make sure we are in text mode 3
@@:        mov  ax,0003h
           int  10h
           mov  word row,0
           mov  word col,0
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; turn off cursor
           mov  ah,01h                  ;
           mov  ch,00100000b            ; bit number 5
           int  10h                     ;

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print the menu text and start getting valid entrys
           push si
           mov  si,offset menu_start
           call display_string
           pop  si

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
           mov  di,offset menu_data
           
first:     mov  ax,cur_entry
           cmp  ax,cur_start
           jb   next_entry
           
           mov  byte color,07h
           mov  ax,cur_selected
           sub  ax,cur_start
           cmp  ax,cur_display
           jne  short @f
           mov  byte color,70h
           
@@:        ; first line of displayed entry
           mov  si,offset blank_line
           call display_string
           mov  word col,7
           
           mov  si,offset bootable_str
           call display_string
           movzx eax,byte [di + S_OUR_ENTRY->bootable]
           mov  si,offset yes_str
           cmp  al,88h
           je   short @f
           mov  si,offset no_str
@@:        call display_string
           
           mov  si,offset media_str
           call display_string
           mov  si,offset mt_no_emu
           movzx ax,byte [di + S_OUR_ENTRY->media]
           and  ax,0Fh
           shl  ax,4  ; mult by 16
           add  si,ax
           call display_string
           
           mov  si,offset platform_str
           call display_string
           mov  si,offset pf_80x86
           movzx ax,byte [di + S_OUR_ENTRY->platform]
           and  ax,03h
           shl  ax,3  ; mult by 8
           add  si,ax
           call display_string
           
           ; second line of displayed entry
           inc  word row         ; move to next row
           mov  word col,7
           
           mov  si,offset blank_line
           call display_string
           mov  word col,7
           
           mov  si,offset base_str
           call display_string
           mov  eax,[di + S_OUR_ENTRY->load_rba]
           call display_dec
           
           mov  si,offset size_str
           call display_string
           movzx eax,word [di + S_OUR_ENTRY->load_cnt]
           call display_dec
           
           inc  word row   ; move to next row
           mov  word col,7
           inc  word cur_display

next_entry:
           add  di,sizeof(S_OUR_ENTRY)
           inc  word cur_entry
           mov  ax,cur_entry
           cmp  ax,tot_entries
           jae  short get_user_input
           cmp  word cur_display,TOTAL_DISPLAY
           
           jae  short get_user_input
           jmp  first
           
get_user_input:
           xor  ah,ah
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
           je   short do_again
           ; else see if we can scroll the screen
           cmp  ax,(TOTAL_DISPLAY-1)
           jae  short @f
           inc  word cur_selected
           jmp  short do_again
@@:        mov  ax,cur_selected
           inc  ax
           cmp  ax,tot_entries
           jae  short do_again
           inc  word cur_start
           inc  word cur_selected
           jmp  short do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; up key
not_down:  cmp  ax,4800h
           jne  short not_up
           mov  ax,cur_selected
           sub  ax,cur_start
           jbe  short @f
           dec  word cur_selected
           jmp  short do_again
@@:        cmp  word cur_start,0
           je   do_again
           dec  word cur_start
           dec  word cur_selected
           jmp  short do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; Enter Key
not_up:    cmp  ax,1C0Dh  ; enter key
           jne  short not_enter
           mov  ax,cur_selected
           jmp  short boot_it
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; I key (for info)
not_enter: cmp  ax,1769h  ; 'I'
           je   short @f
           cmp  ax,1749h  ; 'i'
           jne  short not_i
@@:        mov  ax,cur_selected
           call display_info
           jmp  short do_again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;
not_i:
           ; another key would go here.

do_again:  jmp  again


           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the entry offset
boot_it:   mov  ax,cur_selected
           mov  di,offset menu_data
           mov  bx,sizeof(S_OUR_ENTRY)
           mul  bx
           add  di,ax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now we can load and boot the image specified.
           ; if no emulation is wanted, we need to load specified
           ;  sectors to specified address and jump there
           ;  (remembering that we moved out of the way first)
           mov  al,[di + S_OUR_ENTRY->media]
           and  al,0Fh
           or   al,al
           jnz  short is_emulated
           
           mov  bp,[di + S_OUR_ENTRY->load_seg]
           or   bp,bp
           jnz  short @f
           mov  bp,07C0h
@@:        movzx ecx,word [di + S_OUR_ENTRY->load_cnt]
           add  ecx,3        ; round up
           shr  ecx,2        ; convert to 2048 byte sectors
           xor  edx,edx
           mov  eax,[di + S_OUR_ENTRY->load_rba]
           push es
           mov  es,bp
           xor  bx,bx
           call read_sectors_long
           pop  es
           mov  si,offset bad_read
           jc   reboot
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; now jump to it, maybe clearing the screen first
           mov  ax,0003h
           int  10h
           
           push bp
           push 0000h
           retf
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if emulation is wanted, create packet and call BIOS
           ;  to initiate emulation and boot it.
           ; We used service 4B01h at beginning to get information
           ;  we needed.  Now, just put the new emulated figures in.
is_emulated:
           mov  si,offset boot_packet
           
           ; store size of packet (offset 0)
           mov  byte [si + 0],19
           
           ; store media byte (offset 1)
           mov  al,[di + S_OUR_ENTRY->media]
           mov  [si + 1],al
           
           ; calcuate and store drive number (offset 2)
           or   al,al
           jz   short @f
           cmp  al,3
           ja   short @f
           xor  al,al
           mov  [si + 2],al
           jmp  short do_cntrlr_indx
@@:        mov  dl,80h
           mov  al,[di + S_OUR_ENTRY->bootable]
           cmp  al,88h
           je   short @f
           inc  dl
@@:        mov  [si + 2],dl
           
           ; calcuate controller index (offset 3)
do_cntrlr_indx:
           ; ** stored already by service 4B01h **
                      
           ; lba of image to be emulated (offset 4 - 7)
           mov  eax,[di + S_OUR_ENTRY->load_rba]
           mov  [si + 4],eax
           
           ; device specification (offset 8 and 9)
           ; ** stored already by service 4B01h **
           
           ; user buffer segment (offset 0Ah and 0Bh)
           mov  ax,7000h        ; use this segment for cache (3k in size)
           mov  [si + 0Ah],ax   ;
           
           ; load segment (offset 0Ch and 0Dh)
           mov  ax,[di + S_OUR_ENTRY->load_seg]
           or   ax,ax
           jnz  short @f
           mov  ax,07C0h
           ; if value is zero, the BIOS is suppose to use 0x07C0 by default.
           ; therefore, we could comment out the above three lines...
@@:        mov  [si + 0Ch],ax
           
           ; sector count (offset 0Eh and 0Fh)
           mov  ax,[di + S_OUR_ENTRY->load_cnt]
           mov  [si + 0Eh],ax
           
           ; the remaining three bytes are the CHS values
           ;  from service 08h/int 13h for the media type
           ;  wanted.  ***Only used for floppies****
           ; bits 7:0 of cylinder count (ch of service 08h) (offset 10h)
           ; sector count (cl of service 08h) (offset 11h)
           ; head count (dh of service 08h) (offset 12h)
           mov  al,[di + S_OUR_ENTRY->media]
           cmp  al,1  ; 1.20 meg
           jne  short @f
           ; CHS values for a 1.20 meg
           mov  byte [si + 10h],80  ; ch value
           mov  byte [si + 11h],15  ; cl value
           mov  byte [si + 12h],2   ; dh value
           jmp  short do_emulation_call
           
@@:        cmp  al,2 ; 1.44 meg
           jne  short @f
           ; CHS values for a 1.44 meg
           mov  byte [si + 10h],80  ; ch value
           mov  byte [si + 11h],18  ; cl value
           mov  byte [si + 12h],2   ; dh value
           jmp  short do_emulation_call
           
@@:        cmp  al,3 ; 2.88 meg
           jne  short do_emulation_call
           ; CHS values for a 2.88 meg
           mov  byte [si + 10h],80  ; ch value
           mov  byte [si + 11h],36  ; cl value
           mov  byte [si + 12h],2   ; dh value
           ; do the service call
do_emulation_call:
           mov  ax,4C00h
           int  13h
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  On real hardware, and a decent machine, this should not return
;  to us.  However, I have seen that there are many places that say
;  that most BIOS' don't support this service.  Nice huh????
           
           ; if we came back, we failed at rebooting
           ; give error message and quit
           push ax  ; save error code
           
           ; clear the display box
           mov  cx,(TOTAL_DISPLAY << 1)
           mov  word row,3
           mov  byte color,07h
@@:        mov  word col,7
           mov  si,offset blank_line
           call display_string
           inc  word row
           loop @b
           
           mov  word row,5
           mov  word col,7
           mov  si,offset no_reboot_str
           call display_string
           
           ; print error code
           pop  ax
           ror  ax,8
           call display_hex_byte
           ror  ax,8
           call display_hex_byte
           mov  al,'h'
           call display_char
           mov  al,')'
           call display_char

           mov  word row,6
           mov  word col,7
           mov  si,offset pressakey
           call display_string
           xor  ah,ah
           int  16h
           int  18h
           
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service.
; On entry:
;  edx:eax = starting sector in LBA format
;      ecx = count of sectors to read
;   es:ebx = offset of buffer to read to
read_sectors_long proc near uses eax ebx ecx edx esi
           mov  si,offset long_packet
           mov  word [si],0010h    ; packet it 16 bytes long
           and  cx,7Fh             ; make sure not more than 7Fh
           mov  [si+02h],cx        ; ecx = count of sectors to read
           mov  [si+04h],bx        ; es:ebx -> where to read to
           mov  [si+06h],es        ;
           mov  [si+08h],eax       ; edx:eax = LBA
           mov  [si+0Ch],edx       ;
           mov  ah,42h             ; BIOS Extended Disk Read Service
           mov  dl,drive           ; dl = drive
           int  13h
           
           ret
read_sectors_long endp

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
; display a string to the screen
;
display_string proc near uses ax si
           cld
display_loop:                      ; ds:si = asciiz message
           lodsb
           or   al,al
           jz   short @f
           call display_char
           jmp  short display_loop
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a 32-bit decimal number to the screen using the BIOS
;
display_dec proc near uses eax ebx ecx edx

           or   eax,eax
           jns  short pnot_neg
           push eax
           mov  al,'-'
           call display_char
           pop  eax
           neg  eax

pnot_neg:  mov  cx,0FFFFh               ; Ending flag
           push cx
           mov  ecx,10
PD1:       xor  edx,edx
           div  ecx                     ; Divide by 10
           add  dl,30h                  ; Convert to ASCII
           push dx                      ; Store remainder
           or   eax,eax                 ; Are we done?
           jnz  short PD1               ; No, so continue
PD2:       pop  ax                      ; Character is now in DL
           cmp  ax,0FFFFh               ; Is it the ending flag?
           je   short PD3               ; Yes, so continue
           call display_char
           jmp  short PD2               ; Keep doing it
           
PD3:       ret
display_dec endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display an 8-bit hex number to the screen
;
display_hex_byte proc near uses ax cx
           
           mov  cx,2
@@:        ror  al,4
           push ax
           and  al,0Fh
           daa
           add  al,-16
           adc  al,+64
           call display_char
           pop  ax
           loop @b
           
           ret
display_hex_byte endp

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
; compare two buffers
;  on entry
;   ds:si -> source1
;   es:di -> source2
;      cx = length to check
;  on return
;   zero set if equal
mem_compare   proc near uses si di cx
           cld
           repe
             cmpsb
           ret
mem_compare   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; clear a buffer
;  on entry
;   es:di -> target
;      cx = length to clear
;  on return
;   nothing
clear_memory  proc near uses ax cx di
           xor  al,al
           rep
             stosb
           ret
clear_memory  endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; copy a buffer
;  on entry
;   ds:si -> source
;   es:di -> target
;      cx = length to copy
;  on return
;   nothing
mem_copy   proc near uses cx si di
           rep
             movsb
           ret
mem_copy   endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; checks the crc of the Validation Entry
;  on entry
;   ds:si -> entry
;  on return
;   zero set if okay
check_crc  proc near uses ax cx si
           xor  ax,ax
           mov  cx,(32 / sizeof(word))
@@:        add  ax,[si]
           add  si,sizeof(word)
           loop @b
           
           or   ax,ax
           ret
check_crc  endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; displays information about the currently selected entry
; on entry:
;  ax = index
display_info proc near uses all

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; calculate the entry offset
disp_it:   mov  di,offset menu_data
           mov  bx,sizeof(S_OUR_ENTRY)
           mul  bx
           add  di,ax
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
disp_it1:  ; di -> entry
           ; clear the display box
           mov  cx,(TOTAL_DISPLAY << 1)
           mov  word row,3
           mov  byte color,07h
@@:        mov  word col,7
           mov  si,offset blank_line
           call display_string
           inc  word row
           loop @b
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; print info
           ; first line
           mov  word row,4
           mov  word col,7
           
           mov  si,offset bootable_str
           call display_string
           movzx eax,byte [di + S_OUR_ENTRY->bootable]
           mov  si,offset yes_str
           cmp  al,88h
           je   short @f
           mov  si,offset no_str
@@:        call display_string
           
           mov  si,offset media_str
           call display_string
           mov  si,offset mt_no_emu
           movzx ax,byte [di + S_OUR_ENTRY->media]
           and  ax,0Fh
           shl  ax,4  ; mult by 16
           add  si,ax
           call display_string
           
           mov  si,offset platform_str
           call display_string
           mov  si,offset pf_80x86
           movzx ax,byte [di + S_OUR_ENTRY->platform]
           and  ax,03h
           shl  ax,3  ; mult by 8
           add  si,ax
           call display_string
           
           ; second line
           mov  word row,5
           mov  word col,7
           mov  si,offset base_str
           call display_string
           mov  eax,[di + S_OUR_ENTRY->load_rba]
           call display_dec
                      
           mov  si,offset size_str
           call display_string
           movzx eax,word [di + S_OUR_ENTRY->load_cnt]
           call display_dec

           ; third line
           mov  word row,7
           mov  word col,7
           
           ; display vendor specific stuff
           mov  si,offset vendor_spec_str
           call display_string
           
           mov  cx,VEND_SPEC_LINES
           lea  si,[di + S_OUR_ENTRY->vendor_spec]
vend_loop:
           inc  word row
           mov  word col,8
           
           push cx
           push si
           mov  cx,VEND_SPEC_CHARS
data_chars:
           lodsb
           call display_hex_byte
           mov  al,' '
           cmp  cx,((VEND_SPEC_CHARS / 2) - 1)
           jne  short @f
           mov  al,'-'
@@:        call display_char
           loop data_chars
           pop  si
           
           mov  al,20h
           call display_char
           call display_char
           
           mov  cx,VEND_SPEC_CHARS
           push si
str_chars: lodsb
           cmp  al,20h
           ja   short @f
           mov  al,'.'
@@:        call display_char
           loop str_chars
           pop  si
           pop  cx
           
           add  si,VEND_SPEC_CHARS
           loop vend_loop
           
           xor  ah,ah
           int  16h
           
           ; clear the display box
           mov  cx,(TOTAL_DISPLAY << 1)
           mov  word row,3
           mov  byte color,07h
@@:        mov  word col,7
           mov  si,offset blank_line
           call display_string
           inc  word row
           loop @b
           
           ret
display_info endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  data
drive         db  0
boot_packet   dup 19,0

color         db  07h
row           dw  0
col           dw  0

cur_entry     dw  0    ; current entry selected
cur_start     dw  0    ; starting entry to display as first in list
cur_display   dw  0    ; current entry being displayed
cur_selected  dw  0    ; current entry selected
tot_entries   dw  0    ; total entries

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  string data
start_str     db  'Loading Menu System...',13,10,0

bad_params    db  'Error retrieving drive parameters.',13,10,0
bad_read      db  'Error reading from disk.',13,10,0
pressakey     db  'Press a key to reboot.',13,10,0
bad_brvd_str  db  'Bad Boot Record Volume Descriptor',13,10,0
bad_val_entry db  'Bad Validation Entry.',13,10,0
no_entries_s  db  'No entries found...',13,10,0
no_extentions db  'Requires int 13h extented read service.'13,10,0
no_reboot_str db  'Did not boot to emulated image correctly. (',0

; contents of start of BRVD
;      id = 0
;   ident = 'CD001'
; version = 1
; bsident = 'EL TORITO SPECIFICATION',0,0,0,0,0,0,0,0,0
brvd_contents db  0, 'CD001', 1, 'EL TORITO SPECIFICATION',0,0,0,0,0,0,0,0,0

menu_start  db  'ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍ  FYS OS (aka Konan) Multi-boot CD-ROM v1.00.00  ÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸',13,10
            db  '³                (C)opyright Forever Young Software  1984-2019                ³',13,10
            db  '³    ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ ³',13,10
            db  '³    ³                                                                      ',24,' ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ° ³',13,10
            db  '³    ³                                                                      ',25,' ³',13,10
            db  '³    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ ³',13,10
            db  '³     ENTER = Boot current selected partition entry.                          ³',13,10
            db  '³     (I)   = Info about entry.                                               ³',13,10
            db  '³                                                                             ³',13,10
            db  '³                                                                             ³',13,10
            db  '³                                                                             ³',13,10
            db  '³                                                                             ³',13,10
            db  'ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾',0

blank_line  dup 69,' '
            db  0

bootable_str db  ' Bootable: ',0
media_str    db  '   Media type: ',0
platform_str db  '   Platform: ',0
base_str     db  ' Load LBA: ',0
size_str     db  '   Sectors: ',0

; media type strings. (must remain 16 bytes each)
mt_no_emu    db  'No Emulation   ',0
mt_120meg    db  '1.20 Meg Floppy',0
mt_144meg    db  '1.44 Meg Floppy',0
mt_288meg    db  '2.88 Meg Floppy',0
mt_harddrive db  'Hard Drive Emu ',0

; platform type strings (must remain 8 bytes each)
pf_80x86     db  '80x86  ',0
pf_PowerPC   db  'PowerPC',0
pf_Mac       db  'Mac    ',0

vendor_spec_str db 'Vendor Specific Data:',0

yes_str      db  'Yes',0
no_str       db  'No ',0


;%print ($)     ; Length of file in bytes (not counting .BSS area)

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  end of code is used as 'unallocated' buffer (.BSS area)
.para
long_packet   dup 16,?          ; read services packet

menu_data     dup (sizeof(S_OUR_ENTRY) * ALLOWED_ENTRIES),?

buffer        dup (2048 * 2),?       ; 2 - 2048 byte sector buffer

.para
end_of_code:  ; mark end of code so we know how much to move

;%print ($)    ; Length of all data and code

.end
