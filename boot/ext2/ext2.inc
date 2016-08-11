comment |*******************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: ext2.inc                                                           *
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
*   EQUates for ext2.asm                                                   *
*                                                                          *
* BUILT WITH:   NewBasic Assembler                                         *
*                 http://www.fysnet/newbasic.htm                           *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm ext2<enter>                                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*                                                                          *
***************************************************************************|

; all of these are placed so that we don't have to check for
;  64k boundary read checks from the BIOS
;  (EXT2_ROOTSEG should be set on page boundary)
;  (LOADSEG should be set on 64k boundary)
;

EXT2_SUPER      equ  0400h          ; offset of super and 1024 bytes long
EXT2_GROUP      equ  0800h          ; the first group descriptor will be
                                    ; between 0800h and 1200h
EXT2_ROOT_SIZE  equ  3100h          ; offset were we store the size of the root
EXT2_BLOCK_SIZE equ  3104h          ; offset were we store the size of a block (in bytes)
EXT2_FIRST_GRP  equ  3106h          ; offset were we store the offset of the first group
EXT2_INODE_SIZE equ  310Ah          ; word sized buffer to store inode size

TEMP_BUFFER     equ  3300h          ; temp buffer (4096 bytes long)
EXT2_ROOTOFF    equ  4300h          ; offset to load root dir to

EXT2_STACK_TOP  equ  0FFFEh         ; top of stack

S_EXT2_SUPER  struct
           inodes_count       dword  ; Inodes count
           blocks_count       dword  ; Blocks count
           r_blocks_count     dword  ; Reserved blocks count
           free_blocks_count  dword  ; Free blocks count
           free_inodes_count  dword  ; Free inodes count
           first_data_block   dword  ; First Data Block
           log_block_size     dword  ; Block size
           log_frag_size      dword  ; Fragment size
           blocks_per_group   dword  ; # Blocks per group
           frags_per_group    dword  ; # Fragments per group
           inodes_per_group   dword  ; # Inodes per group
           mtime              dword  ; Mount time
           wtime              dword  ; Write time
           mnt_count           word  ; Mount count
           max_mnt_count       word  ; Maximal mount count
           magic               word  ; Magic signature
           state               word  ; File system state
           errors              word  ; Behaviour when detecting errors
           minor_rev_level     word  ; minor revision level
           lastcheck          dword  ; time of last check
           checkinterval      dword  ; max. time between checks
           creator_os         dword  ; OS
           rev_level          dword  ; Revision level
           def_resuid          word  ; Default uid for reserved blocks
           def_resgid          word  ; Default gid for reserved blocks
           first_ino          dword  ; First non-reserved inode
           inode_size          word  ; size of inode structure
           block_group_nr      word  ; block group # of this superblock
           feature_compat     dword  ; compatible feature set
           feature_incompat   dword  ; incompatible feature set
           feature_ro_compat  dword  ; readonly-compatible feature set
           uuid               dup 16 ; 128-bit uuid for volume
           volume_name        dup 16 ; volume name
           last_mounted       dup 64 ; directory where last mounted
           algorithm_usage_bitmap  dword  ; For compression
           prealloc_blocks     byte  ; Nr of blocks to try to preallocate
           prealloc_dir_blocks byte  ; Nr to preallocate for dirs
           reserved_gdt_blocks word  ; per group table for online growth
           journal_uuid      dup 16  ; uuid of journal superblock
           journal_inum       dword  ; inode number of journal file
           journal_dev        dword  ; device number of journal file
           last_orphan        dword  ; start of list of inodes to delete
           hash_seed         dup 16  ; HTREE hash seed
           def_hash_version    byte  ; Default hash version to use
           jnl_backup_type     byte  ;
           desc_size           word  ; Group descriptor size (INCOMPAT_64BIT)
           default_mount_opts dword  ;
           first_meta_bg      dword  ; First metablock block group
           mkfs_time          dword  ; When the filesystem was created
           jnl_blocks        dup 68  ; Backup of the journal inode block_array 
           blocks_count_hi    dword  ; Blocks count
           r_blocks_count_hi  dword  ; Reserved blocks count
           free_blocks_count_hi dword  ; Free blocks count
           min_extra_isize     word  ; All inodes have at least # bytes
           want_extra_isize    word  ; New inodes should reserve # bytes
           flags              dword  ; Miscellaneous flags
           raid_stride         word  ; RAID stride
           mmp_interval        word  ; # seconds to wait in MMP checking
           mmp_block           word  ; Block for multi-mount protection
           raid_stripe_width  dword  ; blocks on all data disks (N*stride)
           log_groups_per_flex byte  ; FLEX_BG group size    (1 << log_groups_per_flex) ?
           reserved_char_pad   byte  ;
           reserved_pad        word  ; Padding to next 32bits
           reserved          dup 648 ; Padding to the end of the block
S_EXT2_SUPER  ends


S_EXT2_GROUP_DESC struct
           block_bitmap       dword  ;
           inode_bitmap       dword  ;
           inode_table        dword  ;
           free_blocks_count   word  ;
           free_inodes_count   word  ;
           used_dirs_count     word  ;
           flags               word  ;
           reserved            dup 8 ;
           itable_unused       word  ; Unused inodes count
           checksum            word  ; crc16(s_uuid+grouo_num+group_desc)
           block_bitmap_hi    dword  ; Blocks bitmap block MSB
           inode_bitmap_hi    dword  ; Inodes bitmap block MSB
           inode_table_hi     dword  ; Inodes table block MSB
           free_blocks_count_hi word ; Free blocks count MSB
           free_inodes_count_hi word ; Free inodes count MSB
           used_dirs_count_hi   word ;  Directories count MSB
           pad                 word  ;
           reserved2[3]      dup 12  ;
S_EXT2_GROUP_DESC ends

EXT2_S_IFMT         equ 0F000h   ; format mask 
EXT2_S_IFSOCK       equ 0C000h   ; socket 
EXT2_S_IFLNK        equ 0A000h   ; symbolic link 
EXT2_S_IFREG        equ  8000h   ; regular file 
EXT2_S_IFBLK        equ  6000h   ; block device 
EXT2_S_IFDIR        equ  4000h   ; directory 
EXT2_S_IFCHR        equ  2000h   ; character device 
EXT2_S_IFIFO        equ  1000h   ; fifo 

EXT4_HUGE_FILE_FL   equ  40000h  ; Set to each huge file
EXT4_EXTENTS_FL     equ  80000h  ; Inode uses extents

S_EXT2_INODE    struct
           mode                word  ;
           uid                 word  ;
           size               dword  ;
           atime              dword  ;
           ctime              dword  ;
           mtime              dword  ;
           dtime              dword  ;
           gid                 word  ;
           links_count         word  ; hard link count?
           blocks             dword  ; number of 512 byte blocks used  (?)
           flags              dword  ; see above
           osd1               dword  ;
           block_array       dup 60  ;
           generation         dword  ;
           file_acl           dword  ;
           dir_acl            dword  ;
           faddr              dword  ;
           blocks_hi           word  ;
           file_acl_high       word  ;
           uid_high            word  ;
           gid_high            word  ;
           reserved2          dword  ;
S_EXT2_INODE    ends

S_EXT2_DIR    struct
           inode              dword  ;
           rec_len             word  ;
           name_len            byte  ;
           file_type           byte  ;
S_EXT2_DIR    ends


EXT3_EXT_MAGIC  equ  0F30Ah

EXT3_EXTENT_HEADER struct
           magic               word  ; probably will support different formats
           entries             word  ; number of valid entries
           max                 word  ; capacity of store in entries
           depth               word  ; has tree real underlaying blocks?
           generation         dword  ; generation of the tree
EXT3_EXTENT_HEADER ends

S_EXT3_EXTENT struct
           block              dword  ; first logical block extent covers (logical block number within file?)
           len                 word  ; number of blocks covered by extent
           start_hi            word  ; high 16 bits of physical block
           start              dword  ; low 32 bits of physical block
S_EXT3_EXTENT ends

S_EXT3_EXTENT_IDX struct
           block              dword  ; index covers logical blocks from 'block'
           leaf               dword  ; pointer to the physical block of the next level. leaf
           leaf_hi             word  ; high 16 bits of physical block
           unused              word  ;
S_EXT3_EXTENT_IDX ends

.end

