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
 *  mouse.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

#include "../include/ctype.h"
#include "gui.h"
#include "palette.h"
#include "grfx.h"
#include "mouse.h"

// the data to hold our current mouse information
struct S_MOUSE_DATA cur_mouse_struct;

// Standard pointer image (ID = POINTER_ID_STANDARD)
#define P0 GUITRGB(255,   0,   0,   0)
#define P1 GUITRGB(  0,   0,   0,   0)
#define P2 GUITRGB(  0, 255, 255, 255)
static PIXEL __mpointer[10 * 16] = {
   P1, P1, P0, P0, P0, P0, P0, P0, P0, P0,
   P1, P2, P1, P0, P0, P0, P0, P0, P0, P0,
   P1, P2, P2, P1, P0, P0, P0, P0, P0, P0,
   P1, P2, P2, P2, P1, P0, P0, P0, P0, P0,
   P1, P2, P2, P2, P2, P1, P0, P0, P0, P0,
   P1, P2, P2, P2, P2, P2, P1, P0, P0, P0,
   P1, P2, P2, P2, P2, P2, P2, P1, P0, P0,
   P1, P2, P2, P2, P2, P2, P2, P2, P1, P0,
   P1, P2, P2, P2, P2, P2, P2, P2, P2, P1,
   P1, P2, P2, P2, P2, P2, P1, P1, P1, P0,
   P1, P2, P2, P1, P2, P2, P1, P0, P0, P0,
   P1, P2, P1, P0, P1, P2, P2, P1, P0, P0,
   P0, P1, P0, P0, P1, P2, P2, P1, P0, P0,
   P0, P0, P0, P0, P0, P1, P2, P2, P1, P0,
   P0, P0, P0, P0, P0, P1, P2, P2, P1, P0,
   P0, P0, P0, P0, P0, P0, P1, P1, P0, P0
};
#undef P2
#undef P1
#undef P0
struct BITMAP _mpointer = GUIBITMAP_DECLARE(__mpointer, 10, 16);

// Standard text pointer image (ID = POINTER_ID_TEXT)
#define P0 GUITRGB(255,   0,   0,   0)
#define P1 GUITRGB(  0,   0,   0,   0)
#define P2 GUITRGB(  0, 255, 255, 255)
static PIXEL __tpointer[5 * 16] = {
   P1, P1, P0, P1, P1,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P0, P0, P1, P0, P0,
   P1, P1, P0, P1, P1,
};
#undef P2
#undef P1
#undef P0
struct BITMAP _tpointer = GUIBITMAP_DECLARE(__tpointer, 5, 16);

// Standard hand pointer image (ID = POINTER_ID_HAND)
#define P0 GUITRGB(255,   0,   0,   0)
#define P1 GUITRGB(  0,   0,   0,   0)
#define P2 GUITRGB(  0, 255, 255, 255)
static PIXEL __hpointer[17 * 18] = {
  P0, P0, P0, P0, P0, P0, P1, P1, P0, P0, P0, P0, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P0, P0, P0, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P0, P0, P0, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P0, P0, P0, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P0, P0, P0, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P1, P1, P1, P0, P0, P0, P0, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P1, P2, P2, P1, P1, P1, P1, P0, P0, 
  P0, P1, P1, P0, P0, P1, P2, P2, P1, P2, P2, P1, P2, P2, P1, P1, P1, 
  P1, P2, P2, P1, P1, P1, P2, P2, P1, P2, P2, P1, P2, P2, P1, P2, P1, 
  P1, P2, P2, P2, P2, P1, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P1, 
  P0, P1, P1, P2, P2, P1, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P1, 
  P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P1, 
  P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P1, P0, 
  P0, P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P2, P2, P2, P1, P0, 
  P0, P0, P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P2, P1, P0, P0, 
  P0, P0, P0, P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P1, P0, P0, 
  P0, P0, P0, P0, P0, P0, P1, P2, P2, P2, P2, P2, P2, P2, P1, P0, P0, 
  P0, P0, P0, P0, P0, P0, P0, P1, P1, P1, P1, P1, P1, P1, P0, P0, P0
};
#undef P2
#undef P1
#undef P0
struct BITMAP _hpointer = GUIBITMAP_DECLARE(__hpointer, 17, 18);

// a POINTER_STACK_SIZE element stack of mouse pointer ID numbers
int cur_pointer_stack[POINTER_STACK_SIZE];
int cur_pointer_indx = 0;

// pointers to a number of different predefined pointers
struct CUR_MOUSE mpointer[POINTER_ID_COUNT] = { 
  { POINTER_ID_STANDARD, &_mpointer, 10, 16, 0, 0 },
  { POINTER_ID_TEXT,     &_tpointer, 5, 16, 2, 7 },
  { POINTER_ID_HAND,     &_hpointer, 17, 18, 7, 1 },
  { POINTER_ID_BUSY,     NULL, 0, 0, 0, 0 },   // loaded at run time
};

// current pointer (defaults to the first one)
struct CUR_MOUSE *cur_pointer = &mpointer[0];

// set the pointer to the ID given, "pushing" the current on
//  on the stack to be able to restore it later

// TODO: Currently, we have the ability to load an animated static image
//   yet do not have the ability to animate it because the pointer is just
//   a bitmap, not an object such as an IMAGE object.  Therefore, it does
//   not receive any events, namely the SEC_ELAPSED event, where the image_class
//   will animate it for us.

/*  set_pointer()
 *              id = id of pointer to set to (POINTER_ID_*)
 *
 *   set the pointer to a new bitmap
 */
bool set_pointer(const int id) {
  int i;
  
  if (cur_pointer_indx < POINTER_STACK_SIZE) {
    // find the pointer desired
    for (i=0; i<POINTER_ID_COUNT; i++) {
      if ((mpointer[i].id == id) && (mpointer[i].bitmap != NULL)) {
        // push the current pointer's id on the stack
        cur_pointer_stack[cur_pointer_indx++] = cur_pointer->id;
        
        // set the new pointer
        cur_pointer = &mpointer[i];
        
        // draw the new pointer
        gfx_pointer(NULL, NULL, NULL, NULL);
        
        return TRUE;
      }
    }
  }
  
  return FALSE;
}

/*  set_pointer()
 *     no parameters
 *
 *   restore the last pointer from the pointer stack
 */
bool restore_pointer() {
  if (cur_pointer_indx > 0) {
    set_pointer(cur_pointer_stack[--cur_pointer_indx]);
    return TRUE;
  }
  
  return FALSE;
}

/*  load_mouse_pointer()
 *              id = mouse id from enum POINTER_ID in mouse.h (must exist)
 *        image_id = id of image from static images (must exist)
 *            hotx = X coordinate of hotspot relative to left of image
 *            hoty = Y coordinate of hotspot relative to top of image
 *
 * load a mouse cursor image from the static image array (images.sys).
 * this will overwrite an already loaded cursor image
 *
 */
bool load_mouse_pointer(const int id, const int image_id, const int hotx, const int hoty) {
  int i;
  struct BITMAP *bitmap;
  
  for (i=0; i<POINTER_ID_COUNT; i++) {
    if (mpointer[i].id == id) {
      bitmap = get_static_bitmap(image_id, 0, 0);
      if (bitmap) {
        mpointer[i].bitmap = bitmap;
        mpointer[i].w = gui_w(bitmap);
        mpointer[i].h = gui_h(bitmap);
        if (hotx > -1)
          mpointer[i].hw = MIN(hotx, mpointer[i].w);
        else
          mpointer[i].hw = (mpointer[i].w / 2);
        if (hoty > -1)
          mpointer[i].hh = MIN(hoty, mpointer[i].h);
        else
          mpointer[i].hh = (mpointer[i].h / 2);
        return TRUE;
      }
    }
  }
  
  return FALSE;
}

/*  mouse_init()
 *        x_llimit = low limit X coordinate (usually 0)
 *        y_llimit = low limit Y coordinate (usually 0)
 *        x_hlimit = high limit X coordinate (usually width of screen)
 *        y_hlimit = high limit Y coordinate (usually height of screen)
 *               x = X coordinate to start mouse at
 *               y = Y coordinate to start mouse at
 *              tx = delta jump. move mouse this many pixels per single increment (usually 1)
 *              ty = delta jump. move mouse this many pixels per single increment (usually 1)
 *                   (if you have a fairly wide screen, make tx = 2 so it will travel further faster)
 *
 * initialize the mouse
 * NOTE:
 *   this is mostly DOS (DPMI) specific for this demo.  you will need to code it
 *    for your OS you are writing.
 *
 */
bool mouse_init(const int x_llimit, const int y_llimit, const int x_hlimit, const int y_hlimit, 
                const int x, const int y, const bit8u tx, const bit8u ty) {
  
  __dpmi_regs r;
  
  cur_mouse_struct.curx = x;
  cur_mouse_struct.cury = y;
  cur_mouse_struct.curz = 0;
  cur_mouse_struct.left = FALSE;
  cur_mouse_struct.right = FALSE;
  cur_mouse_struct.mid = FALSE;
  cur_mouse_struct.x_thresh = tx;
  cur_mouse_struct.y_thresh = ty;
  cur_mouse_struct.x_llimit = x_llimit;
  cur_mouse_struct.y_llimit = y_llimit;
  cur_mouse_struct.x_hlimit = (x_hlimit * cur_mouse_struct.x_thresh);
  cur_mouse_struct.y_hlimit = (y_hlimit * cur_mouse_struct.y_thresh);
  
  // call the mouse interrupt to reset and initialize the mouse
  // ax = 0 to reset
  r.x.ax = 0x0000;
  __dpmi_int(0x33, &r);
  
  // on return
  //  success: ax = -1 and bx = number of buttons
  //     fail: ax = 0
  if (r.x.ax == 0)
    return FALSE;
  
  return install_mouse_handler(0xFF, mouse_handler);
}

/*  mouse_get_info()
 *   (on return)
 *               x = current x location (within x_llimit and x_hlimit, i.e.:  x_llimit <= x <= x_hlimit )
 *               y = current y location (within y_llimit and y_hlimit, i.e.:  y_llimit <= y <= y_hlimit )
 *               z = current wheel mouse delta (usually -10 to +10)
 *            btns = bitmap of buttons pressed (bit 0 = left, bit 1 = right, bit 2 = middle)
 *
 * get the current mouse info
 * NOTE:
 *   this is the function that returns the mouse info to the GUI system
 *   you need to code it so that it returns this correct information as described above
 *
 */
bool mouse_get_info(int *x, int *y, int *z, int *btns) {
  
  *x = cur_mouse_struct.curx;
  *y = cur_mouse_struct.cury;
  *z = cur_mouse_struct.curz;
  cur_mouse_struct.curz = 0; // once the system gets this value, we need to zero it out again.
  
  *btns = ((cur_mouse_struct.left)  ? MOUSE_LBUT : 0) |
          ((cur_mouse_struct.right) ? MOUSE_RBUT : 0) |
          ((cur_mouse_struct.mid)   ? MOUSE_MBUT : 0);
  
  return TRUE;
}

/*
 * The remaining code here is for DOS only.  This is for this demo
 *  and installs a handler with the installed mouse driver.
 * Your OS code should already have a mouse handler installed.
 */

bool running = FALSE,
     handler_installed = FALSE;
__dpmi_regs         mouse_regs;
_go32_dpmi_seginfo  mouse_cb_info, old_mouse_handler;

bit16s lastx = 0;
bit16s lasty = 0;

// this function gets called from the DOS mouse driver everytime there is a mouse status change
// NOTE: you cannot place DOS services within this handler.  A printf() will crash the handler.
void mouse_handler(__dpmi_regs *regs) {
  
  // don't let us re-enter until we are done
  // don't have your handler do much or you will miss events
  if (running)
    return;
  
  running = TRUE;
  
  // Values interrupt routine is called with:.
  //  AX = condition mask (same bit assignments as call mask).
  //  BX = button state.
  //    CTMOUSE uses BL as button state and BH as wheel-z change
  //  CX = cursor column.
  //  DX = cursor row.
  //  SI = horizontal mickey count.
  //  DI = vertical mickey count
  cur_mouse_struct.left =  ((mouse_regs.x.bx & (1<<0)) == (1<<0));
  cur_mouse_struct.right = ((mouse_regs.x.bx & (1<<1)) == (1<<1));
  cur_mouse_struct.mid =   ((mouse_regs.x.bx & (1<<2)) == (1<<2));
  cur_mouse_struct.curx += ((bit16s) mouse_regs.x.si - lastx);
  cur_mouse_struct.cury += ((bit16s) mouse_regs.x.di - lasty);
  cur_mouse_struct.curz = (int) (bit8s) mouse_regs.h.bh;
  
  // check our limits
  if (cur_mouse_struct.curx < cur_mouse_struct.x_llimit) cur_mouse_struct.curx = cur_mouse_struct.x_llimit;
  if (cur_mouse_struct.cury < cur_mouse_struct.y_llimit) cur_mouse_struct.cury = cur_mouse_struct.y_llimit;
  if (cur_mouse_struct.curx > cur_mouse_struct.x_hlimit) cur_mouse_struct.curx = cur_mouse_struct.x_hlimit;
  if (cur_mouse_struct.cury > cur_mouse_struct.y_hlimit) cur_mouse_struct.cury = cur_mouse_struct.y_hlimit;
  
  // store mickey position for next time
  lastx = mouse_regs.x.si;
  lasty = mouse_regs.x.di;
  
  running = FALSE;
}

// install the mouse handler
// mask is set to allow certain items to trigger the handler above.
//   we set it to 0xFF to allow all items to trigger it
bool install_mouse_handler(unsigned mask, void (*func)(__dpmi_regs *)) {
  __dpmi_regs r;
  
  _go32_dpmi_get_protected_mode_interrupt_vector(0x33, &old_mouse_handler);
  mouse_cb_info.pm_selector = _my_cs();
  mouse_cb_info.pm_offset = (bit32u) func;
  if (_go32_dpmi_allocate_real_mode_callback_retf(&mouse_cb_info, &mouse_regs))
    return FALSE;
  
  // CTMOUSE Notes:
  // on entry, bit 7 of CX (call mask) indicates that the user routine
  //  will be called on a wheel movement the user routine will be called
  //  with BH holding the 8-bit signed counter of wheel movement since
  //  the last call to the routine
  r.x.ax = 0x000C;
  r.x.cx = mask & 0xFF;
  r.x.es = mouse_cb_info.rm_segment;
  r.x.dx = mouse_cb_info.rm_offset;
  __dpmi_int(0x33, &r);
  old_mouse_handler.rm_segment = r.x.es;
  old_mouse_handler.rm_offset = r.x.dx;
  
  if (r.x.flags & 1)
    return FALSE;
  
  return handler_installed = TRUE;
}

// we're done with the mouse handler, so be sure to remove it before we
//  exit to DOS or a mouse event will trigger bogus code...
void remove_mouse_handler(void) {
  __dpmi_regs r;
  
  if (handler_installed) {
    r.x.ax = 0x000C;
    r.x.cx = 0;
    r.x.es = 0;
    r.x.dx = 0;
    __dpmi_int(0x33, &r);
    
    _go32_dpmi_set_protected_mode_interrupt_vector(0x33, &old_mouse_handler);
    _go32_dpmi_free_real_mode_callback(&mouse_cb_info);
  }
  
  return;
}
