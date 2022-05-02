/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2022
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and if distributed, have the same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  1 May 2022
 *
 */

#include "fysos.h"

#include "./ctype/ctype.h"

#include "./stdio/stdio.h"
#include "./stdlib/spinlock.h"
#include "./string/string.h"

#include "memory.h"
#include "malloc.h"

#define MODNAME "malloc.cpp"

spinlock_t malloc_spinlock = 0;

// initialize the heap by simply allocating a single Bucket
//  with a single Pebble in it.  The pebble will be free.
HANDLE malloc_init(size_t size) {

  struct S_MEMORY_BUCKET *bucket = create_bucket(size, NULL);
  
  return bucket;
}

// allocates a linear block of memory, in 'size' bytes, and creates
//  a Bucket for this block, with one (free) Pebble.
struct S_MEMORY_BUCKET *create_bucket(size_t size, void *parent) {

  // do we allocate a minimum?
#ifdef ALLOC_MIN   
  if (size < ALLOC_MIN)
    size = ALLOC_MIN;
#endif
  
  // size must be a even number of pages
  size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

  struct S_MEMORY_BUCKET *bucket = (struct S_MEMORY_BUCKET *) mmap(size / PAGE_SIZE, MALLOC_FLAGS_NONE);
  if (bucket != NULL) {
    bucket->magic = MALLOC_MAGIC_BUCKET;
    bucket->lflags = BUCKET_FLAG_FIRST;
    bucket->size = size / PAGE_SIZE;  // count of pages used
    bucket->largest = size - sizeof(struct S_MEMORY_BUCKET) - sizeof(struct S_MEMORY_PEBBLE);

    bucket->prev = (struct S_MEMORY_BUCKET *) parent;
    bucket->next = NULL;  // no more in this direction
    
    struct S_MEMORY_PEBBLE *first = (struct S_MEMORY_PEBBLE *) ((bit8u *) bucket + sizeof(struct S_MEMORY_BUCKET));
    bucket->first = first;

    first->magic = MALLOC_MAGIC_PEBBLE;
    first->sflags = MALLOC_FLAGS_NONE;
    first->lflags = PEBBLE_FLAG_FREE;
    first->padding = 0;
    first->size = bucket->largest;
#ifdef MEM_USE_DEBUGNAME
    memset(first->name, 0, MAX_DEBUGNAME);
#endif
    first->parent = bucket;
    first->prev = NULL;
    first->next = NULL;
  }

  return bucket;
}

// insert a bucket at destination
void insert_bucket(struct S_MEMORY_BUCKET *bucket, void *destination) {
  struct S_MEMORY_BUCKET *dest = (struct S_MEMORY_BUCKET *) destination;

  if (bucket && dest) {
    bucket->next = dest->next;
    bucket->prev = dest;
    dest->next = bucket;
  }
}

// remove a bucket
void remove_bucket(struct S_MEMORY_BUCKET *bucket) {

  // don't remove the initial bucket
  if (bucket && (bucket != kernel_heap)) {
    if (bucket->prev)
      bucket->prev->next = bucket->next;
    if (bucket->next)
      bucket->next->prev = bucket->prev;
    mmap_free(bucket, bucket->size);
  }
}

// run through the bucket and get the (possibly) new largest size
size_t bucket_update_largest(struct S_MEMORY_BUCKET *bucket) {
  struct S_MEMORY_PEBBLE *p = bucket->first;
  size_t ret = 0;

  while (p != NULL) {
    if (p->size > ret)
      ret = p->size;
    p = p->next;
  }

  // update the value
  bucket->largest = ret;

  return ret;
}

// this takes an already created pebble and tries to place it in a bucket
// it is assumed that the caller has already checked that this bucket
//  isn't full and can hold the pebble, though we check anyway.
struct S_MEMORY_PEBBLE *place_pebble(struct S_MEMORY_BUCKET *bucket, struct S_MEMORY_PEBBLE *pebble) {
  struct S_MEMORY_PEBBLE *start = bucket->first;
  struct S_MEMORY_PEBBLE *best = NULL;
  size_t best_size = -1;
  
  if (bucket->lflags & BUCKET_FLAG_BEST) {
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // BEST FIT method
    // scroll through all the pebbles until we find a free one
    //  large enough to insert our pebble, but the least sized free
    //  entry that satisfies our request.
    while (start != NULL) {
      if (PEBBLE_IS_FREE(start) && (start->size <= pebble->size)) {
        if (start->size < best_size) {
          best = start;
          best_size = start->size;
        }
      }
      start = start->next;
    }
    // did we find one? Do we need to split it?
    if (best != NULL) {
      split_pebble(best, pebble->size);
      best->sflags = pebble->sflags;
      best->lflags = pebble->lflags;
#ifdef MEM_USE_DEBUGNAME
      memcpy(best->name, pebble->name, MAX_DEBUGNAME + 1);
#endif
    }
    start = best;
  } else {
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // FIRST FOUND method
    // scroll through the pebbles until we find a free one
    //  large enough to insert our pebble in.  First one found, we use.
    while (start != NULL) {
      if (PEBBLE_IS_FREE(start) && (start->size >= pebble->size)) {
        // we found one to use.  Do we need to split it?
        split_pebble(start, pebble->size);
        start->sflags = pebble->sflags;
        start->lflags = pebble->lflags;
#ifdef MEM_USE_DEBUGNAME
        memcpy(start->name, pebble->name, MAX_DEBUGNAME + 1);
#endif
        break;
      }
      start = start->next;
    }
  }

  return start;
}

// if the current pebble is large enough, will split a pebble into two
// else it returns NULL
struct S_MEMORY_PEBBLE *split_pebble(struct S_MEMORY_PEBBLE *pebble, size_t size) {
  struct S_MEMORY_PEBBLE *new_pebble = NULL;
  size_t new_size;
  
  if (SPLIT_PEBBLE(pebble->size, size)) {
    new_size = (size + (PEBBLE_MIN_ALIGN - 1)) & ~(PEBBLE_MIN_ALIGN - 1);
    new_pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) pebble + sizeof(struct S_MEMORY_PEBBLE) + new_size);
    memcpy(new_pebble, pebble, sizeof(struct S_MEMORY_PEBBLE));
    new_pebble->size = pebble->size - new_size - sizeof(struct S_MEMORY_PEBBLE);
    new_pebble->prev = pebble;
    pebble->size = new_size;
    pebble->next = new_pebble;
  }

  return new_pebble;
}

// if this pebble is empty *and* if present, the next one is empty,
//  then absorb the next one, into this one.
struct S_MEMORY_PEBBLE *absorb_next(struct S_MEMORY_PEBBLE *pebble) {
  if (pebble && pebble->next) {
    if (PEBBLE_IS_FREE(pebble) && PEBBLE_IS_FREE(pebble->next)) {
      if (pebble->parent->first == pebble->next)  // don't "delete" the Bucket->first pebble before we update it
        pebble->parent->first = pebble;
      pebble->size += pebble->next->size + sizeof(struct S_MEMORY_PEBBLE);
      pebble->next = pebble->next->next;
      if (pebble->next)
        pebble->next->prev = pebble;
      bucket_update_largest(pebble->parent);
    }
  }
  return pebble;
}

// if this pebble is empty, *and* if present the last one is empty,
//  then let the last one absorb this one.
struct S_MEMORY_PEBBLE *melt_prev(struct S_MEMORY_PEBBLE *pebble) {
  if (pebble && pebble->prev) {
    if (PEBBLE_IS_FREE(pebble) && PEBBLE_IS_FREE(pebble->prev)) {
      if (pebble->parent->first == pebble)  // don't "delete" the Bucket->first pebble before we update it
        pebble->parent->first = pebble->prev;
      pebble->prev->size += pebble->size + sizeof(struct S_MEMORY_PEBBLE);
      pebble->prev->next = pebble->next;
      if (pebble->next)
        pebble->next->prev = pebble->prev;
      pebble = pebble->prev;
      bucket_update_largest(pebble->parent);
    }
  }
  return pebble;
}

// shrink the pebble from the current size to a new smaller size
//  if the size is now small enough to split the pebble, we do it
struct S_MEMORY_PEBBLE *shrink_pebble(struct S_MEMORY_PEBBLE *pebble, size_t size) {
  struct S_MEMORY_PEBBLE *ret = NULL;

  if (pebble) {
    split_pebble(pebble, size);
    ret = pebble;
  }

  return ret;
}

#ifdef MALLOC_DEBUG
void malloc_dump(HANDLE bucket) {
  struct S_MEMORY_BUCKET *start = (struct S_MEMORY_BUCKET *) bucket;
  struct S_MEMORY_PEBBLE *pebble, *prev;
  int i, j;

  printf("\n\n");
  i = 0;
  while (start != NULL) {
    printf("Bucket: #%3i:  0x%'p\n", i, start);
    printf("         Magic: 0x%08X\n", start->magic);
    if (start->magic != MALLOC_MAGIC_BUCKET)
      break;
    printf("   local flags: 0x%08X\n", start->lflags);
    printf("  size (pages): %'i\n", start->size);
    printf("largest pebble: %'i\n", start->largest);
    printf("   prev bucket: %'p\n", start->prev);
    printf("   next bucket: %'p\n", start->next);
    printf("  first pebble: %'p\n", start->first);
    
    j = 0;
    prev = NULL;
    pebble = start->first;
    while (pebble) {
      printf("Pebble: #%3i: 0x%'p\n", j, pebble);
      printf("           Magic: 0x%08X\n", pebble->magic);
      if (pebble->magic != MALLOC_MAGIC_PEBBLE)
        break;
      printf("    source flags: 0x%08X\n", pebble->lflags);
      printf("     local flags: 0x%08X\n", pebble->lflags);
      printf("         padding: 0x%08X\n", pebble->padding);
      printf("    size (bytes): %'i\n", pebble->size);
#ifdef MEM_USE_DEBUGNAME
      printf("            name: %s\n", pebble->name);
#endif
      printf("          parent: 0x%'p  (%s)\n", pebble->parent, (pebble->parent == start) ? "good" : "error");
      printf("        previous: 0x%'p  (%s)\n", pebble->prev, (pebble->prev == prev) ? "good" : "error");
      printf("            next: 0x%'p\n", pebble->next);
      j++;
      prev = pebble;
      pebble = pebble->next;
    }

    start = start->next;
    i++;
  }
}
#endif

void *kmalloc(size_t size, bit64u alignment, bit32u flags, char *name) {
  void *ret = NULL;

  // minimum amount of memory we allocate to the caller
  if (size < PEBBLE_MIN_SIZE)
    size = PEBBLE_MIN_SIZE;

  struct S_MEMORY_PEBBLE pebble;
  pebble.magic = MALLOC_MAGIC_PEBBLE;
  pebble.sflags = flags;
  pebble.lflags = PEBBLE_FLAG_IN_USE;
  pebble.padding = 0;
  pebble.size = (size + (PEBBLE_MIN_ALIGN - 1)) & ~(PEBBLE_MIN_ALIGN - 1);
#ifdef MEM_USE_DEBUGNAME
  strncpy(pebble.name, name, MAX_DEBUGNAME);
#endif

  spin_lock(&malloc_spinlock);

  struct S_MEMORY_BUCKET *bucket = (struct S_MEMORY_BUCKET *) kernel_heap;
  while (bucket != NULL) {
    if (bucket->largest >= size) {
      ret = place_pebble(bucket, &pebble);
      bucket_update_largest(bucket);
      if (ret != NULL)
        ret = (bit8u *) ret + sizeof(struct S_MEMORY_PEBBLE);
      break;
    }
    bucket = bucket->next;
  }

  // if ret == NULL, we didn't find a bucket large enough, or with enough empty space.
  //  so allocate another bucket
  if (ret == NULL) {
    size_t new_size = size + (sizeof(struct S_MEMORY_BUCKET) + sizeof(struct S_MEMORY_PEBBLE));
    bucket = create_bucket(new_size, NULL);
    if (bucket) {
      insert_bucket(bucket, kernel_heap);
      ret = place_pebble(bucket, &pebble);
      bucket_update_largest(bucket);
      if (ret != NULL)
        ret = (bit8u *) ret + sizeof(struct S_MEMORY_PEBBLE);
    }
  }

  spin_unlock(&malloc_spinlock);

  // if we are to clear the memory, do it now
  if (ret && (flags & MALLOC_FLAGS_CLEAR))
    memset(ret, 0, size);
  
  return ret;
}

void *realloc(void *ptr, size_t size) {
  struct S_MEMORY_PEBBLE *pebble;
  void *ret = NULL;
  
  if (size == 0) {
    mfree(ptr);
    return NULL;
  }
  
  if (ptr == NULL)
    return kmalloc(size, 0 /* not used */, MALLOC_FLAGS_NONE, MODNAME);

  spin_lock(&malloc_spinlock);

  pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) ptr - sizeof(struct S_MEMORY_PEBBLE));

  spin_unlock(&malloc_spinlock);

  if (size <= pebble->size)
    ret = shrink_pebble(pebble, size);
  else {
    if (pebble->sflags & MALLOC_FLAGS_ALIGNED)
      ret = NULL;
    else {
      // the new requested size is larger than the current pebble, so allocate a new space
      ret = kmalloc(size, 0 /* not used */, pebble->sflags, MODNAME);
      if (ret)
        memcpy(ret, ptr, size);
      mfree(ptr);
    }
  }

  return ret;
}

// free a pebble, possibly merging it with a neighbor(s), and possible removing this
//  now empty Bucket.
void mfree(void *ptr) {

  if (ptr == NULL)
    return;

  spin_lock(&malloc_spinlock);

  struct S_MEMORY_PEBBLE *pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) ptr - sizeof(struct S_MEMORY_PEBBLE));
  pebble->lflags = PEBBLE_FLAG_FREE;

  pebble = melt_prev(pebble);
  absorb_next(pebble);

  // if this empties the bucket, shall we remove the bucket?
  struct S_MEMORY_BUCKET *bucket = pebble->parent;
  if (PEBBLE_IS_FREE(bucket->first) && (bucket->first->next == NULL))
    remove_bucket(bucket);
  else
    bucket_update_largest(bucket);
  
  spin_unlock(&malloc_spinlock);
}
