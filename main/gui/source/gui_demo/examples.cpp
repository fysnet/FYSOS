/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * examples.cpp
 *  
 *  these are some example objects.  They don't have anything to do with the
 *   GUI system.  They are simply items added to show how to use the system.
 *  you should not add these to your GUI system.
 *
 */
#include <memory.h>
#include <string.h>

#include "../include/ctype.h"
#include "gui.h"
#include "grfx.h"

#include "examples.h"
#include "gui_mb.h"

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 0 (handler)
 *  
 *   this is a list example.  It adds roughly 10 items to a list and allows
 *    the user to select an item and make it the current title bar contents
 *
 */
void handler0(struct WIN *win) {
  struct APP0 *app = (struct APP0 *) win;
  struct SCROLL *scroll = (struct SCROLL *) &app->list;
  
  switch (eventstack.event) {
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      if ((GUIOBJ(eventstack.object) == GUIOBJ(app)) ||
          (GUIOBJ(eventstack.object) == GUIOBJ(win->border))) {
        answer(app);
        return;
      }
      break;
      
    case FOCUS_LOST:
      // do we need to do something if the focus is lost?
      break;
      
    case FOCUS_GOT:
      // do we need to do something if the focus is got?
      break;
      
    case GEOMETRY: {
      // Place the objects in a nice place when geometry is set
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        
        // Close button in lower right
        obj_layout(GUIOBJ(&app->close), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y2), GUIOBJ(app), -2, -2);
        
        // Apply button to left of close button
        obj_layout(GUIOBJ(&app->apply), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&app->close), 2, 0);
        
        // The remainig space is for the list
        obj_position(GUIOBJ(&app->list), 2, 2, gui_w(app) - 4, obj_y(GUIOBJ(&app->close)) - 5);
        obj_geometry(GUIOBJ(&app->list));
      }
    } break;
      
    case LIST_CHANGED:
      if (eventstack.data == GUIOBJ(&app->list)) {
        obj_disable(GUIOBJ(&app->apply), (list_count_selected(&app->list) != 1));
        obj_dirty(GUIOBJ(&app->apply), FALSE);
      }
      return;
      
    case SELECT:
      if (GUIOBJ(eventstack.object) == GUIOBJ(&app->apply)) {
        // Change the title of the window when apply is clicked
        const struct LISTELEM *last = list_selected(&app->list, NULL);
        
        if (last) {
          const char *str = textual_text(GUITEXTUAL(last), NULL);
          
          // We need to do this before and after because the new window
          //  border might be smaller with the new title
          win_dirty(GUIWIN(app));
          
          // since the list item's text may change (doubtful, but could), we don't
          //  want the title to change. Therefore, can not point the two textual
          //  objects to the same text.
          textual_copy(GUITEXTUAL(app), str);
          obj_geometry(GUIOBJ(app));
          
          win_dirty(GUIWIN(app));
        }
        return; // Dont let the apply button toggle
      }
      
      listelem_class();
      return;
      
    case LPOINTER_RELEASE:
      // Close when the ok button is set
      if (GUIOBJ(eventstack.object) == GUIOBJ(&app->close)) {
        atom_delete(&win->base.atom);
        return;
      }
      break;
      
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      struct OBJECT *obj;
      
      // first check all of the objects for the '&' char in their text strings
      obj = win_check_mnemonics(win, key_info->ascii);
      if (obj) {
        obj_event(obj, POINTER_CLICK, 0);
        obj_arm(obj, FALSE);
        return;
      }
      
      // this is where other key checks would take place if wanted
      
      // else, call the list class to allow the arrow keys to move the selected item
      eventstack.object = (struct DERIVED *) GUIOBJ(&app->list);
      list_class();
    } return;
  }
  
  win_handler(GUIWIN(app));
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 0 (app)
 *
 *  this is the code the user app would use to create the example
 *
 */
struct APP0 *newapp0(void) {
  // Create the window for the app
  struct APP0 *app = (struct APP0 *) obj_win(NULL, DESKTOP, sizeof(struct APP0), handler0, 0, BORDER_NORMAL);
  
  if (app) {
    // Make a list and fill it with some titles
    obj_list(&app->list, 0, GUIOBJ(app), 0, LIST_MULTIPLE);
    textual_set(GUITEXTUAL(&app->list), "Choose a title", -1, FALSE);
    
    list_append(&app->list, "List Item 1", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "This entry is long and at\nleast two lines long", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 3", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 4", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 5", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 6", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 7", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 8", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 9", -1, FALSE, 0, NULL, NULL);
    list_append(&app->list, "List Item 10", -1, FALSE, 0, NULL, NULL);
    
    // Make a close and apply button
    obj_button(&app->close, 0, GUIOBJ(app), 0, BUTTON_EXIT);
    textual_set(GUITEXTUAL(&app->close), "&Close", -1, FALSE);
    obj_defaultrect(GUIOBJ(&app->close), 0);
    textual_align(GUITEXTUAL(&app->close), ALIGN_CENTER);
    
    obj_button(&app->apply, 0, GUIOBJ(app), 0, BUTTON_DEFAULT);
    textual_set(GUITEXTUAL(&app->apply), "&Apply", -1, FALSE);
    obj_defaultrect(GUIOBJ(&app->apply), 0);
    textual_align(GUITEXTUAL(&app->apply), ALIGN_CENTER);
    obj_event(GUIOBJ(app), LIST_CHANGED, &app->list); // update the apply button status
  }
  return app;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 1 (handler)
 *
 *  this example is a window that contains a progress bar, radio buttons, checkboxes,
 *   onoff object, two slider objects, a few text boxes, and two buttons.
 *
 */
void handler1(struct WIN *win) {
  struct APP1 *app = (struct APP1 *) win;
  int i;
  
  switch (eventstack.event) {
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      if ((GUIOBJ(eventstack.object) == GUIOBJ(app)) ||
          (GUIOBJ(eventstack.object) == GUIOBJ(win->border))) {
        answer(app);
        return;
      }
      break;
      
    case FOCUS_LOST:
      // do we need to do something if the focus is lost?
      break;
      
    case FOCUS_GOT:
      // do we need to do something if the focus is got?
      break;
      
    case GEOMETRY:
      // Place the objects in a nice place when geometry is set
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        // plus button in lower right
        obj_layout(GUIOBJ(&app->plus), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y2), GUIOBJ(app), -2, -2);
        
        // minus button to left of plus button
        obj_layout(GUIOBJ(&app->minus), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&app->plus), 2, 0);
        
        // The progress bar
        obj_position(GUIOBJ(&app->progress), 50, 20, gui_w(win) - 100, 15);
        obj_geometry(GUIOBJ(&app->progress));

        // checkboxes
        obj_layout(GUIOBJ(&app->toggle[0]), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y1), GUIOBJ(app), -10, 20 + 15 + 15);
        for (i=1; i<APP1_TOGGLES; i++)
          obj_layout(GUIOBJ(&app->toggle[i]), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&app->toggle[i-1]), 0, 2);
        
        // radio to left of checkbox
        obj_layout(GUIOBJ(&app->radio[0]), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&app->toggle[0]), 10, 0);
        for (i=1; i<APP1_RADIOS; i++)
          obj_layout(GUIOBJ(&app->radio[i]), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&app->radio[i-1]), 0, 2);
        obj_layout(GUIOBJ(&app->radio_text), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&app->radio[i-1]), 0, 2);
        obj_resize(GUIOBJ(&app->radio_text), gui_w(&app->radio[i-1]), GUIDEF);
        
        // url text
        obj_layout(GUIOBJ(&app->url), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&app->radio_text), 0, 5);
        
        // vslider bar under the progress bar
        obj_position(GUIOBJ(&app->vslider), 10, 20 + gui_h(&app->progress) + 4, 26, 100);
        obj_geometry(GUIOBJ(&app->vslider));
        
        // hslider bar under the progress bar
        obj_position(GUIOBJ(&app->hslider), 10 + gui_w(&app->vslider) + 4, 20 + gui_h(&app->progress) + 4, 100, 26);
        obj_geometry(GUIOBJ(&app->hslider));
        
        // updown under the horz sliderbar
        obj_position(GUIOBJ(&app->updown), 10 + gui_w(&app->vslider) + 30, 20 + gui_h(&app->progress) + 4 + gui_h(&app->hslider) + 20, 10, 10);
        obj_geometry(GUIOBJ(&app->updown));
        
        // on off switch under the updown
        obj_layout(GUIOBJ(&app->onoff_static_text), (LAYOUT) (LAYOUT_HCENTER | LAYOUT_BOTTOM), GUIOBJ(&app->updown), 0, 15);
        obj_geometry(GUIOBJ(&app->onoff));  // do this first to get the correct size of the object via the bitmap
        obj_layout(GUIOBJ(&app->onoff), (LAYOUT) (LAYOUT_HCENTER | LAYOUT_BOTTOM), GUIOBJ(&app->onoff_static_text), 0, 5);
        
        // sunken in static text
        obj_layout(GUIOBJ(&app->static_text), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y2), GUIOBJ(app), 2, -2);
        
        // password enter
        obj_layout(GUIOBJ(&app->static_pass), (LAYOUT) (LAYOUT_X1 | LAYOUT_TOP), GUIOBJ(&app->static_text), 0, 10);
        obj_layout(GUIOBJ(&app->static_user), (LAYOUT) (LAYOUT_X2 | LAYOUT_TOP), GUIOBJ(&app->static_pass), 0, 9);
        obj_layout(GUIOBJ(&app->password), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_VCENTER), GUIOBJ(&app->static_pass), 3, 0);
        obj_layout(GUIOBJ(&app->username), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_VCENTER), GUIOBJ(&app->static_user), 3, 0);
      }
      break;
      
    case SELECT:
      // subtract from the current progress when the minus button is pressed
      if (GUIOBJ(eventstack.object) == GUIOBJ(&app->minus)) {
        if (app->progress.percent > 0) {
          app->progress.percent--;
          obj_dirty(GUIOBJ(&app->progress), FALSE);
        }
        return; // Dont let the button toggle
      }
      
      // add to the current progress when the plus button is pressed
      if (GUIOBJ(eventstack.object) == GUIOBJ(&app->plus)) {
        if (app->progress.percent < 100) {
          app->progress.percent++;
          obj_dirty(GUIOBJ(&app->progress), FALSE);
        }
        return; // Dont let the button toggle
      }
      break;
      
    case RADIO_CHANGED: {
      struct RADIO *radio = (struct RADIO *) eventstack.data;
      char num_str[32];
      sprintf(num_str, "NUM: %i, ID: %i", obj_radio_get_checked_num(radio), obj_radio_get_checked_id(radio));
      textual_copy(&app->radio_text, num_str);
      obj_dirty(GUIOBJ(&app->radio_text), FALSE);
      return;
    }
    
    case CHECK_BOX_CHANGED: {
      struct CHECK_BOX *checkbox = (struct CHECK_BOX *) eventstack.data;
      if (GUIOBJ(checkbox)->id == CHECK_BOX_ID2) {
        obj_disable(GUIOBJ(&app->radio[RAIDO_DISABLE]), checkbox->checked);
        obj_dirty(GUIOBJ(&app->radio[RAIDO_DISABLE]), FALSE);
      } else if (GUIOBJ(checkbox)->id == CHECK_BOX_ID0) {
        app->progress.show ^= TRUE;
        obj_dirty(GUIOBJ(&app->progress), FALSE);
      }
      return;
    }

    case KEY: {
      // TODO:
      //  most if not all of this code for the KEY event should probably be moved
      //   to the win_handler() function, since each window will call it anyway.
      //  however, some windows may not want to check for mnemonics and the sort.
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      struct OBJECT *obj;
      
      if (obj_get_focus() == GUIOBJ(win)) {
        // check all of the objects for the '&' char in their text strings
        obj = win_check_mnemonics(win, key_info->ascii);
        if (obj) {
          obj_event(obj, POINTER_CLICK, 0);
          obj_arm(obj, FALSE);
        }
        return;
      }
      
      // if not mnemonic, get current focus and call underlining event handler(s)
      eventstack.object = (struct DERIVED *) obj_get_focus();
      
      // move to the next object within this window if the TAB key was pressed
      // TODO:
      // this needs a little work since once we find the last object, we stop
      //  looking for the next.  therefore, if the current object is near or
      //  at the end (last), we don't roll-over to the first...
      // however, this does show you how to do it.
      if (key_info->code == VK_TAB) {
        gfx_show_focus(TRUE);
        struct OBJECT *obj = GUIOBJ(eventstack.object)->next;
        while (obj) {
          if (obj_disabled(obj) || !obj_event(obj, FOCUS_WANT, NULL))
            obj = obj->next;
          else {
            obj_focus(obj);
            return;
          }
        }
      }
    }
  }
  
  win_handler(GUIWIN(app));
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 1 (app)
 *
 *   this is the user app that the user would use to create these objects
 *
 */
struct APP1 *newapp1(void) {
  int i;
  char *text;
  
  // Create the window for the app
  struct APP1 *app = (struct APP1 *) obj_win(NULL, DESKTOP, sizeof(struct APP1), handler1, 0, BORDER_NORMAL);
  
  if (app) {
    // Make a progress bar
    obj_progress(&app->progress, sizeof(struct PROGRESS), GUIOBJ(app), 0);
    
    // Make a minus and plus buttons
    obj_button(&app->minus, 0, GUIOBJ(app), 0, 0);
    textual_set(GUITEXTUAL(&app->minus), "&Minus", -1, FALSE);
    obj_defaultrect(GUIOBJ(&app->minus), 0);
    textual_align(GUITEXTUAL(&app->minus), ALIGN_CENTER);
    
    obj_button(&app->plus, 0, GUIOBJ(app), 0, BUTTON_DEFAULT);
    textual_set(GUITEXTUAL(&app->plus), "&Plus", -1, FALSE);
    obj_defaultrect(GUIOBJ(&app->plus), 0);
    textual_align(GUITEXTUAL(&app->plus), ALIGN_CENTER);
    
    // Make a few check box objects
    // Must allocate the text string, since using the stack will overwrite each time
    //  The destroy() function will free the memory for us
    for (i=0; i<APP1_TOGGLES; i++) {
      obj_checkbox(&app->toggle[i], 0, GUIOBJ(app), CHECK_BOX_ID0 + i);  // an ID to use, as long as its different than the rest
      text = (char *) malloc(64);
      switch (i) {
        case 0:
          strcpy(text, "&Toggle Progress");
          break;
        case CHECK_BOX_RADIO:
          strcpy(text, "Toggle Disabled");
          break;
        default:
          sprintf(text, "Temp Toggle: %i", i);
      }
      textual_set(GUITEXTUAL(&app->toggle[i]), text, -1, TRUE);
      obj_defaultrect(GUIOBJ(&app->toggle[i]), 0);
      textual_align(GUITEXTUAL(&app->toggle[i]), ALIGN_LEFT);
    }
    obj_disable(GUIOBJ(&app->toggle[3]), TRUE);  // disable the fourth one to show disabled
    app->toggle[CHECK_BOX_RADIO].checked = TRUE; // radio button is (will be) disabled, so check this box initially

    // Make a few radio objects
    for (i=0; i<APP1_RADIOS; i++) {
      obj_radio(&app->radio[i], 0, GUIOBJ(app), RADIO_ID0 + i); // an ID to use, as long as its different than the rest
      text = (char *) malloc(64);
      if (i==0)
        sprintf(text, "&Radio Num: %i", i);
      else
        sprintf(text, "Radio Num: %i", i);
      textual_set(GUITEXTUAL(&app->radio[i]), text, -1, TRUE);
      obj_defaultrect(GUIOBJ(&app->radio[i]), 0);
      textual_align(GUITEXTUAL(&app->radio[i]), ALIGN_LEFT);
      app->radio[i].prev = (i > 0) ? &app->radio[i-1] : NULL;
      app->radio[i].next = (i < (APP1_RADIOS-1)) ? &app->radio[i+1] : NULL;
    }
    app->radio[2].set = TRUE;
    obj_disable(GUIOBJ(&app->radio[5]), TRUE);  // disable the sixth and seventh ones to show disabled
    obj_disable(GUIOBJ(&app->radio[RAIDO_DISABLE]), TRUE);  // we allow this one to be enabled via a checkbox above
    obj_static_text(&app->radio_text, 0, GUIOBJ(app), 0, "-1");
    textual_set_flags(&app->radio_text, TEXTUAL_FLAGS_BORDER | TEXTUAL_FLAGS_NOCARET);
    textual_align(&app->radio_text, ALIGN_CENTER);
    obj_event(GUIOBJ(app), RADIO_CHANGED, &app->radio[0]);  // update the text to the correct value
    
    // the url text
    obj_static_text(&app->url, 0, GUIOBJ(app), 0, "http://www.fysnet.net");
    app->url.flags = TEXTUAL_FLAGS_READONLY | TEXTUAL_FLAGS_ISLINK;
    textual_set_font(&app->url, NULL);
    textual_align(&app->url, ALIGN_CENTER);
    
    // Make the Slider Bars
    obj_sliderbar(&app->hslider, 0, GUIOBJ(app), 0);
    obj_defaultrect(GUIOBJ(&app->hslider), 0);
    obj_sliderbar(&app->vslider, 0, GUIOBJ(app), 0);
    obj_defaultrect(GUIOBJ(&app->vslider), 0);
    
    // Make the Up/Down object
    obj_updown(&app->updown, 0, GUIOBJ(app), 0);
    obj_defaultrect(GUIOBJ(&app->updown), 0);
    
    // Make an OnOff switch with static text just above it
    obj_static_text(&app->onoff_static_text, 0, GUIOBJ(app), 0, "A switched\nOnOff object");
    obj_onoff(&app->onoff, 0, GUIOBJ(app), 0);
    obj_defaultrect(GUIOBJ(&app->onoff), 0);
    
    // a sunken in static text box
    obj_static_text(&app->static_text, 0, GUIOBJ(app), 0, "A sunken in static text box");
    textual_set_flags(&app->static_text, TEXTUAL_FLAGS_BORDER | TEXTUAL_FLAGS_NOCARET);
    
    // Password Edit
    obj_static_text(&app->static_user, 0, GUIOBJ(app), 0, "User Name:");
    textual_align(&app->static_user, ALIGN_RIGHT);
    obj_static_text(&app->static_pass, 0, GUIOBJ(app), 0, " Password:");
    textual_align(&app->static_pass, ALIGN_RIGHT);
    obj_edit_text(&app->username, 0, GUIOBJ(app), 0);
    textual_copy(&app->username, "myusername");
    obj_edit_text(&app->password, 0, GUIOBJ(app), 0);
    textual_set_flags(&app->password, app->password.flags | TEXTUAL_FLAGS_ISPASS);
    
    win_set_status(GUIWIN(app), "The Status Line.");
  }
  
  return app;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 2 (handler)
 *  
 *   this example shows how to create a window that has a menu, button bar,
 *    and a text edit object.
 *
 */
void handler2(struct WIN *win) {
  struct APP2 *app = (struct APP2 *) win;
  
  switch ((int) eventstack.event) {
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      if ((GUIOBJ(eventstack.object) == GUIOBJ(app)) ||
          (GUIOBJ(eventstack.object) == GUIOBJ(win->border))) {
        answer(app);
        return;
      }
      break;
      
    case FOCUS_LOST:
      // do we need to do something if the focus is lost?
      break;
      
    case FOCUS_GOT:
      // do we need to do something if the focus is got?
      break;
      
    case GEOMETRY:
      // Place the objects in a nice place when geometry is set
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        // The Menu bar (if we created one)
        int menu_height = 0;
        if (win->menu) {
          obj_layout(GUIOBJ(win->menu), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(app), 0, 0);
          obj_geometry(GUIOBJ(win->menu));
          menu_height = gui_h(win->menu);
        }
        
        // The Button bar (if we created one)
        int button_bar_height = 0;
        if (win->button_bar) {
          if (win->menu) obj_layout(GUIOBJ(win->button_bar), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y2), GUIOBJ(win->menu), 0, 0);
          else           obj_layout(GUIOBJ(win->button_bar), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(app), 0, 0);
          obj_geometry(GUIOBJ(win->button_bar));
          button_bar_height = gui_h(win->button_bar);
        }
        
        // The remainig space is for the edit
        obj_position(GUIOBJ(&app->edit), 2, menu_height + button_bar_height + 2, gui_w(app) - 4, gui_h(app) - menu_height - button_bar_height - 2);
        obj_geometry(GUIOBJ(&app->edit));
      }
      break;
    
    case ID_FILE_EXIT: {
      int ret = gui_message_box(win, "Message Box", "This is a message box", GUI_MB_OKCANCEL);
      // in a multitasking environment, the message box would not return until
      //  the user click a button.
      if (ret == ID_MB_OKAY) {
        
      } else if (ret == ID_MB_CANCEL) {
        
      }
      //  // what about the edit stuff?????
      //  atom_delete(&win->base.atom);
    } return;
      
    case GET_RCLICK_MENU:
      // if we have one, return the pointer to the right click menu
      answer(app->rightclick);
      return;
      
    case SELECT:
      scroll_class();
      return;
      
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      // since we only want to catch ALT-key combinations here,
      //  check for them, then pass everything else edit control
      if (KEY_ALT(key_info->shift)) {
        // this is where we handle the ALT-key combinations.  If we have a menu,
        // this is where we would check for the menu item.
        // ** NOTE: we don't catch ALT-KEY combinations yet.  We simple set the
        // **  focus to the main menu and allow the menu class to handle the key press.
        if (win->menu) {
          // this is were we find the first menu item, send it a "click"
          obj_focus(GUIOBJ(win->menu));
          win->menu->current->flags |= MENU_BTN_ACTIVE;
          obj_dirty(GUIOBJ(win->menu->current), TRUE);
        }
        // return so that the textedit object won't see the keypress
        return;
      }
      
      // call the current focused object's handler
      struct OBJECT *obj = obj_get_focus();
      if (obj && obj->_class) {
        eventstack.object = (struct DERIVED *) obj;
        obj->_class();
      }
      
      return;
    }
  }
  
  win_handler(GUIWIN(app));
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 2 (app)
 *  
 *
 */
struct APP2 *newapp2(void) {
  // Create the window for the app
  struct APP2 *app = (struct APP2 *) obj_win(NULL, DESKTOP, sizeof(struct APP2), handler2, 0, BORDER_NORMAL | BORDER_HAS_STATUS);
  struct MENU *menu;
  struct MENU_BUTTON *menu_button, *mb;
  struct BUTTON_BAR *button_bar;
  
  if (app) {
    // set the icon
    win_set_icon_id((struct WIN *) app, ID_ICON_TEXT);
    
    // The edit box
    obj_textedit(&app->edit, sizeof(struct TEXTEDIT), GUIOBJ(app), 0);
    textual_copy(&app->edit.body, 
      "This window has the following items:\n"
      " - a title bar with an App Icon, Title, and Min, Max,\n"
      "   and Exit buttons.\n"
      " - a Menu Bar with three main menu items, each capable\n"
      "   of having multiple sub menus and items.\n"
      " - a Button Bar for quick easy access to some Menu items.\n"
      " - this body text, with vertical and horizontal scroll\n"
      "   bars to the right and below.\n"
      " - a Status Bar below, with a status string, and a resize\n"
      "   button in the lower right corner.\n"
      "\n\n\n\n\n"
      );
    
    // The main menu object
    menu = obj_menu(NULL, 0, GUIOBJ(app), 0, NULL);
    
    // the first menu item: "File"
    menu_button = menu_append(menu, "&File", -1, MENU_BTN_NORMAL, 0);
      // a list of items in the "File" menu (another object)
      menu_button->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu);
      menu_append(menu_button->submenu, "&New", -1, MENU_BTN_NORMAL, ID_FILE_NEW);
      menu_append(menu_button->submenu, "&Open", -1, MENU_BTN_NORMAL, ID_FILE_OPEN);
      menu_append(menu_button->submenu, "Save &as...", -1, MENU_BTN_NORMAL, ID_FILE_SAVEAS);
      menu_append(menu_button->submenu, "&Save", -1, MENU_BTN_NORMAL, ID_FILE_SAVE);
      menu_append(menu_button->submenu, "", -1, MENU_BTN_SEPARATOR, 0);  // separator
      menu_append(menu_button->submenu, "Page Set&up", -1, MENU_BTN_NORMAL, ID_FILE_PAGE_SETUP);
      menu_append(menu_button->submenu, "&Print", -1, MENU_BTN_NORMAL, ID_FILE_PRINT);
      menu_append(menu_button->submenu, "", -1, MENU_BTN_SEPARATOR, 0);  // separator
      mb = menu_append(menu_button->submenu, "Re&cent Files", -1, MENU_BTN_NORMAL, ID_FILE_RECENT);
        mb->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu_button->submenu);
        menu_append(mb->submenu, "File &0", -1, MENU_BTN_NORMAL, ID_RECENT_FILE_0);
        menu_append(mb->submenu, "File &1", -1, MENU_BTN_NORMAL, ID_RECENT_FILE_1);
      menu_append(menu_button->submenu, "", -1, MENU_BTN_SEPARATOR, 0);  // separator
      menu_append(menu_button->submenu, "E&xit", -1, MENU_BTN_NORMAL, ID_FILE_EXIT);

    // the second menu item: "Edit"
    menu_button = menu_append(menu, "&Edit", -1, MENU_BTN_NORMAL, 0);
      menu_button->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu);
      menu_append(menu_button->submenu, "Cu&t", -1, MENU_BTN_NORMAL, ID_EDIT_CUT);
      menu_append(menu_button->submenu, "&Copy", -1, MENU_BTN_NORMAL, ID_EDIT_COPY);
      menu_append(menu_button->submenu, "&Paste", -1, MENU_BTN_DISABLED, ID_EDIT_PASTE);
      menu_append(menu_button->submenu, "&Delete", -1, MENU_BTN_NORMAL, ID_EDIT_DELETE);
      menu_append(menu_button->submenu, "Select &All", -1, MENU_BTN_NORMAL, ID_EDIT_SELECTALL);
      menu_append(menu_button->submenu, "", -1, MENU_BTN_SEPARATOR, 0);  // separator
      menu_append(menu_button->submenu, "&Find", -1, MENU_BTN_NORMAL, ID_EDIT_FIND);
      menu_append(menu_button->submenu, "&Replace", -1, MENU_BTN_NORMAL, ID_EDIT_REPLACE);
      menu_append(menu_button->submenu, "", -1, MENU_BTN_SEPARATOR, 0);  // separator
      menu_append(menu_button->submenu, "&Go To...", -1, MENU_BTN_DISABLED, ID_EDIT_GOTO);
     
    // the third menu item: "Help"
    menu_button = menu_append(menu, "&Help", -1, MENU_BTN_NORMAL, 0);
      menu_button->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu);
      menu_append(menu_button->submenu, "&Help", -1, MENU_BTN_NORMAL, ID_HELP_HELP);
      mb = menu_append(menu_button->submenu, "Si&de Menu", -1, MENU_BTN_NORMAL, 0);
        mb->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu_button->submenu);
        menu_append(mb->submenu, "Side Item &0", -1, MENU_BTN_NORMAL, 0);
        menu_append(mb->submenu, "Side Item &1", -1, MENU_BTN_NORMAL, ID_HELP_HELP);
      menu_append(menu_button->submenu, "&About", -1, MENU_BTN_NORMAL, ID_HELP_ABOUT);
      menu_append(menu_button->submenu, "Disa&bled Button", -1, MENU_BTN_DISABLED, 0);

    // Point our win's menu pointer to this menu
    ((struct WIN *) app)->menu = menu;
    
    // make the 'rightclick' menu
    // it is a child to 'menu' above, so insert it, but make it hidden
    // If we do not link it to the main menu, the menuing system will not
    //  be able to remove the menu once we click on it
    menu_button = menu_append(menu, "RightClick", -1, MENU_BTN_HIDDEN, 0);
      menu_button->submenu = obj_menu(NULL, 0, GUIOBJ(app), 0, menu);
      menu_append(menu_button->submenu, "&New", -1, MENU_BTN_NORMAL, ID_FILE_NEW);
      menu_append(menu_button->submenu, "&Open", -1, MENU_BTN_NORMAL, ID_FILE_OPEN);
      menu_append(menu_button->submenu, "Cu&t", -1, MENU_BTN_NORMAL, ID_EDIT_CUT);
      menu_append(menu_button->submenu, "E&xit", -1, MENU_BTN_NORMAL, ID_FILE_EXIT);
      app->rightclick = menu_button->submenu;
    
    // The button bar
    // You can give a pointer to a list of IDS, or you can append a button yourself,
    //  or do both as this example shows.  Make sure to end the list with a -1.
    // If you use a list, the buttons will default to a 'flags' value of MENU_BTN_NORMAL
    // You have to append it yourself to give it a different flags value.
    //
    // * Notice that we included ID_FILE_SAVEAS and ID_FILE_EXIT but we have no image 
    // *  that has that ID. The system will ignore these entries and move to the next one.
    //
    int button_bar_ids[] = { 
      ID_FILE_NEW,
      ID_FILE_OPEN,
      ID_FILE_SAVEAS,  // ignored because we didn't find one with this ID
      ID_FILE_SAVE,
      ID_FILE_PRINT,
      ID_FILE_EXIT,    // ignored because we didn't find one with this ID
      ID_BUTTON_BAR_DIV,  // divider bar flag
      ID_EDIT_CUT,
      ID_EDIT_COPY,
      ID_EDIT_PASTE,
      ID_BUTTON_BAR_DIV,  // divider bar flag
      ID_EDIT_FIND,
      ID_BUTTON_BAR_DIV,  // divider bar flag
      ID_HELP_HELP,
      -1                  // end of list
    };
    button_bar = obj_button_bar(NULL, 0, GUIOBJ(app), button_bar_ids, 0);
    button_bar_append(button_bar, MENU_BTN_NORMAL, ID_HELP_ABOUT);
    ((struct WIN *) app)->button_bar = button_bar;
    
    // set the status line text
    win_set_status(GUIWIN(app), "A status line would go here...");
  }
  
  return app;
}

