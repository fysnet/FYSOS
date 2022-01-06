/*
 *                             Copyright (c) 1984-2022
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 * we have a simple memory allocator.  We allocate physical ram then divide it up into chunks.
 * our allocator then simply finds one of these chunks that is free and returns with that address.
 * when we free it, we simply mark it as free.
 * all chunks are aligned on at least a 4096 byte alignment.  Sufficient for any memory operand.
 *
 * this is a very simply allocator and is all we need for this purpose.  Your OS will have a
 *  much more advanced allocator.
 *
 */
#define HEAP_START       0x01000000           // at 16 meg must be a multiple of a meg
#define HEAP_CHUNK_SZ    4096                 // size of each chunk (must be a multiple of 4096)
#define HEAP_ELEMS       256                  // chunks available for allocation
#define HEAP_SIZE        ((HEAP_ELEMS * HEAP_CHUNK_SZ) - 1)

#define get_linear(x) (x - HEAP_START)

// operational registers
__dpmi_meminfo base_mi;
int base_selector;

// The Memory Heap
__dpmi_meminfo heap_mi;
int heap_selector;
bool our_heap[HEAP_ELEMS];

bit32u async_base;
bit32u periodic_base;


// 'Allocates' some physical memory
bool get_physical_mapping(__dpmi_meminfo *mi, int *selector) {
  int sel;
  
  if (__dpmi_physical_address_mapping(mi) == -1)
    return FALSE;
  sel = __dpmi_allocate_ldt_descriptors(1);
  if (sel == -1)
    return FALSE;
  if (__dpmi_set_segment_base_address(sel, mi->address) == -1)
    return FALSE;
  if (__dpmi_set_segment_limit(sel, mi->size - 1))
    return FALSE;
  
  *selector = sel;
  return TRUE;
}

// initialize all chunks in our heap as free
void heap_init() {
  int i;
  
  for (i=0; i<HEAP_ELEMS; i++)
    our_heap[i] = TRUE;
}

/* returns a chunk of memory (at least HEAP_CHUNK_SZ in size)
 * will clear the memory to zero
 * returns physical address of memory found
 *
 * We pass a size, just to check our code that HEAP_CHUNK_SZ bytes is enough
 */
bit32u heap_alloc(const bit32u size) {
  int i;
  bit32u addr = HEAP_START;
  
  // just check to be sure we don't try to allocate more than we have allocated
  if (size > HEAP_CHUNK_SZ) {
    printf("\n We are trying to allocate more memory (size = %i) than HEAP_CHUNK_SZ."
           "\n Increase HEAP_CHUNK_SZ, recompile, and run again.", size);
    exit(-1);
  }
  
  for (i=0; i<HEAP_ELEMS; i++) {
    if (our_heap[i] == TRUE) {
      our_heap[i] = FALSE;
      // clear it to zeros
      ehci_clear_phy_mem(addr, HEAP_CHUNK_SZ);
      return addr;
    }
    
    // move to next one
    addr += HEAP_CHUNK_SZ;
  }
  
  printf("\n We ran out of chunks to allocate..."
         "\n Increase HEAP_ELEMS, recompile, and run again.");
  exit(-1);
  
  // keep the compiler happy
  return 0;
}

void heap_free(const bit32u address) {
  
  bit32u addr = address - HEAP_START;
  
  // do some checks just to make sure we didn't destroy something
  if (((addr & (HEAP_CHUNK_SZ - 1)) > 0) ||
       (addr > HEAP_SIZE)) {
    printf("\n Trying to free an invalid address (0x%08X)", address);
    return;
  }
  
  our_heap[(addr / HEAP_CHUNK_SZ)] = TRUE;
}

// Dump a memory contents to the screen
void dump(void *addr, bit32u size) {
  
  bit32u offset = (bit32u) addr;
  bit8u *buf = (bit8u *) addr;
  bit8u *temp_buf;
  unsigned i;
  
  while (size) {
    printf("\n %08X  ", offset);
    offset += 16;
    temp_buf = buf;
    for (i=0; (i<16) && (i<size); i++)
      printf("%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    for (; i<16; i++)
      printf("   ");
    printf("   ");
    for (i=0; (i<16) && (i<size); i++) {
      putchar(isprint(*buf) ? *buf : '.');
      buf++;
    }
    size -= i;
  }
}
