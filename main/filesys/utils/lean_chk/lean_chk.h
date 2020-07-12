/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2014
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
#pragma pack(push, 1)

#define MAX_PATH 260

char strtstr[] = "\nLEAN_CHK  v2.00.00    Forever Young Software 1984-2019";

#define REPAIR_FS_CHK_ONLY  0x00000001
#define REPAIR_FS_REPAIR    0x00000002
#define REPAIR_FS_OPTIMIZE  0x00000004
#define REPAIR_FS_VERBOSE   0x80000000

#define LEAN_SUPER_MAGIC     0x4E41454C
#define LEAN_INODE_MAGIC     0x45444F4E
#define LEAN_INDIRECT_MAGIC  0x58444E49

struct S_LEAN_SUPER {
  bit32u checksum;                // bit32u sum of all fields.
  bit32u magic;                   // 0x4E41454C ('LEAN')
  bit16u fs_version;              // 0x0006 = 0.6
  bit8u  pre_alloc_count;         // count minus one of contiguous sectors that driver should try to preallocate
  bit8u  log_sectors_per_band;    // 1 << log_sectors_per_band = sectors_per_band. Valid values are 12, 13, 14, ...
  bit32u state;                   // bit 0 = unmounted?, bit 1 = error?
  struct S_GUID guid;             // Globally Unique IDentifier
  bit8u  volume_label[64];        // can be modified by the LABEL command
  bit64u sector_count;            // The total number of sectors that form a file system volume
  bit64u free_sector_count;       // The number of free sectors in the volume. A value of zero means disk full.
  bit64u primary_super;           // sector number of primary super block
  bit64u backup_super;            // sector number of backup super block
  bit64u bitmap_start;            // This is the address of the sector where the first bands bitmap starts
  bit64u root_start;              // This is the address of the sector where the root directory of the volume starts, the inode number of the root directory.
  bit64u bad_start;               // This is the address of the sector where the pseudo-file to track bad sectors starts.
  bit8u  reserved[360];           // zeros
};

#define LEAN_INDIRECT_EXTENT_CNT  38
struct S_LEAN_INDIRECT {
  bit32u checksum;                // bit32u sum of all fields before this one.
  bit32u magic;                   // 0x58444E49 ('INDX')
  bit64u sector_count;            // total number of sectors addressable using this indirect sector.
  bit64u inode;                   // the inode number of this file this indirect sector belongs to.
  bit64u this_sector;             // The address of the sector storing this indirect sector.
  bit64u prev_indirect;           // the address of the previous indirect sector.
  bit64u next_indirect;           // the address of the next indirect sector.
  bit8u  extent_count;            // The number of valid extent specifications storing in the indirect struct.
  bit8u  reserved0[3];            // reserved
  bit32u reserved1;               // reserved
  bit64u extent_start[LEAN_INDIRECT_EXTENT_CNT]; // The array of extents
  bit32u extent_size[LEAN_INDIRECT_EXTENT_CNT];
};

#define S_LEAN_INODE_SIZE     176
#define LEAN_INODE_EXTENT_CNT   6
#define LEAN_DATA_OFFSET      512   // data starts at offset 512 from start of inode

// 176 bytes each
struct S_LEAN_INODE {
  bit32u checksum;                // bit32u sum of all fields before this one.
  bit32u magic;                   // 0x45444F4E  ('NODE')
  bit8u  extent_count;            // count of extents in this inode struct.
  bit8u  inode_size;              // Inode size in dwords (176 / 4)
  bit16u reserved;                // reserved
  bit32u indirect_count;          // number of indirect sectors owned by file
  bit32u links_count;             // The number of hard links (the count of directory entries) referring to this file, at least 1
  bit32u uid;                     // currently reserved, set to 0
  bit32u gid;                     // currently reserved, set to 0
  bit32u attributes;              // see table below
  bit64u file_size;               // file size
  bit64u sector_count;            // count of sectors used
  bit64s acc_time;                // last accessed: number of mS elapsed since midnight of 1970-01-01
  bit64s sch_time;                // status change: number of mS elapsed since midnight of 1970-01-01
  bit64s mod_time;                // last modified: number of mS elapsed since midnight of 1970-01-01
  bit64s cre_time;                //       created: number of mS elapsed since midnight of 1970-01-01
  bit64u first_indirect;          // address of the first indirect sector of the file.
  bit64u last_indirect;           // address of the last indirect sector of the file.
  bit64u fork;
  bit64u extent_start[LEAN_INODE_EXTENT_CNT]; // The array of extents
  bit32u extent_size[LEAN_INODE_EXTENT_CNT]; 
};

//attributes:
#define  LEAN_ATTR_IXOTH        (1 << 0)  // Other: execute permission 
#define  LEAN_ATTR_IWOTH        (1 << 1)  // Other: write permission 
#define  LEAN_ATTR_IROTH        (1 << 2)  // Other: read permission 
#define  LEAN_ATTR_IXGRP        (1 << 3)  // Group: execute permission 
#define  LEAN_ATTR_IWGRP        (1 << 4)  // Group: write permission 
#define  LEAN_ATTR_IRGRP        (1 << 5)  // Group: read permission 
#define  LEAN_ATTR_IXUSR        (1 << 6)  // Owner: execute permission 
#define  LEAN_ATTR_IWUSR        (1 << 7)  // Owner: write permission 
#define  LEAN_ATTR_IRUSR        (1 << 8)  // Owner: read permission 
//       LEAN_ATTR_             (1 << 9)  // reserved
#define  LEAN_ATTR_ISUID        (1 << 10) // Other: execute as user id
#define  LEAN_ATTR_ISGID        (1 << 11) // Other: execute as group id 
#define  LEAN_ATTR_HIDDEN       (1 << 12) // Don't show in directory listing 
#define  LEAN_ATTR_SYSTEM       (1 << 13) // Warn that this is a system file 
#define  LEAN_ATTR_ARCHIVE      (1 << 14) // File changed since last backup 
#define  LEAN_ATTR_SYNC_FL      (1 << 15) // Synchronous updates 
#define  LEAN_ATTR_NOATIME_FL   (1 << 16) // Don't update last access time 
#define  LEAN_ATTR_IMMUTABLE_FL (1 << 17) // Don't move file sectors 
#define  LEAN_ATTR_PREALLOC     (1 << 18) // Keep any preallocated sectors beyond fileSize when the file is closed
#define  LEAN_ATTR_INLINEXTATTR (1 << 19) // Remaining bytes after the inode structure are reserved for inline extended attributes
#define  LEAN_ATTR_INLINEDATA   (1 << 20) // Remaining bytes after the inode structure are the file's contents (must be <= to remaining room)
//       LEAN_ATTR_             (1 << 21)  // reserved
//       LEAN_ATTR_             (1 << 22)  // reserved
//       LEAN_ATTR_             (1 << 23)  // reserved
//       LEAN_ATTR_             (1 << 24)  // reserved
//       LEAN_ATTR_             (1 << 25)  // reserved
//       LEAN_ATTR_             (1 << 26)  // reserved
//       LEAN_ATTR_             (1 << 27)  // reserved
//       LEAN_ATTR_             (1 << 28)  // reserved
#define  LEAN_ATTR_IFMT         (7 << 29) // Bit mask to extract the file type 
#define  LEAN_ATTR_IFREG        (1 << 29) // File type: regular file 
#define  LEAN_ATTR_IFDIR        (2 << 29) // File type: directory 
#define  LEAN_ATTR_IFLNK        (3 << 29) // File type: symbolic link 
#define  LEAN_ATTR_IFFRK        (4 << 29) // File type: fork 


#define LEAN_NAME_LEN_MAX  4068   // 255 is largest value allowed in rec_len * 16 bytes per record - 12 bytes for header
#define LEAN_DIRENTRY_NAME   12
struct S_LEAN_DIRENTRY {
  bit64u inode;     // The inode number of the file linked by this directory entry, the address of the first cluster of the file.
  bit8u  type;      // see table below (0 = deleted)
  bit8u  rec_len;   // len of total record in 8 byte units.
  bit16u name_len;  // total length of name.
  bit8u  name[4];   // (UTF-8) must *not* be null terminated, remaining bytes undefined if not a multiple of 8.  UTF-8
};                  // 4 to make it 16 bytes for first para.  Not limited to 4 bytes.

// type:
#define LEAN_FT_MT    0 // File type: Empty
#define LEAN_FT_REG   1 // File type: regular file
#define LEAN_FT_DIR   2 // File type: directory
#define LEAN_FT_LNK   3 // File type: symbolic link
#define LEAN_FT_FRK   4 // File type: fork

void parse_command(int, char *[], char *, bit32u *, size_t *);

size_t read_block(void *, const bit64u, const unsigned);
size_t write_block(void *, const bit64u, const unsigned);

bool is_buf_empty(const void *, unsigned);
const char *get_date_month_str(const int);
bool is_utf8(bit8u);

bool lean_check_directory(const bit64u, const char *, const bit32u);
unsigned lean_compare_bitmaps(const unsigned);

bit64u lean_get_sector_num(struct S_LEAN_INODE *, const bit64u);
bit64u find_free_bit(bit8u *, const bit64u, bit64u, const bool);

bool lean_check_inode(const bit64u, struct S_LEAN_INODE *, const bit32u);
bool lean_check_inode_attrib(const bit32u, const bit32u);

bool lean_valid_indirect(const struct S_LEAN_INDIRECT *);

bit32u lean_calc_crc(const bit32u *, const unsigned);

#pragma pack(pop)