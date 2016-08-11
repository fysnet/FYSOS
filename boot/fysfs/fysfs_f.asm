comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs_f.asm                                                        *
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
*          Command line: nbasm fysfs_f<enter>                              *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*  This bootsector is written for a FYSFS floppy disk.                     *
*                                                                          *
* Loading of files:                                                        *
*  The loader file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*                                                                          *
***************************************************************************|

.model tiny                        ;

include boot.inc                   ;
include fysfs.inc

outfile 'fysfs_f.bin'              ; out filename (using quotes will keep it lowercase)

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
           jmp  short BootErr

not386str  db  13,10,'Processor is not a 386 compatible processor.',0

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; We now can use 386+ code (still in real mode though)
;
.386P   ; allow processor specific code for the 386
@@:        popf                   ; restore the interrupt bit

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  We need to call the BIOS to get the parameters of the current disk.
;  so we can pass it on to loader.sys.  We set it to defaults incase of
;  bad BIOS.
           mov  word boot_data.sect_size,512   ; assumes 512 bytes/sector
           mov  word boot_data.Heads,2         ; must floppies have 2 heads
           mov  word boot_data.SecPerTrack,18  ; assumes 1.44 disk
           
           mov  ah,08h         ; get parameters
           mov  dl,boot_data.drive
           xor  di,di          ; to check for bad BIOS
           mov  es,di          ;
           int  13h            ;
           mov  ax,es          ; if es:di -> 0000:0000h, then it has a bad BIOS
           or   ax,di          ;  skip the initialization below
           jz   short @f       ;
           
           mov  ax,128
           mov  cl,es:[di+S_INT1E_PARMS->sect_size]
           shl  ax,cl  ; cl = (00h = 128, 01h = 256, 02h = 512, 03h = 1024)
           mov  boot_data.sect_size,ax
           movzx ax,byte es:[di+S_INT1E_PARMS->sect_trck]
           mov  boot_data.SecPerTrack,ax
           
; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
;  The next task is to load the rest of the boot code
           push ds
           pop  es
           
           ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
           ; read in 16 more sectors from base + 1
@@:        mov  bx,0200h       ; address 0x07E00
           mov  ax,1           ;
           xor  dx,dx
           mov  cx,16          ; make sure we get the super_block too
           call read_sectors
           
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
; This routine reads in CX sectors using the bios interrupt 13h service
; Since we don't know if the current bios can span heads and cylinders,
;  with multiple counts in CX, we will read one sector at a time, manually
;  updating DH, and CX as we go.
; On entry:
;  DX:AX = starting sector in LBA format
;  ES:BX ->segment:offset to read to
;     CX = count of sectors to read
read_sectors proc near uses ax bx cx dx
           add  ax,[boot_sig.base_lba+0]   ; add base lba
           adc  dx,[boot_sig.base_lba+2]   ; 
read_loop: pusha                   ; save dx:ax, cx, bx
           call lba_to_chs         ; dx:ax(lba) -> (ax/cx/dx)(chs)
           mov  ax,0003h           ; three try's
chck_loop: push ax
           mov  dl,boot_data.drive ; dl = drive
           mov  ax,0201h
           int  13h                ; do the read/write
           pop  ax
           jnc  short int13_no_error
           pusha                   ; save cx & dx
           xor  ah,ah
           int  13h                ; reset disk
           popa
           dec  ax
           jnz  short chck_loop    ; try again

           mov  si,offset diskerrorS ; loading message
           jmp  short BootErr      ; do error display and reboot

int13_no_error:
           popa                    ; restore dx:ax, cx, bx
           add  bh,2               ; point to next position
           add  ax,1               ; inc to next LBA sector
           adc  dx,0               ;
           loop read_loop          ; decrement counter, read another one

           ret
read_sectors endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine converts LBA (DX:AX) to CHS
; Sector   = (LBA mod SPT)+1
; Head     = (LBA  /  SPT) mod Heads
; Cylinder = (LBA  /  SPT)  /  Heads
;    (SPT = Sectors per Track)
lba_to_chs proc near               ; dx:ax = LBA
           mov  cx,boot_data.SecPerTrack  ; sectors per track 
           call div32_16           ; dx:ax = dx:ax / cx  with cx = remdr.
           push cx                 ; save sectors
           mov  cx,boot_data.Heads ; heads per cylinder
           call div32_16           ; dx:ax = dx:ax / cx  with cx = remdr.
           pop  dx                 ; dx = sector
           inc  dx                 ; sectors are one (1) based
           mov  dh,cl
           xchg ah,al
           xchg cx,ax
           ror  cl,2
           and  cl,11000000b
           and  dl,00111111b
           or   cl,dl
           ret
lba_to_chs endp

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; dx:ax = dx:ax / cx  with cx = remainder
div32_16   proc near uses bx
           mov  bx,dx              ;
           xchg bx,ax              ;
           xor  dx,dx              ;
           div  cx                 ; divide high order first
           xchg bx,ax              ;
           div  cx                 ; remainder will be in dx
           mov  cx,dx              ;  save remainder
           mov  dx,bx              ; full 32-bit answer
           ret                     ;
div32_16   endp

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

%print (512-$-2)  ; 85 bytes here

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
           mov  ax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 0]
           mov  dx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root + 2]
           mov  cx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->root_entries]   ; max root entries allowed
           shr  cx,2               ; div by 4
           cmp  cx,FYSFS_ROOTSEG_SIZE ; limit the read to FYSFS_ROOTSEG_SIZE
           jbe  short @f           ;
           mov  cx,FYSFS_ROOTSEG_SIZE
@@:        call read_sectors       ; read in the ROOT directory

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
           ;  We now have the count of entries in EAX and
           ;  the list of entries in ds:edi
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
           add  ax,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 0]
           adc  dx,[FYSFS_SUPER_BLOCK + S_FYSFS_SUPER->data + 2]
           call read_sectors       ; read in cluster (saves all registers used)
           
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

%print (2000h-$)  ; 5615 bytes here

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; this is were the super block is.  As soon as we load
; the remaining 16 sectors, these fields will be filled
; with there corresponding values.
           org 2000h  ; make sure we are at this position

.end
