/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: exfat.c                                                            *
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
*   exFAT file system loader code.                                         *
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
*  This code is not anywhere complete yet.  I have copied the code from    *
*   fat.c to start, but need to complete this code.                        *
*                                                                          *
***************************************************************************/

#ifndef _EXFAT_C
#define _EXFAT_C

#include "./includes/exfat.h"    // EXFAT stuff

// Load the file 'filename' from disk to (farF) 'buf_loc'
int fs_exfat(struct S_BOOT_DATA *boot_data, const char *filename, const bit32u buf_loc) {
//  struct S_FAT_ROOT farE *root = (struct S_FAT_ROOT farE *) 0x00000000;   // FAT_ROOTSEG:0
//  int cnt = boot_data->root_entries;
  bit32u /*start_lba, cur_clust, */ ret_size = 0;
//  bit8u buffer[512];
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Our Root Segment is at FAT_ROOTSEG
  _asm (
    "  push es \n"
    "  mov  ax," #EXFAT_ROOTSEG " \n"
    "  mov  es,ax \n"
  );


  /*
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
  // (since buf_loc is a far pointer and could be > 0xFFFF, we read
  //  to a local buffer, then copy to the far buffer)
  bit32u farF *p = (bit32u farF *) buf_loc;
  bit32u *s, test;
  int i;
  
  while (1) {
    if (read_sectors(start_lba + ((cur_clust - 2) * boot_data->SecPerClust),
                     boot_data->SecPerClust, buffer) == boot_data->SecPerClust) {
      i = (boot_data->SecPerClust << 7);  // sectors to dwords
      s = (bit32u *) buffer;
      while (i--)
        *p++ = *s++;
      ret_size += (boot_data->SecPerClust << 9);  // sectors to bytes
    } else {
      puts("Error reading from file...");
      goto fs_fat_done;
    }
    
    // get next cluster number
    // this code assumes the fat is <= 128 sectors
    _asm (
      "  push gs \n"
      "  mov  ax," #FAT_FATSEG " \n"
      "  mov  gs,ax  \n"
    );
    
    switch (boot_data->file_system) {
      case FS_FAT12:
        cur_clust = * (bit16u farG *) (cur_clust + (cur_clust / 2));
        if (cur_clust & 1)
          cur_clust >>= 3;
        cur_clust &= 0xFFF;
        test = 0x0FF8;
        break;
      case FS_FAT16:
        cur_clust = * (bit16u farG *) cur_clust;
        test = 0xFFF8;
        break;
      case FS_FAT32:
        cur_clust = * (bit32u farG *) (cur_clust << 1);
        test = 0x0FFFFFF8;
        break;
    }
    
    _asm ("  pop  gs \n");
    
    if (cur_clust >= test)
      break;
  }

*/  
fs_exfat_done:
  _asm ("  pop  es \n");
  return ret_size;
}
/*
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
*/

#endif  // _EXFAT_C