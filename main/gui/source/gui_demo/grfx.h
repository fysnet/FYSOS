//////////////////////////////////////////////////////////////////////////
//  grfx.h  v1.00  27 Jan 2008
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS_GRFX
#define FYSOS_GRFX

#pragma pack(1)


#include "gui.h"
#include "stdio.h"

typedef void (*DRAW_FUNC)   (const struct RECT *, PIXEL *, const int);
typedef void (*REDRAW_FUNC) (const struct RECT *);
typedef void (*SCREEN_FUNC) (const int, const int, const int, const PIXEL *);

extern struct IMAGE desktop_img;

// Basic graphic drawing
struct GFX_ARGS {
  int w, h, c;
  struct BITMAP *buffer;
  REDRAW_FUNC redraw;
  SCREEN_FUNC screen_out;
  
  // Information variables
  struct RECT screen;
  bool   showpointer;
  bool   showfocus;
  int    bytes_scanline;  // calculated bytes per scan line for current mode (linear only)
};

// System related functions
void gfx_stop(void);

void gfx_show_focus(const bool);

// Screen updating functions
void gfx_redraw(REDRAW_FUNC);
void gfx_dirty(const struct RECT *);
void gfx_poll(void);

// Interface related functions
bool gfx_pointer(int *, int *, int *, int *);
bool gfx_hidepointer(const bool);
bool gfx_key(bit16u *, bit8u *, bit8u *);

// Bitmaps
// Must remain in this order for GUIBITMAP_DECLARE below
struct BITMAP {
  union {
    struct RECTATOM rectatom;
    struct ATOM atom;
  } base;
  int pitch;
  struct RECT clip;
  
  bit8u  count;                  // count of images (256 max allowed)
  bit8u  current;                // current image being displayed
  bit16u delay_array[256];       // jiffies delay before rendering next image
  int    count_down;             // jiffy count down of current images (if count > 0)
  
  PIXEL *array;
};

// this must match the above in items
#define GUIBITMAP_DECLARE(a, w, h)                                                    \
  {                                        /* struct BITMAP {                   */    \
    {                                      /*   union {                         */    \
      {                                    /*     struct RECTATOM {             */    \
        {                                  /*       union {                     */    \
          {                                /*         struct ATOM {             */    \
            NULL,                          /*           ATOM_DESTRUCT destruct; */    \
            0,                             /*           unsigned int lock;      */    \
            FALSE,                         /*           bool deleting;          */    \
            FALSE                          /*           bool allocated;         */    \
          }                                /*         };                        */    \
        },                                 /*       } base;                     */    \
        {                                  /*       struct RECT {               */    \
          0,                               /*         left                      */    \
          0,                               /*         top                       */    \
          ((w) - 1),                       /*         right                     */    \
          ((h) - 1)                        /*         bottom                    */    \
        }                                  /*       };                          */    \
      }                                    /*     };                            */    \
                                           /*     struct ATOM {                 */    \
                                           /*       ATOM_DESTRUCT destruct;     */    \
                                           /*       unsigned int lock;          */    \
                                           /*       bool deleting;              */    \
                                           /*       bool allocated;             */    \
                                           /*     };                            */    \
    },                                     /*   } base                          */    \
    (w),                                   /*   pitch                           */    \
    {                                      /*   struct RECT {                   */    \
      0,                                   /*     left                          */    \
      0,                                   /*     top                           */    \
      ((w) - 1),                           /*     right                         */    \
      ((h) - 1)                            /*     bottom                        */    \
    },                                     /*   };                              */    \
    1,                                     /*   count                           */    \
    0,                                     /*   current                         */    \
    { 0, },                                /*   delay_array[]                   */    \
    0,                                     /*   count_down                      */    \
    (a)                                    /*   PIXEL *array                    */    \
  }                                        /*  };                               */


void bitmap_destroy(void *);
struct BITMAP *obj_bitmap(int, int, int);
struct BITMAP *bitmap_pcx_grayscale(const char *);
struct BITMAP *bitmap_copy(const struct BITMAP *);

bool bitmap_clip(struct BITMAP *, const struct RECT *);
const struct RECT *bitmap_clip_get(const struct BITMAP *);
void bitmap_offset(struct BITMAP *, int, int);

PIXEL *bitmap_iter(const struct BITMAP *, int, int);
PIXEL *bitmap_begin(const struct BITMAP *);
PIXEL *bitmap_end(const struct BITMAP *);

PIXEL bitmap_getpixel(const struct BITMAP *, int, int);

struct BITMAP *make_random_image(const int, const int);
void bitmap_pixel(struct BITMAP *, int, int, PIXEL);
void bitmap_vline(struct BITMAP *, int, int, int, PIXEL);
void bitmap_hline(struct BITMAP *, int, int, int, PIXEL);
void bitmap_line(struct BITMAP *, int, int, int, int, PIXEL);
void bitmap_rectfill(struct BITMAP *, int, int, int, int, PIXEL);
void bitmap_circle(struct BITMAP *, const int, const int, const int, int, const int, const PIXEL);
void bitmap_clear(struct BITMAP *, PIXEL);
void bitmap_blit(const struct BITMAP *, struct BITMAP *, int, int, int, int, int, int, const bool);
void bitmap_blitcopy(const struct BITMAP *, struct BITMAP *, int, int, int, int, int, int);
void bitmap_blitstretch(const struct BITMAP *, struct BITMAP *, int, int, int, int, int, int, int, int);
void bitmap_box(struct BITMAP *, int, int, int, int, int, PIXEL, PIXEL);
void bitmap_box_dotted(struct BITMAP *bitmap, const int, const int, const int, const int, const PIXEL);
void bitmap_frame(struct BITMAP *, int, int, int, int, int, PIXEL, PIXEL, PIXEL);
void bitmap_decorate(const struct BITMAP *, struct BITMAP *, int, int, int, int, int, int, const bool);
struct BITMAP *get_bitmap(const char *, const bool);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Static Images
 *  
 */
#define STATIC_IMAGES_MAGIC 0xDEADBEEF

// This is the header at the start of the target image file
// Only one of these exist in the whole file and is at the beginning
// it should remain 32 bytes in size
struct STATIC_IMAGES_HDR {
  bit32u magic;         // to make sure we are the .sys file
  bit32u total_size;    // total size of file in bytes (when uncompressed)
  bit32u total_images;  // count of images in this file
  bit8u  comp_type;     // 0 for no compression, 1 = simple RLE encoding, no other type allowed at this time
  bit8u  reserved[19];
};

// This is an entry header, with a count of them just after
//  the header above and before any image data.
// the offset field is a pointer to the start of the image data for this
//  entry.
// all fields are fixed constants.
#define STATIC_NAME_LEN  (64-20)  // 20 being the count of bytes before name[]
struct STATIC_IMAGE {
  bit32u id;
  bit32u width;
  bit32u height;
  bit32u offset;  // offset in file where image data starts
  bit8u  count;
  bit8u  rsvd[3];
  char   name[STATIC_NAME_LEN];
};

bool load_static_images(const char *);
void free_static_images(void);
struct BITMAP *get_static_bitmap(const unsigned, const unsigned, const unsigned);


// Some useful macros for bitmap functions
extern struct GFX_ARGS grx_args;
#define GUISCREEN (grx_args.buffer)


//////////////////////////////////////////////////
// font

// width of our unknown char block character
#define UNKNOWN_CHAR_WIDTH 8

typedef void (*FONT_DRAWER) (struct BITMAP *, const struct RECT *, const struct RECT *, const bit8u *, bit32u, PIXEL, PIXEL);

struct FONT_INFO {
  bit16u index;   // Indices in data of each character
  bit8u  width;   // Width of this character
  bit8u  flags;   // nothing at the moment
  char   deltax;  // +/- offset to print char 
  char   deltay;  // +/- offset to print char (allows for drop chars, etc)
  char   deltaw;  // +/- offset to combine with width above when moving to the next char
  bit8u  resv;    // reserved
};

struct FONT {
  bit8u   sig[4];       // 'FONT'
  bit8u   height;       // height of char set
  bit8u   max_width;    // width of widest char in set
  bit16u  start;        // starting asciiz value (first entry in font == this ascii value)
  bit16u  count;        // count of chars in set ( 0 < count <= 256 )
  bit32u  datalen;      // len of the data section in bytes
  bit32u  total_size;   // total size of this file in bytes
  bit32u  flags;        // bit 0 = fixed width font, remaining bits are reserved
  bit8u   resv[14];     // reserved
  char    name[16];     // 15 chars, 1 null
  FONT_DRAWER drawchar;       // Callbacks for handling fonts
  struct FONT *prev;          // linked chain of fonts
  struct FONT *next;          // 
  //struct FONT_INFO info[];  // char info
  //bit8u data[];     // char data
};

void font_default(struct FONT *);
void font_insert(struct FONT *);
void font_list_destroy();
struct FONT *font_load(const char *, const bool);
const struct FONT *font_find_name(const char *);

void font_bitmap_draw(const struct FONT *, struct BITMAP *, const char *, int, int, int, PIXEL, PIXEL, const bit32u);
void font_bitmap_drawblock(const struct FONT *, struct BITMAP *, const char *, int, int, int, PIXEL, PIXEL, const bit32u);
bit32u font_height(const struct FONT *);
bit32u font_width(const struct FONT *, const char *, int, const bool);
void font_blocksize(const struct FONT *, struct TEXTUAL *, int *, int *);
bool font_bitmap_get_pos(const struct FONT *, const char *, int, const int, struct RECT *, int *);
bit8u *build_block_char(bit32u);
bit8u *font_do_underline(bit8u *, bit32u, const int, const int);
bit8u *font_rotate_char(const bit8u *, const int, const int, const int);

// Private font drawing functions
void font_bitchar(struct BITMAP *, const struct RECT *, const struct RECT *, const bit8u *, bit32u, PIXEL, PIXEL);


/////////////////////////////////////////////////////
#endif  // FYSOS_GRFX
