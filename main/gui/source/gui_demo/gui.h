/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * gui.h
 *  
 */
#ifndef FYSOS_GUI
#define FYSOS_GUI

#pragma pack(1)

#include <stdio.h>
#include <time.h>

#include "../include/ctype.h"
#include "ids.h"

// if you wish to make the title bar transparent, set USE_TRANPARENT_TITLES to 1
#define USE_TRANPARENT_TITLES  0
// stretch the desktop image to fit the screen
#define STRETCH_DESKTOP        0


//DEBUG((dfp, "x = %i y = %i px = %i py = %i  bitmap = %i  iter = %i", x, y, px, py, bitmap, iter));
extern FILE *dfp;
extern time_t timestamp;

//#define DO_DEBUG   // define this if you want to use debug
#ifdef DO_DEBUG
  #define DEBUG(x)      \
    do {                \
      time(&timestamp); \
      fprintf(dfp, "\n[%i: %s(% 4i)] ", (int) timestamp, __FILE__, __LINE__); \
      fprintf x;        \
      fflush(dfp);      \
    } while (0);
#else
  #define DEBUG(x)
#endif

extern char default_path[512];

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * 
 *  
 */
typedef bit32u PIXEL;

#define MINSIZE(s,a)                      \
    if ((s == GUIDEF) || (s < sizeof(a))) \
        s = sizeof(a);

// Handy marker for default vaues
#define GUIDEF    (0x7FFFFFFFL)

// Make our own max/min to avoid conflicts with other libs
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Rect
 *  
 */
struct RECT {
  int left;
  int top;
  int right;
  int bottom;
};

#define RECT_VALID(A)          \
   (((A).right >= (A).left) && \
    ((A).bottom >= (A).top))

#define RECT_INTERSECT(A, B, R)               \
  do {                                        \
    (R).left = MAX((A).left, (B).left);       \
    (R).top = MAX((A).top, (B).top);          \
    (R).right = MIN((A).right, (B).right);    \
    (R).bottom = MIN((A).bottom, (B).bottom); \
  } while (0)

#define RECT_UNION(A, B, R)                   \
  do {                                        \
    (R).left = MIN((A).left, (B).left);       \
    (R).top = MIN((A).top, (B).top);          \
    (R).right = MAX((A).right, (B).right);    \
    (R).bottom = MAX((A).bottom, (B).bottom); \
  } while (0)

// is X,Y within R->base.rectatom.rect
#define RECT_CONTAINS(R, X, Y)  \
   ((X >= (R).left) &&          \
    (X <= (R).right) &&         \
    (Y >= (R).top) &&           \
    (Y <= (R).bottom))

#define RECTS_OVERLAP(A, B)      \
   (!(((B).left > (A).right) ||  \
      ((B).top > (A).bottom) ||  \
      ((B).right < (A).left) ||  \
      ((B).bottom < (A).top)))

#define TOP_RECTS(A, B, R)                  \
   (R).left = (A).left;                     \
   (R).top = (A).top;                       \
   (R).right = (A).right;                   \
   (R).bottom = MIN( (B).top-1, (A).bottom )

#define LEFT_RECTS(A, B, R)                  \
   (R).left = (A).left;                      \
   (R).top = MAX( (B).top, (A).top );        \
   (R).right = MIN( (B).left-1, (A).right ); \
   (R).bottom = MIN( (B).bottom, (A).bottom )

#define RIGHT_RECTS(A, B, R)                 \
   (R).left = MAX( (B).right+1, (A).left );  \
   (R).top = MAX( (B).top, (A).top );        \
   (R).right = (A).right;                    \
   (R).bottom = MIN( (B).bottom, (A).bottom )

#define BOT_RECTS(A, B, R)                 \
   (R).left = (A).left;                    \
   (R).top = MAX( (B).bottom+1, (A).top ); \
   (R).right = (A).right;                  \
   (R).bottom = (A).bottom

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Atom
 *  
 */
typedef void (*ATOM_DESTRUCT) (void *atom);

struct ATOM {
  ATOM_DESTRUCT destruct;
  unsigned int lock;
  bool deleting;
  bool allocated;
};

void *obj_atom(struct ATOM *, ATOM_DESTRUCT, bit32u);

void atom_delete(struct ATOM *);
void atom_lock(struct ATOM *);
bool atom_unlock(struct ATOM *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Rectangular Atom
 *  
 */
struct RECTATOM {
  union {
    struct ATOM atom;
  } base;
  
  struct RECT rect;
};

#define GUIRECTATOM(b)   (&(b)->base.rectatom)
#define GUIRECT(b)       ((const struct RECT *)(&(b)->base.rectatom.rect))

void *obj_rectatom(struct RECTATOM *, ATOM_DESTRUCT, bit32u);
void rectatom_place(struct RECTATOM *, const struct RECT *);

#define rectatom_left(r)   ((r)->rect.left)
#define rectatom_top(r)    ((r)->rect.top)
#define rectatom_right(r)  ((r)->rect.right)
#define rectatom_bottom(r) ((r)->rect.bottom)
#define rectatom_w(r)      ((r)->rect.right - (r)->rect.left)
#define rectatom_h(r)      ((r)->rect.bottom - (r)->rect.top)

#define gui_w(r)     (rectatom_w(GUIRECTATOM(r))+1)
#define gui_h(r)     (rectatom_h(GUIRECTATOM(r))+1)
#define gui_left(r)   rectatom_left(GUIRECTATOM(r))
#define gui_top(r)    rectatom_top(GUIRECTATOM(r))
#define gui_right(r)  rectatom_right(GUIRECTATOM(r))
#define gui_bottom(r) rectatom_bottom(GUIRECTATOM(r))


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Strings
 *  
 */
struct STRING {
  char *str;       // char string
    int len;       // length of allocated buffer, or length of fixed buffer
   bool allocated; // is string allocated
};

void string(struct STRING *);
void string_free(struct STRING *);
void string_set(struct STRING *, const char *, int, const bool);
void string_copy(struct STRING *, const char *);
void string_insert(struct STRING *, const int, const char);
void string_remove(struct STRING *, const int);

const char *string_text(const struct STRING *, int *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Vectors
 *   Vector is a (almost) typesafe C vector object
 *   To use it you make a struct to contain the type that you want:
 *
 *    typedef union {
 *      struct VECTOR vector;
 *      MYTYPE *data;
 *    } MYVECTOR;
 */
struct VECTOR {
  void *data;         // Must be first to match with the users data
  bit32u size;
  int capacity;
};

void vector(struct VECTOR *);
void vector_free(struct VECTOR *);

void vector_set(struct VECTOR *, void *, const bit32u, const bool);

void vector_resize(struct VECTOR *, const bit32u, const bit32u);
void vector_reserve(struct VECTOR *, const bit32u, const bit32u);
void vector_contract(struct VECTOR *, const bit32u);

void *vector_append(struct VECTOR *, const void *, const bit32u, const bit32u);
void *vector_insert(struct VECTOR *, const void *, const bit32u, const bit32u, const bit32u);
void vector_remove(struct VECTOR *, const bit32u, bit32u, const bit32u);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Regions
 *  Regions are vectors of rectangles
 */
union REGION {
  struct VECTOR vector;
  struct RECT *data;
};

void region_clip(union REGION *, const struct RECT *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Macros for Tree
 * 
 */
#define NODE(type)  \
    type *parent;   \
    type *last;     \
    type *next;     \
    type *prev

#define node_insert(o,p)           \
  do {                             \
    if (p) {                       \
      (o)->parent = (p);           \
      if ((p)->last)               \
          (p)->last->next = (o);   \
      (o)->prev = (p)->last;       \
      (o)->next = 0;               \
      (p)->last = (o);             \
    }                              \
  } while (0)

#define node_remove(o)                   \
  do {                                   \
    if ((o)->parent) {                   \
      if ((o)->parent->last == (o))      \
          (o)->parent->last = (o)->prev; \
      if ((o)->prev)                     \
          (o)->prev->next = (o)->next;   \
      if ((o)->next)                     \
          (o)->next->prev = (o)->prev;   \
      (o)->next = 0;                     \
      (o)->prev = 0;                     \
      (o)->parent = 0;                   \
    }                                    \
  } while (0)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Flushing and dirty rectangle system
 * 
 */
typedef void (*FLUSH_FUNC) (struct RECT *);

void drs_area(const int, const int);
void drs_dirty(const struct RECT *, const bool);
bool drs_update(FLUSH_FUNC);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Some path and filename functions
 * 
 */
bit32u file_size(FILE *fp);
void parse_command(int, char *[], bool *, bool *);
void prefix_default_path(char *, const char *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Main functions
 *  
 */
int gui_execute(void);
void gui_stop(void);
void gui_exit(void);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Draw functions
 *  
 */
void gui_redraw(const struct RECT *);

// linear
void vesa_linear8(const int, const int, const int, const PIXEL *);
void vesa_linear15(const int, const int, const int, const PIXEL *);
void vesa_linear16(const int, const int, const int, const PIXEL *);
void vesa_linear24(const int, const int, const int, const PIXEL *);
void vesa_linear32(const int, const int, const int, const PIXEL *);

// bank switching
void vesa_bankswitch_init();
void vesa_bankswitch8(const int, const int, const int, const PIXEL *);
void vesa_bankswitch15(const int, const int, const int, const PIXEL *);
void vesa_bankswitch16(const int, const int, const int, const PIXEL *);
void vesa_bankswitch24(const int, const int, const int, const PIXEL *);
void vesa_bankswitch32(const int, const int, const int, const PIXEL *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Event Handling
 *  
 */
typedef enum EVENT {
   NOTHING = 0,
   DESTRUCT,
   EXPOSE,
   GEOMETRY,
   DEFAULTRECT,
   ARM,
   SELECT,
   TOP,
   ACTIVE,
   VSCROLL,
   HSCROLL,
   
   LIST_CHANGED,
   CHECK_BOX_CHANGED,
   ONOFF_CHANGED,
   RADIO_CHANGED,
   
   INTERACT_BEGIN,
   POINTER_WANT,
   POINTER_ENTER,
   POINTER_LEAVE,
   POINTER_MOVE,
   MPOINTER_MOVE,
   LPOINTER_PRESS,
   MPOINTER_PRESS,
   RPOINTER_PRESS,
   LPOINTER_RELEASE,
   MPOINTER_RELEASE,
   RPOINTER_RELEASE,
   LPOINTER_HELD,
   MPOINTER_HELD,
   RPOINTER_HELD,
   POINTER_CLICK,
   FOCUS_WANT,
   FOCUS_LOST,
   FOCUS_GOT,
   KEY,
   KEY_UNUSED,
   INTERACT_END,

   FILESEL_OK,
   
   MENU_LOST_FOCUS,
   GET_RCLICK_MENU,
   
   SEC_ELAPSED,
   
   LAST
} EVENT;

// Event information known to all objects
struct EVENTSTACK {
  struct DERIVED *object;
  EVENT event;
  const void *data;
  void *answer;
};

extern struct EVENTSTACK eventstack;

// Event handling functions
void event_default(void);

void *obj_event(struct OBJECT *, EVENT, const void *);
void answer(void *);

void *emit(EVENT, const void *);
void *inform(EVENT, const void *);

void destroy(void *atom);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Expose Handling
 *  
 */
const struct RECT *expose_rect(void);
bool is_exposing(void);
void obj_dirty(struct OBJECT *, const bool);
void expose_background(const struct RECT *);

void gui_event(void);

void gui_obj(struct OBJECT *);
void gui_vslider(struct SLIDER *);
void gui_hslider(struct SLIDER *);
void gui_textual_draw(const struct TEXTUAL *, int, int, int, int, int, int);
void gui_textual(struct TEXTUAL *);
void gui_scrollcorner(struct OBJECT *);
void gui_scroll(struct SCROLL *);
void gui_progress(struct PROGRESS *);
void gui_checkbox(struct CHECK_BOX *);
void gui_onoff(struct ONOFF *);
void gui_radio(struct RADIO *);
void gui_thumb(struct THUMB *);
void gui_sliderbar(struct SLIDERBAR *);
void gui_updown(struct UPDOWN *);
void gui_image(struct IMAGE *);
void gui_button(struct BUTTON *);
void gui_taskbar(struct TASKBAR *);
void gui_menu(struct MENU *);
void gui_menu_button(struct MENU_BUTTON *);
void gui_button_bar(struct BUTTON_BAR *);
void gui_button_bar_button(struct BUTTON_BAR_BUTTON *);
void gui_list(struct LIST *);
void gui_listelem(struct LISTELEM *);
void gui_root(struct WIN *);
void gui_winborder(struct WINBORDER *);
void gui_win(struct WIN *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Pointer Handling
 *  
 */
struct POINTER_INFO {
  int x, y, b;
  int dx, dy, dz;
};

const struct POINTER_INFO *pointer_info(void);
bool pointer_hold(void);
bool pointer_release(void);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Key Handling
 *  
 */
struct KEY_INFO {
  bit16u code;
  bit8u ascii;
  bit8u shift;
};


/*
FEDC BA98  7654 3210  Meaning
---- ----  ---- ---X  Right SHIFT is pressed
---- ----  ---- --X-  Left SHIFT is pressed
---- ----  ---- -X--  CTRL is pressed
---- ----  ---- X---  ALT is pressed
---- ----  ---X ----  Scroll Lock locked
---- ----  --X- ----  Num Lock locked
---- ----  -X-- ----  Caps Lock locked
---- ----  X--- ----  Insert locked

---- ---X  ---- ----  Left CTRL is pressed
---- --X-  ---- ----  Left ALT is pressed
---- -X--  ---- ----  Right CTRL is pressed
---- X---  ---- ----  Right ALT is pressed
---X ----  ---- ----  Scroll Lock is pressed
--X- ----  ---- ----  Num Lock is pressed
-X-- ----  ---- ----  Caps Lock is pressed
X--- ----  ---- ----  SysReq is pressed
*/
#define KEY_SHIFT(x)    (((x) & (3<<0)) !=     0 )
#define KEY_RSHIFT(x)   (((x) & (1<<0)) == (1<<0))
#define KEY_LSHIFT(x)   (((x) & (1<<1)) == (1<<1))
#define KEY_CTRL(x)     (((x) & (1<<2)) == (1<<2))
#define KEY_ALT(x)      (((x) & (1<<3)) == (1<<3))
#define KEY_SCRLL(x)    (((x) & (1<<4)) == (1<<4))
#define KEY_NUMLOCK(x)  (((x) & (1<<5)) == (1<<5))
#define KEY_CAPSLOCK(x) (((x) & (1<<6)) == (1<<6))
#define KEY_INSERT(x)   (((x) & (1<<7)) == (1<<7))

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Object
 *  
 */
typedef void (*CLASS) (void);

struct OBJECT {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
  } base;
  CLASS _class;
  NODE(struct OBJECT);
  struct WIN *win;
  int  id;
  bool destroying;
  bool armed;
  bool armable;
  bool selected;
  bool selectable;
  bool disabled;
  bool wantmove;
  bool visible;
  bool has_mnemonic;
};

#define GUIOBJ(o)   (&(o)->base.obj)
#define GUICLASS(o) ((const CLASS)(GUIOBJ(o)->_class))
#define GUIID(o)    ((const int)(GUIOBJ(o)->id))

struct DERIVED {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  // Must have no data!
};

int obj_x(struct OBJECT *);
int obj_y(struct OBJECT *);
void obj_place(struct OBJECT *, const struct RECT *);
void obj_position(struct OBJECT *, int, int, int, int);
void obj_move(struct OBJECT *, int, int);
void obj_resize(struct OBJECT *, int, int);
void obj_geometry(struct OBJECT *);
struct RECT *defaultrect_data(void);
void obj_defaultrect(struct OBJECT *, struct RECT *);

typedef enum LAYOUT {
  LAYOUT_RIGHT = (1 << 0),
  LAYOUT_LEFT = (1 << 1),
  LAYOUT_TOP = (1 << 2),
  LAYOUT_BOTTOM = (1 << 3),
  LAYOUT_X1 = (1 << 4),
  LAYOUT_Y1 = (1 << 5),
  LAYOUT_X2 = (1 << 6),
  LAYOUT_Y2 = (1 << 7),
  LAYOUT_W = (1 << 8),
  LAYOUT_H = (1 << 9),
  LAYOUT_HCENTER = (1 << 10),
  LAYOUT_VCENTER = (1 << 11),
  LAYOUT_CENTER = LAYOUT_HCENTER | LAYOUT_VCENTER
} LAYOUT;

void obj_layout(struct OBJECT *, LAYOUT, const struct OBJECT *, int, int);
void obj_wantmove(struct OBJECT *, bool);
bool obj_armed(const struct OBJECT *);
void *obj_arm(struct OBJECT *, bool);
void obj_armable(struct OBJECT *, bool);
bool obj_is_armable(const struct OBJECT *);
bool obj_selected(const struct OBJECT *);
bool obj_set_mnemonic(struct OBJECT *, const bool);
bool obj_has_mnemonic(const struct OBJECT *);
void *obj_select(struct OBJECT *, bool);
void obj_selectable(struct OBJECT *, bool);
bool obj_is_selectable(const struct OBJECT *);
void obj_disable(struct OBJECT *, bool);
bool obj_disabled(const struct OBJECT *);
void obj_visible(struct OBJECT *, bool);
bool obj_isvisible(const struct OBJECT *);

void *obj_top(struct OBJECT *);
void obj_focus(struct OBJECT *);
struct OBJECT *obj_get_focus();

void obj_event_recurse(struct OBJECT *, EVENT, const void *);

// Some macros to make the code a bit neater
extern struct ROOT_ROOT root;
#define DESKTOP &root.root
#define DESKTOPOBJ GUIOBJ(DESKTOP)

void root_class(void);

// Base object class
void obj_class(void);
void *object(struct OBJECT *, CLASS, bit32u, struct OBJECT *, int);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Image Object
 *  
 */

#define IMAGE_FLAGS_STRETCH   (1<<0)

struct IMAGE {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  bit32u flags;
  struct BITMAP *bitmap;
};

void image_class(void);
struct IMAGE *obj_image(struct IMAGE *, bit32u, struct OBJECT *, int);
void image_ownerdraw(struct IMAGE *, const int, int, int);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Textual Object
 *  
 */

#define VK_ESCAPE     0x011B
#define VK_BACKSPACE  0x0E08
#define VK_TAB        0x0F09
#define VK_ENTER      0x1C0D
#define VK_HOME       0x4700
#define VK_UPARROW    0x4800
#define VK_LFARROW    0x4B00
#define VK_RTARROW    0x4D00
#define VK_END        0x4F00
#define VK_DNARROW    0x5000
#define VK_DELETE     0x5300

struct CARET {
  int    height;
  struct RECT rect;
  struct IMAGE *caret;
};

typedef enum ALIGN {
   ALIGN_NONE = 0,
   ALIGN_RIGHT = (1 << 0),
   ALIGN_LEFT = (1 << 1),
   ALIGN_TOP = (1 << 2),
   ALIGN_BOTTOM = (1 << 3),
   ALIGN_HCENTER = (1 << 4),
   ALIGN_VCENTER = (1 << 5),
   ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER
} ALIGN;

struct TEXTUAL {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  bit32u flags;
  struct STRING text;
  ALIGN align;
  bool  hovering;
  int   textwidth;
  int   textheight;
  PIXEL fore_color;
  PIXEL back_color;
  const struct FONT *font;
  struct BITMAP *decorator;
  int    cur_char_pos;
  struct CARET caret;
};

#define GUITEXTUAL(o)         (&(o)->base.textual)

// flags field
#define TEXTUAL_FLAGS_NONE            0
#define TEXTUAL_FLAGS_BORDER     (1<< 0)
#define TEXTUAL_FLAGS_READONLY   (1<< 1)  // must be for all TEXTUAL's, but can be cleared for TEXTEDIT's
#define TEXTUAL_FLAGS_VSCROLL    (1<< 2)
#define TEXTUAL_FLAGS_HSCROLL    (1<< 3)
#define TEXTUAL_FLAGS_UNDERLINE  (1<< 4)  // allow &F underlined
#define TEXTUAL_FLAGS_STRIKE     (1<< 5)
#define TEXTUAL_FLAGS_ITALIC     (1<< 6)
#define TEXTUAL_FLAGS_BOLD       (1<< 7)
#define TEXTUAL_FLAGS_VERTICAL   (1<< 8)
#define TEXTUAL_FLAGS_NORETURN   (1<< 9)
#define TEXTUAL_FLAGS_NUMONLY    (1<<10)
#define TEXTUAL_FLAGS_NOCARET    (1<<11)
#define TEXTUAL_FLAGS_ISPASS     (1<<12)
#define TEXTUAL_FLAGS_ISLINK     (1<<13)
#define TEXTUAL_FLAGS_MNEMONIC   (1<<14)

void textual_set(struct TEXTUAL *, const char *, const int, const bool);
void textual_set_font(struct TEXTUAL *, const char *);
void textual_set_flags(struct TEXTUAL *, const bit32u);
void textual_copy(struct TEXTUAL *, const char *);
void text_obj_color(struct TEXTUAL *, PIXEL, PIXEL);
const char *textual_text(const struct TEXTUAL *, int *);
int textual_width(const struct TEXTUAL *);
int textual_height(const struct TEXTUAL *);
void textual_align(struct TEXTUAL *, ALIGN align);
ALIGN textual_align_get(const struct TEXTUAL *);
bool textual_get_pos(const TEXTUAL *, const int, struct RECT *, int *);
void textual_update_caret(struct TEXTUAL *);
void textual_class(void);
void our_textual_class(void);
struct TEXTUAL *obj_textual(struct TEXTUAL *, bit32u size, struct OBJECT *, int);
void gui_textual_align(const struct TEXTUAL *, int *, int *, int, int);
struct TEXTUAL *obj_static_text(struct TEXTUAL *, bit32u, struct OBJECT *, int, const char *);
struct TEXTUAL *obj_edit_text(struct TEXTUAL *, bit32u, struct OBJECT *, int);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Button Object
 *  
 */
#define BUTTON_DEFAULT   (1<<0)
#define BUTTON_EXIT      (1<<1)

struct BUTTON {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  struct BITMAP *bitmap;
  bit32u flags;
};

#define GUIBUTTON(o)   (&(o)->base.button)

void button_class(void);
struct BUTTON *obj_button(struct BUTTON *, bit32u, struct OBJECT *, int, const bit32u);
void button_ownerdraw(struct BUTTON *, const int, int, int);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Taskbar Object
 *  
 */

#define TASKBAR_MIN_H 28

#define TASKBAR_COLOR  GUIRGB( 48,182, 18)

struct TASKBAR {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  bit32u flags;
  int    cur_height;
  int    timer;
  struct MENU *menu;
  struct TEXTUAL *time;
};

void taskbar_class(void);
struct TASKBAR *obj_taskbar(struct TASKBAR *, bit32u, struct OBJECT *, int);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Window Object
 *  
 */
typedef void (*HANDLER) (struct WIN * win);

struct WIN {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  bit32u flags;
  HANDLER handler;
  struct BITMAP *icon;  // icon in left top part of title bar
  struct WINBORDER *border;
  struct MENU *menu;
  struct BUTTON_BAR *button_bar;
  NODE(struct WIN);
  int modallock;
  bool ismodal;
};

#define GUIWIN(o)   (&(o)->base.win)

void win_class(void);
void win_handler(struct WIN *);
struct WIN *obj_win(struct WIN *, struct WIN *, bit32u, HANDLER, const int, const bit32u);
bool win_active(const struct WIN *);
struct WIN *cur_active_win(void);
struct MENU *win_get_menu(struct WIN *);
struct BUTTON_BAR *win_get_button_bar(struct WIN *);
bool win_activate(struct WIN *);
void win_dirty(struct WIN *);

struct OBJECT *win_check_mnemonics(const struct WIN *, const char);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Window Border Object
 *  
 */
struct WINBORDER {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  bit32u  flags;
  HANDLER callback;
  struct RECT org_size;
  struct BUTTON close;
  struct BUTTON min;
  struct BUTTON max;
  struct BUTTON resize;
  struct TEXTUAL status;
};

// flags
#define BORDER_CLOSE_DISABLED  (1<<0)
#define BORDER_HAS_MIN         (1<<1)
#define BORDER_HAS_MAX         (1<<2)
#define BORDER_HAS_RESIZE      (1<<3)
#define BORDER_HAS_STATUS      (1<<4)
#define BORDER_NORMAL          (BORDER_HAS_MIN | BORDER_HAS_MAX | BORDER_HAS_RESIZE)

#define BORDER_IS_MINIMIZED    (1<<8)
#define BORDER_IS_MAXIMIZED    (1<<9)


void winborder_class(void);
void winborder_callback(struct WIN *);
struct WINBORDER *winborder(struct WINBORDER *, bit32u, struct WIN *, const int, const bit32u);
void win_set_icon_id(struct WIN *, const bit32u);
void win_set_icon_name(struct WIN *, const char *);
void win_child(struct WIN *, struct WIN *);
void win_modal(struct WIN *, struct WIN *);
void win_set_status(struct WIN *, const char *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Generic Slider Object
 *  
 */
struct SLIDER {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  int min;
  int upper;
  int lower;
  int dim;
  int range;
  int size;
  int value;
};

#define GUISLIDER(o)         (&(o)->base.slider)
#define SLIDER_SIZE 10

void slider_set(struct SLIDER *, int, int, int);
void slider_to(struct SLIDER *, int);
int slider_value(struct SLIDER *);

// Vertical slider object
void vslider_class(void);
struct SLIDER *obj_vslider(struct SLIDER *, bit32u, struct OBJECT *, int);

// Horizontal slider object
void hslider_class(void);
struct SLIDER *obj_hslider(struct SLIDER *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Scrollable Area Object
 *  
 */
struct SCROLL {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  int x, y;
  int w, h;
  int vw, vh;
  struct TEXTUAL text;
  struct SLIDER *hscroll;
  struct SLIDER *vscroll;
  struct OBJECT *corner;
};

#define GUISCROLL(o)         (&(o)->base.scroll)

void scrollcorner_class(void);
void scroll_class(void);
struct SCROLL *obj_scroll(struct SCROLL *, bit32u, struct OBJECT *, int);
bool scroll_scrollable_element(const struct SCROLL *, const struct OBJECT *);
void scroll_geometry(struct SCROLL *, int, int, int, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * List Element Object
 *  
 */
struct LISTELEM {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
    struct BUTTON button;
  } base;
  struct LIST *list;
  struct LISTELEM *prev;
  struct LISTELEM *next;
  struct BITMAP *icon;
  void *attachment;
};

void listelem_class(void);
struct LISTELEM *listelement(struct LISTELEM *, bit32u, struct LIST *, int);

#define LIST_MULTIPLE    (1<<0)   // multiple items selected at once

// Scrollable list object
struct LIST {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
    struct SCROLL scroll;
  } base;
  struct LISTELEM *last;
  bool multiple;
  bool changing;
};

#define GUILIST(o)   (&(o)->base.list)

void list_class(void);
struct LIST *obj_list(struct LIST *, bit32u size, struct OBJECT *, int id, const bit32u);
void list_append(struct LIST *, const char *, int, const bool, int, void *, struct BITMAP *);
void list_multiple(struct LIST *, const bool);
void list_link(struct LIST *, struct LISTELEM *);
void list_unlink(struct LISTELEM *);
int list_count_selected(const struct LIST *);
void list_select_id(const struct LIST *, const int, const bool);
int list_selected_id(const struct LIST *, struct LISTELEM *);
struct LISTELEM *list_selected(const struct LIST *, struct LISTELEM *);
struct LISTELEM *list_iter(const struct LIST *, const struct LISTELEM *);
void list_empty(struct LIST *);
void list_scroll_geometry(struct LIST *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * TextEdit Object
 * 
 */
struct TEXTEDIT {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
    struct SCROLL scroll;
  } base;
  bit32u flags;
  struct TEXTUAL body;
};

#define GUITEXTEDIT(o)         (&(o)->base.textedit)

void textedit_class(void);
struct TEXTEDIT *obj_textedit(struct TEXTEDIT *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Progress Bar Object
 *  
 */
struct PROGRESS {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  bit8u percent;     // percent of progress, 0 - 100
  bool  show;        // show the percent in the box using numerals too
};

void progress_class(void);
struct PROGRESS *obj_progress(struct PROGRESS *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Check Box Object
 *  
 */
#define CHECK_BOX_WIDTH 14

struct CHECK_BOX {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  bool checked;  // checked (return TRUE if checked)
  bool right;    // if TRUE, box is on right of text
};

void checkbox_class(void);
struct CHECK_BOX *obj_checkbox(struct CHECK_BOX *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Radio Button Object
 *  
 */
#define RADIO_WIDTH 14

struct RADIO {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
  } base;
  struct RADIO *prev;
  struct RADIO *next;
  bool set;      // if this radio button is set
  bool right;    // if TRUE, box is on right of text
};

void radio_class(void);
struct RADIO *obj_radio(struct RADIO *, bit32u, struct OBJECT *, int);
int obj_radio_get_checked_id(struct RADIO *);
int obj_radio_get_checked_num(struct RADIO *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * OnOff Object
 *  
 */
struct ONOFF {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  struct BITMAP *bitmap;
  bool on;       // return TRUE if on
};

void onoff_class(void);
struct ONOFF *obj_onoff(struct ONOFF *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * SliderBar Object
 *  
 */
#define SLIDERBAR_TICKS   (1<<0)
#define SLIDERBAR_WIDE    (1<<1)
#define SLIDERBAR_NUMS    (1<<2)
#define SLIDERBAR_RAISED  (1<<3)
#define SLIDERBAR_POINTED (1<<4)  // use pointed thumb
#define SLIDERBAR_HORZ    (0<<5)
#define SLIDERBAR_VERT    (1<<5)

#define SLIDERBAR_MIN_W   75  // minimum width of a slider bar
#define SLIDERBAR_MIN_H   (8 + 15) // minimum height of a slider bar (not counting WIDE and TICKS)

#define SLIDERBAR_EDGE    10
#define SLIDERBAR_THUMBW  10
#define SLIDERBAR_WIDE_H   8  // height of wide block
#define SLIDERBAR_TICK_H   5  // height of a tick

struct THUMB {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
};

struct SLIDERBAR {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  struct THUMB thumb;
  bit32u flags;
  int min;
  int max;
  int cur;
  int ratio;   // amount of tick marks per increment (2 = every 2 increments get a single tick mark)
  float incr;  // pixels between two increments (we save it so we don't have to calulate it each time)
};             //                               (we only calculate it at geometry time)

void sliderbar_setmax(struct SLIDERBAR *, const int);
void sliderbar_setmin(struct SLIDERBAR *, const int);
void sliderbar_setlimits(struct SLIDERBAR *, const int, const int);
void sliderbar_setratio(struct SLIDERBAR *, const int);
void sliderbar_setcur(struct SLIDERBAR *, const int);
int sliderbar_getcur(struct SLIDERBAR *);
void sliderbar_setflags(struct SLIDERBAR *, const bit32u);
bit32u sliderbar_getflags(struct SLIDERBAR *);

void sliderbar_class(void);
struct SLIDERBAR *obj_sliderbar(struct SLIDERBAR *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Up/Down Object
 *  
 */
#define UPDOWN_LEFT_TEXT   (0<<0)
#define UPDOWN_RIGHT_TEXT  (1<<0)
#define UPDOWN_FIT_TO_TEXT (2<<0)

#define UPDOWN_LEFT       4    // pixels on left and right sides before objects
#define UPDOWN_TOP        3    // pixels on top and bottom before objects
#define UPDOWN_TEXT_W    35
#define UPDOWN_BUTTON_W  16
#define UPDOWN_BUTTON_H   8
#define UPDOWN_MIN_W   (UPDOWN_LEFT + UPDOWN_TEXT_W + 4 + UPDOWN_BUTTON_W + UPDOWN_LEFT)  // minimum width of object
#define UPDOWN_MIN_H   (UPDOWN_TOP + UPDOWN_BUTTON_H + 4 + UPDOWN_BUTTON_H + UPDOWN_TOP)  // minimum height of object

struct UPDOWN {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  struct TEXTUAL text;
  struct BUTTON up;
  struct BUTTON down;
  bit32u flags;
  int cur;
};

void updown_setcur(struct UPDOWN *, const int);
int updown_getcur(struct UPDOWN *);
void updown_setflags(struct UPDOWN *, const bit32u);
bit32u updown_getflags(struct UPDOWN *);

void updown_class(void);
struct UPDOWN *obj_updown(struct UPDOWN *, bit32u, struct OBJECT *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Menu Object
 *  
 */
struct MENU {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  struct MENU *parent;        // pointer to parent menu (NULL if this is the first)
  struct MENU_BUTTON *last;   // pointer to last (inserted) button within this menu (Must not be NULL)
  struct MENU_BUTTON *first;  // pointer to first (inserted) button within this menu (Must not be NULL)
  struct MENU_BUTTON *current; // pointer to current selected button
  PIXEL  back_color;
  bool   wide;  // TRUE = width of parent, FALSE = min width to hold all buttons
};

void menu_class(void);
struct MENU *obj_menu(struct MENU *, bit32u, struct OBJECT *, int, struct MENU *);
void insert_menu(struct MENU *, struct WIN *, const int, const int);
bool menu_visible(struct MENU *);
void menu_remove_child_nodes(struct MENU *);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Menu Button Object
 *  
 */
#define MENU_SEPRTR_H  5

typedef enum MENU_BUTTON_FLAGS {
  MENU_BTN_ACTIVE    =  (1<<0),  // used in the menu code.  Can not be set from application
  MENU_BTN_ENABLED   =  (1<<1),  // Button is enabled.
  MENU_BTN_DISABLED  =  (0<<1),  // Button is not enabled.
  MENU_BTN_HIDDEN    =  (1<<2),  // inserted into menu, but not displayed
  MENU_BTN_HAS_OFF   =  (1<<3),  // At least one of the buttons in this menu has an icon, so scoot over...
  
  MENU_BTN_SEPARATOR =  (1<<8),  // is a separator.  Simply print a line
  
  MENU_BTN_NORMAL   = MENU_BTN_ENABLED,
  
} MENU_BUTTON_FLAGS;

struct MENU_BUTTON {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
    struct BUTTON button;
  } base;
  struct MENU *parent;       // pointer to parent MENU object
  struct MENU *submenu;      // pointer to a menu object if this button is a submenu button (NULL if no sub-menu)
  struct MENU_BUTTON *prev;  // previous button in this menu (NULL if first)
  struct MENU_BUTTON *next;  // next button in this menu (NULL if last)
  struct BITMAP *icon;       // icon on left of button
  struct BITMAP *right;      // icon used to show is sub menu ('>')
  bit32u flags;
};

void menu_button_class(void);
struct MENU_BUTTON *obj_menu_button(struct MENU_BUTTON *, bit32u, struct MENU *, int);
struct MENU_BUTTON *menu_append(struct MENU *, const char *, const int, const int, const int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Button Bar Object
 *  
 */
struct BUTTON_BAR {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
  } base;
  struct BUTTON_BAR_BUTTON *last;
};

void button_bar_class(void);
struct BUTTON_BAR *obj_button_bar(struct BUTTON_BAR *, bit32u, struct OBJECT *, int *, int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Button Bar Button Object
 *  
 */
typedef enum BUTTON_BAR_BUTTON_FLAGS {
  BUTTON_BTNBAR_ACTIVE   =  (1<<0),  // used in the code.  Can not be set from application
  BUTTON_BTNBAR_ENABLED  =  (1<<1),  // Button is enabled.
  BUTTON_BTNBAR_DISABLED =  (0<<1),  // Button is not enabled.
  
  BUTTON_BTNBAR_NORMAL   = BUTTON_BTNBAR_ENABLED,
  
} BUTTON_BAR_BUTTON_FLAGS;

struct BUTTON_BAR_BUTTON {
  union {
    struct ATOM atom;
    struct RECTATOM rectatom;
    struct OBJECT obj;
    struct TEXTUAL textual;
    struct BUTTON button;
  } base;
  struct BUTTON_BAR *parent;
  struct BUTTON_BAR_BUTTON *prev;
  struct BUTTON_BAR_BUTTON *next;
  struct BITMAP *icon;
  bit32u flags;
};

void button_bar_button_class(void);
struct BUTTON_BAR_BUTTON *obj_button_bar_button(struct BUTTON_BAR_BUTTON *, bit32u, struct BUTTON_BAR *, int);
struct BUTTON_BAR_BUTTON *button_bar_append(struct BUTTON_BAR *, const int, const int);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Private static data
 *  i.e.: the root
 */
struct ROOT_ROOT {
  struct WIN root;
  struct RECT clip;
  
  struct POINTER_INFO pointer;
  struct OBJECT *pointed;
  struct OBJECT *pointerhold;
  
  struct OBJECT *focus;
  struct WIN *active;
  
  bool exposing;
  bool running;
};

#endif  // FYSOS_GUI
