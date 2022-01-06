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
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  GD_UHCI.EXE
 *   Will enumerate through the PCI, finding a UHCI, then enumerating that UHCI
 *    to see if there are any devices attached.  If so, it will display information
 *    about found device.
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS environment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *
 *  Last updated: 5 Jan 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os gd_uhci.c -o gd_uhci.exe -s
 *
 *  Usage:
 *    gd_uhci
 */


#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

#include <dpmi.h>
#include <go32.h>

#include <libc/farptrgs.h>

#include "../include/ctype.h"
#include "../include/pci.h"
#include "../include/uhci.h"
#include "../include/usb.h"

#include "gd_uhci.h"

// include the code that is common in all four utilities
#include "common.h"

int main(int argc, char *argv[]) {
  struct PCI_DEV pci_dev;
  struct PCI_POS pci_pos;
  
  // print header string
  puts("\n GD_UHCI -- UHCI: Get Device Descriptor.   v1.00.00" COPYRIGHT);
  
  // setup the timer delay code
  if (!setup_timer()) {
    puts(" I didn't find a processor with the RDTSC instruction.");
    return E_NO_RDTSC;
  }
  
  // Find a UHCI controller
  // pci_pos is the current 'address' that we are checking
  pci_pos.bus = 0;  // start at beginning
  pci_pos.dev = 0;
  pci_pos.func = 0;
  while (get_next_cntrlr(&pci_dev, &pci_pos)) {
    if (pci_dev.p_interface == xHC_TYPE_UHCI) {
      // print that we found a controller at this 'address'
      printf("  Found UHCI controller at: 0x%08X\n", pci_dev.base4 & ~0x3);
      // call the function to see if there is a device attached
      process_uhci(&pci_dev, &pci_pos);
    }
    // increment the function for next time.
    pci_pos.func++;
  }
  
  return 0;
}

// reset the host controller, then see if anything attached.
// if device attached, get device descriptor
bool process_uhci(struct PCI_DEV *pci_dev, struct PCI_POS *pos) {
  
  // The UHCI controller uses the dword at base4 and is port_io access
  const bit16u base = (bit16u) (pci_dev->base4 & ~0x0003);
  
  // allow access to data (port IO)
  pci_write_word(pos->bus, pos->dev, pos->func, 0x04, 0x0005);
  
  __dpmi_meminfo frame_mi_base;
  int selector, i, dev_address = 1;
  struct DEVICE_DESC dev_desc;
  
  // do a global reset (5 times 10ms each)
  for (i=0; i<5; i++) {
    outpw(base+UHCI_COMMAND, 0x0004);
    mdelay(USB_TDRST);
    outpw(base+UHCI_COMMAND, 0x0000);
  }
  mdelay(USB_TRSTRCY);
  
  // does the command register contain its default value of 0x0000 ?
  if (inpw(base+UHCI_COMMAND) != 0x0000) return FALSE;
  // does the status register contain its default value of 0x0020 ?  
  if (inpw(base+UHCI_STATUS) != 0x0020) return FALSE;
  // The status register is write clear, let's clear it out.
  outpw(base+UHCI_STATUS, 0x00FF);
  // does the SOF register contain its default value of 0x40  
  if (inpb(base+UHCI_SOF_MOD) != 0x40) return FALSE;
  
  // if we set bit 1 in the Command register, after a specified time the controller should reset it to 0.
  // In my tests, the UHCI controller takes less than 2uS to reset the bit
  outpw(base+UHCI_COMMAND, 0x0002);
  mdelay(10);
  if (inpw(base+UHCI_COMMAND) & 0x0002) return FALSE;
  
  // if we get here, we have a valid UHCI controller, so set it up
  
  // set up the memory access
  frame_mi_base.address = 0x01000000;
  frame_mi_base.size = 4096 + 4096;
  if (get_physical_mapping(&frame_mi_base, &selector) == -1) {
    puts(" Error 'allocating' physical memory.");
    return FALSE;
  }
  
  // set up an empty stack frame
  for (i=0; i<4096; i+=4)
    _farpokel(selector, i, 0x00000001);
  
  // set the Host Controllers schedule
  outpd(base+UHCI_FRAME_BASE, frame_mi_base.address); // physical address
  outpw(base+UHCI_FRAME_NUM, 0);                // start at frame 0
  outpb(base+UHCI_SOF_MOD, 0x40);               // start of frame to default
  outpw(base+UHCI_INTERRUPT, 0x0000);           // disallow error interrupts
  outpw(base+UHCI_STATUS, 0x001F);              // Clear any status bits.
  outpw(base+UHCI_COMMAND, (1<<7) | (1<<6) | (1<<0));
  
  bit8u port = 0x10;
  while (uhci_port_pres(base, port)) {
    // reset the port
    if (uhci_port_reset(base, port)) {
      // is a device is attached?
      if (inpw(base+port) & 1) {
        printf(" Found device at port %i, getting descriptor...\n", (port - 0x10) >> 1);
        bool ls_device = (inpw(base+port) & (1<<8)) ? TRUE : FALSE;
        
        // get first 8 bytes of descriptor
        if (uhci_get_descriptor(base, frame_mi_base.address, selector, &dev_desc, ls_device, 0, 8, 8)) {
          // reset the port again
          uhci_port_reset(base, port);
          // set address of device
          if (uhci_set_address(base, frame_mi_base.address, selector, dev_address, ls_device)) {
            // get all 18 bytes of descriptor
            if (uhci_get_descriptor(base, frame_mi_base.address, selector, &dev_desc, ls_device, dev_address, dev_desc.max_packet_size, dev_desc.len)) {
              printf("  Found Device Descriptor:\n"
                     "                 len: %i\n"
                     "                type: %i\n"
                     "             version: %01X.%02X\n"
                     "               class: %i\n"
                     "            subclass: %i\n"
                     "            protocol: %i\n"
                     "     max packet size: %i\n"
                     "           vendor id: 0x%04X\n"
                     "          product id: 0x%04X\n"
                     "         release ver: %i%i.%i%i\n"
                     "   manufacture index: %i (index to a string)\n"
                     "       product index: %i\n"
                     "        serial index: %i\n"
                     "   number of configs: %i\n",
                     dev_desc.len, dev_desc.type, dev_desc.usb_ver >> 8, dev_desc.usb_ver & 0xFF, dev_desc._class, dev_desc.subclass, 
                     dev_desc.protocol, dev_desc.max_packet_size, dev_desc.vendorid, dev_desc.productid, 
                     (dev_desc.device_rel & 0xF000) >> 12, (dev_desc.device_rel & 0x0F00) >> 8,
                     (dev_desc.device_rel & 0x00F0) >> 4,  (dev_desc.device_rel & 0x000F) >> 0,
                     dev_desc.manuf_indx, dev_desc.prod_indx, dev_desc.serial_indx, dev_desc.configs
              );
              dev_address++;
            } else
              printf(" Error when trying to get all %i bytes of descriptor.\n", dev_desc.len);
          } else
            puts(" Error setting device address.");
        } else
          puts(" Error getting first 8 bytes of descriptor.");
      }
    }
    port += 2;  // move to next port
  }
  
  // stop the controller  
  outpw(base+UHCI_COMMAND, 0x00000000);
  while (!(inpw(base+UHCI_STATUS) & (1<<5)))
    ; // wait for the HCHalted bit to be set
  
  // free the "allocated" memory
  __dpmi_free_physical_address_mapping(&frame_mi_base);
  
  return 0;
}

// See if there is a valid UHCI port at address base+port
// Bit 7 set = Always set.
// See if we can clear it.
bool uhci_port_pres(bit16u base, bit8u port) {
  
  // if bit 7 is 0, not a port
  if ((inpw(base+port) & 0x0080) == 0) return FALSE;

  // try to clear it
  outpw(base+port, inpw(base+port) & ~0x0080);
  if ((inpw(base+port) & 0x0080) == 0) return FALSE;

  // try to write/clear it
  outpw(base+port, inpw(base+port) | 0x0080);
  if ((inpw(base+port) & 0x0080) == 0) return FALSE;

  // let's see if we write a 1 to bits 3:1, if they come back as zero
  outpw(base+port, inpw(base+port) | 0x000A);
  if ((inpw(base+port) & 0x000A) != 0) return FALSE;
  
  // we should be able to assume this is a valid port if we get here
  return TRUE;
}

bool uhci_port_reset(bit16u base, bit8u port) {
  int i;
  bit16u val = 0;
  bool ret = FALSE;
  
  outpw(base+port, inpw(base + port) | (1<<9));
  mdelay(USB_TDRSTR);
  outpw(base + port, inpw(base+port) & ~(1<<9));
  
  for (i=0; i<10; i++) {
    mdelay(USB_TRSTRCY);  // hold for USB_TRSTRCY ms (reset recovery time)
    
    val = inpw(base + port);
    
    // if bit 0 is clear, nothing attached, don't enable
    if (!(val & (1<<0))) {
      ret = TRUE;
      break;
    }
    
    // if either enable_change or connection_change, clear them and continue.
    if (val & ((1<<3) | (1<<1))) {
      outpw(base + port, val & UHCI_PORT_WRITE_MASK);
      continue;
    }
    
    // if the enable bit is set, break.
    if (val & (1<<2)) {
      ret = TRUE;
      break;
    }
    
    // else, set the enable bit
    outpw(base + port, val | (1<<2));
  }
  
  return ret;
}

// set up a queue, and enough TD's to get 'size' bytes
bool uhci_get_descriptor(const bit16u io_base, const bit32u base, const int selector, struct DEVICE_DESC *dev_desc, const bool ls_device, const int dev_address, const int packet_size, const int size) {
  static bit8u setup_packet[8] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
  bit8u our_buff[120];
  int i = 1, t, sz = size, timeout;
  
  struct UHCI_QUEUE_HEAD queue;
  struct UHCI_TRANSFER_DESCRIPTOR td[10];
  
  // set the size of the packet to return
  * ((bit16u *) &setup_packet[6]) = (bit16u) size;
  
  memset(our_buff, 0, 120);
  
  queue.horz_ptr = 0x00000001;
  queue.vert_ptr = (base + 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD));  // 128 to skip past buffers
  
  td[0].link_ptr = ((queue.vert_ptr & ~0xF) + sizeof(struct UHCI_TRANSFER_DESCRIPTOR));
  td[0].reply = (ls_device ? (1<<26) : 0) | (3<<27) | (0x80 << 16);
  td[0].info = (7<<21) | ((dev_address & 0x7F)<<8) | TOKEN_SETUP;
  td[0].buff_ptr = base + 4096;
  
  while ((sz > 0) && (i<9)) {
    td[i].link_ptr = ((td[i-1].link_ptr & ~0xF) + sizeof(struct UHCI_TRANSFER_DESCRIPTOR));
    td[i].reply = (ls_device ? (1<<26) : 0) | (3<<27) | (0x80 << 16);
    t = ((sz <= packet_size) ? sz : packet_size);
    td[i].info = ((t-1)<<21) | ((i & 1) ? (1<<19) : 0) | ((dev_address & 0x7F)<<8) | TOKEN_IN;
    td[i].buff_ptr = base + 4096 + (8 * i);
    sz -= t;
    i++;
  }
  
  td[i].link_ptr = 0x00000001;
  td[i].reply = (ls_device ? (1<<26) : 0) | (3<<27) | (1<<24) | (0x80 << 16);
  td[i].info = (0x7FF<<21) | (1<<19) | ((dev_address & 0x7F)<<8) | TOKEN_OUT;
  td[i].buff_ptr = 0x00000000;
  i++; // for a total count
  
  // make sure status:int bit is clear
  outpw(io_base+UHCI_STATUS, 1);
  
  // now move our queue into the physical memory allocated
  movedata(_my_ds(), (bit32u) setup_packet, selector, 4096 + 0, 8);
  movedata(_my_ds(), (bit32u) our_buff, selector, 4096 + 8, 120);
  movedata(_my_ds(), (bit32u) &queue, selector, 4096 + 128, sizeof(struct UHCI_QUEUE_HEAD));
  movedata(_my_ds(), (bit32u) &td[0], selector, 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD), sizeof(struct UHCI_TRANSFER_DESCRIPTOR) * 10);
  
  // mark the first stack frame pointer
  _farpokel(selector, 0, (base + 4096 + 128) | QUEUE_HEAD_Q);
  
  // wait for the IOC to happen
  timeout = 10000; // 10 seconds
  while (!(inpw(io_base+UHCI_STATUS) & 1) && (timeout > 0)) {
    timeout--;
    mdelay(1);
  }
  if (timeout == 0) {
    puts(" uhci_get_descriptor:UHCI timed out...");
    _farpokel(selector, 0, 1);  // mark the first stack frame pointer invalid
    return FALSE;
  }
  outpw(io_base+UHCI_STATUS, 1);  // acknowledge the interrupt
  
  _farpokel(selector, 0, 1);  // mark the first stack frame pointer invalid
  
  // copy the stack frame back to our local buffer
  movedata(selector, 4096+8, _my_ds(), (bit32u) our_buff, 120);
  movedata(selector, 4096 + 128, _my_ds(), (bit32u) &queue, sizeof(struct UHCI_QUEUE_HEAD));
  movedata(selector, 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD), _my_ds(), (bit32u) &td[0], (sizeof(struct UHCI_TRANSFER_DESCRIPTOR) * 10));
  
  // check the TD's for error
  for (t=0; t<i; t++) {
    if (((td[t].reply & (0xFF<<16)) != 0)) {
      printf(" Found Error in TD #%i: 0x%08X\n", t, td[t].reply);
      return FALSE;
    }
  }
  
  // copy the descriptor to the passed memory block
  memcpy(dev_desc, our_buff, size);
  
  return TRUE;
}

bool uhci_set_address(const bit16u io_base, const bit32u base, const int selector, const int dev_address, const bool ls_device) {
  
  // our setup packet (with the third byte replaced below)
  static bit8u setup_packet[8] = { 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  int i, timeout;
  
  // one queue and two TD's
  struct UHCI_QUEUE_HEAD queue;
  struct UHCI_TRANSFER_DESCRIPTOR td[2];
  
  setup_packet[2] = (bit8u) dev_address;
  
  queue.horz_ptr = 0x00000001;
  queue.vert_ptr = (base + 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD));  // 128 to skip past buffers
  
  td[0].link_ptr = ((queue.vert_ptr & ~0xF) + sizeof(struct UHCI_TRANSFER_DESCRIPTOR));
  td[0].reply = (ls_device ? (1<<26) : 0) | (3<<27) | (0x80 << 16);
  td[0].info = (7<<21) | (0<<8) | TOKEN_SETUP;
  td[0].buff_ptr = base + 4096;
  
  td[1].link_ptr = 0x00000001;
  td[1].reply = (ls_device ? (1<<26) : 0) | (3<<27) | (1<<24) | (0x80 << 16);
  td[1].info = (0x7FF<<21) | (1<<19) | (0<<8) | TOKEN_IN;
  td[1].buff_ptr = 0x00000000;
  
  // make sure status:int bit is clear
  outpw(io_base+UHCI_STATUS, 1);
  
  // now move our queue into the physicall memory allocated
  movedata(_my_ds(), (bit32u) setup_packet, selector, 4096, 8);
  movedata(_my_ds(), (bit32u) &queue, selector, 4096 + 128, sizeof(struct UHCI_QUEUE_HEAD));
  movedata(_my_ds(), (bit32u) &td[0], selector, 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD), (sizeof(struct UHCI_TRANSFER_DESCRIPTOR) * 2));
  
  // mark the first stack frame pointer
  _farpokel(selector, 0, (base + 4096 + 128) | QUEUE_HEAD_Q);
  
  // wait for the IOC to happen
  timeout = 10000; // 10 seconds
  while (!(inpw(io_base+UHCI_STATUS) & 1) && (timeout > 0)) {
    timeout--;
    mdelay(1);
  }
  if (timeout == 0) {
    puts(" uhci_set_address:UHCI timed out...");
    _farpokel(selector, 0, 1);  // mark the first stack frame pointer invalid
    return FALSE;
  }
  outpw(io_base+UHCI_STATUS, 1);  // acknowledge the interrupt
  
  _farpokel(selector, 0, 1);  // mark the first stack frame pointer invalid
  
  // copy the stack frame back to our local buffer
  movedata(selector, 4096 + 128, _my_ds(), (bit32u) &queue, sizeof(struct UHCI_QUEUE_HEAD));
  movedata(selector, 4096 + 128 + sizeof(struct UHCI_QUEUE_HEAD), _my_ds(), (bit32u) &td[0], (sizeof(struct UHCI_TRANSFER_DESCRIPTOR) * 2));
  
  // check the TD's for error
  for (i=0; i<2; i++) {
    if ((td[i].reply & (0xFF<<16)) != 0) {
      printf(" Found Error in TD #%i: 0x%08X\n", i, td[i].reply);
      return FALSE;
    }
  }
  
  return TRUE;
}
