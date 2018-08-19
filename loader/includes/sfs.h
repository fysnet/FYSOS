/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#ifndef _SFS_H
#define _SFS_H

#include "loader.h"
#include "sfs.h"

#pragma pack(push, 1)

#define  SFS_SUPER_MAGIC  0x534653
#define  SFS_VERSION          0x1A

struct S_SFS_SUPER {
  bit32u time_stamp[2];
  bit32u data_block_count[2]; // Size of data area in blocks
  bit32u index_size[2];       // Size of index area *in bytes*
  bit32u magic_version;       // Magic number (0x534653) + SFS version (0x10 for Version 1.0)
  bit32u total_blocks[2];     // Total number of blocks in volume
  bit32u resv_blocks;         // Number of reserved blocks
  bit8u  block_size;          // Block size (2^(x+7) where x = 2 = 512)
  bit8u  crc;                 // zero byte check sum of super block
};

// these all must be 0x1F or less so that it does not interfere with
//  valid filename chars 0x20+
#define SFS_ENTRY_VOL_ID    0x01  // volume ID
#define SFS_ENTRY_START     0x02  // start marker
#define SFS_ENTRY_UNUSED    0x10  // unused
#define SFS_ENTRY_DIR       0x11  // directory entry
#define SFS_ENTRY_FILE      0x12  // file entry
#define SFS_ENTRY_UNUSABLE  0x18  // unusable entry (bad sector(s))
#define SFS_ENTRY_DIR_DEL   0x19  // deleted directory
#define SFS_ENTRY_FILE_DEL  0x1A  // deleted file

#define SFS_ENTRY_SIZE   64

#define DIR_NAME_LEN  53
struct S_SFS_DIR {
  bit8u  type;        // 0x11
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit32u time_stamp[2];
  bit8u  name[DIR_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

#define FILE_NAME_LEN  29
struct S_SFS_FILE {
  bit8u  type;        // 0x12
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit32u time_stamp[2];
  bit32u start_block[2]; // starting block in data area
  bit32u end_block[2];   // end block in data area
  bit32u file_len[2];    // file length in bytes
  bit8u  name[FILE_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

#pragma pack(pop)

struct S_SFS_DATA {
  void *super;
  bit32u data_start;
  void *ida;
  int  ida_idx;
  int  ida_entries;
};


bool sfs_load_data(struct S_SFS_DATA *);
bool sfs_check_crc(const void *, int);
void sfs_convert_path(char *);

#endif   // _LEANFS_H
