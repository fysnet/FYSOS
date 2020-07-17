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
 *  button_bar.cpp
 *  
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>

#include "gui.h"
#include "grfx.h"
#include "palette.h"

/* button_bar_unlink()
 *   button_bar_button = pointer to the button on the button bar
 *
 *  This unlinks a button from a button bar, then deletes its contents
 *
 */
void button_bar_unlink(struct BUTTON_BAR_BUTTON *button_bar_button) {
  // if there is an icon with this button (which there should be),
  //  delete it
  if (button_bar_button->icon)
    atom_delete((struct ATOM *) button_bar_button->icon);
  
  // if button doesn't have a parent, just return
  if (!button_bar_button->parent)
    return;
  
  // link the prev to the next
  if (button_bar_button->prev)
    button_bar_button->prev->next = button_bar_button->next;
  if (button_bar_button->next)
    button_bar_button->next->prev = button_bar_button->prev;
  
  // if this was the last one, we need to set the "next" one as the last one
  if (button_bar_button == button_bar_button->parent->last)
    button_bar_button->parent->last = button_bar_button->prev;
  
  // unlink this one
  button_bar_button->parent = NULL;
  button_bar_button->next = NULL;
  button_bar_button->prev = NULL;
  button_bar_button->icon = NULL;
}

/* button_bar_link()
 *          button_bar = pointer to the parent button bar
 *   button_bar_button = pointer to the button to link to the button bar
 *
 *  This links a button to a button bar
 *
 */
void button_bar_link(struct BUTTON_BAR *button_bar, struct BUTTON_BAR_BUTTON *button_bar_button) {
  button_bar_button->prev = button_bar->last;
  button_bar_button->parent = button_bar;
  
  // we are always the last one
  if (button_bar->last)
    button_bar->last->next = button_bar_button;
  button_bar->last = button_bar_button;
}

/* button_bar_iter()
 *   button_bar = pointer to the parent button bar
 *          ptr = NULL returns last button
 *              = !NULL returns prev button
 *
 */
struct BUTTON_BAR_BUTTON *button_bar_iter(struct BUTTON_BAR *button_bar, struct BUTTON_BAR_BUTTON *ptr) {
  return ptr ? ptr->prev : button_bar->last;
}

/* button_bar_empty()
 *   button_bar = pointer to the parent button bar
 *
 *  this removes all buttons from a button bar
 */
void button_bar_empty(struct BUTTON_BAR *button_bar) {
  struct BUTTON_BAR_BUTTON *button_bar_button;
  
  while (button_bar->last) {
    button_bar_button = button_bar->last;
    button_bar_unlink(button_bar_button);
    node_remove(GUIOBJ(button_bar_button));
    atom_delete(&button_bar_button->base.atom);
  }
}

/* button_bar_button_geometry()
 *   button_bar = pointer to the parent button bar
 *
 *
 *  this places all buttons on a button bar
 *  this will return the max height of all buttons
 *    so we can give a height to the button bar
 */
int button_bar_button_geometry(struct BUTTON_BAR *button_bar) {
  struct BUTTON_BAR_BUTTON *ptr = button_bar->last;
  int h = 0;
  
  // find the left most button
  while (ptr && ptr->prev)
    ptr = ptr->prev;
  
  // start 2 pixels to the right
  int x = 2;
  while (ptr) {
    obj_move(GUIOBJ(ptr), x, 1);
    obj_geometry(GUIOBJ(ptr));
    x += (gui_w(ptr) + 2);
    h = MAX(h, gui_h(ptr));
    ptr = ptr->next;
  }
  
  // we need to return the max height of all buttons
  return h;
}

/* obj_button_bar_button()
 *   button_bar_button = pointer to the button bar button
 *                NULL = allocate the memory here
 *              ! NULL = memory is already allocated/static
 *                size = size of memory to allocate
 *              parent = parent button bar
 *               theid = id of button to retrieve/send to event
 *
 *
 *  this creates a button on the button bar
 */
struct BUTTON_BAR_BUTTON *obj_button_bar_button(struct BUTTON_BAR_BUTTON *button_bar_button, bit32u size, struct BUTTON_BAR *parent, int theid) {
  MINSIZE(size, struct BUTTON_BAR_BUTTON);
  struct BITMAP *icon;
  
  // all buttons must have an icon
  // if it is the divider, it can be what ever size
  // if it is not the divider, it must be 16 x 16
  icon = (theid == ID_BUTTON_BAR_DIV) ? get_static_bitmap(theid, 0, 0) : get_static_bitmap(theid, 16, 16);
  if (icon) {
    // base class is "button"
    button_bar_button = (struct BUTTON_BAR_BUTTON *) obj_button(GUIBUTTON(button_bar_button), size, GUIOBJ(parent), theid, 0);
    if (button_bar_button) {
      // increase the size of the button to 18x18 so we have room for the border
      obj_resize(GUIOBJ(button_bar_button), gui_w(icon) + 2, gui_h(icon) + 2);
      // change class to our button bar button class (which calls the button class anyway)
      GUIOBJ(button_bar_button)->_class = button_bar_button_class;
      // user may click on button
      obj_armable(GUIOBJ(button_bar_button), FALSE);
      button_bar_button->icon = icon;
      // we should always have a parent, but just in case
      if (parent)
        button_bar_link(parent, button_bar_button);
    }
    // return pointer to button
    return button_bar_button;
  }
  
  // there was some kind of error
  return NULL;
}

/* button_bar_append()
 *   button_bar = pointer to the parent button bar
 *        flags = flags of new button
 *        theid = id of button to retrieve/send to event
 *
 *  this adds a new button to a button bar
 */
struct BUTTON_BAR_BUTTON *button_bar_append(struct BUTTON_BAR *button_bar, const int flags, const int theid) {
  // create the button
  struct BUTTON_BAR_BUTTON *button_bar_button = obj_button_bar_button(NULL, 0, button_bar, theid);
  if (button_bar_button) {
    // asign the parent
    button_bar_button->parent = button_bar;
    button_bar_button->flags = flags & ~BUTTON_BTNBAR_ACTIVE;
    // don't allow user to click on divider, and don't frame button when cursor is over it
    if (theid == ID_BUTTON_BAR_DIV)
      button_bar_button->flags &= ~BUTTON_BTNBAR_ENABLED;
    return button_bar_button;
  }
  return NULL;
}

/* button_bar_button_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to button bar button
 *   eventstack.data = rect of win object
 */
void button_bar_button_class(void) {
  struct BUTTON_BAR_BUTTON *button_bar_button = (struct BUTTON_BAR_BUTTON *) eventstack.object;
  int x, y;
  
  switch (eventstack.event) {
    // remove button from button bar
    case DESTRUCT:
      button_bar_unlink(button_bar_button);
      break;
      
    // draw button to screen
    case EXPOSE:
      gui_button_bar_button(button_bar_button);
      return;
    
    // user left clicked on button
    case LPOINTER_PRESS:
      if (button_bar_button->flags & BUTTON_BTNBAR_ENABLED)
        // send the ID of this button_bar button to the active window
        obj_event(GUIOBJ(cur_active_win()), (EVENT) GUIOBJ(button_bar_button)->id, NULL);
      
      // return instead of break so the button does not remain pressed.
      return;
      
    // mouse cursor entered button space, draw frame around button
    case POINTER_ENTER:
      button_bar_button->flags |= BUTTON_BTNBAR_ACTIVE;
      gui_button_bar_button(button_bar_button);
      obj_dirty(GUIOBJ(button_bar_button), FALSE);
      return;
      
    // mouse cursor left button space, un-draw frame around button
    case POINTER_LEAVE:
      button_bar_button->flags &= ~BUTTON_BTNBAR_ACTIVE;
      gui_button_bar_button(button_bar_button);
      obj_dirty(GUIOBJ(button_bar_button), FALSE);
      return;
  }
  
  // if event is other than above, try the button class
  button_class();
}

/* button_bar_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to button bar
 *   eventstack.data = rect of win object
 */
void button_bar_class(void) {
  struct BUTTON_BAR *button_bar = (struct BUTTON_BAR *) eventstack.object;
  struct MENU *menu = win_get_menu(GUIOBJ(button_bar)->win);
  int h, y = 0;
  
  switch (eventstack.event) {
    // remove button bar buttons
    case DESTRUCT:
      while (button_bar->last)
        button_bar_unlink(button_bar->last);
      break;
      
    // draw button bar (no buttons, that is done in button_bar_button_class() above)
    case EXPOSE:
      gui_button_bar(button_bar);
      return;
    
    case FOCUS_WANT:
      answer(button_bar);
      return;
    
    // size button bar from child buttons
    // and place just under menu (if exists) or under title bar
    case GEOMETRY:
      if (menu) y = gui_h(menu);
      h = button_bar_button_geometry(button_bar);
      obj_position(GUIOBJ(button_bar), 0, y, gui_w(GUIOBJ(button_bar)->parent), h + 1);
      return;
    
    // give button bar a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = gui_w(GUIOBJ(button_bar)->parent);
      rect->bottom = 16;
    } return;
  }
  
  // if event is other than above, try the default object class
  obj_class();
}

/* obj_button_bar()
 *   button_bar = pointer to the button bar button
 *         NULL = allocate the memory here
 *       ! NULL = memory is already allocated/static
 *         size = size of memory to allocate
 *   parent_win = parent window
 *          ids = pointer to ids to append to the button bar (can be NULL) (terminate list with an id of -1)
 *
 *
 *  this creates a button bar, optionally adding buttons to it
 */
struct BUTTON_BAR *obj_button_bar(struct BUTTON_BAR *button_bar, bit32u size, struct OBJECT *parent_win, int *ids, int theid) {
  MINSIZE(size, struct BUTTON_BAR);
  
  // create button bar
  button_bar = (struct BUTTON_BAR *) object(GUIOBJ(button_bar), button_bar_class, size, parent_win, theid);
  if (button_bar) {
    button_bar->last = NULL;
    
    // if the list exists, add buttons
    if (ids)
      // terminating as first id of -1
      while (*ids > -1)
        button_bar_append(button_bar, MENU_BTN_NORMAL, *ids++);
  }
  
  // return pointer to button bar
  return button_bar;
}
