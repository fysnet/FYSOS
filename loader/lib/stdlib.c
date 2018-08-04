
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
