comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: embr.inc                                                           *
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
*   embr EQUates                                                           *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************|

ENTRY_VALID    equ  (1<<0)
ENTRY_HIDE     equ  (1<<1)

S_EMBR     struct
  sig0                dword  ; 'EMBR'
  crc                 dword  ; crc of this header and all entries
  entry_count          word  ; total entries in EMBR
  boot_delay           byte  ; seconds to delay before automatic boot (reserved in all but first sector)
  resv1               dup 17 ;
  sig1                dword  ; 'RBME'
S_EMBR      ends


S_EMBR_ENTRY struct
  flags               dword  ;
  signature           dword  ;
  starting_sector     qword  ;
  sector_count        qword  ;
  description         dup 64 ;
  date_created        qword  ;
  date_last_booted    qword  ;
  OS_signature        qword  ;
  reserved            dup 16 ;
S_EMBR_ENTRY ends

S_INT13_PARMS struct
  buf_size             word  ; caller: size of buffer (1Ah)
  flags                word  ; info flags
  cyls                dword  ; number of physical cyls on disk
  heads               dword  ; number of physical heads on disk
  sect_trck           dword  ; number of physical sectors per track on disk
  tot_sects           qword  ; total number of sectors
  sector_size          word  ; bytes per sectors
S_INT13_PARMS ends


.end
