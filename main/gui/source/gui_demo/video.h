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
 *  video.h
 *
 *  Last updated: 17 July 2020
 */

#ifndef FYSOS_VIDEO
#define FYSOS_VIDEO

#pragma pack(1)

#define MAX_MODE_COUNT  128   // max count of modes we support

// must be 512 bytes for VBE 2.0+
struct S_VIDEO_SVGA {
  char   VESASignature[4];     // 4 signature bytes
  bit16u VESAVersion;          // VESA version number
  bit16u oemptr;               // (Pointer to OEM string) from INT 10h
  bit16u oemseg;               //
  bit32u caps;                 // capabilities of the video environment
  bit16u svgamodesptr;         // (pointer to supported Super VGA modes)
  bit16u svgamodesseg;         // (pointer to supported Super VGA modes)
  bit16u memory;               // Number of 64kb memory blocks on board
  bit16u oem_version;          // OEM version (BCD)
  bit16u vendorptr;            // pointer to vendor name
  bit16u vendorseg;            // 
  bit16u productptr;           // pointer to product name
  bit16u productseg;           // 
  bit16u prodverptr;           // pointer to product version string
  bit16u prodverseg;           // 
  bit16u vbe_af_ver;           // if bit 3 set in caps
  bit16u vbemodesptr;          // 
  bit16u vbemodesseg;          // 
  bit8u  vbe_resv[216];        //
  bit8u  OEM_data[256];        //
};

struct S_VIDEO_MODE_INFO {
  bit16u mode_attrb;           // mode attributes
  bit8u  wina_attrb;           // window A attributes
  bit8u  winb_attrb;           // window B attributes
  bit16u win_granularity;      // window granularity (in k)
  bit16u win_size;             // window size
  bit16u wina_segment;         // window A start segment
  bit16u winb_segment;         // window B start segment
  bit32u win_func_ptr;         // pointer to window function
  bit16u bytes_scanline;       // bytes per scan line
  bit16u x_res;                // horizontal resolution
  bit16u y_res;                // vertical resolution
  bit8u  x_char_size;          // character cell width
  bit8u  y_char_size;          // character cell height
  bit8u  num_planes;           // number of memory planes
  bit8u  bits_pixel;           // bits per pixel
  bit8u  num_banks;            // number of banks
  bit8u  memory_model;         // memory model type
  bit8u  bank_size;            // bank size in kb
  bit8u  num_image_pages;      // number of images
  bit8u  resv1;                // reserved for page function
  bit8u  red_mask_size;        // size of direct color red mask in bits
  bit8u  red_field_pos;        // bit position of LSB of red mask
  bit8u  green_mask_size;      // size of direct color green mask in bits
  bit8u  green_field_pos;      // bit position of LSB of green mask
  bit8u  blue_mask_size;       // size of direct color blue mask in bits
  bit8u  blue_field_pos;       // bit position of LSB of blue mask
  bit8u  rsvd_mask_size;       // size of direct color reserved mask in bits
  bit8u  rsvd_field_pos;       // bit position of LSB of reserved mask
  bit8u  direct_color_mode;    // Direct Color mode attributes
  // vesa 2.0+
  bit32u linear_base;          // physical address of linear video buffer
  bit32u offscreen;            // pointer to start of offscreen memory
  bit16u offscreen_size;       // size of offscreen memory in k's
  // vesa 3.0+
  bit16u linear_b_scanline;    // bytes per scan line in linear modes
  bit8u  num_imgs_banked;      // number of images (less one) for banked video modes
  bit8u  num_imgs_linear;      // number of images (less one) for linear video modes
  bit8u  lm_red_mask_s;        // size of direct color red mask (in bits)
  bit8u  lm_red_mask_pos;      // bit position of red mask LSB (e.g. shift count)
  bit8u  lm_grn_mask_s;        // size of direct color green mask (in bits)
  bit8u  lm_grn_mask_pos;      // bit position of green mask LSB (e.g. shift count)
  bit8u  lm_blue_mask_s;       // size of direct color blue mask (in bits)
  bit8u  lm_blue_mask_pos;     // bit position of blue mask LSB (e.g. shift count)
  bit8u  lm_resv_mask_s;       // size of direct color reserved mask (in bits)
  bit8u  lm_resv_mask_pos;     // bit position of reserved mask LSB (e.g. shift count)
  bit32u max_pixel_cnt;        // maximum pixel clock for graphics video mode, in Hz
  bit8u  resv2[190];           // reserved
};

// Super VGA CRTC Information Block
struct S_VID_CRTC_INFO {
  bit16u horizontalTotal;      // Horizontal total in pixels
  bit16u horizontalSyncStart;  // Horizontal sync start in pixels
  bit16u horizontalSyncEnd;    // Horizontal sync end in pixels
  bit16u vericalTotal;         // Vertical total in pixels
  bit16u vericalSyncStart;     // Vertical sync start in pixels
  bit16u vericalSyncEnd;       // Vertical sync end in pixels
  bit8u  flags;                // Flags (Interlaced/DoubleScan/etc).
  bit32u pixelClock;           // Pixel clock in units of Hz
  bit16u refreshRate;          // Refresh rate in units of 0.01 Hz
  bit8u  reserved[40];         // Pad
};

struct S_VID_MODE {
  bit16u mode;
  bool   support; 
  bool   linear;     // TRUE if linear addressing
  bool   bankswitch; // TRUE if linear addressing
  bit8u  pages;
  bit8u  bits_pixel;
  bit16u width;   // in pixels
  bit16u height;  // in pixels
  bit8u  t_width;
  bit8u  t_height;
  bit8u  c_width;
  bit8u  c_height;
	bit32u vid_memory;
	bit32u vid_mem_size;
  struct S_VIDEO_MODE_INFO *mode_info;
};

// this is the systems video mode struct.
// ('* 2' so that we can have each mode allow linear or bankswitch)
struct S_SYS_VIDEO {
  int    mode_count;                                           // total count of vid_modes entries
  struct S_VIDEO_MODE_INFO vid_mode_info[MAX_MODE_COUNT * 2];  // supported video modes.
  int    current_mode_index;                                   // index into "vid_modes" or "vga_modes" for current video mode of system
  struct S_VID_MODE modes[MAX_MODE_COUNT * 2];                 // all supported modes;
};


void vesa_mode_enum(struct S_SYS_VIDEO *, bit16u *, const bool);
int vid_mode_valid(const bit16u);
bool vid_get_mode_info(struct S_VIDEO_MODE_INFO *, const bit16u);
bool vid_get_vid_info(struct S_VIDEO_SVGA *);

struct S_VIDEO_MODE_INFO *vid_get_info(bit16u);
int vid_get_vid_modes(struct S_VIDEO_SVGA *, bit16u *);

bool vid_set_mode(const int, const bool, const bool);
void vid_set_text_mode(const int);

void vid_set_256_palette(const bool);


#endif  // FYSOS_VIDEO

