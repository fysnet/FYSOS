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
 *  decorate.cpp
 *  
 *  When given a bitmap, will "decorate" a rectangle by expanding the bitmap to fit
 *
 *  Last updated: 17 July 2020
 */

#include "../include/ctype.h"

#include "gui.h"
#include "grfx.h"

/* bitmap_decorate()
 *      src = bitmap of source to decorate from
 *     dest = bitmap to decorate to
 *        x = X coordinate of source to start from
 *        y = Y coordinate of source to start from
 *     left = X1 coordinate of destination
 *      top = Y1 coordinate of destination
 *    right = X2 coordinate of destination
 *   bottom = Y2 coordinate of destination
 *   darken = flag to use a darker color (as with disabled items)
 *
 *   passing GUIDEF in X and Y will get the center of the source
 *
 *   this will expand a bitmap to a larger bitmap, decorating it
 *     a good example is the onoff object.  Source is a round ball, while this
 *     function will expand that to a long bar with rounded ends.
 * 
 */
void bitmap_decorate(const struct BITMAP *src, struct BITMAP *dest, int x, int y, int left, int top, int right, int bottom, const bool darken) {
  int i, r, b, w, h;
  PIXEL c;
  
  // safety check
  if (src == NULL)
    return;
  
  // width and height of source
  w = gui_w(src);
  h = gui_h(src);
  
  // if GUIDEF as x and y, get center of source
  if (x == GUIDEF)
    x = (w + 1) / 2;
  if (y == GUIDEF)
    y = (h + 1) / 2;
  
  r = right - w + x;
  b = bottom - h + y;
  
  bitmap_blit(src, dest, 0, 0, left, top, x - 1, y - 1, darken);
  bitmap_blit(src, dest, x, 0, r + 1, top, w - x, y - 1, darken);
  
  bitmap_blit(src, dest, 0, y, left, b + 1, x - 1, h - y, darken);
  bitmap_blit(src, dest, x, y, r + 1, b + 1, w - x, h - y, darken);
  
  for (i = 0; i < y - 1; i++) {
    c = bitmap_getpixel(src, x - 1, i);
    bitmap_hline(dest, left + x, top + i, r, c);
  }
  
  for (i = y; i < h; i++) {
    c = bitmap_getpixel(src, x - 1, i);
    bitmap_hline(dest, left + x, bottom - h + i + 1, r, c);
  }
  
  for (i = 0; i < x - 1; i++) {
    c = bitmap_getpixel(src, i, y - 1);
    bitmap_vline(dest, left + i, top + y, b, c);
  }
  
  for (i = x; i < w; i++) {
    c = bitmap_getpixel(src, i, y - 1);
    bitmap_vline(dest, right - w + i + 1, top + y, b, c);
  }
  
  c = bitmap_getpixel(src, x - 1, y - 1);
  bitmap_rectfill(dest, left + x - 1, top + y - 1, r, b, c);
}
