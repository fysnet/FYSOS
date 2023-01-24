/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2023
 *  
 *  This code is donated to the Freeware community.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and if distributed, have the same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  23 Jan 2023
 *
 */


#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "checksum.h"

uint32_t crc32_table[256]; // CRC lookup table array.

void crc32_initialize(void) {
  memset(crc32_table, 0, sizeof(crc32_table));
  
  // 256 values representing ASCII character codes.
  for (int i=0; i<=0xFF; i++) {
    crc32_table[i] = crc32_reflect(i, 8) << 24;
    
    for (int j=0; j<8; j++)
      crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
    
    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
}

// Reflection is a requirement for the official CRC-32 standard.
//  You can create CRCs without it, but they won't conform to the standard.
uint32_t crc32_reflect(uint32_t reflect, char ch) {
  uint32_t ret = 0;
  
  // Swap bit 0 for bit 7 bit 1 for bit 6, etc....
  for (int i=1; i<(ch + 1); i++) {
    if (reflect & 1)
      ret |= 1 << (ch - i);
    reflect >>= 1;
  }
  
  return ret;
}

uint32_t crc32(void *data, size_t len) {
  uint32_t crc = 0xFFFFFFFF;
  crc32_partial(&crc, data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(uint32_t *crc, void *ptr, size_t len) {
  uint8_t *data = (uint8_t *) ptr;
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}
