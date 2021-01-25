comment |*******************************************************************
*  Copyright (c) 1984-2021    Forever Young Software  Benjamin David Lunt  *
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
*                                                                          *
* Last Updated: 24 Jan 2021                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*                                                                          *
*                                                                          *
***************************************************************************|

; fatseg   (we only load no more than 2 sectors at a time)
;  this gives us enough room for two 4k sectors before ROOTSEG
FATSEG     equ  0E00h          ; segment to load a FAT sector
; rootseg  (lots of room here to load a large root if needed)
ROOTSEG    equ  1000h          ; segment to load root directory

ROOTSEG_SIZE equ 64            ; read up to 64 root sectors 

; number of fats we are including with our image
; Must run ONEFAT.COM on a floppy before it will work
NUM_FATS  equ 1

.end
