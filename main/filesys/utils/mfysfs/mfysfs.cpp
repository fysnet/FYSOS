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
 *  MFYSFS.EXE
 *   See the included files.txt file for an example of the resource file
 *
 *   This utility will take a list of filenames and include those
 *    files in the image, creating root entries.
 *
 *  Assumptions/prerequisites:
 *   - this utility assumes that the count of files you add to this image
 *     will fit within the amount of clusters you allocate for the root
 *
 *   - Remember, I didn't write this utility to be complete or robust.
 *     I wrote it to simply make a fysfs image for use with this book.
 *     Please consider this if you add or modify to this utility.
 *
 *  Last updated: 5 Feb 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os mfysfs.c -o mfysfs.exe -s
 *
 *  Usage:
 *    mfysfs filename.txt /L /E
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
#include "mfysfs.h"

#define ROOT_ENTRIES    ((4 * SPCLUST) * 4)  // 4 per sector * ? sects per clust * 4 clusters
#define BITMAPS         2
#define ROOT_SECTS      ((ROOT_ENTRIES * sizeof(struct S_FYSFS_ROOT)) / 512)

#define COPY_VALID      (1<<1)

bit8u buffer[16 * 512]; // at least 16 * 512

int main(int argc, char *argv[]) {
  bit32u i, j;
  FILE *src, *targ;
  bool large_clusters = FALSE, existing_image = FALSE;
  struct S_RESOURCE *resources;
  char filename[NAME_LEN_MAX];
  char label[NAME_LEN_MAX] = "This is a volume label for this FYSFS volume.";
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, &large_clusters, &existing_image, label);
  
  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources) {
    printf("\n Error with Resource file. '%s'", filename);
    if (resources) free(resources);
    return -1;
  }
  
  // if spc is > 16, give error
  if (!POWERofTWO(SPCLUST) || (SPCLUST > 16)) {
    printf("\n Error is sectors per cluster count.");
    return -1;
  }
  
  // do we need to add sectors to end on a cylinder boundary?
  bit32u cylinders = (bit32u) (resources->tot_sectors + ((16*63)-1)) / (16*63);    // cylinders used
  bit32u add = (bit32u) (((bit64u) cylinders * (16*63)) - resources->tot_sectors); // sectors to add to boundary on cylinder
  if (add && (resources->tot_sectors > 2880)) {  // don't add if floppy image
    printf("\n Total Sectors does not end on cylinder boundary. Expand to %" LL64BIT "i? [Y|N] ", resources->tot_sectors + add);
    if (toupper(getche()) == 'Y') {
      resources->tot_sectors += add;
      // need to calculate again since we added sectors
      cylinders = (bit32u) ((resources->tot_sectors + ((16*63)-1)) / (16*63));
    }
  }
  
  // initialize the CRC code
  crc32_initialize();
  
  // total sectors (minus base_lba) must be a multiple of SPCLUST not exceeding amount requested.
  resources->tot_sectors -= resources->base_lba;
  resources->tot_sectors &= ~(SPCLUST - 1);
  
  // total sectors in image file
  bit32u bitmap_sects = (bit32u) ((((resources->tot_sectors * 2) + 7) / 8) + 511) / 512;
  bit32u copy_cluster = (bit32u) ((resources->tot_sectors + (SPCLUST - 1)) / SPCLUST) - 1;
  
  // create super block
  struct S_FYSFS_SUPER super;
  memset(&super, 0, sizeof(struct S_FYSFS_SUPER));
  super.sig[0] = 0x46595346;  // signature ‘FYSF’ ‘SUPR’ (0x46595346 0x53555052)
  super.sig[1] = 0x53555052;
  super.ver = 0x0162;         // 1.62
  super.sect_clust = SPCLUST;
  super.encryption = 0; // none
  super.bitmaps = BITMAPS;
  super.bitmap_flag = 0x02;
  super.root_entries = ROOT_ENTRIES;
  super.base_lba = resources->base_lba;
  super.root = FIRST_BITMAP_LSN + (bitmap_sects * BITMAPS);
  super.data = super.root;
  super.data_sectors = resources->tot_sectors - super.data;
  super.sectors = resources->tot_sectors;
  super.bitmap = FIRST_BITMAP_LSN;
  super.bitmapspare = FIRST_BITMAP_LSN + bitmap_sects;
  super.chkdsk = 0;
  super.lastopt = 0;
  super.flags = COPY_VALID | (0<<0); // copy of super is in last cluster
  calc_guid(&super.guid);
  strcpy(super.vol_label, label);
  super.crc = crc32(&super, sizeof(struct S_FYSFS_SUPER));
  
  if (!existing_image) {
    // create target file
    if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
      printf("\nError creating target file: '%s'", resources->targ_filename);
      return -1;
    }
  } else {
    // open existing target file
    if ((targ = fopen(resources->targ_filename, "r+b")) == NULL) {
      printf("\nError opening target file: '%s'", resources->targ_filename);
      return -1;
    }
    FSEEK(targ, (resources->base_lba + 16) * 512, SEEK_SET);
  }
  
  // if we are not working on an existing image file, 
  if (!existing_image) {
    // create the MBR and padding sectors
    i = 0;
    if (strlen(resources->mbr_filename)) {
      if ((src = fopen(resources->mbr_filename, "rb")) == NULL) {
        printf("\nError opening mbr.bin file.");
        fclose(targ);
        return -2;
      }
      fread(buffer, 512, 1, src);
    
      // create and write the Disk Indentifier
      srand((unsigned int) time(NULL));  // seed the randomizer
      // We call rand() multiple times because rand() usually is set for 0 -> 32768 only.
      * (bit32u *) &buffer[0x01B8] = (bit32u) ((rand() << 20) | (rand() << 10) | (rand() << 0));
      * (bit16u *) &buffer[0x01BC] = 0x0000;
    }
    if (resources->base_lba > 0) {
      // create a single partition entry pointing to our partition
      struct PART_TBLE *pt = (struct PART_TBLE *) &buffer[0x01BE];
      pt->bi = 0x80;
      lba_to_chs(&pt->start_chs, (bit32u) resources->base_lba);
      pt->si = 0x22;
      lba_to_chs(&pt->end_chs, (bit32u) ((cylinders * resources->heads * resources->spt) - 1 - resources->base_lba));  // last sector - 1 for the MBR
      pt->startlba = (bit32u) resources->base_lba;
      pt->size = (bit32u) ((cylinders * resources->heads * resources->spt) - resources->base_lba);
      printf("\n Writing MBR to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
      buffer[510] = 0x55; buffer[511] = 0xAA;
      fwrite(buffer, 512, 1, targ);
      memset(buffer, 0, 512);
      i = 1;
    }
    if (i<(bit32u) resources->base_lba)
      printf("\n Writing Padding between MBR and Base LBA...");
    for (; i<(bit32u) resources->base_lba; i++)
      fwrite(buffer, 512, 1, targ);
    
    // create the boot sectors (16)
    if (strlen(resources->boot_filename)) {
      if ((src = fopen(resources->boot_filename, "rb")) == NULL) {
        printf("\nError opening boot file.");
        fclose(targ);
        return -3;
      }
      
      // clearing the buffer first makes sure that if we don't read 16 sectors
      //  of boot file (fysfs.bin), the last part will be zeros.
      memset(buffer, 0, 512*16);
      if (fread(buffer, 512, 16, src) > 16)
        printf("\nWarning: Boot file was more than 16 sectors.");
      fclose(src);
    } else
      memset(buffer, 0, 512*16);

    // write the first 16 sectors
    // update the sig and base lba within the given boot code
    //  this assumes that the coder of the boot code knew that this area was
    //  reserved for this data...
    // this reserved area is the last few bytes of the first sector of the partition/disk
    //  sig       dword  (unique id for partition/disk)
    //  base_lba  qword  (0 for floppy, could be 63 for partitions)
    //  boot sig  word   (0xAA55)
    * (bit32u *) &buffer[498] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
    * (bit64u *) &buffer[502] = (bit64u) resources->base_lba;
    
    buffer[510] = 0x55; buffer[511] = 0xAA;
    printf("\n Writing Boot Sectors to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
    fwrite(buffer, 512, 16, targ);
  } // !existing_image
  
  // Write the Super Block
  printf("\n Writing SuperBlock to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
  fwrite(&super, 512, 1, targ);
  resources->tot_sectors -= FIRST_BITMAP_LSN;
  
  // fill in the sectors between the super and the first Bitmap (if any)
  memset(buffer, 0, 512);
  for (i=0; i<(FIRST_BITMAP_LSN-17); i++) {
    fwrite(buffer, 512, 1, targ);
    resources->tot_sectors--;
  }
  
  // now write the bitmap(s) to the image.
  //  we will come back and write it again once we have written the files
  bit64u bitmap_offset = FTELL(targ);
  for (i=0; i<(bitmap_sects * BITMAPS); i++) {
    fwrite(buffer, 512, 1, targ);
    resources->tot_sectors--;
  }
  
  // fill in the sectors between the last bitmap and the root
  bit32u fill = (bit32u) (super.root - (bitmap_sects * BITMAPS) - FIRST_BITMAP_LSN);
  for (i=0; i<fill; i++) {
    fwrite(buffer, 512, 1, targ);
    resources->tot_sectors--;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // create the root
  // We will first write an empty root so that we can get to the data area
  // we will come back and write the updated root when we are done
  bit64u root_offset = FTELL(targ);
  for (i=0; i<ROOT_SECTS; i++) {
    fwrite(buffer, 512, 1, targ);
    resources->tot_sectors--;
  }
  
  /*  Since we have an empty disk and root, we know that we can start with the
   *  first cluster and write files consecutively.  There is no need to find
   *  free clusters or slots when we create new entries.  Therefore, we will
   *  start with the first cluster after the root (cur_clust) and the first
   *  slot within the root (cur_slot).  We will also keep track of how many
   *  clusters we use so that we can update the bitmap(s).
   */
  size_t read;
  int cur_slot = 0;
  bit32u clusters_used = (ROOT_SECTS / SPCLUST);
  bit64u file_size, cur_clust = (ROOT_SECTS / SPCLUST);
  struct S_FYSFS_ROOT *root = (struct S_FYSFS_ROOT *) calloc(ROOT_SECTS, 512);
  for (i=0; i<resources->file_cnt; i++) {
    // get the file to write to the image
    if ((src = fopen(resources->files[i].path_filename, "rb")) == NULL) {
      printf("\nError opening %s file.", resources->files[i].path_filename);
      continue;
    }
    FSEEK(src, 0, SEEK_END);
    file_size = FTELL(src);
    rewind(src);
    
    // create the root entry(s)
    create_root_entry(root, resources->files[i].filename, file_size, &cur_slot, &cur_clust, SPCLUST, large_clusters);
    
    printf("\n Writing %s to LBA %" LL64BIT "i", resources->files[i].filename, (bit64u) (FTELL(targ) / 512));
    do {
      // by clearing the buffer first, we make sure that the "padding" bytes are all zeros
      memset(buffer, 0, SPCLUST * 512);
      read = fread(buffer, 1, SPCLUST * 512, src);
      if (read == 0)
        break;
      fwrite(buffer, 512, SPCLUST, targ);
      resources->tot_sectors -= SPCLUST;
      clusters_used++;
    } while (read == (SPCLUST * 512));
    fclose(src);
  }
  
  // write remaining sectors (as zeros)
  printf("\n Writing rest of partition to LBA %" LL64BIT "i (%" LL64BIT "i sectors)", (bit64u) (FTELL(targ) / 512), resources->tot_sectors);
  memset(buffer, 0, 512);
  while (resources->tot_sectors) {
    fwrite(buffer, 512, 1, targ);
    if ((resources->tot_sectors % 2500) == 0)
      putch('.');
    resources->tot_sectors--;
  }
  
  // if the COPY_VALID flag is set, back up and write a copy of the super 
  //  to the first sector of the last cluster.  Then mark the bitmap(s) too below.
#if (COPY_VALID != 0)
  FSEEK(targ, (bit64u) ((resources->base_lba + (copy_cluster * SPCLUST)) * 512), SEEK_SET);
  printf("\n Writing backup SuperBlock to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
  fwrite(&super, 512, 1, targ);
#endif
  
  // now we need to go back and write the bitmap(s) now that we have updated them.
  bit8u *bitmap = (bit8u *) calloc(bitmap_sects, 512);
  j = 0;
  for (i=0; i<clusters_used; ) {
    bitmap[j] |= (0x40 >> ((i % 4) * 2));
    i++;
    if (!(i % 4))
      j++;
  }
  
  // if copy_valid, mark the last cluster too.
#if (COPY_VALID != 0)
  bitmap[copy_cluster / 4] |= (OCCUPIED >> ((copy_cluster % 4) * 2));
#endif
  
  FSEEK(targ, bitmap_offset, SEEK_SET);
  printf("\n Writing bitmaps to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
  for (i=0; i<BITMAPS; i++)
    fwrite(bitmap, 512, bitmap_sects, targ);
  free(bitmap);
  
  // now we need to go back and write the root
  FSEEK(targ, root_offset, SEEK_SET);
  printf("\n Writing root directory to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / 512));
  fwrite(root, 512, ROOT_SECTS, targ);
  
  // close the file
  fclose(targ);
  
  // free the root and resources buffer
  free(root);
  free(resources);
  
  // return good
  return 0;
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /L         - Tell this app to use 64-bit cluster numbers
 *  /E         - Tell this app to use an existing image to write the fs to.
 *  /V         - Volume Name
 */
void parse_command(int argc, char *argv[], char *filename, bool *large_clusters, bool *existing_image, char *label) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if ((strcmp(s, "L") == 0) ||
          (strcmp(s, "l") == 0))
        *large_clusters = TRUE;
      else if ((strcmp(s, "E") == 0) ||
               (strcmp(s, "e") == 0))
        *existing_image = TRUE;
      else if ((memcmp(s, "v:", 2) == 0) ||
               (memcmp(s, "V:", 2) == 0)) {
        strncpy(label, s + 2, NAME_LEN_MAX - 1);
        label[NAME_LEN_MAX-1] = 0;  // make sure null terminated
      } else
        printf("\n Unknown switch parameter: /%s", s);
    } else
      strcpy(filename, s);
  }
}

void create_root_entry(struct S_FYSFS_ROOT *root, char *filename, const bit64u file_size, int *cur_slot, bit64u *cur_clust, const int spc, const bool large_clusters) {
  
  struct S_FYSFS_ROOT *r = &root[*cur_slot];
  struct S_FYSFS_CONT *c = NULL;
  char *fnp;
  int len;
  
  // make sure that the entry is cleared
  memset(r, 0, sizeof(struct S_FYSFS_ROOT));
  
  // create the ROOT SLOT
  r->sig = S_FYSFS_ROOT_NEW;
  r->fsize = file_size;
  r->attribute = FYSFS_ATTR_ARCHIVE;
  r->created = r->lastaccess = get_seconds();
  
  int name_len = (int) strlen(filename);
  if (name_len <= NAME_FS_SPACE) {
    memcpy(r->name_fat, filename, name_len);
    r->namelen = name_len;
    r->name_continue = 0;
  } else {
    memcpy(r->name_fat, filename, NAME_FS_SPACE);
    r->namelen = NAME_FS_SPACE;
    r->name_continue = *cur_slot + 1;
    name_len -= NAME_FS_SPACE;
    fnp = filename + NAME_FS_SPACE;
    // create a name continuation slot
    while (name_len > 0) {
      (*cur_slot)++;
      c = (struct S_FYSFS_CONT *) &root[*cur_slot];
      memset(c, 0, sizeof(struct S_FYSFS_CONT));
      c->sig = S_FYSFS_CONT_NAME;
      c->previous = *cur_slot - 1;
      c->flags = 0;
      if (name_len <= CONT_NAME_SPACE) {
        c->count = name_len;
        memcpy(c->name_fat, fnp, name_len);
        c->next = 0;
        c->crc = 0; c->crc = (bit8u) crc32(c, sizeof(struct S_FYSFS_CONT));
        break;
      } else {
        c->count = CONT_NAME_SPACE;
        memcpy(c->name_fat, fnp, CONT_NAME_SPACE);
        fnp += CONT_NAME_SPACE;
        name_len -= CONT_NAME_SPACE;
        c->next = *cur_slot + 1;
        c->crc = 0; c->crc = (bit8u) crc32(c, sizeof(struct S_FYSFS_CONT));
      }
    }
  }
  
  // now create the fat entries
  int count = (int) ((file_size + ((512 * spc) - 1)) / (512 * spc));
  
  // calculate the remaining size in the SLOT entry
  // remember that all 64-bit entries must be in a continuation slot
  len = (!large_clusters) ? ((NAME_FS_SPACE - r->namelen) / sizeof(bit32u)) : 0;
  bit8u *fat = &r->name_fat[(r->namelen+3) & ~0x03];
  while (count > 0) {
    for (; len>0; len--) {
      if (large_clusters) {
        * (bit64u *) fat = *cur_clust;
        fat += sizeof(bit64u);
      } else {
        * (bit32u *) fat = (bit32u) *cur_clust;
        fat += sizeof(bit32u);
      }
      if (c) c->count++;
      else   r->fat_entries++;
      (*cur_clust)++;
      if (--count == 0)
        break;
    }
    if (c) {
      c->crc = 0; c->crc = (bit8u) crc32(c, sizeof(struct S_FYSFS_CONT));
    }
    if (count == 0)
      break;
    
    // else we need to make another cont_fat slot
    (*cur_slot)++;
    if (c) {
      c->next = *cur_slot;
      c->crc = 0; c->crc = (bit8u) crc32(c, sizeof(struct S_FYSFS_CONT));
    } else 
      r->fat_continue = *cur_slot;
    c = (struct S_FYSFS_CONT *) &root[*cur_slot];
    memset(c, 0, sizeof(struct S_FYSFS_CONT));
    c->sig = S_FYSFS_CONT_FAT;
    c->previous = *cur_slot - 1;
    c->flags = (large_clusters) ? 0x01 : 0x00;
    fat = (bit8u *) &c->name_fat;
    len = CONT_NAME_SPACE / ((large_clusters) ? 8 : 4);
  }
  (*cur_slot)++;
  
  r->crc = 0; r->crc = (bit8u) crc32(r, sizeof(struct S_FYSFS_ROOT));
}

/* *********************************************************************************************************
 * returns count of seconds since 1 Jan 1980
 * accounts for leap days, but not leap seconds
 * (we can't just use time(NULL) since that is not portable,
 *   and could be different on each compiler)
 */
bit32u get_seconds() {
  time_t rawtime;
  struct tm *timeinfo;
  
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  
  // credit to: http://howardhinnant.github.io/date_algorithms.html
  const int d = timeinfo->tm_mday;
  const int m = timeinfo->tm_mon + 1;
  const int y = (timeinfo->tm_year + 1900) - (m <= 2);
  const int era = (y >= 0 ? y : y-399) / 400;
  const unsigned yoe = (unsigned) (y - era * 400);                  // [0, 399]
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
  const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;           // [0, 146096]
  const unsigned days = era * 146097 + (int) doe - 719468 - 3652;   // 719468 = days between 0000/03/01 and 1970/1/1.
                                                                    // 3652 for the ten years between 1970 and 1980. 10 * 365 + 2 leap days
  // then we need seconds
  const unsigned hours = (days * 24) + timeinfo->tm_hour;
  const unsigned mins = (hours * 60) + timeinfo->tm_min;
  return (mins * 60) + timeinfo->tm_sec;
}

/* *********************************************************************************************************
 * The following is the code used to calculate the CRC's in the SUPER as well as the SLOT's
 */
bit32u crc32_table[256]; // CRC lookup table array.

void crc32_initialize(void) {
  int i, j;
  
  memset(crc32_table, 0, sizeof(crc32_table));
  
  // 256 values representing ASCII character codes.
  for (i=0; i<=0xFF; i++) {
    crc32_table[i] = crc32_reflect(i, 8) << 24;
    
    for (j=0; j<8; j++)
      crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
    
    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
}

// Reflection is a requirement for the official CRC-32 standard.
//  You can create CRCs without it, but they won't conform to the standard.
bit32u crc32_reflect(bit32u reflect, char ch) {
  bit32u ret = 0;
  int i;
  
  // Swap bit 0 for bit 7 bit 1 For bit 6, etc....
  for (i=1; i<(ch + 1); i++) {
    if (reflect & 1)
      ret |= 1 << (ch - i);
    reflect >>= 1;
  }
  
  return ret;
}

bit32u crc32(void *data, bit32u len) {
  bit32u crc = 0xFFFFFFFF;
  crc32_partial(&crc, (bit8u *) data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(bit32u *crc, bit8u *data, bit32u len) {
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}
