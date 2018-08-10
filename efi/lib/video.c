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


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

#include "conout.h"
#include "video.h"



struct EFI_GUID GraphicsOutputProtocol = {
  0x9042A9dE,
  0x23DC, 
  0x4A38, 
  {
    0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A
  }
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *cur_mode_info = NULL;
bit32u *cur_lfb = NULL;

bit16u mode_nums[VIDEO_MAX_MODES];

// convert a bit32u full of bits to a starting position (high byte)
//  and count of bits set (low byte)
bit16u bitmask_convert(bit32u mask) {
  int start = 0, cnt = 0;
  int pos = 0;

  while ((pos < 32) && (!(mask & (1 << pos))))
    pos++;
  if (pos == 32)  // if we didn't find any, return 0x0000
    return 0x0000;
  start = pos;
  
  while ((pos < 32) && (mask & (1 << pos)))
    pos++, cnt++;
  
  return ((start << 8) | cnt);
}

// returns bit position of highest set bit or -1 if none set
int HighestSetBit(bit32u val) {
  int r = 0;
  if (!val)
    return -1;
  
  while (val >>= 1)
    r++;
  
  return r;
}

// returns size of pixel from given maskes
bit16u GetPixelSize(struct EFI_PIXEL_BITMASK *PixelInformation) {
  int HighestPixel = -1;
  int BluePixel = HighestSetBit(PixelInformation->BlueMask);
  int RedPixel = HighestSetBit(PixelInformation->RedMask);
  int GreenPixel = HighestSetBit(PixelInformation->GreenMask);
  int RsvdPixel = HighestSetBit(PixelInformation->ReservedMask);
  
  HighestPixel = ((BluePixel > RedPixel) ? BluePixel : RedPixel);           //MAX(BluePixel, RedPixel);
  HighestPixel = ((HighestPixel > GreenPixel) ? HighestPixel : GreenPixel); //MAX(HighestPixel, GreenPixel);
  HighestPixel = ((HighestPixel > RsvdPixel) ? HighestPixel : RsvdPixel);   //MAX(HighestPixel, RsvdPixel);
  
  return (bit16u) (HighestPixel + 1);
}

// http://forum.osdev.org/viewtopic.php?f=1&t=26796
bit16u GetVideoInfo(struct S_MODE_INFO *mode_info) {
  EFI_STATUS Status;
  EFI_HANDLE *handle_buffer;
  bit32u handle_count = 0;
  bit32u mode_num, size_of_info, Cnt = 0;
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gop_mode_info;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &GraphicsOutputProtocol, NULL, &handle_count, &handle_buffer);
  if (EFI_ERROR(Status)) {
    printf(L"%[LocateHandleBuffer() failed%]\r\n", ERROR_COLOR);
    return 0;
  }
  
  Status = gBS->HandleProtocol(handle_buffer[0], &GraphicsOutputProtocol, (void **) &gop);
  if (EFI_ERROR(Status)) {
    printf(L"%[HandleProtocol() failed%]\r\n", ERROR_COLOR);
    return 0;
  }
  
  for (mode_num = 0; (Status = gop->QueryMode(gop, mode_num, &size_of_info, &gop_mode_info)) == EFI_SUCCESS; mode_num++) {
//    printf(L"Found Video Mode:  %i x %i  (%i) (%i %i %i) %i\r\n", gop_mode_info->HorizontalResolution, gop_mode_info->VerticalResolution,
//      gop_mode_info->PixelFormat, gop_mode_info->PixelInformation.RedMask, gop_mode_info->PixelInformation.GreenMask, gop_mode_info->PixelInformation.GreenMask,
//      gop_mode_info->PixelsPerScanLine);
    mode_info[Cnt].lfb = gop->Mode->FrameBufferBase[0];
    mode_info[Cnt].xres = gop_mode_info->HorizontalResolution;
    mode_info[Cnt].yres = gop_mode_info->VerticalResolution;
    switch (gop_mode_info->PixelFormat) {
      case PixelRedGreenBlueReserved8BitPerColor:
        mode_info[Cnt].red = 0x0008;
        mode_info[Cnt].blu = 0x0808;
        mode_info[Cnt].grn = 0x1008;
        mode_info[Cnt].bits_per_pixel = sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * 8;
        mode_info[Cnt].bytes_per_scanline = (gop_mode_info->PixelsPerScanLine * sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        mode_info[Cnt].memory_model = 6;  // this is a non-standard format, so use the bitmasks
        break;
      case PixelBlueGreenRedReserved8BitPerColor:
        mode_info[Cnt].red = 0x1008;
        mode_info[Cnt].blu = 0x0808;
        mode_info[Cnt].grn = 0x0008;
        mode_info[Cnt].bits_per_pixel = sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * 8;
        mode_info[Cnt].bytes_per_scanline = (gop_mode_info->PixelsPerScanLine * sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        mode_info[Cnt].memory_model = 4;  // this is a normal format for us
        break;
      case PixelBitMask:
        mode_info[Cnt].red = bitmask_convert(gop_mode_info->PixelInformation.RedMask);
        mode_info[Cnt].blu = bitmask_convert(gop_mode_info->PixelInformation.BlueMask);
        mode_info[Cnt].grn = bitmask_convert(gop_mode_info->PixelInformation.GreenMask);
        mode_info[Cnt].bits_per_pixel = GetPixelSize(&gop_mode_info->PixelInformation);
        // GetPixelSize will return a value evenly divisible by 8 due to the pixel masks given
        mode_info[Cnt].bytes_per_scanline = (gop_mode_info->PixelsPerScanLine * (mode_info[Cnt].bits_per_pixel / 8));
        mode_info[Cnt].memory_model = 6;  // this is a non-standard format, so use the bitmasks
        break;
      case PixelBltOnly:
        // we don't support a LFB so don't include this mode in the list
        continue;
    }
    mode_nums[Cnt] = mode_num;
    if (++Cnt == VIDEO_MAX_MODES)
      break;
  }
  
  return Cnt;
}

bit16u FindVideoMode(struct S_MODE_INFO *mode_info, bit16u Cnt, int XRes, int YRes) {
  for (bit16u i=0; i<Cnt; i++)
    if ((mode_info[i].xres == XRes) && (mode_info[i].yres == YRes))
      return i;
  return 0xFFFF;
}

EFI_STATUS SetVidMode(bit32u ModeNum) {
  return gop->SetMode(gop, ModeNum);
}

/*
void DrawBox(int left, int top, int right, int bottom, bit32u color, struct S_MODE_INFO *info) {
  bit32u pos;
  int x, y;
  
  if (info->lfb == 0)
    return;
  
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      pos = (y * info->bytes_per_scanline) + (x * (info->bits_per_pixel / 8));
      *((bit32u *) (info->lfb + pos)) = color;
    }
  }
}
*/
