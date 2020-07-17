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
 *  image_bmp.cpp
 *   This file checks for .BMP and .ICO files
 *   TODO: this is not complete code. FIXME
 *
 *  Last updated: 17 July 2020
 */

#include <math.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"
#include "images.h"
#include "palette.h"

#include "image_bmp.h"


/***********************************************************
 *  .BMP file format
 */

/*  img_is_bmp()
 *          fp = file pointer to opened file
 *           x = pointer to store width of bitmap
 *           y = pointer to store height of bitmap
 *      images = pointer to store count of images
 *
 *  checks to see if file is a bmp file, and if so, returns an array of PIXEL data
 *
 */
PIXEL *img_is_bmp(FILE *fp, int *x, int *y, int *images) {

  struct {
    struct BMP_HDR hdr;
    struct BMP_INFO info;
  } block;
  struct BMP_COLOR_MAP palette[256];
  unsigned dx, dy, i, j, k, l, mask;
  
  // read in header and info block
  if (!fread(&block, 1, sizeof(block), fp))
    return NULL;
  
  if (block.hdr.bf_id != 0x4D42) return NULL;
  if (block.hdr.bf_resv0 || block.hdr.bf_resv1) return NULL;
  if (block.info.bi_size != 40) return NULL;
  
  // if bit count is not 1,4,8, or 24
  if ((block.info.bi_bitcount != 1) && (block.info.bi_bitcount != 4) &&
    (block.info.bi_bitcount != 8) && (block.info.bi_bitcount != 24)) return NULL;
  
  // is a BMP, so load it in to a buffer
  if (x) *x = (int) block.info.bi_width;
  if (y) *y = (int) block.info.bi_height;
  if (images) *images = 1;  // bmp files only have one image per file
  PIXEL *image = (PIXEL *) malloc(block.info.bi_width * block.info.bi_height * sizeof(PIXEL));
  if (image) {
    // get size of color map (palette)
    bit16u pal_size = 0;
    switch (block.info.bi_bitcount) {
      case 24:
        pal_size = 0;
        break;
      case 8:
        if (block.info.bi_clrused) pal_size = (bit16u) block.info.bi_clrused;
        else pal_size = 256;
        break;
      case 4:
        if (block.info.bi_clrused) pal_size =  (bit16u) block.info.bi_clrused;
        else pal_size = 16;
        break;
      case 1:
        pal_size = 2;
        break;
    }
    
    // read in color map
    if (pal_size)
      fread(palette, 1, (pal_size * sizeof(struct BMP_COLOR_MAP)), fp);
    
    fseek(fp, block.hdr.bf_offbits, SEEK_SET);
    bit8u *img = (bit8u *) malloc(block.hdr.bf_size - block.hdr.bf_offbits);
    fread(img, 1, block.hdr.bf_size - block.hdr.bf_offbits, fp);
    
    // now for the actual image (image starts from the right bottom pixel)
    if (block.info.bi_compression) {
      // image is rle compressed
      //  only 4- and 8-bit images are allowed to be rle'd ???
      
      /// TODO: decode it to a different buffer, then use that buffer
      
    } 
    
    switch (block.info.bi_bitcount) {
      case 24:
        i = 0;
        j = ((((block.info.bi_width * 3) + 3) & ~3) * (block.info.bi_height-1));
        for (dy=0; dy<block.info.bi_height; dy++) {
          k = j;
          for (dx=0; dx<block.info.bi_width; dx++) {
            image[i] = GUIRGB(img[k+2], img[k+1], img[k]);
            //if (image[i] == 0) image[i] = GUICOLOR_transparent;
            i++;
            k += 3;
          }
          j -= (((block.info.bi_width * 3) + 3) & ~3);
        }
        break;
        
      case 8:
        i = 0;
        j = (((block.info.bi_width + 3) & ~3) * (block.info.bi_height-1));
        for (dy=0; dy<block.info.bi_height; dy++) {
          k = j;
          for (dx=0; dx<block.info.bi_width; dx++) {
            image[i] = GUIRGB(palette[img[k]].red, palette[img[k]].green, palette[img[k]].blue);
            i++;
            k++;
          }
          j -= ((block.info.bi_width + 3) & ~3);
        }
        break;
        
      case 4:
        i = 0;
        j = ((((((block.info.bi_width + 1) & ~1) / 2) + 3) & ~3) * (block.info.bi_height-1));
        for (dy=0; dy<block.info.bi_height; dy++) {
          k = j;
          mask = 0;
          for (dx=0; dx<block.info.bi_width; dx++) {
            if ((mask & 1) == 0)
              l = (img[k] >> 4);
            else {
              l = (img[k] & 0x0F);
              k++;
            }
            image[i] = GUIRGB(palette[l].red, palette[l].green, palette[l].blue);
            i++;
            mask++;
          }
          j -= (((((block.info.bi_width + 1) & ~1) / 2) + 3) & ~3);
        }
        break;
        
      case 1:
        i = 0;
        j = ((((block.info.bi_width / 8) + 3) & ~3) * (block.info.bi_height-1));
        for (dy=0; dy<block.info.bi_height; dy++) {
          k = j;
          mask = 0x80;
          for (dx=0; dx<block.info.bi_width; dx++) {
            image[i] = (img[k] & mask) ? GUIRGB(palette[1].red, palette[1].green, palette[1].blue) : GUIRGB(palette[0].red, palette[0].green, palette[0].blue);
            i++;
            mask >>= 1;
            if (mask == 0) {
              mask = 0x80;
              k++;
            }
          }
          j -= (((block.info.bi_width / 8) + 3) & ~3);
        }
        break;
    }
    
    free(img);  
  }
  
  return image;
}


/***********************************************************
 *  .ICO file format
 */

// Calculate minimum number of double-words required to store width
// pixels where each pixel occupies bitCount bits. XOR and AND bitmaps
// are stored such that each scanline is aligned on a double-word
// boundary.
int scanline_bytes(const int width, const int bit_count) {
  return (((width * bit_count) + 31) / 32) * 4;
}


/*  img_is_ico()
 *          fp = file pointer to opened file
 *           x = pointer to store width of bitmap
 *           y = pointer to store height of bitmap
 *      images = pointer to store count of images
 *
 *  checks to see if file is a .ico file, and if so, returns an array of PIXEL data
 *
 */
PIXEL *img_is_ico(FILE *fp, int *x, int *y, int *delay, int *images) {
  struct ICO_HDR ico_hdr;
  struct ICO_ENTRY entry;
  struct BMP_INFO bmp_info;
  PIXEL *image = NULL;      // must be NULL for realloc() below
  bit8u *icon_data = NULL;  // must be NULL for realloc() below
  bit8u *xxor, *aand;
  bit32u next_entry_offset = sizeof(struct ICO_HDR);
  int data_len = 0, data_pos = 0,
      i, j, k, w, h,
      width = 0,
      height = 0,
      image_cnt = 0,
      color_count,
      scan_line;
  
  // read in the file
  if (fread(&ico_hdr, 1, sizeof(struct ICO_HDR), fp) != sizeof(struct ICO_HDR))
    return NULL;
  
  // check the file header
  // if reserved !=0, type != 1, or count < 1, then not an ico image
  if (ico_hdr.reserved || (ico_hdr.type != 1) || (ico_hdr.count == 0))
    return NULL;
  
  // loop through the images
  for (unsigned cnt=0; cnt < ico_hdr.count; cnt++) {
    // read the entry header
    fseek(fp, next_entry_offset, SEEK_SET);
    if (fread(&entry, 1, sizeof(struct ICO_ENTRY), fp) != sizeof(struct ICO_ENTRY))
      return image;
    next_entry_offset += sizeof(struct ICO_ENTRY);
    
    // the way our code is set up, we don't allow more than one image of different
    //  sizes.  therefore, stop when there is a size larger than the first.
    if ((image_cnt > 0) && ((entry.width > width) || (entry.height > height)))
      continue;
    
    // TODO: if we want to allow all images within this .ICO file, we could read
    //  in the whole entry table, get the largest size specified, then back up
    //  and make all entries that max size.  Then we wouldn't have to do the check
    //  above, or the update below.
    
    // allocate the memory for the icon data
    // (in case that icon_data == null pointer, realloc behaves like malloc)
    icon_data = (bit8u *) realloc(icon_data, entry.bytes_in_res);
    
    // read in at least 8 bytes so that we can check for BMP and PNG types
    // (if we read the whole BMP header, we don't have to back up and do it
    //  again if this time is found.  It is large enough to check for PNG
    //  type below, where we would have to back up, or at least cast a new type)
    fseek(fp, entry.image_offset, SEEK_SET);
    if (fread(&bmp_info, 1, sizeof(struct BMP_INFO), fp) != sizeof(struct BMP_INFO))
      break;
    
    // check for BMP type
    if (bmp_info.bi_size == 40) {
      if (fread(icon_data, 1, entry.bytes_in_res - sizeof(struct BMP_INFO), fp) != (entry.bytes_in_res - sizeof(struct BMP_INFO)))
        break;
      
      // update our width and height holders
      // only update on first image
      if (image_cnt == 0) {
        width = entry.width;
        height = entry.height;
      }
    
      // (re)allocate the memory for the PIXEL array
      // (in case that image == null pointer, realloc behaves like malloc)
      data_len += (width * height * sizeof(PIXEL));
      image = (PIXEL *) realloc(image, data_len);
    
      // we now have everything ready to translate.
      // icon_data holds the data to translate
      // image[data_pos] is the next PIXEL to write
      PIXEL *palette = (PIXEL *) icon_data;  // if bpp <= 256
      
      if (entry.width == 0)
        entry.width = (bit8u) bmp_info.bi_width;
      if (entry.height == 0)
        entry.height = (bit8u) (bmp_info.bi_height / 2);
      
      // calculate color count
      // If entry.color_count is 0, the number of colors is determined
      // from the planes and bit_count values. For example, the number
      // of colors is 256 when planes is 1 and bit_count is 8. Leave
      // entry.color_count set to 0 when planes is 1 and bit_count is 32.
      color_count = entry.color_count;
      if (color_count == 0) {
        if (bmp_info.bi_planes == 1) {
               if (bmp_info.bi_bitcount == 1) color_count = 2;
          else if (bmp_info.bi_bitcount == 4) color_count = 16;
          else if (bmp_info.bi_bitcount == 8) color_count = 256;
          else if (bmp_info.bi_bitcount != 32)
            color_count = (int) pow(2, bmp_info.bi_bitcount);
        } else
          color_count = (int) pow(2, bmp_info.bi_bitcount * bmp_info.bi_planes);
      }
      
      scan_line = scanline_bytes(entry.width, bmp_info.bi_bitcount);
      xxor = icon_data + (color_count * sizeof(PIXEL));
      aand = xxor + (scan_line * entry.height);
      for (i=0; i<(width * height); i++)
        image[data_pos + i] = GUICOLOR_transparent;
      j = 0;
      k = 0;
      for (h=0; h<entry.height; h++) {
        i = ((width * height) - (width * (h + 1)));
        for (w=0; w<entry.width; w++) {
          if (!get_bit(aand + k, w)) {
            switch (color_count) {
              case 2:
                image[data_pos + i] = palette[get_bit(xxor + j, w)];
                break;
              case 16:
                image[data_pos + i] = palette[get_nibble(xxor + j, w)];
                break;
              case 256:
                image[data_pos + i] = palette[((bit8u *) (xxor + j))[w]];
                break;
              case 0:
                bit8u *px = (xxor + j + (w * sizeof(PIXEL)));
                image[data_pos + i] = GUIRGB(px[2], px[1], px[0]);
                break;
            }
          } else
            image[data_pos + i] = GUICOLOR_transparent;
          i++;
        }
        j += scan_line;
        k += scanline_bytes(entry.width, 1);
      }
      
    // test for PNG type
    } else if (0) {
      bit8u *png = (bit8u *) &bmp_info;
      if ((png[0] == 0x89) &&
          (png[1] == 0x50) && (png[2] == 0x4E) && (png[3] == 0x47) &&  // 'PNG'
          (png[4] == 0x0D) && (png[5] == 0x0A) &&                      // CRLF   (DOS eol)
          (png[6] == 0x1A) &&                                          // stop, or eof in DOS using type command
          (png[7] == 0x0A)) {                                          // LF     (Linux eol)
      // found PNG type, so go back and read PNG data
      //fseek(fp, entry.image_offset, SEEK_SET);
      //if (fread(&bmp_info, 1, sizeof(struct BMP_INFO), fp) != sizeof(struct BMP_INFO))
      //  break;
      ///// I think only WinVista and above use this type....
        
        // for now, we just continue with the .ico file
        continue;
      }
      
    // else we don't know what it is, so skip over and continue to next one
    } else
      continue;
    
    if (x) x[0] = width;  // when we get animated images working correctly, we can change
    if (y) y[0] = height; //  these two lines to entry.width and entry.height...
    if (delay) delay[image_cnt] = 16;
    image_cnt++;
    data_pos += (height * width);
  }
  free(icon_data);
  
  if (images) *images = image_cnt;
  return image;
}
