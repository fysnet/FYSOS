/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Media Storage Devices, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  compile using gcc (DJGPP)
 *   gcc -Os fdc_type.cpp -o fdc_type.exe -s
 *
 *  usage:
 *    fdc_type [-v]
 *
 *    -v indicates verbose output
 *
 *  Notes:
 *   The 765A does not like us reading from the Data Register (+ 05) if there
 *    is no data to be read.  A controller reset must take place before we can
 *    trust any other data.
 *
 *   This code is written for DOS using a DPMI.  This is soley for the memory
 *    allocation.  Since we do not allocation more than 4096 bytes, it could
 *    easily be written so that it does not need the DOS DPMI.  However, the
 *    current Interrupt Handling would need to be modified also.
 *
 *   It is not my intention to show how to interact with the Host Operating
 *    System, when using this utility as is.  It is my intent to show how you
 *    can code you driver, leaving the memory allocation and interrupt handling
 *    to you.
 *
 *   I simply chose DOS and a DPMI since that is the usualy platform I test with.
 *
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

#include "fdc_type.h"

// lock all memory, to prevent it being swapped or paged out
int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY;

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
  { 0, FLOPPY_MEDIA_160,   320, 40,1, 8, 512, 2,15,0,13, 0, 0,0, 2, 42,255, 84, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  160k  (0xFE)
  { 0, FLOPPY_MEDIA_180,   360, 40,1, 9, 512, 2,15,0,13, 0, 0,0, 2, 42,255, 84, 0, FDC_250KBPS, 0,0, 0xFF, 0x00 },  //  180k  (0xFC)
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
      outp(s_fdc[j].base + FDC_DOR, 0x00);  // reset drive
      delay(2);                             // hold for 2 milliseconds
      outp(s_fdc[j].base + FDC_DOR, 0x0C);  // release reset
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
  
  // initial reading should not be 0xFF
  if (inp(fdc->base + FDC_MSR) == 0xFF)
    return FALSE;
  
  // the minimum duration of the reset should be 500ns.
  // A write duration to an ISA I/O port takes longer than that.
  // Therefore, if we write it twice, we will be sure to be long enough.
  outp(fdc->base + FDC_DOR, 0x00); // reset drive
  outp(fdc->base + FDC_DOR, 0x00);
  
  outp(fdc->base + FDC_DOR, 0x04); // release reset
  
  // If FDC_MSR:RQM set and FDC_MSR:CB clear before 200 ms,
  //  then controller found
  delayms = clock() + 200;
  while (delayms > clock()) {
    if (inp(fdc->base + FDC_MSR) == 0x80)
      break;
  }
  
  // if the controller isn't read for input after the delay, return FALSE
  if (inp(fdc->base + FDC_MSR) != 0x80)
    return FALSE;
  
  // get the initial register value from DIR
  //  to pass along to the fdc_get_type() function
  dir = inp(fdc->base + FDC_DIR);
  
  // now see if it will handle a command
  // send the specify command
  buf[0] = FDC_CMD_SPECIFY;
  buf[1] = 0xAF;
  buf[2] = 0x1E;
  if (!fdc_command(fdc, buf, 3))
    return FALSE;
  
  if (verbose)
    printf("Initial Register Values:\n"
           "    Status Register A: %02X\n"
           "    Status Register B: %02X\n"
           "       Digital Output: %02X\n"
           "           Tape Drive: %02X\n"
           " Main Status Register: %02X\n"
           " Data (FIFO) Register: %02X\n"
           "             Reserved: %02X\n"
           "        Digital Input: %02X\n",
      inp(fdc->base + 0), inp(fdc->base + 1), inp(fdc->base + 2), inp(fdc->base + 3),
      inp(fdc->base + 4), inp(fdc->base + 5), inp(fdc->base + 6), inp(fdc->base + 7));
  
  // if we get here, a controller was found
  // get the type of controller
  fdc->type = fdc_get_type(fdc, dir);
  
  return TRUE;
}

/* Get Controller Type
 *  I have checked and have specifications for the following controller types.
 *   These are in order of the tests below by Pass/Fail.
 *       Name        Product Name and Manufacture  (Enhanced)  Test:  1 2 3 4  (F = Fail, P = Pass)
 *   - PC87306       (National Semiconductor)         Y               P P - -
 *   - 82078         (Intel 82078 44-pin)             Y               P P - -
 *   - 82078SL       (Intel 82078 64-pin)             Y               P P - -
 *   - 82077         (Intel 82077AA)                  Y               P F P F
 *   - PC8477B       (National Semiconductor)         Y               P F P P
 *   - 37C78         (SMSC FDC37C78)                  Y               P F P P
 *   - W83977        (W83977(a)F Winbond)             Y               P F P P
 *   - 82072         (Intel 82072)                    Y*              P F F -  (*Dump Regs passes, Version does not)
 *   - 765B          (NEC uPD765b)                    N               F P - -
 *   - DP8473        (National Semiconductor)         N               F F P -
 *   - 72065A        (NEC uPD72065A / NEC uPD72066)   N               F F F P
 *   - 765A and 765B (NEC uPD765a / NEC uPD765b)      N               F F F F 
 *   - 8272          (Intel 8272A)                    N               F F F F
 *  The code below will send specific commands to the controller, eliminating those
 *   that don't support that command, until only one is left, to determine what
 *   controller is present.
 *  This is from my own research and testing and is of the best of my knowledge.
 *   I do not have all of these controllers to test on.
 */
bit8u fdc_get_type(struct S_FLOPPY_CNTRLR *fdc, const bit8u dir) {
  bit8u r, buf[32];
  
  // Test 1: Dump Registers
  write_fdc(fdc, FDC_CMD_DUMP_REGS);
  r = fdc_return(fdc, 10, (bit8u *) &fdc->dump_regs);
  if ((r == 1) && (fdc->dump_regs.pcn0 == 0x80)) {
    fdc->enhanced = FALSE;
    
    // Test 2: Version
    // Dump Registers failed, so test for the Version Byte command
    // Of the controllers that fail on DumpRegisters, only the 765A/B
    //  has the GetVersion command
    // Please note, the 765A may not actually support the Version command, since
    //  an invalid command will return 0x80 anyway.  However, this is how you
    //  determine between a 765A and 765B.
    write_fdc(fdc, FDC_CMD_VERSION);
    r = fdc_return(fdc, 1, buf);
    if (r == 1) {
      if (buf[0] == 0x80) {
        // Test 3: Internal Track Number
        // the DP8473 has an Internal Track Number register that we can read/write
        buf[0] = (0 << 6) | FDC_CMD_SET_TRK; // read current internal track number
        buf[1] = 0x30 | (0 << 2) | 0;        // read LSB
        buf[2] = 0;                          // not used on the read
        if (!fdc_command(fdc, buf, 3) && (fdc_return(fdc, 1, buf) == 1))
          return FDC_TYPE_8272A;   // Type 8272A compatible FDC
        
        // Test 4: Exit Standby
        // the 72065A as the PowerControl/Standby commands
        write_fdc(fdc, FDC_CMD_EXIT_STND);
        r = fdc_return(fdc, 1, buf);
        // if it returned a single byte of 0x80, the command was not supported.
        // if no return, the command was supported.
        if (r == 0)
          return FDC_TYPE_72065A;
        
        // Now we should just have the 765A and 8272 left.
        // At this time, I don't know the difference between these two or how to
        //  detect them, so we will just return a 765A since it was probably a 765A
        //  returning 0x80 as either supporting the version command and returning a
        //  version of 0x80, or most likely, not supporting the command and returing
        //  ST0 = 0x80 (invalid command)
        return FDC_TYPE_765A;
      } else if (buf[0] == 0x90) {
        return FDC_TYPE_765B;
      } else {
        // We found a controller that doesn't pass on the DumpRegisters command,
        //  but does have the Version Command
        printf("Found Controller w/o Dump, but w/ Version... (0x%02X)", buf[0]);
        return FDC_TYPE_UNKNOWN;
      }
    }
  }
   
  // the Dump command is an enhanced command.
  fdc->enhanced = TRUE;
  
  // At this point, the dump command was supported and may or may not returned
  //  all 10 bytes
  if (r == 10)
    fdc->dump_valid = TRUE;
  else if (r > 0)
    printf("\nDumpregs: unexpected count of returned bytes: %i", r);
  
  // The remaining (known) controllers should all support the Configure command
  // We do this so that we can eliminate any unknown controllers, and get the
  //  implied_seek flag.
  if (fdc_configure(fdc, FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16)) {
    fdc->implied_seek = TRUE;
    
    // Test 2: PartID command
    // If the controller supports the PartID command, this will eliminate some
    //  controllers as well as give a version for those that do support it.
    // Try the PartID command
    write_fdc(fdc, FDC_CMD_PARTID);
    r = fdc_return(fdc, 1, buf);
    if ((r == 1) && (buf[0] != 0x80)) {
      fdc->partid_valid = TRUE;
      fdc->partid = buf[0];
      switch (buf[0]) {
        case 0:  // 000xxxx1
          return FDC_TYPE_82078SL;  // 64-pin 82078
        case 2:  // 010xxxx1
          return FDC_TYPE_44_82078; // 44-pin 82078
        case 3:  // 010xxxx1
          return FDC_TYPE_NS_PC87306; // 160-pin 82078 compatible
        default: // ???xxxx1
          return FDC_TYPE_82078;    // must be a 82078 compatible controller????
      }
    } else {
      fdc->partid_valid = FALSE;
      
      // Test 2 above eliminated all the 82072 compatibles, so we now have the
      //  PC8477B, 82072, 82077, 37C78, and W83977.
      
      // Test 3: Perp288 command
      // The 82072 does not know the PERP288 command
      if (write_fdc(fdc, FDC_CMD_PERP288) && fdc_want_more(fdc)) {
        write_fdc(fdc, 0x80);  // finish the command
        
        // controllers left: PC8477B, 82077, 37C78, and W83977.
        // The PC8477B is an enhanced version of the DP8473 which is
        //  compatible with the 82077AA.
        // The 37C78 is compatible with the 82077AA.
        // The W83977 contains an FDC compatible with the 82077 / 765
        //  but has the enhanced features of the 82077AA.
        
        // Test 4: Unlock
        // The original 82077 didn't like the unlock command
        write_fdc(fdc, FDC_CMD_UNLOCK);
        r = fdc_return(fdc, 1, buf);
        if ((r == 1) && (buf[0] != 0x80)) {
          // I have found the the W83977 has a DIR initial register value of 0x7F
          //  (all unused bits high), while the 82077AA has them low.
          if (dir == 0x7F)
            return FDC_TYPE_W83977;
          
          // We return an 82077AA for the PC8477B, later 82077(AA), and 37C78
          return FDC_TYPE_82077AA;
        } else {
          // Pre-1991 82077, doesn't know LOCK/UNLOCK
          return FDC_TYPE_82077_ORIG;
        }
      } else
        return FDC_TYPE_82072;
    }
  } else {
    fdc->enhanced = FALSE;
    
    // we found a controller that supports the DumpRegisters command, but not
    //  the configure command.  This should be rare...
    printf("Found Controller w/ Dump, but w/o Configure.");
    return FDC_TYPE_UNKNOWN;
  }
}

/* Calculate the size of the FIFO
 * [This is experimental. I have seen no documentation that this does not work.]
 * This is done by setting the max size with the configuration command,
 *  then reading it back using the dumpregs command until the two
 *  values match.
 */
int get_max_FIFO_size(struct S_FLOPPY_CNTRLR *fdc) {
  bit8u buf[16];
  int r, size = 15;
  
  while (size >= 0) {
    buf[0] = FDC_CMD_CONFIGURE;
    buf[1] = 0;     // zero
    buf[2] = 0x50 | (bit8u) size;  // EIS = TRUE, Disable Poll, Enable FIFO, FIFO = x bytes
    buf[3] = 0;     // pretrk = 0
    if (!fdc_command(fdc, buf, 4))
      return 0;
    
    write_fdc(fdc, FDC_CMD_DUMP_REGS);
    r = fdc_return(fdc, 10, buf);
    if ((r != 10) || (buf[0] == 0x80))
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
  delay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be set
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2);
  cnt = fdc_return(fdd->cntrlr, 1, &status);
  
  if ((cnt != 1) || ((status & 0x13) != (0x10 | fdd->drv)))
    return FALSE;
  
  // seek to track 5 (any present non-zero track works)
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 5;
  fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
  
  // wait for the head to settle
  delay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be clear
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2);
  cnt = fdc_return(fdd->cntrlr, 1, &status);
  if ((cnt != 1) || ((status & 0x13) != fdd->drv))
    return FALSE;
  
  // seek to track 0
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 0;
  fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
  
  // wait for the head to settle
  delay(FDC_SLT_AFTER_SEEK);
  
  // sense drive status
  // Track Zero Bit should be set
  buf[0] = FDC_CMD_STATUS;
  buf[1] = CMD_STATUS_MOT | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2);
  cnt = fdc_return(fdd->cntrlr, 1, &status);
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
      fdc_command(fdd->cntrlr, buf, 3);
      
      // set data rate speed
      outp(fdd->cntrlr->base + FDC_CCR, fdd->trans_speed);
    }
    
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
  delay(FDC_SLT_AFTER_SEEK);
  
  // seek back to track zero
  buf[0] = FDC_CMD_SEEK;
  buf[1] = (0 << 2) | fdd->drv;
  buf[2] = 0;
  fdc_command_int(fdd, buf, 3, &ret_cnt, &status, &cur_cyl);
  if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != 0))
    return FALSE;
  
  // wait for the head to settle
  delay(FDC_SLT_AFTER_SEEK);
  
  // Configure 
  if (fdd->cntrlr->enhanced) {
    buf[0] = FDC_CMD_CONFIGURE;
    buf[1] = 0;     // zero
    buf[2] = 0x5F;  // EIS = TRUE, Disable Poll, Enable FIFO, FIFO = 16 bytes
    buf[3] = 0;     // pretrk = 0
    if (!fdc_command(fdd->cntrlr, buf, 4))
      return FALSE;
  }
  
  // set the gap length for 1_44, 720, or 1_22 media
  gpl = 0x1B;

  // Specify command
  buf[0] = FDC_CMD_SPECIFY;
  buf[1] = 0xAF;          // Step Rate of 0x0A, Head Unload Time of 0x0F
  buf[2] = (1 << 1) | 0;  // Head Load Time of 0x01, NonDMA if FALSE
  if (!fdc_command(fdd->cntrlr, buf, 3))
    return FALSE;
  
  // TODO: If 82077AA or better (may need index into types), 
  // send 0x00 to port 0x3F7 (500kbps)
  //outp(fdd->cntrlr->base + FDC_DSR, 0x00);
  
  // Recalibrate (Make sure we are at Track Zero)
  buf[0] = FDC_CMD_RECAL;
  buf[1] = fdd->drv;
  fdc_command_int(fdd, buf, 2, NULL, &status, &cur_cyl);
  if ((ret_cnt != 2) || ((status & ~ST0_RESV_MSK) != (ST0_SEEK_END | (0 << 2) | fdd->drv)) || (cur_cyl != 0))
    return FALSE;
  
  // wait for the head to settle
  delay(FDC_SLT_AFTER_SEEK);
  
  // Read ID
  // We try to read from head 1.
  //  If this media is single sided, the controller will return 0x4x in the ST0 register (abnormal termination)
  buf[0] = FDC_MFM | FDC_CMD_READ_ID;
  buf[1] = (1 << 2) | fdd->drv;
  fdc_command(fdd->cntrlr, buf, 2);
  // wait for the interrupt to happen 
  if (fdc_wait_int(FDC_TIMEOUT_INT)) {
    if (fdc_return(fdd->cntrlr, 7, buf) > 0) {
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
    if (!fdc_command(fdd->cntrlr, buf, 3))
      return FALSE;
    
    // set the gap length for 160k or 180k media
    gpl = 0x2A;
    
    buf[0] = FDC_CMD_RECAL;
    buf[1] = fdd->drv;
    fdc_command_int(fdd, buf, 2, NULL, &status, &cur_cyl);
    
    // wait for the head to settle
    delay(FDC_SLT_AFTER_SEEK);
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
    if (fdc_command(fdd->cntrlr, buf, 9)) {
      if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
        if (fdc_return(fdd->cntrlr, 7, buf) == 7) {
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
  delay(FDC_SLT_AFTER_SEEK);

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
      if (fdc_command(fdd->cntrlr, buf, 9)) {
        if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
          if (fdc_return(fdd->cntrlr, 7, buf) == 7) {
            // if ST0 == 000000xx and return cylinder is now on cylinder 1 (1/0/1), we didn't try to read the non-existant sector
            if (((buf[0] & ~0x3) == 0) && (buf[3] == 1) && (buf[5] == 1))
              continue;
            if ((buf[3] == 0) && (buf[5] == (bit8u) (start + i))) {
              if (i == 0)
                // we didn't even read the first sector (start) succesfully
                spt = 0;
              else
                // else, we tried to read the non-existant sector, so (start + i - 1) is last sector of cylinder.
                spt = (start + i - 1);
              break;
            } else
              ; // an unknown result...
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
      delay(FDC_SLT_AFTER_SEEK);
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
    if (fdc_command(fdd->cntrlr, buf, 9)) {
      if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
        if (fdc_return(fdd->cntrlr, 7, buf) == 7) {
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
  // Try the configure command.
  if (write_fdc(fdc, FDC_CMD_CONFIGURE) && fdc_want_more(fdc)) {
    write_fdc(fdc, 0);
    write_fdc(fdc, (flags & ~0x0F) | ((fifo_size - 1) & 0x0F));
    write_fdc(fdc, 0); // pre-compensation from track 0 upwards
    return TRUE;
  }
  return FALSE;
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
  
  // Now read in a sector on a different cylinder, cylinder 20 (zero based)
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
  outp(fdc->base + FDC_DOR, 0x00);   // reset
  delay(2);                          // hold for 2 milliseconds
  outp(fdc->base + FDC_DOR, 0x0C);   // release
  delay(2);                          // hold for 2 milliseconds
  
  // wait for the interrupt, then sense an interrupt on all four drives
  // Page 41 of the FDC 82077AA specs
  if (fdc_wait_int(FDC_TIMEOUT_INT)) {
    for (int i=0; i<4; i++) {
      buf[0] = FDC_CMD_SENSE_INT;
      fdc_command(fdc, buf, 1);
      bit8u count = fdc_return(fdc, 2, buf);
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
  fdc_command(fdc, buf, 3);
  
  // try to configure it
  if (fdc->enhanced)
    fdc_configure(fdc, FDC_IMPLIED_SEEK | FDC_DISABLE_POLL, 16);
  
  return TRUE;
}

void fdc_motor_cntr(struct S_FLOPPY *fdd, const bool on, const bool wait) {
  
  // the FDC_TYPE_765A/B's DOR register is read only, so we can't tell if the drive was on or not
  // therefore simulate a read of 0x00
  bit8u dor = ((fdd->cntrlr->type == FDC_TYPE_765A) || (fdd->cntrlr->type == FDC_TYPE_765B)) ? 0 : inp(fdd->cntrlr->base + FDC_DOR);
  
  const bool was_off = (dor & (0x10 << fdd->drv)) ? FALSE: TRUE;
  if (on) {
    if (was_off) {
      outp(fdd->cntrlr->base + FDC_DOR, ((dor & 0xF0) | ((0x10 << fdd->drv) | 0x0C | fdd->drv)));
      if (wait)
        delay(FDC_SPINUP);
    }
  } else
    outp(fdd->cntrlr->base + FDC_DOR, 0x0C | (dor & ~(0x10 << fdd->drv)));
}

volatile bool fdc_drv_stats = FALSE;
#define FLOPPY_IRQ_VECT  0x0E
#define TIMER_IRQ_VECT   0x08

void floppy_irq(void) {
  fdc_drv_stats = TRUE;
  outp(0x20, 0x20); // EOI
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
    delay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

// checks to see if the controller is expecting more input
// waits 100ms and returns FALSE if not
bool fdc_want_more(struct S_FLOPPY_CNTRLR *fdc) {
  int timeout = 100;
  while (timeout) {
    if ((inp(fdc->base + FDC_MSR) & 0xC0) == 0x80)
      return TRUE;
    delay(1); // hold for 1 millisecond
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
    if ((inp(fdc->base + FDC_MSR) & 0xC0) == 0x80) {
      outp(fdc->base + FDC_CSR, val);      
      return TRUE;
    }
    delay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

bool fdc_command_int(struct S_FLOPPY *fdd, const bit8u *buf, bit8u cnt, bit8u *ret_cnt, bit8u *status, bit8u *cur_cyl) {
  bit8u r, int_ret[16];
  
  if (fdc_command(fdd->cntrlr, buf, cnt)) {
    if (fdc_wait_int(FDC_TIMEOUT_INT)) {
      int_ret[0] = FDC_CMD_SENSE_INT;
      if (fdc_command(fdd->cntrlr, int_ret, 1)) {
        r = fdc_return(fdd->cntrlr, 2, int_ret);
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

bool fdc_command(struct S_FLOPPY_CNTRLR *fdc, const bit8u *buf, bit8u cnt) {
  
  // clear the interrupt flag
  fdc_drv_stats = FALSE;
  
  bit8u  cur = 0;
  int timeout = FDC_TIMEOUT_CNT;
  while (timeout) {
    if (inp(fdc->base + FDC_MSR) & 0x80) {       // if not busy
      if (inp(fdc->base + FDC_MSR) & 0x40) {     // direction?
        inp(fdc->base + FDC_CSR);
        continue;
      } else {
        outp(fdc->base + FDC_CSR, buf[cur]);
        if (++cur == cnt)
          return TRUE;
      }
    }
    delay(1); // hold for 1 millisecond
    timeout--;
  }
  
  fdd_init_controller(fdc);
  return FALSE;
}

bit8u fdc_return(struct S_FLOPPY_CNTRLR *fdc, const bit8u cnt, bit8u *buf) {
  bit8u  cur = 0;
  int timeout = FDC_TIMEOUT_CNT;
  
  while (1) {
    while (((inp(fdc->base + FDC_MSR) & 0xC0) != 0xC0) && timeout) {
      delay(1); // hold for 1 millisecond
      timeout--;
    }
    if (timeout == 0) {
      fdd_init_controller(fdc);
      break;
    } else {
      buf[cur++] = inp(fdc->base + FDC_CSR);
      if (cur == cnt) break;  // don't do more than requested
      timeout = FDC_TIMEOUT_CNT;
    }
  }
  
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
  delay(FDC_SLT_AFTER_SEEK);
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
      if (fdc_command(fdd->cntrlr, buf, 9)) {
        if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
          if (fdc_return(fdd->cntrlr, 7, buf) == 7) {
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
  if (inp(fdd->cntrlr->base + FDC_DIR) & FDC_DIR_CHNG_LINE) {
    // seek to track 1
    buf[0] = FDC_CMD_SEEK;
    buf[1] = (0 << 2) | fdd->drv;
    buf[2] = 1;
    fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
    
    // wait for the head to settle
    delay(FDC_SLT_AFTER_SEEK);
    
    // seek to track 0
    buf[2] = 0;
    fdc_command_int(fdd, buf, 3, NULL, NULL, NULL);
    
    // wait for the head to settle
    delay(FDC_SLT_AFTER_SEEK);
    
    status->inserted = ((inp(fdd->cntrlr->base + FDC_DIR) & FDC_DIR_CHNG_LINE) == 0);
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
  fdc_command(fdd->cntrlr, buf, 2);
  fdc_return(fdd->cntrlr, 1, buf);
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
  if (fdc_command(fdd->cntrlr, buf, 2)) {
    if (fdc_wait_int(FDC_TIMEOUT_INT)) {  // wait for the interrupt to happen
      if (fdc_return(fdd->cntrlr, 7, buf) == 7) {
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
  outp(DMA_MASK_REG, (1<<2) | CHANNEL_FDC);  // DMA-1 Mask Register Bit Port (8-bit)
  
  //  DMA-1 Clear Byte Flip-Flop (write anything)
  outp(DMA_FLIP_FLOP, 0xFF);
  
  // low 16-bits of address
  outp(0x04, (bit8u) ((address >>  0) & 0xFF)); // DMA-1 Channel 2 Output (low byte)
  outp(0x04, (bit8u) ((address >>  8) & 0xFF)); // DMA-1 Channel 2 Output (high byte)
  
  //  DMA-1 Clear Byte Flip-Flop (write anything)
  outp(DMA_FLIP_FLOP, 0xFF);
  
  // size of transfer
  size--;  // size is always 1 less from sent size
  outp(0x05, (bit8u) ((size    >>  0) & 0xFF)); // DMA-1 Channel 2 Output size (low byte)
  outp(0x05, (bit8u) ((size    >>  8) & 0xFF)); // DMA-1 Channel 2 Output size (high byte)
  
  // page register (bits 23-16 of address)
  outp(0x81, (bit8u) ((address >> 16) & 0xFF)); // DMA-1 Channel 2 Output (page value)
  
  // write command code
  outp(DMA_MODE_REG, dmacmd | CHANNEL_FDC);     // Set DMA-1 Mode Register port
  
  // select (unmask the channel)
  outp(DMA_MASK_REG, CHANNEL_FDC);              // DMA-1 Mask Register Bit Port (8-bit)
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
