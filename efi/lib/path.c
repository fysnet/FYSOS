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
 * Note:  Since this code uses wide chars (wchar_t), you *MUST* have my modified 
 *        version of SmallerC.  Contact me for more information.
 *        
 */


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

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

bit32u DevicePathSize(struct EFI_DEVICE_PATH *DevPath) {
  struct EFI_DEVICE_PATH *Start;
  
  // Search for the end of the device path structure
  Start = DevPath;
  while (!(((DevPath->Type & EFI_DP_TYPE_MASK) == END_DEVICE_PATH_TYPE) && (DevPath->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)))
    DevPath = (struct EFI_DEVICE_PATH *) ((bit8u *) DevPath + DevPath->Length);
  
  // Compute the size
  return ((bit32u) DevPath - (bit32u) Start) + sizeof(struct EFI_DEVICE_PATH);
}

struct EFI_DEVICE_PATH *DuplicateDevicePath(struct EFI_DEVICE_PATH *DevPath) {
  struct EFI_DEVICE_PATH *NewDevPath;
  bit32u Size;    
  
  // Compute the size
  Size = DevicePathSize(DevPath);
  
  // Make a copy
  NewDevPath = AllocatePool(Size);
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

struct EFI_DEVICE_PATH *DevicePathInstance(struct EFI_DEVICE_PATH **DevicePath, bit32u *Size) {
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
      // BugBug: Debug code to catch bogus device paths
      //DEBUG((D_ERROR, "DevicePathInstance: DevicePath %x Size %d", *DevicePath, ((UINT8 *) DevPath) - ((UINT8 *) Start) ));
      //DumpHex (0, 0, ((UINT8 *) DevPath) - ((UINT8 *) Start), Start);
      break;
    }
    
    DevPath = Next;
  }
  
  //ASSERT (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE ||
  //        DevicePathSubType(DevPath) == END_INSTANCE_DEVICE_PATH_SUBTYPE);
  
  // Set next position
  if (DevPath->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)
    Next = NULL;
  
  *DevicePath = Next;
  
  // Return size and start of device path instance
  *Size = ((bit8u *) DevPath) - ((bit8u *) Start);
  
  return Start;
}

bit32u DevicePathInstanceCount(struct EFI_DEVICE_PATH *DevicePath) {
  bit32u Count = 0, Size;
  
  while (DevicePathInstance(&DevicePath, &Size))
    Count++;
  
  return Count;
}

// Src1 may have multiple "instances" and each instance is appended
// Src2 is appended to each instance is Src1.  (E.g., it's possible
//  to append a new instance to the complete device path by passing 
//  it in Src2)
struct EFI_DEVICE_PATH *AppendDevicePath(struct EFI_DEVICE_PATH *Src1, struct EFI_DEVICE_PATH *Src2) {
  bit32u Src1Size, Src1Inst, Src2Size, Size;
  struct EFI_DEVICE_PATH *Dst, *Inst;
  bit8u *DstPos;
  
  // If there's only 1 path, just duplicate it
  if (Src1 == NULL) {
    //ASSERT (!IsDevicePathUnpacked(Src2));
    return DuplicateDevicePath(Src2);
  }
  
  if (Src2 == NULL) {
    //ASSERT (!IsDevicePathUnpacked (Src1));
    return DuplicateDevicePath(Src1);
  }
  
  // Verify we're not working with unpacked paths
  //    ASSERT (!IsDevicePathUnpacked (Src1));
  //    ASSERT (!IsDevicePathUnpacked (Src2));
  
  // Append Src2 to every instance in Src1
  Src1Size = DevicePathSize(Src1);
  Src1Inst = DevicePathInstanceCount(Src1);
  Src2Size = DevicePathSize(Src2);
  Size = Src1Size * Src1Inst + Src2Size;
  
  Dst = AllocatePool(Size);
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
struct EFI_DEVICE_PATH *FileDevicePath(void *Device, wchar_t *FileName) {
  bit32u Size, dword;
  struct FILEPATH_DEVICE_PATH *FilePath;
  struct EFI_DEVICE_PATH *Eop, *DevicePath;
  
  Size = wstrsize(FileName);
  FilePath = AllocateZeroPool(Size + ((bit32u) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))) + sizeof(struct EFI_DEVICE_PATH));
  DevicePath = NULL;
  
  if (FilePath) {
    // Build a file path
    FilePath->Header.Type = MEDIA_DEVICE_PATH;
    FilePath->Header.SubType = MEDIA_FILEPATH_DP;
    
    //SetDevicePathNodeLength(&FilePath->Header, Size + ((bit32u) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))));
    dword = (Size + ((bit32u) (&(((struct FILEPATH_DEVICE_PATH *) 0)->PathName))));
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
    }
  }
  
  return DevicePath;
}
