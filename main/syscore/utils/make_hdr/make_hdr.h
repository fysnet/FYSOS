/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
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

#ifndef MAKE_HDR
#define MAKE_HDR

#pragma pack(1)

char strtstr[] = "\nMake HDR   v1.00.00    Forever Young Software 1984-2013\n";


#define BUF_SIZE 4096   // must be a multiple of 4

#define  HALT_ON_ERROR  (1<<0)
#define  IS_KERNEL      (1<<1)

struct HDR {
  bit32u id;
  bit32u location;
  bit8u  comp_type;
  bit8u  flags;
  bit8u  hdr_crc;
  bit32u file_crc;
  bit8u  resv[17];
};


bit8u hdr_crc(void *);


/* *********************************************************************************************************
 * The following is used to calculate the CRC's in the header
 */
#define CRC32_POLYNOMIAL 0x04C11DB7

  void crc32_initialize(void);
bit32u crc32_reflect(bit32u, char);
bit32u crc32_full(void *, bit32u);
  void crc32_partial(bit32u *, bit8u *, bit32u);

#endif // MAKE_HDR
