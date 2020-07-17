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
 *  slidebar.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <stdlib.h>  // need abs()

#include "gui.h"

/*  sliderbar_setmax()
 *    sliderbar = pointer to the sliderbar object
 *          max = max value it will have
 *
 *  sets the max value a sliderbar will have
 *   
 */
void sliderbar_setmax(struct SLIDERBAR *sliderbar, const int max) {
  if (max > sliderbar->min)
    sliderbar->max = max;
}

/*  sliderbar_setmin()
 *    sliderbar = pointer to the sliderbar object
 *          min = min value it will have
 *
 *  sets the min value a sliderbar will have
 *   
 */
void sliderbar_setmin(struct SLIDERBAR *sliderbar, const int min) {
  if (min < sliderbar->max)
    sliderbar->min = min;
}

/*  sliderbar_setlimits()
 *    sliderbar = pointer to the sliderbar object
 *          min = min value it will have
 *          max = max value it will have
 *
 *  sets the min and max values a sliderbar will have
 *   
 */
void sliderbar_setlimits(struct SLIDERBAR *sliderbar, const int min, const int max) {
  sliderbar->min = min;
  if (max > sliderbar->min)
    sliderbar->max = max;
  else
    sliderbar->max = min + 1;
}

/*  sliderbar_setratio()
 *    sliderbar = pointer to the sliderbar object
 *        ratio = increment amount between values
 *
 *  sets the ratio amount for the sliderbar
 *   
 */
void sliderbar_setratio(struct SLIDERBAR *sliderbar, const int ratio) {
  if (ratio < (sliderbar->max - sliderbar->min))
    sliderbar->ratio = ratio;
}

/*  sliderbar_setcur()
 *    sliderbar = pointer to the sliderbar object
 *          cur = current value
 *
 *  sets the current value for the sliderbar
 *   
 */
void sliderbar_setcur(struct SLIDERBAR *sliderbar, const int cur) {
  if ((cur >= sliderbar->min) && (cur <= sliderbar->max))
    sliderbar->cur = cur;
}

/*  sliderbar_getcur()
 *    sliderbar = pointer to the sliderbar object
 *
 *  gets the current value for the sliderbar
 *   
 */
int sliderbar_getcur(struct SLIDERBAR *sliderbar) {
  return sliderbar->cur;
}

/*  sliderbar_setflags()
 *    sliderbar = pointer to the sliderbar object
 *        flags = flags for the sliderbar object
 *
 *  sets the flags for the slider bar object
 *   
 */
void sliderbar_setflags(struct SLIDERBAR *sliderbar, const bit32u flags) {
  sliderbar->flags = flags;
  if (flags & SLIDERBAR_NUMS)
    sliderbar->flags |= SLIDERBAR_WIDE;
  if (!(flags & SLIDERBAR_WIDE))
    sliderbar->flags &= ~SLIDERBAR_NUMS;
}

/*  sliderbar_getflags()
 *    sliderbar = pointer to the sliderbar object
 *
 *  gets the flags for the slider bar object
 *   
 */
bit32u sliderbar_getflags(struct SLIDERBAR *sliderbar) {
  return sliderbar->flags;
}

/*  sliderbar_getflags()
 *    sliderbar = pointer to the sliderbar object
 *            z = postition of mouse on movement
 *
 *  calcuate current position for thumb by mouse position
 *   
 */
int sliderbar_calc_thumb(struct SLIDERBAR *sliderbar, const int z) {
  const struct RECT *objrect = GUIRECT(sliderbar);
  int i, z1, z2;
  
  if (sliderbar->flags & SLIDERBAR_VERT) {
    z1 = objrect->top;
    z2 = objrect->bottom;
  } else {
    z1 = objrect->left;
    z2 = objrect->right;
  }
  
  if ((z >= ((z1 + SLIDERBAR_EDGE) - (SLIDERBAR_THUMBW / 2))) &&
      (z <= ((z2 - SLIDERBAR_EDGE) + (SLIDERBAR_THUMBW / 2)))) {
    i = (int) ((float) (z - (z1 + SLIDERBAR_EDGE)) / sliderbar->incr) - abs(sliderbar->min);
    if (i > sliderbar->max)
      return sliderbar->max;
    if (i < sliderbar->min)
      return sliderbar->min;
    return i;
  } else
    return sliderbar->cur;
}

/* thumb_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void thumb_class(void) {
  struct THUMB *thumb = (struct THUMB *) eventstack.object;
  struct SLIDERBAR *sliderbar = (struct SLIDERBAR *) (GUIOBJ(thumb)->parent);
  const struct POINTER_INFO *info = pointer_info();
  
  switch (eventstack.event) {
    // draw the thumb object of the sliderbar
    case EXPOSE:
      gui_thumb(thumb);
      return;
      
    // allow movement of the thumb
    case ARM:
      obj_dirty(GUIOBJ(thumb), TRUE);
      obj_class();
      obj_wantmove(GUIOBJ(thumb), obj_armed(GUIOBJ(thumb)));
      return;
      
    // allow the mouse to "grab" the thumb
    case LPOINTER_PRESS:
      pointer_hold();
      break;
      
    // we are done moving the thumb
    case LPOINTER_RELEASE:
      obj_dirty(GUIOBJ(thumb), TRUE);
      pointer_release();
      break;
      
    // move the thumb
    case POINTER_MOVE:
      if (obj_armed(GUIOBJ(thumb))) {
        sliderbar->cur = sliderbar_calc_thumb(sliderbar, (sliderbar->flags & SLIDERBAR_VERT) ? info->y : info->x);
        obj_geometry(GUIOBJ(thumb));
        obj_dirty(GUIOBJ(sliderbar), TRUE);
      }
      break;
      
    // set the geometry of the sliderbar and thumb
    case GEOMETRY: {
      const int index = sliderbar->cur - sliderbar->min;
      const int wide = (sliderbar->flags & SLIDERBAR_WIDE) ? SLIDERBAR_WIDE_H : 2;
      const int range = (sliderbar->max - sliderbar->min);
      if (sliderbar->flags & SLIDERBAR_VERT) {
        const int width = gui_h(GUIOBJ(thumb)->parent) - (SLIDERBAR_EDGE * 2);  // width of slider bar (not object)
        const int center = (gui_w(GUIOBJ(thumb)->parent) / 2);  // vert center
        sliderbar->incr = (float) ((float) width / (float) range);
        const int thumbx = SLIDERBAR_EDGE + ((float) index * sliderbar->incr) - (SLIDERBAR_THUMBW / 2);
        const int thumby = (center - wide - 4) - 2;
        obj_position(GUIOBJ(thumb), thumby, thumbx, (wide * 2) + 10, SLIDERBAR_THUMBW);
      } else {
        const int width = gui_w(GUIOBJ(thumb)->parent) - (SLIDERBAR_EDGE * 2);  // width of slider bar (not object)
        const int center = (gui_h(GUIOBJ(thumb)->parent) / 2);  // vert center
        sliderbar->incr = (float) ((float) width / (float) range);
        const int thumbx = SLIDERBAR_EDGE + ((float) index * sliderbar->incr) - (SLIDERBAR_THUMBW / 2);
        const int thumby = (center - wide - 4) - 2;
        obj_position(GUIOBJ(thumb), thumbx, thumby, SLIDERBAR_THUMBW, (wide * 2) + 10);
      }
    } return;
      
    // give a default size for the sliderbar
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      if (sliderbar->flags & SLIDERBAR_VERT) {
        rect->right = rect->left + SLIDERBAR_WIDE_H + 4;
        rect->bottom = rect->top + SLIDERBAR_THUMBW;
      } else {
        rect->right = rect->left + SLIDERBAR_THUMBW;
        rect->bottom = rect->top + SLIDERBAR_WIDE_H + 4;
      }
    } return;
  }
  
  // if we didn't handle the event, let the object class do it
  obj_class();
}

/*  obj_thumb()
 *        thumb = pointer to the thumb object
 *         size = size of memory to allocate
 *       parent = parent object (sliderbar object)
 *        theid = the id for the slider (ignored)
 *
 *  create a thumb object
 *   
 */
struct THUMB *obj_thumb(struct THUMB *thumb, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct THUMB);
  
  thumb = (struct THUMB *) object(GUIOBJ(thumb), thumb_class, size, parent, theid);
  if (thumb) {
    // is armable and selectable
    obj_armable(GUIOBJ(thumb), TRUE);
    obj_selectable(GUIOBJ(thumb), TRUE);
  }
  return thumb;
}

/* sliderbar_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void sliderbar_class(void) {
  struct SLIDERBAR *sliderbar = (struct SLIDERBAR *) eventstack.object;
  int i;
  
  switch (eventstack.event) {
    // draw sliderbar to screen
    case EXPOSE:
      gui_sliderbar(sliderbar);
      return;
      
    // set the geometry of the sliderbar
    case GEOMETRY:
      // make sure at least the min tall
      if (sliderbar->flags & SLIDERBAR_VERT) {
        if (gui_h(sliderbar) < SLIDERBAR_MIN_W)
          obj_resize(GUIOBJ(sliderbar), GUIDEF, SLIDERBAR_MIN_W);
      } else {
        if (gui_w(sliderbar) < SLIDERBAR_MIN_W)
          obj_resize(GUIOBJ(sliderbar), SLIDERBAR_MIN_W, GUIDEF);
      }
      
      // make sure at least the min length
      i = SLIDERBAR_MIN_H;
      if (sliderbar->flags & SLIDERBAR_WIDE)
        i += SLIDERBAR_WIDE_H;
      if (sliderbar->flags & SLIDERBAR_TICKS)
        i += 5 + SLIDERBAR_TICK_H + 2;
      if (sliderbar->flags & SLIDERBAR_VERT) {
        if (gui_w(sliderbar) < i)
          obj_resize(GUIOBJ(sliderbar), i, GUIDEF);
      } else {
        if (gui_h(sliderbar) < i)
          obj_resize(GUIOBJ(sliderbar), GUIDEF, i);
      }
      
      // do the thumb's geometry too
      obj_geometry(GUIOBJ(&sliderbar->thumb));
      break;
      
    // give the sliderbar a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      if (sliderbar->flags & SLIDERBAR_VERT) {
        rect->right = rect->left + SLIDERBAR_MIN_W;
        rect->bottom = rect->top + SLIDERBAR_MIN_H;
      } else {
        rect->right = rect->left + SLIDERBAR_MIN_H;
        rect->bottom = rect->top + SLIDERBAR_MIN_W;
      }
    } return;
  }
  
  // if we didn't handle the event, let the object class do it
  obj_class();
}

/*  obj_sliderbar()
 *    sliderbar = pointer to the sliderbar object
 *         size = size of memory to allocate
 *       parent = parent object (sliderbar object)
 *        theid = the id for the slider (ignored)
 *
 *  create a sliderbar object
 *   
 */
struct SLIDERBAR *obj_sliderbar(struct SLIDERBAR *sliderbar, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct SLIDERBAR);
  
  sliderbar = (struct SLIDERBAR *) object(GUIOBJ(sliderbar), sliderbar_class, size, parent, theid);
  if (sliderbar) {
    // not armable or selectable
    obj_armable(GUIOBJ(sliderbar), FALSE);
    obj_selectable(GUIOBJ(sliderbar), FALSE);
    
    // set defaults
    sliderbar->flags = SLIDERBAR_TICKS | SLIDERBAR_HORZ;
    sliderbar->min = 0;
    sliderbar->max = 10;
    sliderbar->cur = 5;
    sliderbar->ratio = 1;
    
    // create the thumb object too
    obj_thumb(&sliderbar->thumb, 0, GUIOBJ(sliderbar), 0);
    obj_defaultrect(GUIOBJ(&sliderbar->thumb), 0);
  }
  
  // return slider bar object
  return sliderbar;
}
