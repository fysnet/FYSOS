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
 *   for non-profit only and if distributed, have the same requirements.
 *  Any project for profit that uses this code must have written 
 *   permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update: 16 Feb 2025
 *
 */


#include "malloc.h"

#define MODNAME "bucket.cpp"


#define MALLOC_FLAGS_VIRTUAL    (1 << 0)   // any memory, physical or linear, non contiguous, any location
#define MALLOC_FLAGS_PHYSICAL   (1 << 1)   // must be physical address and contiguous, inclusive
#define MALLOC_FLAGS_CLEAR      (1 << 2)   // clear the memory on return
#define MALLOC_FLAGS_LOW1MEG    (1 << 3)   // must be before the 1 Meg mark, inclusive
#define MALLOC_FLAGS_LOW16MEG   (1 << 4)   // must be before the 16 Meg mark, inclusive
#define MALLOC_FLAGS_LOW4GIG    (1 << 5)   // must be before the 4 Gig mark, inclusive
#define MALLOC_FLAGS_ANYWHERE   (1 << 6)   // can be anywhere in the 32-bit or 64-bit memory range
#define MALLOC_FLAGS_ALIGNED    (1 << 7)   // must be aligned.  The aligned parameter is now used, else it is ignored.
#define MALLOC_HARDWARE32       (MALLOC_FLAGS_LOW4GIG | MALLOC_FLAGS_PHYSICAL | MALLOC_FLAGS_CLEAR)
#define MALLOC_HARDWARE64       (                       MALLOC_FLAGS_PHYSICAL | MALLOC_FLAGS_CLEAR)

//  A few comments about the code:
// there are a few non-standard typedef's within this code.
// for example, the HANDLE typedef is simply a 'void *'.
// a few others in malloc.cpp are bit64u and bit32u.  These are simply 64-bit and 32-bit unsigned integers.
// in fact, the double-forwardslash found at the beginning of this line is considered non-standard.
//  therefore, you might wish to modify it to use /*  and   */

// kernel's memory heap
HANDLE kernel_heap = NULL;

int main( ... ) {

  kernel_heap = malloc_init( some size in bytes );

  void *ptr = malloc(4096, 0 /* no alignment */, MALLOC_FLAGS_VIRTUAL, MODNAME);
  if (ptr) {
    // do something with it

    mfree(ptr);
  }

  return 0;
}

// this function allocates 'size' pages of virtual memory from the system
void *mmap(size_t size) {
  void *ptr = NULL;

  ///// If you are a beginner starting your kernel:
  // if you don't have a virtual memory system, or any other memory allocator
  //  you can simply set-aside a region of physical memory for this.
  // For example, if you set-aside the physical memory range:
  //   0x00000000_01000000 -> 0x00000000_01FFFFFF  (16-meg to 32-meg)
  // and simply return:
  //  return (void *) 0x00000000_01000000;
  // this heap allocator will use a 16 meg Bucket.
  //
  // As long as you can guarantee that this will be enough,
  //  meaning that this call will never be called twice, you
  //  can use this allocator without the mmap() functionality.
  // i.e.: If 16 meg will be enough for all memory allocation for your
  //  kernel, simply return a value as above.
  // however, if it isn't and the call is called twice, the second
  //  call will overwrite anything the first call created...
  
  
  
  
  // TODO: call the system to get 'size' pages of virtual memory

  
  return ptr;
}

// this function free's the allocated virtual memory
void mmap_free(void *ptr, size_t size) {

  // free ptr from virtual memory

}

void spin_lock(spinlock_t lock) {

  // creates a spinlock
  
  // if your system is single tasking/threading, you can simply return.
  // (no need for a spinlock)

}

void spin_unlock(spinlock_t lock) {

  // releases a spinlock

  // if your system is single tasking/threading, you can simply return.
  // (no need for a spinlock)

}
