/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * scroll.cpp
 *  
 *  this is the scroll object file
 *
 */

#include "gui.h"
#include "grfx.h"
#include "palette.h"

/*  scroll_hscroll()
 *       scroll = pointer to the scroll object
 *
 *   create a horz scroll bar (hslider)
 *   
 */
void scroll_hscroll(struct SCROLL *scroll) {
  if (!scroll->hscroll) {
    scroll->hscroll = obj_hslider(NULL, 0, GUIOBJ(scroll), 0);
    obj_defaultrect(GUIOBJ(scroll->hscroll), 0);
  }
}

/*  scroll_vscroll()
 *       scroll = pointer to the scroll object
 *
 *   create a vert scroll bar (vslider)
 *   
 */
void scroll_vscroll(struct SCROLL *scroll) {
  if (!scroll->vscroll) {
    scroll->vscroll = obj_vslider(NULL, 0, GUIOBJ(scroll), 0);
    obj_defaultrect(GUIOBJ(scroll->vscroll), 0);
  }
}

/*  scrollcorner_class()
 *     no parameters
 *
 *   creates that small little box in the right-hand corner when
 *    both horz and vert sliders are visible.  if we didn't do
 *    something with it, it would not be drawn
 *   
 */
void scrollcorner_class(void) {
  switch (eventstack.event) {
    // draw to screen
    case EXPOSE:
      gui_scrollcorner(GUIOBJ(eventstack.object));
      return;
  }
  
  // else let the object class handle it
  obj_class();
}

/*  scroll_geometry()
 *       scroll = pointer to the scroll object
 *         left = X coordinate relative to left of scroll object
 *          top = Y coordinate relative to top of scroll object
 *            w = width of visible scroll area
 *            h = height of visible scroll area
 *
 *  sets the geometry of the visible part of the scroll area, creating
 *   horz and vert (and corner) objects when needed.
 *   
 */
void scroll_geometry(struct SCROLL *scroll, int left, int top, int w, int h) {
  int vsize = 0;
  int hsize = 0;
  bool need_v = FALSE;
  bool need_h = FALSE;
  
  scroll->x = left;
  scroll->y = top;
  scroll->w = w;
  scroll->h = h - 1;
  scroll->vw = gui_w(scroll) - 1;
  scroll->vh = gui_h(scroll);
  
  if (scroll->h > scroll->vh) {
    scroll_vscroll(scroll);
    scroll->vw -= gui_w(scroll->vscroll);
    need_v = TRUE;
  }
  if (scroll->w > scroll->vw) {
    scroll_hscroll(scroll);
    scroll->vh -= gui_h(scroll->hscroll);
    need_h = TRUE;
  }
  if ((scroll->h > scroll->vh) && !need_v) {
    scroll_vscroll(scroll);
    scroll->vw -= gui_w(scroll->vscroll);
    need_v = TRUE;
  }
  if ((scroll->w > scroll->vw) && !need_h) {
    scroll_hscroll(scroll);
    scroll->vh -= gui_h(scroll->hscroll);
    need_h = TRUE;
  }
  
  if (!need_h && scroll->hscroll) {
    atom_delete(&scroll->hscroll->base.atom);
    scroll->hscroll = NULL;
  }
  if (!need_v && scroll->vscroll) {
    atom_delete(&scroll->vscroll->base.atom);
    scroll->vscroll = NULL;
  }
  
  node_remove(GUIOBJ(&scroll->text));
  node_insert(GUIOBJ(&scroll->text), GUIOBJ(scroll));
  
  if (scroll->vscroll) {
    vsize = gui_w(scroll->vscroll);
    
    obj_position(GUIOBJ(scroll->vscroll), gui_w(scroll) - vsize, 0, vsize, scroll->vh);
    obj_geometry(GUIOBJ(scroll->vscroll));
    
    slider_set(scroll->vscroll, scroll->h, scroll->vh, -top);
    
    node_remove(GUIOBJ(scroll->vscroll));
    node_insert(GUIOBJ(scroll->vscroll), GUIOBJ(scroll));
  }
  
  if (scroll->hscroll) {
    hsize = gui_h(scroll->hscroll);
    
    obj_position(GUIOBJ(scroll->hscroll), 0, gui_h(scroll) - hsize, scroll->vw + 1, hsize);
    obj_geometry(GUIOBJ(scroll->hscroll));
    
    slider_set(scroll->hscroll, scroll->w, scroll->vw, -left);
    
    node_remove(GUIOBJ(scroll->hscroll));
    node_insert(GUIOBJ(scroll->hscroll), GUIOBJ(scroll));
  }
  
  if (scroll->vscroll && scroll->hscroll) {
    if (!scroll->corner)
      scroll->corner = (struct OBJECT *) object(NULL, scrollcorner_class, 0, GUIOBJ(scroll), 0);
    obj_position(scroll->corner, gui_w(scroll) - vsize, gui_h(scroll) - hsize, vsize, hsize);
    
    node_remove(scroll->corner);
    node_insert(scroll->corner, GUIOBJ(scroll));
  } else if (scroll->corner) {
    atom_delete(&scroll->corner->base.atom);
    scroll->corner = NULL;
  }
}

/*  scroll_scrollable_element()
 *       scroll = pointer to the scroll object
 *          ptr = scrollable object pointer
 *
 *  returns TRUE if the object pointed to be ptr is part of the scrollable area
 *    and not the scroll object items
 *   
 */
bool scroll_scrollable_element(const struct SCROLL *scroll, const struct OBJECT *ptr) {
  if ((ptr != GUIOBJ(scroll->vscroll)) && (ptr != GUIOBJ(scroll->hscroll)) &&
      (ptr != GUIOBJ(&scroll->text)) && (ptr != scroll->corner))
    return TRUE;
  return FALSE;
}

/*  scroll_do()
 *       scroll = pointer to the scroll object
 *           dx = X delta to scroll
 *           dy = Y delta to scroll
 *
 *  scrolls the view area by dx and dy
 *  (in other words, moves all objects within the scroll area)
 */
void scroll_do(struct SCROLL *scroll, const int dx, const int dy) {
  struct OBJECT *ptr = GUIOBJ(scroll)->last;
  
  while (ptr) {
    if (scroll_scrollable_element(scroll, ptr)) {
      struct RECT pos = *GUIRECT(ptr);
      pos.left -= dx;
      pos.top -= dy;
      pos.right -= dx;
      pos.bottom -= dy;
      rectatom_place(GUIRECTATOM(ptr), &pos);
    }
    ptr = ptr->prev;
  }
  
  scroll->x -= dx;
  scroll->y -= dy;
}

/* scroll_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void scroll_class(void) {
  struct SCROLL *scroll = (struct SCROLL *) eventstack.object;
  const struct POINTER_INFO *info = pointer_info();
  
  switch (eventstack.event) {
    // draw the scroll to the screen
    // (this is only a filled rectangle due to being the back ground
    //   of all other items)
    case EXPOSE:
      gui_scroll(scroll);
      return;
    
    // wheel mouse moved so pass the event to the scrolling
    //  parts (the sliders)
    case MPOINTER_MOVE:
      // if there is a vert scroll, scroll it
      if (scroll->vscroll)
        obj_event(GUIOBJ(scroll->vscroll), MPOINTER_MOVE, NULL);
      // else if no vert and is a horz, scroll it
      else if (scroll->hscroll)
        obj_event(GUIOBJ(scroll->hscroll), MPOINTER_MOVE, NULL);
      return;
      
    // do a horz scroll
    case HSCROLL:
      if (scroll->hscroll) {
        const int dx = slider_value(scroll->hscroll) + scroll->x;
        
        scroll_do(scroll, dx, 0);
        obj_dirty(GUIOBJ(scroll), TRUE);
      }
      break;
      
    // do a vert scroll
    case VSCROLL:
      if (scroll->vscroll) {
        const int dy = slider_value(scroll->vscroll) + scroll->y;
        
        scroll_do(scroll, 0, dy);
        obj_dirty(GUIOBJ(scroll), TRUE);
      }
      break;

    // must catch the focus want and just return.
    //  if we do not, the textual_class call will freeze it
    case FOCUS_WANT:
      return;
  }
  
  // else call the textual class
  textual_class();
}

/*  obj_scroll()
 *       scroll = pointer to the scroll object
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the scroll (usually ignored)
 *
 *  creates a scroll object
 *   
 */
struct SCROLL *obj_scroll(struct SCROLL *scroll, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct SCROLL);
  
  scroll = (struct SCROLL *) object(GUIOBJ(scroll), scroll_class, size, parent, theid);
  
  return scroll;
}
