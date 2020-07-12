/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * mouse.h
 *  
 */
#ifndef FYSOS_MOUSE
#define FYSOS_MOUSE

#include <dpmi.h>

#pragma pack(1)


#define POINTER_STACK_SIZE  100

typedef enum POINTER_ID {
  POINTER_ID_STANDARD = 0,
  POINTER_ID_TEXT,
  POINTER_ID_HAND,
  POINTER_ID_BUSY,
  
  POINTER_ID_COUNT    // the count of ID's above
} POINTER_ID;

struct CUR_MOUSE {
  int  id;
  struct BITMAP *bitmap;
  int  w, h;     // width and height
  int  hw, hh;   // hot spot
};
extern struct CUR_MOUSE *cur_pointer;


#define MOUSE_LBUT  1     // bit 0
#define MOUSE_RBUT  2     // bit 1
#define MOUSE_MBUT  4     // bit 2

struct S_MOUSE_DATA {
  int    curx;                 // current x position
  int    cury;                 // current y position
  int    curz;                 // current z position
  bool   left;                 // 1 if pressed, 0 if not
  bool   mid;                  // 1 if pressed, 0 if not
  bool   right;                // 1 if pressed, 0 if not
  int    x_llimit;             // x lower limit
  int    y_llimit;             // y lower limit
  int    x_hlimit;             // x upper limit
  int    y_hlimit;             // y upper limit
  bit8u  x_thresh;             // x - value to multiply with
  bit8u  y_thresh;             // y - value to multiply with
};

bool mouse_init(const int, const int, const int, const int, const int, const int, const bit8u, const bit8u);
bool mouse_get_info(int *, int *, int *, int *);

bool set_pointer(const int);
bool restore_pointer();
bool load_mouse_pointer(const int, const int, const int, const int);

void mouse_handler(__dpmi_regs *);
bool install_mouse_handler(unsigned, void (*)(__dpmi_regs *));
void remove_mouse_handler(void);


#endif  // FYSOS_MOUSE
