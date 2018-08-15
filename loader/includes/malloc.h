/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */

#ifndef MALLOC_H
#define MALLOC_H

//#define MEM_DEBUG_ON

#define MEM_START    0x0000BE00           // must preserve 0x07C00 -> ~33 sectors
#define MEM_END      MEMORY_MIN_REQUIRED  // 128 meg (assumed)

#define LOADER_RESV  0x30000   // size we reserve for the loader code, data, and stack (64k * 3)

#define MIN_CHUNK_SIZE  64

#define MAGIC_USED  0x55534544  // 'USED'
#define MAGIC_FREE  0x46524545  // 'FREE'

#pragma pack(push, 1)

struct MEMORY_BLOCK {
  bit32u magic;
  int    size;
  struct MEMORY_BLOCK *prev;
  struct MEMORY_BLOCK *next;
};

#pragma pack(pop)

void mem_initialize(void);

void *malloc(const int);
void *calloc(const int);
void mfree(void *);
void *mrealloc(void *, const int);

#ifdef MEM_DEBUG_ON
  void mem_print(void);
#endif


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// the following is for the retrieval of the memory map from the BIOS

#pragma pack(push, 1)

// BIOS interrupt 15h/E820 return buffer
struct S_BIOS_MEME820 {
  bit32u base[2];
  bit32u size[2];
  bit32u type;
};

struct S_MEMORY {
  bit16u   word;        // (0 = not used, 1 = E820h, 2 = 0E801h, 3 = 88h, 4 = cmos)
  bit32u   size[2];     // size of memory in bytes
  bit16u   blocks;      // count of bases returned (usually 2)
   struct S_MEMORY_BLKS {
     bit32u base[2];
     bit32u size[2];
     bit32u type;
     bit32u attrib[2];
   } block[48];
};

#pragma pack(pop)

// BIOS Memory Types
typedef enum {
  BIOSAvailable = 1,         // 01h memory, available to OS
  BIOSReservedMemoryType,    // 02h reserved, not available (e.g. system ROM, memory-mapped device)
  BIOSACPIReclaimMemory,     // 03h ACPI Reclaim Memory (usable by OS after reading ACPI tables)
  BIOSACPIMemoryNVS,         // 04h ACPI NVS Memory (OS is required to save this memory between NVS sessions)
} BIOS_MEMORY_TYPE;

bool get_memory(struct S_MEMORY *);

#endif // MALLOC_H
