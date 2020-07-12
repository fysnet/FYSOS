/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Input and Output Devices, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

/*
 * Shared with sblaster.c
 */

#ifndef FYSOS_BITS
#define FYSOS_BITS

// returns number of set bits in byte
int bitcount(bit8u byte) {
  int i, count = 0;
  
  for (i=0; i<8; i++)
    if (byte & (1<<i))
      count++;
  
  return count;
}

// returns position of the lowest set bit in byte, or -1 if none set
int bitpos(bit8u byte) {
  int i;
  
  for (i=0; i<8; i++)
    if (byte & (1<<i))
      return i;
  
  return -1;
}

#endif // FYSOS_BITS
