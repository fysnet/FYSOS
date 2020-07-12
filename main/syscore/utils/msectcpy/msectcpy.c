/************************************************************************ 
  MSECTCPY  COPY a file to an image    v00.10.00
  Forever Young Software            Benjamin David Lunt

  This utility was desinged for use with Bochs to copy a file 
   to an image.

  Bochs is located at:
    http://bochs.sourceforge.net

  I designed this program to be used for testing my own OS,
   though you are welcome to use it any way you wish.

  Please note that I release it and it's code for others to
   use and do with as they want.  You may copy it, modify it,
   do what ever you want with it as long as you release the
   source code and display this entire comment block in your
   source or documentation file.
   (you may add to this comment block if you so desire)

  Please use at your own risk.  I do not specify that this
   code is correct and unharmful.  No warranty of any kind
   is given for its release.

  I take no blame for what may or may not happen by using
   this code with your purposes.

  'nuff of that!  You may modify this to your liking and if you
   see that it will help others with their use of Bochs, please
   send the revised code to fys@fysnet.net.  I will then
   release it as I have this one.

  You may get the latest and greatest at:
    http://www.fysnet.net/fysos.htm

  Thanks, and thanks to those who contributed to Bochs....

  ********************************************************

  Things to know:
  - the third parameter is Radix aware.  i.e: the code
    can determine hex, dec, and oct.
    012  is oct (leading zero)
    123  is dec
    0x12 is hex

  ********************************************************
  
  To compile using DJGPP:  (http://www.delorie.com/djgpp/)
     gcc -Os msectcpy.c -o msectcpy.exe -s  (DOS .EXE requiring DPMI)

  Compiles as is with MS VC++ 6.x       (Win32 .EXE file)

  Compiles as is with MS QC2.5          (TRUE DOS only)

  ********************************************************

  Usage:
    MSECTCPY image_file_name.img file_to_copy.bin 0

  It places the file_to_copy.bin image into the 'image_file_name.img' 
   at sector 0

************************************************************************/

// don't know which ones are needed or not needed.  I just copied them
//  across from another project. :)
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#include "msectcpy.h"   // our include

#define TRUE    1
#define FALSE   0

FILE *targf, *srcf;

unsigned  char buffer[512];  // a temp buffer

int main(int argc, char *argv[]) {
  
  long slen, tlen, targ_off = 0;
  unsigned long read, total = 0;
  
  // print start string
  printf(strtstr);
  
  // check count of parameters given.
  if (argc != 4) {
    printf(usagestr);
    return 0;
  }
  
  // open target image file
  if ((targf = fopen(argv[1], "r+b")) == NULL) {
    printf("\n Error opening target image file");
    return -1;
  }
  
  // open souce image file
  if ((srcf = fopen(argv[2], "rb")) == NULL) {
    printf("\n Error opening source image file");
    fclose(targf);
    return -2;
  }
  
  // get the sector offset to store the file
  targ_off = strtoul(argv[3], 0, 0) * 512;
  
  // get the length of each file
  fseek(srcf, 0, SEEK_END);
  slen = ftell(srcf);
  fseek(targf, 0, SEEK_END);
  tlen = ftell(targf);
  
  // if the target offset is past the end of file, ask to be sure,
  if ((targ_off + slen) > tlen) {
    printf("\nGiven offset (%i) writes past eof (%i) on target.  Is this Okay? [Y]: ", targ_off, tlen);
    if (toupper(getche()) == 'N') {
      fclose(targf);
      fclose(srcf);
      return -3;
    }
  }
  
  // seek to offset within target, and start at first of source
  fseek(targf, targ_off, SEEK_SET);
  rewind(srcf);
  
  // now do the copy
  do {
    read = fread(buffer, 1, 512, srcf);
    fwrite(buffer, 1, read, targf);
    total += read;
  } while (read > 0);
  
  // close the files
  fclose(targf);
  fclose(srcf);
  
  printf("\nCopied %li bytes (%li sectors) from '%s' to lba %li of '%s'.\n", 
    total, (unsigned long) (total / 512), argv[2], (unsigned long) (targ_off / 512), argv[1]);
  
  return 0;
}
