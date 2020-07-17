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
 *  taskbar.cpp
 *
 *  Last updated: 17 July 2020
 */

 /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  *  the time part of this (time_t) is compiler specific.  You should call your own
  *   routine to get the current time
  */

#include <string.h>

#include "gui.h"
#include "grfx.h"
#include "palette.h"
#include "gui_mb.h"

/*  taskbar_update_time()
 *    taskbar = pointer to the taskbar object
 *       time = current time_t to set the time to
 *
 *  sets the time on the task bar
 *   
 */
void taskbar_update_time(struct TASKBAR *taskbar, time_t *time) {
  struct tm *timeinfo = localtime(time);
  char str[64];
  const  int hour = (timeinfo->tm_hour > 12) ? timeinfo->tm_hour - 12 : timeinfo->tm_hour;
  const char ch   = (timeinfo->tm_hour > 12) ? 'p' : 'm';
  
  sprintf(str, "%2i:%02i:%02i%cm", hour, timeinfo->tm_min, timeinfo->tm_sec, ch);
  textual_copy(taskbar->time, str);
  obj_dirty(GUIOBJ(taskbar->time), TRUE);
}

/* taskbar_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = depends on event
 */
void taskbar_class(void) {
  struct TASKBAR *taskbar = (struct TASKBAR *) eventstack.object;
  struct OBJECT *root = (struct OBJECT *) GUIOBJ(taskbar)->parent;
  const struct POINTER_INFO *info = pointer_info();
  struct RECT rect;
  int w, h, y;
  
  switch ((int) eventstack.event) {
    // we don't delete the taskbar
    case DESTRUCT:
      break;
    
    // draw the taskbar to the screen
    case EXPOSE:
      gui_taskbar(taskbar);
      return;
      
    // give the taskbar a size and place
    case GEOMETRY: {
      obj_position(GUIOBJ(taskbar), 0, 0, gui_w(root), taskbar->cur_height);
      
      // need to call the menu's geometry to size it first
      obj_geometry(GUIOBJ(taskbar->menu));
      y = (gui_h(taskbar) - gui_h(taskbar->menu)) / 2;
      obj_move(GUIOBJ(taskbar->menu), 4, y);
      
      w = textual_width(taskbar->time);
      h = textual_height(taskbar->time);
      y = (gui_h(taskbar) - h) / 2;
      obj_position(GUIOBJ(taskbar->time), gui_w(taskbar) - 10 - w, y - 3, w + 8, h + 4);
    } break;
      
    // give the taskbar a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + gui_w(root);
      rect->bottom = rect->top + TASKBAR_MIN_H;
    } return;
      
    // called everytime there is a clock() tick
    case SEC_ELAPSED:
      taskbar_update_time(taskbar, (time_t *) eventstack.data);
      
      // for some reason, POINTER_ENTER and POINTER_LEAVE doesn't work like I want it with this.
      //  so we see if the mouse pointer is close to the top of the screen.
      //  i don't know why POINTER_ENTER and POINTER_LEAVE don't work like I want.  maybe I can
      //  investigate this and find out why.
      // anyway, we "hide" the taskbar when it is not in use, but resizing it to a three pixels
      //  in height until the mouse cursor "touches" it.  it then stays full height for a specific
      //  amount of time before it "hides" again.
      if (!menu_visible(taskbar->menu)) {
        if ((info->y <= taskbar->cur_height) && (taskbar->cur_height < TASKBAR_MIN_H)) {
          taskbar->cur_height = TASKBAR_MIN_H;
          obj_resize(GUIOBJ(taskbar), GUIDEF, taskbar->cur_height);
          obj_dirty(GUIOBJ(taskbar), TRUE);
          memcpy(&rect, GUIRECT(taskbar), sizeof(struct RECT));
          rect.bottom = TASKBAR_MIN_H - 1;
          gfx_dirty((const struct RECT *) &rect);
          taskbar->timer = 100;
          obj_top(GUIOBJ(taskbar));  // bring the task bar to the top
        } else if (info->y > TASKBAR_MIN_H) {
          if (taskbar->timer == 0) {
            if (taskbar->cur_height > 3) {
              taskbar->cur_height -= 3;
              if (taskbar->cur_height < 3) taskbar->cur_height = 3;
              obj_resize(GUIOBJ(taskbar), GUIDEF, taskbar->cur_height);
              obj_dirty(GUIOBJ(taskbar), TRUE);
              memcpy(&rect, GUIRECT(taskbar), sizeof(struct RECT));
              rect.bottom = TASKBAR_MIN_H - 1;
              gfx_dirty((const struct RECT *) &rect);
            }
          } else
            taskbar->timer--;
        }
      }
      return;
      
    // we clicked on the taskbar or something else to exit the menu
    case LPOINTER_PRESS:
    case MENU_LOST_FOCUS:
      menu_remove_child_nodes(taskbar->menu);
      return;
    
    // start of Menu Item ID commands
    // these are passed to use by the menu code
    case ID_FILE_EXIT:
      w = gui_message_box(GUIOBJ(taskbar)->win, "Shutdown?", "Do you want to exit?", GUI_MB_YESNO);
      // We need a multi-tasking environment to make message_box's work correctly,
      //  so we comment out the test below
      //if (w == ID_MB_YES)
        gui_stop();
      return;
      
    case ID_HELP_HELP:
      gui_message_box(GUIOBJ(taskbar)->win, "Message Box", "This is a message box: ID_HELP_HELP", GUI_MB_OKCANCEL);
      return;
  }
  
  // else let the object class handle the event
  obj_class();
}

/*  obj_taskbar()
 *      taskbar = pointer to the taskbar object
 *         size = size of memory to allocate
 *       parent = parent object (usually the root)
 *        theid = id of the scroll (usually ignored)
 *
 *  creates a taskbar object
 *   
 */
struct TASKBAR *obj_taskbar(struct TASKBAR *taskbar, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct TASKBAR);
  time_t cur_time;
  time(&cur_time);
  struct MENU *menu;
  struct MENU_BUTTON *menu_button, *mb;

  taskbar = (struct TASKBAR *) object(GUIOBJ(taskbar), taskbar_class, size, parent, theid);
  if (taskbar) {
    // not armable or selectable
    obj_armable(GUIOBJ(taskbar), FALSE);
    obj_selectable(GUIOBJ(taskbar), FALSE);
    // initial height
    taskbar->cur_height = TASKBAR_MIN_H;
    taskbar->timer = 100;
    
    // create the menu
    menu = obj_menu(NULL, 0, GUIOBJ(taskbar), 0, NULL);
    menu->wide = FALSE;
    menu->back_color = TASKBAR_COLOR;
    menu_button = menu_append(menu, "", 0, MENU_BTN_NORMAL, 0);
    button_ownerdraw(GUIBUTTON(menu_button), ID_SYSTEM_MENU, 64, 20);
      menu_button->submenu = obj_menu(NULL, 0, GUIOBJ(taskbar), 0, menu);
      menu_append(menu_button->submenu, "&New", -1, MENU_BTN_NORMAL, ID_FILE_NEW);
      menu_append(menu_button->submenu, "&Open", -1, MENU_BTN_NORMAL, ID_FILE_OPEN);
      menu_append(menu_button->submenu, "Save &as...", -1, MENU_BTN_NORMAL, ID_FILE_SAVEAS);
      menu_append(menu_button->submenu, "&Save", -1, MENU_BTN_NORMAL, ID_FILE_SAVE);
      menu_append(menu_button->submenu, "E&xit", -1, MENU_BTN_NORMAL, ID_FILE_EXIT);
      menu_append(menu_button->submenu, "&Help", -1, MENU_BTN_NORMAL, ID_HELP_HELP);
      mb = menu_append(menu_button->submenu, "Si&de Menu", -1, MENU_BTN_NORMAL, 0);
        mb->submenu = obj_menu(NULL, 0, GUIOBJ(taskbar), 0, menu_button->submenu);
        menu_append(mb->submenu, "Side Item &0", -1, MENU_BTN_NORMAL, ID_FILE_EXIT);
        menu_append(mb->submenu, "Side Item &1", -1, MENU_BTN_NORMAL, ID_HELP_HELP);
      menu_append(menu_button->submenu, "&About", -1, MENU_BTN_NORMAL, ID_HELP_ABOUT);
      menu_append(menu_button->submenu, "Disa&bled Button", -1, MENU_BTN_DISABLED, 0);
    taskbar->menu = menu;
    
    // create the time display
    taskbar->time = obj_textual(NULL, 0, GUIOBJ(taskbar), 0);
    textual_set_font(taskbar->time, "Simple");
    textual_set(taskbar->time, "                         ", -1, FALSE);
    textual_set_flags(taskbar->time, TEXTUAL_FLAGS_READONLY | TEXTUAL_FLAGS_BORDER);
    text_obj_color(taskbar->time, GUICOLOR_black, TASKBAR_COLOR);
    taskbar->time->decorator = get_static_bitmap(ID_TASKBAR_DOWN, 0, 0);
    obj_defaultrect(GUIOBJ(taskbar->time), NULL);
    textual_align(taskbar->time, ALIGN_CENTER);
    taskbar_update_time(taskbar, &cur_time);
  }
  
  // return the taskbar
  return taskbar;
}
