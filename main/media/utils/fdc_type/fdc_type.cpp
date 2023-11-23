/*
 *                             Copyright (c) 1984-2023
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
 *  FDC_TYPE.EXE
 *   Will probe the Floppy Disk Controller and return the type found.
 *
 *  ** Does not work in an emulated environment within VirtualBox. 
 *     VirtualBox does not handle reads past end of track correctly.
 *     It is noted within the VirtualBox source code as well. :-)
 *     (To get around this, we could change the technique from reading
 *      sectors to reading ID's...)
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS envirnment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *
 *  Last updated: 23 Nov 2023
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os fdc_type.cpp -o fdc_type.exe -s
 *
 *  Usage:
 *    fdc_type [-v]
 *
 *    -v indicates verbose output
 */

#include <conio.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <crt0.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/movedata.h>

#include "..\include\ctype.h"
#include "..\include\dma.h"
#include "..\include\pci.h"
#include "..\include\timer.h"

#include "fdc_type.h"

// lock all memory, to prevent it being swapped or paged out
int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY;

volatile bool fdc_drv_stats = FALSE;

struct S_FLOPPY_CNTRLR s_fdc[2] = {
  { 0x3F0, },
  { 0x370, }
};

const char *fdc_dev_types[] = {
  "82078SL or 82078-1",
  "82078",
  "44pin 82078",
  "S82078B",
  "NS PC87306",
  "82072",
  "82072A",
  "pre-1991 82077",
  "82077",
  "82077AA",
  "8272A",
  "765A",
  "765B",
  "72065A",
  "DP8473",
  "NEC 72065B",
  "Winbond W83977",
  "37c78",
  "Unknown or faulty"
};

#define MEDIA_COUNT  10
const struct S_FLOPPY floppy_media[MEDIA_COUNT] = {
/* inserted
    |  type of disk/drive
    |  |                  tot_secs
    |  |                    |  cylinders
    |  |                    |   | heads
    |  |                    |   |  | sect_trk
    |  |                    |   |  |  |  b_sector
    |  |                    |   |  |  |   |  headsttl
    |  |                    |   |  |  |   |   | headunload
    |  |                    |   |  |  |   |   |  | stepping
    |  |                    |   |  |  |   |   |  | | steprate
    |  |                    |   |  |  |   |   |  | |  | densf
    |  |                    |   |  |  |   |   |  | |  |  | headmap[2]
    |  |                    |   |  |  |   |   |  | |  |  |   | sectlen
    |  |                    |   |  |  |   |   |  | |  |  |   |   | gaplen
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   | datalen
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |  gaplenf
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |    | filler
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |    |  |    trans_speed code
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |    |  |    |         perp mode 
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |    |  |    |          | drv 
    |  |                    |   |  |  |   |   |  | |  |  |   |   |   |  |    |  |    |          | | cur_cly
    |  |                    |   |  |  |   |   |  | |  |  |  / \  |   |  |    |  |    |          | |     |   *cntrlr         media desc ? */
  { 0, FLOPPY_MEDIA_UNKNOWN, 0, 0, 0, 0,   0, 0, 0,0, 0, 0, 0,0, 0,  0,  0,  0, 0,           0, 0,0,    0,    0 },  //          |
  { 0, FLOPPY_MEDIA_160,   320, 40,1, 8, 512, 2,15,0,13, 1, 0,0, 2, 42,255, 84, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  160k  (0xFE)
  { 0, FLOPPY_MEDIA_180,   360, 40,1, 9, 512, 2,15,0,13, 1, 0,0, 2, 42,255, 84, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  180k  (0xFC)
  { 0, FLOPPY_MEDIA_320,   640, 40,2, 8, 512, 2,15,0,13, 1, 0,1, 2, 42,255, 84, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  320k  (0xFF)
  { 0, FLOPPY_MEDIA_360,   720, 40,2, 9, 512, 2,15,0,13, 1, 0,1, 2, 35,255, 80, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  360k  (0xFD)
  { 0, FLOPPY_MEDIA_1_20, 2400, 80,2,15, 512, 2,15,0,13, 1, 0,1, 2, 27,255, 84, 0, FDC_500KBPS, 0,0, 0xFF, 0x00 },  // 1.20m  (0xF9)
  { 0, FLOPPY_MEDIA_720,  1440, 80,2, 9, 512, 2,15,0,10, 1, 0,1, 2, 32,255, 80, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  720k  (0xF9)
  { 0, FLOPPY_MEDIA_1_44, 2880, 80,2,18, 512, 2,15,0,10, 1, 0,1, 2, 27,255,108, 0, FDC_500KBPS, 0,0, 0xFF, 0x00 },  // 1.44m  (0xF0)
  { 0, FLOPPY_MEDIA_1_72, 3360, 80,2,21, 512, 2,15,0,13, 1, 0,1, 2,  4,255, 12, 0, FDC_500KBPS, 0,0, 0xFF, 0x00 },  // 1.72m  ( ? )
  { 0, FLOPPY_MEDIA_2_88, 5760, 80,2,36, 512, 2,15,0,10, 1, 0,1, 2, 27,255, 76, 0,FDC_1000KBPS, 1,0, 0xFF, 0x00 }   // 2.88m  ( ? )
};

const char *fdd_types_str[10] = {
  "UNKNOWN", "160k", "180k", "320k", "360k", "1.20M", "720k", "1.44M", "1.72M", "2.88M"
};

bool verbose = FALSE;

int main(int argc, char *argv[]) {
  int i, j;

  // parse the command line parameters
  if (!get_parameters(argc, argv))
    return -1;
  
  // Initialize and allow interrupts for the FDC
  init_ext_int();

  // setup our delay system
  if (!setup_timer()) {
    printf("Error setting up the timer...\n");
    return -2;
  }

  // two controller ports
  for (j=0; j<2; j++) {
    if (fdc_detect(&s_fdc[j])) {
      printf("Found %sFloppy Disk Controller at 0x%04X with type %s\n", 
        (s_fdc[j].enhanced) ? "an Enhanced " : "a ",
        s_fdc[j].base,
        fdc_dev_types[s_fdc[j].type]
      );
      
      printf("Is an Enanced FDC: %s\n", (s_fdc[j].enhanced) ? "yes" : "no");
      printf(" Has Implied Seek: %s\n", (s_fdc[j].implied_seek) ? "yes" : "no");
      if (s_fdc[j].partid_valid) printf("      Has Part ID: yes (%02Xh)\n", s_fdc[j].partid);
      else                       printf("      Has Part ID: no\n");
      if (s_fdc[j].dump_valid) {
        printf("  PCN Drive 0: %2i\n", s_fdc[j].dump_regs.pcn0);
        printf("  PCN Drive 1: %2i\n", s_fdc[j].dump_regs.pcn1);
        printf("  PCN Drive 2: %2i\n", s_fdc[j].dump_regs.pcn2);
        printf("  PCN Drive 3: %2i\n", s_fdc[j].dump_regs.pcn3);
        printf("      SRT/HUT: %02Xh\n", s_fdc[j].dump_regs.srt_hut);
        printf("       HLT/ND: %02Xh\n", s_fdc[j].dump_regs.hlt_nd);
        printf(" End of Track: %02Xh\n", s_fdc[j].dump_regs.eot);
        printf("   others    : %02Xh\n", s_fdc[j].dump_regs.perp_info);
        printf("   others    : %02Xh (FIFO size = %i)\n", s_fdc[j].dump_regs.config_info, (s_fdc[j].dump_regs.config_info & 0xF) + 1);
        printf("       PRETRK: %02Xh\n", s_fdc[j].dump_regs.write_pre);
        printf(" Calculated max size of FIFO: %i\n", get_max_FIFO_size(&s_fdc[j]));
      }
      
      // detect the floppy disks attached
      for (i=0; i<4; i++) {
        s_fdc[j].fdd[i].cntrlr = &s_fdc[j];
        s_fdc[j].fdd[i].drv = (bit8u) i;
        if (det_floppy_drive(&s_fdc[j].fdd[i])) {
          printf("Found Floppy drive at index %i\n", i);
          if (s_fdc[j].fdd[i].inserted) {
            if (s_fdc[j].fdd[i].type > 0)
              printf("Found Floppy disk with type %s and %i sectors\n",
                fdd_types_str[s_fdc[j].fdd[i].type], s_fdc[j].fdd[i].tot_secs);
            else
              printf("Found Floppy disk with unknown type\n");
          }
        }
      }
      
      // reset the controller
      outpb(s_fdc[j].base + FDC_DOR, 0x00);  // reset drive
      mdelay(2);                             // hold for 2 milliseconds
      outpb(s_fdc[j].base + FDC_DOR, 0x0C);  // release reset
      
      // wait for the interrupt, then sense an interrupt on all four drives
      // Page 41 of the FDC 82077AA specs
      if (fdc_wait_int(FDC_TIMEOUT_INT)) {
        bit8u buf[16];
        for (int i=0; i<4; i++) {
          buf[0] = FDC_CMD_SENSE_INT;
          fdc_command(&s_fdc[j], buf, 1, FALSE);
          fdc_result(&s_fdc[j], 2, buf);
        }
      } else {
        printf("No interrupt....\n");
        return FALSE;
      }
      fdc_configure(&s_fdc[j], FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16);

    }
  }
  
  // restore the interrupt vector
  exit_ext_int();
  
  return 0;
}

bool fdc_detect(struct S_FLOPPY_CNTRLR *fdc) {
  bit8u dir;
  bit8u buf[16];
  clock_t delayms;

  fdc_drv_stats = 0;
  
  // initial reading should not be 0xFF
  if (inpb(fdc->base + FDC_MSR) == 0xFF)
    return FALSE;
  
  // reset the controller
  outpb(fdc->base + FDC_DOR, 0x00);  // reset
  mdelay(2);                         // hold for 2 milliseconds
  outpb(fdc->base + FDC_DOR, 0x0C);  // release
  mdelay(2);                         // hold for 2 milliseconds
  
  // wait for the interrupt, then sense an interrupt on all four drives
  // Page 41 of the FDC 82077AA specs
  if (fdc_wait_int(FDC_TIMEOUT_INT)) {
    for (int i=0; i<4; i++) {
      buf[0] = FDC_CMD_SENSE_INT;
      fdc_command(fdc, buf, 1, FALSE);
      fdc_result(fdc, 2, buf);
    }
  } else {
    printf("No interrupt....\n");
    return FALSE;
  }
  
  // If FDC_MSR:RQM set and FDC_MSR:CB clear before 200 ms,
  //  then controller found
  delayms = clock() + 200;
  while (delayms > clock()) {
    if (inpb(fdc->base + FDC_MSR) == 0x80)
      break;
  }
  
  // if the controller isn't ready for input after the delay, return FALSE
  if (inpb(fdc->base + FDC_MSR) != 0x80)
    return FALSE;
  
  // get the initial register value from DIR
  //  to pass along to the fdc_get_type() function
  dir = inpb(fdc->base + FDC_DIR);
  
  // now see if it will handle a command
  // send the specify command
  buf[0] = FDC_CMD_SPECIFY;
  buf[1] = 0xAF;
  buf[2] = 0x1E;
  if (!fdc_command(fdc, buf, 3, TRUE))
    return FALSE;
  
  if (verbose)
    printf("Initial Register Values:\n"
           "    Status Register A: %02X\n"
           "    Status Register B: %02X\n"
           "       Digital Output: %02X\n"
           "           Tape Drive: %02X\n"
           " Main Status Register: %02X\n"
       /*  " Data (FIFO) Register: %02X\n" */ // Reading the FIFO with DIO = Write, has undefined results.
           "             Reserved: %02X\n"
           "        Digital Input: %02X\n",
      inpb(fdc->base + 0), inpb(fdc->base + 1), inpb(fdc->base + 2), inpb(fdc->base + 3),
      inpb(fdc->base + 4), /* inpb(fdc->base + 5), */ inpb(fdc->base + 6), inpb(fdc->base + 7));
  
  // if we get here, a controller was found
  // get the type of controller
  fdc->type = fdc_get_type(fdc, dir);
  
  return TRUE;
}

/* Get Controller Type
 *  The code below will send specific commands to the controller, eliminating those
 *   that don't support that command, until only one is left, to determine what
 *   controller is present.
 */
bit8u fdc_get_type(struct S_FLOPPY_CNTRLR *fdc, const bit8u dir) {
  bit8u r = 0, buf[32];
  fdc->enhanced = FALSE;
  
  if (fdc_configure(fdc, FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16))
    fdc->implied_seek = TRUE;
  else
    printf("fdc_configure returned false\n");
  
  // first see if dumpregs command works
  // supported: 82077AA  82078  37c78  PC87306
  // not supported: NEC765  DP8473
  
  buf[0] = FDC_CMD_DUMP_REGS;
  if (fdc_command(fdc, buf, 1, FALSE))
    r = fdc_result(fdc, 10, (bit8u *) &fdc->dump_regs);
  if (r == 0) {
    // could be a 765A, 765B, or DP8473
    // Of these three, only the 765B supports the version command
    buf[0] = FDC_CMD_VERSION;
    if (fdc_command(fdc, buf, 1, FALSE) && 
       (fdc_result(fdc, 1, buf) == 1)) {
      if (buf[0] == 0x90)
        return FDC_TYPE_765B;
      // unknown controller that doesn't have DUMP, but does have VERSION
      return FDC_TYPE_UNKNOWN;
      
    } else {
      // could be a 765A or DP8473
      // the DP8473 has an Internal Track Number register that we can read/write, the other three do not
      buf[0] = FDC_CMD_SET_TRK;     // read current internal track number
      buf[1] = 0x30 | (0 << 2) | 0; // read LSB
      buf[2] = 0;                   // not used on the read
      if (fdc_command(fdc, buf, 3, FALSE) &&
         (fdc_result(fdc, 1, buf) == 1))
        return FDC_TYPE_DP8473;  // Type DP8473 compatible FDC
      return FDC_TYPE_765A;    // Type 765A compatible FDC
    }
  }
  
  // if it did not return 10 bytes, unexpected
  // (the DUMP command is an enhanced command, so mark as so)
  if (r == 10) {
    // if both the DUMP REGISTERS and the VERSION commands are supported,
    //  we have an enhanced controller.
    buf[0] = FDC_CMD_VERSION;
    if (((fdc_command(fdc, buf, 1, FALSE)) && 
         (fdc_result(fdc, 1, buf) == 1)) && 
         (buf[0] == 0x90))
      fdc->enhanced = TRUE;
    fdc->dump_valid = TRUE;
  } else
    printf("Dumpregs: unexpected count of returned bytes: %i\n", r);
  
  // could be a 82077AA  82078  37c78  PC87306
  // the 82072 is the only (known) controller with the MOTOR_ON_OFF command.
  //  (the 82072 is not register compatible with the remaining. We can not support it yet)
  //buf[0] = FDC_CMD_MOTOR_ON;
  //if (fdc_command(fdc, buf, 1, TRUE))
  //  return FDC_TYPE_82072;

  // could be a 82077AA  82078  37c78  PC87306
  // Try the Save command.  The Intel 82078(44) and the 82078SL(64) have this command
  buf[0] = FDC_CMD_SAVE;
  if (fdc_command(fdc, buf, 1, FALSE) &&
     (fdc_result(fdc, 16, buf) >= 15)) {
    // TODO: now determine which it is
    // the difference is that the 82078 has StatusA reserved (base+0)
    //  or
    // the 82078SL returns 0x41 in the part_id command, the 82078 returns 0x01
    buf[0] = FDC_CMD_PARTID;
    if (fdc_command(fdc, buf, 1, FALSE) &&
       (fdc_result(fdc, 1, buf) == 1)) {
      if (buf[0] == 0x01)
        return FDC_TYPE_82078SL;
      if (buf[0] == 0x41)
        return FDC_TYPE_82078;
    }
    return FDC_TYPE_UNKNOWN;
  }
  
  // could be a 82077AA  37c78  PC87306
  // the PC87306 has part_id, the other two do not
  buf[0] = FDC_CMD_PARTID;
  if (fdc_command(fdc, buf, 1, FALSE) &&
     (fdc_result(fdc, 1, buf) == 1))
    return FDC_TYPE_NS_PC87306;
  
  // could be a 82077AA  37c78
  // the 82077AA has the scan commands, the 37c78 does not
  // however, the 82077AA has StatusRegA and B, the 37c78 does not.
  // we assume a non-supported will return 0xFF and a supported will return !0xFF
  if (inpb(fdc->base+FDC_SRA) != 0xFF)
    return FDC_TYPE_82077AA;
  
  // probably is the 37c78
  return FDC_TYPE_37c78;
}

/* Calculate the size of the FIFO
 * [This is experimental. I have seen no documentation that this does not work.]
 * This is done by setting the max size with the configuration command,
 *  then reading it back using the dumpregs command until the two
 *  values match.
 */
int get_max_FIFO_size(struct S_FLOPPY_CNTRLR *fdc) {
  bit8u buf[16];
  int size = 23; // try a max size of 24 (most will be 16)
  
  while (size >= 0) {
    buf[0] = FDC_CMD_CONFIGURE;
    buf[1] = 0;     // zero
    buf[2] = 0x50 | (bit8u) size;  // EIS = TRUE, Disable Poll, Enable FIFO, FIFO = x bytes
    buf[3] = 0;     // pretrk = 0
    if (!fdc_command(fdc, buf, 4, TRUE))
      return 0;
    
    int r = 0;
    buf[0] = FDC_CMD_DUMP_REGS;
    if (fdc_command(fdc, buf, 1, FALSE))
      r = fdc_result(fdc, 10, buf);
    if (r != 10)
      return 0;
    
    if ((int) (buf[8] & 0xF) == size)
      return size + 1;
    
    size--;
  }
  
  return 0;
}

// detect floppy drives one at a time
// works whether disk is in drive or not
bool det_floppy_drive(struct S_FLOPPY *fdd) {
  struct S_BLOCK_STATUS b_status;
  bit8u buf[32], status, cnt;
  bool eis_chk = FALSE;
  
  // recalibrate the drive (turns motor on for us)
  fdd_recalibrate(fdd);
  
  // seek to track zero
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 0;
  fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be set
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2, FALSE);
  cnt = fdc_result(fdd->cntrlr, 1, &status);
  
  if ((cnt != 1) || ((status & 0x13) != (0x10 | fdd->drv)))
    return FALSE;
  
  // seek to track 5 (any present non-zero track works)
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 5;
  fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be clear
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2, FALSE);
  cnt = fdc_result(fdd->cntrlr, 1, &status);
  if ((cnt != 1) || ((status & 0x13) != fdd->drv))
    return FALSE;
  
  // seek to track 0
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 0;
  fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be set
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2, FALSE);
  cnt = fdc_result(fdd->cntrlr, 1, &status);
  if ((cnt != 1) || ((status & 0x13) != (0x10 | fdd->drv)))
    return FALSE;
  
  // found a drive
  // set unknown media type for now
  fdd->type = FLOPPY_MEDIA_UNKNOWN;
  
  // If there is a disk in the drive, detect the disk type as drive type.
  fdd->inserted = fdd_inserted(fdd, &b_status);
  if (fdd->inserted) {
    fdd_det_media_type(fdd);
    if (fdd->type != FLOPPY_MEDIA_UNKNOWN) {
      // send the specify command
      buf[0] = FDC_CMD_SPECIFY;
      buf[1] = (fdd->steprate << 4) | fdd->headunload;
      buf[2] = (fdd->headsttl << 1) | 0;
      fdc_command(fdd->cntrlr, buf, 3, TRUE);
      
      // set data rate speed
      outpb(fdd->cntrlr->base + FDC_CCR, fdd->trans_speed);
      
      // Verify the drive can "automatically seek" before a read and write.
      if (fdd->cntrlr->enhanced) {
        eis_chk = fdc_detect_implied_seek(fdd);
        if (eis_chk && !fdd->cntrlr->implied_seek)
          printf("Implied Seek Check passed, but Configure(IES) failed...\n");
        else if (!eis_chk && fdd->cntrlr->implied_seek)
          printf("Implied Seek Check failed, but Configure(IES) passed...\n");
        else if (eis_chk && fdd->cntrlr->implied_seek)
          printf("Implied Seek confirmed available.\n");
        else if (!eis_chk && !fdd->cntrlr->implied_seek)
          printf("Implied Seek confirmed not available.\n");
      }
    } else {
      printf(" *** Did not detect the media type\n");
    }
  }
  
  // turn off motor(s)
  fdc_motor_cntr(fdd, MOTOR_OFF, 0);
  
  return TRUE;
}

///////////////////////////////////////////////////////////////////
// detect media type
//  We will probe for (non) existant sectors.
//  If sector not found, eliminates some types, etc.
// *must have a valid fdd->cntrlr->base and fdd->drv passed to it*
// if successful, return and updated fdd block
bool fdd_det_media_type(struct S_FLOPPY *fdd) {
  bit8u buf[16], gpl;
  bit8u status, cur_cyl, ret_cnt;
  bit32u phy_address;
  int sel, i, start;
  bool ret = FALSE;
  
  // our calculated values
  int cyls = 0, heads = 0, spt = 0;
  
  // select and turn motor on
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_WAIT);
  
  // let's start with assuming we don't know the drive type
  fdd->type = FLOPPY_MEDIA_UNKNOWN;
  
  // seek to a track that is not track zero
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 1;
  fdc_command_int(fdd, buf, 3, &ret_cnt, &status, &cur_cyl);
  if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != 1))
    return FALSE;
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // seek back to track zero
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 0;
  fdc_command_int(fdd, buf, 3, &ret_cnt, &status, &cur_cyl);
  if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != 0))
    return FALSE;
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // Configure 
  if (fdd->cntrlr->enhanced) {
    buf[0] = FDC_CMD_CONFIGURE;
    buf[1] = 0;     // zero
    buf[2] = 0x5F;  // EIS = TRUE, Disable Poll, Enable FIFO, FIFO = 16 bytes
    buf[3] = 0;     // pretrk = 0
    if (!fdc_command(fdd->cntrlr, buf, 4, TRUE))
      return FALSE;
  }
  
  // set the gap length for 1_44, 720, or 1_22 media
  gpl = 0x1B;
  
  // Specify command
  buf[0] = FDC_CMD_SPECIFY;
  buf[1] = 0xAF;          // Step Rate of 0x0A, Head Unload Time of 0x0F
  buf[2] = (1 << 1) | 0;  // Head Load Time of 0x01, NonDMA if FALSE
  if (!fdc_command(fdd->cntrlr, buf, 3, TRUE))
    return FALSE;
  
  // TODO: If 82077AA or better (may need index into types), 
  // send 0x00 to port 0x3F7 (500kbps)
  //outpb(fdd->cntrlr->base + FDC_DSR, 0x00);
  
  // Recalibrate (Make sure we are at Track Zero)
  buf[0] = FDC_CMD_RECAL;
  buf[1] = fdd->drv;
  fdc_command_int(fdd, buf, 2, NULL, &status, &cur_cyl);
  if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != 0))
    return FALSE;
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  
  // Read ID
  // We try to read from head 1.
  //  If this media is single sided, the controller will return 0x4x in the ST0 register (abnormal termination)
  buf[0] = FDC_MFM | FDC_CMD_READ_ID;
  buf[1] = (1 << 2) | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2, FALSE);
  // wait for the interrupt to happen 
  if (fdc_wait_int(FDC_TIMEOUT_INT)) {
    if (fdc_result(fdd->cntrlr, 7, buf) > 0) {
      if (((buf[0] & ~ST0_RESV_MSK) == ((1 << 2) | fdd->drv)) && (buf[3] == 0) && (buf[4] == 1))
        heads = 2;
      else if (((buf[0] & 0xF0) == 0x40) && (buf[1] == 0x01))
        heads = 1;
      else
        ; // unknown error/condition
    }
  }
  
  /*
   * We now have the count of heads.  Move on to the SPT.
   */

  // if there is only one head, then specify for a 5 1/2" disk
  if (heads == 1) {
    buf[0] = FDC_CMD_SPECIFY;
    buf[1] = 0xDF;          // Step Rate of 0x0D, Head Unload Time of 0x0F
    buf[2] = (1 << 1) | 0;  // Head Load Time of 0x01, NonDMA if FALSE
    if (!fdc_command(fdd->cntrlr, buf, 3, TRUE))
      return FALSE;
    
    // set the gap length for 160k or 180k media
    gpl = 0x2A;
    
    buf[0] = FDC_CMD_RECAL;
    buf[1] = fdd->drv;
    fdc_command_int(fdd, buf, 2, NULL, &status, &cur_cyl);
    
    // wait for the head to settle
    mdelay(FDC_SLT_AFTER_SEEK);
  }
  
  /* To find out how many sectors per track there are, we can read a number of sectors (8) and see if the
   *  return has incremented the cylinder count.  i.e.: the return has the next CHS address to read.
   *  Therefore, see if the next address is on the next cylinder.
   * The controller will increment to the next cylinder when the command byte EOT is found.
   * To do this, we will read 8 sectors at different points until we see that the read crossed to the
   *  next cylinder.  We will then read a single sector from that start value until the return CHS
   *  address equals the same Command CHS address.  At that point, we have read a non-existant
   *  sector.
   * If the command byte EOT is the trigger (and it is less than or equal to the physical EOT value),
   *  the return will have a CHS value of (cyl + 1)/0/1.
   * This indicates that we (most likely) did not read past End of Track, with the case that
   *  we actually read up to the last sector.  This is why we read in multiple places.
   * Note: The contoller will increment to the next cylinder, first sector of that cylinder, at end
   *  of track as long as the Multi-Track bit is clear in the command byte.
   * If the command byte EOT is greater than the count of sectors per track, when the controller tries
   *  to read the first non-existant track, it will return an error.
   * Therefore, if we have 9 SPT (physically) and set the command byte EOT to 10, the controller will
   *  try to read the 10th sector (instead of moving to the next cylinder) and fail.
   *
   * This all works because the controller takes the command byte EOT as literal end of track.  When
   *  it is done reading from a sector, if that sector value == EOT, it will increment the Cylinder
   *  count, and start at the first sector of the next track. (or at least it thinks it does.)
   * If the EOT value is greater than the physical EOT, it will try to read a non-existant sector
   *  and return an error.
   */
  
  // allocate DOS memory for the read (8 sectors worth)
  //  We will actually allocate 16 sectors worth so that if the first
  //  address would cross a 64k boundary, we can use the second half.
  if (__dpmi_allocate_dos_memory((512 * 16) / 16, &sel) == -1) {
    printf("Error allocating DOS memory\n");
    return FALSE;
  }
  __dpmi_get_segment_base_address(sel, &phy_address);
  
  // check to see we don't cross a 64k boundary, and move to the
  //  second half if we do.  (We can modify phy_address since we
  //  use 'sel' to free the memory and not 'phy_address')
  if (((phy_address + (512 * 8)) & 0xFFFF) < (512 * 8)) {
    printf("Physical Address crosses a 64k boundary (0x%08X).\n"
           "Moving to: 0x%08X\n", phy_address, (phy_address + (8 * 512)));
    phy_address += (8 * 512);
  }
  
  for (start = 6; start < 21; start += 3) {
    // Read 8 sectors from (C = 0, H = 0, S = Start)
    dma_init_dma(DMA_CMD_READ, phy_address, (512 * 8)); // Setup the DMA for transfering data
    buf[0] = FDC_MFM | FDC_CMD_READ;
    buf[1] = (0 << 2) | fdd->drv;
    buf[2] = 0;  // C
    buf[3] = 0;  // H
    buf[4] = (bit8u) start;
    buf[5] = 2;  // size
    buf[6] = (bit8u) (start + 8 - 1); // (eot)
    buf[7] = gpl;   // Gap Length
    buf[8] = 0xFF;  // DTL
    if (fdc_command(fdd->cntrlr, buf, 9, FALSE)) {
      if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
        if (fdc_result(fdd->cntrlr, 7, buf) == 7) {
          // if ST0 == 000000xx and return cylinder now on cylinder 1, we did not cross a cylinder boundary
          if (((buf[0] & ~0x3) == 0) && (buf[3] == 1) && (buf[5] == 1))
            continue;
          // else, we crossed a boundary so start reading from 'start', a sector at
          //  a time until return CHS == command CHS. (i.e: return isn't 1/0/1)
          break;
        }
      }
    }
    // error reading sectors, so free the memory we used
    __dpmi_free_dos_memory(sel);
    
    // and return FALSE
    return FALSE;
  }
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);

  // Recalibrate (Make sure we are at Track Zero)
  buf[0] = FDC_CMD_RECAL;
  buf[1] = fdd->drv;
  fdc_command_int(fdd, buf, 2, NULL, NULL, NULL);
  
  // if start == 21, we didn't cross a boundary
  if (start < 21) {
    for (i=0; i<8; i++) {
      // Read 1 sector from (C = 0, H = 0, S = (Start + i))
      dma_init_dma(DMA_CMD_READ, phy_address, 512); // Setup the DMA for transfering data
      buf[0] = FDC_MFM | FDC_CMD_READ;
      buf[1] = (0 << 2) | fdd->drv;
      buf[2] = 0;  // C
      buf[3] = 0;  // H
      buf[4] = (bit8u) (start + i);
      buf[5] = 2;  // size
      buf[6] = (bit8u) (start + i); // (eot)
      buf[7] = gpl;   // Gap Length
      buf[8] = 0xFF;  // DTL
      if (fdc_command(fdd->cntrlr, buf, 9, FALSE)) {
        if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
          if (fdc_result(fdd->cntrlr, 7, buf) == 7) {
            // if ST0 == 000000xx and return cylinder is now on cylinder 1 (1/0/1), we didn't try to read the non-existant sector
            if (((buf[0] & ~0x3) == 0) && (buf[3] == 1) && (buf[5] == 1))
              continue;
            if ((buf[3] == 0) && (buf[5] == (bit8u) (start + i))) {
              if (i == 0) {
                // we didn't even read the first sector (start) succesfully
                spt = 0;
                printf("Found zero SPT...\n");
              } else
                // else, we tried to read the non-existant sector, so (start + i - 1) is last sector of cylinder.
                spt = (start + i - 1);
              break;
            }
          }
        }
      }

      // error reading sectors, so free the memory we used
      __dpmi_free_dos_memory(sel);
      
      // and return FALSE
      return FALSE;
    }
  } else {
    // read more than 21 sectors with out error...
    //  (could be a 2.88 sector disk)
    //
  }
  
  /*
   * We now have the count of heads & the count of SPT, all we need now is the count of cylinders.
   * Since it can only be 40 or 80, we can read from any cylinder after the 40th and if successful,
   *  we have an 80 cylinder disk.
   * We will read from cylinder 39 just to be sure we have 40, then 79 to see if we have 80.
   * As a note, if we found only 1 head above, we can assume 40 cylinders and quite right now, however,
   *  we will not and say we did.
   */
  for (start = 39; start < 80; start += 40) {
    // if we don't have an enhanced controller, we have to seek to the cylinder
    if (!fdd->cntrlr->enhanced) {
      buf[0] = FDC_CMD_SEEK;
      buf[1] = (0 << 2) | fdd->drv;
      buf[2] = (bit8u) start;
      fdc_command_int(fdd, buf, 3, &ret_cnt, &status, &cur_cyl);
      if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != (bit8u) start))
        break;
      
      // wait for the head to settle
      mdelay(FDC_SLT_AFTER_SEEK);
    }
    
    dma_init_dma(DMA_CMD_READ, phy_address, 512); // Setup the DMA for transfering data
    buf[0] = FDC_MFM | FDC_CMD_READ;
    buf[1] = (0 << 2) | fdd->drv;
    buf[2] = (bit8u) start; // C
    buf[3] = 0;    // H
    buf[4] = 1;    // S
    buf[5] = 2;    // size
    buf[6] = spt;  // (eot)
    buf[7] = gpl;  // Gap Length
    buf[8] = 0xFF; // DTL
    if (fdc_command(fdd->cntrlr, buf, 9, FALSE)) {
      if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
        if (fdc_result(fdd->cntrlr, 7, buf) == 7) {
          // if ST0 == 000000xx and return cylinder is still on 'start', then good read
          if (((buf[0] & ~0x3) == 0) && (buf[3] == (bit8u) start) && (buf[5] == 2))
            cyls = start + 1;
          else
            break;
        }
      }
    }
  }
  
  // free the memory we used
  __dpmi_free_dos_memory(sel);
  
  // run through the floppy_media[] and see if we find a match.
  //  if so, copy that floppy_media[] to fdd (without overwriting stuff) and return TRUE.
  //  if not, return FALSE.
  for (i=1; i<MEDIA_COUNT; i++) {
    if ((floppy_media[i].cylinders == cyls) &&
        (floppy_media[i].heads == heads) &&
        (floppy_media[i].sect_trk == spt)) {
      memcpy(&fdd->type, &floppy_media[i].type, S_FLPY_SIZE_TO_CPY);
      ret = TRUE;
      break;
    }
  }
  
  return ret;
}

bool fdd_recalibrate(struct S_FLOPPY *fdd) {
  bit8u buf[32], ret_cnt, status, cur_cyl;
  
  // turn motor on
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_NO_WAIT);
  
  // recalibrate drive to cylinder 0, head 0.
  buf[0] = FDC_CMD_RECAL;
  buf[1] = fdd->drv;
  if (fdc_command_int(fdd, buf, 2, &ret_cnt, &status, &cur_cyl)) {
    if ((ret_cnt == 2) && ((status & 0x70) == 0x20) && (cur_cyl == 0))
      return TRUE;
  }
  
  return FALSE;
}

bool fdc_configure(struct S_FLOPPY_CNTRLR *fdc, const bit8u flags, const bit8u fifo_size) {
  bit8u buf[32];
  
  // Try the configure command.
  buf[0] = FDC_CMD_CONFIGURE;
  buf[1] = 0;
  buf[2] = (flags & ~0x0F) | ((fifo_size - 1) & 0x0F);
  buf[3] = 0; // pre-compensation from track 0 upwards
  return fdc_command(fdc, buf, 4, TRUE);
}

/*
 * check to see if the drive can "automatically seek" before a read and write.
 *  If it can, it is an enhanced controller and we don't have to seek before
 *  the read and writes.
 * To do so, make sure the head is on a cylinder other than a specified cylinder.
 *  then read (verify instead?) a sector from that specified cylinder.  Now sence
 *  where the heads are.  If they are now on the specified cylinder, then we
 * have an enhanced controller *and* we don't have to seek before reads/writes.
 */
#define TEST_CYL  20
bool fdc_detect_implied_seek(struct S_FLOPPY *fdd) {
  bit8u buffer[512];
  
  // first send the configure (enchanced) command, enabling Implied Seek.
  fdc_configure(fdd->cntrlr, FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16);
  
  // seek to cylinder 0 (lba 0 is on cyl 0)
  if (!fdd_seek_to_track(fdd, 0))
    return FALSE;
  
  // Now read in a sector on a different cylinder, cylinder TEST_CYL (zero based)
  //  to see if the controller seeked for us.
  if (!fdd_read_sectors(fdd, fdd->sect_trk * fdd->heads * TEST_CYL, 1, buffer))
    return FALSE;
  
  // now get the cylinder we are currently on.
  // if it matches the cylinder that the above read sector is on,
  //  then we have an enhanced controller.
  struct S_BLOCK_STATUS status;
  fdd_get_status(fdd, &status);
  
  return (status.cur_cyl == TEST_CYL);
}

bool fdd_init_controller(struct S_FLOPPY_CNTRLR *fdc) {
  bit8u buf[16];
  
  // reset the controller
  outpb(fdc->base + FDC_DOR, 0x00);  // reset
  mdelay(2);                         // hold for 2 milliseconds
  outpb(fdc->base + FDC_DOR, 0x0C);  // release
  mdelay(2);                         // hold for 2 milliseconds
  
  // wait for the interrupt, then sense an interrupt on all four drives
  // Page 41 of the FDC 82077AA specs
  if (fdc_wait_int(FDC_TIMEOUT_INT)) {
    for (int i=0; i<4; i++) {
      buf[0] = FDC_CMD_SENSE_INT;
      fdc_command(fdc, buf, 1, FALSE);
      bit8u count = fdc_result(fdc, 2, buf);
      if ((count != 2) || ((buf[0] & 0x3) != i) || (buf[1] != 0x00))
        return FALSE;
    }
  } else {
    // TODO: do we need to have a counter here so that we don't loop indefinately?
    fdd_init_controller(fdc);
    return FALSE;
  }
  
  // Specify command
  buf[0] = FDC_CMD_SPECIFY;
  buf[1] = 0xAF; // until we know what disk is in the drive, just use 0xAF
  buf[2] = (2 << 1) | 0;
  fdc_command(fdc, buf, 3, TRUE);
  
  // try to configure it
  //if (fdc->enhanced)
    fdc_configure(fdc, FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16);
  
  return TRUE;
}

void fdc_motor_cntr(struct S_FLOPPY *fdd, const bool on, const bool wait) {
  
  // the FDC_TYPE_765A/B's DOR register is read only, so we can't tell if the drive was on or not
  // therefore simulate a read of 0x00
  bit8u dor = ((fdd->cntrlr->type == FDC_TYPE_765A) || (fdd->cntrlr->type == FDC_TYPE_765B)) ? 0 : inpb(fdd->cntrlr->base + FDC_DOR);
  
  const bool was_off = (dor & (0x10 << fdd->drv)) ? FALSE: TRUE;
  if (on) {
    if (was_off) {
      outpb(fdd->cntrlr->base + FDC_DOR, ((dor & 0xF0) | ((0x10 << fdd->drv) | 0x0C | fdd->drv)));
      if (wait)
        mdelay(FDC_SPINUP);
    }
  } else
    outpb(fdd->cntrlr->base + FDC_DOR, 0x0C | (dor & ~(0x10 << fdd->drv)));
}

#define FLOPPY_IRQ_VECT  0x0E
#define TIMER_IRQ_VECT   0x08

void floppy_irq(void) {
  fdc_drv_stats = TRUE;
  outpb(0x20, 0x20); // EOI
}

/*
 * We have to set the BIOS data area so that the BIOS (real mode timer interrupt) doesn't
 *  turn off the motors while we are trying to read from them.
 * Once we exit from this app, if the motor is still turning, the BIOS will now count down
 *  the status and turn it off for us.
 * This is only because of the DOS DMPI that we are using.  Remember, you will not have
 *  to interface with DOS when you write this code for your driver.
 * As I was writing this code, as a utility under DOS DPMI, I could not figure out why
 *  the motors were turning off prematurally.  Come to find out, the DPMI tests for the
 *  drives even though we did not set a bit in the BIOS area.  It must do this for us...
 * Therefore, I set the timeout to the max 18.2 times a second, just so the BIOS won't
 *  turn off the motors for us until we are done.
 */
void timer_irq(void) {
  bit8u zz = 0x00;
  bit8u ff = 0xFF;
  
  dosmemput(&zz, 1, 0x043F);
  dosmemput(&ff, 1, 0x0440);
}

_go32_dpmi_seginfo fdc_old, fdc_new, timer_old, timer_new;

void init_ext_int() {
  
  // Floppy setup
  // get the old handler info
  _go32_dpmi_get_protected_mode_interrupt_vector(FLOPPY_IRQ_VECT, &fdc_old);
  
  // set new handler info
  fdc_new.pm_offset = (bit32u) floppy_irq;
  fdc_new.pm_selector = _go32_my_cs();
  
  _go32_dpmi_allocate_iret_wrapper(&fdc_new);
  _go32_dpmi_set_protected_mode_interrupt_vector(FLOPPY_IRQ_VECT, &fdc_new);

  // Timer setup
  // get the old handler info
  _go32_dpmi_get_protected_mode_interrupt_vector(TIMER_IRQ_VECT, &timer_old);

  // set new handler info
  timer_new.pm_offset = (bit32u) timer_irq;
  timer_new.pm_selector = _go32_my_cs();
  
  _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER_IRQ_VECT, &timer_new);
}

void exit_ext_int() {
  
  // floppy free
  _go32_dpmi_set_protected_mode_interrupt_vector(FLOPPY_IRQ_VECT, &fdc_old);
  _go32_dpmi_free_iret_wrapper(&fdc_new);
  
  // timer free
  _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_IRQ_VECT, &timer_old);
}

// wait for completion interrupt
//  returns FALSE if takes more than given time
bool fdc_wait_int(int timeout) {
  while (timeout) {
    if (fdc_drv_stats) {
      fdc_drv_stats = FALSE;
      return TRUE;
    }
    mdelay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

// checks to see if the controller is expecting more input
// waits 100ms and returns FALSE if not
bool fdc_want_more(struct S_FLOPPY_CNTRLR *fdc) {
  int timeout = 100;
  while (timeout) {
    if ((inpb(fdc->base + FDC_MSR) & 0xC0) == 0x80)
      return TRUE;
    mdelay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

// waits for controller to be ready for a write
// if it isn't ready after 1000ms, it returns FALSE (timeout)
//  else it writes the value, and returns TRUE
bool write_fdc(struct S_FLOPPY_CNTRLR *fdc, const bit8u val) {
  int timeout = 1000;
  while (timeout) {
    if ((inpb(fdc->base + FDC_MSR) & 0xC0) == 0x80) {
      outpb(fdc->base + FDC_CSR, val);      
      return TRUE;
    }
    mdelay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

bool fdc_command_int(struct S_FLOPPY *fdd, const bit8u *buf, bit8u cnt, bit8u *ret_cnt, bit8u *status, bit8u *cur_cyl) {
  bit8u r, int_ret[16];
  
  if (fdc_command(fdd->cntrlr, buf, cnt, TRUE)) {
    if (fdc_wait_int(FDC_TIMEOUT_INT)) {
      int_ret[0] = FDC_CMD_SENSE_INT;
      if (fdc_command(fdd->cntrlr, int_ret, 1, FALSE)) {
        r = fdc_result(fdd->cntrlr, 2, int_ret);
        if (ret_cnt) *ret_cnt = r;
        if (status) *status = int_ret[0];
        if (cur_cyl) *cur_cyl = int_ret[1];
        return TRUE;
      }
    }
  }
  fdd_init_controller(fdd->cntrlr);
  fdd_recalibrate(fdd);
  return FALSE;
}

// this is a command
// will return TRUE if command supported, FALSE if not (or other error)
// noresult = 1 indicates that we are not expecting a result phase
bool fdc_command(const struct S_FLOPPY_CNTRLR *fdc, const bit8u *buf, bit8u cnt, bool noresult) {
  
  // clear the interrupt flag
  fdc_drv_stats = FALSE;
  
  bit8u cur = 0, msr, byte;
  int timeout = FDC_TIMEOUT_CNT;
  bool ret = FALSE;
  
  // if we are already in a command, we can't send another
  msr = inpb(fdc->base+FDC_MSR);
  if (msr & FDC_MSR_CBZY) {
    printf("FDC already in a command...\n");
    //fdd_init_controller(fdc);
    return FALSE;
  }
  
  //printf("FDC_COMMAND: %02X", buf[0]);
  while (timeout) {
    msr = inpb(fdc->base+FDC_MSR);
    if ((msr & (FDC_MSR_RQM | FDC_MSR_DIO)) == FDC_MSR_RQM) {
      outpb(fdc->base+FDC_CSR, buf[cur++]);
      if (cur == cnt) {
        ret = TRUE;
        break;
      }
    }
    mdelay(1);
    timeout--;
  }
  
  // if we are not expecting a result phase and we only sent 1 command byte,
  //  or we sent less than expected, check to see if the controller went to
  //  the result phase to return 0x80.
  // if we are expecting a result phase, we *can not* read in the byte here
  //  or the result phase will be out of phase by one byte. If this command
  //  is expecting a result phase, is invalid, and returns 0x80, we let the 
  //  result phase code check for this.
  if ((noresult && (cur == 1)) || (cur < cnt)) {
    msr = inpb(fdc->base+FDC_MSR);
    if ((msr & (FDC_MSR_RQM | FDC_MSR_DIO)) == (FDC_MSR_RQM | FDC_MSR_DIO)) {
      byte = inpb(fdc->base+FDC_CSR);
      ret = FALSE;
      //printf("fdc_command: read = 0x%02X (illegal command?)\n", byte);
    }
  }
  
  // if the command doesn't expect a result, then wait for the BZY bit to clear
  if (!ret || noresult) {
    timeout = FDC_TIMEOUT_CNT;
    while ((inpb(fdc->base+FDC_MSR) & FDC_MSR_CBZY) && timeout--)
      mdelay(1);
    if (inpb(fdc->base+FDC_MSR) & FDC_MSR_CBZY) {
      printf("Command: Still busy\n");
      //fdd_init_controller(fdc);
    }
  }
  
  return ret;
}

// The CBZY bit will be set as long as there is a byte to read.
//  The command is not completed until this bit is zero.
// returns 0 if error (command not supported?)
bit8u fdc_result(const struct S_FLOPPY_CNTRLR *fdc, const bit8u cnt, bit8u *buf) {
  bit8u cur = 0, msr, byte;
  int timeout = FDC_TIMEOUT_CNT;

  while (timeout) {
    // even if we read in the desired count of bytes, we continue to check
    //  if there are more, so that the controller finishes the command.
    msr = inpb(fdc->base+FDC_MSR);
    // if there is data available as a read, read in the byte
    if ((msr & FDC_MSR_CBZY) &&
       ((msr & (FDC_MSR_RQM | FDC_MSR_DIO)) == (FDC_MSR_RQM | FDC_MSR_DIO))) {
      byte = inpb(fdc->base+FDC_CSR);
      // if this is the first byte and it is 0x80,
      //  then there was an error (command not supported?).
      if ((cur == 0) && (byte == 0x80))
        break;
      // don't read in more than the caller expects
      if (cur < cnt)
        buf[cur++] = byte;
    } else {
      mdelay(1);
      timeout--;
    }
  }

  // wait for the BZY bit to clear
  timeout = FDC_TIMEOUT_CNT;
  while ((inpb(fdc->base+FDC_MSR) & FDC_MSR_CBZY) && timeout--)
    mdelay(1);
  if (inpb(fdc->base+FDC_MSR) & FDC_MSR_CBZY)
    printf("Result: Still busy\n");
  
  // return the count of result bytes (could be zero to return an error)
  return cur;
}

/* 
 * Sector   = (LBA mod SPT) + 1       (SPT = Sectors per Track)
 * Head     = (LBA  /  SPT) mod Heads
 * Cylinder = (LBA  /  SPT)  /  Heads
 */
void fdd_lba_to_chs(int lba, int spt, int heads, bit8u *cyl, bit8u *head, bit8u *sector) {
  *sector = (bit8u) ((lba % spt) + 1);
  *head   = (bit8u) ((lba / spt) % heads);
  *cyl    = (bit8u) ((lba / spt) / heads);
}

/*
 * Can only be called *after* we detect the media type
 */
bool fdd_seek_to_track(struct S_FLOPPY *fdd, int lba) {
  bit8u cyl, head, sector, new_cyl;
  bit8u buf[16];
  
  //  convert lba to chs 
  fdd_lba_to_chs(lba, fdd->sect_trk, fdd->heads, &cyl, &head, &sector);
  
  // turn on the motor
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_WAIT);
  
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (head << 2) | fdd->drv;
  buf[2] = cyl;
  if (!fdc_command_int(fdd, buf, 3, NULL, NULL, &new_cyl)) {
    fdd->cur_cylinder = 0xFF; // force recal next time
    return FALSE;
  }
  
  // wait for the head to settle
  mdelay(FDC_SLT_AFTER_SEEK);
  return TRUE;
}

/*
 * Can only be called *after* we detect the media type
 */
bool fdd_read_sectors(struct S_FLOPPY *fdd, int lba, bit32u cnt, bit8u *buf) {
  bit8u cyl, head, sector;
  unsigned count = 0, i;
  int sel;
  bit32u phy_address;
  bool ret = FALSE;
  
  // make sure that a disk is in the drive
  struct S_BLOCK_STATUS status;
  if (!fdd_inserted(fdd, &status))
    return FALSE;
  
  // allocate at most a full cylinder
  count = fdd->sect_trk * fdd->heads;
  if (__dpmi_allocate_dos_memory((512 * count) / 16, &sel) == -1) {
    printf("Error allocating DOS memory\n");
    return FALSE;
  }
  __dpmi_get_segment_base_address(sel, &phy_address);
  
  // do the transfer
  while (cnt) {
    fdd_lba_to_chs(lba, fdd->sect_trk, fdd->heads, &cyl, &head, &sector); //  convert lba to chs 
    // we can read up to the EOT, or if two sided and enhanced controller, we can read up to
    //  EOT on the second head, before we have to seek to next cylinder
    count = (fdd->sect_trk - (sector - 1));
    if (fdd->cntrlr->enhanced && (fdd->heads > 1))
      count += (((fdd->heads - 1) - head) * fdd->sect_trk);
    if (count > cnt) count = cnt;
    for (i=0; i<3 && !ret; i++) {
      dma_init_dma(DMA_CMD_READ, phy_address, count * 512); // Setup the DMA for transfering data
      buf[0] = (fdd->cntrlr->enhanced ? FDC_MT : 0) | (fdd->densf ? FDC_MFM : 0) | FDC_CMD_READ;
      buf[1] = (head << 2) | fdd->drv;
      buf[2] = cyl;
      buf[3] = head;
      buf[4] = sector;
      buf[5] = 2;    // size
      buf[6] = fdd->sect_trk;
      buf[7] = fdd->gaplen;
      buf[8] = 0xFF;
      if (fdc_command(fdd->cntrlr, buf, 9, FALSE)) {
        if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
          if (fdc_result(fdd->cntrlr, 7, buf) == 7) {
            ret = ((buf[0] & ~0x3) == 0);
            break;
          }
        }
      }
    }
    
    if (ret) {
      dosmemget(phy_address, (512 * count), buf);
      buf += (512 * count);
      lba += count;
      cnt -= count;
    } else
      break;
  }
  
  __dpmi_free_dos_memory(sel);
  return ret;
}

/*
 * return TRUE if a floppy is inserted.
 * can call before we detect media type as long as
 *  fdd->cntrlr->base and fdd->drv are valid values
 */
bool fdd_inserted(struct S_FLOPPY *fdd, struct S_BLOCK_STATUS *status) {
  bit8u buf[16];
  
  /* The way you check the change line is to select the drive and turn on
   * the motor for that drive.  Then you test the changeline and if on, you have
   * to cause the drive to seek to another track and then back to track zero.
   * Then you test the changeline again and if it is still on you have an empty
   * floppy drive.  If off, the media was changed or the same floppy was removed
   * and reinserted.
   */ 
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_NO_WAIT);  // turn on motor, no wait
  if (inpb(fdd->cntrlr->base + FDC_DIR) & FDC_DIR_CHNG_LINE) {
    // seek to track 1
    buf[0] = FDC_CMD_SEEK;
    buf[1] = (0 << 2) | fdd->drv;
    buf[2] = 1;
    fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
    
    // wait for the head to settle
    mdelay(FDC_SLT_AFTER_SEEK);
    
    // seek to track 0
    buf[2] = 0;
    fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
    
    // wait for the head to settle
    mdelay(FDC_SLT_AFTER_SEEK);
    
    status->inserted = ((inpb(fdd->cntrlr->base + FDC_DIR) & FDC_DIR_CHNG_LINE) == 0);
    status->changed = TRUE;
  } else {
    status->inserted = TRUE;
    status->changed = FALSE;
  }
  
  return status->inserted;
}

/*
 * Can only be called *after* we detect the media type
 */
void fdd_get_status(struct S_FLOPPY *fdd, struct S_BLOCK_STATUS *status) {
  bit8u buf[16];
  
  // get current cyl, head, sector, and sector size
  // must do before "inserted" check, that check moves the arm (seeks).
  if (!fdc_get_cur_pos(fdd, 0, &status->cur_cyl, &status->cur_head, &status->cur_sector, &status->sector_size)) {
    status->cur_cyl = 
    status->cur_head = 
    status->cur_sector = 
    status->sector_size = 0xFF;
  }
  
  // get inserted status
  fdd_inserted(fdd, status);
  
  // make sure motor is running
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_WAIT);
  
  // Bit 6 of ST3 - Indicates the status of the WP (Write Protect) pin.
  // (Use Sense Drive Status command to get the ST3 register).
  // fdc command 04h will return bit 6=write protected disk
  buf[0] = FDC_CMD_STATUS;
  buf[1] = (0 << 2) | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2, FALSE);
  fdc_result(fdd->cntrlr, 1, buf);
  status->write_prot = ((buf[0] & 0x40) == 0x40);
}

/*
 * Can only be called *after* we detect the media type
 */
bool fdc_get_cur_pos(struct S_FLOPPY *fdd, const bit8u hs, bit32u *cyl, bit32u *head, bit32u *sect, bit32u *sect_size) {
  bit8u buf[16];
  
  fdc_motor_cntr(fdd, MOTOR_ON, MOTOR_WAIT);
  
  buf[0] = (fdd->densf ? FDC_MFM : 0) | FDC_CMD_READ_ID;
  buf[1] = (hs << 2) | fdd->drv;
  // we don't call fdc_command_int() here because we don't want to sense interrupt
  if (fdc_command(fdd->cntrlr, buf, 2, FALSE)) {
    if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
      if (fdc_result(fdd->cntrlr, 7, buf) == 7) {
        if ((buf[0] & 0x40) == 0) {
          if (cyl) *cyl = buf[3];
          if (head) *head = buf[4];
          if (sect) *sect = buf[5];
          if (sect_size) *sect_size = buf[6];
          return TRUE;
        }
      }
    }
  }
  
  return FALSE;
}

// setup the dma for a floppy transfer
void dma_init_dma(const bit8u dmacmd, const bit32u address, bit16u size) {
  
  // mask this channel for no interruptions
  outpb(DMA_MASK_REG, (1<<2) | CHANNEL_FDC);  // DMA-1 Mask Register Bit Port (8-bit)
  
  //  DMA-1 Clear Byte Flip-Flop (write anything)
  outpb(DMA_FLIP_FLOP, 0xFF);
  
  // low 16-bits of address
  outpb(0x04, (bit8u) ((address >>  0) & 0xFF)); // DMA-1 Channel 2 Output (low byte)
  outpb(0x04, (bit8u) ((address >>  8) & 0xFF)); // DMA-1 Channel 2 Output (high byte)
  
  //  DMA-1 Clear Byte Flip-Flop (write anything)
  outpb(DMA_FLIP_FLOP, 0xFF);
  
  // size of transfer
  size--;  // size is always 1 less from sent size
  outpb(0x05, (bit8u) ((size    >>  0) & 0xFF)); // DMA-1 Channel 2 Output size (low byte)
  outpb(0x05, (bit8u) ((size    >>  8) & 0xFF)); // DMA-1 Channel 2 Output size (high byte)
  
  // page register (bits 23-16 of address)
  outpb(0x81, (bit8u) ((address >> 16) & 0xFF)); // DMA-1 Channel 2 Output (page value)
  
  // write command code
  outpb(DMA_MODE_REG, dmacmd | CHANNEL_FDC);     // Set DMA-1 Mode Register port
  
  // select (unmask the channel)
  outpb(DMA_MASK_REG, CHANNEL_FDC);              // DMA-1 Mask Register Bit Port (8-bit)
}

// simply parses the command line parameters for specific values
bool get_parameters(int argc, char *argv[]) {
  
  for (int i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "v") == 0)
        verbose = TRUE;
      else {
        printf("Unknown parameter: %s\n", argv[i]);
        return FALSE;
      }
    } else {
      printf("Unknown parameter: %s\n", argv[i]);
      return FALSE;
    }
  }
  
  return TRUE;
}

/*
// Used if you plan to debug your code
char int_code_str[4][32] = { "Normal Termination", "Abnormal Termination", "Invalid Command", "Abnormal due to polling" };

void printf_st0(const bit8u st0) {
  printf("  ST0 = %02X:  bit 7:6 = %i%i (Interrupt code: %s)\n"
         "               bit 5 = %i (Seek End)\n"
         "               bit 4 = %i (Equip Check)\n"
         "               bit 3 = %i (should be 0)\n"
         "               bit 2 = %i (head)\n"
         "            bits 1:0 = %i (drive)\n",
         st0,
         (st0 & (1<<7)) >> 7, (st0 & (1<<6)) >> 6, int_code_str[st0 >> 6],
         (st0 & (1<<5)) > 0, (st0 & (1<<4)) > 0,
         (st0 & (1<<3)) > 0, (st0 & (1<<2)) > 0,
         (st0 & (3<<0))
  );
};

void printf_st1(const bit8u st1) {
  printf("  ST1 = %02X:  bit 7 = %i (End of Cyl)\n"
         "             bit 6 = %i (should be 0)\n"
         "             bit 5 = %i (Data Error)\n"
         "             bit 4 = %i (Over/Under)\n"
         "             bit 3 = %i (should be 0)\n"
         "             bit 2 = %i (No Data)\n"
         "             bit 1 = %i (Not Writable)\n"
         "             bit 0 = %i (Missing Addr Mark)\n",
         st1,
         (st1 & (1<<7)) > 0, (st1 & (1<<6)) > 0, (st1 & (1<<5)) > 0,
         (st1 & (1<<4)) > 0, (st1 & (1<<3)) > 0, (st1 & (1<<2)) > 0,
         (st1 & (1<<1)) > 0, (st1 & (1<<0)) > 0
  );
};

void printf_st2(const bit8u st2) {
  printf("  ST2 = %02X:  bit 7 = %i (should be 0)\n"
         "             bit 6 = %i (Control Mark)\n"
         "             bit 5 = %i (Data Error)\n"
         "             bit 4 = %i (Wrong Cylinder)\n"
         "             bit 3 = %i (should be 0)\n"
         "             bit 2 = %i (should be 0)\n"
         "             bit 1 = %i (Bad Cyl)\n"
         "             bit 0 = %i (Missing Addr Mark)\n",
         st2,
         (st2 & (1<<7)) > 0, (st2 & (1<<6)) > 0, (st2 & (1<<5)) > 0,
         (st2 & (1<<4)) > 0, (st2 & (1<<3)) > 0, (st2 & (1<<2)) > 0,
         (st2 & (1<<1)) > 0, (st2 & (1<<0)) > 0
  );
};

void printf_st3(const bit8u st3) {
  printf("  ST3 = %02X:  bit 7 = %i (should be 0)\n"
         "           bit 6 = %i (WP)\n"
         "           bit 5 = %i (should be 1)\n"
         "           bit 4 = %i (Track 0)\n"
         "           bit 3 = %i (should be 1)\n"
         "           bit 2 = %i (head)\n"
         "        bits 1:0 = %i (drive)\n",
         st3,
         (st3 & (1<<7)) > 0, (st3 & (1<<6)) > 0, (st3 & (1<<5)) > 0,
         (st3 & (1<<4)) > 0, (st3 & (1<<3)) > 0, (st3 & (1<<2)) > 0,
         (st3 & (3<<0))
  );
};
*/
