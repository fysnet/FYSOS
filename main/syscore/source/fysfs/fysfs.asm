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

comment /******************************************************************\
*                            FYS OS version 2.0                            *
* FILE: fysfs.asm                                                          *
*                                                                          *
* Boot sector for an IBM-PC compatible machine.                            *
* Written to fit into the first sector(s) of a hard disk.                  *
*                                                                          *
*  Built with:  NBASM ver 00.26.52                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
* Last Update:  23 June 2013                                               *
*                                                                          *
* This bootsector is written for a FYSFS hard disk.                        *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*                                                                          *
* Hardware Requirements:                                                   *
*  This code uses commands that are valid for a 386 or later Intel x86     *
*  or compatible CPU.                                                      *
*                                                                          *
***************************************************************************/

.model tiny                        ;

include ..\include\boot.inc        ;
include ..\include\fysfs.inc

outfile 'fysfs.bin'                ; out filename (using quotes will keep it lowercase)

.code                              ;
.rmode                             ; bios starts with (un)real mode
.186                               ; we assume 80x186 for boot sector
           org  00h                ; 07C0:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; start of our boot code
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  setup the stack and seg registers (real mode)
           cli                     ; don't allow interrupts
           mov  bp,07C0h           ; 07C0:0000h = 0000:7C00h = 0x07C00
           mov  ds,bp              ;
           mov  es,bp              ;
           mov  ss,bp              ; start of stack segment (07C0h)
           mov  sp,STACK_OFFSET    ; first push at 07C0:43FEh
                                   ; (17k size in bytes)
                                   ; (07C0:4400h = 0C00:0000h which is just
                                   ;    under 0C00:0000h where ROOT resides)
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           mov  boot_data.drive,dl ; Store drive number (supplied by BIOS startup code)
           sti                     ; allow interrupts again
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Check for 386+ machine.  If not, give error and halt
           pushf                   ; save the interrupt bit
           mov  ax,0F000h          ; if bits 15:14 are still set
           push ax                 ;  after pushing/poping to/from
           popf                    ;  the flags register then we have
           pushf                   ;  a 386+
           pop  ax                 ;
           and  ax,0F000h          ;
           jnz  short @f           ; it's a 386+
           mov  si,offset not386str
           jmp  BootErr

not386str  db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf                   ; restore the interrupt bit

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now check for the BIOS disk extensions.
; We assume we will not be on a floppy disk.
           mov  ah,41h
           mov  bx,55AAh
           int  13h                     ; dl still = drive_num
           jc   short @f
           cmp  bx,0AA55h
           jne  short @f
           test cx,1
           jnz  short read_remaining
@@:        mov  si,offset no_extentions_found
           jmp  short BootErr

no_extentions_found db  'Requires int 13h extented read service.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to call the BIOS to get the parameters of the current disk.
;  so we can pass it on to loader.sys
read_remaining:
           mov  ah,48h
           mov  dl,boot_data.drive
           mov  si,offset read_buffer
           mov  word [si],26
           int  13h
           mov  ax,[si+S_INT13_PARMS->sect_size]
           mov  boot_data.sect_size,ax
           mov  ax,[si+S_INT13_PARMS->heads]
           mov  boot_data.Heads,ax
           mov  ax,[si+S_INT13_PARMS->sect_trck]
           mov  boot_data.SecPerTrack,ax
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the rest of the boot code

           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; read in 16 more sectors from base + 1
           mov  bx,0200h       ; address 0x07E00
           mov  eax,1          ;
           cdq
           mov  cx,16          ; make sure we get the super_block too
           call read_sectors_long

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; jump to the rest of the code we just loaded
           jmp  remaining_code

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  This is if we have an error of some kind when trying to boot
;
BootErr:   call display_string     ;
           xor  ah,ah              ; Wait for keypress
           int  16h

           int  18h                ; boot specs say that 'int 18h'
                                   ;  should be issued here.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Procedures used with in code follow:
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; The following code and data must remain in the first sector

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;  edx:eax = starting sector in LBA format
;       cx = count of sectors to read (doesn't check for > 7Fh)
;    es:bx = offset to store data, es:offset
; NOTE: This should get called with <= 7Fh sectors to read
read_sectors_long proc near uses eax bx cx edx si di

           mov  si,offset read_buffer
           mov  word [si],0010h
           mov  [si+2],cx
           mov  [si+04h],bx
           mov  [si+06h],es
           add  eax,[boot_sig.base_lba+0]  ; base lba
           adc  edx,[boot_sig.base_lba+4]
           mov  [si+08h],eax
           mov  [si+0Ch],edx
           
           mov  ah,42h             ; read
           mov  dl,boot_data.drive ; dl = drive
           int  13h
           mov  si,offset diskerrorS
           jc   BootErr
           
           ret
read_sectors_long endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; display a string to the screen using the BIOS
; 
display_string proc near uses ax bx si
@@:        lodsb                   ; ds:si = asciiz message
           or   al,al
           jz   short @f
           mov  ah,0Eh             ; print char service
           xor  bx,bx              ; 
           int  10h                ; output the character
           jmp  short @b
@@:        ret
display_string endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  start of our data
read_buffer   dup 26,0

boot_data     st  S_BOOT_DATA  ; booted data to pass to loader.sys

diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press a key...',0
loadname      db  'loader.sys',0

%print (512-$-2)  ; 84 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of 1st sector. Pad out to fill 512 bytes, including final word 0xAA55
              org (200h-16)
boot_sig      st  S_BOOT_SIG uses 46595342h, 32, 0, 0AA55h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Start of remaining sectors.  None of this code can be used until
;  these sectors are loaded from the first sector code.

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the root directory into memory
;
remaining_code:
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  the ROOT (which contains the fat too.)
           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           xor  bx,bx
           mov  eax,[SUPER_BLOCK + S_FYSFS_SUPER->root + 0]
           mov  edx,[SUPER_BLOCK + S_FYSFS_SUPER->root + 4]
           mov  ecx,[SUPER_BLOCK + S_FYSFS_SUPER->root_entries]   ; max root entries allowed
           shr  ecx,2               ; div by 4
           cmp  ecx,FYSFS_ROOTSEG_SIZE ; limit the read to FYSFS_ROOTSEG_SIZE
           jbe  short @f           ;
           mov  ecx,FYSFS_ROOTSEG_SIZE
@@:        call read_sectors_long  ; read in the ROOT directory
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  initialize the crc check
;
           call crc32_initialize
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Search for loader file in root directory
;
           xor  ebx,ebx            ; current slot number to look in.
           mov  si,offset loadname ; loader file name
           mov  di,FYSFS_BUFFER    ; buffer for found name
           mov  ecx,[SUPER_BLOCK + S_FYSFS_SUPER->root_entries]  ; count of entries to search
s_loader:  call get_name
           jc   short @f
           call stricmp
           or   al,al
           jz   f_loader
@@:        inc  ebx                ; next slot number
           .adsize                 ; use ECX as the counter
           loop s_loader
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; if we get here, the loader file wasn't found
           mov  si,offset noloaderS
           jmp  BootErr

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Print 'Starting...' string
;
f_loader:  mov  si,offset os_load_str  ; loading message
           call display_string     ; saves all registers

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Load the Loader
;
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  Walk the FAT chain reading all 'clusters' of
           ;   the loader file.
           ; ebx = first dir entry of file
           mov  edi,FYSFS_BUFFER   ; buffer for found 32-bit fat entries
           call get_fat_entries    ; get all fat entries, returns eax = entries
           jnc  short @f
           mov  si,offset fat_slot_errorS
           jmp  BootErr
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ;  We now have the count of entries in AX and
           ;  the list of entries in ds:di
@@:        movzx ecx,word [SUPER_BLOCK + S_FYSFS_SUPER->sect_clust]   ; number of sectors to read (per cluster)
           push LOADSEG            ; es-> loader seg
           pop  es                 ;
           xor  bx,bx              ; es:bx = offset to load the data
           
           mov  ebp,eax            ; count of clusters to read
           mov  si,FYSFS_BUFFER
@@:        mov  eax,[si+0]
           mov  edx,[si+4]
           add  si,sizeof(qword)
           mul  ecx                ; cluster based
           add  eax,[SUPER_BLOCK + S_FYSFS_SUPER->data + 0]
           adc  edx,[SUPER_BLOCK + S_FYSFS_SUPER->data + 4]
           call read_sectors_long  ; read in cluster (saves all registers used)
           
           ; calculate next position
           xor  edx,edx
           movzx eax,word boot_data.sect_size
           mul  ecx
           add  bx,ax
           
           dec  ebp
           jnz  short @b
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  Now jump to the loader.
;  Send the following information in the registers:
;   ds:si-> boot data (includes  'booted from drive')
           push 07C0h              ; set up some data for LOADER.BIN
           pop  ds                 ; ds = 07C0h
           mov  si,offset boot_data
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the name of the current chain in ds:si
;  on entry:
;   ds:di-> 256 byte buffer to place asciiz name
;   bx = current slot to start in
;  on return
;   if this slot was a SLOT, and no errors
;     ds:di-> is filled with asciiz name.
;     carry clear
;   else
;     carry set
get_name   proc near uses eax ebx ecx dx si edi es

           ; ds:di-> passed buffer
           mov  si,di              ;

           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           
           movzx ebx,bx
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_name_error
           
           cmp  dword es:[ebx + S_FYSFS_ROOT->sig + 0],S_FYSFS_ROOT_NEW
           jne  short get_name_error
           
           ; is a SLOT, so start the name
           movzx ecx,byte es:[ebx + S_FYSFS_ROOT->namelen]
           jcxz short next_slot
           
           xor  edi,edi
@@:        mov  al,es:[ebx + edi + S_FYSFS_ROOT->name_fat]
           mov  [si],al
           inc  si
           inc  edi
           loop @b
           
next_slot: mov  ebx,es:[ebx + S_FYSFS_ROOT->name_continue]
           or   ebx,ebx
           jz   short get_name_done

next_cont: shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_ROOT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_name_error
           
           cmp  dword es:[ebx + S_FYSFS_CONT->sig + 0],S_FYSFS_CONT_NAME
           jne  short get_name_error
           
           movzx cx,byte es:[ebx + S_FYSFS_CONT->count]
           jcxz short get_name_done

           xor  edi,edi
@@:        mov  al,es:[ebx + edi + S_FYSFS_CONT->name_fat]
           mov  [si],al
           inc  si
           inc  di
           loop @b

           mov  ebx,es:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont

get_name_done:
           mov  byte [si],0   ; asciiz it
           clc
           ret

get_name_error:
           stc
           ret
get_name   endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; return the fat entries of the current chain in ds:si
;  on entry:
;   ds:edi-> 512 byte buffer to place entries
;   ebx = current slot to start in
;  on return
;   if this slot was a SLOT, all found entries are 64-bit, and no errors
;     ds:di-> 64-bit fat entries
;     eax = count of entries found
;     carry clear
;   else
;     carry set
get_fat_entries proc near uses ebx ecx edx esi edi ebp es

           ; starting count
           xor  ebp,ebp

           ; ds:edi-> passed buffer
           mov  esi,edi            ;

           push FYSFS_ROOTSEG      ; es -> to ROOT memory area
           pop  es                 ;
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  get_fat_error
           
           cmp  dword es:[ebx + S_FYSFS_ROOT->sig],S_FYSFS_ROOT_NEW
           jne  get_fat_error
           
           ; is a SLOT, so start the fat
           movzx ecx,byte es:[ebx + S_FYSFS_ROOT->fat_entries]
           .adsize
           jcxz short next_slot_fat
           
           add  ebp,ecx
           
           ; calculate start of entries.
           lea  edi,es:[ebx + S_FYSFS_ROOT->name_fat]
           movzx eax,byte es:[ebx + S_FYSFS_ROOT->namelen]
           add  eax,3
           and  eax,(~3)
           add  edi,eax
get_loop0: mov  eax,es:[edi]
           mov  [esi],eax
           add  edi,4
           add  esi,4
           xor  eax,eax
           test word es:[ebx + S_FYSFS_ROOT->flags],FYSFS_LARGE_FAT
           jz   short @f
           mov  eax,es:[edi]
           add  edi,4
@@:        mov  [esi],eax
           add  esi,4
           loop get_loop0

next_slot_fat:
           mov  ebx,es:[ebx + S_FYSFS_ROOT->fat_continue]
           or   ebx,ebx
           jz   short get_fat_done
           
next_cont_fat:
           shl  ebx,7              ; multiply by 128
           
           mov  edi,S_FYSFS_CONT->crc ; offset of this slots crc
           call chk_crc            ; check the crc of this slot
           jnz  short get_fat_error
           
           cmp  dword es:[ebx + S_FYSFS_CONT->sig],S_FYSFS_CONT_FAT
           jne  short get_fat_error
           
           movzx ecx,byte es:[ebx + S_FYSFS_CONT->count]
           jcxz short get_fat_done
           
           add  ebp,ecx
           
           xor  edi,edi
get_loop1: mov  eax,es:[ebx + edi + S_FYSFS_CONT->name_fat]
           mov  [esi],eax
           add  edi,4
           add  esi,4
           xor  eax,eax
           test word es:[ebx + S_FYSFS_CONT->flags],FYSFS_LARGE_FAT
           jz   short @f
           mov  eax,es:[edi]
           add  edi,4
@@:        mov  [esi],eax
           add  esi,4
           .adsize
           loop get_loop1
           
           mov  ebx,es:[ebx + S_FYSFS_CONT->next]
           or   ebx,ebx
           jnz  short next_cont_fat

get_fat_done:
           mov  eax,ebp
           clc
           ret

get_fat_error:
           stc
           ret
get_fat_entries endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Initialize the crc32 code
crc32_initialize proc near uses alld
           
           ; 256 values representing ASCII character codes.
           xor  ecx,ecx
           mov  edi,offset crc32_table
           
init_loop: mov  ebx,8
           call crc32_reflect
           shl  eax,24
           mov  [edi+ecx*4],eax
           
poly_loop: mov  eax,[edi+ecx*4]
           mov  edx,eax
           shl  edx,1
           mov  edx,04C11DB7h
           jc   short @f
           xor  edx,edx
@@:        shl  eax,1
           xor  eax,edx
           mov  [edi+ecx*4],eax
           dec  ebx
           jnz  short poly_loop
           
           push ecx
           mov  ecx,[edi+ecx*4]
           mov  ebx,32
           call crc32_reflect
           pop  ecx
           mov  [edi+ecx*4],eax
           
           inc  ecx
           cmp  ecx,256
           jb   short init_loop
           
           ret
crc32_initialize endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; Reflection is a requirement for the official CRC-32 standard.
;  You can create CRCs without it, but they won't conform to the standard.
; on entry:
;  ebx = char
;  ecx = reflect
; on exit
;  eax = result
crc32_reflect proc near uses ebx ecx edx

           mov  edx,ecx   ; edx = reflect
           mov  ecx,ebx   ; ecx = char
           mov  ebx,1
           
           xor  eax,eax
           
           ; swap bit 0 for bit 7 bit 1 For bit 6, etc....
reflect_l: shr  edx,1
           jnc  short @f
           
           push ecx
           push edx
           sub  ecx,ebx
           mov  edx,1
           shl  edx,cl
           or   eax,edx
           pop  edx
           pop  ecx
           
@@:        inc  ebx
           cmp  ebx,ecx
           jbe  short reflect_l
           
           ret
crc32_reflect endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; check the CRC of the slot
; on entry
;  es:ebx-> slot
;  edi = offset of crc field
; on exit
;  zero flag set if crc is okay
chk_crc    proc near uses eax ebx ecx
           
           mov  dl,es:[ebx + edi]
           mov  byte es:[ebx + edi],0
           
           mov  eax,0FFFFFFFFh
           mov  ecx,128
           call crc32_partial
           xor  eax,0FFFFFFFFh
           
           ; restore the original crc value
           mov  es:[ebx + edi],dl
           cmp  al,dl
           
           ret
chk_crc    endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; on entry:
;  eax = crc
;  es:ebx -> buffer (slot)
;  ecx = length of buffer
; on exit
;  eax = new crc
crc32_partial proc near uses ebx edx edi
           
           mov  edi,offset crc32_table
           
@@:        mov  edx,eax
           and  edx,0FFh
           xor  dl,es:[ebx]
           inc  ebx
           
           shr  eax,8
           xor  eax,[edi+edx*4]
           
           .adsize
           loop @b
           
           ret
crc32_partial endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; string compare case insensitive
; on entry:
;  ds:si-> string0
;  ds:di-> string1
; on return:
;  al = 0 if equal, -1 if string0[x] < string1[x], 1 if string0[x] > string1[x]
stricmp    proc near uses si di

@@:        lodsb
           call to_upper
           mov  ah,al
           mov  al,[di]
           inc  di
           call to_upper

           cmp  al,ah
           jne  short stricmp_done
           or   al,al
           jnz  short @b
           ret

stricmp_done:
           jb   short @f
           mov  al,1
           ret

@@:        mov  al,-1
           ret
stricmp    endp

to_upper   proc near
           cmp  al,'a'
           jb   short @f
           cmp  al,'z'
           ja   short @f
           sub  al,32
@@:        ret
to_upper   endp


; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  data

os_load_str   db  13,10,'Starting FYSOS...',0

fat_slot_errorS  db 13,10,'Found error when parsing FAT entries...',0
noloaderS        db 13,10,'Did not find loader file...',0

crc32_table      dup (256*sizeof(dword)),0   ; CRC lookup table array.


%print (2000h-$)  ; 5608 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is were the super block is.  As soon as we load
; the remaining 16 sectors, these fields will be filled
; with there corresponding values.
           org 2000h  ; make sure we are at this position
           
.end
