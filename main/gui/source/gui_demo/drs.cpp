/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * drs.cpp
 *  
 *  this is used to keep track of the items that are dirty, then pushing them to the
 *   screen at certain times.
 *  we don't want to constantly be updating the screen since it will slow down the
 *   other processes and may add flicker to the display.
 *
 */

#include "../include/ctype.h"
#include "gui.h"


#define REGION_CHUNK 64

struct RECT drs_area_data = { 0, 0, 0, 0 };
union REGION drs_dirtyinverse_data = { {0, 0, 0} };
union REGION drs_dirty_data = { {0, 0, 0} };
bool drs_started = FALSE;
bool drs_dirtychanged = FALSE;

/* drs_stop()
 *   no parameters
 *
 *  stop the updates 
 *  
 */
void drs_stop(void) {
  if (drs_dirtyinverse_data.data)
    vector_free(&drs_dirtyinverse_data.vector);

  if (drs_dirty_data.data)
    vector_free(&drs_dirty_data.vector);

  drs_started = FALSE;
}

/* drs_start()
 *   no parameters
 *
 *  start the updates 
 *  
 */
void drs_start(void) {
  vector(&drs_dirtyinverse_data.vector);
  vector_reserve(&drs_dirtyinverse_data.vector, REGION_CHUNK, sizeof(drs_dirtyinverse_data.data[0]));
  
  vector(&drs_dirty_data.vector);
  vector_reserve(&drs_dirty_data.vector, REGION_CHUNK, sizeof(drs_dirty_data.data[0]));
  
  drs_stop();
  drs_started = TRUE;
}

/* drs_area()
 *     w = width of area relative to top-left corner
 *     h = height of area relative to top-left corner
 *
 *  set the DRS area
 *  
 */
void drs_area(const int w, const int h) {
  if (!drs_started)
    drs_start();
  
  drs_area_data.left = 0;
  drs_area_data.top = 0;
  drs_area_data.right = w;
  drs_area_data.bottom = h;
  
  vector_resize(&drs_dirtyinverse_data.vector, 0, sizeof(drs_dirtyinverse_data.data[0]));
  drs_dirtychanged = TRUE;
}

/* drs_update()
 *     flush = function call pointer to function used to update screen
 *
 *  update the screen with saved dirty items
 *  
 */
bool drs_update(FLUSH_FUNC flush) {
  unsigned int i;
  bool drawn = FALSE;
  unsigned int size;
  
  // start it if not already started
  if (!drs_started)
    drs_start();
  
  // if no dirty items, no need to continue
  if (!drs_dirtychanged)
    return FALSE;
  
  // Invert the dirty area before display
  vector_resize(&drs_dirty_data.vector, 0, sizeof(drs_dirty_data.data[0]));
  vector_reserve(&drs_dirty_data.vector, REGION_CHUNK, sizeof(drs_dirty_data.data[0]));
  vector_append(&drs_dirty_data.vector, &drs_area_data, 1, sizeof(drs_dirty_data.data[0]));
  
  size = (const int) drs_dirtyinverse_data.vector.size;
  for (i = 0; i < size; i++) {
    const struct RECT *rect = &drs_dirtyinverse_data.data[i];
    
    region_clip(&drs_dirty_data, rect);
  }
  
  /* Keep at least some places allocated so updating goes fast. If this is
   * done before the actual redraw/flush call then we can call drs_dirty 
   * inside an expose event without causing the update to get lost or
   * create an infinite loop. 
   */
  vector_resize(&drs_dirtyinverse_data.vector, REGION_CHUNK, sizeof(drs_dirtyinverse_data.data[0]));
  vector_contract(&drs_dirtyinverse_data.vector, sizeof(drs_dirtyinverse_data.data[0]));
  vector_resize(&drs_dirtyinverse_data.vector, 0, sizeof(drs_dirtyinverse_data.data[0]));
  vector_append(&drs_dirtyinverse_data.vector, &drs_area_data, 1, sizeof(drs_dirtyinverse_data.data[0]));
  drs_dirtychanged = FALSE;
  
  // Update the areas one by one
  while ((size = (const int) drs_dirty_data.vector.size) != 0) {
    struct RECT rect = drs_dirty_data.data[size - 1];
    
    if (flush)
      flush(&rect);
    
    // We have to clip here instead of just remove because the buffer
    //   might have been smaller than the rect to update
    region_clip(&drs_dirty_data, &rect);
    drawn = TRUE;
  }
  
  return drawn;
}

/* drs_dirty()
 *     rect = area to mark as dirty
 *     mark = 0 = mark as clean
 *            1 = mark as dirty
 *
 *  mark an area as dirty or clean
 *  
 */
void drs_dirty(const struct RECT *rect, const bool mark) {
  // start if not already started
  if (!drs_started)
    drs_start();
  
  // Clip from the inverse means to mark as dirty
  if (mark)
    region_clip(&drs_dirtyinverse_data, rect);
  
  // Add to inverse means mark as clean
  else
    vector_append(&drs_dirtyinverse_data.vector, rect, 1, sizeof(drs_dirtyinverse_data.data[0]));
  
  drs_dirtychanged = TRUE;
}
