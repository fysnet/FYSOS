/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The System Core, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 * 
 * File Version: 1.00.00
 * Last mod: 23 June 2013
 *
 * This code is designed to add the 32-byte FYSOS System File header
 *  to a file.
 * 
 * To use this code, type the following at the command prompt:
 * 
 *   make_hdr target.sys source.sys /l0x00600000 /c0 /e /y
 * 
 * The 'target.sys' parameter is the filename of the target file.
 *  This is the resultant file after the header is added.
 * The 'source.sys' parameter is the source filename of the file
 *  you want to add the header to
 * If you don't include the 'source.sys' filename parameter, this
 *  code will create a temp file as the target file.  Then when
 *  complete, will delete the source file and rename the temp file
 *  to the source file's filename.
 * The /l parameter indicates what physical address the loader will
 *  place the file
 * The /c parameter indicates what type of compression the file has.
 *  0 = none. No compression. File is byte for byte the file to load
 *            to memory.
 *  1 = Bz2.  This compression is explained at:
 *              http://en.wikipedia.org/wiki/Bzip2
 *  2-9 = not implemented yet.
 * (Please note that the file has already been compressed. This code
 *  does not compress the file. It assumes the file already has been)
 * The /e parameter indicates to set the "Halt on Error" flag in the
 *  header of this file.
 * The /k parameter indicates to set the "This is the Kernel" flag in the
 *  header of this file.
 * The /y parameter indicates to not prompt for [Y].
 *
 * This code assumes the following:
 * - You are running in a TRUE DOS environment, with a DPMI
 * - You have enough memory accessable to your program
 *
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"
#include "make_hdr.h"


  FILE *src, *targ;
struct HDR hdr;
bit32u location = 0x00100000, crc32;
size_t len;
 bit8u halt = 0, ctype = 0, kernel = 0, prompt = 1;
   int i;
  char *f, buf[BUF_SIZE];
  char srcfile[256] = { 0, };
  char targfile[256] = { 0, };
  bool same_name = FALSE;
  char ctypestr[10][10] = { "None", "Bz2", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown" };

int main(int argc, char *argv[]) {
  
  // print start string
  printf(strtstr);
  
  // initialize the CRC code
  crc32_initialize();
  
  f = targfile;
  for (i=1; i<argc; i++) {
    if (argv[i][0] == '/') {
      // if /e given, set the halt on error flag in the header
      if (argv[i][1] == 'e')
        halt = TRUE;
      // if /k given, set the kernel flag in the header
      else if (argv[i][1] == 'k')
        kernel = TRUE;
      // if /y given, don't prompt for [Y]
      else if (argv[i][1] == 'y')
        prompt = FALSE;
      //  /cX where X = 0-9. compression type.
      else if (argv[i][1] == 'c') {
        if (isdigit(argv[i][2]))
          ctype = (bit8u) (argv[i][2] - '0');
        else {
          printf("\n Illegal compression type.  0-9 accepted");
          return -3;
        }
      // /l0x00000000 = 32-bit physical location to store file
      } else if (argv[i][1] == 'l') {
        location = strtoul(&argv[i][2], NULL, 0);
      } else {
        printf("\n Unknown parameter %s", argv[i]);
        return -4;
      }
    } else {
      strcpy(f, argv[i]);
      f = srcfile;
    }
  }
  
  //////////////////////////////////////////////////////////
  // check that at least targfile was given
  if (!strlen(targfile)) {
    printf("\n Error in filename(s)...");
    return -4;
  }
  
  //////////////////////////////////////////////////////////
  // Make sure ctype is 0-9
  if (ctype > 9) {
    printf("\n Compression Type must be 0 through 9.");
    fclose(src);
    return -1;
  }
  
  //////////////////////////////////////////////////////////
  // if the two filenames are the same, or srcfile not given
  //  then set 'same_name' and change targname to temp filename
  // (for now, keep both as same name, change to temp after display below)
  if (!strlen(srcfile) || !strcmp(srcfile, targfile)) {
    same_name = TRUE;
    strcpy(srcfile, targfile);
  }
  
  //////////////////////////////////////////////////////////
  // Print information and ask for verification
  printf("\n Target File: %s%c"
         "\n Source File: %s"
         "\n   Physical Address = 0x%08X"
         "\n   Compression Type = %i (%s)"
         "\n          Is Kernel = %s"
         "\n Halt on Error flag = %s",
           targfile, (same_name) ? '*' : ' ', srcfile, location, 
           ctype, ctypestr[ctype], (kernel) ? "Yes" : "No",
           (halt) ? "Yes" : "No");
  printf("\n Continue? [Y] ");
  if (prompt) {
    gets(buf);
    if (!strlen(buf))
      strcpy(buf, "YES");
    if (strcmp(buf, "Y") && strcmp(buf, "Yes") && strcmp(buf, "YES"))
      return -1;
  }
  
  //////////////////////////////////////////////////////////
  // if same_name == TRUE, find a filename that does not exist
  //  in the current directory.
  // We can not use
  //   targfile = tmpnam(targfile);
  // since it is not guarenteed to create that temp filename
  //  in the current directory.
  // We can not use
  //   targ = tempfile();
  // since at fclose(targ) time, the file is deleted.
  if (same_name) {
    i = 0;
    do {
      sprintf(targfile, "FYSOS%03i.$$$", i++);
      if ((targ = fopen(targfile, "r+b")) != NULL)
        fclose(targ);
    } while (targ && (i <= 999));    
    
    //////////////////////////////////////////////////////////
    // on the rare case that i > 999, exit with error
    if (i >= 999) {
      printf("\n Could create temp filename.");
      return -2;
    }
  }
  
  //////////////////////////////////////////////////////////
  // Try to open the source file.
  // We must do this before the target file, or the 
  //  open(targfile) below will truncate the file if 
  //  we don't find the source file.
  if ((src = fopen(srcfile, "rb")) == NULL) {
    printf("\n Could not open '%s'", srcfile);
    return -1;
  }
  
  //////////////////////////////////////////////////////////
  // if same_name == TRUE, we found a filename to use.
  //    same_name == FALSE, we use name given.
  if ((targ = fopen(targfile, "w+b")) == NULL) {
    printf("\n Could not create %s", targfile);
    return -2;
  }
  
  //////////////////////////////////////////////////////////
  // write a dummy header (just to skip over)
  // we will return and write the valid one later
  fwrite(buf, sizeof(hdr), 1, targ);
  
  //////////////////////////////////////////////////////////
  // if 'same_name' used and file already has a header,
  //  then skip over the header.
  if (same_name) {
    struct HDR temp;
    len = fread(&temp, 1, sizeof(hdr), src);
    if ((len != sizeof(hdr)) || (temp.id != 0x46595332) || (hdr_crc(&temp) != 0))
      rewind(src);
  }
  
  //////////////////////////////////////////////////////////
  // copy the file, byte for byte
  crc32 = 0;
  do {
    len = fread(buf, 1, BUF_SIZE, src);
    crc32_partial(&crc32, (bit8u *) buf, len);
    fwrite(buf, 1, len, targ);
  } while ((len == BUF_SIZE) && !feof(src));
  
  //////////////////////////////////////////////////////////
  // rewind the target and write the hdr
  memset(&hdr, 0, sizeof(hdr));
  hdr.id = 0x46595332;
  hdr.file_crc = crc32;
  hdr.location = location;
  hdr.comp_type = ctype;
  hdr.flags = (halt) ? HALT_ON_ERROR : 0;
  hdr.flags |= (kernel) ? IS_KERNEL : 0;
  hdr.hdr_crc = -hdr_crc(&hdr);
  
  rewind(targ);
  fwrite(&hdr, sizeof(hdr), 1, targ);
  
  //////////////////////////////////////////////////////////
  // close both files
  fclose(src);
  fclose(targ);
  
  //////////////////////////////////////////////////////////
  // if smae_name == TRUE, delete srcfile and rename target
  //  file to srcfile
  if (same_name) {
    remove(srcfile);
    rename(targfile, srcfile);
  }
  
  // done
  return 0;
}

// calculate CRC for HDR
bit8u hdr_crc(void *ptr) {
  int i;
  bit8u *p = (bit8u *) ptr, crc = 0;
  
  for (i=0; i<sizeof(hdr); i++)
    crc = (bit8u) (crc + p[i]);
  return crc;
}

/* *********************************************************************************************************
 * The following is the code used to calculate the CRC's in the Header
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

bit32u crc32_full(void *data, bit32u len) {
  bit32u crc = 0xFFFFFFFF;
  crc32_partial(&crc, (bit8u *) data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(bit32u *crc, bit8u *data, bit32u len) {
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}
