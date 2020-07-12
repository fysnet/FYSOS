/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * image_bmp.h
 *  
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
