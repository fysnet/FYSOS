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

#include "ctype.h"

#include "string.h"

char *strchr(const char *s, int c) {
  do {
    if (*s == (char) c)
      return (char *) s;
  } while (*s++);
  
  return NULL;
}

unsigned strlen(const char *str) {
  unsigned ret = 0;
  
  while (str[ret])
    ret++;
  
  return ret;
}

char *strcpy(char *targ, const char *src) {
  char *t = targ;
  
  while (*t++ = *src++) ;
  
  return targ;
}

char *strncpy(char *targ, const char *src, unsigned n) {
  char *t = targ;
  
  // should not do "n--" in the conditional as n is unsigned and we have
  // to check it again for > 0 in the next loop below, so we must not risk underflow.
  while ((n > 0) && (*targ++ = *src++))
    --n;
  
  // Checking against 1 as we missed the last --n in the loop above.
  while (n-- > 1)
    *targ++ = '\0';
  
  return t;
}

char *strcat(char *targ, const char *src) {
  char *rc = targ;
  if (*targ)
    while (*++targ)
      ;
  while (*targ++ = *src++)
    ;
  return rc;
}

void *memset(void *targ, bit8u val, int cnt) {
  bit8u *t = (bit8u *) targ;
  
  while (cnt--)
    *t++ = val;
  
  return targ;
}

void *memcpy(void *s1, void *s2, int cnt) {
  bit8u *targ = (bit8u *) s1;
  const bit8u *src = (const bit8u *) s2;
  
  while (cnt--)
    *targ++ = *src++;
  
  return s1;
}

void *memmove(void *s1, const void *s2, int cnt) {
  char *targ = (char *) s1;
  const char *src = (const char *) s2;
  
  if (targ <= src) {
    while (cnt--)
      *targ++ = *src++;
  } else {
    src += cnt;
    targ += cnt;
    while (cnt--)
      *--targ = *--src;
  }
  
  return s1;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2))
     s1++, s2++;
  
  return ((int) (bit8u) *s1) - ((int) (bit8u) *s2);
}

int stricmp(const char *s1, const char *s2) {
  while (*s1 && (toupper(*s1) == toupper(*s2)))
     s1++, s2++;
  
  return ((int) (bit8u) toupper(*s1)) - ((int) (bit8u) toupper(*s2));
}
