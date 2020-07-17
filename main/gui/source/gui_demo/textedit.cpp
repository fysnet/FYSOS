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
 *  textedit.cpp
 *
 *  Last updated: 17 July 2020
 */

#include <memory.h>

#include "gui.h"
#include "grfx.h"
#include "palette.h"

/*  textedit_scroll_geometry()
 *   textedit = pointer to the textedit object
 *
 *  sets the geometry and size of the textedit object
 *   
 */
void textedit_scroll_geometry(struct TEXTEDIT *textedit) {
  struct TEXTUAL *textual = (struct TEXTUAL *) &textedit->body;
  int w, h;
  
  obj_move(GUIOBJ(textual), 0, 0);
  
  font_blocksize(textual->font, textual, &w, &h);
  scroll_geometry(GUISCROLL(textedit), 0, 0, w += 10, h += 10);  // (pad the width and height by 10 each)
  
  w = MAX(w, gui_w(GUIOBJ(textedit)));
  h = MAX(h, gui_h(GUIOBJ(textedit)));
  obj_resize(GUIOBJ(textual), w, h);
  obj_geometry(GUIOBJ(textual));
}

/* our_textual_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *   (this is the class event handler for the textedit's body)
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = depends on event
 */
void our_textual_class(void) {
  struct TEXTUAL *textual = (struct TEXTUAL *) eventstack.object;
  struct TEXTEDIT *textedit = (struct TEXTEDIT *) (GUIOBJ(textual)->parent);
  struct MENU *menu;
  
  switch (eventstack.event) {
    // ignore geometry events but don't let them pass to the textual class
    case GEOMETRY:
      return;
      
    // allow keys to add text to the text edit
    case KEY:
      // call the textual class
      textual_class();
      // call the scroll geometry incase our text now exceeds the current scroll
      textedit_scroll_geometry(textedit);
      obj_dirty(GUIOBJ(textedit), TRUE);
      return;
      
    // ignore select events but don't let them pass to the textual class
    case SELECT:
      return;
    
    // we have a menu pop up if the user presses the right mouse button
    //  this is to show as an example how to do it.
    case RPOINTER_PRESS: {
      // if there is a menu, make sure all windows of that menu are closed
      menu = win_get_menu(GUIOBJ(textedit)->win);
      if (menu)
        obj_event(GUIOBJ(menu), MENU_LOST_FOCUS, NULL);
      
      struct WIN *win = cur_active_win();
      const struct POINTER_INFO *info = pointer_info();
      menu = (struct MENU *) obj_event(GUIOBJ(win), GET_RCLICK_MENU, NULL);
      if (menu) {
        int x = info->x - obj_x(GUIOBJ(win));
        int y = info->y - obj_y(GUIOBJ(win));
        insert_menu(menu, win, x, y);
      }
    } return;
      
    // user pressed something so 86 the menu
    case LPOINTER_PRESS:
    case FOCUS_GOT:
      // if there is a menu, make sure all windows of that menu are closed
      menu = win_get_menu(GUIOBJ(textedit)->win);
      if (menu)
        obj_event(GUIOBJ(menu), MENU_LOST_FOCUS, NULL);
      break;
      
    // send the wheel movements to the scroll class
    case MPOINTER_MOVE:
      obj_event(GUIOBJ(textedit), MPOINTER_MOVE, NULL);
      break;
  }
  
  // if the our_textual_class class didn't catch the event, try the textual class
  textual_class();
}

/* textedit_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *   (this is the class event handler for the textedit)
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = depends on event
 */
void textedit_class(void) {
  struct TEXTEDIT *textedit = (struct TEXTEDIT *) eventstack.object;
  
  switch (eventstack.event) {
    // set the geometry of the textedit
    case GEOMETRY:
      textedit_scroll_geometry(textedit);
      return;
      
    // ignore select events but don't let them pass to the textual class
    case SELECT:
      return;
  }
  
  // allow the scroll class to take over
  scroll_class();
}

/*  obj_textedit()
 *     textedit = pointer to the textedit object
 *         size = size of memory to allocate
 *       parent = parent object (usually the root)
 *        theid = id of the scroll (usually ignored)
 *
 *  creates a textedit object
 *   
 */
struct TEXTEDIT *obj_textedit(struct TEXTEDIT *textedit, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct TEXTEDIT);
  
  textedit = (struct TEXTEDIT *) obj_scroll(GUISCROLL(textedit), size, parent, theid);
  if (textedit) {
    GUIOBJ(textedit)->_class = textedit_class;
    
    obj_textual(&textedit->body, sizeof(struct TEXTUAL), GUIOBJ(textedit), 0);
    text_obj_color(&textedit->body, GUICOLOR_black, GUICOLOR_white);
    textual_set(&textedit->body, (char *) calloc(4096, 1), 4096, TRUE);
    textual_align(&textedit->body, ALIGN_LEFT);
    textual_set_font(&textedit->body, "Lucida Cal");
    GUIOBJ(&textedit->body)->_class = our_textual_class;
    textual_set_flags(&textedit->body, TEXTUAL_FLAGS_BORDER);
  }
  
  // return the textedit object
  return textedit;
}
