/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * image_gif.h
 *  
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
