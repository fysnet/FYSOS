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

#include "stdio.h"
#include "windows.h"

bool isprint(char ch) {
  return ((ch > 31) && (ch < 255));
}

void fdebug(const void *addr, bit32u size) {
  bit32u offset = (bit32u) addr;
  bit8u *buf = (bit8u *) addr;
  bit8u *temp_buf;
  unsigned i;
  
  while (size) {
    if (main_win) win_printf(main_win, "\n %08X  ", offset);
    else printf("\n %08X  ", offset);
    offset += 16;
    temp_buf = buf;
    for (i=0; (i<16) && (i<size); i++) {
      if (main_win) win_printf(main_win, "%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
      else printf("%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    }
    for (; i<16; i++) {
      if (main_win) win_printf(main_win, "   ");
      else printf("   ");
    }
    if (main_win) win_printf(main_win, "   ");
    else printf("   ");
    for (i=0; (i<16) && (i<size); i++) {
      if (main_win) win_printf(main_win, "%c", isprint(*buf) ? *buf : '.');
      else putchar(isprint(*buf) ? *buf : '.');
      buf++;
    }
    size -= i;
  }
}
