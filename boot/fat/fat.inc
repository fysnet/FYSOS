comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat.inc                                                            *
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
*   EQUates for fatxx.asm                                                  *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm fat1x<enter>                                *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*   *** Assumes this is for a hard drive ***                               *
*                                                                          *
***************************************************************************|

; all three of these are placed so that we don't have to check for
;  64k boundary read checks from the BIOS
;  (ROOTSEG should be set on page boundary)
;  (FATSEG and LOADSEG should be set on 64k boundary)
;
; rootseg  16k = 32 sectors. Maximum of 512 root directory entries
ROOTSEG    equ  0C00h          ; segment to load root dir to (0x0C000 = 48k)
; fatseg   128k  This allows for the maximum of 256 sectors (65535 entries)
FATSEG     equ  1000h          ; segment to load FAT to      (0x10000 = 64k)

; size in sectors
ROOTSEG_SIZE  equ   32
FATSEG_SIZE   equ  127   ; 127 * 512 = (65536-512)

; number of fats we are including with our image
; Must run ONEFAT.COM on a floppy before it will work
NUM_FATS  equ 1

.end
