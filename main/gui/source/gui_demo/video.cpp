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
 *  video.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <stdio.h>
#include <string.h>

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <pc.h>

#include "../include/ctype.h"
#include "video.h"

// global pointer to our video card data
struct S_SYS_VIDEO *sys_video = NULL;

/*  vesa_mode_enum()
 *       video = pointer to the holder for our video card data
 *   mode_list = pointer to a string of WORDs each holding a mode number
 *     verbose = print out to debug file?
 *
 *  enumerates the given mode numbers
 *   
 */
void vesa_mode_enum(struct S_SYS_VIDEO *video, bit16u *mode_list, const bool verbose) {
  unsigned found = 0, mode_count = 0;
  
  if (verbose) puts("\n");
  
  // the mode list is terminated with 0xFFFF
  video->mode_count = 0;
  while ((*mode_list < 0xFFFF) && (found < MAX_MODE_COUNT)) {
    if (verbose) printf("\r Checking found mode: 0x%04X: ", *mode_list);
    // get the video mode information and if it states that we are compatible
    //  with the card *and* the monitor, modify a few things and add it to our list
    if (vid_get_mode_info(&video->vid_mode_info[mode_count], *mode_list) &&
       (video->vid_mode_info[mode_count].mode_attrb & 0x01) &&  // supported by the current hardware (video card and monitor)
        (
         (video->vid_mode_info[mode_count].memory_model == 4) || // model = 4 = packed pixel
         (video->vid_mode_info[mode_count].memory_model == 6)    // model = 6 = direct color
        )
       ) {
      // now set the modes struct item
      video->modes[video->mode_count].bits_pixel = video->vid_mode_info[mode_count].bits_pixel;
      video->modes[video->mode_count].c_height = video->vid_mode_info[mode_count].y_char_size;
      video->modes[video->mode_count].c_width = video->vid_mode_info[mode_count].x_char_size;
      video->modes[video->mode_count].height = video->vid_mode_info[mode_count].y_res;
      video->modes[video->mode_count].width = video->vid_mode_info[mode_count].x_res;
      video->modes[video->mode_count].pages = 0; /////
      video->modes[video->mode_count].support = TRUE;
      video->modes[video->mode_count].t_height = 25; ///////
      video->modes[video->mode_count].t_width = 80; ///////
      video->modes[video->mode_count].vid_mem_size = 0; //////////
      video->modes[video->mode_count].mode = *mode_list;
      video->modes[video->mode_count].mode_info = &video->vid_mode_info[mode_count];
      
      // can we use linear buffer addressing?
      video->modes[video->mode_count].linear = 
         ((video->vid_mode_info[mode_count].mode_attrb & (1<<7)) &&
          (video->vid_mode_info[mode_count].linear_base > 0));
      // can we use bank switching
      video->modes[video->mode_count].bankswitch = 
         (!(video->vid_mode_info[mode_count].mode_attrb & (1<<6)) &&
          ((video->vid_mode_info[mode_count].wina_attrb & 5) == 5) &&
           (video->vid_mode_info[mode_count].wina_segment > 0));
      
      // if both are available, let's make two mode entries to allow us to choose the way we draw to the screen
      if (video->modes[video->mode_count].linear && video->modes[video->mode_count].bankswitch) {
        memcpy(&video->modes[video->mode_count + 1], &video->modes[video->mode_count], sizeof(struct S_VID_MODE));
        video->modes[video->mode_count].bankswitch = FALSE;
        video->modes[video->mode_count + 1].linear = FALSE;
        video->mode_count++;
      }
      
      // move to next entry
      video->mode_count++;
      
      if (verbose) puts(" Supported.");
      mode_count++;
    } else {
      if (verbose) puts(" Not supported.");
    }
    mode_list++;
    found++;
  }
  if (verbose) printf("\r Modes found: %i  Modes supported: %i            ", found, mode_count);
}

/*  vid_get_vid_info()
 *       info = pointer to the holder for our video card info
 *
 *  BIOS service to retrieve card info
 *
 * NOTE:
 *   this is DOS DPMI and DJGPP specific.  you will need to write this
 *    routine to match your OS
 *   
 */
bool vid_get_vid_info(struct S_VIDEO_SVGA *info) {
  bool ret = FALSE;
  __dpmi_regs r;
  int i;
  
  for (i=0; i<sizeof(struct S_VIDEO_SVGA); i++)
    _farpokeb(_dos_ds, __tb + i, 0);
  
  dosmemput("VBE2", 4, __tb);
  
  r.x.ax = 0x4F00;
  r.x.di = __tb & 0xF;
  r.x.es = __tb >> 4;
  __dpmi_int(0x10, &r);
  
  if (r.h.ah == 0x00) {
    ret = TRUE;
    
    // store the info
    if (info) {
      dosmemget(__tb, sizeof(struct S_VIDEO_SVGA), info);
      if (strncmp(info->VESASignature, "VESA", 4) != 0)
        ret = FALSE;
    }
  }
  
  return ret;
}

/*  vid_get_vid_modes()
 *       info = pointer to our video card info
 *      modes = pointer to store the modes found
 *
 *  BIOS service to retrieve mode info
 *
 * NOTE:
 *   this is DOS DPMI and DJGPP specific.  you will need to write this
 *    routine to match your OS
 *   
 */
int vid_get_vid_modes(struct S_VIDEO_SVGA *info, bit16u *modes) {
  int i = 0;
  bit16u mode;
  bit32u ptr = (info->svgamodesseg << 4) + info->svgamodesptr;
  
  do {
    mode = _farpeekw(_dos_ds, ptr);
    ptr += 2;
    modes[i++] = mode;
  } while ((mode < 0xFFFF) && (i < MAX_MODE_COUNT));
  
  // return the count of modes found
  return i;
}

/*  vid_get_mode_info()
 *       info = pointer to the holder for our video mode info
 *       mode = mode number to get info for
 *
 *  BIOS service to retrieve mode info
 *
 * NOTE:
 *   this is DOS DPMI and DJGPP specific.  you will need to write this
 *    routine to match your OS
 *   we only get info for modes 0x100 and above
 *   
 */
bool vid_get_mode_info(struct S_VIDEO_MODE_INFO *info, const bit16u mode) {
  bool ret = FALSE;
  
  __dpmi_regs r;
  int i;
  
  if (mode >= 0x100) {
    r.x.ax = 0x4F01;
    r.x.cx = mode;
    r.x.di = __tb & 0xF;
    r.x.es = __tb >> 4;
    __dpmi_int(0x10, &r);
    
    // Now check eax to see if supported.  If so, get next part.  etc....
    if (r.h.ah == 0) {
      // store the info
      if (info) {
        dosmemget(__tb, sizeof(struct S_VIDEO_MODE_INFO), info);
        
        // do some fixups
        if (info->memory_model == 4) { // packed pixel
          switch (info->bits_pixel) {
            case 8:
              info->rsvd_field_pos = 8;
              info->rsvd_mask_size = 24;
              info->red_field_pos = 5;
              info->red_mask_size = 3;
              info->green_field_pos = 2;
              info->green_mask_size = 3;
              info->blue_field_pos = 0;
              info->blue_mask_size = 2;
              break;
            case 15:
              info->rsvd_field_pos = 15;
              info->rsvd_mask_size = 17;
              info->red_field_pos = 10;
              info->red_mask_size = 5;
              info->green_field_pos = 5;
              info->green_mask_size = 5;
              info->blue_field_pos = 0;
              info->blue_mask_size = 5;
              break;
            case 16:
              info->rsvd_field_pos = 16;
              info->rsvd_mask_size = 16;
              info->red_field_pos = 11;
              info->red_mask_size = 5;
              info->green_field_pos = 5;
              info->green_mask_size = 6;
              info->blue_field_pos = 0;
              info->blue_mask_size = 5;
              break;
            case 24:
            case 32:
              info->rsvd_field_pos = 24;
              info->rsvd_mask_size = 0;
              info->red_field_pos = 16;
              info->red_mask_size = 8;
              info->green_field_pos = 8;
              info->green_mask_size = 8;
              info->blue_field_pos = 0;
              info->blue_mask_size = 8;
              break;
          }
        }
      }
      ret = TRUE;
    }
  }
  
  return ret;
}

/*  vid_set_mode()
 *         indx = index into sys_video->modes[] for mode to use
 *       linear = 1 = is linear mode
 *      clr_mem = 1 = clear the video memory on mode set
 *
 *  Set video mode via VESA BIOS
 *
 *  Notes:
 *   I have a Dell Inspiron 2500 Laptop that when I use this function to set
 *    the mode (any given mode), it crashes the system.
 *   The Inspiron has VBE 3.0+ VESA BIOS.
 */
bool vid_set_mode(const int indx, const bool linear, const bool clr_mem) {
  
  __dpmi_regs r;
  struct S_VID_CRTC_INFO crtc_info;
  
  memset(&crtc_info, 0, sizeof(struct S_VID_CRTC_INFO));
  dosmemput(&crtc_info, sizeof(struct S_VID_CRTC_INFO), __tb);
  
  // VESA Set Mode
  r.x.ax = 0x4F02;
  
  //  15      = 0 - Clear display memory
  //          = 1 - Don't clear display memory  
  //  14      = 0 - Use windowed frame buffer model
  //          = 1 - Use linear/flat frame buffer model
  //  13:12   = Reserved (must be 0)
  //  11      = 0 - Use current default refresh rate
  //          = 1 - Use user specfieid CRTC values for refresh rate
  //  10:9    = Reserved (must be 0)
  //  8:0     = Mode Number
  r.x.bx = (sys_video->modes[indx].mode & 0x1FF) | 
           (0<<11)                               |
           (linear  ? (1<<14) : 0)               |
           (clr_mem ? 0 : (1<<15));
  
  // VBE 3.0+
  r.x.es = __tb >> 4;
  r.x.di = __tb & 0x0F;
  
  __dpmi_int(0x10, &r);
  
  return (r.x.ax == 0x004F);
}

/*  vid_set_text_mode()
 *         mode = text mode to set the screen to
 *
 *  Set text mode via Video BIOS
 *
 */
void vid_set_text_mode(const int mode) {
  __dpmi_regs r;
  
  r.x.ax = mode;
  __dpmi_int(0x10, &r);
}

/*  vsync()
 *     no parameters
 * 
 *   wait for the vertical sync to start
 */
void vsync(void) {
  // wait until any previous retrace has ended
  while (inportb(0x3DA) & (1<<3));
  
  // wait until a new retrace has just begun
  while (!(inportb(0x3DA) & (1<<3)));
}

/*  vid_set_256_palette()
 *         port_io = use port I/O
 *
 *  this sets the 256 bpp mode palette
 *  if the card isn't VGA compatible, we have
 *   to use the VESA BIOS to set the palette
 *
 *  The RGB components can be 0 - 63 each, with 256 entries
 *   within the table to choose from.
 */
void vid_set_256_palette(const bool port_io) {
  int i;
  __dpmi_regs r;
  bit8u palette256_info[256 * 4], *p;
  
  // some controllers are not VGA compatible
  if (port_io) {
    // wait for the retrace to start (older video cards require this)
    vsync();
    // mode is VGA compatible, so use the VGA regs
    for (i=0; i<255; i++) {
      outportb(0x3C8, (bit8u) i);
      outportb(0x3C9, (bit8u) (i & 0xE0) >> 2);
      outportb(0x3C9, (bit8u) (i & 0x1C) << 1);
      outportb(0x3C9, (bit8u) (i & 0x03) << 4);
    }
    // white
    outportb(0x3C8, (bit8u) 255);
    outportb(0x3C9, 0xFF);
    outportb(0x3C9, 0xFF);
    outportb(0x3C9, 0xFF);
  } else {
    // must use VBE to set the DAC (palette)
    
    // ax = 0x4F09
    // bl = 0x80  set palette info while vertical retrace
    // cx = number of registers (256)
    // dx = start (0)
    // es:di -> address to buffer
    //  buffer is:
    //    {
    //     db  blue
    //     db  green
    //     db  red
    //     db  resv
    //    } [256];
    
    p = palette256_info;
    for (i=0; i<255; i++) {
      *p++ = (bit8u) (i & 0xE0) >> 2;
      *p++ = (bit8u) (i & 0x1C) << 1;
      *p++ = (bit8u) (i & 0x03) << 4;
      *p++ = 0;
    }
    // white
    *p++ = 0xFF;
    *p++ = 0xFF;
    *p++ = 0xFF;
    *p++ = 0;
    
    // put it into memory our DPMI call can access
    dosmemput(palette256_info, 256 * 4, __tb);
    
    // call the service
    r.x.ax = 0x4F09;
    r.x.bx = 0x0080;
    r.x.cx = 256;
    r.x.dx = 0;
    r.x.es = __tb >> 4;
    r.x.di = __tb & 0x0F;
    __dpmi_int(0x10, &r);
  }
}
