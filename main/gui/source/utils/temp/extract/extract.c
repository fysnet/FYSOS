/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
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
 * Last update:  15 June 2016
 *
 * usage:
 *   extract image.bmp 
 *   extract image.bmp > image.c
 * 
 * This utility will take a 24-bit bmp image file and dump
 *   the pixels to the stdout as a C style array for inclusion
 *   within your source code.
 *
 * Assumptions:
 *
 *
 *  - Remember, I didn't write this utility to be complete or robust.
 *    I wrote it to simply extract an image file for use with this book.
 *    Please consider this if you add or modify to this utility.
 *
 *  Thank you for your purchase and interest in my work.
 *
 * compile using gcc
 *  gcc -Os extract.c -o extract.exe -s
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
