/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  Last updated: 15 July 2020
 */

#ifndef FYSOS_SATA
#define FYSOS_SATA

#pragma pack(push, 1)

#define SATA_MODE_IDE   0
#define SATA_MODE_AHCI  1
#define SATA_MODE_RAID  2
#define SATA_MODE_RESV  3

// Host Control Register offsets
#define HBA_HC_Capabilities       0x00
#define HBA_HC_Glob_host_cntrl    0x04
#define HBA_HC_Interrupt_status   0x08
#define HBA_HC_Ports_implemented  0x0C
#define HBA_HC_Version            0x10
#define HBA_HC_Ccc_ctl            0x14
#define HBA_HC_Ccc_ports          0x18
#define HBA_HC_Em_location        0x1C
#define HBA_HC_Em_control         0x20
#define HBA_HC_Capabilities_ext   0x24
#define HBA_HC_Bohc               0x28


// Port Register offsets
#define HBA_PORT_PxCLB     0x00
#define HBA_PORT_PxCLBU    0x04
#define HBA_PORT_PxFB      0x08
#define HBA_PORT_PxFBU     0x0C
#define HBA_PORT_PxIS      0x10
  #define HBA_PORT_IS_tfes(x) (((x) & 0x00000001) << 30)  // put
  #define HBA_PORT_IS_TFES(x) (((x) & 0x40000000) >> 30)  // get
#define HBA_PORT_PxIE      0x14
#define HBA_PORT_PxCMD     0x18
  #define HBA_PORT_CMD_st(x)  (((x) & 0x00000001) <<  0)  // put
  #define HBA_PORT_CMD_ST(x)  (((x) & 0x00000001) >>  0)  // get
  #define HBA_PORT_CMD_fre(x) (((x) & 0x00000001) <<  4)
  #define HBA_PORT_CMD_FRE(x) (((x) & 0x00000010) >>  4)
  #define HBA_PORT_CMD_fr(x)  (((x) & 0x00000001) << 14)
  #define HBA_PORT_CMD_FR(x)  (((x) & 0x00004000) >> 14)
  #define HBA_PORT_CMD_cr(x)  (((x) & 0x00000001) << 15)
  #define HBA_PORT_CMD_CR(x)  (((x) & 0x00008000) >> 15)
  // reserved              0x1C
#define HBA_PORT_PxTFD     0x20
#define HBA_PORT_PxSIG     0x24
#define HBA_PORT_PxSSTS    0x28
  #define HBA_PORT_SSTS_det(x)  (((x) & 0x0000000F) <<  0)  // put
  #define HBA_PORT_SSTS_DET(x)  (((x) & 0x0000000F) >>  0)  // get
  #define HBA_PORT_SSTS_spd(x)  (((x) & 0x0000000F) <<  4)
  #define HBA_PORT_SSTS_SPD(x)  (((x) & 0x000000F0) >>  4)
  #define HBA_PORT_SSTS_ipm(x)  (((x) & 0x0000000F) <<  8)
  #define HBA_PORT_SSTS_IPM(x)  (((x) & 0x00000F00) >>  8)
#define HBA_PORT_PxSCTL    0x2C
#define HBA_PORT_PxSERR    0x30
#define HBA_PORT_PxSACT    0x34
#define HBA_PORT_PxCI      0x38
#define HBA_PORT_PxSNTF    0x3C
#define HBA_PORT_PxFBS     0x40
#define HBA_PORT_PxDEVSLP  0x44
  // reserved              0x48-0x6F
#define HBA_PORT_PxVS      0x70 // -0x7F

#define HBA_PORT_ADDR(n, p) (0x100 + (n * 0x80) + p)


#define HBA_MAX_PORTS      32
#define HBA_MAX_CMD_SLOTS  32

// PORT:SIG values
#define	SATA_SIG_ATA     0x00000101  // SATA drive
#define	SATA_SIG_ATAPI   0xEB140101  // SATAPI drive
#define	SATA_SIG_SEMB    0xC33C0101  // Enclosure management bridge
#define	SATA_SIG_PM      0x96690101  // Port multiplier
#define	SATA_SIG_NONE    0x00000000  // None Present
#define	SATA_SIG_NON_ACT 0x0000FFFF  // Not Active


#define HBA_PORT_PxSSTS_IPM_NONE      0
#define HBA_PORT_PxSSTS_IPM_ACTIVE    1
#define HBA_PORT_PxSSTS_IPM_PARTIAL   2
#define HBA_PORT_PxSSTS_IPM_SLUMBER   6
#define HBA_PORT_PxSSTS_IPM_DEVSLEEP  8

#define HBA_PORT_PxSSTS_DET_NONE      0
#define HBA_PORT_PxSSTS_DET_NO_PHY    1
#define HBA_PORT_PxSSTS_DET_PRES_PHY  3
#define HBA_PORT_PxSSTS_DET_PHY_OFF   4


// FIS TYPES
enum {
  FIS_TYPE_REG_H2D	  = 0x27,	// Register FIS - host to device
  FIS_TYPE_REG_D2H	  = 0x34,	// Register FIS - device to host
  FIS_TYPE_DMA_ACT	  = 0x39,	// DMA activate FIS - device to host
  FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
  FIS_TYPE_DATA		    = 0x46,	// Data FIS - bidirectional
  FIS_TYPE_BIST		    = 0x58,	// BIST activate FIS - bidirectional
  FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
  FIS_TYPE_DEV_BITS	  = 0xA1, // Set device bits FIS - device to host
};

// sata: page 193
#define FIS_PIO_SETUP_pmport(x)   (((x) & 0x0F) << 0)  // put
#define FIS_PIO_SETUP_PMPORT(x)   (((x) & 0x0F) >> 0)  // get
#define FIS_PIO_SETUP_d(x)        (((x) & 0x01) << 5)
#define FIS_PIO_SETUP_D(x)        (((x) & 0x20) >> 5)
#define FIS_PIO_SETUP_i(x)        (((x) & 0x01) << 6)
#define FIS_PIO_SETUP_I(x)        (((x) & 0x40) >> 6)
#define FIS_PIO_SETUP_a(x)        (((x) & 0x01) << 7)
#define FIS_PIO_SETUP_A(x)        (((x) & 0x80) >> 7)

struct S_FIS_DMA_SETUP {
  bit8u  fis_type;    // FIS_TYPE_DMA_SETUP (0x41)
  bit8u  flags;
  bit16u resv0;

  bit64u dma_buffer_id;  // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.

  bit32u resv1;

  bit32u dma_buf_offset; // Byte offset into buffer. First 2 bits must be 0

  bit32u transfer_count; // Number of bytes to transfer. Bit 0 must be 0

  bit32u resv2;
};

// sata: page 198
#define FIS_PIO_SETUP_pmport(x)   (((x) & 0x0F) << 0)  // put
#define FIS_PIO_SETUP_PMPORT(x)   (((x) & 0x0F) >> 0)  // get
#define FIS_PIO_SETUP_d(x)        (((x) & 0x01) << 5)
#define FIS_PIO_SETUP_D(x)        (((x) & 0x20) >> 5)
#define FIS_PIO_SETUP_i(x)        (((x) & 0x01) << 6)
#define FIS_PIO_SETUP_I(x)        (((x) & 0x40) >> 6)

struct S_FIS_PIO_SETUP {
  bit8u  fis_type;    // FIS_TYPE_PIO_SETUP (0x5F)
  bit8u  flags;
  bit8u  status;
  bit8u  error;

  bit8u  lba_0;      // sect_num
  bit8u  lba_1;      // cyl_low;
  bit8u  lba_2;      // cyl_high;
  bit8u  dev_head;

  bit8u  lba_3;      // sect_num_exp;
  bit8u  lba_4;      // cyl_low_exp;
  bit8u  lba_5;      // cyl_high_exp;
  bit8u  resv0;

  bit8u  sect_count_low;
  bit8u  sect_count_high;
  bit8u  resv1;
  bit8u  e_status;
  
  bit16u transfer_count;
  bit16u resv2;
};

// device to host
// sata: page 189
#define FIS_REG_D2H_pmport(x)   (((x) & 0x0F) << 0)  // put
#define FIS_REG_D2H_PMPORT(x)   (((x) & 0x0F) >> 0)  // get
#define FIS_REG_D2H_i(x)        (((x) & 0x01) << 6)
#define FIS_REG_D2H_I(x)        (((x) & 0x40) >> 6)

struct S_FIS_REG_D2H {
  bit8u  fis_type;    // FIS_TYPE_REG_D2H (0x34)
  bit8u  flags;
  bit8u  status;
  bit8u  error;

  bit8u  lba_0;      // sect_num
  bit8u  lba_1;      // cyl_low;
  bit8u  lba_2;      // cyl_high;
  bit8u  dev_head;

  bit8u  lba_3;      // sect_num_exp;
  bit8u  lba_4;      // cyl_low_exp;
  bit8u  lba_5;      // cyl_high_exp;
  bit8u  resv0;

  bit8u  sect_count_low;
  bit8u  sect_count_high;
  bit8u  resv1;
  bit8u  resv2;
  
  bit32u resv3;
};

// host to device
// sata: page 187
#define FIS_REG_H2D_pmport(x)   (((x) & 0x0F) << 0)  // put
#define FIS_REG_H2D_PMPORT(x)   (((x) & 0x0F) >> 0)  // get
#define FIS_REG_H2D_c(x)        (((x) & 0x01) << 7)
#define FIS_REG_H2D_C(x)        (((x) & 0x80) >> 7)

struct S_FIS_REG_H2D {
  bit8u  fis_type;    // FIS_TYPE_REG_H2D (0x27)
  bit8u  flags;
  bit8u  command;
  bit8u  features;

  bit8u  lba_0;      // sect_num
  bit8u  lba_1;      // cyl_low;
  bit8u  lba_2;      // cyl_high;
  bit8u  dev_head;

  bit8u  lba_3;      // sect_num_exp;
  bit8u  lba_4;      // cyl_low_exp;
  bit8u  lba_5;      // cyl_high_exp;
  bit8u  features_exp;

  bit8u  sect_count_low;
  bit8u  sect_count_high;
  bit8u  reserved;
  bit8u  control;
  
  bit32u resv;
};

// sata: page 187
#define FIS_DATA_pmport(x)   (((x) & 0x0F) << 0)  // put
#define FIS_DATA_PMPORT(x)   (((x) & 0x0F) >> 0)  // get
struct S_FIS_DATA {
  bit8u  fis_type;    // FIS_TYPE_DATA (0x46)
  bit8u  flags;
  bit16u resv0;
  
  // bit32u data[x];	// Payload (min 1, max 2048)
};

// Command List
#define CMD_LIST_ALIGNMENT  1024
// page 48
#define CMD_LIST_a(x)      (((x) & 0x00000001) <<  5)
#define CMD_LIST_A(x)      (((x) & 0x00000020) >>  5)
#define CMD_LIST_w(x)      (((x) & 0x00000001) <<  6)  // put
#define CMD_LIST_W(x)      (((x) & 0x00000040) >>  6)  // get
#define CMD_LIST_c(x)      (((x) & 0x00000001) << 10)  // put
#define CMD_LIST_C(x)      (((x) & 0x00000400) >> 10)  // get
#define CMD_LIST_prdtl(x)  (((x) & 0x0000FFFF) << 16)
#define CMD_LIST_PRDTL(x)  (((x) & 0xFFFF0000) >> 16)

struct S_HBA_CMD_LIST {
  bit32u dword0;
  bit32u prdbc;
  bit32u ctba;
  bit32u ctbau;
  bit32u resv[4];
};

// page 51
#define PRDT_ENTRY_dbc(x)      (((x) - 1) & 0x003FFFFF)  // put
#define PRDT_ENTRY_DBC(x)      (((x) & 0x003FFFFF) + 1)  // get
#define PRDT_ENTRY_int(x)      (((x) & 0x00000001) << 31)
#define PRDT_ENTRY_INT(x)      (((x) & 0x80000000) >> 31)
struct S_HBA_PRDT_ENTRY {
  bit32u dba;    // Data base address
  bit32u dbau;   // Data base address upper 32 bits
  bit32u resv;   // Reserved
  bit32u dword3;
};

struct S_HBA_CMD_TABLE {
  bit8u  cfis[64];    // Command FIS
  bit8u  acmd[16];    // ATAPI command, 12 or 16 bytes
  bit8u  resv0[48];   // Reserved
  struct S_HBA_PRDT_ENTRY prdt_entry[8]; // Physical region descriptor table entries, 0 ~ 65535
};

struct S_HBA_RCV_FIS {
  struct S_FIS_DMA_SETUP dsfis;  // DMA Setup FIS
  bit32u resv0;
  
  struct S_FIS_PIO_SETUP psfis;  // PIO Setup FIS
  bit32u resv1[3];
  
  struct S_FIS_REG_D2H   rfis;   // Register – Device to Host FIS
  bit32u resv2;
  
  bit32u sdbfis[2];              // Set Device Bit FIS
  
  bit8u  ufis[64];               // 

  bit8u  resv3[96];              // padding
};


bool ata_sata_detect(struct S_ATA_CNTRLR *cntrlr, const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func, const char *str);
bool sata_gain_ownership(struct S_ATA_CNTRLR *cntrlr);

bit32u sata_get_port_type(struct S_ATA *ata);
bool sata_port_initialize(struct S_ATA *ata);
void sata_free_port(struct S_ATA *ata);
void sata_start_cmd(struct S_ATA *ata);
bool sata_stop_cmd(struct S_ATA *ata);
bool sata_port_reset(struct S_ATA *ata);
bool sata_tx_rx_data(const struct S_ATA *ata, const bool dir, const bit8u command, const bool is_atapi,
                     bit64u lba, const int buflen, int count, bit8u *packet, const void *buffer);
int sata_find_cmdslot(const struct S_ATA *ata);
bool sata_capacity(struct S_ATA *ata, bit64u *lbas);
bool sata_get_status(struct S_ATA *ata, struct S_BLOCK_STATUS *status);
bool satapi_read_capacity(struct S_ATA *ata, void *buf, bit16u buflen);
bool satapi_get_config(struct S_ATA *ata, void *buf, bit16u buflen, bit16u feature_num);


#pragma pack(pop)

#endif  // FYSOS_SATA
