/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  STRING.CPP
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.CPP
 */

#include "ctype.h"
#include "efi_common.h"

// we must turn off optimizations here or the compiler will
//  call memset() within this function which will then create
//  an infinate loop.
#pragma optimize( "", off )
void _memset(void *targ, UINT8 val, UINTN len) {
  bit8u *t = (bit8u *) targ;
  while (len--)
    *t++ = val;
}
#pragma optimize( "", on )

void *memset(void *targ, int val, size_t len) {
#ifdef EFI_1_10_SYSTEM_TABLE_REVISION
  gBS->SetMem(targ, len, val);
#else
  _memset(targ, val, len);
#endif
  return targ;
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

int memcmp(void *targ, void *src, UINTN len) {
  const bit8u *t = (const bit8u *) targ;
  const bit8u *s = (const bit8u *) src;

  if (len == 0) return 0;

  while (--len && (*t == *s)) {
    t++;
    s++;
  }
  return (*t < *s) ? -1 :
         (*t > *s) ?  1 : 0;
}

wchar_t *wstrchr(wchar_t *s, int c) {
  while (*s) {
    if (*s == c)
      return s;
    s++;
  }
  
  return NULL;
}

wchar_t *wstrcpy(wchar_t *t, wchar_t *s) {
  wchar_t *targ = t;
  
  while (*t++ = *s++)
    ;
  
  return targ;
}

wchar_t *wstrcat(wchar_t *t, wchar_t *s) {
  wchar_t *targ = t;
  
  while (*t)
    t++;
  
  while (*t++ = *s++)
    ;
  
  return targ;
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
