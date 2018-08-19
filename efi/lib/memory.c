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
 *
 */


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

#include "conout.h"
#include "string.h"


void *AllocatePool(const bit32u MemSize) {
  void *Buffer;
  return (gBS->AllocatePool(EfiLoaderData, MemSize, &Buffer) < 0) ? NULL : Buffer;
}

void *AllocatePhysical(const bit32u PhysicalAddress, const bit32u MemSize) {
  EFI_STATUS Status;
  PHYS_ADDRESS Address[2];
  
  Address[0] = PhysicalAddress;
  Address[1] = 0;
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, ((MemSize + 4095) / 4096), &Address);
  if (EFI_ERROR(Status)) {
    printf(L"%[Error allocating physical memory: 0x%08X (%X)%]\r\n", ERROR_COLOR, PhysicalAddress, Status);
    return NULL;
  }
  
  if (Address[0] != PhysicalAddress) {
    printf(L"%[Did not allocate the physical address for file. Address desired: 0x%08X, address returned: 0x%08X\r\n ...Halting...%]",
          ERROR_COLOR, PhysicalAddress, Address[0]);
    return NULL;
  }
  
  return (void *) Address[0];
}

void *AllocateZeroPool(const bit32u MemSize) {
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
