/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * textual.cpp
 *  
 */
#include <string.h>
#include <memory.h>

#include "gui.h"
#include "grfx.h"
#include "palette.h"
#include "mouse.h"

/*  gui_textsize()
 *   text = pointer to the textual object
 *      w = pointer to save the width to
 *      h = pointer to save the height to
 *
 *  get the width and height of a block of text
 *   
 */
void gui_textsize(struct TEXTUAL *text, int *w, int *h) {
  font_blocksize(text->font, text, w, h);
}

/*  textual_set()
 *       text = pointer to the textual object
 *        str = char string to set textual to
 *        len = length of buffer allocated, or length of fixed buffer sent
 *  allocated = if the str is allocated or not
 *   
 */
void textual_set(struct TEXTUAL *text, const char *str, const int len, const bool allocated) {
  string_set(&text->text, str, len, allocated);
  gui_textsize(text, &text->textwidth, &text->textheight);
  textual_update_caret(text);
}

/*  textual_copy()
 *       text = pointer to the textual object
 *        str = char string to copy to the textual
 * 
 *  copy a string to the textual
 */
void textual_copy(struct TEXTUAL *text, const char *str) {
  string_copy(&text->text, str);
  gui_textsize(text, &text->textwidth, &text->textheight);
  textual_update_caret(text);
}

/*  textual_copy()
 *       text = pointer to the textual object
 *       fore = foreground color of text
 *       back = background color of text
 *   
 *  set the text color
 */
void text_obj_color(struct TEXTUAL *text, PIXEL fore, PIXEL back) {
  text->fore_color = fore;
  text->back_color = back;
}

/*  textual_text()
 *       text = pointer to the textual object
 *        len = pointer to hold the returned len of string
 *   
 *   returns the string of text and its len
 */
const char *textual_text(const struct TEXTUAL *text, int *len) {
  return string_text(&text->text, len);
}

/*  textual_width()
 *       text = pointer to the textual object
 *   
 *   returns the width of the pixel data of the textual
 */
int textual_width(const struct TEXTUAL *text) {
  return text->textwidth;
}

/*  textual_height()
 *       text = pointer to the textual object
 *   
 *   returns the height of the pixel data of the textual
 */
int textual_height(const struct TEXTUAL *text) {
  return text->textheight;
}

/*  textual_align()
 *       text = pointer to the textual object
 *      align = is the alignment value to align the text to
 *   
 *   sets the alignment of the textual
 */
void textual_align(struct TEXTUAL *text, ALIGN align) {
  text->align = align;
}

/*  textual_align_get()
 *       text = pointer to the textual object
 *   
 *   returns the alignment of the textual
 */
ALIGN textual_align_get(const struct TEXTUAL *text) {
  return text->align;
}

/*  gui_textual_align()
 *       text = pointer to the textual object
 *       left = X1 coordinate relative to the textual
 *        top = Y1 coordinate relative to the textual
 *      right = X2 coordinate relative to the textual
 *     bottom = Y2 coordinate relative to the textual
 *   
 *   aligns the text within the rect given returning the top-left coordinate to start drawing
 */
void gui_textual_align(const struct TEXTUAL *textual, int *left, int *top, int right, int bottom) {
  if (textual->align & ALIGN_RIGHT)
    *left = right - textual->textwidth;
  
  else if (textual->align & ALIGN_HCENTER)
    *left += (right - *left) / 2 - (textual->textwidth) / 2;
  
  if (textual->align & ALIGN_BOTTOM)
    *top = bottom - textual->textheight;
  
  else if (textual->align & ALIGN_VCENTER)
    *top += (bottom - (*top)) / 2 - (textual->textheight) / 2;
}

/*  textual_set_font()
 *       text = pointer to the textual object
 *  font_name = name of font to set it to (name, not file name)
 *   
 * this will call the font code to search for a loaded font.
 *  if it is found, it will set it.
 *  if not found, it will return NULL
 * the system uses the default font when NULL is given
 */
void textual_set_font(struct TEXTUAL *text, const char *font_name) {
  if (font_name == NULL)
    text->font = NULL;
  else
    text->font = font_find_name(font_name);
  
  // now that the font has changed, we need to update the sizes and caret position
  gui_textsize(text, &text->textwidth, &text->textheight);
  textual_update_caret(text);
  obj_defaultrect(GUIOBJ(text), NULL);
}

/*  textual_set_flags()
 *       text = pointer to the textual object
 *      flags = flags to set to the textual
 *   
 * This makes sure that if setting to non READONLY, the string
 *  is allocated.  If it is not, this will not allow the call
 *  to remove the READONLY flag.
 * Therefore, all calls to this must be after the textual_set() call.
 */
void textual_set_flags(struct TEXTUAL *text, const bit32u flags) {
  
  bool border_change = ((text->flags & TEXTUAL_FLAGS_BORDER) ^ (flags & TEXTUAL_FLAGS_BORDER));
  
  text->flags = flags;
  
  // Should not have the textual writtable if the string is not allocated.
  if (!(flags & TEXTUAL_FLAGS_READONLY) && !text->text.allocated)
    text->flags |= TEXTUAL_FLAGS_READONLY;
  
  // if readonly, make sure the nocaret is set
  if (flags & TEXTUAL_FLAGS_READONLY)
    text->flags |= TEXTUAL_FLAGS_NOCARET;
  
  // set the color of the text
  if (text->flags & TEXTUAL_FLAGS_READONLY)
    text_obj_color(text, GUICOLOR_black, GUIRGB(192, 192, 192));
  else
    text_obj_color(text, GUICOLOR_black, GUICOLOR_white);
  
  // create the caret if we are not readonly
  if (flags & TEXTUAL_FLAGS_NOCARET) {
    if (text->caret.caret) {
      atom_delete(&text->caret.caret->base.atom);
      text->caret.caret = NULL;
    }
  } else {
    if (!text->caret.caret) {
      struct BITMAP *bitmap;
      int i;
      
      // Make the caret a child of the body so that all geometry is relative to it.
      text->caret.caret = obj_image(NULL, sizeof(struct IMAGE), GUIOBJ(text), 0);
      text->caret.height = MIN(text->textheight, 10);
      bitmap = obj_bitmap(2, text->caret.height, 2);  // w, height, 2 images
      text->caret.caret->bitmap = bitmap;
      
      // clear all of the image(s)
      for (i=0; i<(2 * text->caret.height) * bitmap->count; i++)
        bitmap->array[i] = GUICOLOR_transparent;
      // give the first image, the caret, a color of black
      for (i=0; i<(2 * text->caret.height); i++)
        bitmap->array[i] = GUICOLOR_black;
      for (i=0; i<bitmap->count; i++)
        bitmap->delay_array[i] = 8;  // default to 8 timer ticks (DJGPP's DOS timer tick is 91 ticks per second)
      bitmap->count_down = bitmap->delay_array[0];
      
      obj_position(GUIOBJ(text->caret.caret), 4, 5, 2, text->caret.height);
      obj_visible(GUIOBJ(text->caret.caret), FALSE);
      text->caret.rect.left = 0;
      text->caret.rect.top = 0;
    }
  }
  
  // if we changed the border flag, we need to re-defaultrect it.
  if (border_change)
    obj_defaultrect(GUIOBJ(text), NULL);
}

/*  textual_do()
 *     textual = pointer to the textual object
 *    key_info = key info for key pressed
 *   
 *   actually inserts the key pressed into the string of text
 */
void textual_do(struct TEXTUAL *textual, const struct KEY_INFO *key_info) {
  int len;
  const char *start = string_text(&textual->text, &len);
  const char *end = start + len;
  
  // only allow ascii printable characters
  if (((key_info->ascii >= 32) && (key_info->ascii <= 126)) ||
       (key_info->code == 0x1C0D)) {
    
    // don't allow an enter key if TEXTUAL_FLAGS_NORETURN set
    if ((textual->flags & TEXTUAL_FLAGS_NORETURN) && (key_info->code == 0x1C0D))
      return;
    
    // don't allow anything other than 0->9, '-', and '+' if TEXTUAL_FLAGS_NUMONLY set
    if ((textual->flags & TEXTUAL_FLAGS_NUMONLY) &&
      !(((key_info->ascii >= '0') && (key_info->ascii <= '9')) ||
         (key_info->ascii == '+') || (key_info->ascii == '-'))
      ) return;
    
    // standard ascii printable characters and the enter key
    string_insert(&textual->text, textual->cur_char_pos, key_info->ascii);
    textual->cur_char_pos++;
  
  // TAB key
  } else if (key_info->code == VK_TAB) {
    if (textual->flags & TEXTUAL_FLAGS_NUMONLY)
      return;
    // turn tabs into 2 spaces
    string_insert(&textual->text, textual->cur_char_pos, 0x20);
    string_insert(&textual->text, textual->cur_char_pos, 0x20);
    textual->cur_char_pos += 2;
  
  // back space key
  } else if (key_info->code == VK_BACKSPACE) {
    // backspace deletes char to the left
    if (textual->cur_char_pos > 0) {
      textual->cur_char_pos--;
      string_remove(&textual->text, textual->cur_char_pos);
    }
  
  // delete key
  } else if (key_info->code == VK_DELETE) {
    // del key deletes char to the right
    if (textual->cur_char_pos < strlen(string_text(&textual->text, NULL)))
      string_remove(&textual->text, textual->cur_char_pos);
  
  // right arrow
  } else if (key_info->code == VK_RTARROW) {
    if (textual->cur_char_pos < strlen(string_text(&textual->text, NULL)))
      textual->cur_char_pos++;    
  
  
  // left arrow
  } else if (key_info->code == VK_LFARROW) {
    if (textual->cur_char_pos > 0)
      textual->cur_char_pos--;
  
  // up arrow
  } else if (key_info->code == VK_UPARROW) {
    if (textual->flags & TEXTUAL_FLAGS_NUMONLY)
      return;
    // search backward until we find a NL, counting the characters before it
    // Then search backward again, until we find another NL.
    // Then advance forward until we count saved amount of chars, or find the NL.
    // If we find the start of the string before we find the first NL, stop and return.
    char *p = (char *) start + textual->cur_char_pos;
    int c = 0;
    while ((p > start) && (*(p-1) != 13)) {
      p--; c++;
    }
    if (p > start) {
      p--;
      while ((p > start) && (*(p-1) != 13))
        p--;
      while (c && (*p != 13)) {
        p++; c--;
      }
      textual->cur_char_pos = (p - start);
    }
  
  // down arrow
  } else if (key_info->code == VK_DNARROW) {
    if (textual->flags & TEXTUAL_FLAGS_NUMONLY)
      return;
    // similar as the up arrow above, except in the down direction
    // first going backward to find the distance from the start of the line
    char *temp, *p = (char *) start + textual->cur_char_pos;
    int c = 0;
    temp = p;
    while ((p > start) && (*(p-1) != 13)) {
      p--; c++;
    }
    p = temp + 1;
    while ((p < end) && (*(p-1) != 13))
      p++;
    if (p < end) {
      while (c && (*p != 13) && (p < end)) {
        p++; c--;
      }
      textual->cur_char_pos = (p - start);
    }
  
  // home key
  } else if (key_info->code == VK_HOME) {
    char *p = (char *) start + textual->cur_char_pos;
    while ((p > start) && (*(p-1) != 13))
      p--;
    textual->cur_char_pos = (p - start);
  
  // end key
  } else if (key_info->code == VK_END) {
    char *p = (char *) start + textual->cur_char_pos;
    while ((p < end) && (*p != 13))
      p++;
    textual->cur_char_pos = (p - start);
  
  } else {
    // ignore any other key.  After all, this is a simple word processor...
  }
}

/*  textual_get_pos()
 *     textual = pointer to the textual object
 *         pos = character position in string
 *        rect = area returned of character
 *      height = pointer to buffer to hold returned height
 *   
 *   returns the position (left, top, right, bottom) of the char at pos in string
 */
bool textual_get_pos(const struct TEXTUAL *textual, const int pos, struct RECT *rect, int *height) {
  int len;
  const char *str = textual_text(textual, &len);
  return font_bitmap_get_pos(textual->font, str, len, pos, rect, height);
}

/*  textual_update_caret()
 *     textual = pointer to the textual object
 *   
 *   updates the caret within the textual object
 */
void textual_update_caret(struct TEXTUAL *text) {
  
  if (text->flags & TEXTUAL_FLAGS_NOCARET)
    return;
  
  // calculate the caret cursor position
  int len = 0;
  textual_text(text, &len);
  if (len < text->cur_char_pos)
    text->cur_char_pos = len;
  
  if (text->caret.caret) {
    textual_get_pos(text, text->cur_char_pos, &text->caret.rect, NULL);
    obj_dirty(GUIOBJ(text->caret.caret), FALSE);  // once before...
    obj_position(GUIOBJ(text->caret.caret), text->caret.rect.left + 4, text->caret.rect.bottom - text->caret.height, 2, text->caret.height);
    obj_dirty(GUIOBJ(text->caret.caret), FALSE);  // once after
  }
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
void textual_class(void) {
  struct TEXTUAL *textual = (struct TEXTUAL *) eventstack.object;
  
  switch (eventstack.event) {
    // kill the textual
    case DESTRUCT:
      string_free(&textual->text);
      break;
      
    // draw the textual to the screen
    case EXPOSE:
      gui_textual(textual);
      return;
      
    // give the textual a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + textual->textwidth;
      rect->bottom = rect->top + textual->textheight;
      // add 6 pixels if we add a border
      if (textual->flags & TEXTUAL_FLAGS_BORDER) {
        rect->right += 4;
        rect->bottom += 4;
      }
    } return;
      
    // a key was entered
    case KEY:
      if (!(textual->flags & TEXTUAL_FLAGS_READONLY)) {
        // do the word processing, display it, mark it as dirty, and return.
        const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
        textual_do(textual, key_info);
        gui_textual(textual);
        obj_dirty(GUIOBJ(textual), FALSE);
        
        // calculate the caret cursor position
        textual_update_caret(textual);
      }
      return;
      
    // hovering so change to text mouse cursor
    case POINTER_ENTER:
      textual->hovering = TRUE;
      obj_dirty(GUIOBJ(textual), FALSE);
      if (textual->flags & TEXTUAL_FLAGS_ISLINK) {
        set_pointer(POINTER_ID_HAND);
        return;
      } else if (!(textual->flags & TEXTUAL_FLAGS_READONLY)) {
        set_pointer(POINTER_ID_TEXT);
        return;
      }
      break;
      
    // left hovering so change back to last mouse cursor
    case POINTER_LEAVE:
      textual->hovering = FALSE;
      obj_dirty(GUIOBJ(textual), FALSE);
      if ((!(textual->flags & TEXTUAL_FLAGS_READONLY)) ||
            (textual->flags & TEXTUAL_FLAGS_ISLINK)) {
        restore_pointer();
        return;
      }
      break;
      
    // allow it to gain focus if it isn't a readonly textual
    case FOCUS_WANT:
      // if this window allows the focus to be set to it, return TRUE;
      if (!(textual->flags & TEXTUAL_FLAGS_READONLY)) {
        answer(textual);
        return;
      }
      break;
    
    // turn off the caret if we lost focus
    case FOCUS_LOST:
      if (!(textual->flags & TEXTUAL_FLAGS_NOCARET)) {
        // send the parent a notification.
        if (GUIOBJ(textual)->parent)
          obj_event(GUIOBJ(textual)->parent, FOCUS_LOST, textual);
        obj_visible(GUIOBJ(textual->caret.caret), FALSE);
        obj_dirty(GUIOBJ(textual), TRUE);
      }
      break;
      
    // turn on the caret if we gain focus
    case FOCUS_GOT:
      if (!(textual->flags & TEXTUAL_FLAGS_NOCARET)) {
        obj_visible(GUIOBJ(textual->caret.caret), TRUE);
        obj_dirty(GUIOBJ(textual), TRUE);
      }
      break;
  }
  
  // TODO:
  // since the scroll only moves things within the parent node, it doesn't scroll the children of that parent
  // since the caret is a child of the body, we need to write the scroll code to include children,
  if (!(textual->flags & TEXTUAL_FLAGS_NOCARET) && textual->caret.caret)
    obj_position(GUIOBJ(textual->caret.caret), textual->caret.rect.left + 4, textual->caret.rect.bottom - textual->caret.height, 2, textual->caret.height);
  
  // if the event was not handled above, let the object class do it
  obj_class();
}

/*  obj_textual()
 *         text = pointer to the textual object
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the scroll (usually ignored)
 *
 *  creates a textual object
 *   
 */
struct TEXTUAL *obj_textual(struct TEXTUAL *text, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct TEXTUAL);
  
  text = (struct TEXTUAL *) object(GUIOBJ(text), textual_class, size, parent, theid);
  if (text) {
    string(&text->text);
    textual_set(text, NULL, -1, FALSE);
    text_obj_color(text, GUICOLOR_black, GUICOLOR_transparent);
    text->flags = (TEXTUAL_FLAGS_READONLY | TEXTUAL_FLAGS_NOCARET);
    text->hovering = FALSE;
    text->font = NULL; // assume default (system) font.
    text->decorator = NULL;
    text->caret.caret = NULL;
    text->cur_char_pos = 0;
  }
  
  // return the textual object
  return text;
}

/*  obj_static_text()
 *         text = pointer to the textual object
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the scroll (usually ignored)
 *
 *  This simply creates a readonly textual object
 *   
 */
struct TEXTUAL *obj_static_text(struct TEXTUAL *text, bit32u size, struct OBJECT *parent, int theid, const char *str) {
  MINSIZE(size, struct TEXTUAL);
  
  text = obj_textual(text, size, parent, theid);
  if (text) {
    obj_armable(GUIOBJ(text), FALSE);
    obj_selectable(GUIOBJ(text), FALSE);    
    textual_set(text, str, -1, FALSE);
    //obj_defaultrect(GUIOBJ(text), NULL);  // set_font below does this for us
    textual_align(text, ALIGN_LEFT);
    textual_set_font(text, "Simple");  // if we don't set the font here, uncomment obj_defaultrect() above
  }
  
  // return the textual object
  return text;
}

/*  obj_edit_text()
 *     textedit = pointer to the textedit object
 *         size = size of memory to allocate
 *       parent = parent object (usually the root)
 *        theid = id of the scroll (usually ignored)
 *
 *  This creates a one-line text edit
 *   
 */
struct TEXTUAL *obj_edit_text(struct TEXTUAL *text, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct TEXTUAL);
  
  text = obj_textual(text, size, parent, theid);
  if (text) {
    textual_set(text, (const char *) calloc(1024, 1), 1024, TRUE);
    textual_set_flags(text, TEXTUAL_FLAGS_BORDER | TEXTUAL_FLAGS_NORETURN);
    obj_resize(GUIOBJ(text), 125, text->textheight + 4);
    textual_align(text, ALIGN_LEFT);
  }
  
  // return the textual object
  return text;
}
