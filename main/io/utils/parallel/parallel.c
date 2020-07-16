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
 *  PARALLEL.EXE
 *
 *  Assumptions/prerequisites:
 *   - This code is heavily based on the Linux Parallel Port code found at:
 *      https://github.com/torvalds/linux/blob/master/drivers/parport/parport_pc.c
 *  
 *   - This example is coded and compiled for 16-bit DOS using a 16-bit compiler.
 *   - There is no need for a DPMI driver.
 *   - All variables and numbers are 16-bit.
 *      char = bit8s = byte = 8-bits  (AL)
 *       int = bit16s = word = 16-bits (AX)
 *      long = bit32s = dword = 32-bits (DX:AX)
 *
 *  Last updated: 15 July 2020
 *
 *  Compiled using MS QuickC 2.5
 *
 *  Usage:
 *    parallel
 */

#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/ctype.h"
#include "../include/pic.c"
#include "parallel.h"

struct DET_PAR {
  bit16u base;      // base of this controller
  bit8u  irq;       // irq of port
} det_par[] = {
  { 0x002E, 7 }, // Dell Enhanced Parallel Port (EPP)
  { 0x015C, 7 }, // Dell Enhanced Parallel Port (EPP)
  { 0x026E, 7 }, // Dell Enhanced Parallel Port (EPP)
  { 0x0398, 7 }, // Dell Enhanced Parallel Port (EPP)
  { 0x03BC, 7 },
  { 0x0378, 7 },
  { 0x0278, 5 },
  { 0, 0 }
};

// information about the currently found Parallel port
bit16u base;
 bit8u type = 0;
  bool dma = 0;
   int irq = 0;
   int FIFO_depth = 0;
   int writeIntrThreshold = 0;
   int readIntrThreshold = 0;
 bit8u word_size = 0;  // 8-bit, 16-bit, or 32-bit reads/writes
  bool ecr = 0, bidirectional = 0;

int main(int argc, char *arg[]) {
  int curpar = 0;
  
  printf("Detect Parallel Port Controllers.    v1.00.00\n"
         "Forever Young Software -- Copyright 1984-2020\n\n");
  
  while (det_par[curpar].base) {
    base = det_par[curpar].base;
    
    // first, check for SPP (Standard Parallel Port)
    if (par_detect_SPP()) {
      type = PAR_TYPE_SPP;
      irq = det_par[curpar].irq;
      dma = 0; // DMA not supported on SPP
      
      // check if Bi-directional mode is available
      bidirectional = par_detect_SPP_PS2();
      
      // if port = 0x3BC, then no need to continue.  It is too old of a device.
      // these were integrated on video cards and do not support ECP addresses.
      if (base != 0x03BC) {
        // detect the ECR (Extended Control Register)
        ecr = par_detect_ECR();
        
        // let's try EPP
        if (par_detect_EPP()) {
          type = PAR_TYPE_EPP;
          irq = det_par[curpar].irq;
          dma = 0; // DMA not supported on EPP
        } else {
          // if no EPP, try SMC style EPP+ECP
          if (par_detect_ECPEPP()) {
            type = PAR_TYPE_EPPECP;
            irq = det_par[curpar].irq;
            dma = 0; // DMA not supported on EPP mode (even under ECP)
          }
        }
        
        // let's try ECP
        if (par_detect_ECP()) {
          type = PAR_TYPE_ECP;
          irq = par_ecp_get_irq();
          dma = par_ecp_prog_dma_support();
        }
      }
      
      // we found one....
      printf(" Found Parallel Port with type %s at base 0x%04X\n", par_type[type], base);
      printf("            dma: %i\n", dma);
      printf("            irq: %i\n", irq);
      printf("     FIFO depth: %i\n", FIFO_depth);
      printf("Write Threshold: %i\n", writeIntrThreshold);
      printf(" Read Threshold: %i\n", readIntrThreshold);
      printf("      Word size: %i\n", word_size);
      printf(" bi-directional: %i\n", bidirectional);
      printf("        Has ecr: %i\n", ecr);
    }
    
    curpar++;
  }
  
  // return to DOS
  return 0;
}

// Write a byte to the SPP Control register
//  mask = the mask to use.  
//    If a mask bit is 1, the SPP bit may be modified
//    If a mask bit is 0, the SPP bit is preserved
//  val = value to write to SPP using mask
void par_ctrl_write(const bit8u mask, const bit8u val) {
  bit8u ctrl = 0;
  
  // if we have a mask, read in the original value first
  if (mask != 0xFF)
    ctrl = inpb(base + PAR_CONTROL);
  
  // write the value to the port.
  // if mask == 0xFF, then (ctrl & ~mask) = 0.
  outpb(base + PAR_CONTROL, (ctrl & ~mask) ^ val);
}

bool par_detect_SPP() {
  bit8u r, w;
  
  // first clear an eventually pending EPP timeout 
  // A SMSC chipset may not respond to SPP cycles if 
  //  an EPP timeout is pending
  par_clear_epp_timeout();
  
  // Do a simple read-write test to make sure the port exists.
  w = 0x0C;
  outpb(base + PAR_CONTROL, w);
  r = inpb(base + PAR_CONTROL);
  if ((r & 0x0F) == w) {
    w = 0x0E;
    outpb(base + PAR_CONTROL, w);
    r = inpb(base + PAR_CONTROL);
    outpb(base + PAR_CONTROL, 0x0C);
    if ((r & 0x0F) == w)
      return TRUE;
  }
  
  // if that didn't work, try the data register.  The data lines aren't 
  // tri-stated at this stage, so we expect back what we wrote.
  w = 0xAA;
  outpb(base + PAR_DATA, w);
  r = inpb(base + PAR_DATA);
  if (r == w) {
    w = 0x55;
    outpb(base + PAR_DATA, w);
    r = inpb(base + PAR_DATA);
    if (r == w)
      return TRUE;
  }
  
  return FALSE;
}

bool par_detect_SPP_PS2(void) {
  bool fnd = 0;
  
  par_clear_epp_timeout();
  // try to tri-state the buffer
  par_ctrl_write((1<<5), (1<<5));       // enable bi-directional
  
  outpb(base + PAR_DATA, 0x55);
  if (inpb(base + PAR_DATA) != 0x55)
    fnd = 1;
  outpb(base + PAR_DATA, 0xAA);
  if (inpb(base + PAR_DATA) != 0xAA)
    fnd = 1;
  // cancel input mode
  par_ctrl_write((1<<5), (0<<5));       // disable bi-directional
  
  // fnd not zero = yes, else no bi-directional support
  return fnd;
}

bool par_detect_EPP(void) {
  bit8u i;
  // Bit 0 of STR is the EPP timeout bit, this bit is 0 when EPP is possible 
  // and is set high when an EPP timeout occurs (EPP uses the HALT line to 
  // stop the CPU while it does the byte transfer, an EPP timeout occurs if 
  // the attached device fails to respond after 10 microseconds).
  // This bit is cleared by either reading it (National Semi) or writing a 1 
  // to the bit (SMC, UMC, WinBond), others ???
  // This bit is always high in non EPP modes.
  
  // If EPP timeout bit clear then EPP available
  if (!par_clear_epp_timeout())
    return FALSE;
  
  // Check for Intel bug. ???
  if (ecr) {
    for (i=0x00; i < 0x80; i += 0x20) {
      par_ecr_write(PAR_ECR_NO_MASK, i);
      if (par_clear_epp_timeout()) {
        // Phony EPP in ECP.
        //putstr("\n  Found faulty EPP in ECP (buggy Intel chip?)");
        return FALSE;
      }
    }
  }
  
  return TRUE;
}

bool par_detect_ECPEPP(void) {
  bit8u org_ecr;
  bool res;
  
  // If no ECR, no chance of an ECPEPP
  if (!ecr)
    return FALSE;
  
  org_ecr = inpb(base + PAR_ECP_CONTROL);
  // Search for SMC style EPP+ECP mode
  par_set_mode(PAR_ECR_EPP, 0, 0, 0);  // epp mode, no interrupts, no dma
  outpb(base + PAR_CONTROL, 0x04);
  res = par_detect_EPP();
  par_ecr_write(PAR_ECR_NO_MASK, org_ecr);
  return res;
}

bool par_detect_ECP(void) {
  int i = 0;
  
  // If no ECR, no chance of an ECP
  if (!ecr)
    return FALSE;
  
  // find out FIFO depth
  par_set_mode(PAR_ECR_SPP, 0, 0, 0);   // reset FIFO
  par_set_mode(PAR_ECR_TST, 0, 0, 0);   // test FIFO mode, no interrupts
  
  // check at top of loop so we don't mistake the threshold to be 1.
  while ((i < 512) && !(inpb(base + PAR_ECP_CONTROL) & PAR_ECR_FIFO_FL)) {
    outpb(base + PAR_ECP_FIFO, 0xAA);
    i++;
  }
  
  // if i == 512, we can't have an FIFO and probably don't have an ECP
  if (i == 512) {
    par_set_mode(PAR_ECR_SPP, 0, 0, 0);  // reset FIFO
    FIFO_depth = 0;
    return FALSE;
  }
  
  // FIFO depth (usually 16 bytes)
  FIFO_depth = i;
  
  // find writeIntrThreshold
  par_ecr_write(PAR_ECR_SERVICE, PAR_ECR_SERVICE);
  par_ecr_write(PAR_ECR_SERVICE, 0);
  for (i=1; i <= FIFO_depth; i++) {
    inpb(base + PAR_ECP_FIFO);
    mdelay(1);  // 50 uS is enough
    if (inpb(base + PAR_ECP_CONTROL) & PAR_ECR_SERVICE) 
      break;
  }
  if (i <= FIFO_depth)
    writeIntrThreshold = i;
  else
    writeIntrThreshold = 0;
  
  // find readIntrThreshold
  par_set_mode(PAR_ECR_BYTE, 0, 0, 0);  // reset FIFO and enable bi-directional
  par_ctrl_write((1<<5), (1<<5));       // enable bi-directional
  par_set_mode(PAR_ECR_TST, 0, 0, 0);   // test FIFO
  par_ecr_write(PAR_ECR_SERVICE, PAR_ECR_SERVICE);
  par_ecr_write(PAR_ECR_SERVICE, 0);
  for (i=1; i <= FIFO_depth; i++) {
    outpb(base + PAR_ECP_FIFO, 0xAA);
    if (inpb(base + PAR_ECP_CONTROL) & PAR_ECR_SERVICE) 
      break;
  }
  if (i <= FIFO_depth)
    readIntrThreshold = i;
  else
    readIntrThreshold = 0;
  
  par_set_mode(PAR_ECR_SPP, 0, 0, 0);        // reset the FIFO
  par_set_mode(PAR_ECR_CNF, 1, 0, 1);        // Configuration mode, interrupts, no dma, service bit
  
  word_size = ((inpb(base + PAR_ECP_CONFIG_A) >> 4) & 0x07);
  switch (word_size) {
    case 0:        // Supported word_size size is 16-bit
      word_size = 2;
      break;
    case 2:        // Supported word_size size is 32-bit
      word_size = 4;
      break;
    case 1:        // Supported word_size size is 8-bit
    default:       // Unknown word_size (assume 1)
      word_size = 1;
  }
  
  // Go back to SPP mode
  par_set_mode(PAR_ECR_SPP, 0, 0, 0);
  return TRUE;
}

// detects the Extended Control Register
// returns FALSE if not detected
//
// Old style XT ports alias io ports every 0x400, hence accessing ECR
// on these cards actually accesses the SPP Control Register.
// Modern cards don't do this but reading from ECR will return 0xFF
// regardless of what is written here if the card does NOT support ECP.
// We first check to see if ECR is the same as SPP Control Register.
// If not, the low two bits of ECR aren't writable, so we check by 
// writing ECR and reading it back to see if it's what we expect.
bool par_detect_ECR(void) {
  bit8u r = 0x0C;
  
  outpb(base + PAR_CONTROL, r);
  if ((inpb(base + PAR_ECP_CONTROL) & 0x03) == (r & 0x03)) {
    outpb(base + PAR_CONTROL, r ^ 0x02); // toggle bit 1
    r = inpb(base + PAR_CONTROL);
    if ((inpb(base + PAR_ECP_CONTROL) & 0x02) == (r & 0x02)) {
      outpb(base + PAR_CONTROL, 0x0C);
      return FALSE;
    }
  }
  
  if ((inpb(base + PAR_ECP_CONTROL) & 0x03) != 0x01) {
    outpb(base + PAR_CONTROL, 0x0C);
    return FALSE;
  }
  
  par_ecr_write(PAR_ECR_NO_MASK, 0x34);
  if (inpb(base + PAR_ECP_CONTROL) != 0x35) {
    outpb(base + PAR_CONTROL, 0x0C);
    return FALSE;
  }
  
  // We have an ECR
  // so go to mode SPP
  outpb(base + PAR_CONTROL, 0x0C);
  par_set_mode(PAR_ECR_SPP, 0, 0, 0);
  return TRUE;
}

// Write a byte to the ECR register
//  mask = the mask to use.  
//    If a mask bit is 1, the ECR bit may be modified
//    If a mask bit is 0, the ECR bit is preserved
//  val = value to write to ECR using mask
void par_ecr_write(const bit8u mask, const bit8u val) {
  bit8u ectr = 0;
  
  // if we have a mask, read in the original value first
  if (mask != 0xFF)
    ectr = inpb(base + PAR_ECP_CONTROL);
  
  // write the value to the port.
  // if mask == 0xFF, then (ectr & ~mask) = 0.
  outpb(base + PAR_ECP_CONTROL, (ectr & ~mask) ^ val);
}

// sets the mode of the ECP in the ECR register with flags for interrupts, dma, and service bit
void par_set_mode(const int mode, const bool interrupts, const bool dma, const bool service) {
  par_ecr_write(PAR_ECR_NO_MASK, (bit8u) ((mode << 5) | (interrupts << 4) | (dma << 3) | (service << 2)));
}

bool par_clear_epp_timeout(void) {
  bit8u r;
  
  if (!(inpb(base + PAR_STATUS) & 0x01))
    return TRUE;
  
  // To clear timeout some chips require double read
  inpb(base + PAR_STATUS);
  r = inpb(base + PAR_STATUS);
  outpb(base + PAR_STATUS, (r | 0x01)); // some reset by writing 1
  outpb(base + PAR_STATUS, (r & 0xFE)); // others by writing 0
  r = inpb(base + PAR_STATUS);
  return !(r & 0x01);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// IRQ detection:
//  only done if supports ECP mode
//  if returns 0, then IRQ set via jumpers
bit8u par_ecp_prog_irq_support(void) {
  bit8u irq, intrLine;
  bit8u org_ecr = inpb(base + PAR_ECP_CONTROL);
  const bit8u lookup[8] = { 0, 7, 9, 10, 11, 14, 15, 5	};
  
  par_set_mode(PAR_ECR_CNF, 0, 0, 0);   // Configuration mode, no interrupts, no dma
  intrLine = (inpb(base + PAR_ECP_CONFIG_B) >> 3) & 0x07;
  irq = lookup[intrLine];
  
  par_ecr_write(PAR_ECR_NO_MASK, org_ecr);
  return irq;
}

// A temp interrupt handler so we can find the IRQ for the InPort card
bit8u found = 0;
void interrupt temp_irq() {
  found = 1;
  outpb(0x20, 0x20);
}

bit8u par_ecp_irq_test(void) {
  int i = 0, j;
  const int irq[] = { 5, 7, 9, 10, 11, 14, 15, 0 };
  // allow irq 5, 7, 9, 10, 11, 14, and 15
  
  // start the find_irq_code
  while (irq[i]) {
    old_isr = _dos_getvect(ivt_num[irq[i]]);
    _dos_setvect(ivt_num[irq[i]], temp_irq);
    
    found = 0;
    
    // unmask IRQ
    picunmask(irq[i]);
    
    par_set_mode(PAR_ECR_SPP, 0, 0, 0);  // Reset the FIFO
    par_set_mode(PAR_ECR_TST, 0, 0, 1);  // Test mode with service bit
    par_set_mode(PAR_ECR_TST, 0, 0, 0);  // Test mode w/o service bit
    
    // If Full FIFO, make sure that writeIntrThreshold is generated 
    for (j=0; (j < 512) && !(inpb(base + PAR_ECP_CONTROL) & PAR_ECR_FIFO_FL); j++)
      outpb(base + PAR_ECP_FIFO, 0xAA);
    
    // restore original mask
    picmask(irq[i]);
    _dos_setvect(ivt_num[irq[i]], old_isr);
    
    // if found is set, we found the IRQ (given in 'irq[i]')
    if (found)
      break;
    
    i++;
  }
  
  par_set_mode(PAR_ECR_SPP, 0, 0, 0);
  
  return irq[i];
}

bit8u par_ecp_get_irq(void) {
  bit8u irq = 7;  // default to 7
  
  if (ecr) {
    irq = par_ecp_prog_irq_support();
    if (irq == 0)
      irq = par_ecp_irq_test();
  }
  
  return irq;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// DMA detection:
// Only if chipset conforms to ECP ISA Interface Standard 
bit8u par_ecp_prog_dma_support(void) {
  bit8u  dma;
  bit8u  org_ecr = inpb(base + PAR_ECP_CONTROL);
  
  if (!ecr)
    return 0;
  
  par_set_mode(PAR_ECR_CNF, 0, 0, 0);
  dma = inpb(base + PAR_ECP_CONFIG_B) & 0x07;
  
  // 000: Indicates jumpered 8-bit DMA if read-only.
  // 100: Indicates jumpered 16-bit DMA if read-only. 
  if ((dma & 0x03) == 0)
    dma = 0;
  
  par_ecr_write(PAR_ECR_NO_MASK, org_ecr);
  return dma;
}

// CLOCKS_PER_SEC must be at least 1 per mS
#if (CLOCKS_PER_SEC < 1000)
  #error "CLOCKS_PER_SEC < 1000"
#endif

// This is not a very accurate way to wait when calling
//  multiple times within a loop with a small 'ms' amount.
// It takes considerable amount of time to calculate the
//  time to wait each time it is called.
// However, for our purposes, it will be just fine.
void mdelay(const int ms) {
  clock_t pause;
  
  pause = (clock_t) ((ms * (CLOCKS_PER_SEC / 1000)) + clock());
  while (clock() < pause)
    ;
}
