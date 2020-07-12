
#ifndef FYSOS_HDC
#define FYSOS_HDC

#pragma pack(push, 1)

#define CNTRLR_COUNT   2  // we check 2 controller addresses

//////////////////////////////////////////////////////////////////////////
// ATA constants
#define  ATA_DATA             0x000    // data register
#define  ATA_ERROR            0x001    // error register
#define  ATA_FEATURES         0x001    // wpc4 register (features)
#define  ATA_SECTOR_COUNT     0x002    // sector count register
#define  ATA_SECTOR_NUMBER    0x003    // sector number register
#define  ATA_LBA_LOW_BYTE     0x003    // LBA mode 7:0 register
#define  ATA_CYL_LOW          0x004    // cylinder low register
#define  ATA_LBA_MID_BYTE     0x004    // LBA mode 15:8 register
#define  ATA_CYL_HIGH         0x005    // cylinder high register
#define  ATA_LBA_HIGH_BYTE    0x005    // LBA mode 23:16 register
#define  ATA_DRV_HEAD         0x006    // drive and head register
#define  ATA_STATUS           0x007    // status register
#define  ATA_COMMAND          0x007    // command register

#define  ATA_ADPT_CNTRL1      0x3F4    // HDC1 (R)atlernate status/(W)control register
                                       //  (IRQ14 if interrupts are allowed (bit1))
#define  ATA_ADPT_CNTRL2      0x374    // HDC2 (R)atlernate status/(W)control register
                                       //  (IRQ15 if interrupts are allowed (bit1))

#define  ATA_ALT_STATUS       0x002    // Alternate Status Register (R)
#define  ATA_DEV_CONTROL      0x002    // Device Control Register (W)
#define    ATA_DEV_CNTRL_HOB     (1 << 7)  // High Order Byte
#define    ATA_DEV_CNTRL_4HEADS  (1 << 3)  // Obsolete
#define    ATA_DEV_CNTRL_RESET   (1 << 2)  // Reset
#define    ATA_DEV_CNTRL_nINT    (1 << 1)  // disable interrupts
#define    ATA_DEV_CNTRL_eINT    (0 << 1)  // enable interrupts
#define  ATA_DEV_ADDRESS      0x003    // Device Address Register

#define  ATA_DIR_SEND             0    // send to drive
#define  ATA_DIR_RECV             1    // recieve from drive

#define  ATA_TIMEOUT_CNT       2000    // timeout count for i/o (2000mS)
#define  ATA_WAIT_RDY          1500    // 1.5 seconds
#define  ATAPI_WAIT_RDY        6000    // 6 seconds

#define  ATA_CHANNEL_PRIMARY      0    // primary channel   (must be zero)
#define  ATA_CHANNEL_SECONDARY    1    // secondary channel (must be one)

// DEVICE/HEAD register flags
#define  ATA_DH_SET_BITS       0xA0    // ata version 3+ has these bits set. (before has them clear)
#define  ATA_DH_ISLBA          0x40    // set when the bottom half of register has LBA24 in it.

// STATUS register flags
#define  ATA_STATUS_BSY        (1 << 7) // busy bit
#define  ATA_STATUS_RDY        (1 << 6) // Ready bit
#define  ATA_STATUS_DF         (1 << 5) // Drive Fault Error
#define  ATA_STATUS_DSC        (1 << 4) // Seek Complete   ?
#define  ATA_STATUS_DRQ        (1 << 3) // Data Ready
#define  ATA_STATUS_CORR       (1 << 2) // Data corrected  ?
#define  ATA_STATUS_IDX        (1 << 1) // Index Mark      ?
#define  ATA_STATUS_ERR        (1 << 0) // Error

// ERROR register flags
#define  ATA_ERROR_WP          (1 << 6) // Write Protected
#define  ATA_ERROR_MC          (1 << 5) // Media Changed
#define  ATA_ERROR_ABRT        (1 << 2) // Abort
#define  ATA_ERROR_NM          (1 << 1) // No Media


// Bus Mastering
#define  BM0_COMMAND   0x00
#define  BM0_STATUS    0x02
#define    BM_STATUS_ACTIVE  (1 << 0)  // Active
#define    BM_STATUS_ERROR   (1 << 1)  // Error
#define    BM_STATUS_INTR    (1 << 2)  // IDE Interrupt
#define    BM_STATUS_DRV0    (1 << 5)  // Drive 0 is capable of DMA (Semiphore)
#define    BM_STATUS_DRV1    (1 << 6)  // Drive 1 is capable of DMA (Semiphore)
#define    BM_STATUS_SIMPLX  (1 << 7)  // Simplex Only
#define  BM0_ADDRESS   0x04
#define  BM1_COMMAND   0x08
#define  BM1_STATUS    0x0A
#define  BM1_ADDRESS   0x0C

// DMA transfer types
#define DMA_TYPE_NONE      0  // (00000xxx) PIO default mode
#define DMA_TYPE_MULTIWORD 4  // (00100xxx)
#define DMA_TYPE_ULTRA     8  // (01000xxx)

#define ATA_TRNS_TYPE_PIO  0    // use PIO mode (must be zero)
#define ATA_TRNS_TYPE_DMA  1    // use DMA mode (must be one)

// TODO: Change the names of this slightly
#define XFER_UDMA_7      0x47    // 01000_111 // ultra DMA, mode 7
#define XFER_UDMA_6      0x46    // 01000_110 // ultra DMA, mode 6
#define XFER_UDMA_5      0x45    // 01000_101 // ultra DMA, mode 5
#define XFER_UDMA_4      0x44    // 01000_100 // ultra DMA, mode 4
#define XFER_UDMA_3      0x43    // 01000_011 // ultra DMA, mode 3
#define XFER_UDMA_2      0x42    // 01000_010 // ultra DMA, mode 2
#define XFER_UDMA_1      0x41    // 01000_001 // ultra DMA, mode 1
#define XFER_UDMA_0      0x40    // 01000_000 // ultra DMA, mode 0
                                 // 00100_111 // reserved
                                 // 00100_110 // reserved
                                 // 00100_101 // reserved
                                 // 00100_100 // reserved
                                 // 00100_011 // reserved
#define XFER_MW_DMA_2    0x22    // 00100_010 // multiword DMA, mode 2
#define XFER_MW_DMA_1    0x21    // 00100_001 // multiword DMA, mode 1
#define XFER_MW_DMA_0    0x20    // 00100_000 // multiword DMA, mode 0
#define XFER_SW_DMA_2    0x12    // 00010_010 // singleword DMA, not used in version 4+
#define XFER_SW_DMA_1    0x11    // 00010_001 // singleword DMA, not used in version 4+
#define XFER_SW_DMA_0    0x10    // 00010_000 // singleword DMA, not used in version 4+
#define XFER_PIO_7       0x0F    // 00001_111 
#define XFER_PIO_6       0x0E    // 00001_110 
#define XFER_PIO_5       0x0D    // 00001_101 
#define XFER_PIO_4       0x0C    // 00001_100 
#define XFER_PIO_3       0x0B    // 00001_011 
#define XFER_PIO_2       0x0A    // 00001_010 
#define XFER_PIO_1       0x09    // 00001_001 
#define XFER_PIO_0       0x08    // 00001_000 
#define XFER_PIO_SLOW_D  0x01    // 00000_001 // disable IORDY
#define XFER_PIO_SLOW    0x00    // 00000_000 // default PIO mode

// Standard ATA/ATAPI commands
#define  ATA_CMD_RECALIBRATE               0x10   // ata    3
#define  ATA_CMD_READ                      0x20   // ata 2, 3, 4, 5, 6
#define  ATA_CMD_READ_EXT                  0x24   // ata             6
#define  ATA_CMD_WRITE                     0x30   // ata    3, 4, 5, 6
#define  ATA_CMD_WRITE_EXT                 0x34   // ata             6
#define  ATA_CMD_VERIFY                    0x40   // ata    3, 4, 5, 6
#define  ATA_CMD_VERIFY_EXT                0x42   // ata             6
#define  ATA_CMD_FORMAT                    0x50   // vendor specific
#define  ATA_CMD_SEEK                      0x70   // ata    3, 4, 5, 6
#define  ATA_CMD_ID_PACKET_DEVICE          0xA1   // ata    3, 4, 5, 6
#define  ATA_CMD_GET_MEDIA_STATUS          0xDA   // ata       4, 5, 6
#define  ATA_CMD_ID_DEVICE                 0xEC   // ata    3, 4, 5, 6

#define  ATAPI_MAX_PACKET_SIZE               16

// The generic packet command opcodes for CD/DVD Logical Units,
//  Code 5: MMC commands
#define  ATAPI_CMD_TEST_READY              0x00
#define  ATAPI_CMD_REQUEST_SENSE           0x03
#define  ATAPI_CMD_FORMAT_UNIT             0x04
#define  ATAPI_CMD_INQUIRY                 0x12
#define  ATAPI_CMD_START_STOP              0x1B
#define  ATAPI_CMD_LOCK_UNLOCK             0x1E
#define  ATAPI_CMD_READ_FORMAT_CAPACITIES  0x23
#define  ATAPI_CMD_READ_CAPACITY           0x25
#define  ATAPI_CMD_READ                    0x28
#define  ATAPI_CMD_WRITE_10                0x2A
#define  ATAPI_CMD_WRITE_12                0xAA
#define  ATAPI_CMD_SEEK                    0x2B
#define  ATAPI_CMD_WRITE_AND_VERIFY_10     0x2E
#define  ATAPI_CMD_VERIFY_10               0x2F
#define  ATAPI_CMD_FLUSH_CACHE             0x35
#define  ATAPI_CMD_READ_SUB_CHANNEL        0x42
#define  ATAPI_CMD_READ_TOC                0x43
#define  ATAPI_CMD_READ_HEADER             0x44
#define  ATAPI_CMD_PLAY_AUDIO              0x45
#define  ATAPI_CMD_GET_CONFIGURATION       0x46
#define  ATAPI_CMD_PLAY_AUDIO_MSF          0x47
#define  ATAPI_CMD_PLAY_AUDIO_TI           0x48
#define  ATAPI_CMD_GET_EVENT_STATUS_NOTIFICATION 0x4A
#define  ATAPI_CMD_PAUSE_RESUME            0x4B
#define  ATAPI_CMD_GET_PERFORMANCE         0xAC
#define  ATAPI_CMD_STOP_PLAY_SCAN          0x4E
#define  ATAPI_CMD_READ_DISC_INFO          0x51
#define  ATAPI_CMD_READ_TRACK_RZONE_INFO   0x52
#define  ATAPI_CMD_RESERVE_RZONE_TRACK     0x53
#define  ATAPI_CMD_SEND_OPC                0x54
#define  ATAPI_CMD_MODE_SELECT             0x55
#define  ATAPI_CMD_REPAIR_RZONE_TRACK      0x58
#define  ATAPI_CMD_MODE_SENSE              0x5A
#define  ATAPI_CMD_CLOSE_TRACK             0x5B
#define  ATAPI_CMD_BLANK                   0xA1
#define  ATAPI_CMD_SEND_EVENT              0xA2
#define  ATAPI_CMD_SEND_KEY                0xA3
#define  ATAPI_CMD_REPORT_KEY              0xA4
#define  ATAPI_CMD_LOAD_UNLOAD             0xA6
#define  ATAPI_CMD_SET_READ_AHEAD          0xA7
#define  ATAPI_CMD_READ_12                 0xA8
#define  ATAPI_CMD_READ_DVD_STRUCTURE      0xAD
#define  ATAPI_CMD_SET_STREAMING           0xB6
#define  ATAPI_CMD_READ_CD_MSF             0xB9
#define  ATAPI_CMD_SCAN                    0xBA
#define  ATAPI_CMD_SET_SPEED               0xBB
#define  ATAPI_CMD_PLAY_CD                 0xBC
#define  ATAPI_CMD_MECH_STATUS             0xBD
#define  ATAPI_CMD_READ_CD                 0xBE

// Most manditory and optional ATA commands (from ATA-3)
#define  ATA_CMD_NOP                       0x00
#define  ATA_CMD_CFA_REQUEST_EXT_ERR_CODE  0x03
#define  ATA_CMD_DEVICE_RESET              0x08
#define CMD_READ_NATIVE_MAX_EXT          0x27
#define CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#define CMD_WRITE_VERIFY                 0x3C
#define CMD_CFA_TRANSLATE_SECTOR         0x87
#define CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#define CMD_INITIALIZE_DRIVE_PARAMETERS  0x91
#define CMD_STANDBY_IMMEDIATE2           0x94
#define CMD_IDLE_IMMEDIATE2              0x95
#define CMD_STANDBY2                     0x96
#define CMD_IDLE2                        0x97
#define CMD_CHECK_POWER_MODE2            0x98
#define CMD_SLEEP2                       0x99
#define  ATA_CMD_PACKET                    0xA0
#define CMD_CFA_ERASE_SECTORS            0xC0
#define CMD_READ_MULTIPLE                0xC4
#define CMD_WRITE_MULTIPLE               0xC5
#define CMD_SET_MULTIPLE_MODE            0xC6
#define CMD_READ_DMA_QUEUED              0xC7
#define CMD_READ_DMA                     0xC8
#define CMD_READ_DMA_EXT                 0x25
#define CMD_WRITE_DMA                    0xCA
#define CMD_WRITE_DMA_EXT                0x35
#define CMD_WRITE_DMA_QUEUED             0xCC
#define CMD_CFA_WRITE_MULTIPLE_WO_ERASE  0xCD
#define CMD_STANDBY_IMMEDIATE1           0xE0
#define CMD_IDLE_IMMEDIATE1              0xE1
#define CMD_STANDBY1                     0xE2
#define CMD_IDLE1                        0xE3
#define CMD_READ_BUFFER                  0xE4
#define CMD_CHECK_POWER_MODE1            0xE5
#define CMD_SLEEP1                       0xE6
#define CMD_FLUSH_CACHE                  0xE7
#define CMD_WRITE_BUFFER                 0xE8
#define CMD_SET_FEATURES                 0xEF
#define CMD_READ_NATIVE_MAX              0xF8

// set feature registers, etc.
#define  ATA_FEATURE_CODE     0x001    // feature sub code register
#define  ATA_FEATURE_RESULT   0x001    // feature result register (Error register)
#define  ATA_FEATURE_CODE_1   0x002    // feature sub code register 1
#define  ATA_FEATURE_CODE_2   0x003    // feature sub code register 2
#define  ATA_FEATURE_CODE_3   0x004    // feature sub code register 3
#define  ATA_FEATURE_CODE_4   0x005    // feature sub code register 4

// atapi_mode_sense values
#define PC_CURRENT_VALUES     0x00
#define PC_CHANGEABLE_VALUES  0x01
#define PC_DEFAULT_VALUES     0x02
#define PC_SAVED_VALUES       0x03
#define PCODE_READ_ERR_RECOV  0x01
#define PCODE_CDROM_PAGE      0x0D
#define PCODE_CDROM_AUDIO     0x0E
#define PCODE_CAPS_MECHS      0x2A
#define PCODE_ALL_PAGES       0x3F


struct SHORT_CABLES {
  bit16u device;
  bit16u sub_vendor;
  bit16u sub_device;  
};

#define CABLE_TYPE_UNKNOWN       0
#define CABLE_TYPE_40WIRE        1
#define CABLE_TYPE_40WIRE_SHORT  2
#define CABLE_TYPE_80WIRE        3

// set feature command codes
// see page 233 of ATAPI-6.pdf
#define ATA_FEATURE_SET_TRANSFER_MODE   0x03

#define ATA_FEATURE_DEVICE_SPINUP       0x07

#define ATA_DEV_TYPE_OPTICAL_BASE  16        // base sector of data CD (sector of pvd)

// return info returned by ATAPI_READ_CDROM_CAPS
// In Big Endian Format
struct CAPS {
  bit32u lba;
  bit32u size;  // remembering that the top byte (byte 4) is reserved and the Descriptor Code.
};

// Profile numbers
// MMC-5  Table 88
enum {
  MEDIA_PROFILE_0000 = 0x00, // Reserved
  MEDIA_PROFILE_0001 = 0x01, // Non-removable disk Re-writable disk, capable of changing behavior (obsolete)
  MEDIA_PROFILE_0002 = 0x02, // Removable disk Re-writable; with removable media
  MEDIA_PROFILE_0003 = 0x03, // MO Erasable Magneto-Optical disk with sector erase capability
  MEDIA_PROFILE_0004 = 0x04, //  Optical Write Once Optical write once
  MEDIA_PROFILE_0005 = 0x05, // AS-MO Advance Storage – Magneto-Optical
              //0006 – 0007 Reserved
  MEDIA_PROFILE_0008 = 0x08, // CD-ROM Read only Compact Disc capable
  MEDIA_PROFILE_0009 = 0x09, // CD-R Write once Compact Disc capable
  MEDIA_PROFILE_000A = 0x0A, // CD-RW Re-writable Compact Disc capable
              //000B – 000F Reserved
  MEDIA_PROFILE_0010 = 0x10, // DVD-ROM Read only DVD
  MEDIA_PROFILE_0011 = 0x11, // DVD-R Sequential Recording Write once DVD using Sequential recording
  MEDIA_PROFILE_0012 = 0x12, // DVD-RAM Re-writable DVD
  MEDIA_PROFILE_0013 = 0x13, // DVD-RW Restricted Overwrite Re-recordable DVD using Restricted Overwrite
  MEDIA_PROFILE_0014 = 0x14, // DVD-RW Sequential recording Re-recordable DVD using Sequential recording
  MEDIA_PROFILE_0015 = 0x15, // DVD-R Dual Layer Sequential Recording Dual Layer DVD-R using Sequential recording
  MEDIA_PROFILE_0016 = 0x16, // DVD-R Dual Layer Jump Recording Dual Layer DVD-R using Layer Jump recording
  MEDIA_PROFILE_0017 = 0x17, // DVD-RW Dual Layer
  MEDIA_PROFILE_0018 = 0x18, // DVD-Download Disk Recording
              //0019        Reserved
  MEDIA_PROFILE_001A = 0x1A, // DVD+RW DVD+ReWritable
  MEDIA_PROFILE_001B = 0x1B, // DVD+R DVD+Recordable
              //001C – 0029 Reserved
  MEDIA_PROFILE_002A = 0x2A, // DVD+RW Dual Layer DVD+Rewritable Dual Layer
  MEDIA_PROFILE_002B = 0x2B, // DVD+R Dual Layer DVD+Recordable Dual Layer
              //002C - 003F Reserved
  MEDIA_PROFILE_0040 = 0x40, // BD-ROM Blu-ray Disc ROM
  MEDIA_PROFILE_0041 = 0x41, // BD-R SRM Blu-ray Disc Recordable – Sequential Recording Mode
  MEDIA_PROFILE_0042 = 0x42, // BD-R RRM Blu-ray Disc Recordable – Random Recording Mode
  MEDIA_PROFILE_0043 = 0x43, // BD-RE Blu-ray Disc Rewritable
              //0044 – 004F Reserved
  MEDIA_PROFILE_0050 = 0x50, // HD DVD-ROM Read-only HD DVD
  MEDIA_PROFILE_0051 = 0x51, // HD DVD-R Write-once HD DVD
  MEDIA_PROFILE_0052 = 0x52, // HD DVD-RAM Rewritable HD DVD
  MEDIA_PROFILE_0053 = 0x53, // HD DVD-RW Dual Layer
              //0054 - 0057 Reserved
  MEDIA_PROFILE_0058 = 0x58, // HD DVD-R Dual Layer
              //0059        Reserved
  MEDIA_PROFILE_005A = 0x5A, // HD DVD-RW Dual Layer
              //005B - FFFE Reserved
              //FFFF The Drive does not conform to any Profile.
};

struct S_GET_CONFIG_HDR {
  bit32u length;  // in big endian
  bit16u resv;
  bit16u current; // in big endian
};

struct S_GET_CONFIG_DESC {
  bit16u code;    // in big endian
  bit8u  ver_per_cur;
  bit8u  add_len;
  // data
};

struct S_ATA_INFO {
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
  bit16u caps;     // w49      // b7:b6 = 00 = no ATA or ATAPI
                               //         01 = ATA (at least version 3)
                               //         10 = ATAPI
                               //         11 = reserved
                               // b5    =  0 = CHS support only
                               //          1 = LBA support
                               // b4    =  1 = doubleword read/write allowed
                               // b3:b1 =000 = Not optical
                               //        001 = CD-ROM
                               //        010 = CD-R/CD-RW
                               //        011 = DVD
                               //        1xx = reserved
                               // b0    =  0 = reserved
                               // b7    =  1 = DMA supported
                               // b6:b4 =xxx = dma used (0 - 5)
                               // b3:b0 =  0 = reserved
  bit16u caps1;   // w50 (v6)  //
  bit16u PIO_timing;
  bit16u DMA_timing;
  bit16u rest_valid;
  bit16u num_cur_cylinders; // w54
  bit16u num_cur_heads;
  bit16u num_cur_sect_track;
  bit32u cur_capacity;
  bit16u resv2;
  bit32u lba_capacity;      // w60  32-bit capacity (ATA1,2,3,4,5,6,7,8)
  bit16u resv3;
  bit16u multiword_dma;     // w63
  bit16u pio_modes;
  bit16u min_multi_dma;
  bit16u manuf_min_multi_dma;
  bit16u min_pio_cycle0;
  bit16u min_pio_cycle1;
  bit16u resv4[2];
  bit16u resv5[4];
  bit16u queue_depth;
  bit16u resv6[4];
  bit16u major_ver;            // bit 2 = ATA2, bit 3= ATA3, bit 4=ATA/ATAPI4 through 14
  bit16u minor_ver;
  bit16u command_set1;  // w82
  bit16u command_set2;  // w83
  bit16u command_set3;  // w84
  bit16u command_set4;  // w85
  bit16u command_set5;  // w86
  bit16u command_set6;  // w87
  bit16u dma_ultra;     // w88
  bit16u trseuc;
  bit16u trsEuc;
  bit16u cur_apm_values;
  bit16u mprc;
  bit16u hw_config;
  bit16u acoustic;      // w94
  bit16u msrqs;
  bit16u sxfert;
  bit16u sal;
  bit32u spg;           // w98-99
  bit64u lba_capacity2; // w100-103:  48-bit total number of sectors ( = last LBA + 1 )
  bit16u resv7[22];     // w104-125
  bit16u last_lun;      // w126
  bit16u word127;       // w127
  bit16u dlf;
  bit16u csfo;
  bit16u resv8[30];     //w130-159
  bit16u cfa_power;     //w160
  bit16u resv9[15];     //w130-159
  bit16u cur_media_sn[30];
  bit16u resvA[49];     //w206-254
  bit16u integrity;     //w255
};

struct S_ATA {
  int    drv;                  // 0 or 1 (drive on this ata controller) (0 to 31 if sata controller)
  bool   atapi;                // 0 = ata, 1 = atapi
  bit8u  atapi_type;           // PATA, PATAPI, SATA, SATAPI (above)
  bool   removable;            // 0 = fixed media, 1 = device allows removable media
  bit8u  version;              // highest ata(pi) version supported
  bool   large_cap;            // 48-bit capable
  int    words_sect;           // words per sector
  bool   dword_io;             // set if 32-bit IO is allowed
  bit64u capacity;             // capacity of the drive
  bit32u cylinders;            // our calculated cylinders
  bit8u  heads;                //  "       "     heads
  bit8u  sect_track;           //  "       "     spt
  bit8u  command_set;          // ATAPI device, packet command set used
  bool   dma_supported;        // supports DMA?
  bool   lba_supported;        // supports LBA addressing?
  bit8u  transfer_type;        // 0 = no DMA, must be PIO, 4 = multi-word dma, 8 = ultra dma
  bit8u  transfer_mode;        // 0, 1, 2, ..., 6.  mode of above transfer type.
  bool   stat_notif_support;   // TRUE if Media Status Notification is supported
  bool   stat_notif_enabled;   // TRUE if enabled, FALSE if not
  bit8u  cable_type;           // CABLE_TYPE_x0WIRE (80 or 40)
  struct S_ATA_CNTRLR *cntrlr; // pointer to the hdc of this drive.
  struct S_ATA_INFO info;      // info block
  // SATA port specific
  bit32u phy_address;          // physical memory pointer for command list
  int    selector;             // selector to use for reading/writing and freeing memory
  bit32u cmd_list_addr;        // Command List Physical Address (aligned to 1024) (32 entries of 32 bytes each)
  bit32u cmd_table_addr;       // Command Table Physical Address (aligned to 1024) (32 entries of 256 bytes each)
  bit32u fis_add;              // FIS Physical Address (aligned to 256) 256 bytes
  struct S_HBA_CMD_LIST cmd_list[HBA_MAX_CMD_SLOTS];  // our saved command list
};

struct S_PCI_DEV {
  bit8u bus;
  bit8u dev;
  bit8u func;
};

// if you change this, you will need to change
//   struct S_ATA_CNTRLR s_hdc[CNTRLR_COUNT] = {
// in hdc_type.cpp
struct S_ATA_CNTRLR {
  __dpmi_meminfo base_mi;      // operational registers  (Base only)
  int base_selector;           //  ... if the controller is MemMapped
  bit32u base;                 // port base (Legacy or AHCI)
  bit32u alt_base;             // alt port base
  bit32u bus_master;           // port address of bus_master interface (0 if none, use ISA type DMA)
  int    channel;              // channel 0 or 1
  int    irq;                  // irq to use
  int    type;                 // enum of type we found
  int    command_slots;        // AHCI: Command Slots
  bool   mode;                 // Legacy (0) or Native (1)
  bool   is_pci_dev;           // set if we found it as a PCI device
  struct S_PCI_DEV pci_dev;    // if a pci device, this holds some info about it
  struct S_ATA drive[32];      // (AHCI allows up to 32 (root) ports)
};

#define HDC_MODE_LEGACY  0
#define HDC_MODE_NATIVE  1

enum {
  CNTRL_TYPE_UNKNOWN = 0,
  CNTRL_TYPE_PIIX,
  CNTRL_TYPE_PIIX3,
  CNTRL_TYPE_PIIX4,
  CNTRL_TYPE_ICH,
  CNTRL_TYPE_ICH0,
  CNTRL_TYPE_ICH2,
  CNTRL_TYPE_ICH3,
  CNTRL_TYPE_ICH4,
  CNTRL_TYPE_ICH5,
  CNTRL_TYPE_ICH6,
  CNTRL_TYPE_0571,
  
  // SATA's start here
  CNTRL_TYPE_STD_SATA = 0x40,
  CNTRL_TYPE_SATA_SIL,
  
  // SATA's with AHCI start here
  CNTRL_TYPE_ICH9R = 0x80,
  
};

#define ATA_TYPE_PATA    0       // Port IO ATA
#define ATA_TYPE_SATA    1       // Port IO SATA
#define ATA_TYPE_PATAPI  2       // Port IO ATAPI
#define ATA_TYPE_SATAPI  3       // Port IO SATAPI

struct S_BLOCK_STATUS {
  bool   inserted;             // is a disk inserted
  bool   changed;              // has the media been changed (or same media re-inserted)
  bool   write_prot;           // is the disk write protected
  bool   writable;             // writable
  bool   dvd_capable;          // drived is capable of reading a DVD
  bool   read_serial_num;      // drive is capable of reading the media's serial number
  bit16u medium_type;          // 
  bool   removable;            // is the media removable
  bool   is_optical_disc;      // is it a CD, CDR, CDRW, DVD, etc.
};

bool ata_ide_detect(struct S_ATA_CNTRLR *cntrlr, const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func, const char *str);
int ata_controller_type(const bit16u vendor_id, const bit16u device_id, char *str);

bool ata_wait_int(const struct S_ATA *ata, int timeout);
void init_ext_int(const int irq);
void exit_ext_int(const int irq);
void enable_irq_at_8259(int irq_num);
void disable_irq_at_8259(int irq_num);

void dma_init_dma(const struct S_ATA *ata, const bit32u address, const int size);
void dma_start_dma(const struct S_ATA *ata, const bool dir);
bit8u dma_stop_dma(const struct S_ATA *ata);

bool det_ata_controller(struct S_ATA_CNTRLR *);
bool ata_device_reset(struct S_ATA_CNTRLR *, const int, bit16u *);
bool det_ata_drive(struct S_ATA *);
bool ata_get_identify(struct S_ATA *ata, void *buffer);

bool ata_select_drv(const bit16u base, const bit8u drv, const bit8u flags, const bit8u lba24_head);
bool ata_wait_busy(const bit16u, const bit16u);
bool ata_wait(const struct S_ATA *, const bit8u, int);
bool atapi_wait(const struct S_ATA *, const bit8u, int);

void fix_ide_string(char *s, int len);
bit8u ata_get_cable_reporting(const struct S_ATA *ata);
void ata_create_rw_packet(const bit8u command_set, bit8u *packet, const bool dir, const bit64u lba, const bit32u count);
bool ata_check_identify(void *);
bit8u ata_highest_ata_version(const bit16u, const bool);
bool ata_version_supported(const bit16u major_ver, const int vers);
bool ata_capacity(struct S_ATA *, bit64u *);
bool atapi_read_capacity(struct S_ATA *, void *, bit16u);

bool ata_tx_rx_data(const struct S_ATA *ata, bool ttpye, const bool dir, const bit8u command, 
                    const int wait, const bit16u features, const bit64u lba, void *buf,
                    int buflen, const bit32u phy_address);
bool atapi_tx_packet_rx_data(struct S_ATA *ata, const bool ttype, const bool dir, bit8u *packet, void *buf, 
                             int buflen, const bit32u phy_address);
int  words_per_sector(struct S_ATA *, const bool);
bool ata_inserted(struct S_ATA *);
bool ata_get_dma_mode(struct S_ATA *);
bool ata_get_status(struct S_ATA *, struct S_BLOCK_STATUS *);
bool atapi_request_sense(struct S_ATA *, void *, bit16u);
bool atapi_mode_sense(struct S_ATA *, void *, bit16u, bit8u, bit8u);
bool atapi_inquiry(struct S_ATA *, void *, bit16u, bit8u);
bool atapi_get_config(struct S_ATA *, void *, bit16u, bit16u);

bool get_parameters(int argc, char *argv[]);
void dump(const void *addr, bit32u size);
void dump_regs(const struct S_ATA *);

#pragma pack(pop)

#endif  // FYSOS_HDC
