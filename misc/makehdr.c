/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: loader.c                                                           *
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
* BUILT WITH:   LCC                                                        *
*             https://www.cs.virginia.edu/~lcc-win32/                      *
*          Command line: lcc makehdr.c<enter>                              *
*                        lcclnk makehdr.obj<enter>                         *
*                                                                          *
*   (or any compiler that will make a DOS like app)                        *
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

#pragma pack(1)

#define BUF_SIZE 4096   // must be a multiple of 4 (and >= 1024)
#define TRUE 1
#define FALSE 0

struct HDR {
  unsigned  long id;
  unsigned  long location;
  unsigned  long flags;
  unsigned  long file_crc;
  unsigned  char comp_type;
  unsigned  char hdr_crc;
            char resv[14];
} hdr;

unsigned int calc_crc(unsigned char *, int, unsigned int);


int main(int argc, char *argv[]) {

  FILE *src, *targ;
  unsigned long location = 0x00100000, org_crc = 0, *ptr;
  size_t l;
  unsigned char *p;
  unsigned char crc = 0, halt = 0, ctype = 0, clear = 0, iskernel = 0;
  int i;
  char buf[BUF_SIZE];
  char srcfile[256];
  char targfile[256];
  char orgfile[256];
  
  strcpy(srcfile, "");
  strcpy(targfile, "");
  strcpy(orgfile, "");
  
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
          ctype = (unsigned char) (argv[i][2] - '0');
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
  
  if (!strlen(srcfile)) {
    puts(" Need a filename...\n");
    return -4;
  }
  
  if (!strlen(orgfile)) {
    puts(" Need the filename of the original file for crc checking...\n");
    return -4;
  }
  
  // get the crc of the original file
  if ((src = fopen(orgfile, "rb")) == NULL) {
    printf("\n Could not open %s", orgfile);
    return -1;
  }
  
  fread(buf, 1, 4, src);
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
      memset(buf, 0, 0x400 - 4);
      clear = 0;
    }
    org_crc = calc_crc(buf, l, org_crc);
  } while (l == BUF_SIZE);
  fclose(src);


  if ((src = fopen(srcfile, "r+b")) == NULL) {
    printf("\n Could not open %s", srcfile);
    return -1;
  }
  
  if ((targ = fopen(targfile, "w+b")) == NULL) {
    printf("\n Could not create %s", targfile);
    return -2;
  }
  
  memset(&hdr, 0, sizeof(hdr));
  hdr.id = 0x46595332;
  hdr.location = location;
  hdr.flags = (((halt) ? 1 : 0) << 0) | (((iskernel) ? 1 : 0) << 1);
  hdr.file_crc = org_crc;
  hdr.comp_type = ctype;
  p = (unsigned char *) &hdr;
  for (i=0; i<sizeof(hdr); i++)
    crc = (unsigned char) (crc + p[i]);
  hdr.hdr_crc = -crc;
  
  fwrite(&hdr, sizeof(hdr), 1, targ);
  
  do {
    l = fread(buf, 1, BUF_SIZE, src);
    fwrite(buf, 1, l, targ);
  } while (l == BUF_SIZE);
  
  fclose(src);
  fclose(targ);
  
  remove(srcfile);
  rename(targfile, srcfile);
  
  return 0;
}

#define QUOTIENT  0x04C11DB7

unsigned int calc_crc(unsigned char *data, int len, unsigned int crc) {
  unsigned int  result;
  int           i, j;
  unsigned char octet;
  
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
