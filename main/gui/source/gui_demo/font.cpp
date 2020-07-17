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
 *  font.cpp
 *  
 *  This is used to create, find, load, and draw fonts
 *
 *  Last updated: 17 July 2020
 */

#include <dirent.h>

#include "../include/ctype.h"
#include "string.h"
#include "memory.h"

#include "gui.h"
#include "grfx.h"
#include "palette.h"

struct FONT *last_font = NULL;
struct FONT *current_font = NULL;

/* font_default()
 *      font = font to use
 *  
 *  sets the default font to font given
 */
void font_default(struct FONT *font) {
  if (font)
    current_font = font;
}

/* font_load()
 *          name = filename of the font to load
 *   set_default = 0 = do not set as default
 *                 1 = set as default
 *  
 *  tries to load the font given by name
 */
struct FONT *font_load(const char *name, const bool set_default) {
  struct FONT temp, *font = NULL;
  FILE *fp;
  int sz;
  char filename[512];
  
  // prefix default path to name
  prefix_default_path(filename, name);
  
  // try to open the file
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    DEBUG((dfp, "\n Error loading '%s'.", filename));
    return NULL;
  }
  
  // read in the first part of the font
  if (fread(&temp, 1, sizeof(struct FONT), fp) != sizeof(struct FONT)) {
    fclose(fp);
    DEBUG((dfp, "\n Error loading font structure..."));
    return NULL;
  }
  
  // check that the signature is correct
  if (memcmp(temp.sig, "FONT", 4)) {
    fclose(fp);
    DEBUG((dfp, "\n Found invalid Signature in Font file..."));
    return NULL;
  }
  
  // allocate the memory for the font
  sz = sizeof(struct FONT) + (sizeof(struct FONT_INFO) * temp.count) + temp.datalen;
  font = (struct FONT *) calloc(sz, 1);
  if (font == NULL) {
    fclose(fp);
    DEBUG((dfp, "\n Error allocating font..."));
    return NULL;
  }
  
  // read in the whole font
  rewind(fp);
  if (fread(font, 1, sz, fp) != sz) {
    fclose(fp);
    DEBUG((dfp, "\n Error loading font..."));
    free(font);
    return NULL;
  }

  // close the handle
  fclose(fp);
  
  // set the drawing routine for this font
  font->drawchar = font_bitchar;
  
  // insert the font into our font list
  font_insert(font);
  
  // set as default if specified to do so
  if (set_default)
    font_default(font);
  
  // return a pointer to this font
  return font;
}

/* font_find_name()
 *          name = name of the font (font name, not filename)
 *  
 *  tries to find a font by first looking through all loaded fonts,
 *   then by loading all files with the .fnt extension from the given 
 *   path, searching the font file for the name, then will load that font
 *
 */
const struct FONT *font_find_name(const char *name) {
  struct FONT *font = last_font;
  struct dirent *pDirent;
  struct FONT temp;
  char filename[512];
  
  // first see if it is a font we already have loaded
  while (font) {
    if (!stricmp(font->name, name))
      return font;
    font = font->prev;
  }
  
  // Did not find the font within our already loaded fonts.
  // Search in the system/font directory for the file.
  // If found, load and return its pointer.
  DIR *pDir = opendir(default_path);
  if (pDir == NULL)
    return NULL;
  
  while ((pDirent = readdir(pDir)) != NULL) {
    char *p = strstr(pDirent->d_name, ".fnt");
    if (p && (p[4] == 0)) {
      strcpy(filename, default_path);
      strcat(filename, pDirent->d_name);
      
      // try to open the file
      FILE *fp = fopen(filename, "r+b");
      if (fp == NULL)
        continue;
      
      // read in the first part of the font
      int sz = fread(&temp, 1, sizeof(struct FONT), fp);
      fclose(fp);
      if (sz != sizeof(struct FONT))
        continue;
      
      // check that the signature is correct
      if (memcmp(temp.sig, "FONT", 4))
        continue;
      
      if (stricmp(temp.name, name) == 0) {
        closedir(pDir);
        return font_load(pDirent->d_name, FALSE);
      }
    }
  }
  
  closedir(pDir);
  return NULL;
}

/* font_list_destroy()
 *   no parameters
 *  
 *  free all loaded fonts
 *
 */
void font_list_destroy() {
  struct FONT *temp;
  
  while (last_font) {
    temp = last_font->prev;
    free(last_font);         
    last_font = temp;
  }
}

/* font_insert()
 *   font = pointer to font to insert
 *  
 *  insert a font into the chain of fonts
 *
 */
void font_insert(struct FONT *font) {
  if (last_font) {
    last_font->next = font;
    font->next = NULL;
    font->prev = last_font;
    last_font = font;
  } else {
    last_font = font;
    last_font->next = NULL;
    last_font->prev = NULL;
  }
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// fontdrawing routines
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/* font_in_range()
 *   font = pointer to font to insert
 *     ch = character index into font
 *  
 * is the character within the range of this font
 * this allows for the offset of font->start
 *
 */
const struct FONT_INFO *font_in_range(const struct FONT *font, const int ch) {
  if ((ch >= font->start) && (ch < (font->start + font->count)))
    return (struct FONT_INFO *) ((bit32u) font + sizeof(struct FONT) + (sizeof(struct FONT_INFO) * (ch - font->start)));
  
  return NULL;
}

/* font_get_bit()
 *      p = pointer to starting byte to check
 *      i = bit index
 *  
 * returns the bit value of the bit 'i' away from offset 'p'
 *  i = 0 = bit 7 of first byte
 */
bool font_get_bit(const bit8u *p, const int i) {
  return (p[(i / 8)] & (0x80 >> (i % 8))) ? TRUE : FALSE;
}

/* font_put_bit()
 *      p = pointer to starting byte to check
 *      i = bit index
 *  
 * sets the value of the bit 'i' away from offset 'p' to 'val'
 *  i = 0 = bit 7 of first byte
 */
void font_put_bit(bit8u *p, const int i, const bool val) {
  p[(i / 8)] &= ~(0x80 >> (i % 8));  // first clear it
  if (val)
    p[(i / 8)] |= (0x80 >> (i % 8));  // then set it (if val)
}

/* font_bitchar()
 *     bitmap = target bitmap to draw to
 *   charrect = rect of area of character
 *   drawrect = rect of area to draw to
 *       data = char bitmap data
 *      width = width of char
 *      color = color to draw character
 *       back = color of background
 *  
 * this 'prints' a single char to the screen
 * this expects the char data to be a string of bits, each bit representing a pixel or no pixel
 * this does not assume a width to the data stream of bits
 */
void font_bitchar(struct BITMAP *bitmap, const struct RECT *charrect, const struct RECT *drawrect, const bit8u *data,
                  bit32u width, PIXEL color, PIXEL back) {
  int r;
  bit32u bpix = 0;  // current bit position
  bit32u xoff, yoff, x2off;
  const bool fill = (color != back);
  
  xoff = drawrect->left - charrect->left;
  yoff = drawrect->top - charrect->top;
  x2off = charrect->right - drawrect->right;
  
  // Move the data over to correct to the clipping on the top and the width of the character
  bpix += (yoff * width);
  
  for (r = drawrect->top; r <= drawrect->bottom; r++) {
    PIXEL *iter = bitmap_iter(bitmap, drawrect->left, r);
    const PIXEL *end = bitmap_iter(bitmap, drawrect->right, r);
    
    // Move the data over to correct for the clipping on the left
    bpix += xoff;
    
    while (iter <= end) {
      // Foreground pixel
      if (font_get_bit(data, bpix))
        *iter = GUIBLEND(color, *iter);
      // Background pixel
      else if (fill)
        *iter = GUIBLEND(back, *iter);
      
      iter++;
      bpix++;
    }
    
    // Skip data to the right if the right side is clipped
    bpix += x2off;
  }
}

/* font_clipped()
 *       clip = clip area
 *       font = current font
 *          x = X coordinate
 *          y = Y coordinate
 */
bool font_clipped(const struct RECT *clip, const struct FONT *font, int x, const int y) {
  // if it is under or over or too far to the right of the clipping rect then
  //  we can assume the whole string is clipped
  if ((y + (int) font->height < clip->top) || (y > clip->bottom) || (x > clip->right))
    return TRUE;
  
  return FALSE;
}

/* font_bitmap_draw()
 *       font = current font
 *     bitmap = bitmap to draw to
 *       text = string of chars to draw
 *        len = len of string
 *          x = X coordinate
 *          y = Y coordinate
 *       fore = foreground color
 *       back = background color
 *      flags = flags to use
 *
 *   draws a string of text to a bitmap
 *   NOTE: a single line of text only. use font_bitmap_drawblock() for mulitple lines of text (i.e.: \n)
 *
 */
void font_bitmap_draw(const struct FONT *font, struct BITMAP *bitmap, const char *text, int len, int x, int y, PIXEL fore, PIXEL back, const bit32u flags) {
  int n = 0;
  bool underline;
  
  struct RECT charrect, clip;
  memset(&charrect, 0, sizeof(struct RECT));
  
  // if font not specified, use current (default) font
  if (!font)
    font = current_font;
  
  // copy the contents of bitmap->clip to clip
  clip = *bitmap_clip_get(bitmap);
  
  // if user sent -1, we use large string length to be sure we get all
  if (len < 0)
    len = 0x7FFF;
  
  // Skip easy clipping conditions
  if (font_clipped(&clip, font, x, y))
    return;
  
  // Write each character of the string
  underline = (flags & TEXTUAL_FLAGS_UNDERLINE) > 0;
  while (*text && (n < len)) {
    if ((flags & TEXTUAL_FLAGS_MNEMONIC) && (text[0] == '&') && (text[1] != '&')) {
      underline = TRUE;
      text++;
      n++;
      continue;
    }
    
    // HORZ: If left side of char farther than right side of clipping, we are done
    if ((x > clip.right) && !(flags & TEXTUAL_FLAGS_VERTICAL))
      return;
    // VERT: If top side of char farther than bottom of clipping, we are done
    if ((y > clip.bottom) && (flags & TEXTUAL_FLAGS_VERTICAL))
      return;
    
    struct RECT drawrect;
    bit8u *data;
    bit32u width;
    const struct FONT_INFO *info;
    
    if (flags & TEXTUAL_FLAGS_ISPASS)
      info = font_in_range(font, '*');
    else
      info = font_in_range(font, (int) (bit8u) *text);
    if (info) {
      data = (bit8u *) ((bit32u) font + sizeof(struct FONT) + (sizeof(struct FONT_INFO) * font->count));
      data += info->index;
      width = info->width;
      data = font_do_underline(data, (underline) ? TEXTUAL_FLAGS_UNDERLINE : 0, width, font->height);
    } else {
      // create a 'block char of this height'
      data = build_block_char(font->height);
      width = UNKNOWN_CHAR_WIDTH;
    }
    
    // charrect is the rect the char will occupy
    charrect.left = x;
    charrect.top = y;
    
    // adjust left and top for this char's deltas
    if (info) {
      charrect.left += info->deltax;
      charrect.top -= info->deltay;
    }
    
    if (flags & TEXTUAL_FLAGS_VERTICAL) {
      charrect.right = charrect.left + font->height - 1;
      charrect.bottom = charrect.top + width - 1;
      data = font_rotate_char(data, 90, width, font->height);
    } else {
      charrect.right = charrect.left + width - 1;
      charrect.bottom = charrect.top + font->height - 1;
    }
    
    // Only draw characters that are not completely clipped
    RECT_INTERSECT(charrect, clip, drawrect);
    if (RECT_VALID(drawrect))
      font->drawchar(bitmap, &charrect, &drawrect, data, (flags & TEXTUAL_FLAGS_VERTICAL) ? font->height : width, fore, back);
    
    // move to next char position
    if (flags & TEXTUAL_FLAGS_VERTICAL)
      y = ((charrect.bottom + 1) + ((info) ? info->deltaw : 0));
    else
      x = ((charrect.right + 1) + ((info) ? info->deltaw : 0));
    
    underline = (flags & TEXTUAL_FLAGS_UNDERLINE) > 0;
    
    // increment for next loop
    text++;
    n++;
  }
}

/* build_block_char()
 *     height = height of char
 *
 *  if char not found in font, draw a block char
 *  allows for up to 32x32 pixels
 *
 * NOTE: if width or height is more than 32, we may crash. TODO: fix me
 *
 */
bit8u build_block[32*32];
bit8u *build_block_char(bit32u height) {
  bit32u h;
  
  if (height > 32)
    height = 32;
  if (height < 3)
    height = 3;
  
  build_block[0] = 0xFF;    // top row of pixels
  for (h = 1; h < height-2; h++)
    build_block[h] = 0x81;  // intermidiate row(s)
  build_block[h] = 0xFF;    // bottom row of pixels
  
  return build_block;  
}

/* font_do_underline()
 *     data = char bitmap data
 *    flags = flags used to manipulate char
 *    width = width of char
 *   height = height of char
 *
 * TODO: we need to pass more flags (ital, strike, etc..)
 *        remember that 'underline' needs to be accounted for here
 *        via 'allow_underline' above.
 *
 * NOTE: if width or height is more than 32, we may crash. TODO: fix me
 *
 * must have separate mem blocks so we send the same address to be modified
 */
bit8u our_block[32*32];
bit8u *font_do_underline(bit8u *data, bit32u flags, const int width, const int height) {
  int i, x;
  
  memcpy(our_block, data, width * height);
  
  // TODO: if flags and underline/strike/italisize/bold/etc,
  //  copy data to our_block, modify, then return our block
  if (flags & TEXTUAL_FLAGS_UNDERLINE) {
    i = (height - 2) * width;
    for (x=0; x<width; x++)
      font_put_bit(our_block, i++, TRUE);
  }
  if (flags & TEXTUAL_FLAGS_STRIKE) {
    i = (height / 2) * width;
    for (x=0; x<width; x++)
      font_put_bit(our_block, i++, TRUE);
  }
  if (flags & TEXTUAL_FLAGS_ITALIC) {
    // TODO: maybe start at top and go down moving top
    //  half to the left and bottom half to the right.
    //  if we use a fraction, the math will drop the fractional
    //  part and we can move the top few lines 2, middle none,
    //  and bottom few 2.
  }
  
  return our_block;
}

/* font_rotate_char()
 *     data = char bitmap data
 *     degs = degrees to rotate (90, 180, or 270)
 *    width = width of char
 *   height = height of char
 *
 * rotate the char in 'data', 90, 180, or 270 degrees, returning new buffer
 * data -> buffer holding stream of bits 'width' wide and 'height' tall.
 *
 * NOTE: if width or height is more than 32, we may crash. TODO: fix me
 */
bit8u rot_block[32*32];
bit8u *font_rotate_char(const bit8u *data, const int degs, const int width, const int height) {
  int x, y, i = 0;
  
  switch (degs) {
    case 90:
      // rotate 90 degrees (90 to the right)
      for (x=0; x<width; x++)
        for (y=height; y>0; y--)
          font_put_bit(rot_block, i++, font_get_bit(data, ((y - 1) * width) + x));
      break;
      
    case 180:
      // rotate 180 degrees (vert flip)
      for (y=height; y>0; y--)
        for (x=0; x<width; x++)
          font_put_bit(rot_block, i++, font_get_bit(data, ((y - 1) * width) + x));
      break;
      
    case 270:
      // rotate 270 degrees (90 to the left)
      for (x=width; x>0; x--)
        for (y=0; y<height; y++)
          font_put_bit(rot_block, i++, font_get_bit(data, (y * width) + (x-1)));
      break;
  }
  
  // return the updated block
  return rot_block;
}

/* font_height()
 *     font = font to use (NULL = use current font)
 *
 * returns the fixed height of the font given
 *
 */
bit32u font_height(const struct FONT *font) {
  if (!font)
    font = current_font;
  
  // safety catch
  if (font)
    return font->height;
  
  return 0;
}

/* font_width()
 *        font = font to use (NULL = use current font)
 *        text = string of text to check
 *         num = length of string
 *  underlined = 0 = don't account for mnemonics
 *               1 = account for mnemonics
 *
 * returns the width of the text given using the font given
 *
 */
bit32u font_width(const struct FONT *font, const char *text, int num, const bool underlined) {
  int count = 0;
  bit32u len = 0;
  
  // if NULL use current font
  if (!font)
    font = current_font;
  
  // safety catch
  if (!font)
    return 0;
  
  // if user sent -1, we use large string length to be sure we get all
  if (num < 0)
    num = 0x7FFF;
  
  while (text && *text && (count < num)) {
    // if we allow a char to be underlined (as in a button mnemonic),
    //  skip the '&' in the calculation.
    if (!((*text == '&') && underlined)) {
      const struct FONT_INFO *info = font_in_range(font, (int) (bit8u) *text);
      len += ((info) ? (info->width + info->deltaw) : UNKNOWN_CHAR_WIDTH);
    }
    
    text++;
    count++;
  }
  
  return len;
}

/* font_bitmap_drawblock()
 *       font = current font
 *     bitmap = bitmap to draw to
 *       text = string of chars to draw
 *        len = len of string
 *          x = X coordinate
 *          y = Y coordinate
 *       fore = foreground color
 *       back = background color
 *      flags = flags to use
 *
 *   draws a string of text to a bitmap
 *   to draw single lines of text, call font_bitmap_draw() (as does this function)
 */
void font_bitmap_drawblock(const struct FONT *font, struct BITMAP *bitmap, const char *text, int len, int x, int y,
                           PIXEL fore, PIXEL back, const bit32u flags) {
  int n = 0;
  int lineheight;
  
  if (!font)
    font = current_font;
  
  // safety check
  if (!font)
    return;
  
  // get the height
  lineheight = font_height(font);
  
  // draw looping through for found EOL's (\n)
  while (text && *text && (n < len)) {
    int i = 0;
    const char *p = text;
    
    // calcuate the length of the string we will draw,
    //  stopping at end or end of line
    while (*p && (*p != '\r') && (*p != '\n') && (n < len)) {
      p++;
      i++;
      n++;
    }
    
    // draw a line of text
    font_bitmap_draw(font, bitmap, text, i, x, y, fore, back, flags);
    
    // skip CR
    if (*p == '\r') {
      p++;
      n++;
    }
    
    // skip LF
    if (*p == '\n') {
      p++;
      n++;
    }
    
    // move down to next scanline to draw to
    y += lineheight;
    text = p;
  }
}

/* font_blocksize()
 *       font = current font
 *    textual = textual object containing the text to size
 *         tw = pointer to an integer size memory block to store width to
 *         th = pointer to an integer size memory block to store height to
 *
 *   calculates the width and height of a text block using the specified font
 *
 */
void font_blocksize(const struct FONT *font, struct TEXTUAL *textual, int *tw, int *th) {
  int n = 0, lineheight, len;
  const char *text = string_text(&textual->text, &len);
  
  // clear passed return value pointers
  if (tw) *tw = 0;
  if (th) *th = 0;
  
  // if NULL specified, use current (default) font
  if (!font)
    font = current_font;
  
  // safety check
  if (!font)
    return;
  
  // get the height of a line of text
  lineheight = font_height(font);
  
  // if user supplied -1, check up to 65535 chars
  if (len < 0)
    len = 0xFFFF;
  
  if (th) *th = lineheight;
  
  // loop through each line, getting the width, summing the return
  while (text && *text && (n < len)) {
    int sw, i = 0;
    const char *p = text;
    unsigned newline = FALSE;
    
    // find the length of the line
    while (*p && (*p != '\r') && (*p != '\n') && (n < len)) {
      p++;
      i++;
      n++;
    }
    
    // get width
    sw = font_width(font, text, i, ((textual->flags & TEXTUAL_FLAGS_UNDERLINE) == TEXTUAL_FLAGS_UNDERLINE));
    
    // update the width value
    if (tw && (*tw < sw))
      *tw = sw;
    
    // skip CR
    if (*p == '\r') {
      p++;
      n++;
      newline = TRUE;
    }
    
    // skip LF
    if (*p == '\n') {
      p++;
      n++;
      newline = TRUE;
    }
    
    // update height
    if (newline && th)
      *th += lineheight;
    
    // new text position
    text = p;
  }
}

/* font_bitmap_get_pos()
 *       font = current font
 *       text = string of text we are checking
 *        len = length of text string
 *        pos = character index of current cursor position in text
 *       rect = pointer to a RECT to fill with area of character found
 *     height = where to store the returned height
 *
 *   calculates the position in pixels of the current cursor
 *
 */
bool font_bitmap_get_pos(const struct FONT *font, const char *text, int len, const int pos, struct RECT *rect, int *height) {
  int n = 0;
  int x = 0, y = 0;
  const char *p = text;
  
  if (!font)
    font = current_font;
  
  // safety check
  if (!font)
    return FALSE;
  
  if (height) *height = font_height(font);
  
  // if we are check for past the length, then error
  if (pos > len)
    return FALSE;
  
  while (*p && (n < len)) {
    const struct FONT_INFO *info = font_in_range(font, (int) (bit8u) *p);
    if (n == pos) {
      rect->left = x;
      rect->right = x + ((info) ? (info->width + info->deltaw) : UNKNOWN_CHAR_WIDTH);
      rect->top = y;
      rect->bottom = y + font_height(font);
      return TRUE;
    }
    
    // if CR[LF] (which may or may not be in the font), move to next line
    if (*p == 13) {
      x = 0;
      y += font_height(font);
    } else if (*p == 10) {
      ; // do nothing
    } else if (*p == 9) {
      x += ((info) ? (info->width * 2) : (UNKNOWN_CHAR_WIDTH * 2));
    } else {
      x += ((info) ? (info->width + info->deltaw) : UNKNOWN_CHAR_WIDTH);
    }
    
    p++;
    n++;
  }
  
  // if we get here, we are at the end of the string
  rect->left = x;
  rect->right = x + UNKNOWN_CHAR_WIDTH;
  rect->top = y;
  rect->bottom = y + font_height(font);
  
  return TRUE;
}
