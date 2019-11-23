comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: lba2chs.inc                                                        *
*                                                                          *
* DESCRIPTION:                                                             *
*   Convert LBA (eax) to CHS                                               *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 22 May 2017                                                *
*                                                                          *
*   Requires 32-bit and above x86 processor                                *
*                                                                          *
***************************************************************************|

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; if we have included this after the first sector, we are in error...
.ifndef SKIP_200h_CHECK
.ifa $, 200h
%error 1, 'This must be in the first sector of the code...'
.endif
.endif

; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; This routine converts LBA (EAX) to CHS
; Sector   = (LBA mod SPT)+1
; Head     = (LBA  /  SPT) mod Heads
; Cylinder = (LBA  /  SPT)  /  Heads
;    (SPT = Sectors per Track)
; on entry:
;  eax = LBA
; on return
;  DX and CX = BIOS formatted CHS values
;   cl = sector (1 based) (lower 6 bits)
;   ch = cyl (high 2 bits of cl also)
;   dh = head
; assumptions:
;   eax <= 0x7FFFFFFF (i.e.: bit 31 = clear)
;   (0x7FFFFFFF = 2,147,483,647)
;   If we are having to use this service, LBA will definately
;    be less than this.
lba_to_chs proc near
           cdq
           ; edx:eax = LBA
           
           movzx ecx,word SecPerTrack ; sectors per track
           div  ecx                ; eax = edx:eax / ecx  with edx = remdr.
           ; eax = (LBA / SPT)
           ; edx = remainder (zero based sector)
           
           push dx                 ; save zero based sector
           
           cdq
           ; edx:eax = (LBA / SPT)
           
           movzx ecx,word Heads    ; heads per cylinder
           div  ecx                ; eax = edx:eax / ecx  with edx = remdr.
           ; eax = cylinder
           ; edx = head
           
           mov  dh,dl              ; save head number in dh
           ; dh = head
           
           pop  cx                 ; cx = zero based sector
           inc  cx                 ; sectors are one (1) based
           ; cx = 1 based sector
           
           ; eax = cylinder
           mov  ch,al
           shl  ah,6
           or   cl,ah
           ; cx = LLLLLLLLHHSSSSSS  (L = low cyl 8 bits, H = high 2 bits, S = 1 based sector number)
           
           ret
lba_to_chs endp

; we assume 2 and 18 respectively
Heads         dw   2
SecPerTrack   dw  18

.end
