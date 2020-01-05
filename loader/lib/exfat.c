/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2020
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
 * Last update:  05 Jan 2020
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "loader.h"

#ifdef FS_EXFAT

#include "disks.h"
#include "paraport.h"
#include "stdlib.h"
#include "string.h"

#include "exfat.h"

#include "windows.h"


bool exfat_data_valid = FALSE;
struct S_ExFAT_DATA exfat_data;

bit32u fs_exfat(const char *filename, void *target) {
  int i, j, l, secs;
  bit32u cluster, file_size;
  char name[256], *n;
  bool fnd = FALSE;
  bit8u flags = 0;
  
  // have we loaded the root and fat yet
  if (!exfat_data_valid)
    if (!exfat_load_data(&exfat_data))
      return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // find the entry in the root
  struct S_EXFAT_ROOT *root = (struct S_EXFAT_ROOT *) exfat_data.root_dir;
  while (root->entry_type != EXFAT_DIR_EOD) {
    if (root->entry_type == EXFAT_DIR_ENTRY) {
      secs = root[0].type.dir_entry.sec_count;
      if (root[1].entry_type == EXFAT_DIR_STRM_EXT) {
        cluster = root[1].type.stream_ext.first_cluster;
        file_size = root[1].type.stream_ext.data_len[0];
        flags = root[1].type.stream_ext.flags;
        l = root[1].type.stream_ext.name_len;
        n = name;
        for (i=1; i<secs && l; i++)
          for (j=0; j<15 && l; j++, l--)
            *n++ = (char) root[1+i].type.file_name_ext.name[j];
        *n = '\0';
        if (fnd = (stricmp(name, filename) == 0))
          break;
      }
      root += secs;
    } else
      root++;
  }
  
  // did we find the file?
  if (!fnd) {
    win_printf(main_win, "Did not find file...\n");
    return 0;
  }
  
  // we found the file.
  // cluster = starting cluster
  // file_size = size of the file
  if (spc_key_F2)
    para_printf("ExFAT: Reading File\n");
  
  // initialize the progress bar
  j = 0;
  win_init_progress(file_size);
  
  // calculate count of clusters to read
  // we have to do this so if the NOFAT flag is set
  int count = (int) (file_size / exfat_data.sect_per_clust / exfat_data.bytes_per_sector) + 1;
  
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) exfat_data.vbr;
  void *p = target;
  bit32u *fat = (bit32u *) exfat_data.fat_loc;
  while ((cluster < 0xFFFFFFF8) && count--) {
    if (read_sectors(vbr->data_region_lba + ((cluster - 2) * exfat_data.sect_per_clust),
                     exfat_data.sect_per_clust, p) != exfat_data.sect_per_clust) {
        win_printf(main_win, "Error reading from file...\n");
        return 0;
    }
    j += (exfat_data.sect_per_clust << 9);  // clusters to bytes
    win_put_progress(j, 0);
    // if the NOFAT bit is set, we increment the cluster
    if (flags & EXFAT_NO_FAT_CHAIN_VALID)
      cluster++;
    else
      cluster = fat[cluster];
    p = (void *) ((bit32u) p + (exfat_data.sect_per_clust * exfat_data.bytes_per_sector));
  }
  
  // if we got here, we read the file okay.
  win_put_progress(file_size, 0);
  return file_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to retrieve the VBR, root directory, and FAT
bool exfat_load_data(struct S_ExFAT_DATA *data) {
  void *buffer = malloc(512);
  int root_clusters = 4, cnt = 0;

  // read in the VBR
  if (spc_key_F2)
    para_printf("ExFAT: Reading VBR\n");
  if (read_sectors(0, 1, buffer) != 1) {
    mfree(buffer);
    return FALSE;
  }
  data->vbr = buffer;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) buffer;
  data->sect_per_clust = (1 << vbr->log_sects_per_clust);
  data->bytes_per_sector = (1 << vbr->log_bytes_per_sect);
  
  // read the FAT
  if (spc_key_F2)
    para_printf("ExFAT: Reading FAT\n");
  data->fat_loc = malloc(vbr->sect_per_fat * 512);
  if (read_sectors(vbr->first_fat, vbr->sect_per_fat, data->fat_loc) != vbr->sect_per_fat) {
    mfree(data->vbr);
    mfree(data->fat_loc);
    return FALSE;
  }
  
  // read the root
  if (spc_key_F2)
    para_printf("ExFAT: Reading Root Directory\n");
  data->root_dir = malloc(root_clusters * (data->bytes_per_sector * data->sect_per_clust));
  void *p = data->root_dir;
  bit32u *fat = (bit32u *) data->fat_loc;
  bit32u cluster = vbr->root_dir_cluster;
  while ((cluster < 0xFFFFFFF8) && root_clusters) {
    if (read_sectors(vbr->data_region_lba + ((cluster - 2) * data->sect_per_clust),
      data->sect_per_clust, p) != data->sect_per_clust) {
        mfree(data->vbr);
        mfree(data->fat_loc);
        mfree(data->root_dir);
        return FALSE;
    }
    cluster = fat[cluster];
    p = (void *) ((bit32u) p + (data->sect_per_clust * data->bytes_per_sector));
    root_clusters--;
  }
  
  return exfat_data_valid = TRUE;
}

/*
bit32u exfat_vrb_crc(const bit8u *buffer, const int len) {
  bit32u crc = 0;
  
  for (int i=0; i<len; i++) {
    if (i == 106 || i == 107 || i == 112)
      continue;
    crc = ((crc << 31) | (crc >> 1)) + (bit32u) buffer[i];
  }
  
  return crc;
}
*/

#endif   // FS_EXT2
