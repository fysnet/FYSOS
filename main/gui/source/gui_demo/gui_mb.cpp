/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * gui_mb.cpp
 *  
 *
 *  this is the message box code. it isn't part of the GUI system, though is the most
 *   used window of all of the system, so I show how to create it here.
 *  you may wish to leave it in your code since it is easy to implement.
 *  however, please read the notes below about multi-tasking environments
 *
 */

#include "../include/ctype.h"
#include "gui.h"
#include "palette.h"

#include "gui_mb.h"

// Types of message boxes we allow
//   at most GUI_MB_MAX_BUTTONS buttons, at minimum atleast 1 button
struct GUI_MB_BUTTON_INFO {
  GUI_MB_TYPE type;   // type of mb (see enum GUI_MB_TYPE)
  int icon_id;        // id of icon to display (-1 = none)
  int count;          // count of buttons used
  struct INFO {
    int  id;          // id to return if pressed
    char str[16];     // text to display on button
  } button[GUI_MB_MAX_BUTTONS];
} mb_buttons[GUI_MB_TYPES] = {
  {  
    GUI_MB_OK,                    // type
    -1,                           // icon id (none)
    1,                            // count
    {
      { ID_MB_OKAY,   "&Okay"},   // button[0] (right most button)
    },
  },
  {  
    GUI_MB_OKCANCEL,              // type
    ID_ICON_INFO,                 // icon id
    2,                            // count
    {
      { ID_MB_CANCEL, "&Cancel"}, // button[0] (right most button)
      { ID_MB_OKAY,   "&Okay"},   // button[1]
    },
  },
  {  
    GUI_MB_RETRYCANCEL,           // type
    ID_ICON_CAUTION,              // icon id
    2,                            // count
    {
      { ID_MB_CANCEL, "&Cancel"}, // button[0] (right most button)
      { ID_MB_RETRY,  "&Retry"},  // button[1]
    },
  },
  {  
    GUI_MB_YESNO,                 // type
    ID_ICON_QUESTION,             // icon id
    2,                            // count
    {
      { ID_MB_NO,     "&No"},     // button[0] (right most button)
      { ID_MB_YES,    "&Yes"},    // button[1]
    },
  },
  {  
    GUI_MB_YESNOCANCEL,           // type
    ID_ICON_CAUTION,              // icon id
    3,                            // count
    {
      { ID_MB_CANCEL, "&Cancel"}, // button[0] (right most button)
      { ID_MB_NO,     "&No"},     // button[1]
      { ID_MB_YES,    "&Yes"},    // button[2]
    },
  },
  {  
    GUI_MB_ABORT,                 // type
    ID_ICON_STOP,                 // icon id
    1,                            // count
    {
      { ID_MB_CANCEL, "&Abort"},  // button[0] (right most button)
    },
  },
};

/* gui_message_box()
 *      win = parent window calling for the message box (NULL for root)
 *    title = title bar string to use
 *      str = message of message box
 *     type = type of message box to use (listed above)
 *  
 *  creates and displays a message box
 *
 */
int gui_message_box(struct WIN *win, const char *title, const char *str, GUI_MB_TYPE type) {
  int i, j, x, y, w, h;
  bool fnd = FALSE;
  
  // create message box object
  struct GUI_MB *gui_mb = (struct GUI_MB *) obj_win(NULL, win, sizeof(struct GUI_MB), mb_handler, 0, 0);
  if (gui_mb) {
    // check for valid type passed, else default to GUI_MB_OKCANCEL
    for (i=0; i<GUI_MB_TYPES && !fnd; i++)
      fnd = (mb_buttons[i].type == type);
    if (!fnd)
      type = GUI_MB_OKCANCEL;
    
    // set the defaults/type
    gui_mb->ret_val = -1;
    gui_mb->type = type;
    w = 0;
    h = 0;
    
    // find the type given
    for (i=0; i<GUI_MB_TYPES; i++) {
      if (mb_buttons[i].type == type) {
        // create the count of buttons
        for (j=0; j<mb_buttons[i].count; j++) {
          obj_button(&gui_mb->button[j], sizeof(struct BUTTON), GUIOBJ(gui_mb), mb_buttons[i].button[j].id, BUTTON_DEFAULT);
          textual_set(GUITEXTUAL(&gui_mb->button[j]), mb_buttons[i].button[j].str, -1, FALSE);
          text_obj_color(GUITEXTUAL(&gui_mb->button[j]), GUICOLOR_black, GUICOLOR_transparent);
          obj_defaultrect(GUIOBJ(&gui_mb->button[j]), NULL);
          textual_align(GUITEXTUAL(&gui_mb->button[j]), ALIGN_CENTER);
          w = MAX(w, gui_w(&gui_mb->button[j]));  // save the largest width of each button
          h = MAX(h, gui_h(&gui_mb->button[j]));  // save the largest height of each button
        }
        
        // now make all the buttons the same size
        for (j=0; j<mb_buttons[i].count; j++)
          obj_resize(GUIOBJ(&gui_mb->button[j]), w, h);
        
        // include an icon image?
        if (mb_buttons[i].icon_id > -1) {
          obj_image(&gui_mb->icon, sizeof(struct IMAGE), GUIOBJ(gui_mb), 0);
          obj_defaultrect(GUIOBJ(&gui_mb->icon), NULL);
          image_ownerdraw(&gui_mb->icon, mb_buttons[i].icon_id, 0, 0);
        }
        
        // done
        break;
      }
    }
    
    // the message
    obj_textual(&gui_mb->body, sizeof(struct TEXTUAL), GUIOBJ(gui_mb), 0);
    textual_set(&gui_mb->body, str, -1, FALSE);
    // if you want the "sunken in" look, uncomment the next line
    //textual_set_flags(&gui_mb->body, TEXTUAL_FLAGS_BORDER);
    text_obj_color(&gui_mb->body, GUICOLOR_black, GUICOLOR_transparent);
    obj_defaultrect(GUIOBJ(&gui_mb->body), NULL);
    textual_align(&gui_mb->body, ALIGN_LEFT);
    // if you want to change from the default font, uncomment the next line
    //textual_set_font(&gui_mb->body, "Courier New");
    
    // Title Bar
    textual_set(GUITEXTUAL(gui_mb), title, -1, FALSE);
    
    // calculate size
    // height
    if ((mb_buttons[i].icon_id > -1) && (gui_h(&gui_mb->icon) > textual_height(&gui_mb->body)))
      h = gui_h(&gui_mb->icon) + 14;
    else
      h = textual_height(&gui_mb->body) + 20;
    h += gui_h(&gui_mb->button[0]);     // height of a button
    
    // width
    w = 15;
    if (mb_buttons[i].icon_id > -1)
      w += gui_w(&gui_mb->icon);        // width of icon if used
    w += textual_width(&gui_mb->body);  // width of body
    
    // make sure we are wide enough for all buttons too
    x = 0;
    for (j=0; j<mb_buttons[i].count; j++)
      x += gui_w(&gui_mb->button[j]);
    w = MAX(w, x + 4);
    
    // calculate position
    if (win) {
      // center in parent
      x = gui_left(win) + ((gui_w(win) - w) / 2);
      y = gui_top(win) + ((gui_h(win) - h) / 2);
    } else {
      // else center in root
      x = (gui_w(DESKTOP) - w) / 2;
      y = (gui_h(DESKTOP) - h) / 2;
    }
    obj_position(GUIOBJ(gui_mb), x, y, w, h);
    
    // do the Geometry, calling the handler to do it for us
    obj_geometry(GUIOBJ(gui_mb));
    
    // if has parent, lock the parent
    if (win)
      win_modal(GUIWIN(gui_mb), win);
    
    // make sure we are on top and draw the message box
    win_activate(GUIWIN(gui_mb));
    win_dirty(GUIWIN(gui_mb));
    
    // *************************************************************
    // in a multitasking environment, you would not return
    //  to the caller window until the user ended this message box
    //
    //    while (gui_mb->ret_val < 0)
    //      ;
    //    atom_delete(&gui_mb->base.atom);
    //    obj_top(GUIOBJ(win));
    //    win_activate(win);
    //    return gui_mb->ret_val;
    //
    // since this demo is not a multitasking environment, we simply
    // "close" the message box in the handler below.
    // *************************************************************
    
    // return the ID of the button pressed (gui_mb->ret_val updated in the handler below)
    return gui_mb->ret_val;
  } else
    return -1;
}

/* mb_handler()
 *      win = message box window
 *  
 *   eventstack.event = current event
 *   eventstack.object = GUIOBJ(win)
 *   eventstack.data = depending on event: key: keyinfo
 *
 *  message box handler
 *
 */
void mb_handler(struct WIN *win) {
  int i, j;
  struct GUI_MB *gui_mb = (struct GUI_MB *) win;
  
  switch ((int) eventstack.event) {
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      // TODO: This needs to be conditional on the object being selected
      answer(gui_mb);
      return;
    
    case FOCUS_LOST:
      // do we need to do something if the focus is lost?
      break;
      
    case FOCUS_GOT:
      // do we need to do something if the focus is got?
      break;
      
    // called to set the geometry of the message box
    // this will do the layout of all the objects
    case GEOMETRY:
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        win_handler(win);
        
        for (i=0; i<GUI_MB_TYPES; i++) {
          if (mb_buttons[i].type == gui_mb->type) {
            obj_layout(GUIOBJ(&gui_mb->button[0]), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y2), GUIOBJ(gui_mb), -2, -2);
            for (j=1; j<mb_buttons[i].count; j++)
              obj_layout(GUIOBJ(&gui_mb->button[j]), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&gui_mb->button[j-1]), 2, 0);
            if (mb_buttons[i].icon_id > -1) {
              obj_layout(GUIOBJ(&gui_mb->body), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y1), GUIOBJ(gui_mb), -4, 6);
              // if the icon is taller than the body text, VCENTER will put the top of the icon into the title bar. 
              if (gui_h(&gui_mb->icon) > gui_h(&gui_mb->body))
                obj_layout(GUIOBJ(&gui_mb->icon), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&gui_mb->body), 4, 0);
              else
                obj_layout(GUIOBJ(&gui_mb->icon), (LAYOUT) (LAYOUT_LEFT | LAYOUT_VCENTER), GUIOBJ(&gui_mb->body), 4, 0);
            } else
              obj_layout(GUIOBJ(&gui_mb->body), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(gui_mb), 6, 6);
            break;
          }
        }
    
        return;
      }
      break;
      
    // makes a default sized message box
    case DEFAULTRECT:
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        struct RECT *rect = defaultrect_data();
        rect->right = rect->left + 320-1;
        rect->bottom = rect->top + 200-1;
        return;
      }
      break;
      
    // catch any key that may be a mnemonic on a button
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      struct OBJECT *obj;
      
      // check all of the objects for the '&' char in their text strings
      obj = win_check_mnemonics(win, key_info->ascii);
      if (obj) {
        obj_event(obj, POINTER_CLICK, 0);
        obj_arm(obj, FALSE);
      }
    } return;
  }
  
  // check to see if it was one of the button press ID's for this Message Box type
  for (i=0; i<GUI_MB_TYPES; i++) {
    if (mb_buttons[i].type == gui_mb->type) {
      for (j=0; j<mb_buttons[i].count; j++) {
        if ((int) eventstack.event == mb_buttons[i].button[j].id) {
          gui_mb->ret_val = (int) eventstack.event;
          
          // see notes at 'return' in the function above.
          // in a multitasking environment, we would simply
          //  return here.  however, to keep the MB active
          //  for this demo, we need a place to delete the
          //  MB when the user clicks on a button.
          // Therefore, delete these three lines when you
          //  get a multi tasking environment going.
          atom_delete(&gui_mb->base.atom);
          obj_top(GUIOBJ(win));
          win_activate(win);
          
          return;
        }
      }
    }
  }
  
  // else call the window handler
  win_handler(win);
}
