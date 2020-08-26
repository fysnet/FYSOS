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
 *  VIDEO.C
 *  This is a helper C source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit only
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.C
 */

typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,  // xBGR
  PixelBlueGreenRedReserved8BitPerColor,  // xRGB
  PixelBitMask,                           // use masks in EFI_PIXEL_BITMASK
  PixelBltOnly,                           // does not have a LFB
  PixelFormatMax                          // marks highest value for this enum (not a valid format number)
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
  bit32u MaxMode;  // Max Mode number
  bit32u Mode;     // current mode number
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  bit32u SizeOfInfo;         // size of the structure above
  bit32u FrameBufferBase[2]; // frame buffer physical address
  bit32u FrameBufferSize;    // frame buffer in bytes
};

struct EFI_GUID GraphicsOutputProtocol = {
  0x9042A9DE,
  0x23DC, 
  0x4A38, 
  {
    0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A
  }
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
  EFI_STATUS   (*QueryMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, bit32u ModeNumber, bit32u *SizeOfInfo, struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);
  EFI_STATUS   (*SetMode)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, bit32u ModeNumber);
  EFI_STATUS   (*Blt)(struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This, struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer, EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
                      bit32u SourceX, bit32u SourceY, bit32u DestinationX, bit32u DestinationY, bit32u Width, bit32u Height, bit32u Delta);
  struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} *gop;

EFI_STATUS GetVideoInfo(void) {
  EFI_STATUS Status;
  EFI_HANDLE *handle_buffer;
  bit32u handle_count = 0;
  bit32u mode_num, size_of_info;
  struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *gop_mode_info;
  
  Status = gSystemTable->BootServices->LocateHandleBuffer(ByProtocol, &GraphicsOutputProtocol, NULL, &handle_count, &handle_buffer);
  if (Status != EFI_SUCCESS) {
    printf(L"LocateHandleBuffer() failed\n");
    return Status;
  }
  
  // we currently assume there is only one video card and screen, so handle_buffer[0], the first handle is used.
  Status = gSystemTable->BootServices->HandleProtocol(handle_buffer[0], &GraphicsOutputProtocol, (void **) &gop);
  if (Status != EFI_SUCCESS) {
    printf(L"HandleProtocol() failed\n");
    return Status;
  }
  
  for (mode_num = 0; gop->QueryMode(gop, mode_num, &size_of_info, &gop_mode_info) == EFI_SUCCESS; mode_num++) {
    printf(L"Mode %2i:  %4i x %4i, pixel format: %i, pixels/scanline: %4i\n", 
      mode_num,
      gop_mode_info->HorizontalResolution, gop_mode_info->VerticalResolution,
      gop_mode_info->PixelFormat, 
      gop_mode_info->PixelsPerScanLine);
    if (gop_mode_info->PixelFormat == PixelBitMask)
      printf(L"      masks: %i %i %i %i\n", 
        gop_mode_info->PixelInformation.RedMask, gop_mode_info->PixelInformation.GreenMask, 
        gop_mode_info->PixelInformation.BlueMask, gop_mode_info->PixelInformation.ReservedMask);
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS SetVidMode(bit32u ModeNum) {
  return gop->SetMode(gop, ModeNum);
}

// This assumes 32-bits per pixel
void DrawBox(int left, int top, int right, int bottom, bit32u color) {
  bit32u pos, lfb = gop->Mode->FrameBufferBase[0];
  int x, y;
  
  for (y=top; y<=bottom; y++) {
    for (x=left; x<=right; x++) {
      pos = (y * (gop->Mode->Info->PixelsPerScanLine * 4)) + (x * (32 / 8));
      *((bit32u *) (lfb + pos)) = color;
    }
  }
}
