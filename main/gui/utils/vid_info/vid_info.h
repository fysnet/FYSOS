/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * vid_info.h
 *  
 */
#ifndef FYSOS_VID_INFO
#define FYSOS_VID_INFO

#pragma pack(1)

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
  bit16u win_func_ptr;         // pointer to window function
  bit16u win_func_seg;         // 
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

bool vid_get_vid_info(struct S_VIDEO_SVGA *);
int vid_get_vid_modes(const bit16u, const bit16u, bit16u *);
void vesa_mode_enum(bit16u *, const int);
bool vid_get_mode_info(struct S_VIDEO_MODE_INFO *, const bit16u);

const char *do_yesno(const bit32u);
const char *do_string(const bit16u, const bit16u);
const char *do_mem_model(const int);

#endif  // FYSOS_VID_INFO
