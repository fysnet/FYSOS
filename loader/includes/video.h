/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#ifndef _VIDEO_H
#define _VIDEO_H

extern bit16u vid_modes[];

#pragma pack(push, 1)

#define VIDEO_MAX_MODES  32
#define VESA_MODE_SIZE  64

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

struct S_MODE_INFO {
  bit32u lfb;
  bit16u xres;
  bit16u yres;
  bit16u bytes_per_scanline;
  bit16u bits_per_pixel;
  bit8u  memory_model;
  bit16u red;  // red bit mask (high byte is bit position of lowest bit, low byte is count of bits used)
  bit16u blu;  //   (ditto)
  bit16u grn;  //   (ditto)
  bit8u  resv[5];
};

struct S_EEDID {
  bit8u  header[8];
  struct {
    bit16u manuf_id;
    bit16u product_code;
    bit32u serial_number;
    bit8u  manuf_week;
    bit8u  manuf_year;
  } vendor;
  bit8u  version;
  bit8u  revision;
  bit8u  input_def;
  bit8u  max_horz_cm;
  bit8u  max_vert_cm;
  bit8u  gamma;
  bit8u  features;
  bit8u  color_bits[10];
  bit8u  est_timings1;
  bit8u  est_timings2;
  bit8u  est_timings_resv;
  bit16u stand_timing_id[8];
  bit8u  detailed_timing1[18];
  bit8u  detailed_timing2[18];
  bit8u  detailed_timing3[18];
  bit8u  detailed_timing4[18];
  bit8u  extentions;
  bit8u  crc;
};

#pragma pack(pop)

bit16u get_video_info(struct S_MODE_INFO *);
bit16u get_video_mode(struct S_MODE_INFO *, bit16u, int, int, int);
void vid_set_256_palette(const bool);

void get_video_eedid(void);

#endif   // _VIDEO_H
