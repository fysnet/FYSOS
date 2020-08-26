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
 * Last update:  24 Aug 2018
 *
 * compile using Visual Studio 2008 or better
 *
 */

#ifndef VIDEO_H
#define VIDEO_H


typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

struct EFI_PIXEL_BITMASK {
  UINT32 RedMask;
  UINT32 GreenMask;
  UINT32 BlueMask;
  UINT32 ReservedMask;
};

struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL {
  UINT8 Blue;
  UINT8 Green;
  UINT8 Red;
  UINT8 Reserved;
};

typedef enum {
  EfiBltVideoFill,
  EfiBltVideoToBltBuffer,
  EfiBltBufferToVideo, 
  EfiBltVideoToVideo,
  EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
  UINT32 Version;
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  struct EFI_PIXEL_BITMASK PixelInformation;
  UINT32 PixelsPerScanLine;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
  UINT32 MaxMode;
  UINT32 Mode;
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN  SizeOfInfo;
  EFI_PHYSICAL_ADDRESS FrameBufferBase;
  UINTN  FrameBufferSize;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
  EFI_STATUS   (*QueryMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber, UINTN *SizeOfInfo, struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);
  EFI_STATUS   (*SetMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, UINT32 ModeNumber);
  EFI_STATUS   (*Blt)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
                      UINTN SourceX, UINTN SourceY, UINTN DestinationX, UINTN DestinationY, UINTN Width, UINTN Height, UINTN Delta);
  struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
};


#pragma pack(push, 1)

struct S_MODE_INFO {
  bit64u lfb;
  bit16u xres;
  bit16u yres;
  bit16u bytes_per_scanline;
  bit16u bits_per_pixel;
  bit8u  memory_model;
  bit16u red;  // red bit mask (high byte is bit position of lowest bit, low byte is count of bits used)
  bit16u blu;  //   (ditto)
  bit16u grn;  //   (ditto)
  bit8u  resv;
};

#pragma pack(pop)


#define VIDEO_MAX_MODES  64
extern bit16u mode_nums[VIDEO_MAX_MODES];

extern struct EFI_GUID GraphicsOutputProtocol;



EFI_STATUS SetVidMode(bit32u ModeNum);
bit16u GetVideoInfo(struct S_MODE_INFO *mode_info);
bit16u FindVideoMode(struct S_MODE_INFO *mode_info, bit16u Cnt, int XRes, int YRes);
void DrawBox(int left, int top, int right, int bottom, bit32u color);


#endif // VIDEO_H
