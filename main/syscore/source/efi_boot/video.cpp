/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  VIDEO.CPP
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.CPP
 */

#define EFI_VIDEO_DEBUG

#pragma warning(disable: 4474)
#pragma warning(disable: 4476)

#include "ctype.h"
#include "efi_common.h"

#include "conout.h"
#include "video.h"

#ifdef EFI_VIDEO_DEBUG
  #include "stdlib.h"
#endif

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
  UINTN handle_count = 0;
  UINTN size_of_info;
  UINT32 mode_num, Cnt = 0;
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gop_mode_info;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &GraphicsOutputProtocol, NULL, &handle_count, &handle_buffer);
  if (EFI_ERROR(Status)) {
    printf(L"LocateHandleBuffer() failed\r\n");
    return 0;
  }
  
  Status = gBS->HandleProtocol(handle_buffer[0], &GraphicsOutputProtocol, (void **) &gop);
  if (EFI_ERROR(Status)) {
    printf(L"HandleProtocol() failed\r\n");
    return 0;
  }
  
  for (mode_num = 0; (Status = gop->QueryMode(gop, mode_num, &size_of_info, &gop_mode_info)) == EFI_SUCCESS; mode_num++) {
#ifdef EFI_VIDEO_DEBUG
    printf(L"Found Video Mode:  %i x %i  (%i) (0x%08X 0x%08X 0x%08X) %i\r\n", gop_mode_info->HorizontalResolution, gop_mode_info->VerticalResolution,
      gop_mode_info->PixelFormat, gop_mode_info->PixelInformation.RedMask, gop_mode_info->PixelInformation.GreenMask, gop_mode_info->PixelInformation.GreenMask,
      gop_mode_info->PixelsPerScanLine);
#endif
    mode_info[Cnt].lfb = (bit64u) gop->Mode->FrameBufferBase;
    mode_info[Cnt].xres = (bit16u) gop_mode_info->HorizontalResolution;
    mode_info[Cnt].yres = (bit16u) gop_mode_info->VerticalResolution;
    switch (gop_mode_info->PixelFormat) {
      case PixelRedGreenBlueReserved8BitPerColor:
        mode_info[Cnt].red = 0x0008;
        mode_info[Cnt].blu = 0x0808;
        mode_info[Cnt].grn = 0x1008;
        mode_info[Cnt].bits_per_pixel = sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * 8;
        mode_info[Cnt].bytes_per_scanline = (bit16u) ((gop_mode_info->PixelsPerScanLine * sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL)));
        mode_info[Cnt].memory_model = 6;  // this is a non-standard format, so use the bitmasks
        break;
      case PixelBlueGreenRedReserved8BitPerColor:
        mode_info[Cnt].red = 0x1008;
        mode_info[Cnt].blu = 0x0808;
        mode_info[Cnt].grn = 0x0008;
        mode_info[Cnt].bits_per_pixel = sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * 8;
        mode_info[Cnt].bytes_per_scanline = (bit16u) ((gop_mode_info->PixelsPerScanLine * sizeof(struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL)));
        mode_info[Cnt].memory_model = 4;  // this is a normal format for us
        break;
      case PixelBitMask:
        mode_info[Cnt].red = bitmask_convert(gop_mode_info->PixelInformation.RedMask);
        mode_info[Cnt].blu = bitmask_convert(gop_mode_info->PixelInformation.BlueMask);
        mode_info[Cnt].grn = bitmask_convert(gop_mode_info->PixelInformation.GreenMask);
        mode_info[Cnt].bits_per_pixel = GetPixelSize(&gop_mode_info->PixelInformation);
        // GetPixelSize will return a value evenly divisible by 8 due to the pixel masks given
        mode_info[Cnt].bytes_per_scanline = (bit16u) ((gop_mode_info->PixelsPerScanLine * (mode_info[Cnt].bits_per_pixel / 8)));
        mode_info[Cnt].memory_model = 6;  // this is a non-standard format, so use the bitmasks
        break;
      case PixelBltOnly:
        // we don't support a LFB so don't include this mode in the list
        continue;
    }
    mode_nums[Cnt] = (bit16u) mode_num;
    if (++Cnt == VIDEO_MAX_MODES)
      break;
  }
  
  return (bit16u) Cnt;
}

bit16u FindVideoMode(struct S_MODE_INFO *mode_info, bit16u Cnt, int XRes, int YRes) {
  for (bit16u i=0; i<Cnt; i++)
    if ((mode_info[i].xres == XRes) && (mode_info[i].yres == YRes))
      return i;
  return 0xFFFF;
}

EFI_STATUS SetVidMode(bit32u ModeNum) {
  EFI_STATUS Status;
  
  Status = gop->SetMode(gop, ModeNum);
  if (!EFI_ERROR(Status))
    cls();
  
  return Status;
}

void DrawBox(int left, int top, int right, int bottom, bit32u color) {
  bit32u pos;
  int x, y;
  
  if (gop->Mode->FrameBufferBase == 0)
    return;
  
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      pos = (y * (gop->Mode->Info->PixelsPerScanLine * 4)) + (x * (32 / 8));
      *((bit32u *) (gop->Mode->FrameBufferBase + pos)) = color;
    }
  }
}
