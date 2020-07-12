//////////////////////////////////////////////////////////////////////////
//  diskinfo.h
//////////////////////////////////////////////////////////////////////////
//
//

// set it to 1 (align on byte)
#pragma pack (1)

struct S_INT_PATH_ISA {
  bit16u base_addr;
  bit8u  resv[6];
};

struct S_INT_PATH_PCI {
  bit8u  bus;
  bit8u  dev;
  bit8u  func;
  bit8u  resv[5];
};

struct S_DEV_PATH_ATA {
  bit8u  flag;
  bit8u  resv[7];
};

struct S_DEV_PATH_ATAPI {
  bit8u  flag;
  bit8u  lun;
  bit8u  resv[6];
};

struct S_DEV_PATH_SCSI {
  bit8u  lun;
  bit8u  resv[7];
};

struct S_DEV_PATH_USB {
  bit8u  unknown;
  bit8u  resv[7];
};

struct S_DEV_PATH_IEEE {
  bit64u guid;
};

struct S_DEV_PATH_FIBRE {
  bit64u wwn;
};

struct S_DRV_PARAMS {
  bit16u size;
  bit16u flags;
  bit32u cylinders;
  bit32u heads;
  bit32u sects_per_trk;
  bit64u tot_sectors;
  bit16u sector_size;
  // v2.0
  bit32u config_params_ptr;
  // v3.0
  bit16u bedd_sig;
  bit8u  bedd_len;
  bit8u  resv0[3];
  bit8u  host_bus_name[4];
  bit8u  interface_type[8];
  union {
    struct S_INT_PATH_ISA isa;
    struct S_INT_PATH_PCI pci;
  } int_path;
  union {
    struct S_DEV_PATH_ATA   ata;
    struct S_DEV_PATH_ATAPI atapi;
    struct S_DEV_PATH_SCSI  scsi;
    struct S_DEV_PATH_USB   usb;
    struct S_DEV_PATH_IEEE  ieee;
    struct S_DEV_PATH_FIBRE fibre;
  } dev_path;
  bit8u  resv1;
  bit8u  crc;
};

struct S_FIXED_DISK_PARAMS {
  bit16u port_base;
  bit16u cntrl_base;
  bit8u  flags;
  bit8u  propty_info;
  bit8u  irq;
  bit8u  multi_sect_cnt;
  bit8u  dma_cntrl;
  bit8u  prog_io_cntrl;
  bit16u options;
  bit8u  resv[2];
  bit8u  ext_revision;
  bit8u  crc;
};

struct S_1E_DISK_PARAMS {
  bit8u specify0;
  bit8u specify1;
  bit8u delay;
  bit8u bytes_sector;
  bit8u sects_track;
  bit8u gap_len;
  bit8u data_len;
  bit8u gap_len_f;
  bit8u filler;
  bit8u head_settle;
  bit8u motor_start;
};

struct S_IDENTIFY {
  bit16u get_conf;
  bit16u cylinders;
  bit16u resv0;
  bit16u heads;
  bit16u un_bytes_trck;
  bit16u un_bytes_sect;
  bit16u sects_track;
  bit16u vendor_unique0[3];
  char   serial_num[20];
  bit16u buffer_type;
  bit16u buff_size;            // in 512 byte increments
  bit16u num_eec_bytes;        //
  char   firmware_rev[8];
  char   model_num[40];
  bit16u vendor_unique1;
  bit16u double_words;
  bit16u caps;                 // high byte
                               //  b7:b6 = 00 = no ATA or ATAPI
                               //          01 = ATA (at least version 3)
                               //          10 = ATAPI
                               //          11 = reserved
                               //  b5    =  0 = CHS support only
                               //           1 = LBA support
                               //  b4    =  1 = doubleword read/write allowed
                               //  b3:b1 =000 = Not optical
                               //         001 = CD-ROM
                               //         010 = CD-R/CD-RW
                               //         011 = DVD
                               //         1xx = reserved
                               //  b0    =  0 = reserved
                               // low byte
                               //  b7    =  1 = DMA supported
                               //  b6:b4 =xxx = dma used (0 - 5)
                               //  b3:b0 =  0 = reserved
  bit16u caps2;                    
  bit16u PIO_timing;
  bit16u DMA_timing;
  bit16u rest_valid;
  bit16u num_cur_cylinders;
  bit16u num_cur_heads;
  bit16u num_cur_sect_track;
  bit32u cur_capacity;
  bit16u resv2;
  bit32u lba_capacity;      // 32-bit capacity (ATA1,2,3,4,5,6,7,8)
  bit16u resv3;
  bit16u multiword_dma;     // 
  bit16u pio_modes;
  bit16u min_multi_dma;
  bit16u manuf_min_multi_dma;
  bit16u min_pio_cycle0;
  bit16u min_pio_cycle1;
  bit16u resv4[2];
  bit16u resv5[4];
  bit16u queue_depth;
  bit16u resv6[4];
  bit16u major_ver;        // bit 2 = ATA2, bit 3= ATA3, bit 4=ATA/ATAPI4 through 14
  bit16u minor_ver;
  bit16u command_set1;
  bit16u command_set2;
  bit16u cfsse;
  bit16u cfs_enable_1;
  bit16u cfs_enable_2;
  bit16u csf_default;
  bit16u dma_ultra;
  bit16u trseuc;
  bit16u trsEuc;
  bit16u cur_apm_values;
  bit16u mprc;
  bit16u hw_config;
  bit16u acoustic;
  bit16u msrqs;
  bit16u sxfert;
  bit16u sal;
  bit32u spg;
  bit64u lba_capacity2;   // 48-bit total number of sectors ( = last LBA + 1 )
  bit16u resv7[22];
  bit16u last_lun;
  bit16u word127;
  bit16u dlf;
  bit16u csfo;
  bit16u resv8[30];
  bit16u cfa_power;
  bit16u resv9[15];
  char   cur_media_sn[60];
  bit16u resvA[49];
  bit16u integrity;
};


int int13_extentions(__dpmi_regs *, const int, const bit32u);
int standard_params(__dpmi_regs *, const int);
bit8u calc_crc(void *, int);


