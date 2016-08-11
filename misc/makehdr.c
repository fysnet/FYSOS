/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: makehdr.c                                                          *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*  Loader code for the FYS OS version 2.0 operating system.                *
*                                                                          *
* BUILT WITH:                                                              *
*      LCC    https://www.cs.virginia.edu/~lcc-win32/                      *
*          Command line: lcc makehdr.c<enter>                              *
*                        lcclnk makehdr.obj<enter>                         *
*   or                                                                     *
*      DJGPP  http://www.delorie.com/djgpp/                                *
*          Command line: gcc -Os makehdr.c -o makehdr.exe -s<enter>        *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*  - This is a quick and dirty app to add a 32-byte header to a kernel     *
*      system file.                                                        *
*  - This allows a few command line parameters:                            *
*     /l address  (address is in C's 0x00000000 format)                    *
*       this gives a load address to the file so the loader file will      *
*       load it to that address.                                           *
*     /e                                                                   *
*       halt if load error on this file, else continue with next file.     *
*     /x                                                                   *
*       skips the first 0x400 bytes of the file when checking for CRC      *
*     /k                                                                   *
*       mark this file as the kernel file. i.e.: jump to this address      *
*       when the loader is done loading the files.                         *
*     /c type                                                              *
*       specifies the compression type to use to compress the file.        *
*        0 = no compression                                                *
*        1 = bz2                                                           *
*        (note: this app does not compress the file.  the file must        *
*          already be compressed)                                          *
*     /o orginal file name                                                 *
*       this specifies the orginal file (the one before compression)       *
*       so that we can get a crc of the orginal file to place in this      *
*       header.                                                            *
*                                                                          *
*                                                                          *
****************************************************************************
*                                                                          *
* TODO:                                                                    *
*                                                                          *
***************************************************************************/

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

// size of memory operands
typedef   signed  char      bool;
typedef   signed  char      bit8s;
typedef unsigned  char      bit8u;
typedef   signed short      bit16s;
typedef unsigned short      bit16u;
typedef   signed  long      bit32s;
typedef unsigned  long      bit32u;


#pragma pack(1)

#define BUF_SIZE 4096   // must be a multiple of 4 (and >= 1024)
#define TRUE 1
#define FALSE 0

// our 32-byte header we prefix to the kernel file(s)
struct HDR {
  bit32u id;
  bit32u location;
  bit32u flags;
  bit32u file_crc;
  bit8u  comp_type;
  bit8u  hdr_crc;
  bit8u  resv[14];
};

// prototype
bit32u calc_crc(bit8u *, const int, const bit32u);

// main.
int main(int argc, char *argv[]) {
  FILE *src, *targ;
  struct HDR hdr;
  bit32u location = 0x00100000, org_crc = 0, *ptr;
  size_t l;
  bit8u *p;
  bit8u crc = 0, ctype = 0;
  bool halt = FALSE, clear = FALSE, iskernel = FALSE;
  int i;
  char buf[BUF_SIZE];
  char srcfile[256];
  char targfile[256];
  char orgfile[256];
  
  // "clear out" the filenames
  strcpy(srcfile, "");
  strcpy(targfile, "");
  strcpy(orgfile, "");
  
  // go through the command line parameters
  for (i=1; i<argc; i++) {
    if (argv[i][0] == '/') {
      if (argv[i][1] == 'e')
        halt = TRUE;
      else if (argv[i][1] == 'x')
        clear = TRUE;
      else if (argv[i][1] == 'k')
        iskernel = TRUE;
      else if (argv[i][1] == 'c') {
        if (isdigit(argv[i][2]))
          ctype = (bit8u) (argv[i][2] - '0');
        else {
          puts(" Illegal compression type.  0-9 accepted\n");
          return -3;
        }
      } else if (argv[i][1] == 'l') {
        if ((argv[i][2] == '0') && (argv[i][3] == 'x')) {
          p = &argv[i][4];
          while (*p) {
            if (isxdigit(*p)) {
              location <<= 4;
              location |=
                ((*p - '0') <= 9) ? (*p - '0') : ((toupper(*p) - 'A') + 0xA);
            } else {
              printf("\n Unknown digit in Location:  %c", *p);
              return -4;
            }
            p++;
          }
        }
      } else if (argv[i][1] == 'o') {
        strcpy(orgfile, (char *) &argv[i][2]);
      } else {
        printf("\n Unknown parameter %s", argv[i]);
        return -4;
      }
    } else if (isalpha(argv[i][0])) {
      strcpy(srcfile, argv[i]);
      strcpy(targfile, argv[i]);
      strcat(targfile, ".$$$");
    } else {
      printf("\n Unknown parameter %s", argv[i]);
      return -4;
    }
  }
  
  ////////////////////////////////////////////////////////
  // did we specify a source file name?
  if (!strlen(srcfile)) {
    puts(" Need a filename...\n");
    return -4;
  }
  
  ////////////////////////////////////////////////////////
  // we need a filename for the orginial file so we can 
  // calculate the CRC
  if (!strlen(orgfile)) {
    puts(" Need the filename of the original file for crc checking...\n");
    return -4;
  }
  
  ////////////////////////////////////////////////////////
  // get the crc of the original file
  if ((src = fopen(orgfile, "rb")) == NULL) {
    printf("\n Could not open %s", orgfile);
    return -1;
  }
  if (fread(buf, 1, sizeof(bit32u), src) != sizeof(bit32u)) {  // get the first dword
    printf("Could not read from %s\n", orgfile);
    fclose(src);
    return -5;
  }
  org_crc  = buf[0] << 24;
  org_crc |= buf[1] << 16;
  org_crc |= buf[2] <<  8;
  org_crc |= buf[3];
  do {
    l = fread(buf, 1, BUF_SIZE, src);
    // if the clear flag is set, skip the first 0x400 bytes of the
    //  file with the CRC check below
    if (clear) {
      org_crc = 0;
      memset(buf, 0, 0x400 - 4);  // - 4 because we already read the first dword
      clear = FALSE;
    }
    org_crc = calc_crc(buf, l, org_crc);
  } while (l == BUF_SIZE);
  fclose(src);

  ////////////////////////////////////////////////////////
  // open the source file
  if ((src = fopen(srcfile, "r+b")) == NULL) {
    printf("\n Could not open %s", srcfile);
    return -1;
  }
  
  ////////////////////////////////////////////////////////
  // create the target file
  if ((targ = fopen(targfile, "w+b")) == NULL) {
    printf("\n Could not create %s", targfile);
    return -2;
  }
  
  ////////////////////////////////////////////////////////
  // initialize the header
  //  and calcuate the header's crc byte
  memset(&hdr, 0, sizeof(struct HDR));
  hdr.id = 0x46595332;
  hdr.location = location;
  hdr.flags = (((halt) ? 1 : 0) << 0) | (((iskernel) ? 1 : 0) << 1);
  hdr.file_crc = org_crc;
  hdr.comp_type = ctype;
  p = (bit8u *) &hdr;
  for (i=0; i<sizeof(struct HDR); i++)
    crc = (bit8u) (crc + p[i]);
  hdr.hdr_crc = -crc;
  
  ////////////////////////////////////////////////////////
  // write the header
  fwrite(&hdr, 1, sizeof(struct HDR), targ);
  
  ////////////////////////////////////////////////////////
  // copy the remaining file
  // (too bad there wasn't a file prefix write, huh?)
  // (it's easy to add to the end of a file, but not the beginning...)
  do {
    l = fread(buf, 1, BUF_SIZE, src);
    fwrite(buf, 1, l, targ);
  } while (l == BUF_SIZE);
  
  ////////////////////////////////////////////////////////
  // close both files
  fclose(src);
  fclose(targ);
  
  ////////////////////////////////////////////////////////
  // delete the source and rename the target back to the source name
  remove(srcfile);
  rename(targfile, srcfile);
  
  ////////////////////////////////////////////////////////
  // return a successful conversion
  return 0;
}

#define QUOTIENT  0x04C11DB7

////////////////////////////////////////////////////////
// calcuate a 32-bit CRC for the file's data
bit32u calc_crc(bit8u *data, const int len, const bit32u crc) {
  bit32u  result;
  int     i, j;
  bit8u   octet;
  
  result = ~crc;
  for (i=0; i<len; i++) {
    octet = *(data++);
    for (j=0; j<8; j++) {
      if (result & 0x80000000) {
        result = (result << 1) ^ QUOTIENT ^ (octet >> 7);
      } else {
        result = (result << 1) ^ (octet >> 7);
      }
      octet <<= 1;
    }
  }
  
  return ~result;             // The complement of the remainder
}
