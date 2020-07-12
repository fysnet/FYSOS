/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * checkbox.cpp
 *  
 *  this is used to create checkboxes, radio buttons, and onoff objects
 *
 */

#include "gui.h"
#include "grfx.h"

/* checkbox_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to checkbox
 *   eventstack.data = rect of win object
 */
void checkbox_class(void) {
  struct CHECK_BOX *checkbox = (struct CHECK_BOX *) eventstack.object;
  
  switch (eventstack.event) {
    // draw checkbox to screen
    case EXPOSE:
      gui_checkbox(checkbox);
      return;
    
    // checkbox was clicked so change state, draw it, and send change notice to parent
    case POINTER_CLICK:
      obj_select(GUIOBJ(checkbox), !obj_selected(GUIOBJ(checkbox)));
      checkbox->checked ^= TRUE;
      obj_dirty(GUIOBJ(checkbox), TRUE);
      // inform the parent that we changed the checkbos setting
      emit(CHECK_BOX_CHANGED, checkbox);
      return;
      
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      switch (key_info->code) {
        case VK_ENTER:
          obj_event(GUIOBJ(checkbox), POINTER_CLICK, NULL);
          break;
      }
    } break;
      
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + textual_width(GUITEXTUAL(checkbox)) + (CHECK_BOX_WIDTH * 2);
      rect->bottom = rect->top + textual_height(GUITEXTUAL(checkbox)) + 4;
      return;
    }
  }

  // of event not handled here, try the textual class
  textual_class();
}

/* obj_checkbox()
 *     checkbox = pointer to the checkbox object
 *           NULL = allocate the memory here
 *         ! NULL = memory is already allocated/static
 *         size = size of memory to allocate
 *       parent = parent object (usually a window object)
 *        theid = id value to return to object when checked
 *
 *
 *  this creates a check box
 */
struct CHECK_BOX *obj_checkbox(struct CHECK_BOX *checkbox, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct CHECK_BOX);
  
  checkbox = (struct CHECK_BOX *) obj_textual(GUITEXTUAL(checkbox), size, parent, theid);
  if (checkbox) {
    // point to checkbox class
    GUIOBJ(checkbox)->_class = checkbox_class;
    // allow underline for mnemonics
    GUITEXTUAL(checkbox)->flags |= TEXTUAL_FLAGS_UNDERLINE;
    
    // armable and has mnemonic
    obj_armable(GUIOBJ(checkbox), TRUE);
    obj_set_mnemonic(GUIOBJ(checkbox), TRUE);
    
    // it starts as not checked, and text is to the right, box on left
    checkbox->checked = FALSE;
    checkbox->right = FALSE;
  }
  
  // return pointer to checkbox
  return checkbox;
}

/* onoff_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to onoff object
 *   eventstack.data = rect of win object
 */
void onoff_class(void) {
  struct ONOFF *onoff = (struct ONOFF *) eventstack.object;
  
  // returns the rect for the object
  struct RECT *rect = defaultrect_data();
  
  switch (eventstack.event) {
    case DESTRUCT:
      atom_delete((struct ATOM *) onoff->bitmap);
      break;
    
    // draw to the screen
    case EXPOSE:
      gui_onoff(onoff);
      return;
      
    // onoff was clicked so change state, draw it, and send change notice to parent
    case POINTER_CLICK:
      obj_select(GUIOBJ(onoff), !obj_selected(GUIOBJ(onoff)));
      onoff->on ^= TRUE;
      obj_dirty(GUIOBJ(onoff), TRUE);
      // inform the parent that we changed the onoff setting
      emit(ONOFF_CHANGED, onoff);
      return;
      
    // set the size of the object to the icon selected
    case GEOMETRY:
      obj_resize(GUIOBJ(onoff), gui_w(onoff->bitmap), gui_h(onoff->bitmap));
      break;
    
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      switch (key_info->code) {
        case VK_ENTER:
          obj_event(GUIOBJ(onoff), POINTER_CLICK, NULL);
          break;
      }
    } break;
      
    // give a default size for object
    case DEFAULTRECT:
      rect->right = rect->left + 40;  // default width of 40
      rect->bottom = rect->top + 20;  // default height of 20
      return;
  }
  
  // if event not handled here, try the object class handler
  obj_class();
}

/* obj_onoff()
 *        onoff = pointer to the onoff object
 *           NULL = allocate the memory here
 *         ! NULL = memory is already allocated/static
 *         size = size of memory to allocate
 *       parent = parent object (usually a window object)
 *        theid = id value to return to object when set
 *
 *
 *  this creates an onoff object
 */
struct ONOFF *obj_onoff(struct ONOFF *onoff, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct ONOFF);
  
  onoff = (struct ONOFF *) object(GUIOBJ(onoff), onoff_class, size, parent, theid);
  if (onoff) {
    // set the class to our onoff class
    GUIOBJ(onoff)->_class = onoff_class;
    
    // armable
    obj_armable(GUIOBJ(onoff), TRUE);
    
    // the object to draw from
    onoff->bitmap = get_static_bitmap(ID_ON_OFF, 40, 20);
    // initially off
    onoff->on = FALSE;
  }

  // return pointer to object
  return onoff;
}

/* radio_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to radio object
 *   eventstack.data = rect of win object
 */
void radio_class(void) {
  struct RADIO *p, *radio = (struct RADIO *) eventstack.object;
  struct RECT *rect = defaultrect_data();
  
  switch (eventstack.event) {
    // draw the radio button to the screen
    case EXPOSE:
      gui_radio(radio);
      return;
      
    // user clicked on radio button
    //  only one can be set at a time, within a group, so check all
    //  other buttons and clear them.
    case POINTER_CLICK:
      // we need to find the other one that is set within this group.
      // first try to find it in the previous direction
      if (!radio->set) {
        p = radio->prev;
        while (p) {
          if (p->set) {
            p->set = FALSE;
            obj_dirty(GUIOBJ(p), FALSE);
            radio->set = TRUE;
            break;
          }
          p = p->prev;
        }
      }
      // if we are now not set, we didn't find it in the previous direction.
      // so, try to find it in the next direction
      if (!radio->set) {
        p = radio->next;
        while (p) {
          if (p->set) {
            p->set = FALSE;
            obj_dirty(GUIOBJ(p), FALSE);
            radio->set = TRUE;
            break;
          }
          p = p->next;
        }
      }
      // If we still didn't find it, then all were clear, so just set it
      // (this should not happen, but can if the user doesn't initially set one of them)
      if (!radio->set)
        radio->set = TRUE;
      
      // mark this one as dirty
      obj_dirty(GUIOBJ(radio), TRUE);
      
      // inform the parent that we changed the radio button setting
      emit(RADIO_CHANGED, radio);
      return;
      
    // if an up or down arrow key was pressed, move to the prev or next button
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      switch (key_info->code) {
        case VK_UPARROW:
          p = radio->prev;
          while (p) {
            if (!obj_disabled(GUIOBJ(p))) {
              obj_event(GUIOBJ(p), POINTER_CLICK, NULL);
              obj_focus(GUIOBJ(p));
              break;
            } else
              p = p->prev;
          }
          break;
        case VK_DNARROW:
          p = radio->next;
          while (p) {
            if (!obj_disabled(GUIOBJ(p))) {
              obj_event(GUIOBJ(p), POINTER_CLICK, NULL);
              obj_focus(GUIOBJ(p));
              break;
            } else
              p = p->next;
          }
          break;
        case VK_ENTER:
          obj_event(GUIOBJ(radio), POINTER_CLICK, NULL);
          break;
      }
    } break;
    
    // give a default size to the button
    case DEFAULTRECT:
      rect->right = rect->left + textual_width(GUITEXTUAL(radio)) + (RADIO_WIDTH * 2);
      rect->bottom = rect->top + textual_height(GUITEXTUAL(radio)) + 4;
      return;
  }
  
  // if we didn't handle the event here, try the textual class
  textual_class();
}

/* obj_radio()
 *        onoff = pointer to the radio object
 *           NULL = allocate the memory here
 *         ! NULL = memory is already allocated/static
 *         size = size of memory to allocate
 *       parent = parent object (usually a window object)
 *        theid = id value to return to object when set
 *
 *
 *  this creates a radio object
 */
struct RADIO *obj_radio(struct RADIO *radio, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct RADIO);
  
  // radio objects are base classed to textual objects
  radio = (struct RADIO *) obj_textual(GUITEXTUAL(radio), size, parent, theid);
  if (radio) {
    // set the class to radio so it goes here first
    GUIOBJ(radio)->_class = radio_class;
    // allow underlines for mnemonics
    GUITEXTUAL(radio)->flags |= TEXTUAL_FLAGS_UNDERLINE;
    
    // is armable and has mnemonics
    obj_armable(GUIOBJ(radio), TRUE);
    obj_set_mnemonic(GUIOBJ(radio), TRUE);
    
    // unlink initially
    radio->prev = NULL;
    radio->next = NULL;
    
    // initially not set
    radio->set = FALSE;
    
    // radio on left, text on right (initially)
    radio->right = FALSE;
  }

  // return radio object
  return radio;
}

/* obj_radio_get_checked_id()
 *        radio = pointer to the radio object
 *
 * This function will return the ID of the set radio button within the group
 *  that 'radio' is a part of, of -1 if none are set
 */
int obj_radio_get_checked_id(struct RADIO *radio) {
  struct RADIO *p;
  
  // if it is this one, return its id
  if (radio->set)
    return GUIOBJ(radio)->id;
  
  // else, search in the prev direction
  p = radio->prev;
  while (p) {
    if (p->set)
      return GUIOBJ(p)->id;
    p = p->prev;
  }
  
  // else, search in the next direction
  p = radio->next;
  while (p) {
    if (p->set)
      return GUIOBJ(p)->id;
    p = p->next;
  }
  
  // If we didn't find the one set, return -1
  return -1;
}

/* obj_radio_get_checked_num()
 *        radio = pointer to the radio object
 *
 * This function will return the sequence number (0 - n) of the set radio button 
 *  within the group that 'radio' is a part of, of -1 if none are set
 */
int obj_radio_get_checked_num(struct RADIO *radio) {
  int n = 0;
  
  // find the first one
  while (radio->prev)
    radio = radio->prev;
  
  // search until we find the one set
  do {
    if (radio->set)
      return n;
    n++;
    radio = radio->next;
  } while (radio);
  
  // else return that there was none set
  return -1;  
}
