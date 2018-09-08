comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: read_chs.inc                                                       *
*                                                                          *
* DESCRIPTION:                                                             *
*   Read Sectors using CHS                                                 *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 22 May 2017                                                *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we have included this after the first sector, we are in error...
.ifa $, 200h
%error 1, 'This must be in the first sector of the code...'
.endif

; if we haven't defined BOOT_DATA_IS_PTR, we assume it to be zero
.ifndef BOOT_DATA_IS_PTR
BOOT_DATA_IS_PTR  equ  0
.endif

.if (_VER < 2673h)
%error 1, 'This source requires NBASM version 00.26.73 or higher'
.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine reads in CX sectors using the bios interrupt 13h service
; Since we don't know if the current bios can span heads and cylinders,
;  with multiple counts in CX, we will read one sector at a time, manually
;  updating DH, and CX as we go.
; On entry:
;   EAX = starting sector in LBA format
;   EBX ->physical address to read to
;    CX = count of sectors to read
read_sectors proc near uses eax ebx ecx edx es
.if BOOT_DATA_IS_PTR
           add  eax,[boot_data+S_BOOT_DATA->base_lba]     ; add base lba
.else      
           add  eax,[boot_data.base_lba]     ; add base lba
.endif     
read_loop: push eax
           push cx
           push ebx
           
           ; ebx = physical address.  We need
           ;  to convert to a segmented address.
           push bx
           shr  ebx,4
           mov  es,bx
           pop  bx
           and  bx,0Fh
           
           call lba_to_chs         ; eax(lba) -> (ax/cx/dx)(chs)
           mov  ax,0003h           ; three try's
chck_loop: push ax
.if BOOT_DATA_IS_PTR
           mov  dl,[boot_data+S_BOOT_DATA->drive] ; dl = drive
.else      
           mov  dl,boot_data.drive ; dl = drive
.endif     
           mov  ax,0201h
           int  13h                ; do the read/write
           pop  ax
           jnc  short int13_no_error
           push cx
           push dx
           xor  ah,ah
           int  13h                ; reset disk
           pop  dx
           pop  cx
           dec  ax
           jnz  short chck_loop    ; try again
           
           mov  si,offset diskerrorS
           jmp  short BootErr      ; do error display and reboot
           
int13_no_error:
           ; calculate next position
           pop  ebx
           add  ebx,512            ; assumes all sectors are 512 bytes each
           
           pop  cx                 ; restore sector count
           pop  eax                ; restore current LBA
           inc  eax                ; inc to next LBA sector
           loop read_loop          ; decrement counter, read another one
           
           ret
read_sectors endp

diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press any key',0

.end
