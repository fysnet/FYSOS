/*
 *                             Copyright (c) 1984-2025
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
 *             https://www.fysnet.net/osdesign_book_series.htm
 *
 *
 *  Last updated: 27 July 2025
 */

#define ENDIAN_16U(x)   ((((x) & 0xFF) <<  8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_32U(x)   ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8) | (((x) & 0xFF000000) >> 24))

#pragma pack(push, 1)

#ifdef _WIN64
  #define VERSION_INFO "Font Edit\nVersion 1.61.00 (64-bit)\n\nForever Young Software\n(C)opyright 1984-2025\n\nhttps://www.fysnet.net"
#else
  #define VERSION_INFO "Font Edit\nVersion 1.61.00 (32-bit)\n\nForever Young Software\n(C)opyright 1984-2025\n\nhttps://www.fysnet.net"
#endif

#define FTYPE_FONT   1
#define FTYPE_PSFv1  2
#define FTYPE_PSFv2  3
#define FTYPE_PF2    4

#define APP_WIDTH       700
#define APP_WIDTH_WIDE  975
#define APP_HEIGHT      700
#define APP_HEIGHT_TALL 800
#define APP_GET_HEIGHT ((font->height >= 20) ? APP_HEIGHT_TALL : APP_HEIGHT)

// max height and width of our font (app only, specs allow much more)
#define MINW   1
#define MINH   5
#define MAXW  24
#define MAXH  24

// max count of chars we allow (app only, specs allow much more)
#define MAX_COUNT 65536

// We currently don't have any way to know if we need to extend the memory used for the bitmap
//  when the user widens a char.  Therefore, we do the following:
// We allocate an extra amount of memory for the char bitmap array, so that when the user
//  widens a char, we still have enough memory to store the larger bitmap.
// However, we need to be sure and allocate enough extra.
//  The extreme case would be if the user created a MINW x MAXH x MAX_COUNT set, then 
//  widened each character to MAXW.  Therefore, the extra needs to be the following:
//   extra_bits = (((MAXW - MINW) * MAXH) * MAX_COUNT);  // in bits
// With the current settings above, this amounts to just over 4.3Meg of RAM tacked on to the end of the memory buffer.
//  Even older machines, mid 1990's, will allow this with ease :-)
#define MAX_EXTRA_MEM (((((MAXW - MINW) * MAXH) * MAX_COUNT) >> 3) + 64)  // 64 extra beyond just to make sure

#define FLAGS_FIXED_WIDTH  (1 << 0)

#define BOXSIZE   30   // size of a pixel box

#define GRIDSTART  175
#define BUTTONSRT  22

#define TEXTBOXW  135
#define TEXTBOXH  25   // should not be more than BOXSIZE

#define SLIDERW   100
#define SLIDERH    22

#define DIAG_DISABLE(x) (((x) > 0) ? -(x) : INT_MIN)
#define DIAG_ENABLE(x)    (x)
#define DIAG_VALUE(x)   (((x) == INT_MIN) ? 0 : abs(x))
#define DIAG_ENABLED(x)  ((x) > -1)

// font matrix structure
struct FONT_INFO {
  bit32u index;   // Indicies in data of each character
  bit8u  width;   // Width of character in pixels
  char   deltax;  // +/- offset to print char 
  char   deltay;  // +/- offset to print char (allows for drop chars, etc)
  char   deltaw;  // +/- offset to combine with width above when moving to the next char
  bit8u  resv[4]; // reserved
};

#define MAX_NAME_LEN  32

struct FONT {
  bit8u  sig[4];       // 'Font'
  bit8u  height;       // height of char set
  bit8u  max_width;    // width of widest char in set
  bit16u info_start;   // zero based offset to the first FONT_INFO block
  bit32s start;        // starting value (first entry in font == this value) (*** Signed, though must always be positive ***)
  bit32s count;        // count of chars in set ( 0 < count <= 0x10FFFF ) (*** Signed, though must always be positive ***)
  bit32u datalen;      // len of the data section in bytes
  bit32u total_size;   // total size of this file in bytes
  bit32u flags;        // bit 0 = fixed width font, remaining bits are reserved
  char   name[MAX_NAME_LEN]; // utf-8 null terminated
  bit8u  resv[36];     // reserved and preserved
};

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

struct PSFv1_FONT {
  bit8u  magic[2];     //  0x36 0x04
  bit8u  mode;         //  mode byte
  bit8u  charsize;     //  heigth of a char (width is fixed at 8)
};

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xB5
#define PSF2_MAGIC2     0x4A
#define PSF2_MAGIC3     0x86

struct PSFv2_FONT {
  bit8u  magic[4];
  bit32u version;
  bit32u headersize;    // offset of bitmaps in file
  bit32u flags;
  bit32u length;        // number of glyphs
  bit32u charsize;      // number of bytes for each character
  bit32u height;        // max height of glyphs
  bit32u width;         // max width of glyphs
};

// Grub Font
// http://grub.gibibit.com/New_font_format

struct PFF2_CHIX_HEADER {
  bit32u code_point;
  bit8u  flags;
  bit32u offset;
};

struct PFF2_DATA_HEADER {
  bit16u width;
  bit16u height;
  bit16u x_off;
  bit16u y_off;
  bit16u d_width;
};

#pragma pack(pop)

//  Declare  procedures
HWND CreateButton(HWND, DWORD, const char *, const int, const int);
HWND CreateTrackBar(HWND, const int, const int);

void SetTitleStr(const HWND);
void DisableItems(HMENU);
void EnableItems(HMENU, struct FONT *);
void DrawTextBox(HDC, DWORD, const int, const int, const int, const int, const char *);
int  LoadCurChar(HWND hwnd, struct FONT *font, const int ch, const bool update);
void SaveCurChar(struct FONT *font, const int ch, const int height);
void DumpFont(HWND, struct FONT *);
void FontMoveData(struct FONT *, int, int);

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
BOOL OpenFileDialog(HWND, LPTSTR);
BOOL SaveFileDialog(HWND, LPTSTR, LPCSTR, LPCTSTR, LPCTSTR);

struct FONT *InitFontData(struct FONT *, const int, const int, const int, const int, const int, const char *);
void SaveFile(HWND, struct FONT *);
struct FONT *OpenFile(HWND, struct FONT *);
struct FONT *OpenPFF2File(HWND hwnd, FILE *fp);

void CompressBitmap(bit8u *, bit8u *, const int, const int);
bit8u GetBit(bit8u *p, const int i);
void SetBit(bit8u *p, const int i, const bit8u value);
