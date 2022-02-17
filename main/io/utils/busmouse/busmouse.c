/*
 *                             Copyright (c) 1984-2022
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
 *             https://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  BUSMOUSE.EXE
 *   Will detect an old style bus mouse
 *
 *  Assumptions/prerequisites:
 *  - This example is coded and compiled for 16-bit DOS using a 16-bit compiler.
 *  - There is no need for a DPMI driver.
 *  - All variables and numbers are 16-bit.
 *     char = bit8s = byte = 8-bits  (AL)
 *      int = bit16s = word = 16-bits (AX)
 *     long = bit32s = dword = 32-bits (DX:AX)
 *
 *  Last updated: 12 Feb 2022
 *
 *  compiled using MS QuickC 2.5
 *
 *  Usage:
 *    busmouse
 */

#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"
#include "../include/pic.c"
#include "busmouse.h"

// our coordinates (middle of text screen)
int  dx = (80 / 2), dy = (25 / 2);
bool left = 0, mid = 0, right = 0;
bool leftx = 0, midx = 0, rightx = 0;

int main(int argc, char *arg[]) {
  bit8u sig0, sig1;
  int id = -1, irq_num;
  
  printf("Detect Bus Mouse Controller.         v1.00.00\n"
         "Forever Young Software -- Copyright 1984-2022\n\n");
  
  // print the base address we are using.  The InPort card
  //  has a jumper that allows us to choose between 0x23C and 0x238
  printf("Base = 0x%04X\n", ABS_BASE);
  
  // identify the card
  id = bus_identify();
  switch (id) {
    case CARD_ID_INPORT:
      printf("Found MS Inport Card...\n");
      break;
    case CARD_ID_LOGITECH:
      printf("Found MS/Logitech Card...\n");
      break;
    default:
      printf("Unknown or no card found...\n");
      return -1;  // return to DOS
  }
  
  // get the IRQ number of card
  irq_num = bus_find_irq(id);
  if (irq_num == -1) {
    printf("Did not find IRQ value...\n");
    return -1;
  }
  printf("Found IRQ number: %i\n", irq_num);
  
  // save old IRQ handler
  old_isr = _dos_getvect(ivt_num[irq_num]);
  
  // unmask IRQ
  picunmask(irq_num);
  
  // start the interrupts
  switch (id) {
    case CARD_ID_INPORT:
      _dos_setvect(ivt_num[irq_num], inport_mouse_irq);
      outpb(INPORT_CTRL_PORT, INPORT_REG_MODE);
      outpb(INPORT_DATA_PORT, INPORT_MODE_BASE | INPORT_MODE_IRQ);
      break;
    case CARD_ID_LOGITECH:
      _dos_setvect(ivt_num[irq_num], log_mouse_irq);
      outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_ENABLE_IRQ);
      break;
  }
  
  // Print a line of data.  The first pair is the X & Y coordinates
  // The second six digits are set(1) or clear(0) for button presses.
  puts("");
  while (!kbhit())
    printf("%i %i   %i%i%i%i%i%i      \r", dx, dy, leftx, midx, rightx, left, mid, right);
  
  // clear the line and get the key used to stop the capture
  puts("                             ");
  getch();
  
  // turn off interrupts
  switch (id) {
    case CARD_ID_INPORT:
      outpb(INPORT_CTRL_PORT, INPORT_REG_MODE);
      outpb(INPORT_DATA_PORT, INPORT_MODE_BASE);
      break;
    case CARD_ID_LOGITECH:
      // disable irq's
      outpb(LOGM_CTRL_PORT, LOGM_DISABLE_IRQ);
      break;
  }
  
  // restore original mask
  picmask(irq_num);
  _dos_setvect(ivt_num[irq_num], old_isr);         // replace old ISR
  
  // return to DOS
  return 0;
}

// Logitech Bus Mouse / MS Bus Mouse
// IRQ Handler
void interrupt log_mouse_irq() {
  bit8s x, y;
  bit8u b;
  
  // read the data, setting the Hold bit
  outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_READ_X_LOW);
  x = (inpb(LOGM_DATA_PORT) & 0x0F);
  outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_READ_X_HIGH);
  x |= (inpb(LOGM_DATA_PORT) & 0x0F) << 4;
  outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_READ_Y_LOW);
  y = (inpb(LOGM_DATA_PORT) & 0x0F);
  outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_READ_Y_HIGH);
  b = inpb(LOGM_DATA_PORT);
  y |= (b & 0x0F) << 4;
  dx += (int) x;  if (dx < 0) dx = 0; if (dx > 79) dx = 79;
  dy += (int) y;  if (dy < 0) dy = 0; if (dy > 24) dy = 24;
  leftx = midx = rightx = 0;
  left = (b & LOGM_BUTTON_LEFT) == 0;
  mid = (b & LOGM_BUTTON_MID) == 0;
  right = (b & LOGM_BUTTON_RIGHT) == 0;
  
  // turn off the hold bit
  outpb(LOGM_CTRL_PORT, LOGM_ALLOW_COUNTER);
  
  // acknowledge interrupt
  outpb(0x20, 0x20);
}

// Inport Bus Mouse
// IRQ Handler
void interrupt inport_mouse_irq() {
  bit8u b;
  
  // set the hold bit
  outpb(INPORT_CTRL_PORT, INPORT_REG_MODE);
  outpb(INPORT_DATA_PORT, INPORT_MODE_HOLD | INPORT_MODE_BASE | INPORT_MODE_IRQ);
  
  // read the X data
  outpb(INPORT_CTRL_PORT, INPORT_REG_X);
  dx += (int) (bit8s) inpb(INPORT_DATA_PORT);
  if (dx < 0) dx = 0; if (dx > 79) dx = 79;
  
  // read the Y data
  outpb(INPORT_CTRL_PORT, INPORT_REG_Y);
  dy += (int) (bit8s) inpb(INPORT_DATA_PORT);
  if (dy < 0) dy = 0; if (dy > 24) dy = 24;
  
  // read the Button data
  outpb(INPORT_CTRL_PORT, INPORT_REG_BTNS);
  b = inpb(INPORT_DATA_PORT);
  leftx = (b & INPORT_BUTTON_LEFTx) > 0;
  midx = (b & INPORT_BUTTON_MIDx) > 0;
  rightx = (b & INPORT_BUTTON_RIGHTx) > 0;
  left = (b & INPORT_BUTTON_LEFT) > 0;
  mid = (b & INPORT_BUTTON_MID) > 0;
  right = (b & INPORT_BUTTON_RIGHT) > 0;
  
  // Clear the Hold Bit
  outpb(INPORT_CTRL_PORT, INPORT_REG_MODE);
  outpb(INPORT_DATA_PORT, INPORT_MODE_BASE | INPORT_MODE_IRQ);
  
  // acknowledge interrupt
  outpb(0x20, 0x20);
}

// Identify the card adapter
int bus_identify(void) {
  bit8u sig0, sig1, sig2, sig3;
  
  // first try the InPort
  // We don't check for 0xDE, 0x12, 0xDE since somewhere something
  //  may have already read from this port making the controller
  //  swap to the next digit (0x12).  We just check that the sig
  //  alternates back and forth.
  sig0 = inpb(INPORT_SIG_PORT);
  sig1 = inpb(INPORT_SIG_PORT);
  sig2 = inpb(INPORT_SIG_PORT);
  sig3 = inpb(INPORT_SIG_PORT);
  if ((sig0 == sig2) && (sig1 == sig3) && (sig0 != sig1))
    return CARD_ID_INPORT;
  
  // check for the Logitetch/MS Bus Mouse
  outpb(LOGM_CNFIG_PORT, 0x91);
  outpb(LOGM_SIG_PORT, 0xA5);
  outpb(LOGM_CNFIG_PORT, 0x11);  // I don't know what this value is, but the Logitech driver does it, so I will too.
  sig0 = inpb(LOGM_SIG_PORT);
  if (sig0 == 0xA5)
    return CARD_ID_LOGITECH;
  
  // else return error.  We didn't find one.
  return -1;
}

// A temp interrupt handler so we can find the IRQ for the InPort card
bit8u found = 0;
void interrupt temp_irq() {
  found = 1;
  outpb(0x20, 0x20);
}

// find the IRQ of the card.
int bus_find_irq(const int id) {
  bit8u b0, b1, acc = 0;
  int i, j, irq = -1;

  switch (id) {
    // The InPort card doesn't have any mechanism to get it, other than
    //  firing an interrupt and seeing which IRQ was fired (5, 4, or 3)
    //  (The card allows IRQ 2, but we are an AT machine, so IRQ 2 is occupied)
    case CARD_ID_INPORT:
      for (i=5; i>=3; i--) {
        old_isr = _dos_getvect(ivt_num[i]);
        _dos_setvect(ivt_num[i], temp_irq);
        
        found = 0;
        
        // unmask IRQ
        picunmask(i);
        
        outpb(INPORT_CTRL_PORT, INPORT_RESET);
        outpb(INPORT_CTRL_PORT, INPORT_REG_MODE);
        
        for (j=0; j<10; j++) {
          outpb(INPORT_DATA_PORT, INPORT_MODE_BASE);
          outpb(INPORT_DATA_PORT, INPORT_RAISE_IRQ);
        }
        
        // restore original mask
        picmask(i);
        _dos_setvect(ivt_num[i], old_isr);
        
        // if found is set, we found the IRQ (given in 'i')
        if (found) {
          irq = i;
          break;
        }
      }
      break;
      
    case CARD_ID_LOGITECH:
      // the logitech card needs IRQ's on for this to work.
      // the MS card does not
      outpb(LOGM_CTRL_PORT, LOGM_HOLD_COUNTER | LOGM_ENABLE_IRQ);
      
      b0 = inpb(LOGM_CTRL_PORT);
      for (i=0; i<10000; i++) {
        b1 = inpb(LOGM_CTRL_PORT);
        b0 ^= b1;
        acc |= b0;
        b0 = b1;
      }
      // now acc = 00000001 = irq5
      //           00000010 = irq4
      //           00000100 = irq3
      //           00001000 = irq2
      switch (acc) {
        case 1: irq = 5; break;
        case 2: irq = 4; break;
        case 4: irq = 3; break;
        case 8: irq = 2; break;
        default:
          printf("Invalid IRQ returned: acc = 0x%02X", acc);
      }
  }
  
  // each controller only allows IRQ 2 to IRQ 5
  if ((irq < 2) && (irq > 5)) {
    printf("Found invalid IRQ number: %i\n", irq);
    return -1;
  }
  
  return irq;
}
