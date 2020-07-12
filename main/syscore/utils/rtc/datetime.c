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
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"

bit8u cmos_read(const bit8u reg) {
  outpb(0x70, reg);
  return inpb(0x71);
}

void cmos_write(const bit8u reg, const bit8u val) {
  outpb(0x70, reg);
  outpb(0x70, val);
}

// wait until the "update in progress" flag goes from 1 to 0
// NOTE: this could take up to 1 second to complete.
void wait_for_update(void) {
  // wait for it to go to 1 (could already be 1)
  while ((cmos_read(0x0A) & 0x80) == 0)
    ;
  // wait for it to go to 0
  while ((cmos_read(0x0A) & 0x80) != 0)
    ;
}

int bcd_to_bin(const bit8u val) {
  return ((val & 0xF0) >> 1) + ((val & 0xF0) >> 3) + (val & 0x0F);
}

int year_to_century(int year) {
  return ((year < 90) ? 2000 : 1900) + year;
}

int main(int argc, char *argv[]) {
  int month, day, year;
  int hour, min, sec;
  char ampm;  // 'a' or 'p'
  
  // print start string
  printf("\nDateTime  v00.10.00 (C)opyright   Forever Young Software 1984-2017\n");
  
  // in binary mode?
  if (cmos_read(0x0B) & (1<<2)) {
    wait_for_update();
    month = cmos_read(0x08);
    day = cmos_read(0x07);
    year = cmos_read(0x09);
    hour = (cmos_read(0x04) & 0x7F);
    min = cmos_read(0x02);
    sec = cmos_read(0x00);
  } else {
    wait_for_update();
    month = bcd_to_bin(cmos_read(0x08));
    day = bcd_to_bin(cmos_read(0x07));
    year = year_to_century(bcd_to_bin(cmos_read(0x09)));
    hour = bcd_to_bin((bit8u) (cmos_read(0x04) & 0x7F));
    min = bcd_to_bin(cmos_read(0x02));
    sec = bcd_to_bin(cmos_read(0x00));
  }
  
  // if in 24-hour mode, change 'hour' to 12-hour mode
  if (cmos_read(0x0B) & (1<<1)) {
    if (hour >= 12) {
      ampm = 'p';
      hour -= 12;
    } else
      ampm = 'a';
    if (hour == 0)
      hour = 12;
  } else {
    // bit 7 = pm
    if (cmos_read(0x04) & (1<<7))
      ampm = 'p';
    else
      ampm = 'a';
  }
  
  printf("Current Date: %i/%i/%i\n", month, day, year);
  printf("Current Time: %i:%02i.%i%c\n", hour, min, sec, ampm);
  
  return 0;
}
