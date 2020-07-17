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
 *  image_gif.cpp
 *   This file checks for .GIF files
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

#include "image_gif.h"

/*  img_is_gif()
 *          fp = file pointer to opened file
 *           x = pointer to store width of bitmap
 *           y = pointer to store height of bitmap
 *      images = pointer to store count of images
 *
 *  checks to see if file is a .gif file, and if so, returns an array of PIXEL data
 *
 */
PIXEL *img_is_gif(FILE *fp, int *x, int *y, int *delay, int *images) {
  
  bit8u  gif_type;
  const  bit32u filesize = (bit32u) file_size(fp);
  bit8u *file_ptr = (bit8u *) malloc(filesize);  // to hold file data
  bit8u *file = file_ptr;
  unsigned u;
  
  *images = 0;
  
  // read in the file
  if (fread(file, 1, filesize, fp) != filesize) {
    free(file_ptr);
    return NULL;
  }
   
  // is it a .GIF
  struct GIF_HDR *gif_hdr = (struct GIF_HDR *) file;
  file += sizeof(struct GIF_HDR);
  if (memcmp(gif_hdr->sig, "GIF", 3)) {
    free(file_ptr);
    return NULL;
  }
  
  // get the version of .gif
       if (!memcmp(gif_hdr->ver, "87a", 3)) gif_type = 0x87;
  else if (!memcmp(gif_hdr->ver, "89a", 3)) gif_type = 0x89;
  else {
    free(file_ptr);
    return NULL;
  }
  
  // Logical Screen Descriptor
  struct GIF_LSD *gif_lsd = (struct GIF_LSD *) file;
  file += sizeof(struct GIF_LSD);
  
  // Global-Color-Table (palette) if any
  bit8u *glob_palette = file;
  if (gif_lsd->flags & GCT_FLAG)
    file += (6 << (GCT_SIZE_MASK & gif_lsd->flags));
  
  // image loop starts here (we support a max of 255)
  struct GIF_CNTRL_EXT *gif_cntrl_ext = NULL;
  struct GIF_CNTRL_PLAIN_TEXT *plain_text = NULL;
  struct GIF_APP_DATA *app_data = NULL;
  bit8u *loc_palette, *targ, *src;
  struct GIF_TID *gif_tid;
  PIXEL *image = NULL;
  bit32u image_ptr = 0;
  bit32u ret_size, size, init_code_size;
  
  while ((*file != 0x3B) && (*images < 255)) {
    bit8u  block_size;
    bit8u *return_data;
    bit8u *palette;
    
    switch (*file) {
      case 0x21:  // Extension Introducer
        file++;
        switch (*file++) {
          case 0x01: // plain text extention
            //putstr("\n GIF: Plain Text Extention");  ///////
            block_size = *file++;
            plain_text = (struct GIF_CNTRL_PLAIN_TEXT *) file;
            file += sizeof(struct GIF_CNTRL_PLAIN_TEXT);
            while (block_size = *file++)
              file += block_size;
            break;
          case 0xF9:
            block_size = *file++;
            gif_cntrl_ext = (struct GIF_CNTRL_EXT *) file;
            file += (sizeof(struct GIF_CNTRL_EXT) + 1);
            delay[*images] = gif_cntrl_ext->delay_time + 10; // store as mS
            DEBUG((dfp, " Delay time in jiffies: %i", gif_cntrl_ext->delay_time));
            break;
          case 0xFE:  // comment
            //putstr("\n GIF: 0xFE: Comment: \n");  ///////
            while (block_size = *file++) {
              //char comment[256];  ///////
              //memcpy(comment, file, block_size);  ///////
              //comment[block_size] = 0;  ///////
              //DEBUG((dfp, "%s", comment));  ///////
              file += block_size;
            }
            break;
          case 0xFF: // application extention
            block_size = *file++;
            app_data = (struct GIF_APP_DATA *) file;
            file += sizeof(struct GIF_APP_DATA);
            while (block_size = *file++)
              file += block_size;
            break;
          default:
            DEBUG((dfp, "\n GIF: Unknown Extention 0x%02X", *(file-1)));
        }
        break;
        
      case 0x2C: // The Image Descriptor (TID)
        gif_tid = (struct GIF_TID *) file;
        file += sizeof(struct GIF_TID);
        x[*images] = gif_tid->img_width;
        y[*images] = gif_tid->img_height;
        (*images)++;
        
        // Local-Color-Table (palette) if any
        loc_palette = NULL;
        if (gif_tid->flags & GCT_FLAG) {
          loc_palette = file;
          file += (6 << (GCT_SIZE_MASK & gif_tid->flags));
        }
        
        // resize the image block
        image = (PIXEL *) realloc(image, (image_ptr + (gif_tid->img_width * gif_tid->img_height)) * sizeof(PIXEL));
        
        // Image data
        init_code_size = *file++;
        
        // First slide all the blocks together
        size = 0;
        targ = src = file;
        while (*src) {
          const bit32u temp = *src++;
          size += temp;
          memcpy(targ, src, temp);
          src += temp;
          targ += temp;
        }
        
        // Now decompress it
        return_data = lzw_decompress(file, size, init_code_size, &ret_size);
        file = src + 1;
        
        // now translate to the image buffer (ret_size = bytes to translate)
        // use the local palette if not NULL, or the global palette if is NULL
        palette = (loc_palette != NULL) ? loc_palette : glob_palette;
        // is the image interlaced?
        if (gif_tid->flags & TID_FLAG_INTERLACED) {
          static struct {
            unsigned start;
            unsigned increment;
          } pass_info[4] = { {0, 8}, {4, 8}, {2, 4}, {1, 2} };
          unsigned x0, y0, pass;
          u = 0;
          for (pass=0; ((pass<4) && (u<ret_size)); pass++) {
            for (y0=pass_info[pass].start; ((y0<gif_tid->img_height) && (u<ret_size)); y0 += pass_info[pass].increment) {
              const bit32u offset = (y0 * gif_tid->img_width);
              for (x0=0; ((x0<gif_tid->img_width) && (u<ret_size)); x0++) {
                const bit32u tripplet = (return_data[u] * 3);
                if (gif_cntrl_ext && (gif_cntrl_ext->flags & 0x01) && (gif_cntrl_ext->trans_indx == return_data[u]))
                  image[image_ptr+offset+x0] = GUICOLOR_transparent;
                else
                  image[image_ptr+offset+x0] = GUIRGB(palette[tripplet+0], palette[tripplet+1], palette[tripplet+2]);
                u++;
              }
            }
          }
        } else {
          for (u=0; u<ret_size; u++) {
            const bit32u tripplet = (return_data[u] * 3);
            if (gif_cntrl_ext && (gif_cntrl_ext->flags & 0x01) && (gif_cntrl_ext->trans_indx == return_data[u]))
              image[image_ptr+u] = GUICOLOR_transparent;
            else
              image[image_ptr+u] = GUIRGB(palette[tripplet+0], palette[tripplet+1], palette[tripplet+2]);
          }
        }
        free(return_data);
        
        // update for next image
        image_ptr += ret_size;
        
        // clear out the data items
        gif_cntrl_ext = NULL;
        plain_text = NULL;
        app_data = NULL;
        break;
    }
  }
  
  // free the file data buffer
  free(file_ptr);  // buffer holding file data
  
  return image;
}

struct node_t {
  int prev;
  bit8u color;
};

struct compress_node_t {
  int down;
  int right;
  bit8u color;
};

/*  lzw_decompress()
 *
 *  used in .gif code above to decompress the image
 *
 */
bit8u *lzw_decompress(bit8u *src, bit32u src_len, bit32u code_size, bit32u *ret_val) {
  
  bit8u *targ = (bit8u *) malloc(65536);
  bit32u targ_capacity = 65536;
  bit32u ret_size = 0;
  
  bit8u temp_buffer[4097];
  struct node_t node[4097];
  int t, r, x, y;
  unsigned int curr_code_size, data_count;
  unsigned int holding, bitptr;
  int code, old_code, next_code, start_table_size;
  int clear_code, eof_code, mask;
  unsigned int ptr, fileptr;
  
  start_table_size = (1 << code_size);
  
  for (t=0; t<start_table_size; t++) {
    node[t].prev = -1;
    node[t].color = t;
  }
  
  clear_code = t;
  eof_code = t+1;
  next_code = t+2;
  
  code_size++;
  
  x = 0;
  y = 0;
  holding = 0;
  bitptr = 0;
  curr_code_size = code_size;
  old_code = -1;
  
  mask = (1 << curr_code_size) - 1;
  data_count = 0;
  
  ptr = 0;
  fileptr = 0;
  
  while (1) {
    while (bitptr < curr_code_size) {
      t = src[fileptr++];
      holding += (t << bitptr);
      bitptr += 8;
    }
    
    if (fileptr > src_len)
      break;
    
    code = holding & mask;
    
    holding >>= curr_code_size;
    bitptr -= curr_code_size;
    
    if (code == clear_code) {
      curr_code_size = code_size;
      next_code = start_table_size + 2;
      old_code = -1;
      mask = (1 << curr_code_size) - 1;
      continue;
    } else if (old_code == -1) {
      if (ret_size > (targ_capacity - 256))
        targ = (bit8u *) realloc(targ, targ_capacity += 65536);
      targ[ret_size++] = node[code].color;
    } else if (code == eof_code) {
      break;
    } else {
      if (code < next_code) {
        t = 0;
        r = code;
        while (1) {
          temp_buffer[t++] = node[r].color;
          if (node[r].prev == -1) break;
          r = node[r].prev;
        }
        
        if (ret_size > (targ_capacity - t - 256))
          targ = (bit8u *) realloc(targ, targ_capacity += 65536);
        for (r= t-1; r>=0; r--)
          targ[ret_size++] = temp_buffer[r];
        node[next_code].color = temp_buffer[t-1];
        node[next_code].prev = old_code;
        
        if ((next_code == mask) && (mask != 4095)) {
          curr_code_size++;
          mask = (1<<curr_code_size) - 1;
        }
        next_code++;
      } else {
        t = 0;
        r = old_code;
        while(1) {
          temp_buffer[t++] = node[r].color;
          if (node[r].prev == -1) break;
          r = node[r].prev;
        }
        node[next_code].color = temp_buffer[t-1];
        node[next_code].prev = old_code;
        
        if ((next_code == mask) && (mask != 4095)) {
          curr_code_size++;
          mask = (1<<curr_code_size)-1;
        }
        
        next_code++;
        
        if (ret_size > (targ_capacity - t - 1 - 256))
          targ = (bit8u *) realloc(targ, targ_capacity += 65536);
        for (r = t-1; r>=0; r--)
          targ[ret_size++] = temp_buffer[r];
        
        targ[ret_size++] = temp_buffer[t-1];
      }
    }
    
    old_code = code;
  }
  
  *ret_val = ret_size;
  return targ;
}

