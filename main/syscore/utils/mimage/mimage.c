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
 *  MIMAGE.EXE
 *   This utility will create an "empty" file of sectors
 *
 *  Assumptions/prerequisites:
 *  - none
 *
 *  Last updated: 20 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os mimage.c -o mimage.exe -s
 *
 *  Usage:
 *   mimage filename.txt /#nnnnnn
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
