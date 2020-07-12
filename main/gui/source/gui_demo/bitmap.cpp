/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * bitmap.cpp
 *  
 *  these functions are used to create, destroy, and manipulate bitmaps
 */

#include <string.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"
#include "grfx.h"
#include "palette.h"
#include "images.h"

/* bitmap_destroy()
 *   atom = points to the bitmap object's memory
 *
 * this free's the memory used by array
 */
void bitmap_destroy(void *atom) {
  struct BITMAP *bitmap = (struct BITMAP *) atom;
  if (bitmap->array)
    free(bitmap->array);
}

/* obj_bitmap()
 *    width = the width of the object to create
 *   height = the height of the object to create
 *    count = number of images within the bitmap (default = 1)
 *
 * create a bitmap object
 */
struct BITMAP *obj_bitmap(int width, int height, int count) {
  int i;
  struct RECT area;
  struct BITMAP *bitmap;
  
  // create the object
  bitmap = (struct BITMAP *) obj_rectatom(NULL, bitmap_destroy, sizeof(struct BITMAP));
  
  // create a rect for it and put into the rectatom's rect fields
  area.left = 0;
  area.top = 0;
  area.right = width - 1;
  area.bottom = height - 1;
  rectatom_place(GUIRECTATOM(bitmap), &area);
  
  // give it some attributes and allocate the pixel array
  bitmap->pitch = width;
  bitmap->clip = *GUIRECT(bitmap);
  bitmap->count = count;
  bitmap->current = 0;
  bitmap->delay_array[0] = 0;
  bitmap->array = (PIXEL *) malloc(width * height * count * sizeof(PIXEL));
  
  // fill the bitmap with white so it has something in it...
  for (i=0; i<(width * height * count); i++)
    bitmap->array[i] = GUICOLOR_white;
  
  // return a pointer to this object
  return bitmap;
}

/* bitmap_copy()
 *    other = the source bitmap to copy from
 *
 * create a bitmap object and copies the contents from source given
 */
struct BITMAP *bitmap_copy(const struct BITMAP *other) {
  
  // get the width and height and create the object
  const int w = gui_w(other);
  const int h = gui_h(other);
  struct BITMAP *bitmap = obj_bitmap(w, h, other->count);
  
  // update the rectatom fields
  rectatom_place(GUIRECTATOM(bitmap), GUIRECT(other));
  
  // attributes and copy the bitmap array
  bitmap->pitch = other->pitch;
  bitmap->clip = other->clip;
  bitmap->count = other->count;
  bitmap->current = other->current;
  for (int i=0; i<other->count; i++)
    bitmap->delay_array[i] = other->delay_array[i];
  
  // copy the bitmaps contents
  // (remember that the array may be different that the actual contents of the bitmap)
  bitmap_blitcopy(other, bitmap, 0, 0, 0, 0, w, h);
  
  // return the new bitmap object
  return bitmap;
}

/* bitmap_clip()
 *    bitmap = the source bitmap
 *   newclip = the target bitmap
 *
 *   sets the clip area of bitmap to the size of newclip
 */
bool bitmap_clip(struct BITMAP *bitmap, const struct RECT *newclip) {
  if (newclip)
    RECT_INTERSECT(*newclip, *GUIRECT(bitmap), bitmap->clip);
  else
    bitmap->clip = *GUIRECT(bitmap);
  
  return RECT_VALID(bitmap->clip) ? TRUE : FALSE;
}

/* bitmap_clip_get()
 *    bitmap = the source bitmap
 *
 *   returns the address of the clip rect of source bitmap
 */
const struct RECT *bitmap_clip_get(const struct BITMAP *bitmap) {
   return &bitmap->clip;
}

/* bitmap_offset()
 *    bitmap = the source bitmap
 *     left = X coordinate relative to left side of bitmap
 *      top = Y coordinate relative to top of bitmap
 *
 *   
 */
void bitmap_offset(struct BITMAP *bitmap, int left, int top) {
  struct RECT area;
  int dx, dy;
  
  area = *GUIRECT(bitmap);
  dx = left - area.left;
  dy = top - area.top;
  
  area.left += dx;
  area.top += dy;
  area.right += dx;
  area.bottom += dy;
  rectatom_place(GUIRECTATOM(bitmap), &area);
  
  bitmap->clip.left += dx;
  bitmap->clip.top += dy;
  bitmap->clip.right += dx;
  bitmap->clip.bottom += dy;
}

/* bitmap_iter()
 *    bitmap = the source bitmap
 *     left = X coordinate relative to left side of bitmap
 *      top = Y coordinate relative to top of bitmap
 *
 *  returns the address of the pixel specified by left, top.
 *  this will check to make sure is within the boundaries
 *  this will account for multiple images and gets "current" image
 *
 */
PIXEL *bitmap_iter(const struct BITMAP *bitmap, int left, int top) {
  const struct RECT *area;
  PIXEL *array = (PIXEL *) bitmap->array;
  
  // check to see if we are in range
  if ((left < bitmap->clip.left) ||
      (left > bitmap->clip.right) ||
      (top < bitmap->clip.top) ||
      (top > bitmap->clip.bottom))
    return NULL;
  
  // a pointer to the area
  area = GUIRECT(bitmap);
  
  // move to "current" image if multiple images are used
  if ((bitmap->count > 1) && (bitmap->current > 0))
    array = (PIXEL *) ((bit8u *) bitmap->array + (bitmap->current * gui_w(bitmap) * gui_h(bitmap) * sizeof(PIXEL)));
  
  // return address to specified pixel
  return &array[(top - area->top) * bitmap->pitch + (left - area->left)];
}

/* bitmap_begin()
 *    bitmap = the source bitmap
 *
 *  returns the address of the first pixel in array
 *
 */
PIXEL *bitmap_begin(const struct BITMAP *bitmap) {
  return bitmap->array;
}

/* bitmap_end()
 *    bitmap = the source bitmap
 *
 *  returns the address of the pixel after the last pixel in array
 *
 */
PIXEL *bitmap_end(const struct BITMAP *bitmap) {
  return bitmap_iter(bitmap, gui_w(bitmap), gui_h(bitmap)) + 1;
}

/* bitmap_blit()
 *     src = the source bitmap
 *    dest = the source bitmap
 *      sx = X coordinate relative to left side of source bitmap
 *      sy = Y coordinate relative to top side of source bitmap
 *      dx = X coordinate relative to left side of destination bitmap
 *      dy = Y coordinate relative to top side of destination bitmap
 *       w = width of bitmap to copy
 *       h = height of bitmap to copy
 *  darken = TRUE if we need to darken the pixels (for a disabled look)
 *
 *  this copies a portion of src bitmap to the destination bitmap
 *
 */
void bitmap_blit(const struct BITMAP *src, struct BITMAP *dest, int sx, int sy, int dx, int dy, int w, int h, const bool darken) {
  int y;
  struct RECT srca, dsta;
  
  srca.left = sx;
  srca.top = sy;
  srca.right = sx + w;
  srca.bottom = sy + h;
  
  RECT_INTERSECT(*GUIRECT(src), srca, srca);
  if (!RECT_VALID(srca))
      return;
  
  w = srca.right - srca.left;
  h = srca.bottom - srca.top;
  
  dsta.left = dx;
  dsta.top = dy;
  dsta.right = dx + w;
  dsta.bottom = dy + h;
  
  RECT_INTERSECT(dest->clip, dsta, dsta);
  if (!RECT_VALID(dsta))
      return;
  
  // don't copy more than there is in width and height
  w = MIN(w, dsta.right - dsta.left);
  h = MIN(h, dsta.bottom - dsta.top);
  
  sx = srca.left + dsta.left - dx;
  sy = srca.top + dsta.top - dy;
  dx = dsta.left;
  dy = dsta.top;
  
  // if width or height are zero, no need to do anything
  if ((h < 0) || (w < 0))
      return;
    
  // Copying from one bitmap to another
  for (y = 0; y <= h; y++) {
    PIXEL *srci = bitmap_iter(src, sx, sy + y);
    PIXEL *dsti = bitmap_iter(dest, dx, dy + y);
    const PIXEL *endi = dsti + w + 1;
    
    while (dsti != endi) {
      if (darken && (*srci != 0xFF000000))
        *dsti = GUIBLENDT(*srci, *dsti, 0x80);
      else
        *dsti = GUIBLEND(*srci, *dsti);
      dsti++;
      srci++;
    }
  }
}

/* bitmap_blitstretch()
 *     src = the source bitmap
 *    dest = the source bitmap
 *      sx = X coordinate relative to left side of source bitmap
 *      sy = Y coordinate relative to top side of source bitmap
 *      sw = width of bitmap to stretch from
 *      sh = height of bitmap to stretch from
 *      dx = X coordinate relative to left side of destination bitmap
 *      dy = Y coordinate relative to top side of destination bitmap
 *      dw = width of bitmap to stretch to
 *      dh = height of bitmap to stretch to
 *
 *  stretch the image to fit into dw x dh
 *
 */
void bitmap_blitstretch(const struct BITMAP *src, struct BITMAP *dest, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
  int x, y;
  PIXEL *srci, *dsti;
  
  // if they are the same size, just blit it
  if ((dw == sw) && (dh == sh)) {
    bitmap_blit(src, dest, sx, sy, dx, dy, dw, dh, FALSE);
    return;
  }
  
  // if not visible, don't do it
  if (dy + dh < dest->clip.top)
    return;
  if (dx + dw < dest->clip.left)
    return;
  
  sw++;
  sh++;
  dw++;
  dh++;
  
  for (y=0; y<dh; y++) {
    const int ddy = dy + y;
    
    if (ddy > dest->clip.bottom)
      y = dh;
    else if (ddy >= dest->clip.top) {
      dsti = bitmap_iter(dest, dest->clip.left, ddy);
      srci = bitmap_iter(src, sx, sy + (y * sh) / dh);
      
      dsti += dx - dest->clip.left;
      
      for (x=0; x<dw; x++) {
         const int ddx = dx + x;
         
         if (ddx > dest->clip.right)
            x = dw;
         else if (ddx >= dest->clip.left)
            *dsti = GUIBLEND(srci[(x * sw) / dw], *dsti);
         
         dsti++;
      }
    }
  }
}

/* bitmap_blitcopy()
 *     src = the source bitmap
 *    dest = the source bitmap
 *      sx = X coordinate relative to left side of source bitmap
 *      sy = Y coordinate relative to top side of source bitmap
 *      dx = X coordinate relative to left side of destination bitmap
 *      dy = Y coordinate relative to top side of destination bitmap
 *       w = width of bitmap to copy
 *       h = height of bitmap to copy
 *
 *  this copies all of src bitmap to the destination bitmap
 *
 */
void bitmap_blitcopy(const struct BITMAP *src, struct BITMAP *dest, int sx, int sy, int dx, int dy, int w, int h) {
  int y;
  const struct RECT *srcarea = GUIRECT(src);
  struct RECT srca, dsta;
  
  srca.left = sx;
  srca.top = sy;
  srca.right = sx + w;
  srca.bottom = sy + h;
  
  RECT_INTERSECT(*srcarea, srca, srca);
  if (!RECT_VALID(srca))
    return;
  
  w = srca.right - srca.left;
  h = srca.bottom - srca.top;
  
  dsta.left = dx;
  dsta.top = dy;
  dsta.right = dx + w;
  dsta.bottom = dy + h;
  
  RECT_INTERSECT(dest->clip, dsta, dsta);
  if (!RECT_VALID(dsta))
    return;
  
  w = MIN(w, dsta.right - dsta.left);
  h = MIN(h, dsta.bottom - dsta.top);
  
  sx = srca.left + dsta.left - dx;
  sy = srca.top + dsta.top - dy;
  dx = dsta.left;
  dy = dsta.top;
  
  for (y = 0; y <= h; y++)
    memcpy(bitmap_iter(dest, dx, dy + y), bitmap_iter(src, sx, sy + y), (w + 1) * sizeof(PIXEL));
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Static Images
 *  This is a set of images used by the GUI to display as buttons,
 *  icons, etc.  Each static image has an ID number so we can simply
 *  call get_static_bitmap(id, 0, 0) to retrieve the image
 */
struct STATIC_IMAGES_HDR *static_image_buffer = NULL;

/* rle_decompress()
 *       fp = file pointer to read from
 *   target = pointer to start storing uncompressed data to
 *  
 *  decompresses the data from FP to target using simple RLE encoding
 *  will return count of bytes decoded
 *  
 *  * we write the pixels in big-endian form for this to work *
 *  this relies upon the fact that our pixels will all have the
 *   high byte (the transparent byte) either 0x00 or 0xFF.
 *  if a 0x00 or 0xFF is found in the rle stream, it is a the
 *   actual T part of the pixel.  If anything else if found in 
 *   the stream, it is a byte count and the next four bytes is 
 *   the pixel value.
 *  this is an RLE of pixels, not bytes
 *
 */
size_t rle_decompress(FILE *fp, PIXEL *target) {
  size_t count = 0;
  PIXEL pixel;
  
  int ch = fgetc(fp);
  while (ch != EOF) {
    if ((ch == 0x00) || (ch == 0xFF)) {
      pixel  = (ch        << 24);
      pixel |= (fgetc(fp) << 16);
      pixel |= (fgetc(fp) <<  8);
      pixel |= (fgetc(fp) <<  0);
      *target++ = pixel;
      count++;
    } else {
      fread(&pixel, 1, sizeof(PIXEL), fp);
      pixel = ENDIAN_32(pixel);
      count += ch;
      while (ch--)
        *target++ = pixel;
    }
    ch = fgetc(fp);
  }
  
  return count * sizeof(PIXEL);
}

/* load_static_images()
 *       name = filename of "images.sys" file
 *
 * a function to read in the static image file.
 *  should only be done once
 *   
 */
bool load_static_images(const char *name) {
  struct STATIC_IMAGES_HDR hdr;
  FILE *fp;
  char filename[512];
  size_t read, image_start;
  
  prefix_default_path(filename, name);
  
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    puts("\n Error loading images...");
    return FALSE;
  }
  
  // read in the header
  fread(&hdr, 1, sizeof(struct STATIC_IMAGES_HDR), fp);
  if (hdr.magic != STATIC_IMAGES_MAGIC) {
    printf("\n Error with image file header. (%08X)", hdr.magic);
    fclose(fp);
    return FALSE;
  }
  
  // allocate the static image buffer
  // the hdr.total_size field is the size of all the data when uncompressed.
  static_image_buffer = (struct STATIC_IMAGES_HDR *) malloc(hdr.total_size);
  
  // backup to the start of the image again
  rewind(fp);
  
  // was compression used?
  switch (hdr.comp_type) {
    case 0:  // no compression
      read = fread(static_image_buffer, 1, hdr.total_size, fp);
      fclose(fp);
      if (read == hdr.total_size)
        return TRUE;
      break;
    case 1:  // simple RLE compression
      // let's first read in the main header and all the entry headers
      image_start = sizeof(struct STATIC_IMAGES_HDR) + (hdr.total_images * sizeof(struct STATIC_IMAGE));
      read = fread(static_image_buffer, 1, image_start, fp);
      if (read == image_start) {
        read = image_start + rle_decompress(fp, (PIXEL *) ((bit8u *) static_image_buffer + image_start));
        if (read == hdr.total_size)
          return TRUE;
      }
      break;
    default:
      puts("Unknown compression type used in Static Images File...");
  }
  
  printf("Did not read all of images.sys file...(read %i of %i)\n", read, hdr.total_size);
  return FALSE;
}

/* get_static_bitmap()
 *    no parameters
 *
 *  frees the static image buffer
 */
void free_static_images(void) {
  if (static_image_buffer)
    free(static_image_buffer);
}

/* get_static_bitmap()
 *     id = id of image to retrieve
 *      w = width of image (0 if width doesn't matter)
 *      h = height of image (0 if height doesn't matter)
 *
 * this will try to retrieve the image with the id of 'id' and if w and h > 0, the size specified.
 *  if an image is found with that ID but it does not match the size specified (if given),
 *  it will search for the next image with that ID and size.  If none found, it will return NULL.
 * if no size is given (w == h == 0), then it will return the first image with the given ID.
 *
 * This function has no idea that the file was compressed.  The function above does the
 *  decompressing and patching of the offset field.
 */
struct BITMAP *get_static_bitmap(const unsigned id, const unsigned w, const unsigned h) {
  bit32u i;
  int j;
  struct STATIC_IMAGES_HDR *images = static_image_buffer;
  
  if (images) {
    // start with first image
    struct STATIC_IMAGE *image = (struct STATIC_IMAGE *) ((bit8u *) images + sizeof(struct STATIC_IMAGES_HDR));
    for (i=0; i<images->total_images; i++) {
      if (image[i].id == id) {
        if ((!w && !h) || ((w == image[i].width) && (h == image[i].height))) {
          struct BITMAP *bitmap = obj_bitmap(image[i].width, image[i].height, image[i].count);
          memcpy(bitmap->array, ((bit8u *) images + image[i].offset), image[i].width * image[i].height * sizeof(PIXEL) * image[i].count);
          // TODO: We currently do not specify the delay time for each
          //  image in make_img.exe. Therefore, we do a default value and initialize delay[w] here.
          if (image[i].count > 1) {
            for (j=0; j<image[i].count; j++)
              bitmap->delay_array[j] = 8;  // default to 8 timer ticks (DJGPP's DOS timer tick is 91 ticks per second)
            bitmap->count_down = bitmap->delay_array[0];
          }
          return bitmap;
        }
      }
    }
  }
  
  // did not find one so return NULL
  return NULL;
}

/* get_bitmap()
 *   name = filename of image to get
 *   fail = 0 = don't fail--return a 32x32 white box
 *          1 = fail--return NULL if error
 *
 *  this will try to open the filename given, then convert the file to
 *   a bitmap, returning the newly created bitmap image.
 *  this will allow up to 256 images to be placed in this bitmap object.
 *  TODO: There is no check for overflow of 256 though...
 *
 */
struct BITMAP *get_bitmap(const char *name, const bool fail) {
  int i;
  int w[256], h[256], delay[256], image_count;
  char filename[512];
  
  // prefix the default path to the filename?
  prefix_default_path(filename, name);
  
  PIXEL *image = get_image(filename, w, h, delay, &image_count, fail);
  if (image) {
    // always use the size of the first image of the set (w[0], and h[0])
    struct BITMAP *bitmap = obj_bitmap(w[0], h[0], image_count);
    memcpy(bitmap->array, image, (w[0] * h[0] * image_count * sizeof(PIXEL)));
    bitmap->count = image_count;
    bitmap->current = 0;
    for (i=0; i<image_count; i++)
      bitmap->delay_array[i] = delay[i];
    bitmap->count_down = bitmap->delay_array[0];
    free(image);
    return bitmap;
  }
  
  // if we didn't get the image, we return NULL
  return NULL;
}
