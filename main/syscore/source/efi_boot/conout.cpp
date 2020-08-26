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
 *  CONOUT.CPP
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

#include "conout.h"
#include "string.h"

void cls(void) {
  gSystemTable->ConOut->ClearScreen(gSystemTable->ConOut);
}

EFI_STATUS cursor(const bool on) {
  return gSystemTable->ConOut->EnableCursor(gSystemTable->ConOut, on);
}

void GetCurrentCursorPos(int *Col, int *Row) {
  *Col = gSystemTable->ConOut->Mode->CursorColumn;
  *Row = gSystemTable->ConOut->Mode->CursorRow;
}

EFI_STATUS SetCurrentCursorPos(const int Col, const int Row) {
  return gSystemTable->ConOut->SetCursorPosition(gSystemTable->ConOut, Col, Row);
}

// save the attribute set
int gBack, gFore;

void GetTextAttribute(int *Back, int *Fore) {
  if (Back) *Back = gBack;
  if (Fore) *Fore = gFore;
}

EFI_STATUS SetTextMode(const UINTN Mode, UINTN *Width, UINTN *Height) {

  // TODO:

  return 0;
}

EFI_STATUS SetTextAttribute(const int Back, const int Fore) {
  gBack = Back;
  gFore = Fore;
  return gSystemTable->ConOut->SetAttribute(gSystemTable->ConOut, Back | Fore);
}

void UpdateTitle(wchar_t *fmt, ...) {
  va_list vargs = (va_list) ((bit8u *) &fmt + sizeof(wchar_t *));
  int Col, Row;
  int Back, Fore;
  
  GetCurrentCursorPos(&Col, &Row);
  SetCurrentCursorPos(0, 0);
  GetTextAttribute(&Back, &Fore);
  vprintf(fmt, vargs);
  SetCurrentCursorPos(Col, Row);
  SetTextAttribute(Back, Fore);
}

#if DO_SER_DEBUG
  #include "serial.h"
  extern struct EFI_SERIAL_IO_PROTOCOL *SerialOut;
#endif

void PutChar(const int ch) {
  wchar_t text[2];
  text[0] = (wchar_t) ch;
  text[1] = 0;
  gSystemTable->ConOut->OutputString(gSystemTable->ConOut, text);
  
#if DO_SER_DEBUG
  UINTN Size = 1;
  if (SerialOut != NULL)
    SerialOut->Write(SerialOut, &Size, (void *) &ch);
#endif
}

int printf(wchar_t *fmt, ...) {
  va_list vargs = (va_list) ((bit8u *) &fmt + sizeof(wchar_t *));
  return vprintf(fmt, vargs);
}

// see http://www.cplusplus.com/reference/cstdio/printf/ for parameters
//
int vprintf(wchar_t *fmt, va_list vargs) {
  wchar_t out[1024];
  int o = 0;
  int c, sign, width, precision, lmodifier;
  bool ljust, alt, lzeroes;
  
  while (c = *fmt++) {
    if (c != '%' || *fmt == '%') {
      out[o++] = c;
      fmt += (c == '%');
      continue;
    }
    if ((c = (bit8u) *fmt++) == '\0')
      return -1;
    
    ljust = alt = lzeroes = FALSE;
    sign = 0;
    for (;;) {
      if (c == '-') {
        ljust = TRUE;
        lzeroes = FALSE;
      } else if (c == '+')
        sign = '+';
      else if (c == ' ') {
        if (!sign)
          sign = ' ';
      } else if (c == '#')
        alt = TRUE;
      else if (c == '0') {
        if (!ljust)
          lzeroes = TRUE;
      } else
        break;
      
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    }
    
    width = -1;
    if (isdigit(c)) {
      width = 0;
      while (isdigit(c)) {
        width = width * 10 + (c - '0'); // TBD??? overflow check???
        if ((c = (bit8u) *fmt++) == '\0')
          return -1;
      }
    } else if (c == '*') {
      width = * (int *) vargs; vargs += sizeof(size_t);
      if (width < 0) {
        ljust = TRUE;
        lzeroes = FALSE;
        width = -width; // TBD??? overflow check???
      }
      if ((c = *fmt++) == '\0')
        return -1;
    }
    
    // set the text attribute
    if (c == '[') {
      if (o > 0) {
        out[o] = 0;
        gSystemTable->ConOut->OutputString(gSystemTable->ConOut, out);
        o = 0;
      }
      const bit32u attr = * (bit32u *) vargs; vargs += sizeof(size_t);
      SetTextAttribute(attr & 0x000000F0, attr & 0x0000000F);
      continue;
    }
    // restore the default text attribute
    if (c == ']') {
      if (o > 0) {
        out[o] = 0;
        gSystemTable->ConOut->OutputString(gSystemTable->ConOut, out);
        o = 0;
      }
      SetTextAttribute(EFI_BACKGROUND_BLACK, EFI_LIGHTGRAY);
      continue;
    }
    
    precision = -1;
    if (c == '.') {
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
      precision = 0;
      lzeroes = FALSE;
      if (isdigit(c)) {
        while (isdigit(c)) {
          precision = precision * 10 + (c - '0'); // TBD??? overflow check???
          if ((c = (bit8u) *fmt++) == '\0')
            return -1;
        }
      }  else if (c == '*') {
        precision = * (int *) vargs; vargs += sizeof(size_t);
        if ((c = *fmt++) == '\0')
          return -1;
      }
    }
    
    lmodifier = 0;
    if (c == 'h') {
      if (*fmt == 'h') {
        fmt++;
        lmodifier = 'H';
      } else
        lmodifier = c;
    } else if (wstrchr(L"jztl", c))
      lmodifier = c;
    if (lmodifier)
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    
    if (c == 'i')
      c = 'd';
    if (!wstrchr(L"douxXcsp", c))
      return -1;
    
    if (c == 'c') {
      int ch = (bit8u) * (int *) vargs; vargs += sizeof(size_t);
      if (!ljust)
        while (width > 1) {
          out[o++] = ' ';
          width--;
        }
      out[o++] = ch;
      
      if (ljust)
        while (width > 1) {
          out[o++] = ' ';
          width--;
        }
      continue;
    } else if (c == 's') {
      int len, i;
      wchar_t *s = * (wchar_t **) vargs; vargs += sizeof(wchar_t *);
      
      if (precision < 0)
        len = wstrlen(s); // TBD??? overflow check???
      else {
        len = 0;
        while (len < precision)
          if (s[len]) len++;
          else        break;
      }
      
      if (!ljust) {
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      }
      
      i = len;
      while (i--)
        out[o++] = *s++;
      
      if (ljust) {
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      }
      continue;
    } else {
      bit64u v, tmp;
      char s[22]; // up to 22 octal digits in 64-bit numbers ?
      char *p = s + sizeof(s);
      unsigned base = (c == 'p') ? 16 : 10;
      char *digits = "0123456789abcdef";
      char *hexpfx = NULL;
      int dcnt;
      int len;
      
#ifdef _WIN64
      v = * (size_t *) vargs;
      vargs += sizeof(size_t);
#else      
      if (lmodifier == 'l') {
        v = * (bit64u *) vargs;
        vargs += sizeof(bit64u);
      } else {
        v = * (size_t *) vargs;
        vargs += sizeof(size_t);
      }
#endif

      if (c == 'o')
        base = 8;
      else if (toupper(c) == 'X') {
        base = 16;
        if (c == 'X')
          digits = "0123456789ABCDEF";
        if (alt && v)
          hexpfx = (c == 'X') ? "0X" : "0x";
      }
      
      if (c != 'd') {
        if (lmodifier == 'H')
          v = (bit8u) v;
        else if (lmodifier == 'h')
          v = (unsigned short) v;
        sign = 0;
      } else {
        if (lmodifier == 'H')
          v = (signed char) v;
        else if (lmodifier == 'h')
          v = (short) v;
        if ((bit64s) v < 0) {
          v = (bit64u) (- (bit64s) v);
          sign = '-';
        }
      }
      
      tmp = v;
      do {
        *--p = digits[tmp % base];
        tmp /= base;
      } while (tmp);
      dcnt = (int) (s + sizeof(s) - p);
      
      if (precision < 0)
        precision = 1;
      else if ((v == 0) && (precision == 0))
        dcnt = 0;
      
      if (alt && (c == 'o'))
        if (((v == 0) && (precision == 0)) || (v && (precision <= dcnt)))
          precision = dcnt + 1;
      
      if (precision < dcnt)
        precision = dcnt;
      
      // width padding:
      // - left/right
      // - spaces/zeroes (zeroes are to appear after sign/base prefix)
      // sign:
      // - '-' if negative
      // - '+' or '-' always
      // - space if non-negative or empty
      // alt:
      // - octal: prefix 0 to conversion if non-zero or empty
      // - hex: prefix "0x"/"0X" to conversion if non-zero
      // precision:
      // - prefix conversion digits with zeroes to precision
      // - special case: 0 with precision=0 results in empty conversion
      // [leading spaces] [sign/hex prefix] [leading zeroes] [(precision-dcnt) zeroes] [dcnt digits] [trailing spaces]
      len = (sign != 0) + (hexpfx != NULL) * 2 + precision;
      
      if (!ljust && !lzeroes)
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      
      if (sign)
        out[o++] = sign;
      else if (hexpfx) {
        out[o++] = hexpfx[0];
        out[o++] = hexpfx[1];
      }
      
      if (!ljust && lzeroes)
        while (width > len) {
          out[o++] = '0';
          width--;
        }
      
      while (precision-- > dcnt)
        out[o++] = '0';
      
      while (dcnt--)
        out[o++] = *p++;
      
      if (ljust)
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      
      continue;
    }
  }
  
  out[o++] = 0;
  gSystemTable->ConOut->OutputString(gSystemTable->ConOut, out);
  return 0;
}

void fdebug(const void *addr, bit32u size) {
  PHYS_ADDRESS offset = (PHYS_ADDRESS) addr;
  bit8u *buf = (bit8u *) addr;
  bit8u *temp_buf;
  unsigned i;
  
  while (size) {
    printf(L" %08X  ", (bit32u) offset);
    offset += 16;
    temp_buf = buf;
    for (i=0; (i<16) && (i<size); i++)
      printf(L"%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    for (; i<16; i++)
      printf(L"   ");
    printf(L"   ");
    for (i=0; (i<16) && (i<size); i++) {
      PutChar(isprint(*buf) ? *buf : '.');
      buf++;
    }
    size -= i;
    printf(L"\r\n");
  }
}
