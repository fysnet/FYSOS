/*
 *                             Copyright (c) 1984-2020
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
 *  EXTRACT.EXE
 *   This utility will take a 24-bit bmp image file and dump
 *    the pixels to the stdout as a C style array for inclusion
 *    within your source code.
 *
 *  Assumptions/prerequisites:
 *  - .BMP file(s) must use 24-bit pixels
 *
 *  Last updated: 13 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os extract.c -o extract.exe -s
 *
 *  Usage:
 *    extract image.bmp 
 *    extract image.bmp > image.c
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"

#include "extract.h"


int main(int argc, char *argv[]) {
  
  FILE *src;
  struct BMP_FILE_HDR bmp_file_hdr;
  bit8u *data, *p;
  char filename[256], name[256];
  size_t read;
  unsigned int pixels, x;
           int y;
  
  // print start string
  fprintf(stderr, strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename);
  
  if ((src = fopen(filename, "r+b")) == NULL) {
    printf("Error opening file [%s]...\n", filename);
    return -1;
  }
  
  // read in the .BMP file's header
  read = fread(&bmp_file_hdr, 1, sizeof(struct BMP_FILE_HDR), src);
  if (read != sizeof(struct BMP_FILE_HDR)) {
    puts("Error reading .BMP header.");
    fclose(src);
    return -1;
  }
  
  // check to make sure this is a .BMP file and is the
  //  format we are expecting.
  if ((bmp_file_hdr.bmp_hdr.bf_id != 0x4D42) ||
      (bmp_file_hdr.bmp_hdr.bf_resv0 || bmp_file_hdr.bmp_hdr.bf_resv1) ||
      (bmp_file_hdr.bmp_hdr.bf_offbits != 54)) { 
    puts("...Did not find valid BMP header at beginning of file.");
    fclose(src);
    return -1;
  }
  
  if ((bmp_file_hdr.bmp_info.bi_size != 40) ||
      (bmp_file_hdr.bmp_info.bi_planes > 1) ||
      (bmp_file_hdr.bmp_info.bi_compression > 0) ||
      (bmp_file_hdr.bmp_info.bi_bitcount != 24)) {
    puts("...Did not find valid BMP Info header, or BMP doesn't use 24 bits per pixel.");
    fclose(src);
    return -1;
  }
  
  // allocate the memory to hold the image data
  data = (bit8u *) malloc(bmp_file_hdr.bmp_info.bi_sizeimage);
  if (data == NULL) {
    puts("Error allocating memory for image data.");
    fclose(src);
    return -1;
  }
  
  read = fread(data, 1, bmp_file_hdr.bmp_info.bi_sizeimage, src);
  if (read != bmp_file_hdr.bmp_info.bi_sizeimage) {
    puts("Error reading .BMP image data.");
    fclose(src);
    return -1;
  }
  
  // create member name
  // this only works if there is not a space in the file name
  strcpy(name, filename);
  p = (bit8u *) strchr(name, '.');
  if (p) *p = 0;
  pixels = bmp_file_hdr.bmp_info.bi_width * bmp_file_hdr.bmp_info.bi_height;
  
  // print a comment at the first of the dump
  printf("/* pixel extraction of: %s\n"
         " * %i x %i, 24 bits per pixel, %i total pixels\n"
         " */\n"
         "PIXEL %s[%i] = {", 
         filename, bmp_file_hdr.bmp_info.bi_width, bmp_file_hdr.bmp_info.bi_height, pixels, name, pixels);
  
  // dump the pixels, 4 per line
  const unsigned int bytes_per_row = ((bmp_file_hdr.bmp_info.bi_width * 3) + 3) & ~0x3;
  for (y = bmp_file_hdr.bmp_info.bi_height - 1; y > -1; y--) {
    for (x=0; x < bmp_file_hdr.bmp_info.bi_width; x++) {
      p = data + ((y * bytes_per_row) + (x * 3));
      if ((x % 4) == 0)
        printf("\n ");
      pixels--;
      if (pixels > 0)
        printf("GUIRTGB(0,%3i,%3i,%3i), ", /*red*/ p[2], /*grn*/ p[1], /*blu*/ p[0]);
      else
        printf("GUITRGB(0,%3i,%3i,%3i)",   /*red*/ p[2], /*grn*/ p[1], /*blu*/ p[0]);
    }
  }
  puts("\n};");
  
  // free the memory block
  free(data);
  
  // close the file
  fclose(src);
  
  // good return
  return 0;
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
      if (strcmp(s, "?") == 0)
        puts("Usage: make_img filename.txt");
      else
        printf("Unknown switch parameter: /%s\n", s);
    } else
      strcpy(filename, s);
  }
}
