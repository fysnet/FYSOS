/*
 *                             Copyright (c) 1984-2022
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  LEAN_CHK.EXE
 * This utility will check a given leanfs file image for errors and if
 *    prompted to, will repair these errors.
 *
 *  Assumptions/prerequisites:
 *  - this utility assumes that the bitmap(s) will fit within the buffer
 *    allocated at run time. i.e.: we can load the whole bitmap at once.
 *
 *  - Remember, I didn't write this utility to be complete or robust.
 *    I wrote it to simply check a leanfs image for use with this book.
 *    Please consider this if you add or modify to this utility.
 *
 *  - This code only works with 512-byte blocks.  Any other size should by
 *    easy to adjust to, though here we only use 512-byte blocks.
 *
 *  Last updated: 8 Feb 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os lean_chk.c -o lean_chk.exe -s
 *
 *  Usage:
 *   lean_chk filename.ext /v /r /b:xx
 */

#pragma warning(disable: 4996)  // disable the _s warning for sprintf(), etc.

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "..\include\ctype.h"
#include "..\include\misc.h"

#include "lean_chk.h"

size_t base_lba = 0;
bit64u tot_sects_used = 0;
size_t unmarked = 0;
bit8u *super_buffer = NULL;
bit8u *act_bitmap = NULL;
bit8u *new_bitmap = NULL;
int    dirs = 1, files = 0;
FILE  *fp;
char month_str[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// Check, optimize, and repair if specified.
int main(int argc, char *argv[]) {
  unsigned u, j, byte, bit, count;
  unsigned diags = 0, errors = 0;
  bool super_dirty = FALSE, bitmap_dirty;
  struct S_LEAN_SUPER *s, super;
  int i;
  bit64u k, img_size, super_lba, backup_lba = 0;
  bit32u flags = 0, crc;
  char filename[MAX_PATH+1], temp_str[255];
  bit8u *ptr;
  __time64_t timestamp;
  struct tm timeinfo;
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, &flags, &base_lba);
  
  // try to open the file.
  if ((fp = fopen(filename, "r+b")) == NULL) {
    printf("Error opening file: '%s'\n", filename);
    return -1;
  }
  
  if (base_lba > 0)
    printf("Using a base LBA of %zi\n", base_lba);
  
  // get and store the image size in 512-byte blocks
  FSEEK(fp, 0, SEEK_END);
  img_size = (FTELL(fp) / 512) - base_lba;
  rewind(fp);
  
  /* First check will be to read in the first 33 blocks and see if we find a super block
   *  We will search for the super's magic number, then checksum it.
   *  If we get that far, we will then validate items within the super block
   * (Again, this assumes 512-byte block sizes.  To not assume this, we would need to
   *  re-write this code a bit)
   */
  super_buffer = (bit8u *) malloc(33 * 512);
  if (read_block(super_buffer, 0, 33) != 33) {
    puts(" Error reading the first 33 blocks of the volume...");
    fclose(fp);
    return -1;
  }
  
  for (u=1; u<33; u++) {
    s = (struct S_LEAN_SUPER *) &super_buffer[u * sizeof(struct S_LEAN_SUPER)];
    crc = lean_calc_crc((bit32u *) s, sizeof(struct S_LEAN_SUPER) / sizeof(bit32u));
    if ((s->magic == LEAN_SUPER_MAGIC) && (s->checksum == crc)) {
      // save the super items for later.
      super_lba = u;
      memcpy(&super, s, sizeof(struct S_LEAN_SUPER));
      printf(" Super found at LBA %i with a checksum of 0x%08X.\n", u, crc);
      // the fs_version should be at least 0.6 (0x0006)
      printf(" Found version number %i.%i.\n", super.fs_version >> 8, super.fs_version & 0xFF);
      // at the moment, only version 0.7 is supported
      if (super.fs_version != 0x0007) {
        printf(" *Only version 0.7 is supported.");
        errors++;
      }
      // pre_alloc_count, though can be anything from 0 to 255, should be 1 less than a multiple of 2
      printf(" PreAllocate Count is %i\n", super.pre_alloc_count);
      if ((super.pre_alloc_count + 1) & 0x01) {
        puts("   Count would be more logical as an odd number, calculating to an even number.");
        diags++;
      }
      // Logical blocks per band  (1 << logs = blocks per band) (must be at least 12)
      printf(" Logical blocks per band is %i (%i blocks per band)\n", super.log_blocks_per_band, 1 << super.log_blocks_per_band);
      if (super.log_blocks_per_band < 12) {
        puts(" Logical blocks per band must be at least 12");
        errors++;
      }
      // state must have all bits zero except bit 1 and 0.
      printf(" State is 0x%08X\n", super.state);
      if (super.state & 0xFFFFFFFC) {
        puts(" Unknown bits set in super->state");
        errors++;
      }
      // if the guid field is empty, give it a guid.
      if (is_buf_empty(&super.guid, sizeof(struct S_GUID))) {
        calc_guid(&super.guid);
        printf(" No GUID found, setting to: %08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X\n", super.guid.data1, super.guid.data2, 
          super.guid.data3, super.guid.data4, super.guid.data5[0], super.guid.data5[1], super.guid.data5[2], 
          super.guid.data5[3], super.guid.data5[4], super.guid.data5[5]);
        super_dirty = TRUE;
        errors++;
      }
      // if the label field is empty, give it a label.
      if (is_buf_empty(&super.volume_label, 64)) {
        _time64(&timestamp);
        _localtime64_s(&timeinfo, &timestamp);
        sprintf((char *) super.volume_label, "LEAN Volume repaired on %02i%s%04i %02i:%02i:%02i", timeinfo.tm_mday, get_date_month_str(timeinfo.tm_mon), 
          timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        printf(" No label found, setting to: \"%s\"\n", super.volume_label); 
        super_dirty = TRUE;
        errors++;
      }
      // is block_count > image size
      printf(" Volume block count is %" LL64BIT "i\n", super.block_count);
      if (super.block_count > img_size) {
        printf(" Block count is larger than actual disk size of %" LL64BIT "i (%" LL64BIT "i).\n", super.block_count, img_size);
        super.block_count = img_size;
        super_dirty = TRUE;
        errors++;
      }
      // Free block count
      printf(" Free block count is %" LL64BIT "i\n", super.free_block_count);
      // primary super location
      if (super.primary_super != u) {
        printf(" Super->primary_super != %i\n", u);
        super.primary_super = u;
        super_dirty = TRUE;
        errors++;
      }
      // Bitmap start
      printf(" Bitmap starts at %" LL64BIT "i\n", super.bitmap_start);
      // Root start
      printf(" Super indicates root Inode is at %" LL64BIT "i\n", super.root_start);
      // bad_start
      printf(" Super indicates bad block Inode is at %" LL64BIT "i\n", super.bad_start);
      // journal_start
      printf(" Super indicates Journal Inode is at %" LL64BIT "i\n", super.journal_start);
      
      // block size
      printf(" Super indicates Block Size is %i\n", 1 << super.log_block_size);
      if (super.log_block_size != 9) {
        puts(" Block Size is not 512.  Stopping...");
        fclose(fp);
        return -1;
      }
      
      // reserved0[7]
      if (!is_buf_empty(&super.reserved0, 7)) {
        puts(" Super->reserved0[7] is not zero'd");
        memset(super.reserved0, 0, 7);
        super_dirty = TRUE;
        errors++;
      }
      // reserved1[344]
      if (!is_buf_empty(&super.reserved1, 344)) {
        puts(" Super->reserved1[344] is not zero'd");
        memset(super.reserved1, 0, 344);
        super_dirty = TRUE;
        errors++;
      }
      // backup super location
      puts(" Checking the backup Super Block block...");
      if (read_block(super_buffer, super.backup_super, 1) == 1) {
        s = (struct S_LEAN_SUPER *) super_buffer;
        crc = lean_calc_crc((bit32u *) s, sizeof(struct S_LEAN_SUPER) / sizeof(bit32u));
        if ((s->magic == LEAN_SUPER_MAGIC) && (s->checksum == crc)) {
          printf("  Found backup super at block: %" LL64BIT "i\n", super.backup_super);
          backup_lba = super.backup_super;
          if (flags & REPAIR_FS_REPAIR) {
            printf("  Restore primary from backup? [Yes|No] ");
            scanf("%s", temp_str);
            if (strcmp(temp_str, "Yes") == 0) {
              memcpy(&super, s, sizeof(struct S_LEAN_SUPER));
              super_dirty = TRUE;
            }
            puts("");
          }
        } else
          printf(" Did not find valid backup super at block %" LL64BIT "i\n", super.backup_super);
      } else
        puts(" Error reading the backup super block.");
      // break out of the loop
      break;
    }
  }
  // if u == 33, we didn't find the super, so abort
  if (u == 33) {
    puts(" Did not find a valid super block within blocks 1 -> 32.  Aborting.");
    fclose(fp);
    return -1;
  }
  
  /* Second test is to load all of the bitmaps and count the set/clear bits
   *  If the count of clear bits doesn't match super.free_count, give error
   *
   */
  unsigned band;
  unsigned sects_band = (1 << super.log_blocks_per_band);
  unsigned tot_bands = (unsigned) ((super.block_count + (sects_band - 1)) / sects_band);
  unsigned sects_bitmap = ((sects_band >> 3) + 511) >> 9; // each bands bitmap size in blocks
  unsigned tot_bitmap_size = (tot_bands * (sects_bitmap << 9));  // in bytes
  
  act_bitmap = (bit8u *) malloc(tot_bitmap_size);
  new_bitmap = (bit8u *) calloc(tot_bitmap_size, 1);

  /* We create a clean bitmap and mark it as we test the volume.  Then we compare
   *  the new bitmap with the original bitmap and display inconsistancies.
   */
  // first 33 blocks are reserved for the boot and superblock
  new_bitmap[0] = new_bitmap[1] = new_bitmap[2] = new_bitmap[3] = 0xFF;
  new_bitmap[4] = 1;
  
  // if we found the backup super, we need to set that bit in the new bitmap
  if (backup_lba > 0) {
    byte = (unsigned) (backup_lba / 8);
    bit  = (unsigned) (backup_lba % 8);
    new_bitmap[byte] |= (1 << bit);
  }
  
  // load the bitmaps
  ptr = act_bitmap;
  for (band = 0; (band < tot_bands); band++) {
    bit64u bitmap_block = (sects_band * band);
    if (band == 0) bitmap_block = super.bitmap_start;
    if (((band * sects_band) + sects_bitmap) > super.block_count)
      sects_bitmap = (unsigned) (super.block_count - (band * sects_band));
    
    // need to set these bits in the new bitmap too
    for (j=0; j<sects_bitmap; j++) {
      byte = (unsigned) ((bitmap_block + j) / 8);
      bit  = (unsigned) ((bitmap_block + j) % 8);
      new_bitmap[byte] |= (1 << bit);
    }
    
    // read it in.
    read_block(ptr, bitmap_block, sects_bitmap);
    ptr += (sects_bitmap << 9);
  }
  
  // need to mark the end of the bitmap with 1's.  The non-existant blocks
  for (k=super.block_count; k<(tot_bitmap_size * 8); k++) {
    byte = (unsigned) (k / 8);
    bit  = (unsigned) (k % 8);
    new_bitmap[byte] |= (1 << bit);
  }
  
  // now count the set bits (only doing a total of super.block_count)
  unsigned our_free_count = 0;
  for (u=0; (u < (unsigned) (super.block_count >> 3)); u++) {
    if (act_bitmap[u] == 0x00)
      our_free_count += 8;
    else if (act_bitmap[u] == 0xFF)
      ;
    else {
      for (i=0; i<8; i++)
        if ((act_bitmap[u] & (0x80 >> i)) == 0)
          our_free_count++;
    }
  }
  
  printf(" Total bands found: %i\n", tot_bands);
  printf(" Each bands bitmap size in blocks: %i\n", sects_bitmap);
  printf(" Counted %i free blocks.  Super reported %" LL64BIT "i free blocks\n", our_free_count, super.free_block_count);
  
  /* Next test would be to load the directories, test its inode, then loop through the entries.
   *  If a directory is found, push onto stack and start with it
   * So that we can push to a stack, or recurse directories, we call the function now
   */
  puts(" Checking: \\");
  lean_check_directory(super.root_start, "\\", flags);
  
  // compare the actual bitmap and the new bitmap
  count = lean_compare_bitmaps(tot_bitmap_size);
  diags += count;
  bitmap_dirty = (count > 0);
  printf(" Found %zi unmarked blocks.\n", unmarked);
  
  // if flags indicate to repair, do the following:
  if (flags & REPAIR_FS_REPAIR) {
    // if there was not backup super found, ask to create one.
    if (backup_lba == 0) {
      printf(" No Backup Super Block found, create? [Yes|No] ");
      scanf("%s", temp_str);
      puts("");
      if (strcmp(temp_str, "Yes") == 0) {
        // find a free block
        bit64u fnd_block = find_free_bit(new_bitmap, super.block_count, sects_band - 1, TRUE);
        if (fnd_block > 0) {
          super.backup_super = fnd_block;
          super.free_block_count--;
          super_dirty = TRUE;
          bitmap_dirty = TRUE;
          super.checksum = lean_calc_crc((bit32u *) &super, sizeof(struct S_LEAN_SUPER) / sizeof(bit32u));
          write_block(&super, fnd_block, 1);
        } else
          puts(" Didn't find a free block for backup Super Block.");
      }
    }
    
    //  - if (super_dirty) write super and backup super. (asking first for backup?)
    if (super_dirty) {
      printf(" Writing Super Block to %" LL64BIT "i\n", super_lba);
      super.checksum = lean_calc_crc((bit32u *) &super, sizeof(struct S_LEAN_SUPER) / sizeof(bit32u));
      write_block(&super, super_lba, 1);
      if (backup_lba > 0) {
        printf(" Write Backup to %" LL64BIT "i? [Yes|No] ", backup_lba);
        scanf("%s", temp_str);
        puts("");
        if (strcmp(temp_str, "Yes") == 0)
          write_block(&super, backup_lba, 1);
      }
    }
    
    //  - if (bitmap_dirty) write bitmap(s) (new_bitmap)
    if (bitmap_dirty) {
      ptr = new_bitmap;
      for (band = 0; (band < tot_bands); band++) {
        bit64u bitmap_block = (sects_band * band);
        if (band == 0) bitmap_block = super.bitmap_start;
        if (((band * sects_band) + sects_bitmap) > super.block_count)
          sects_bitmap = (unsigned) (super.block_count - (band * sects_band));
        
        // write it.
        write_block(ptr, bitmap_block, sects_bitmap);
        ptr += (sects_bitmap << 9);
      }
    }
  }
  
  free(new_bitmap);
  free(act_bitmap);
  free(super_buffer);
  
  fclose(fp);
  
  printf("\n Extent Blocks used: %" LL64BIT "i\n", tot_sects_used);
  printf("     Total Directors: %i\n", dirs);
  printf("         Total Files: %i\n", files);
  
  puts("");
  
  printf("        Errors Found: %i\n", errors);
  printf("   Diagnostics Found: %i\n", diags);
  
  puts("");
  return 0;
}

/* Check directory (recursable)
 *     dpb -> disk parameter block
 *  bitmap -> filled bitmap for entire volume
 *  start_cluster = inode number of this directory
 *
 *  This function checks the inode (via a call to lean_check_inode), then checks
 *   each directory entry and its inode (again via a call to lean_check_inode).
 *  If the entry is a directory, it recuses.
 */
bool lean_check_directory(const bit64u start_cluster, const char *parent, const bit32u flags) {
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) malloc(512);
  unsigned ul;
  
  if (read_block(inode, start_cluster, 1) != 1)
    return FALSE;
  
  // Check the inode
  if (!lean_check_inode(start_cluster, inode, flags))
    return TRUE;  // return TRUE so we continue on
  
  // re-allocate the memory block to allow the size of the directory and read it in
  inode = (struct S_LEAN_INODE *) realloc(inode, (bit32u) (inode->block_count * 512));
  bit8u *ptr = (bit8u *) inode + LEAN_DATA_OFFSET;
  for (ul=1; ul<inode->block_count; ul++) {
    bit64u block_num = lean_get_block_num(inode, ul);
    read_block(ptr, block_num, 1);
    ptr += 512;
  }
  
  // parse through the entries.
  unsigned i;
  unsigned cur_offset = LEAN_DATA_OFFSET;
  if (!(inode->attributes & LEAN_ATTR_INLINEXTATTR))
    cur_offset = S_LEAN_INODE_SIZE;
  unsigned limit = (unsigned) (inode->file_size + LEAN_DATA_OFFSET);
  bit8u *name;
  char ill_char, buffer[512];
  char name_z[LEAN_NAME_LEN_MAX+1];
  
  // now walk the entries and check to make sure they are good
  while (cur_offset < limit) {
    struct S_LEAN_DIRENTRY *entry = (struct S_LEAN_DIRENTRY *) ((bit8u *) inode + cur_offset);
    switch (entry->type) {
      case LEAN_FT_MT:
        if (entry->name_len == 0)
          break;
      case LEAN_FT_REG:
      case LEAN_FT_DIR:
        // check each char in name to make sure valid chars
        name = (bit8u *) ((bit8u *) entry + LEAN_DIRENTRY_NAME);
        ill_char = 0;
        for (i=0; i<entry->name_len; i++) {
          if (!is_utf8(name[i])) {
            ill_char = name[i];
            break;
          }
          name_z[i] = name[i];
        }
        name_z[i] = 0;
        printf(" Checking: %s%s\n", parent, name_z);
        if (ill_char)
          printf("*Illegal char (%i) in name at position %i\n", ill_char, i);
        
        // don't check or recurse on the . and .. entries
        if (!strcmp(name_z, ".") || !strcmp(name_z, "..")) {
          /* TODO:  This could check to see if the '.' and '..' entries actually
           *        points to the correct respective Inodes.
           */
          break;
        }
        
        // if it is a directory, recurse
        if (entry->type == LEAN_FT_DIR) {
          strcpy(buffer, parent);
          strcat(buffer, name_z);
          strcat(buffer, "\\");
          lean_check_directory(entry->inode, buffer, flags);
          dirs++;
        } else if (entry->type == LEAN_FT_REG) {
          // read in the inode
          if (read_block(buffer, entry->inode, 1) != 1) {
            printf("*Did not read in Inode at %" LL64BIT "i\n", entry->inode);
            return FALSE;
          }
          lean_check_inode(entry->inode, (struct S_LEAN_INODE *) buffer, flags);
          files++;
        } else if (entry->type == LEAN_FT_MT) {
          printf(" [Deleted: %" LL64BIT "i]", entry->inode);
        }
        break;
        
      case LEAN_FT_LNK:
        // currently nothing
        break;
        
      case LEAN_FT_FRK:
        // currently nothing
        break;
        
      default:
        // found unknown or illegal type, exit out of this directory
        printf("*Found unknown or illegal type in directory entry.  Type = 0x%02X\n", entry->type);
    }
    
    // if the current rec len is zero, this loop will never exit
    if (entry->rec_len > 0)
      cur_offset += (entry->rec_len << 4);
    else
      break;
  }
  
  free(inode);
  
  return TRUE;
}
/* Check Inode
 *  This function checks the validity of the inode, then checks to see if the corresponding
 *   bits in the bitmap are set for the extents in the inode.
 */
bool lean_check_inode(const bit64u inode_num, struct S_LEAN_INODE *inode, const bit32u flags) {
  
  struct S_LEAN_INDIRECT indirect;
  bit64u ul, block_num, tot_blocks = 0;
  int i, j;
  bit32u crc = lean_calc_crc((bit32u *) inode, S_LEAN_INODE_SIZE / 4);
  __time64_t timestamp;
  struct tm *timeinfo;
  
  printf("Inode Number: %" LL64BIT "i\n", inode_num);
  
  // if the inode's MAGIC doesn't match or the CRC isn't correct, not a valid INODE.
  if ((inode->magic == LEAN_INODE_MAGIC) && (inode->checksum == crc)) {
    
    // Check the inodes attributes
    if (!lean_check_inode_attrib(inode->attributes, flags))
      return FALSE;
    
    // check each extent in this inode
    for (j = 0; j<inode->extent_count; j++) {
      tot_blocks += inode->extent_size[j];
      if ((inode->extent_start[j] > 0) && (inode->extent_size == 0))
        printf("*[%" LL64BIT "i]: Inode Extent has zero count for extent size in extent #%i\n", inode_num, j);
    }
    
    // check the count of indirect extents and see if they point to correct indirects.
    if (inode->indirect_count == 0) {
      // if indirect_count was zero, first and last indirect fiels should also be zero
      if (inode->first_indirect || inode->last_indirect) {
        printf("*[%" LL64BIT "i]: Inode has a count of zero for indirect count, but the first and/or last\n"
               "  indirect address field is non zero.\n", inode_num);
      }
    } else {
      // if indirect count > 0, walk through the indirect blocks and check them for validity
      i = 0;
      block_num = 0;
      ul = inode->first_indirect;
      while (ul > 0) {
        block_num = ul;
        read_block(&indirect, ul, 1);
        if (!lean_valid_indirect(&indirect)) {
          printf("*[%" LL64BIT "i]: Bad indirect block at LBA %" LL64BIT "i (%i of %i)\n", 
            inode_num, ul, i + 1, inode->indirect_count);
          break;
        }
        
        // check each extent in this indirect block
        for (j = 0; j<indirect.extent_count; j++) {
          tot_blocks += indirect.extent_size[j];
          if ((indirect.extent_start[j] > 0) && (indirect.extent_size == 0))
            printf("*[%" LL64BIT "i]: Indirect Extent has zero count for extent size in extent #%i\n", inode_num, j);
        }
        
        // update and continue
        ul = indirect.next_indirect;
        i++;
      }
      
      // print the results of the indirect check
      printf(" Indirect Count = %i, counted indirect blocks %i\n", inode->indirect_count, i);
      if (block_num != inode->last_indirect)
        printf("*[%" LL64BIT "i]: Last indirect block was not block specified in last_indirect\n", inode_num);
    }
    
    // if the reserved field in the inode is not zero, give error
    if (inode->reserved[0] || inode->reserved[1] || inode->reserved[2])
      printf("*[%" LL64BIT "i]: Inode->reserved non zero... (%02X  %02X  %02X)\n", inode_num, inode->reserved[0], inode->reserved[1], inode->reserved[2]);
    
    // if the links_count is < 1, give error
    if (inode->links_count < 1)
      printf("*[%" LL64BIT "i]: Inode->links_count is zero...\n", inode_num);
    
    // check the block_count field.  It should be the same as all extents checked above.
    if (inode->block_count != tot_blocks)
      printf("*[%" LL64BIT "i]: inode->block_count != counted blocks used... (%" LL64BIT "i, %" LL64BIT "i)\n", 
        inode_num, inode->block_count, tot_blocks);
    if (flags & REPAIR_FS_VERBOSE)
      printf(" Total blocks used by this Inode: %" LL64BIT "i (bytes: %" LL64BIT "i)\n", tot_blocks, inode->file_size);
    tot_sects_used += tot_blocks;
    
    // currently the uid and gid should be zeros
    if (inode->uid || inode->gid)
      printf("*[%" LL64BIT "i]: Inode->uid or Inode->gid non zero... (%08X %08X)\n",
        inode_num, inode->uid, inode->gid);
    
    // if any of the access times is before the creation time, give error
    if ((inode->cre_time > inode->acc_time) || (inode->cre_time > inode->sch_time) || (inode->cre_time > inode->mod_time)) {
      printf("*[%" LL64BIT "i]: creation time for inode is after other access time.\n", inode_num);
      if (flags & REPAIR_FS_VERBOSE) {
        timestamp = (time_t) (inode->cre_time / 1000);
        timeinfo = gmtime(&timestamp);
        printf("       Creation: %04i/%02i/%02i %02i:%02i:%02i (yyyy/mm/dd hh:mm:ss)\n", timeinfo->tm_year, timeinfo->tm_mon,
          timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        timestamp = (time_t) (inode->acc_time / 1000);
        timeinfo = gmtime(&timestamp);
        printf("  Last Accessed: %04i/%02i/%02i %02i:%02i:%02i\n", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        timestamp = (time_t) (inode->sch_time / 1000);
        timeinfo = gmtime(&timestamp);
        printf(" Status Changed: %04i/%02i/%02i %02i:%02i:%02i\n", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        timestamp = (time_t) (inode->mod_time / 1000);
        timeinfo = gmtime(&timestamp);
        printf("  Last Modified: %04i/%02i/%02i %02i:%02i:%02i\n", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      }
    }
    
    /* TODO:
     *  Check to see if the fork field points to a valid Inode.
     *  We may or may not find the fork again, so this is when
     *   we should recurse (?) and check this forked inode.
     */

    // Check to see that the bitmap bits are set for each block in this file's extents
    unsigned clear_bits = 0;
    for (ul=0; ul<inode->block_count; ul++) {
      block_num = lean_get_block_num(inode, ul);
      unsigned byte = (unsigned) (block_num / 8);
      unsigned bit  = (unsigned) (block_num % 8);
      if ((act_bitmap[byte] & (1 << bit)) == 0)
        { clear_bits++; unmarked++; }
      new_bitmap[byte] |= (1 << bit);  // set the bit in the new_bitmap so we can compare
    }
    if (clear_bits > 0)
      printf("*[%" LL64BIT "i]: Inode uses %i unallocated blocks.\n", inode_num, clear_bits);
    
    return TRUE;
  } else {
    printf("*[%" LL64BIT "i]: Inode doesn't pass MAGIC and/or CRC test\n", inode_num);
    return FALSE;
  }
}

/* This checks the attribute sent for validity
 *
 *
 */
bool lean_check_inode_attrib(const bit32u attrib, const bit32u flags) {
  
  bool ret = TRUE; // assume good return
  int i;
  
  if (flags & REPAIR_FS_VERBOSE) {
    puts(" Permissions:");
    printf("       Other: ");
    i = 0;
    if (attrib & LEAN_ATTR_IXOTH) { puts("Execute "); i++; }
    if (attrib & LEAN_ATTR_IWOTH) { puts("Write "); i++; }
    if (attrib & LEAN_ATTR_IROTH) { puts("Read "); i++; }
    if (!i)                         puts("None");
    i = 0;
    printf("  Group: ");
    if (attrib & LEAN_ATTR_IXGRP) { puts("Execute "); i++; }
    if (attrib & LEAN_ATTR_IWGRP) { puts("Write "); i++; }
    if (attrib & LEAN_ATTR_IRGRP) { puts("Read "); i++; }
    if (!i)                         puts("None");
    i = 0;
    printf("  User: ");
    if (attrib & LEAN_ATTR_IXUSR) { puts("Execute "); i++; }
    if (attrib & LEAN_ATTR_IWUSR) { puts("Write "); i++; }
    if (attrib & LEAN_ATTR_IRUSR) { puts("Read "); i++; }
    if (!i)                         puts("None");
    
    if (attrib & LEAN_ATTR_ISUID) puts("Execute as User ID");
    if (attrib & LEAN_ATTR_ISGID) puts("Execute as Group ID");
  }
  
  // if bit 9 is set, give error
  if (attrib & (1 << 9)) {
    puts("*Bit 9 is set...");
    ret = FALSE;
  }
  
  // if both ISUID and ISGID are set, give error
  if ((attrib & LEAN_ATTR_ISUID) && (attrib & LEAN_ATTR_ISGID)) {
    puts("Both Execute as USR/GRP ID's are set.");
    ret = FALSE;
  }
  
  // print the standard attrib's
  if (flags & REPAIR_FS_VERBOSE) {
    printf(" Attributes: ");
    if (attrib & LEAN_ATTR_HIDDEN)  puts("Hidden ");
    if (attrib & LEAN_ATTR_SYSTEM)  puts("System ");
    if (attrib & LEAN_ATTR_ARCHIVE) puts("Archive ");
    if (attrib & LEAN_ATTR_SYNC_FL) puts("Synchonoys Updates ");
    if (attrib & LEAN_ATTR_NOATIME_FL) puts("No Last Time Update ");
    if (attrib & LEAN_ATTR_IMMUTABLE_FL) puts("Unmovable ");
    if (attrib & LEAN_ATTR_PREALLOC) puts("Keep Preloc's ");
  
    if (attrib & LEAN_ATTR_INLINEXTATTR) puts("\n EA's are after inode.");
    else                                 puts("\n File starts after inode data.");
  }  
  
  // if any of bits 28:20 are set, give error
  if (attrib & ((1<<28) | (1<<27) | (1<<26) | (1<<25) | (1<<24) | (1<<23) | (1<<22) | (1<<21) | (1<<20))) {
    puts("*One or more of Bits 28:20 are set.");
    ret = FALSE;
  }
  
  // print file type
  if (flags & REPAIR_FS_VERBOSE) {
    if (attrib & LEAN_ATTR_IFREG) puts("\n File Type: Regular File.");
    if (attrib & LEAN_ATTR_IFDIR) puts("\n File Type: Directory.");
    if (attrib & LEAN_ATTR_IFLNK) puts("\n File Type: Symbolic Link.");
    if (attrib & LEAN_ATTR_IFFRK) puts("\n File Type: Fork.");
  }
  
  // if more than one File Type is set, give error
  if (((attrib & LEAN_ATTR_IFREG) && (attrib & (LEAN_ATTR_IFMT & ~LEAN_ATTR_IFREG))) ||
      ((attrib & LEAN_ATTR_IFDIR) && (attrib & (LEAN_ATTR_IFMT & ~LEAN_ATTR_IFDIR))) ||
      ((attrib & LEAN_ATTR_IFLNK) && (attrib & (LEAN_ATTR_IFMT & ~LEAN_ATTR_IFLNK))) ||
      ((attrib & LEAN_ATTR_IFFRK) && (attrib & (LEAN_ATTR_IFMT & ~LEAN_ATTR_IFFRK)))) {
    printf("*Illegal File Type given: 0x%02X.\n", (attrib & LEAN_ATTR_IFMT) >> 29);
    ret = FALSE;
  }
  
  return ret;
}

/* This function checks certain parts of the inode's indirect struct to see if
 *  it is a valid indirect struct.
 */
bool lean_valid_indirect(const struct S_LEAN_INDIRECT *indirect) {
  
  // check to see if indirect->magic = 'INDX';
  if (indirect->magic != LEAN_INDIRECT_MAGIC)
    return FALSE;
  
  // check to see if crc is correct
  if (indirect->checksum != lean_calc_crc((bit32u *) indirect, sizeof(struct S_LEAN_INDIRECT) / sizeof(bit32u)))
    return FALSE;
  
  // we can have no more than LEAN_INDIRECT_EXTENT_CNT extents
  if (indirect->extent_count > LEAN_INDIRECT_EXTENT_CNT)
    return FALSE;
  
  return TRUE;
}

/* This compares the actual bitmap with the new bitmap
 *  The new bitmap will only have the actual used bits set
 *  This flags any bits that were set in the actual that are
 *   not set in the new. i.e.: finds orphands bits.
 */
unsigned lean_compare_bitmaps(const unsigned tot_bitmap_size) {
  
  unsigned u, count = 0;
  int i;
  
  for (u=0; u<tot_bitmap_size; u++) {
    if (act_bitmap[u] != new_bitmap[u]) {
      for (i=0; i<8; i++) {
        if ((act_bitmap[u] & (1<<i)) && !(new_bitmap[u] & (1<<i))) {
          printf("*Found orphand bitmap entry at block: %i\n", (u * 8) + i);
          count++;
        }
      }
    }
  }
  
  printf(" Found %i orphand blocks.\n", count);
  
  // return count of different bits
  return count;
}

/* This returns the block number of the starting block that contains the first byte of 
 *  the requested position
 *  *inode -> the inode of the file
 *  index   = the linear block offset  ie: f_pos / 512
 */
bit64u lean_get_block_num(struct S_LEAN_INODE *inode, const bit64u index) {
  
  int i;
  bit64u cur_index = 0;
  bit32s count = inode->extent_count;
  for (i=0; (i < LEAN_INODE_EXTENT_CNT) && count; i++) {
    if ((cur_index + inode->extent_size[i]) > index)
      return (inode->extent_start[i] + (index - cur_index));
    cur_index += inode->extent_size[i];
    count--;
  }
  
  if (count > 0) {
    bit64u next_indirect = inode->first_indirect;
    struct S_LEAN_INDIRECT indirect;
    while (count) {
      read_block(&indirect, next_indirect, 1);
      if (!lean_valid_indirect(&indirect))
        return 0;
      if ((cur_index + indirect.block_count) < index) {
        cur_index += indirect.block_count;
        count -= indirect.extent_count;
      } else {
        for (i=0; (i < LEAN_INDIRECT_EXTENT_CNT) && count; i++) {
          if ((cur_index + indirect.extent_size[i]) > index)
            return (indirect.extent_start[i] + (index - cur_index));
          cur_index += indirect.extent_size[i];
          count--;
        }
        return 0;
      }
      next_indirect = indirect.next_indirect;
    }
  }
  
  return 0;
}

bit64u find_free_bit(bit8u *bitmap, const bit64u total_sects, bit64u block, const bool mark_it) {
  unsigned byte, bit;
  
  for (block; block < (total_sects >> 3); block++) {
    byte = (unsigned) (block / 8);
    bit = (unsigned) (block % 8);
    if ((bitmap[byte] & (1 << bit)) == 0) {
      if (mark_it)
        bitmap[byte] |= (1 << bit);
      return block;
    }
  }
  
  return 0;
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /v         - Tell this app to print verbose output strings
 *  /r         - Tell this app to repair the image
 */
void parse_command(int argc, char *argv[], char *filename, bit32u *flags, size_t *base_lba) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if ((strcmp(s, "V") == 0) ||
          (strcmp(s, "v") == 0))
        *flags |= REPAIR_FS_VERBOSE;
      else if ((strcmp(s, "R") == 0) ||
               (strcmp(s, "r") == 0))
        *flags |= REPAIR_FS_REPAIR;
      else if ((memcmp(s, "B:", 2) == 0) ||
               (memcmp(s, "b:", 2) == 0))
        *base_lba = strtoul(s + 2, NULL, 0);
      else
        printf(" Unknown switch parameter: /%s\n", s);
    } else
      strcpy(filename, s);
  }
}

size_t read_block(void *buffer, const bit64u lba, const unsigned count) {
  FSEEK(fp, (base_lba + lba) * 512, SEEK_SET);
  return fread(buffer, 512, count, fp);
}

size_t write_block(void *buffer, const bit64u lba, const unsigned count) {
  FSEEK(fp, (base_lba + lba) * 512, SEEK_SET);
  return fwrite(buffer, 512, count, fp);
}

// returns TRUE if all bytes from ptr[0] to ptr[len - 1] are zero
bool is_buf_empty(const void *ptr, unsigned len) {
  bit8u *p = (bit8u *) ptr;
  
  while (len--)
    if (*p++)
      return FALSE;
  
  return TRUE;
}

const char *get_date_month_str(const int month) {
  if (month < 13)
    return month_str[month-1];
  else
    return "***";
}

/* Check to see if the character is a UTF8 compatible char
 *  
 * TODO: This actually needs to decode the UTF8 char instead
 *       of this quick assumption that it is ascii.
 *       See the Appendix within the book on Unicode Chars.
 *
 */
bool is_utf8(bit8u code) {
  return ((code > 31) || (code == 0)) ? TRUE : FALSE;
}

/* This always skips the first dword since it is the crc field.
 *  The CRC is calculated as:
 *     crc = 0;
 *     loop (n times)
 *       crc = ror(crc) + dword[x]
 */
bit32u lean_calc_crc(const bit32u *ptr, const unsigned dwords) {
  
  bit32u crc = 0;
  unsigned i;
  
  for (i=1; i<dwords; i++)
    crc = (crc << 31) + (crc >> 1) + ptr[i];
  
  return crc;
}
