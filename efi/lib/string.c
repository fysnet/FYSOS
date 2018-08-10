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
 *
 * Note:  Since this code uses wide chars (wchar_t), you *MUST* have my modified 
 *        version of SmallerC.  Contact me for more information.
 *        
 */


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

void _memset(void *targ, UINT8 val, UINTN len) {
  bit8u *t = (bit8u *) targ;
  while (len--)
    *t++ = val;
}

void memset(void *targ, UINT8 val, UINTN len) {
#ifdef EFI_1_10_SYSTEM_TABLE_REVISION
  gBS->SetMem(targ, len, val);
#else
  _memset(targ, val, len);
#endif
}

void _memcpy(void *targ, void *src, UINTN len) {
  bit8u *t = (bit8u *) targ;
  bit8u *s = (bit8u *) src;
  if (len > 0) {
    // check to see if target is in the range of src and if so, do a memmove() instead
    if ((t > s) && (t < (s + len))) {
      t += (len - 1);
      s += (len - 1);
      while (len--)
        *t-- = *s++;
    } else {
      while (len--)
        *t++ = *s++;
    }
  }
}

void memcpy(void *targ, void *src, UINTN len) {
#ifdef EFI_1_10_SYSTEM_TABLE_REVISION
  gBS->CopyMem(targ, src, len);
#else
  _memcpy(targ, src, len);
#endif
}

wchar_t *wstrchr(wchar_t *s, int c) {
  while (*s) {
    if (*s == c)
      return s;
    s++;
  }
  
  return NULL;
}

int wstrlen(wchar_t *s) {
  int i = 0;
  while (*s)
    i++, s++;
  
  return i;
}

bit32u wstrsize(wchar_t *s) {
  return (wstrlen(s) + 1) * sizeof(wchar_t);
}
