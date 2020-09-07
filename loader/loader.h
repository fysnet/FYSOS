/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2020
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
 * Last update:  07 Sept 2020
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#ifndef _LOADER_H
#define _LOADER_H

#define TITLE_BUILD_VER " FYSOS v2.0 Loader (32/64-bit) (Build 3600)"

#include "disks.h"
#include "malloc.h"
#include "time.h"
#include "video.h"

#define MEMORY_MIN_REQUIRED  0x08000000  // 128 meg
#define DECOMP_BUFFER_SIZE   0x01000000  // 16 meg

#define FILES_IS_KERNEL  (1<<0)
#define FILES_IS_BSOD    (1<<1)
struct FILES {
  char   FileName[64];
  void  *Data;
  bit32u Size;
  bit32u Target;
  bit32u Flags;
};

struct BSOD_HEADER {
  bit32u start_address[2];       // start address (relative to 0x00010000)
  struct S_MODE_INFO mode_info;  // video information
  bit32u reserved[8];            // reserved for future use
};

// if one or more of these are defined, that respected fs code is
//  included with this loader.  If one or more lines are commented
//  out, that fs system is also commented out.
// usually, you only have one defined at a time, since you are building
//  the loader.sys file for a single file system.
// however, if you define more than one, or all of them, you only
//  have to build this loader once, then just copy it to each file-
//  system supported.
#define FS_LEAN     1
#define FS_EXT2     2
#define FS_SFS      3
#define FS_FAT12   12
#define FS_FAT16   16
#define FS_FAT32   32
#define FS_FYSFS   22
#define FS_EXFAT   11

#if !defined(FS_LEAN) && !defined(FS_EXT2) && !defined(FS_SFS) && !defined(FS_FAT12) && \
    !defined(FS_FAT16) && !defined(FS_FAT32) && !defined(FS_FYSFS) && !defined(FS_EXFAT)
  #error "Must define at least one file system."
#endif

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
  bit32u magic0;                // first magic number
  bit32u bios_type;             // 'BIOS' = legacy BIOS, 'UEFI' = UEFI booted
  bit32u uefi_image_handle;     // UEFI Image Handle
    bit32u uefi_image_handle_hi;
  bit32u uefi_system_table;     // UEFI System Table Pointer
    bit32u uefi_system_table_hi;
  bit32u uefi_rsdp_pointer;     // UEFI System Table Pointer
    bit32u uefi_rsdp_pointer_hi;
  struct S_BOOT_DATA boot_data; // booted data
  bit32u magic1;              // second magic number
  struct S_TIME time;         // current time passed to kernel
  struct S_MEMORY memory;     // memory blocks returned by INT 15h memory services
  bit32u magic2;              // third magic number
  bit16u vid_mode_cnt;        // count of video mode info blocks found
  bit16u cur_vid_index;       // index into mode_info[] of chosen/default/current mode
  struct S_MODE_INFO mode_info[VIDEO_MAX_MODES]; // video modes information for kernel
  bool   has_cpuid;           // set if we detect a 486+ with a CPUID instruction
  bit8u  a20_tech;            // number of technique loader.sys used to enable the a20 line
  bool   text_only;           // Use screen mode 3, text only
  bit8u  padding[1151];       // reserved
  bit32u magic3;              // fourth magic number
};

#pragma pack(pop)

extern struct S_SYS_BLOCK sys_block;

void finish_32bit();
void finish_64bit();
void LoadFile(struct FILES *);

#endif // _LOADER_H
