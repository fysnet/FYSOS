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
 * Last update:  08 Nov 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */
#ifndef _FAT_H
#define _FAT_H

#include "loader.h"

#define FAT_ATTR_LONG_FILE 0x0F
#define FAT_ATTR_ARCHIVE   0x20
#define FAT_ATTR_SUB_DIR   0x10
#define FAT_ATTR_VOLUME    0x08
#define FAT_ATTR_SYSTEM    0x04
#define FAT_ATTR_HIDDEN    0x02
#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_ALL       0x3F

#pragma pack(push, 1)

struct S_FAT1216_BPB {
	bit8u  jmp[3];
	char   oem_name[8];
	bit16u bytes_per_sect;
	bit8u  sect_per_clust;
	bit16u sect_reserved;
	bit8u  fats;
	bit16u root_entrys;
	bit16u sectors;
	bit8u  descriptor;
	bit16u sect_per_fat;
	bit16u sect_per_trk;
	bit16u heads;
	bit32u hidden_sects;
	bit32u sect_extnd;
	bit8u  drive_num;  // not FAT specific
	bit8u  resv;
	bit8u  sig;
	bit32u serial;
	char   label[11];
	char   sys_type[8];
};

struct S_FAT32_BPB {
	bit8u  jmp[3];
	char   oem_name[8];
	bit16u bytes_per_sect;
	bit8u  sect_per_clust;
	bit16u sect_reserved;
	bit8u  fats;
	bit16u root_entrys;
	bit16u sectors;
	bit8u  descriptor;
	bit16u sect_per_fat;
	bit16u sect_per_trk;
	bit16u heads;
	bit32u hidden_sects;
	bit32u sect_extnd;
	bit32u sect_per_fat32;
	bit16u ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
	bit16u fs_version;
	bit32u root_base_cluster;
	bit16u fs_info_sec;
	bit16u backup_boot_sec;
	bit8u  reserved[12];
	bit8u  drive_num;       // not FAT specific
	bit8u  resv;
	bit8u  sig;
	bit32u serial;
	char   label[11];
	char   sys_type[8];
};

struct S_FAT_ROOT {
  bit8u  name[8];    // name
  bit8u  ext[3];     // ext
  bit8u  attrb;      // attribute
  union U_TYPE {
    bit8u  resv[10];   // reserved in fat12/16
    struct S_FAT32 {
      bit8u  nt_resv;    // reserved for WinNT
      bit8u  crt_time_tenth; // millisecond stamp at creation time
      bit16u crt_time;   // time file was created
      bit16u crt_date;   // date file was created
      bit16u crt_last;   // date file was last accessed
      bit16u strtclsthi; // hi word of FAT32 starting cluster
    } fat32;
  } type;
  bit16u time;       // time
  bit16u date;       // date
  bit16u strtclst;   // starting cluster number
  bit32u filesize;   // file size in bytes
};

#pragma pack(pop)

struct S_FAT_DATA {
  void *root_dir;
  void *fat_loc;
  int  root_entries;
  int  sec_per_fat;
  int  sect_per_clust;
  int  sec_resv;
  int  num_fats;
};


bool fat_load_data(struct S_FAT_DATA *);
struct S_FAT_ROOT *fat_search(struct S_FAT_ROOT *root, int entry_count, char *filename, bit8u attr);
bit32u fs_fat_read_file(struct S_FAT_ROOT *root, void *target, struct S_FAT_DATA *fat_data);

void convert_fat83(const struct S_FAT_ROOT *, char *);
bit32u fat_get_next_cluster(void *, bit32u);

#endif   // _FAT_H
