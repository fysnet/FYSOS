
#ifndef _DISKS_H
#define _DISKS_H

struct S_DISK_DATA {
  int  sec_per_track;
  int  num_heads;
};

#pragma pack(push, 1)

struct S_READ_PACKET {
  bit8u  size;    // size of packet (10h or 18h)
  bit8u  resv;    // reserved (0)
  bit16u cnt;     // number of blocks to transfer (max 007Fh for Phoenix EDD)
  bit32u buffer;  // -> transfer buffer (seg:off)
  bit32u lba[2];  // starting absolute block number
//  bit32u flat[2]; // (EDD-3.0, optional) 64-bit flat address of transfer buffer;
//                  //  used if DWORD at 04h is FFFFh:FFFFh
};

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

#pragma pack(pop)

extern bool large_disk;
bool large_disk_support(const int, bool *);
int read_sectors(bit32u lba, int cnt, void *buffer);

bit32u update_int1e(struct S_FLOPPY1E *);

void get_drv_parameters(struct S_DRV_PARAMS *);

void floppy_off(void);

#endif   // _DISKS_H
