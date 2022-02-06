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
 *  MLEANFS.EXE
 *   See the included files.txt file for an example of the resource file
 *
 *   This utility will take a list of filenames and include those
 *    files in the image, creating root entries.
 *
 *   This utility will allow sub-directories to be used.  For example,
 *    if the filename is given as
 *      first/second/filename.txt
 *    this utility will create a sub-directory called 'first' in the root
 *    directory, then create a sub-directory called 'second' in the 'first'
 *    directory, and then create the 'filename.txt' entry.
 *
 *   If you include another filename using the same directory structure, such as
 *     first/filename.txt
 *    this utility won't need to create a new directory, because it already exists.
 *
 *  Assumptions/prerequisites:
 *   - requires 64-bit integers
 *
 *  Last updated: 5 Feb 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os mleanfs.c -o mleanfs.exe -s
 *
 *  Usage:
 *    mleanfs filename.ext /e /v
 */

#pragma warning(disable: 4996)  // disable the _s warning for sprintf(), etc.

#include <ctype.h>
#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/ctype.h"
#include "../include/misc.h"

#include "mleanfs.h"

struct S_FOLDERS *folders = NULL;
size_t cur_folder = 0;
size_t cur_folder_size = 0;

size_t cur_block = 0;
uint8_t *bitmaps = NULL;
size_t blocks_used = 0;
size_t band_size, bitmap_size, tot_bands;
size_t tot_blocks, block_size, cylinders;

int main(int argc, char *argv[]) {
  size_t super_loc = LEAN_SUPER_LOCATION;
  size_t boot_size, size;
  unsigned int u;
  
  struct S_RESOURCE *resources;
  bool existing_image = FALSE;
  char filename[MAX_PATH + 1];
  FILE *src, *targ;
  char label[256] = "A Label for a Lean FS volume.";
  
  time_t timestamp;
  srand((unsigned) time(&timestamp)); // seed the randomizer

  // print start string
  printf(strtstr);

  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, &existing_image, label);

  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources) {
    printf(" Error with Resource file. '%s'\n", filename);
    if (resources) free(resources);
    return -1;
  }
  
  // done getting parameters, start building the image
  block_size = ((uint64_t) 1 << resources->param1);
  
  // this gives a rough cylinder usage (TODO: We need to use resources->heads and resources->spt here?)
  cylinders = (size_t) (((resources->tot_sectors * block_size) / SECT_SIZE) / (16 * 63));    // roughly the cylinders used

  // total blocks in partition
  tot_blocks = (size_t) resources->tot_sectors;
  
  // make sure log_blocks_size is within boundaries
  // (we only support up to 64k blocks here)
  if ((resources->param1 < 8) || (resources->param1 > 16)) {
    printf(" Log Block Size (2^%i) must be within boundaries.  Default to 12 and 512-byte blocks? [Y|N] ", resources->param1);
    if (toupper(getchecho()) == 'Y')
      resources->param1 = 9;
    else
      return -1;
    puts("");
  }

  // make sure log_band_size is within boundaries
  if ((resources->param0 < 12) || (resources->param0 > 31)) {
    printf(" Log Band Size (2^%i) must be within boundaries.  Default to 12 and 512-byte blocks? [Y|N] ", resources->param0);
    if (toupper(getchecho()) == 'Y') {
      resources->param0 = 12;
      resources->param1 = 9;
    } else
      return -1;
    puts("");
  }

  // make sure log_band_size is within log block size values
  if ((resources->param0 - 3) < resources->param1) {
    printf(" A block size of %i (2^%i) must have a band size of at least %i (2^%i). Okay? [Y|N] ", 
      1 << resources->param1, resources->param1, 1 << (resources->param1 + 3), resources->param1 + 3);
    if (toupper(getchecho()) == 'Y')
      resources->param0 = (resources->param1 + 3);
    else
      return -1;
    puts("");
  }
  
  if (!existing_image) {
    // create target file
    if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
      printf(" Error creating target file: '%s'\n", resources->targ_filename);
      return -1;
    }
  }
  else {
    // open existing target file
    if ((targ = fopen(resources->targ_filename, "r+b")) == NULL) {
      printf(" Error opening target file: '%s'\n", resources->targ_filename);
      return -1;
    }
  }

  // allocate our temp buffer
  // must be at least 33 blocks in length
  uint8_t *buffer = (uint8_t *) calloc(33 * block_size, 1);

  // if we are not working on an existing image file, 
  if (!existing_image) {
    // *** At this point, we read/write SECT_SIZE-byte sectors ***
    // create the MBR and padding sectors
    u = 0;
    if (strlen(resources->mbr_filename)) {
      if ((src = fopen(resources->mbr_filename, "rb")) == NULL) {
        printf(" Error opening mbr.bin file.\n");
        fclose(targ);
        free(buffer);
        return -2;
      }
      fread(buffer, SECT_SIZE, 1, src);
      
      // create and write the Disk Indentifier
      // We call rand() multiple times.
      * (uint32_t *) &buffer[0x01B8] = (uint32_t) ((rand() << 20) | (rand() << 10) | (rand() << 0));
      * (uint16_t *) &buffer[0x01BC] = 0x0000;
    } 
    if (resources->base_lba > 0) {
      // create a single partition entry pointing to our partition
      struct PART_TBLE *pt = (struct PART_TBLE *) &buffer[0x01BE];
      pt->bi = 0x80;
      lba_to_chs(&pt->start_chs, (size_t) resources->base_lba);
      pt->si = 0xEA;  //lEAn
      lba_to_chs(&pt->end_chs, (size_t) ((cylinders * resources->heads * resources->spt) - 1 - resources->base_lba));  // last sector - 1 for the MBR
      pt->startlba = (uint32_t) resources->base_lba;
      //pt->size = (uint32_t) ((cylinders * resources->heads * resources->spt) - resources->base_lba);
      pt->size = (uint32_t) tot_blocks;
      buffer[510] = 0x55; buffer[511] = 0xAA;
      printf(" Writing MBR to LBA %" LL64BIT "i\n", FTELL(targ) / SECT_SIZE);
      fwrite(buffer, SECT_SIZE, 1, targ);
      u = 1;
    }
    memset(buffer, 0, SECT_SIZE);

    if (u < resources->base_lba)
      printf(" Writing Padding between MBR and Base LBA...\n");
    for (; u < resources->base_lba; u++)
      fwrite(buffer, SECT_SIZE, 1, targ);
    //resources->tot_sectors -= resources->base_lba;
    //tot_blocks -= (size_t) resources->base_lba;
    
    // *** At this point, we read/write block_size blocks ***
    // create the boot code sectors (up to 32)
    if (strlen(resources->boot_filename)) {
      if ((src = fopen(resources->boot_filename, "rb")) == NULL) {
        printf(" Error opening boot file.\n");
        fclose(targ);
        free(buffer);
        return -3;
      }

      // clearing the buffer first makes sure that if we don't read 33 sectors
      //  of boot file (leanfs.bin), the last part will be zeros.
      memset(buffer, 0, block_size * 32);
      boot_size = fread(buffer, block_size, 32, src);  // boot_size = boot code in blocks
      fclose(src);
    } else {
      boot_size = 1;  // must reserve at least 1 sector for boot
      memset(buffer, 0, block_size * 32); // but can be up to 32 with default LEAN_SUPER_LOCATION = 32
    }

    // update the sig and base lba within the given boot code
    //  this assumes that the coder of the boot code knew that this area was
    //  reserved for this data...
    // this reserved area is the last few bytes of the first sector of the partition/disk
    //  sig       dword  (unique id for partition/disk)
    //  base_lba  qword  (0 for floppy, could be 63 for partitions)
    //  boot sig  word   (0xAA55)
    * (uint32_t *) &buffer[498] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
    //if (* (uint64_t *) &buffer[502] == 0)  // we only update the base if the .bin file had it as zeros already.
    * (uint64_t *) &buffer[502] = FTELL(targ) / SECT_SIZE;

    // calculate the location of the super block
    if (boot_size > (super_loc - 1)) {
      super_loc = boot_size;
      if (super_loc > 32) {
        printf(" Boot code exceeds allowed boot sector area.\n"
          "  Boot = %zi sectors (LBA 0 to LBA %zi), while Super is at LBA %zi\n", boot_size, boot_size - 1, super_loc);
        fclose(targ);
        free(buffer);
        return -1;
      }
      if (super_loc != LEAN_SUPER_LOCATION)
        printf(" Moving superblock to LBA %zi (boot code = %zi sectors)\n", super_loc, boot_size);
    }

    // write the first (up to 32) sectors
    buffer[510] = 0x55; buffer[511] = 0xAA;
    printf(" Writing Boot Sector(s) to LBA %" LL64BIT "i\n", (uint64_t) (FTELL(targ) / SECT_SIZE));
    fwrite(buffer, SECT_SIZE, super_loc, targ);
  } // !existing_image

  // we could let the code figure out the band size
  //const uint8_t log_band_size = lean_calc_log_band_size(block_size, tot_blocks);
  // or we get it from the resource file
  const uint8_t log_band_size = resources->param0;
  
  uint32_t band_size = (1 << log_band_size);
  bitmap_size = (band_size / block_size / 8);
  tot_bands = (unsigned) (tot_blocks + (band_size - 1)) / band_size;

  // now create a super block
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) buffer;
  memset(super, 0, block_size);
  super->magic = LEAN_SUPER_MAGIC;
  super->fs_version = 0x0007;  // 0.7
  super->log_blocks_per_band = log_band_size;
  super->pre_alloc_count = (8 - 1);
  super->state = (0 << 1) | (1 << 0);  // clean unmount
  calc_guid(&super->guid);
  strncpy((char *) super->volume_label, label, 63);
  super->volume_label[63] = 0;  // make sure label is null terminated
  super->block_count = tot_blocks;
  super->free_block_count = 0; // we update this later
  super->primary_super = super_loc;
  super->backup_super = ((tot_blocks < band_size) ? tot_blocks : band_size) - 1; // last block in first band
  super->bitmap_start = super_loc + 1;
  super->root_start = super->bitmap_start + bitmap_size;
  super->bad_start = 0;  // no bad sectors (yet?)
  super->journal_inode = 0;
  super->log_block_size = resources->param1;

  // create a buffer for the bitmap(s), and mark the first few bits as used.
  bitmaps = (uint8_t *) calloc(tot_bands * (bitmap_size * block_size), 1);
  bitmap_mark(0, 0, (int) super->root_start, TRUE);
  bitmap_mark(0, (int) band_size - 1, 1, TRUE);  // backup super
  for (u = 1; u < tot_bands; ++u)
    bitmap_mark(u, 0, (int) bitmap_size, TRUE);

  // allocate the folders data
  folders = (struct S_FOLDERS *) calloc(DEF_FOLDER_CNT * sizeof(struct S_FOLDERS), 1);
  cur_folder_size = DEF_FOLDER_CNT;

  // mark where to start looking for free blocks
  cur_block = (size_t) super->root_start;

  // create the root
  strcpy(folders[0].name, "/");
  folders[0].root = calloc(FOLDER_SIZE * block_size, 1);
  root_start((struct S_LEAN_DIRENTRY *) folders[0].root, super->root_start, super->root_start);
  folders[0].cur_rec = 2;
  folders[0].links_count = 2;   // self ('.' link & '..' link)
  folders[0].buf_size = FOLDER_SIZE;
  folders[0].ext_cnt = get_next_extent(FOLDER_SIZE + 1, &folders[0].extent);
  folders[0].parent = NULL;
  ++cur_folder;

  /*  Since we have an empty disk and root, we know that we can start with the
   *  first block and write files consecutively.  There is no need to find
   *  free blocks when we create new entries.  Therefore, we will start with
   *  the first block after the root and the next entry within the root.
   *  We will also keep track of how many blocks we use so that we can
   *  update the bitmap.
   */
  size_t read;
  uint64_t file_size;
  struct S_LEAN_EXTENT extent;
  size_t ext_cnt;
  for (u = 0; u < resources->file_cnt; u++) {
    // get the file to write to the image
    if ((src = fopen(resources->files[u].path_filename, "rb")) == NULL) {
      printf(" Error opening %s file.\n", resources->files[u].path_filename);
      continue;
    }
    FSEEK(src, 0, SEEK_END);
    file_size = FTELL(src);
    rewind(src);

    // create the root's entry
#ifdef INODE_HAS_EAS
    size = (file_size + (block_size - 1)) / block_size;
#else
    size = (size_t) (file_size - (block_size - S_LEAN_INODE_SIZE) + (block_size - 1)) / block_size;
#endif
    ext_cnt = get_next_extent(size + 1, &extent);
    create_root_entry(0, resources->files[u].filename, 0, extent.start[0], super->root_start);

    // create the inode
    printf(" % 2i: Writing %s to block %" LL64BIT "i\n", u, resources->files[u].filename, resources->base_lba + extent.start[0]);
    FSEEK(targ, (resources->base_lba + extent.start[0]) * block_size, SEEK_SET);
    create_inode(targ, file_size, file_size, LEAN_ATTR_ARCHIVE | LEAN_ATTR_IFREG, 1, ext_cnt, &extent);

#ifndef INODE_HAS_EAS
    memset(buffer + block_size, 0, (block_size - S_LEAN_INODE_SIZE));
    read = fread(buffer + block_size, 1, (block_size - S_LEAN_INODE_SIZE), src);
    fwrite(buffer + block_size, (block_size - S_LEAN_INODE_SIZE), 1, targ);
#endif

    unsigned int uu = 0;
    ++extent.start[0];  // skip over inode
    --extent.size[0];   // skip over inode
    do {
      // by clearing the buffer first, we make sure that the "padding" bytes are all zeros
      //  (buffer is used by the super, we need to move to next block)
      memset(buffer + block_size, 0, block_size);
      read = fread(buffer + block_size, 1, block_size, src);
      if (read == 0)
        break;
      FSEEK(targ, (resources->base_lba + extent.start[uu]) * block_size, SEEK_SET);
      fwrite(buffer + block_size, block_size, 1, targ);
      ++extent.start[uu];
      if (--extent.size[uu] == 0)
        ++uu;
    } while (read == block_size);
    fclose(src);
  }

  // write the folders
  for (u = 0; u < cur_folder; u++) {
    printf(" Writing folder '%s' to block %" LL64BIT "i\n", folders[u].name, resources->base_lba + folders[u].extent.start[0]);
    // if we added to the folder size, we need to allocate some more blocks
    // this works since we have an empty canvas, it won't have more than two extents allocated,
    //  and we only had at most two already allocated, totaling less than the limit we allow.
    if (folders[u].buf_size > FOLDER_SIZE) {
      ext_cnt = get_next_extent(folders[u].buf_size - FOLDER_SIZE, &extent);
      for (size_t uu = 0; uu < ext_cnt; ++uu) {
        folders[u].extent.start[folders[u].ext_cnt] = extent.start[uu];
        folders[u].extent.size[folders[u].ext_cnt] = extent.size[uu];
        ++folders[u].ext_cnt;
      }
    }
    FSEEK(targ, (resources->base_lba + folders[u].extent.start[0]) * block_size, SEEK_SET);
    create_inode(targ, folders[u].cur_rec * sizeof(struct S_LEAN_DIRENTRY), folders[u].buf_size * block_size,
      LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC, folders[u].links_count, folders[u].ext_cnt, &folders[u].extent);
    void *p = folders[u].root;
#ifndef INODE_HAS_EAS
    fwrite(p, (block_size - S_LEAN_INODE_SIZE), 1, targ);
    p = (void *) ((uint8_t *) p + (block_size - S_LEAN_INODE_SIZE));
#endif
    unsigned uu = 0;
    for (unsigned int f = 0; f < folders[u].buf_size; f++) {
      ++folders[u].extent.start[uu];
      FSEEK(targ, (resources->base_lba + folders[u].extent.start[uu]) * block_size, SEEK_SET);
      fwrite(p, block_size, 1, targ);
      if (--folders[u].extent.size[uu] == 0)
        ++uu;
      p = (void *) ((uint8_t *) p + block_size);
    }
    free(folders[u].root);
  }
  free(folders);

  // now write the first band to the disk
  FSEEK(targ, (resources->base_lba + super_loc) * block_size, SEEK_SET);
  printf(" Writing Super Block to block %" LL64BIT "i\n", FTELL(targ) / block_size);
  super->free_block_count = resources->tot_sectors - blocks_used;
  super->checksum = lean_calc_crc(super, block_size);
  fwrite(super, block_size, 1, targ);
  FSEEK(targ, (resources->base_lba + super->backup_super) * block_size, SEEK_SET);
  printf(" Writing Backup Super Block to block %" LL64BIT "i\n", (uint64_t) (FTELL(targ) / block_size));
  fwrite(super, block_size, 1, targ);
  --tot_blocks;

  // now create and write each remaining band (just the bitmap)
  size_t count_blocks = tot_blocks;
  uint8_t *b = bitmaps;
  for (u = 0; u < tot_bands; ++u) {
    if (u == 0)
      FSEEK(targ, (resources->base_lba + super_loc + 1) * block_size, SEEK_SET);
    else
      FSEEK(targ, (resources->base_lba + (band_size * u)) * block_size, SEEK_SET);
    printf(" Writing Bitmap #%i to block %" LL64BIT "i\n", (int) (u + 1), (uint64_t) (FTELL(targ) / block_size));
    fwrite(b, block_size, bitmap_size, targ);
    b += (bitmap_size * block_size);
    if (u < (tot_bands - 1))
      count_blocks -= band_size;
    else
      count_blocks = (size_t) (resources->base_lba + (resources->tot_sectors - (FTELL(targ) / block_size)));
  }

  // if there was only one band, we have written all sectors
  //  due to the fact that we wrote the backup super to the last
  //  sector of the band,
  // else we need to write remaining sectors (as zeros)
  if (tot_bands > 1) {
    memset(buffer, 0, block_size);
    while (count_blocks--)
      fwrite(buffer, block_size, 1, targ);
  }

  // print space used so we know how much more we have
  puts("");
  printf("     Total space used: %i%%\n", (int) ((cur_block * 100) / tot_blocks));
  printf(" Total bytes remaining: %" LL64BIT "i  (%i Meg)\n", (uint64_t) ((tot_blocks - cur_block) * block_size),
    (int) (((tot_blocks - cur_block) + (2048 - 1)) >> 11));  // 2047 to round up to the next meg

  // done, cleanup and return
  free(buffer);
  free(bitmaps);

  fclose(targ);

  return 0;
}

/* Calculate how many blocks each band will use
 *   returns the log value.  i.e.: returns (x) of  2^x 
 */
uint8_t lean_calc_log_band_size(const size_t block_size, const size_t tot_blocks) {
  uint8_t ret = 12;
  int i;

  for (i=63; i>16; i--) {
    if (tot_blocks & ((uint64_t) 1 << i)) {
      ret = (uint8_t) (i - 4);
      break;
    }
  }

  // A band must be large enough to occupy all bits in a bitmap block.
  //  therefore, 512-byte blocks must return at least a log2 of 12.
  //             1024-byte blocks must return at least a log2 of 13.
  //             2048-byte blocks must return at least a log2 of 14.
  //             4096-byte blocks must return at least a log2 of 15, etc
  if (ret < (LOG2(block_size) + 3))
    return (LOG2(block_size) + 3);
  
  return ret;
}

/* This always skips the first dword since it is the crc field.
 *  The CRC is calculated as:
 *     crc = 0;
 *     loop (n times)
 *       crc = ror(crc) + dword[x]
 */
uint32_t lean_calc_crc(const void *ptr, size_t size) {
  uint32_t crc = 0;
  const uint32_t *p = (const uint32_t *) ptr;
  unsigned int i;

  size /= sizeof(uint32_t);
  for (i = 1; i < size; ++i)
    crc = (crc << 31) + (crc >> 1) + p[i];

  return crc;
}

/* *********************************************************************************************************
 * returns count of micro-seconds since 1 Jan 1970
 * accounts for leap days, but not leap seconds
 * (we can't just use time(NULL) since that is not portable,
 *   and could be different on each compiler)
 */
uint64_t get_useconds(void) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  // credit to: http://howardhinnant.github.io/date_algorithms.html
  const int d = timeinfo->tm_mday;
  const int m = timeinfo->tm_mon + 1;
  const int y = (timeinfo->tm_year + 1900) - (m <= 2);
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned) (y - era * 400);                  // [0, 399]
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;           // [0, 146096]
  const unsigned days = era * 146097 + (int)doe - 719468;          // 719468 = days between 0000/03/01 and 1970/1/1.

  // then we need seconds
  const unsigned hours = (days * 24) + timeinfo->tm_hour;
  const unsigned mins = (hours * 60) + timeinfo->tm_min;
  return (uint64_t) (((uint64_t) mins * 60) + (uint64_t) timeinfo->tm_sec) * (uint64_t) 1000000;

  // 1493751050000000 = 2 May 2017, ~6:45p
}

// band = band to mark
// start = first bit in band
// count = count of consecutive bits to mark
// used = true = these need to be counted as used
void bitmap_mark(const int band, int start, int count, const bool used) {
  // count the sectors used
  if (used)
    blocks_used += count;

  // point to the correct starting bitmap
  uint8_t *p = bitmaps + (band * (bitmap_size * block_size));
  while (count--) {
    p[start / 8] |= (1 << (start % 8));
    ++start;
  }
}

uint64_t bitmap_find(void) {
  while (bitmaps[cur_block / 8] & (1 << (cur_block % 8))) {
    ++cur_block;
    if (cur_block >= (tot_blocks - 1)) {  // -1 for the last sector (in the band) being the backup super (we just do -1 for ease)
      printf("Image size too small.  No more available sectors...\n");
      exit(-1);
    }
  }

  // mark it
  bitmaps[cur_block / 8] |= (1 << (cur_block % 8));
  ++blocks_used;

  return cur_block++;
}

// In theory, this should only need a maximum of two extents
//  most of the time, only one, since we have a blank canvas.
//  however, when we get to the next band, we need to skip over the
//   bitmap (and super backup), so this will need two extents.
// size = bytes (remember to subtract if EA's not used)
size_t get_next_extent(size_t size, struct S_LEAN_EXTENT *extent) {
  size_t i = 0;
  uint64_t block, c = 1;

  extent->start[0] = bitmap_find();
  extent->size[0] = 1;
  while (--size) {
    block = bitmap_find();
    if (block == (extent->start[i] + c)) {
      ++extent->size[i];
      ++c;
    }
    else {
      ++i;
      if (i == LEAN_INODE_EXTENT_CNT) {
        printf("Extent size reached more than %i...\n", LEAN_INODE_EXTENT_CNT);
        exit(-1);
      }
      extent->start[i] = block;
      extent->size[i] = 1;
      c = 1;
    }
  }

  // return count of extents used
  return i + 1;
}

void create_root_entry(const size_t folder, char *filename, const size_t pos, uint64_t block, const uint64_t parent) {
  char name[512], *p;
  size_t i;
  bool is_folder = FALSE, fnd = FALSE;

  // check to see if the file name given is part of a path (i.e.: has directory names first)
  if (p = strchr(filename + pos, '/')) {
    memcpy(name, filename, (p - filename));
    name[(p - filename)] = '\0';

    // now see if we have already made this folders block
    for (i = 0; i < cur_folder; i++) {
      if (strcmp(folders[i].name, name) == 0) {
        fnd = TRUE;
        break;
      }
    }

    // if we didn't find an existing folder, create one
    if (!fnd) {
      if (cur_folder == DEF_FOLDER_CNT) {
        cur_folder_size += DEF_FOLDER_CNT;
        folders = (struct S_FOLDERS *) realloc(folders, cur_folder_size * sizeof(struct S_FOLDERS));
      }
      // i == cur_folder from for() loop above
      strcpy(folders[i].name, name);
      folders[i].buf_size = FOLDER_SIZE;
      folders[i].root = calloc(FOLDER_SIZE * block_size, 1);
      folders[i].ext_cnt = get_next_extent(FOLDER_SIZE + 1, &folders[i].extent);
      root_start((struct S_LEAN_DIRENTRY*) folders[i].root, folders[i].extent.start[0], parent);
      folders[i].cur_rec = 2;
      folders[i].links_count = 1;   // self ('.' link)
      folders[i].parent = &folders[folder];
      is_folder = TRUE;
      ++cur_folder;
    }

    create_root_entry(i, filename, strlen(name) + 1, block, folders[i].extent.start[0]);
    block = folders[i].extent.start[0];
    strcpy(name, name + pos);
  }	else
    strcpy(name, filename + pos);

  // if it was not already found as an existing directory, we need to make the entry
  if (!fnd) {
    const size_t rec_len = ((strlen(name) + 12 + 15) / 16);
    // if we are near the end of the buffer, we need to add to the buffer size
    if (((folders[folder].cur_rec + rec_len) * sizeof(struct S_LEAN_DIRENTRY)) > (folders[folder].buf_size * block_size)) {
      folders[i].buf_size += FOLDER_SIZE;
      folders[folder].root = (void *) realloc(folders[folder].root, (folders[i].buf_size * block_size));
    }
    struct S_LEAN_DIRENTRY *entry = (struct S_LEAN_DIRENTRY *)
      ((uint8_t *) folders[folder].root + (folders[folder].cur_rec * sizeof(struct S_LEAN_DIRENTRY)));
    entry->name_len = (uint16_t) strlen(name);
    entry->inode = block;
    entry->type = (is_folder) ? LEAN_FT_DIR : LEAN_FT_REG;
    entry->rec_len = (uint8_t) rec_len;
    memcpy(entry->name, name, entry->name_len);

    // move to next record
    folders[folder].cur_rec += entry->rec_len;

    if (is_folder) {
      ++folders[i].links_count;
      if (folders[i].parent)
        ++folders[i].parent->links_count;
    }
  }
}

// create the first two entries of a root directory
void root_start(struct S_LEAN_DIRENTRY *record, const uint64_t self, const uint64_t parent) {
  // since '.' and '..' will be less than 4 chars each, we can cheat and use the record index [0] and [1]
  // The "." entry
  record[0].inode = self;
  record[0].type = LEAN_FT_DIR;
  record[0].rec_len = 1;
  record[0].name_len = 1;
  record[0].name[0] = '.';
  // The ".." entry
  record[1].inode = parent;
  record[1].type = LEAN_FT_DIR;
  record[1].rec_len = 1;
  record[1].name_len = 2;
  record[1].name[0] = '.';
  record[1].name[1] = '.';
}

void create_inode(FILE *fp, const uint64_t file_size, const uint64_t allocation_size, const uint32_t attrib,
  const int link_count, const size_t ext_cnt, struct S_LEAN_EXTENT *extent) {

  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) malloc(block_size);
  memset(inode, 0, block_size);

  inode->magic = LEAN_INODE_MAGIC;
  inode->extent_count = (uint8_t) ext_cnt;
  memset(inode->reserved, 0, 3);
  inode->indirect_count = 0;
  inode->links_count = link_count;
  inode->attributes = LEAN_ATTR_IXUSR | LEAN_ATTR_IRUSR | LEAN_ATTR_IWUSR |
#ifdef INODE_HAS_EAS
    LEAN_ATTR_INLINEXTATTR |   // make the data start on the next sector boundary
#endif                         
    attrib;
  inode->file_size = file_size;
#ifdef INODE_HAS_EAS
  inode->block_count = ((allocation_size + (block_size - 1)) / block_size) + 1;
#else
  if (allocation_size > (block_size - S_LEAN_INODE_SIZE))
    inode->block_count = (((allocation_size - (block_size - S_LEAN_INODE_SIZE)) + (block_size - 1)) / block_size) + 1;
  else
    inode->block_count = 1;  // everything fits in the INODE sector
#endif
  inode->acc_time =
    inode->sch_time =
    inode->mod_time =
    inode->cre_time = get_useconds();
  inode->first_indirect = 0;
  inode->last_indirect = 0;
  inode->fork = 0;
  for (size_t e = 0; e < ext_cnt; ++e) {
    inode->extent_start[e] = extent->start[e];
    inode->extent_size[e] = extent->size[e];
  }
  inode->checksum = lean_calc_crc((uint32_t *) inode, S_LEAN_INODE_SIZE);

#ifdef INODE_HAS_EAS
  // extended attributes
  uint32_t *ea = (uint32_t *) ((uint32_t) inode + S_LEAN_INODE_SIZE);
  ea[0] = block_size - S_LEAN_INODE_SIZE - sizeof(uint32_t);
#endif

  // write the inode to the disk
#ifdef INODE_HAS_EAS
  fwrite(inode, block_size, 1, fp);
#else
  fwrite(inode, S_LEAN_INODE_SIZE, 1, fp);
#endif
  
  free(inode);
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /E         - Tell this app to use an existing image to write the fs to.
 *  /V         - Volume Name
 */
void parse_command(int argc, char *argv[], char *filename, bool *existing_image, char *label) {
  int i;
  const char *s;

  strcpy(filename, "");

  for (i = 1; i < argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if ((strcmp(s, "E") == 0) ||
        (strcmp(s, "e") == 0))
        * existing_image = TRUE;
      else if ((memcmp(s, "v:", 2) == 0) ||
        (memcmp(s, "V:", 2) == 0)) {
        strncpy(label, s + 2, NAME_LEN_MAX - 1);
        label[NAME_LEN_MAX - 1] = 0;  // make sure null terminated
      }
      else
        printf(" Unknown switch parameter: /%s\n", s);
    }
    else
      strcpy(filename, s);
  }
}
