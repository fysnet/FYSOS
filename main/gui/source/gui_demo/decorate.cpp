/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * decorate.cpp
 *  
 *   when given a bitmap, will "decorate" a rectangle by expanding the bitmap to fit
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
