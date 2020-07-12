/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * examples.h
 *  
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
