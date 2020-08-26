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
 *  EFI-32.H
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit only
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.C
 */

#pragma pack(push, 1)

typedef unsigned int EFI_STATUS;
typedef void *EFI_HANDLE;

#define  EFI_SUCCESS   0

#define  EFI_SYSTEM_TABLE_SIGNATURE       0x20494249
#define  EFI_SYSTEM_TABLE_SIGNATURE2      0x54535953
#define  EFI_RUNTIME_SERVICES_SIGNATURE   0x544E5552
#define  EFI_RUNTIME_SERVICES_SIGNATURE2  0x56524553

struct EFI_TABLE_HEADER {
  bit32u Signature[2];
  bit32u Revision;
  bit32u HeaderSize;
  bit32u CRC32;
  bit32u Reserved;
};

struct EFI_GUID {
  bit32u  Data1;
  bit16u  Data2;
  bit16u  Data3;
  bit8u   Data4[8]; 
};

struct EFI_CONFIGURATION_TABLE {
  struct EFI_GUID VendorGuid;
  void *VendorTable;
};

struct EFI_SYSTEM_TABLE {
  struct EFI_TABLE_HEADER Hdr;
  bit16u    *FirmwareVendor;
  bit32u     FirmwareRevision;

  EFI_HANDLE ConsoleInHandle;
  struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;

  EFI_HANDLE ConsoleOutHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut;

  EFI_HANDLE StandardErrorHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *StdErr;

  struct EFI_RUNTIME_SERVICES *RuntimeServices;
  
  void  *BootServices;
  
  bit32u NumberOfTableEntries;
  
  struct EFI_CONFIGURATION_TABLE *ConfigurationTable;
};

/*
 * The Simple Text Output Interface protocol
 *  we only "define" the two used functions.  if you use
 *  any of the others, you will need to define their parameters.
 *  (see the efi_boot source folder for a more detailed and
 *   an already defined set.)
 */
struct SIMPLE_TEXT_OUTPUT_INTERFACE {
  void  *Reset;
  void  (*OutputString)(bit16u *text, struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut);
  void  *TestString;
  void  *QueryMode;
  void  *SetMode;
  void  *SetAttribute;
  void (*ClearScreen)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut);
  void  *SetCursorPosition;
  void  *EnableCursor;
  void  *Mode;
};

/*
 * The Simple Text Output Interface protocol
 */
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
  void  *Reset;
  void  *ReadKeyStroke;
  void  *WaitForKey;
};

/*
 * The Runtime Serives
 *  (see the efi_boot source folder for a more detailed and
 *   an already defined set.)
 */
struct EFI_RUNTIME_SERVICES {
  struct EFI_TABLE_HEADER Hdr;
  void  *GetTime;
  void  *SetTime;
  void  *GetWakeUpTime;
  void  *SetWakeUpTime;
  void  *SetVirtualAddressMap;
  void  *ConvertPointer;
  void  *GetVariable;
  void  *GetNextVariableName;
  void  *SetVariable;
  void  *GetNextHighMonotonicCount;
  void  *ResetSystem;
};

#pragma pack(pop)

/*
 * Our prototypes
 */
bool InitializeLib(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable);
void cls(void);
void puts(wchar_t *);
