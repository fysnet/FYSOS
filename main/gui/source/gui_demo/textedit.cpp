/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * textedit.cpp
 *  
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
