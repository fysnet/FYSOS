/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * gui_mb.h
 *  
 */
#ifndef FYSOS_GUI_MB
#define FYSOS_GUI_MB

#pragma pack(1)

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Message Box Object
 *  
 */

#define GUI_MB_TYPES   6

enum GUI_MB_TYPE {
  GUI_MB_OK = 1,
  GUI_MB_OKCANCEL,
  GUI_MB_RETRYCANCEL,
  GUI_MB_YESNO,
  GUI_MB_YESNOCANCEL,
  GUI_MB_ABORT,
};

#define GUI_MB_MAX_BUTTONS  3
struct GUI_MB {
   union {
      struct ATOM atom;
      struct RECTATOM rectatom;
      struct OBJECT obj;
      struct TEXTUAL textual;
      struct WIN win;
   } base;
   struct BUTTON button[GUI_MB_MAX_BUTTONS];
   struct IMAGE icon;
   struct TEXTUAL body;
   bit32u type;
   int    ret_val;
};

int  gui_message_box(struct WIN *, const char *, const char *, GUI_MB_TYPE);
void mb_handler(struct WIN *);




#endif  // FYSOS_GUI_MB
