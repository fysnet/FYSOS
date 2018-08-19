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

#ifndef BOOT_H
#define BOOT_H

struct FILES {
  wchar_t FileName[16];
  void  *Data;
  bit32u Size;
  bit32u Target;
  bool   IsKernel;
};


#pragma pack(push, 1)

struct GDT {
	bit16u limitlow;             // low word of limit
	bit16u baselow;              // low word of base
	bit8u  basemid;              // mid byte of base
	bit8u  flags0;               // 
	bit8u  flags1;               // 
	bit8u  basehigh;             // high byte of base
};

//                                  ----index---- L PR  ; L = set = LDT, clear = GDT
#define FLATZERO    0x00000000  //  0000000000000_0_00b ; PR = protection level (0-3)
#define FLATCODE    0x00000008  //  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
#define FLATDATA    0x00000010  //  0000000000010_0_00b ;  0 = ring 0 (highest priv.)
#define LOADCODE16  0x00000008  //  0000000000001_0_00b ;  0 = ring 0 (highest priv.)
#define FLATDATA16  0x00000010  //  0000000000010_0_00b ;  0 = ring 0 (highest priv.)
#define LOADSTACK16 0x00000018  //  0000000000011_0_00b ;  0 = ring 0 (highest priv.)
#define LOADDATA16  0x00000020  //  0000000000100_0_00b ;  0 = ring 0 (highest priv.)

// locations to set our stack to
#define STACK_BASE   0x01000000

struct S_MEMORY {
  bit16u   word;        // (0 = not used, 1 = E820h, 2 = 0E801h, 3 = 88h, 4 = cmos, 16 = uEFI)
  bit32u   size[2];     // size of memory in bytes
  bit16u   blocks;      // count of bases returned
   struct S_MEMORY_BLKS {
     bit32u base[2];
     bit32u size[2];
     bit32u type;
     bit32u attrib[2];
   } block[48];
};

EFI_STATUS get_memory(struct S_MEMORY *memory, bit32u *MemMapKey);


#define SIZEOF_S_BOOT_DATA 48

// first two items must remain at top and in that order
struct S_BOOT_DATA {
  bit32u signature;    // signature used for finding booted from partition
  bit32u base_lba[2];  // base lba of partition
  bit32u loader_base;  // base address of loader.sys
  bit8u  file_system;  // filesystem number
  bit8u  drive;        // BIOS drive number
  bit8u  reserved[30]; // padding/reserved
};

struct S_BIOS_PCI {
  bit32u sig;
  bit8u  flags;
  bit8u  major;
  bit8u  minor;
  bit8u  last;
};
void get_pci_info(struct S_BIOS_PCI *);

#define SIZEOF_S_FLOPPY1E 11
struct S_FLOPPY1E {
  bit8u  spec0;
  bit8u  spec1;
  bit8u  offdelay;
  bit8u  sect_size;
  bit8u  spt;
  bit8u  gaplen;
  bit8u  datalen;
  bit8u  gaplen_f;
  bit8u  filler;
  bit8u  settle;
  bit8u  start;
};

struct S_TIME {
  bit16u year;
  bit8u  month;
  bit8u  day;
  bit8u  hour;
  bit8u  min;
  bit8u  sec;
  bit8u  jiffy;
  bit16u msec;
  bit8u  d_savings;
  bit8u  weekday;
  bit16u yearday;
};
void get_bios_time(struct S_TIME *);

struct S_APM {
  bit8u  present;
  bit8u  initialized;
  bit8u  maj_ver;
  bit8u  min_ver;
  bit16u flags;
  bit8u  batteries;
  bit16u cap_flags;
  bit16u error_code;
  bit8u  resv0[5];
  bit16u cs_segment32;
  bit32u entry_point;
  bit16u cs_segment;
  bit32u cs16_gdt_idx;
  bit32u cs32_gdt_idx;
  bit32u ds_gdt_idx;
  bit16u ds_segment;
  bit16u cs_length32;
  bit16u cs_length16;
  bit16u ds_length;
};
//void apm_bios_check(struct S_APM *);

#define VESA_INFO_SIZE  256
#define VESA_MODE_SIZE  64


struct S_BIOS_DRV_PARAMS {
  bit16u ret_size;          // size of buffer returned (1Ah = v1.x, 1Eh = v2.x, 42h = v3.x)
  bit16u info_flags;        // see RBIL table 00274
  bit32u cylinders;         // number of physical cylinders
  bit32u heads;             // number of physical heads
  bit32u spt;               // number of physical sectors per track
  bit32u tot_sectors[2];    // total sectors on drive
  bit16u bytes_per_sector;  // bytes per sector
  bit32u EDD_config_ptr;    // see RBIL table 00278
  bit16u v3_sig;            // will be BEDD if this info is valid
  bit8u  v3_len;            // length of this data (includes sig and length) (24h)
  bit8u  resv0[3];
  bit8u  host_name[4];      // ASCIIZ host ("ISA" or "PCI")
  bit8u  interface_name[8]; // ASCIIZ interface type ("ATA", "ATAPI", "SCSI", "USB", "1394", "FIBRE"
  bit8u  interface_path[8]; // see RBIL table 00275
  bit8u  device_path[8];    // see RBIL table 00276
  bit8u  resv1;
  bit8u  v3crc;             // bytes 1Eh-40h = zero  (two's complement)
};

struct S_DRV_PARAMS {
  bit8u  drv_num;           // 80h, 81h, etc. (0 = not used)
  bit8u  extended_info;     // was service 48h used?
  // start of BIOS data
  struct S_BIOS_DRV_PARAMS bios_params;
  // our reserved to pad to 80 bytes
  bit8u  padding[12];
  bit8u  dpte[16];          // parameters from address EDD_config_ptr above
};

#define LDR_HDR_FLAGS_HALT     (1<<0)
#define LDR_HDR_FLAGS_ISKERNEL (1<<1)

struct S_LDR_HDR {
  bit32u id;             // 0x46595332 = 'FYS2'
  bit32u location;       // location to store the file
  bit32u flags;          // bit 0 = halt on error, bit 1 = is kernel file
  bit32u file_crc;       // uncompressed/moved files crc
  bit8u  comp_type;      // compression type (0=none, 1 = bz2)
  bit8u  hdr_crc;        // byte sum check sum of hdr
  bit32u file_size;      // size of uncompressed file
  bit8u  resv[10];       // reserved
};

#pragma pack(pop)


EFI_STATUS LoadFile(struct FILES *SystemFile);
EFI_STATUS Decompressor(void *Buffer, struct FILES *SystemFile);
bit32u calc_crc(void *location, const int size);
void freeze(void);

void CreateIDT(bit32u address, int count);

#endif // BOOT_H
