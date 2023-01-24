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
 * Last update:  22 Jan 2023
 *
 */

#ifndef FYSOS_CRC32
#define FYSOS_CRC32

#define CRC32_POLYNOMIAL 0x04C11DB7


void crc32_initialize(void);
uint32_t crc32(void *data, size_t len);
void crc32_partial(uint32_t *crc, void *ptr, size_t len);
uint32_t crc32_reflect(uint32_t reflect, char ch);

#endif   // FYSOS_CRC32
