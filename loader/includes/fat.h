/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat.h                                                              *
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
*  If we modify this file, we need to modify the fat.inc file to match     *
*                                                                          *
***************************************************************************/

#ifndef _FAT_H
#define _FAT_H

// boot_data:
//   root_loc = far pointer to root (seg:off with off in low word and seg in high word of dword)
//  other_loc = far pointer to fat (seg:off with off in low word and seg in high word of dword)

// size in sectors
#define  FAT_ROOTSEG_SIZE  32
#define  FAT_FATSEG_SIZE  127   // 127 * 512 = (65536-512)

#pragma pack(push, 1)

struct S_FAT_ROOT {
  bit8u  name[8];    // name
  bit8u  ext[3];     // ext
  bit8u  attrb;      // attribute
  union U_TYPE {
    bit8u  resv[10];   // reserved in fat12/16
    struct S_FAT32 {
      bit8u  nt_resv;    // reserved for WinNT
      bit8u  crt_time_tenth; // millisecond stamp at creation time
      bit16u crt_time;   // time file was created
      bit16u crt_date;   // date file was created
      bit16u crt_last;   // date file was last accessed
      bit16u strtclsthi; // hi word of FAT32 starting cluster
    } fat32;
  } type;
  bit16u time;       // time
  bit16u date;       // date
  bit16u strtclst;   // starting cluster number
  bit32u filesize;   // file size in bytes
};

/*
struct S_FAT_LFN_ROOT {
  bit8u  sequ_flags;
  bit8u  name0[10];
  bit8u  attrb;
  bit8u  resv;
  bit8u  sfn_crc;
  bit8u  name1[12];
  bit16u clust_zero;
  bit8u  name2[4];
};
*/

bit32u fat_get_next_cluster(struct S_BOOT_DATA *, bit32u);
void convert_fat83(struct S_FAT_ROOT farE *, char *);




#pragma pack(pop)

#endif  // _FAT_H
