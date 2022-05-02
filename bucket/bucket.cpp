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


#include "malloc.h"

#define MODNAME "bucket.cpp"


#define MALLOC_FLAGS_NONE       (1 << 0)   // any memory, physical or linear, non contiguous, any location
#define MALLOC_FLAGS_CLEAR      (1 << 1)   // clear the memory on return
#define MALLOC_FLAGS_LOW1MEG    (1 << 2)   // must be before the 1 Meg mark, inclusive
#define MALLOC_FLAGS_LOW16MEG   (1 << 3)   // must be before the 16 Meg mark, inclusive
#define MALLOC_FLAGS_LOW4GIG    (1 << 4)   // must be before the 4 Gig mark, inclusive
#define MALLOC_FLAGS_PHYSICAL   (1 << 5)   // must be physical address and contiguous, inclusive
#define MALLOC_FLAGS_ALIGNED    (1 << 6)   // must be aligned.  The aligned parameter is now used, else it is ignored.
#define MALLOC_HARDWARE32       (MALLOC_FLAGS_LOW4GIG | MALLOC_FLAGS_PHYSICAL | MALLOC_FLAGS_CLEAR)
#define MALLOC_HARDWARE64       (                       MALLOC_FLAGS_PHYSICAL | MALLOC_FLAGS_CLEAR)


// kernel's memory heap
HANDLE kernel_heap = NULL;

int main( ... ) {

  kernel_heap = malloc_init( some size in bytes );

  void *ptr = malloc(4096, 0 /* no alignment */, MALLOC_FLAGS_NONE, MODNAME);
  if (ptr) {
    // do something with it

    mfree(ptr);
  }

  return 0;
}

// this function allocates 'size' pages of virtual memory from the system
void *mmap(size_t size) {
  void *ptr = NULL;

  
  // call the system to get 'size' pages of virtual memory

  
  return ptr;
}

// this function free's the allocated virtual memory
void mmap_free(void *ptr, size_t size) {

  // free ptr from virtual memory

}

void spin_lock(spinlock_t lock) {

  // creates a spinlock

}

void spin_unlock(spinlock_t lock) {

  // releases a spinlock

}

