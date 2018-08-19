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

#include "boot.h"

#include "pci.h"

struct EFI_GUID PCIRootBridgeIOProtocol = {
  0x2F707EBB, 
  0x4A1A, 
  0x11D4, 
  {
    0x9A, 0x38, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D
  }
};

void get_pci_info(struct S_BIOS_PCI *pci) {
  EFI_STATUS Status;
  EFI_HANDLE DeviceHandle;
  struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RootBridge;
  
  // File the file system interface to the device
  Status = gBS->HandleProtocol(DeviceHandle, &PCIRootBridgeIOProtocol, (void **) &RootBridge);
  
  // if we got the handle, we assume we have a PCI compatible machine
  pci->sig = 0x20494350;
  pci->flags = 0;  // characteristics
  pci->major = 2;  // PCI Interface level (BCD)
  pci->minor = 0;  // 
  pci->last = 0;   // number of last bus in system
}
