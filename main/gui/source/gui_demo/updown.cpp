/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * updown.cpp
 *  
 */

#include <string.h>
#include <memory.h>

#include "gui.h"
#include "palette.h"
#include "grfx.h"

/*  updown_setcur()
 *   updown = pointer to the updown object
 *      cur = value to set the object to
 *
 *  sets the current value for the object
 *   
 */
void updown_setcur(struct UPDOWN *updown, const int cur) {
  char str[32];
  
  updown->cur = cur;
  sprintf(str, "%i", updown->cur);
  textual_copy(&updown->text, str);
  obj_dirty(GUIOBJ(&updown->text), FALSE);
}

/*  updown_getcur()
 *   updown = pointer to the updown object
 *
 *  returns the current value for the object
 *   
 */
int updown_getcur(struct UPDOWN *updown) {
  return updown->cur;
}

/*  updown_getcur()
 *   updown = pointer to the updown object
 *    flags = flags to set the object to
 *
 *  set the flags for the object
 *   
 */
void updown_setflags(struct UPDOWN *updown, const bit32u flags) {
  updown->flags = flags;
}

/*  updown_getflags()
 *   updown = pointer to the updown object
 *
 *  get the flags of the object
 *   
 */
bit32u updown_getflags(struct UPDOWN *updown) {
  return updown->flags;
}

/* textual_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = depends on event (DEFAULTRECT: rect of textual)
 */
void updown_class(void) {
  struct UPDOWN *updown = (struct UPDOWN *) eventstack.object;
  int w;
  
  switch (eventstack.event) {
    // kill the object
    case DESTRUCT:
      // obj_class will do all of this for us, so break instead of return
      break;
      
    // draw the object to the screen
    case EXPOSE:
      gui_updown(updown);
      return;
      
    // set the size and position of the text and buttons within the object
    case GEOMETRY:
      // make sure at least the min width
      if (gui_w(updown) < UPDOWN_MIN_W)
        obj_resize(GUIOBJ(updown), UPDOWN_MIN_W, GUIDEF);
      
      // make sure at least the min height
      if (gui_h(updown) < UPDOWN_MIN_H)
        obj_resize(GUIOBJ(updown), GUIDEF, UPDOWN_MIN_H);
      
      // can we resize the updown object to the size of the text?
      if (updown->flags & UPDOWN_FIT_TO_TEXT) {
        // if we don't round up to the next multiple of 8 pixels,
        //  the font might be a non-fixed width font and will make
        //  the updown object resize at every change.  this way
        //  it will not change unless a text width of more than 7
        //  pixels is needed.
        w = ((textual_width(&updown->text) + 8) & ~7); // *next* multiple of 8
        w = MAX(w + 4, UPDOWN_TEXT_W);
        obj_resize(GUIOBJ(&updown->text), w, GUIDEF);
        w += UPDOWN_LEFT + 2 + UPDOWN_BUTTON_W + UPDOWN_LEFT;
        // if we are making the updown object smaller, we need to mark the area as dirty
        //  before we resize it.  Either that or we have to mark the whole window
        //  and all its children as dirty which takes longer for the GUI system to update.
        // if the item is not larger than before, we have to update it too. so do before
        //  and after just to be sure
        obj_dirty(GUIOBJ(updown), TRUE);
        obj_resize(GUIOBJ(updown), w, GUIDEF);
        obj_dirty(GUIOBJ(updown), TRUE);
      } else {
        w = gui_w(updown) - 2 - UPDOWN_BUTTON_W - 2 - 4;
        obj_resize(GUIOBJ(&updown->text), w, GUIDEF);
      }
      
      // do the layout
      if (updown->flags & UPDOWN_RIGHT_TEXT) {
        obj_layout(GUIOBJ(&updown->up), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(updown), UPDOWN_LEFT, UPDOWN_TOP);
        obj_layout(GUIOBJ(&updown->down), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&updown->up), 0, 3);
        obj_layout(GUIOBJ(&updown->text), (LAYOUT) (LAYOUT_X2 | LAYOUT_VCENTER), GUIOBJ(updown), -4, 0);
      } else {
        obj_layout(GUIOBJ(&updown->up), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y1), GUIOBJ(updown), -UPDOWN_LEFT, UPDOWN_TOP);
        obj_layout(GUIOBJ(&updown->down), (LAYOUT) (LAYOUT_X2 | LAYOUT_BOTTOM), GUIOBJ(&updown->up), 0, 3);
        obj_layout(GUIOBJ(&updown->text), (LAYOUT) (LAYOUT_X1 | LAYOUT_VCENTER), GUIOBJ(updown), 2, 0);
      }
      break;
      
    // set a default size for the object
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + UPDOWN_MIN_W;
      rect->bottom = rect->top + UPDOWN_MIN_H;
    } return;
      
    // the up button was pressed
    case ID_BUTTON_UP:
      updown_setcur(updown, updown->cur + 1);
      // we don't want the button to toggle, so mark as
      //  selected so that framework will toggle it to 
      //  unselected.
      obj_select(GUIOBJ(&updown->up), TRUE);
      if (updown->flags & UPDOWN_FIT_TO_TEXT)
        obj_geometry(GUIOBJ(updown));
      return;
    
    // the down button was pressed
    case ID_BUTTON_DOWN:
      updown_setcur(updown, updown->cur - 1);
      // we don't want the button to toggle, so mark as
      //  selected so that framework will toggle it to 
      //  unselected.
      obj_select(GUIOBJ(&updown->down), TRUE);
      if (updown->flags & UPDOWN_FIT_TO_TEXT)
        obj_geometry(GUIOBJ(updown));
      return;
  }
  
  // else let the object class handle it
  obj_class();
}

/*  obj_updown()
 *       updown = pointer to the updown object
 *         size = size of memory to allocate
 *       parent = parent object (usually the root)
 *        theid = id of the scroll (usually ignored)
 *
 *  This creates an updown object
 *   
 */
struct UPDOWN *obj_updown(struct UPDOWN *updown, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct UPDOWN);
  struct BUTTON *button;
  struct TEXTUAL *text;
  char *ptr;
  
  updown = (struct UPDOWN *) object(GUIOBJ(updown), updown_class, size, parent, theid);
  if (updown) {
    // is not armable or selectable
    obj_armable(GUIOBJ(updown), FALSE);
    obj_selectable(GUIOBJ(updown), FALSE);
    
    // set defaults
    updown->flags = UPDOWN_LEFT_TEXT;
    updown->cur = 0;
    
    // create the textual object
    text = obj_edit_text(&updown->text, sizeof(struct TEXTUAL), GUIOBJ(updown), 0);
    textual_copy(text, "0");
    textual_align(text, ALIGN_CENTER);
    textual_set_flags(text, TEXTUAL_FLAGS_BORDER | TEXTUAL_FLAGS_READONLY);
    text_obj_color(text, GUICOLOR_black, GUICOLOR_white);
    textual_set_font(text, "Simple");
    
    // create the up button
    button = obj_button(&updown->up, 0, GUIOBJ(updown), ID_BUTTON_UP, 0);
    if (!button)
      return NULL;
    obj_defaultrect(GUIOBJ(button), NULL);
    obj_resize(GUIOBJ(button), UPDOWN_BUTTON_W, UPDOWN_BUTTON_H);
    
    // create the down button
    button = obj_button(&updown->down, 0, GUIOBJ(updown), ID_BUTTON_DOWN, 0);
    if (!button)
      return NULL;
    obj_defaultrect(GUIOBJ(button), NULL);
    obj_resize(GUIOBJ(button), UPDOWN_BUTTON_W, UPDOWN_BUTTON_H);
  }
  
  // return the updown object
  return updown;
}
