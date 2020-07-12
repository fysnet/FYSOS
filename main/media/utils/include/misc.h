
#ifndef MEDIA_MISC
#define MEDIA_MISC

void dump(const void *addr, bit32u size) {
  bit32u offset = 0;
  bit8u *buf = (bit8u *) addr;
  bit8u *temp_buf;
  unsigned i;
  
  while (size) {
    printf("0x%04X  ", offset);
    offset += 16;
    temp_buf = buf;
    for (i=0; (i<16) && (i<size); i++)
      printf("%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    for (; i<16; i++)
      printf("   ");
    printf("   ");
    for (i=0; (i<16) && (i<size); i++) {
      putchar(isprint(*buf) ? *buf : '.');
      buf++;
    }
    printf("\n");
    size -= i;
  }
}

#endif // MEDIA_MISC
