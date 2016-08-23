/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: loader.h                                                           *
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
*   Loader #defines for the FYS OS version 2.0 operating system.           *
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
*  If we modify this file, we need to modify the lean.inc file to match    *
*                                                                          *
***************************************************************************/

#ifndef _LOADER_H
#define _LOADER_H

// this is simply to let VC2008 ignore the farX identifier
//   since we use VC2008's IDE to work with this code
#ifdef _MSC_VER
  #define farE
  #define farF
  #define farG
#endif

//void debugit(int n);

char *strchr(char *, int);
unsigned strlen(char *);

typedef char *va_list;
int printf(const char *, ...);
int putchar(const int);
int puts(const char *);

int putchar16(const int);
int puts16(const char *);

void *memset(void *, const bit8u, const int);
void *memcpy(void *, const void *, const int);
void *memcpy_ds(void *, const void *, const int, const int);
void *memmove(void *, const bit16u, void *, const unsigned int);
int stricmp(const char *, const char *);

void freeze(void);

int chk_486();
bool is_64bit_cpu();
bit8u set_a20_line();
bool test_a20();
extern bit8u a20_tech;

bool wait_kbd_status(const int);

 bit8u inpb(int);
bit16u inpw(int);
bit32u inpd(int);
  void outpb(int, bit8u);
  void outpw(int, bit16u);
  void outpd(int, bit32u);

void add64(void *, void *);

void init_progress(const bit32u limit);
void put_progress(const bit32u lo, const bit32u hi);

// CRC32
#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
bit32u crc32(void *, bit32u);
void crc32_partial(bit32u *, void *, bit32u);
bit32u crc32_reflect(bit32u, char);

struct REGS {
  bit32u eax;
  bit32u ebx;
  bit32u ecx;
  bit32u edx;
  bit32u esi;
  bit32u edi;
  bit32u ebp;
  bit32u eflags;
};

#pragma pack(push, 1)

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

// BIOS interrupt 15h/E820 return buffer
struct S_BIOS_MEME820 {
  bit32u base[2];
  bit32u size[2];
  bit32u type;
};

struct S_MEMORY {
  bit16u   word;        // (0 = not used, 1 = E820h, 2 = 0E801h, 3 = 88h, 4 = cmos)
  bit32u   size[2];     // size of memory in bytes
  bit16u   blocks;      // count of bases returned (usually 2)
   struct S_MEMORY_BLKS {
     bit32u base[2];
     bit32u size[2];
     bit32u type;
     bit32u attrib;
   } block[48];
};

struct S_BIOS_PCI {
  bit32u sig;
  bit8u  flags;
  bit8u  major;
  bit8u  minor;
  bit8u  last;
};
extern struct S_BIOS_PCI bios_pci;
void get_pci_info(struct S_BIOS_PCI *);

// Prototype it, but don't allocate it here
#define LOCAL_BUFF_SECT_SIZE  9
extern bit8u local_buffer[];
extern struct S_MEMORY memory;
bool get_memory(struct S_MEMORY *);

bool intx(const int, struct REGS *);

#define VESA_INFO_SIZE  256
#define VESA_MODE_SIZE  64
extern bit8u vesa_video[];
extern bit16u vesa_modes[];
extern bit8u  vesa_oem_name[];
bool get_vesa_info(bit8u *);

extern bool large_disk;
bool large_disk_support(const int drv);
int read_sectors(bit32u lba, const int cnt, const void *buffer);

struct S_READ_PACKET {
  bit8u  size;    // size of packet (10h or 18h)
  bit8u  resv;    // reserved (0)
  bit16u cnt;     // number of blocks to transfer (max 007Fh for Phoenix EDD)
  bit32u buffer;  // -> transfer buffer (seg:off)
  bit32u lba[2];  // starting absolute block number
//  bit32u flat[2]; // (EDD-3.0, optional) 64-bit flat address of transfer buffer;
//                  //  used if DWORD at 04h is FFFFh:FFFFh
};

extern int main_i;
extern int ret_size;
extern struct S_LDR_HDR farF *ldr_hdr;
extern struct REGS main_regs;
extern char system_files[][11];

extern bit32u decomp_buf_loc;   // buffer where the compressed file is stored
extern bit32u decomp_mem_loc;   // buffer ptr to allocated buffers during decompression
extern bit32u kernel_base;      // base of our kernel, loaded from kernel.sys' header

int decompressor(const bit32u, const bit32u);
bit32u calc_crc(const bit32u, const int);

bool do_decomp_flat(void *, const bit32u, const int *);
bool do_decomp_bz2(void *, const bit32u, const int *);

int fs_fat(struct S_BOOT_DATA *, const char *, const bit32u);
int fs_lean(struct S_BOOT_DATA *, const char *, const bit32u);
int fs_ext2(struct S_BOOT_DATA *, const char *, const bit32u);
int fs_fysfs(struct S_BOOT_DATA *, const char *, const bit32u);
int fs_exfat(struct S_BOOT_DATA *, const char *, const bit32u);

void update_int1e(void);

extern bit16u bios_equip;
extern bit8u  kbd_bits;
void get_bios_equ_list(bit16u *, bit8u *);


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

extern struct S_TIME time;
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

extern struct S_APM apm;
void apm_bios_check(struct S_APM *);

void floppy_off(void);

#define LDR_HDR_FLAGS_HALT     (1<<0)
#define LDR_HDR_FLAGS_ISKERNEL (1<<1)

struct S_LDR_HDR {
  bit32u id;             // 0x46595332 = 'FYS2'
  bit32u location;       // location to store the file
  bit32u flags;          // bit 0 = halt on error, bit 1 = is kernel file
  bit32u file_crc;       // uncompressed/moved files crc
  bit8u  comp_type;      // compression type (0=none, 1 = bz2)
  bit8u  hdr_crc;        // byte sum check sum of hdr
  bit8u  resv[14];       // reserved
};

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

extern struct S_DRV_PARAMS drive_params[];
void get_drv_parameters(struct S_DRV_PARAMS *params);

#pragma pack(pop)

#endif // _LOADER_H
