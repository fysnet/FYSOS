/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * atom.cpp
 * 
 *   This file is used to allocate and delete an ATOM, the core structure of all Objects
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
