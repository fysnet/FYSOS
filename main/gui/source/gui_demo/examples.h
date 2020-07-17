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
 *  examples.h
 *  
 *  Last updated: 17 July 2020
 */

#ifndef FYSOS_EXAMPLES
#define FYSOS_EXAMPLES

#pragma pack(1)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 0 (header code)
 *  
 *   this is a list example.  It adds roughly 10 items to a list and allows
 *    the user to select an item and make it the current title bar contents
 *
 */
struct APP0 {
   union {
      struct WIN win;
      struct TEXTUAL textual;
      struct OBJECT obj;
      struct RECTATOM rectatom;
      struct ATOM atom;
   } base;
   struct LIST list;
   struct BUTTON apply;
   struct BUTTON close;
};

struct APP0 *newapp0(void);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 1 (header code)
 *
 *  this example is a window that contains a progress bar, radio buttons, checkboxes,
 *   onoff object, two slider objects, a few text boxes, and two buttons.
 *  
 */
#define APP1_TOGGLES  5
#define APP1_RADIOS   8  // at least 7 for our examples. (we disable [5] and [6])

#define CHECK_BOX_RADIO  2
#define CHECK_BOX_ID0   32768
#define CHECK_BOX_ID2   (CHECK_BOX_ID0 + CHECK_BOX_RADIO)
#define RAIDO_DISABLE   6

#define RADIO_ID0       32782

struct APP1 {
   union {
      struct WIN win;
      struct TEXTUAL textual;
      struct OBJECT obj;
      struct RECTATOM rectatom;
      struct ATOM atom;
   } base;
   struct PROGRESS progress;
   struct BUTTON minus;
   struct BUTTON plus;
   struct CHECK_BOX toggle[APP1_TOGGLES];
   struct RADIO radio[APP1_RADIOS];
   struct TEXTUAL radio_text;
   struct SLIDERBAR hslider;
   struct SLIDERBAR vslider;
   struct UPDOWN updown;
   struct TEXTUAL onoff_static_text;
   struct ONOFF onoff;
   struct TEXTUAL static_text;
   struct TEXTUAL static_user;
   struct TEXTUAL username;
   struct TEXTUAL static_pass;
   struct TEXTUAL password;
   struct TEXTUAL url;
};

struct APP1 *newapp1(void);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * test example 2 (header code)
 *  
 *   this example shows how to create a window that has a menu, button bar,
 *    and a text edit object.
 *
 */
struct APP2 {
   union {
      struct WIN win;
      struct TEXTUAL textual;
      struct OBJECT obj;
      struct RECTATOM rectatom;
      struct ATOM atom;
   } base;
   struct TEXTEDIT edit;
   struct MENU *rightclick;
};

struct APP2 *newapp2(void);


#endif    // FYSOS_EXAMPLES
