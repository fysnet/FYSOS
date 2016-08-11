/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: lean.c                                                             *
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
*   leanfs file system loader code.                                        *
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
*   The original LEAN file system specification can be found at:           *
*     http://freedos-32.sourceforge.net/lean/                              *
*                                                                          *
***************************************************************************/

#ifndef _LEAN_C
#define _LEAN_C

#include "./includes/boot.h"     // S_BOOT_DATA stuff
#include "./includes/lean.h"     // LEAN stuff

// load the file 'filename' from disk to (farF) 'buf_loc'
int fs_lean(struct S_BOOT_DATA *boot_data, const char *filename, const bit32u buf_loc) {
  struct S_LEAN_INODE farE *root = (struct S_LEAN_INODE farE *) (boot_data->root_loc & 0x0000FFFF);   // LEAN_ROOTSEG:LEAN_ROOTOFF
  struct S_LEAN_DIRENTRY farE *dir;
  bit8u farE *n;
  bit32u root_size = 0, ret_size = 0;
  int i;
  bit8u temp[64];
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Our Root Segment is at LEAN_ROOTSEG
  _asm (
    "  push es \n"
    "  mov  ebx,PARAM0                           \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_ROOT_LOC " + 2] \n"
    "  mov  es,ax \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Search for file in root directory
  if (root->attributes & LEAN_ATTR_INLINEXTATTR)
    dir = (struct S_LEAN_DIRENTRY farE *) LEAN_INODE_SIZE;
  else
    dir = (struct S_LEAN_DIRENTRY farE *) boot_data->sect_size;
  
  // find the dir entry
  while (dir->rec_len) {
    if (dir->type == (LEAN_ATTR_IFREG >> 29)) {
      n = (bit8u farE *) ((bit32u) dir + LEAN_DIRENTRY_NAME);
      for (i=0; i<dir->name_len; i++)
        temp[i] = n[i];
      temp[i] = '\0';
      // we assume the volume is case insensitive 
      //  (we would have to read the super to find out)
      //  (or send it along with boot_data)
      if (!stricmp(temp, filename)) {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Now load the file.
        ret_size = lean_load_file(dir->inode[0], buf_loc);
        goto fs_lean_done;
      }
    }
    
    root_size += (dir->rec_len << 4);
    dir = (struct S_LEAN_DIRENTRY farE *) ((bit32u) dir + (dir->rec_len << 4));
    if (root_size >= root->file_size[0])
      break;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  puts("\7Did not find file...");
  
fs_lean_done:
  _asm ("  pop  es \n");
  return ret_size;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Load a file to given memory (LEAN)
// on entry:
//   sector number to inode (first sector)
//   location to store the file
// on return:
//   file size
int lean_load_file(const bit32u inode_lba, const bit32u buf_loc) {
  struct S_LEAN_EXTENT extents[1365];
  struct S_LEAN_INDIRECT *indirect;
  int fsize, i, j, k;
  bit8u buffer[512*40];
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) buffer;
  bit32u farF *p = (bit32u farF *) buf_loc;
  bit32u *s, lba;
  
  // we must start by loading at least one sector to get the inode
  if (read_sectors(inode_lba, 1, buffer) != 1) {
    puts("Error reading from file...");
    return 0;
  }

  // check to make sure that 'magic' is correct
  if (inode->magic != LEAN_INODE_MAGIC) {
    puts("Error reading from file...");
    return 0;
  }
  
  // gather info from the inode before we destroy the buffer[]
  fsize = (int) inode->file_size[0];
  
  // if some of the data is in the inode (LEAN_ATTR_INLINEXTATTR) move it first
  // doesn't matter if we 'move' more than the filesize bytes.
  if (inode->attributes & LEAN_ATTR_INLINEXTATTR) {
    s = (bit32u *) (buffer + LEAN_INODE_SIZE);
    j = ((512 - LEAN_INODE_SIZE) / 4);
    while (j--)
      *p++ = *s++;
  }
  
  // create a list of sectors from the direct extent list and indirect extent list(s)
  // this works as long as there is 1365 or less extents total
  int ext_cnt = 0, cnt = inode->extent_count;
  bit32u next_ind_lba = inode->first_indirect[0];
  bit32u *start = (bit32u *) inode->extent_start;
  bit32u *size  = (bit32u *) inode->extent_size;
  
  // note: the buffer 'inode' uses will now be destroyed.
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
      puts("Error reading from file...");
      return 0;
    }
    
    indirect = (struct S_LEAN_INDIRECT *) buffer;
    
    // check to make sure that 'magic' is correct
    if (indirect->magic != LEAN_INDIRECT_MAGIC) {
      puts("Error reading from file...");
      return 0;
    }
    
    next_ind_lba = indirect->next_indirect;
    cnt = indirect->extent_count;
    
    start = (bit32u *) indirect->extent_start;
    size  = (bit32u *) indirect->extent_size;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now do the loading of the file, an extent at a time
  for (i=0; i<ext_cnt; i++) {
    lba = extents[i].start[0];
    while (extents[i].size) {
      j = (extents[i].size <= 40) ? extents[i].size : 40;
      if (read_sectors(lba, j, buffer) != j) {
        puts("Error reading from file...");
        return 0;
      }
      
      s = (bit32u *) buffer;
      k = (j * (512 / 4));
      while (k--)
        *p++ = *s++;
      
      lba += j;
      extents[i].size -= j;
    }
  }
  
  return fsize;
}

#endif  // _LEAN_C
