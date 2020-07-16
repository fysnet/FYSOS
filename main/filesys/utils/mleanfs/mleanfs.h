/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  Last updated: 15 July 2020
 */

// set it to 1 (align on byte)
#pragma pack(push, 1)

// define if you want the inode to contain EA's.
//  the file data will start on the next sector boundary.
//  undefine to have the file data start just after the INODE.
//#define INODE_HAS_EAS

#define SECT_SIZE  512

#define MAX_PATH 260

char strtstr[] = "\nMLEANFS  v2.00.00    Forever Young Software 1984-2019\n\n";

// default superblock location (will sector pad to this location if boot_size is less than)
//  if boot code is larger, will move super location to just past boot code
//  (use '1' to place super just after boot code)
// first bitmap to follow superblock
// root directory usually follows first bitmap, but can be any available area on the disk
#define LEAN_SUPER_LOCATION  16

// superblock must be within LBA 1 to LBA 32
#if ((LEAN_SUPER_LOCATION < 1) || (LEAN_SUPER_LOCATION > 32))
  #error "Superblock must be at LBA 1 through 32"
#endif

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

#define S_LEAN_INODE_SIZE     176
#define LEAN_INODE_EXTENT_CNT   6
#define LEAN_DATA_OFFSET      512   // data starts at offset 512 from start of inode

// 176 bytes each
struct S_LEAN_INODE {
  bit32u checksum;                // bit32u sum of all fields before this one.
  bit32u magic;                   // 0x45444F4E  ('NODE')
  bit8u  extent_count;            // count of extents in this inode struct.
  bit8u  reserved[3];             // reserved
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
//       LEAN_ATTR_             (1 << 20)  // reserved
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


#define FOLDER_SIZE  15  // default sectors per sub folder (inode + FOLDER_SIZE)

struct S_LEAN_EXTENT {
  bit64u start[LEAN_INODE_EXTENT_CNT]; // The array of extents
  bit32u size[LEAN_INODE_EXTENT_CNT]; 
};

#pragma pack(pop)

#define DEF_FOLDER_CNT  32

struct S_FOLDERS {
  char name[256];    // folder name
  void *root;        // pointer to memory holding root data
  unsigned buf_size; // size of this buffer (in sectors)
  size_t  cur_rec;      // record position of next available (last free) entry
  int  links_count;  // link count of entries within this directory plus itself
  struct S_LEAN_EXTENT extent;
  size_t ext_cnt;      // count of extents used (1 or 2)
  struct S_FOLDERS *parent;
};


bit64u get_useconds(void);
void bitmap_mark(const int band, int start, int count, const bool used);
bit64u bitmap_find(void);
size_t get_next_extent(size_t, struct S_LEAN_EXTENT *);
void create_root_entry(const size_t, char *, const size_t, bit64u, const bit64u);
void root_start(struct S_LEAN_DIRENTRY *record, const bit64u, const bit64u);
void create_inode(FILE *, const bit64u, const bit64u, const bit32u, const int, const size_t, struct S_LEAN_EXTENT *);

void parse_command(int, char *[], char *, bool *, char *);

bit32u lean_calc_crc(const void *, unsigned int);
