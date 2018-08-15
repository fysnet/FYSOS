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

#ifdef FS_SFS

#include "disks.h"
#include "malloc.h"
#include "paraport.h"
#include "string.h"
#include "sys.h"
#include "windows.h"

#include "sfs.h"

bool sfs_data_valid = FALSE;
struct S_SFS_DATA sfs_data;

bit32u fs_sfs(const char *filename, void *target) {
  struct S_SFS_FILE *file;
  struct S_SFS_DIR  *dir;
  int i;
  bit32u progress;
  
  // have we loaded the root yet
  if (!sfs_data_valid)
    if (!sfs_load_data(&sfs_data))
      return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Search for file in Index Data Area
  file = (struct S_SFS_FILE *) sfs_data.ida;
  i = sfs_data.ida_idx;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // The caller may pass a path along with it, 
  //  e.g.:  system\boot\file.txt
  sfs_convert_path(filename);
  
  // find the entry
  while (i < sfs_data.ida_entries) {
    switch (file[i].type) {
      case SFS_ENTRY_FILE:
        if (sfs_check_crc(&file[i], ((1 + file[i].num_cont) * SFS_ENTRY_SIZE))) {
          if (!stricmp(file[i].name, filename)) {
            // initialize the progress bar
            win_init_progress(file[i].file_len[0]);
            progress = 0;
            
            // Now load the file.
            bit32u lba = file[i].start_block[0];
            while (lba <= file[i].end_block[0]) {
              if (read_sectors(/*sfs_data.data_start + */lba, 1, target) == 1) {
                lba++;
                target = (void *) ((bit8u *) target + 512);
                progress += 512;
                win_put_progress(progress, 0);
              } else
                return 0;
            }
            
            win_put_progress(file[i].file_len[0], 0);
            return file[i].file_len[0];
          }
          i += file[i].num_cont;
        }
        break;
      // we have to watch for DIR, DIR_DEL, and FILE_DEL entries too,
      //   so we can skip their continuation slots, if present
      case SFS_ENTRY_DIR:
      case SFS_ENTRY_DIR_DEL:
      case SFS_ENTRY_FILE_DEL:
        if (sfs_check_crc(&dir[i], ((1 + dir[i].num_cont) * SFS_ENTRY_SIZE)))
          i += dir[i].num_cont;
        break;
    }
    i++;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  //win_printf(main_win, "Did not find file...");
  
  return 0;
}

bool sfs_check_crc(const void *ptr, int cnt) {
  bit8u *p = (bit8u *) ptr;
  bit8u crc = 0;
  
  while (cnt--)
    crc += *p++;
  
  return crc == 0;
}

// Convert a full path to a path SFS understands
// For the SFS, all we have to do is change '\' to '/',
//  making sure there is no '\' at the first.
void sfs_convert_path(char *filename) {
  int i = 0;
  
  if ((filename[0] == '\\') || (filename[0] == '/'))
    strcpy(filename, filename + 1);
  
  while (filename[i]) {
    if (filename[i] == '\\')
      filename[i] = '/';
    i++;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to load the root directory
bool sfs_load_data(struct S_SFS_DATA *sfs_data) {
  bit8u *buffer = (bit8u *) malloc(512);
  struct S_SFS_SUPER *super = (struct S_SFS_SUPER *) &buffer[0x18E];
  
  // read in the super (LBA 0)
  if (spc_key_F2)
    para_printf("SFS: Reading Super Block\n");
  if (read_sectors(0, 1, buffer) != 1) {
    mfree(buffer);
    return FALSE;
  }
    
  // if the magic is correct and the CRC matches, we found it.
  // TODO: check that version is (major == 1) && (minor >= 10)
  if ((super->magic_version == ((SFS_VERSION << 24) | SFS_SUPER_MAGIC)) && sfs_check_crc(&super->magic_version, 18)) {
    // we found the super
    sfs_data->super = super;
    
    // now load the Index Data Area
    if (spc_key_F2)
      para_printf("SFS: Reading Index Data Area\n");
    
    bit32u index_size = super->index_size[0];
    bit32u blocks = ((index_size + 511) / 512);
    bit32u base = super->total_blocks[0] - blocks;
    sfs_data->ida_entries = (blocks * 512) / SFS_ENTRY_SIZE;
    sfs_data->ida_idx = ((blocks * 512) - index_size) / SFS_ENTRY_SIZE;
    sfs_data->ida = malloc(blocks * 512);
    if (read_sectors(base, blocks, sfs_data->ida) != blocks) {
      mfree(sfs_data->ida);
      mfree(buffer);
      return FALSE;
    }
    
    sfs_data->data_start = super->resv_blocks;
    return sfs_data_valid = TRUE;
  }
  
  mfree(buffer);
  return FALSE;
}

#endif  // FS_SFS
