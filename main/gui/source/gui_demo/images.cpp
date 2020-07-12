/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * images.cpp
 *  
 *  this file tries to open an image file of .BMP, .ICO, .GIF, .PCX, etc.
 *    then returns an array of PIXEL data if found
 *
 */

#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"
#include "images.h"
#include "palette.h"

#include "image_bmp.h"
#include "image_pcx.h"
#include "image_gif.h"

/*  get_image()
 *        path = string holding path and filename of file to open
 *           x = pointer to store width of bitmap
 *           y = pointer to store height of bitmap
 *       delay = pointer to store delay (array?) TODO: FIXME
 *      images = pointer to store count of images
 *        fail = 0 = don't fail--return a 32x32 white box
 *            1 = fail--return NULL if error
 *
 * this function reads in the first of the file and trys to
 *  determine what type of image file it is.
 * it returns NULL if no image found, or unknown image found
 * returns a pointer to a buffer holding the RGB pixels of the image,
 *  and *x = width, *h = height, *images = images.
 * if images > 1, then each image is right after the last image in the buffer.
 *
 * if an image is not found, returns a 32x32 blank PIXEL array
 *
 * it is up to the callee to free the memory returned
 *
 */
PIXEL *get_image(const char *path, int *x, int *y, int *delay, int *images, const bool fail) {
  FILE *fp;
  PIXEL *image = NULL;
  
  if (delay) *delay = 0;
  
  // open file
  if ((fp = fopen(path, "rb")) != NULL) {
    // Is it a .BMP file?
    rewind(fp);
    if ((image = img_is_bmp(fp, x, y, images)) != NULL) {
      fclose(fp);
      return image;
    }

    // Is it a .ICO file?
    rewind(fp);
    if ((image = img_is_ico(fp, x, y, delay, images)) != NULL) {
      fclose(fp);
      return image;
    }
    
    // is it a .PCX file?
    rewind(fp);
    if ((image = img_is_pcx(fp, x, y, images)) != NULL) {
      fclose(fp);
      return image;
    }
    
    // is it a .GIF file?
    rewind(fp);
    if ((image = img_is_gif(fp, x, y, delay, images)) != NULL) {
      fclose(fp);
      return image;
    }
  }
  
  // Either an unknown image type, or file not found
  if (!fail) {
    image = (PIXEL *) malloc(32 * 32 * sizeof(PIXEL));
    for (unsigned i=0; i<(32*32); i++)
      image[i] = GUICOLOR_white;
  
    if (x) *x = 32;
    if (y) *y = 32;
    if (images) *images = 1;
  }
  
  fclose(fp);
  
  return image;
}

/* get_bit()
 *      p = pointer to starting byte to check
 *      i = bit index
 *  
 * returns the bit value of the bit 'i' away from offset 'p'
 *  i = 0 = bit 7 of first byte
 */
bool get_bit(const bit8u *p, const int i) {
  return (p[(i / 8)] & (0x80 >> (i % 8))) > 0;
}

/* get_nibble()
 *      p = pointer to starting byte to check
 *      i = nibble index
 *  
 * returns the nibble 'i' away from offset 'p'
 */
int get_nibble(const bit8u *p, const int i) {
  if (i & 1)
    return (p[(i / 2)] & 0x0F) >> 0;
  else
    return (p[(i / 2)] & 0xF0) >> 4;
}
