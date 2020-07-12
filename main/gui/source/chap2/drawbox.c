/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Graphical User Interface, and is for that purpose only.
 *   You have the right to use it for learning purposes only.  You may
 *   not modify it for redistribution for any other purpose unless you
 *   have written permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 * Last update:  05 May 2016
 *
 * usage:
 *   drawbox
 * 
 * This utility simply checks for a valid video system, draws a box
 *   to the screen, then waits for a key press.
 *   
 * Assumptions:
 *  - this code assumes and is coded for 16-bit pixels only.
 *
 *  Thank you for your purchase and interest in my work.
 *
 * compile using gcc
 *  gcc -Os drawbox.c -o drawbox.exe -s
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
