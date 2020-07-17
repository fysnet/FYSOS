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
 *  DRAWBOX.EXE
 *   This utility simply checks for a valid video system, draws a box
 *    to the screen, then waits for a key press.
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS envirnment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *   - this code assumes and is coded for 16-bit pixels only.
 *
 *  Last updated: 17 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os drawbox.c -o drawbox.exe -s
 *
 *  Usage:
 *    drawbox
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"
#include "..\include\video.h"

struct S_VIDEO_SVGA       video_info;
struct S_VIDEO_MODE_INFO  mode_info;

// This example is coded for 16-bit pixels only
// However, you may choose one of the following resolutions.
//#define VID_MODE    0x111   //  640x 480x64K (16-bit)
//#define VID_MODE    0x114   //  800x 600x64K (16-bit)
#define VID_MODE    0x117     // 1024x 768x64K (16-bit)
//#define VID_MODE    0x11A   // 1280x1024x64K (16-bit)

// our pixels, when within our code, are always 32 bit values
#define BACK_COLOR  0x002222FF  // 24 bit (0x00RRGGBB)
#define LINE_COLOR  0x00FF2222  // 24 bit (0x00RRGGBB)

#define WIDTH  (mode_info.x_res / 2)
#define HEIGHT (mode_info.y_res / 2)
#define X1 ((mode_info.x_res - WIDTH) / 2)
#define X2 (X1 + WIDTH)
#define Y1 ((mode_info.y_res - HEIGHT) / 2)
#define Y2 (Y1 + HEIGHT)

int main(int argc, char *argv[]) {
  int x, y;
  unsigned int start;
  
  // make sure we have a compatible video system
  if (!vid_get_vid_info(&video_info)) {
    printf("\n Did not find a compatible video system...");
    return -1;
  }
  
  // get the info for mode VID_MODE
  if (!vid_get_mode_info(&mode_info, VID_MODE) || !(mode_info.mode_attrb & 0x01)) {
    printf("\n Video Mode 0x%04X not supported...", VID_MODE);
    return -1;
  }
  
  // found good video and mode, so set up the display buffer access
  // setup flat descriptor to the linear base
  base_mi.address = mode_info.linear_base;
  base_mi.size = mode_info.y_res * mode_info.bytes_scanline;
  if (!get_physical_mapping(&base_mi, &base_selector)) {
    printf("\n Error 'allocating' physical memory for video.");
    return -1;
  }
  
  // now set the video mode
  if (!vid_set_mode(VID_MODE, TRUE)) {
    printf("\n Error setting video mode.");
    return -1;
  }
  
  // make a background
  start = 0;
  for (y=0; y<mode_info.y_res; y++) {
    for (x=0; x<mode_info.x_res; x++) {
      _farpokew(base_selector, start, GUIRGB565(BACK_COLOR));
      start += 2;
    }
  }
  
  // draw the box, starting with the top and bottom lines
  start = ((Y1 * mode_info.bytes_scanline) + (X1 * 2));
  for (x=0; x<((X2 - X1) + 1); x++) {
    _farpokew(base_selector, start, GUIRGB565(LINE_COLOR));
    start += 2;
  }
  start = ((Y2 * mode_info.bytes_scanline) + (X1 * 2));
  for (x=0; x<((X2 - X1) + 1); x++) {
    _farpokew(base_selector, start, GUIRGB565(LINE_COLOR));
    start += 2;
  }
  
  // then the left and right lines
  start = ((Y1 * mode_info.bytes_scanline) + (X1 * 2));
  for (y=0; y<((Y2 - Y1) + 1); y++) {
    _farpokew(base_selector, start, GUIRGB565(LINE_COLOR));
    start += mode_info.bytes_scanline;
  }
  start = ((Y1 * mode_info.bytes_scanline) + (X2 * 2));
  for (y=0; y<((Y2 - Y1) + 1); y++) {
    _farpokew(base_selector, start, GUIRGB565(LINE_COLOR));
    start += mode_info.bytes_scanline;
  }
  
  // wait for a keypress and return to DOS
  getch();
  
  // free the screen buffer selector
  __dpmi_free_physical_address_mapping(&base_mi);
  
  // return back to screen mode 3
  vid_set_mode(0x03, TRUE);
  
  return 0;
}
