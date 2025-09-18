/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2025
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and if distributed, have these same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update: 17 Sept 2025
 *
 */

#include "fysos.h"

#include "./ctype/ctype.h"

#include "./cpu/cpu.h"
#include "./stdio/stdio.h"
#include "./stdlib/spinlock.h"
#include "./stdlib/stdlib.h"
#include "./string/string.h"

#include "memory.h"
#include "malloc.h"

#define MODNAME "malloc.cpp"

// define this to allow the debug printing stuff
#define MALLOC_DEBUG

// initialize the heap by simply allocating a single Bucket
//  with a single Pebble in it.  The pebble will be free.
HANDLE malloc_init(size_t size) {

  struct S_MEMORY_BUCKET *bucket = create_bucket(size, MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PERSISTENT);

  return bucket;
}

// allocates a physical/linear block of memory, in 'size' bytes, and creates
//  a Bucket for this block, with one (free) Pebble.
struct S_MEMORY_BUCKET *create_bucket(size_t size, const bit32u flags) {
  struct S_MEMORY_PEBBLE *first;

  // do we allocate a minimum?
#ifdef ALLOC_MIN   
  if (size < ALLOC_MIN)
    size = ALLOC_MIN;
#endif
  
  // size must be a multiple of a page
  size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

  // if the MALLOC_FLAGS_PHYSICAL flag is given, prepend a page to the size
  if (flags & MALLOC_FLAGS_PHYSICAL)
    size += PAGE_SIZE;

  // mmap will watch for the PHYSICAL and limit flags.
  struct S_MEMORY_BUCKET *bucket = (struct S_MEMORY_BUCKET *) mmap(size / PAGE_SIZE, flags);
  if (bucket != NULL) {
    bucket->magic = MALLOC_MAGIC_BUCKET;
    bucket->lflags = (flags << 8) | BUCKET_FLAG_FIRST;
    bucket->size = size / PAGE_SIZE;  // count of pages used
    bucket->largest = size - sizeof(struct S_MEMORY_BUCKET) - sizeof(struct S_MEMORY_PEBBLE);
    bucket->prev = NULL;  // these get assigned by insert_bucket()
    bucket->next = NULL;
    
    // assume just after the bucket
    first = (struct S_MEMORY_PEBBLE *) ((bit8u *) bucket + sizeof(struct S_MEMORY_BUCKET));
    bucket->first = first;

    // the Pebble
    first->magic = MALLOC_MAGIC_PEBBLE;
    first->lflags = ((flags & MALLOC_FLAGS_ALIGNED) ? PEBBLE_FLAG_ALIGNED : 0) | PEBBLE_FLAG_FREE;
    first->alignment = 1;  // at this time, the alignment value is not used.
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
// 'bucket' is the new bucket to insert
// 'destination' is the existing bucket
void insert_bucket(struct S_MEMORY_BUCKET *bucket, void *destination) {
  struct S_MEMORY_BUCKET *dest = (struct S_MEMORY_BUCKET *) destination;
  
  if (bucket && dest) {
    spin_lock(&dest->spinlock);
    UPDATE_NODE(bucket, dest);
    spin_unlock(&dest->spinlock);
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
// we can assume, by the checks that the caller made, that there is at
//  least one pebble within this bucket for this pebble to fit, even with
//  alignment attributes and all.
struct S_MEMORY_PEBBLE *place_pebble(struct S_MEMORY_BUCKET *bucket, struct S_MEMORY_PEBBLE *pebble) {
  struct S_MEMORY_PEBBLE *start = bucket->first;
  struct S_MEMORY_PEBBLE *best = NULL;
  struct S_MEMORY_PEBBLE *ret = NULL;
  size_t needed_size, best_size = -1;

  // calculate the size we need (maybe the ALIGNMENT flag was used)
  needed_size = (pebble->lflags & PEBBLE_FLAG_ALIGNED) ? pebble->size + pebble->alignment + sizeof(struct S_MEMORY_PEBBLE) : pebble->size;
  
  if (bucket->lflags & BUCKET_FLAG_BEST) {
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // BEST FIT method
    // scroll through all the pebbles until we find a free one
    //  large enough to insert our pebble, but the least sized free
    //  entry that satisfies our request.
    while (start != NULL) {
      if ((PEBBLE_IS_FREE(start) && (start->size <= needed_size))) {
        if (start->size < best_size) {
          best = start;
          best_size = start->size;
        }
      }
      start = start->next;
    }
    // did we find one? Do we need to split it?
    if (best != NULL) {
      best = split_pebble(best, pebble);
      best->lflags = pebble->lflags;
#ifdef MEM_USE_DEBUGNAME
      memcpy(best->name, pebble->name, MAX_DEBUGNAME + 1);
#endif
      ret = best;
    }
  } else {
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // FIRST FOUND method
    // scroll through the pebbles until we find a free one
    //  large enough to insert our pebble in.  First one found, we use.
    while (start != NULL) {
      if (PEBBLE_IS_FREE(start) && 
         (start->size >= needed_size)) {
        // we found one to use.  Do we need to split it?
        start = split_pebble(start, pebble);
        start->lflags = pebble->lflags;
#ifdef MEM_USE_DEBUGNAME
        memcpy(start->name, pebble->name, MAX_DEBUGNAME + 1);
#endif
        ret = start;
        break;
      }
      start = start->next;
    }
  }

  return ret;
}

// if the current pebble is large enough, will split a pebble into two
// it returns the pebble we want to modify
//  (if not alignment, it returns the original split pebble)
//  (if alignment, it may return the second pebble, the one after the split)
struct S_MEMORY_PEBBLE *split_pebble(struct S_MEMORY_PEBBLE *this_pebble, struct S_MEMORY_PEBBLE *src) {
  struct S_MEMORY_PEBBLE *dummy, *new_pebble;
  size_t new_size, org_size;
  bool is_aligned = FALSE;
  void *pos;

  // if the current pebble is already aligned to the requested alignment,
  //  mark it as aligned and skip the alignment tri-pebble below
  if ((src->lflags & PEBBLE_FLAG_ALIGNED) && 
      (((size_t) ((bit8u *) this_pebble + sizeof(struct S_MEMORY_PEBBLE)) & ((size_t) src->alignment - 1)) == 0)) {
    this_pebble->alignment = src->alignment;
    is_aligned = TRUE;
  }

  // is the new pebble to be aligned?
  // if it is, we can assume that the found is large enough
  //  to split and align the new pebble due to the caller
  //  making all of the checks for us
  // (skip this if the current pebble is already aligned on the requested alignment)
  if ((src->lflags & PEBBLE_FLAG_ALIGNED) && !is_aligned) {
    // there is a possibility of splitting a pebble into three new pebbles.
    // 1) at the same place, to pad up to the new aligned pebble
    // 2) the new aligned pebble, where the PEBBLE block will be at the aligned address - sizeof(PEBBLE)
    // 3) any remaining space after the aligned pebble that is not used by the aligned pebble
    
    // calculate the distance between the current pebble and the new aligned pebble
    // previous callers should have verified that there is enough room to do this, as well
    //  as leaving enough room for the first pebble.
    pos = (void *) (((size_t) this_pebble + (sizeof(struct S_MEMORY_PEBBLE) * 2) + PEBBLE_MIN_SIZE + ((size_t) src->alignment - 1)) & ~((size_t) src->alignment - 1));
    org_size = this_pebble->size;
    
    // create the new aligned pebble (i.e.: the second pebble)
    new_pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) pos - sizeof(struct S_MEMORY_PEBBLE));  // move back so the first byte of the usable memory is the byte on the alignment.
    memcpy(new_pebble, src, sizeof(struct S_MEMORY_PEBBLE));
    UPDATE_NODE(new_pebble, this_pebble);
    this_pebble->size = (size_t) ((bit8u *) new_pebble - (bit8u *) this_pebble) - sizeof(struct S_MEMORY_PEBBLE);
    org_size -= this_pebble->size;
    org_size -= (new_pebble->size + sizeof(struct S_MEMORY_PEBBLE));
    
    // if there is still a lot of room after the new pebble, we need to split it too (i.e.: the third pebble)
    if (org_size >= (sizeof(struct S_MEMORY_PEBBLE) + PEBBLE_MIN_SIZE)) {
      // we need a dummy pebble to place as the new one if it does split
      dummy = (struct S_MEMORY_PEBBLE *) ((bit8u *) new_pebble + sizeof(struct S_MEMORY_PEBBLE) + new_pebble->size);
      dummy->magic = MALLOC_MAGIC_PEBBLE;
      dummy->lflags = PEBBLE_FLAG_FREE;
      dummy->alignment = 1;
      dummy->size = org_size - sizeof(struct S_MEMORY_PEBBLE);
    #ifdef MEM_USE_DEBUGNAME
      memset(dummy->name, 0, MAX_DEBUGNAME + 1);
    #endif
      dummy->parent = this_pebble->parent;
      UPDATE_NODE(dummy, new_pebble);
    }
    
    // return the new aligned pebble
    this_pebble = new_pebble;
  } else {
    if (SPLIT_PEBBLE(this_pebble->size, src->size)) {
      new_size = (src->size + (PEBBLE_MIN_ALIGN - 1)) & ~(PEBBLE_MIN_ALIGN - 1);
      new_pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) this_pebble + sizeof(struct S_MEMORY_PEBBLE) + new_size);
      memcpy(new_pebble, this_pebble, sizeof(struct S_MEMORY_PEBBLE));
      new_pebble->size = this_pebble->size - new_size - sizeof(struct S_MEMORY_PEBBLE);
      new_pebble->alignment = 1;
      new_pebble->prev = this_pebble;
      if (this_pebble->next)
        this_pebble->next->prev = new_pebble;
      this_pebble->size = new_size;
      this_pebble->next = new_pebble;
    }
  }

  return this_pebble;
}

// if this pebble is empty *and* if present, the next one is empty,
//  then absorb the next one, into this one.
struct S_MEMORY_PEBBLE *absorb_next(struct S_MEMORY_PEBBLE *pebble) {
  if ((pebble && pebble->next) &&                                   // check for NULL pointers
      (PEBBLE_IS_FREE(pebble) && PEBBLE_IS_FREE(pebble->next))) {   // both must be free
    if (pebble->parent->first == pebble->next)  // don't "delete" the Bucket->first pebble before we update it
      pebble->parent->first = pebble;
    pebble->size += pebble->next->size + sizeof(struct S_MEMORY_PEBBLE);
    pebble->next = pebble->next->next;
    if (pebble->next)
      pebble->next->prev = pebble;
    bucket_update_largest(pebble->parent);
  }
  return pebble;
}

// if this pebble is empty, *and* if present the last one is empty,
//  then let the last one absorb this one.
struct S_MEMORY_PEBBLE *melt_prev(struct S_MEMORY_PEBBLE *pebble) {
  if ((pebble && pebble->prev) &&                                   // check for NULL pointers
      (PEBBLE_IS_FREE(pebble) && PEBBLE_IS_FREE(pebble->prev))) {   // both must be free
    if (pebble->parent->first == pebble)  // don't "delete" the Bucket->first pebble before we update it
      pebble->parent->first = pebble->prev;
    pebble->prev->size += pebble->size + sizeof(struct S_MEMORY_PEBBLE);
    pebble->prev->next = pebble->next;
    if (pebble->next)
      pebble->next->prev = pebble->prev;
    pebble = pebble->prev;
    bucket_update_largest(pebble->parent);
  }
  return pebble;
}

// shrink the pebble from the current size to a new smaller size
//  if the size is now small enough to split the pebble, we do it
struct S_MEMORY_PEBBLE *shrink_pebble(struct S_MEMORY_PEBBLE *pebble, size_t size) {
  struct S_MEMORY_PEBBLE *ret = NULL;
  struct S_MEMORY_PEBBLE dummy;

  // we need a dummy pebble to place as the new one if it does split
  dummy.magic = MALLOC_MAGIC_PEBBLE;
  dummy.lflags = PEBBLE_FLAG_FREE;
  dummy.alignment = 1;
  dummy.size = size;
#ifdef MEM_USE_DEBUGNAME
  memset(dummy.name, 0, MAX_DEBUGNAME + 1);
#endif
  dummy.parent = pebble->parent;

  if (pebble) {
    split_pebble(pebble, &dummy);
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
      printf("Pebble: #%3i: 0x%'p  (0x%'p)\n", j, pebble, (size_t) pebble + sizeof(struct S_MEMORY_PEBBLE));
      printf("           Magic: 0x%08X\n", pebble->magic);
      if (pebble->magic != MALLOC_MAGIC_PEBBLE)
        break;
      printf("     local flags: 0x%08X  (%s)\n", pebble->lflags, (pebble->lflags & PEBBLE_FLAG_IN_USE) ? "in use" : "free");
      printf("       alignment: %'i\n", pebble->alignment);
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

// allocate memory trying to use the heap only.  If not, call the underlining layers.
//
//      size = bytes to allocate.  Actual may be more.
// alignment = alignment value (if used, must be a power of two)
//     flags = see memory.h
//      name = a string of characters to "mark" the memory with. (Could be disabled)
void *kmalloc(size_t size, bit32u alignment, bit32u flags, const char *name) {
  void *ret = NULL;

  // set defaults
  if ((flags & (MALLOC_FLAGS_LOW1MEG | MALLOC_FLAGS_LOW16MEG | MALLOC_FLAGS_LOW4GIG)) == 0)
    flags |= MALLOC_FLAGS_LOWANY;

  // the VIRTUAL flag or the PHYSICAL flag must be set, one or the other, not both.
  if ((flags & (MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PHYSICAL)) == (MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PHYSICAL))
    return NULL;
  // else if neither, assume VIRTUAL
  if ((flags & (MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PHYSICAL)) == 0)
    flags |= MALLOC_FLAGS_VIRTUAL;

  // minimum amount of memory we allocate to the caller
  if (size < PEBBLE_MIN_SIZE)
    size = PEBBLE_MIN_SIZE;

  // make sure the alignment is at least a certain size and must be a power of 2.
  // if making the alignment value larger, this still guarantees
  //  a requested alignmnet.  However, this code requires a min alignment.
  if (flags & MALLOC_FLAGS_ALIGNED) {
    if (alignment < PEBBLE_MIN_ALIGN)
      alignment = PEBBLE_MIN_ALIGN;
    if (!power_of_two(alignment))
      alignment = (bit32u) nearest_upper_power((size_t) alignment);
  }

  // check that the alignment size is not out of range
  if (alignment > 0x00100000)  // 1 Meg max alignment
    return NULL;

  struct S_MEMORY_PEBBLE pebble;
  pebble.magic = MALLOC_MAGIC_PEBBLE;
  pebble.lflags = ((flags & MALLOC_FLAGS_CLEAR) ? PEBBLE_FLAG_CLEARED : 0) |
                  ((flags & MALLOC_FLAGS_ALIGNED) ? PEBBLE_FLAG_ALIGNED : 0) | PEBBLE_FLAG_IN_USE;
  pebble.alignment = alignment;
  if (flags & MALLOC_FLAGS_PHYSICAL)
    pebble.size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
  else
    pebble.size = (size + (PEBBLE_MIN_ALIGN - 1)) & ~(PEBBLE_MIN_ALIGN - 1);
#ifdef MEM_USE_DEBUGNAME
  strncpy(pebble.name, name, MAX_DEBUGNAME);
#endif

  struct S_MEMORY_BUCKET *bucket = (struct S_MEMORY_BUCKET *) kernel_heap;

  // try what we have already allocated in the heap
  for ( ; bucket != NULL; bucket = bucket->next) {
    
    // lock the bucket
    spin_lock(&bucket->spinlock);

    // the bucket must match the requested virtual or physical aspect
    if ((flags & (MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PHYSICAL)) != 
        ((bucket->lflags >> 8) & (MALLOC_FLAGS_VIRTUAL | MALLOC_FLAGS_PHYSICAL))) {
      spin_unlock(&bucket->spinlock);
      continue;
    }

    if (flags & MALLOC_FLAGS_PHYSICAL) {
      // do we have a upper limit set?
      if (((flags & MALLOC_FLAGS_LOW1MEG) && !ISBELOW(bucket, ISBELOW1MEG)) ||
          ((flags & MALLOC_FLAGS_LOW16MEG) && !ISBELOW(bucket, ISBELOW16MEG)) ||
          ((flags & MALLOC_FLAGS_LOW4GIG) && !ISBELOW(bucket, ISBELOW4GIG))) {
        spin_unlock(&bucket->spinlock);
        continue;
      }
    }

    // if the alignment flag is set, we need to see if there
    //  is a pebble large enough to hold the requested size, 
    //  plus any space before it to align the request on the
    //  requested alignment.
    if ((flags & MALLOC_FLAGS_ALIGNED) && 
        (bucket->largest < (pebble.size + (alignment - 1) + sizeof(struct S_MEMORY_PEBBLE)))) {
      spin_unlock(&bucket->spinlock);
      continue;
    }

    // we made it past all of the checks and have found a free pebble that
    //  has enough room, is under our limit, and past other checks.
    //  The place_pebble() should now be able to place the pebble using
    //   pebble.lflags accordingly.
    if (bucket->largest >= pebble.size) {
      pebble.parent = bucket;
      ret = place_pebble(bucket, &pebble);
      if (ret != NULL) {
        bucket_update_largest(bucket);
        ret = (bit8u *) ret + sizeof(struct S_MEMORY_PEBBLE);
        spin_unlock(&bucket->spinlock);
        break;
      }
    }

    // else, loop and move to the next bucket?
    spin_unlock(&bucket->spinlock);
  }

  // if ret == NULL, 
  //  - we didn't find a bucket large enough, or with enough empty space,  or
  //  - we are wanting an alignment that couldn't be satisfied above,      or
  //  - we are wanting physical memory,
  //  so allocate another bucket.  It will use the lflags to (hopefully) satisfy
  //   the request.
  if (ret == NULL) {
    size_t new_size = size + (sizeof(struct S_MEMORY_BUCKET) + sizeof(struct S_MEMORY_PEBBLE));
    if (flags & MALLOC_FLAGS_ALIGNED) // size must accommodate the alignment too
      new_size += alignment + sizeof(struct S_MEMORY_PEBBLE);
    bucket = create_bucket(new_size, flags);
    if (bucket) {
      insert_bucket(bucket, kernel_heap);
      spin_lock(&bucket->spinlock);
      pebble.parent = bucket;
      ret = place_pebble(bucket, &pebble);
      bucket_update_largest(bucket);
      spin_unlock(&bucket->spinlock);
      if (ret != NULL)
        ret = (bit8u *) ret + sizeof(struct S_MEMORY_PEBBLE);
    }
  }

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
    return kmalloc(size, 0 /* not used */, MALLOC_FLAGS_VIRTUAL, MODNAME);

  pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) ptr - sizeof(struct S_MEMORY_PEBBLE));
  if (pebble->magic != MALLOC_MAGIC_PEBBLE)
    return NULL;

  if (size <= pebble->size) {
    spin_lock(&pebble->parent->spinlock);
    ret = (void *) ((bit8u *) shrink_pebble(pebble, size) + sizeof(struct S_MEMORY_PEBBLE));
    spin_unlock(&pebble->parent->spinlock);
  } else {
    // the new requested size is larger than the current pebble, so allocate a new space
    ret = kmalloc(size, pebble->alignment, pebble->lflags, MODNAME);
    if (ret)
      memcpy(ret, ptr, size);
    mfree(ptr);
  }

  return ret;
}

// free a pebble, possibly merging it with a neighbor(s), and possible removing this
//  now empty Bucket.
void mfree(void *ptr) {

  if (ptr == NULL)
    return;

  struct S_MEMORY_PEBBLE *pebble = (struct S_MEMORY_PEBBLE *) ((bit8u *) ptr - sizeof(struct S_MEMORY_PEBBLE));

  // check that it actually is a pebble
  if (pebble->magic != MALLOC_MAGIC_PEBBLE)
    return;

  spin_lock(&pebble->parent->spinlock);

  // mark it as free
  pebble->lflags = PEBBLE_FLAG_FREE;
  pebble->alignment = 1;

  // see if we can absorb any of the neighbors
  pebble = melt_prev(pebble);
  absorb_next(pebble);

  spin_unlock(&pebble->parent->spinlock);

  // if this empties the bucket, shall we remove the bucket?
  struct S_MEMORY_BUCKET *bucket = pebble->parent;
  if (!((bucket->lflags >> 8) & MALLOC_FLAGS_PERSISTENT)) {
    if (PEBBLE_IS_FREE(bucket->first) && (bucket->first->prev == NULL) && (bucket->first->next == NULL))
      remove_bucket(bucket);
    else
      bucket_update_largest(bucket);
  }
}
