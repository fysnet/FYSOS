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

#if defined(FS_FAT12) || defined(FS_FAT16) || defined(FS_FAT32)

#include "fat.h"
#include "disks.h"
#include "malloc.h"
#include "paraport.h"
#include "string.h"
//#include "stdio.h"
#include "sys.h"
#include "windows.h"

bool fat_data_valid = FALSE;
struct S_FAT_DATA fat_data;

bit32u fs_fat(const char *filename, void *target) {
  struct S_FAT_ROOT *root;
  bit8u *targ = (bit8u *) target;
  bit8u buffer[32];
  bit32u start_lba, cur_clust, clust_cnt, next_clust;
  int i, j = 0;
  
  // have we loaded the root and fat yet
  if (!fat_data_valid)
    if (!fat_load_data(&fat_data))
      return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // find the entry in the root
  root = (struct S_FAT_ROOT *) fat_data.root_dir;
  for (i=0; i<fat_data.root_entries; i++) {
    convert_fat83(root, buffer);
    if (!stricmp(buffer, filename))
      break;
    root++;
  }
  
  // did we find the file?
  if (i == fat_data.root_entries) {
    win_printf(main_win, "Did not find file...\n");
    return 0;
  }
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Walk the FAT chain reading all 'clusters' of the file.
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Calculate the first data sector
  start_lba = fat_data.num_fats * fat_data.sec_per_fat + fat_data.sec_resv;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // start_lba now points to root sector
  // need to skip the root if we are a FAT12/16 fs
#if defined(FS_FAT12) || defined(FS_FAT16)
#ifdef FS_FAT32
  if (sys_block.boot_data.file_system != FS_FAT32)
#endif
    start_lba += (fat_data.root_entries >> 4);
#endif
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // since the first two cluster numbers are reserved,
  // the first cluster number is 02h, so all reads are 2 based
  cur_clust = root->strtclst;
#ifdef FS_FAT32
  if (sys_block.boot_data.file_system == FS_FAT32)
    cur_clust |= (root->type.fat32.strtclsthi << 16);
#endif
  
  // initialize the progress bar
  win_init_progress(root->filesize);
  
  if (spc_key_F2)
    para_printf("FAT: Reading File\n");
  
  while (cur_clust < 0xFFFFFFFF) {
    clust_cnt = 1;
    next_clust = cur_clust;
    
    // we check to see if the next cluster is consecutively on the disk,
    //  (physically just after this one), and up the count to read it as well.
    next_clust = fat_get_next_cluster(fat_data.fat_loc, next_clust);
    while (1) {
      if (next_clust != (cur_clust + clust_cnt))
        break;
      clust_cnt++;
      next_clust = fat_get_next_cluster(fat_data.fat_loc, next_clust);
    }
    bit32u sectors = clust_cnt * fat_data.sect_per_clust;
    bit32u sect_lba = start_lba + ((cur_clust - 2) * fat_data.sect_per_clust);
    while (sectors > 0) {
      // so that we update the progress, we only read so many sectors at a time
      bit32u sec_cnt = (sectors > 16) ? 16 : sectors;
      if (read_sectors(sect_lba, sec_cnt, targ) == sec_cnt) {
        targ += (sec_cnt << 9);
        j += (sec_cnt << 9);  // sectors to bytes
        win_put_progress(j, 0);
      } else {
        win_printf(main_win, "Error reading from file...\n");
        return 0;
      }
      sectors -= sec_cnt;
      sect_lba += sec_cnt;
    }
    
    // get next cluster number
    cur_clust = fat_get_next_cluster(fat_data.fat_loc, cur_clust + (clust_cnt - 1));
  }
  
  // if we got here, we read the file okay.
  win_put_progress(root->filesize, 0);
  return root->filesize;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// "mount" the file system.
// we need to retrieve the BPB, extract some info, and
//  then load the root directory and fat 
bool fat_load_data(struct S_FAT_DATA *fat_data) {
  int secs;
  bit8u *buffer = (bit8u *) malloc(512);
#if defined(FS_FAT12) || defined(FS_FAT16)
  struct S_FAT1216_BPB *bpb = buffer;
#endif
#ifdef FS_FAT32
  struct S_FAT32_BPB *bpb32 = buffer;
#endif
  
  // read in the BPB
  if (spc_key_F2)
    para_printf("FAT: Reading BPB\n");
  if (read_sectors(0, 1, buffer) != 1) {
    mfree(buffer);
    return FALSE;
  }
  
  switch (sys_block.boot_data.file_system) {
#if defined(FS_FAT12) || defined(FS_FAT16)
    case FS_FAT12:
    case FS_FAT16:
      // fill in the data
      fat_data->root_entries = bpb->root_entrys;
      fat_data->sec_per_fat = bpb->sect_per_fat;
      fat_data->sect_per_clust = bpb->sect_per_clust;
      fat_data->sec_resv = bpb->sect_reserved;
      fat_data->num_fats = bpb->fats;
      
      // read the root
      if (spc_key_F2)
        para_printf("FAT: Reading Root Directory\n");
      secs = (bpb->root_entrys >> 4);  // div by 16 = ((entries * 32) / 512)
      fat_data->root_dir = malloc(secs * 512);
      if (read_sectors(fat_data->sec_resv +             // skip reserved sectors
                      (bpb->sect_per_fat * bpb->fats),  // skip fats
                       secs, fat_data->root_dir) != secs) {
                         mfree(fat_data->root_dir);
                         mfree(buffer);
                         return FALSE;
      }
      
      // read the FAT
      if (spc_key_F2)
        para_printf("FAT: Reading FAT\n");
      secs = (bpb->sect_per_fat * bpb->fats);
      fat_data->fat_loc = malloc(secs * 512);
      if (read_sectors(fat_data->sec_resv,       // skip reserved sectors
                       secs, fat_data->fat_loc) != secs) {
                         mfree(fat_data->root_dir);
                         mfree(fat_data->fat_loc);
                         mfree(buffer);
                         return FALSE;
      }
      break;
#endif

#ifdef FS_FAT32
    case FS_FAT32:
      // fill in the data
      fat_data->root_entries = 0; // calculated via the FAT32 code below
      fat_data->sec_per_fat = bpb32->sect_per_fat32;
      fat_data->sect_per_clust = bpb32->sect_per_clust;
      fat_data->sec_resv = bpb32->sect_reserved;
      fat_data->num_fats = bpb32->fats;
      
      // read the FAT first
      if (spc_key_F2)
        para_printf("FAT: Reading FAT\n");
      secs = (bpb32->sect_per_fat32 * bpb32->fats);
      fat_data->fat_loc = malloc(secs * 512);
      if (read_sectors(fat_data->sec_resv,       // skip reserved sectors
                       secs, fat_data->fat_loc) != secs) {
                         mfree(buffer);
                         return FALSE;
      }
      
      // read the root
      // the root is a file just like a file is. therefore, read it as if it was a file
      //  we will start with 16 clusters and go from there
      if (spc_key_F2)
        para_printf("FAT: Reading Root Directory\n");
      int cur_max = 16, i = 0;
      fat_data->root_dir = malloc(cur_max * (512 * fat_data->sect_per_clust));
      bit32u *fat = (bit32u *) fat_data->fat_loc;
      bit32u cluster = bpb32->root_base_cluster;
      bit32u data_region_lba = bpb32->sect_reserved + (bpb32->sect_per_fat32 * bpb32->fats);
      while (cluster < 0xFFFFFFF8) {
        // if we have reached our limit, expand the buffer
        if (i == cur_max) {
          cur_max += 16;
          fat_data->root_dir = mrealloc(fat_data->root_dir, cur_max * (512 * fat_data->sect_per_clust));
        }
        if (read_sectors(data_region_lba + ((cluster - 2) * fat_data->sect_per_clust),  // lba
                         fat_data->sect_per_clust,   // count
                         (void *) ((bit32u) fat_data->root_dir + ((i * fat_data->sect_per_clust) * 512))  // location
           ) != fat_data->sect_per_clust) {
            mfree(buffer);
            mfree(fat_data->fat_loc);
            mfree(fat_data->root_dir);
            return FALSE;
        }
        i++;
        cluster = fat_get_next_cluster(fat, cluster);
        fat_data->root_entries += (fat_data->sect_per_clust * (512 / 32));
      }
      break;
#endif
  }
  
  mfree(buffer);
  return fat_data_valid = TRUE;
}

// extract the filename from the root
void convert_fat83(const struct S_FAT_ROOT *root, char *filename) {
  char *p;
  char *f = filename;
  int i;
  
  p = root->name;
  i = 8;
  while ((*p != 0x20) && i) {
    *f++ = *p++;
    i--;
  }
  
  *f++ = '.';
  
  p = root->ext;
  i = 3;
  while ((*p != 0x20) && i) {
    *f++ = *p++;
    i--;
  }
  
  // if no ext given, drop the '.'
  if (i == 3)
    f--;
  
  // asciiz it
  *f++ = '\0';
}

// get next cluster number
// this code assumes the fat is <= 128 sectors
bit32u fat_get_next_cluster(void *fat_loc, bit32u cur_clust) {
  bool  odd;
  bit32u test;
  
  switch (sys_block.boot_data.file_system) {
#ifdef FS_FAT12
    case FS_FAT12:
      odd = (bool) (cur_clust & 1);
      cur_clust = * (bit16u *) ((bit32u) fat_loc + (cur_clust + (cur_clust / 2)));
      if (odd)
        cur_clust >>= 4;
      cur_clust &= 0xFFF;
      test = 0x0FF8;
      break;
#endif
#ifdef FS_FAT16
    case FS_FAT16:
      cur_clust = * (bit16u *) ((bit32u) fat_loc + (cur_clust * 2));
      test = 0xFFF8;
      break;
#endif
#ifdef FS_FAT32
    case FS_FAT32:
      cur_clust = * (bit32u *) ((bit32u) fat_loc + (cur_clust * 4));
      test = 0x0FFFFFF8;
      break;
#endif
  }
  
  if (cur_clust >= test)
    return 0xFFFFFFFF;
  else
    return cur_clust;
}

#endif  // fat12, fat16, fat32
