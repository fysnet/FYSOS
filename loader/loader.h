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
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */

#ifndef _LOADER_H
#define _LOADER_H

#include "apm.h"
#include "disks.h"
#include "malloc.h"
#include "pci.h"
#include "time.h"
#include "video.h"

// Set this define to allow for older machines with:
//  - no CPUID or RDTSC instruction
//  - less memory (16 Meg required)
// Still requires a 80x386 (32-bit) machine...
#define  ALLOW_SMALL_MACHINE  1  // 0 = error on 486 or less (486+ w/ CPUID *and* RDTSC required)
                                 // 1 = allow 486 without CPUID and/or RDTSC
#if ALLOW_SMALL_MACHINE
  #define MEMORY_MIN_REQUIRED  0x01000000  // 16 meg
  #define DECOMP_BUFFER_SIZE   0x00400000  //  4 meg
#else
  #define MEMORY_MIN_REQUIRED  0x08000000  // 128 meg
  #define DECOMP_BUFFER_SIZE   0x01000000  // 16 meg
#endif

// if one or more of these are defined, that respected fs code is
//  included with this loader.  If one or more lines are commented
//  out, that fs system is also commented out.
//#define FS_LEAN     1
//#define FS_EXT2     2
//#define FS_SFS      3
#define FS_FAT12   12
#define FS_FAT16   16
#define FS_FAT32   32
//#define FS_FYSFS   22
//#define FS_EXFAT   11

#ifdef FS_LEAN
  bit32u fs_leanfs(const char *, void *);
#endif
#ifdef FS_EXT2
  bit32u fs_ext2(const char *, void *);
#endif
#ifdef FS_SFS
  bit32u fs_sfs(const char *, void *);
#endif
#if defined(FS_FAT12) || defined(FS_FAT16) || defined(FS_FAT32)
  bit32u fs_fat(const char *, void *);
#endif
#ifdef FS_FYSFS
  bit32u fs_fysfs(const char *, void *);
#endif
#ifdef FS_EXFAT
  bit32u fs_exfat(const char *, void *);
#endif

#pragma pack(push, 1)

// first two items must remain at top and in that order
struct S_BOOT_DATA {
  bit32u signature;    // signature used for finding booted from partition
  bit32u base_lba[2];  // base lba of partition
  bit32u loader_base;  // base address of loader.sys
  bit8u  file_system;  // filesystem number
  bit8u  drive;        // BIOS drive number
  bit8u  reserved[30]; // padding/reserved
};

#define SYS_B_MAGIC0  0x464F5245  // 'FORE'
#define SYS_B_MAGIC1  0x56455259  // 'VERY'
#define SYS_B_MAGIC2  0x4F554E47  // 'OUNG'
#define SYS_B_MAGIC3  0x534F4654  // 'SOFT'

struct S_SYS_BLOCK {
  bit32u magic0;  // first magic number                                                //    4
  bit16u gdtoff;  // = ((256*8)-1);  // Address of our GDT
  bit32u gdtoffa; // = 0x00110000;  // KERN_GDT in memory.h
  
  bit16u idtoff;  // = ((256*8)-1);  // 256 = number of interrupts we allow
  bit32u idtoffa; // = 0x00110800;  // KERN_IDT in memory.h
  
  bit32u bios_type;             // 'BIOS' = legacy BIOS, 'UEFI' = UEFI booted          //    4
  bit32u uefi_image_handle;     // UEFI Image Handle                                   //    4
  bit32u uefi_system_table;     // UEFI System Table Pointer                           //    4
  struct S_BOOT_DATA boot_data; // booted data                                         //   48
  bit8u  resv0[32];             // reserved                                            //   32
  bit32u magic1;  // second magic number                                               //    4
  struct S_BIOS_PCI bios_pci;   // PCI information from the BIOS                       //    8
  bit32u org_int1e;             // original INT1Eh address                             //    4
  struct S_FLOPPY1E floppy_1e;  // floppies status                                     //   11
  struct S_TIME time;         // current time passed to kernel                         //   14
  struct S_APM apm;           // Advanced Power Management                             //   44
  bool   has_cpuid;           // set if we detect a 486+ with a CPUID instruction      //    1
  bool   has_rdtsc;           // set if we detect a 486+ with a RDTSC instruction      //    1
  bit8u  resv1;               // dword alignment                                       //    1
  bit16u bios_equip;          // bios equipment list at INT 11h (or 0040:0010h)        //    2
  bit8u  kbd_bits;            // bits at 0x00417                                       //    1
  bit32u magic2;  // third magic number                                                //    4
  struct S_MEMORY memory;     // memory blocks returned by INT 15h memory services     // 1356
  bit8u  a20_tech;            // the technique number used to enable the a20 line      //    1
  bool   text_only;           // Use screen mode 3, text only                          //    1
  bit16u vid_mode_cnt;        // count of video mode info blocks found                 //    2
  bit16u cur_vid_index;       // index into mode_info[] of chosen/default/current mode //    2
  struct S_MODE_INFO mode_info[VIDEO_MAX_MODES]; // video modes information for kernel //   VIDEO_MAX_MODES * 24
  bit8u  resv2[28];           // reserved                                              //   28
  struct S_DRV_PARAMS drive_params[10];  // up to 10 hard drive parameter tables       //  960
  bit8u  padding[1795];       // padding to xxxx bytes                                 // 
  bit32u magic3;  // fourth magic number                                               //    4
};

#pragma pack(pop)

extern struct S_SYS_BLOCK sys_block;

void finish();

#endif // _LOADER_H
