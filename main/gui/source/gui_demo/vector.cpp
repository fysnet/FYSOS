/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * vector.cpp
 *  
 */

#include <string.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"

/*  vector()
 *       ptr = pointer to the vector "object"
 *
 *  clears a vector object
 *   
 */
void vector(struct VECTOR *ptr) {
  ptr->data = NULL;
  ptr->size = 0;
  ptr->capacity = 0;
}

/*  vector_set()
 *       ptr = pointer to the vector "object"
 *      data = pointer to data to vector
 *         n = capacity
 *      owns = flag to indicate if we own it
 *
 *  clears a vector object
 *   
 */
void vector_set(struct VECTOR *ptr, void *data, const bit32u n, const bool owns) {
  if ((ptr->data) && (ptr->data != data))
    vector_free(ptr);

  ptr->data = data;
  ptr->size = n;
  ptr->capacity = n;

  if (!owns)
    ptr->capacity *= -1;
}

/*  vector_free()
 *       ptr = pointer to the vector "object"
 *
 *  free the data associated with vector, then clear it out
 *   
 */
void vector_free(struct VECTOR *ptr) {
  if ((ptr->capacity > 0) && ptr->data)
    free(ptr->data);
  
  ptr->data = NULL;
  ptr->size = 0;
  ptr->capacity = 0;
}

/*  vector_reserve()
 *         ptr = pointer to the vector "object"
 *     newsize = new size of the data
 *  sizeofdata = size of the data (byte, word, dword, etc)
 *
 *  resize the data or allocate a new space for it
 *   
 */
void vector_reserve(struct VECTOR *ptr, const bit32u newsize, const bit32u sizeofdata) {
  // Do we need more room
  if (newsize > (bit32u) ptr->capacity) {
    // Reallocate to make enough room
    if (ptr->capacity > 0) {
      ptr->data = realloc(ptr->data, newsize * sizeofdata);
      ptr->capacity = newsize;
      
      // We don't own the data, we must make a copy to make room
    } else {
      void *olddata = ptr->data;
      
      ptr->data = malloc(newsize * sizeofdata);
      ptr->capacity = newsize;
      memcpy(ptr->data, olddata, ptr->size * sizeofdata);
    }
  }
}

/*  vector_resize()
 *         ptr = pointer to the vector "object"
 *     newsize = new size of the data
 *  sizeofdata = size of the data (byte, word, dword, etc)
 *
 *  caller called: resize the data or allocate a new space for it
 *   
 */
void vector_resize(struct VECTOR *ptr, const bit32u newsize, const bit32u sizeofdata) {
  vector_reserve(ptr, newsize, sizeofdata);
  ptr->size = newsize;
}

/*  vector_contract()
 *         ptr = pointer to the vector "object"
 *  sizeofdata = size of the data (byte, word, dword, etc)
 *
 *  
 *   
 */
void vector_contract(struct VECTOR *ptr, const bit32u sizeofdata) {
  // we do nothing if we don't own the buffer
  if (ptr->capacity <= 0)
    return;
  
  // contract the memory just to fit the real data
  if (ptr->size) {
    ptr->data = realloc(ptr->data, ptr->size * sizeofdata);
    
    // we have no real data, only allocated room, so free everything
  } else {
    free(ptr->data);
    ptr->data = NULL;
  }
  
  ptr->capacity = ptr->size;
}

/*  vector_append()
 *         ptr = pointer to the vector "object"
 *        data = pointer to data
 *         num = count of data elements
 *  sizeofdata = size of the data element (byte, word, dword, etc)
 *   
 */
void *vector_append(struct VECTOR *ptr, const void *data, const bit32u num, const bit32u sizeofdata) {
  char *ret;
  const bit32u size = ptr->size + num;
  
  // do we need to make more room
  if (size > (bit32u) abs(ptr->capacity))
    vector_reserve(ptr, 2 * size, sizeofdata);
  
  ret = ((char *) ptr->data) + ptr->size * sizeofdata;
  
  // Copy the data over and update the size
  memcpy(ret, data, num * sizeofdata);
  ptr->size = size;
  
  return ret;
}

/*  vector_insert()
 *         ptr = pointer to the vector "object"
 *        data = pointer to data
 *         num = count of data elements
 *       place = position
 *  sizeofdata = size of the data element (byte, word, dword, etc)
 *   
 */
void *vector_insert(struct VECTOR *ptr, const void *data, const bit32u num, const bit32u place, const bit32u sizeofdata) {
  char *ret;
  const bit32u size = ptr->size + num;
  const bit32u index = (place < ptr->size) ? place : ptr->size - 1;
  const bit32u offset = index * sizeofdata;
  const bit32u databytes = num * sizeofdata;
  
  // do we need to make more room
  if (size > (bit32u) ptr->capacity)
    vector_reserve(ptr, 2 * size, sizeofdata);
  
  ret = ((char *) ptr->data) + offset;
  
  // copy the data over and update the size
  memmove(ret + databytes, ret, ptr->size * sizeofdata - offset);
  memcpy(ret, data, databytes);
  ptr->size = size;
  
  return ret;
}

/*  vector_remove()
 *         ptr = pointer to the vector "object"
 *       index = position
 *         num = count of data elements
 *  sizeofdata = size of the data element (byte, word, dword, etc)
 *   
 */
void vector_remove(struct VECTOR *ptr, const bit32u index, bit32u num, const bit32u sizeofdata) {
  if (index < ptr->size) {
    char *begin = (char *) ptr->data + index * sizeofdata;
    char *end = (char *) ptr->data + ptr->size * sizeofdata;
    const bit32u bytes = end - begin - num * sizeofdata;
    
    // put limits on the number of elements to be removed
    if ((index + num) > ptr->size)
      num = ptr->size - index;
    
    // copy remaining values down
    if (bytes)
      memmove(begin, begin + num * sizeofdata, bytes);
    
    // Adjust the size
    ptr->size -= num;
  }
}
