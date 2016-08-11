/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: ext2.c                                                             *
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
*   ext2 file system loader code.                                          *
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

#ifndef _EXT2_C
#define _EXT2_C

#include "./includes/boot.h"     // S_BOOT_DATA stuff
#include "./includes/ext2.h"     // EXT2 stuff

// load the file 'filename' from disk to (farF) 'buf_loc'
int fs_ext2(struct S_BOOT_DATA *boot_data, const char *filename, const bit32u buf_loc) {
  struct S_EXT2_DIR farE *root = (struct S_EXT2_DIR farE *) (boot_data->root_loc & 0x0000FFFF);  // EXT2_ROOTSEG:EXT2_ROOTOFF
  struct S_EXT2_SUPER farE *super = (struct S_EXT2_SUPER farE *) (boot_data->other_loc & 0x0000FFFF); // superblock
  bit8u farE *n;
  bit32u lba, ofs, root_size, cur_size = 0, ret_size = 0, sect_block;
  int i;
  bit8u temp[512];
  struct S_EXT2_INODE *inode = (struct S_EXT2_INODE *) temp;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Our Root Segment is at EXT2_ROOTSEG
  // Our Super Segment is the same one.
  // Our Group Segment is the same one.
  _asm (
    "  push es                                   \n"
    "  mov  ebx,PARAM0                           \n"
    "  mov  ax,[ebx + " #S_BOOT_DATA_ROOT_LOC " + 2] \n"
    "  mov  es,ax                                \n"
  );
  
  root_size = boot_data->misc0;  // root size
  sect_block = boot_data->misc2 >> 9;
  if (sect_block > LARGEST_BLOCK_SIZE) {
    printf("We don't support block sizes larger than %i (%i)", LARGEST_BLOCK_SIZE, sect_block);
    goto fs_ext2_done;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Search for file in root directory
  while (1) {
    if (root->inode > 0) {
      n = (bit8u farE *) ((bit32u) root + sizeof(struct S_EXT2_DIR));
      for (i=0; i<root->name_len; i++)
        temp[i] = n[i];
      temp[i] = '\0';
      if (!stricmp(temp, filename)) {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  get inode table
        struct S_EXT2_GROUP_DESC farE *group = (struct S_EXT2_GROUP_DESC farE *) (* (bit16u farE *) boot_data->misc1);
        //  calculate offset within table to read
        lba = (super->inode_size * (root->inode - 1)) / 512;  // sector number
        ofs = (super->inode_size * (root->inode - 1)) % 512;  // offset within sector
        // add sector of inode table
        lba += group->inode_table * sect_block;
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  read in the inode
        if (read_sectors(lba, 1, inode) != 1) {
          puts("Error reading from file...");
          goto fs_ext2_done;
        }
        
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        //  make sure the mode == regular file
        if ((inode->mode & EXT2_S_IFMT) == EXT2_S_IFREG) {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          //  load file
          ret_size = ext2_load_file(inode, buf_loc, boot_data->misc2);
          goto fs_ext2_done;
        }
        
        // else if we get here, there was an error with the file
        puts("Error with file...");
        goto fs_ext2_done;
      }
    }
    
    if (root->rec_len == 0)
      break;
    
    cur_size += root->rec_len;
    root = (struct S_EXT2_DIR farE *) ((bit32u) root + root->rec_len);
    if (cur_size >= root_size)
      break;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if we get here, the file wasn't found
  puts("\7Did not find file...");
  
fs_ext2_done:
  _asm ("  pop  es \n");
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
int ext2_load_file(struct S_EXT2_INODE *inode, const bit32u buf_loc, const bit32u block_size) {
  int i, j, k, cnt;
  bit32u blocks, lba, ret, sect_block, *s;
  bit32u farF *p;
  bit8u buffer[512*LARGEST_BLOCK_SIZE];
  bit32u indirect[(512*LARGEST_BLOCK_SIZE)>>2];
  
  sect_block = block_size >> 9;

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if extents, do the extent code
  if (inode->flags & EXT4_EXTENTS_FL) {
    struct S_EXT3_EXTENT *extent = (struct S_EXT3_EXTENT *) inode->u.extents.extents;
    for (i=0; i<inode->u.extents.extent_hdr.entries; i++) {
      cnt = extent[i].len * sect_block;
      lba = (/*((bit64u) extent[i].start_hi << 32) +*/ extent[i].start) * sect_block;
      while (cnt) {
        j = (cnt <= LARGEST_BLOCK_SIZE) ? cnt : LARGEST_BLOCK_SIZE;
        if (read_sectors(lba, j, buffer) != j) {
          puts("Error reading from file...");
          return 0;
        }
        
        p = (bit32u farF *) (buf_loc + (extent[i].block * block_size));
        s = (bit32u *) buffer;
        k = (j * (512 / sizeof(bit32u)));
        while (k--)
          *p++ = *s++;
        
        lba += j;
        cnt -= j;
      }
    }
    return inode->size;
  } else {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // direct blocks
    p = (bit32u farF *) buf_loc;
    blocks = (inode->size + (block_size - 1)) / block_size;  // blocks to read
    if ((ret = ext2_direct(inode->u.block_array.block, sect_block, &blocks, 12, p)) == 0) {
      puts("Error reading from file...");
      return 0;
    }
    if (blocks == 0)
      return inode->size;
    p = (bit32u farF *) ret;
    
    // did 12 blocks, now do (double) indirect blocks
    if ((ret = ext2_indirect(inode->u.block_array.indirect_block, sect_block, &blocks, p)) == 0)
      return 0;
    if (blocks == 0)
      return inode->size;
    p = (bit32u farF *) ret;
    
    // read the dbl indirect block
    if (read_sectors(inode->u.block_array.dbl_indirect_block, sect_block, indirect) != sect_block) {
      puts("Error reading indirect blocks...");
      return 0;
    }
    
    // do double indirect reads
    for (i=0; i<(block_size / 4); i++) {
      if ((ret == ext2_indirect(indirect[i], sect_block, &blocks, p)) == 0)
        return 0;
      if (blocks == 0)
        return inode->size;
      p = (bit32u farF *) ret;
    }
    
    // we don't support tripple indirect blocks, but if we did, we would just move the dbl
    //  above to a function, like the single indirect below, then call the dbl, calling the
    //  single, calling the direct.
    // For now, return error
    puts("Tripple Indirects are not supported...");
    return 0;    
  }
}

bit32u ext2_direct(const bit32u *lbas, const bit32u sect_block, bit32u *blocks, const int cnt, bit32u farF *p) {
  bit8u buffer[512*LARGEST_BLOCK_SIZE];
  bit32u *s;
  int i, k;
  
  for (i=0; i<cnt; i++) {
    if (read_sectors(lbas[i], sect_block, buffer) != sect_block) {
      puts("Error reading from file...");
      return 0;
    }
    s = (bit32u *) buffer;
    k = (sect_block * (512 / sizeof(bit32u)));
    while (k--)
      *p++ = *s++;
    
    if (--blocks == 0)
      break;
  }
  
  return (bit32u) p;
}

// do ext2 indirect blocks reads
bit32u ext2_indirect(const bit32u lba, const bit32u sect_block, bit32u *blocks, bit32u farF *p) {
  bit8u buffer[512*LARGEST_BLOCK_SIZE];
  bit32u indirect[(512*LARGEST_BLOCK_SIZE)>>2], *s, ret;
  int i;
  
  // read the indirect block
  if (read_sectors(lba, sect_block, indirect) != sect_block) {
    puts("Error reading indirect blocks...");
    return 0;
  }
  
  if ((ret = ext2_direct(indirect, sect_block, blocks, (sect_block * (512 / 4)), p)) == 0) {
    puts("Error reading indirect blocks...");
    return 0;
  }
  
  return ret;
}

#endif  // _EXT2_C