/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * bitpixel.cpp
 *  
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
