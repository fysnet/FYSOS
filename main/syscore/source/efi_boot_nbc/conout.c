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
 *  CONOUT.C
 *  This is a helper C source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit only
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.C
 */

typedef char *va_list;

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

void PutChar(const int ch) {
  wchar_t text[2];
  text[0] = (wchar_t) ch;
  text[1] = 0;
  gSystemTable->ConOut->OutputString(gSystemTable->ConOut, text);
}

int printf(wchar_t *fmt, ...) {
  va_list vl = (va_list) ((char *) &fmt + sizeof(wchar_t *));
  
  wchar_t out[1024];
  int o = 0;
  int c, sign, width, precision, lmodifier;
  bool ljust, alt, lzeroes;
  
  while ((c = (bit8u) *fmt++) != '\0') {
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
      width = *(int *) vl; vl += sizeof(int);
      if (width < 0) {
        ljust = TRUE;
        lzeroes = FALSE;
        width = -width; // TBD??? overflow check???
      }
      if ((c = *fmt++) == '\0')
        return -1;
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
        precision = *(int *) vl; vl += sizeof(int);
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
    } else if (wstrchr(L"jzt", c))
      lmodifier = c;
    if (lmodifier)
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    
    if (c == 'i')
      c = 'd';
    if (!wstrchr(L"douxXcsp", c))
      return -1;
    
    if (c == 'c') {
      int ch = (bit8u) *(int *) vl; vl += sizeof(int);
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
      wchar_t *s = *(wchar_t **) vl; vl += sizeof(wchar_t *);
      
      if (precision < 0)
        len = wstrlen(s); // TBD??? overflow check???
      else {
        len = 0;
        while (len < precision)
          if (s[len]) len++;
          else        break;
      }
      
      if (!ljust)
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      
      i = len;
      while (i--)
        out[o++] = *s++;
       
      if (ljust)
        while (width > len) {
          out[o++] = ' ';
          width--;
        }
      continue;
    } else {
      unsigned v = *(unsigned *) vl, tmp;
      char s[11]; // up to 11 octal digits in 32-bit numbers
      char *p = s + sizeof(s);
      unsigned base = (c == 'p') ? 16 : 10;
      char *digits = "0123456789abcdef";
      char *hexpfx = NULL;
      int dcnt;
      int len;
      vl += sizeof(unsigned);
      
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
        if ((int) v < 0) {
          v = -v;
          sign = '-';
        }
      }
      
      tmp = v;
      do {
        *--p = digits[tmp % base];
        tmp /= base;
      } while (tmp);
      dcnt = s + sizeof(s) - p;
      
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
