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
 *  EFI_COMMON.H
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

#ifndef EFI_COMMON_H
#define EFI_COMMON_H

#include "../include/ctype.h"

//
// efi.h
//

// which efi version do we support (if it is defined here, we support it)
//#define EFI_2_60_SYSTEM_TABLE_REVISION ((2<<16) | (60))
//#define EFI_2_50_SYSTEM_TABLE_REVISION ((2<<16) | (50))
//#define EFI_2_40_SYSTEM_TABLE_REVISION ((2<<16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION ((2<<16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION ((2<<16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION ((2<<16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION ((2<<16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION ((1<<16) | (02))

#ifndef EFI_1_02_SYSTEM_TABLE_REVISION
#  error At least version 1.02 must be defined...
#endif

// uefi v2.6: section 2.3.1, pg 23
#ifdef _WIN64
  typedef bit64u UINTN;       // 64-bit unsigned int
  typedef bit64s INTN;        // 64-bit signed int
  typedef bit64u PHYS_ADDRESS;  // physical address
#else
  typedef bit32u UINTN;       // 32-bit unsigned int
  typedef bit32s INTN;        // 32-bit signed int
  typedef bit32u PHYS_ADDRESS;  // physical address
#endif

typedef bool   BOOLEAN;
typedef bit8s  INT8;
typedef bit8u  UINT8;
typedef bit16s INT16;
typedef bit16u UINT16;
typedef bit32s INT32;
typedef bit64u UINT64;
typedef bit64s INT64;
typedef bit32u UINT32;
typedef bit8u  CHAR8;
typedef bit16u CHAR16;
typedef void   VOID;

typedef bit64u EFI_PHYSICAL_ADDRESS;
typedef bit64u EFI_VIRTUAL_ADDRESS;

typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;

#ifdef _WIN64
  #define EFI_ERROR_BIT    ((UINT64) 1<<63)
#else
  #define EFI_ERROR_BIT    ((UINT32) 1<<31)
#endif

#define EFI_ERROR(s) ((Status & EFI_ERROR_BIT) != 0)

#define EFI_SUCCESS                                              0
#define EFI_LOAD_ERROR           ((EFI_STATUS) (EFI_ERROR_BIT |  1))
#define EFI_INVALID_PARAMETER    ((EFI_STATUS) (EFI_ERROR_BIT |  2))
#define EFI_UNSUPPORTED          ((EFI_STATUS) (EFI_ERROR_BIT |  3))
#define EFI_BAD_BUFFER_SIZE      ((EFI_STATUS) (EFI_ERROR_BIT |  4))
#define EFI_BUFFER_TOO_SMALL     ((EFI_STATUS) (EFI_ERROR_BIT |  5))
#define EFI_NOT_READY            ((EFI_STATUS) (EFI_ERROR_BIT |  6))
#define EFI_DEVICE_ERROR         ((EFI_STATUS) (EFI_ERROR_BIT |  7))
#define EFI_WRITE_PROTECTED      ((EFI_STATUS) (EFI_ERROR_BIT |  8))
#define EFI_OUT_OF_RESOURCES     ((EFI_STATUS) (EFI_ERROR_BIT |  9))
#define EFI_VOLUME_CORRUPTED     ((EFI_STATUS) (EFI_ERROR_BIT | 10))
#define EFI_VOLUME_FULL          ((EFI_STATUS) (EFI_ERROR_BIT | 11))
#define EFI_NO_MEDIA             ((EFI_STATUS) (EFI_ERROR_BIT | 12))
#define EFI_MEDIA_CHANGED        ((EFI_STATUS) (EFI_ERROR_BIT | 13))
#define EFI_NOT_FOUND            ((EFI_STATUS) (EFI_ERROR_BIT | 14))
#define EFI_ACCESS_DENIED        ((EFI_STATUS) (EFI_ERROR_BIT | 15))
#define EFI_NO_RESPONSE          ((EFI_STATUS) (EFI_ERROR_BIT | 16))
#define EFI_NO_MAPPING           ((EFI_STATUS) (EFI_ERROR_BIT | 17))
#define EFI_TIMEOUT              ((EFI_STATUS) (EFI_ERROR_BIT | 18))
#define EFI_NOT_STARTED          ((EFI_STATUS) (EFI_ERROR_BIT | 19))
#define EFI_ALREADY_STARTED      ((EFI_STATUS) (EFI_ERROR_BIT | 20))
#define EFI_ABORTED              ((EFI_STATUS) (EFI_ERROR_BIT | 21))
#define EFI_ICMP_ERROR           ((EFI_STATUS) (EFI_ERROR_BIT | 22))
#define EFI_TFTP_ERROR           ((EFI_STATUS) (EFI_ERROR_BIT | 23))
#define EFI_PROTOCOL_ERROR       ((EFI_STATUS) (EFI_ERROR_BIT | 24))
#define EFI_INCOMPATIBLE_VERSION ((EFI_STATUS) (EFI_ERROR_BIT | 25))
#define EFI_SECURITY_VIOLATION   ((EFI_STATUS) (EFI_ERROR_BIT | 26))
#define EFI_CRC_ERROR            ((EFI_STATUS) (EFI_ERROR_BIT | 27))
#define EFI_END_OF_MEDIA         ((EFI_STATUS) (EFI_ERROR_BIT | 28))
#define EFI_END_OF_FILE          ((EFI_STATUS) (EFI_ERROR_BIT | 31))
#define EFI_INVALID_LANGUAGE     ((EFI_STATUS) (EFI_ERROR_BIT | 32))
#define EFI_COMPROMISED_DATA     ((EFI_STATUS) (EFI_ERROR_BIT | 33))


#define  EFI_SYSTEM_TABLE_SIGNATURE       ((UINT64) 0x5453595320494249)
#define  EFI_RUNTIME_SERVICES_SIGNATURE   0x544E5552
#define  EFI_RUNTIME_SERVICES_SIGNATURE2  0x56524553

/* Unless otherwise specified all data types are naturally aligned.
 * Structures are aligned on boundaries equal to the largest internal datum 
 * of the structure and internal data are implicitly padded to achieve 
 * natural alignment.
 */
struct EFI_TABLE_HEADER {
  UINT64 Signature;
  UINT32 Revision;
  UINT32 HeaderSize;
  UINT32 CRC32;
  UINT32 Reserved;
};

#pragma pack(push, 1)

struct EFI_GUID {
  bit32u  Data1;
  bit16u  Data2;
  bit16u  Data3;
  bit8u   Data4[8]; 
};

#pragma pack(pop)

struct EFI_MEMORY_DESCRIPTOR {
  UINT32 Type;              // EFI_MEMORY_TYPE below
  // next item is naturally aligned (it is a 64-bit qword)
  //  so we must pad 32-bits to get there. 
  //bit32u Pad;  // if your compiler won't, add this line
  EFI_PHYSICAL_ADDRESS PhysicalStart;
  EFI_VIRTUAL_ADDRESS VirtualStart;
  UINT64 NumberOfPages;
  UINT64 Attribute;
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

#define EVT_TIMER  0x80000000
typedef void *EFI_EVENT;
typedef UINTN EFI_TPL;
typedef void (*EFI_EVENT_NOTIFY) (EFI_EVENT Event, void *Context);
EFI_STATUS WaitForSingleEvent(EFI_EVENT Event, UINT64 Timeout);

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
  UINT8  Type;
  UINT8  SubType;
  bit16u  Length;  // the specs put this as two bit8u's (I don't know why)
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
  UINT32     Attributes;
  UINT32     OpenCount;
};

EFI_STATUS cpu_init_protocol();

struct EFI_BOOT_SERVICES {
  struct EFI_TABLE_HEADER Hdr;
  
  // ********
  // EFI 1.0+
  // Task priority functions
  UINTN   (*RaiseTPL)(EFI_TPL NewTPL);
  VOID    (*RestoreTPL)(EFI_TPL OldTPL);
  
  // Memory functions
  EFI_STATUS   (*AllocatePages)(EFI_ALLOCATE_TYPE Type, EFI_MEMORY_TYPE MemoryType, UINTN NoPages, EFI_PHYSICAL_ADDRESS *Memory);
  EFI_STATUS   (*FreePages)(EFI_PHYSICAL_ADDRESS Memory, UINTN NoPages);
  EFI_STATUS   (*GetMemoryMap)(UINTN *MemoryMapSize, struct EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);
  EFI_STATUS   (*AllocatePool)(EFI_MEMORY_TYPE PoolType, UINTN Size, void **Buffer);
  EFI_STATUS   (*FreePool)(void *Buffer);
  
  // Event & timer functions
  EFI_STATUS   (*CreateEvent)(UINT32 Type, EFI_TPL NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, void *NotifyContext, EFI_EVENT *Event);
  EFI_STATUS   (*SetTimer)(EFI_EVENT Event, EFI_TIMER_DELAY Type, UINT64 TriggerTime);
  EFI_STATUS   (*WaitForEvent)(UINTN NumberOfEvents, EFI_EVENT *Event, UINTN *Index);
  EFI_STATUS   (*SignalEvent)(EFI_EVENT Event);
  EFI_STATUS   (*CloseEvent)(EFI_EVENT Event);
  EFI_STATUS   (*CheckEvent)(EFI_EVENT Event);

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
  EFI_STATUS   (*LoadImage)(BOOLEAN BootPolicy, EFI_HANDLE ParentImageHandle, struct EFI_DEVICE_PATH_PROTOCOL *FilePath, void *SourceBuffer, UINTN SourceSize, EFI_HANDLE *ImageHandle);
  EFI_STATUS   (*StartImage)(EFI_HANDLE ImageHandle, UINTN *ExitDataSize, CHAR16 **ExitData);
  EFI_STATUS   (*Exit)(EFI_HANDLE ImageHandle, EFI_STATUS ExitStatus, UINTN *ExitDataSize, CHAR16 **ExitData);
  EFI_STATUS   (*UnloadImage)(EFI_HANDLE ImageHandle);
  EFI_STATUS   (*ExitBootServices)(EFI_HANDLE ImageHandle, UINTN MapKey);
  
  // Misc functions
  EFI_STATUS   (*GetNextMonotonicCount)(UINT64 *Count);
  EFI_STATUS   (*Stall)(UINTN Microseconds);
  EFI_STATUS   (*SetWatchdogTimer)(UINTN Timeout, UINT64 WatchDogCode, UINTN DataSize, CHAR16 *WatchdogData);
  
  // ********
  // EFI 1.1+
  // DriverSupport Services
  EFI_STATUS   (*ConnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE *DriverImageHandle, struct EFI_DEVICE_PATH *RemainingDevicePath, BOOLEAN Recursive);
  EFI_STATUS   (*DisconnectController)(EFI_HANDLE ControllerHandle, EFI_HANDLE DriverImageHandle, EFI_HANDLE ChildHandle);
  
  // Open and Close Protocol Services
  EFI_STATUS   (*OpenProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, void **Interface, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle, UINT32 Attributes);
  EFI_STATUS   (*CloseProtocol)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, EFI_HANDLE AgentHandle, EFI_HANDLE ControllerHandle);
  EFI_STATUS   (*OpenProtocolInformation)(EFI_HANDLE Handle, struct EFI_GUID *Protocol, struct EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer, UINTN *EntryCount);

  // Library Services
  EFI_STATUS   (*ProtocolsPerHandle)(EFI_HANDLE Handle, struct EFI_GUID ***ProtocolBuffer, UINTN *ProtocolBufferCount);
  EFI_STATUS   (*LocateHandleBuffer)(EFI_LOCATE_SEARCH_TYPE SearchType, struct EFI_GUID *Protocol, void *SearchKey, UINTN *NoHandles, EFI_HANDLE **Buffer);
  EFI_STATUS   (*LocateProtocol)(struct EFI_GUID *Protocol, void *Registration, void **Interface);
  EFI_STATUS   (*InstallMultipleProtocolInterfaces)(EFI_HANDLE *Handle, ...);
  EFI_STATUS   (*UninstallMultipleProtocolInterfaces)(EFI_HANDLE *Handle, ...);
  
  // 32-bit CRC Services
  EFI_STATUS   (*CalculateCrc32)(void *Data, UINTN DataSize, UINT32 *Crc32);
  
  // Misc Services
  VOID         (*CopyMem)(void *Destination, void *Source, UINTN Length);
  VOID         (*SetMem)(void *Buffer, UINTN Size, UINT8 Value);
  
  // ********
  // EFI 2.0+
  EFI_STATUS   (*CreateEventEx)(UINT32 Type, EFI_TPL NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, const void *NotifyContext, const struct EFI_GUID *EventGroup, EFI_EVENT *Event);
};

struct EFI_CONFIGURATION_TABLE {
  struct EFI_GUID VendorGuid;
  void *VendorTable;
};

struct EFI_SYSTEM_TABLE {
  struct EFI_TABLE_HEADER Hdr;
  CHAR16    *FirmwareVendor;
  UINT32     FirmwareRevision;
  
  EFI_HANDLE ConsoleInHandle;
  struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn;
  
  EFI_HANDLE ConsoleOutHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut;
  
  EFI_HANDLE StandardErrorHandle;
  struct SIMPLE_TEXT_OUTPUT_INTERFACE *StdErr;
  
  struct EFI_RUNTIME_SERVICES *RuntimeServices;
  
  struct EFI_BOOT_SERVICES *BootServices;
  
  UINTN  NumberOfTableEntries;
  struct EFI_CONFIGURATION_TABLE *ConfigurationTable;
};

struct SIMPLE_TEXT_OUTPUT_MODE {
  INT32   MaxMode;
  // current settings
  INT32   Mode;
  INT32   Attribute;
  INT32   CursorColumn;
  INT32   CursorRow;
  BOOLEAN CursorVisible;
};

struct SIMPLE_TEXT_OUTPUT_INTERFACE {
  EFI_STATUS   (*Reset)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, BOOLEAN ExtendedVerification);
  EFI_STATUS   (*OutputString)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, CHAR16 *string);
  EFI_STATUS   (*TestString)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, CHAR16 *string);
  EFI_STATUS   (*QueryMode)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, UINTN ModeNumber, UINTN *Columns, UINTN *Rows);
  EFI_STATUS   (*SetMode)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, UINTN ModeNumber);
  EFI_STATUS   (*SetAttribute)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, UINTN Attribute);
  EFI_STATUS   (*ClearScreen)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut);
  EFI_STATUS   (*SetCursorPosition)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, UINTN Column, UINTN Row);
  EFI_STATUS   (*EnableCursor)(struct SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, BOOLEAN Enable);
  struct SIMPLE_TEXT_OUTPUT_MODE *Mode;
};

#define EFI_SHIFT_STATE_VALID     0x80000000
#define EFI_RIGHT_SHIFT_PRESSED   0x00000001
#define EFI_LEFT_SHIFT_PRESSED    0x00000002
#define EFI_RIGHT_CONTROL_PRESSED 0x00000004
#define EFI_LEFT_CONTROL_PRESSED  0x00000008
#define EFI_RIGHT_ALT_PRESSED     0x00000010
#define EFI_LEFT_ALT_PRESSED      0x00000020
#define EFI_RIGHT_LOGO_PRESSED    0x00000040
#define EFI_LEFT_LOGO_PRESSED     0x00000080
#define EFI_MENU_KEY_PRESSED      0x00000100
#define EFI_SYS_REQ_PRESSED       0x00000200

#define EFI_TOGGLE_STATE_VALID    0x80
#define EFI_KEY_STATE_EXPOSED     0x40
#define EFI_SCROLL_LOCK_ACTIVE    0x01
#define EFI_NUM_LOCK_ACTIVE       0x02
#define EFI_CAPS_LOCK_ACTIVE      0x04

struct EFI_INPUT_KEY {
  UINT16 ScanCode;
  CHAR16 UnicodeChar;
};

typedef UINT8 EFI_KEY_TOGGLE_STATE;

struct EFI_KEY_STATE {
  UINT32 KeyShiftState;
  EFI_KEY_TOGGLE_STATE KeyToggleState;
};

struct EFI_KEY_DATA {
  EFI_INPUT_KEY Key;
  EFI_KEY_STATE KeyState;
};

struct SIMPLE_TEXT_INPUT_INTERFACE {
  EFI_STATUS   (*Reset)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, BOOLEAN ExtendedVerification);
  EFI_STATUS   (*ReadKeyStroke)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, struct EFI_INPUT_KEY *Key);
  VOID  *WaitForKey;
};

typedef EFI_STATUS (*EFI_KEY_NOTIFY_FUNCTION)(EFI_KEY_DATA *KeyData);

struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL {
  EFI_STATUS   (*Reset)(struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleEx, BOOLEAN ExtendedVerification);
  EFI_STATUS   (*ReadKeyStrokeEx)(struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleEx, struct EFI_KEY_DATA *KeyData);
  EFI_EVENT    *WaitForKeyEx;
  EFI_STATUS   (*SetState)(struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleEx, EFI_KEY_TOGGLE_STATE *KeyToggleState);
  EFI_STATUS   (*RegisterKeyNotify)(struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleEx, struct EFI_KEY_DATA *KeyData, EFI_KEY_NOTIFY_FUNCTION KeyNotificationFunction, VOID **NotifyHandle);
  EFI_STATUS   (*UnRegisterKeyNotify)(struct EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleEx, EFI_HANDLE NotificationHandle);
};

//*******************************************************
#define EFI_TIME_ADJUST_DAYLIGHT 0x01
#define EFI_TIME_IN_DAYLIGHT 0x02

struct EFI_TIME {
  UINT16 Year;
  UINT8  Month;
  UINT8  Day;
  UINT8  Hour;
  UINT8  Minute;
  UINT8  Second;
  UINT8  Pad1;
  UINT32 Nanosecond;
  INT16  TimeZone;
  UINT8  Daylight;
  UINT8  Pad2;
};

struct EFI_TIME_CAPABILITIES {
  UINT32  Resolution;     // 1e-6 parts per million
  UINT32  Accuracy;       // hertz
  BOOLEAN SetsToZero;     // Set clears sub-second time
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
  EFI_STATUS   (*GetWakeUpTime)(BOOLEAN *Enabled, BOOLEAN *Pending, struct EFI_TIME *efi_time);
  EFI_STATUS   (*SetWakeUpTime)(BOOLEAN Enable, struct EFI_TIME *efi_time);
  EFI_STATUS   (*SetVirtualAddressMap)(UINTN MemoryMapSize, UINTN DescriptorSize, UINT32 DescriptorVersion, struct EFI_MEMORY_DESCRIPTOR *VirtualMap);
  EFI_STATUS   (*ConvertPointer)(UINTN DebugDisposition, void **Address);
  EFI_STATUS   (*GetVariable)(CHAR16 *VariableName, struct EFI_GUID *VendorGuid, UINT32 *Attributes, UINTN *DataSize, void *Data);
  EFI_STATUS   (*GetNextVariableName)(UINTN *VariableNameSize, CHAR16 *VariableName, struct EFI_GUID *VendorGuid);
  EFI_STATUS   (*SetVariable)(CHAR16 *VariableName, struct EFI_GUID *VendorGuid, UINT32 Attributes, UINTN DataSize, void *Data);
  EFI_STATUS   (*GetNextHighMonotonicCount)(UINT32 *HighCount);
  EFI_STATUS   (*ResetSystem)(EFI_RESET_TYPE ResetType, EFI_STATUS ResetStatus, UINTN DataSize, void *ResetData);
};

struct EFI_LOADED_IMAGE {
  UINT32 Revision;
  EFI_HANDLE ParentHandle;
  struct EFI_SYSTEM_TABLE *SystemTable;
  // Source location of the image
  EFI_HANDLE DeviceHandle;
  struct EFI_DEVICE_PATH_PROTOCOL *FilePath;
  VOID *Reserved;
  // Image’s load options
  UINT32 LoadOptionsSize;
  VOID *LoadOptions;
  // Location where image was loaded
  VOID *ImageBase;
  bit32u ImageSize[2];
  EFI_MEMORY_TYPE ImageCodeType;
  EFI_MEMORY_TYPE ImageDataType;
  EFI_STATUS   (*UnLoad)(EFI_HANDLE ImageHandle);
};

extern struct EFI_GUID LoadedImageProtocolGUID;

extern EFI_HANDLE gImageHandle;
extern struct EFI_SYSTEM_TABLE *gSystemTable;
extern struct EFI_BOOT_SERVICES *gBS;

BOOLEAN InitializeLib(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable);

void efi_check_version(void);

#endif  // EFI_COMMON_H
