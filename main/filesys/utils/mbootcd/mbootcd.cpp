/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2020
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 * Last update:  26 April 2019
 *
 * usage:
 *   mbootcd filename.txt
 * 
 * See the included files.txt file for an example of the resource file
 *
 * This utility will take a list of filenames and include those
 *    files in the iso image, creating boot images within
 *
 * Assumptions:
 *
 *
 *  Thank you for your purchase and interest in my work.
 *
 * compile using gcc
 *  gcc -Os mbootcd.c -o mbootcd.exe -s
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

#include "mbootcd.h"

#define CAT_SIZE          2  // Boot Catalog size in sectors (2 sectors = HDR + 126 entries + HDR)

#define PATH_SECT_SIZE    1  // path size in sectors
#define ROOT_SECT_SIZE    1  // root size in sectors
#define TEMP_FILE_SIZE    1  // sectors occupied by temp file

/* Layout of image
 *
 *  reserved                  16 sectors
 *  pvd                        1 sector
 *  boot                       1 sector
 *  svd                        1 sector
 *  term                       1 sector
 *  catalog             CAT_SIZE sector(s)
 *  first bootable image       X sector(s) (As required)
 *  path table    PATH_SECT_SIZE sector(s) (ASCII)
 *  root table    ROOT_SECT_SIZE sector(s) (ASCII)
 *  path table    PATH_SECT_SIZE sector(s) (UTF-16)
 *  root table    ROOT_SECT_SIZE sector(s) (UTF-16)
 *  temp file                  1 sector(s)
 */

#define PVD_SECT                     16   // pvd: first 16 are reserved
#define BVD_SECT          (PVD_SECT + 1)  // boot after pvd
#define SVD_SECT          (BVD_SECT + 1)  // svd
#define TVD_SECT          (SVD_SECT + 1)  // Terminator Descriptor
#define CAT_SECT          (TVD_SECT + 1)  // our boot catalog sector
#define IMG_START  (CAT_SECT + CAT_SIZE)  // our first boot image starts here

#define PVD_PATH_TABLE(x)  (x + 0)
#define PVD_ROOT_LOCAT(x)  (x + 0 + PATH_SECT_SIZE)
#define SVD_PATH_TABLE(x)  (x + 0 + PATH_SECT_SIZE + ROOT_SECT_SIZE)
#define SVD_ROOT_LOCAT(x)  (x + 0 + PATH_SECT_SIZE + ROOT_SECT_SIZE + PATH_SECT_SIZE)
#define TEMP_FILE_LOCAT(x) (x + 0 + PATH_SECT_SIZE + ROOT_SECT_SIZE + PATH_SECT_SIZE + ROOT_SECT_SIZE)

#define SECT_SIZE      2048  // sector size



int main(int argc, char *argv[]) {
  
  struct S_RESOURCE *resources;
    char filename[NAME_LEN_MAX];
    FILE *targ, *src;
   bit8u *buffer;
  //bit32u img_size = 0;  // size of initial image
  bit32u tot_sectors = IMG_START;
     int i;
  unsigned u;
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename);
  
  // now retrieve the resource file's contents
  resources = parse_resource(filename);
  if (!resources || !resources->file_cnt) {
    printf("\n Error with Resource file. '%s'", filename);
    if (resources) free(resources);
    return -1;
  }
  
  // create target file
  if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
    printf("\nError creating target file: '%s'", resources->targ_filename);
    return -1;
  }
  
  // start at the first image placement
  fseek(targ, IMG_START * SECT_SIZE, SEEK_SET);
  
  // allocate the buffer
  // [0] = misc, [1] = pvd, [2] = boot, [3] = term, [4...] = cat (must be last)
  buffer = (bit8u *) malloc(SECT_SIZE * (4 + CAT_SIZE));
  struct PVD  *pvd  = (struct PVD *) &buffer[SECT_SIZE * 1];
  struct BRVD *brvd = (struct BRVD *) &buffer[SECT_SIZE * 2];
  struct TERM *term = (struct TERM *) &buffer[SECT_SIZE * 3];
  struct CAT  *cat  = (struct CAT *) &buffer[SECT_SIZE * 4];
  bit16u crc = 0, *crc_p = (bit16u *) cat;
  
  memset(cat, 0, SECT_SIZE * CAT_SIZE);
  cat->val_entry.id = 1;        // Header ID = 1
  cat->val_entry.platform = 0;  // 80x86
  cat->val_entry.key55 = 0x55;  //
  cat->val_entry.keyAA = 0xAA;  //
  for (i=0; i<16; i++)
    crc += crc_p[i];
  cat->val_entry.crc = -crc;
  
  struct SECT_HEADER *sect_hdr = (struct SECT_HEADER *) ((bit8u *) cat + (32 * 2)); // offset 64
  struct SECT_ENTRY  *sect_entry = (struct SECT_ENTRY  *) ((bit8u *) sect_hdr + 32);
  struct SECT_ENTRY_X *sect_entry_x;
  int entry = 0;
  sect_hdr->id = 0x91;
  sect_hdr->num = 0;
  sect_hdr->platform = 0; // 80x86
  
  size_t read;
  bit32u cur_sect = IMG_START, file_size;
  for (u=0; u<resources->file_cnt; u++) {
    // get the file to write to the image
    if ((src = fopen(resources->files[u].path_filename, "rb")) == NULL) {
      printf("\nError opening %s file.", resources->files[u].path_filename);
      continue;
    }
    
    // get the file's size
    fseek(src, 0, SEEK_END);
    file_size = ftell(src);
    rewind(src);
    
    // we need to create the entry in the boot catalog
    // if this is the first one, put in the initial entry.
    if (u == 0) {
      cat->init_entry.bootable = 0x88;
      cat->init_entry.media = (bit8u) resources->files[u].param;
      cat->init_entry.load_seg = 0; // bios boot start seg for 0x07C0 when this is zero
      cat->init_entry.sys_type = 0;
      cat->init_entry.load_cnt = (bit16u) ((file_size + 511) / 512);  // load the whole file to 0x07C0:0000 (only if no emulation given)
      cat->init_entry.load_rba = cur_sect;
    } else {
      sect_entry[entry].bootable = 0x88;
      sect_entry[entry].media = (bit8u) resources->files[u].param;
      sect_entry[entry].load_seg = 0; // bios boot start seg for 0x07C0 when this is zero
      sect_entry[entry].sys_type = 0; // TODO: Needs to be the SYS_ID byte from the image's MBR
      sect_entry[entry].load_cnt = (bit16u) ((file_size + 511) / 512);  // load the whole file to 0x07C0:0000 (only if no emulation given)
      sect_entry[entry].load_rba = cur_sect;
      sect_entry[entry].criteria = 0;
      
      // here we can put up to 19 bytes of vendor specific data
      // it can be the name of the image to boot, or whatever.
      // if you want to put more than 19, you will need to use
      //  extension entries.
      
      // just for fun, let's put the file name in this area.
      char *p = resources->files[u].path_filename;
      int len = (int) (strlen(p) + 1);  // plus 1 for the null.
      int this_len = (len <= 19) ? len : 19;
      memcpy(sect_entry[entry].vendor, p, this_len);
      len -= this_len;
      p += this_len;
      
      if (len > 0) {
        sect_entry[entry].media |= (1<<5);
        sect_entry_x = (struct SECT_ENTRY_X *) &sect_entry[entry + 1];
        while (len) {
          this_len = (len <= 30) ? len : 30;
          sect_entry_x->id = 0x44;
          sect_entry_x->final = (len > 30) ? 0x20 : 0x00;
          memcpy(sect_entry_x->vendor, p, this_len);
          p += this_len;
          len -= this_len;
          sect_entry_x++;
          entry++;
        }
      }
      
      // increment for next time
      entry++;
      sect_hdr->num++;
    }
    
    printf("\n Writing %s to LBA %i", resources->files[u].path_filename, ftell(targ) / SECT_SIZE);
    do {
      // by clearing the buffer first, we make sure that the "padding" bytes are all zeros
      memset(buffer, 0, SECT_SIZE);
      read = fread(buffer, 1, SECT_SIZE, src);
      if (read == 0)
        break;
      fwrite(buffer, SECT_SIZE, 1, targ);
      cur_sect++;
      tot_sectors++;
    } while (read == SECT_SIZE);
    fclose(src);
    
    // we only allow for X entries
    // ((CAT_SIZE * 2048) / 32) - 2 section headers
    if (entry >= ((CAT_SIZE * 2048) / 32) - 2) {
      printf("\n No more room in Boot Catalog.");
      break;
    }
  }
  
  // If entry > 0, then there was at least one entry in the Section Header
  //   we need to move to the end and create an "empty" Section header
  // If entry == 0, then there was only one entry and it was in the Initial
  //   entry of that Catalog.  Therefore, mark the first Section Header
  //   as empty (sect_hdr == first one from above)
  sect_hdr = (struct SECT_HEADER *) &sect_entry[entry];
  sect_hdr->id = 0x90; // mark current section header as last one
  
  // move back and start the writing of the descriptors
  rewind(targ);
  
  // first 16 sectors are zero'd
  printf("\n Writing 16 sectors to reserved area at LBA %i", ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 0
  memset(buffer, 0, SECT_SIZE);
  for (i=0; i<16; i++)
    fwrite(buffer, SECT_SIZE, 1, targ);
  
  // Include the Root at the end of the image set
  // We do it twice, once for the PVD and once for the SVD
  tot_sectors += (((PATH_SECT_SIZE + ROOT_SECT_SIZE) * 2) + TEMP_FILE_SIZE);
  
  // cur_sect now is the end of all the boot images
  //  We now can go back and write the volume descriptors  
  
  // initialize pvd and write it
  memset(pvd, 0, SECT_SIZE);
  pvd->id = DESC_TYPE_PVD;
  memcpy(pvd->ident, "CD001", 5);
  pvd->ver = 1;
  memcpy(pvd->sys_ident, "                                ", 32); // a-characters
  memcpy(pvd->vol_ident, "FOREVER YOUNG SOFTWARE 1984 2014", 32); // d-characters
  pvd->num_lbas = tot_sectors;
  pvd->num_lbas_b = ENDIAN_32(tot_sectors);
  pvd->set_size = 0x0001;
  pvd->set_size_b = 0x0100;
  pvd->sequ_num = 0x0001;
  pvd->sequ_num_b = 0x0100;
  pvd->lba_size = SECT_SIZE;
  pvd->lba_size_b = ENDIAN_16(SECT_SIZE);
  pvd->path_table_size = PATH_SECT_SIZE * SECT_SIZE;
  pvd->path_table_size_b = ENDIAN_32(PATH_SECT_SIZE * SECT_SIZE);
  pvd->PathL_loc = PVD_PATH_TABLE(cur_sect);
  pvd->PathLO_loc = 0;
  pvd->PathM_loc = ENDIAN_32(PVD_PATH_TABLE(cur_sect));
  pvd->PathMO_loc = 0;
  pvd->root.len = 34;
  pvd->root.e_attrib = 0;
  pvd->root.extent_loc = PVD_ROOT_LOCAT(cur_sect);
  pvd->root.extent_loc_b = ENDIAN_32(PVD_ROOT_LOCAT(cur_sect));
  pvd->root.data_len = ROOT_SECT_SIZE * SECT_SIZE;
  pvd->root.data_len_b = ENDIAN_32(ROOT_SECT_SIZE * SECT_SIZE);
  fill_date(&pvd->root.date);
  pvd->root.flags = 0x02;  // directory
  pvd->root.unit_size = 0;
  pvd->root.gap_size = 0;
  pvd->root.sequ_num = 0x0001;
  pvd->root.sequ_num_b = 0x0100;
  pvd->root.fi_len = 1;
  pvd->root.ident = 0;
  memset(pvd->set_ident, 0x20, 128);
  memset(pvd->pub_ident, 0x20, 128);
  memset(pvd->prep_ident, 0x20, 128);
  memset(pvd->app_ident, 0x20, 128);
  strcpy(pvd->app_ident, "FOREVER YOUNG SOFTWARE  MBOOTCD.EXE   " VERSION_STR);
  memset(pvd->copy_ident, 0x20, 37);
  memset(pvd->abs_ident, 0x20, 37);
  memset(pvd->bib_ident, 0x20, 37);
  fill_e_date(&pvd->vol_date);
  fill_e_date(&pvd->mod_date);
  memset(&pvd->exp_date, '0', 16); pvd->exp_date.gmt_off = 0;
  fill_e_date(&pvd->val_date);
  pvd->struct_ver = 1;
  printf("\n Writing Primary Volume Descriptor at LBA %i", ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 16
  fwrite(pvd, SECT_SIZE, 1, targ);
  
  // Boot Record Volume Descriptor
  memset(brvd, 0, SECT_SIZE);
  brvd->id = DESC_TYPE_BOOT;
  memcpy(brvd->ident, "CD001", 5);
  brvd->ver = 1;
  memcpy(brvd->bsident, "EL TORITO SPECIFICATION", 23);
  brvd->boot_cat = CAT_SECT;
  printf("\n Writing Boot Volume Descriptor at LBA %i", ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 17
  fwrite(brvd, SECT_SIZE, 1, targ);
  
  // initialize svd and write it
  pvd->id = DESC_TYPE_SVD;
  fill_utf16(pvd->sys_ident, 0x20, 32/2);                // a1-characters
  ascii2utf16(pvd->vol_ident, (char *) " FYS 1984 2014  ", 32/2); // d1-characters
  pvd->flags = 0;
  pvd->PathL_loc = SVD_PATH_TABLE(cur_sect);
  pvd->PathM_loc = ENDIAN_32(SVD_PATH_TABLE(cur_sect));
  pvd->root.extent_loc = SVD_ROOT_LOCAT(cur_sect);
  pvd->root.extent_loc_b = ENDIAN_32(SVD_ROOT_LOCAT(cur_sect));
  // escape sequences
  // Level 1     (25)(2F)(40)   '%\@'         
  // Level 2     (25)(2F)(43)   '%\C'         
  // Level 3     (25)(2F)(45)   '%\E' 
  memset(pvd->escape_sequ, 0, 32);
  memcpy(pvd->escape_sequ, "\x25\x2F\x40", 3);
  fill_utf16(pvd->set_ident, 0x20, 128/2);
  fill_utf16(pvd->pub_ident, 0x20, 128/2);
  fill_utf16(pvd->prep_ident, 0x20, 128/2);
  fill_utf16(pvd->app_ident, 0x20, 128/2);
  ascii2utf16(pvd->app_ident, (char *) "FOREVER YOUNG SOFTWARE  MBOOTCD.EXE   " VERSION_STR, 128/2);
  fill_utf16(pvd->copy_ident, 0x20, 37/2);
  fill_utf16(pvd->abs_ident, 0x20, 37/2);
  fill_utf16(pvd->bib_ident, 0x20, 37/2);
  printf("\n Writing Supplementary Volume Descriptor at LBA %i", ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 18
  fwrite(pvd, SECT_SIZE, 1, targ);

  // write Term Volume Descriptor sector
  memset(term, 0, SECT_SIZE);
  term->id = DESC_TYPE_TERM;
  memcpy(term->ident, "CD001", 5);
  term->ver = 1;
  printf("\n Writing Terminate Set Volume Descriptor at LBA %i", ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 19
  fwrite(term, SECT_SIZE, 1, targ);
    
  // write Boot Catalog sector(s)
  printf("\n Writing %i sector(s) of Boot Catalog at LBA %i", CAT_SIZE, ftell(targ) / SECT_SIZE); // just a check to be sure we are at LBA 20
  fwrite(cat, SECT_SIZE, CAT_SIZE, targ);
  
  // Move to end of image for Path Table, Root Table, and file(s)
  fseek(targ, cur_sect * SECT_SIZE, SEEK_SET);
  
  ///////////////////////////////////////////////////////////////////////////////////
  // Write the Path Table and Root Table for the PVD
  //
  // PVD path table
  memset(buffer, 0, SECT_SIZE);
  // only one directory.  The ROOT.
  struct PATH_TAB *path_tab = (struct PATH_TAB *) buffer;
  path_tab->len_di = 1;
  path_tab->ext_attrib = 0;
  path_tab->loc = PVD_ROOT_LOCAT(cur_sect);
  path_tab->parent = 1;
  memset(path_tab->ident, 0, 2);
  printf("\n Writing %i sector(s) of PVD Path Table at LBA %i", PATH_SECT_SIZE, ftell(targ) / SECT_SIZE);
  fwrite(buffer, SECT_SIZE, PATH_SECT_SIZE, targ);
  
  // PVD root table
  memset(buffer, 0, SECT_SIZE);
  // two directory entrys.  One dir, one file (required?)
  struct ROOT *root_tab = (struct ROOT *) buffer;
  root_tab->len = 34;
  root_tab->e_attrib = 0;
  root_tab->extent_loc = PVD_ROOT_LOCAT(cur_sect);
  root_tab->extent_loc_b = ENDIAN_32(PVD_ROOT_LOCAT(cur_sect));
  root_tab->data_len = SECT_SIZE;
  root_tab->data_len_b = ENDIAN_32(SECT_SIZE);
  fill_date(&root_tab->date);
  root_tab->flags = 0x02;
  root_tab->unit_size = 0;
  root_tab->gap_size = 0;
  root_tab->sequ_num = 0;
  root_tab->sequ_num_b = 0;
  root_tab->fi_len = 1;
  root_tab->ident = 0;
  // second entry only requires a difference of a 1 as the ident
  memcpy(root_tab + 1, root_tab, sizeof(struct ROOT));
  root_tab++;
  root_tab->ident = 1;
  // A temp file for good measure
  root_tab++;
  root_tab->len = sizeof(struct ROOT) - 1 + 12;
  root_tab->e_attrib = 0;
  root_tab->extent_loc = TEMP_FILE_LOCAT(cur_sect);
  root_tab->extent_loc_b = ENDIAN_32(TEMP_FILE_LOCAT(cur_sect));
  root_tab->data_len = 46;
  root_tab->data_len_b = ENDIAN_32(46);
  fill_date(&root_tab->date);
  root_tab->flags = 0;
  root_tab->unit_size = 0;
  root_tab->gap_size = 0;
  root_tab->sequ_num = 0;
  root_tab->sequ_num_b = 0;
  root_tab->fi_len = 12;
  memcpy(&root_tab->ident, "README.TXT;1", 12);
  printf("\n Writing %i sector(s) of Root Table at LBA %i", ROOT_SECT_SIZE, ftell(targ) / SECT_SIZE);
  fwrite(buffer, SECT_SIZE, ROOT_SECT_SIZE, targ);
  // make sure to update  pvd->root.data_len  above to match length of data actually stored (if modified here)

  ///////////////////////////////////////////////////////////////////////////////////
  // Write the Path Table and Root Table for the SVD
  //
  // SVD path table
  memset(buffer, 0, SECT_SIZE);
  // only one directory.  The ROOT.
  path_tab = (struct PATH_TAB *) buffer;
  path_tab->len_di = 1;
  path_tab->ext_attrib = 0;
  path_tab->loc = SVD_ROOT_LOCAT(cur_sect);
  path_tab->parent = 1;
  memset(path_tab->ident, 0, 2);
  printf("\n Writing %i sector(s) of SVD Path Table at LBA %i", PATH_SECT_SIZE, ftell(targ) / SECT_SIZE);
  fwrite(buffer, SECT_SIZE, PATH_SECT_SIZE, targ);
  
  // SVD root table
  memset(buffer, 0, SECT_SIZE);
  // two directory entrys.  One dir, one file (required?)
  root_tab = (struct ROOT *) buffer;
  root_tab->len = 34;
  root_tab->e_attrib = 0;
  root_tab->extent_loc = SVD_ROOT_LOCAT(cur_sect);
  root_tab->extent_loc_b = ENDIAN_32(SVD_ROOT_LOCAT(cur_sect));
  root_tab->data_len = SECT_SIZE;
  root_tab->data_len_b = ENDIAN_32(SECT_SIZE);
  fill_date(&root_tab->date);
  root_tab->flags = 0x02;
  root_tab->unit_size = 0;
  root_tab->gap_size = 0;
  root_tab->sequ_num = 0;
  root_tab->sequ_num_b = 0;
  root_tab->fi_len = 1;
  root_tab->ident = 0;
  // second entry only requires a difference of a 1 as the ident
  memcpy(root_tab + 1, root_tab, sizeof(struct ROOT));
  root_tab++;
  root_tab->ident = 1;
  // A temp file for good measure
  root_tab++;
  root_tab->len = sizeof(struct ROOT) - 1 + (12 * 2);
  root_tab->e_attrib = 0;
  root_tab->extent_loc = TEMP_FILE_LOCAT(cur_sect);
  root_tab->extent_loc_b = ENDIAN_32(TEMP_FILE_LOCAT(cur_sect));
  root_tab->data_len = 46;
  root_tab->data_len_b = ENDIAN_32(46);
  fill_date(&root_tab->date);
  root_tab->flags = 0;
  root_tab->unit_size = 0;
  root_tab->gap_size = 0;
  root_tab->sequ_num = 0;
  root_tab->sequ_num_b = 0;
  root_tab->fi_len = (12 * 2);
  ascii2utf16(&root_tab->ident, (char *) "README.TXT;1", 12);
  printf("\n Writing %i sector(s) of Root Table at LBA %i", ROOT_SECT_SIZE, ftell(targ) / SECT_SIZE);
  fwrite(buffer, SECT_SIZE, ROOT_SECT_SIZE, targ);
  // make sure to update  svd->root.data_len  above to match length of data actually stored (if modified here)
  
  // create a file
  memset(buffer, 0, SECT_SIZE);
  memcpy(buffer, "This is a temp file for testing purposes..\015\012\015\012", 46);
  printf("\n Writing %i sector(s) of Temp File at LBA %i", TEMP_FILE_SIZE, ftell(targ) / SECT_SIZE);
  fwrite(buffer, SECT_SIZE, TEMP_FILE_SIZE, targ);
  
  // free memory
  free(buffer);
  
  // close the file
  fclose(targ);
  
  printf("\n Wrote %i sectors (%3.3f Meg) to %s\n", tot_sectors, (float) (tot_sectors * SECT_SIZE) / (float) (1024 * 1024), resources->targ_filename);
  
  return 0x00;
}

void fill_date(struct DIR_DATE *date) {

  struct tm *tmptr;
  time_t lt;

  lt = time(NULL);
  tmptr = localtime(&lt);

  date->since_1900 = tmptr->tm_year;
  date->month      = tmptr->tm_mon;
  date->day        = tmptr->tm_mday;
  date->hour       = tmptr->tm_hour;
  date->min        = tmptr->tm_min;
  date->sec        = tmptr->tm_sec;
  date->gmt_off    = (signed char) 0x00;
  
  return;
}

void fill_e_date(struct VOL_DATE *date) {
  
  struct tm *tmptr;
  time_t lt;
  
  lt = time(NULL);
  tmptr = localtime(&lt);
  
  sprintf(date->year, "%04i", tmptr->tm_year + 1900);
  sprintf(date->month, "%02i", tmptr->tm_mon + 1);
  sprintf(date->day, "%02i", tmptr->tm_mday);
  sprintf(date->hour, "%02i", tmptr->tm_hour);
  sprintf(date->min, "%02i", tmptr->tm_min);
  sprintf(date->sec, "%02i", tmptr->tm_sec);
  sprintf(date->sec100, "%02i", 0);
  date->gmt_off = 0;
}

// converts char string in src to a UTF-16 wide char string, padding with 0x20 0x00 if needed.
//  Does this in BIG ENDIAN format for target
void ascii2utf16(void *targ, char *src, int len) {
  
  bit16u *t = (bit16u *) targ;
  
  while (*src && len) {
    *t++ = ENDIAN_16(*src);
    src++;
    len--;
  }
  
  while (len--)
    *t++ = ENDIAN_16(0x20);
}

// fills a UTF-16 wide char string with ch
//  Does this in BIG ENDIAN format for target
void fill_utf16(void *targ, bit16u ch, int len) {
  
  bit16u *t = (bit16u *) targ;
  
  while (len--)
    *t++ = ENDIAN_16(ch);
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 */
void parse_command(int argc, char *argv[], char *filename) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if (strcmp(s, "?") == 0) {
        printf("\n Usage:"
               "\n  MBOOTCD resource.txt"
               "\n");
      } else
        printf("\n Unknown switch parameter: /%s", s);
    } else
      strcpy(filename, s);
  }
}
