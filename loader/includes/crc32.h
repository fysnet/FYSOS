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
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */
#ifndef CRC32_H
#define CRC32_H

// CRC32
#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
bit32u crc32(void *, bit32u);
void crc32_partial(bit32u *, void *, bit32u);
bit32u crc32_reflect(bit32u, char);

#endif  // CRC32_H
