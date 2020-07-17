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
 *  region.cpp
 *
 *  Last updated: 17 July 2020
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
