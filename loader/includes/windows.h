
#ifndef _WINDOWS_H
#define _WINDOWS_H

#define WIN_HAS_TITLE    (1<< 0)
#define WIN_HAS_STATUS   (1<< 1)
#define WIN_HAS_BORDER   (1<< 2)
#define WIN_HAS_SHADDOW  (1<< 3)
#define WIN_IS_DIALOG    (1<< 4)
#define WIN_HAS_VSCROLL  (1<< 5)
#define WIN_XCENTER      (1<< 6)
#define WIN_YCENTER      (1<< 7)
#define WIN_CNTR_TITLE   (1<< 8)

#define WIN_CENTER       (WIN_XCENTER | WIN_YCENTER)

#define WIN_DEF_TEXT_SIZE  32768

#define BLACK     0
#define BLUE      1
#define GREEN     2
#define CYAN      3
#define RED       4
#define MAGENTA   5
#define BROWN     6
#define WHITE     7
#define GREY      8
#define B_WHITE  15

// main window colors
#define WIN_TITLE_COL   (((WHITE << 4) | BLACK) << 8)
#define WIN_STATUS_COL  (((WHITE << 4) | BLACK) << 8)
#define WIN_SHADDOW_COL (((BLUE << 4) | BLACK) << 8)
#define WIN_BORDER_COL  (((BLUE << 4) | WHITE) << 8)
#define WIN_BODY_COL    (((BLUE << 4) | WHITE) << 8)

// dialog window colors
#define DIA_TITLE_COL   (((B_WHITE << 4) | BLACK) << 8)
#define DIA_STATUS_COL  (((GREY << 4) | B_WHITE) << 8)
#define DIA_SHADDOW_COL (((BLUE << 4) | BLACK) << 8)
#define DIA_BORDER_COL  (((WHITE << 4) | WHITE) << 8)
#define DIA_BODY_COL    (((WHITE << 4) | BLACK) << 8)

struct S_WIN_SYS {
  int width, height;
  
};

#define WIN_MAX_TITLE_LEN  96

#define WINDOW_DEF_TIMER   (18 * 3)   // 3 seconds

struct S_WINDOW {
  bool is_active;
  void *parent;
  bit8u *title;
  bit8u *status;
  bit32u flags;
  int  x, y;
  int  width, height;
  int  line_num, tot_lines;
  bit8u *text;
  unsigned text_size;
  unsigned alloc_size;
  bit32u status_timer;
};

extern void *main_win;

void win_initialize(void);
void win_timer(void);
void *win_create(void *, const char *, const char *, int, int, int, int, bit32u);
void win_destroy(void *);
void win_printf(void *, const char *, ...);
void win_draw_win(void *);
void win_title_update(void *, const char *);
void win_status_clear(void *);
void win_status_update(void *, char *, const bool);

void win_init_progress(bit32u);
void win_put_progress(bit32u, bit32u);

extern void *help_scr;
void help_screen(void);

#endif  // _WINDOWS_H
