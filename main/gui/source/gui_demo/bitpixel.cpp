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
 *  bitpixel.cpp
 *  
 *  Last updated: 17 July 2020
 */

#include <stdlib.h>

#include "../include/ctype.h"
#include "gui.h"
#include "grfx.h"
#include "palette.h"

/* bitmap_getpixel()
 *   bitmap = source bitmap
 *     left = X coordinate of pixel to get
 *      top = Y coordinate of pixel to get
 *
 *  returns PIXEL value of pixel at given location
 *  does not account for multiple images within array due to the fact
 *   that this routine is used only on single image arrays
 */
PIXEL bitmap_getpixel(const struct BITMAP *bitmap, int left, int top) {
  const struct RECT *area = GUIRECT(bitmap);
  
  // if outside of area, return BLACK (have to return something)
  if ((left < area->left) || (left > area->right) || (top < area->top) || (top > area->bottom))
    return GUICOLOR_black;
  
  // return value of PIXEL at location given
  return bitmap->array[(top - area->top) * bitmap->pitch + (left - area->left)];
}

/* bitmap_pixel()
 *   bitmap = destination bitmap
 *     left = X coordinate of pixel to put
 *      top = Y coordinate of pixel to put
 *    color = color of pixel to put
 *
 *  places a PIXEL value of color at given location
 *  does not account for multiple images within array due to the fact
 *   that this routine is used only on single image arrays
 */
void bitmap_pixel(struct BITMAP *bitmap, int left, int top, PIXEL color) {
  // if outside of area, just return
  if ((left < bitmap->clip.left) || (left > bitmap->clip.right) || (top < bitmap->clip.top) || (top > bitmap->clip.bottom))
    return;
  
  const struct RECT *area = GUIRECT(bitmap);
  PIXEL *pixel = &bitmap->array[(top - area->top) * bitmap->pitch + (left - area->left)];
  *pixel = GUIBLEND(color, *pixel);
}

/* make_random_image()
 *       w = width of image to create
 *       h = height of image to create
 *
 *  this makes a very simple pattern image within the bitmap
 */
struct BITMAP *make_random_image(const int width, const int height) {
  int w, h;
  PIXEL *p;
  
  // initialize random seed
  srand(time(NULL));
  
  // make the bitmap object
  struct BITMAP *bitmap = obj_bitmap(width, height, 1);
  if (bitmap == NULL)
    return NULL;
  
  // point to our bitmap
  p = bitmap->array;
  for (h=0; h<height; h++)
    for (w=0; w<width; w++)
      *p++ = GUIRGB(96, (156 - 16) + (rand() & 31), 185);
  
  return bitmap;
}
