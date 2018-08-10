/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "conio.h"
#include "malloc.h"
#include "paraport.h"
#include "stdio.h"
#include "string.h"
#include "sys.h"

#include "windows.h"

void *main_win = NULL;
void *help_scr = NULL;
struct S_WIN_SYS win_sys;
bit32u old_isr8;

void win_initialize(void) {
  struct REGS regs;
  
  regs.eax = 0x00000F00;
  intx(0x10, &regs);
  win_sys.width = (regs.eax & 0x0000FF00) >> 8;
  win_sys.height = 25;
  
  // http://scatcat.fhsu.edu/~jplang2/asm-blink.html
  inpb(0x3DA);             // reset the flip-flop
  outpb(0x3C0, 0x30);      // index 0x10  (20h + 10h)?
  outpb(0x3C0, inpb(0x3C1) & ~(1<<3));  // clear bit 3 to disable blink
  
  // hook into the BIOS Timer interrupt.
  // Then if main_win != NULL, and main_win->status_timer != 0, decrement the timer.
  //  if the timer value reaches zero, clear the status string of the window.
  old_isr8 = hook_vector(8, &win_timer);
}

void win_timer(void) {
  // save all registers used
  asm (
    " pushad       \n"
    " push  ds     \n"
    " xor  ax,ax   \n"
    " mov  ds,ax   \n"
  );
  
  if (main_win != NULL) {
    struct S_WINDOW *win = (struct S_WINDOW *) main_win;
    if (win->status_timer)
      if (--win->status_timer == 0)
        win_status_clear(main_win);
  }
  
  asm (
    "  pop  ds           \n"  
    "  popad             \n"  // restore all registers used
    "  add  sp,4         \n"  // remove the 'win' local parameter
    "  pop  ebp          \n"  // restore ebp
    "  sub  sp,4         \n"  // make room for the seg:off
    "  push eax          \n"  // save eax
    "  mov  eax,[dword fs:_old_isr8]  \n"  // must have the 'dword' operand or SmallerC won't create a relocation for it
    "  mov  [esp+4],eax  \n"  // put the seg:eax in the room we allocated
    "  pop  eax          \n"  // restore eax
    "  retf              \n"  // jump (removing 4 bytes from the stack)
    );
}

void *win_create(void *parent, const char *title, const char *status, int x, int y, int w, int h, bit32u flags) {
  struct S_WINDOW *win = (struct S_WINDOW *) malloc(sizeof(struct S_WINDOW));
  
  win->is_active = TRUE;
  if (parent)
    ((struct S_WINDOW *) parent)->is_active = FALSE;
  win->parent = parent;
  win->title = malloc(WIN_MAX_TITLE_LEN);
  win->title[0] = '\0';
  if (title) strcpy(win->title, title);
  win->status = malloc(WIN_MAX_TITLE_LEN);
  win->status[0] = '\0';
  if (status) strcpy(win->status, status);
  win->flags = flags;
  if (flags & WIN_XCENTER)
    win->x = (((win_sys.width - w) / 2) - 1) + x;
  else
    win->x = (x < win_sys.width) ? x : 0;
  if (flags & WIN_YCENTER)
    win->y = (((win_sys.height - h) / 2) - 1) + y;
  else
    win->y = (y < win_sys.height) ? y : 0;
  win->width = (w > -1) ? w : win_sys.width;
  win->height = (h > -1) ? h : win_sys.height;
  win->line_num = 0;
  win->tot_lines = 0;
  win->text = (char *) malloc(WIN_DEF_TEXT_SIZE);
  win->text[0] = '\0';
  win->alloc_size = WIN_DEF_TEXT_SIZE;
  win->text_size = 0;
  win->status_timer = 0;
  
  win_draw_win(win);
  
  return (void *) win;
}

void win_destroy(void *handle) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  
  if (win->parent) {
    ((struct S_WINDOW *) win->parent)->is_active = TRUE;
    win_draw_win(win->parent);
  }
  
  if (win->text)
    mfree(win->text);
  if (win->title)
    mfree(win->title);
  if (win->status)
    mfree(win->status);
  mfree(win);
}

void win_printf(void *handle, const char *str, ...) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  va_list vargs = (va_list) ((char *) &str + sizeof(char *));
  
  if (handle == NULL)
    return;
  
#ifdef MEM_DEBUG_ON
  char targ[1024];
#else
  char *targ = (char *) malloc(1024);
#endif
  
  bit16u sz = vsprintf(targ, str, vargs);
  
  if ((win->text_size + sz) > win->alloc_size) {
    win->alloc_size = win->text_size + sz + WIN_DEF_TEXT_SIZE;
    win->text = mrealloc(win->text, win->alloc_size);
  }
  
  if (spc_key_F2) {
    for (int i=0; i<sz; i++) {
      if (targ[i] == 10)
        para_putch(13);
      para_putch(targ[i]);
    }
  }
  
  // append the text
  strcpy(win->text + win->text_size, targ);
  win->text_size += sz;

  for (int i=0; i<sz; i++) 
    if (targ[i] == 10)
      win->tot_lines++;
  if ((win->tot_lines - win->line_num) > (win->height - 2))
    win->line_num = win->tot_lines - (win->height - 2);
  
#ifndef MEM_DEBUG_ON
  mfree(targ);
#endif
  
  win_draw_win(win);
}

void win_draw_title(void *handle) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  const bit16u title_col = (win->flags & WIN_IS_DIALOG) ? DIA_TITLE_COL : WIN_TITLE_COL;
  char title[WIN_MAX_TITLE_LEN+1];
  int x, i, j;
  
  strcpy(title, win->title);
  if (spc_key_F2)
    strcat(title, " (Debug Mode)");
  
  // if window does not have a title, return
  if (!(win->flags & WIN_HAS_TITLE))
    return;
  
  bit16u *p = (bit8u *) (0x0B8000 + (((win->y * win_sys.width) + win->x) * 2));
  *p++ = title_col | ' ';
  
  x = win->width - 1;
  if (win->flags & WIN_HAS_TITLE) {
    if (win->flags & WIN_CNTR_TITLE) {
      j = ((x - strlen(title)) / 2) - 1;
      while (j--) {
        *p++ = title_col | ' ';
        x--;
      }
    }
    i = 0;
    while ((x > 0) && title[i]) {
      *p++ = title_col | title[i];
      x--, i++;
    }
  }
  while (x > 0) {
    *p++ = title_col | ' ';
    x--;
  }
}

// this assumes that the programmer knows the size of the window,
//  knows the location, and knows that it fits within the parent window.
// no checks are made for windows shown past end of screen.
void win_draw_win(void *handle) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  bit16u *p;
  int x, i, j, l;
  //const bit16u title_col = (win->flags & WIN_IS_DIALOG) ? DIA_TITLE_COL : WIN_TITLE_COL;
  const bit16u status_col = (win->flags & WIN_IS_DIALOG) ? DIA_STATUS_COL : WIN_STATUS_COL;
  const bit16u shaddow_col = (win->flags & WIN_IS_DIALOG) ? DIA_SHADDOW_COL : WIN_SHADDOW_COL;
  const bit16u border_col = (win->flags & WIN_IS_DIALOG) ? DIA_BORDER_COL : WIN_BORDER_COL;
  const bit16u body_col = (win->flags & WIN_IS_DIALOG) ? DIA_BODY_COL : WIN_BODY_COL;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // draw the title bar
  win_draw_title(handle);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now draw body of window
  x = win->height - 2;
  i = 0;
  while (i < x) {
    p = (bit16u *) (0x0B8000 + ((((win->y + i + 1) * win_sys.width) + win->x) * 2));
    *p++ = border_col | 179;
    for (j=2; j<win->width; j++)
      *p++ = body_col | ' ';
    if (win->flags & WIN_HAS_VSCROLL) {
      if (i == 0)
        *p++ = border_col | 24;
      else if (i == (x-1))
        *p++ = border_col | 25;
      else
        *p++ = border_col | 176;
    } else
      *p++ = border_col | 179;
    if (win->flags & WIN_HAS_SHADDOW)
      *p++ = shaddow_col | 219;
    i++;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now draw status line
  p = (bit16u *) (0x0B8000 + ((((win->y + win->height - 1) * win_sys.width) + win->x) * 2));
  if (win->flags & WIN_HAS_STATUS) {
    *p++ = status_col | ' ';
    
    x = win->width - 1;
    i = 0;
    while ((x > 0) && win->status[i]) {
      *p++ = status_col | win->status[i];
      x--, i++;
    }
    while (x > 0) {
      *p++ = status_col | ' ';
      x--;
    }
  } else {
    if (win->flags & WIN_IS_DIALOG) {
      for (x=0; x<win->width; x++)
        *p++ = body_col | ' ';
    } else {
      *p++ = body_col | 192;
      for (x=0; x<win->width - 2; x++)
        *p++ = body_col | 196;
      *p++ = body_col | 217;
    }
  }
  
  if (win->flags & WIN_HAS_SHADDOW)
    *p++ = shaddow_col | 219;
  
  if (win->flags & WIN_HAS_SHADDOW) {
    p = (bit16u *) (0x0B8000 + ((((win->y + win->height) * win_sys.width) + win->x) * 2));
    p++;
    for (j=0; j<win->width; j++)
      *p++ = shaddow_col | 223;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now draw the text within the window
  // first finding the current line to display
  l = 0;
  j = 0;
  while (l < win->line_num) {
    while ((win->text[j] != 10) && (j < win->text_size))
      j++;
    if (win->text[j] == 10)
      j++;
    l++;
  }
  if (win->text[j] == 13)
    j++;
  
  i = 0;
  while (i < (win->height - 2)) {
    p = (bit16u *) (0x0B8000 + ((((win->y + i + 1) * win_sys.width) + win->x) * 2));
    p++;
    x = 1;
    if (i <= (win->tot_lines - l)) {
      while (win->text[j] && (win->text[j] != 10) && (x < (win->width - 1))) {
        *p++ = body_col | win->text[j];
        j++;
        x++;
      }
      if (win->text[j] == 10)
        j++;
      if (win->text[j] == 13)
        j++;
    }
    while (x < (win->width - 1)) {
      *p++ = body_col | ' ';
      x++;
    }
    i++;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now update the scroll bar
  if (win->flags & WIN_HAS_VSCROLL) {
    
    
  }  
}

void win_status_clear(void *handle) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  
  win_status_update(handle, "", FALSE);
}

void win_title_update(void *handle, const char *str) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  
  // if window does not have a title, return
  if (!(win->flags & WIN_HAS_TITLE))
    return;
  
  // save it to the window object too 
  if (str != win->title)
    strncpy(win->title, str, WIN_MAX_TITLE_LEN-1);
  
  win_draw_title(handle);
}

void win_status_update(void *handle, char *str, const bool activate_timer) {
  struct S_WINDOW *win = (struct S_WINDOW *) handle;
  bit16u *p;
  int i, x;
  const bit16u status_col = (win->flags & WIN_IS_DIALOG) ? DIA_STATUS_COL : WIN_STATUS_COL;
  
  // if window does not have a status line, return
  if (!(win->flags & WIN_HAS_STATUS))
    return;
  
  // if we are the main window, we need to add the F8 items at the end
  if (win == main_win) {
    sprintf(win->status, "%-39s | %s, %s, %s", str,
      (!spc_key_F1) ? "F1=Help" : " *Help*",
      (!spc_key_F8 && !spc_key_F9) ? "F8=Video Mode" : " *Video Mode*",
      (!spc_key_F9) ? "F9=Text only" : " *Text only*");
    win->status_timer = (activate_timer) ? WINDOW_DEF_TIMER : 0;
  } else
    // save it to the window object too 
    // (as long as the two buffers are not the same address)
    if (str != win->status)
      strcpy(win->status, str);
  
  p = (bit16u *) (0x0B8000 + ((((win->y + win->height - 1) * win_sys.width) + win->x) * 2));
  *p++ = status_col | ' ';
  
  x = win->width - 1;
  if (win->flags & WIN_HAS_STATUS) {
    i = 0;
    while ((x > 0) && win->status[i]) {
      *p++ = status_col | win->status[i];
      x--, i++;
    }
  }
  while (x > 0) {
    *p++ = status_col | ' ';
    x--;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Progress bar
bit32u progress_tot_size  = 0;  // set to total size at start of progress
int    progress_last      = 0;  // last percent printed

// this will not work for numbers greater than 42,949,671.
// for example, the calculation of
//   val = ((lo + 1) * 100);
// when lo = 42,949,672 would be
//   val = ((42,949,672 + 1) * 100) = 4,294,967,300 = 0x1_0000_0004
// and 0x1_0000_0004 is a 33 bit number
// so, until we can use 64-bit numbers, 42,949,671 is the limit
void win_init_progress(bit32u limit) {
  limit--;  // zero based
  
  if (limit > 42949671)
    limit = 42949671;
  if (limit < 0)
    limit = 0;
  
  progress_tot_size = limit;
  progress_last = -1;
  
  win_status_clear(main_win);
  win_put_progress(0, 0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Progress counter.  Displays (n%)
// on entry
//  current progress (64-bit)
#define PP_INDICIES 12
void win_put_progress(bit32u lo, bit32u hi) {
  bit32u val, cnt, i, j;
  char buffer[32], *p = buffer;
  
  // watch the limit
  if (lo > 42949671)
    lo = 42949671;
  
  // don't allow it to be more than the set total size
  //  from init_progress()
  if (lo > progress_tot_size)
    lo = progress_tot_size;
  
  // increment it to make it print 100% when done
  //  (1 based) (1  to 100, not 0 to 99)
  // this line is the limit maker.  If we could do
  //  this calculation in 64-bit, there would be a 
  //  much larger limit.
  val = ((lo + 1) * 100) / progress_tot_size;
  // percent = (bit32u) (incremental * (float) current);
  //_asm ("  fild  dword [ebp+6] \n"  // lo
  //      "  fstp  dword _temp   \n"
  //      "  fld   dword _progress_iter \n"
  //      "  fmul  dword _temp    \n"
  //      "  fist  dword [ebp-36] \n"); // val
  
  // TODO: The above calculation is quite slow.  It is done for every
  //  iteration even if the 'val' is the same value.  To speed this
  //  up, we can calcuate the iteration (the value between 1% and 2%)
  //  in init_progress() above, then just use add/multiply here.
  //  However, this means we have to have a floating point value.
  //  say .1 for a total value of 10, 1.1 for a value of 110, etc.
  //  NBC (SmallerC) doesn't do floats yet.
  //    bit32u  current = 0;
  //    bit32u  total = 0;
  //    bit32u  percent = 0;
  //    float   iter = 0;
  //  in init_progress:
  //    iter = ((float) 100.0 / (float) total);
  //  in put_progress: (here)
  //    percent = (bit32u) (iter * (float) current);
  
  // don't let val be more than 100%
  // may happen on numbers less than PP_INDICIES
  if (val > 100)
    val = 100;
  
  if (val != progress_last) {
    progress_last = val;
    
    // print so many bars  ( ascii 221, then 219)
    *p++ = 179;
    cnt = ((PP_INDICIES * 2 * (val + 1)) + 99) / 100;
    j = cnt / 2;
    for (i=0; i<j; i++)
      if (i < (j - 1))
        *p++ = 219;
      else {
        if (cnt & 1) *p++ = 219;
        else         *p++ = 221;
      }
    for (; i<PP_INDICIES; i++)
      *p++ = 249;
    *p++ = 179;
    
    sprintf(p, " (%i%%) ", val);
    
    win_status_update(main_win, buffer, FALSE);
  }
}

void help_screen(void) {
  spc_key_F1 = FALSE;
  help_scr = win_create(main_win, "Help", "  Press any other key to exit", 0, 0, 54, 16, 
    WIN_HAS_TITLE | WIN_CNTR_TITLE | WIN_HAS_STATUS | WIN_HAS_SHADDOW | WIN_IS_DIALOG | WIN_CENTER);
  win_printf(help_scr, " FYSOS Loader Help Screen:\n"
                       " \n"
                       " F1 = This help screen.\n"
                       " F2 = Debug to parallel port.\n"
                       " F3 =  nothing at this time.\n"
                       " F4 =  nothing at this time.\n"
                       " F5 =  nothing at this time.\n"
                       " F6 =  nothing at this time.\n"
                       " F7 =  nothing at this time.\n"
                       " F8 = Manually choose an available screen mode.\n"
                       " F9 = Do not change to the graphics mode.\n"
                       "                            (Stay in text mode)\n"
                       " \n"
                       "  Press a listed key above to toggle that function.");
  win_status_update(main_win, "Press a key to return", FALSE);
  win_title_update(main_win, ((struct S_WINDOW *) main_win)->title);
  
  while (!kbhit()) ;
  getscancode();
  
  spc_key_F1 = FALSE;
  help_scr = NULL;
}
