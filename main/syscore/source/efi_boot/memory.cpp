/*
 *                             Copyright (c) 1984-2020
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
 *  MEMORY.CPP
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.CPP
 */

#include "efi_common.h"

#include "conout.h"
#include "stdlib.h"
#include "string.h"

#include "memory.h"

void *AllocatePool(const UINTN MemSize) {
  void *Buffer;
  return (gBS->AllocatePool(EfiLoaderData, MemSize, &Buffer) < 0) ? NULL : Buffer;
}

void *AllocatePhysical(const PHYS_ADDRESS PhysicalAddress, const UINTN MemSize) {
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS Address = PhysicalAddress;
  
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, (MemSize + 4095) / 4096, &Address);
  if (EFI_ERROR(Status)) {
    printf(L"%[Error allocating physical memory: 0x%08X (size = %i pages) (%X)%]\r\n", ERROR_COLOR, (bit32u) Address, (int) ((MemSize + 4095) / 4096), (bit32u) Status);
    return NULL;
  }
  
  if (Address != PhysicalAddress) {
    printf(L"%[Did not allocate the physical address for file. Address desired: 0x%08X, address returned: 0x%08X\r\n ...Halting...%] ",
           ERROR_COLOR, (bit32u) PhysicalAddress, (bit32u) Address);
    return NULL;
  }
  
  return (void *) Address;
}

void *AllocateZeroPool(const UINTN MemSize) {
  void *Buffer = AllocatePool(MemSize);
  if (Buffer != NULL) {
    memset(Buffer, 0, MemSize);
    return Buffer;
  }
  return NULL;
}

void FreePool(void *buffer) {
  gBS->FreePool(buffer);
}

EFI_STATUS GetMemory(UINTN *rMemMapKey, const bool DoDisplay) {
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN MemMapKey = 0, DescriptorSize = 0, Count = 0;
  UINTN MemMapSize = sizeof(struct EFI_MEMORY_DESCRIPTOR) * 16;
  UINTN MemMapSizeOut = MemMapSize;
  UINT32 Version = 0, i = 0;
  bit8u *Buffer = NULL;
  struct EFI_MEMORY_DESCRIPTOR *DescriptorPtr;
  
  do {
    Buffer = (bit8u *) AllocatePool(MemMapSize);
    if (Buffer == NULL)
      break;
    
    // Normally, we only need to set "MemMapSizeOut" once, since the call should return the size
    //  we need.  However:
    // Intel's latest has a bug where it requires "MemMapSizeOut" to contain the size of the buffer
    //  allocated.  If not, freeing the buffer (below) crashes the system...
    MemMapSizeOut = MemMapSize;
    Status = gBS->GetMemoryMap(&MemMapSizeOut, (struct EFI_MEMORY_DESCRIPTOR *) Buffer, 
        &MemMapKey, &DescriptorSize, &Version);
    // One of the only reasons it won't return success is that we didn't give it enough memory
    //  to fill.  So add to it and try again.
    if (EFI_ERROR(Status)) {
      FreePool(Buffer);
      MemMapSize += (sizeof(struct EFI_MEMORY_DESCRIPTOR) * 16);
    }
  } while (EFI_ERROR(Status));
  
  if (Buffer != NULL) {
    // We only want to display it once, but we call it twice
    if (DoDisplay) {
      Count = MemMapSizeOut / DescriptorSize;
      
      printf(L"Count: %i\r\n", Count);
      for (i=0; i<Count; i++) {
        DescriptorPtr = (struct EFI_MEMORY_DESCRIPTOR *) (Buffer + (i * DescriptorSize));
        printf(L"%2i: 0x%08X  Pages: %6i  Type: %2i  Attribute 0x%08X\r\n",
          i,
          (bit32u) DescriptorPtr->PhysicalStart,
          (bit32u) DescriptorPtr->NumberOfPages,
          DescriptorPtr->Type,
          (bit32u) DescriptorPtr->Attribute);
      }
      // if we free the buffer, it will effect the "MemMapKey" ID and
      //  it won't allow us to exit boot services...
      // therefore, only free it if "DoDisplay" is set
      FreePool(Buffer);
    }
  }
  
  // we need to return the key value so that ExitBIOSServices will function correctly.
  *rMemMapKey = MemMapKey;
  return Status;
}
