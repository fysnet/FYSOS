/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "string.h"

#include "crc32.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC32
bit32u crc32_table[256]; // CRC lookup table array.
bool crc_valid = FALSE;

void crc32_initialize(void) {
  if (crc_valid)
    return;
  
  memset(crc32_table, 0, sizeof(crc32_table));
  
  // 256 values representing ASCII character codes.
  for (int i=0; i<=0xFF; i++) {
    crc32_table[i] = crc32_reflect(i, 8) << 24;
    
    for (int j=0; j<8; j++)
      crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
    
    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
  
  crc_valid = TRUE;
}

// Reflection is a requirement for the official CRC-32 standard.
//  You can create CRCs without it, but they won't conform to the standard.
bit32u crc32_reflect(bit32u reflect, char ch) {
  bit32u ret = 0;
  
  // Swap bit 0 for bit 7 bit 1 For bit 6, etc....
  for (int i=1; i<(ch + 1); i++) {
    if (reflect & 1)
      ret |= 1 << (ch - i);
    reflect >>= 1;
  }
  
  return ret;
}

bit32u crc32(void *data, bit32u len) {
  bit32u crc = 0xFFFFFFFF;
  crc32_partial(&crc, data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(bit32u *crc, void *ptr, bit32u len) {
  bit8u *data = (bit8u *) ptr;
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}
