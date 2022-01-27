/*
 *                             Copyright (c) 1984-2022
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
 */

/*
 *  Last updated: 25 Jan 2022
 */

#pragma pack(push, 1)

#ifdef _WIN64
  #define VERSION_INFO "Font Edit\nVersion 1.50.00 (64-bit)\n\nForever Young Software\n(C)opyright 1984-2022\n\nhttps://www.fysnet.net"
#else
  #define VERSION_INFO "Font Edit\nVersion 1.50.00 (32-bit)\n\nForever Young Software\n(C)opyright 1984-2022\n\nhttps://www.fysnet.net"
#endif

#define APP_WIDTH       700
#define APP_WIDTH_WIDE  975
#define APP_HEIGHT      700
#define APP_HEIGHT_TALL 800
#define APP_GET_HEIGHT ((font->height >= 20) ? APP_HEIGHT_TALL : APP_HEIGHT)

// max height and width of our font
#define MAXW  24
#define MAXH  24

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
  bit8u  width;   // Width of character
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
  //struct FONT_INFO info[];  // char info
  //bit8u data[];     // char data
};

#pragma pack(pop)

//  Declare  procedures
HWND CreateButton(HWND, DWORD, const char *, const int, const int);
HWND CreateTrackBar(HWND, const int, const int);

void SetTitleStr(const HWND);
void DisableItems(HMENU);
void EnableItems(HMENU, struct FONT *);
void DrawTextBox(HDC, DWORD, const int, const int, const int, const int, const char *);
int  LoadCurChar(HWND, struct FONT *, const int);
void SaveCurChar(struct FONT *, const int);
struct FONT *ConvertFont(HWND, struct FONT *, const int, const int, const int, const int, const char *);
void DumpFont(HWND, struct FONT *);
void FontMoveData(struct FONT *, int, int);

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
BOOL OpenFileDialog(HWND, LPTSTR, LPTSTR);
BOOL SaveFileDialog(HWND, LPTSTR, LPTSTR, LPTSTR);

struct FONT *InitFontData(struct FONT *, const int, const int, const int, const int, const int, const char *);
void SaveFile(HWND, struct FONT *);
struct FONT *OpenFile(HWND, struct FONT *);
