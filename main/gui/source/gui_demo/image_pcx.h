/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * image_pcx.h
 *  
 */
#ifndef FYSOS_IMAGE_PCX
#define FYSOS_IMAGE_PCX

#pragma pack(1)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * PCX Headers
 *  
 */
struct PCX_HDR {
  bit8u  flag;          // 10 (0x0A) = ZSoft
  bit8u  version;       // 0 = Version 2.5
                        // 2 = Version 2.8 w/palette information
                        // 3 = Version 2.8 w/o palette information
                        // 4 = Version 4.0 (PC Paintbrush for Windows)
                        // 5 = Version 3.0
  bit8u  encoding;      // 1 = RLE
  bit8u  bitsperpixel;  // bits per pixel per plane
  bit16u x_min;         //
  bit16u y_min;         // this is the rect to display
  bit16u x_max;         //
  bit16u y_max;         //
  bit16u horz_res;      // width DPI
  bit16u vert_res;      // height DPI
  bit8u  palette[48];   // 
  bit8u  resv0;         // reserved (0)
  bit8u  planes;        // color planes 
  bit16u bytesperline;  // bytes per line per plane
  bit16u hdrpaletteinterp; // palette type (1 = color/BW, 2 = greyscale)
  bit16u x_vidscrsize;  //
  bit16u y_vidscrsize;  //
  bit8u  filler[54];    //
};



PIXEL *img_is_pcx(FILE *, int *, int *, int *);
PIXEL *pcx_get_palette(FILE *, struct PCX_HDR *);
int pcx_decode_line(FILE *, bit8u *, const int);

#endif // FYSOS_IMAGE_PCX
