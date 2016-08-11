/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: boot.h                                                             *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*   EQUates for all boot sectors and loader.asm                            *
*                                                                          *
* BUILT WITH:   NewBasic Compiler and Assembler                            *
*                 http://www.fysnet/newbasic.htm                           *
*               NBC   ver 00.20.25                                         *
*          Command line: nbc loader<enter>                                 *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm loader loader.sys -d<enter>                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
*  If we modify this file, we need to modify the boot.inc file to match    *
*                                                                          *
***************************************************************************/

#ifndef _BOOT_H
#define _BOOT_H

// FS types
#define FS_LEAN     1
#define FS_EXT2     2
#define FS_FAT12   12
#define FS_FAT16   16
#define FS_FAT32   32
#define FS_FYSFS   22
#define FS_EXFAT   11

// loadseg  128k  This is the max size of our loader file
#define LOADSEG       0x03000   // start of loader load area   (0x30000 = 192k)

//                                  ----index---- L PR  ; L = set = LDT, clear = GDT
#define FLATZERO    0x00000000  //  0000000000000_0_00b ; PR = protection level (0-3)
#define FLATCODE    0x00000008  //  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
#define FLATDATA    0x00000010  //  0000000000010_0_00b ;  0 = ring 0 (highest priv.)
#define LOADCODE16  0x00000008  //  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
#define FLATDATA16  0x00000010  //  0000000000010_0_00b ;  0 = ring 0 (highest priv.)
#define LOADSTACK16 0x00000018  //  0000000000011_0_00b ;  0 = ring 0 (highest priv.)
#define LOADDATA16  0x00000020  //  0000000000100_0_00b ;  0 = ring 0 (highest priv.)

#pragma pack(push, 1)

#define S_BOOT_DATA_ROOT_LOC   29   // offset from top of S_BOOT_DATA to ->root_loc
#define S_BOOT_DATA_OTHER_LOC  33   // offset from top of S_BOOT_DATA to ->other_loc

#define SIZEOF_S_BOOT_DATA 48
struct S_BOOT_DATA {
  bit32u SecPerFat;    //  FAT: Sectors per FAT
  bit8u  FATs;         //  FAT: Number of FATs
  bit16u SecPerClust;  //  FAT: Sectors per Cluster
  bit16u SecRes;       //  FAT: Sectors reserved for Boot Record
  bit16u SecPerTrack;  //  FAT: Sectors per Track
  bit16u Heads;        //  FAT: Number of Heads
  bit32u root_entries; //  FAT: Root entries
  bit32u base_lba[2];  //  ALL: base lba of partition
  bit8u  file_system;  //  ALL: filesystem number
  bit8u  drive;        //  ALL: BIOS drive number
  bit16u sect_size;    //  ALL: Sector size: 512, 1024, etc.
  bit32u root_loc;     //  ALL: Far pointer (seg:off) to current root location (loaded by the boot code)
  bit32u other_loc;    // SOME: Far pointer (seg:off) to current super/fat/etc location (loaded by the boot code)
  bit16u misc0;        // SOME: Misc word used buy some file systems
  bit16u misc1;        // SOME: Misc word used buy some file systems
  bit16u misc2;        // SOME: Misc word used buy some file systems
  bit8u  reserved[5];  // padding/reserved
};
extern struct S_BOOT_DATA boot_data;


#pragma pack(pop)

#endif  // _BOOT_H
