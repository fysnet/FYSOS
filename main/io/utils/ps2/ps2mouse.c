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
 *  PS2MOUSE.EXE
 *    This app uses a very simple form of delay which is extremely different
 *     on slower and faster machines.  Therefore, indicate on the command line
 *      -veryslow for older very slow machines (50Mhz)
 *      -slow for older slow machines (250Mhz)
 *      -fast for newer somewhat faster machines (1,500Mhz)
 *      -veryfast for newer faster machines (2,000Mhz +)
 *     You can simply give no parameter for a machine in the range of 1,000Mhz
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
 *  Compiled using MS QuickC 2.5
 *
 *  Usage:
 *    ps2mouse [-veryslow | -slow | -fast | -veryfast]
 */

#include <conio.h>
#include <dos.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"
#include "../include/pic.c"
#include "ps2share.c"

// our coordinates (middle of text screen)
int  id = -1, dx = (80 / 2), dy = (25 / 2), dz;
bool left = 0, mid = 0, right = 0, B4 = 0, B5 = 0;

#define MOUSE_BUFFER_SIZE 16
bit8u mouse_buffer[MOUSE_BUFFER_SIZE];
int mouse_buff_us = 0;   // our current position
int mouse_buff_irq = 0;  // the IRQ's current position

bit8u info0, info1, info2;
int  packet_size = 0;  // size of packet in bytes

void interrupt ps2_mouse_irq(void);
void ps2_mouse_packet(const bit8u byte);

int main(int argc, char *argv[]) {
  bit8u ports, byte;
  bool is_second = FALSE;
  bool mode = KEY_MODE_PS2;
  
  printf("Detect PS/2 Controller and Mouse.      v1.00.00\n"
         "Forever Young Software  --  Copyright 1984-2016\n\n");
  
  // for slow machines, don't count so high
  // for fast machines, delay needs to be larger counter
  // larger number needed for faster machines.
  // (cheap way to delay)
  if (argc == 2) {
    if (!strcmp(argv[1], "-veryslow"))
      time_out = 100;
    else if (!strcmp(argv[1], "-slow"))
      time_out = 2000;
    else if (!strcmp(argv[1], "-fast"))
      time_out = 12000;
    else if (!strcmp(argv[1], "-veryfast"))
      time_out = 20000;
    else
      time_out = 8000;  // default
  }
  
  // when using FreeDOS, we must disallowed its handler since
  //  it may already be active, hence reading in bytes not allowing
  //  us to read in command ACK's
  picmask(PS2MOUSEIRQ);  // make sure the IRQ is off
  picmask(PS2KEYIRQ);
  
  // detect the ports (channels)
  ports = det_ps2_port();
  if (ports == 0) {
    printf("No PS/2 Port(s) detected...\n");
    return -1;
  }
  printf("Found Type%i PS/2 port...\n", ps2_type);
  if (ports & 1)
    printf("Found first PS/2 Port and is available...\n");
  if (ports & 2) {
    printf("Found second PS/2 Port and is available...\n");
    is_second = TRUE;
  }
  
  // Get current interface.  Should return 0x00 or 0x01.
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_GET_MODE);
  byte = keyboard_read();
  if (byte & 1) {
    printf("Current interface is (MCA) PS/2\n");
    mode = KEY_MODE_PS2;
  } else {
    printf("Current interface is (ISA) AT\n");
    mode = KEY_MODE_AT;
  }
  
  // reset the device on the channel
  // after reset, we should then recieve a KEY_CMD_SELFTEST byte (and possibly an ID byte)
  if (!keyboard_cmnd60(KEY_CMD_RESET, is_second)) {
    printf("Error sending Reset Command...\n");
    goto done;
  }
  //mdelay(500); // must wait 500mS to 750mS for command to complete
  // since we use a cheap delay, let's hope this is enough.
  // your code will need a better way of delaying more accurately.
  io_delay(INT_MAX);
  if (keyboard_read() == KEY_CMD_SELFTEST) {
    // reset command might return an ID too...
    io_delay(INT_MAX);  // must wait on slower machines
    id = keyboard_read();
    if (id != KEYBOARD_ERROR)
      printf("Found a PS/2 Device with an initial id of %i\n", id);
  } else {
    printf("Did not receive KEY_CMD_SELFTEST byte\n");
    goto done;
  }
  
  // For the mouse, we should receive an ID after the reset...
  //  the id should be zero indicating a mouse
  //  (All PS/2 mice should start in default mode with an id == 0)
  if (id != 0) {
    printf("Id was not zero, unknown device attached...\n");
    goto done;
  }
  
  // get mouse status information (will be defaults after reset)
  if (keyboard_cmnd60(KEY_GET_MOUSE_INFO, is_second)) {
    info0 = keyboard_read();
    info1 = keyboard_read();
    info2 = keyboard_read();
    printf("Get Status Info returned: %02X %02X %02X\n", info0, info1, info2);
  }
  
  // Let's see if it is a Z-wheel mouse
  // Set the sample rate to 200, then 100, then 80
  //  A standard PS/2 Mouse will return an id of 0
  //  A Trackball returns an id of 2
  //  A non-Intellimouse wheel mouse will return an id of 3
  //  An Intellimouse wheel mouse will return an id of 4
  //  4D mouse return an id of 6
  //  4DPlus and Typhoon return an id of 8
  if (keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(200, is_second) &&
      keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(100, is_second) &&
      keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(80, is_second)) {
    // get mouse ID
    if (keyboard_cmnd60(KEY_CMD_ID, is_second)) {
      id = keyboard_read();
      // the device may send more than one byte of ID, so clear the buffer.
      keyboard_clear();
      // if the id == 3, try to see if it is a 5 button mouse by sending 200, 200, 80
      if (id == 3) {
        if (keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(200, is_second) &&
            keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(200, is_second) &&
            keyboard_cmnd60(KEY_CMD_TPRATE, is_second) && keyboard_cmnd60(80, is_second)) {
          // get mouse ID
          if (keyboard_cmnd60(KEY_CMD_ID, is_second)) {
            id = keyboard_read();
            // the device may send more than one byte of ID, so clear the buffer.
            keyboard_clear();
          }
        }
      }
      switch (id) {
        case 0:
          printf("Found standard PS/2 mouse...\n");
          packet_size = 3;
          break;
        case 3:
          printf("Found 3-button Wheel Mouse...\n");
          packet_size = 4;
          break;
        case 4:
          printf("Found 5-button Wheel Mouse...\n");
          packet_size = 4;
          break;
        default:
          printf("Found unknown Mouse with id: %i\n", id);
      }
    } else {
      printf("Error receiving ID byte...\n");
      goto done;
    }
  }
  printf("Packet size in bytes: %i\n", packet_size);
  
  // Set Resolution
  // 0 =  25 dpi, 1 count per millimeter
  // 1 =  50 dpi, 2 count per millimeter
  // 2 = 100 dpi, 4 count per millimeter
  // 3 = 200 dpi, 8 count per millimeter
  printf("Setting resolution...\n");
  if (!keyboard_cmnd60(KEY_SET_RES, is_second) || !keyboard_cmnd60(3, is_second))
    goto done;

  // Set Scaling to 1:1
  printf("Setting scaling...\n");
  if (!keyboard_cmnd60(KEY_SET_SCALE1, is_second))
    goto done;
  
  // Set Sample Rate
  printf("Setting sample rate...\n");
  if (!keyboard_cmnd60(KEY_CMD_TPRATE, is_second) || !keyboard_cmnd60(40, is_second))
    goto done;
  
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_KEYB);  // re-enable the keyboard
  picunmask(PS2KEYIRQ);
  
  // set up the IRQ and start catching movements
  old_isr = _dos_getvect(ivt_num[PS2MOUSEIRQ]);
  _dos_setvect(ivt_num[PS2MOUSEIRQ], ps2_mouse_irq);
  
  inpb(KEYBOARD_DATA);  // clear any byte that may be in the buffer
  keyboard_cmnd60(KEY_CMD_ENABLE, is_second);  // enable the mouse
  picunmask(PS2MOUSEIRQ);  // make sure the IRQ is on
  
  // our loop.  
  // This would be a seperate task/thread in your OS, constantly
  //  waiting for data from the mouse and processing it when it
  //  arrived.
  puts("");
  do {
    if (mouse_buff_us != mouse_buff_irq) {
      ps2_mouse_packet(mouse_buffer[mouse_buff_us]);
      if (++mouse_buff_us == MOUSE_BUFFER_SIZE)
        mouse_buff_us = 0;
    }
  } while (!kbhit());
  
  picmask(PS2MOUSEIRQ);  // make sure the IRQ is off
  keyboard_cmnd60(KEY_CMD_RESDIS_MOUSE, is_second);  // disable the mouse
  
  // clear the line and get the key used to stop the capture
  puts("                                     ");
  getch();
  
  return 0;
  
done:
  printf("Error...\n");
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_KEYB);  // re-enable the keyboard
  picunmask(PS2KEYIRQ);
  return -1;
}

// an IRQ to simply read in the byte and 
//  place it into our rotating buffer
// (this routine does not check for buffer overflow/overwrite)
void interrupt ps2_mouse_irq() {
  // place the byte into the buffer.
  mouse_buffer[mouse_buff_irq] = inpb(KEYBOARD_DATA);
  if (++mouse_buff_irq == MOUSE_BUFFER_SIZE)
    mouse_buff_irq = 0;

  // acknowledge IRQ(s)
  outpb(PIC_SLAVE, EOI);       // end of interrupt on controller
  outpb(PIC_MASTER, EOI);      // end of interrupt on controller
}

// standard packet
  // packet data:
  // Yo Xo Ys Xs  1  M  R  L
  // X7 X6 X5 X4 X3 X2 X1 X0
  // Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
// 3-button Wheel Mouse
  // packet data:
  // Yo Xo Ys Xs  1  M  R  L
  // X7 X6 X5 X4 X3 X2 X1 X0
  // Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
  // Z7 Z6 Z5 Z4 Z3 Z2 Z1 Z0
// 5-button Wheel Mouse
  // packet data:
  // Yo Xo Ys Xs  1  M  R  L
  // X7 X6 X5 X4 X3 X2 X1 X0
  // Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
  //  0  0 B5 B4 Z3 Z2 Z1 Z0
long packets = 0;
bit8u byte0 = 0, byte1 = 0, byte2 = 0, byte3 = 0;
int  cur_byte = 0;

void ps2_mouse_packet(const bit8u byte) {
  
  switch (cur_byte) {
    // all packet types have same 1st, 2nd, and 3rd bytes
    case 0:
      // the first byte should have bit 3 set.  If not, we are
      //  out of sequence, so maybe start at next byte.  Maybe.
      if (byte & 0x08) {
        byte0 = byte;    // save for byte 1 and 2 below
        left = (byte & 0x01) > 0;
        right = (byte & 0x02) > 0;
        mid = (byte & 0x04) > 0;
      } else
        cur_byte = -1;
      break;
    case 1:
      byte1 = byte;
      if (!(byte0 & 0x40)) {
        if (byte0 & 0x10)
          dx += (int) (0xFF00 | byte);
        else
          dx += (int) (bit16u) byte;
        if (dx < 0) dx = 0; if (dx > 79) dx = 79;
      }
      break;
    case 2:  
      byte2 = byte;
      if (!(byte0 & 0x80)) {
        if (byte0 & 0x20)
          dy -= (int) (0xFF00 | byte);
        else
          dy -= (int) (bit16u) byte;
        if (dy < 0) dy = 0; if (dy > 24) dy = 24;
      }
      break;
    case 3:
      byte3 = byte;
      // from here on, remaining packet size and format is specific to device
      switch (id) {
        case 3:
          dz = (int) (bit8s) byte;
          B4 = 0;
          B5 = 0;
          break;
        case 4:
          dz = (int) (bit8s) (byte & 0x0F);
          B4 = (byte & 0x10) > 0;
          B5 = (byte & 0x20) > 0;
          break;
      }
      break;
  }
  
  // increment for next byte
  cur_byte++;
  
  if (cur_byte >= packet_size) {
    cur_byte = 0;
    
    // This is where you would send the contents of the packet to
    //  your users mouse handler, or whatever you are going to do
    //  with the data
    
    // Print a line of data.  The first pair is the X & Y coordinates
    // The second three digits are set(1) or clear(0) for button presses.
    printf("%i %i %i   %i%i%i  %02X %02X %02X %02X   %li \r", dx, dy, dz, left, mid, right, byte0, byte1, byte2, byte3, packets);
    packets++;
  }
}
