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
 *  MSECTCPY.EXE
 *   Copies a file to a file at a specified sector offset.
 *
 *  Assumptions/prerequisites:
 *
 *  Last updated: 20 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os msectcpy.c -o msectcpy.exe -s
 *
 *  Usage:
 *   msectcpy image_file_name.img file_to_copy.bin 0
 *
 *    Places the file_to_copy.bin image into the 'image_file_name.img' at sector 0
 *
 *    The third parameter is Radix aware.  i.e: the code can determine hex, dec, and oct.
 *     012  is oct (leading zero)
 *     123  is dec
 *     0x12 is hex
 */

// don't know which ones are needed or not needed.  I just copied them across from another project. :)
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
