/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * rectatom.cpp
 * 
 *   This file is used to allocate a RECTATOM, the semi-core structure of all Objects
 */

#include "gui.h"

/*  obj_rectatom()
 *    rectatom = NULL = allocate the memory here
 *             ! NULL = memory is already allocated/static
 *    destruct = pointer to a function to call when freeing the memory
 *    datasize = minimum amount of memory needed for object.
 *
 *  calls the base creation of an object, the atom
 *
 */
void *obj_rectatom(struct RECTATOM *rectatom, ATOM_DESTRUCT destruct, bit32u datasize) {
  if (datasize < sizeof(struct RECTATOM))
    datasize = sizeof(struct RECTATOM);
  
  return obj_atom(&rectatom->base.atom, destruct, datasize);
}

/*  rectatom_place()
 *    rectatom = target rectatom to update
 *        rect = source RECT to copy to recatom->rect
 *
 *  copies the contents of rect to rectatom->rect
 *
 */
void rectatom_place(struct RECTATOM *rectatom, const struct RECT *rect) {
  rectatom->rect = *rect;
}
