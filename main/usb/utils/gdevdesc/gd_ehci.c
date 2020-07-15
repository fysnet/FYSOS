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
 *  GD_EHCI.EXE
 *   Will enumerate through the PCI, finding a EHCI, then enumerating that EHCI
 *    to see if there are any devices attached.  If so, it will display information
 *    about found device.
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS envirnment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *   - This code assumes the attached device is a high-speed device.  If a full-
 *     or low-speed device is attached, the device will not be found by this code.
 *     Use GD_UHCI or GD_OHCI for full- and low-speed devices.
 *
 *  Last updated: 14 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os gd_ehci.c -o gd_ehci.exe -s
 *
 *  Usage:
 *    gd_ehci
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <dpmi.h>
#include <go32.h>

#include <libc/farptrgs.h>

#include "../include/ctype.h"
#include "../include/pci.h"
#include "../include/ehci.h"
#include "../include/usb.h"

#include "gd_ehci.h"

// operational registers
__dpmi_meminfo base_mi;
int base_selector;

// The Memory Heap
__dpmi_meminfo heap_mi;
int heap_selector;
bit32u cur_heap_ptr;

bit32u async_base;

bit32u op_base_off, hccparams, hcsparams;
bit8u num_ports;

// include the code that is common in all four utilities
#include "common.h"

int main(int argc, char *argv[]) {
  struct PCI_DEV pci_dev;
  struct PCI_POS pci_pos;
  
  // print header string
  printf("\n GD_EHCI -- EHCI: Get Device Descriptor.   v1.10.00" COPYRIGHT);
  
  // setup the timer delay code
  if (!setup_timer()) {
    printf("\n I didn't find a processor with the RDTSC instruction.");
    return E_NO_RDTSC;
  }
  
  // Find a USB controller
  // pci_pos is the current 'address' that we are checking
  pci_pos.bus = 0;  // start at beginning
  pci_pos.dev = 0;
  pci_pos.func = 0;
  while (get_next_cntrlr(&pci_dev, &pci_pos)) {
    if (pci_dev.p_interface == xHC_TYPE_EHCI) {
      // print that we found a controller at this 'address'
      printf("\n  Found EHCI controller at: 0x%08X", pci_dev.base0 & ~0xF);
      // call the function to see if there is a device attached
      process_ehci(&pci_dev, &pci_pos);
    }
    // increment the function for next time.
    pci_pos.func++;
  }
  
  printf("\n");
  return 0;
}

// reset the controller, give control of all ports to the companion controllers.
bool process_ehci(struct PCI_DEV *pci_dev, struct PCI_POS *pos) {
  int timeout, i;
  
  // allow access to data (memmapped IO)
  pci_write_word(pos->bus, pos->dev, pos->func, 0x04, 0x0006);
  
  // set up the memory access
  // The EHCI controller uses the dword at base0 and is memmapped access
  base_mi.address = pci_dev->base0 & ~0xF;
  base_mi.size = 1024;
  if (get_physical_mapping(&base_mi, &base_selector) == -1) {
    printf("\n Error 'allocating' physical memory.");
    return FALSE;
  }
  
  // create a heap of memory for us to use
  cur_heap_ptr = 0;          // start at the beginning of our memory heap
  heap_mi.address = HEAP_START;
  heap_mi.size = HEAP_SIZE;
  if (!get_physical_mapping(&heap_mi, &heap_selector)) {
    printf("\n Error 'allocating' physical memory for our heap.");
    __dpmi_free_physical_address_mapping(&base_mi);
    return FALSE;
  }
  
  // calculate the operational base
  // must be before any ehci_read_op_reg() or ehci_write_op_reg() calls
  op_base_off = (bit32u) _farpeekb(base_selector, EHC_CAPS_CapLength);
  
  // reset the controller, returning false after 50mS if it doesn't reset
  timeout = 50;
  ehci_write_op_reg(EHC_OPS_USBCommand, (1<<1));
  while (ehci_read_op_reg(EHC_OPS_USBCommand) & (1<<1)) {
    mdelay(1);
    if (--timeout == 0)
      return FALSE;
  }
  
  /*
   * if we get here, we have a valid EHCI controller, so set it up.
   */
  
  // store the read only registers
  hcsparams = ehci_read_cap_reg(EHC_CAPS_HCSParams);
  hccparams = ehci_read_cap_reg(EHC_CAPS_HCCParams);
  
  // Turn off legacy support for Keyboard and Mice
  if (!ehci_stop_legacy(pos, hccparams)) {
    printf("\n BIOS did not release Legacy support...");
    return FALSE;
  }
  
  // get num_ports from EHCI's HCSPARAMS register
  num_ports = (bit8u) (hcsparams & 0x0F);  // at least 1 and no more than 15
  printf("\n  Found %i root hub ports.", num_ports);
  
  // allocate then initialize the async queue list (Control and Bulk TD's)
  async_base = heap_alloc(16 * EHCI_QUEUE_HEAD_SIZE, 32);
  ehci_init_stack_frame(async_base);
  
  // set and start the Host Controllers schedule
  if ((hccparams & (1<<0)) == 1)
    ehci_write_op_reg(EHC_OPS_CtrlDSSegemnt, 0x00000000);   // we use only 32-bit addresses
  ehci_write_op_reg(EHC_OPS_PeriodicListBase, 0x00000000);  // physical address
  ehci_write_op_reg(EHC_OPS_AsyncListBase, async_base);     // physical address
  ehci_write_op_reg(EHC_OPS_FrameIndex, 0);                 // start at (micro)frame 0
  ehci_write_op_reg(EHC_OPS_USBInterrupt, 0);               // disallow interrupts
  ehci_write_op_reg(EHC_OPS_USBStatus, 0x3F);               // clear any pending interrupts
  
  // start the host controller: 8 micro-frames, start schedule (frame list size = 1024)
  ehci_write_op_reg(EHC_OPS_USBCommand, (8<<16) | (1<<0));
  
  // enable the asynchronous list
  if (!ehci_enable_async_list(TRUE)) {
    printf("\n Did not enable the Ascynchronous List");
    return FALSE;
  }
  
  // Setting bit 0 in the ConfigFlags reg tells all ports to use the EHCI controller.
  ehci_write_op_reg(EHC_OPS_ConfigFlag, 1);
  
  // if we have control to change the port power, we need to power each port to 1
  if (hcsparams & (1<<4))
    for (i=0; i<num_ports; i++)
      ehci_write_op_reg(EHC_OPS_PortStatus + (i * 4), ehci_read_op_reg(EHC_OPS_PortStatus + (i * 4)) | EHCI_PORT_PP);
  
  // after powering a port, we must wait 20mS before using it.
  mdelay(20);
  
  // we should be ready to detect any ports that are occupied
  for (i=0; i<num_ports; i++) {
    // power and reset the port
    if (ehci_reset_port(i))
      // if the reset was good, get the descriptor
      ehci_get_descriptor(i);
  }
  
  // stop the controller  
  ehci_write_op_reg(EHC_OPS_USBCommand, 0x00000000);
  
  // free the "allocated" memory, and opregs selector
  __dpmi_free_physical_address_mapping(&heap_mi);
  __dpmi_free_physical_address_mapping(&base_mi);
  
  return 0;
}

bool ehci_reset_port(const int port) {
  bool ret = FALSE;
  bit32u HCPortStatusOff = EHC_OPS_PortStatus + (port * 4);
  bit32u dword;
  
  // Clear the enable bit and status change bits (making sure the PP is set)
  ehci_write_op_reg(HCPortStatusOff, EHCI_PORT_PP | EHCI_PORT_OVER_CUR_C | EHCI_PORT_ENABLE_C | EHCI_PORT_CSC);
  
  // read the port and see if a device is attached
  // if device attached and is a hs device, the controller will set the enable bit.
  // if the enable bit is not set, then there was an error or it is a low- or full-speed device.
  // if bits 11:10 = 01b, then it isn't a high speed device anyway, skip the reset.
  dword = ehci_read_op_reg(HCPortStatusOff);
  if ((dword & EHCI_PORT_CCS) && (((dword & EHCI_PORT_LINE_STATUS) >> 10) != 0x01)) {
    // set bit 8 (writing a zero to bit 2)
    ehci_write_op_reg(HCPortStatusOff, EHCI_PORT_PP | EHCI_PORT_RESET);
    mdelay(USB_TDRSTR);  // at least 50 ms for a root hub
    
    // clear the reset bit leaving the power bit set
    ehci_write_op_reg(HCPortStatusOff, EHCI_PORT_PP);
    mdelay(USB_TRSTRCY);
  }
  
  dword = ehci_read_op_reg(HCPortStatusOff);
  if (dword & EHCI_PORT_CCS) {
    // if after the reset, the enable bit is set, we have a high-speed device
    if (dword & EHCI_PORT_ENABLED) {
      // Found a high-speed device.
      // clear the status change bit(s)
      ehci_write_op_reg(HCPortStatusOff, ehci_read_op_reg(HCPortStatusOff) & EHCI_PORT_WRITE_MASK);
      
      return TRUE;
    } else {
      printf("\nFound a low- or full-speed device.  Use gd_ohci or gd_uhci.");
      // disable and power off the port
      ehci_write_op_reg(HCPortStatusOff, 0);
      mdelay(10);
      
      // the next two lines are not necassary for this utility, but they remain included
      //  to show what you would need to do to release ownership of the port.
      ehci_write_op_reg(HCPortStatusOff, EHCI_PORT_OWNER);
      // wait for the owner bit to actually be set, and the ccs bit to clear
      ehci_handshake(HCPortStatusOff, (EHCI_PORT_OWNER | EHCI_PORT_CCS), EHCI_PORT_OWNER, 25);
    }
  }
  
  return FALSE;
}

bool ehci_get_descriptor(const int port) {
  
  struct DEVICE_DESC dev_desc;
  
  /*
   * Since most high-speed devices will only work with a max packet size of 64,
   *  we don't request the first 8 bytes, then set the address, and request
   *  the all 18 bytes like the uhci/ohci controllers.  However, I have included
   *  the code below just to show how it could be done.
   */
  
  bit8u max_packet = 64;
  bit8u dev_address = 1;
  
  // send the "get_descriptor" packet (get 18 bytes)
  if (!ehci_control_in(&dev_desc, 18, max_packet, 0))
    return FALSE;
  
  // reset the port
  ehci_reset_port(port);
  
  // set address
  ehci_set_address(max_packet, dev_address);
  
  // get the whole packet.
  memset(&dev_desc, 0, 18);
  if (!ehci_control_in(&dev_desc, 18, max_packet, dev_address))
    return FALSE;
  
  // print the descriptor
  printf("\n  Found Device Descriptor:"
         "\n                 len: %i"
         "\n                type: %i"
         "\n             version: %01X.%02X"
         "\n               class: %i"
         "\n            subclass: %i"
         "\n            protocol: %i"
         "\n     max packet size: %i"
         "\n           vendor id: 0x%04X"
         "\n          product id: 0x%04X"
         "\n         release ver: %i%i.%i%i"
         "\n   manufacture index: %i (index to a string)"
         "\n       product index: %i"
         "\n        serial index: %i"
         "\n   number of configs: %i",
         dev_desc.len, dev_desc.type, dev_desc.usb_ver >> 8, dev_desc.usb_ver & 0xFF, dev_desc._class, dev_desc.subclass, 
         dev_desc.protocol, dev_desc.max_packet_size, dev_desc.vendorid, dev_desc.productid, 
         (dev_desc.device_rel & 0xF000) >> 12, (dev_desc.device_rel & 0x0F00) >> 8,
         (dev_desc.device_rel & 0x00F0) >> 4,  (dev_desc.device_rel & 0x000F) >> 0,
         dev_desc.manuf_indx, dev_desc.prod_indx, dev_desc.serial_indx, dev_desc.configs
  );
  
  return TRUE;
}

/* allocates some memory in our heap on the alignment specified, rouding up to the nearest dword
 * Checks for errors, but does not return an error.  Simply "exits" if error found
 * will clear the memory to zero
 * returns physical address of memory found
 * alignment and boundary must be a power of 2
 */
bit32u heap_alloc(bit32u size, const bit32u alignment) {
  bit32u i;
  
  // align to the next alignment
  cur_heap_ptr = (cur_heap_ptr + (alignment - 1)) & ~(alignment - 1);
  
  // round up to the next dword size
  size = (size + 3) & ~3;
  
  // check to see if we are out of bounds
  if (((cur_heap_ptr + size) - 1) >= HEAP_SIZE) {
    printf("\n Error in allocating memory within our heap");
    exit(-1);
  }
  
  // calculate physical address
  bit32u addr = heap_mi.address + cur_heap_ptr;
  
  // clear it to zeros
  ehci_clear_phy_mem(addr, size);
  
  // update our pointer for next time
  cur_heap_ptr += size;
  
  // return the physical address
  return addr;
}

// enable/disable one of the lists.
// if the async member is set, it disables/enables the asynchronous list, else the periodic list
bool ehci_enable_async_list(const bool enable) {
  bit32u command;
  
  // first make sure that both bits are the same
  // should not modify the enable bit unless the status bit has the same value
  command = ehci_read_op_reg(EHC_OPS_USBCommand);
  if (ehci_handshake(EHC_OPS_USBStatus, (1<<15), (command & (1<<5)) ? (1<<15) : 0, 100)) {
    if (enable) {
      if (!(command & (1<<5)))
        ehci_write_op_reg(EHC_OPS_USBCommand, command | (1<<5));
      return ehci_handshake(EHC_OPS_USBStatus, (1<<15), (1<<15), 100);
    } else {
      if (command & (1<<5))
        ehci_write_op_reg(EHC_OPS_USBCommand, command & ~(1<<5));
      return ehci_handshake(EHC_OPS_USBStatus, (1<<15), 0, 100);
    }
  }
  
  return FALSE;
}

/* This routine waits for the value read at (base, reg) and'ed by mask to equal result.
 * It returns TRUE if this happens before the alloted time expires
 * returns FALSE if this does not happen
 */
bool ehci_handshake(const bit32u reg, const bit32u mask, const bit32u result, unsigned ms) {
  do {
    if ((ehci_read_op_reg(reg) & mask) == result)
      return TRUE;
    
    mdelay(1);
  } while (--ms);
  
  return FALSE;
}

// initialize the async queue list (Control and Bulk TD's)
void ehci_init_stack_frame(const bit32u async_base) {
  int i;
  bit32u cur_addr = async_base;
  
  // the async queue (Control and Bulk TD's) is a round robin set of 16 Queue Heads.
  for (i=0; i<16; i++) {
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_HORZ_PTR, (cur_addr + EHCI_QUEUE_HEAD_SIZE) | QH_HS_TYPE_QH | QH_HS_T0);
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_ENDPT_CAPS, (0 << 16) | ((i==0) ? (1<<15) : (0<<15)) | QH_HS_EPS_HS | (0<<8) | 0);
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_HUB_INFO, (1<<30));
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_NEXT_QTD_PTR, QH_HS_T1);
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_ALT_NEXT_QTD_PTR, QH_HS_T1);
    cur_addr += EHCI_QUEUE_HEAD_SIZE;
  }
  
  // backup and point the last one at the first one
  cur_addr -= EHCI_QUEUE_HEAD_SIZE;
  ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_HORZ_PTR, (async_base | QH_HS_TYPE_QH | QH_HS_T0));
}

/* Release BIOS ownership of controller
 * On Entry:
 *      pci: pointer to the pci config space we read in
 *   params: the dword value of the capability register
 * On Return:
 *   TRUE if ownership released
 *
 * Set bit 24 to indicate to the BIOS to release ownership
 * The BIOS should clear bit 16 indicating that it has successfully done so
 * Ownership is released when bit 24 is set *and* bit 16 is clear.
 * This will wait EHC_LEGACY_TIMEOUT ms for the BIOS to release ownership.
 *   (It is unknown the exact time limit that the BIOS has to release ownership.)
 */
bool ehci_stop_legacy(const struct PCI_POS *pos, const bit32u params) {
  const bit8u eecp = (bit8u) ((params & 0x0000FF00) >> 8);
  
  if (eecp >= 0x40) {
    
    // set bit 24 asking the BIOS to release ownership
    pci_write_dword(pos->bus, pos->dev, pos->func, eecp + EHC_LEGACY_USBLEGSUP, 
      (pci_read_dword(pos->bus, pos->dev, pos->func, eecp + EHC_LEGACY_USBLEGSUP) | EHC_LEGACY_OS_OWNED));
    
    // Timeout if bit 24 is not set and bit 16 is not clear after EHC_LEGACY_TIMEOUT milliseconds
    int timeout = EHC_LEGACY_TIMEOUT;
    while (timeout--) {
      if ((pci_read_dword(pos->bus, pos->dev, pos->func, eecp + EHC_LEGACY_USBLEGSUP) & EHC_LEGACY_OWNED_MASK) == EHC_LEGACY_OS_OWNED)
        return TRUE;
      mdelay(1);
    }
    
    return FALSE;
  } else
    return TRUE;
}

bool ehci_set_address(const bit8u max_packet, const bit8u address) {
  
  // allocate enough memory to hold the out_packet, queue and the TD's
  bit32u packet = heap_alloc(64 + EHCI_QUEUE_HEAD_SIZE + (2 * EHCI_TD_SIZE), 64);
  bit32u queue = packet + 64;
  bit32u td0 = queue + EHCI_QUEUE_HEAD_SIZE;
  
  static bit8u temp[8] = { 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  temp[2] = address;
  
  ehci_copy_to_phy_mem(packet, temp, sizeof(struct REQUEST_PACKET));
  
  ehci_queue(queue, td0, 0, max_packet, 0);
  ehci_setup_packet(td0, packet);
  ehci_packet(td0 + EHCI_TD_SIZE, 0, 0, 0, TRUE, 1, EHCI_TD_PID_IN, max_packet);
  
  ehci_insert_queue(queue, QH_HS_TYPE_QH);
  int ret = ehci_wait_interrupt(td0, 2000, NULL);
  ehci_remove_queue(queue);
  
  return (ret == SUCCESS);
}

bool ehci_control_in(void *targ, const int len, const int max_packet, const bit8u address) {
  
  // allocate enough memory to hold the packet, queue, and the TD's
  bit32u packet = heap_alloc(64 + EHCI_QUEUE_HEAD_SIZE + (16 * EHCI_TD_SIZE), 64);
  bit32u queue = packet + 64;
  bit32u td0 = queue + EHCI_QUEUE_HEAD_SIZE;
  
  bit32u buffer_addr = heap_alloc(256, 1);  // get a physical address buffer and then copy from it later
  
  static struct REQUEST_PACKET temp = { STDRD_GET_REQUEST, GET_DESCRIPTOR, ((DEVICE << 8) | 0), 0, 0 };
  temp.length = len;
  
  // copy the request packet from local memory to physical address memory
  ehci_copy_to_phy_mem(packet, &temp, sizeof(struct REQUEST_PACKET));
  
  bool spd = 0;
  const int last = 1 + ((len + (max_packet-1)) / max_packet);
  
  ehci_queue(queue, td0, 0, max_packet, address);
  ehci_setup_packet(td0, packet);
  ehci_packet(td0 + EHCI_TD_SIZE, td0 + (last * EHCI_TD_SIZE), buffer_addr, len, FALSE, 1, EHCI_TD_PID_IN, max_packet);
  ehci_packet(td0 + (last * EHCI_TD_SIZE), NULL, NULL, 0, TRUE, 1, EHCI_TD_PID_OUT, max_packet);
  
  ehci_insert_queue(queue, QH_HS_TYPE_QH);
  int ret = ehci_wait_interrupt(td0, 2000, &spd);
  ehci_remove_queue(queue);
  
  if (ret == SUCCESS) {
    // now copy from the physical buffer to the specified buffer
    ehci_copy_from_phy_mem(targ, buffer_addr, len);
    return TRUE;
  }
  
  return FALSE;
}

void ehci_queue(bit32u addr, const bit32u qtd, bit8u endpt, const bit16u mps, const bit8u address) {
  int i;
  
  // clear it to zeros
  ehci_clear_phy_mem(addr, EHCI_QUEUE_HEAD_SIZE);
  
  ehci_write_phy_mem(addr + EHCI_QH_OFF_HORZ_PTR, 1);
  ehci_write_phy_mem(addr + EHCI_QH_OFF_ENDPT_CAPS, (8<<28) | ((mps & 0x7FF) << 16) | (0<<15) | 
    (1<<14) | (2<<12) | ((endpt & 0x0F) << 8) | (0<<7) | (address & 0x7F));
  ehci_write_phy_mem(addr + EHCI_QH_OFF_HUB_INFO, (1<<30) | (0<<23) | (0<<16));
  ehci_write_phy_mem(addr + EHCI_QH_OFF_NEXT_QTD_PTR, qtd);
}

int ehci_setup_packet(const bit32u addr, bit32u request) {
  int i;
  
  // clear it to zeros
  ehci_clear_phy_mem(addr, EHCI_TD_SIZE);
  
  ehci_write_phy_mem(addr + EHCI_TD_OFF_NEXT_TD_PTR, addr + EHCI_TD_SIZE);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_ALT_NEXT_QTD_PTR, QH_HS_T1);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_STATUS, (0<<31) | (8<<16) | (0<<15) | (0<<12) | (3<<10) |
    (EHCI_TD_PID_SETUP<<8) | 0x80);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_BUFF0_PTR, request);
  request = (request + 0x1000) & ~0x0FFF;
  ehci_write_phy_mem(addr + EHCI_TD_OFF_BUFF1_PTR, request);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_BUFF2_PTR, request + 0x1000);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_BUFF3_PTR, request + 0x2000);
  ehci_write_phy_mem(addr + EHCI_TD_OFF_BUFF4_PTR, request + 0x3000);
  
  return 1;
}

int ehci_packet(bit32u addr, const bit32u status_qtd, bit32u buffer, const bit32u size, const bool last, 
                bit8u data0, const bit8u dir, const bit16u mps) {

  int cnt = 0, i;
  int sz = size;
  int max_size = (0x1000 - (addr & 0x0FFF)) + (4 * 0x1000);
  if (max_size > mps)
    max_size = mps;
  bit32u current = addr;
  
  do {
    // clear it to zeros
    ehci_clear_phy_mem(current, EHCI_TD_SIZE);
    
    ehci_write_phy_mem(current + EHCI_TD_OFF_NEXT_TD_PTR, (current + EHCI_TD_SIZE) | ((last && (sz <= max_size)) ? QH_HS_T1 : 0));
    ehci_write_phy_mem(current + EHCI_TD_OFF_ALT_NEXT_QTD_PTR, (!status_qtd) ? QH_HS_T1 : status_qtd);
    ehci_write_phy_mem(current + EHCI_TD_OFF_STATUS, (data0<<31) | (((sz < max_size) ? sz : max_size)<<16) | (0<<15) | 
      (0<<12) | (3<<10) | (dir<<8) | 0x80);
    ehci_write_phy_mem(current + EHCI_TD_OFF_BUFF0_PTR, buffer);
    if (buffer) {
      bit32u buff = (buffer + 0x1000) & ~0x0FFF;
      ehci_write_phy_mem(current + EHCI_TD_OFF_BUFF1_PTR, buff);
      ehci_write_phy_mem(current + EHCI_TD_OFF_BUFF2_PTR, buff + 0x1000);
      ehci_write_phy_mem(current + EHCI_TD_OFF_BUFF3_PTR, buff + 0x2000);
      ehci_write_phy_mem(current + EHCI_TD_OFF_BUFF4_PTR, buff + 0x3000);
    }
    
    buffer += max_size;
    data0 ^= 1;
    current += EHCI_TD_SIZE;
    cnt++;
    sz -= max_size;
  } while (sz > 0);
  
  return cnt;
}

void ehci_insert_queue(bit32u queue, const bit8u type) {
  ehci_write_phy_mem(queue + EHCI_QH_OFF_HORZ_PTR, ehci_read_phy_mem(async_base + EHCI_QH_OFF_HORZ_PTR));
  ehci_write_phy_mem(async_base + EHCI_QH_OFF_HORZ_PTR, queue | type);
  ehci_write_phy_mem(queue + EHCI_QH_OFF_PREV_PTR, async_base);
}

// removes a queue from the async list
// EHCI section 4.8.2, shows that we must watch for three bits before we have "fully and successfully" removed
//   the queue(s) from the list
bool ehci_remove_queue(bit32u queue) {
  bit32u temp_addr;
  
  temp_addr = ehci_read_phy_mem(queue + EHCI_QH_OFF_PREV_PTR);
  ehci_write_phy_mem(temp_addr + EHCI_QH_OFF_HORZ_PTR, ehci_read_phy_mem(queue + EHCI_QH_OFF_HORZ_PTR));
  
  temp_addr = ehci_read_phy_mem(queue + EHCI_QH_OFF_HORZ_PTR) & ~EHCI_QUEUE_HEAD_PTR_MASK;
  ehci_write_phy_mem(temp_addr + EHCI_QH_OFF_PREV_PTR, ehci_read_phy_mem(queue + EHCI_QH_OFF_PREV_PTR));
  
  // now wait for the successful "doorbell"
  // set bit 6 in command register (to tell the controller that something has been removed from the schedule)
  // then watch for bit 5 in the status register.  Once it is set, we can assume all removed correctly.
  // We ignore the interrupt on async bit in the USBINTR.  We don't need an interrupt here.
  bit32u command = ehci_read_op_reg(EHC_OPS_USBCommand);
  ehci_write_op_reg(EHC_OPS_USBCommand, command | (1<<6));
  if (ehci_handshake(EHC_OPS_USBStatus, (1<<5), (1<<5), 100)) {
    ehci_write_op_reg(EHC_OPS_USBStatus, (1<<5)); // acknowledge the bit
    return TRUE;
  } else
    return FALSE;
}

int ehci_wait_interrupt(bit32u addr, const bit32u timeout, bool *spd) {
  int ret = -1;
  bit32u status;
  
  int timer = timeout;
  while (timer) {
    status = ehci_read_phy_mem(addr + EHCI_TD_OFF_STATUS) & ~1;  // ignore bit 0 (?)
    if ((status & 0x00000080) == 0) {
      ret = SUCCESS;
      if ((status & 0x7F) == 0x00) {
        if ((((status & 0x7FFF0000) >> 16) > 0) && (((status & (3<<8))>>8) == 1))
          if (spd) *spd = TRUE;
      } else {
        if (status & (1<<6))
          ret = ERROR_STALLED;
        else if (status & (1<<5))
          ret = ERROR_DATA_BUFFER_ERROR;
        else if (status & (1<<4))
          ret = ERROR_BABBLE_DETECTED;
        else if (status & (1<<3))
          ret = ERROR_NAK;
        else if (status & (1<<2))
          ret = ERROR_TIME_OUT;
        else {
          printf("\n 0) USB EHCI wait interrupt qtd->status = 0x%08X", status);
          ret = ERROR_UNKNOWN;
        }
        return ret;
      }
      if ((((status & 0x7FFF0000) >> 16) > 0) && (((status & (3<<8))>>8) == 1)) {
        if ((ehci_read_phy_mem(addr + EHCI_TD_OFF_ALT_NEXT_QTD_PTR) & 1) == 0) {
          addr = ehci_read_phy_mem(addr + EHCI_TD_OFF_ALT_NEXT_QTD_PTR);
          timer = timeout;
        } else
          return ret;
      } else {
        if ((ehci_read_phy_mem(addr + EHCI_TD_OFF_NEXT_TD_PTR) & 1) == 0) {
          addr = ehci_read_phy_mem(addr + EHCI_TD_OFF_NEXT_TD_PTR);
          timer = timeout;
        } else
          return ret;
      }
    }
    mdelay(1);
    timer--;
  }
  
  if (ret == -1) {
    printf("\n USB EHCI Interrupt wait timed out.");
    ret = ERROR_TIME_OUT;
  }
  
  return ret;
}

void ehci_clear_phy_mem(const bit32u address, const int len) {
  int i;
  
  for (i=0; i<len; i++)
    _farpokeb(heap_selector, get_linear(address + i), 0);
}

void ehci_copy_to_phy_mem(const bit32u address, void *src, const int len) {
  bit8u *s = (bit8u *) src;
  int i;
  
  for (i=0; i<len; i++)
    _farpokeb(heap_selector, get_linear(address + i), s[i]);
}

void ehci_copy_from_phy_mem(void *targ, const bit32u address, const int len) {
  bit8u *t = (bit8u *) targ;
  int i;
  
  for (i=0; i<len; i++)
    t[i] = _farpeekb(heap_selector, get_linear(address + i));
}

void ehci_write_phy_mem(const bit32u address, bit32u val) {
  _farpokel(heap_selector, get_linear(address), val);
}

bit32u ehci_read_phy_mem(const bit32u address) {
  return _farpeekl(heap_selector, get_linear(address));
}

void ehci_write_cap_reg(const bit32u offset, const bit32u val) {
  _farpokel(base_selector, offset, val);
}

void ehci_write_op_reg(const bit32u offset, const bit32u val) {
  ehci_write_cap_reg(op_base_off + offset, val);
}

bit32u ehci_read_cap_reg(const bit32u offset) {
  return _farpeekl(base_selector, offset);
}

bit32u ehci_read_op_reg(const bit32u offset) {
  return ehci_read_cap_reg(op_base_off + offset);
}
