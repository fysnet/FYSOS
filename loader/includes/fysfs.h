/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs.h                                                            *
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
*   #defines for fysfs.c                                                   *
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
*  If we modify this file, we need to modify the fysfs.inc file to match   *
*                                                                          *
***************************************************************************/

#ifndef _FYSFS_H
#define _FYSFS_H

// boot_data:
//   root_loc = far pointer to root (seg:off with off in low word and seg in high word of dword)
//  other_loc = far pointer to super (seg:off with off in low word and seg in high word of dword) (seg: is assumed same a root_loc)


#pragma pack(push, 1)

// 512 byte block starting at sector SUPER_BLOCK_LSN
struct S_FYSFS_SUPER {
  bit32u sig[2];        // signature   'FYSF' 'SUPR' (0x46595346 0x53555052)
  bit16u ver;           // version in BCD (0x0150 = 1.50)
  bit16u sect_clust;    // sectors per cluster (actual) must be a power of 2
  bit8u  encryption;    // type of encryption used (0 = none)
  bit8u  bitmaps;       // number of bitmaps (1 or 2)
  bit16u bitmap_flag;   // flags describing the bitmaps
  bit32u root_entries;  // count of 'slots' in the root directory
  bit32u base_lba[2];   // physical sector LSN 0 occupies
  bit32u root[2];       // LSN pointer to root.  Must be in data block
  bit32u data[2];       // LSN pointer to the first sector of the data block
  bit32u data_sectors[2]; // count of sectors in data block
  bit32u sectors[2];    // total sectors in volume
  bit32u bitmap[2];     // LSN of list of bitmap sectors
  bit32u bitmapspare[2];// LSN of list of bitmap sectors (second copy)
  bit32u chkdsk;        // Seconds since 0:00 1-1-1980 when last chkdsk was ran
  bit32u lastopt;       // Seconds since 0:00 1-1-1980 when last optimized
  bit32u flags;         // volume flags
  bit32u crc;           // crc of this (and copies) super block
  struct S_GUID guid;   // serial number in GUID format (see below)
  char   vol_label[64]; // asciiz volume label
  bit8u  filler[250];   // filler (to pad to 512 bytes)
  bit8u  enc_misc[10];  // encryption misc data area (RC4 10 byte IV) Should only be initialize once
  bit8u  enc_check[80]; // encryption check (updated each commit_super() to be sure it is correct
};


#define S_FYSFS_ROOT_NEW   0x534C4F54  // 'SLOT'
#define S_FYSFS_CONT_NAME  0x4E414D45  // 'NAME'
#define S_FYSFS_CONT_FAT   0x46415420  // 'FAT '

#define NAME_FAT_SPACE 80

// 128 byte block
struct S_FYSFS_ROOT {
  bit32u sig;           // signature of this slot type
  bit32u attribute;     // file attributes
  bit8u  resv[5];       // 
  bit8u  fat_entries;   // fat entries in this directory entry
  bit8u  crc;           // crc of this slot.
  bit8u  scratch;       // OS scratch byte.  Should be zero when written to disk.
  bit32u created;       // seconds since 00:00:00 1-1-80
  bit32u lastaccess;    // seconds since 00:00:00 1-1-80  (read or written)
  bit32u fsize[2];      // file size
  bit32u fat_continue;  // next entry that continues the FAT (0 if none)
  bit32u name_continue; // next entry that continues the name (0 if none)
  bit16u flags;         // file flags (etc)
  bit8u  namelen;       // length of name in this slot
  bit8u  resv1[5];      // 
  // align to 32-bits
    // name starts here (up to 255 8-bit chars)
    // name is zero padded to 32-bits
    // fat starts here
  bit8u  name_fat[NAME_FAT_SPACE];
};

// set when using 64-bit fat_entries
#define CONT_FLAGS_LARGE    0x01   // ignored in the name_cont slot
#define CONT_NAME_FAT_SPACE 112

// 128 byte block
struct S_FYSFS_CONT {
  bit32u sig;           // ‘NAME’ for name, ‘FAT ’ for FAT
  bit32u previous;      // pointer to the previous slot in this slot chain
  bit32u next;          // next slot that continues the remainder (0 if none)
  bit8u  count;         // length of name or FAT entries in *this* slot
  bit8u  flags;         // see below
  bit8u  crc;           // crc of this slot.
  bit8u  scratch;       // scratch byte
  bit8u  name_fat[CONT_NAME_FAT_SPACE];
};

#pragma pack(pop)

bool fysfs_get_name(const int, char *, const struct S_FYSFS_ROOT farE *);
bool fysfs_good_crc(void *);
int fysfs_get_fat_entries(int, bit32u *, const struct S_FYSFS_ROOT farE *);





#endif  // _FYSFS_H

