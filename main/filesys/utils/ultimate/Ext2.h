/*
 *                             Copyright (c) 1984-2022
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

#if !defined(AFX_EXT2_H__0971FB9F_D6D8_4D80_B09C_C0DE6C70175A__INCLUDED_)
#define AFX_EXT2_H__0971FB9F_D6D8_4D80_B09C_C0DE6C70175A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ext2.h : header file
//

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#pragma pack(push, 1)

// super:state flags
#define  EXT2_VALID_FS      1
#define  EXT2_ERROR_FS      2

// super:errors flags
#define EXT2_ERRORS_CONTINUE 1
#define EXT2_ERRORS_RO       2
#define EXT2_ERRORS_PANIC    3

#define EXT2_OS_LINUX   0
#define EXT2_OS_HURD    1
#define EXT2_OS_MASIX   2
#define EXT2_OS_FREEBSD 3
#define EXT2_OS_LITES4  4
#define EXT2_OS_WIN32   5
#define EXT2_OS_FYSOS   6

// defines which bitmap to use
#define EXT2_BLOCK_BITMAP 0
#define EXT2_INODE_BITMAP 1

#define  EXT2_GOOD_OLD_REV  0
#define  EXT2_DYNAMIC_REV   1

struct S_EXT2_SUPER {
  DWORD   inodes_count;      // Inodes count
  DWORD   blocks_count;      // Blocks count
  DWORD   r_blocks_count;    // Reserved blocks count
  DWORD   free_blocks_count; // Free blocks count
  DWORD   free_inodes_count; // Free inodes count
  DWORD   first_data_block;  // First Data Block
  DWORD   log_block_size;    // Block size
  INT32   log_frag_size;     // Fragment size
  DWORD   blocks_per_group;  // # Blocks per group
  DWORD   frags_per_group;   // # Fragments per group
  DWORD   inodes_per_group;  // # Inodes per group
  DWORD   mtime;             // Mount time
  DWORD   wtime;             // Write time
  WORD    mnt_count;         // Mount count
  INT16   max_mnt_count;     // Maximal mount count
  WORD    magic;             // Magic signature
  WORD    state;             // File system state
  WORD    errors;            // Behaviour when detecting errors
  WORD    minor_rev_level;   // minor revision level
  DWORD   lastcheck;         // time of last check
  DWORD   checkinterval;     // max. time between checks
  DWORD   creator_os;        // OS   // 0	Linux
                                     // 1 GNU HURD
                                     // 2 MASIX
                                     // 3 FreeBSD
                                     // 4 Lites
  DWORD   rev_level;         // Revision level
  WORD    def_resuid;        // Default uid for reserved blocks
  WORD    def_resgid;        // Default gid for reserved blocks
  
  // These fields are for EXT2_DYNAMIC_REV (rev_level == 1) superblocks only.
  // if rev_level == 0, these fields should not be honored.
  DWORD   first_ino;         // First non-reserved inode
  WORD    inode_size;        // size of inode structure
  WORD    block_group_nr;    // block group # of this superblock
  DWORD   feature_compat;    // compatible feature set (The fs driver is free to support them or not without risk of damaging the meta-data.)
  DWORD   feature_incompat;  // incompatible feature set (The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.)
  DWORD   feature_ro_compat; // readonly-compatible feature set (see notes below)
  struct S_GUID uuid;        // 128-bit uuid for volume
  char    volume_name[16];   // volume name
  char    last_mounted[64];  // directory where last mounted
  DWORD   algorithm_usage_bitmap; // For compression
  // Performance hints.  Directory preallocation should only happen if the EXT2_COMPAT_PREALLOC flag is on.
  BYTE    prealloc_blocks;     // Nr of blocks to try to preallocate
  BYTE    prealloc_dir_blocks; // Nr to preallocate for dirs
  WORD    reserved_gdt_blocks; // per group table for online growth
  
  // Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
  struct S_GUID journal_uuid;  // uuid of journal superblock
  DWORD   journal_inum;         // inode number of journal file
  DWORD   journal_dev;          // device number of journal file
  DWORD   last_orphan;          // start of list of inodes to delete
  DWORD   hash_seed[4];         // HTREE hash seed
  BYTE    def_hash_version;     // Default hash version to use
  BYTE    jnl_backup_type;
  WORD    desc_size;            // Group descriptor size (INCOMPAT_64BIT)
  DWORD   default_mount_opts;   
  DWORD   first_meta_bg;        // First metablock block group
  DWORD   mkfs_time;            // When the filesystem was created
  DWORD   jnl_blocks[17];       // Backup of the journal inode block_array [0 -> 14], [15] = unused?, [16] = inode->size;
  // 64bit support valid if EXT4_FEATURE_COMPAT_64BIT
  DWORD   blocks_count_hi;      // Blocks count
  DWORD   r_blocks_count_hi;    // Reserved blocks count
  DWORD   free_blocks_count_hi; // Free blocks count
  WORD    min_extra_isize;      // All inodes have at least # bytes
  WORD    want_extra_isize;     // New inodes should reserve # bytes
  DWORD   flags;                // Miscellaneous flags (see below)
  WORD    raid_stride;          // RAID stride
  WORD    mmp_interval;         // # seconds to wait in MMP checking
  DWORD64 mmp_block;            // Block for multi-mount protection
  DWORD   raid_stripe_width;    // blocks on all data disks (N*stride)
  BYTE    log_groups_per_flex;  // FLEX_BG group size    (1 << log_groups_per_flex) ?
  BYTE    reserved_char_pad;
  WORD    reserved_pad;         // Padding to next 32bits
  DWORD64 kbytes_written;       // Number of KiB wri.en to this filesystem over its lifetime
  DWORD   snapshot_inum;        //  inode number of active snapshot
  DWORD   snapshot_id;          //  Sequential ID of active snapshot
  DWORD64 snapshot_r_blocks_count; //  Number of blocks reserved for active snapshot's future use
  DWORD   snapshot_list;        //  inode number of the head of the on-disk snapshot list
  DWORD   error_count;          //  Number of errors seen
  DWORD   first_error_time;     //  First time an error happened, in seconds since the epoch
  DWORD   first_error_ino;      //  inode involved in first error
  DWORD64 first_error_block;    //  Number of block involved of first error
  BYTE    first_error_func[32]; //  Name of function where the error happened
  DWORD   first_error_line;     //  Line number where error happened
  DWORD   last_error_time;      //  Time of most recent error, in seconds since the epoch
  DWORD   last_error_ino;       //  inode involved in most recent error
  DWORD   last_error_line;      //  Line number where most recent error happened
  DWORD64 last_error_block;     //  Number of block involved in most recent error
  BYTE    last_error_func[32];  //  Name of function where the most recent error happened
  BYTE    mount_opts[64];       //  ASCIIZ string of mount options
  DWORD   usr_quota_inum;       //  Inode number of user quota file
  DWORD   grp_quota_inum;       //  Inode number of group quota file
  DWORD   overhead_blocks;      //  Overhead blocks/clusters in fs (huh?)
  DWORD   reserved[108];        //  Padding to the end of the block
  DWORD   checksum;             //  Superblock checksum
};

// super.flags ????
#define EXT4_FLAGS_S_HASH_IN_USE     1
#define EXT4_FLAGS_UNS_HASH_IN_USE   2
#define EXT4_FLAGS_TEST_DEV_CODE     4

// items we support
#define SUPP_COMPAT      0
#define SUPP_INCOMPAT    EXT2_FEATURE_INCOMPAT_FILETYPE
#define SUPP_RO_COMPAT  (EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER | EXT4_FEATURE_RO_COMPAT_GDT_CSUM | EXT4_FEATURE_RO_COMPAT_HUGE_FILE)

// The EXTx_ prefix number is the lowest version of ext that supports this feature
// The fs driver is free to support them or not without risk of damaging the meta-data.
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC        0x0001 
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES       0x0002 
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL         0x0004 
#define EXT2_FEATURE_COMPAT_EXT_ATTR            0x0008 
#define EXT3_FEATURE_COMPAT_RESIZE_INO          0x0010 
#define EXT3_FEATURE_COMPAT_DIR_INDEX           0x0020 
#define EXT2_FEATURE_COMPAT_LAZY_BG             0x0040 
#define EXT4_FEATURE_COMPAT_EXCLUDE_INODE       0x0080
#define EXT4_FEATURE_COMPAT_EXCLUDE_BITMAP      0x0100

// The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.
#define EXT2_FEATURE_INCOMPAT_COMPRESSION       0x0001 
#define EXT2_FEATURE_INCOMPAT_FILETYPE          0x0002 // has file type byte in directory blocks (instead of 16 bit name_len)
#define EXT3_FEATURE_INCOMPAT_RECOVER           0x0004 
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV       0x0008 
#define EXT2_FEATURE_INCOMPAT_META_BG           0x0010 
#define EXT3_FEATURE_INCOMPAT_EXTENTS           0x0040 // has extents
#define EXT4_FEATURE_INCOMPAT_64BIT             0x0080 
#define EXT4_FEATURE_INCOMPAT_MMP               0x0100 
#define EXT4_FEATURE_INCOMPAT_FLEX_BG           0x0200 
#define EXT4_FEATURE_INCOMPAT_EA_INODE          0x0400
#define EXT4_FEATURE_INCOMPAT_DIRDATA           0x1000
#define EXT4_FEATURE_INCOMPAT_BG_USE_META_CSUM  0x2000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR          0x4000
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA       0x8000

// If the fs driver doesn't support any of these, it still can read from the system, but shouldn't write to it.
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER     0x0001 // has sparse super blocks instead of every block containing superblock copies
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE       0x0002 
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR        0x0004 
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE        0x0008 // uses a 48-bit block count
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM         0x0010 // the Group Descriptors have a checksum that needs to be updated.
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK        0x0020 
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE      0x0040 
#define EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT     0x0080
#define EXT4_FEATURE_RO_COMPAT_QUOTA            0x0100
#define EXT4_FEATURE_RO_COMPAT_BIGALLOC         0x0200
#define EXT4_FEATURE_RO_COMPAT_METADATA_CSUM    0x0400

#define EXT2_BG_INODE_UNINIT  0x0001 // Inode table/bitmap not initialized
#define EXT2_BG_BLOCK_UNINIT  0x0002 // Block bitmap not initialized
#define EXT2_BG_INODE_ZEROED  0x0004 // On-disk itable initialized to zero

// The size of a group descritor = (EXT4_FEATURE_INCOMPAT_64BIT) ? super->desc_size : 32;

struct S_EXT2_GROUP_DESC {
  DWORD   block_bitmap;
  DWORD   inode_bitmap;
  DWORD   inode_table;
  WORD    free_blocks_count;
  WORD    free_inodes_count;
  WORD    used_dirs_count;
  WORD    flags;
  DWORD   bg_exclude_bitmap_lo;    // Lower 32-bits of location of snapshot exclusion bitmap
  WORD    bg_block_bitmap_csum_lo; // Lower 16-bits of the block bitmap checksum
  WORD    bg_inode_bitmap_csum_lo; // Lower 16-bits of the inode bitmap checksum
  WORD    itable_unused; // Unused inodes count
  WORD    checksum;      // crc16(s_uuid+grouo_num+group_desc)
  // ext4 stuff starts here
  DWORD   block_bitmap_hi;      // Blocks bitmap block MSB
  DWORD   inode_bitmap_hi;      // Inodes bitmap block MSB
  DWORD   inode_table_hi;       // Inodes table block MSB
  WORD    free_blocks_count_hi; // Free blocks count MSB
  WORD    free_inodes_count_hi; // Free inodes count MSB
  WORD    used_dirs_count_hi;   // Directories count MSB
  WORD    bg_itable_unused_hi;     // Upper 16-bits of unused inode count.
  DWORD   bg_exclude_bitmap_hi;    // Upper 32-bits of location of snapshot exclusion bitmap.
  WORD    bg_block_bitmap_csum_hi; // Upper 16-bits of the block bitmap checksum.
  WORD    bg_inode_bitmap_csum_hi; // Upper 16-bits of the inode bitmap checksum.
  DWORD   bg_reserved;             // Padding to 64 bytes.
};



// imode:  
//   -- file format -- 
#define EXT2_S_IFMT     0xF000   // format mask 
#define EXT2_S_IFSOCK   0xC000   // socket 
#define EXT2_S_IFLNK    0xA000   // symbolic link 
#define EXT2_S_IFREG    0x8000   // regular file 
#define EXT2_S_IFBLK    0x6000   // block device 
#define EXT2_S_IFDIR    0x4000   // directory 
#define EXT2_S_IFCHR    0x2000   // character device 
#define EXT2_S_IFIFO    0x1000   // fifo 

//  -- access rights -- 
#define EXT2_S_ISUID    0x0800   // SUID 
#define EXT2_S_ISGID    0x0400   // SGID 
#define EXT2_S_ISVTX    0x0200   // sticky bit 

#define EXT2_S_IRWXU    0x01C0   // user access rights mask 
#define EXT2_S_IRUSR    0x0100   // read 
#define EXT2_S_IWUSR    0x0080   // write 
#define EXT2_S_IXUSR    0x0040   // execute 

#define EXT2_S_IRWXG    0x0038   // group access rights mask 
#define EXT2_S_IRGRP    0x0020   // read 
#define EXT2_S_IWGRP    0x0010   // write 
#define EXT2_S_IXGRP    0x0008   // execute 

#define EXT2_S_IRWXO    0x0007   // others access rights mask 
#define EXT2_S_IROTH    0x0004   // read 
#define EXT2_S_IWOTH    0x0002   // write 
#define EXT2_S_IXOTH    0x0001   // execute 


#define EXT2_S_STND_ACCCES ((EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR) | (EXT2_S_IRGRP | EXT2_S_IXGRP) | (EXT2_S_IROTH | EXT2_S_IXOTH))


// Inode flags
#define EXT2_SECRM_FL       0x00000001 // Secure deletion
#define EXT2_UNRM_FL        0x00000002 // Undelete
#define EXT2_COMPR_FL       0x00000004 // Compress file
#define EXT2_SYNC_FL        0x00000008 // Synchronous updates
#define EXT2_IMMUTABLE_FL   0x00000010 // Immutable file (unchangable file) (read only file)
#define EXT2_APPEND_FL      0x00000020 // writes to file may only append
#define EXT2_NODUMP_FL      0x00000040 // do not dump file
#define EXT2_NOATIME_FL     0x00000080 // do not update atime
// Reserved for compression usage...
#define EXT2_DIRTY_FL       0x00000100
#define EXT2_COMPRBLK_FL    0x00000200 // One or more compressed clusters
#define EXT2_NOCOMP_FL      0x00000400 // Don't compress
#define EXT2_ECOMPR_FL      0x00000800 // Compression error
// End compression flags --- maybe not all used 
#define EXT2_BTREE_FL       0x00001000 // btree format dir
#define EXT2_IMAGIC_FL      0x00002000
#define EXT3_JOURNAL_DATA_FL 0x00004000 // file data should be journaled
#define EXT2_NOTAIL_FL      0x00008000 // file tail should not be merged
#define EXT2_DIRSYNC_FL     0x00010000 // Synchronous directory modifications
#define EXT2_TOPDIR_FL      0x00020000 // Top of directory hierarchies
#define EXT4_HUGE_FILE_FL   0x00040000 // Set to each huge file
#define EXT4_EXTENTS_FL     0x00080000 // Inode uses extents
#define EXT2_RESERVED_FL    0x80000000 // reserved for ext2 lib

#define EXT2_FL_USER_VISIBLE    0x00001FFF // User visible flags
#define EXT2_FL_USER_MODIFIABLE 0x000000FF // User modifiable flags


// Extents
#define EXT3_EXT_MAGIC    0xF30A

// each block (leaves and indexes), even inode-stored has header
struct EXT3_EXTENT_HEADER {
  WORD    magic;       // probably will support different formats
  WORD    entries;     // number of valid entries
  WORD    max;         // capacity of store in entries
  WORD    depth;       // has tree real underlaying blocks?
  DWORD   generation;  // generation of the tree
};

// used at the bottom of the tree
struct S_EXT3_EXTENT {
  DWORD   block;     // first logical block extent covers (logical block number within file?)
  WORD    len;       // number of blocks covered by extent
  WORD    start_hi;  // high 16 bits of physical block
  DWORD   start;     // low 32 bits of physical block
};

// used at all the levels, but the bottom
struct S_EXT3_EXTENT_IDX {
  DWORD   block;   // index covers logical blocks from 'block'
  DWORD   leaf;    // pointer to the physical block of the next level. leaf or next index could be here
  WORD    leaf_hi; // high 16 bits of physical block
  WORD    unused;
};


// This is from the linux ext2.h file.  This says that there is a triple indirect block.
//  How does one know for sure if there is a regular indirect, dbl, and/or tripple?
//  Constants relative to the data blocks
//#define EXT2_NDIR_BLOCKS    12
//#define EXT2_IND_BLOCK      EXT2_NDIR_BLOCKS
//#define EXT2_DIND_BLOCK     (EXT2_IND_BLOCK + 1)
//#define EXT2_TIND_BLOCK     (EXT2_DIND_BLOCK + 1)
//#define EXT2_N_BLOCKS     (EXT2_TIND_BLOCK + 1)

// get the actual value of the inode.size member
#define  INODE_SIZE(i)  (((m_super.rev_level == 1) && (((i)->mode & EXT2_S_IFMT) == EXT2_S_IFREG)) ?  /* if rev == 1 and is a Reg File  */  \
                         (((DWORD64) (i)->dir_acl << 32) | (i)->size) :                               /*  then the dir_acl is the high 32-bits of the size */ \
                          (i)->size)                                                                  /*  else, it is just the 32-bit size field */

struct S_EXT2_INODE {
  WORD    mode;
  WORD    uid;
  DWORD   size;        // if REG file, total size = ((dir_acl << 32) | size)
  DWORD   atime;       // access time (seconds since 1 Jan 1970, midnight)
  DWORD   ctime;       // creation time
  DWORD   mtime;       // mod time
  DWORD   dtime;       // deteled time
  WORD    gid;
  WORD    links_count; // hard link count?
  DWORD   blocks;      // number of *512* byte blocks used
  DWORD   flags;       // see above
  DWORD   osd1;
  union {
    struct {
      DWORD   block[12];
      DWORD   indirect_block;
      DWORD   dbl_indirect_block;
      DWORD   trpl_indirect_block;
    } block_array;
    BYTE    symbolic_link[60];  // If symbolic link, contains path if <= 60 bytes (inode.size <= 60)
    struct {
      struct EXT3_EXTENT_HEADER extent_hdr;
      BYTE    extents[60-12];
    } extents;  // if flags & EXT4_EXTENTS_FL
  };
  DWORD   generation;
  DWORD   file_acl;
  DWORD   dir_acl;
  DWORD   faddr;
  WORD    blocks_hi;   // Ext4 only
  WORD    file_acl_high;
  WORD    uid_high;
  WORD    gid_high;
  DWORD   reserved2;
  WORD    extra_isize;    // Size of this inode - 128.  (i.e.: extra_isize == 128: = 256-byte inode)
  WORD    checksum_hi;    // Upper 16-bits of the inode checksum.
  DWORD   ctime_extra;    // Extra change time bits. This provides sub-second precision.
  DWORD   mtime_extra;    // Extra modification time bits. This provides sub-second precision.
  DWORD   atime_extra;    // Extra access time bits. This provides sub-second precision.
  DWORD   crtime;         // File creation time, in seconds since the epoch.
  DWORD   crtime_extra;   // Extra file creation time bits. This provides sub-second precision.
  DWORD   version_hi;     // Upper 32-bits for version number.

  BYTE    resv[100];      // TODO: extra values. We pad to a 256-byte inode. (see extra_isize above)
};

#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_CHRDEV   3
#define EXT2_FT_BLKDEV   4
#define EXT2_FT_FIFO     5
#define EXT2_FT_SOCK     6
#define EXT2_FT_SYMLINK  7
#define EXT2_FT_MAX      8 

// If the EXT2_FEATURE_INCOMPAT_FILETYPE is set in the super->feature_incompat field, then use this one.
//  The file_type is here so we don't have to load the inode to see if it is a file or a directory.
struct S_EXT2_DIR {
  DWORD   inode;     // 1 based  (since a value of zero = not used)
  WORD    rec_len;
  BYTE    name_len;
  BYTE    file_type;
  // name goes here up to 255 chars
};

// If the EXT2_FEATURE_INCOMPAT_FILETYPE is clear in the super->feature_incompat field, then use this one.
//  Since the file_type is not here, we have to load the inode to see if it is a file or a directory.
struct S_EXT2_DIR_OLD {
  DWORD   inode;     // 1 based  (since a value of zero = not used)
  WORD    rec_len;
  WORD    name_len;
  // name goes here up to 255 chars
};


/*
// http://tomoyo.sourceforge.jp/cgi-bin/lxr/source/include/linux/jbd.h#L116
#define JOURNAL_MAGIC_NUMBER 0xC03B3998

// types
#define JFS_DESCRIPTOR_BLOCK    1
#define JFS_COMMIT_BLOCK        2
#define JFS_SUPERBLOCK_V1       3
#define JFS_SUPERBLOCK_V2       4
#define JFS_REVOKE_BLOCK        5

// Remember that all these entries are big endian
struct S_EXT2_JOURNAL_HEADER {
  DWORD   magic;
  DWORD   blocktype;
  DWORD   sequence;
};

// Remember that all these entries are big endian
struct S_EXT2_JOURNAL_SUPERBLOCK  {
  struct S_EXT2_JOURNAL_HEADER hdr;
  // Static information describing the journal
  DWORD   blocksize;            // journal device blocksize
  DWORD   maxlen;               // total blocks in journal file
  DWORD   first;                // first block of log information
  // Dynamic information describing the current state of the log
  DWORD   sequence;             // first commit ID expected in log
  DWORD   start;                // blocknr of start of log
  // Error value, as set by journal_abort().
  DWORD   error;
  // Remaining fields are only valid in a version-2 superblock
  DWORD   feature_compat;       // compatible feature set
  DWORD   feature_incompat;     // incompatible feature set
  DWORD   feature_ro_compat;    // readonly-compatible feature set
  struct S_GUID uuid;          // 128-bit uuid for journal
  DWORD   nr_users;             // Nr of filesystems sharing log
  DWORD   dynsuper;             // Blocknr of dynamic superblock copy
  DWORD   max_transaction;      // Limit of journal blocks per trans.
  DWORD   max_trans_data;       // Limit of data blocks per trans.
  DWORD   padding[44];
  BYTE    users[16*48];         // ids of all fs'es sharing the log
};
*/

// cannot be > 240 bytes
struct S_EXT2_ITEMS {
  unsigned InodeNum;
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  
};

#pragma pack(pop)

#define  EXT2_BLOCK_BITMAP  0
#define  EXT2_INODE_BITMAP  1

/////////////////////////////////////////////////////////////////////////////
// CExt2 dialog

class CExt2 : public CPropertyPage {
  DECLARE_DYNCREATE(CExt2)

// Construction
public:
  CExt2();
  ~CExt2();
  
// Dialog Data
  //{{AFX_DATA(CExt2)
  enum { IDD = IDD_EXT2 };
  CMyTreeCtrl m_dir_tree;
  CString m_block_count;
  CString m_blocks_group;
  CString m_creator_os;
  CString m_error_b;
  CString m_first_data_block;
  CString m_frags_group;
  CString m_free_block_count;
  CString m_free_inode_count;
  CString m_gid;
  CString m_inode_count;
  CString m_inodes_group;
  CString m_last_check;
  CString m_last_check_int;
  CString m_log_block_size;
  CString m_log_frag_size;
  CString m_magic;
  CString m_max_mount_count;
  CString m_min_rev_level;
  CString m_mount_count;
  CString m_mount_time;
  CString m_r_block_count;
  CString m_rev_level;
  CString m_state;
  CString m_uid;
  CString m_write_time;
  //}}AFX_DATA
  
  DWORD GetNewColor(int index);
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  void GetGroupDescTable(void);
  void WriteGroupDescTable(void);
  void GetGroupInodeTable(const int group);
  void WriteGroupInodeTable(const int group);
  int  FindRootInode(const int group);
  void Ext2PutInode(const unsigned InodeNum, struct S_EXT2_INODE *inode);
  unsigned Ext2GetInode(const unsigned InodeNum, struct S_EXT2_INODE *inode);
  DWORD64 Ext2GetBlockNum(unsigned InodeNum, struct S_EXT2_INODE *inode, unsigned index);
  void GetInodeBlocks(unsigned InodeNum, struct S_EXT2_INODE *inode);
  
  void ParseDir(void *root, DWORD64 root_size, HTREEITEM parent);
  void *Ext2ReadFile(struct S_EXT2_INODE *inode, const unsigned InodeNum, const unsigned group);
  
  void SendToDialog(struct S_EXT2_SUPER *super);
  void ReceiveFromDialog(struct S_EXT2_SUPER *super);
  
  void SaveItemInfo(HTREEITEM hItem, const unsigned InodeNum, BOOL CanCopy);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  /*
  void InsertFile(DWORD64 Inode, CString csName, CString csPath);
  void InsertFolder(DWORD64 Inode, CString csName, CString csPath);
  void DeleteFile(HTREEITEM hItem);
  void DeleteFolder(HTREEITEM hItem);
  */
  
  bool Ext2Format(const BOOL AskForBoot);
  void MarkBitmap(const int group, const int bitmap, const int start, const int length, const BOOL mark);

  void DisplayFreeSpace(void);
  size_t CalcFreeBlocks(void);
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  struct S_EXT2_SUPER m_super;
  void   *m_desc_table;
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->Ext2[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;
  
  unsigned m_ext2_size;   // 2 = Ext2, 3 = Ext3, 4 = Ext4
  unsigned m_inode_size;  // 
  unsigned m_block_size;  // block size: 1024, 2048, etc.
  unsigned m_groups;      // total groups
  unsigned m_sectors_per_block; // sectors per block
  
  int      m_cur_group;   // current group's Inode Table we have in memory
  DWORD    m_inode_table_size; // size in bytes of inode table
  void    *m_cur_inode_table;
  BOOL     m_inode_dirty;
  int      m_RootInode;
  
  DWORD64 *m_block_table;
  unsigned m_block_tab_count;  // blocks in table
  unsigned m_block_table_inode;

  BOOL    m_del_clear;
  size_t  m_free_blocks;
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CExt2)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CExt2)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnFysosSig();
  afx_msg void OnDynRev();
  afx_msg void OnGroups();
  afx_msg void OnExt2Apply();
  afx_msg void OnExt2Copy();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnExt2Entry();
  afx_msg void OnUpdateCode();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  afx_msg void OnFormat();
  afx_msg void OnClean();
  afx_msg void OnCheck();
  afx_msg void OnMountTime();
  afx_msg void OnLastCheck();
  afx_msg void OnWriteTime();
  afx_msg void OnLastCheckInt();
  afx_msg void OnDelClear();
  afx_msg void OnCreatorOs();
  afx_msg void OnState();
  afx_msg void OnErrorB();
  afx_msg void OnChangeRevLevel();
  afx_msg void OnMagic();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXT2_H__0971FB9F_D6D8_4D80_B09C_C0DE6C70175A__INCLUDED_)
