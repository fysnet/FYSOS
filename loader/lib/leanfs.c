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
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "loader.h"

#ifdef FS_LEAN

#include "disks.h"
#include "malloc.h"
#include "paraport.h"
#include "string.h"
#include "windows.h"

#include "leanfs.h"

bool leanfs_data_valid = FALSE;
struct S_LEANFS_DATA leanfs_data = { 0, };

bit32u fs_leanfs(const char *filename, void *target) {
  struct S_LEAN_SUPER *super;
  struct S_LEAN_DIRENTRY *root;
  char temp[64];
  
  // have we loaded the root yet
  if (!leanfs_data_valid)
    if (!leanfs_load_data(&leanfs_data))
      return 0;
  
  // make sure we always start at the root dir
  super = (struct S_LEAN_SUPER *) leanfs_data.super;
  if (!lean_load_dir(super->root_start[0], &leanfs_data))
    return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // the caller may have sent us a full path.
  // we need to move to that directory first
  char *s = filename;
  char *e = strchr(s, '\\');
  while (e) {
    strcpy(temp, s);
    temp[e - s] = '\0';
    root = lean_search(temp, (LEAN_ATTR_IFDIR >> 29), &leanfs_data);
    if (root) {
      if (!lean_load_dir(root->inode[0], &leanfs_data))
        return 0;
      s = e + 1;
      e = strchr(s, '\\');
    } else
      return 0;
  }
  
  root = lean_search(s, (LEAN_ATTR_IFREG >> 29), &leanfs_data);
  if (root)
    return lean_load_file(root->inode[0], target);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  //win_printf(main_win, "Did not find file...");
  
  return 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Search for file in directory
struct S_LEAN_DIRENTRY *lean_search(const char *name, const bit8u type, struct S_LEANFS_DATA *fs_data) {
  struct S_LEAN_DIRENTRY *root = (struct S_LEAN_INODE *) fs_data->root_dir;
  bit32u root_size = 0;
  bit8u temp[64];
  bit8u *n;
  int i;
  
  // find the dir entry
  while (root->rec_len) {
    if (root->type == type) {
      n = (bit8u *) ((bit32u) root + LEAN_DIRENTRY_NAME);
      for (i=0; i<root->name_len; i++)
        temp[i] = n[i];
      temp[i] = '\0';
      // the volume is case sensitive
      // however, per the FYSOS specs, the first in-sensitive name found is used...
      if (!stricmp(temp, name))
        return root;
    }
    
    root_size += (root->rec_len << 4);
    root = (struct S_LEAN_DIRENTRY *) ((bit32u) root + (root->rec_len << 4));
    if (root_size >= fs_data->root_size)
      break;
  }
  
  return NULL;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Load a file to given memory (LEAN)
// on entry:
//   sector number to inode (first sector)
//   location to store the file
// on return:
//   file size
bit32u lean_load_file(const bit32u inode_lba, void *target) {
  struct S_LEAN_EXTENT *extents = NULL;
  struct S_LEAN_INDIRECT *indirect;
  bit32u fsize, fcnt, i, j, k, progress, lba;
  bit8u *buffer = NULL;
  struct S_LEAN_INODE *inode;
  bit8u *p = (bit8u *) target;
  
  // allocate some memory for the buffer
  buffer = (bit8u *) malloc(LEANFS_MAX_SECT_CNT * 512);
  inode = (struct S_LEAN_INODE *) buffer;
  
  // we must start by loading at least one sector to get the inode
  if (read_sectors(inode_lba, 1, buffer) != 1) {
    win_printf(main_win, "Error reading from file...");
    mfree(buffer);
    return 0;
  }
  
  // check to make sure that 'magic' is correct
  if (inode->magic != LEAN_INODE_MAGIC) {
    win_printf(main_win, "Error reading from file...");
    mfree(buffer);
    return 0;
  }
  
  // gather info from the inode before we destroy the buffer[]
  fcnt = fsize = (int) inode->file_size[0];
  
  // if some of the data is in the inode (LEAN_ATTR_INLINEXTATTR == 0) move it first
  if (!(inode->attributes & LEAN_ATTR_INLINEXTATTR)) {
    j = (fcnt < (512 - LEAN_INODE_SIZE)) ? fcnt : (512 - LEAN_INODE_SIZE);
    memcpy(p, (void *) (buffer + LEAN_INODE_SIZE), j);
    fcnt -= j;
    p += j;
  }
  
  // was all of the file in the inode (1st sector)?
  if (fcnt == 0) {
    mfree(buffer);
    return fsize;
  }
  
  // create a list of sectors from the direct extent list and indirect extent list(s)
  // this works as long as there is 1365 or less extents total
  int ext_cnt = 0, cnt = inode->extent_count;
  bit32u next_ind_lba = inode->first_indirect[0];
  bit32u *start = (bit32u *) inode->extent_start;
  bit32u *size  = (bit32u *) inode->extent_size;
  
  // allocate some memory for the extents
  extents = (struct S_LEAN_EXTENT *) malloc(1365 * sizeof(struct S_LEAN_EXTENT));
  
  if (spc_key_F2)
    para_printf("LEAN: Reading File\n");
  
  // note: the buffer that 'inode' used will now be destroyed.
  while (cnt) {
    for (i=0; i<cnt; i++) {
      extents[ext_cnt].start[0] = *start++; start++; // skip high dword
      extents[ext_cnt].size = *size++;
      ext_cnt++;
    }
    
    if (next_ind_lba == 0)
      break;
    
    // read in 1 sector of indirect extents
    if (read_sectors(next_ind_lba, 1, buffer) != 1) {
      win_printf(main_win, "Error reading from file...");
      mfree(extents);
      mfree(buffer);
      return 0;
    }
    
    indirect = (struct S_LEAN_INDIRECT *) buffer;
    
    // check to make sure that 'magic' is correct
    if (indirect->magic != LEAN_INDIRECT_MAGIC) {
      win_printf(main_win, "Error reading from file...");
      mfree(extents);
      mfree(buffer);
      return 0;
    }
    
    next_ind_lba = indirect->next_indirect;
    cnt = indirect->extent_count;
    
    start = (bit32u *) indirect->extent_start;
    size  = (bit32u *) indirect->extent_size;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // the first sector we will read is the inode.  However,
  //  we have already retrieved the data from it, if LEAN_ATTR_INLINEDATA
  //  was set.  If it was not set, the inode takes up the whole sector.
  // either way, we need to skip the first sector of the first extent.
  extents[0].start[0]++;
  extents[0].size--;
  
  // initialize the progress bar
  win_init_progress(fsize);
  progress = 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now do the loading of the file, an extent at a time
  for (i=0; i<ext_cnt; i++) {
    lba = extents[i].start[0];
    while (extents[i].size) {
      j = (extents[i].size <= LEANFS_MAX_SECT_CNT) ? extents[i].size : LEANFS_MAX_SECT_CNT;
      if (read_sectors(lba, j, buffer) != j) {
        win_printf(main_win, "Error reading from file...");
        if (extents)
          mfree(extents);
        mfree(buffer);
        return 0;
      }
      
      // copy the data over to the target buffer
      k = (fcnt < (j * 512)) ? fcnt : (j * 512);
      memcpy(p, buffer, k);
      fcnt -= k;
      p += k;
      
      lba += j;
      extents[i].size -= j;
      
      progress += k;
      win_put_progress(progress, 0);
    }
  }
  
  if (extents)
    mfree(extents);
  mfree(buffer);
  
  win_put_progress(fsize, 0);
  //win_init_progress(fsize);
  return fsize;
}

/* This always skips the first dword since it is the crc field.
 *  The CRC is calculated as:
 *     crc = 0;
 *     loop (n times)
 *       crc = ror(crc) + dword[x]
 */
bit32u lean_calc_crc(const bit32u *ptr, const unsigned dwords) {
  bit32u crc = 0;
  
  for (unsigned i=1; i<dwords; i++)
    crc = (crc << 31) + (crc >> 1) + ptr[i];
  
  return crc;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// load in a directory
bool lean_load_dir(const bit32u lba, struct S_LEANFS_DATA *fs_data) {
  
  // if we are already at this lba, just return
  if (fs_data->root_lba == lba)
    return TRUE;
  
  // else, we need to read in the directory
  struct S_LEAN_INODE *inode = malloc(512);
  if (read_sectors(lba, 1, inode) != 1) {
    mfree(inode);
    return FALSE;
  }
  
  // resize the buffer to the size of the root
  bit32u sz = inode->file_size[0];
  inode = (struct S_LEAN_INODE *) mrealloc(inode, sz);
  if (lean_load_file(lba, inode) != sz) {
    mfree(inode);
    return FALSE;
  }
  
  // if we already had a dir loaded, free the buffer it used
  if (fs_data->root_dir)
    mfree(fs_data->root_dir);
  
  // we found the root
  fs_data->root_dir = inode;
  fs_data->root_size = sz;
  fs_data->root_lba = lba;
  
  return TRUE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to load the root directory
bool leanfs_load_data(struct S_LEANFS_DATA *fs_data) {
  struct S_LEAN_SUPER *super = malloc(512);
  int i;
  
  // now find the super.
  // read in up to the first 33 sectors to find the sector
  if (spc_key_F2)
    para_printf("LEAN: Reading Super Block\n");
  for (i=1; i<=33; i++) {
    if (read_sectors(i, 1, super) != 1) {
      mfree(super);
      return FALSE;
    }
    
    // if the magic is correct and the CRC matches, we found it.
    if ((super->magic == LEAN_SUPER_MAGIC) &&
         super->checksum == lean_calc_crc((bit32u *) super, (sizeof(struct S_LEAN_SUPER) / 4))) {
      
      // we found the super
      fs_data->super = super;
      fs_data->root_lba = 0;
      fs_data->root_dir = NULL;
      
      // now load the root directory
      if (spc_key_F2)
        para_printf("LEAN: Reading Root Directory\n");
      if (lean_load_dir(super->root_start[0], fs_data))
        return leanfs_data_valid = TRUE;
      break;
    }
  }
  
  mfree(super);
  return FALSE;
}

#endif  // FS_LEAN
