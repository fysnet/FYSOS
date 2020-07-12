/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
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
 *
 * Last update:  22 Aug 2015
 *
 * usage:
 *   mimage filename.txt /#nnnnnn
 * 
 * This utility will create an "empty" file of sectors
 *
 * Assumptions:
 *  - none
 *
 *  Thank you for your purchase and interest in my work.
 *
 * compile using gcc
 *  gcc -Os mimage.c -o mimage.exe -s
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"
#include "mimage.h"

int main(int argc, char *argv[]) {
  
  bit8u buffer[512];
  int i;
  bit32u sectors = 0;
  FILE *targ;
  char filename[65];
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  parse_command(argc, argv, filename, &sectors);
  
  // check that filename
  if (strlen(filename) == 0) {
    printf("\n Must specify a file name...");
    return -1;
  }
  
  // check that sectors is > 0
  if (sectors == 0) {
    printf("\n Sector count must not be zero...");
    return -1;
  }
  
  // insanity check
  if (sectors > 500000) {
    printf("\n Sector count should be less than or equal to 500,000?");
    return -1;
  }
  
  // first try to open for read only
  if ((targ = fopen(filename, "r+b")) != NULL) {
    fclose(targ);
    printf("\n '%s' already exists.  Overwrite? [Y] ", filename);
    i = getch();
    if ((i != 'Y') && (i != 13))
      return -1;
    putch('Y');
  }
  
  // either filename didn's exist, or it did exist and user said okay to overwrite.
  if ((targ = fopen(filename, "w+b")) == NULL) {
    printf("\n Error opening/creating '%s'...", filename);
    return -1;
  }
  
  printf("\n Writing %i sectors\n", sectors);
  memset(buffer, 0, 512);
  while (sectors--) {
    fwrite(buffer, 1, 512, targ);
    if ((sectors % 2500) == 0)
      putch('.');
  }
  
  printf("Done\n");
  
  // close the file
  fclose(targ);
  
  // return good
  return 0;
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /#nnnnn    - Indicate the size in sectors of the file
 */
void parse_command(int argc, char *argv[], char *filename, bit32u *sectors) {
  
  int i;
  const char *s;
  
  strcpy(filename, "");
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if (*s == '#')
        *sectors = strtoul(s+1, NULL, 0);
      else
        printf("\n Unknown switch parameter: /%s", s);
    } else
      strcpy(filename, s);
  }
}
