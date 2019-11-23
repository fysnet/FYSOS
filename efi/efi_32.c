/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2019
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
#include "stdlib.h"


EFI_HANDLE gImageHandle;
struct EFI_SYSTEM_TABLE *gSystemTable;
struct EFI_BOOT_SERVICES *gBS;

bool InitializeLib(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable) {
  gImageHandle = ImageHandle;
  gSystemTable = SystemTable;
  gBS = gSystemTable->BootServices;
  
  // checking the signature
  if ((SystemTable->Hdr.Signature[0] != EFI_SYSTEM_TABLE_SIGNATURE) ||
      (SystemTable->Hdr.Signature[1] != EFI_SYSTEM_TABLE_SIGNATURE2))
        return FALSE;
  
  // initialize the io protocol
//  EFI_STATUS Status;
//  if (EFI_ERROR(Status = cpu_init_protocol())) {
//    printf(L"%[Error allocating CPU IO protocol. 0x%08X%]\r\n", ERROR_COLOR, Status);
//    freeze();
//    return FALSE;
//  }
  
  // successfully initialized our code
  return TRUE;
}

// check the version of the EFI found
// if we are compiled for a version higher than version found,
//  we can't go any further, so specify and freeze.
//
void efi_check_version(void) {
  bool error = FALSE;
  const UINT32 version = gSystemTable->Hdr.Revision;
  
#if defined(EFI_2_60_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_60_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_50_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_50_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_40_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_40_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_31_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_31_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_30_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_30_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_20_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_20_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_10_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_10_SYSTEM_TABLE_REVISION);
#elif defined(EFI_2_00_SYSTEM_TABLE_REVISION)
  error = (version < EFI_2_00_SYSTEM_TABLE_REVISION);
#endif
  if (error) {
    printf(L"\r\n %[We are compiled for UEFI version 2.6, but found %i.%i%]\r\n", ERROR_COLOR, (version >> 16), version & 0xFFFF);
    freeze();
  }
}

EFI_STATUS WaitForSingleEvent(EFI_EVENT Event, bit32u Timeout) {
  EFI_STATUS Status;
  bit32u Index;
  EFI_EVENT TimerEvent;
  EFI_EVENT WaitList[2];
  
  if (Timeout) {
    Status = gBS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TimerEvent);
    if (!EFI_ERROR(Status)) {
      // Set the timer event
      gBS->SetTimer(TimerEvent, TimerRelative, Timeout, 0);
      
      // Wait for the original event or the timer
      WaitList[0] = Event;
      WaitList[1] = TimerEvent;
      Status = gBS->WaitForEvent(2, WaitList, &Index);
      gBS->CloseEvent(TimerEvent);
      
      // If the timer expired, change the return to timed out
      if (!EFI_ERROR(Status) &&  (Index == 1))
        Status = EFI_TIMEOUT;
    }
  } else {
    // No timeout... just wait on the event
    Status = gBS->WaitForEvent(1, &Event, &Index);
  }
  
  return Status;
}

// since EFI_STATUS uses bit 31 to indicate an error,
//  as long as we maintain a 32-bit platform *only*,
//  we could use it as a signed value.
// however, as soon as we went to 64-bit, bit 31 no
//  longer indicates a signed value.
// (I have read that bit 63 on a 64-bit machine may 
//    or may not be used to indicate an error????)
// therefore, we check for bit 31 to be set.
// also, NBC does not allow macros:
//   EFI_ERROR(x) (((x) & EFI_ERROR_BIT) > 0)
// so until it does, we have to call as a function
BOOLEAN EFI_ERROR(UINTN Status) {
  return (Status & EFI_ERROR_BIT) > 0;
}




struct EFI_GUID LoadedImageProtocolGUID = {
  0x5B1B31A1, 
  0x9562, 
  0x11D2, 
  {
    0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

/*
EFI_STATUS ConnectAll(void) {
  printf(L"Start\r\n");
  //
  // Connect All Handles Example
  // The following example recursively connects all controllers in a platform.
  //
  EFI_STATUS Status;
  UINTN HandleCount;
  EFI_HANDLE *HandleBuffer;
  UINTN HandleIndex;
  //
  // Retrieve the list of all handles from the handle database
  //
  Status = gBS->LocateHandleBuffer(AllHandles, NULL, NULL, &HandleCount, &HandleBuffer);
  if (!EFI_ERROR(Status)) {
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      Status = gBS->ConnectController(HandleBuffer[HandleIndex], NULL, NULL, TRUE);
    }
    gBS->FreePool(HandleBuffer);
  }
  //
  // Connect Device Path Example
  // The following example walks the device path nodes of a device path, and
  // connects only the drivers required to force a handle with that device path
  // to be present in the handle database. This algorithms guarantees that
  // only the minimum number of devices and drivers are initialized.
  //
  EFI_STATUS Status;
  struct EFI_DEVICE_PATH *DevicePath;
  struct EFI_DEVICE_PATH *RemainingDevicePath;
  EFI_HANDLE Handle;
  do {
    //
    // Find the handle that best matches the Device Path. If it is only a
    // partial match the remaining part of the device path is returned in
    // RemainingDevicePath.
    //
    RemainingDevicePath = DevicePath;
    Status = gBS->LocateDevicePath(&DevicePathProtocol, &RemainingDevicePath, &Handle);
    if (EFI_ERROR(Status)) {
      printf(L"0: Not found\r\n");
      return EFI_NOT_FOUND;
    }
    //
    // Connect all drivers that apply to Handle and RemainingDevicePath
    // If no drivers are connected Handle, then return EFI_NOT_FOUND
    // The Recursive flag is FALSE so only one level will be expanded.
    //
    Status = gBS->ConnectController(Handle, NULL, RemainingDevicePath, FALSE);
    if (EFI_ERROR(Status)) {
      printf(L"1: Not found\r\n");
      return EFI_NOT_FOUND;
    }
    //
    // Loop until RemainingDevicePath is an empty device path
    //
  } while (!IsDevicePathEnd(RemainingDevicePath));
  //
  // A handle with DevicePath exists in the handle database
  //
  printf(L"End\r\n");
  return EFI_SUCCESS;
}
*/
