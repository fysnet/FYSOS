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

#include "ctype.h"
#include "loader.h"

#ifdef FS_EXT2

#include "malloc.h"
#include "paraport.h"
#include "string.h"
#include "windows.h"

#include "ext2.h"

bool ext2_data_valid = FALSE;
struct S_EXT2_DATA ext2_data;

// load the file 'filename' from disk to 'buf_loc'
bit32u fs_ext2(const char *filename, void *target) {
  char *p, temp[128];
  bit32u cur_size = 0, inode_lba, inode_ofs;
  int i;
  bit32u ret_size = 0;
  
  // have we loaded the root yet
  if (!ext2_data_valid)
    if (!ext2_load_data(&ext2_data))
      return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Search for file in root directory
  struct S_EXT2_SUPER *super = (struct S_EXT2_SUPER *) ext2_data.super;
  struct S_EXT2_DIR *root = (struct S_EXT2_DIR *) ext2_data.root_dir;
  void *inode_buff = malloc(512);
  bit32u inode_size = (super->rev_level == 0) ? 128 : super->inode_size;
  while (1) {
    if (root->inode > 0) {
      p = (char *) ((bit8u *) root + sizeof(struct S_EXT2_DIR));
      for (i=0; i<root->name_len; i++)
        temp[i] = p[i];
      temp[i] = '\0';
      if (!stricmp(temp, filename)) {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  get inode table
        struct S_EXT2_GROUP_DESC *group = (struct S_EXT2_GROUP_DESC *) ext2_data.group;
        //  calculate offset within table to read
        inode_lba = (inode_size * (root->inode - 1)) / 512;  // sector number
        inode_ofs = (inode_size * (root->inode - 1)) % 512;  // offset within sector
        // add sector of inode table
        inode_lba += group->inode_table * ext2_data.block_size;
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  read in the inode
        if (read_sectors(inode_lba, 1, inode_buff) != 1) {
          para_printf("Error reading from file...\n");
          goto fs_ext2_done;
        }
        
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  make sure the mode == regular file
        struct S_EXT2_INODE *inode = (struct S_EXT2_INODE *) ((bit8u *) inode_buff + inode_ofs);
        if ((inode->mode & EXT2_S_IFMT) == EXT2_S_IFREG) {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          //  load file
          ret_size = ext2_load_file(inode, target, ext2_data.block_size);
          goto fs_ext2_done;
        }
        
        // else if we get here, there was an error with the file
        para_printf("Error with file...\n");
        goto fs_ext2_done;
      }
    }
    
    if (root->rec_len == 0)
      break;
    
    cur_size += root->rec_len;
    root = (struct S_EXT2_DIR *) ((bit8u *) root + root->rec_len);
    if (cur_size >= ext2_data.root_size)
      break;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  para_printf("Did not find file...\n");
  
fs_ext2_done:
  mfree(inode_buff);
  return ret_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Load a file to given memory (EXT2)
// on entry:
//   *inode
//   location to store the file
//   sectors per block
// on return:
//   file size
int ext2_load_file(struct S_EXT2_INODE *inode, void *buf_loc, const bit32u block_size) {
  int i, j, k, cnt;
  bit32u blocks, lba, prog = 0;
  void *ret;
  
  // initialize the progress bar
  win_init_progress(inode->size);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if extents, do the extent code
  if (inode->flags & EXT4_EXTENTS_FL) {
    struct S_EXT3_EXTENT *extent = (struct S_EXT3_EXTENT *) inode->u.extents.extents;
    for (i=0; i<inode->u.extents.extent_hdr.entries; i++) {
      cnt = extent[i].len * block_size;
      lba = (/*((bit64u) extent[i].start_hi << 32) +*/ extent[i].start) * block_size;
      // cnt = sectors to read
      while (cnt) {
        j = (cnt < 8) ? cnt : 8;
        if (read_sectors(lba, j, buf_loc) != j) {
          para_printf("Error reading from file...\n");
          return 0;
        }
        buf_loc = (void *) ((bit8u *) buf_loc + (j * 512));
        lba += j;
        cnt -= j;
        prog += (j * 512);
        win_put_progress(prog, 0);
      }
    }
    return inode->size;
  } else {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // direct blocks
    //blocks = (inode->size + ((block_size * 512) - 1)) / (block_size * 512);  // blocks to read
    blocks = ((inode->size / 512) + (block_size - 1)) / block_size;  // blocks to read
    prog += (((blocks <= 12) ? blocks : 12) * 512);
    if ((ret = ext2_direct(inode->u.block_array.block, block_size, &blocks, 12, buf_loc)) == NULL) {
      para_printf("Error reading from file...\n");
      return 0;
    }
    win_put_progress(prog, 0);
    if (blocks == 0)
      return inode->size;
    
    // did 12 blocks, now do (double) indirect blocks
    if ((ret = ext2_indirect(inode->u.block_array.indirect_block, block_size, &blocks, ret)) == NULL)
      return 0;
    if (blocks == 0)
      return inode->size;
    
    // read the dbl indirect block
    bit32u *indirect = (bit32u *) malloc(block_size * 512);
    if (read_sectors(inode->u.block_array.dbl_indirect_block, block_size, indirect) != block_size) {
      para_printf("Error reading indirect blocks...\n");
      mfree(indirect);
      return 0;
    }
    
    // do double indirect reads
    for (i=0; i<(block_size / sizeof(bit32u)); i++) {
      if ((ret = ext2_indirect(indirect[i], block_size, &blocks, ret)) == NULL) {
        mfree(indirect);
        return 0;
      }
      if (blocks == 0) {
        mfree(indirect);
        return inode->size;
      }
    }
    mfree(indirect);
    
    // we don't support tripple indirect blocks, but if we did, we would just move the dbl
    //  above to a function, like the single indirect below, then call the dbl, calling the
    //  single, calling the direct.
    // For now, return error
    para_printf("Triple Indirects are not supported...\n");
    return 0;    
  }
}

void *ext2_direct(const bit32u *lbas, const bit32u block_size, bit32u *blocks, const int cnt, void *ptr) {
  for (int i=0; i<cnt && *blocks; i++) {
    if (read_sectors(lbas[i] * block_size, block_size, ptr) != block_size) {
      para_printf("Error reading from file...\n");
      return NULL;
    }
    ptr = (void *) ((bit8u *) ptr + (block_size * 512));
    (*blocks)--;
  }
  return ptr;
}

// do ext2 indirect blocks reads
void *ext2_indirect(const bit32u lba, const bit32u block_size, bit32u *blocks, void *ptr) {
  void *ret = NULL;
  
  // read the indirect block
  bit32u *indirect = (bit32u *) malloc(block_size * 512);
  if (read_sectors(lba * block_size, block_size, indirect) != block_size) {
    para_printf("Error reading indirect blocks...\n");
    mfree(indirect);
    return 0;
  }
  
  if ((ret = ext2_direct(indirect, block_size, blocks, (block_size * (512 / sizeof(bit32u))), ptr)) == NULL) {
    para_printf("Error reading indirect blocks...\n");
    mfree(indirect);
    return NULL;
  }
  
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to load the root directory
bool ext2_load_data(struct S_EXT2_DATA *ext2_data) {
  struct S_EXT2_SUPER *super = malloc(8 * 512);
  struct S_EXT2_GROUP_DESC *group;
  bit32u block_size, first_group, inode_size;
  struct S_EXT2_INODE *inode;
  struct S_EXT2_DIR *root;
  int i;
  
  // now find the super.
  if (spc_key_F2)
    para_printf("EXT2: Reading Super Block\n");
  
  // EXT2 Super is always at LBA 2 ????? and could be up to 8 sectors long ?????
  if (read_sectors(2, 8, super) != 8) {
    para_printf("EXT2: Error Reading Super Block\n");
    mfree(super);
    return FALSE;
  }
  
  // we found the super
  ext2_data->super = super;
  
  // now load the root directory
  if (spc_key_F2)
    para_printf("EXT2: Reading Root Directory\n");
  
  // Calculate where the first group is
  block_size = 1024 << super->log_block_size;
  first_group = (block_size > 1024) ? block_size : block_size * 2;
  block_size >>= 9; // in sectors
  group = (struct S_EXT2_GROUP_DESC *) ((bit32u) super + first_group - 0x400);
  inode_size = (super->rev_level == 0) ? 128 : super->inode_size;
  
  // now read in one block.  The second inode will be in the first block
  inode = (struct S_EXT2_INODE *) malloc(block_size * 512);
  if (read_sectors(group->inode_table * block_size, block_size, inode) != block_size) {
    para_printf("EXT2: Did not read in inode table\n");
    goto load_data_error;
  }
  
  // Load the root directory into memory.  No need to load the bitmap.
  inode = (struct S_EXT2_INODE *) ((bit8u *) inode + inode_size);
  if ((inode->mode & EXT2_S_IFMT) != EXT2_S_IFDIR) {
    para_printf("EXT2: Did not find the Root Directory Inode\n");
    goto load_data_error;
  }
  
  root = (struct S_EXT2_DIR *) malloc(inode->blocks * 512);  // inode->file_size[0] ???
  if (ext2_load_file(inode, root, block_size) == 0) {
    para_printf("EXT2: Did not load Root Directory\n");
    goto load_data_error;
  }
  
  // we found the root
  ext2_data->root_dir = root;
  ext2_data->root_size = inode->blocks * 512;
  
  // we successfully loaded the data
  ext2_data->group = group;
  ext2_data->block_size = block_size;
  return ext2_data_valid = TRUE;
  
load_data_error:
  mfree(inode);
  mfree(super);
  return FALSE;
}

#endif  // FS_EXT2
