/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * image_pcx.cpp
 *  
 *  this file checks for .PCX files
 *
 *  TODO: this is not complete code. FIXME
 *
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
