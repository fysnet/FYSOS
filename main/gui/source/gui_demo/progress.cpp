/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * progress.cpp
 *  
 *   this file is the progress object
 */

#include "gui.h"

// see notes below why this is commented out
//#include "mouse.h"

/* progress_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to menu button
 *   eventstack.data = depends on event
 */
void progress_class(void) {
  struct PROGRESS *progress = (struct PROGRESS *) eventstack.object;
  
  switch (eventstack.event) {
    // draw progress bar to the screen
    case EXPOSE:
      gui_progress(progress);
      return;
      
    // we don't want the progress bar to gain focus
    case FOCUS_WANT:
      answer(NULL);
      return;
      
    /*
     * these two are simply to show that you can change the mouse cursor image
     *  when the cursor enters an object, and then again leaves that object
     * therefore, uncomment these seven lines (and the #include above) to show what is done
    case POINTER_ENTER:
      set_pointer(POINTER_ID_BUSY);
      return;
    
    case POINTER_LEAVE:
      restore_pointer();
      return;
    */
  }
  
  // call the object class
  obj_class();
}

/*  obj_progress()
 *     progress = pointer to progress object
 *         size = size of memory to allocate
 *       parent = parent object to link to (window)
 *        theid = id to give to progress bar
 *
 *   creates a progress bar
 *   
 */
struct PROGRESS *obj_progress(struct PROGRESS *progress, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct PROGRESS);
  
  // we use a textual base class so we have the percent (optionally) shown in text
  progress = (struct PROGRESS *) obj_textual(GUITEXTUAL(progress), size, parent, theid);
  if (progress) {
    GUIOBJ(progress)->_class = progress_class;
    
    // not armable or selectable
    obj_armable(GUIOBJ(progress), FALSE);
    obj_selectable(GUIOBJ(progress), FALSE);

    // initially set to zero percent
    progress->percent = 0;
    
    // initially don't show percent
    progress->show = FALSE;
  }

  // return pointer to created progress bar (or NULL if error)
  return progress;
}
