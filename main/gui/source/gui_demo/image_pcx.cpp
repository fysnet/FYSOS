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
 *  image_pcx.cpp
 *   This file checks for .PCX files
 *   TODO: this is not complete code. FIXME
 *
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"
#include "images.h"
#include "palette.h"

#include "image_pcx.h"

/*  img_is_pcx()
 *          fp = file pointer to opened file
 *           x = pointer to store width of bitmap
 *           y = pointer to store height of bitmap
 *      images = pointer to store count of images
 *
 *  checks to see if file is a .pcx file, and if so, returns an array of PIXEL data
 *
 */
PIXEL *img_is_pcx(FILE *fp, int *width, int *height, int *images) {
  PIXEL *image = NULL;
  PIXEL *palette = NULL;
  
  bit8u *line;
  struct PCX_HDR pcx_hdr;
  int w = 0, h = 0,
      x, y, p,
      datapos = 0,
      scanline;
  
  // read in header
  if (!fread(&pcx_hdr, 1, sizeof(struct PCX_HDR), fp)) {
    DEBUG((dfp, "\n Error reading from pcx file"));
    return NULL;
  }
  
  // is it a .PCX
  if ((pcx_hdr.flag != 0x0A) ||   // 10 = ZSoft
      (pcx_hdr.encoding != 1)     // 1 = RLE
     ) {
    return NULL;
  }
  
  // get the palette
  palette = pcx_get_palette(fp, &pcx_hdr);
  
  w = (int) (pcx_hdr.x_max - pcx_hdr.x_min + 1);
  h = (int) (pcx_hdr.y_max - pcx_hdr.y_min + 1);
  image = (PIXEL *) calloc(w * h, sizeof(PIXEL));

  line = (bit8u *) malloc(pcx_hdr.bytesperline * pcx_hdr.planes);
  
  // I don't know why we have to do this, but if the planes == 4, we
  //  only use 1 bit per pixel per plane....
  if (pcx_hdr.planes > 1)
    if ((w / pcx_hdr.bytesperline / 8) != pcx_hdr.bitsperpixel)
      pcx_hdr.bitsperpixel = (w / pcx_hdr.bytesperline / 8);
  
  if (image && line) {
    switch (pcx_hdr.planes) {
      case 1:  // 1 plane
        switch (pcx_hdr.bitsperpixel) {
          case 1:  // 1 bpp
            for (y=0; y<h; y++) {
              if (pcx_decode_line(fp, line, pcx_hdr.bytesperline) == -1) 
                goto done;
              for (x=0; x<w; x++)
                image[datapos++] = palette[get_bit(line, x)];
            }
            break;
          case 2:  // 2 bpp
            for (y=0; y<h; y++) {
              if (pcx_decode_line(fp, line, pcx_hdr.bytesperline) == -1) 
                goto done;
              p = 0;
              for (x=0; x<w; x++) {
                image[datapos++] = palette[(get_bit(line, p) << 1) | get_bit(line, p + 1)];
                p += 2;
              }
            }
            break;
          case 8:  // 8 bpp
            for (y=0; y<h; y++) {
              if (pcx_decode_line(fp, line, pcx_hdr.bytesperline) == -1) 
                goto done;
              for (x=0; x<w; x++)
                image[datapos++] = palette[line[x]];
            }
            break;
        }
        break;
      case 4:  // 4 planes
        switch (pcx_hdr.bitsperpixel) {
          case 1:  // 1 bpp
            for (y=0; y<h; y++) {
              // no guarentee that there is a RLE break between planes, but
              // there is guarenteed to be between horz lines.
              if (pcx_decode_line(fp, line, pcx_hdr.bytesperline * 4) == -1) 
                goto done;
              for (p=0; p<pcx_hdr.planes; p++) {
                switch (p) {
                  case 0:
                    for (x=0; x<w; x++)
                      if (get_bit(line + (pcx_hdr.bytesperline * p), x))
                        image[datapos + x] |= 0x00FF0000;
                    break;
                  case 1:
                    for (x=0; x<w; x++)
                      if (get_bit(line + (pcx_hdr.bytesperline * p), x))
                        image[datapos + x] |= 0x0000FF00;
                    break;
                  case 2:
                    for (x=0; x<w; x++)
                      if (get_bit(line + (pcx_hdr.bytesperline * p), x))
                        image[datapos + x] |= 0x000000FF;
                    break;
                  case 3:
                    break;
                }
              }
              datapos += w;
            }
            break;
        }
        break;
    }
  }
  
done:
  if (width) *width = w;
  if (height) *height = h;
  if (images) *images = 1;  // pcx files only have one image per file
  
  if (palette)
    free(palette);
  if (line)
    free(line);
  
  return image;
}


PIXEL *pcx_get_palette(FILE *fp, struct PCX_HDR *pcx_hdr) {
  int i, j;
  
  if (pcx_hdr->planes > 1)
    return NULL;
  
  PIXEL *palette = (PIXEL *) malloc(256 * sizeof(PIXEL));
  if (!palette)
    return NULL;
  
  if ((pcx_hdr->bitsperpixel == 8) && (pcx_hdr->version >= 5)) {
    const long cur = ftell(fp);
    fseek(fp, -769, SEEK_END);
    if (fgetc(fp) == 12) {
      for (i=0; i<256; i++)
        palette[i] = (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
    }
    // restore the file position
    fseek(fp, cur, SEEK_SET);
  } else {
    for (i=0, j=0; i<16; i++, j+=3)
      palette[i] = (pcx_hdr->palette[j] << 16) | (pcx_hdr->palette[j+1] << 8) | (pcx_hdr->palette[j+2] << 0);
  }
  
  return palette;
}

// decode a line of pixels
int pcx_decode_line(FILE *fp, bit8u *line, const int count) {
  int c, ch, i = 0;
  
  while (i < count) {
    c = fgetc(fp);
    if (c == EOF)
      return -1;
    if ((c & 0xC0) == 0xC0) {
      c &= 0x3F;
      ch = fgetc(fp);
      if (ch == EOF)
        return -1;
    } else {
      ch = c;
      c = 1;
    }
    // this assumes that the PCX is correctly formated and
    //  that the RLE ends at the end of the line
    while (c--)
      line[i++] = (bit8u) ch;
  }
  return 0;
}
