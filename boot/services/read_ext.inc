comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: read_ext.inc                                                       *
*                                                                          *
* DESCRIPTION:                                                             *
*   Read Sectors Long                                                      *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 10 Apr 2017                                                *
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
; This routine reads in CX sectors using the bios interrupt 13h extended
;  read service. 
; On entry:
;  edx:eax = starting sector in LBA format
;       cx = count of sectors to read
;      ebx = physical address to store data
; NOTE: Since Phoenix BIOS's can't handle a count > 7Fh,
;  we check here and loop if count is > 7Fh.
;
; we need to save all the bytes we can.  Therefore, this routine:
;  - preserves all registers using a single pushad instruction (uses alld)
;  - uses the stack space for the 16-byte disk data block
;  - 
;
; if there is an error, carry set after BIOS call, this routine jumps
;  back to 'BootErr' within the main source file.  The stack is *not*
;  restored...
;
; this also assumes you have the 'boot_data' structure available (globaly)
;
read_sectors_long proc near uses alld
           
read_loop: pushad                  ; save eax, edx, ebx, and cx
           
           ; if > 7Fh, need to do 7Fh at a time
           cmp  cx,7Fh
           jbe  short @f
           mov  cx,7Fh

.if BOOT_DATA_IS_PTR
@@:        add  eax,[boot_data+S_BOOT_DATA->base_lba+0]     ; base lba
           adc  edx,[boot_data+S_BOOT_DATA->base_lba+4]
.else      
@@:        add  eax,[boot_data.base_lba+0]     ; base lba
           adc  edx,[boot_data.base_lba+4]
.endif     
           push edx        ; offset 12
           push eax        ; offset 8
           
           shl  ebx,12
           shr  bx,12
           push ebx        ; offset 4
           
           push cx         ; offset 2
           
           push 10h        ; offset 0
           
           mov  si,sp
           mov  ah,42h             ; read
.if BOOT_DATA_IS_PTR
           mov  dl,[boot_data+S_BOOT_DATA->drive] ; dl = drive
.else      
           mov  dl,boot_data.drive ; dl = drive
.endif     
           int  13h
           
           add  sp,16      ; remove the items from the stack
           
           mov  si,offset diskerrorS
           jc   short BootErr      ; doesn't care that the stack is occupied.  We don't return...
           
           popad                   ; restore eax, edx, ebx, and cx
           
           add  eax,7Fh
           adc  edx,0
           add  ebx,(512 * 7Fh)
           
           sub  cx,7Fh             ;
           jnle short read_loop    ; if ((total - 7Fh) <= 0) then we are done
           
           ret
read_sectors_long endp

diskerrorS    db  13,10,07,'Error reading disk or non-system disk.'
              db  13,10,'Press any key',0

.end
