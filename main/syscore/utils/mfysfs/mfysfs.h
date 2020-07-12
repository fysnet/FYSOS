/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
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
#pragma pack (1)

#define SPCLUST  resources->param1

#define FYSFS_ATTR_ARCHIVE   0x00000001
#define FYSFS_ATTR_SUB_DIR   0x00000002
#define FYSFS_ATTR_VOLUME    0x00000004
#define FYSFS_ATTR_SYSTEM    0x00000008
#define FYSFS_ATTR_HIDDEN    0x00000010
#define FYSFS_ATTR_READ_ONLY 0x00000020
#define FYSFS_ATTR_ALL       0x0000003F

char strtstr[] = "\nMFYSFS  v1.10.02    Forever Young Software 1984-2017";

#define FIRST_BITMAP_LSN  17

#define UNOCCUPIED  0 // 00b
#define OCCUPIED    1 // 01b
#define DELETED     2 // 10b
#define SYSTEM      3 // 11b

// 512 byte block starting at sector SUPER_BLOCK_LSN
struct S_FYSFS_SUPER {
  bit32u sig[2];        // signature   'FYSF' 'SUPR' (0x46595346 0x53555052)
  bit16u ver;           // version in BCD (0x0150 = 1.50)
  bit16u sect_clust;    // sectors per cluster (actual) must be a power of 2
  bit8u  encryption;    // type of encryption used (0 = none)
  bit8u  bitmaps;       // number of bitmaps (1 or 2)
  bit16u bitmap_flag;   // flags describing the bitmaps
  bit32u root_entries;  // count of 'slots' in the root directory
  bit64u base_lba;      // physical sector LSN 0 occupies
  bit64u root;          // LSN pointer to root.  Must be in data block
  bit64u data;          // LSN pointer to the first sector of the data block
  bit64u data_sectors;  // count of sectors in data block
  bit64u sectors;       // total sectors in volume
  bit64u bitmap;        // LSN of list of bitmap sectors
  bit64u bitmapspare;   // LSN of list of bitmap sectors (second copy)
  bit32u chkdsk;        // Seconds since 0:00 1-1-1980 when last chkdsk was ran
  bit32u lastopt;       // Seconds since 0:00 1-1-1980 when last optimized
  bit32u flags;         // volume flags
  bit32u crc;           // crc of this (and copies) super block
  struct S_GUID guid;   // serial number in GUID format (see below)
  char   vol_label[64]; // asciiz volume label
  bit8u  filler[250];   // filler (to pad to 512 bytes)
  bit8u  enc_check[90]; // encryption check
};


#define S_FYSFS_ROOT_NEW        0x534C4F54  // 'SLOT'
#define S_FYSFS_CONT_NAME       0x4E414D45  // 'NAME'
#define S_FYSFS_CONT_FAT        0x46415420  // 'FAT '

#define NAME_FS_SPACE 80

// 128 byte block
struct S_FYSFS_ROOT {
  bit32u sig;           // signature of this slot type
  bit32u attribute;     // file attributes
  bit8u  resv[5];       // 
  bit8u  fat_entries;   // fat entries in this directory entry
  bit8u  crc;           // crc of this slot.  8-bit sum
  bit8u  scratch;       // OS scratch byte.  Should be zero when written to disk.
  bit32u created;       // seconds since 00:00:00 1-1-80
  bit32u lastaccess;    // seconds since 00:00:00 1-1-80  (read or written)
  bit64u fsize;         // file size
  bit32u fat_continue;  // next entry that continues the FAT (0 if none)
  bit32u name_continue; // next entry that continues the name (0 if none)
  bit16u flags;         // file flags (encryption, etc)
  bit8u  namelen;       // length of name in this slot
  bit8u  resv1[5];      // 
  bit8u  name_fat[NAME_FS_SPACE]; // name/fat buffer of this entry
};

#define CONT_NAME_SPACE 112

// 128 byte block
struct S_FYSFS_CONT {
  bit32u sig;           // ‘NAME’ for name, ‘FAT ’ for FAT
  bit32u previous;      // pointer to the previous slot in this slot chain
  bit32u next;          // next slot that continues the remainder (0 if none)
  bit8u  count;         // length of name or FAT entries in *this* slot
  bit8u  flags;         // see below
  bit8u  crc;           // crc of this slot.  8-bit sum
  bit8u  scratch;       // scratch byte
  bit8u  name_fat[CONT_NAME_SPACE]; // 
};

/* *********************************************************************************************************
 * The following is used to calculate the CRC's in the SUPER as well as the SLOT's
 */
#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
bit32u crc32_reflect(bit32u, char);
bit32u crc32(void *, bit32u);
void crc32_partial(bit32u *, bit8u *, bit32u);

void parse_command(int, char *[], char *, bool *, bool *, char *);
void create_root_entry(struct S_FYSFS_ROOT *, char *, const bit64u, int *, bit64u *, const int, const bool);
bit8u calc_crc(void *, int);
bit32u get_seconds();

