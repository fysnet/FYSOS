/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Graphical User Interface, and is for that purpose only.
 *   You have the right to use it for learning purposes only.  You may
 *   not modify it for redistribution for any other purpose unless you
 *   have written permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#define GUIRGB565(c) (((((bit32u)c) & 0x00F80000) >> 8) | \
                      ((((bit32u)c) & 0x0000FC00) >> 5) | \
                      ((((bit32u)c) & 0x000000F8) >> 3))

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

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Screen Buffer Access
 *  We need to set up a descriptor pointing to the linear buffer of the screen
 *   memory so that we can write to it.
 */
__dpmi_meminfo base_mi;
int base_selector;

bool get_physical_mapping(__dpmi_meminfo *mi, int *selector) {
  int sel;
  
  if (__dpmi_physical_address_mapping(mi) == -1)
    return FALSE;
  sel = __dpmi_allocate_ldt_descriptors(1);
  if (sel == -1)
    return FALSE;
  if (__dpmi_set_segment_base_address(sel, mi->address) == -1)
    return FALSE;
  if (__dpmi_set_segment_limit(sel, mi->size - 1))
    return FALSE;
  
  *selector = sel;
  return TRUE;
}

bool vid_get_vid_info(struct S_VIDEO_SVGA *info) {
  bool ret = FALSE;
  
  __dpmi_regs r;
  bit32u dosbuf;
  int i;
  
  dosbuf = __tb & 0xFFFFF;
  
  for (i=0; i<sizeof(struct S_VIDEO_SVGA); i++)
    _farpokeb(_dos_ds, dosbuf + i, 0);
  
  dosmemput("VBE2", 4, dosbuf);
  
  r.x.ax = 0x4F00;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  __dpmi_int(0x10, &r);
  
  if (r.h.ah == 0x00) {
    ret = TRUE;
    
    // store the info
    if (info) {
      dosmemget(dosbuf, sizeof(struct S_VIDEO_SVGA), info);
      if (strncmp(info->VESASignature, "VESA", 4) != 0)
        ret = FALSE;
    }
  }
  
  return ret;
}

bool vid_get_mode_info(struct S_VIDEO_MODE_INFO *info, const bit16u mode) {
  bool ret = FALSE;
  
  __dpmi_regs r;
  int i;
  
  if (mode < 0x100)
    return FALSE;
  else {
    r.x.ax = 0x4F01;
    r.x.cx = mode;
    r.x.di = __tb & 0xF;
    r.x.es = __tb >> 4;
    __dpmi_int(0x10, &r);
    
    // Now check eax to see if supported.  If so, get next part.  etc....
    if (r.h.ah == 0) {
      // store the info
      if (info) {
        dosmemget(__tb, sizeof(struct S_VIDEO_MODE_INFO), info);
        
        // do some fixups
        switch (mode) {
          // these vid modes don't place the RGB mask size and bit positions in the buffer
          case 0x0100:
          case 0x0101:
          case 0x0103:
          case 0x0105:
            info->red_field_pos = 5;
            info->red_mask_size = 3;
            info->green_field_pos = 2;
            info->green_mask_size = 3;
            info->blue_field_pos = 0;
            info->blue_mask_size = 2;
            break;
          case 0x0116:
            info->bits_pixel = 15;
            break;
        }
      }
      ret = TRUE;
    }
  }
  
  return ret;
}

bool vid_set_mode(const int mode, const bool clr_mem) {
  
  __dpmi_regs r;
  bit32u dosbuf;
  
  dosbuf = __tb & 0xFFFFF;
  
  if (mode >= 0x100) {
    r.x.ax = 0x4F02;
    r.x.bx = (mode & 0x1FF) | (clr_mem ? 0 : (1<<15));
    r.x.di = dosbuf & 0xF;
    r.x.es = (dosbuf >> 4) & 0xFFFF;
    __dpmi_int(0x10, &r);
    
    return (r.x.ax == 0x004F);
  } else {
    r.x.ax = (mode & 0x7F) | (clr_mem ? 0 : (1<<7));
    __dpmi_int(0x10, &r);
    return TRUE;
  }
  
  return FALSE;
}
