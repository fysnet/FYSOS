/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2022
 *  
 *  This code is included on the disc that is included with the book
 *   GUI: The Graphic User Interface, and is for that purpose only.  You 
 *   have the right to use it for learning purposes only.  You may not
 *   modify it for redistribution for any other purpose unless you have
 *   written permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *
 * Last update:  27 Jan 2022
 *
 * usage:
 *   conv file.fnt
 * 
 * This utility will take a filename of an old 'FONT' file format and
 *   convert it to the new 'Font' file format.
 *
 * Notes:
 *  - 
 *  
 *  Thank you for your purchase and interest in my work.
 *  
 * compile using gcc
 *  gcc -Os conv.c -o conv.exe -s
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"

#include "conv.h"

int main(int argc, char *argv[]) {
  FILE *src, *targ;
  struct FONT_OLD old;
  struct FONT_NEW new;
  struct FONT_INFO_OLD old_info;
  struct FONT_INFO_NEW new_info;
  
  // make sure both filenames are given
  if (argc != 3) {
    puts("\n"
         " Both a source filename and a target filename are required!\n"
         " \n"
         " usage:\n"
         "  conv oldfile.fnt newfile.fnt");
    return -1;
  }
  
  // make sure the targ filename isn't the same as the source filename
  if (strcmp(argv[1], argv[2]) == 0) {
    puts("Target filename must not be same as source filename!");
    return -1;
  }
  
  // try to open the source file
  if ((src = fopen(argv[1], "rb")) == NULL) {
    printf("Could not open source file: [%s]\n", argv[1]);
    return -2;
  }

  // now make sure the source file is the old format
  // read in 96 bytes and check a few items.
  if (fread(&old, 1, sizeof(struct FONT_OLD), src) != sizeof(struct FONT_OLD)) {
    puts("Did not read header from file!");
    fclose(src);
    return -3;
  }
  
  // check the sig == 'FONT'
  if (memcmp(old.sig, "FONT", 4) != 0) {
    puts("Not a known (FYSOS) FONT file!");
    fclose(src);
    return -4;
  }
  
  // now check a few items just to make sure
  // (Seems like I never implemented these three members anyway)
  // (It's a good thing we are getting rid of them now)
  /*
  if ((old.version != 0x10) ||
      (old.type != 0x00) ||
      (old.type_vers != 0x10)) {
    puts("Not a known (FYSOS) FONT file!");
    printf("0x%02X  0x%02X  0x%02X\n", old.version, old.type, old.type_vers);
    fclose(src);
    return -4;
  }
  */

  // print an info string
  printf(" Found FONT file with %u characters.\n", old.count);
  
  // create the new file.
  // this will overwrite any file that exists.
  if ((targ = fopen(argv[2], "wb")) == NULL) {
    printf("Could not open/create target file: [%s]\n", argv[2]);
    return -5;
  }
  
  // create the new header file from the old
  memset(&new, 0, sizeof(struct FONT_NEW));
  memcpy(new.sig, "Font", 4);
  new.height = old.height;
  new.max_width = old.max_width;
  new.info_start = sizeof(struct FONT_NEW);
  new.start = (bit32u) old.start & 0xFFFF; // make sure no sign extend is done
  new.count = old.count;
  new.datalen = old.datalen;
  new.total_size = sizeof(struct FONT_NEW) + (old.count * sizeof(struct FONT_INFO_NEW)) + old.datalen;
  new.flags = old.flags;
  memcpy(new.name, old.name, 16); // the memset() above clears the remaining 16 bytes.
  
  // write the new header
  puts(" Writing new header...");
  if (fwrite(&new, 1, sizeof(struct FONT_NEW), targ) != sizeof(struct FONT_NEW)) {
    puts("Error writing header to file.");
    fclose(src);
    fclose(targ);
    return -6;
  }
  
  // now loop through the characters and create the new INFO blocks
  // using a 32-bit index ('i'), we won't have a race condition when count == 65535
  printf(" Writing %i INFO blocks...\n", new.count);
  bit32u i, limit = (bit32u) old.count & 0xFFFF;
  for (i=0; i<limit; i++) {  
    if (fread(&old_info, 1, sizeof(struct FONT_INFO_OLD), src) != sizeof(struct FONT_INFO_OLD)) {
      puts("Error reading from source file!");
      fclose(src);
      fclose(targ);
      return -7;
    }
    new_info.index = (bit32u) old_info.index & 0xFFFF;
    new_info.width = old_info.width;
    new_info.deltax = old_info.deltax;
    new_info.deltay = old_info.deltay;
    new_info.deltaw = old_info.deltaw;
    memset(new_info.resv, 0, 4);
    if (fwrite(&new_info, 1, sizeof(struct FONT_INFO_NEW), targ) != sizeof(struct FONT_INFO_NEW)) {
      puts("Error writing to target file!");
      fclose(src);
      fclose(targ);
      return -7;
    }
  }
  
  //
  // from here on, we assume the reads and writes will not error.
  //
  
  // now simply copy the bitmap from the source file to the target file
  // reading 32768 byte chunks at a time
  bit8u *buffer = (bit8u *) malloc(32768);
  if (buffer == NULL) {
    puts(" Error allocating memory!");
    fclose(src);
    fclose(targ);
    return -8;
  }
  size_t total = 0, read = fread(buffer, 1, 32768, src);
  while (read > 0) {
    total += read;
    fwrite(buffer, 1, read, targ);
    read = fread(buffer, 1, 32768, src);
  };
  
  printf(" Wrote %u bytes of bitmap...\n", total);
  
  // done.  Free Buffer and close files.
  free(buffer);
  fclose(src);
  fclose(targ);
  
  return 0;
}
