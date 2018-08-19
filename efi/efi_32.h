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

//
// efi.h
//

// which efi version do we support (if it is defined, we support it)
//#define EFI_2_60_SYSTEM_TABLE_REVISION ((2<<16) | (60))
//#define EFI_2_50_SYSTEM_TABLE_REVISION ((2<<16) | (50))
//#define EFI_2_40_SYSTEM_TABLE_REVISION ((2<<16) | (40))
//#define EFI_2_31_SYSTEM_TABLE_REVISION ((2<<16) | (31))
//#define EFI_2_30_SYSTEM_TABLE_REVISION ((2<<16) | (30))
//#define EFI_2_20_SYSTEM_TABLE_REVISION ((2<<16) | (20))
//#define EFI_2_10_SYSTEM_TABLE_REVISION ((2<<16) | (10))
//#define EFI_2_00_SYSTEM_TABLE_REVISION ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION ((1<<16) | (02))

#ifndef EFI_1_02_SYSTEM_TABLE_REVISION
#  error At least version 1.02 must be defined...
#endif

// uefi v2.6: section 2.3.1, pg 23
#ifdef IS64BIT
  typedef bit64u UINTN;       // 64-bit unsigned int
  typedef bit64s INTN;        // 64-bit signed int
  typedef bit64s INT64;
  typedef bit64u UINT64;
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
typedef bit32u UINT32;
typedef bit8u  CHAR8;
typedef bit16u CHAR16;
typedef void   VOID;

typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;

BOOLEAN EFI_ERROR(UINTN Status);

#ifdef IS64BIT
#define EFI_ERROR_BIT    (1<<63)
#else
#define EFI_ERROR_BIT    (1<<31)
#endif

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



#define  EFI_SYSTEM_TABLE_SIGNATURE       0x20494249
#define  EFI_SYSTEM_TABLE_SIGNATURE2      0x54535953
#define  EFI_RUNTIME_SERVICES_SIGNATURE   0x544E5552
#define  EFI_RUNTIME_SERVICES_SIGNATURE2  0x56524553

/* Unless otherwise specified all data types are naturally aligned.
 * Structures are aligned on boundaries equal to the largest internal datum 
 * of the structure and internal data are implicitly padded to achieve 
 * natural alignment.
 */
struct EFI_TABLE_HEADER {
  bit32u Signature[2];
  UINT32 Revision;
  UINT32 HeaderSize;
  UINT32 CRC32;
  UINT32 Reserved;
};

struct EFI_GUID {
  bit32u  Data1;
  bit16u  Data2;
  bit16u  Data3;
  bit8u   Data4[8]; 
};

struct EFI_MEMORY_DESCRIPTOR {
  UINT32 Type;              // EFI_MEMORY_TYPE below
  // next item is naturally aligned (it is a 64-bit qword)
  //  so we must pad 32-bits to get there. 
  // (the compiler won't since we don't support 64-bits yet)
  // (it still thinks it is 2 dwords)
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

#define EVT_TIMER  0x80000000
typedef void *EFI_EVENT;
typedef void (*EFI_EVENT_NOTIFY) (void *Event, void *Context);
EFI_STATUS WaitForSingleEvent(EFI_EVENT Event, bit32u Timeout);

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
  UINTN   (*RaiseTPL)(bit32u NewTPL);
  VOID    (*RestoreTPL)(bit32u OldTPL);
  
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
  
  // ********
  // EFI 1.1+
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
  VOID         (*CopyMem)(void *Destination, void *Source, bit32u Length);
  VOID         (*SetMem)(void *Buffer, bit32u Size, bit8u Value);
  
  // ********
  // EFI 2.0+
  EFI_STATUS   (*CreateEventEx)(bit32u Type, bit32u NotifyTpl, EFI_EVENT_NOTIFY NotifyFunction, const void *NotifyContext, const struct EFI_GUID *EventGroup, void *Event);
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
  UINT16 ScanCode;
  CHAR16 UnicodeChar;
};

struct SIMPLE_TEXT_INPUT_INTERFACE {
  EFI_STATUS   (*Reset)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, bool ExtendedVerification);
  EFI_STATUS   (*ReadKeyStroke)(struct SIMPLE_TEXT_INPUT_INTERFACE *ConIn, struct EFI_INPUT_KEY *Key);
  VOID  *WaitForKey;
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

  /*
  struct EFI_LOADED_IMAGE *SelfLoadedImage;
  Status = gBS->HandleProtocol(gImageHandle, &LoadedImageProtocolGUID, (void **) &SelfLoadedImage);
  if (Status < 0) {
    printf(L"Error while getting a LoadedImageProtocol handle\n");
    return Status;
  }
  */

extern struct EFI_GUID LoadedImageProtocolGUID;

extern EFI_HANDLE gImageHandle;
extern struct EFI_SYSTEM_TABLE *gSystemTable;
extern struct EFI_BOOT_SERVICES *gBS;

bool InitializeLib(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable);

void efi_check_version(void);

//void print(wchar_t *);
