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
 *  SERIAL.EXE
 *
 *  Assumptions/prerequisites:
 *   - This example is coded and compiled for 16-bit DOS using a 16-bit compiler.
 *   - There is no need for a DPMI driver.
 *   - All variables and numbers are 16-bit.
 *      char = bit8s = byte = 8-bits  (AL)
 *       int = bit16s = word = 16-bits (AX)
 *      long = bit32s = dword = 32-bits (DX:AX)
 *
 *  Last updated: 15 July 2020
 *
 *  Compile using MS QuickC 2.5
 *
 *  Usage:
 *    serial
 */

#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/ctype.h"
#include "../include/pic.c"
#include "serial.h"

struct DET_SER {
  bit16u base;      // base of this controller
  bit8u  irq;       // irq of port
} det_ser[SER_NUM] = {
  { 0x03F8, 4 },
  { 0x02F8, 3 },
  { 0x03E8, 4 },
  { 0x02E8, 3 }
};

// must be same as ENUM
char types[9][20] = {
  "none",               // SER_TYPE_NONE
  "8250",               // SER_TYPE_8250
  "8250A or 16450",     // SER_TYPE_8250A_16450
  "16C1450",            // SER_TYPE_16C1450
  "16550 w/ bad FIFO",  // SER_TYPE_16550_BAD_FIFO
  "16550AF/C/CF",       // SER_TYPE_16550AF_C_CF
  "16C1550",            // SER_TYPE_16C1550
  "16552 w/ dual UART", // SER_TYPE_16552_DUAL
  "82510",              // SER_TYPE_82510
};

struct S_MOUSE_INFO info = { 0, };

int main(int argc, char *arg[]) {
  int curser;
  bit8u byte;
  bool test;
  char type_str[STR_LEN + 1]; // must be at least 8+1 bytes
  
  printf("Detect Serial Controller and Mouse.    v1.00.00\n"
         "Forever Young Software  --  Copyright 1984-2020\n\n");
  
  for (curser=0; curser<SER_NUM; curser++) {
    info.base = det_ser[curser].base;
    info.irq_num = det_ser[curser].irq;
    info.type = SER_TYPE_NONE;
    
    // write 0x5A and 0xA5 to BAUD RATE LOW register
    //  to see if we can read it back
    ser_set_dlab(info.base, 1);
    if (ser_iotest(info.base, 0x5A) && ser_iotest(info.base, 0xA5)) {
      info.type = SER_TYPE_8250;  // we at least have a controller of some kind (could be a 8250)
      
      // now check if the scratch pad register is working (missing on 8250)
      ser_set_dlab(info.base, 0);
      if (ser_iotest(info.base + SER_SCRATCH, 0x5A) && ser_iotest(info.base + SER_SCRATCH, 0xA5)) {
        // check for 1655x and 82510 FIFO versions
        if ((inpb(info.base + SER_INT_ID) & 0xC0) != 0xC0) {
          // try to enable the FIFO on 1655x
          outpb(info.base + SER_INT_ID, 0x01);
          byte = inpb(info.base + SER_INT_ID) & 0xC0;
          outpb(info.base + SER_INT_ID, 0x00);
          if (byte != 0xC0) {
            if (byte == 0x40)
              info.type = SER_TYPE_16550AF_C_CF;
            else if (byte == 0x80)
              info.type = SER_TYPE_16550_BAD_FIFO;
            else {
              // not 16550, so now check for possible 82510 by switching to
              // bank 3 (which is only possible on a 82510)
              outpb(info.base + SER_ALT_FUNC, 0x60);
              byte = inpb(info.base + SER_ALT_FUNC);
              outpb(info.base + SER_ALT_FUNC, 0x00);
              if ((byte & 0x60) != 0x60) {
                // no FIFO, so UART is 8250A or 16450 variant
                // Check if power down bit functions on this UART,
                //  in the modem control register
                if (ser_can_pwrdown(info.base))
                  info.type = SER_TYPE_16C1450;
                else 
                  info.type = SER_TYPE_8250A_16450;
              } else 
                info.type = SER_TYPE_82510;
            }
            goto ser_done;
          } // this needs to fall through
        } // this too
        // FIFO detected, must be 16550 series
        ser_set_dlab(info.base, 1);
        test = ser_iotest(info.base + SER_ALT_FUNC, 0x07);
        outpb(info.base + SER_ALT_FUNC, 0x00);
        if (!test) {
          // Check if power down bit functions on this UART, in the modem control register (only on 16C1550)
          if (ser_can_pwrdown(info.base))
            info.type = SER_TYPE_16C1550;
          else 
            info.type = SER_TYPE_16550AF_C_CF;
        } else 
          info.type = SER_TYPE_16552_DUAL;
      }
    }
    
ser_done:
    if (info.type > SER_TYPE_NONE) {
      ser_set_dlab(info.base, 0);
      
      // reset UART (if it is a 16C1450)
      if (info.type == SER_TYPE_16C1450) {
        outpb(info.base + SER_MODEM_CNTRL, (1<<2));
        mdelay(60);
        outpb(info.base + SER_MODEM_CNTRL, 0x03);
      }
      
      printf(" Found serial controller at: 0x%04X with type: %i [%s]\n", info.base, info.type, types[info.type]);
      
      if (find_serial_mouse(info.base, type_str)) {
        if (!strcmp(type_str, "M")) {          // Two Button MS compatible
          info.buttons = 2;
          info.packet_type = MOUSE_SERIAL_0;
          // We are already at a 7-bit packet, so no need to modify baud/char len/stop/parity
          puts("  Found Two Button MS Compatible Mouse.");
        } else if(!strcmp(type_str, "MZ@")) {  // MS Wheel/Intellimouse
          info.buttons = 3;
          info.packet_type = MOUSE_SERIAL_1;
          info.wheel = TRUE;
          // We need to receive an 8-bit packet, so modify baud/char len/stop/parity
          ser_set_dlab(info.base, 0);
          outpb(info.base + SER_LINE_CNTRL, SER_BITS_8 | SER_PARITY_NONE | SER_STOP_1);
          puts("  Found Three Button MS Wheel/Intellimouse.");
        } else if(!strcmp(type_str, "M3")) {   // Three Button Logitech
          info.buttons = 3;
          info.packet_type = MOUSE_SERIAL_2;
          puts("  Found Three Button Logitech Mouse.");
        } else if(!strcmp(type_str, "m")) {    // Three Button MS compatible
          info.buttons = 3;
          info.packet_type = MOUSE_SERIAL_2;
          puts("  Found Three Button MS Compatible Mouse.");
        } else if(!strcmp(type_str, "H")) {    // Three Button ?? compatible
          info.buttons = 3;
          info.packet_type = MOUSE_SERIAL_2;
          puts("  Found Three Button Mouse.");
        } else if(!strcmp(type_str, "C")) {    // old style packet passed 30 times a second
          puts(" Oh wow!  A 'C' style mouse...");
        } else {
          printf("Mouse returned unknown ID: [%s] [%i %i %i]\n", type_str, type_str[0], type_str[1], type_str[2]);
          info.packet_type = 0;
        }
        
        // set our IRQ handler
        old_isr = _dos_getvect(ivt_num[info.irq_num]);   // install our ISR
        _dos_setvect(ivt_num[info.irq_num], ser_mouse_irq); // Put our ISR in
        
        // unmask IRQ
        picunmask((bit8u) info.irq_num);
        
        // set the IRQ and get the packets...
        ser_activate(info.base, TRUE);
        
        // we currently just print the delta's and the button status
        puts("");
        while (!kbhit())
          printf("\r%i %i %i  %i%i%i     ", info.x, info.y, info.z, info.left, info.mid, info.right);
        puts("");
        
        // restore original mask
        picmask((bit8u) info.irq_num);
        _dos_setvect(ivt_num[info.irq_num], old_isr);      // replace old ISR
        
        // clear the IRQ
        ser_activate(info.base, FALSE);
        
      } else {
        printf("  Did not find a serial mouse at this address...\n");
      }
    } else {
      printf(" Did not find serial controller at: 0x%04X\n", info.base);
    }
  }
}

// Set or clear the Divisor Latch Access bit
void ser_set_dlab(const bit16u base, const bool set) {
  if (set)
    outpb(base + SER_LINE_CNTRL, inpb(base + SER_LINE_CNTRL) | 0x80);
  else
    outpb(base + SER_LINE_CNTRL, inpb(base + SER_LINE_CNTRL) & ~0x80);
}

// Write given value to port, then read back value and compare to original value.
bool ser_iotest(const bit16u port, const bit8u val) {
  outpb(port, val);
  return (inpb(port) == val);
}

// see if the controller can power down.
// returns TRUE if it can
bool ser_can_pwrdown(const bit16u base) {
  bit8u byte;
  
  ser_set_dlab(base, 0);
  outpb(base + SER_MODEM_CNTRL, 0x80);
  byte = inpb(base + SER_MODEM_CNTRL);
  outpb(base + SER_MODEM_CNTRL, 0x00);
  return ((byte & 0x80) == 0x80);
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

bool find_serial_mouse(bit16u base, char *str) {
  int i, j, timeout;
  
  // set mtype to zero = no mouse 
  memset(str, 0, STR_LEN + 1);
  
  // set baud rate 
  ser_set_dlab(base, 1);
  outpb(base + SER_DIVISOR_MSB, (SER_BAUD_1200 >> 8));
  outpb(base + SER_DIVISOR_LSB, (SER_BAUD_1200 & 0xFF));
  
  // set stop and data bits 
  ser_set_dlab(base, 0);
  outpb(base + SER_LINE_CNTRL, SER_BITS_7 | SER_PARITY_NONE | SER_STOP_1);
  
  inpb(base + SER_LINE_STATUS);         // clear error bits
  inpb(base + SER_MODEM_STATUS);        // clear error bits 
  
  outpb(base + SER_INT_ENABLE, 0x00);   // interrupt off 
  outpb(base + SER_INT_ID, 0x00);       // disable FIFO 
  
  outpb(base + SER_MODEM_CNTRL, 0x00);  // drop RTS and DTR 
  mdelay(100);                          // hold for 100 milliseconds for power to be dropped
  outpb(base + SER_MODEM_CNTRL, 0x01);  // raise DTR 
  mdelay(100);                          // hold for 100 milliseconds for power to settle
  inpb(base + SER_RECEIVE);             // flush receive buffer 
  outpb(base + SER_MODEM_CNTRL, 0x03);  // raise RTS 
  
  i = 0;
  timeout = 20; // 20 mS is sufficient
  while (timeout && (i < STR_LEN)) {
    if (inpb(base + SER_LINE_STATUS) & 0x01) {
      str[i++] = inpb(base + SER_RECEIVE) & 0x7F; // we are 7-bit reads
      timeout = 20;
      continue;
    }
    mdelay(1);
    timeout--;
  }
  
  // bytes returned
  if (i > 0) {
    printf("  [%s] (%i byte(s) returned:", str, i);
    for (j=0; j<i; j++)
      printf(" 0x%02X", str[j]);
    puts(")");
  }
  
  return (i > 0);
}

void ser_activate(const bit16u base, const bool activate) {
  if (activate) {
    info.cur_byte = -1; // start with -1 so we ignore any byte before the first byte of the packet
    info.ser_x = info.ser_y = info.ser_z = info.ser_buttons = 0;
    outpb(base + SER_MODEM_CNTRL, 0x0B);  // enable IRQ
    outpb(base + SER_INT_ENABLE, 0x01);  // interrupt on Receive Data
    // clear current buffer
    while (inpb(base + SER_LINE_STATUS) & 0x01)
      inpb(base + SER_RECEIVE);
  } else {
    outpb(base + SER_INT_ENABLE, 0x00); // disable all interrupts
    outpb(base + SER_MODEM_CNTRL, 0x00); // disable IRQ, drop RTS and DTR
  }
}

/* packets:
/// logitech
The information of the third button state is sent using one extra byte which is send after the normal packet 
when needed. Value 32 (dec) is sent every time when the center button is pressed down. It is also sent every 
time with the data packet when center button is kept down and the mouse data packet is sent for other reasons.
When center button is released, the mouse sends the normal data packet followed by data bythe which has value 0 (dec).
As you can see the extra data byte is sent only when you mess with the center button.

????? MOUSE SYSTEMS ????
1200bps, 8 databits, 1 stop-bit

   D7 D6 D5 D4 D3 D2 D1 D0
1.  1  0  0  0  0 LB CB RB
2. X7 X6 X5 X4 X3 X2 X1 X0
3. Y7 Y6 Y5 Y4 Y3 Y4 Y1 Y0
4. X7' X6' X5' X4' X3' X2' X1' X0'
5. Y7' Y6' Y5' Y4' Y3' Y4' Y1' Y0'

LB is left button state (0=pressed, 1=released)
CB is center button state (0=pressed, 1=released)
RB is right button state (0=pressed, 1=released)
X7-X0 movement in X direction since last packet in signed byte
  format (-128..+127), positive direction right
Y7-Y0 movement in Y direction since last packet in signed byte
  format (-128..+127), positive direction up
X7'-X0' movement in X direction since sending of X7-X0 packet in signed byte
  format (-128..+127), positive direction right
Y7'-Y0' movement in Y direction since sending of Y7-Y0 in signed byte
  format (-128..+127), positive direction up 
The last two bytes in the packet (bytes 4 and 5) contains information about movement data 
 changes which have occured after data butes 2 and 3 have been sent.
*/

// this gets called by the IRQ handler.
void interrupt ser_mouse_irq() {
  bit8u id, byte;
  
  id = inpb(info.base + SER_INT_ID);
  if ((id & 0x07) == 0x04) {
    byte = inpb(info.base + SER_RECEIVE);
    switch (info.packet_type) {
      case MOUSE_SERIAL_0:
        //   D7 D6 D5 D4 D3 D2 D1 D0
        //1. X   1 LB RB Y7 Y6 X7 X6
        //2. X   0 X5 X4 X3 X2 X1 X0
        //3. X   0 Y5 Y4 Y3 Y2 Y1 Y0
        byte &= 0x7F;
        if (byte & 0x40)
          info.cur_byte = 0;
        switch (info.cur_byte) {
          case -1:
            // we are ignoring anything until the first packet byte
            break;
          case 0:  // is it the first byte
            info.ser_buttons = (byte & 0x30);
            info.ser_x = (byte & 0x03) << 6;
            info.ser_y = (byte & 0x0C) << 4;
            break;
          case 1:  // second byte
            info.ser_x |= (byte & ~0xC0);
            break;
          case 2:  // third byte
            info.ser_y |= (byte & ~0xC0);
            // we have now received the third byte, so update our globals
            info.x += (bit8s) info.ser_x;
            if (info.x < 0) info.x = 0; if (info.x > 79) info.x = 79;
            info.y += (bit8s) info.ser_y;
            if (info.y < 0) info.y = 0; if (info.y > 24) info.y = 24;
            info.left  = (info.ser_buttons & 0x20) ? TRUE : FALSE;
            info.right = (info.ser_buttons & 0x10) ? TRUE : FALSE;
            break;
          default:
            ; // error: this protocol should not have more than 3 bytes
        }
        break;
      case MOUSE_SERIAL_1:
        //   D7 D6 D5 D4 D3 D2 D1 D0
        //1.  ?  ? Y8 X8  1 CB RB LB
        //2. X7 X6 X5 X4 X3 X2 X1 X0
        //3. Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
        //4. Z7 Z6 Z5 Z4 Z3 Z2 Z1 Z0
        
        // TODO:
        break;
      case MOUSE_SERIAL_2:
        
        // TODO:
        break;
    }
    
    // increment to next byte
    info.cur_byte++;
  }
  
  // acknowledge interrupt
  outp(0x20, 0x20);
}
