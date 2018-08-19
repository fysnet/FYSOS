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
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "loader.h"

#include "conio.h"
#include "malloc.h"
#include "paraport.h"
#include "stdio.h"
#include "string.h"
#include "sys.h"
#include "windows.h"

struct MEM_INIT {
  bit32u base;
  bit32u flag;
} mem_init[] = {
  // first block of memory is at MEM_START;
  { MEM_START,  MAGIC_FREE },  // base MEM_START up to Loader          (free)
  { 0,          MAGIC_USED },  // base of LOADSEG to size of loader    (used)
  { 0,          MAGIC_FREE },  // base just after loader to 0x00080000 (free)
  { 0x0009FC00, MAGIC_USED },  // base of 0x00080000 to 1 meg          (used) (EBDA)
#if ALLOW_SMALL_MACHINE
  { 0x00100000, MAGIC_USED },  // base of 1meg to 7.5meg               (used) (FYSOS)
  { 0x00780000, MAGIC_FREE },  // base of 7.5meg to ???                (free)
#else
  { 0x00100000, MAGIC_USED },  // base of 1meg to 32meg                (used) (FYSOS)
  { 0x02000000, MAGIC_FREE },  // base of 32meg to ???                 (free)
#endif
  { MEM_END, 0 }
};

void mem_initialize(void) {
  struct MEMORY_BLOCK *mcb = (struct MEMORY_BLOCK *) MEM_START;
  int i = 0;
  
  // first initialize the base address of loader.sys
  mem_init[1].base = sys_block.boot_data.loader_base;
  mem_init[2].base = sys_block.boot_data.loader_base + LOADER_RESV;
  
  while (mem_init[i].base < MEM_END) {
    mcb->magic = mem_init[i].flag;
    mcb->size = mem_init[i+1].base - (bit32u) mcb - sizeof(struct MEMORY_BLOCK);
    mcb->prev = (i == 0) ? 0 : mem_init[i-1].base;
    mcb->next = (mem_init[i+1].base == MEM_END) ? 0 : mem_init[i+1].base;
    mcb = mcb->next;
    i++;
  }
}

void malloc_split_block(struct MEMORY_BLOCK *mcb, const int size) {
  struct MEMORY_BLOCK *new_next;
  
  new_next = (struct MEMORY_BLOCK *) ((bit32u) mcb + size + sizeof(struct MEMORY_BLOCK));
  new_next->magic = MAGIC_FREE;
  new_next->next = mcb->next;
  if (mcb->next)
    mcb->next->prev = new_next;
  new_next->prev = mcb;
  new_next->size = mcb->size - size - sizeof(struct MEMORY_BLOCK);
  mcb->next = new_next;
  mcb->size = size;
}

void *malloc(const int size) {
  struct MEMORY_BLOCK *mcb = (struct MEMORY_BLOCK *) MEM_START, *new_next;
  int our_size = (size < MIN_CHUNK_SIZE) ? MIN_CHUNK_SIZE : size;
  our_size = (our_size + 15) & ~15;  // make them paragraph sized
  
  do {
    if ((mcb->magic == MAGIC_FREE) && (mcb->size >= our_size)) {
      if (mcb->size >= (our_size + MIN_CHUNK_SIZE))
        malloc_split_block(mcb, our_size);
      mcb->magic = MAGIC_USED;
      return (void *) ((bit32u) mcb + sizeof(struct MEMORY_BLOCK));
    }
    mcb = mcb->next;
  } while (mcb);
  
  if (main_win)
    win_printf(main_win, "Could not allocate any more memory... (%i)\n\n", size);
  else {
    asm ( "  mov  ax,0003h  \n"
          "  int  10h  \n" );
    puts("Could not allocate any more memory...");
  }
  freeze();
  return NULL;  
}

void *calloc(const int size) {
  void *ptr = malloc(size);
  if (ptr)
    memset(ptr, 0, size);
  return ptr;
}

void mfree(void *ptr) {
  struct MEMORY_BLOCK *mcb = (struct MEMORY_BLOCK *) ((bit32u) ptr - sizeof(struct MEMORY_BLOCK));
  
  if (mcb->magic == MAGIC_USED) {
    mcb->magic = MAGIC_FREE;
    if (mcb->next && (mcb->next->magic == MAGIC_FREE)) {
      mcb->size = mcb->size + mcb->next->size + sizeof(struct MEMORY_BLOCK);
      mcb->next = mcb->next->next;
      mcb->next->prev = mcb;
    }
    if (mcb->prev && (mcb->prev->magic == MAGIC_FREE)) {
      mcb->prev->size = mcb->prev->size + mcb->size + sizeof(struct MEMORY_BLOCK);
      mcb->prev->next = mcb->next;
      mcb->next->prev = mcb->prev;
    }
  } else {
    asm ( "  mov  ax,0003h  \n"
          "  int  10h  \n" );
    puts("Error in mfree()");
    freeze();
  }
}

void *mrealloc(void *ptr, const int size) {
  struct MEMORY_BLOCK *mcb = (struct MEMORY_BLOCK *) ((bit32u) ptr - sizeof(struct MEMORY_BLOCK));
  void *targ;
  
  if (ptr == NULL)
    return malloc(size);
  
  if (size <= mcb->size) {
    if (size < (mcb->size - MIN_CHUNK_SIZE))
      malloc_split_block(mcb, size);
    return ptr;
  } else {
    targ = malloc(size);
    memcpy(targ, ptr, mcb->size);
    mfree(ptr);
    return targ;
  }
}

#ifdef MEM_DEBUG_ON
// remember that this allocates memory itself to printf()
void mem_print(void) {
  struct MEMORY_BLOCK *mcb = (struct MEMORY_BLOCK *) MEM_START;
  int i = 0;
  bit32u total = 0;
  
  if (main_win)
    win_printf(main_win, "--- Memory Dump ---\n");
  else
    puts("--- Memory Dump ---");
  do {
    if (main_win) {
      win_printf(main_win, "mcb %2i: 0x%08X, %s, size= %9u, prev= 0x%08X, next= 0x%08X\n", 
        i, (bit32u) mcb,
        (mcb->magic == MAGIC_FREE) ? "free" : "used",
        mcb->size, mcb->prev, mcb->next);
    } else {
      printf("mcb %2i: 0x%08X, %s, size= %9u, prev= 0x%08X, next= 0x%08X\n", 
        i, (bit32u) mcb,
        (mcb->magic == MAGIC_FREE) ? "free" : "used",
        mcb->size, mcb->prev, mcb->next);
    }
    total += (mcb->size + sizeof(struct MEMORY_BLOCK)); // we include the MCB too
    mcb = mcb->next;
    i++;
  } while (mcb);  
  if (main_win)
    win_printf(main_win, "--- Done ---                    %9u (%iM)\n", total, (total >> 20));
  else
    printf("--- Done ---                    %9u (%iM)\n", total, (total >> 20));
}

#endif

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// the following is for the retrieval of the memory map from the BIOS

// get the memory size from the bios (int 15h/e820/e801/88 or cmos)
bool get_memory(struct S_MEMORY *memory) {
  bit32u temp[2];
  int  i;
  bool r, present;
  struct REGS regs;
  bit8u local[256];
  struct S_BIOS_MEME820 *bios_memE820 = (struct S_BIOS_MEME820 *) local;
  
  // clear it out
  memset(memory, 0, sizeof(struct S_MEMORY));
  memory->size[0] = 0x00100000;  // start with 1 meg
  
  if (spc_key_F2)
    para_printf(" Retrieving the memory map...\n"
                " Starting with service E820h.\n");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now try the most recent/best service
  present = FALSE;
  regs.ebx = 0;
  do {
    regs.eax = 0x0000E820;
    regs.edx = 0x534D4150;  // 'SMAP'
    regs.ecx = sizeof(struct S_BIOS_MEME820);
    regs.edi = MK_OFF((bit32u) local);
    regs.es = MK_SEG((bit32u) local);
    if (intx(0x15, &regs) || (regs.eax != 0x534D4150))
      break;
    
    if ((regs.ecx < 20) || (regs.ecx > sizeof(struct S_BIOS_MEME820))) {
      // bug in bios - all returned descriptors must be
      // at least 20 bytes long, and cannot be larger then
      // the input buffer size sent in ecx.
      if (spc_key_F2)
        para_printf("Bug in E820 BIOS.  Descriptor != 20 bytes in length.\n");
      break;
    }
    
    if (spc_key_F2)
      para_printf(" Addr: 0x%08X, size: %12u, type: %i\n",
        bios_memE820->base[0], bios_memE820->size[0], bios_memE820->type);

// printf("\xB3 Addr: 0x%08X_%08X, size: 0x%08X_%08X, type: %i\n",
//   bios_memE820->base[1], bios_memE820->base[0],
//   bios_memE820->size[1], bios_memE820->size[0],
//   bios_memE820->type);
    
    present = TRUE;
    
    memory->block[memory->blocks].base[0] = bios_memE820->base[0];
    memory->block[memory->blocks].base[1] = bios_memE820->base[1];
    memory->block[memory->blocks].size[0] = bios_memE820->size[0];
    memory->block[memory->blocks].size[1] = bios_memE820->size[1];
    memory->block[memory->blocks].type = bios_memE820->type;
    memory->block[memory->blocks].attrib[0] = 0;
    memory->block[memory->blocks].attrib[1] = 0;
    
    // add to the accumulator
    add64(memory->size, memory->block[memory->blocks].size);
    
    memory->word = 1; // (our) type goes here
    memory->blocks++;
  } while (regs.ebx != 0);

  if (spc_key_F2)
    para_printf("                  Total: %12u\n", memory->size[0]);
  
  if (present && (memory->blocks > 0))
    goto check_mem_limits;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if service E820 didn't work, try service E881 and E801
  // first, clear it out (again).
  memset(memory, 0, sizeof(struct S_MEMORY));
  
  if (spc_key_F2)
    para_printf("Trying service E881 and E801.\n");
  
  // service E881 is identical to E801 except that E881 returns
  //  the values as 32-bits while E801 returns them as 16-bits
  regs.eax = 0x00000E881;
  if (r = intx(0x15, &regs)) {
    regs.eax = 0x00000E801;
    if (!(r = intx(0x15, &regs))) {
      // clear out high word for the code below
      regs.eax &= 0xFFFF;
      regs.ebx &= 0xFFFF;
      regs.ecx &= 0xFFFF;
      regs.edx &= 0xFFFF;
    }
  }
  
  if (!r) {
    regs.eax <<= 10;  // in K's
    regs.ebx <<= 16;  // in 64K's
    if (!regs.eax & !regs.ebx) {
      regs.eax = regs.ecx << 10; // in K's
      regs.ebx = regs.edx << 16; // in 64K's
    }
    
    temp[0] = regs.eax + (1 << 20);  // add a meg for the first meg of the address space
    temp[1] = 0;
    add64(memory->size, temp);
    temp[0] = regs.ebx;
    add64(memory->size, temp);
    
    memory->word = 2; // type goes here
    goto check_mem_limits;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now try service 88h
  //  service 88h only returns up to 15 meg.
  //  we need to try service C7h for the rest.
  memset(memory, 0, sizeof(struct S_MEMORY));
  
  if (spc_key_F2)
    para_printf("Trying service 88h.\n");
  
  regs.eax = 0x000008800;
  if (!intx(0x15, &regs)) {
    // AX = number of contiguous KB starting at absolute address 100000h
    temp[0] = ((regs.eax & 0x0000FFFF) + 1024) << 10;
    temp[1] = 0;
    add64(memory->size, temp);
    
    // now see if service C7h is available
    // on return:
    //  carry clear and ah = 0
    //   es:bx->rom table
    regs.eax = 0x00000C000;
    if (!intx(0x15, &regs)) {
      if ((regs.eax & 0x0000FF00) == 0) {  // ah = 0 = successful
        // service C7h is available if bit 4 in feature_byte 2 (offset 06h) is set.
        // (We are in unreal mode, flat address space, so convert es:bx to 32-bit flat address)
        bit8u *ptr = (bit8u *) ((regs.es << 4) + (regs.ebx & 0x0000FFFF));
        if (ptr[6] & (1<<4)) {
          regs.eax = 0x0000C700;
          regs.esi = MK_OFF((bit32u) local);
          regs.es = MK_SEG((bit32u) local);
          if (!intx(0x15, &regs)) {
            // dword at 0Eh = system memory between 16M and 4G, in 1K blocks
            // max value return = 0x003FC000 so shifting left by 10 won't overflow
            temp[0] = (* (bit32u *) &local[0x0E]) << 10;
            temp[1] = 0;
            add64(memory->size, temp);
          }
        }
      }
    }
    memory->word = 3; // type goes here   3 = serive 88h or older
    goto check_mem_limits;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now try the cmos value
  //  reg 17h = low byte
  //  reg 18h = high byte  (in kb's)
  if (spc_key_F2)
    para_printf("Trying the CMOS (wow, you have an old machine).\n");
  
  outpb(0x70, 0x18);
  memory->size[0] = (inpb(0x71) << (8 + 10));
  outpb(0x70, 0x17);
  memory->size[0] |= (inpb(0x71) << (0 + 10));
  memory->size[1] = 0;
  memory->word = 4; // type goes here
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now we need to set up our limits.
  // if anything in the hi dword of memory.size, we have plenty :)
check_mem_limits:
  // calculate the amount of conventional memory found
  for (i=0; i<memory->blocks; i++) {
    //win_printf(main_win, " %i %i\n", memory->block[i].size[0], memory->block[i].type);
    switch (memory->block[i].type) {
      case BIOSACPIReclaimMemory:
        add64(memory->size, memory->block[i].size);
        break;
    }
  }
  
  // if not at least MEMORY_MIN_REQUIRED, then give error and halt
  if (memory->size[1] == 0) {  // if high dword > 0, we have enough memory, else
    if (memory->size[0] < MEMORY_MIN_REQUIRED) {
      // not enough memory for preset limits, so adjust to compensate.
      // first, we need to make sure we have at least the min allowed.
      win_printf(main_win, "\n\n  *** Not enough physical memory ***"
             "\n   Size in megabytes found: %i"
             "\n     Size of memory needed: %i (in Megs)", memory->size[0] >> 20, MEMORY_MIN_REQUIRED >> 20);
      freeze();
    }
  }
  
  return TRUE;
}
