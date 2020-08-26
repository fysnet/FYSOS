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
 *  PATH.CPP
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


#include "ctype.h"
#include "efi_common.h"

#include "conout.h"
#include "memory.h"
#include "string.h"

#include "path.h"


struct EFI_GUID DevicePathProtocol = {
  0x9576E91, 
  0x6D3F, 
  0x11D2, 
  {
    0x8E, 0x39, 0x0, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

struct EFI_DEVICE_PATH EndInstanceDevicePath[] = {
  { 
    END_DEVICE_PATH_TYPE, 
    END_INSTANCE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH
  }
};

struct EFI_DEVICE_PATH EndDevicePath[] = {
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH
  }
};


UINT8 DevicePathType(const VOID *Node) {
  return ((struct EFI_DEVICE_PATH *)(Node))->Type;
}

BOOLEAN IsDevicePathEndType(const VOID *Node) {
  return (BOOLEAN) (DevicePathType(Node) == END_DEVICE_PATH_TYPE);
}

UINT8 DevicePathSubType(const VOID *Node) {
  return ((struct EFI_DEVICE_PATH *)(Node))->SubType;
}

BOOLEAN IsDevicePathEnd(const VOID *Node) {
  return (BOOLEAN) (IsDevicePathEndType(Node) && DevicePathSubType(Node) == END_ENTIRE_DEVICE_PATH_SUBTYPE);
}

EFI_STATUS LibDevicePathToInterface(struct EFI_GUID *Protocol, struct EFI_DEVICE_PATH *FilePath, void **Interface) {
  EFI_STATUS Status;
  void *Device;
  
  Status = gBS->LocateDevicePath(Protocol, &FilePath, &Device);
  if (!EFI_ERROR(Status)) {
    // If we didn't get a direct match return not found
    Status = EFI_NOT_FOUND;
    if (((FilePath->Type & EFI_DP_TYPE_MASK) == END_DEVICE_PATH_TYPE) && (FilePath->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE))
      // It was a direct match, lookup the protocol interface
      Status = gBS->HandleProtocol(Device, Protocol, (void **) Interface);
  }
  
  // If there was an error, do not return an interface
  if (EFI_ERROR(Status))
    *Interface = NULL;
  
  return Status;
}

UINTN DevicePathSize(struct EFI_DEVICE_PATH *DevPath) {
  struct EFI_DEVICE_PATH *Start;
  
  // Search for the end of the device path structure
  Start = DevPath;
  while (!(((DevPath->Type & EFI_DP_TYPE_MASK) == END_DEVICE_PATH_TYPE) && (DevPath->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)))
    DevPath = (struct EFI_DEVICE_PATH *) ((bit8u *) DevPath + DevPath->Length);
  
  // Compute the size
  return (UINTN) (((PHYS_ADDRESS) DevPath - (PHYS_ADDRESS) Start) + sizeof(struct EFI_DEVICE_PATH));
}

struct EFI_DEVICE_PATH *DuplicateDevicePath(struct EFI_DEVICE_PATH *DevPath) {
  struct EFI_DEVICE_PATH *NewDevPath;
  UINTN Size;    
  
  // Compute the size
  Size = DevicePathSize(DevPath);
  
  // Make a copy
  NewDevPath = (struct EFI_DEVICE_PATH *) AllocatePool(Size);
  if (NewDevPath)
    memcpy(NewDevPath, DevPath, Size);
  
  return NewDevPath;
}

struct EFI_DEVICE_PATH *DevicePathFromHandle(void *Handle) {
  EFI_STATUS Status;
  struct EFI_DEVICE_PATH *DevicePath;
  
  Status = gBS->HandleProtocol(Handle, &DevicePathProtocol, (void **) &DevicePath);
  if (EFI_ERROR(Status))
    DevicePath = NULL;
  
  return DevicePath;
}

struct EFI_DEVICE_PATH *DevicePathInstance(struct EFI_DEVICE_PATH **DevicePath, UINTN *Size) {
  struct EFI_DEVICE_PATH *Start, *Next, *DevPath;
  bit32u Count;
  
  DevPath = *DevicePath;
  Start = DevPath;
  
  if (DevPath == NULL)
    return NULL;
  
  // Check for end of device path type
  for (Count=0; ; Count++) {
    Next = (struct EFI_DEVICE_PATH *) ((bit8u *) DevPath + DevPath->Length);
    if ((((DevPath->Type) & EFI_DP_TYPE_MASK) == END_DEVICE_PATH_TYPE))
      break;
    
    if (Count > 01000) {
      // catch bogus device paths
      break;
    }
    
    DevPath = Next;
  }
  
  // Set next position
  if (DevPath->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)
    Next = NULL;
  
  *DevicePath = Next;
  
  // Return size and start of device path instance
  *Size = ((bit8u *) DevPath) - ((bit8u *) Start);
  
  return Start;
}

bit32u DevicePathInstanceCount(struct EFI_DEVICE_PATH *DevicePath) {
  bit32u Count = 0;
  UINTN Size;
  
  while (DevicePathInstance(&DevicePath, &Size))
    Count++;
  
  return Count;
}

// Src1 may have multiple "instances" and each instance is appended
// Src2 is appended to each instance is Src1.  (E.g., it's possible
//  to append a new instance to the complete device path by passing 
//  it in Src2)
struct EFI_DEVICE_PATH *AppendDevicePath(struct EFI_DEVICE_PATH *Src1, struct EFI_DEVICE_PATH *Src2) {
  UINTN Src1Size, Src1Inst, Src2Size, Size;
  struct EFI_DEVICE_PATH *Dst, *Inst;
  bit8u *DstPos;
  
  // If there's only 1 path, just duplicate it
  if (Src1 == NULL) {
    return DuplicateDevicePath(Src2);
  }
  
  if (Src2 == NULL) {
    return DuplicateDevicePath(Src1);
  }
  
  // Append Src2 to every instance in Src1
  Src1Size = DevicePathSize(Src1);
  Src1Inst = DevicePathInstanceCount(Src1);
  Src2Size = DevicePathSize(Src2);
  Size = Src1Size * Src1Inst + Src2Size;
  
  Dst = (struct EFI_DEVICE_PATH *) AllocatePool(Size);
  if (Dst) {
    DstPos = (bit8u *) Dst;
    // Copy all device path instances
    while ((Inst = DevicePathInstance(&Src1, &Size))) {
      memcpy(DstPos, Inst, Size);
      DstPos += Size;
      
      memcpy(DstPos, Src2, Src2Size);
      DstPos += Src2Size;
      
      memcpy(DstPos, EndInstanceDevicePath, sizeof(struct EFI_DEVICE_PATH));
      DstPos += sizeof(struct EFI_DEVICE_PATH);
    }
    
    // Change last end marker
    DstPos -= sizeof(struct EFI_DEVICE_PATH);
    memcpy(DstPos, EndDevicePath, sizeof(struct EFI_DEVICE_PATH));
  }
  
  return Dst;
}

// Results are allocated from pool.  The caller must FreePool the resulting device path structure
struct EFI_DEVICE_PATH *FileDevicePath(void *Device, wchar_t *FileName, BOOLEAN *FreeIt) {
  UINTN Size, dword;
  struct FILEPATH_DEVICE_PATH *FilePath;
  struct EFI_DEVICE_PATH *Eop, *DevicePath = NULL;
  
  Size = wstrsize(FileName);
  FilePath = (struct FILEPATH_DEVICE_PATH *) AllocateZeroPool(Size + ((UINTN) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))) + sizeof(struct EFI_DEVICE_PATH));
  if (FreeIt) *FreeIt = TRUE;
  
  if (FilePath) {
    // Build a file path
    FilePath->Header.Type = MEDIA_DEVICE_PATH;
    FilePath->Header.SubType = MEDIA_FILEPATH_DP;
    
    //SetDevicePathNodeLength(&FilePath->Header, Size + ((bit32u) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))));
    dword = (Size + ((UINTN) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))));
    FilePath->Header.Length = (bit16u) dword;
    
    memcpy(FilePath->PathName, FileName, Size);
    Eop = (struct EFI_DEVICE_PATH *) ((bit8u *) &FilePath->Header + FilePath->Header.Length);
    Eop->Type = END_DEVICE_PATH_TYPE;
    Eop->SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;
    Eop->Length = sizeof(struct EFI_DEVICE_PATH);
    
    // Append file path to device's device path
    DevicePath = (struct EFI_DEVICE_PATH *) FilePath;
    if (Device) {
      DevicePath = AppendDevicePath(DevicePathFromHandle(Device), DevicePath);
      FreePool(FilePath);
      // don't let the caller free the memory since we appended it
      if (FreeIt) *FreeIt = FALSE;
    }
  }
  
  return DevicePath;
}
