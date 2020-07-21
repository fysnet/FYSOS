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
 *  PIT.EXE
 *
 *  Assumptions/prerequisites:
 *
 *  Last updated: 20 July 2020
 *
 *  This utility is compiled as a 16-bit DOS .EXE using a 16-bit compiler.
 *  This utility uses 16-bit segmented far pointers where a far pointer is
 *   made up as a 32-bit address of two 16-bit values:  seg:offset
 *
 *  Usage:
 *   PIT
 */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"

void (_cdecl _interrupt _far * _cdecl oldvect)();
volatile int _far fired = 0;

void _far _interrupt wait_for_irq(void) {
  fired = 1;
  _chain_intr(oldvect);
}

int main(int argc, char *argv[]) {
  int marker = 1, dir = 1;
  bit16u init;
  char line[81];
  
  // create our display line
  memset(line, '-', 80);
  line[0] = '[';
  line[77] = ']';
  line[78] = '\r';
  line[79] = '\0';
  
  // save old vector
  oldvect = _dos_getvect(0x08);
  
  // set our vector
  _dos_setvect(0x08, wait_for_irq);
  
  // since this is the timer interrupt and we are in DOS
  //  we need to leave it at 18.2 Hz so that the BIOS time
  //  remains valid.  However, this gives you the idea of
  //  how to set the freqency
  init = 0x0000;
  outpb(0x43, 0x34);                  // select channel 0, access = low/high, mode = 2 (rate generator)
  outpb(0x40, (bit8u) (init & 0xFF)); // low 8 bits first
  outpb(0x40, (bit8u) (init >> 8));   // high 8 bits last
  
  // our main loop
  while(1) {
    if (kbhit()) {
      if (getch() == 27)
        break;      
    }
    if (!fired)
      continue;
    line[marker] = '-';
    if (((dir > 0) && (marker == 76)) ||
        ((dir < 0) && (marker == 1)))
      dir = -dir;
    marker += dir;
    line[marker] = '*';
    printf(line);
    fired = 0;
  }
  
  // restore old vector
  _dos_setvect(0x08, oldvect);
  
  return 0;
}
