/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2019
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

// set it to 1 (align on byte)
#pragma pack (push, 1)

char strtstr[] = "\nMKSFS  v2.00.00    Forever Young Software 1984-2019";

// at offset 0x18E in first part of partition (first sector on floppy, etc)
struct S_SFS_SUPER {
  bit64s time_stamp;
  bit64u data_block_count;   // Size of data area in blocks
  bit64u index_size;         // Size of index area *in bytes*
  bit32u magic_version;      // Magic number (0x534653) + SFS version (0x10 for Version 1.0)
  bit64u total_blocks;       // Total number of blocks in volume
  bit32u resv_blocks;        // Number of reserved blocks
  bit8u  block_size;         // Block size (2^(x+7) where x = 2 = 512)
  bit8u  crc;                // zero byte check sum of super block
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

#define VOLID_NAME_LEN  52
struct S_SFS_VOL_ID {
  bit8u  type;        // 0x01
  bit8u  crc;         // zero sum byte check sum
  bit16u resvd;       // reserved
  bit64s time_stamp;  // time of media format/volume creation
  bit8u  name[52];    // UTF-8 null terminated
};

struct S_SFS_START {
  bit8u  type;        // 0x02
  bit8u  crc;         // zero sum byte check sum
  bit8u  resvd[62];   // reserved
};

struct S_SFS_UNUSED {
  bit8u  type;        // 0x10
  bit8u  crc;         // zero sum byte check sum
  bit8u  resvd[62];   // reserved
};

#define DIR_NAME_LEN  53
struct S_SFS_DIR {
  bit8u  type;        // 0x11
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit64s time_stamp;  // 
  bit8u  name[DIR_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

#define FILE_NAME_LEN  29
struct S_SFS_FILE {
  bit8u  type;        // 0x12
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit64s time_stamp;  // 
  bit64u start_block; // starting block in data area
  bit64u end_block;   // end block in data area
  bit64u file_len;    // file length in bytes
  bit8u  name[FILE_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

struct S_SFS_UNUSABLE {
  bit8u  type;        // 0x18
  bit8u  crc;         // zero sum byte check sum
  bit8u  resv0[8];    // reserved
  bit64u start_block; // starting block in data area
  bit64u end_block;   // end block in data area
  bit8u  resv1[38];   // reserved
};

struct S_SFS_DIR_DEL {
  bit8u  type;        // 0x19
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit64s time_stamp;  // 
  bit8u  name[DIR_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

struct S_SFS_FILE_DEL {
  bit8u  type;        // 0x1A
  bit8u  crc;         // zero sum byte check sum
  bit8u  num_cont;    // number of cont slots
  bit64s time_stamp;  // 
  bit64u start_block; // starting block in data area
  bit64u end_block;   // end block in data area
  bit64u file_len;    // file length in bytes
  bit8u  name[FILE_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};


void parse_command(int, char *[], char *, char *);
bit64s get_64kseconds();
bit8u calc_crc(void *ptr, int cnt);

#pragma pack (pop)
