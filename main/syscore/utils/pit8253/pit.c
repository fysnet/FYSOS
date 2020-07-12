/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
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
