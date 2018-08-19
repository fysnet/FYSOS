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
 */


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

#include "conout.h"

#include "progress.h"


// Progress bar
bit32u progress_tot_size  = 0;  // set to total size at start of progress
int    progress_last      = 0;  // last percent printed
int    progress_x, progress_y;  // cursor start location

// this will not work for numbers greater than 42,949,671.
// for example, the calculation of
//   val = ((lo + 1) * 100);
// when lo = 42,949,672 would be
//   val = ((42,949,672 + 1) * 100) = 4,294,967,300 = 0x1_0000_0004
// and 0x1_0000_0004 is a 33 bit number
// so, until we can use 64-bit numbers, 42,949,671 is the limit
void init_progress(bit32u limit) {
  limit--;  // zero based
  
  if (limit > 42949671)
    limit = 42949671;
  if (limit < 0)
    limit = 0;
  
  progress_tot_size = limit;
  progress_last = -1;
  
  // get and save the current cursor position
  GetCurrentCursorPos(&progress_y, &progress_x);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Progress counter.  Displays (n%)
// on entry
//  current progress (64-bit)
#define PP_INDICIES 12
void put_progress(bit32u lo, bit32u hi) {
  bit32u val, cnt, i, j;
  
  // watch the limit
  if (lo > 42949671)
    lo = 42949671;
  
  // don't allow it to be more than the set total size
  //  from init_progress()
  if (lo > progress_tot_size)
    lo = progress_tot_size;
  
  // increment it to make it print 100% when done
  //  (1 based) (1  to 100, not 0 to 99)
  // this line is the limit maker.  If we could do
  //  this calculation in 64-bit, there would be a 
  //  much larger limit.
  val = ((lo + 1) * 100) / progress_tot_size;
  
  // don't let val be more than 100%
  // may happen on numbers less than PP_INDICIES
  if (val > 100)
    val = 100;
  
  if (val != progress_last) {
    progress_last = val;
    
    // print so many bars  ( ascii 221, then 219)
    PutChar(' ');
    PutChar(BOXDRAW_VERTICAL);
    cnt = ((PP_INDICIES * 2 * (val + 1)) + 99) / 100;
    j = cnt / 2;
    for (i=0; i<j; i++)
      //if (i < (j - 1))
        PutChar(BLOCKELEMENT_FULL_BLOCK);
      //else {
      //  if (cnt & 1) PutChar(BLOCKELEMENT_FULL_BLOCK);
      //  else         PutChar(GEOMETRICSHAPE_RIGHT_TRIANGLE);
      //}
    for (; i<PP_INDICIES; i++)
      PutChar('_'); // or do 176
    PutChar(BOXDRAW_VERTICAL);
    
    printf(L" (%[%i%%%])", HIGHLIGHT_COLOR, val);
    
    // set the location of the cursor to the saved position
    // either for the next call, or to clear the bar when we
    //  are done with the progress.
    SetCurrentCursorPos(progress_y, progress_x);
  }
}
