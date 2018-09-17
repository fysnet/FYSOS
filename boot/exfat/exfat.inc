comment |*******************************************************************
*  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: exfat.inc                                                          *
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
*   exfat boot code                                                        *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm exfat<enter>                                *
*                                                                          *
* Last Updated: 17 Sept 2018                                               *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

; all three of these are placed so that we don't have to check for
;  64k boundary read checks from the BIOS
;  (ROOTSEG should be set on page boundary)
;  (FATSEG and LOADSEG should be set on 64k boundary)
;
; rootseg  16k = 32 sectors. Maximum of 512 root directory entries
FAT_ROOTSEG    equ  0C00h          ; segment to load root dir to (0x0C000 = 48k)
; fatseg   128k  This allows for the maximum of 256 sectors (65535 entries)
FAT_FATSEG     equ  1000h          ; segment to load FAT to      (0x10000 = 64k)

; size in sectors
FATSEG_SIZE   equ  127   ; 127 * 512 = (65536-512)

EXFAT_DIR_EOD        equ  0x00  ; end of directory
EXFAT_DIR_ENTRY      equ  0x85
EXFAT_DIR_STRM_EXT   equ  0xC0
EXFAT_DIR_NAME_EXT   equ  0xC1


S_EXFAT_ROOT struct
  entry_type   byte   ; entry type
  content      dup 31 ; content of entry (see below)
S_EXFAT_ROOT ends

S_EXFAT_DIR  struct
  entry_type   byte   ; entry type
  sec_count    byte   ; secondary count
  crc          word   ; crc
  attributes   word   ; attribs
  resv1        word   ;
  created     dword   ; 
  last_mod    dword   ;
  last_acc    dword   ;
  create_ms    byte   ;
  last_mode_ms byte   ;
  create_tz    byte   ;
  last_mod_tz  byte   ;
  last_acc_tz  byte   ;
  resv2       dup 7   ;
S_EXFAT_DIR  ends

S_EXFAT_STRM struct
  entry_type   byte   ; entry type
  flags        byte   ;
  resv1        byte   ;
  name_len     byte   ;
  name_hash    word   ;
  resv2        word   ;
  valid_data_len qword ;
  resv3       dword   ;
  first_clust dword   ;
  data_len    qword   ;
S_EXFAT_STRM ends

S_EXFAT_NAME struct
  entry_type   byte   ; entry type
  flags        byte   ;
  name        dup 30  ; 15 words
S_EXFAT_NAME ends

.end
