
#ifndef _EXT2_H
#define _EXT2_H

#include "loader.h"
#include "sys.h"      // for S_GUID


#pragma pack(push, 1)

struct S_EXT2_DIR {
  bit32u inode;
  bit16u rec_len;
  bit8u  name_len;
  bit8u  file_type;
};

struct S_EXT2_GROUP_DESC {
  bit32u block_bitmap;
  bit32u inode_bitmap;
  bit32u inode_table;
  bit16u free_blocks_count;
  bit16u free_inodes_count;
  bit16u used_dirs_count;
  bit16u flags;
  bit8u  reserved[8];
  bit16u itable_unused;        // Unused inodes count
  bit16u checksum;             // crc16(s_uuid+grouo_num+group_desc)
  bit32u block_bitmap_hi;      // Blocks bitmap block MSB
  bit32u inode_bitmap_hi;      // Inodes bitmap block MSB
  bit32u inode_table_hi;       // Inodes table block MSB
  bit16u free_blocks_count_hi; // Free blocks count MSB
  bit16u free_inodes_count_hi; // Free inodes count MSB
  bit16u used_dirs_count_hi;   // Directories count MSB
  bit16u pad;
  bit8u  reserved2[12];
};

struct S_EXT2_SUPER {
  bit32u inodes_count;      // Inodes count
  bit32u blocks_count;      // Blocks count
  bit32u r_blocks_count;    // Reserved blocks count
  bit32u free_blocks_count; // Free blocks count
  bit32u free_inodes_count; // Free inodes count
  bit32u first_data_block;  // First Data Block
  bit32u log_block_size;    // Block size
  bit32s log_frag_size;     // Fragment size
  bit32u blocks_per_group;  // # Blocks per group
  bit32u frags_per_group;   // # Fragments per group
  bit32u inodes_per_group;  // # Inodes per group
  bit32u mtime;             // Mount time
  bit32u wtime;             // Write time
  bit16u mnt_count;         // Mount count
  bit16s max_mnt_count;     // Maximal mount count
  bit16u magic;             // Magic signature
  bit16u state;             // File system state
  bit16u errors;            // Behaviour when detecting errors
  bit16u minor_rev_level;   // minor revision level
  bit32u lastcheck;         // time of last check
  bit32u checkinterval;     // max. time between checks
  bit32u creator_os;        // OS
  bit32u rev_level;         // Revision level
  bit16u def_resuid;        // Default uid for reserved blocks
  bit16u def_resgid;        // Default gid for reserved blocks
  
  // These fields are for EXT2_DYNAMIC_REV (rev_level == 1) superblocks only.
  // if rev_level == 0, these fields should not be honored.
  bit32u first_ino;         // First non-reserved inode
  bit16u inode_size;        // size of inode structure
  bit16u block_group_nr;    // block group # of this superblock
  bit32u feature_compat;    // compatible feature set (The fs driver is free to support them or not without risk of damaging the meta-data.)
  bit32u feature_incompat;  // incompatible feature set (The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.)
  bit32u feature_ro_compat; // readonly-compatible feature set (see notes below)
  struct S_GUID uuid;       // 128-bit uuid for volume
  char   volume_name[16];   // volume name
  char   last_mounted[64];  // directory where last mounted
  bit32u algorithm_usage_bitmap; // For compression
  // Performance hints.  Directory preallocation should only happen if the EXT2_COMPAT_PREALLOC flag is on.
  bit8u  prealloc_blocks;   // Nr of blocks to try to preallocate
  bit8u  prealloc_dir_blocks; // Nr to preallocate for dirs
  bit16u reserved_gdt_blocks; // per group table for online growth
  
  // Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
  struct S_GUID journal_uuid;  // uuid of journal superblock
  bit32u journal_inum;         // inode number of journal file
  bit32u journal_dev;          // device number of journal file
  bit32u last_orphan;          // start of list of inodes to delete
  bit32u hash_seed[4];         // HTREE hash seed
  bit8u  def_hash_version;     // Default hash version to use
  bit8u  jnl_backup_type;
  bit16u desc_size;            // Group descriptor size (INCOMPAT_64BIT)
  bit32u default_mount_opts;
  bit32u first_meta_bg;        // First metablock block group
  bit32u mkfs_time;            // When the filesystem was created
  bit32u jnl_blocks[17];       // Backup of the journal inode block_array [0 -> 14], [15] = unused?, [16] = inode->size;
  // 64bit support valid if EXT4_FEATURE_COMPAT_64BIT
  bit32u blocks_count_hi;      // Blocks count
  bit32u r_blocks_count_hi;    // Reserved blocks count
  bit32u free_blocks_count_hi; // Free blocks count
  bit16u min_extra_isize;      // All inodes have at least # bytes
  bit16u want_extra_isize;     // New inodes should reserve # bytes
  bit32u flags;                // Miscellaneous flags
  bit16u raid_stride;          // RAID stride
  bit16u mmp_interval;         // # seconds to wait in MMP checking
  bit32u mmp_block[2];         // Block for multi-mount protection
  bit32u raid_stripe_width;    // blocks on all data disks (N*stride)
  bit8u  log_groups_per_flex;  // FLEX_BG group size    (1 << log_groups_per_flex) ?
  bit8u  reserved_char_pad;
  bit16u reserved_pad;         // Padding to next 32bits
  bit32u reserved[162];        // Padding to the end of the block
};

#define EXT3_EXT_MAGIC    0xF30A

// each block (leaves and indexes), even inode-stored has header
struct EXT3_EXTENT_HEADER {
  bit16u magic;       // probably will support different formats
  bit16u entries;     // number of valid entries
  bit16u max;         // capacity of store in entries
  bit16u depth;       // has tree real underlaying blocks?
  bit32u generation;  // generation of the tree
};

struct S_EXT3_EXTENT {
  bit32u block;       // first logical block extent covers (logical block number within file?)
  bit16u len;         // number of blocks covered by extent
  bit16u start_hi;    // high 16 bits of physical block
  bit32u start;       // low 32 bits of physical block
};

#define  EXT2_S_IFMT    0xF000  // format mask 
#define  EXT2_S_IFSOCK  0xC000  // socket 
#define  EXT2_S_IFLNK   0xA000  // symbolic link 
#define  EXT2_S_IFREG   0x8000  // regular file 
#define  EXT2_S_IFBLK   0x6000  // block device 
#define  EXT2_S_IFDIR   0x4000  // directory 
#define  EXT2_S_IFCHR   0x2000  // character device 
#define  EXT2_S_IFIFO   0x1000  // fifo 

#define  EXT4_HUGE_FILE_FL  0x00040000  // Set to each huge file
#define  EXT4_EXTENTS_FL    0x00080000  // Inode uses extents

struct S_EXT2_INODE {
  bit16u mode;
  bit16u uid;
  bit32u size;
  bit32u atime;
  bit32u ctime;
  bit32u mtime;
  bit32u dtime;
  bit16u gid;
  bit16u links_count; // hard link count?
  bit32u blocks;      // number of 512 byte blocks used  (?)
  bit32u flags;       // see above
  bit32u osd1;
  union U_UNION {
    struct S_BLOCK_ARRAY {
      bit32u block[12];
      bit32u indirect_block;
      bit32u dbl_indirect_block;
      bit32u trpl_indirect_block;
    } block_array;
    bit8u symbolic_link[60];  // If symbolic link, contains path if <= 60 bytes (inode.size <= 60)
    struct S_EXTENTS {
      struct EXT3_EXTENT_HEADER extent_hdr;
      bit8u extents[60-12];
    } extents;  // if flags & EXT4_EXTENTS_FL
  } u;
  bit32u generation;
  bit32u file_acl;
  bit32u dir_acl;
  bit32u faddr;
  bit16u blocks_hi;
  bit16u file_acl_high;
  bit16u uid_high;
  bit16u gid_high;
  bit32u reserved2;
};

#pragma pack(pop)

int ext2_load_file(struct S_EXT2_INODE *, void *, const bit32u);
void *ext2_direct(const bit32u *, const bit32u, bit32u *, const int, void *);
void *ext2_indirect(const bit32u, const bit32u, bit32u *, void *);

struct S_EXT2_DATA {
  void *root_dir;     // root directory
  void *super;        // super block
  void *group;
  bit32u root_size;   // in sectors
  bit32u block_size;  // in sectors
};

bool ext2_load_data(struct S_EXT2_DATA *);


#endif   // _EXT2_H
