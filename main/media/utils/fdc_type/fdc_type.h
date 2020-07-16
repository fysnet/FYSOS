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

#ifndef FYSOS_FDC
#define FYSOS_FDC

#pragma pack(push, 1)

//////////////////////////////////////////////////////////////////////////
// FDC constants
#define  FDC_SRA    0x000      // FDC Diskette Status Register A at 3x0h (PS/2)
#define  FDC_SRB    0x001      // FDC Diskette Status Register B at 3x1h (PS/2)
#define  FDC_DOR    0x002      // FDC Digital Output Register at 3x2h  (all systems)
#define  FDC_TDR    0x003
#define  FDC_MSR    0x004      // FDC Main Status Register at 3x4h  (all systems)
#define  FDC_DSR    0x004
#define  FDC_CSR    0x005      // FDC Command Status Register 0 at 3x5h  (all systems)
#define  FDC_FIFO   0x005
#define  FDC_RES    0x006      // when read, returns 0x50 on all controllers I tried
#define  FDC_DIR    0x007      // FDC Digital Input Register at 3x7h (PS/2)
#define    FDC_DIR_CHNG_LINE  0x80
#define  FDC_CCR    0x007      // FDC Configuration Control Register at 3x7h (PS/2)

#define  FDC_TIMEOUT_CNT   500   // timeout count for i/o (500mS)
#define  FDC_TIMEOUT_INT  3000   // timeout count for int set (3000mS = 3 sec)

// Standard FDC driver services
#define FDC_CMD_RESET     0x01  // Reset (Intel 8271)
#define FDC_CMD_MODE      0x01  // National Semiconductor DP8473
#define FDC_CMD_READ_TRK  0x02
#define FDC_CMD_SPECIFY   0x03
#define FDC_CMD_STATUS    0x04
#define FDC_CMD_WRITE     0x05
#define FDC_CMD_READ      0x06
#define FDC_CMD_RECAL     0x07
#define FDC_CMD_SENSE_INT 0x08
#define FDC_CMD_WRITE_DEL 0x09
#define FDC_CMD_READ_ID   0x0A
#define FDC_CMD_MOTOR_ON  0x0B  // Intel 82072
#define FDC_CMD_READ_DEL  0x0C
#define FDC_CMD_FORMAT    0x0D
#define FDC_CMD_DUMP_REGS 0x0E
#define FDC_CMD_SEEK      0x0F
#define FDC_CMD_VERSION   0x10
#define FDC_CMD_SCAN_EQ   0x11
#define FDC_CMD_PERP288   0x12
#define FDC_CMD_CONFIGURE 0x13
#define FDC_CMD_UNLOCK    0x14  // bit 7 + 0x14 (bit 7 = 1 = lock)
#define FDC_CMD_LOCK      0x94  
#define FDC_CMD_VERIFY    0x16
#define FDC_CMD_POWERDOWN 0x17
#define FDC_CMD_PARTID    0x18
#define FDC_CMD_SCAN_LEQ  0x19
#define FDC_CMD_SCAN_HEQ  0x1D
#define FDC_CMD_SET_TRK   0x21  // National Semiconductor DP8473
#define FDC_CMD_SAVE      0x2E
#define FDC_CMD_OPTION    0x33
#define FDC_CMD_EXIT_STND 0x34  // uPD72065A/66 (among others)
#define FDC_CMD_STNDBY    0x35  // uPD72065A/66 (among others)
#define FDC_CMD_H_RESET   0x36  // uPD72065A/66 (among others)
#define FDC_CMD_RESTORE   0x4E
#define FDC_CMD_DRV_SPEC  0x8E  // Intel 82078
#define FDC_CMD_REL_SEEK  0x8F
#define FDC_CMD_FOR_WRITE 0xAD

#define FDC_CMD_VERIFY_SK     (FDC_CMD_READ | FDC_SKIP)

#define FDC_IMPLIED_SEEK  0x40
#define FDC_DISABLE_FIFO  0x20
#define FDC_DISABLE_POLL  0x10

#define DMA_CMD_READ      (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_WRITE)
#define DMA_CMD_WRITE     (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_READ)
#define DMA_CMD_VERIFY    (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_VERIFY)
#define DMA_CMD_FORMAT    (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_READ)

#define FDC_MT            (1<<7)
#define FDC_MFM           (1<<6)
#define FDC_SKIP          (1<<5)

// if bit 7 is set (only on 82072 controllers?), the Sense Status command
//  does not wait for the motor to spin up before getting the status
#define CMD_STATUS_MOT    (1<<7)

// motor control values
#define  MOTOR_ON       TRUE
#define  MOTOR_OFF      FALSE

#define  MOTOR_WAIT       TRUE   // wait FDC_SPINUP regarless of drive status
#define  MOTOR_NO_WAIT    FALSE  // don't wait at all

#define  FDC_TURNOFF_CNT  2000   // time to wait before we turn off drive (2000mS)
#define  FDC_SLT_AFTER_SEEK 25   // time to wait after a seek for heads to settle
// time to wait for drive to spin up
#define  FDC_SPINUP        750   // 1.44 needs 500ms, 720k needs 750ms


#define ST0_RESV_MSK  (1 << 3)   // on the 765A/B, bit 3 is the "not ready" (zero on later controllers)
#define ST0_INT_CODE  (3 << 6)
#define ST0_SEEK_END  (1 << 5)
#define ST0_EQU_CHK   (1 << 4)
#define ST0_HEAD_SEL  (1 << 2)
#define ST0_DRV_SEL   (3 << 0)


// these have to be in order with fdc_dev_types[]
enum {
  FDC_TYPE_82078SL = 0,
  FDC_TYPE_82078,
  FDC_TYPE_44_82078,
  FDC_TYPE_S82078B,
  FDC_TYPE_NS_PC87306,
  FDC_TYPE_82072,
  FDC_TYPE_82072A,
  FDC_TYPE_82077_ORIG,
  FDC_TYPE_82077,
  FDC_TYPE_82077AA,
  FDC_TYPE_8272A,
  FDC_TYPE_765A,
  FDC_TYPE_765B,
  FDC_TYPE_72065A,
  FDC_TYPE_DP8473,
  FDC_TYPE_NEC72065B,
  FDC_TYPE_W83977,
  FDC_TYPE_UNKNOWN
};

enum { 
  FLOPPY_MEDIA_UNKNOWN = 0,
  FLOPPY_MEDIA_160,
  FLOPPY_MEDIA_180,
  FLOPPY_MEDIA_320,
  FLOPPY_MEDIA_360,
  FLOPPY_MEDIA_1_20,
  FLOPPY_MEDIA_720,
  FLOPPY_MEDIA_1_44,
  FLOPPY_MEDIA_1_72,
  FLOPPY_MEDIA_2_88
};

// code sent to FDC_CCR register
#define FDC_500KBPS    0x00
#define FDC_300KBPS    0x01
#define FDC_250KBPS    0x02
#define FDC_1000KBPS   0x03

struct S_BLOCK_STATUS {
  bool   inserted;             // is a disk inserted
  bool   changed;              // has the media been changed (or same media re-inserted)
  bool   write_prot;           // is the disk write protected
  bit32u cur_cyl;              // current cylinder number of drv specified
  bit32u cur_head;             // current head number of drv specified
  bit32u cur_sector;           // current sector number of drv specified
  bit32u sector_size;          // current sector size of current track of drv specified (2 = 512)
};

// don't won't to overwrite last item when updating these items in fdd_det_media_type()
#define S_FLPY_SIZE_TO_CPY  (sizeof(struct S_FLOPPY) - sizeof(struct S_FLOPPY_CNTRLR *) - sizeof(bit8u) - sizeof(bit8u) - sizeof(bit8u))

struct S_FLOPPY {
  bool   inserted;             // is there a disk in the drive (i.e.: can we trust disk specific items below)
  bit8u  type;                 // type (see FLOPPY_MEDIA_XXX) (0xFF = unknown drive type)
  bit16u tot_secs;             // total sectors
  bit16u cylinders;            // total cylinders
  bit8u  heads;                // total heads
  bit8u  sect_trk;             // sectors per track
  bit16u b_sector;             // bytes per sector
  bit8u  headsttl;             // head settle time
  bit8u  headunload;           // head unload time (table value)
  bit8u  stepping;             // double step ( 0 = no, 1 = yes, other value illegal...)
  bit8u  steprate;             // step rate
  bool   densf;                // density flag
  bit8u  headmap[2];           // head map
  bit8u  sectlen;              // sector len (2 = 512 bytes)
  bit8u  gaplen;               // gap len
  bit8u  datalen;              // max 255 secs
  bit8u  gaplenf;              // gap len (format)
  bit8u  filler;               // filler byte
  bit8u  trans_speed;          // transfer speed code (1000KPS, 500KPS, 300KPS, 250KPS
  bool   perp_mode;            // 1 = use perpendicular288 mode
  bit8u  drv;                  // 0,1,2, or 3
  bit8u  cur_cylinder;         // current cylinder
  struct S_FLOPPY_CNTRLR *cntrlr; // pointer to the fdc of this drive.
};

struct S_FLOPPY_CNTRLR {
  bit16u base;                 // port base
  bit8u  type;                 // index in a type string array
  bit8u  partid;               // part id
  bool   partid_valid;         // did we get a valid partid value?
  bool   enhanced;             // is it an enhanced controller?
  bool   implied_seek;         // can the controller seek for us?
  bool   dump_valid;
  struct {
    bit8u  pcn0;
    bit8u  pcn1;
    bit8u  pcn2;
    bit8u  pcn3;
    bit8u  srt_hut;
    bit8u  hlt_nd;
    bit8u  eot;
    bit8u  perp_info;
    bit8u  config_info;
    bit8u  write_pre;
  } dump_regs;
  struct S_FLOPPY fdd[4];
};

void init_ext_int();
void exit_ext_int();

bool fdc_want_more(struct S_FLOPPY_CNTRLR *);
bool write_fdc(struct S_FLOPPY_CNTRLR *, const bit8u);
bool fdc_wait_int(int);
void fdc_motor_cntr(struct S_FLOPPY *, const bool, const bool);

bool fdd_init_controller(struct S_FLOPPY_CNTRLR *);
bool fdc_detect(struct S_FLOPPY_CNTRLR *);
bit8u fdc_get_type(struct S_FLOPPY_CNTRLR *, const bit8u);
int get_max_FIFO_size(struct S_FLOPPY_CNTRLR *);
bool det_floppy_drive(struct S_FLOPPY *);
bool fdd_det_media_type(struct S_FLOPPY *);

void fdd_lba_to_chs(int, int, int, bit8u *, bit8u *, bit8u *);
bool fdd_inserted(struct S_FLOPPY *, struct S_BLOCK_STATUS *);
void fdd_get_status(struct S_FLOPPY *, struct S_BLOCK_STATUS *);
bool fdc_get_cur_pos(struct S_FLOPPY *, const bit8u, bit32u *, bit32u *, bit32u *, bit32u *);

bool fdc_detect_implied_seek(struct S_FLOPPY *);
bool fdc_configure(struct S_FLOPPY_CNTRLR *, const bit8u, const bit8u);

bool fdc_command(struct S_FLOPPY_CNTRLR *, const bit8u *, bit8u);
bool fdc_command_int(struct S_FLOPPY *fdd, const bit8u *buf, bit8u cnt, bit8u *ret_cnt, bit8u *status, bit8u *cur_cyl);
bit8u fdc_return(struct S_FLOPPY_CNTRLR *, const bit8u, bit8u *);

bool fdd_recalibrate(struct S_FLOPPY *);

bool fdd_seek_to_track(struct S_FLOPPY *, int);
bool fdd_read_sectors(struct S_FLOPPY *, int, bit32u, bit8u *);

bool get_parameters(int argc, char *argv[]);

// Used if you plan to debug your code
//void printf_st0(const bit8u st0);
//void printf_st1(const bit8u st1);
//void printf_st2(const bit8u st2);
//void printf_st3(const bit8u st3);

#pragma pack(pop)

#endif  // FYSOS_FDC
