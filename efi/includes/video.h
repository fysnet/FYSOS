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
 *
 * Note:  Since this code uses wide chars (wchar_t), you *MUST* have my modified 
 *        version of SmallerC.  Contact me for more information.
 *        
 */

#ifndef VIDEO_H
#define VIDEO_H


#pragma pack(push, 1)

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

struct EFI_PIXEL_BITMASK {
  bit32u RedMask;
  bit32u GreenMask;
  bit32u BlueMask;
  bit32u ReservedMask;
};

struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL {
  bit8u Blue;
  bit8u Green;
  bit8u Red;
  bit8u Reserved;
};

typedef enum {
  EfiBltVideoFill,
  EfiBltVideoToBltBuffer,
  EfiBltBufferToVideo, 
  EfiBltVideoToVideo,
  EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
  bit32u Version;
  bit32u HorizontalResolution;
  bit32u VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  struct EFI_PIXEL_BITMASK PixelInformation;
  bit32u PixelsPerScanLine;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
  bit32u MaxMode;
  bit32u Mode;
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  bit32u SizeOfInfo;
  bit32u FrameBufferBase[2];
  bit32u FrameBufferSize;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
  EFI_STATUS   (*QueryMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, bit32u ModeNumber, bit32u *SizeOfInfo, struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);
  EFI_STATUS   (*SetMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, bit32u ModeNumber);
  EFI_STATUS   (*Blt)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
                      bit32u SourceX, bit32u SourceY, bit32u DestinationX, bit32u DestinationY, bit32u Width, bit32u Height, bit32u Delta);
  struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
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

#pragma pack(pop)


#define VIDEO_MAX_MODES  32
extern bit16u mode_nums[VIDEO_MAX_MODES];

extern struct EFI_GUID GraphicsOutputProtocol;



EFI_STATUS SetVidMode(bit32u ModeNum);
bit16u GetVideoInfo(struct S_MODE_INFO *mode_info);
bit16u FindVideoMode(struct S_MODE_INFO *mode_info, bit16u Cnt, int XRes, int YRes);


#endif // VIDEO_H
