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
#if UEFI_INCLUDE_MOUSE

#include "ctype.h"
#include "efi_32.h"

#include "conout.h"

#include "mouse.h"

struct EFI_SIMPLE_POINTER_PROTOCOL *spp;
struct EFI_SIMPLE_POINTER_STATE ps;

struct EFI_GUID SimplePointerProtocol = {
  0x31878C87, 
  0x0B75, 
  0x11D5, 
  {
    0x9A, 0x4F, 0x0, 0x90, 0x27, 0x3F, 0xC1, 0x4D
  }
};

int InitMouse(void) {
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer;
  bit32u HandleCount = 0;
  bit32u i = 0;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &SimplePointerProtocol, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) printf(L"UEFI broken (0x%08X)", Status);
  //printf(L"Handle Count = %i\r\n", HandleCount);
  
  Status = gBS->HandleProtocol(HandleBuffer[0], &SimplePointerProtocol, (void **) &spp);
  if (EFI_ERROR(Status)) {
    printf(L"Mouse required! (0x%08X)", Status);
    return 0;
  }
  
  Status = spp->Reset(spp, FALSE);
  if (EFI_ERROR(Status)) {
    printf(L"Failed to initialize pointy I/O (0x%08X)", Status);
    return 0;
  }
  
  printf(L"Move the Mouse....\r\n");
  while (1) {
    Status = spp->GetState(spp, &ps);
    if (Status == EFI_NOT_READY) {
      printf(L"%i: Not Ready...\r  ", i++);
      continue;
    } else if (Status == EFI_SUCCESS) {
      printf(L"%i %i %i %i\r\n", ps.RelativeMovementX, ps.RelativeMovementY, ps.RelativeMovementZ, ps.LeftButton, ps.RightButton);
      continue;
    } else {
      printf(L"Failed to get State of Mouse (0x%08X)", Status);
      break;
    }
  }
  
  return 1;
}

void UpdateMouse() {
  printf(L"DDD\r\n");
  WaitForSingleEvent(spp->WaitForInput, 0);
  printf(L"EEE\r\n");
  EFI_STATUS Status = spp->GetState(spp, &ps);
  printf(L"%d, %d, %d\r\n", ps.RelativeMovementX, ps.RelativeMovementY, ps.RelativeMovementZ);
  if (Status == EFI_NOT_READY) printf(L"Not ready");
  if (Status == EFI_DEVICE_ERROR) printf(L"Error");
}

/*
struct EFI_GUID AbsolutePointerProtocol = {
  0x8D59D32B, 
  0xC655, 
  0x4AE9, 
  {
    0x9B, 0x15, 0xF2, 0x59, 0x04, 0x99, 0x2A, 0x43
  }
};

struct EFI_ABSOLUTE_POINTER_MODE {
	bit32u AbsoluteMinX[2];
	bit32u AbsoluteMinY[2];
	bit32u AbsoluteMinZ[2];
	bit32u AbsoluteMaxX[2];
	bit32u AbsoluteMaxY[2];
	bit32u AbsoluteMaxZ[2];
	bit32u Attributes;
};

struct EFI_ABSOLUTE_POINTER_STATE {
	bit32u CurrentX[2];
	bit32u CurrentY[2];
	bit32u CurrentZ[2];
	bit32u ActiveButtons;
};

#define EFI_ABSP_SupportsAltActive 0x00000001
#define EFI_ABSP_SupportsPressureAsZ 0x00000002
#define EFI_ABSP_TouchActive 0x00000001
#define EFI_ABS_AltActive 0x00000002

struct EFI_ABSOLUTE_POINTER_PROTOCOL {
  EFI_STATUS (*Reset)(struct EFI_ABSOLUTE_POINTER_PROTOCOL *abp, bool ExtendedVerification);
  EFI_STATUS (*GetState)(struct EFI_ABSOLUTE_POINTER_PROTOCOL *abp, struct EFI_ABSOLUTE_POINTER_STATE *state);
  void  *WaitForInput;
  struct EFI_ABSOLUTE_POINTER_MODE *Mode;
};

struct EFI_ABSOLUTE_POINTER_PROTOCOL *abs;
struct EFI_ABSOLUTE_POINTER_STATE ps;

int InitMouse(void) {
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer;
  bit32u HandleCount = 0;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &AbsolutePointerProtocol, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) printf(L"UEFI broken (0x%08X)", Status);
  
  printf(L"Handle Count = %i\r\n", HandleCount);
  
  Status = gBS->HandleProtocol(HandleBuffer[0], &AbsolutePointerProtocol, (void **) &abs);
  
  if (EFI_ERROR(Status)) {
    printf(L"Mouse required! (0x%08X)", Status);
    return 0;
  }
  
  Status = abs->Reset(abs, FALSE);
  if (EFI_ERROR(Status)) {
    printf(L"Failed to initialize pointy I/O (0x%08X)", Status);
    return 0;
  }
  
  return 1;
}

void UpdateMouse() {
  printf(L"We are here...\r\n");
  //WaitForSingleEvent(abs->WaitForInput, 0);
  EFI_STATUS Status = abs->GetState(abs, &ps);
  while (1) {
    if (Status == EFI_NOT_READY)
      continue;
    if (Status == EFI_DEVICE_ERROR) {
      printf(L"Error...\r\n");
      break;
    }
    printf(L"%i %i %i %i\r", ps.CurrentX[0], ps.CurrentY[0], ps.CurrentZ[0], ps.ActiveButtons);
  }
}
*/

#endif  // UEFI_INCLUDE_MOUSE
