/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2020
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 */

// set it to 1 (align on byte)
#pragma pack (push, 1)

#define VERSION_STR  "02.00.00"

char strtstr[] = "\nMake Bootable CDROM Image  v" VERSION_STR "  Forever Young Software 1984-2019\n";

#define DESC_TYPE_BOOT    0
#define DESC_TYPE_PVD     1
#define DESC_TYPE_SVD     2
#define DESC_TYPE_PART    3
#define DESC_TYPE_TERM  255

// Since CDROM's are "universal" to all platforms, if a value stored
//  in one of the following structures is more than a byte, the value
//  is stored twice.  The first being little_endian, the second, big_endian.
struct VOL_DATE {
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char min[2];
    char sec[2];
    char sec100[2];
   bit8s gmt_off;
};

struct DIR_DATE {
   bit8u since_1900;
   bit8u month;
   bit8u day;
   bit8u hour;
   bit8u min;
   bit8u sec;
   bit8s gmt_off;
};

struct PATH_TAB {
   bit8u len_di;
   bit8u ext_attrib;
  bit32u loc;
  bit16u parent;
   bit8u ident[16];
};

struct ROOT {
   bit8u len;
   bit8u e_attrib;
  bit32u extent_loc;
  bit32u extent_loc_b;
  bit32u data_len;
  bit32u data_len_b;
  struct DIR_DATE date;
   bit8u flags;
   bit8u unit_size;
   bit8u gap_size;
  bit16u sequ_num;
  bit16u sequ_num_b;
   bit8u fi_len;
   bit8u ident;
};

struct PVD {
   bit8u id;
    char ident[5];
   bit8u ver;
   bit8u flags;        // PCD = reserved, SVD = flags
    char sys_ident[32];
    char vol_ident[32];
   bit8u resv1[8];
  bit32u num_lbas;
  bit32u num_lbas_b;
   bit8u escape_sequ[32];
  bit16u set_size;
  bit16u set_size_b;
  bit16u sequ_num;
  bit16u sequ_num_b;
  bit16u lba_size;
  bit16u lba_size_b;
  bit32u path_table_size;
  bit32u path_table_size_b;
  bit32u PathL_loc;
  bit32u PathLO_loc;
  bit32u PathM_loc;
  bit32u PathMO_loc;
  struct ROOT root;  
    char set_ident[128];
    char pub_ident[128];
    char prep_ident[128];
    char app_ident[128];
    char copy_ident[37];
    char abs_ident[37];
    char bib_ident[37];
  struct VOL_DATE vol_date;
  struct VOL_DATE mod_date;
  struct VOL_DATE exp_date;
  struct VOL_DATE val_date;
   bit8u struct_ver;
   bit8u resv3;
   bit8u app_use[512];
   bit8u resv4[653];
};

struct BRVD {
   bit8u id;
    char ident[5];
   bit8u ver;
    char bsident[32];
   bit8u resv0[32];
  bit32u boot_cat;
   bit8u resv1[1973];
};

struct TERM {
   bit8u id;
    char ident[5];
   bit8u ver;
   bit8u resv1[2041];
};

struct CAT {
  struct VAL_ENTRY {
     bit8u id;
     bit8u platform;
    bit16u resv0;
      char ident[24];
    bit16u crc;
     bit8u key55;
     bit8u keyAA;
  } val_entry;
  struct INIT_ENTRY {
     bit8u bootable;
     bit8u media;
    bit16u load_seg;
     bit8u sys_type;
     bit8u resv0;
    bit16u load_cnt;
    bit32u load_rba;
     bit8u resv1[20];
  } init_entry;
};

struct SECT_HEADER {
   bit8u id;
   bit8u platform;
  bit16u num;
   bit8u ident[28];
};

struct SECT_ENTRY {
   bit8u bootable;
   bit8u media;
  bit16u load_seg;
   bit8u sys_type;
   bit8u resv0;
  bit16u load_cnt;
  bit32u load_rba;
   bit8u criteria;
   bit8u vendor[19];
};

struct SECT_ENTRY_X {
   bit8u id;
   bit8u final;
   bit8u vendor[30];
};

void fill_date(struct DIR_DATE *);
void fill_e_date(struct VOL_DATE *);
void ascii2utf16(void *, char *, int);
void fill_utf16(void *, bit16u, int);
void parse_command(int, char *[], char *);

#pragma pack (pop)
