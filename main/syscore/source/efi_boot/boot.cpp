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
 *  BOOT.CPP
 *  This is the main CPP source file for a demo bootable image for UEFI.
 *  This code will show how to clear the screen, print chars to the screen,
 *   get the available video screen modes, memory map, read files from the
 *   disk, and display graphics to the screen.
 *
 *  To use:
 *   You need a GPT formatted disk image with at least one partition entry
 *   formatted to FAT32 (FAT16 works with most EFI systems), with the following
 *    files in the root directory:
 *     BOOTIA32.EFI
 *     BOOTx64.EFI
 *     startup.nsh
 *     readme.txt
 *   Then boot the image using an EFI compatible emulator such as Oracle VM VirtualBox
 *    or QEMU
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   You need fairly modern C Compiler that can produce EFI PE/COFF files.
 *   I used a well known compiler freely available from the vendor.
 */

#include "efi_common.h"

#include "ctype.h"
#include "conin.h"
#include "conout.h"
#include "file_io.h"
#include "memory.h"
#include "path.h"
#include "stdlib.h"
#include "video.h"

EFI_STATUS GetTime(struct EFI_TIME *Time);
EFI_STATUS LoadFile(wchar_t *filename, void **buffer, UINTN *Size);

wchar_t *Months[12] = { L"January", L"February", L"March", L"April", L"May", L"June", 
                        L"July", L"August", L"September", L"October", L"November", L"December" };

bit32u Color[10] = { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00FF0000, 0x0000FF00,
                     0x000000FF, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00FFFFFF };

// Our Main.  This is what gets called by the EFI Entry
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable) {
  bit32u i;
  EFI_STATUS Status;
  UINTN MemMapKey, FileSize;
  struct EFI_INPUT_KEY Key;
  bit8u *FileBuffer = (bit8u *) NULL;
  struct EFI_TIME Time;
  
  // initialize our library code
  if (!InitializeLib(ImageHandle, SystemTable))
    return 1;
  
  // clear the screen
  cls();
  
  // turn off the video cursor
  cursor(FALSE);
  
  // print the Hello World string
#ifdef _WIN64
  printf(L"EFI 64-bit Boot Demo    v1.50.00        (C)opyright 1984-2020\r\n");
#else
  printf(L"EFI 32-bit Boot Demo    v1.50.00        (C)opyright 1984-2020\r\n");
#endif
  
  // get the current time
  GetTime(&Time);
  printf(L"Current Date and Time: %i %s %i  %02i:%02i:%02i.%i\r\n", 
    Time.Day, Months[Time.Month-1], Time.Year,
    Time.Hour, Time.Minute, Time.Second, Time.Nanosecond);
  printf(L"Press a key to continue...\r\n");
  getkeystroke(&Key);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Load a text file and display it to the screen to show how to
  //   load files.
  printf(L"Loading readme.txt file\r\n");
  LoadFile(L"\\efi\\boot\\readme.txt", (void **) &FileBuffer, &FileSize);
  for (i=0; i<FileSize; i++)
    PutChar(FileBuffer[i]);
  printf(L"\r\nPress a key to continue...\r\n");
  getkeystroke(&Key);
  FreePool(FileBuffer);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Get the available video modes
  struct S_MODE_INFO mode_info[VIDEO_MAX_MODES];
  GetVideoInfo(mode_info);
  printf(L"Press a key to continue...\r\n");
  getkeystroke(&Key);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Get the memory map from the uEFI BIOS
  GetMemory(&MemMapKey, TRUE);
  printf(L"Press a key to continue...\r\n");
  getkeystroke(&Key);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // shutdown the uEFI bios
  GetMemory(&MemMapKey, FALSE);
  Status = gBS->ExitBootServices(ImageHandle, MemMapKey);
  if (EFI_ERROR(Status)) {
    printf(L"We didn't shut down the Boot Serivces...(%X)\r\n", Status);
    freeze();
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Draw a few boxes
  // Since we have already gotten the Linear Frame Buffer pointer and we no
  //  longer call any EFI BIOS functions, we can draw to the screen *after*
  //  we exit the services above.  This will show that we can continue to
  //  use the machine, just not call any more functions.
  extern struct EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  DrawBox(0, 0, gop->Mode->Info->HorizontalResolution, gop->Mode->Info->VerticalResolution, 0x000000CC);
  int x = 100, y = 100;
  for (i=0; i<10; i++) {
    DrawBox(x, y, x + 100, y + 100, Color[i]);
    x += 25;
    y += 25;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // This is where you now set up your base kernel and jump to it.
  // You have already loaded it using LoadFile() above and used AllocatePhysical()
  //  to place it.
  // However, for this demo, we will just stop or as the name implies, freeze().
  freeze();

  return 0;  // keep the compiler happy
}

EFI_STATUS GetTime(struct EFI_TIME *Time) {
  return gSystemTable->RuntimeServices->GetTime(Time, NULL);
}

EFI_STATUS LoadFile(wchar_t *Filename, void **FileBuffer, UINTN *FileSize) {
  struct EFI_DEVICE_PATH *Path = NULL;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE ReadHandle = NULL;
  UINTN HandleCount, HandleIdx, RetSize;
  EFI_HANDLE *HandleBuffer;
  void *Buffer;
  BOOLEAN FreeIt = FALSE;
  
  *FileBuffer = NULL;
  *FileSize = 0;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &FileSystemProtocol, NULL, &HandleCount, &HandleBuffer);
  if (Status != EFI_SUCCESS)
    return Status;
  
  for (HandleIdx = 0; HandleIdx < HandleCount; HandleIdx++) {
    EFI_HANDLE DeviceHandle;
    
    Path = FileDevicePath(HandleBuffer[HandleIdx], Filename, &FreeIt);
    if (Path == NULL) {
      Status = EFI_NOT_FOUND;
      break;
    }
    
    Status = OpenSimpleReadFile(TRUE, NULL, 0, &Path, &DeviceHandle, &ReadHandle);
    if (Status == EFI_SUCCESS)
      break;
    if (FreeIt)
      FreePool(Path);
    
    Path = NULL;
  }
  
  if (Status == EFI_SUCCESS) {
    Status = OpenSimpleFileLen(ReadHandle, &RetSize);
    if (Status == EFI_SUCCESS) {
      Buffer = AllocatePool(RetSize);
      if (Buffer == NULL)
        return EFI_OUT_OF_RESOURCES;
      // read in the file
      Status = ReadSimpleReadFile(ReadHandle, 0, &RetSize, Buffer);
      if (Status == EFI_SUCCESS) {
        *FileBuffer = Buffer;
        *FileSize = RetSize;
      }
    }
  }
  
  if (ReadHandle)
    CloseSimpleReadFile(ReadHandle);
  
  if (FreeIt && Path)
    FreePool(Path);
  
  FreePool(HandleBuffer);
  return Status;
}
