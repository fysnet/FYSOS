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
 *  drs.cpp
 *  
 *  This is used to keep track of the items that are dirty, then pushing them to the
 *   screen at certain times.
 *  We don't want to constantly be updating the screen since it will slow down the
 *   other processes and may add flicker to the display.
 *
 *  Last updated: 17 July 2020
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
