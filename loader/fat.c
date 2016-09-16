/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fat.c                                                              *
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
*   fat file system loader code.                                           *
*                                                                          *
* BUILT WITH:   NewBasic Compiler and Assembler                            *
*                 http://www.fysnet/newbasic.htm                           *
*               NBC   ver 00.20.25                                         *
*          Command line: nbc loader<enter>                                 *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm loader loader.sys -d<enter>                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
***************************************************************************/

#ifndef _FAT_C
#define _FAT_C

#define FAT_MAX_SECTOR_COUNT  16

#include "./includes/boot.h"     // S_BOOT_DATA stuff
#include "./includes/fat.h"      // FAT12/16/32 stuff

// Load the file 'filename' from disk to (farF) 'buf_loc'
int fs_fat(struct S_BOOT_DATA *boot_data, const char *filename, const bit32u buf_loc) {
  struct S_FAT_ROOT farE *root = (struct S_FAT_ROOT farE *) (boot_data->root_loc & 0x0000FFFF);   // FAT_ROOTSEG:FAT_OFFSET
  int cnt = boot_data->root_entries;
  bit32u start_lba, cur_clust, next_clust, clust_cnt, ret_size = 0;
  bit8u buffer[FAT_MAX_SECTOR_COUNT * 512];  // boot_data->SecPerClust MAX Value (8???)
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Our Root Segment is at FAT_ROOTSEG
  _asm (
    "  push es \n"
    "  mov  ebx,PARAM0 \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_ROOT_LOC " + 2] \n"
    "  mov  es,ax      \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // find the entry in the root
  while (cnt--) {
    convert_fat83(root, buffer);
    if (!stricmp(buffer, filename))
      break;
    root++;
  }
  
  // didn't find the file
  if (!cnt) {
    puts("\7Did not find file...");
    goto fs_fat_done;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Walk the FAT chain reading all 'clusters' of the loader file.
  //  Calculate the first data sector
  start_lba = boot_data->FATs * boot_data->SecPerFat + boot_data->SecRes;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // start_lba now points to root sector
  // need to skip the root if we are a FAT12/16 fs
  if (boot_data->file_system != FS_FAT32)
    start_lba += (boot_data->root_entries >> 4);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // since the first two cluster numbers are reserved,
  // the first cluster number is 02h, so all reads are 2 based
  cur_clust = root->strtclst;
  if (boot_data->file_system == FS_FAT32)
    cur_clust |= (root->type.fat32.strtclsthi << 16);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // since FAT12 and FAT16 should never have a FAT larger than 64k,
  //  we don't have to worry about 64k wrapping with our addressing
  // since buf_loc is a far pointer and could be > 0xFFFF, we read
  //  to a local buffer, then copy to the far buffer
  bit32u farF *p = (bit32u farF *) buf_loc;
  bit32u *s, j = 0, sectors;
  int i;
  
  // initialize the progress bar
  init_progress(root->filesize);

  while (cur_clust < 0xFFFFFFFF) {
    clust_cnt = 1;
    next_clust = cur_clust;
    
    // we check to see if the next cluster is consecutively on the disk,
    //  (physically just after this one), and up the count to read it as well.
    // Read up to FAT_MAX_SECTOR_COUNT sectors at a time.
    next_clust = fat_get_next_cluster(boot_data, next_clust);
    while (1) {
      if (next_clust != (cur_clust + clust_cnt))
        break;
      if (((clust_cnt + 1) * boot_data->SecPerClust) > FAT_MAX_SECTOR_COUNT)
        break;
      clust_cnt++;
      next_clust = fat_get_next_cluster(boot_data, next_clust);
    }
    
    sectors = clust_cnt * boot_data->SecPerClust;
    if (read_sectors(start_lba + ((cur_clust - 2) * boot_data->SecPerClust), sectors, buffer) == sectors) {
      i = (sectors << 7);  // sectors to dwords
      s = (bit32u *) buffer;
      while (i--)
        *p++ = *s++;
      
      j += (sectors << 9);  // sectors to bytes
      put_progress(j, 0);
    } else {
      puts("Error reading from file...");
      goto fs_fat_done;
    }
    
    // get next cluster number
    cur_clust = fat_get_next_cluster(boot_data, cur_clust + (clust_cnt - 1));
  }
  
  // if we got here, we read the file okay.
  ret_size = root->filesize;
  
fs_fat_done:
  _asm ("  pop  es \n");
  return ret_size;
}

// get next cluster number
// this code assumes the fat is <= 128 sectors
bit32u fat_get_next_cluster(struct S_BOOT_DATA *boot_data, bit32u cur_clust) {
  bool  odd;
  bit32u test;
  
  _asm (
    "  push gs                                    \n"
    "  mov  ebx,PARAM0                            \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_OTHER_LOC " + 2] \n"
    "  mov  gs,ax                                 \n"
  );
  
  switch (boot_data->file_system) {
    case FS_FAT12:
      odd = (bool) (cur_clust & 1);
      cur_clust = * (bit16u farG *) (cur_clust + (cur_clust / 2));
      if (odd)
        cur_clust >>= 4;
      cur_clust &= 0xFFF;
      test = 0x0FF8;
      break;
    case FS_FAT16:
      cur_clust = * (bit16u farG *) (cur_clust * 2);
      test = 0xFFF8;
      break;
    case FS_FAT32:
      cur_clust = * (bit32u farG *) (cur_clust * 4);
      test = 0x0FFFFFF8;
      break;
  }
  
  _asm ("  pop  gs \n");

  if (cur_clust >= test)
    return 0xFFFFFFFF;
  else
    return cur_clust;
}

// extract the filename from the root
void convert_fat83(struct S_FAT_ROOT farE *root, char *filename) {
  char farE *p;
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
  
  if (i == 3)
    f--;
  *f++ = '\0';
}

#endif  // _FAT_C
