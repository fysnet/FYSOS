/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * region.cpp
 *  
 */

#include "../include/ctype.h"
#include "gui.h"

/*  region_clip()
 *       region = pointer to region (it is a union of a vector)
 *      therect = rect of area
 *
 *   clip a region
 *   
 */
void region_clip(union REGION *region, const struct RECT *therect) {
  bit32u i;
  bit32u num = (const int) region->vector.size;
  struct RECT rect = *therect;
  
  if (!RECT_VALID(rect))
    return;
  
  // Make a list of intersecting rectangles
  for (i = 0; i < num; i++) {
    struct RECT s = region->data[i];
    struct RECT trial;
    
    RECT_INTERSECT(s, rect, trial);
    if (RECT_VALID(trial)) {
      // Remove the intersecting rectangles
      vector_remove(&region->vector, i, 1, sizeof(region->data[0]));
      i--, num--;
      
      // add to a temp list of the clipped rectangles
      TOP_RECTS(s, rect, trial);
      if (RECT_VALID(trial))
         vector_append(&region->vector, &trial, 1, sizeof(region->data[0]));
      
      LEFT_RECTS(s, rect, trial);
      if (RECT_VALID(trial))
         vector_append(&region->vector, &trial, 1, sizeof(region->data[0]));
      
      RIGHT_RECTS(s, rect, trial);
      if (RECT_VALID(trial))
         vector_append(&region->vector, &trial, 1, sizeof(region->data[0]));
      
      BOT_RECTS(s, rect, trial);
      if (RECT_VALID(trial))
         vector_append(&region->vector, &trial, 1, sizeof(region->data[0]));
    }
  }
}
