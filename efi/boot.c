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

#include "conin.h"
#include "conout.h"
#include "memory.h"
#include "stdlib.h"
#include "string.h"

#include "file_io.h"
#include "loaderbz.h"
#include "mouse.h"
#include "path.h"
#include "progress.h"
#include "video.h"
#include "volume.h"

/*  The Firmware that QEMU uses, places the firmware code/data at 0x0080000,
 *   right were we place our Kernel.sys file.  Therefore, we can't use the
 *   firmware to allocate this memory to copy our kernel.sys file to it.
 *  However:
 *  We can't exit boot services, then copy the data to the physical locations,
 *   because we don't know if we would be overwriting SystemFiles[1].Data with
 *   SystemFiles[0]'s physical location.  i.e.: The firmware could have allocated
 *   the space to store another file at SystemFiles[0]'s physical location.
 *  Luckily, it allocates near top of memory so this doesn't happen.
 *  Therefore, we can safely move the data *after* we exit boot services.
 *  However, this is an assumption and a hack just to get QEMU to work.
 *  
 *  The best thing to do then is to make kernel.sys relocatable as well as all
 *   other files we load.  This will be a lot of work to the kernel's source files though...
 *  
 *  Uncomment the line below to get this code to work with QEMU.
 */
#define DO_QEMU_HACK


#if MEM_DEBUG
  wchar_t *efi_memory_types[EfiMaxMemoryType] = {
    L"EfiReservedMemoryType",
    L"EfiLoaderCode",
    L"EfiLoaderData",
    L"EfiBootServicesCode",
    L"EfiBootServicesData",
    L"EfiRuntimeServicesCode",
    L"EfiRuntimeServicesData",
    L"EfiConventionalMemory",
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
  };
#endif

////////////////////////////////////////////////////////////////////////////////
// Our main data block we transfer to the kernel
struct GDT act_gdt[3] = {
  { 
    0,  // first is always null.
  },
  {
    0xFFFF,   // -------------> limit 4gig + (byte below)
    0x0000,   // ______/------> base at 0x00000000
    0x00,     // /----/
    0x9A,     // |   Code(E/R), S=1, Priv = 00b, present = Yes
    0xCF,     // |   F (limit), avl = 0, 0, 32-bit, gran = 1
    0x00      // /
  },
  {
    // data desc (matches cs (mostly))
    0xFFFF,   // -------------> limit 4gig + (byte below)
    0x0000,   // ______/------> base at 0x00000000
    0x00,     // /----/
    0x92,     // |   Data(R/W), S=1, Priv = 00b, present = Yes
    0xCF,     // |   F (limit), avl = 0, 0, 32-bit, gran = 1
    0x00      // /
  }
};

#define S_LOADER_MAGIC0  0x464F5245  // 'FORE'
#define S_LOADER_MAGIC1  0x56455259  // 'VERY'
#define S_LOADER_MAGIC2  0x4F554E47  // 'OUNG'
#define S_LOADER_MAGIC3  0x534F4654  // 'SOFT'

#pragma pack(push, 1)

struct SYSTEM_BLOCK {
  bit32u magic0;
  bit16u gdtoff;                // Address of our GDT                          //    2
  bit32u gdtoffa;               //  KERN_GDT in memory.h                       //    4
  bit16u idtoff;                // 256 = number of interrupts we allow         //    2
  bit32u idtoffa;               //  KERN_IDT in memory.h                       //    4
  bit32u bios_type;             // 'BIOS' = legacy BIOS, 'UEFI' = UEFI booted          //    4
  bit32u uefi_image_handle;     // UEFI Image Handle                                   //    4
  bit32u uefi_system_table;     // UEFI System Table Pointer                           //    4
  struct S_BOOT_DATA boot_data; // booted data                                         //   48
  bit8u  resv0[32];             // reserved                                            //   32
  bit32u magic1;
  struct S_BIOS_PCI bios_pci;   // PCI information from the BIOS                       //    8
  bit32u org_int1e;             // original INT1Eh address                             //    4
  struct S_FLOPPY1E floppy_1e;  // floppies status                                     //   11
  struct S_TIME time;         // current time passed to kernel                         //   14
  struct S_APM apm;           // Advanced Power Management                             //   44
  bool   has_cpuid;           // set if we detect a 486+ with a CPUID instruction      //    1
  bool   has_rdtsc;           // set if we detect a 486+ with a RDTSC instruction      //    1
  bool   is_small_machine;    // set if we detect/set for a "small" machine            //    1
  bit16u bios_equip;          // bios equipment list at INT 11h (or 0040:0010h)        //    2
  bit8u  kbd_bits;            // bits at 0x00417                                       //    1
  bit32u magic2;
  struct S_MEMORY memory;     // memory blocks returned by INT 15h memory services     // 1356
  bit8u  a20_tech;            // the technique number used to enable the a20 line      //    1
  bool   text_only;           // Use screen mode 3, text only                          //    1
  bit16u vid_mode_cnt;        // count of video mode info blocks found                 //    2
  bit16u cur_vid_index;       // index into mode_info[] of chosen/default/current mode //    2
  struct S_MODE_INFO mode_info[VIDEO_MAX_MODES]; // video modes information for kernel //   VIDEO_MAX_MODES * 24 (768)
  bit8u  resv2[28];           // reserved                                              //   29
  struct S_DRV_PARAMS drive_params[10];  // up to 10 hard drive parameter tables       //  960
  bit8u  padding[1795];       // padding to 0x1400 bytes                               // 
  bit32u magic3;
} sys_block;

#pragma pack(pop)

asm (
  "%if (($ - _sys_block) != 0x1400) \n"
  "%error *** transfer block not 0x1400 bytes in size ****   \n"
  "%endif  \n"
);

// Physical addresses and location for setting up GDT
asm (
  "section .data       \n"
	"  align 16          \n"
  "_gdtoff:            \n"
  "  dw  ((256*8)-1)   \n"
  "  dd  0x00110000    \n"
  "_idtoff:            \n"
  "  dw  ((256*8)-1)   \n"
  "  dd  0x00110800    \n"
);

// this is the list of files we need to load via this loader
// They can be any length but no paths, and must have valid characters
//  for all filesystems used.  However, since FAT only allows 8.3 format
//  filenames, this must remain 8.3 (unless we write code to get LFN's in FAT)
#define FILE_COUNT  2
struct FILES SystemFiles[FILE_COUNT] = {
  { L"kernel.sys", NULL, 0, 0, TRUE },
  { L"system.sys", NULL, 0, 0, FALSE }
};

bit32u kernel_base = 0;

// Our Main.  This is what gets called by the EFI Entry
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable) {
  int i;
  EFI_STATUS Status;
  bit32u MemMapKey;
  struct EFI_INPUT_KEY Key;
  
  // initialize our library code
  if (!InitializeLib(ImageHandle, SystemTable))
    return 1;
  
  // clear the screen
  cls();
  
  // turn off the video cursor
  cursor(FALSE);
  
  // Status line
  printf(L"%[ Press F8 to choose video screen mode.                                         \r\n", EFI_BACKGROUND_LIGHTGRAY | EFI_BLACK);
  SetTextAttribute(EFI_BACKGROUND_BLACK, EFI_LIGHTGRAY);
  
  // print the Hello World string
  printf(L"%]The %[FYSOS %[EFI Boot Loader%]      v%[1.01.00%]        %[(C)opyright 1984-2018%]\r\n"
         L"Loading FYSOS...\r\n", 
         EFI_BACKGROUND_BLACK | EFI_LIGHTGREEN, // FYSOS
         EFI_BACKGROUND_BLACK | EFI_LIGHTBLUE,  // EFI Boot Loader
         EFI_BACKGROUND_BLACK | EFI_WHITE,      // version
         EFI_BACKGROUND_BLACK | EFI_WHITE);     // Copyright
  
  // check that we can run on this version of EFI
  efi_check_version();  // doesn't return if not
  
#if UEFI_INCLUDE_MOUSE
  // The Mouse is not supported in the version of UEFI that VBox and QEMU support
  //ConnectAll();
  //int r = InitMouse();
  //if (r)
  //  UpdateMouse();
  //freeze();
#endif
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear our parameter block, and set the defaults
  memset(&sys_block.magic0, 0, 0x1400);
  sys_block.magic0 = S_LOADER_MAGIC0;
  sys_block.magic1 = S_LOADER_MAGIC1;
  sys_block.magic2 = S_LOADER_MAGIC2;
  sys_block.magic3 = S_LOADER_MAGIC3;
  sys_block.gdtoff  = ((256*8)-1);
  sys_block.gdtoffa = 0x00110000;
  sys_block.idtoff  = ((256*8)-1);
  sys_block.idtoffa = 0x00110800;
  sys_block.has_cpuid = 1;  // if we have UEFI, we have CPUID
  sys_block.has_rdtsc = 1;  // if we have UEFI, we have RDTSC ?????
  
  // Mark that we are using a UEFI BIOS to boot
  sys_block.bios_type = 0x55454649;  // 'UEFI'
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // set the a20 activate technique to uEFI
  sys_block.a20_tech = 16;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // get keyboard bits
  sys_block.kbd_bits = 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // one of the last things we need to do before we jump,
  // is get the time from the bios.
  get_bios_time(&sys_block.time);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // see if we have and APM BIOS
  //apm_bios_check(&apm);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Now get the PCI information. (Detection only)
  get_pci_info(&sys_block.bios_pci);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Now load the system file(s)
  //  Search for the files in root directory
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // print the loading string
  printf(L"%[Loading system files...%]\r\n", HIGHLIGHT_COLOR);
  
  for (i=0; i<FILE_COUNT; i++) {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // print the status string
    printf(L"Loading: %[%s%]", HIGHLIGHT_COLOR, SystemFiles[i].FileName);
    LoadFile(&SystemFiles[i]);
    printf(L"\r\n");
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // get the memory map from the uEFI BIOS
  // (The first time is for our kernel data block)
  get_memory(&sys_block.memory, &MemMapKey);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Get the available video modes
  sys_block.vid_mode_cnt = GetVideoInfo(sys_block.mode_info);
  if (sys_block.vid_mode_cnt > 0) {
    // see if a key was pressed (F8 maybe) and if so,
    //  let the user choose a mode, else choose one
    //  for them
    sys_block.cur_vid_index = FindVideoMode(sys_block.mode_info, sys_block.vid_mode_cnt, 800, 600);
    if (sys_block.cur_vid_index == 0xFFFF)
      sys_block.cur_vid_index = FindVideoMode(sys_block.mode_info, sys_block.vid_mode_cnt, 640, 480);
    if ((sys_block.cur_vid_index == 0xFFFF) || ((kbhit(&Key) == EFI_SUCCESS) && (Key.ScanCode == SCAN_F8))) {
      // print the available modes and ask for a selection of one of them.
      // if there are more modes than will fit on the screen, an 'enter' key
      //  will scroll to the next page.
      // the ESC key will exit
      int avail[24];
      int j, i = 0;
      bit16u ch;
next_entry:
      for (j=0; i<sys_block.vid_mode_cnt && j<23; i++) {
        printf(L" %[%c%]: %4i x %4i %2i bits\r\n", HIGHLIGHT_COLOR, j + 'A', sys_block.mode_info[i].xres, sys_block.mode_info[i].yres, sys_block.mode_info[i].bits_per_pixel);
        avail[j] = i;
        j++;
      }
      if (i < sys_block.vid_mode_cnt)
        printf(L" ** '%[Enter%]' for more\r\n", HIGHLIGHT_COLOR);
      do {
        printf(L"Please select a video mode to use %[[A-%c]%]: ", HIGHLIGHT_COLOR, j + 'A' - 1);
        getkeystroke(&Key);
        if (Key.ScanCode == SCAN_ESC)  { // esc (start over)
          i = 0;
          printf(L"\r\n");
          goto next_entry;
        }
        if ((Key.UnicodeChar == CHAR_CARRIAGE_RETURN) && (i < sys_block.vid_mode_cnt)) {
          printf(L"\r\n");
          goto next_entry;
        }
        ch = toupper(Key.UnicodeChar);
        printf(L"%c\r\n", ch);
        ch -= 'A';
      } while ((ch < 0) || (ch >= j));
      sys_block.cur_vid_index = avail[ch];
    }
  } else {
    printf(L"%[FYSOS requires a VESA capable video card with a Linear Base Frame Buffer...%]\r\n", ERROR_COLOR);
    freeze();
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear the boot_data structure.
  // since we didn't boot from a legacy type MBR or PBR, the boot_data
  //  struct won't have any valid items in it (yet).
  memset(&sys_block.boot_data, 0, SIZEOF_S_BOOT_DATA);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We run through all of the disk drives
  // (this also sets boot_data.base_lba for us)
  GetVolumeInfo(sys_block.drive_params, &sys_block.boot_data);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // copy the files to their physical memory locations
  // (all updates to LOADER block must be done before here)
  void *PhysAddr;
#ifndef DO_QEMU_HACK
  for (i=0; i<FILE_COUNT; i++) {
    if (SystemFiles[i].IsKernel) {
      // we need to allocate the page before it too, so that we
      //  can copy the data block to it.
      PhysAddr = AllocatePhysical(SystemFiles[i].Target - 0x1000, SystemFiles[i].Size + 0x1000);
      if (PhysAddr) {
        printf(L"Moving %[%s%] to physical address %[0x%08X%]\r\n", HIGHLIGHT_COLOR, SystemFiles[i].FileName, HIGHLIGHT_COLOR, SystemFiles[i].Target);
        memcpy((void *) SystemFiles[i].Target, SystemFiles[i].Data, SystemFiles[i].Size);
        // move the 0x1400 byte block of data to just before the kernel
        // kernel base should be on a meg boundary and since is a PE file,
        //  will have the first 0x400 bytes free for our use.  Therefore,
        //  we back up 0x1000 from the base and this is how we have 0x1400
        //  bytes of space for use.
        memcpy((void *) (SystemFiles[i].Target - 0x1000), &sys_block.magic0, 0x1400);
      } else freeze();
    } else {
      PhysAddr = AllocatePhysical(SystemFiles[i].Target, SystemFiles[i].Size);
      if (PhysAddr) {
        printf(L"Moving %[%s%] to physical address %[0x%08X%]\r\n", HIGHLIGHT_COLOR, SystemFiles[i].FileName, HIGHLIGHT_COLOR, (bit32u) PhysAddr);
        memcpy(PhysAddr, SystemFiles[i].Data, SystemFiles[i].Size);
      } else freeze();
    }
  }
#endif
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // move the actual GDT entries (first 3)
  PhysAddr = AllocatePhysical(sys_block.gdtoffa, 256 * 8);
  if (PhysAddr) {
    printf(L"Moving GDT's to physical address %[0x%08X%]\r\n", HIGHLIGHT_COLOR, (bit32u) PhysAddr);
    memset(PhysAddr, 0, 256 * 8);
    memcpy(PhysAddr, &act_gdt, 3 * 8);
  } else {
    printf(L"%[Could not move GDT's to physical address%]\r\n", ERROR_COLOR);
    freeze();
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Change to that screen mode
  SetVidMode(mode_nums[sys_block.cur_vid_index]);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Finally get the memory map from the uEFI BIOS
  // (This should be one of the last things before we ExitBootServices());
  // (The second time is so the UEFI will exit successfully)
  get_memory(&sys_block.memory, &MemMapKey);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // shutdown the uEFI bios
  Status = gBS->ExitBootServices(ImageHandle, MemMapKey);
  if (EFI_ERROR(Status)) {
    printf(L"%[We didn't shut down the Boot Serivces...(%X)%]\r\n", ERROR_COLOR, Status);
    freeze();
  }
  
// Page: 151
//Following the
//ExitBootServices() call, the image implicitly owns all unused memory in the map. This
//includes memory types EfiLoaderCode, EfiLoaderData, EfiBootServicesCode,
//EfiBootServicesData, and EfiConventionalMemory. An EFI-compatible loader and
//operating system must preserve the memory marked as EfiRuntimeServicesCode and
//EfiRuntimeServicesData.

#ifdef DO_QEMU_HACK
  void _memcpy(void *targ, void *src, UINTN len);
  
  for (i=0; i<FILE_COUNT; i++) {
    _memcpy((void *) SystemFiles[i].Target, SystemFiles[i].Data, SystemFiles[i].Size);
    if (SystemFiles[i].IsKernel) {
      // move the 0x1400 byte block of data to just before the kernel
      // kernel base should be on a meg boundary and since is a PE file,
      //  will have the first 0x400 bytes free for our use.  Therefore,
      //  we back up 0x1000 from the base and this is how we have 0x1400
      //  bytes of space for use.
      _memcpy((void *) (SystemFiles[i].Target - 0x1000), &sys_block.magic0, 0x1400);
    }
  }
#endif
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We now have all the system files loaded and stored in 
  //  buffers pointed to by 'SystemFiles'
  // We now need to get this data to physical memory, set up
  //  our flat address space, and jump to the kernel...
  
  // after  ExitBootServices(), the Runtime calls are still available.
  //  the shell usually loads an EFI application (a PE file) which uses
  //  the boot and runtime to load other files and drivers.  Once done,
  //  it  ExitBootServices() and still uses the runtime to load the
  //  operating system.
  // As long as the access to the runtime is a flat address space, it
  //  can be accessed however and whenever (with a few exceptions).

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // load the actual GDT and IDT
  asm (
    "  cli              \n"
    "  lgdt  [_gdtoff]  \n"
    "  lidt  [_idtoff]  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // we need to make sure that CR4.TSD = 0 (bit 2)
  // i.e.: allows the RDTSC instruction
  // we need to make sure that CR4.VME = 0 (bit 0)
  asm (
    "  mov  eax,cr4  \n"
    "  and  al,~5    \n"
    "  mov  cr4,eax  \n"
    "  ; we need to add to _kernel_base now, before we\n"
    "  ; change the ds register/selector below.\n"
    "  mov  eax,[_kernel_base]   \n"
    "  add  eax,400h             \n"
    "  mov  [__kernel_base],eax  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  set up the segment descriptors
  //  ds, es, fs, and ss have a base of 00000000h
  asm (
    //"  mov  eax," #FLATDATA " ; Selector for 4Gb data seg \n"
    "  mov  eax,0x00000010 ; Selector for 4Gb data seg \n"
    
    "  mov  ds,ax         ;  ds  \n"
    "  mov  es,ax         ;  es  \n"
    "  mov  fs,ax         ;  fs  \n"
    "  mov  gs,ax         ;  gs  \n"
    "  mov  ss,ax         ;  ss  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  set up a stack at STACK_BASE (physical) of 4 meg size
  //asm ("  mov  esp,((" #STACK_BASE " + 00400000h) - 4) \n");
  asm ("  mov  esp,((0x01000000 + 00400000h) - 4) \n" );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We now have PMODE setup and all our segment selectors correct.
  // CS              = 0x00000000
  // SS & remaining  = 0x00000000
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Here is the jump.
  // We will jump to physical address 'kernel_base' + 400h
  
  asm (
    "              db  0EAh           \n"
    "__kernel_base dd  0  ; *in* pmode, so *dword* sized \n"
    //"              dw  " #FLATCODE "  \n"
    "              dw  0x00000008  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Kernel file should have taken over from here
  //   but to be sure
  //printf(L" We didn't make it dude...\n");
  freeze();
}

//// http://stackoverflow.com/questions/17591351/converting-efi-memory-map-to-e820-map
EFI_STATUS get_memory(struct S_MEMORY *memory, bit32u *rMemMapKey){
  
  // clear it out
  memset(memory, 0, sizeof(struct S_MEMORY));
  memory->size[0] = 0x00100000;  // start with 1meg already
  
  EFI_STATUS Status = EFI_SUCCESS;
  bit32u MemMapSize = sizeof(struct EFI_MEMORY_DESCRIPTOR) * 16;
  bit32u MemMapSizeOut = MemMapSize;
  bit32u MemMapKey = 0;
  bit32u MemMapDescriptorSize = 0;
  bit32u MemMapDescriptorVersion = 0;
  bit32u DescriptorCount = 0;
  bit32u i = 0, j = 48;  // we only have 48 memory slots to fill in S_MEMORY
  bit8u *Buffer = NULL;
  struct EFI_MEMORY_DESCRIPTOR *MemoryDescriptorPtr = NULL;
  struct EFI_INPUT_KEY Key;
  
  do {
    Buffer = AllocatePool(MemMapSize);
    if (Buffer == NULL)
      break;
    
    Status = gBS->GetMemoryMap(&MemMapSizeOut, (struct EFI_MEMORY_DESCRIPTOR *) Buffer, 
        &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
    // The only reason it won't return success is that we didn't give it enough memory
    //  to fill.  So add to it and try again.
    if (EFI_ERROR(Status)) {
      FreePool(Buffer);
      MemMapSize += sizeof(struct EFI_MEMORY_DESCRIPTOR) * 16;
    }
  } while (EFI_ERROR(Status));
  
  if (Buffer != NULL) {
    DescriptorCount = MemMapSizeOut / MemMapDescriptorSize;
    MemoryDescriptorPtr = (struct EFI_MEMORY_DESCRIPTOR *) Buffer;
    
#if MEM_DEBUG
    printf(L"MemoryMap: DescriptorCount: %i\r\n", DescriptorCount);
#endif
    memory->word = 16; // (our) type goes here
    for (i=0; (i<DescriptorCount) && (j>0); i++, j--) {
      MemoryDescriptorPtr = (struct EFI_MEMORY_DESCRIPTOR *) (Buffer + (i * MemMapDescriptorSize));

#if MEM_DEBUG
      printf(L"Start: 0x%08X -> 0x%08X, Pages: %i, %s\r\n",
        MemoryDescriptorPtr->PhysicalStart[0],
        MemoryDescriptorPtr->PhysicalStart[0] + (MemoryDescriptorPtr->NumberOfPages[0] * 4096) - 1,
        MemoryDescriptorPtr->NumberOfPages[0],
        efi_memory_types[MemoryDescriptorPtr->Type]);
      getkeystroke(&Key);
#endif
      
      // convert to bytes (from pages)
      shl64(MemoryDescriptorPtr->NumberOfPages, 12);
      
      memory->block[memory->blocks].type = MemoryDescriptorPtr->Type;
      memory->block[memory->blocks].base[0] = MemoryDescriptorPtr->PhysicalStart[0];
      memory->block[memory->blocks].base[1] = MemoryDescriptorPtr->PhysicalStart[1];
      memory->block[memory->blocks].size[0] = MemoryDescriptorPtr->NumberOfPages[0];
      memory->block[memory->blocks].size[1] = MemoryDescriptorPtr->NumberOfPages[1];
      memory->block[memory->blocks].attrib[0] = MemoryDescriptorPtr->Attribute[0];  // Page 157 in EFI v2.5
      memory->block[memory->blocks].attrib[1] = MemoryDescriptorPtr->Attribute[1];  // lxr.free-electrons.com/source/include/linux/efi.h#L601
      
      // add to the accumulator
      add64(memory->size, MemoryDescriptorPtr->NumberOfPages);
      memory->blocks++;
    }
    FreePool(Buffer);
  }
  
  // make sure we have enough memory to run our kernel
  if ((memory->size[1] == 0) && (memory->size[0] < MIN_MEM_REQU)) {
    printf(L"\r\n  *** Not enough physical memory ***"
           L"\r\n   Size in megabytes found: %i"
           L"\r\n     Size of memory needed: %i", memory->size[0] >> 20, MIN_MEM_REQU >> 20);
    freeze();
  }
  
  *rMemMapKey = MemMapKey;
  return Status;
}

void get_bios_time(struct S_TIME *time) {
  struct EFI_TIME efi_time;
  
  EFI_STATUS Status = gSystemTable->RuntimeServices->GetTime(&efi_time, NULL);
  if (!EFI_ERROR(Status)) {
    time->year = efi_time.Year;
    time->month = efi_time.Month;
    time->day = efi_time.Day;
    time->hour = efi_time.Hour;
    time->min = efi_time.Minute;
    time->sec = efi_time.Second;
    time->jiffy = (efi_time.Nanosecond / 10000000);
    time->msec = (efi_time.Nanosecond / 1000000);
    time->d_savings = ((efi_time.Daylight & EFI_TIME_IN_DAYLIGHT) > 0);
    time->weekday = 0;
    time->yearday = 0;
  }
}

//http://www.cplusplus.com/forum/beginner/110402/
EFI_STATUS LoadFile(struct FILES *SystemFile) {
  struct EFI_DEVICE_PATH *Path;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE ReadHandle = NULL;
  bit32u HandleCount, HandleIdx;
  EFI_HANDLE *HandleBuffer;
  bit32u RetSize;
  void *Buffer;
  
  Status = gBS->LocateHandleBuffer(ByProtocol, &FileSystemProtocol, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status))
    return Status;
  
  for (HandleIdx = 0; HandleIdx < HandleCount; HandleIdx++) {
    EFI_HANDLE DeviceHandle;
    
    Path = FileDevicePath(HandleBuffer[HandleIdx], SystemFile->FileName);
    if (Path == NULL) {
      Status = EFI_NOT_FOUND;
      break;
    }
    
    Status = OpenSimpleReadFile(TRUE, NULL, 0, &Path, &DeviceHandle, &ReadHandle);
    if (!EFI_ERROR(Status))
      break;
    
    FreePool(Path);
    Path = NULL;
  }
  
  if (!EFI_ERROR(Status)) {
    // if this was our kernel file and we wanted to just load it,
    //  we coud do
    //  LoadImage(ReadHandle);
    // However, this means it would have to be a PE/COFF file.
    //
    Status = OpenSimpleFileLen(ReadHandle, &RetSize);
    if (!EFI_ERROR(Status)) {
      Buffer = AllocatePool(RetSize);
      if (Buffer == NULL)
        return EFI_OUT_OF_RESOURCES;
      // read in the file
      Status = ReadSimpleReadFile(ReadHandle, 0, &RetSize, Buffer);
      if (!EFI_ERROR(Status)) {
        struct S_LDR_HDR *ldr_hdr = (struct S_LDR_HDR *) Buffer;
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // now that it is at 'Buffer', call the decompressor and let
        // it place the decompressed file to the actual location
        SystemFile->Size = RetSize;
        Status = Decompressor(Buffer, SystemFile);
        if (!EFI_ERROR(Status)) {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          // Now do a CRC check on the file.
          printf(L"...checking crc");
          if (calc_crc(SystemFile->Data, SystemFile->Size) == ldr_hdr->file_crc) {
            printf(L"(file crc passed)     ");
            SystemFile->Target = ldr_hdr->location;
            // if this is the kernel, we need to update our pointer
            if (ldr_hdr->flags & LDR_HDR_FLAGS_ISKERNEL) {
              // if the kernel_base is not zero, we already have found a
              //  file with this flag set.  So give error.
              if (kernel_base != 0) {
                printf(L"Already set 'kernel_base' to 0x%08X...\r\n...halting...\r\n", kernel_base);
                freeze();
              } else {
                kernel_base = ldr_hdr->location;
                //printf(L"\r\nSetting 'kernel_base' to 0x%08X", kernel_base);
                SystemFile->IsKernel = TRUE;
              }
            }
          } else
            printf(L"...Invalid file crc   ");
        } else {
          // was an error decompressing the file.
          printf(L"...Error decompressing file...");
          if (ldr_hdr->flags & LDR_HDR_FLAGS_HALT) {
            printf(L"...halting...\r\n");
            freeze();
          }
        }
      }
      // free the buffer used
      FreePool(Buffer);
    }
  }
  
  if (ReadHandle)
    CloseSimpleReadFile(ReadHandle);
  
  if (Path)
    FreePool(Path);
  
  FreePool(HandleBuffer);
  return Status;
}

EFI_STATUS do_decomp_flat(void *Location, const void *Src, const int *Size) {
  printf(L"...moving");
  memcpy(Location, Src, *Size);
  return EFI_SUCCESS;
}

EFI_STATUS do_decomp_bz2(void *Targ, const void *Src, const int *Size) {
  int RetSize = *Size, ret;
  EFI_STATUS Status;
  
  printf(L"...decompressing(bz2)");
  
  if ((ret = bz2_decompressor(Targ, Src, &RetSize)) != BZ_OK) {
    printf(L"...Error decompressing file.  error: %i", ret);
    *Size = 0;
    return EFI_COMPROMISED_DATA;
  }
  
  *Size = RetSize;
  return EFI_SUCCESS;
}

// we have the compressed file in Buffer with a Size count of bytes
//  we need to uncompress/move the file to the specified location
//  we return the newly uncompressed/moved size to the caller
EFI_STATUS Decompressor(void *Buffer, struct FILES *SystemFile) {
  struct S_LDR_HDR *ldr_hdr = (struct S_LDR_HDR *) Buffer;
  bit8u *p = (bit8u *) Buffer;
  bit32u RetSize = 0;
  bit8u crc = 0;
  EFI_STATUS Status;
  
  // see if the first 32 bytes of the file is a loader header
  // the first 32-bits will == 46595332h
  if (ldr_hdr->id != 0x46595332) {
    printf(L"...Did not find load header id dword\r\n");
    return EFI_INVALID_PARAMETER;
  }
  
  // now check the header's crc
  for (int i=0; i<sizeof(struct S_LDR_HDR); i++)
    crc += p[i];
  
  if (crc) {
    printf(L"...Invalid header crc\r\n");
    return EFI_CRC_ERROR;
  }
  
  // now check for the compression type
  RetSize = SystemFile->Size - sizeof(struct S_LDR_HDR);
  switch(ldr_hdr->comp_type) {
    case 0:
      SystemFile->Data = AllocatePool(RetSize);
      Status = do_decomp_flat(SystemFile->Data, (bit8u *) Buffer + sizeof(struct S_LDR_HDR), &RetSize);
      break;
    case 1:
      SystemFile->Data = AllocatePool(ldr_hdr->file_size + 4095);  // orignal file size plus finish out the page.
      Status = do_decomp_bz2(SystemFile->Data, (bit8u *) Buffer + sizeof(struct S_LDR_HDR), &RetSize);
      break;
    default:
      printf(L"...Unknown decompression type found\r\n");
      Status = EFI_COMPROMISED_DATA;
  }
  
  SystemFile->Size = !EFI_ERROR(Status) ? RetSize : 0;
  return Status;
}

bit32u calc_crc(void *Location, const int Size) {
  bit8u *p = (bit8u *) Location;
  bit8u octet;
  bit32u Result = 0;
  int cnt = Size;
  
  if (cnt > 4) {
    // initialize the progress proc
    init_progress(Size);
    
    Result  = *p++; Result <<= 8;
    Result |= *p++; Result <<= 8;
    Result |= *p++; Result <<= 8; 
    Result |= *p++;
    Result  = ~Result;
    
    cnt -= 4;
    
    for (int i=0; i<cnt; i++) {
      // display the progress
      put_progress(i, 0);
      
      octet = *p++;
      for (int j=0; j<8; j++) {
        if (Result & 0x80000000) {
          Result = (Result << 1) ^ 0x04C11DB7 ^ (octet >> 7);
        } else {
          Result = (Result << 1) ^ (octet >> 7);
        }
        octet <<= 1;
      }
    }
    
    // The complement of the remainder
    Result = ~Result;
  }
  
  return Result;
}
