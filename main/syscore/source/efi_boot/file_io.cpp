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
 *  FILE_IO.CPP
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
#include "file_io.h"
#include "memory.h"
#include "string.h"


struct EFI_GUID FileSystemProtocol = {
  0x964E5B22, 
  0x6459, 
  0x11D2, 
  {
    0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

struct EFI_GUID LoadFileProtocol = {
  0x56EC3091,
  0x954C,
  0x11D2,
  {
    0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

struct EFI_GUID GenericFileInfo = {
  0x9576E92, 
  0x6D3F, 
  0x11D2,
  {
    0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

struct EFI_FILE_HANDLE *LibOpenRoot(EFI_HANDLE DeviceHandle) {
  EFI_STATUS Status;
  struct EFI_FILE_IO_INTERFACE *Volume;
  struct EFI_FILE_HANDLE *File;
  
  // File the file system interface to the device
  Status = gBS->HandleProtocol(DeviceHandle, &FileSystemProtocol, (void **) &Volume);
  
  // Open the root directory of the volume 
  if (!EFI_ERROR(Status))
    Status = Volume->OpenVolume(Volume, &File);
  
  return EFI_ERROR(Status) ? NULL : File;
}

// Opens a file for (simple) reading.  The simple read abstraction
//  will access the file either from a memory copy, from a file
//  system interface, or from the load file interface. 
// returns a handle to access the file
EFI_STATUS OpenSimpleReadFile(bool BootPolicy, void *SourceBuffer, UINTN SourceSize, struct EFI_DEVICE_PATH **FilePath, 
                              EFI_HANDLE *DeviceHandle, EFI_HANDLE *SimpleReadHandle) {

  struct SIMPLE_READ_HANDLE   *FHand;
  struct EFI_DEVICE_PATH      *UserFilePath;
  struct EFI_DEVICE_PATH      *TempFilePath;
  struct EFI_DEVICE_PATH      *TempFilePathPtr;
  struct FILEPATH_DEVICE_PATH *FilePathNode;
  struct EFI_FILE_HANDLE      *FileHandle, *LastHandle;
  EFI_STATUS                  Status;
  struct EFI_LOAD_FILE_INTERFACE *LoadFile;
  
  FHand = NULL;
  UserFilePath = *FilePath;
  
  // Allocate a new simple read handle structure
  FHand = (struct SIMPLE_READ_HANDLE *) AllocateZeroPool(sizeof(struct SIMPLE_READ_HANDLE));
  if (FHand == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  
  *SimpleReadHandle = (EFI_HANDLE) FHand;
  FHand->Signature = 0x72647273; // 's','r','d','r' in little endian format
  
  // If the caller passed a copy of the file, then just use it
  if (SourceBuffer) {
    FHand->Source = SourceBuffer;
    FHand->SourceSize = SourceSize;
    *DeviceHandle = NULL;
    Status = EFI_SUCCESS;
    goto Done;
  } 
  
  // Attempt to access the file via a file system interface
  FileHandle = NULL;
  Status = gBS->LocateDevicePath(&FileSystemProtocol, FilePath, DeviceHandle);
  if (!EFI_ERROR(Status))
    FileHandle = LibOpenRoot(*DeviceHandle);
  Status = (FileHandle != NULL) ? EFI_SUCCESS : EFI_UNSUPPORTED;
  
  // To access as a filesystem, the filepath should only
  // contain filepath components.  Follow the filepath nodes
  // and find the target file
  FilePathNode = (struct FILEPATH_DEVICE_PATH *) *FilePath;
  while (!(((FilePathNode->Header.Type & EFI_DP_TYPE_MASK) == END_DEVICE_PATH_TYPE) && (FilePathNode->Header.SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE))) {
    // For filesystem access each node should be a filepath component
    if (((FilePathNode->Header.Type & EFI_DP_TYPE_MASK) != MEDIA_DEVICE_PATH) ||
         (FilePathNode->Header.SubType != MEDIA_FILEPATH_DP))
        Status = EFI_UNSUPPORTED;

    // If there's been an error, stop
    if (EFI_ERROR(Status))
        break;
    
    // Open this file path node
    LastHandle = FileHandle;
    FileHandle = NULL;
    
    Status = LastHandle->Open(LastHandle, &FileHandle, FilePathNode->PathName, EFI_FILE_MODE_READ, 0);
    
    // Close the last node
    LastHandle->Close(LastHandle);
    
    // Get the next node
    FilePathNode = (struct FILEPATH_DEVICE_PATH *) ((bit8u *) &FilePathNode->Header + FilePathNode->Header.Length);
  }
  
  // If success, return the FHand
  if (!EFI_ERROR(Status)) {
    FHand->FileHandle = FileHandle;
    goto Done;
  }
  
  // Cleanup from filesystem access
  if (FileHandle) {
    FileHandle->Close(FileHandle);
    FileHandle = NULL;
    *FilePath = UserFilePath;
  }
  
  // If the error is something other then unsupported, return it
  if (Status != EFI_UNSUPPORTED)
    goto Done;
  
  /////////////////////////////
  // Attempt to access the file via the load file protocol
  Status = LibDevicePathToInterface(&LoadFileProtocol, *FilePath, (EFI_HANDLE *) &LoadFile);
  if (!EFI_ERROR(Status)) {
    TempFilePath = DuplicateDevicePath(*FilePath);
    TempFilePathPtr = TempFilePath;
    
    Status = gBS->LocateDevicePath(&LoadFileProtocol, &TempFilePath, DeviceHandle);
    FreePool(TempFilePathPtr);
    
    // Determine the size of buffer needed to hold the file
    SourceSize = 0;
    Status = LoadFile->LoadFile(LoadFile, (struct FILEPATH_DEVICE_PATH *) *FilePath, BootPolicy, &SourceSize, NULL);
    
    // We expect a buffer too small error to inform us of the buffer size needed
    if (Status == EFI_BUFFER_TOO_SMALL) {
      SourceBuffer = AllocatePool(SourceSize);
      if (SourceBuffer) {
        FHand->FreeBuffer = TRUE;
        FHand->Source = SourceBuffer;
        FHand->SourceSize = SourceSize;
        Status = LoadFile->LoadFile(LoadFile, (struct FILEPATH_DEVICE_PATH *) *FilePath, BootPolicy, &SourceSize, SourceBuffer);
      }
    }
    
    // If success, return FHand
    if (!EFI_ERROR(Status) || (Status == EFI_ALREADY_STARTED))
      goto Done;
  }
  
  // Nothing else to try
  printf(L"OpenSimpleReadFile: Device did not support a known load protocol\r\n");
  Status = EFI_UNSUPPORTED;

Done:
  // If the file was not accessed, clean up
  if (EFI_ERROR(Status) && (Status != EFI_ALREADY_STARTED)) {
    if (FHand) {
      if (FHand->FreeBuffer)
        FreePool(FHand->Source);
      FreePool (FHand);
    }
  }
  
  return Status;
}

EFI_STATUS ReadSimpleReadFile(EFI_HANDLE UserHandle, UINTN Offset, UINTN *ReadSize, void *Buffer) {
  UINTN EndPos;
  struct SIMPLE_READ_HANDLE *FHand = (struct SIMPLE_READ_HANDLE *) UserHandle;
  EFI_STATUS Status;
  
//  ASSERT (FHand->Signature == 'srdr');
  if (FHand->Source) {
    // Move data from our local copy of the file
    EndPos = Offset + *ReadSize;
    if (EndPos > FHand->SourceSize) {
      *ReadSize = FHand->SourceSize - Offset;
      if (Offset >= FHand->SourceSize)
        *ReadSize = 0;
    }
    
    memcpy(Buffer, (bit8u *) FHand->Source + Offset, *ReadSize);
    Status = EFI_SUCCESS;
  } else {
    // Read data from the file
    Status = FHand->FileHandle->SetPosition(FHand->FileHandle, Offset);
    if (!EFI_ERROR(Status))
      Status = FHand->FileHandle->Read(FHand->FileHandle, ReadSize, Buffer);
  }
  
  return Status;
}

void CloseSimpleReadFile(EFI_HANDLE UserHandle) {
  struct SIMPLE_READ_HANDLE *FHand = (struct SIMPLE_READ_HANDLE *) UserHandle;
  
  //ASSERT (FHand->Signature == 'srdr');
  // Free any file handle we opened
  if (FHand->FileHandle)
    FHand->FileHandle->Close(FHand->FileHandle);
  
  // If we allocated the Source buffer, free it
  if (FHand->FreeBuffer)
    FreePool(FHand->Source);
  
  // Done with this simple read file handle
  FreePool(FHand);
}

EFI_STATUS OpenSimpleFileLen(EFI_HANDLE UserHandle, UINTN *RetSize) {
  struct SIMPLE_READ_HANDLE *FHand = (struct SIMPLE_READ_HANDLE *) UserHandle;
  EFI_STATUS Status;
  bit8u Buffer[256];
  UINTN Size = 256;
  struct EFI_FILE_INFO *FileInfo = (struct EFI_FILE_INFO *) Buffer;
  
  *RetSize = 0;
  Status = FHand->FileHandle->GetInfo(FHand->FileHandle, &GenericFileInfo, &Size, FileInfo);
  if (!EFI_ERROR(Status))
    *RetSize = (UINTN) FileInfo->FileSize;
  
  return Status;
}
