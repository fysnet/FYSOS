comment /******************************************************************\
*                            FYS OS version 2.0                            *
*                         Forever Young Software                           *
*                        Copyright (c) 1984-2019                           *
*                                                                          *
* FILE: cd_menu.inc                                                        *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* EQUates for cd_menu.asm                                                  *
*                                                                          *
*  Built with:  NBASM ver 00.26.xx                                         *
*                 http:\\www.fysnet.net\newbasic.htm                       *
*                                                                          *
* Last Update:  22 Sept 2018                                               *
*                                                                          *
***************************************************************************/

; initial stack location
STACK_OFFSET  equ 0FFFEh        ; first push at 07C0:FFFE

; count of entries in window at one time, must be at least 2
TOTAL_DISPLAY equ  7

; 'El Torito' Boot Descriptor
S_BRVD       struct
  id              byte   ; 0 = boot
  ident           dup 5  ; 'CD001'
  version         byte   ; 1
  bsident         dup 32 ;
  resv0           dup 32 ;
  boot_cat_lba   dword   ; LBA of Boot Catalog
  resv1           dup 1973
S_BRVD       ends

; Boot Catalog Validation Entry
S_VAL_ENTRY  struct
  id              byte   ; 1
  platform        byte   ; 0, 1, 2
  resv0           word   ;
  ident           dup 24 ;
  crc             word   ; crc
  key55           byte   ;
  keyAA           byte   ;
S_VAL_ENTRY  ends

S_SECT_HDR   struct
  id              byte   ; 0x90 = no more entries, 0x91 = more entries follow
  platform        byte   ; platform values for entries in this section
  num             word   ; number of entries in this section
  ident           dup 28 ;
S_SECT_HDR   ends

S_SECT_ENTRY struct
  bootable        byte   ;
  media           byte   ;
  load_seg        word   ;
  sys_type        byte   ;
  resv0           byte   ;
  load_cnt        word   ;
  load_rba       dword   ;
  criteria        byte   ;
  vendor_spec     dup 19 ;
S_SECT_ENTRY ends

S_SECT_EXTRA struct
  indicator       byte   ;
  next            byte   ;
  vendor_spec     dup 30 ;
S_SECT_ENTRY ends

; allowed total entries
; must be a multiple of 64
ALLOWED_ENTRIES equ 128

; allowed extension entries per entry
ALLOWED_EXTRAS  equ 6

S_OUR_ENTRY  struct
  bootable        byte   ;
  media           byte   ;
  load_seg        word   ;
  sys_type        byte   ;
  platform        byte   ;
  load_cnt        word   ;
  load_rba       dword   ;
  criteria        byte   ;
  vendor_spec     dup (19 + (ALLOWED_EXTRAS * 30))
S_OUR_ENTRY  ends

VEND_SPEC_LINES equ 8
VEND_SPEC_CHARS equ 16  ; must be a multiple of 2 (even number)

.end
