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
 *  list.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "grfx.h"
#include "gui.h"
#include "palette.h"


/*  list_unlink()
 *     listelem = pointer to list element
 *
 *   inlinks a list element from a list
 */
void list_unlink(struct LISTELEM *listelem) {
  // if not a part of a parent, do nothing
  if (!listelem->list)
    return;
  
  if (listelem->icon)
    atom_delete((struct ATOM *) listelem->icon);
  
  // link neighbors
  if (listelem->prev)
    listelem->prev->next = listelem->next;
  if (listelem->next)
    listelem->next->prev = listelem->prev;
  
  // if was last, link prev as now last
  if (listelem == listelem->list->last)
    listelem->list->last = listelem->prev;
  
  // unlink
  listelem->list = NULL;
  listelem->next = NULL;
  listelem->prev = NULL;
  
  // if had attachment data, free it
  if (listelem->attachment)
    free(listelem->attachment);
}

/*  list_count_selected()
 *     list = pointer to list
 *
 *   returns the count of selected elements in list
 */
int list_count_selected(const struct LIST *list) {
  struct LISTELEM *element;
  int i = 0;
  
  if (!list)
    return 0;
  
  element = list->last;
  while (element) {
    if (obj_selected(GUIOBJ(element)))
      i++;
    element = element->prev;
  }
  
  return i;
}

/* listelem_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list element
 *   eventstack.data = depends on event
 */
void listelem_class(void) {
  struct LISTELEM *listelem = (struct LISTELEM *) eventstack.object;
  struct LIST *list = listelem->list;
  const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
  
  switch (eventstack.event) {
    // kill the element
    case DESTRUCT:
      list_unlink(listelem);
      break;
      
    // draw the element to the screen
    case EXPOSE:
      gui_listelem(listelem);
      return;
      
    // if wheel mouse moved, pass this to the parent
    case MPOINTER_MOVE:
      obj_event(GUIOBJ(list), MPOINTER_MOVE, NULL);
      return;
      
    // if selected, change background to show selection
    // while also unselecting others (if not multiple)
    case SELECT:
      if (list && !list->changing) {
        struct LISTELEM *ptr;
        list->changing = TRUE;
        int cnt = list_count_selected(list);
        
        // if one or more already selected, only allow another selection
        //  if the ctrl key is pressed and we have list->multiple set.
        // if the ctrl key is not pressed, clear out all others in either case
        bit8u shift;
        gfx_key(NULL, NULL, &shift);
        
        // if already selected, don't unselected unless cnt > 1 and ctrl key is pressed
        if (obj_selected(GUIOBJ(listelem))) {
          if (cnt > 1) {
            if (KEY_CTRL(shift)) {
              // allow the button_class to toggle this one
              list->changing = FALSE;
              button_class();
            } else {
              // clear all others except this one
              ptr = list->last;
              while (ptr) {
                if (ptr != listelem)
                  obj_select(GUIOBJ(ptr), FALSE);
                ptr = ptr->prev;
              }
            }
          }
        } else {
          if ((cnt == 0) || (KEY_CTRL(shift) && list->multiple)) {
            // allow the button_class to toggle this one
            list->changing = FALSE;
            button_class();
          } else {
            // clear all
            ptr = list->last;
            while (ptr) {
              obj_select(GUIOBJ(ptr), FALSE);
              ptr = ptr->prev;
            }
            // allow the button_class to toggle this one
            list->changing = FALSE;
            button_class();
          }
        }
        list->changing = FALSE;
        emit(LIST_CHANGED, GUIOBJ(list));
        return;
      }
      break;
    
    // default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      int h = textual_height(GUITEXTUAL(listelem));
      rect->right = rect->left + textual_width(GUITEXTUAL(listelem));
      if (listelem->icon)
        h = MAX(h, gui_h(listelem->icon));
      rect->bottom = rect->top + h;
      return;
    }
  }
  
  // else call the button class
  button_class();
}

/*  listelement()
 *     listelem = pointer to list element
 *         size = size of memory to allocate
 *       parent = list this element will be linked to
 *        theid = id to give to this element
 *
 *   creates a list element and links it to the parent list
 */
struct LISTELEM *listelement(struct LISTELEM *listelem, bit32u size, struct LIST *parent, int theid) {
  MINSIZE(size, struct LISTELEM);
  
  // a list element is a base class of a button
  listelem = (struct LISTELEM *) obj_button(GUIBUTTON(listelem), size, GUIOBJ(parent), theid, 0);
  if (listelem) {
    // change the class so it calls us first
    GUIOBJ(listelem)->_class = listelem_class;
    GUITEXTUAL(listelem)->align = (ALIGN) (ALIGN_LEFT | ALIGN_VCENTER);
    textual_set_flags(GUITEXTUAL(listelem), TEXTUAL_FLAGS_NOCARET);
    text_obj_color(GUITEXTUAL(listelem), GUICOLOR_black, GUICOLOR_transparent);
    textual_set_font(GUITEXTUAL(listelem), "Courier New");
    
    // is armable
    obj_armable(GUIOBJ(listelem), TRUE);
    
    // link to the parent list
    if (parent)
      list_link(parent, listelem);
    
    // currently no icon or attachement
    listelem->icon = NULL;
    listelem->attachment = NULL;
  }

  // return created list element (or NULL if not created)
  return listelem;
}

/*  list_scroll_geometry()
 *         list = pointer to list
 *
 *   set the size of the list
 */
void list_scroll_geometry(struct LIST *list) {
  struct LISTELEM *ptr = list->last;
  struct SCROLL *scroll = (struct SCROLL *) GUISCROLL(list);
  int h = 0, w = 0, x, y;
  
  // find first element
  while (ptr && ptr->prev)
    ptr = ptr->prev;
  
  // set the default size of each and get the max width
  while (ptr) {
    obj_defaultrect(GUIOBJ(ptr), 0);
    obj_move(GUIOBJ(ptr), 0, h);
    h = obj_y(GUIOBJ(ptr)) + gui_h(ptr);
    w = MAX(w, gui_w(ptr));
    ptr = ptr->next;
  }
  
  // set the scroll geometry to the width and height found
  scroll_geometry(scroll, 0, 0, w, h);

  // make all listelem's the width of the scroll box
  ptr = list->last;
  while (ptr) {
    x = MAX(gui_w(list), w);
    obj_resize(GUIOBJ(ptr), x, gui_h(ptr));
    obj_geometry(GUIOBJ(ptr));
    ptr = ptr->prev;
  }
}

/* list_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void list_class(void) {
  struct LIST *list = (struct LIST *) eventstack.object;
  
  switch (eventstack.event) {
    // kill list?
    case DESTRUCT:
      while (list->last)
        list_unlink(list->last);
      break;
      
    // draw list to screen
    case EXPOSE:
      gui_list(list);
      return;
      
    // we allow the up and down arrows to move the selection
    case KEY: {
      // TODO: we need to scroll the scroll list if we move off of the area
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      if (list_count_selected(list) == 1) {
        struct LISTELEM *element = list_selected(list, NULL);
        switch (key_info->code) {
          case VK_UPARROW:
            if (element->prev)
              obj_event(GUIOBJ(element->prev), SELECT, eventstack.data);
            break;
          case VK_DNARROW:
            if (element->next)
              obj_event(GUIOBJ(element->next), SELECT, eventstack.data);
            break;
        }
      }
    } return;
      
    // set the geomtry of the list
    case GEOMETRY:
      list_scroll_geometry(list);
      return;
  }
  
  // if event not handled here, call the scroll class
  scroll_class();
}

/*  obj_list()
 *         list = pointer to list
 *         size = size of memory to allocate
 *       parent = parent object of list (win)
 *        theid = id to give to this element
 *        flags = flags for the list
 *
 *   creates an empty list
 */
struct LIST *obj_list(struct LIST *list, bit32u size, struct OBJECT *parent, int theid, const bit32u flags) {
  MINSIZE(size, struct LIST);
  
  // a list is a scroll class base
  list = (struct LIST *) obj_scroll(GUISCROLL(list), size, parent, theid);
  if (list) {
    GUIOBJ(list)->_class = list_class;
    
    // make sure it is empty
    list->last = NULL;
    
    // can multiple items be selected at once?
    list->multiple = (flags & LIST_MULTIPLE) ? TRUE : FALSE;
  }
  
  // return created list (or NULL if not created)
  return list;
}

/*  list_multiple()
 *         list = pointer to list
 *        multi = 0 = make list not multiple selectable
 *                1 = make list multiple selectable
 */
void list_multiple(struct LIST *list, const bool multi) {
  list->multiple = multi;
}

/*  list_link()
 *         list = pointer to list
 *     listelem = pointer to list element
 * 
 *  add a an already created listelem to the list
 * 
 */
void list_link(struct LIST *list, struct LISTELEM *listelem) {
  listelem->prev = list->last;
  listelem->list = list;
  
  if (list->last)
    list->last->next = listelem;
  list->last = listelem;
}

/*  list_append()
 *         list = pointer to list
 *         text = string of list element
 *          len = length of string (or -1)
 *    allocated = is text allocated
 *        theid = the id of this element
 *   attachment = void pointer to information to attach to element
 *         icon = pointer to a bitmap to add as icon
 * 
 *  create and add a listelem to the list
 * 
 */
void list_append(struct LIST *list, const char *text, int len, const bool allocated, int theid, void *attachment, struct BITMAP *icon) {
  struct LISTELEM *listelem = listelement(NULL, 0, list, theid);
  textual_set(GUITEXTUAL(listelem), text, len, allocated);
  listelem->icon = icon;
  listelem->attachment = attachment;
}

/*  list_select_id()
 *         list = pointer to list
 *        theid = the id of element to find
 *         doit = 0 = clear selection
 *                1 = set selection
 * 
 *  selects or unselects an element in a list
 * 
 */
void list_select_id(const struct LIST *list, const int theid, const bool doit) {
  struct LISTELEM *ptr = list->last;
  
  while (ptr) {
    if (GUIID(ptr) == theid) {
      if ((obj_selected(GUIOBJ(ptr)) && !doit) || (!obj_selected(GUIOBJ(ptr)) && doit))
        obj_select(GUIOBJ(ptr), doit);
    }
    ptr = ptr->prev;
  }
}

/*  list_empty()
 *         list = pointer to list
 * 
 *  destroys all elements from a list
 * 
 */
void list_empty(struct LIST *list) {
  while (list->last) {
    struct LISTELEM *listelem = list->last;
    list_unlink(listelem);
    node_remove(GUIOBJ(listelem));
    atom_delete(&listelem->base.atom);
  }
}

/*  list_iter()
 *         list = pointer to list
 *          ptr = starting element (or NULL to start at last one)
 * 
 *  gets next element in list
 * 
 */
struct LISTELEM *list_iter(const struct LIST *list, const struct LISTELEM *ptr) {
  return ptr ? ptr->prev : list->last;
}

/*  list_selected()
 *         list = pointer to list
 *          ptr = element (or NULL to for last one)
 * 
 *  find first selected element (from bottom up)
 *  returns NULL if none selected
 *
 */
struct LISTELEM *list_selected(const struct LIST *list, struct LISTELEM *ptr) {
  if (!ptr)
    ptr = list->last;
  
  while (ptr && !obj_selected(GUIOBJ(ptr)))
    ptr = ptr->prev;
  
  return ptr;
}

/*  list_selected_id()
 *         list = pointer to list
 *          ptr = element to get ID from
 * 
 *  returns ID of "first" selected element (from bottom up)
 *   or -1 if none selected
 * 
 */
int list_selected_id(const struct LIST *list, struct LISTELEM *ptr) {
  ptr = list_selected(list, ptr);
  if (ptr)
    return GUIID(ptr);
  
  return -1;
}
