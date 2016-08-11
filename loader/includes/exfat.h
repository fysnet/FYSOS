/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: exfat.h                                                            *
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
*   #defines for fat.c                                                     *
*                                                                          *
* BUILT WITH:   NewBasic Compiler and Assembler                            *
*                 http://www.fysnet/newbasic.htm                           *
*               NBC   ver 00.20.25                                         *
*          Command line: nbc loader<enter>                                 *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm loader loader.sys -d<enter>                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*  If we modify this file, we need to modify the exfat.inc file to match   *
*                                                                          *
***************************************************************************/

#ifndef _EXFAT_H
#define _EXFAT_H

// all three of these are placed so that we don't have to check for
//  64k boundary read checks from the BIOS
//  (ROOTSEG should be set on page boundary)
//  (FATSEG should be set on 64k boundary)

// rootseg  16k = 32 sectors. Maximum of 512 root directory entries
#define EXFAT_ROOTSEG   0x00C00  // segment to load root dir to (0x0C000 = 48k)
#define EXFAT_FATSEG    0x1000   // segment to load FAT to      (0x10000 = 64k)


#pragma pack(push, 1)





#pragma pack(pop)

#endif  // _EXFAT_H
