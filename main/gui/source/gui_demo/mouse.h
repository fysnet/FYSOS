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
 *  mouse.h
 *
 *  Last updated: 17 July 2020
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
