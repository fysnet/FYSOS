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
 *  STDLIB.C
 *  This is a helper C source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit only
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.C
 */

void memset(void *targ, const bit8u val, unsigned int len) {
  gSystemTable->BootServices->SetMem(targ, len, val);
}

void memcpy(void *targ, void *src, unsigned int len) {
  gSystemTable->BootServices->CopyMem(targ, src, len);
}

void *AllocatePool(const bit32u MemSize) {
  void *Buffer;
  return (gSystemTable->BootServices->AllocatePool(EfiLoaderData, MemSize, &Buffer) < 0) ? NULL : Buffer;
}

void *AllocatePhysical(const bit32u PhysicalAddress, const bit32u MemSize) {
  EFI_STATUS Status;
  PHYS_ADDRESS Address[2];
  
  Address[0] = PhysicalAddress;
  Address[1] = 0;
  Status = gSystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, ((MemSize + 4095) / 4096), &Address);
  if (Status < 0) {
    printf(L"Error allocating physical memory: 0x%08X (%X)\n", PhysicalAddress, Status);
    return NULL;
  }
  
  if (Address[0] != PhysicalAddress) {
    printf(L"Did not allocate the physical address for file. Address desired: 0x%08X, address returned: 0x%08X\n ...Halting...",
          PhysicalAddress, Address[0]);
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
  gSystemTable->BootServices->FreePool(buffer);
}

int isdigit(int c) {
  return ((c >= '0') && (c <= '9'));
}

int toupper(int c) {
  if ((c >= 'a') && (c <= 'z'))
    return (c - ('a' - 'A'));
  return c;
}

int tolower(int c) {
  if ((c >= 'A') && (c <= 'Z'))
    return (c + ('a' - 'A'));
  return c;
}

wchar_t *wstrchr(wchar_t *s, int c) {
  while (*s) {
    if (*s == c)
      return s;
    s++;
  }
  
  return NULL;
}

int wstrlen(wchar_t *s) {
  int i = 0;
  while (*s)
    i++, s++;
  
  return i;
}

bit32u wstrsize(wchar_t *s) {
  return (wstrlen(s) + 1) * sizeof(wchar_t);
}
