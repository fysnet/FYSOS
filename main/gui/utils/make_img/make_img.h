/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   GUI: The Graphic User Interface, and is for that purpose only.  You 
 *   have the right to use it for learning purposes only.  You may not
 *   modify it for redistribution for any other purpose unless you have
 *   written permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

// set it to 1 (align on byte)
#pragma pack(push, 1)

char strtstr[] = "MAKE_IMG  v1.00.00    Forever Young Software 1984-2016\n";


#define GUIRGB(r, g, b) ((((bit32u) (r)) << 16) | \
                        (( (bit32u) (g)) <<  8) | \
                        (( (bit32u) (b))))

#define GUITRGB(a, r, g, b) ((((bit32u) (a)) << 24) | \
                             (((bit32u) (r)) << 16) | \
                             (((bit32u) (g)) <<  8) | \
                             (((bit32u) (b))))

#define GUIT(c) ((bit8u) (((c) >> 24) & 0xFF))
#define GUIR(c) ((bit8u) (((c) >> 16) & 0xFF))
#define GUIG(c) ((bit8u) (((c) >> 8) & 0xFF))
#define GUIB(c) ((bit8u) (((c) & 0xFF)))


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


// this is used to parse the .BMP header
// .BMP header
struct BMP_FILE_HDR {
  struct BMP_HDR {
    bit16u bf_id;           // ascii 'BM' (B then M, not stored as MB)
    bit32u bf_size;         // size of file
    bit16u bf_resv0;        // should be zeros
    bit16u bf_resv1;        // should be zeros
    bit32u bf_offbits;      // offset in file where image begins
  } bmp_hdr;
  struct BMP_INFO {
    bit32u bi_size;         // this header size (must be 40)
    bit32u bi_width;        // width in pixels
    bit32u bi_height;       // height in pixels
    bit16u bi_planes;       // must be 1
    bit16u bi_bitcount;     // bits per pixel (must be 24 for this utility)
    bit32u bi_compression;  // 
    bit32u bi_sizeimage;    //
    bit32u bi_x_pelspermeter;
    bit32u bi_y_pelspermeter;
    bit32u bi_clrused;      //
    bit32u bi_clrimportant; //
  } bmp_info;
};


#define NAME_LEN_MAX  512

// resource file contents (after we parse it)
struct S_RESOURCE {
  bit32u file_cnt;
  char targ_filename[NAME_LEN_MAX];
  char id_filename[NAME_LEN_MAX];
  struct {
    char ID_name[64];
    char filename[NAME_LEN_MAX];
    bit32u param0;
    bit32u param1;
  } files[1];  // allocated later
};

void rle_compress(bit32u *, int, FILE *);

void parse_command(int, char *[], char *);
int resource_id(FILE *, const char *);
struct S_RESOURCE *parse_resource(const char *filename);

#pragma pack(pop)
