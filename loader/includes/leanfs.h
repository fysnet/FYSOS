
#ifndef _LEANFS_H
#define _LEANFS_H

#include "loader.h"
#include "sys.h"

#define LEANFS_MAX_SECT_CNT  40  // max count of sectors to read at a time

#pragma pack(push, 1)

#define LEAN_SUPER_MAGIC     0x4E41454C  // 'LEAN'
#define LEAN_INODE_MAGIC     0x45444F4E  // 'NODE'
#define LEAN_INDIRECT_MAGIC  0x58444E49  // 'INDX'

struct S_LEAN_SUPER {
  // all of this (except for reserved bits in STATE) is 0.6.1 compatible
  bit32u checksum;                // bit32u sum of all fields.
  bit32u magic;                   // 0x4E41454C ('LEAN')
  bit16u fs_version;              // 0x0006 = 0.6, 0x0007 = 0.7
  bit8u  pre_alloc_count;         // count minus one of contiguous sectors that driver should try to preallocate
  bit8u  log_sectors_per_band;    // 1 << log_sectors_per_band = sectors_per_band. Valid values are 12, 13, 14, ...
  bit32u state;                   // bit 0 = unmounted?, bit 1 = error?
  struct S_GUID guid;             // Globally Unique IDentifier
  bit8u  volume_label[64];        // can be modified by the LABEL command
  bit32u sector_count[2];         // The total number of sectors that form a file system volume
  bit32u free_sector_count[2];    // The number of free sectors in the volume. A value of zero means disk full.
  bit32u primary_super[2];        // sector number of primary super block
  bit32u backup_super[2];         // sector number of backup super block
  bit32u bitmap_start[2];         // This is the address of the sector where the first bands bitmap starts
  bit32u root_start[2];           // This is the address of the sector where the root directory of the volume starts, the inode number of the root directory.
  bit32u bad_start[2];            // This is the address of the sector where the pseudo-file to track bad sectors starts.
#ifdef LEAN_JOURNAL
  bit32u journal[2];              // if not zero, inode number of journal file
  bit8u  reserved[352];           // zeros
#else
  bit8u  reserved[360];           // zeros
#endif
};

#define LEAN_INDIRECT_EXTENT_CNT  38
struct S_LEAN_INDIRECT {
  bit32u checksum;                // bit32u sum of all fields before this one.
  bit32u magic;                   // 0x58444E49 ('INDX')
  bit32u sector_count[2];         // total number of sectors addressable using this indirect sector.
  bit32u inode[2];                // the inode number of this file this indirect sector belongs to.
  bit32u this_sector[2];          // The address of the sector storing this indirect sector.
  bit32u prev_indirect[2];        // the address of the previous indirect sector.
  bit32u next_indirect[2];        // the address of the next indirect sector.
  bit8u  extent_count;            // The number of valid extent specifications storing in the indirect struct.
  bit8u  reserved0[3];            // reserved
  bit32u reserved1;               // reserved
  bit32u extent_start[LEAN_INDIRECT_EXTENT_CNT][2]; // The array of extents
  bit32u extent_size[LEAN_INDIRECT_EXTENT_CNT];
};

#define  LEAN_INODE_SIZE       176
#define  LEAN_INODE_EXTENT_CNT   6
#define  LEAN_DATA_OFFSET      512   // data starts at offset 512 from start of inode
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
  bit32u file_size[2];            // file size
  bit32u sector_count[2];         // count of sectors used
  bit32s acc_time[2];             // last accessed: number of uS elapsed since midnight of 1970-01-01
  bit32s sch_time[2];             // status change: number of uS elapsed since midnight of 1970-01-01
  bit32s mod_time[2];             // last modified: number of uS elapsed since midnight of 1970-01-01
  bit32s cre_time[2];             //       created: number of uS elapsed since midnight of 1970-01-01
  bit32u first_indirect[2];       // address of the first indirect sector of the file.
  bit32u last_indirect[2];        // address of the last indirect sector of the file.
  bit32u fork[2];
  bit32u extent_start[LEAN_INODE_EXTENT_CNT][2]; // The array of extents
  bit32u extent_size[LEAN_INODE_EXTENT_CNT]; 
};

struct S_LEAN_EXTENT {
  bit32u start[2];
  bit32u size;
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
  bit32u inode[2];  // The inode number of the file linked by this directory entry, the address of the first cluster of the file.
  bit8u  type;      // see table below (0 = deleted)
  bit8u  rec_len;   // len of total record in 8 byte units.
  bit16u name_len;  // total length of name.
  bit8u  name[4];   // (UTF-8) must *not* be null terminated, remaining bytes undefined if not a multiple of 8.  UTF-8
};                  // 4 to make it 16 bytes for first para.  Not limited to 4 bytes.
#pragma pack(pop)

struct S_LEANFS_DATA {
  void *root_dir;
  void *super;
  bit32u root_size;
  bit32u root_lba;
};


bit32u lean_load_file(const bit32u, void *);
struct S_LEAN_DIRENTRY *lean_search(const char *, const bit8u type, struct S_LEANFS_DATA *);
bool leanfs_load_data(struct S_LEANFS_DATA *);
bool lean_load_dir(const bit32u, struct S_LEANFS_DATA *);

#endif   // _LEANFS_H
