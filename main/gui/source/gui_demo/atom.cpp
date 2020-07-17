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
 *  atom.cpp
 *  
 *   This file is used to allocate and delete an ATOM, the core structure of all Objects
 *
 *  Last updated: 17 July 2020
 */

#include <string.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"

/*  obj_atom()
 *        atom = NULL = allocate the memory here
 *             ! NULL = memory is already allocated/static
 *    destruct = pointer to a function to call when freeing the memory
 *    datasize = minimum amount of memory needed for object.
 *
 *  Base creation of an object, the atom
 *  This will allocate the memory if atom == NULL
 *
 */
void *obj_atom(struct ATOM *atom, ATOM_DESTRUCT destruct, bit32u datasize) {
  bool allocated = FALSE;
  
  // be sure at least the size of this ATOM, though the
  //  caller should have this size much larger
  if (datasize < sizeof(struct ATOM))
    datasize = sizeof(struct ATOM);
  
  // if atom == NULL, we will allocate the memory block here
  if (atom == NULL) {
    atom = (struct ATOM *) malloc(datasize);
    allocated = TRUE;
  }
  
  // clear the memory and set the two flags
  memset(atom, 0, datasize);
  atom->destruct = destruct;
  atom->allocated = allocated;
  
  // return a pointer to the memory
  return atom;
}

/* atom_delete()
 *    atom = pointer to the memory block we need to detele (free)
 *
 * This will free all memory/objects linked below this atom and then
 *   free this atom on completion.
 * If memory was allocated (allocated == TRUE), it will free it.
 *
 */
void atom_delete(struct ATOM *atom) {
  // we at least need a pointer to the atom
  if (!atom)
    return;
  
  // if we are already deleting this group, don't race-condition and try again
  if (atom->deleting)
    return;
  
  // mark as deleting so we don't get in a race condition and loop forever
  atom->deleting = TRUE;
  
  // if we have locked this atom, don't free it
  //  (lock == 0 is not locked, otherwise any value greater than zero is locked)
  if (atom->lock)
    return;
  
  // if we gave a destruct parameter value other than null, call it for this atom
  if (atom->destruct) {
    ATOM_DESTRUCT destruct = atom->destruct;
    
    // null the parameter so we don't race-condition here either
    atom->destruct = NULL;
    
    // call it
    destruct(atom);
  }
  
  // if it was allocated, free it
  if (atom->allocated)
    free(atom);
}

/*  atom_lock()
 *    atom = pointer to the memory block we need to detele (free)
 *
 *  Lock the atom so it can not be deleted.
 *  We increment it so that we can call it from multiple functions without
 *    letting one function unlock it when another function needs it locked.
 *  Sort of a stack type lock.  If greater than zero, it is locked.
 */
void atom_lock(struct ATOM *atom) {
  atom->lock++;
}

/* atom_unlock()
 *    atom = pointer to the atom object to unlock
 *
 *  if we unlock it, (lock now == 0), and deleting == TRUE, lets free it
 */
bool atom_unlock(struct ATOM *atom) {
  bool didit = FALSE;
  
  // decrement the lock counter
  if (atom->lock)
    atom->lock--;
  
  // if we decrement to zero *and* we are deleting it (marked with atom_delete()),
  //  lets free it.
  if ((atom->lock == 0) && atom->deleting) {
    if (atom->destruct) {
      ATOM_DESTRUCT destruct = atom->destruct;
      
      // null the parameter so we don't race-condition here either
      atom->destruct = NULL;

      // call it
      destruct(atom);
    }

    // if it was allocated, free it
    if (atom->allocated)
      free(atom);
    
    // return that we successfully freed it
    didit = TRUE;
  }
  
  // return status
  return didit;
}
