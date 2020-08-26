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
 *  EFI_32.H
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

typedef bit32s EFI_STATUS;
typedef void  *EFI_HANDLE;
typedef bit32u PHYS_ADDRESS;  // physical address

#define EFI_SUCCESS                                           0
#define EFI_LOAD_ERROR           ((EFI_STATUS) (0x80000000 |  1))
#define EFI_INVALID_PARAMETER    ((EFI_STATUS) (0x80000000 |  2))
#define EFI_UNSUPPORTED          ((EFI_STATUS) (0x80000000 |  3))
#define EFI_BAD_BUFFER_SIZE      ((EFI_STATUS) (0x80000000 |  4))
#define EFI_BUFFER_TOO_SMALL     ((EFI_STATUS) (0x80000000 |  5))
#define EFI_NOT_READY            ((EFI_STATUS) (0x80000000 |  6))
#define EFI_DEVICE_ERROR         ((EFI_STATUS) (0x80000000 |  7))
#define EFI_WRITE_PROTECTED      ((EFI_STATUS) (0x80000000 |  8))
#define EFI_OUT_OF_RESOURCES     ((EFI_STATUS) (0x80000000 |  9))
#define EFI_VOLUME_CORRUPTED     ((EFI_STATUS) (0x80000000 | 10))
#define EFI_VOLUME_FULL          ((EFI_STATUS) (0x80000000 | 11))
#define EFI_NO_MEDIA             ((EFI_STATUS) (0x80000000 | 12))
#define EFI_MEDIA_CHANGED        ((EFI_STATUS) (0x80000000 | 13))
#define EFI_NOT_FOUND            ((EFI_STATUS) (0x80000000 | 14))
#define EFI_ACCESS_DENIED        ((EFI_STATUS) (0x80000000 | 15))
#define EFI_NO_RESPONSE          ((EFI_STATUS) (0x80000000 | 16))
#define EFI_NO_MAPPING           ((EFI_STATUS) (0x80000000 | 17))
#define EFI_TIMEOUT              ((EFI_STATUS) (0x80000000 | 18))
#define EFI_NOT_STARTED          ((EFI_STATUS) (0x80000000 | 19))
#define EFI_ALREADY_STARTED      ((EFI_STATUS) (0x80000000 | 20))
#define EFI_ABORTED              ((EFI_STATUS) (0x80000000 | 21))
#define EFI_ICMP_ERROR           ((EFI_STATUS) (0x80000000 | 22))
#define EFI_TFTP_ERROR           ((EFI_STATUS) (0x80000000 | 23))
#define EFI_PROTOCOL_ERROR       ((EFI_STATUS) (0x80000000 | 24))
#define EFI_INCOMPATIBLE_VERSION ((EFI_STATUS) (0x80000000 | 25))
#define EFI_SECURITY_VIOLATION   ((EFI_STATUS) (0x80000000 | 26))
#define EFI_CRC_ERROR            ((EFI_STATUS) (0x80000000 | 27))
#define EFI_END_OF_MEDIA         ((EFI_STATUS) (0x80000000 | 28))
#define EFI_END_OF_FILE          ((EFI_STATUS) (0x80000000 | 31))
#define EFI_INVALID_LANGUAGE     ((EFI_STATUS) (0x80000000 | 32))
#define EFI_COMPROMISED_DATA     ((EFI_STATUS) (0x80000000 | 33))

#define  EFI_SYSTEM_TABLE_SIGNATURE       0x20494249
#define  EFI_SYSTEM_TABLE_SIGNATURE2      0x54535953
#define  EFI_RUNTIME_SERVICES_SIGNATURE   0x544E5552
#define  EFI_RUNTIME_SERVICES_SIGNATURE2  0x56524553

#pragma pack(push, 1)

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

struct EFI_MEMORY_DESCRIPTOR {
  bit32u Type;              // EFI_MEMORY_TYPE below
  bit32u Pad;
  bit32u PhysicalStart[2];  // Field size is 64 bits
  bit32u VirtualStart[2];   // Field size is 64 bits
  bit32u NumberOfPages[2];  // Field size is 64 bits
  bit32u Attribute[2];      // Field size is 64 bits
};

typedef enum {
  AllocateAnyPages,
  AllocateMaxAddress,
  AllocateAddress,
  MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
  EfiReservedMemoryType,
  EfiLoaderCode,
  EfiLoaderData,
  EfiBootServicesCode,
  EfiBootServicesData,
  EfiRuntimeServicesCode,
  EfiRuntimeServicesData,
  EfiConventionalMemory,
  EfiUnusableMemory,
  EfiACPIReclaimMemory,
  EfiACPIMemoryNVS,
  EfiMemoryMappedIO,
  EfiMemoryMappedIOPortSpace,
  EfiPalCode,
  EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// Note: An image that calls ExitBootServices() first calls GetMemoryMap() to obtain the current memory map. 
// Following the ExitBootServices() call, the image implicitly owns all unused memory in the map. This includes 
// memory types EfiLoaderCode, EfiLoaderData, EfiBootServicesCode, EfiBootServicesData, and EfiConventionalMemory.
// A UEFI-compatible loader and operating system must preserve the memory marked as EfiRuntimeServicesCode and 
// EfiRuntimeServicesData. 

typedef void (*EFI_EVENT_NOTIFY) (void *Event, void *Context);
typedef enum {
  TimerCancel,
  TimerPeriodic,
  TimerRelative,
  TimerTypeMax
} EFI_TIMER_DELAY;

typedef enum {
  EFI_NATIVE_INTERFACE,
  EFI_PCODE_INTERFACE
} EFI_INTERFACE_TYPE;

typedef enum {
  AllHandles,
  ByRegisterNotify,
  ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

struct EFI_DEVICE_PATH {
  bit8u  Type;
  bit8u  SubType;
  bit8u  Length[2];
};

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

struct EFI_OPEN_PROTOCOL_INFORMATION_ENTRY {
  EFI_HANDLE AgentHandle;
  EFI_HANDLE ControllerHandle;
  bit32u     Attributes;
  bit32u     OpenCount;
};

struct EFI_BOOT_SERVICES {
  struct EFI_TABLE_HEADER Hdr;
  
  // Task priority functions
  bit32u   (*RaiseTPL)(bit32u NewTPL);
  void     (*RestoreTPL)(bit32u OldTPL);
  
  // Memory functions
  EFI_STATUS   (*AllocatePages)(EFI_ALLOCATE_TYPE Type, EFI_MEMORY_TYPE MemoryType, bit32u NoPages, PHYS_ADDRESS *Memory);
  EFI_STATUS   (*FreePagse)(PHYS_ADDRESS MemoryL, PHYS_ADDRESS MemoryH, bit32u NoPages);
  EFI_STATUS   (*GetMemoryMap)(bit32u *MemoryMapSize, struct EFI_MEMORY_DESCRIPTOR *MemoryMap, bit32u *MapKey, bit32u *DescriptorSize, bit32u *DescriptorVersion);
  EFI_STATUS   (*AllocatePool)(EFI_MEMORY_TYPE PoolType, bit32u Size, void **Buffer);
  EFI_STATUS   (*FreePool)(void *Buffer);
  
  // Event & timer functions
  EFI_STATUS   (*CreateEvent)(bit32u Type, bit32u NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, void *NotifyContext, void *Event);
  EFI_STATUS   (*SetTimer)(void *Event, EFI_TIMER_DELAY Type, bit32u TriggerTimeL, bit32u TriggerTimeH);
  EFI_STATUS   (*WaitForEvent)(bit32u NumberOfEvents, void *Event, bit32u *Index);
  EFI_STATUS   (*SignalEvent)(void *Event);
  EFI_STATUS   (*CloseEvent)(void *Event);
  EFI_STATUS   (*CheckEvent)(void *Event);

  // Protocol handler functions
  EFI_STATUS   (*InstallProtocolInterface)(EFI_HANDLE *Handle, struct EFI_GUID *Protocol, EFI_INTERFACE_TYPE InterfaceType, void *Interface);
  EFI_STATUS   (*ReinstallProtocolInterface)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void *OldInterface, void *NewInterface);
  EFI_STATUS   (*UninstallProtocolInterface)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void *Interface);
  EFI_STATUS   (*HandleProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void **Interface);
  EFI_STATUS   (*PCHandleProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void **Interface);
  EFI_STATUS   (*RegisterProtocolNotify)(struct EFI_GUID *Protocol, void *Event, void **Registration);
  EFI_STATUS   (*LocateHandle)(EFI_LOCATE_SEARCH_TYPE SearchType, struct EFI_GUID *Protocol, void *SearchKey, bit32u *BufferSize, EFI_HANDLE *Buffer);
  EFI_STATUS   (*LocateDevicePath)(struct EFI_GUID *Protocol, struct EFI_DEVICE_PATH **DevicePath, EFI_HANDLE *Device);
  EFI_STATUS   (*InstallConfigurationTable)(struct EFI_GUID *Guid, void *Table);
  
  // Image functions
  EFI_STATUS   (*LoadImage)(bool BootPolicy, EFI_HANDLE ParentImageHandle, struct EFI_DEVICE_PATH *FilePath, void *SourceBuffer, bit32u SourceSize, EFI_HANDLE *ImageHandle);
  EFI_STATUS   (*StartImage)(EFI_HANDLE ImageHandle, bit32u *ExitDataSize, bit16u **ExitData);
  EFI_STATUS   (*Exit)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, bit32u *ExitDataSize, bit16u **ExitData);
  EFI_STATUS   (*UnloadImage)(EFI_HANDLE ImageHandle);
  EFI_STATUS   (*ExitBootServices)(EFI_HANDLE ImageHandle, bit32u MapKey);
  
  // Misc functions
  EFI_STATUS   (*GetNextMonotonicCount)(bit32u *Count);  // should be bit64u
  EFI_STATUS   (*Stall)(bit32u Microseconds);
  EFI_STATUS   (*SetWatchdogTimer)(bit32u Timeout, bit32u WatchDogCodeL, bit32u WatchDogCodeH, bit32u DataSize, bit16u *WatchdogData);

  // DriverSupport Services
  EFI_STATUS   (*ConnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE *DriverImageHandle, struct EFI_DEVICE_PATH *RemainingDevicePath, bool Recursive);
  EFI_STATUS   (*DisconnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE DriverImageHandle, EFI_HANDLE ChildHandle);
  
  // Open and Close Protocol Services
  EFI_STATUS   (*OpenProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void **Interface, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle, bit32u Attributes);
  EFI_STATUS   (*CloseProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle);
  EFI_STATUS   (*OpenProtocolInformation)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, struct EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer, bit32u *EntryCount);

  // Library Services
  EFI_STATUS   (*ProtocolsPerHandle)(EFI_HANDLE Handle, struct EFI_GUID ***ProtocolBuffer, bit32u *ProtocolBufferCount);
  EFI_STATUS   (*LocateHandleBuffer)(EFI_LOCATE_SEARCH_TYPE SearchType, struct EFI_GUID *Protocol, void *SearchKey, bit32u *NoHandles, EFI_HANDLE **Buffer);
  EFI_STATUS   (*LocateProtocol)(struct EFI_GUID *Protocol, void *Registration, void **Interface);
  EFI_STATUS   (*InstallMultipleProtocolInterfaces)(EFI_HANDLE *Handle, ...);
  EFI_STATUS   (*UninstallMultipleProtocolInterfaces)(EFI_HANDLE *Handle, ...);
  
  // 32-bit CRC Services
  EFI_STATUS   (*CalculateCrc32)(void *Data, bit32u DataSize, bit32u *Crc32);
  
  // Misc Services
  void         (*CopyMem)(void *Destination, void *Source, bit32u Length);
  void         (*SetMem)(void *Buffer, bit32u Size, bit8u Value);
  
  EFI_STATUS   (*CreateEventEx)(bit32u Type, bit32u NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, const void *NotifyContext, const struct EFI_GUID *EventGroup, void *Event);
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
  struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn;

  EFI_HANDLE ConsoleOutHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut;

  EFI_HANDLE StandardErrorHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *StdErr;

  struct EFI_RUNTIME_SERVICES *RuntimeServices;
  
  struct EFI_BOOT_SERVICES *BootServices;
  
  bit32u NumberOfTableEntries;
  struct EFI_CONFIGURATION_TABLE *ConfigurationTable;
};

struct SIMPLE_TEXT_OUTPUT_MODE {
  int  MaxMode;
  // current settings
  int  Mode;
  int  Attribute;
  int  CursorColumn;
  int  CursorRow;
  bool CursorVisible;
};

struct SIMPLE_TEXT_OUTPUT_INTERFACE {
  EFI_STATUS   (*Reset)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bool ExtendedVerification);
  EFI_STATUS   (*OutputString)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, wchar_t *string);
  EFI_STATUS   (*TestString)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, wchar_t *string);
  EFI_STATUS   (*QueryMode)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bit32u ModeNumber, bit32u *Columns, bit32u *Rows);
  EFI_STATUS   (*SetMode)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bit32u ModeNumber);
  EFI_STATUS   (*SetAttribute)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bit32u Attribute);
  EFI_STATUS   (*ClearScreen)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut);
  EFI_STATUS   (*SetCursorPosition)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bit32u Column, bit32u Row);
  EFI_STATUS   (*EnableCursor)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, bool Enable);
  struct SIMPLE_TEXT_OUTPUT_MODE *Mode;
};

struct EFI_INPUT_KEY {
  bit16u ScanCode;
  bit16u UnicodeChar;
};

struct SIMPLE_TEXT_INPUT_INTERFACE {
  EFI_STATUS   (*Reset)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, bool ExtendedVerification);
  EFI_STATUS   (*ReadKeyStroke)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, struct EFI_INPUT_KEY *Key);
  void  *WaitForKey;
};

//*******************************************************
#define EFI_TIME_ADJUST_DAYLIGHT 0x01
#define EFI_TIME_IN_DAYLIGHT 0x02

struct EFI_TIME {
  bit16u Year;
  bit8u  Month;
  bit8u  Day;
  bit8u  Hour;
  bit8u  Minute;
  bit8u  Second;
  bit8u  Pad1;
  bit32u Nanosecond;
  bit16u TimeZone;
  bit8u  Daylight;
  bit8u  Pad2;
};

struct EFI_TIME_CAPABILITIES {
  bit32u Resolution;     // 1e-6 parts per million
  bit32u Accuracy;       // hertz
  bool   SetsToZero;     // Set clears sub-second time
};

typedef enum {
  EfiResetCold,
  EfiResetWarm,
  EfiResetShutdown
} EFI_RESET_TYPE;

struct EFI_RUNTIME_SERVICES {
  struct EFI_TABLE_HEADER Hdr;
  EFI_STATUS   (*GetTime)(struct EFI_TIME *efi_time, struct EFI_TIME_CAPABILITIES *caps);
  EFI_STATUS   (*SetTime)(struct EFI_TIME *efi_time);
  EFI_STATUS   (*GetWakeUpTime)(bool *Enabled, bool *Pending, struct EFI_TIME *efi_time);
  EFI_STATUS   (*SetWakeUpTime)(bool Enable, struct EFI_TIME *efi_time);
  EFI_STATUS   (*SetVirtualAddressMap)(bit32u MemoryMapSize, bit32u DescriptorSize, bit32u DescriptorVersion, struct EFI_MEMORY_DESCRIPTOR *VirtualMap);
  EFI_STATUS   (*ConvertPointer)(bit32u DebugDisposition, void **Address);
  EFI_STATUS   (*GetVariable)(wchar_t *VariableName, struct EFI_GUID *VendorGuid, bit32u *Attributes, bit32u *DataSize, void *Data);
  EFI_STATUS   (*GetNextVariableName)(bit32u *VariableNameSize, wchar_t *VariableName, struct EFI_GUID *VendorGuid);
  EFI_STATUS   (*SetVariable)(wchar_t *VariableName, struct EFI_GUID *VendorGuid, bit32u Attributes, bit32u DataSize, void *Data);
  EFI_STATUS   (*GetNextHighMonotonicCount)(bit32u *HighCount);
  EFI_STATUS   (*ResetSystem)(EFI_RESET_TYPE ResetType, EFI_STATUS ResetStatus, bit32u DataSize, wchar_t *ResetData);
};



#pragma pack(pop)


bool InitializeLib(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable);

void cls(void);
void print(wchar_t *);

