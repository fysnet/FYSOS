/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fysfs.c                                                            *
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
*   fysfs file system loader code.                                         *
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

#ifndef _FYSFS_C
#define _FYSFS_C

#include "./includes/boot.h"     // S_BOOT_DATA stuff
#include "./includes/fysfs.h"    // FYSFS stuff

// load the file 'filename' from disk to (farF) 'buf_loc'
int fs_fysfs(struct S_BOOT_DATA *boot_data, const char *filename, const bit32u buf_loc) {
  struct S_FYSFS_ROOT farE *root = (struct S_FYSFS_ROOT farE *) (boot_data->root_loc & 0x0000FFFF);
  struct S_FYSFS_SUPER farG *super = (struct S_FYSFS_SUPER farG *) (boot_data->other_loc & 0x0000FFFF); // superblock
  bit32u ret_size = 0, *s, ret_size = 0;
  int slot, j, k, cnt;
  bit8u buffer[512];
  bit32u fat_entries[128];
  bit32u farF *p = (bit32u farF *) buf_loc;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Our Root Segment is at FYSFS_ROOTSEG
  _asm (
    "  push es                                    \n"
    "  mov  ebx,PARAM0                            \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_ROOT_LOC " + 2]  \n"
    "  mov  es,ax                                 \n"
    "  push gs                                    \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_OTHER_LOC " + 2] \n"
    "  mov  gs,ax                                 \n"
  );
  
  // make sure the crc stuff is initialized and ready to use
  crc32_initialize();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // find the entry in the root
  for (slot=0; slot<super->root_entries; slot++) {
    fysfs_get_name(slot, buffer, root);
    if (!stricmp(buffer, filename)) {
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      //  Walk the (FYS)FAT chain reading all 'clusters' of the file.
      memset(fat_entries, 0, 128 * sizeof(bit32u));
      cnt = fysfs_get_fat_entries(slot, fat_entries, root);
      for (j=0; j<cnt; j++) {
        if (read_sectors(fat_entries[j], 1, buffer) != 1) {
          puts("Error reading from file...");
          return 0;
        }
        s = (bit32u *) buffer;
        k = 512 / 4;
        while (k--)
          *p++ = *s++;
      }      
      
      ret_size = root[slot].fsize[0];
      goto fs_fysfs_done;
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  puts("\7Did not find file...");
  
fs_fysfs_done:
  _asm ("  pop  gs \n");
  _asm ("  pop  es \n");
  return ret_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// return the name of the current chain in ds:si
//  on entry:
//   current slot to start in
//   256 byte buffer to place asciiz name
//   far pointer to root
//  on return
//   if this slot was a SLOT, and no errors
//     buffer filled with asciiz name.
//     return TRUE
//   else
//     return FALSE
bool fysfs_get_name(int slot, char *buffer, const struct S_FYSFS_ROOT farE *root) {
  unsigned len, buffer_len = 511;
  
  if ((root[slot].sig == S_FYSFS_ROOT_NEW) && fysfs_good_crc(&root[slot])) {
    len = (root[slot].namelen < buffer_len) ? root[slot].namelen : buffer_len;
	  memcpy(buffer, root[slot].name_fat, len);
    buffer += len;
    buffer_len -= len;
	  slot = root[slot].name_continue;
    while (slot && (buffer_len > 0)) {
      struct S_FYSFS_CONT farE *cont = (struct S_FYSFS_CONT farE *) &root[slot];
      if (fysfs_good_crc(cont) && (cont->sig == S_FYSFS_CONT_NAME)) {
        len = (cont->count < buffer_len) ? cont->count : buffer_len;
	      memcpy(buffer, cont->name_fat, len);
	      buffer += len;
        buffer_len -= len;
    	  slot = cont->next;
      } else
        break;
    }
  }
  
	*buffer = '\0';
	
	return TRUE;
}

bool fysfs_good_crc(void *slot) {
  struct S_FYSFS_ROOT farE *root = (struct S_FYSFS_ROOT farE *) slot;
  
  bit8u farE *p = (bit8u farE *) slot;
  bit8u buffer[sizeof(struct S_FYSFS_ROOT)];
  int i;
  
  // we must copy from farE to local so the crc32 will work
  for (i=0; i<sizeof(struct S_FYSFS_ROOT); i++)
    buffer[i] = p[i];
  
  bit8u org_crc = root->crc;
  root->crc = 0;
  bool ret = (org_crc == (bit8u) crc32(buffer, sizeof(struct S_FYSFS_ROOT)));
  root->crc = org_crc;
	
  return ret;
}

int fysfs_get_fat_entries(int slot, bit32u *buffer, const struct S_FYSFS_ROOT farE *root) {
  int i, next, cnt = 0;
  
  // get the ones in the SLOT entry first
  for (i=0; i<root[slot].fat_entries; i++) {
    bit32u farE *p = (bit32u farE *) (root[slot].name_fat + ((root[slot].namelen + 3) & ~0x03));
    *buffer++ = p[i];
    cnt++;
  }
  
  // now the chain of CONT entries
  next = root[slot].fat_continue;
  while (next) {
    struct S_FYSFS_CONT farE *cont = (struct S_FYSFS_CONT farE*) &root[next];
    
    // check a few things
    if ((cont->sig == S_FYSFS_CONT_FAT) && fysfs_good_crc(cont) && cont->count) {
      for (i=0; i<cont->count; i++) {
        bit32u farE *p = (bit32u farE *) cont->name_fat;
        *buffer++ = p[i];
        if (cont->flags & CONT_FLAGS_LARGE)
          buffer++;  // we need to skip hi part of 64-bit count (no check to see if it was > 0????)
        cnt++;
      }
    } else
      return cnt;
    
    // get the next CONT slot
    next = cont->next;
  }
  
  return cnt;
}

#endif // _FYSFS_C
