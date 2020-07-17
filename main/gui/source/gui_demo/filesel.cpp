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
 *  filesel.cpp
 *  
 *  This file is a file select box.  It is another example of what you can create
 *   for your GUI.  It is not part of the GUI system, and is not needed to function.
 *   it is just an example of what you can do.
 *
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>

#include <dir.h>
#include <mntent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/ctype.h"
#include "gui.h"
#include "filesel.h"


#define FA_DRIVE        64  // *not* <dir.h> standard.  We use it here to indicate a drive was specified.


/*
   We have a running string array of the last LAST_DEPTH directories
   This array is a revolving string array with two pointers:
    last_cur  -> current string array index where we hold the last saved entry
    last_last -> the last (top most) empty string array index
   We increment in the positive direction.
   For example, let's say we have 10 strings in our array
  
     [0] [1] [2] [3] [4] [5] [6] [7] [8] [9]
              |               |
           last_cur         last_last
  
   This shows that our current string ([2]) would be the next
    string we return when the 'back' button is pressed.
   Then we decrement to [1].
   All strings from this point [1,0,9,8,7] are valid and are
    in consecutive order, with the string in [1] being the next
    most recent previous directory.
   When a string is inserted, meaning we have changed to a new
    directory, we would insert it into [2], since [2,3,4,5] are 
    now currently unused.
   If we ever change the directory and last_cur and last_last
    point to the same position, this means that either we have
    not yet inserted a string, or all 10 strings are being used.
   Either way, we need to insert the given string. 
  
     [0] [1] [2] [3] [4] [5] [6] [7] [8] [9]
                      |
                  last_cur
                  last_last
   
   In the example above, insert the string at position [5] and
    point last_cur at [5].

   When we are retrieving a string, meaning the 'back' button was
    pressed, we check that last_cur != last_last.  If they do not equal,
    return the string at last_cur, then decrement last_cur, rolling
    over to [9] if we go lower than [0].
    
   Also, if we ever need to 'push' a string onto the stack, and
    now last_cur == last_last, we simply increment both, and place the
    string at the new position.  This will remove the oldest, deepest
    saved directory string.


  *** Note, this needs a lot of work, but I have added the current
       capability to show you how it can be done.  This code is to
       show you how to display a GUI, not read and write from disks.

 */

/* filesel_push_back()
 *       sel = file select object
 *   current = string of current path (or filename)
 *
 *  pushes a string on to our stack
 *   
 */
void filesel_push_back(struct FILESEL *sel, const char *current) {
  sel->last_cur++;
  if (sel->last_cur == LAST_DEPTH)
    sel->last_cur = 0;
  if (sel->last_cur == sel->last_last) {
    sel->last_last++;
    if (sel->last_last == LAST_DEPTH)
      sel->last_last = 0;
  }
  strcpy(sel->last[sel->last_cur], current);  
}

/* filesel_pop_back()
 *       sel = file select object
 *       str = pointer to memory to fill with popped string
 *
 *  try to retrieve a string from our stack
 *   
 */
bool filesel_pop_back(struct FILESEL *sel, char *str) {
  if (sel->last_cur != sel->last_last) {
    strcpy(str, sel->last[sel->last_cur]);
    sel->last_cur--;
    if (sel->last_cur == -1)
      sel->last_cur = LAST_DEPTH - 1;
    return TRUE;
  } else  
    return FALSE;
}

/* filesel_can_pop()
 *       sel = file select object
 *
 *  returns TRUE if we have an item on the stack
 *   
 */
bool filesel_can_pop(const struct FILESEL *sel) {
  return (sel->last_cur != sel->last_last);
}

/* filesel_geometry()
 *       sel = file select object
 *
 *  used to set the size of each item in the window
 *   so that when the user resizes the window (if allowed),
 *    everything is changed accordingly.
 *   
 */
void filesel_geometry(struct FILESEL *sel) {
  int i;
  
  obj_defaultrect(GUIOBJ(&sel->dir), NULL);
  obj_resize(GUIOBJ(&sel->dir), (int) (gui_w(sel) * 0.60), GUIDEF);
  obj_layout(GUIOBJ(&sel->dir), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(sel), 30, 4);
  
  obj_layout(GUIOBJ(&sel->back), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_Y1), GUIOBJ(&sel->dir), 10, 2);
  obj_layout(GUIOBJ(&sel->up), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_Y1), GUIOBJ(&sel->back), 5, 0);
  
  obj_defaultrect(GUIOBJ(&sel->cancel), 0);
  obj_defaultrect(GUIOBJ(&sel->moreless), 0);
  obj_defaultrect(GUIOBJ(&sel->open), 0);
  const int bw = MAX(gui_w(&sel->cancel), gui_w(&sel->open));
  obj_resize(GUIOBJ(&sel->cancel), bw, GUIDEF);
  obj_resize(GUIOBJ(&sel->open), bw, GUIDEF);
  obj_resize(GUIOBJ(&sel->moreless), bw, GUIDEF);
  obj_layout(GUIOBJ(&sel->cancel), (LAYOUT) (LAYOUT_X2 | LAYOUT_Y2), GUIOBJ(sel), -2, -2);
  obj_layout(GUIOBJ(&sel->moreless), (LAYOUT) (LAYOUT_LEFT | LAYOUT_Y1), GUIOBJ(&sel->cancel), 6, 0);
  obj_layout(GUIOBJ(&sel->open), (LAYOUT) (LAYOUT_X2 | LAYOUT_TOP), GUIOBJ(&sel->cancel), 0, 4);
  
  obj_defaultrect(GUIOBJ(&sel->file), 0);
  obj_position(GUIOBJ(&sel->file), 3, obj_y(GUIOBJ(&sel->open)), gui_w(sel) - gui_w(&sel->open) - 10, gui_h(&sel->open));
  
  if (sel->more)
    obj_position(GUIOBJ(&sel->list), 3, obj_y(GUIOBJ(&sel->dir)) + gui_h(&sel->dir) + 4, gui_w(sel) - 7, 
      obj_y(GUIOBJ(&sel->file)) - obj_y(GUIOBJ(&sel->dir)) - gui_h(&sel->file) - 12 - sel->more_add);
  else
    obj_position(GUIOBJ(&sel->list), 3, obj_y(GUIOBJ(&sel->dir)) + gui_h(&sel->dir) + 4, gui_w(sel) - 7, 
      obj_y(GUIOBJ(&sel->file)) - obj_y(GUIOBJ(&sel->dir)) - gui_h(&sel->file) - 12);
  
  for (i=0; i<3; i++) {
    obj_defaultrect(GUIOBJ(&sel->radio[i]), NULL);
    if (i==0)
      obj_layout(GUIOBJ(&sel->radio[0]), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&sel->list), 50, 5);
    else
      obj_layout(GUIOBJ(&sel->radio[i]), (LAYOUT) (LAYOUT_X1 | LAYOUT_BOTTOM), GUIOBJ(&sel->radio[i-1]), 0, 2);
  }
  obj_defaultrect(GUIOBJ(&sel->check_box), NULL);
  obj_layout(GUIOBJ(&sel->check_box), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_Y1), GUIOBJ(&sel->radio[0]), 2, 0);
  
  list_scroll_geometry(&sel->list);
}

// default sizes to icons in list
#define TYPE_ID_WIDTH   16
#define TYPE_ID_HEIGHT  14

// default ID's for icons in list
struct ASSOCIATE {
  char ext[6];
  int  id;
} associate[] = {
  { "asm",  ID_TYPE_ASM },
  { "bat",  ID_TYPE_BAT },
  { "c",    ID_TYPE_C   },
  { "com",  ID_TYPE_COM },
  { "cpp",  ID_TYPE_CPP },
  { "exe",  ID_TYPE_EXE },
  { "fnt",  ID_TYPE_FNT },
  { "h",    ID_TYPE_H   },
  { "sys",  ID_TYPE_SYS },
  { "txt",  ID_TYPE_TXT },
  { "bmp",  ID_TYPE_IMG },
  { "pcx",  ID_TYPE_IMG },
  { "png",  ID_TYPE_IMG },
  { "gif",  ID_TYPE_IMG },
  { "jpg",  ID_TYPE_IMG },
  { "jpeg", ID_TYPE_IMG },
  { "zip",  ID_TYPE_ZIP },
  
  { "", -1 }
};

/* filesle_get_icon()
 *      name = string of filename currently selected
 *
 *  this function will return an icon for a given extension of the file given
 *   
 */
struct BITMAP *filesle_get_icon(const char *name) {
  struct BITMAP *bitmap;
  int i = 0, id = ID_TYPE_UNKNOWN;
  char *t = (char *) name;
  char *p = NULL;
  
  // find the last '.'
  while (t = strchr(t, '.'))
    p = t++;
  
  // get extension and search list above
  if (p) {
    p++;  // skip over '.'
    while (associate[i].id > -1) {
      if (!stricmp(p, associate[i].ext)) {
        id = associate[i].id;
        break;
      }
      i++;
    }
  }
  
  // get the icon
  bitmap = get_static_bitmap(id, TYPE_ID_WIDTH, TYPE_ID_HEIGHT);

  // if none found, use standard unknown (white box)
  if (!bitmap)
    bitmap = get_static_bitmap(ID_TYPE_UNKNOWN, TYPE_ID_WIDTH, TYPE_ID_HEIGHT);
  
  // return the icon
  return bitmap;
}

/* filesel_refresh()
 *       sel = file select object
 *
 *  this function refreshes the list with the current directory
 *   
 */
void filesel_refresh(struct FILESEL *sel) {
  list_empty(&sel->list);
  
  // http://www.delorie.com/djgpp/doc/libc/libc_326.html
  struct ffblk f, *ff;
  char search_str[PATH_MAX];
  int done;
  
  strcpy(search_str, sel->current);
  strcat(search_str, "\\*.*");
  
  // first, get all drive letters
  // NOTE: This is a DJGPP/DOS specific call.  Your OS will need to retrieve the available drive letters.
  // http://www.delorie.com/djgpp/doc/libc/libc_405.html
  FILE *mntentptr = setmntent(NULL, NULL);
  struct mntent *fsdetails;
  while (fsdetails = getmntent(mntentptr)) {
    ff = (struct ffblk *) calloc(sizeof(struct ffblk), 1);
    strncpy(ff->ff_name, fsdetails->mnt_dir, 2);
    ff->ff_name[2] = '\0';
    ff->ff_attrib = FA_DRIVE;
    if (!strcmp(fsdetails->mnt_type, "hd")) {
      // we don't use the first part of ff and we need an "allocated" memory block, so use it to store the name
      sprintf((char *) ff, "%s (%s)", fsdetails->mnt_fsname, ff->ff_name);
      list_append(&sel->list, (char *) ff, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_DRIVE, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
    } else if (!strcmp(fsdetails->mnt_type, "fd")) {
      sprintf((char *) ff, "floppy (%s)", ff->ff_name);
      list_append(&sel->list, (char *) ff, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_FLOPPY, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
    } else if (!strcmp(fsdetails->mnt_type, "cdrom")) {
      list_append(&sel->list, ff->ff_name, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_CDROM, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
    } else if (!strcmp(fsdetails->mnt_type, "ram")) {
      list_append(&sel->list, ff->ff_name, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_RAM, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
    } else {
      list_append(&sel->list, ff->ff_name, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_UNKNOWN, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
    }
  }
  
  // next, get all directories
  done = findfirst(search_str, &f, FA_DIREC);
  while (!done) {
    if (f.ff_attrib & FA_DIREC) {
      // we skip the . .. entries
      if (strcmp(f.ff_name, ".") && strcmp(f.ff_name, "..")) {
        // we need a saved (allocated) block stored for the list.
        // 'allocated' = FALSE for the string since the destroy_list will free 'ff' for us.
        ff = (struct ffblk *) malloc(sizeof(struct ffblk));
        memcpy(ff, &f, sizeof(struct ffblk));
        list_append(&sel->list, ff->ff_name, -1, FALSE, 0, ff, get_static_bitmap(ID_TYPE_DIR, TYPE_ID_WIDTH, TYPE_ID_HEIGHT));
      }
    }
    done = findnext(&f);
  }
  
  // next, get all files
  done = findfirst(search_str, &f, FA_HIDDEN | FA_SYSTEM);
  while (!done) {
    // we need a saved (allocated) block stored for the list.
      // 'allocated' = FALSE for the string since the destroy_list will free 'ff' for us.
    ff = (struct ffblk *) malloc(sizeof(struct ffblk));
    memcpy(ff, &f, sizeof(struct ffblk));
    list_append(&sel->list, ff->ff_name, -1, FALSE, 0, ff, filesle_get_icon(ff->ff_name));
    done = findnext(&f);
  }
}

/* filesel_handler()
 *       win = pointer to parent window
 *
 *   called by GUI system when an Window event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to window
 *   eventstack.data = rect of win object
 */
void filesel_handler(struct WIN *win) {
  struct FILESEL *sel = (struct FILESEL *) win;
  
  switch (eventstack.event) {
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      // TODO: This needs to be conditional on the object being selected
      answer(sel);
      return;
    
    case FOCUS_LOST:
      // do we need to do something if the focus is lost?
      break;
      
    case FOCUS_GOT:
      // do we need to do something if the focus is got?
      break;
      
    case GEOMETRY:
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        win_handler(win);
        filesel_geometry(sel);
        return;
      }
      break;
      
    // set a default size for the object
    case DEFAULTRECT:
      if (GUIOBJ(eventstack.object) == GUIOBJ(win)) {
        struct RECT *rect = defaultrect_data();
        rect->right = rect->left + 320;
        rect->bottom = rect->top + 200;
        return;
      }
      break;
      
    // list changed so update
    case LIST_CHANGED:
      // Has a file been clicked, then enable/disable Open button
      if (GUIOBJ(eventstack.object) == GUIOBJ(&sel->list)) {
        if (list_selected(&sel->list, NULL)) {
          // If selected is a dir, then make sure button is "open"
          struct LISTELEM *selected = list_selected(&sel->list, NULL);
          struct ffblk *f = (struct ffblk *) selected->attachment;
          if (!sel->type)
            if (f->ff_attrib & FA_DIREC)
              textual_set(GUITEXTUAL(&sel->open), "&Open", -1, FALSE);
            else
              textual_set(GUITEXTUAL(&sel->open), "Save &as...", -1, FALSE);
          obj_disable(GUIOBJ(&sel->open), FALSE);
          textual_set(&sel->file, f->ff_name, -1, FALSE);
          obj_dirty(GUIOBJ(&sel->file), FALSE);
        } else {
          if (sel->type)
            textual_set(GUITEXTUAL(&sel->open), "&Open", -1, FALSE);
          else
            textual_set(GUITEXTUAL(&sel->open), "Save &as...", -1, FALSE);
          obj_disable(GUIOBJ(&sel->open), TRUE);
          textual_set(&sel->file, "", -1, FALSE);
          obj_dirty(GUIOBJ(&sel->file), FALSE);
        }
        obj_dirty(GUIOBJ(&sel->open), TRUE);
        
        obj_disable(GUIOBJ(&sel->back), !filesel_can_pop(sel));
        obj_dirty(GUIOBJ(&sel->back), TRUE);
      }
      break;
      
    // an object was selected
    case SELECT:
      // Button open selected?
      if (GUIOBJ(eventstack.object) == GUIOBJ(&sel->open)) {
        // Put the selected filename in the current place
        struct LISTELEM *selected = list_selected(&sel->list, NULL);
        if (selected) {
          const char *text = textual_text(GUITEXTUAL(selected), NULL);
          
          // save the current for the back button
          filesel_push_back(sel, sel->current);
          
          // if it is a directory, then we need to move to that directory
          //  and do it again.
          struct ffblk *f = (struct ffblk *) selected->attachment;
          if (f->ff_attrib & FA_DRIVE) {
            strcpy(sel->current, f->ff_name);
            strcat(sel->current, "\\");
            
            obj_arm(GUIOBJ(&sel->open), FALSE);
            obj_dirty(GUIOBJ(&sel->open), TRUE);
            obj_disable(GUIOBJ(&sel->open), TRUE);
            filesel_refresh(sel);
            filesel_geometry(sel);
            textual_set(&sel->dir, sel->current, -1, FALSE);
            obj_dirty(GUIOBJ(&sel->file), TRUE);
            obj_dirty(GUIOBJ(&sel->list), TRUE);
            obj_dirty(GUIOBJ(&sel->dir), TRUE);
            return; // Dont let the button toggle
          } else if (f->ff_attrib & FA_DIREC) {
            strcat(sel->current, "\\");
            strcat(sel->current, text);
            
            // we need to "fix" the pathname, combining ".."'s etc.
            char temp[PATH_MAX];
            strcpy(temp, sel->current);
            _fixpath(temp, sel->current);
            
            obj_arm(GUIOBJ(&sel->open), FALSE);
            obj_dirty(GUIOBJ(&sel->open), TRUE);
            obj_disable(GUIOBJ(&sel->open), TRUE);
            filesel_refresh(sel);
            filesel_geometry(sel);
            textual_set(&sel->dir, sel->current, -1, FALSE);
            obj_dirty(GUIOBJ(&sel->file), TRUE);
            obj_dirty(GUIOBJ(&sel->list), TRUE);
            obj_dirty(GUIOBJ(&sel->dir), TRUE);
            return; // Dont let the button toggle
          }
          
          //if (text && len) {
            //char *end = (char *) basename(sel->current);
            //if (len < 0) {
            //  strcpy(end, text);
            //} else {
            //  strncpy(end, text, len);
            //  end[len] = 0;
            //}
          //}
        }
        
        //inform(FILESEL_OK, sel->current);
        //atom_delete(&sel->base.atom);
        //return;
      } else
      
      // Button UP selected?
      if (GUIOBJ(eventstack.object) == GUIOBJ(&sel->up)) {
        // save the current for the back button
        filesel_push_back(sel, sel->current);
        
        // add \.. to the end and refresh
        strcat(sel->current, "\\");
        strcat(sel->current, "..");
        
        // we need to "fix" the pathname, combining ".."'s etc.
        char temp[PATH_MAX];
        strcpy(temp, sel->current);
        _fixpath(temp, sel->current);
        
        filesel_refresh(sel);
        filesel_geometry(sel);
        textual_set(&sel->dir, sel->current, -1, FALSE);
        obj_dirty(GUIOBJ(&sel->file), TRUE);
        obj_dirty(GUIOBJ(&sel->list), TRUE);
        obj_dirty(GUIOBJ(&sel->dir), TRUE);
        return; // Dont let the button toggle
      } else
      
      // Button BACK selected
      if (GUIOBJ(eventstack.object) == GUIOBJ(&sel->back)) {
        // get the last directory, if available
        if (filesel_pop_back(sel, sel->current)) {
          filesel_refresh(sel);
          filesel_geometry(sel);
          textual_set(&sel->dir, sel->current, -1, FALSE);
          obj_dirty(GUIOBJ(&sel->file), TRUE);
          obj_dirty(GUIOBJ(&sel->list), TRUE);
          obj_dirty(GUIOBJ(&sel->dir), TRUE);
        }
        return; // Dont let the button toggle
      } else

      // more/less button pressed
      if (GUIOBJ(eventstack.object) == GUIOBJ(&sel->moreless)) {
        int i;
        sel->more ^= 1;
        win_dirty(GUIWIN(sel));
        if (sel->more) {
          obj_resize(GUIOBJ(GUIWIN(sel)), GUIDEF, gui_h(sel) + sel->more_add);
          textual_copy(GUITEXTUAL(&sel->moreless), "&Less");
        } else {
          obj_resize(GUIOBJ(GUIWIN(sel)), GUIDEF, gui_h(sel) - sel->more_add);
          textual_copy(GUITEXTUAL(&sel->moreless), "&More");
        }
        for (i=0; i<3; i++) {
          obj_visible(GUIOBJ(&sel->radio[i]), sel->more);
          obj_disable(GUIOBJ(&sel->radio[i]), !sel->more);
        }
        obj_visible(GUIOBJ(&sel->check_box), sel->more);
        obj_disable(GUIOBJ(&sel->check_box), !sel->more);
        obj_geometry(GUIOBJ(GUIWIN(sel)));
        win_dirty(GUIWIN(sel));
        return; // Dont let the button toggle
      }
      
      listelem_class();
      return;
      
    // we allow the up and down arrows to move the selection
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      switch (key_info->code) {
        case VK_UPARROW:
        case VK_DNARROW:
          eventstack.object = (struct DERIVED *) GUIOBJ(&sel->list);
          list_class();
          return;
      }
    } break;
  }
  
  // if event not handled here, call the window handler
  win_handler(win);
}

/* fileselwin()
 *       sel = pointer to select window object
 *           NULL = allocate the memory here
 *         ! NULL = memory is already allocated/static
 *   parent_win = parent window of this new file select box
 *         size = size of memory to allocate
 *      handler = windows handler
 *        theid = id value to return to object when set
 *         open = 0 = this is a saveas dialog
 *                1 = this is an open dialog
 *
 *   creates a file select window
 *
 */
struct FILESEL *fileselwin(struct FILESEL *sel, struct WIN *parent_win, bit32u size, HANDLER handler, const int theid, const bool open) {
  MINSIZE(size, struct FILESEL);
  int i;
  char *text;
  
  // if no handler given, use our handler we code above
  if (!handler)
    handler = filesel_handler;
  
  // create the window object
  sel = (struct FILESEL *) obj_win(GUIWIN(sel), parent_win, size, handler, theid, BORDER_NORMAL & ~BORDER_HAS_RESIZE);
  if (sel) {
    sel->type = open;
    
    // create up button using a user drawn icon
    obj_button(&sel->up, 0, GUIOBJ(sel), 0, 0);
    button_ownerdraw(&sel->up, ID_ICON_ARROW_UP, 16, 16);
    
    // create back button using a user drawn icon
    obj_button(&sel->back, 0, GUIOBJ(sel), 0, 0);
    button_ownerdraw(&sel->back, ID_ICON_ARROW_LEFT, 16, 16);
    // initially disabled
    obj_disable(GUIOBJ(&sel->back), TRUE);
    
    // create open/save as button
    obj_button(&sel->open, 0, GUIOBJ(sel), 0, BUTTON_DEFAULT);
    if (open)
      textual_set(GUITEXTUAL(&sel->open), "&Open", -1, FALSE);
    else
      textual_set(GUITEXTUAL(&sel->open), "Save &as...", -1, FALSE);
    textual_align(GUITEXTUAL(&sel->open), ALIGN_CENTER);
    obj_disable(GUIOBJ(&sel->open), TRUE);
    
    // create cancel button
    obj_button(&sel->cancel, 0, GUIOBJ(sel), 0, 0);
    textual_set(GUITEXTUAL(&sel->cancel), "Cancel", -1, FALSE);
    textual_align(GUITEXTUAL(&sel->cancel), ALIGN_CENTER);

    // create moreless button
    obj_button(&sel->moreless, 0, GUIOBJ(sel), ID_BUTTON_MORE, 0);
    text = (char *) malloc(64);
    strcpy(text, "More");
    textual_set(GUITEXTUAL(&sel->moreless), text, -1, TRUE);
    textual_align(GUITEXTUAL(&sel->moreless), ALIGN_CENTER);
    sel->more = FALSE;
    
    // create the three (more) radio buttons
    sel->more_add = 0;
    for (i=0; i<3; i++) {
      obj_radio(&sel->radio[i], 0, GUIOBJ(sel), 12345 + i); // an ID to use, as long as its different than the rest
      text = (char *) malloc(64);
      sprintf(text, "Radio Num: %i", i);
      textual_set(GUITEXTUAL(&sel->radio[i]), text, -1, TRUE);
      textual_set_font(GUITEXTUAL(&sel->radio[i]), "Courier New");
      obj_defaultrect(GUIOBJ(&sel->radio[i]), 0);
      sel->more_add += gui_h(&sel->radio[i]) + 1;
      textual_align(GUITEXTUAL(&sel->radio[i]), ALIGN_LEFT);
      sel->radio[i].prev = (i > 0) ? &sel->radio[i-1] : NULL;
      sel->radio[i].next = (i < (3-1)) ? &sel->radio[i+1] : NULL;
      obj_visible(GUIOBJ(&sel->radio[i]), FALSE);
      obj_disable(GUIOBJ(&sel->radio[i]), TRUE);
    }
    
    // create the (more) checkbox
    obj_checkbox(&sel->check_box, 0, GUIOBJ(sel), 12344);  // an ID to use, as long as its different than the rest
    text = (char *) malloc(64);
    strcpy(text, "Toggle");
    textual_set(GUITEXTUAL(&sel->check_box), text, -1, TRUE);
    textual_set_font(GUITEXTUAL(&sel->check_box), "Courier New");
    obj_defaultrect(GUIOBJ(&sel->check_box), 0);
    textual_align(GUITEXTUAL(&sel->check_box), ALIGN_LEFT);
    obj_visible(GUIOBJ(&sel->check_box), FALSE);
    obj_disable(GUIOBJ(&sel->check_box), TRUE);
    
    // create the list
    obj_list(&sel->list, 0, GUIOBJ(sel), 0, 0);
    textual_set(GUITEXTUAL(&sel->list), "list", -1, FALSE);
    
    // create file text box
    obj_textual(&sel->file, 0, GUIOBJ(sel), 0);
    textual_set_flags(&sel->file, TEXTUAL_FLAGS_BORDER);
    
    // create directory text box
    obj_textual(&sel->dir, 0, GUIOBJ(sel), 0);
    textual_set_flags(&sel->dir, TEXTUAL_FLAGS_BORDER | TEXTUAL_FLAGS_NOCARET);
    
    // clear out our stack
    for (int i=0; i<LAST_DEPTH; i++)
      strcpy(sel->last[i], "");
    sel->last_cur = sel->last_last = 0;
    
    // get current directory and start with file in it
    if (getcwd(sel->current, MAX_PF_LEN) == NULL)
      strcpy(sel->current, "\\");
    textual_set(&sel->dir, sel->current, -1, FALSE);
    filesel_refresh(sel);
  }
  
  // return the window object
  return sel;
}
