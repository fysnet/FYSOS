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
 *  MKSFS.EXE
 *   See the included files.txt file for an example of the resource file
 *
 *   This utility will take a list of filenames and include those
 *    files in the image, creating root entries.
 *
 *  Assumptions/prerequisites:
 *   - this utility assumes that the count of files you add to this image
 *     will fit within the amount of space you have for the Data Block Area
 *     and is hard coded for no more than 1024 files.
 *
 *   - Remember, I didn't write this utility to be complete or robust.
 *     I wrote it to simply make a SFS image for use with this book.
 *     Please consider this if you add or modify to this utility.
 *
 *  Last updated: 5 Feb 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os gd_ehci.c -o gd_ehci.exe -s
 *
 *  Usage:
 *   gcc -Os mksfs.cpp -o mksfs.exe -s
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

#include "mksfs.h"

// we only support block sizes of 512.  I didn't look to see if
//  we can change this yet and still have the code work correctly.
#define SFS_BLOCK_SIZE  512

int main(int argc, char *argv[]) {
  bit32u i;
  int name_len, cnt;
  size_t read;
  bit32u reserved_blocks;
  FILE *src, *targ;
  struct S_RESOURCE *resources;
  char filename[NAME_LEN_MAX];
  char label[NAME_LEN_MAX] = "This is a volume label for this SFS volume.";
  char *s, *t;
  bit8u buffer[SFS_BLOCK_SIZE];
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, label);
  
  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources) {
    printf("\n Error with Resource file. '%s'", filename);
    if (resources) free(resources);
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
  
  // create super block
  struct S_SFS_SUPER super;
  memset(&super, 0, sizeof(struct S_SFS_SUPER));
  super.time_stamp = get_64kseconds();
  //super.data_block_count;   // Size of data area in blocks    (updated later)
  //super.index_size;         // Size of index area *in bytes*  (updated later)
  super.magic_version = 0x1A534653; // Magic number (0x534653) + SFS version (0x10 for Version 1.0, 0x1A for v1.10)
  super.total_blocks = resources->tot_sectors - resources->base_lba; // Total number of blocks in volume
  //super.resv_blocks = 1;            // Number of reserved blocks (updated later)
  super.block_size = SFS_BLOCK_SIZE >> 8;         // Block size (2^(x+7) where x = 2 = 512)
  
  // create target file
  if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
    printf("\nError creating target file: '%s'", resources->targ_filename);
    return -1;
  }
  
  // create the MBR and padding sectors, if mbr file given
  i = 0;
  if (strlen(resources->mbr_filename)) {
    if ((src = fopen(resources->mbr_filename, "rb")) == NULL) {
      printf(" Error opening mbr.bin file.\n");
      fclose(targ);
      return -2;
    }
    fread(buffer, SFS_BLOCK_SIZE, 1, src);
    
    // create and write the Disk Indentifier
    // We call rand() multiple times.
    * (bit32u *) &buffer[0x01B8] = (bit32u) ((rand() << 20) | (rand() << 10) | (rand() << 0));
    * (bit16u *) &buffer[0x01BC] = 0x0000;
  }
  if (resources->base_lba > 0) {
    // create a single partition entry pointing to our partition
    struct PART_TBLE *pt = (struct PART_TBLE *) &buffer[0x01BE];
    pt->bi = 0x80;
    lba_to_chs(&pt->start_chs, (bit32u) resources->base_lba);
    pt->si = 0x53;  //'S'
    lba_to_chs(&pt->end_chs, (bit32u) ((cylinders * resources->heads * resources->spt) - 1 - resources->base_lba));  // last sector - 1 for the MBR
    pt->startlba = (bit32u) resources->base_lba;
    pt->size = (bit32u) ((cylinders * resources->heads * resources->spt) - resources->base_lba);
    printf(" Writing MBR to LBA %" LL64BIT "i\n", (bit64u) (FTELL(targ) / SFS_BLOCK_SIZE));
    * (bit16u *) &buffer[0x01FE] = 0xAA55;
    fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
    memset(buffer, 0, SFS_BLOCK_SIZE);
    i = 1;
  }
  if (i<(bit32u) resources->base_lba)
    printf(" Writing Padding between MBR and Base LBA...\n");
  for (; i<(bit32u) resources->base_lba; i++)
    fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
  resources->tot_sectors -= resources->base_lba;
  
  // write boot sector
  reserved_blocks = 1;
  if (strlen(resources->boot_filename)) {
    if ((src = fopen(resources->boot_filename, "rb")) == NULL) {
      printf("\nError opening boot file.");
      fclose(targ);
      return -2;
    }
    fread(buffer, SFS_BLOCK_SIZE, 1, src);
    
    // update the sig and base lba within the given boot code
    //  this assumes that the coder of the boot code knew that this area was
    //  reserved for this data...
    // this reserved area is the last few bytes of the first sector of the partition/disk
    //  sig       dword  (unique id for partition/disk)
    //  base_lba  qword  (0 for floppy, could be 63 for partitions)
    //  boot sig  word   (0xAA55)
    * (bit32u *) &buffer[498] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
    * (bit64u *) &buffer[502] = (bit64u) resources->base_lba;
    * (bit16u *) &buffer[0x01FE] = 0xAA55;
    printf("\n Writing boot code to LBA %" LL64BIT "i", (bit64u) (FTELL(targ) / SFS_BLOCK_SIZE));
    fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
    memset(buffer, 0, SFS_BLOCK_SIZE);
    
    // are there any more sectors of the boot code?
    while (fread(buffer, SFS_BLOCK_SIZE, 1, src)) {
      fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
      reserved_blocks++;
    }
  } else {
    // write a dummy boot sector
    memset(buffer, 0, SFS_BLOCK_SIZE);
    * (bit16u *) &buffer[0x01FE] = 0xAA55;
    fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
  }
  resources->tot_sectors -= reserved_blocks;
  
  // allocate the Index Data Area buffer
  // TODO: we currently assume less than 1024 files
  int ida_count = 0;
  void *ida = calloc(1024 * SFS_ENTRY_SIZE, 1);
  
  // create the Start Marker Entry
  struct S_SFS_START *start = (struct S_SFS_START *) ((bit8u *) ida + (ida_count * SFS_ENTRY_SIZE));
  start->type = SFS_ENTRY_START;
  start->crc = calc_crc(start, SFS_ENTRY_SIZE);
  ida_count++;
  
  /*  Since we have an empty disk, we know that we can start with the
   *  first cluster and write files consecutively.  Therefore, we will
   *  start with the first block after the boot.
   */
  bit64u file_size, data_used = 0;
  for (i=0; i<resources->file_cnt; i++) {
    // if we are a directory only, just create the entry for it.
    // we don't put anything in the data block area
    if (resources->files[i].param == 1) {
      // create the entry(s)
      struct S_SFS_DIR *dir = (struct S_SFS_DIR *) ((bit8u *) ida + (ida_count * SFS_ENTRY_SIZE));
      dir->type = SFS_ENTRY_DIR;
      dir->num_cont = 0;
      dir->time_stamp = get_64kseconds();
      s = resources->files[i].filename;
      name_len = (int) (strlen(s) + 1); // include the NULL
      cnt = (name_len < DIR_NAME_LEN) ? name_len : DIR_NAME_LEN;
      memcpy(dir->name, s, cnt);
      name_len -= cnt;
      s += cnt;
      ida_count++;
      while (name_len > 0) {
        dir->num_cont++;
        t = (char *) ((bit8u *) dir + (dir->num_cont * SFS_ENTRY_SIZE));
        cnt = (name_len < SFS_ENTRY_SIZE) ? name_len : SFS_ENTRY_SIZE;
        memcpy(t, s, cnt);
        name_len -= cnt;
        ida_count++;
      }
      dir->crc = 0;
      dir->crc = calc_crc(dir, (1 + dir->num_cont) * SFS_ENTRY_SIZE);
      
      if (strlen(resources->files[i].filename) < 32)
        printf("\n  Adding \"/%s\"", resources->files[i].filename);
      else {
        char temp[33];
        strncpy(temp, resources->files[i].filename, 32);
        temp[32] = '\0';
        printf("\n  Adding \"/%s\"...", temp);
      }
    } else {
      // get the file to write to the image
      if ((src = fopen(resources->files[i].path_filename, "rb")) == NULL) {
        printf("\nError opening %s file.", resources->files[i].path_filename);
        continue;
      }
      FSEEK(src, 0, SEEK_END);
      file_size = FTELL(src);
      rewind(src);
      
      // create the entry(s)
      struct S_SFS_FILE *file = (struct S_SFS_FILE *) ((bit8u *) ida + (ida_count * SFS_ENTRY_SIZE));
      file->type = SFS_ENTRY_FILE;
      file->num_cont = 0;
      file->time_stamp = get_64kseconds();
      file->start_block = reserved_blocks + data_used;  // files are zero based from start of volume
      file->end_block = (file->start_block + ((file_size + (SFS_BLOCK_SIZE - 1)) & ~(SFS_BLOCK_SIZE - 1)) / SFS_BLOCK_SIZE) - 1;
      file->file_len = file_size;
      s = resources->files[i].filename;
      name_len = (int) (strlen(s) + 1); // include the NULL
      cnt = (name_len < FILE_NAME_LEN) ? name_len : FILE_NAME_LEN;
      memcpy(file->name, s, cnt);
      name_len -= cnt;
      s += cnt;
      ida_count++;
      while (name_len > 0) {
        file->num_cont++;
        t = (char *) ((bit8u *) file + (file->num_cont * SFS_ENTRY_SIZE));
        cnt = (name_len < SFS_ENTRY_SIZE) ? name_len : SFS_ENTRY_SIZE;
        memcpy(t, s, cnt);
        name_len -= cnt;
        ida_count++;
      }
      file->crc = 0;
      file->crc = calc_crc(file, (1 + file->num_cont) * SFS_ENTRY_SIZE);
      
      if (strlen(resources->files[i].filename) < 32)
        printf("\n Writing \"%s\" to LBA %" LL64BIT "i", resources->files[i].filename, (bit64u) (FTELL(targ) / SFS_BLOCK_SIZE));
      else {
        char temp[33];
        strncpy(temp, resources->files[i].filename, 32);
        temp[32] = '\0';
        printf("\n Writing \"%s\"... to LBA %" LL64BIT "i", temp, (bit64u) (FTELL(targ) / SFS_BLOCK_SIZE));
      }
      
      do {
        // by clearing the buffer first, we make sure that the "padding" bytes are all zeros
        memset(buffer, 0, SFS_BLOCK_SIZE);
        read = fread(buffer, 1, SFS_BLOCK_SIZE, src);
        if (read == 0)
          break;
        fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
        resources->tot_sectors--;
        data_used++;
      } while (read == SFS_BLOCK_SIZE);
      fclose(src);
    }
    
    // sanity check
    if (ida_count >= 1023) {
      printf("We went over...\n");
      break;
    }
  }
  
  // write remaining sectors (as zeros) (Free Space)
  // we still write to end of media and then back track since the
  //  Index Data Area may not start on a block boundary
  int ida_size = ((ida_count * SFS_ENTRY_SIZE) + (SFS_BLOCK_SIZE - 1)) / SFS_BLOCK_SIZE;  // size of IDA in blocks
  printf("\n Writing free space at LBA %" LL64BIT "i (%" LL64BIT "i blocks)", (bit64u) (FTELL(targ) / SFS_BLOCK_SIZE), resources->tot_sectors - ida_size);
  memset(buffer, 0, SFS_BLOCK_SIZE);
  while (resources->tot_sectors) {
    fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
    if ((resources->tot_sectors % 2500) == 0)
      putch('.');
    resources->tot_sectors--;
  }
  
  // create the Volume ID entry
  struct S_SFS_VOL_ID *id = (struct S_SFS_VOL_ID *) ((bit8u *) ida + (ida_count * SFS_ENTRY_SIZE));
  id->type = SFS_ENTRY_VOL_ID;
  id->time_stamp = get_64kseconds();
  name_len = (int) (strlen(label) + 1); // include the NULL
  cnt = (name_len < VOLID_NAME_LEN) ? name_len : VOLID_NAME_LEN;
  memcpy(id->name, label, cnt);
  id->crc = calc_crc(id, 1 * SFS_ENTRY_SIZE);
  ida_count++;
  
  // now back up and write the index data area buffer
  FSEEK(targ, -(ida_count * SFS_ENTRY_SIZE), SEEK_CUR);
  fwrite(ida, ida_count * SFS_ENTRY_SIZE, 1, targ);
  
  // now we need to go back and write the super
  FSEEK(targ, resources->base_lba * SFS_BLOCK_SIZE, SEEK_SET);
  printf("\n Updating Super block...");
  fread(buffer, SFS_BLOCK_SIZE, 1, targ);
  super.data_block_count = data_used;
  super.index_size = ida_count * SFS_ENTRY_SIZE;
  super.resv_blocks = reserved_blocks;
  super.crc = calc_crc(&super.magic_version, 17); // zero byte check sum of super block
  memcpy(&buffer[0x18E], &super, sizeof(struct S_SFS_SUPER));
  FSEEK(targ, resources->base_lba * SFS_BLOCK_SIZE, SEEK_SET);
  fwrite(buffer, SFS_BLOCK_SIZE, 1, targ);
  
  // close the file
  fclose(targ);
  
  // free the index and resources buffer
  free(ida);
  free(resources);
  
  // return good
  return 0;
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /V         - Volume Name
 */
void parse_command(int argc, char *argv[], char *filename, char *label) {
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if ((memcmp(s, "v:", 2) == 0) ||
          (memcmp(s, "V:", 2) == 0)) {
        strncpy(label, s + 2, NAME_LEN_MAX - 1);
        label[NAME_LEN_MAX-1] = 0;  // make sure null terminated
      } else
        printf("\n Unknown switch parameter: /%s", s);
    } else
      strcpy(filename, s);
  }
}

// time(NULL) returns seconds since 1 Jan 1970
// days from 1 Jan 1970 to 1 Jan 1980 = (365 * 10) + 2
// seconds per day = (60 * 60 * 24)
bit64s get_64kseconds() {
#ifdef _MSC_VER
  return (bit64s) time(NULL) * (bit64s) 65536;
#elif defined DJGPP
  // DJGPP doesn't have an epoc of 1 Jan 1970 ??????
  return (((bit64s) time(NULL) - (bit64s) (((365 * 10) + 2) * (60 * 60 * 24))) * (bit64s) 65536);
#else
# error "need a good time conversion routine"
#endif
}

bit8u calc_crc(void *ptr, int cnt) {
  bit8u crc = 0, *p = (bit8u *) ptr;
  
  while (cnt--)
    crc += *p++;
  
  return -crc;
}
