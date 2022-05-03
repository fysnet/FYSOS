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

#ifndef MALLOC_H
#define MALLOC_H

// define this to allow the debug printing stuff
#define MALLOC_DEBUG

// this is the minimum allocation size
#define ALLOC_MIN  (65536 + sizeof(struct S_MEMORY_BUCKET) + sizeof(struct S_MEMORY_PEBBLE))

// define if we want to add the debug name to the tag
#define MEM_USE_DEBUGNAME
#ifdef MEM_USE_DEBUGNAME
  #define MAX_DEBUGNAME     31  // max length of a debug name not counting ending null
#endif

// magic value for a tag
#define MALLOC_MAGIC_BUCKET 'BUCK'
#define MALLOC_MAGIC_PEBBLE 'ROCK'

// local flags for a bucket (only applies to this bucket)
#define BUCKET_FLAG_FIRST   (0 <<  0)  // if clear, use first find method
#define BUCKET_FLAG_BEST    (1 <<  0)  // if set, use best fit method

// local flags for a pebble
#define PEBBLE_FLAG_FREE    (0 <<  0)  // if set, is in use, if clear, free for use
#define PEBBLE_FLAG_IN_USE  (1 <<  0)  //  ...


#define PEBBLE_MIN_ALIGN 64  // minimum power of 2 to align the next pebble (1 or a power of 2)
#define PEBBLE_MIN_SIZE  64  // a pebble must be at least this size
#if PEBBLE_MIN_SIZE > PEBBLE_MIN_ALIGN
  #error "PEBBLE_MIN_ALIGN must be at least PEBBLE_MIN_SIZE"
#endif

// macro to see if the free chunk is large enough to split
//                                 if ceil(current pebble size, 64)              >   new pebble size with a remainder < (sizeof(PEBBLE) + PEBBLE_MIN_SIZE)
#define SPLIT_PEBBLE(s0, s1) ((((s0) + PEBBLE_MIN_ALIGN - 1) & ~PEBBLE_MIN_SIZE) > ((s1) + sizeof(struct S_MEMORY_PEBBLE) + PEBBLE_MIN_SIZE))

#define PEBBLE_IS_FREE(p) (((p)->lflags & PEBBLE_FLAG_IN_USE) == PEBBLE_FLAG_FREE)


#pragma pack(push, 1)

struct S_MEMORY_PEBBLE {
  bit32u magic;         // a pebble in a bucket
  bit32u lflags;        // local flags for this pebble
  bit32u sflags;        // sent flags for this pebble
  bit32u padding;       // padding/alignment
  size_t size;          // count of bytes requested
#ifdef MEM_USE_DEBUGNAME
  char  name[MAX_DEBUGNAME + 1];
#endif
  struct S_MEMORY_BUCKET *parent; // parent bucket of this pebble

  // linked list of pebbles
  struct S_MEMORY_PEBBLE *prev;
  struct S_MEMORY_PEBBLE *next;
};

struct S_MEMORY_BUCKET {
  bit32u magic;     //  a bucket full of pebbles
  bit32u lflags;    //  local flags for this bucket
  size_t size;      //  count of 4096 pages used for this bucket
  size_t largest;   //  largest free block in this bucket

  // linked list of buckets
  struct S_MEMORY_BUCKET *prev;
  struct S_MEMORY_BUCKET *next;

  struct S_MEMORY_PEBBLE *first;
};

#pragma pack(pop)

#define malloc(s, f, n)      kmalloc((s), 1, (f) & ~MALLOC_FLAGS_CLEAR, n)
#define calloc(s, f, n)      kmalloc((s), 1, (f) | MALLOC_FLAGS_CLEAR, n)
#define amalloc(s, a, f, n)  kmalloc((s), (a), (f) & ~MALLOC_FLAGS_CLEAR, n)
#define acalloc(s, a, f, n)  kmalloc((s), (a), (f) | MALLOC_FLAGS_CLEAR, n)


// local functions
struct S_MEMORY_BUCKET *create_bucket(size_t size);
struct S_MEMORY_PEBBLE *place_pebble(struct S_MEMORY_BUCKET *bucket, struct S_MEMORY_PEBBLE *pebble);
struct S_MEMORY_PEBBLE *split_pebble(struct S_MEMORY_PEBBLE *pebble, size_t size);

// public data/functions
extern HANDLE kernel_heap;

HANDLE malloc_init(size_t size);
#ifdef MALLOC_DEBUG
  void malloc_dump(HANDLE bucket);
#endif

void *kmalloc(size_t size, bit64u alignment, bit32u flags, char *name);
void *realloc(void *ptr, size_t size); // The standard function.
void mfree(void *ptr);   // The standard function.

#endif   // MALLOC_H
