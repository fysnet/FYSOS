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

#include "conout.h"
#include "string.h"


typedef unsigned char *va_list;

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

EFI_STATUS SetTextAttribute(const int Back, const int Fore) {
  return gSystemTable->ConOut->SetAttribute(gSystemTable->ConOut, Back | Fore);
}

void PutChar(const int ch) {
  wchar_t text[2];
  text[0] = (wchar_t) ch;
  text[1] = 0;
  gSystemTable->ConOut->OutputString(gSystemTable->ConOut, text);
}

void para_out(wchar_t ch);

// see http://www.cplusplus.com/reference/cstdio/printf/ for parameters
//
int printf(wchar_t *fmt, ...) {
  va_list vl = (va_list) ((bit8u *) &fmt + sizeof(wchar_t *));
  
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
      width = *(int *) vl; vl += sizeof(int);
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
      bit32u attr = *(bit32u *) vl; vl += sizeof(bit32u);
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
      wchar_t *s = * (wchar_t **) vl; vl += sizeof(wchar_t *);
      
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
/*
#define TIMEOUT_VAL  256
void para_out(wchar_t ch) {
  if (ch != 0x09) {
    bit8u val;
    int ii;
    for (ii=0; ii<TIMEOUT_VAL; ii++) {
      val = efi_ioread(0x379, 1);
      if ((val & 0x80) == 0x80) break;
    }
    if (ii == TIMEOUT_VAL) return;
    efi_iowrite(0x378, (bit8u) ch, 1);
    val = efi_ioread(0x37A, 1);
    val = val | 1;
    efi_iowrite(0x37A, val, 1);
    for (ii=0; ii<TIMEOUT_VAL; ii++) ;
    val = efi_ioread(0x37A, 1);
    val = val & 0xFE;
    efi_iowrite(0x37A, val, 1);
    for (ii=0; ii<TIMEOUT_VAL; ii++) {
      val = efi_ioread(0x379, 1);
      if ((val & 0x40) == 0x00) break;
    }
  }
}
*/
