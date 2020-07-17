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
 *  image_bmp.h
 *
 *  Last updated: 17 July 2020
 */

#ifndef FYSOS_IMAGE_BMP
#define FYSOS_IMAGE_BMP

#pragma pack(1)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * BMP Headers
 *  
 */
struct BMP_HDR {
  bit16u bf_id;           // ascii 'BM'
  bit32u bf_size;         // size of file
  bit16u bf_resv0;
  bit16u bf_resv1;
  bit32u bf_offbits;      // offset in file where image begins
};

struct BMP_INFO {
  bit32u bi_size;               // icon: used
  bit32u bi_width;              // icon: used
  bit32u bi_height;             // icon: used
  bit16u bi_planes;             // icon: used
  bit16u bi_bitcount;           // icon: used
  bit32u bi_compression;        // icon: not used
  bit32u bi_sizeimage;          // icon: used
  bit32u bi_x_pelspermeter;     // icon: not used
  bit32u bi_y_pelspermeter;     // icon: not used
  bit32u bi_clrused;            // icon: not used
  bit32u bi_clrimportant;       // icon: not used
};

struct BMP_COLOR_MAP {
  bit8u  blue;
  bit8u  green;
  bit8u  red;
  bit8u  resv;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * ICO headers
 *  
 */
struct ICO_HDR {
  bit16u  reserved;   // Reserved (must be 0)
  bit16u  type;       // Resource Type (1 for icons)
  bit16u  count;      // How many images?
};

struct ICO_ENTRY {
  bit8u   width;          // Width, in pixels, of the image (0 = 256)
  bit8u   height;         // Height, in pixels, of the image (0 = 256)
  bit8u   color_count;    // Number of colors in image (0 if >=8bpp)
  bit8u   reserved;       // Reserved ( must be 0)
  bit16u  planes;         // Color Planes
  bit16u  bit_count;      // Bits per pixel (if zero and non PNG, then we have to calculate from W x H)
  bit32u  bytes_in_res;   // How many bytes in this resource (includes BMP_INFO header)
  bit32u  image_offset;   // Where in the file (from start of file) is this image?
};


PIXEL *img_is_bmp(FILE *, int *, int *, int *);
PIXEL *img_is_ico(FILE *, int *, int *, int *, int *);


#endif // FYSOS_IMAGE_BMP
