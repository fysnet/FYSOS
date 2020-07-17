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
 *  images.cpp
 *   This file tries to open an image file of .BMP, .ICO, .GIF, .PCX, etc.
 *    then returns an array of PIXEL data if found
 *
 *  Last updated: 17 July 2020
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
