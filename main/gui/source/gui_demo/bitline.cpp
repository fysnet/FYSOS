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
 *  bitline.cpp
 *  
 *  These functions are used to draw lines, boxes, filled boxes, and circles
 *
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>
#include <math.h>

#include "../include/ctype.h"

#include "gui.h"
#include "grfx.h"
#include "palette.h"

// we check to see if these lines will actually be visible.  
// If not, no need to draw it.  Afterall, we need speed here...
#define CLIP_VLINE(rect, xa, ya, yb)                                           \
   (((xa) < (rect).left) ||                                                    \
    ((xa) > (rect).right) ||                                                   \
    (((yb) = MIN((yb), ((rect).bottom))) < ((ya) = MAX((ya), ((rect).top)))))

#define CLIP_HLINE(rect, xa, ya, xb)                                         \
   (((ya) < (rect).top) ||                                                    \
    ((ya) > (rect).bottom) ||                                                    \
    (((xb) = MIN((xb), ((rect).right))) < ((xa) = MAX((xa), ((rect).left)))))

#define CLIP_RECT(rect, left, top, right, bottom)                                      \
   ((((right) = MIN((right), ((rect).right))) < ((left) = MAX((left), ((rect).left)))) ||   \
    (((bottom) = MIN((bottom), ((rect).bottom))) < ((top) = MAX((top), ((rect).top)))))


/* bitmap_vline()
 *   bitmap = points to the bitmap to draw this line to.
 *     left = X coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *    color = pixel to use for line
 *
 *  draw a vertical line to a bitmap
 *  if the line is not visable (covered up or full transparent), it does not
 *   draw it.  Speed, remember.
 *
 */
void bitmap_vline(struct BITMAP *bitmap, int left, int top, int bottom, PIXEL color) {
  PIXEL *start;
  const struct RECT *area;
  const bit8u trans = GUIT(color);
  
  // if the line isn't visible, due to the fact that the line is outside
  //   of the current dimensions of the object, don't need to draw it...
  if (CLIP_VLINE(bitmap->clip, left, top, bottom))
    return;
  
  // if we show full transparency via the color, don't draw it either
  if (trans == 0xFF)
    return;
  
  // get the area of the bitmap to draw to
  area = GUIRECT(bitmap);
  
  // get relative to area visible
  top -= area->top;
  bottom -= area->top;
  start = &bitmap->array[top * bitmap->pitch + (left - area->left)];
  
  // draw the line from top to bottom
  while (top <= bottom) {
    *start = (trans) ? GUIBLENDT(color, *start, trans) : color;
    start += bitmap->pitch;
    top++;
  }
}

/* fasthline()
 *   bitmap = points to the bitmap to draw this line to.
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *    color = pixel to use for line
 *
 *  draw a horizontal line to a bitmap
 *  this function only checks for transparency, not visibility
 */
void fasthline(struct BITMAP *bitmap, int left, int top, int right, PIXEL color) {
  PIXEL *start, *end;
  const bit8u trans = GUIT(color);
  
  // if we show full transparency via the color, don't draw it either
  if (trans == 0xFF)
    return;
  
  // get the area of the bitmap to draw to
  const struct RECT *area = GUIRECT(bitmap);
  start = &bitmap->array[(top - area->top) * bitmap->pitch + (left - area->left)];
  end = start + (right - left);
  
  while (start <= end) {
    *start = (trans) ? GUIBLENDT(color, *start, trans) : color;
    start++;
  }
}

/* bitmap_hline()
 *   bitmap = points to the bitmap to draw this line to.
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *    color = pixel to use for line
 *
 *  draw a horizontal line to a bitmap
 *  if the line is not visable (covered up or full transparent), it does not
 *   draw it.  Speed, remember.
 *
 */
void bitmap_hline(struct BITMAP *bitmap, int left, int top, int right, PIXEL color) {
  
  // check to see if it is visible
  if (CLIP_HLINE(bitmap->clip, left, top, right))
    return;
  
  // call the function to draw the line
  fasthline(bitmap, left, top, right, color);
}

/* bitmap_rectfill()
 *   bitmap = points to the bitmap to draw this rect to
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *    color = pixel to use for fill
 *
 *  draw a filled rectangle box to the bitmap given
 *  if the box is not visable (covered up or full transparent), it does not
 *   draw it.  Speed, remember.
 *
 */
void bitmap_rectfill(struct BITMAP *bitmap, int left, int top, int right, int bottom, PIXEL color) {
  // are we visible?
  if (CLIP_RECT(bitmap->clip, left, top, right, bottom))
    return;
  
  while (top <= bottom) {
    fasthline(bitmap, left, top, right, color);
    top++;
  }
}

/* bitmap_circle()
 *   bitmap = points to the bitmap to draw this rect to
 *        x = X coordinate center of circle relative to left side of bitmap
 *        y = Y coordinate center of circle relative to top side of bitmap
 *   radius = radius of the circle
 *    start = degree to start drawing from
 *      end = degree to stop at (including this degree)
 *    color = pixel to use for circle
 *
 *  draw a (full/partial) circle to the bitmap given
 *  if the circle is not visable (covered up or full transparent), it does not
 *   draw it.  Speed, remember.
 *
 */
void bitmap_circle(struct BITMAP *bitmap, const int x, const int y, const int radius, int start, const int end, const PIXEL color) {
  int left = x - radius;
  int top = y - radius;
  int right = x + radius;
  int bottom = y + radius;
  
  // are we visible?
  if (CLIP_RECT(bitmap->clip, left, top, right, bottom))
    return;
  
  // draw the circle
  //  x' = r * cos( theta )
  //  y' = r * sin( theta )
  //  Where r is the radius of the desired circle and theta is the angle. (x, y) will be your point on the given circle.
  // Then take given coords from caller and sub x and y for pixel location.
  //  i.e.:  pixel(x) = x - r * cos(theta)
  //         pixel(y) = y - r * sin(theta)
  // We start with 0 degrees as the point to the left (due west)
  PIXEL *iter;
  const bit8u trans = GUIT(color);
  int px, py;
  double dcos, dsin;
  
  while (start != end) {
#if ((__DJGPP__ >= 2) && (__DJGPP_MINOR__ >= 5))
    sincos((double) start * 0.017453292, &dcos, &dsin);  // Radians = degrees * (pi / 180)  degrees = radians * (180 / pi)
#else
    sincos(&dcos, &dsin, (double) start * 0.017453292);  // Radians = degrees * (pi / 180)  degrees = radians * (180 / pi)
#endif
    px = x - (int) ((double) radius * dcos);
    py = y - (int) ((double) radius * dsin);
    
    iter = bitmap_iter(bitmap, px, py);
    if (iter)
      *iter = (trans) ? GUIBLENDT(color, *iter, trans) : color;
    
    if (++start == 360) start = 0;
  }
}

/* bitmap_circle()
 *   bitmap = points to the bitmap to draw this rect to
 *    color = pixel to use clear
 *
 *  completely clears a bitmap to the given color
 */
void bitmap_clear(struct BITMAP *bitmap, PIXEL color) {
  int left = gui_left(bitmap);
  int top = gui_top(bitmap);
  int right = gui_right(bitmap);
  int bottom = gui_bottom(bitmap);
  
  // are we visible?
  if (CLIP_RECT(bitmap->clip, left, top, right, bottom))
    return;
  
  // since all bitmaps are rectangles, call the rectfill() function
  bitmap_rectfill(bitmap, left, top, right, bottom, color);
}

// a few macros to help out (non-vertical, non-horz) line drawer
#define SWAP(a, b)      \
  do {                  \
    const int temp = a; \
    a = b;              \
    b = temp;           \
  } while (0)

#define GETCODE(x, y, code)       \
  do {                            \
    if (x < bitmap->clip.left)    \
        code |= 1;                \
    if (x > bitmap->clip.right)   \
        code |= 2;                \
    if (y < bitmap->clip.top)     \
        code |= 4;                \
    if (y > bitmap->clip.bottom)  \
        code |= 8;                \
  } while (0)


/* bitmap_line()
 *   bitmap = points to the bitmap to draw this line to.
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *    color = pixel to use for line
 *
 *  draw a non-vert, non-horz line to the given bitmap
 *  
 */
void bitmap_line(struct BITMAP *bitmap, int left, int top, int right, int bottom, PIXEL color) {
  int code1 = 0, code2 = 0;
  int dy = abs(bottom - top) << 1;
  int dx = abs(right - left) << 1;
  PIXEL *iter;
  const bit8u trans = GUIT(color);
  
  // check for full transparency
  if (trans == 0xFF)
    return;
  
  // are we visible?
  GETCODE(left, top, code1);
  GETCODE(right, bottom, code2);
  if (code1 & code2)
    return;
  
  if (!RECT_VALID(bitmap->clip))
    return;
  
  // if we are a vert line, call it instead
  if (dx == 0) {
    if (bottom < top) {
      SWAP(left, right);
      SWAP(top, bottom);
    }
    bitmap_vline(bitmap, left, top, bottom, color);
    
  // if we are a horz line, call it instead
  } else if (dy == 0) {
    if (right < left) {
      SWAP(left, right);
      SWAP(top, bottom);
    }
    bitmap_hline(bitmap, left, top, right, color);
    
  } else if (dx > dy) {
    int stepy, stopy;
    int fraction = dy - (dx >> 1);

    if (right < left) {
      SWAP(left, right);
      SWAP(top, bottom);
    }

    if (bottom > top) {
      stepy = 1;
      stopy = bitmap->clip.bottom + 1;
    } else {
      stepy = -1;
      stopy = bitmap->clip.top - 1;
    }

    if (left < bitmap->clip.left) {
      const int xn = bitmap->clip.left - left;
      const int cy = (fraction + xn * dy - dy + dx) / dx;

      left = bitmap->clip.left;
      top = top + cy * stepy;
      fraction = fraction - cy * dx + xn * dy;
    }

    right = MIN(right, bitmap->clip.right);
    if (left > right)
      return;

    if (((stepy == 1) && (top > bitmap->clip.bottom)) || ((stepy == -1) && (top < bitmap->clip.top)))
      return;

    if (bottom == top) {
      bitmap_hline(bitmap, left, top, right, color);
      return;

    } else if ((stepy == 1) && (top < bitmap->clip.top)) {
      const int yn = bitmap->clip.top - top;
      const int cx = (dy - dx - fraction + yn * dx + dy - 1) / dy;

      left += cx;
      top = bitmap->clip.top;
      fraction = fraction - yn * dx + cx * dy;

    } else if ((stepy == -1) && (top > bitmap->clip.bottom)) {
      const int yn = top - bitmap->clip.bottom;
      const int cx = (dy - dx - fraction + yn * dx + dy - 1) / dy;

      left += cx;
      top = bitmap->clip.bottom;
      fraction = fraction - yn * dx + cx * dy;
    }

    iter = bitmap_iter(bitmap, left, top);

    if (trans) {
      while ((left <= right) && (top != stopy)) {
         *iter = GUIBLENDT(color, *iter, trans);

         if (fraction >= 0) {
            top += stepy;
            iter += bitmap->pitch * stepy;
            fraction -= dx;
         }
         left++;
         iter++;
         fraction += dy;
      }
    } else {
      while ((left <= right) && (top != stopy)) {
         *iter = color;

         if (fraction >= 0) {
            top += stepy;
            iter += bitmap->pitch * stepy;
            fraction -= dx;
         }
         left++;
         iter++;
         fraction += dy;
      }
    }
  } else {
    int stepx, stopx;
    int fraction = dx - (dy >> 1);

    if (bottom < top) {
      SWAP(top, bottom);
      SWAP(left, right);
    }

    if (right > left) {
      stepx = 1;
      stopx = bitmap->clip.right + 1;
    } else {
      stepx = -1;
      stopx = bitmap->clip.left - 1;
    }

    if (top < bitmap->clip.top) {
      const int yn = bitmap->clip.top - top;
      const int cx = (fraction + yn * dx - dx + dy) / dy;

      top = bitmap->clip.top;
      left = left + cx * stepx;
      fraction = fraction - cx * dy + yn * dx;
    }

    bottom = MIN(bottom, bitmap->clip.bottom);
    if (top > bottom)
      return;

    if (((stepx == 1) && (left > bitmap->clip.right)) || ((stepx == -1) && (left < bitmap->clip.left)))
      return;

    if (right == left) {
      bitmap_vline(bitmap, left, top, bottom, color);
      return;

    } else if ((stepx == 1) && (left < bitmap->clip.left)) {
      const int xn = bitmap->clip.left - left;
      const int cy = (dx - dy - fraction + xn * dy + dx - 1) / dx;

      top = top + cy;
      left = bitmap->clip.left;
      fraction = fraction - xn * dy + cy * dx;

    } else if ((stepx == -1) && (left > bitmap->clip.right)) {
      const int xn = left - bitmap->clip.right;
      const int cy = (dx - dy - fraction + xn * dy + dx - 1) / dx;

      top = top + cy;
      left = bitmap->clip.right;
      fraction = fraction - xn * dy + cy * dx;
    }

    iter = bitmap_iter(bitmap, left, top);

    if (trans) {
      while ((top <= bottom) && (left != stopx)) {
         *iter = GUIBLENDT(color, *iter, trans);

         if (fraction >= 0) {
            left += stepx;
            iter += stepx;
            fraction -= dy;
         }
         ++top;
         iter += bitmap->pitch;
         fraction += dx;
      }
    } else {
      while ((top <= bottom) && (left != stopx)) {
         *iter = color;

         if (fraction >= 0) {
            left += stepx;
            iter += stepx;
            fraction -= dy;
         }
         ++top;
         iter += bitmap->pitch;
         fraction += dx;
      }
    }
  }
}

/* bitmap_box()
 *   bitmap = points to the bitmap to draw this rect to
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *    width = how many pixels in width to draw the box
 *    light = pixel to use for light side of box (top and left)
 *     dark = pixel to use for dark side of box (right and bottom)
 *
 *  use a negative number in width to shade opposite
 *
 *  draw a non-filled rectangle box to the bitmap given
 *  if part of box is not visable (covered up or full transparent), it does not
 *   draw that part.  Speed, remember.
 *
 */
void bitmap_box(struct BITMAP *bitmap, int left, int top, int right, int bottom, int width, PIXEL light, PIXEL dark) {
  int i;
  
  // opposite shade (sunken in)?
  if (width < 0) {
    SWAP(light, dark);
    width *= -1;
  }
  
  // draw the box, moving in for each count of width
  for (i=0; i<width; i++) {
    bitmap_vline(bitmap, left, top, bottom, light);
    bitmap_hline(bitmap, left + 1, top, right, light);
    
    bitmap_vline(bitmap, right, top + 1, bottom, dark);
    bitmap_hline(bitmap, left + 1, bottom, right - 1, dark);
    
    left++, top++, right--, bottom--;
  }
}

/* bitmap_box_dotted()
 *   bitmap = points to the bitmap to draw this rect to
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *
 *  draw a non-filled rectangle dotted box to the bitmap given
 *  if part of box is not visable (covered up or full transparent), it does not
 *   draw that part.  Speed, remember.
 *
 */
#define DOTTED_LEN    7
#define DOTTED_LINE   3
void bitmap_box_dotted(struct BITMAP *bitmap, const int left, const int top, const int right, const int bottom, const PIXEL color) {
  int i, l = left, t = top;
  
  i = top + DOTTED_LINE;
  while (i < bottom) {
    bitmap_vline(bitmap, left, t, i, color);
    bitmap_vline(bitmap, right, t, i, color);
    t += DOTTED_LEN;
    i += DOTTED_LEN;
  }
  
  i = left + DOTTED_LINE;
  while (i < right) {
    bitmap_hline(bitmap, l, top, i, color);
    bitmap_hline(bitmap, l, bottom, i, color);
    l += DOTTED_LEN;
    i += DOTTED_LEN;
  }
}

/* bitmap_frame()
 *   bitmap = points to the bitmap to draw this rect to
 *     left = X1 coordinate relative to left side of bitmap
 *      top = Y1 coordinate relative to top of bitmap
 *    right = X2 coordinate relative to left of bitmap
 *   bottom = Y2 coordinate relative to top of bitmap
 *    width = how many pixels in width to draw the box
 *    light = pixel to use for light side of box (top and left)
 *     dark = pixel to use for dark side of box (right and bottom)
 *     fill = pixel to use to fill the box with
 *
 *  use a negative number in width to shade opposite
 *
 *  draw a filled rectangle frame to the bitmap given
 *  if part of box is not visable (covered up or full transparent), it does not
 *   draw that part.  Speed, remember.
 *
 */
void bitmap_frame(struct BITMAP *bitmap, int left, int top, int right, int bottom, int width, PIXEL light, PIXEL dark, PIXEL fill) {
   bitmap_box(bitmap, left, top, right, bottom, width, light, dark);
   bitmap_rectfill(bitmap, left + abs(width), top + abs(width), right - abs(width), bottom - abs(width), fill);
}
