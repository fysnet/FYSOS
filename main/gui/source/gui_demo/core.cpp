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
 *  core.cpp
 *  
 *  Last updated: 17 July 2020
 */

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <bios.h>

#include "../include/ctype.h"

#include "gui.h"
#include "grfx.h"
#include "mouse.h"
#include "palette.h"
#include "video.h"

struct GFX_ARGS grx_args;

extern struct S_VIDEO_MODE_INFO *video_mode_info;

void vesa_flush(struct RECT *);

// mouse position and button information
int mouse_x, mouse_y, mouse_z, mouse_b;
struct RECT mouse_prev = { 0, };

/* gui_exit()
 *    no parameters
 *
 * exit the GUI system
 *
 */
void gui_exit(void) {
  font_list_destroy();
  font_default(NULL);
  gfx_stop();
}

/* gfx_stop()
 *    no parameters
 *
 * stop the GUI system
 *
 */
void gfx_stop(void) {
  if (grx_args.buffer) {
    atom_unlock(&grx_args.buffer->base.atom);
    atom_delete(&grx_args.buffer->base.atom);
  }
  
  memset(&grx_args, 0, sizeof(GFX_ARGS));
}

/* gfx_redraw()
 *    redraw = function to call to redraw the screen
 *
 * set the redraw func and mark the whole screen as dirty
 *
 */
void gfx_redraw(REDRAW_FUNC redraw) {
  grx_args.redraw = redraw;
  gfx_dirty(&grx_args.screen);
}

/* gfx_flush()
 *    draw = function to use to draw to the screen
 *    rect = the area to draw
 *
 * flush the screen buffer(s)
 *
 */
void gfx_flush(DRAW_FUNC draw, struct RECT *rect) {
  
  atom_lock(&grx_args.buffer->base.atom);
  
  // Make sure the area isn't too large and that it has the correct alignment
  bitmap_offset(grx_args.buffer, rect->left, rect->top);
  RECT_INTERSECT(*GUIRECT(grx_args.buffer), *rect, *rect);
  
  // Actually get the bitmap to be updated
  bitmap_clip(grx_args.buffer, rect);
  grx_args.redraw(rect);
  
  // Draw the pointer
  bitmap_clip(grx_args.buffer, rect);
  if (grx_args.showpointer)
    bitmap_blit(cur_pointer->bitmap, GUISCREEN, 0, 0, mouse_x - cur_pointer->hw, mouse_y - cur_pointer->hh, cur_pointer->w, cur_pointer->h, FALSE);
  
  // Push the buffer to the visible buffer
  draw(rect, grx_args.buffer->array, gui_w(grx_args.buffer));
  atom_unlock(&grx_args.buffer->base.atom);
}

/* gfx_poll()
 *    no parameters
 *
 * do we need to update the screen?
 *
 */
void gfx_poll(void) {
  drs_update(vesa_flush);
}

/* gfx_dirty()
 *    rect = pointer to area
 *
 * mark area as dirty
 *
 */
void gfx_dirty(const struct RECT *rect) {
  if (!rect)
    rect = &grx_args.screen;
  drs_dirty(rect, TRUE);
}

/* gfx_pointer()
 *       x = pointer to place to store X
 *       y = pointer to place to store Y
 *       z = pointer to place to store Z
 *       b = pointer to place to store buttons
 *
 * retrieve the mouse information
 *
 */
bool gfx_pointer(int *x, int *y, int *z, int *b) {
  unsigned ret = 0;
  const int oldx = mouse_x;
  const int oldy = mouse_y;
  
  // get the mouse coordinates and button data
  // and limit the mouse coordinates to 'on screen'
  mouse_get_info(&mouse_x, &mouse_y, &mouse_z, &mouse_b);
  mouse_x = MID(grx_args.screen.left, mouse_x, grx_args.screen.right);
  mouse_y = MID(grx_args.screen.top, mouse_y, grx_args.screen.bottom);
  
  // Update the users values
  if (x)
    *x = mouse_x;
  if (y)
    *y = mouse_y;
  if (z)
    *z = mouse_z;
  if (b)
    *b = mouse_b;
  
  if ((mouse_x != oldx) || (mouse_y != oldy)) {
    struct RECT rect;
    
    rect.left = mouse_x - cur_pointer->hw;
    rect.top = mouse_y - cur_pointer->hh;
    rect.right = rect.left + cur_pointer->w - 1;
    rect.bottom = rect.top + cur_pointer->h - 1;
    
    // If mouse positions overlap, draw them together
    if (RECTS_OVERLAP(mouse_prev, rect)) {
      RECT_UNION(mouse_prev, rect, rect);
      gfx_dirty(&rect);
      // Mouse moved a great distance, draw each position seperately
    } else {
      gfx_dirty(&rect);
      gfx_dirty(&mouse_prev);
    }
    
    // copy the current to the previous
    memcpy(&mouse_prev, &rect, sizeof(struct RECT));
  }
  return TRUE;
}

/* gfx_hidepointer()
 *      hide = 1 = hide the pointer
 *
 * shows or hides the pointer.
 * returns old value to be able to restore it correctly
 *
 */
bool gfx_hidepointer(const bool hide) {
  const bool old = grx_args.showpointer;
  grx_args.showpointer = hide;
  return old;
}

/* gfx_key()
 *      scan = place to store the scan code (may be NULL)
 *     ascii = place to store the ascii code (may be NULL)
 *     shift = place to store the shift state (may be NULL)
 *
 *  this routine gets the current state of the keyboard and a key press
 *  you need to modify this to match your OS.  it currently matches DOS.
 *
 */
bool gfx_key(bit16u *scan, bit8u *ascii, bit8u *shift) {
  bit16u word, shifts;
  
  // get shift status
  shifts = bioskey(0x12);
  
  // if there is not a key pressed, return false
  if (((shifts & 0xFF00) == 0) && (bioskey(0x11) == 0))
    return FALSE;
  
  // if a key pressed, get it
  if (bioskey(0x11))
    word = bioskey(0x00);   // get a key
  else
    word = 0;
  if (scan)  *scan = word;
  if (ascii) *ascii = (word & 0xFF);
  if (shift) *shift = shifts;
  
  return TRUE;
}

PIXEL gui_wincolor = GUIRGB(222, 223, 196);        // the current object color (field of buttons, etc)
PIXEL gui_backcolor = GUIRGB(255, 255, 255);       // 
PIXEL gui_forecolor = GUIRGB(0, 0, 0);             //
PIXEL gui_winshaddowcolor = GUIRGB(127, 127, 127); // 
PIXEL gui_winbrightcolor = GUIRGB(255, 255, 255);  // 
PIXEL gui_selectbackcolor = GUIRGB(0, 85, 229);    // current "top" window
PIXEL gui_selectforecolor = GUIRGB(255, 255, 255); // 
PIXEL gui_armcolor = GUIRGB(130, 169, 233);        // any window "under" top window
PIXEL gui_disabledcolor = GUIRGB(127, 127, 127);   // 
PIXEL gui_desktop_color = GUIRGB(0, 128, 192);     // blank desktop color


/* win_check_mnemonics()
 *       win = window object to search children for
 *        ch = search child for this mnemonic char
 *
 * this is called from a windows handler when a key is pressed from that handler.
 * it scrolls through the objects within that window finding all the '&' chars
 *  within the strings.  If one is found that matches the key pressed, it returns
 *  that objects pointer.
 * example:  a button with the text of "&Okay".  If the 'o' is pressed, this
 *  returns the pointer to this object (button).
 *
 */
struct OBJECT *win_check_mnemonics(const struct WIN *win, const char ch) {
  int len = 0;
  const char *text;
  struct OBJECT *parent = (struct OBJECT *) GUIOBJ(win);
  
  if (parent->last) {
    struct OBJECT *child = parent->last;
    while (child) {
      if (obj_has_mnemonic(child)) {
        struct TEXTUAL *textual = (struct TEXTUAL *) child;
        text = textual_text(textual, &len);
        if (text && (len > 0)) {
          // find '&' char
          char *p = strchr(text, '&');
          if (p && (tolower(p[1]) == tolower(ch)))
            return child;
        }
      }
      child = child->prev;
    }
  }
  
  return NULL;
}

/* gui_event()
 *       no parameters
 *
 *  do an event...
 *
 */
void gui_event(void) {
  struct RECT *rect = defaultrect_data();
  
  if (rect) {
    // For a close, min, max, or resize button make it so big as the bitmap
    if ((GUIID(eventstack.object) == ID_BORDER_CLOSE) ||
        (GUIID(eventstack.object) == ID_BORDER_MIN)   ||
        (GUIID(eventstack.object) == ID_BORDER_MAX)   ||
        (GUIID(eventstack.object) == ID_BORDER_RESIZE)) {
      struct BUTTON *button = (struct BUTTON *) eventstack.object;
      rect->right = rect->left + gui_w(button) - 1;
      rect->bottom = rect->top + gui_h(button) - 1;
      return;
      
      // For a window border, make it a bit wider
    } else if (GUIID(eventstack.object) == ID_BORDER_WIN) {
      event_default();
      rect->left -= 2;
      rect->right += 2;
      return;
      
      // Sliders have to have a minimum width / height
    } else if (GUICLASS(eventstack.object) == vslider_class) {
      struct SLIDER *slider = (struct SLIDER *) eventstack.object;
      slider->min = 5;
      rect->right = rect->left + 10;
      return;
      
    } else if (GUICLASS(eventstack.object) == hslider_class) {
      struct SLIDER *slider = (struct SLIDER *) eventstack.object;
      slider->min = 5;
      rect->bottom = rect->top + 10;
      return;
      
    }
    
    if (GUICLASS(eventstack.object) == winborder_class) {
      event_default();
      rect->bottom += 2;
      return;
    }
    
    // Position close, min, max, and resize buttons differently
  } else if (GUICLASS(eventstack.object) == winborder_class) {
    if (eventstack.event == GEOMETRY) {
      struct WINBORDER *border = (struct WINBORDER *) eventstack.object;
      struct OBJECT *obj = GUIOBJ(&border->close);
      obj_layout(obj, (LAYOUT) (LAYOUT_X2 | LAYOUT_Y1), GUIOBJ(border), -3, 3);
      if (border->flags & BORDER_HAS_MAX) {
        obj_layout(GUIOBJ(&border->max), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), obj, 1, 0);
        obj = GUIOBJ(&border->max);
      }
      if (border->flags & BORDER_HAS_MIN)
        obj_layout(GUIOBJ(&border->min), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), obj, 1, 0);
      
      if (border->flags & BORDER_HAS_RESIZE)
        obj_layout(GUIOBJ(&border->resize), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y2), GUIOBJ(border), -2, -2);
      if (border->flags & BORDER_HAS_STATUS)
        obj_layout(GUIOBJ(&border->status), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y2), GUIOBJ(border), 2, 0);
      
      return;
    }
  }
  
  // else call the default event handler
  event_default();
}


//////////////////////////////////////////////////////////////////////
// Start of drawing functions

/* gui_obj()
 *       obj = pointer to object to draw
 *
 *  draw an object
 *
 *  if we make it clear to here, then the class of the object didn't
 *   draw us, so simply place a rectangle here
 */
void gui_obj(struct OBJECT *obj) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(obj);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_backcolor);
  }
}

/* gui_vslider()
 *       slider = pointer to object to draw
 *
 *  draw (vert) slider. part of a scroll object
 *
 */
void gui_vslider(struct SLIDER *slider) {
  if (is_exposing()) {
    int x, y;
    const struct RECT *objrect = GUIRECT(slider);
    struct BITMAP *bit = get_static_bitmap(ID_SCROLL_BACK, 16, 16);
    
    // draw the scrollback
    for (y = 0; y < gui_h(slider); y += gui_h(bit))
      for (x = 0; x < gui_w(slider); x += gui_w(bit))
        bitmap_blit(bit, GUISCREEN, 0, 0, objrect->left + x, objrect->top + y, gui_w(bit) - 1, gui_h(bit) - 1, FALSE);
    atom_delete((struct ATOM *) bit);
    
    // draw the button/slider/button
    bool darken = obj_disabled(GUIOBJ(slider));
    bit = get_static_bitmap(ID_SCROLL_UP, SLIDER_SIZE, SLIDER_SIZE);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->top + SLIDER_SIZE, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->top + SLIDER_SIZE, darken);
    atom_delete((struct ATOM *) bit);
    bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top + slider->upper + SLIDER_SIZE, objrect->right, objrect->top + slider->lower - (SLIDER_SIZE + 1), gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top + slider->upper + SLIDER_SIZE, objrect->right, objrect->top + slider->lower - (SLIDER_SIZE + 1), darken);
    atom_delete((struct ATOM *) bit);
    bit = get_static_bitmap(ID_SCROLL_DOWN, SLIDER_SIZE, SLIDER_SIZE);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->bottom - SLIDER_SIZE, objrect->right, objrect->bottom, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->bottom - SLIDER_SIZE, objrect->right, objrect->bottom, darken);
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_hslider()
 *       slider = pointer to object to draw
 *
 *  draw (horz) slider. part of a scroll object
 *
 */
void gui_hslider(struct SLIDER *slider) {
  if (is_exposing()) {
    int x, y;
    const struct RECT *objrect = GUIRECT(slider);
    struct BITMAP *bit = get_static_bitmap(ID_SCROLL_BACK, 16, 16);
    
    // draw the scroll background
    for (y = 0; y < gui_h(slider); y += gui_h(bit))
      for (x = 0; x < gui_w(slider); x += gui_w(bit))
         bitmap_blit(bit, GUISCREEN, 0, 0, objrect->left + x, objrect->top + y, gui_w(bit) - 1, gui_h(bit) - 1, FALSE);
    atom_delete((struct ATOM *) bit);
    
    // draw the button/slider/button
    bool darken = obj_disabled(GUIOBJ(slider));
    bit = get_static_bitmap(ID_SCROLL_LEFT, SLIDER_SIZE, SLIDER_SIZE);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->left + SLIDER_SIZE, objrect->bottom, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->left + SLIDER_SIZE, objrect->bottom, darken);
    atom_delete((struct ATOM *) bit);
    bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
    bitmap_rectfill(GUISCREEN, objrect->left + slider->upper + SLIDER_SIZE, objrect->top, objrect->left + slider->lower - (SLIDER_SIZE + 1), objrect->bottom, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left + slider->upper + SLIDER_SIZE, objrect->top, objrect->left + slider->lower - (SLIDER_SIZE + 1), objrect->bottom, darken);
    atom_delete((struct ATOM *) bit);
    bit = get_static_bitmap(ID_SCROLL_RIGHT, SLIDER_SIZE, SLIDER_SIZE);
    bitmap_rectfill(GUISCREEN, objrect->right - SLIDER_SIZE, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->right - SLIDER_SIZE, objrect->top, objrect->right, objrect->bottom, darken);
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_scrollcorner()
 *       scrollcorner = pointer to object to draw
 *
 * If there is a horz and vert scroll bar, the little box where they connect needs to be displayed.
 * This routine simply fills in that box with the current Window color
 *
 */
void gui_scrollcorner(struct OBJECT *scrollcorner) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(scrollcorner);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
  }
}

/* gui_scroll()
 *       scroll = pointer to object to draw
 *
 * simply draws a background for the scrolling area
 *
 */
void gui_scroll(struct SCROLL *scroll) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(scroll);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
  }
}

/* gui_textual_draw()
 *       textual = pointer to textual object to draw
 *          left = X1 coordinate (absolute)
 *           top = Y1 coordinate (absolute)
 *         right = X2 coordinate (absolute)
 *        bottom = Y2 coordinate (absolute)
 *       offsetx = offset to add/sub from left coordinate
 *       offsety = offset to add/sub from top coordinate
 *
 *   draw text to the screen, with optional border, etc.
 *
 */
void gui_textual_draw(const TEXTUAL *textual, int left, int top, int right, int bottom, int offsetx, int offsety) {
  int len = 0;
  const char *text = string_text(&textual->text, &len);
  int fore = textual->fore_color;
  int back = textual->back_color;
  struct RECT rect;
  bit32u flags = textual->flags;
  
  // if disabled, change the foreground color
  if (obj_disabled(GUIOBJ(textual)))
    fore = gui_armcolor;
  // if it is selected, change the colors
  else if (obj_selected(GUIOBJ(textual))) {
    fore = gui_selectforecolor;
    back = gui_selectbackcolor;
  } else if ((textual->hovering) && (textual->flags & TEXTUAL_FLAGS_ISLINK)) {
    fore = GUICOLOR_steelblue1;
    flags |= TEXTUAL_FLAGS_UNDERLINE;
  }
  
  // place a border around the field?
  if (textual->flags & TEXTUAL_FLAGS_BORDER) {
    if (textual->decorator)
      bitmap_decorate(textual->decorator, GUISCREEN, GUIDEF, GUIDEF, left, top, right, bottom, FALSE);
    else {
      struct BITMAP *bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left, top, right, bottom, FALSE);
      atom_delete((struct ATOM *) bit);
      bitmap_rectfill(GUISCREEN, left + 1, top + 1, right - 1, bottom - 1, back);
    }
    offsetx += 2;
    offsety += 2;
  } else {
    rect.bottom = bottom;
    rect.left = left;
    rect.right = right;
    rect.top = top;
    expose_background(&rect);
  }
  
  // if has text, draw the text
  if (text) {
    gui_textual_align(textual, &left, &top, right, bottom);
    font_bitmap_drawblock(textual->font, GUISCREEN, text, len, left + offsetx, top + offsety, fore, back, flags);
  }
}

/* gui_textual()
 *       textual = pointer to textual object to draw
 *
 *   draw text to the screen, with optional border, etc.
 *
 */
void gui_textual(struct TEXTUAL *text) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(text);
    gui_textual_draw(text, objrect->left, objrect->top, objrect->right, objrect->bottom, 0, 0);
  }
}

/* gui_progress()
 *       progress = pointer to object to draw
 *
 *   draw the progress bar image
 *
 */
void gui_progress(struct PROGRESS *progress) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(progress);
    struct BITMAP *bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
    unsigned width = (unsigned) ((float) (gui_w(progress) - 2) * ((float) progress->percent / 100.0));
    if (width < 15) width = 15;
    
    // draw the background box
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
    bitmap_rectfill(GUISCREEN, objrect->left + 1, objrect->top + 1, objrect->right - 1, objrect->bottom - 1, GUICOLOR_gray);
    atom_delete((struct ATOM *) bit);
    
    // draw the "bubble"
    if (progress->percent > 0) {
      bit = get_static_bitmap(ID_GREEN_BALL, 16, 16);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left + 1, objrect->top + 1, objrect->left + width, objrect->bottom - 1, FALSE);
      atom_delete((struct ATOM *) bit);
    }
    
    // draw the text?
    if (progress->show) {
      char text[16];
      sprintf(text, "%3i%%", progress->percent);
      const unsigned text_width = font_width(GUITEXTUAL(progress)->font, text, -1, FALSE);
      font_bitmap_drawblock(GUITEXTUAL(progress)->font, GUISCREEN, text, strlen(text), objrect->left + 1 + (gui_w(progress) - text_width) / 2,
        objrect->top + 2, GUICOLOR_black, GUICOLOR_transparent, TEXTUAL_FLAGS_NONE);
    }
  }
}

/* gui_checkbox()
 *       checkbox = pointer to object to draw
 *
 *   draw the check box image
 *
 */
void gui_checkbox(struct CHECK_BOX *checkbox) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(checkbox);
    struct BITMAP *bit;
    int left = objrect->left;
    int top = objrect->top;
    int right = objrect->right;
    int bottom = objrect->bottom;
    int len = 0;
    const char *text = textual_text(GUITEXTUAL(checkbox), &len);
    
    // calculate where box and text are, horizontally
    unsigned box_offset, txt_offset;
    if (checkbox->right) {
      box_offset = textual_width(GUITEXTUAL(checkbox)) + CHECK_BOX_WIDTH;
      txt_offset = 0;
    } else {
      box_offset = 0;
      txt_offset = CHECK_BOX_WIDTH;
    }
    
    // clean up the button area (allow the window's background to show through)
    expose_background(objrect);
    if (!obj_isvisible(GUIOBJ(checkbox)))
      return;
    
    if (obj_disabled(GUIOBJ(checkbox))) {
      // draw check box
      bitmap_box(GUISCREEN, left + 2 + box_offset, top + 2, left + CHECK_BOX_WIDTH + box_offset, top + CHECK_BOX_WIDTH, 2, GUICOLOR_gray48, GUICOLOR_gray65);
      // draw the 'checked'
      if (checkbox->checked)
        bitmap_rectfill(GUISCREEN, left + 5 + box_offset, top + 5, left + CHECK_BOX_WIDTH - 3 + box_offset, top + CHECK_BOX_WIDTH - 3, GUICOLOR_gray55);
    } else {
      // draw check box
      bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left + 2 + box_offset, top + 2, left + CHECK_BOX_WIDTH + box_offset, top + CHECK_BOX_WIDTH, FALSE);
      atom_delete((struct ATOM *) bit);
      // draw the 'checked'
      if (checkbox->checked) {
        bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
        bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left + 5 + box_offset, top + 5, left + CHECK_BOX_WIDTH - 3 + box_offset, top + CHECK_BOX_WIDTH - 3, FALSE);
        atom_delete((struct ATOM *) bit);
      }
    }
    
    // if text, draw the text
    if (text) {
      int fore = (obj_disabled(GUIOBJ(checkbox)) ? gui_disabledcolor : gui_forecolor);
      gui_textual_align(GUITEXTUAL(checkbox), &left, &top, right, bottom);
      font_bitmap_drawblock(GUITEXTUAL(checkbox)->font, GUISCREEN, text, len, left + txt_offset + 5, top + 1, fore, GUICOLOR_transparent, TEXTUAL_FLAGS_MNEMONIC);
      // if this check box is the current focus, draw a dotted line around the text part
      if ((obj_get_focus() == GUIOBJ(checkbox)) && grx_args.showfocus)
        bitmap_box_dotted(GUISCREEN, left + txt_offset + 1, top + 1, right - 1, bottom - 1, GUICOLOR_black);
    }
  }
}

/* gui_onoff()
 *       onoff = pointer to object to draw
 *
 *   draw the onoff image
 *
 */
void gui_onoff(struct ONOFF *onoff) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(onoff);
    
    // clean up the onoff area (allow the window's background to show through)
    expose_background(objrect);
    
    // draw onoff from static image (bitmap has two images)
    onoff->bitmap->current = (onoff->on) ? 1 : 0;
    bitmap_blit(onoff->bitmap, GUISCREEN, 0, 0, objrect->left, objrect->top, gui_w(onoff->bitmap), gui_h(onoff->bitmap), FALSE);
    
    // if this onoff is the current focus, draw a dotted line around the text part
    if ((obj_get_focus() == GUIOBJ(onoff)) && grx_args.showfocus)
      bitmap_box_dotted(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, GUICOLOR_black);
  }
}

/* gui_radio()
 *       radio = pointer to object to draw
 *
 *   draw the radio image
 *
 */
void gui_radio(struct RADIO *radio) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(radio);
    const unsigned radius = RADIO_WIDTH / 2;
    int left = objrect->left;
    int top = objrect->top;
    int right = objrect->right;
    int bottom = objrect->bottom;
    int len = 0;
    const char *text = textual_text(GUITEXTUAL(radio), &len);
    PIXEL white = GUICOLOR_white;
    PIXEL black = GUICOLOR_black;
    
    // if disabled, change colors
    if (obj_disabled(GUIOBJ(radio))) {
      white = GUICOLOR_gray50;
      black = GUICOLOR_gray50;
    }
    
    // calculate where radio and text are, horizontally
    unsigned rad_offset, txt_offset;
    if (radio->right) {
      rad_offset = textual_width(GUITEXTUAL(radio)) + RADIO_WIDTH;
      txt_offset = 0;
    } else {
      rad_offset = 0;
      txt_offset = RADIO_WIDTH;
    }
    
    // clean up the background area (allow the window's background to show through)
    expose_background(objrect);
    if (!obj_isvisible(GUIOBJ(radio)))
      return;
    
    // draw a circle and fill it if it is set
    bitmap_circle(GUISCREEN, left + radius + rad_offset, top + radius, radius, 135, 314, white);
    bitmap_circle(GUISCREEN, left + radius + rad_offset, top + radius, radius, 315, 134, black);
    
    // draw set radio button
    if (radio->set) {
      bitmap_circle(GUISCREEN, left + radius + rad_offset, top + radius, radius - 2, 0, 359, black);
      bitmap_circle(GUISCREEN, left + radius + rad_offset, top + radius, radius - 3, 135, 314, black);
      bitmap_circle(GUISCREEN, left + radius + rad_offset, top + radius, radius - 3, 315, 134, white);
    }
    
    // if text, draw the text
    if (text) {
      int fore = (obj_disabled(GUIOBJ(radio)) ? gui_disabledcolor : gui_forecolor);
      gui_textual_align(GUITEXTUAL(radio), &left, &top, right, bottom);
      font_bitmap_drawblock(GUITEXTUAL(radio)->font, GUISCREEN, text, len, left + txt_offset + 5, top + 1, fore, GUICOLOR_transparent, TEXTUAL_FLAGS_MNEMONIC);
      // if this radio button is the current focus, draw a dotted line around the text part
      if ((obj_get_focus() == GUIOBJ(radio)) && grx_args.showfocus)
        bitmap_box_dotted(GUISCREEN, left + txt_offset + 1, top + 1, right - 1, bottom - 1, GUICOLOR_black);
    }
  }
}

/* gui_thumb()
 *       thumb = pointer to object to draw
 *
 *   draw the thumb image (part of a sliderbar object)
 *
 */
void gui_thumb(struct THUMB *thumb) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(thumb);
    const struct SLIDERBAR *sliderbar = (const struct SLIDERBAR *) GUIOBJ(thumb)->parent;
    
    // clean up the background area (allow the window's background to show through)
    expose_background(objrect);
    
    if (((struct SLIDERBAR *) (GUIOBJ(thumb)->parent))->flags & SLIDERBAR_POINTED) {
      // VERTICAL control (horz thumb)
      if (sliderbar->flags & SLIDERBAR_VERT) {
        const int x = objrect->right - (SLIDERBAR_THUMBW / 2);
        // heart of thumb
        bitmap_rectfill(GUISCREEN, objrect->left + 2, objrect->top + 2, x + 2, objrect->bottom - 2, gui_wincolor);
        
        // "top" of thumb (blunt end)
        bitmap_vline(GUISCREEN, objrect->left + 0, objrect->top, objrect->bottom, GUIRGB(255, 255, 255));
        bitmap_vline(GUISCREEN, objrect->left + 1, objrect->top, objrect->bottom, GUIRGB(224, 224, 224));
        
        // "left" side of thumb (bottom)
        bitmap_hline(GUISCREEN, objrect->left + 0, objrect->bottom - 0, x, GUIRGB(000, 000, 000));
        bitmap_hline(GUISCREEN, objrect->left + 1, objrect->bottom - 1, x, GUIRGB(127, 127, 127));
        
        // "right" side of thumb (top)
        bitmap_hline(GUISCREEN, objrect->left + 0, objrect->top + 0, x, GUIRGB(255, 255, 255));
        bitmap_hline(GUISCREEN, objrect->left + 1, objrect->top + 1, x, GUIRGB(224, 224, 224));
        
        // point of thumb
        bitmap_line(GUISCREEN, x + 2, objrect->top + 2, objrect->right - 2, objrect->bottom - (SLIDERBAR_THUMBW / 2), gui_wincolor);
        bitmap_line(GUISCREEN, x + 1, objrect->top + 1, objrect->right - 1, objrect->bottom - (SLIDERBAR_THUMBW / 2), GUIRGB(204, 204, 204));
        bitmap_line(GUISCREEN, x + 0, objrect->top + 0, objrect->right - 0, objrect->bottom - (SLIDERBAR_THUMBW / 2), GUIRGB(240, 240, 240));
        
        bitmap_line(GUISCREEN, x + 2, objrect->bottom - 2, objrect->right - 2, objrect->bottom - (SLIDERBAR_THUMBW / 2), gui_wincolor);
        bitmap_line(GUISCREEN, x + 1, objrect->bottom - 1, objrect->right - 1, objrect->bottom - (SLIDERBAR_THUMBW / 2), GUIRGB(127, 127, 127));
        bitmap_line(GUISCREEN, x + 0, objrect->bottom - 0, objrect->right - 0, objrect->bottom - (SLIDERBAR_THUMBW / 2), GUIRGB(000, 000, 000));
        
      // HORIZONTAL
      } else {
        const int y = objrect->bottom - (SLIDERBAR_THUMBW / 2);
        // heart of thumb
        bitmap_rectfill(GUISCREEN, objrect->left + 2, objrect->top + 2, objrect->right - 2, y + 2, gui_wincolor);
        
        // "top" of thumb (blunt end)
        bitmap_hline(GUISCREEN, objrect->left, objrect->top + 0, objrect->right, GUIRGB(255, 255, 255));
        bitmap_hline(GUISCREEN, objrect->left, objrect->top + 1, objrect->right, GUIRGB(224, 224, 224));
        
        // "left" side of thumb
        bitmap_vline(GUISCREEN, objrect->left + 0, objrect->top + 0, y - 0, GUIRGB(255, 255, 255));
        bitmap_vline(GUISCREEN, objrect->left + 1, objrect->top + 1, y - 0, GUIRGB(224, 224, 224));
        
        // "right" side of thumb
        bitmap_vline(GUISCREEN, objrect->right - 0, objrect->top, y - 0, GUIRGB(000, 000, 000));
        bitmap_vline(GUISCREEN, objrect->right - 1, objrect->top, y - 0, GUIRGB(127, 127, 127));
        
        // point of thumb
        bitmap_line(GUISCREEN, objrect->left + 2, y + 2, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 2, gui_wincolor);
        bitmap_line(GUISCREEN, objrect->left + 1, y + 1, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 1, GUIRGB(224, 224, 224));
        bitmap_line(GUISCREEN, objrect->left + 0, y + 0, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 0, GUIRGB(255, 255, 255));
        
        bitmap_line(GUISCREEN, objrect->right - 2, y + 2, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 2, gui_wincolor);
        bitmap_line(GUISCREEN, objrect->right - 1, y + 1, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 1, GUIRGB(127, 127, 127));
        bitmap_line(GUISCREEN, objrect->right - 0, y + 0, objrect->left + (SLIDERBAR_THUMBW / 2), objrect->bottom - 0, GUIRGB(000, 000, 000));
      }
    } else {
      // just do a "decorated" rectangle
      struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
      bitmap_rectfill(GUISCREEN, objrect->left + 2, objrect->top + 2, objrect->right - 2, objrect->bottom - 2, gui_wincolor);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, obj_disabled(GUIOBJ(thumb)));
      atom_delete((struct ATOM *) bit);
    }
  }
}

/* gui_sliderbar()
 *       sliderbar = pointer to object to draw
 *
 *   draw the sliderbar image
 *
 */
void gui_sliderbar(struct SLIDERBAR *sliderbar) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(sliderbar);
    const int wide = (sliderbar->flags & SLIDERBAR_WIDE) ? SLIDERBAR_WIDE_H : 2;
    const int range = (sliderbar->max - sliderbar->min);
    int textx, i, x, len;
    char text[16];
    struct BITMAP *bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
    
    if (sliderbar->cur)
      sprintf(text, "%+i", sliderbar->cur);
    else
      strcpy(text, "0");  // so sprintf doesn't put a '+0'
    len = strlen(text);
    
    // draw the objects outer edge (raised)
    if (sliderbar->flags & SLIDERBAR_RAISED) {
      expose_background(objrect);
      struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
      atom_delete((struct ATOM *) bit);
    } else
      bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
    
    // VERTICAL
    if (sliderbar->flags & SLIDERBAR_VERT) {
      const int center = objrect->left + (gui_w(sliderbar) / 2);  // horz center
      
      // draw the field
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, center - wide - 2, objrect->top + SLIDERBAR_EDGE, center + wide - 2, objrect->bottom - SLIDERBAR_EDGE, FALSE);
      bitmap_rectfill(GUISCREEN, center - wide, objrect->top + SLIDERBAR_EDGE + 2, (center + wide) - 4, objrect->bottom - SLIDERBAR_EDGE - 2, GUICOLOR_white);
      
      // draw the ticks
      if (sliderbar->flags & SLIDERBAR_TICKS) {
        x = objrect->top + SLIDERBAR_EDGE;
        for (i=0; i<(range / sliderbar->ratio); i++) {
          bitmap_hline(GUISCREEN, center + wide + 4 + 2, x, center + wide + 4 + 2 + 3, GUICOLOR_black);
          x += (int) (sliderbar->incr * (float) sliderbar->ratio);
        }
        // make sure that last one is at the right edge (due to rounding errors with divide's in above)
        bitmap_hline(GUISCREEN, center + wide + 4 + 2, objrect->bottom - SLIDERBAR_EDGE, center + wide + 4 + 2 + 3, GUICOLOR_black);
      }
      
      // draw text
      if ((sliderbar->flags & (SLIDERBAR_WIDE | SLIDERBAR_NUMS)) == (SLIDERBAR_WIDE | SLIDERBAR_NUMS)) {
        if (sliderbar->cur <= (sliderbar->max - (range / 2)))
          textx = objrect->bottom - SLIDERBAR_EDGE - 10 - font_width(NULL, text, len, FALSE);
        else
          textx = objrect->top + SLIDERBAR_EDGE + 10;
        font_bitmap_drawblock(font_find_name("simple"), GUISCREEN, text, len, (center - wide + 1), textx, GUICOLOR_black, GUICOLOR_transparent, TEXTUAL_FLAGS_VERTICAL);
      }
      
    // HORIZONTAL
    } else {
      const int center = objrect->top + (gui_h(sliderbar) / 2);  // vert center
      
      // draw the field
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left + SLIDERBAR_EDGE, center - wide - 2, objrect->right - SLIDERBAR_EDGE, center + wide - 2, FALSE);
      bitmap_rectfill(GUISCREEN, objrect->left + SLIDERBAR_EDGE + 2, center - wide, objrect->right - SLIDERBAR_EDGE - 2, (center + wide) - 4, GUICOLOR_white);
      
      // draw the ticks
      if (sliderbar->flags & SLIDERBAR_TICKS) {
        x = objrect->left + SLIDERBAR_EDGE;
        for (i=0; i<(range / sliderbar->ratio); i++) {
          bitmap_vline(GUISCREEN, x, center + wide + 4 + 2, center + wide + 4 + 2 + 3, GUICOLOR_black);
          x += (int) (sliderbar->incr * (float) sliderbar->ratio);
        }
        // make sure that last one is at the right edge (due to rounding errors with divide's in above)
        bitmap_vline(GUISCREEN, objrect->right - SLIDERBAR_EDGE, center + wide + 4 + 2, center + wide + 4 + 2 + 3, GUICOLOR_black);
      }
      
      // draw text
      if ((sliderbar->flags & (SLIDERBAR_WIDE | SLIDERBAR_NUMS)) == (SLIDERBAR_WIDE | SLIDERBAR_NUMS)) {
        if (sliderbar->cur <= (sliderbar->max - (range / 2)))
          textx = objrect->right - SLIDERBAR_EDGE - 10 - font_width(NULL, text, len, FALSE);
        else
          textx = objrect->left + SLIDERBAR_EDGE + 10;
        font_bitmap_drawblock(font_find_name("simple"), GUISCREEN, text, len, textx, (center - wide + 1), GUICOLOR_black, GUICOLOR_transparent, TEXTUAL_FLAGS_NONE);
      }
    }
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_updown()
 *       updown = pointer to object to draw
 *
 *   draw the updown image
 *   (draw only the rectangle, the GUI will draw the children for us)
 */
void gui_updown(struct UPDOWN *updown) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(updown);
    struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
    
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_image()
 *       image = pointer to object to draw
 *
 *   draw an image object
 *   
 */
void gui_image(struct IMAGE *image) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(image);
    
    expose_background(objrect);
    if (obj_isvisible(GUIOBJ(image)))
      bitmap_blit(image->bitmap, GUISCREEN, 0, 0, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
  }
}

/* gui_button()
 *       button = pointer to object to draw
 *
 *   draw a button object
 *   
 */
void gui_button(struct BUTTON *button) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(button);
    int offset = 0;
    struct BITMAP *bit;
    
    // clean up the background area (allow the window's background to show through)
    expose_background(objrect);
    
    int fore = gui_forecolor;
    int left = objrect->left;
    int top = objrect->top;
    int right = objrect->right;
    int bottom = objrect->bottom;
    int len = 0;
    const char *text = textual_text(GUITEXTUAL(button), &len);
    
    // Special handling for close, min, max, and resize buttons
    if ((GUIID(eventstack.object) == ID_BORDER_CLOSE)  ||
        (GUIID(eventstack.object) == ID_BORDER_MIN)    ||
        (GUIID(eventstack.object) == ID_BORDER_MAX)    ||
        (GUIID(eventstack.object) == ID_BORDER_RESIZE)) {
      const bool darken = ((obj_disabled(GUIOBJ(eventstack.object)) || !win_active(GUIOBJ(button)->win)) && 
                           (GUIID(eventstack.object) != ID_BORDER_RESIZE));
      bit = ((struct BUTTON *) eventstack.object)->bitmap;
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left, top, right, bottom, darken);
      return;
    }
    
    // User supplied bitmap buttons?
    if (button->bitmap) {
      // Here is were we can check to see if the button is selected or not.
      // If the button is not pressed, use the first bitmap of button->bitmap
      // If the button is pressed, use the second bitmap.
      if ((button->bitmap->count > 1) && obj_armed(GUIOBJ(button)))
        button->bitmap->current = 1;
      else
        button->bitmap->current = 0;
      bitmap_blit(button->bitmap, GUISCREEN, 0, 0, left, top, gui_w(button->bitmap), gui_h(button->bitmap), obj_disabled(GUIOBJ(button)));
    } else {
      // Normal buttons
      if (obj_selected(GUIOBJ(button))) {
        bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
        offset = 1;
      } else if (obj_armed(GUIOBJ(button)))
        bit = get_static_bitmap(ID_BUTTON_DOWN, 6, 6);
      else
        bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
      
      if (obj_disabled(GUIOBJ(button)))
        fore = gui_disabledcolor;
      
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left, top, right, bottom, FALSE);
      atom_delete((struct ATOM *) bit);
    }
    
    if (text) {
      gui_textual_align(GUITEXTUAL(button), &left, &top, right, bottom);
      font_bitmap_drawblock(GUITEXTUAL(button)->font, GUISCREEN, text, len, left + offset, top + offset, fore, fore, GUITEXTUAL(button)->flags);
      // if this button is the current focus, draw a dotted line around the text part
      if ((obj_get_focus() == GUIOBJ(button)) && grx_args.showfocus)
        bitmap_box_dotted(GUISCREEN, left + offset + 1, top + offset + 1, right - 2, bottom - 2, GUICOLOR_black);
    }
  }
}

/* gui_taskbar()
 *       taskbar = pointer to object to draw
 *
 *   draw a taskbar object
 *   (draw only the rectangle, the GUI will draw the children for us)
 *   
 */
void gui_taskbar(struct TASKBAR *taskbar) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(taskbar);
    struct BITMAP *bit = get_static_bitmap(ID_TASKBAR_UP, 0, 0);
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_menu()
 *       menu = pointer to object to draw
 *
 *   This displays the background rectangle for the menu
 *   
 */
void gui_menu(struct MENU *menu) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(menu);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, ((menu->back_color) ? menu->back_color : gui_wincolor));
    if (menu->parent) {
      struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
      atom_delete((struct ATOM *) bit);
    }
  }
}

/* gui_menu_button()
 *       menu_button = pointer to object to draw
 *
 *   this displays a button within the menu
 *   
 */
void gui_menu_button(struct MENU_BUTTON *menu_button) {
  if (is_exposing()) {
    // display button.
    if (GUIBUTTON(menu_button)->bitmap)
      gui_button((struct BUTTON *) menu_button);
    else {
      const struct RECT *objrect = GUIRECT(menu_button);
      struct TEXTUAL *textual = (struct TEXTUAL *) menu_button;
      PIXEL fore = textual->fore_color;
      int len, x_off = (menu_button->flags & MENU_BTN_HAS_OFF) ? 16 : 0;
      
      // if is separator, display a sunken line
      if (menu_button->flags & MENU_BTN_SEPARATOR) {
        bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
        if ((objrect->right - 10) > (objrect->left + 10)) {
          bitmap_hline(GUISCREEN, objrect->left + 10, objrect->top + (MENU_SEPRTR_H / 2), objrect->right - 10, GUIRGB(120,124,120));
          bitmap_hline(GUISCREEN, objrect->left + 10, objrect->top + ((MENU_SEPRTR_H / 2) + 1), objrect->right - 10, GUIRGB(248,252,248));
        }
      // else display the button, icon, and text
      } else {
        const char *str = string_text(&textual->text, &len);
        bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
        if ((menu_button->flags & (MENU_BTN_ACTIVE | MENU_BTN_ENABLED)) == (MENU_BTN_ACTIVE | MENU_BTN_ENABLED)) {
          struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
          bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
          atom_delete((struct ATOM *) bit);
        }
        if (!(menu_button->flags & MENU_BTN_ENABLED) || obj_disabled(GUIOBJ(menu_button)))
          fore = gui_disabledcolor;
        if (menu_button->icon)
          bitmap_blit(menu_button->icon, GUISCREEN, 0, 0, objrect->left + 1, objrect->top + 1, gui_w(menu_button->icon) - 1, gui_h(menu_button->icon) - 1, FALSE);
        font_bitmap_drawblock(textual->font, GUISCREEN, str, len, objrect->left + 3 + x_off, objrect->top + 3, fore, gui_wincolor, textual->flags);
        if (menu_button->right)
          bitmap_blit(menu_button->right, GUISCREEN, 0, 0, objrect->right - 17, objrect->top + 1, gui_w(menu_button->right) - 1, gui_h(menu_button->right) - 1, FALSE);
      }
    }
  }
}

/* gui_button_bar()
 *       button_bar = pointer to object to draw
 *
 *   this displays the background for the button bar
 *   
 */
void gui_button_bar(struct BUTTON_BAR *button_bar) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(button_bar);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
  }
}

/* gui_button_bar_button()
 *       button_bar_button = pointer to object to draw
 *
 *   this displays the button on the button bar
 *   
 */
void gui_button_bar_button(struct BUTTON_BAR_BUTTON *button_bar_button) {
  if (is_exposing()) {
    // display button.
    const struct RECT *objrect = GUIRECT(button_bar_button);
    
    // clean up the background area (allow the window's background to show through)
    expose_background(objrect);
    
    if ((button_bar_button->flags & (BUTTON_BTNBAR_ACTIVE | BUTTON_BTNBAR_ENABLED)) == (BUTTON_BTNBAR_ACTIVE | BUTTON_BTNBAR_ENABLED)) {
      struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
      bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, objrect->left, objrect->top, objrect->right, objrect->bottom, FALSE);
      atom_delete((struct ATOM *) bit);
    }
    if (button_bar_button->icon) {
      bitmap_rectfill(GUISCREEN, objrect->left + 1, objrect->top + 1, gui_w(button_bar_button->icon) - 1, gui_h(button_bar_button->icon) - 1, gui_wincolor);
      bool darken = obj_disabled(GUIOBJ(button_bar_button));
      bitmap_blit(button_bar_button->icon, GUISCREEN, 0, 0, objrect->left + 1, objrect->top + 1, gui_w(button_bar_button->icon) - 1, gui_h(button_bar_button->icon) - 1, darken);
    }
  }
}

/* gui_list()
 *       list = pointer to object to draw
 *
 *   this displays the background for the list
 *   
 */
void gui_list(struct LIST *list) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(list);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, GUIRGB(255, 255, 255));
  }
}

/* gui_listelem()
 *       listelem = pointer to object to draw
 *
 *   this displays the list element on the list
 *   
 */
void gui_listelem(struct LISTELEM *listelem) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(listelem);
    int left = objrect->left + 2;
    
    // clean up the background area (allow the window's background to show through)
    expose_background(objrect);
    
    if (listelem->icon) {
      bitmap_blit(listelem->icon, GUISCREEN, 0, 0, left, objrect->top, gui_w(listelem->icon), gui_h(listelem->icon), FALSE);
      left += gui_w(listelem->icon) + 2;
    }
    gui_textual_draw(GUITEXTUAL(listelem), left, objrect->top, objrect->right, objrect->bottom, 0, 0);
  }
}

/* gui_root()
 *       root = pointer to object to draw
 *
 *   display the root (the desktop)
 *   
 *  TODO:
 *    once the desktop image is created, it never changes, yet we draw it to the GUISCEEN (screen buffer)
 *     quite often, even for just a mouse cursor move.  This must take time.  We should find a more
 *     efficient way to do this.
 *    with this in mind, we can't put a random pattern here, because it would have to redraw the whole
 *     random pattern each time it was called, yet only the small portion would be pushed to the physical
 *     screen.  Therefore, it would be distorted.
 *
 */
void gui_root(struct WIN *root) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(root);
    
    // is there a background image to display
    if (desktop_img.bitmap) {
      const int w = MIN(gui_w(desktop_img.bitmap), grx_args.w);
      const int h = MIN(gui_h(desktop_img.bitmap), grx_args.h);
      // if stretch it, use the whole screen
      if (desktop_img.flags & IMAGE_FLAGS_STRETCH)
        bitmap_blitstretch(desktop_img.bitmap, GUISCREEN, 0, 0, w, h, 0, 0, grx_args.w, grx_args.h);
      else {
        // if it doesn't use the whole screen draw a background color first
        int x = 0, y = 0;
        if ((w < (objrect->right + 1 - objrect->left)) || (h < (objrect->bottom + 1 - objrect->top))) {
          bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_desktop_color);
          x = (((objrect->right + 1 - objrect->left) - w) / 2);
          y = (((objrect->bottom + 1 - objrect->top) - h) / 2);
        }
        // draw image to desktop
        bitmap_blitcopy(desktop_img.bitmap, GUISCREEN, 0, 0, objrect->left + x, objrect->top + y, w, h);
      }
    } else {
      // no desktop image specified, so draw something (blank color?)
      bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_desktop_color);
    }    
  }
}

/* gui_winborder()
 *       border = pointer to object to draw
 *
 *   draw the window border
 *   
 */
void gui_winborder(struct WINBORDER *border) {
  if (is_exposing()) {
    struct WIN *owner = GUIOBJ(border)->win;
    
#if USE_TRANPARENT_TITLES
    struct RECT title_rect;
#endif
    const struct RECT *objrect = GUIRECT(border);
    int fill = gui_armcolor;
    int fore = gui_disabledcolor;
    int left = objrect->left;
    int top = objrect->top;
    int right = objrect->right;
    int bottom = objrect->bottom;
    int title_height = (owner) ? gui_top(owner) - 1 : 10;
    struct BITMAP *bit = get_static_bitmap(ID_BUTTON_UP, 6, 6);
    
    if (win_active(owner)) {
      fill = gui_selectbackcolor;
      fore = gui_selectforecolor;
    }
    
#if USE_TRANPARENT_TITLES
    // fill the whole window minus the titlebar with the win_color
    bitmap_rectfill(GUISCREEN, left, title_height, right, bottom, gui_wincolor);
    // make it a button style
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left, title_height, right, bottom, FALSE);
    
    title_rect = *objrect;
    title_rect.bottom = title_height;
    expose_background(&title_rect);
    bitmap_rectfill(GUISCREEN, left, top, right, title_height, GUITRANS(0x80, fill));
#else
    // make it a button style
    bitmap_decorate(bit, GUISCREEN, GUIDEF, GUIDEF, left, top, right, bottom, FALSE);
    
    // draw title bar background
    bitmap_rectfill(GUISCREEN, left + 2, top + 2, right - 2, title_height, fill);
    
    // fill the rest of the window with the win_color
    bitmap_rectfill(GUISCREEN, left + 2, title_height + 1, right - 2, bottom - 2, gui_wincolor);
#endif
    
    if (owner) {
      int len = 0, offset = 0;
      if (owner->icon) {
        bitmap_blit(owner->icon, GUISCREEN, 0, 0, left + 3, top + 3, gui_w(owner->icon) - 1, gui_h(owner->icon) - 1, FALSE);
        offset = 18;
      }
      char *text = (char *) textual_text(GUITEXTUAL(owner), &len);
      if (text) {
        gui_textual_align(GUITEXTUAL(owner), &left, &top, right, bottom);
        font_bitmap_drawblock(GUITEXTUAL(owner)->font, GUISCREEN, text, len, left + 3 + offset, top + 3, fore, fore, TEXTUAL_FLAGS_NONE);
      }
      if (border->flags & BORDER_HAS_STATUS) {
        char *text = (char *) textual_text(&border->status, &len);
        font_bitmap_drawblock(border->status.font, GUISCREEN, text, len, gui_left(&border->status), gui_top(&border->status), 
          border->status.fore_color, border->status.back_color, TEXTUAL_FLAGS_NONE);
      }
    }
    atom_delete((struct ATOM *) bit);
  }
}

/* gui_win()
 *       win = pointer to object to draw
 *
 *   draw the window
 *   draws a rect background....
 *   
 */
void gui_win(struct WIN *win) {
  if (is_exposing()) {
    const struct RECT *objrect = GUIRECT(win);
    bitmap_rectfill(GUISCREEN, objrect->left, objrect->top, objrect->right, objrect->bottom, gui_wincolor);
  }
}
