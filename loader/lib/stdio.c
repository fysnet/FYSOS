
#include "ctype.h"
#include "malloc.h"
#include "paraport.h"
#include "string.h"
#include "sys.h"

#include "stdio.h"

// see http://www.cplusplus.com/reference/cstdio/printf/ for parameters
bit16u printf(const char *str, ...) {
  va_list vargs = (va_list) ((char *) &str + sizeof(char *));

#ifdef MEM_DEBUG_ON
  char targ[1024];
#else
  char *targ = (char *) malloc(1024);
#endif
  
  bit16u ret = vsprintf(targ, str, vargs);
  
  // can't call puts because it puts another '\n' at the end of the string.
  // also, we don't need to check for '\r' before '\n' because vsprintf()
  //  already did this for us
  for (bit16u i=0; i<ret; i++)
    putchar(targ[i]);
  
#ifndef MEM_DEBUG_ON
  mfree(targ);
#endif
  
  return ret;
}

bit16u sprintf(char *targ, const char *src, ...) {
  bit16u ret;
  va_list vargs = (va_list) ((char *) &src + sizeof(char *));
  
  ret = vsprintf(targ, src, vargs);
  
  return ret;
}

bit16u vsprintf(char *targ, const char *fmt, va_list vargs) {
  int c, sign, width, precision, lmodifier, cnt = 0;
  bool ljust, alt, lzeroes;
  
  while ((c = (bit8u) *fmt++) != '\0') {
    if (c != '%' || *fmt == '%') {
      *targ++ = c, cnt++;
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
      width = *(int *) vargs; vargs += sizeof(int);
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
        precision = *(int *) vargs; vargs += sizeof(int);
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
    } else if (strchr("jzt", c))
      lmodifier = c;
    if (lmodifier)
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    
    if (c == 'i')
      c = 'd';
    if (!strchr("douxXcsp", c))
      return -1;
    
    if (c == 'c') {
      int ch = (bit8u) *(int *) vargs; vargs += sizeof(int);
      if (!ljust)
        while (width > 1) {
          *targ++ = ' ', cnt++;
          width--;
        }
      *targ++ = ch, cnt++;
      
      if (ljust)
        while (width > 1) {
          *targ++ = ' ', cnt++;
          width--;
        }
      continue;
    } else if (c == 's') {
      int len, i;
      char *s = * (char **) vargs; vargs += sizeof(char *);
      
      if (precision < 0)
        len = strlen(s); // TBD??? overflow check???
      else {
        len = 0;
        while (len < precision)
          if (s[len]) len++;
          else        break;
      }
      
      if (!ljust)
        while (width > len) {
          *targ++ = ' ', cnt++;
          width--;
        }
      
      i = len;
      while (i--)
        *targ++ = *s++, cnt++;
       
      if (ljust)
        while (width > len) {
          *targ++ = ' ', cnt++;
          width--;
        }
      continue;
    } else {
      unsigned v = *(unsigned *) vargs, tmp;
      char s[11]; // up to 11 octal digits in 32-bit numbers
      char *p = s + sizeof(s);
      unsigned base = (c == 'p') ? 16 : 10;
      char *digits = "0123456789abcdef";
      char *hexpfx = NULL;
      int dcnt;
      int len;
      vargs += sizeof(unsigned);
      
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
          *targ++ = ' ', cnt++;
          width--;
        }
      
      if (sign)
        *targ++ = sign, cnt++;
      else if (hexpfx) {
        *targ++ = hexpfx[0], cnt++;
        *targ++ = hexpfx[1], cnt++;
      }
      
      if (!ljust && lzeroes)
        while (width > len) {
          *targ++ = '0', cnt++;
          width--;
        }
      
      while (precision-- > dcnt)
        *targ++ = '0', cnt++;
      
      while (dcnt--)
        *targ++ = *p++, cnt++;
      
      if (ljust)
        while (width > len) {
          *targ++ = ' ', cnt++;
          width--;
        }
      
      continue;
    }
  }
  
  *targ++ = '\0';
  
  return cnt;
}

int putchar(const int ch) {
  struct REGS regs;
  
  if (ch == 10)
    putchar(13);
  
  regs.eax = (0x0E00 | (ch & 0xFF));
  regs.ebx = 0;
  intx(0x10, &regs);
  
  if (spc_key_F2)
    para_putch(ch);
  
  return ch;
}

int puts(const char *s) {
  bit8u ch;
  while (*s != '\0')
    putchar(ch = *s++);
  putchar(10);
  return ch;
}


void set_vector(const int i, const bit32u address) {
  /*
  _asm (" push edi             \n"
        " mov  edi,PARAM0      \n"
        " shl  edi,2           \n"
        " mov  eax,PARAM1      \n"
        " and  ax,0Fh          \n"
        " mov  fs:[edi+0],ax   \n"
        " mov  eax,PARAM1      \n"
        " shr  eax,4           \n"
        " mov  fs:[edi+2],ax   \n"
        " pop  edi             \n");
  */
}

