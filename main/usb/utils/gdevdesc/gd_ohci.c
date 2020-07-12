/*
 *  gd_ohci.c  v1.10.00       (C) Forever Young Software 1984-2016
 *  
 *  Last update: 10 Oct 2014
 *
 * This code is included on the disc that is included with the book
 *     FYSOS -- USB: The Universal Serial Bus
 * and is for that purpose only.  You have the right to use it for 
 * learning purposes only.  You may not modify it or redistribute for 
 * any other purpose unless you have written permission from the author.
 *
 * You may modify it and use it in your own projects as long as they
 * are for non profit only.  Any project for profit that uses this code
 * must have written permission from the author.
 *
 * The purpose of this code is to show how to retrieve the device descriptor
 *  from an attached usb device.
 *
 * This code finds a OHCI controller, then uses that controller to see 
 *  if something is attached. If so, it attempts to retrieve the device's 
 *  descriptor.
 *
 * Assumptions:
 *  - you have at least a 486 with the CPUID instruction.
 *  - no external hubs have any devices attached.
 *     (this code won't get the device descriptor of any devices plugged
 *      in to external hubs)
 *  - all memory above 1meg is available for use with this code
 *
 * *** Please Note ***
 * This code is for Real Mode DOS with the use of a DMPI extender such as
 *  the one included on the CDROM.  This code will not work under a Windows
 *  DOS session or any other emulated real mode environment.  This code
 *  was written for and tested with FreeDOS (www.freedos.org)
 *
 * compile using gcc (djgpp) for DOS
 *  gcc -Os gd_ohci.c -o gd_ohci.exe -s
 *
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

#include <dpmi.h>
#include <go32.h>

#include "../include/ctype.h"
#include "../include/pci.h"
#include "../include/ohci.h"
#include "../include/usb.h"

#include "gd_ohci.h"

__dpmi_meminfo opregs_mi, hcca_mi, hcca_mi_base;
int opregs_selector, hcca_selector;

// include the code that is common in all four utilities
#include "common.h"

int main(int argc, char *argv[]) {
  
  struct PCI_DEV pci_dev;
  struct PCI_POS pci_pos;

  // print header string
  printf("\n GD_OHCI -- OHCI: Get Device Descriptor.   v1.10.00" COPYRIGHT);
  
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
    if (pci_dev.p_interface == xHC_TYPE_OHCI) {
      // print that we found a controller at this 'address'
      printf("\n  Found OHCI controller at: 0x%08X", pci_dev.base0 & ~0xF);
      // call the function to see if there is a device attached
      process_ohci(&pci_dev, &pci_pos);
    }
    // increment the function for next time.
    pci_pos.func++;
  }
  
  printf("\n");
  return 0;
}

// reset the host controller, then see if anything attached.
// if device attached, get device descriptor
bool process_ohci(struct PCI_DEV *pci_dev, struct PCI_POS *pos) {
  
  int timeout, ndp = 0, potpgt, i, dev_address = 1, desc_len, desc_mps;
  bool good_ret;
  bit32u dword;
  struct DEVICE_DESC dev_desc;
  
  // allow access to data (memmapped IO)
  write_pci(pos->bus, pos->dev, pos->func, 0x04, sizeof(bit16u), 0x0006);
  
  // set up the memory access
  opregs_mi.address = pci_dev->base0 & ~0xF;
  opregs_mi.size = 1024;
  if (get_physical_mapping(&opregs_mi, &opregs_selector) == -1) {
    printf("\n Error 'allocating' physical memory.");
    return FALSE;
  }
  
  // first, read the version register
  if ((ohci_read_op_reg(OHCRevision) & 0xFF) != 0x10)
    return FALSE;
  
  // reset the controller, returning false after 30mS if it doesn't reset
  timeout = 30;
  ohci_write_op_reg(OHCCommandStatus, (1 << 0));
  while (ohci_read_op_reg(OHCCommandStatus) & (1 << 0)) {
    mdelay(1);
    if (--timeout == 0)
      return FALSE;
  }
  
  // see if the controllers funtional state is in the suspend state
  if ((ohci_read_op_reg(OHCControl) & 0xC0) != 0xC0)
    return FALSE;
  
  // see if the frameinterval register is 0x2EDF
  if ((ohci_read_op_reg(OHCFmInterval) & 0x00003FFF) != 0x2EDF)
    return FALSE;
  
  // if we get here, we have a valid OHCI controller, so set it up.
  //  we only set up the Control list.  We don't use the bulk or interrupt list.
  
  // Set up a valid HCCA (Host Controller Communication Area) and stack frame at 2meg
  hcca_mi.address = 0x00200000;  // 2 meg
  hcca_mi.size = 131072;   // 128k
  if (get_physical_mapping(&hcca_mi, &hcca_selector) == -1) {
    printf("\n Error 'allocating' physical memory.");
    return FALSE;
  }
  
  // make a local frame list, and clear it out
  struct OHCI_FRAME our_frame;
  memset(&our_frame, 0, sizeof(struct OHCI_FRAME));
    
  // create the control list, locally
  for (i=0; i<16; i++) {
    our_frame.control_ed[i].nexted = (bit32u) (hcca_mi.address + ((bit32u) &our_frame.control_ed[i+1] - (bit32u) &our_frame));
    our_frame.control_ed[i].flags = (1<<13); // sKip bit
  }
  our_frame.control_ed[15].nexted = 0x00000000;  // mark the last one
  
  // reset the root hub
  ohci_write_op_reg(OHCControl, 0x00000000);
  udelay(50);
  ohci_write_op_reg(OHCControl, 0x000000C0);  // suspend (stop reset)
  
  // set the Frame Interval, periodic start, 
  ohci_write_op_reg(OHCFmInterval, 0xA7782EDF);
  ohci_write_op_reg(OHCPeriodicStart, 0x00002A2F);
  
  // get the number of downstream ports
  ndp = (bit8u) (ohci_read_op_reg(OHCRhDescriptorA) & 0x000000FF);
  printf("\n  Found %i root hub ports.", ndp);
  
  // write the offset of our HCCA
  ohci_write_op_reg(OHCHCCA, (bit32u) hcca_mi.address);
  
  // write the offset of the control head ed
  ohci_write_op_reg(OHCControlHeadED, (bit32u) (hcca_mi.address + ((bit32u) &our_frame.control_ed[0] - (bit32u) &our_frame)));
  ohci_write_op_reg(OHCControlCurrentED, 0x00000000);
  
  // disallow interrupts (we poll the DoneHeadWrite bit instead)
  ohci_write_op_reg(OHCInterruptDisable, 0x80000000);
  
  // copy our local stack to the real stack
  movedata(_my_ds(), (bit32u) &our_frame, hcca_selector, 0, sizeof(struct OHCI_FRAME));
  
  // start the controller  
  ohci_write_op_reg(OHCControl, 0x00000690);  // CLE & operational
  
  // set port power switching
  dword = ohci_read_op_reg(OHCRhDescriptorA);
  potpgt = (bit16u) (((dword >> 24) * 2) + 2);  // plus two to make sure we wait long enough
  ohci_write_op_reg(OHCRhDescriptorA, ((dword & OHCRhDescriptorA_MASK) & ~(1<<9)) | (1<<8));
  
  // loop through the ports
  for (i=0; i<ndp; i++) {
    // power the port
    ohci_write_op_reg(OHCRhPortStatus + (i * 4), (1<<8));
    mdelay(potpgt);
    
    dword = ohci_read_op_reg(OHCRhPortStatus + (i * 4));
    if (dword & 1) {
      if (!ohci_reset_port(i, opregs_selector))
        continue;
      
      // port has been reset, and is ready to be used
      // we have a port that has a device attached and is ready for data transfer.
      // so lets create our stack and send it along.
      bool ls_device = (ohci_read_op_reg(OHCRhPortStatus + (i * 4)) & (1<<9)) ? 1 : 0;
      
      // Some devices will only send the first 8 bytes of the device descriptor
      //  while in the default state.  We must request the first 8 bytes, then reset
      //  the port, set address, then request all 18 bytes.
      ohci_create_stack(&our_frame, hcca_mi.address, 8, 8, ls_device, 0);
      good_ret = ohci_request_desc(&our_frame, &hcca_mi, hcca_selector, opregs_selector, 8);
      if (good_ret) {
        desc_len = our_frame.packet[0];  // get the length of the descriptor (always 18)
        desc_mps = our_frame.packet[7];  // get the mps of the descriptor
        
        // reset the port again
        if (!ohci_reset_port(i, opregs_selector))
          continue;
        
        // set address
        ohci_set_address(&our_frame, hcca_mi.address, dev_address, ls_device);
        good_ret = ohci_request_desc(&our_frame, &hcca_mi, hcca_selector, opregs_selector, 0);
        if (!good_ret) {
          printf("\n Error when trying to set device address to %i", dev_address);
          continue;
        }
        
        // now request the whole descriptor
        ohci_create_stack(&our_frame, hcca_mi.address, desc_mps, desc_len, ls_device, dev_address);
        good_ret = ohci_request_desc(&our_frame, &hcca_mi, hcca_selector, opregs_selector, desc_len);
        if (good_ret) {
          memcpy(&dev_desc, our_frame.packet, sizeof(struct DEVICE_DESC));
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
        } else {
          printf("\n Error when trying to get all %i bytes of descriptor.", desc_len);
          continue;
        }
        
        dev_address++;
      }
    }
  }
  
  // stop the controller  
  ohci_write_op_reg(OHCControl, 0x000000C0);  // suspend state
  
  // free the "allocated" memory, and opregs selector
  __dpmi_free_physical_address_mapping(&hcca_mi);
  __dpmi_free_physical_address_mapping(&opregs_mi);
  
  return 0;
}

bool ohci_reset_port(int port, int opregs_selector) {
  int timeout = 30;
  ohci_write_op_reg(OHCRhPortStatus + (port * 4), (1<<4)); // reset port
  while ((ohci_read_op_reg(OHCRhPortStatus + (port * 4)) & (1<<20)) == 0) {
    mdelay(1);
    if (--timeout == 0)
      break;
  }
  if (timeout == 0) {
    printf("\n Port did not reset after 30mS.");
    return FALSE;
  }
  mdelay(USB_TRSTRCY);  // hold for USB_TRSTRCY ms (reset recovery time)
  
  // clear status change bits
  ohci_write_op_reg(OHCRhPortStatus + (port * 4), (0x1F<<16));
  
  return TRUE;
}

// create a valid stack frame with our GetDeviceDescriptor tranfser packets
void ohci_create_stack(struct OHCI_FRAME *frame, const bit32u base, const int mps, int cnt, const bool ls_device, const int address) {
  
  bit8u setup_packet[8] = { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
  int i, p, t;
  
  // fill in the setup packet
  * ((bit16u *) &setup_packet[6]) = cnt;
  memcpy(frame->setup, setup_packet, 8);
  
  // clear the return packet
  memset(frame->packet, 0, 18);
  
  // create the setup td
  frame->our_tds[0].flags = (14<<28) | (0 << 26) | (2 << 24) | (7<<21) | (TD_DP_SETUP << 19);
  frame->our_tds[0].cbp = base + ((bit32u) &frame->setup[0] - (bit32u) frame);
  frame->our_tds[0].nexttd = base + ((bit32u) &frame->our_tds[1] - (bit32u) frame);
  frame->our_tds[0].be = frame->our_tds[0].cbp + 7;
  
  // create the rest of the in tds
  i = 1; p = 0; t = 1;
  while (cnt > 0) {
    frame->our_tds[i].flags = (14<<28) | (0 << 26) | ((2 | (t & 1)) << 24) | (7<<21) | (TD_DP_IN << 19);
    frame->our_tds[i].cbp = base + ((bit32u) &frame->packet[p] - (bit32u) frame);
    frame->our_tds[i].nexttd = base + ((bit32u) &frame->our_tds[i+1] - (bit32u) frame);
    frame->our_tds[i].be = frame->our_tds[i].cbp + ((cnt > mps) ? (mps - 1) : (cnt - 1));
    t ^= 1;
    p += mps;
    i++;
    cnt -= mps;
  }
  
  // create the status td
  frame->our_tds[i].flags = (14<<28) | (0 << 26) | (3 << 24) | (0<<21) | (TD_DP_OUT << 19);
  frame->our_tds[i].cbp = 0;
  frame->our_tds[i].nexttd = base + ((bit32u) &frame->our_tds[i+1] - (bit32u) frame);
  frame->our_tds[i].be = 0;
  
  // create the ED, using one already in the control list
  frame->control_ed[0].flags = (mps << 16) | (0 << 15) | (0 << 14) | (ls_device ? (1<<13) : 0) | (0 << 11) | (0 << 7) | (address & 0x7F);
  frame->control_ed[0].tailp = frame->our_tds[i].nexttd;
  frame->control_ed[0].headp = base + ((bit32u) &frame->our_tds[0] - (bit32u) frame);
}

void ohci_set_address(struct OHCI_FRAME *frame, bit32u base, int dev_address, bool ls_device) {

  bit8u setup_packet[8] = { 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  
  setup_packet[2] = (bit8u) dev_address;
  
  // fill in the setup packet
  memcpy(frame->setup, setup_packet, 8);
  
  // create the setup td
  frame->our_tds[0].flags = (14<<28) | (2<<24) | (7<<21) | (0<<19);
  frame->our_tds[0].cbp = base + ((bit32u) &frame->setup[0] - (bit32u) frame);
  frame->our_tds[0].nexttd = base + ((bit32u) &frame->our_tds[1] - (bit32u) frame);
  frame->our_tds[0].be = frame->our_tds[0].cbp + 7;
  
  // create the status td
  frame->our_tds[1].flags = (14<<28) | (3<<24) | (0<<21) | (2<<19);
  frame->our_tds[1].cbp = 0;
  frame->our_tds[1].nexttd = base + ((bit32u) &frame->our_tds[2] - (bit32u) frame);
  frame->our_tds[1].be = 0;
  
  // create the ED, using one already in the control list
  frame->control_ed[0].flags = 0x00080000 | ((ls_device) ? (1<<13) : 0);
  frame->control_ed[0].tailp = frame->our_tds[1].nexttd;
  frame->control_ed[0].headp = base + ((bit32u) &frame->our_tds[0] - (bit32u) frame);
}

bool ohci_request_desc(struct OHCI_FRAME *our_frame, __dpmi_meminfo *hcca_mi, int hcca_selector, int opregs_selector, int cnt) {
  int timeout, i;
  bool good_ret;
  
  // clear all the bits in the interrupt status register
  ohci_write_op_reg(OHCInterruptStatus, (1<<30) | 0x7F);
  
  // copy our local stack to the real stack
  movedata(_my_ds(), (bit32u) our_frame, hcca_selector, 0, sizeof(struct OHCI_FRAME));
  
  // set ControlListFilled bit
  ohci_write_op_reg(OHCCommandStatus, (1<<1));
  
  // wait for bit 1 to be set in the status register
  timeout = 2000; // 2 seconds
  while ((ohci_read_op_reg(OHCInterruptStatus) & (1<<1)) == 0) {
    mdelay(1);
    if (--timeout == 0)
      break;
  }
  if (timeout == 0) {
    printf("\n Bit 1 in the HCInterruptStatus register never was set.");
    return FALSE;
  }
  
  // copy the real stack to our local stack
  movedata(hcca_selector, 0, _my_ds(), (bit32u) our_frame, sizeof(struct OHCI_FRAME));
  
  // here is were we would read the HCCA.DoneHead field, and go through the list
  //  to see if there were any errors.
  // For the purpose of this example, we will not use the HCCA.DoneHead, but just
  //  scroll through our transfer descriptors in our local stack.
  
  // clear the bit for next time
  ohci_write_op_reg(OHCInterruptStatus, (1<<1));
  
  // calculate how many tds to check.
  if (cnt > 0)
    cnt /= 8;
  cnt += 2;
  
  good_ret = TRUE;
  for (i=0; i<cnt; i++) {
    if ((our_frame->our_tds[i].flags & 0xF0000000) != 0) {
      good_ret = FALSE;
      printf("\n our_tds[%i].cc != 0  (%i)", i, (our_frame->our_tds[i].flags & 0xF0000000) >> 28);
      break;
    }
  }
  
  return good_ret;
}


void ohci_write_op_reg(const bit32u offset, const bit32u val) {
  _farpokel(opregs_selector, offset, val);
}

bit32u ohci_read_op_reg(const bit32u offset) {
  return _farpeekl(opregs_selector, offset);
}
