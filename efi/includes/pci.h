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

#ifndef PCI_H
#define PCI_H


// So that I don't have to define all protocol handles/functions, I just give them
//  the pointer type.  I don't use any of them anyway.  In future code, we may
//  need to define them.  (Section 13.2 of EFI v2.5 (page 676))
struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL {
  EFI_HANDLE ParentHandle;
  EFI_STATUS  (*PollMem)();
  EFI_STATUS  (*PollIo)();
  EFI_STATUS  (*Mem)();
  EFI_STATUS  (*Io)();
  EFI_STATUS  (*Pci)();
  EFI_STATUS  (*CopyMem)();
  EFI_STATUS  (*Map)();
  EFI_STATUS  (*Unmap)();
  EFI_STATUS  (*AllocateBuffer)();
  EFI_STATUS  (*FreeBuffer)();
  EFI_STATUS  (*Flush)();
  EFI_STATUS  (*GetAttributes)();
  EFI_STATUS  (*SetAttributes)();
  EFI_STATUS  (*Configuration)();
  bit32u SegmentNumber;
};


#endif // PCI_H
