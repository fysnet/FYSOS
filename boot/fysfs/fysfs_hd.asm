comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs_hd.asm                                                       *
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
*   EQUates for fysfs.asm                                                  *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm fysfs_hd<enter>                             *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*   This bootsector is written for a FYSFS hard disk.                      *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include boot.inc                   ;
include fysfs.inc

outfile 'fysfs_hd.bin'             ; out filename (using quotes will keep it lowercase)

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

%print (512-$-2)  ; 86 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  End of 1st sector. Pad out to fill 512 bytes, including final word 0xAA55
              org (200h-16)
boot_sig      st  S_BOOT_SIG uses 46595342h, 0, 0, 0AA55h

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
           mov  eax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 0]
           mov  edx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 4]
           mov  ecx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root_entries]   ; max root entries allowed
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
           mov  ecx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root_entries]  ; count of entries to search
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
@@:        movzx ecx,word [FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->sect_clust]   ; number of sectors to read (per cluster)
           push LOADSEG            ; es-> loader seg
           pop  es                 ;
           xor  bx,bx              ; es:bx = offset to load the data
           
           mov  ebp,eax            ; count of clusters to read
           mov  si,FYSFS_BUFFER
@@:        mov  eax,[si+0]
           mov  edx,[si+4]
           add  si,sizeof(qword)
           mul  ecx                ; cluster based
           add  eax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 0]
           adc  edx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 4]
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
           
           mov  byte boot_data.file_system,FYSFS
           
           mov  eax,((FYSFS_ROOTSEG << 16) | 0)
           mov  boot_data.root_loc,eax
           mov  eax,((007C0h << 16) | FYSFS_SUPER_BLOCK)
           mov  boot_data.other_loc,eax
           xor  ax,ax
           mov  boot_data.misc0,ax
           mov  boot_data.misc1,ax
           mov  boot_data.misc2,ax
           
           ; jump to the loader
           push LOADSEG            ; LOADSEG:0000h for RETF below
           push 0000h              ;
           retf                    ; jump to LOADSEG:0000h

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; included the common items

include fysfs_common.inc

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

%print (2000h-$)  ; 5603 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is were the super block is.  As soon as we load
; the remaining 16 sectors, these fields will be filled
; with there corresponding values.
           org 2000h  ; make sure we are at this position

.end
