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
#pragma pack (1)

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
    bit32u bi_compression;  // (for this utility, the remaining fields are ignored)
    bit32u bi_sizeimage;    //
    bit32u bi_x_pelspermeter;
    bit32u bi_y_pelspermeter;
    bit32u bi_clrused;      //
    bit32u bi_clrimportant; //
  } bmp_info;
};

char strtstr[] = "EXTRACT  v1.00.00    Forever Young Software 1984-2016\n";

void parse_command(int, char *[], char *);
