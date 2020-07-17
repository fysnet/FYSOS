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
 *  menu.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <ctype.h>
#include <string.h>

#include "gui.h"
#include "grfx.h"

/*
 * Private funcitons start here
 */


/* TODO: This is a cheap and not so good way to close out a
 *  menu if the user clicks on something else.  This needs
 *  to be changed to something else...
 */
bool clicked = FALSE;

/*  menu_unlink()
 *     menu_button = pointer to menu button object
 *
 *   unlinks a menu button from a menu
 */
void menu_unlink(struct MENU_BUTTON *menu_button) {
  // if no parent, return
  if (!menu_button->parent)
    return;
  
  // link the neighbors
  if (menu_button->prev)
    menu_button->prev->next = menu_button->next;
  if (menu_button->next)
    menu_button->next->prev = menu_button->prev;
  
  // if was the last, make the prev the last
  if (menu_button == menu_button->parent->last)
    menu_button->parent->last = menu_button->prev;
  
  // free icon(s)
  if (menu_button->icon)
    atom_delete((struct ATOM *) menu_button->icon);
  if (menu_button->right)
    atom_delete((struct ATOM *) menu_button->right);
  
  // unlink it
  menu_button->submenu = NULL;
  menu_button->parent = NULL;
  menu_button->next = NULL;
  menu_button->prev = NULL;
  menu_button->icon = NULL;
  menu_button->right = NULL;
}

/*  menu_link()
 *            menu = pointer to menu object
 *     menu_button = pointer to menu button object
 *
 *   links a menu button to a menu
 */
void menu_link(struct MENU *menu, struct MENU_BUTTON *menu_button) {
  // link to last
  menu_button->prev = menu->last;
  menu_button->parent = menu;
  
  // store the first for faster searches
  if (!menu->first) {
    menu->first = menu_button;
    menu->current = menu_button;
  }
  
  // if more than one, link them together
  if (menu->last)
    menu->last->next = menu_button;
  menu->last = menu_button;
}

/*  menu_iter()
 *            menu = pointer to menu object
 *             ptr = pointer to menu button object
 *
 *   find menu item from prt or last
 */
struct MENU_BUTTON *menu_iter(struct MENU *menu, struct MENU_BUTTON *ptr) {
  return ptr ? ptr->prev : menu->last;
}

/*  menu_empty()
 *            menu = pointer to menu object
 *
 *   destroy all items in a menu
 */
void menu_empty(struct MENU *menu) {
  struct MENU_BUTTON *menu_button;
  
  while (menu->last) {
    menu_button = menu->last;
    if (menu_button->submenu)  // recurse if one button is a submenu
      menu_empty(menu_button->submenu);
    menu_unlink(menu_button);
    node_remove(GUIOBJ(menu_button));
    atom_delete(&menu_button->base.atom);
  }
}

/*  menu_emit_id()
 *     menu_button = pointer to menu button object
 *              id = event
 *
 *   emit the id to the parent menu
 */
void menu_emit_id(struct MENU_BUTTON *menu_button, const int id) {
  struct MENU *menu = menu_button->parent;
  
  while (menu->parent)
    menu = menu->parent;
  
  obj_event(GUIOBJ(menu)->parent, (EVENT) id, NULL);
}

/*  remove_child_nodes()
 *            menu = pointer to menu object
 *             top = 0 = don't start at top of menu system
 *                   1 = start at top of menu system
 *
 *   remove all child nodes from menu
 */
void remove_child_nodes(struct MENU *menu, const bool top) {
  // if we passed a null menu, return
  if (!menu)
    return;
  
  // if top, start at top parent of menu(s)
  if (top) {
    while (menu->parent != NULL)
      menu = menu->parent;
  }
  
  struct MENU_BUTTON *menu_button = menu_iter(menu, NULL);
  while (menu_button) {
    if (menu_button->submenu) {
      remove_child_nodes(menu_button->submenu, FALSE);  // recurse to remove all child nodes
      node_remove(GUIOBJ(menu_button->submenu));
      win_dirty(GUIOBJ(menu_button->parent)->win);
      gfx_dirty(GUIRECT(GUIOBJ(menu_button->submenu)));
    }
    if (top)
      menu_button->flags &= ~MENU_BTN_ACTIVE;  // make sure all buttons are not active for next time
    menu_button = menu_iter(menu, menu_button);
  }
}

/*  hmenu_button_geometry()
 *            menu = pointer to menu object
 *           width = pointer to store width value
 *
 *   gets the width and height of a horizontal menu bar
 *   returns height
 */
int hmenu_button_geometry(struct MENU *menu, int *width) {
  struct MENU_BUTTON *ptr = menu->last;
  
  // find the left most button
  while (ptr && ptr->prev)
    ptr = ptr->prev;
  
  // place and get width and height
  int w = 2, h = 0;
  while (ptr) {
    if (!(ptr->flags & MENU_BTN_HIDDEN)) {
      obj_defaultrect(GUIOBJ(ptr), 0);
      obj_move(GUIOBJ(ptr), w, 2);
      obj_geometry(GUIOBJ(ptr));
      w += gui_w(ptr) + 2;
      h = MAX(h, gui_h(ptr));
    }
    ptr = ptr->next;
  }
  
  // store the width
  if (width) *width = w;
  
  // return the height
  return h;
}

/*  vmenu_button_geometry()
 *            menu = pointer to menu object
 *           width = pointer to store width value
 *
 *   gets the width and height of a vertical menu
 *   returns height
 */
int vmenu_button_geometry(struct MENU *menu, int *width) {
  struct MENU_BUTTON *ptr = menu->last;
  bool has_off = FALSE;
  int w = 0, h = 2, iw = 0, x, y;
  
  // find the top most button, and calculate the width of the icon.
  while (ptr) {
    // if one menu button has an icon, all should be
    //  moved over to line up the text.
    if (ptr->icon) {
      has_off = TRUE;
      iw = MAX(iw, gui_w(ptr->icon));
    }
    
    // if it has a sub menu and we haven't given 
    //  it the arrow icon yet, do so now.
    if (ptr->submenu && !ptr->right)
      ptr->right = get_static_bitmap(ID_ICON_SYS_RIGHT, 16, 16);
    
    if (ptr->prev == NULL)
      break;
    
    ptr = ptr->prev;
  }
  
  // place and get width and height
  while (ptr) {
    if (!(ptr->flags & MENU_BTN_HIDDEN)) {
      obj_defaultrect(GUIOBJ(ptr), 0);
      obj_move(GUIOBJ(ptr), 4, h);
      obj_geometry(GUIOBJ(ptr));
      y = gui_h(ptr);
      x = gui_w(ptr);
      if (has_off)
        x += iw;
      if (ptr->flags & MENU_BTN_SEPARATOR)
        y = MENU_SEPRTR_H;
      else {
        if (ptr->icon)
          y = MAX(y, gui_h(ptr->icon));
        if (ptr->right) {
          x += gui_w(ptr->right);
          y = MAX(y, gui_h(ptr->right));
        }
      }
      w = MAX(w, x);    // width of widest button
      h += y;           // h = total height of all buttons
      // resize the height now, we will do the width below
      obj_resize(GUIOBJ(ptr), GUIDEF, y);
    }
    ptr = ptr->next;
  }
  
  // make all buttons the width of the widest button
  ptr = menu->last;
  while (ptr) {
    if (has_off) ptr->flags |= MENU_BTN_HAS_OFF;
    obj_resize(GUIOBJ(ptr), w, GUIDEF);
    obj_geometry(GUIOBJ(ptr));
    ptr = ptr->prev;
  }
  
  // store the width
  if (width)  *width = w;
  
  // return the height
  return h;
}

/*  menu_get_next()
 *            menu = pointer to menu object
 *     menu_button = pointer to menu button object
 *
 *   returns next enabled button or NULL if no more
 *   
 */
struct MENU_BUTTON *menu_get_next(struct MENU *menu, struct MENU_BUTTON *menu_button) {
  struct MENU_BUTTON *mb;
  
  mb = menu_button->next;
  while (mb) {
    if (!(mb->flags & MENU_BTN_HIDDEN) && (mb->flags & MENU_BTN_ENABLED))
      return mb;
    mb = mb->next;
  }
  
  return NULL;
}

/*  menu_get_prev()
 *            menu = pointer to menu object
 *     menu_button = pointer to menu button object
 *
 *   returns prev enabled button or NULL if no more
 *   
 */
struct MENU_BUTTON *menu_get_prev(struct MENU *menu, struct MENU_BUTTON *menu_button) {
  struct MENU_BUTTON *mb;
  
  mb = menu_button->prev;
  while (mb) {
    if (!(mb->flags & MENU_BTN_HIDDEN) && (mb->flags & MENU_BTN_ENABLED))
      return mb;
    mb = mb->prev;
  }
  
  return NULL;
}

/* menu_button_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to menu button
 *   eventstack.data = depends on event
 */
void menu_button_class(void) {
  struct MENU_BUTTON *menu_button = (struct MENU_BUTTON *) eventstack.object;
  int x, y;
  
  switch (eventstack.event) {
    // kill menu button
    case DESTRUCT:
      menu_unlink(menu_button);
      break;
      
    // draw menu button to screen
    case EXPOSE:
      if (!(menu_button->flags & MENU_BTN_HIDDEN))
        gui_menu_button(menu_button);
      return;
    
    // user clicked on menu button
    case LPOINTER_PRESS:
      // remove any submenus within this menu that may/may not be open
      remove_child_nodes(menu_button->parent, FALSE);
      
      if (menu_button->submenu && (menu_button->flags & MENU_BTN_ENABLED)) {
        if (!menu_button->parent->parent) {
          // go to the bottom of the main menu
          x = obj_x(GUIOBJ(menu_button)) + 5;
          y = obj_y(GUIOBJ(menu_button)) + gui_h(menu_button) + 2;
        } else {
          // go to the right of the current menu
          x = obj_x(GUIOBJ(menu_button->parent)) + gui_w(menu_button->parent) - 2;
          y = obj_y(GUIOBJ(menu_button->parent)) + obj_y(GUIOBJ(menu_button)) - 2;
        }
        insert_menu(menu_button->submenu, GUIOBJ(menu_button->parent)->win, x, y);
        menu_button->submenu->current->flags &= ~MENU_BTN_ACTIVE;
        menu_button->submenu->current = menu_button->submenu->first;
        menu_button->submenu->current->flags |= MENU_BTN_ACTIVE;
      } else {
        if (menu_button->flags & MENU_BTN_ENABLED) {
          remove_child_nodes(menu_button->parent, TRUE);  // remove all menu nodes
          menu_emit_id(menu_button, GUIOBJ(menu_button)->id);
        }
      }
      clicked = TRUE;
      return;
      
    // outline menu button when cursor hovers over it
    case POINTER_ENTER:
      if (!clicked) {
        menu_button->flags |= MENU_BTN_ACTIVE;
        obj_dirty(GUIOBJ(menu_button), FALSE);
      } else
      if (menu_button->flags & MENU_BTN_ENABLED) {
        // remove any submenus within this menu that may/may not be open
        remove_child_nodes(menu_button->parent, FALSE);
        
        if (!(menu_button->flags & MENU_BTN_HIDDEN)) {
          if (menu_button->parent->current) {
            menu_button->parent->current->flags &= ~MENU_BTN_ACTIVE;
            obj_dirty(GUIOBJ(menu_button->parent->current), FALSE);
          }
          menu_button->flags |= MENU_BTN_ACTIVE;
          menu_button->parent->current = menu_button;
          obj_dirty(GUIOBJ(menu_button), FALSE);
          if (menu_button->submenu)
            obj_event(GUIOBJ(menu_button), LPOINTER_PRESS, NULL);
        }
      }
      return;
      
    // remove outline of menu button when cursor leaves
    case POINTER_LEAVE:
      if (!clicked) {
        if (!(menu_button->flags & MENU_BTN_HIDDEN)) {
          menu_button->flags &= ~MENU_BTN_ACTIVE;
          gui_menu_button(menu_button);
          obj_dirty(GUIOBJ(menu_button), FALSE);
        }
      }
      return;
  }
  
  // if event not handled here, call the button class
  button_class();
}

/*
 * Public funcitons start here
 */

/* menu_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to menu button
 *   eventstack.data = depends on event
 */
void menu_class(void) {
  struct MENU *menu = (struct MENU *) eventstack.object;
  int w, h;
  
  switch (eventstack.event) {
    // kill menu
    case DESTRUCT:
      while (menu->last)
        menu_unlink(menu->last);
      break;
      
    // draw menu to screen
    case EXPOSE:
      gui_menu(menu);
      return;
      
    // allow us to gain the focus
    case FOCUS_WANT:
      answer(menu);
      return;
    
    // set the geometry of the menu (this is only the blank rectangle the buttons sit on)
    case GEOMETRY:
      if (menu->parent == NULL) {
        h = hmenu_button_geometry(menu, &w);
        if (menu->wide) 
          w = gui_w(GUIOBJ(menu)->parent);
        obj_resize(GUIOBJ(menu), w, h + 2);
      }
      return;
      
    // give a default size to the menu
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      
      if (menu->parent == NULL) {
        rect->right = gui_w(GUIOBJ(menu)->parent);
        rect->bottom = 16;
      } else {
        rect->right = 100;
        rect->bottom = 100;
      }
    } return;
      
    // remove menus from screen when something is pressed
    case LPOINTER_PRESS:
    case MENU_LOST_FOCUS:
      if (clicked) {
        remove_child_nodes(menu, TRUE);
        clicked = FALSE;
      }
      return;
      
    // allow mnemonics to select menu items
    //  as well as the arrow keys to move through menus
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      struct MENU_BUTTON *menu_button;
      
      if (key_info->code == VK_ESCAPE) {          // esc
        menu_remove_child_nodes(menu);
      } else if (key_info->code == VK_UPARROW) {  // up
        if (menu->parent) {
          menu_button = menu_get_prev(menu, menu->current);
          if (menu_button) {
            menu->current->flags &= ~MENU_BTN_ACTIVE;
            menu->current = menu_button;
            menu->current->flags |= MENU_BTN_ACTIVE;
            obj_dirty(GUIOBJ(menu), TRUE);
          }
        }
      } else if (key_info->code == VK_DNARROW) {  // down
        if (menu->parent == NULL) {  // main menu (horizontal)
          obj_event(GUIOBJ(menu->current), LPOINTER_PRESS, NULL);
        } else {
          menu_button = menu_get_next(menu, menu->current);
          if (menu_button) {
            menu->current->flags &= ~MENU_BTN_ACTIVE;
            menu->current = menu_button;
            menu->current->flags |= MENU_BTN_ACTIVE;
            obj_dirty(GUIOBJ(menu), TRUE);
          }
        }
      } else if (key_info->code == 0x4B00) {  // left
        if (menu->parent == NULL) {  // main menu (horizontal)
          menu_button = menu_get_prev(menu, menu->current);
          if (menu_button) {
            menu->current->flags &= ~MENU_BTN_ACTIVE;
            menu->current = menu_button;
            menu->current->flags |= MENU_BTN_ACTIVE;
            obj_dirty(GUIOBJ(menu), TRUE);
          }
        } else {
          menu_button = menu_get_prev(menu->parent, menu->parent->current);
          if (menu_button) {
            // remove any submenus within this menu that may/may not be open
            remove_child_nodes(menu_button->parent, FALSE);
            menu->parent->current->flags &= ~MENU_BTN_ACTIVE;
            menu->parent->current = menu_button;
            menu->parent->current->flags |= MENU_BTN_ACTIVE;
            obj_event(GUIOBJ(menu_button), LPOINTER_PRESS, NULL);
          }          
        }
      } else if (key_info->code == 0x4D00) {  // right
        if (menu->parent == NULL) {
          menu_button = menu_get_next(menu, menu->current);
          if (menu_button) {
            menu->current->flags &= ~MENU_BTN_ACTIVE;
            menu->current = menu_button;
            menu->current->flags |= MENU_BTN_ACTIVE;
            obj_dirty(GUIOBJ(menu->parent), TRUE);
          }
        } else {
          menu_button = menu_get_next(menu->parent, menu->parent->current);
          if (menu_button) {
            // remove any submenus within this menu that may/may not be open
            remove_child_nodes(menu_button->parent, FALSE);
            menu->parent->current->flags &= ~MENU_BTN_ACTIVE;
            menu->parent->current = menu_button;
            menu->parent->current->flags |= MENU_BTN_ACTIVE;
            obj_event(GUIOBJ(menu_button), LPOINTER_PRESS, NULL);
          }          
        }
      } else if (key_info->code == 0x1C0D) {  // enter
        obj_event(GUIOBJ(menu->current), LPOINTER_PRESS, NULL);
      } else {
        // This is where you would put the check for mnemonics in the current menu
        // Scroll through the MENU BUTTON objects within this menu to see if
        //  any of them have that mnemonic.
        menu_button = menu_iter(menu, NULL);
        while (menu_button) {
          if (obj_has_mnemonic(GUIOBJ(menu_button))) {
            int len = 0;
            const char *text = textual_text(GUITEXTUAL(menu_button), &len);
            if (text && (len > 0)) {
              // find '&' char
              char *p = strchr(text, '&');
              if (p && (tolower(p[1]) == tolower(key_info->ascii))) {
                menu->current->flags &= ~MENU_BTN_ACTIVE;
                menu->current = menu_button;
                menu->current->flags |= MENU_BTN_ACTIVE;
                obj_event(GUIOBJ(menu_button), LPOINTER_PRESS, NULL);
                return;
              }
            }
          }
          menu_button = menu_iter(menu, menu_button);
        }
      }
    } return;
  }
  
  // if event not handled here, try the object class
  obj_class();
}

/*  insert_menu()
 *            menu = pointer to menu object
 *             win = pointer to parent window
 *               x = X coordinate to position menu
 *               y = Y coordinate to position menu
 *
 *   inserts a menu into the Win object
 *   
 */
void insert_menu(struct MENU *menu, struct WIN *win, const int x, const int y) {
  int w, h;
  
  node_insert(GUIOBJ(menu), GUIOBJ(win));
  
  h = vmenu_button_geometry(menu, &w);
  obj_position(GUIOBJ(menu), x, y, w + 7, h + 2);
  obj_geometry(GUIOBJ(menu));
  obj_top(GUIOBJ(menu));
  obj_dirty(GUIOBJ(menu), TRUE);
  obj_focus(GUIOBJ(menu));
}

/*  obj_menu_button()
 *     menu_button = pointer to menu button object
 *            size = size of memory to allocate
 *          parent = parent menu to link to
 *           theid = id to give to menu button
 *
 *   creates and links a menu_button to a menu
 *   
 */
struct MENU_BUTTON *obj_menu_button(struct MENU_BUTTON *menu_button, bit32u size, struct MENU *parent, int theid) {
  MINSIZE(size, struct MENU_BUTTON);
  
  menu_button = (struct MENU_BUTTON *) obj_button(GUIBUTTON(menu_button), size, GUIOBJ(parent), theid, 0);
  if (menu_button) {
    GUIOBJ(menu_button)->_class = menu_button_class;
    if (parent->parent)
      textual_align(GUITEXTUAL(menu_button), (ALIGN) (ALIGN_LEFT | ALIGN_VCENTER));
    
    obj_armable(GUIOBJ(menu_button), FALSE);
    if (theid > 0)
      menu_button->icon = get_static_bitmap(theid, 16, 16);
    if (parent)
      menu_link(parent, menu_button);
  }
  
  return menu_button;
}

/*  menu_append()
 *            menu = pointer to menu object
 *            text = string text for button
 *             len = len of text (or -1)
 *           flags = flags of button
 *           theid = id to give to menu button
 *
 *   creates and appends a menu_button to a menu
 *   
 */
struct MENU_BUTTON *menu_append(struct MENU *menu, const char *text, const int len, const int flags, const int theid) {
  struct MENU_BUTTON *menu_button = obj_menu_button(NULL, 0, menu, theid);
  textual_set(GUITEXTUAL(menu_button), text, len, FALSE);
  menu_button->parent = menu;
  menu_button->flags = flags & ~MENU_BTN_ACTIVE;
  menu_button->submenu = NULL;
  if (flags & MENU_BTN_SEPARATOR)
    menu_button->flags |= MENU_BTN_DISABLED;
  return menu_button;
}

/*  obj_menu()
 *            menu = pointer to menu object
 *            size = size of memory to allocate
 *          parent = parent object
 *           theid = id to give to menu button
 *     parent_menu = parent menu object (if any)
 *
 *   creates and optionally appends a menu to a menu and/or window object
 *   
 */
struct MENU *obj_menu(struct MENU *menu, bit32u size, struct OBJECT *parent, int theid, struct MENU *parent_menu) {
  MINSIZE(size, struct MENU);
  
  menu = (struct MENU *) object(GUIOBJ(menu), menu_class, size, parent, theid);
  if (menu) {
    menu->first = NULL;
    menu->last = NULL;
    menu->current = NULL;
    menu->parent = parent_menu;
    menu->wide = TRUE;  // assume as wide as the parent
    menu->back_color = 0; // do current win color
    
    // unless we are the main menu, remove from the 'queue'
    // we will insert it when needed, then remove it when
    // not needed.
    if (parent_menu)
      node_remove(GUIOBJ(menu));
  }
  
  return menu;
}

/*  menu_visible()
 *            menu = pointer to menu object
 *
 *   Returns TRUE if a menu has a sub menu attached (opened / visible)
 *   
 */
bool menu_visible(struct MENU *menu) {
  struct MENU_BUTTON *menu_button = menu_iter(menu, NULL);
  while (menu_button) {
    if (menu_button->submenu && GUIOBJ(menu_button->submenu)->parent)
      return TRUE;
    menu_button = menu_iter(menu, menu_button);
  }
  
  return FALSE;
}

/*  menu_remove_child_nodes()
 *            menu = pointer to menu object
 *
 *   kills all menu items
 *   this function is called from userland, remove_child_nodes is called in this file only
 *   
 */
void menu_remove_child_nodes(struct MENU *menu) {
  remove_child_nodes(menu, TRUE);
}
