/*
 *                             Copyright (c) 1984-2026
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
 *  MKDOSFS.EXE
 *   See the included files.txt file for an example of the resource file
 *
 *   This utility will take a list of filenames and include those
 *    files in the image, creating root entries.
 *
 *
 *  Assumptions/prerequisites:
 *   - this utility assumes that the count of files you add to this image
 *     will fit within the amount of clusters you allocate for the root
 *
 *   - Remember, I didn't write this utility to be complete or robust.
 *     I wrote it to simply make a fat image for use with this book.
 *     Please consider this if you add or modify to this utility.
 *
 *   - Since the FAT FS won't allow file sizes larger than 32-bit, no
 *     need to use the 64-bit forms of FSEEK() and FTELL()
 *
 *  Last updated: 8 Aug 2026
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *  gcc -Os mkdosfs.cpp -o mkdosfs.exe -s
 *
 *  Compile using VS2019
 *   cl mkdosfs.cpp /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /O2 /sdl /Zc:inline /fp:precise 
 *     /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- 
 *     /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:classic
 * 
 *  Usage:
 *    mkdosfs filename.txt /E /v /1
 */

#pragma warning(disable: 4996)  // disable the _s warning for sprintf(), etc.

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <math.h>

#include "..\include\ctype.h"
#include "..\include\misc.h"

#include "mkdosfs.h"   // our include

struct S_FOLDERS *folders = NULL;
unsigned cur_folder = 0;
unsigned cur_folder_size = 0;
bit32u this_pos, cur_clust = 2; // FAT12/16 -> first available cluster, FAT32 -> first ROOT Cluster

struct S_RESOURCE *resources;

FILE *src, *targ;

int main(int argc, char *argv[]) {
  bool existing_image = FALSE;
  bool one_fat = FALSE;
  char filename[NAME_LEN_MAX];
  char label[NAME_LEN_MAX] = "This is a default label";
  char strbuff[16];
  
  unsigned i, j, k, u, spfat, num_fats = 2;
  int    boot_size = 1;  // boot sector size in sectors (default = 1)
  int    root_size;      // default root size in sectors (default = ROOT_SIZE)
  int    last_cnt;
  bit64u tot_sects;
  
  bit8u *buffer;
  bit8u *fat_buf;
  struct S_FAT1216_BPB *bpb;
  struct S_FAT32_BPB *bpb32;
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, &existing_image, &one_fat, label);
  
  if (one_fat) num_fats = 1;
  
  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources) {
    printf(" Error with Resource file. '%s'\n", filename);
    if (resources) free(resources);
    return -1;
  }
  
  // do we need to add sectors to end on a cylinder boundary?
  unsigned cylinders = (unsigned) (resources->tot_sectors + ((16*63)-1)) / (16*63);    // cylinders used
  int add = (int) (((bit64u) cylinders * (16*63)) - resources->tot_sectors - resources->base_lba); // sectors to add to boundary on cylinder
  if ((add > 0) && (resources->tot_sectors > 2880)) {  // don't add if floppy image
    printf(" Total Sectors does not end on cylinder boundary. Expand to %" LL64BIT "i? [Y|N] ", resources->tot_sectors + add);
    if (toupper(getche()) == 'Y') {
      resources->tot_sectors += add;
      // need to calculate again since we added sectors
      cylinders = (bit32u) ((resources->tot_sectors + ((16*63)-1)) / (16*63));
    }
    puts("");
  }
  tot_sects = resources->tot_sectors;
  
  // check a few things
  if ((FAT_TYPE != 12) && (FAT_TYPE != 16) && (FAT_TYPE != 32)) {
    printf(" FAT size must be 12, 16, or 32.\n");
    return -1;
  }
  
  if (!POWERofTWO(SPCLUST) || (SPCLUST > 64)) {
    printf(" Sectors Per Cluster must be a power of 2 and <= 64\n");
    return -1;
  }
  
  if (!existing_image) {
    // create target file
    if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
      printf("Error creating target file: '%s'\n", resources->targ_filename);
      return -1;
    }
  } else {
    // open existing target file
    if ((targ = fopen(resources->targ_filename, "r+b")) == NULL) {
      printf("Error opening target file: '%s'\n", resources->targ_filename);
      return -1;
    }
    fseek(targ, (bit32u) resources->base_lba * SECT_SIZE, SEEK_SET);
  }
  
  buffer = (bit8u *) calloc(SECT_RES32 * SECT_SIZE, 1); // at least SECT_RES32 * SECT_SIZE
  
  // if we are not working on an existing image file, 
  if (!existing_image) {
    // create the MBR and padding sectors
    i = 0;
    if (strlen(resources->mbr_filename)) {
      if ((src = fopen(resources->mbr_filename, "rb")) == NULL) {
        puts("Error opening mbr.bin file.");
        fclose(targ);
        free(buffer);
        return -2;
      }
      fread(buffer, SECT_SIZE, 1, src);
      fclose(src);

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
      if (FAT_TYPE == 12)
        pt->si = 0x01;  // FAT12
      else if (FAT_TYPE == 16) {
        if (resources->tot_sectors <= 62500)
          pt->si = 0x04;
        else
          pt->si = 0x06;
      } else
        pt->si = 0x0B;
      lba_to_chs(&pt->end_chs, (bit32u) ((cylinders * resources->heads * resources->spt)));
      pt->startlba = (bit32u) resources->base_lba;
      pt->size = (bit32u) ((cylinders * resources->heads * resources->spt) - resources->base_lba);
      buffer[510] = 0x55; buffer[511] = 0xAA;
      printf(" Writing MBR to LBA %i\n", ftell(targ) / SECT_SIZE);
      fwrite(buffer, SECT_SIZE, 1, targ);
      memset(buffer, 0, SECT_SIZE);
      i = 1;
    }
    if (i < (bit32u) resources->base_lba)
      puts(" Writing Padding between MBR and Base LBA...");
    for (; i < (int) resources->base_lba; i++)
      fwrite(buffer, SECT_SIZE, 1, targ);
    resources->tot_sectors -= resources->base_lba;
  } // !existing_image
  
  // create the boot sectors (1+)
  memset(buffer, 0, SECT_SIZE);
  boot_size = (FAT_TYPE == 32) ? SECT_RES32 : 1;
  if (strlen(resources->boot_filename)) {
    if ((src = fopen(resources->boot_filename, "rb")) != NULL) {
      // read in the boot sector(s)
      fseek(src, 0, SEEK_END);
      boot_size = (ftell(src) + (SECT_SIZE-1)) / SECT_SIZE;
      if (FAT_TYPE == 32) boot_size = SECT_RES32; // if FAT32, is always 32
      if (boot_size > SECT_RES32) boot_size = SECT_RES32;  // don't let it be more than SECT_RES32
      rewind(src);
      fread(buffer, SECT_SIZE, boot_size, src);
      fclose(src);
    } else
      puts("Error opening boot file. Using Default values.");
  }
  
  switch (FAT_TYPE) {
    case 12:
      if ((resources->tot_sectors / (bit64u) SPCLUST) > 4086L) {
        puts(" *** Illegal Size disk with FAT 12 ***");
        free(buffer);
        return -1;
      }
      spfat = ((unsigned) ((bit32u) resources->tot_sectors * 1.5)) / (SECT_SIZE * SPCLUST);
      if (((spfat * SECT_SIZE) / 1.5) < ((bit32u) resources->tot_sectors / SPCLUST)) spfat++;
      break;
    case 16:
      if ((resources->tot_sectors / (bit64u) SPCLUST) > 65526L) {
        puts(" *** Illegal Size disk with FAT 16 *** ");
        free(buffer);
        return -1;
      }
      spfat = ((bit32u) resources->tot_sectors * 2) / (SECT_SIZE * SPCLUST);
      if (((spfat * SECT_SIZE) / 2) < ((bit32u) resources->tot_sectors / SPCLUST)) spfat++;
      break;
    default:
      spfat = ((bit32u) resources->tot_sectors * 4) / (SECT_SIZE * SPCLUST);
      if (((spfat * SECT_SIZE) / 4) < ((bit32u) resources->tot_sectors / SPCLUST)) spfat++;
  }
  
  // calculate a good root size
  // use the default size, but round up to a multiple of SPC
  // SPCLUST will always be a power of two, so this calculation works just fine.
  root_size = (ROOT_SIZE + (SPCLUST - 1)) & ~(SPCLUST - 1);
  
  bpb = (struct S_FAT1216_BPB *) buffer;
  bpb32 = (struct S_FAT32_BPB *) buffer;
  
  // buffer[] still holds the first sector of the boot code (if any)
  // create BPB/boot block
  switch (FAT_TYPE) {
    case 12:
    case 16:
      bpb->jmps[0] = 0xEB; bpb->jmps[1] = 0x3C;
      bpb->nop = 0x90;
      memcpy(bpb->oemname, "MKDOSFS ", 8);     // OEM name
      bpb->nBytesPerSec = SECT_SIZE;           // Bytes per sector
      bpb->nSecPerClust = SPCLUST;             // Sectors per cluster
      bpb->nSecRes = boot_size;                // Sectors reserved for Boot Record
      bpb->nFATs = num_fats;                   // Number of FATs
      bpb->nRootEnts = (root_size * SECT_SIZE) / 32; // Max Root Directory Entries allowed
      if (resources->tot_sectors < 65536) {     // Number of Logical Sectors
        bpb->nSecs = (unsigned short) resources->tot_sectors;
        bpb->nSecsExt = 0;                     // This value used when there are more
      } else {
        bpb->nSecs = 0;                        // This value used when there are more
        bpb->nSecsExt = (bit32u) resources->tot_sectors;
      }
      bpb->mDesc = media_descriptor(resources->base_lba == 0, resources->heads, resources->spt, cylinders);
      bpb->nSecPerFat = spfat;                 // Sectors per FAT
      bpb->nSecPerTrack = resources->spt;      // Sectors per Track
      bpb->nHeads = resources->heads;          // Number of Heads
      bpb->nSecHidden = (bit32u) resources->base_lba;   // Number of Hidden Sectors
      bpb->DriveNum = 0;                       // Physical drive number
      bpb->nResByte = 0;                       // Reserved (we use for FAT type (12- 16-bit)
      bpb->sig = 0x29;                         // Signature for Extended Boot Record
      bpb->SerNum = fat_build_serial_num();    // Volume Serial Number
      create_label_entry(bpb->VolName, label); // Volume Label
      sprintf(strbuff, "FAT%2i   ", FAT_TYPE);
      memcpy(bpb->FSType, strbuff, 8);         // File system type
      break;
    
    case 32:
      bpb32->jmps[0] = 0xEB; bpb32->jmps[1] = 0x58;
      bpb32->nop = 0x90;
      memcpy(bpb32->oemname, "MKDOSFS ", 8);   // OEM name
      bpb32->nBytesPerSec = SECT_SIZE;         // Bytes per sector
      bpb32->nSecPerClust = SPCLUST;           // Sectors per cluster
      bpb32->nSecRes = SECT_RES32;             // Sectors reserved for Boot Record
      bpb32->nFATs = num_fats;                        
      bpb32->nRootEnts = 0;                    
      bpb32->nSecs = 0;
      bpb32->nSecsExt = (bit32u) resources->tot_sectors;
      bpb32->mDesc = media_descriptor(resources->base_lba == 0, resources->heads, resources->spt, cylinders);
      bpb32->nSecPerFat = 0;
      bpb32->nSecPerTrack = resources->spt;    // Sectors per Track
      bpb32->nHeads = resources->heads;        // Number of Heads
      bpb32->nSecHidden = (bit32u) resources->base_lba; // Number of Hidden Sectors
      bpb32->sect_per_fat32 = spfat; 
      bpb32->DriveNum = 0;                     // Physical drive number
      bpb32->ext_flags = 0x00;
      bpb32->fs_version = 0;
      bpb32->root_base_cluster = cur_clust;
      bpb32->fs_info_sec = FS_INFO_SECT;
      bpb32->backup_boot_sec = FS_BACKUP_SECT;
      bpb32->nResByte = 0;                     // Reserved (we use for FAT type (12- 16-bit)
      bpb32->sig = 0x29;                       // Signature for Extended Boot Record
      bpb32->SerNum = fat_build_serial_num();
      create_label_entry(bpb32->VolName, label); // Volume Label
      memcpy(bpb32->FSType, "FAT32   ", 8);    // File system type
      break;
  }
  
  // update the sig and base lba within the given boot code
  //  this assumes that the coder of the boot code knew that this area was
  //  reserved for this data...
  // this reserved area is the last few bytes of the first sector of the partition/disk
  //  sig       dword  (unique id for partition/disk)
  //  base_lba  qword  (0 for floppy, could be 63 for partitions)
  //  boot sig  word   (0xAA55)
  * (bit32u *) &buffer[496] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
  * (bit64u *) &buffer[500] = resources->base_lba;
  * (bit16u *) &buffer[508] = 0;
  
  // write the first sectors.  We will come back and write the BPB later
  buffer[510] = 0x55; buffer[511] = 0xAA;
  printf(" Writing Boot Sector(s) to LBA %i\n", (ftell(targ) / SECT_SIZE));
  fwrite(buffer, SECT_SIZE, boot_size, targ);
  
  // Calculate the unused cluster entries at the end of the FAT.  We don't do anything
  //  with them in this utility, but this shows how you would calculate them.
  switch (FAT_TYPE) {
  case 12:
    // actual count bytes on last fat sector needed as zeros
    last_cnt = ((int) ((spfat * SECT_SIZE) / 1.5)) - (((bit32u) resources->tot_sectors / SPCLUST) - (bpb->nSecRes + (bpb->nFATs * spfat) + root_size));
    break;
  case 16:
    // actual count bytes on last fat sector needed as zeros
    last_cnt = ((spfat * SECT_SIZE) / 2) - (((bit32u) resources->tot_sectors / SPCLUST) - (bpb->nSecRes + (bpb->nFATs * spfat) + root_size));
    break;
  default:
    // actual count bytes on last fat sector needed as zeros
    last_cnt = ((spfat * SECT_SIZE) / 4) - (((bit32u) resources->tot_sectors / SPCLUST) - (bpb32->nSecRes + (bpb32->nFATs * spfat)));
  }
  
  // if fat32, write the Backup Sector
  if (FAT_TYPE == 32) {
    fseek(targ, (bit32u) ((resources->base_lba + FS_BACKUP_SECT) * SECT_SIZE), SEEK_SET);
    fwrite(buffer, SECT_SIZE, 1, targ);
  }
  
  // create the FAT
  fat_buf = (bit8u *) calloc(spfat * SECT_SIZE, 1);
  switch (FAT_TYPE) {
    case 32:
      j = 8;
      k = ((root_size + (SPCLUST - 1)) / SPCLUST);
      for (i=0; i<k; i++) {
        if (i == (k - 1)) {
          fat_buf[j+3] = 0x0F;
          fat_buf[j+2] = 0xFF;
          fat_buf[j+1] = 0xFF;
          fat_buf[j+0] = 0xFF;
        } else {
          fat_buf[j+3] = (bit8u) (((cur_clust + 1) & 0x0F000000) >> 24);
          fat_buf[j+2] = (bit8u) (((cur_clust + 1) & 0x00FF0000) >> 16);
          fat_buf[j+1] = (bit8u) (((cur_clust + 1) & 0x0000FF00) >>  8);
          fat_buf[j+0] = (bit8u) (((cur_clust + 1) & 0x000000FF) >>  0);
        }
        j += 4;
        cur_clust++;
      }
      
      fat_buf[7] = 0x0F;  // reserved entry
      fat_buf[6] = 0xFF;
      fat_buf[5] = 0xFF;
      fat_buf[4] = 0xFF;
    
      fat_buf[3] = 0x0F;  // reserved entry
      fat_buf[2] = 0xFF;
      fat_buf[1] = 0xFF;
      fat_buf[0] = media_descriptor(resources->base_lba == 0, resources->heads, resources->spt, cylinders);
      break;
    case 16:
      fat_buf[3] = 0xFF;
    case 12:
      fat_buf[2] = 0xFF;
      fat_buf[1] = 0xFF;
      fat_buf[0] = media_descriptor(resources->base_lba == 0, resources->heads, resources->spt, cylinders);
  }
  
  /*  Since we have an empty disk and root, we know that we can start with the
   *  first cluster and write files consecutively.  There is no need to find
   *  free clusters or slots when we create new entries.  Therefore, we will
   *  start with the first cluster after the root (cur_clust) and the first
   *  slot within the root (cur_slot).  We will also keep track of how many
   *  clusters we use so that we can update the fat(s).
   */
  if ((FAT_TYPE == 12) || (FAT_TYPE == 16))
    FSEEK(targ, (bit32u) ((resources->base_lba + boot_size + (num_fats * spfat) + root_size) * SECT_SIZE), SEEK_SET);
  else {
    k = (root_size + (SPCLUST - 1)) & ~(SPCLUST - 1); // must be cluster aligned
    FSEEK(targ, (bit32u) ((resources->base_lba + boot_size + (num_fats * spfat) + k) * SECT_SIZE), SEEK_SET);
  }
  
  // allocate the folders data
  folders = (struct S_FOLDERS *) calloc(DEF_FOLDER_CNT * sizeof(struct S_FOLDERS), 1);
  cur_folder_size = DEF_FOLDER_CNT;
  
  // create the root
  strcpy(folders[0].name, "/");
  folders[0].root = calloc(root_size * SECT_SIZE, 1); 
  folders[0].root_pos = 0;
  folders[0].buf_size = root_size;
  folders[0].start = 2;
  folders[0].parent = NULL;
  ++cur_folder;
  
  /* Create the Volume Label entry.
   *  We would first remove the quotes from the given label, if any
   *  Then we insert it as both a LFN and a SFN
   *  ** Please note that Windows will automatically remove the quotes from
   *     the parameter if found.  I don't know what Unix boxes do.
   */
  create_root_entry(0, label, 0, 0, 0, NULL, FAT_ATTRIB_VOL, FAT_TYPE, 0);
  
  // Now insert the specified files
  size_t read;
  bit32u file_size;
  for (i=0; i<resources->file_cnt; i++) {
    // get the file to write to the image
    if ((src = fopen(resources->files[i].path_filename, "rb")) == NULL) {
      printf("* Error: opening %s\n  Press a key to continue...", resources->files[i].path_filename);
      getch();
      continue;
    }
    FSEEK(src, 0, SEEK_END);
    file_size = (bit32u) FTELL(src);
    rewind(src);

    // create the root entry(s)
    create_root_entry(0, resources->files[i].filename, 0, file_size, cur_clust, fat_buf, FAT_ATTRIB_ARCH, FAT_TYPE, SPCLUST);
    
    printf(" % 2i: Writing %s to LBA %" LL64BIT "i\n", i, resources->files[i].filename, FTELL(targ) / SECT_SIZE);
    do {
      // by clearing the buffer first, we make sure that the "padding" bytes are all zeros
      memset(buffer, 0, SPCLUST * SECT_SIZE);
      read = fread(buffer, 1, SPCLUST * SECT_SIZE, src);
      if (read == 0)
        break;
      fwrite(buffer, SECT_SIZE, SPCLUST, targ);
      tot_sects -= SPCLUST;
      cur_clust++;
    } while (read == (SPCLUST * SECT_SIZE));
    fclose(src);
  }

  // save the position after all of the files have been written
  this_pos = ftell(targ);

  // write the folders
  for (u = 0; u < cur_folder; u++) {
    unsigned start = (unsigned) (resources->base_lba + boot_size + (num_fats * spfat));
    if (u > 0) {
      if ((FAT_TYPE == 12) || (FAT_TYPE == 16))
        start += root_size;
      start += ((folders[u].start - 2) * SPCLUST);
    }
    printf(" Writing folder '%s' to lba %i\n", folders[u].name, start);
    FSEEK(targ, start * SECT_SIZE, SEEK_SET);
    fwrite(folders[u].root, folders[u].buf_size, SECT_SIZE, targ);
    tot_sects -= folders[u].buf_size;
    free(folders[u].root);
  }
  free(folders);
  
  // if fat32, write the info sector
  if (FAT_TYPE == 32) {
    struct S_FAT32_FSINFO fsInfo;
    memset(&fsInfo, 0, SECT_SIZE);
    fsInfo.sig0 = 0x41615252;
    fsInfo.sig1 = 0x61417272;
    fsInfo.free_clust_cnt = (bit32u) ((resources->tot_sectors / SPCLUST) - cur_clust);
    fsInfo.next_free_clust = cur_clust;
    fsInfo.trail_sig = 0xAA550000;
    FSEEK(targ, (bit32u) ((resources->base_lba + FS_INFO_SECT) * SECT_SIZE), SEEK_SET);
    fwrite(&fsInfo, SECT_SIZE, 1, targ);
  }
  
  // adjust the amount of sectors to write 
  tot_sects -= boot_size;
  
  // write the FAT(s)
  FSEEK(targ, (bit32u) (resources->base_lba + boot_size) * SECT_SIZE, SEEK_SET);
  printf(" Writing FAT(s) to LBA %i\n", ftell(targ) / SECT_SIZE);
  for (i=0; i<num_fats; i++)
    fwrite(fat_buf, SECT_SIZE, spfat, targ);
  tot_sects -= (spfat * num_fats);

  // write remaining sectors (as zeros)
  FSEEK(targ, this_pos, SEEK_SET);
  printf(" Writing rest of partition to LBA %" LL64BIT "i, %" LL64BIT "i sectors.", FTELL(targ) / SECT_SIZE, tot_sects);
  memset(buffer, 0, SECT_SIZE);
  while (tot_sects) {
    fwrite(buffer, SECT_SIZE, 1, targ);
    if ((tot_sects % 2500) == 0)
      putch('.');
    tot_sects--;
  }
  puts("");
    
  // free the buffers
  free(buffer);
  free(fat_buf);
  
  // close the file
  fclose(targ);
  
  return 0;
}

/*
  http://support.microsoft.com/?scid=kb%3Ben-us%3B140418&x=22&y=11
  Byte   Capacity   Media Size and Type
  F0     2.88 MB     3.5-inch, 2-sided, 36-sector
  F0     1.44 MB     3.5-inch, 2-sided, 18-sector
  F9      720 KB     3.5-inch, 2-sided,  9-sector
  F9      1.2 MB    5.25-inch, 2-sided, 15-sector
  FD      360 KB    5.25-inch, 2-sided,  9-sector
  FF      320 KB    5.25-inch, 2-sided,  8-sector
  FC      180 KB    5.25-inch, 1-sided,  9-sector
  FE      160 KB    5.25-inch, 1-sided,  8-sector
  F8     -----      Fixed disk
 */
bit8u media_descriptor(const bool floppy, const int heads, const int spt, const int cylinders) {
  if (!floppy)
    return 0xF8;
  
  if ((heads==2) && ((spt==9) || (spt==15)) && (cylinders==80))
    return 0xF9;
  if ((heads==2) && (spt==9) && (cylinders==40)) 
    return 0xFD;
  if ((heads==2) && (spt==8) && (cylinders==40)) 
    return 0xFF;
  if ((heads==1) && (spt==9) && (cylinders==40)) 
    return 0xFC;
  if ((heads==1) && (spt==8) && (cylinders==40)) 
    return 0xFE;
  return 0xF0;
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /E         - Tell this app to use an existing image to write the fs to.
 *  /V         - Volume Name
 *  /1         - only create 1 fat instead of 2
 */
void parse_command(int argc, char *argv[], char *filename, bool *existing_image, bool *one_fat, char *label) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if ((strcmp(s, "E") == 0) ||
          (strcmp(s, "e") == 0))
        *existing_image = TRUE;
      else if (strcmp(s, "1") == 0)
        *one_fat = TRUE;
      else if ((memcmp(s, "v:", 2) == 0) ||
               (memcmp(s, "V:", 2) == 0)) {
        strncpy(label, s + 2, NAME_LEN_MAX - 1);
        label[NAME_LEN_MAX-1] = 0;  // make sure null terminated
      } else
        printf(" Unknown switch parameter: /%s\n", s);
    } else
      strcpy(filename, s);
  }
}

// The following characters are not legal in any bytes of DIR_Name:
//  Values less than 0x20 except for the special case of 0x05 in DIR_Name[0],
//  0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, and 0x7C.
unsigned char fat_bad_chars[] = {
  0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, // '"', '*', '+', '-', '.', '/', ':', ';',
  0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C, // '<', '=', '>', '?', '[', '\', ']', '|'
  0x00
};

bool fat_valid_char(const bit8u ch) {
  return ((ch > 0x20) && !strchr((char *) fat_bad_chars, (int) ch) && (ch < 0x80));
}

// lname = long file name to convert
// name = buffer to hold short name.  Must be atleast 8 bytes in size
// ext = buffer to hold short ext.  Must be atleast 3 bytes in size
void format_name(const bit8u *lname, bit8u *name, bit8u *ext) {
  const bit8u *lp = NULL, *p;
  int i, j;
  
  // find last period
  p = lname;
  while (p = (bit8u *) strchr((char *) p, '.'))
    lp = p++;
  
  p = lname;
  for (j=0; *p && j<8; p++) {
    if (lp && (p == lp))
      break;
    if (fat_valid_char(*p))
      name[j++] = toupper(*p);
  }
  
  // catch an empty name
  if (p == lname)
    memcpy(name, "$$$$$$~1", 8);
  else {  
    if ((j < 8) || (*p && (p < lp))) {
      while (j < 8)
        name[j++] = ' ';
    } else {
      name[6] = '~';
      name[7] = '1';
    }
  }
  
  if (lp) {
    i = 0;
    for (; *lp && i<3; lp++)
      if (fat_valid_char(*lp))
        ext[i++] = toupper(*lp);
    while (i < 3)
      ext[i++] = ' ';
  } else
    memset(ext, ' ', 3);
}

// Creates a new LFN (& SFN) root entry.
void create_root_entry(size_t folder, char *filename, unsigned pos, bit32u file_size, bit32u cur_cluster, bit8u *fat_buf, const bit8u attribute, const int type, const int spc) {
  char name[512], *p;
  bit32u filesize = file_size;
  bool is_folder = FALSE, fnd = FALSE;
  int i;
  unsigned slots, j, k, l, len, index;
  bit8u *s, new_crc;
  bit16u *t;
  char sfn_name[8], sfn_ext[3];
  //char illegal[] = "\"*+,/:;<=>?[\\]| ";
  struct S_FAT_LFN_ROOT *f_root = (struct S_FAT_LFN_ROOT *) folders[folder].root;
  struct S_FAT_ROOT *root = (struct S_FAT_ROOT *) folders[folder].root;
  bit32u dword, value;
  struct tm *tmptr;
  time_t lt;

  // check to see if the file name given is part of a path (i.e.: has directory names first)
  if (p = strchr(filename + pos, '/')) {
    memcpy(name, filename, (p - filename));
    name[(p - filename)] = '\0';
    
    // now see if we have already made this folder's block
    for (j = 0; j < cur_folder; j++) {
      if (strcmp(folders[j].name, name) == 0) {
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
      // j == cur_folder from for() loop above
      strcpy(folders[j].name, name);
      folders[j].buf_size = FOLDER_SIZE * SPCLUST;
      folders[j].root = calloc(FOLDER_SIZE * SPCLUST * SECT_SIZE, 1);
      folders[j].start = cur_clust;
      folders[j].root_pos = root_start((struct S_FAT_ROOT *) folders[j].root, cur_clust, folders[folder].start, type);
      folders[j].parent = &folders[folder];

      // we need to move ahead the size of this directory
      filesize = FOLDER_SIZE * SPCLUST * SECT_SIZE;
      cur_cluster = cur_clust;
      cur_clust += FOLDER_SIZE;
      FSEEK(targ, FOLDER_SIZE * SPCLUST * SECT_SIZE, SEEK_CUR);

      // mark we are a folder
      is_folder = TRUE;
      cur_folder++;
    }
    
    create_root_entry(j, filename, (unsigned) (strlen(name) + 1), file_size, cur_clust, fat_buf, FAT_ATTRIB_ARCH, FAT_TYPE, SPCLUST);
    strcpy(name, name + pos);
  }	else
    strcpy(name, filename + pos);

  // if it was not already found as an existing directory, we need to make the entry
  if (!fnd) {
    lt = time(NULL);
    tmptr = localtime(&lt);
    
    // generate short filename
    format_name((const bit8u *) name, (bit8u *) sfn_name, (bit8u *) sfn_ext);
    
    // calculate crc
    s = (bit8u *) sfn_name;
    new_crc = *s++;
    for (i=1; i<11; i++) {
      if (i==8) s = (bit8u *) sfn_ext;
      new_crc = ror_byte(new_crc);
      new_crc += *s++;
    }
    
    // create the lfn entries
    index = folders[folder].root_pos;
    len = (unsigned) strlen(name);
    slots = ((len + 12) / 13);

    // check to make sure we don't over do it.
    if ((index + slots + 1) >= (folders[folder].buf_size * SECT_SIZE)) {
      puts("Folder is too full...");
      abort();
    }

    k = 0;
    l = 1;
    for (i=slots-1; i >= 0; i--) {
      f_root[index + i].sequ_flags = ((i==0) ? 0x40 : 0x00) | l++;
      f_root[index + i].attrb = 0x0F;
      f_root[index + i].resv = 0x00;
      f_root[index + i].clust_zero = 0x0000;
      f_root[index + i].sfn_crc = new_crc;
      t = f_root[index + i].name0;
      for (j=0; j<13; j++, t++) {
        if (j==5) t = f_root[index + i].name1;
        if (j==11) t = f_root[index + i].name2;
        if (k < len) *t = name[k];
        else if (k == len) *t = 0x0000;
        else *t = 0xFFFF;
        k++;
      }
    }
    index += slots;
    
    // create the SFN slot
    memcpy(root[index].name, sfn_name, 8);
    memcpy(root[index].ext, sfn_ext, 3);
    root[index].attrb = (is_folder) ? FAT_ATTRIB_DIR : attribute;
    root[index].time = fat_time_word(tmptr);
    root[index].date = fat_date_word(tmptr);
    root[index].strtclst = (bit16u) cur_cluster;
    root[index].filesize = (is_folder) ? 0 : filesize;
    if (type != 32)
      memset(root[index].type.resv, 0, 10);
    else {  // FAT32 entries.
      root[index].type.fat32.nt_resv = 0;
      root[index].type.fat32.crt_time_tenth = 0;
      root[index].type.fat32.crt_time = fat_time_word(tmptr);
      root[index].type.fat32.crt_date = fat_date_word(tmptr);
      root[index].type.fat32.last_acc = fat_date_word(tmptr);
      root[index].type.fat32.strtclst32 = (bit16u) (cur_cluster >> 16);
    }
    index++;
    
    // update root position
    folders[folder].root_pos = index;
    
    // update the fat(s)
    if (filesize > 0) {
      i = (filesize + ((SECT_SIZE * spc) - 1)) / (SECT_SIZE * spc);
      while (i--) {
        value = (i == 0) ? 0x0FFFFFFF : (cur_cluster + 1);
        switch (type) {
          case 12:
            s = fat_buf + cur_cluster;
            s += (cur_cluster / 2);
            dword = * ((bit16u *) s);
            if (cur_cluster & 1)
              * ((bit16u *) s) = (bit16u) ((dword & ~0xFFF0) | ((value & 0xFFF)<<4));
            else
              * ((bit16u *) s) = (bit16u) ((dword & ~0x0FFF) | (value & 0xFFF));
            break;
          case 16:
            s = fat_buf + (cur_cluster << 1);
            * ((bit16u *) s) = (bit16u) (value & 0xFFFF);
            break;
          case 32:
            // top 4 bits are reserved and should not be modified
            s = fat_buf + (cur_cluster << 2);
            * ((bit32u *) s) = (value & 0x0FFFFFFF);
        }
        cur_cluster++;
      }
    }
  }
}

// create the first two entries of a root directory
unsigned root_start(struct S_FAT_ROOT *root, const bit32u current, const bit32u parent, const int type) {
  struct tm *tmptr;
  time_t lt;

  lt = time(NULL);
  tmptr = localtime(&lt);

  // The "." entry
  memcpy(root[0].name, ".       ", 8);
  memcpy(root[0].ext, "   ", 3);
  root[0].attrb = FAT_ATTRIB_DIR;
  root[0].time = fat_time_word(tmptr);
  root[0].date = fat_date_word(tmptr);
  root[0].strtclst = (bit16u) current;
  root[0].filesize = 0;
  if (type != 32)
    memset(root[0].type.resv, 0, 10);
  else {  // FAT32 entries.
    root[0].type.fat32.nt_resv = 0;
    root[0].type.fat32.crt_time_tenth = 0;
    root[0].type.fat32.crt_time = fat_time_word(tmptr);
    root[0].type.fat32.crt_date = fat_date_word(tmptr);
    root[0].type.fat32.last_acc = fat_date_word(tmptr);
    root[0].type.fat32.strtclst32 = (bit16u) (current >> 16);
  }

  // The ".." entry
  memcpy(root[1].name, "..      ", 8);
  memcpy(root[1].ext, "   ", 3);
  root[1].attrb = FAT_ATTRIB_DIR;
  root[1].time = fat_time_word(tmptr);
  root[1].date = fat_date_word(tmptr);
  root[1].strtclst = (bit16u) parent;
  root[1].filesize = 0;
  if (type != 32)
    memset(root[1].type.resv, 0, 10);
  else {  // FAT32 entries.
    root[1].type.fat32.nt_resv = 0;
    root[1].type.fat32.crt_time_tenth = 0;
    root[1].type.fat32.crt_time = fat_time_word(tmptr);
    root[1].type.fat32.crt_date = fat_date_word(tmptr);
    root[1].type.fat32.last_acc = fat_date_word(tmptr);
    root[1].type.fat32.strtclst32 = (bit16u) (parent >> 16);
  }

  return 2;
}

// this simply places a space trailing 11 byte label into the BPB style label entry
void create_label_entry(char *targ, const char *label) {
  int i;
  
  memset(targ, 0x20, 11);
  for (i=0; i<11; i++) {
    if (label[i] == 0)
      break;
    targ[i] = label[i];
  }
}

// rotate a byte 1 bit to the right carrying the lsb to msb
bit8u ror_byte(bit8u byte) {
  return (byte >> 1) | (byte << 7);
}

// Calculate and return serial number
// AA = Second
// BB = Hundred / 2
// CC = Month
// DD = Day
// EE = Hour (24 hour clock)
// FF = Min
// GGGG = Year
// Serial = ((AABB + CCDD) << 16) | (EEFF + GGGG))
bit32u fat_build_serial_num() {
  
  struct tm *tmptr;
  time_t lt;
  
  lt = time(NULL);
  tmptr = localtime(&lt);
  
  //                   AA           BB                   CC                     DD
  return ((((tmptr->tm_sec << 8) | (0>>1)) + ((tmptr->tm_mon << 8) | tmptr->tm_mday)) << 16) |
         (((tmptr->tm_hour << 8) | tmptr->tm_min) + (tmptr->tm_year + 1900));
  //                   EE           FF                   GGGG
}

bit16u fat_time_word(struct tm *tmptr) {
  bit16u word;
  
  word = tmptr->tm_hour << 11;
  word |= tmptr->tm_min << 5;
  word |= tmptr->tm_sec;
  
  return word;
}

bit16u fat_date_word(struct tm *tmptr) {
  bit16u word;
  
  word = ((tmptr->tm_year + 1900) - 1980) << 9;
  word |= (tmptr->tm_mon + 1) << 5;
  word |= tmptr->tm_mday;
  
  return word;
}
