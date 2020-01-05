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
 * Last update:  05 Jan 2020
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

//               24                16                 8                 0
//+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
//|Y|Y|Y|Y|Y|Y|Y|M| |M|M|M|D|D|D|D|D| |h|h|h|h|h|m|m|m| |m|m|m|s|s|s|s|s|
//+-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
// \___________/\________/\_________/ \________/\____________/\_________/
//      year      month       day         hour       minute     second

#ifndef _EXFAT_H
#define _EXFAT_H

#include "sys.h"

#define EXFAT_ATTR_ARCHIVE   0x20
#define EXFAT_ATTR_SUB_DIR   0x10
#define EXFAT_ATTR_SYSTEM    0x04
#define EXFAT_ATTR_HIDDEN    0x02
#define EXFAT_ATTR_READ_ONLY 0x01
#define EXFAT_ATTR_ALL       0x37

#define EXFAT_DIR_EOD        0x00  // end of directory
#define EXFAT_DIR_BITMAP     0x81
#define EXFAT_DIR_UCASE      0x82
#define EXFAT_DIR_LABEL      0x83
#define EXFAT_DIR_ENTRY      0x85
#define EXFAT_DIR_STRM_EXT   0xC0
#define EXFAT_DIR_NAME_EXT   0xC1
#define EXFAT_DIR_WINCE_ACC  0xE2
#define EXFAT_DIR_GUID       0xA0
#define EXFAT_DIR_TEXFAT     0xA1


#pragma pack(push, 1)

struct S_EXFAT_VBR {
  bit8u  jmp[3];
  char   oem_name[8];
  bit8u  reserved0[53];
  bit32u part_offset[2];
  bit32u total_sectors[2];
  bit32u first_fat;
  bit32u sect_per_fat;
  bit32u data_region_lba;
  bit32u data_region_size;   // in clusters
  bit32u root_dir_cluster;
  bit32u serial;
  bit16u fs_version;
  bit16u flags;
  bit8u  log_bytes_per_sect;
  bit8u  log_sects_per_clust;
  bit8u  num_fats;
  bit8u  drive_sel;
  bit8u  percent_heap;
  bit8u  reserved1[7];
  bit8u  boot_code[390];
  bit16u boot_sig;           // 0xAA55
};

#define EXFAT_MIN_SECONDARY_COUNT   2
#define EXFAT_MAX_SECONDARY_COUNT  18

#define EXFAT_NO_FAT_CHAIN_VALID   (1<<1)

struct S_EXFAT_ROOT {
  bit8u entry_type;
  union {
    struct {
      bit8u  sec_count;
      bit16u crc;
      bit16u attributes;
      bit16u resv1;
      bit32u created;
      bit32u last_mod;
      bit32u last_acc;
      bit8u  created_ms;
      bit8u  last_mod_ms;
      bit8u  created_tz_offset;
      bit8u  last_mod_tz_offset;
      bit8u  last_access_tz_offset;
      bit8u  resv2[7];
    } dir_entry;
    struct {
      bit8u  flags;
      bit8u  resv1;
      bit8u  name_len;
      bit16u name_hash;
      bit16u resv2;
      bit32u valid_data_len[2];
      bit32u resv3;
      bit32u first_cluster;
      bit32u data_len[2];
    } stream_ext;
    struct {
      bit8u  flags;
      bit16u name[15];
    } file_name_ext;
    struct {
      bit8u  len;
      bit16u name[11];
      bit8u  resv1[8];
    } label;
    struct {
      bit8u  flags;
      bit8u  resv1[18];
      bit32u cluster_strt;
      bit32u data_len[2];
    } bitmap;
    struct {
      bit8u  resv1[3];
      bit32u crc;
      bit8u  resv2[12];
      bit32u cluster_strt;
      bit32u data_len[2];
    } up_case_table;
    struct {
      bit8u  sec_count;  // always 0
      bit16u crc;
      bit16u flags;
      struct S_GUID guid;
      bit8u  resv2[10];
    } guid;
  } type;
};

#pragma pack(pop)

struct S_ExFAT_DATA {
  void *vbr;
  void *root_dir;
  void *fat_loc;
  int  sect_per_clust;
  int  bytes_per_sector;
};

bool exfat_load_data(struct S_ExFAT_DATA *data);
bit32u exfat_vrb_crc(const bit8u *buffer, const int len);



#endif   // _EXFAT_H
