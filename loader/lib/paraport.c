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
#include "loader.h"

#include "paraport.h"

#include "conio.h"
#include "malloc.h"
#include "stdio.h"
#include "sys.h"

void para_putch(const bit8u ch) {
  bit8u val;
  int i;
  
  if (ch != 0x09) {   // don't send the tabs since we change them to spaces
    for (i=0; i<TIMEOUT_VAL; i++) {
      val = inpb(0x379);
      if ((val & 0x80) == 0x80) break;
    }
    if (i == TIMEOUT_VAL) 
      return;
    outpb(0x378, ch);
    val = inpb(0x37A);
    val = val | 1;
    outpb(0x37A, val);
    for (i=0; i<TIMEOUT_VAL; i++) ;
    val = inpb(0x37A);
    val = val & 0xFE;
    outpb(0x37A, val);
    for (i=0; i<TIMEOUT_VAL; i++) {
      val = inpb(0x379);
      if ((val & 0x40) == 0x00) break;
    }
  }
}

bit16u para_printf(const char *str, ...) {
  va_list vargs = (va_list) ((char *) &str + sizeof(char *));

  char *targ = (char *) malloc(1024);
  
  bit16u ret = vsprintf(targ, str, vargs);
  
  // can't call puts because it puts another '\n' at the end of the string.
  // also, we don't need to check for '\r' before '\n' because vsprintf()
  //  already did this for us
  for (bit16u i=0; i<ret; i++) {
    if (targ[i] == 10)
      para_putch(13);
    para_putch(targ[i]);
  }
  
  mfree(targ);
  
  return ret;
}
