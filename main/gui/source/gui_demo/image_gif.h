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
 *  image_gif.h
 *
 *  Last updated: 17 July 2020
 */

#ifndef FYSOS_IMAGE_GIF
#define FYSOS_IMAGE_GIF

#pragma pack(1)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * GIF Headers
 *  
 */
#define GCT_FLAG             0x80    // Global-Color-Table present
#define GCT_SIZE_MASK        0x07    // GCT size
#define TID_FLAG_INTERLACED  0x40    // Image is interlaced

struct GIF_HDR {
  bit8u  sig[3];        // "GIF"
  bit8u  ver[3];        // "87a" or "89a"
};

// Logical Screen Descriptor
struct GIF_LSD {
  bit16u width;
  bit16u height;
  bit8u  flags;
  bit8u  background;
  bit8u  pixel_ratio;
};

// The Image Descriptor
struct GIF_TID {
  bit8u  seperator;       // 0x2C
  bit16u img_left_pos;
  bit16u img_top_pos;
  bit16u img_width;
  bit16u img_height;
  bit8u  flags;
};

// Extention Introducer
struct GIF_CNTRL_EXT {
  bit8u  flags;           //  (bits 4:2) disposal, (bit 1) user input, and (bit 0) transperency
  bit16u delay_time;      // in jiffies
  bit8u  trans_indx;      // palette number of trans color
};

struct GIF_CNTRL_PLAIN_TEXT {
  bit16u grid_left_pos;
  bit16u grid_right_pos;
  bit16u grid_width;
  bit16u grid_height;
  bit8u  char_width;
  bit8u  char_height;
  bit8u  fore_indx;
  bit8u  back_indx;
};

struct GIF_APP_DATA {
  bit8u  id[8];
  bit8u  auth_code[3];
};


bit8u *lzw_decompress(bit8u *, bit32u, bit32u, bit32u *);
PIXEL *img_is_gif(FILE *, int *, int *, int *, int *);



#endif // FYSOS_IMAGE_GIF
