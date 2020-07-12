/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * filesel.h
 *  
 */
#ifndef FYSOS_FILESEL
#define FYSOS_FILESEL

#pragma pack(1)

#include "gui.h"
#include "grfx.h"


#define PATH_MAX      512
#define FNAME_MAX     512
#define MAX_PF_LEN    (PATH_MAX + FNAME_MAX + 1)
#define LAST_DEPTH     10   // count of saved directories to go back to

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * File Select Object
 *  
 */
struct FILESEL {
  union {
	  struct WIN win;
	  struct TEXTUAL textual;
	  struct OBJECT obj;
	  struct RECTATOM rectatom;
	  struct ATOM atom;
  } base;
  struct LIST list;
  struct BUTTON up;
  struct BUTTON back;
  struct BUTTON open;
  struct BUTTON cancel;
  struct BUTTON moreless;
  struct TEXTUAL dir;
  struct TEXTUAL file;
  struct TEXTUAL file_types;
  struct RADIO radio[3];
  struct CHECK_BOX check_box;
  
  bool type; // if set, is an open dialog, clear is a save as
  bool more; // if set, displaying more items
  int  more_add; // amount to add to height when displaying more
  char current[MAX_PF_LEN];
   int currentlen;
  char last[LAST_DEPTH][MAX_PF_LEN];
   int last_cur;
   int last_last;
};

void filesel_handler(struct WIN *);
struct FILESEL *fileselwin(struct FILESEL *, struct WIN *, bit32u, HANDLER, const int, const bool);
void filesel_refresh(struct FILESEL *);


#endif  // FYSOS_FILESEL
