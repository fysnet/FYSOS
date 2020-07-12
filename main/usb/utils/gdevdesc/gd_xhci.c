/*
 *  gd_xhci.c  v1.10.20       (C) Forever Young Software 1984-2016
 *  
 *  Last update: 20 May 2016
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
 * This code finds a XHCI controller, then uses that controller to see 
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
 * This code does not do all of the error checking that your driver code
 *  should do.  This code does a bare minimum of error checking and preeration
 *  to get the device descriptor.  Your driver code should have a lot more
 *  to it than just this.  However, this should get you started.
 *
 * *** Please Note ***
 * This code also assumes that you will have a large enough TRB ring for the
 *  control endpoint context.  It does not check for the end of the ring.
 *
 * *** Please Note ***
 * This code is for Real Mode DOS with the use of a DMPI extender such as
 *  the one included on the CDROM.  This code will not work under a Windows
 *  DOS session or any other emulated real mode environment.  This code
 *  was written for and tested with FreeDOS (www.freedos.org)
 *
 * compile using gcc (djgpp) for DOS
 *  gcc -Os gd_xhci.c -o gd_xhci.exe -s
 *
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <dpmi.h>
#include <go32.h>

#include "../include/ctype.h"
#include "../include/pci.h"
#include "../include/xhci.h"
#include "../include/usb.h"

#include "gd_xhci.h"

// common constant dwords
bit32u hccparams1, hccparams2, hcsparams1, hcsparams2, rts_offset, db_offset;
bit32u context_size, page_size, op_base_off;

// port information
struct S_XHCI_PORT_INFO port_info[16];  // this code allows up to 16 total ports

// The Memory Heap
__dpmi_meminfo heap_mi;
int heap_selector;
bit32u cur_heap_ptr;

// operational registers
__dpmi_meminfo base_mi;
int base_selector;

// include the code that is common in all four utilities
#include "common.h"

// command ring
bit32u cmnd_ring_addr;
bit32u cmnd_trb_addr;
bit32u cmnd_trb_cycle;

// event ring
bit32u cur_event_ring_addr;
bit32u cur_event_ring_cycle;

bit32u dcbaap_start;

// slot and ep contexts
// since we will only work with one slot and ep at a time, we can use the
//  same slot and ep context members for all devices
struct xHCI_SLOT_CONTEXT slot;
struct xHCI_EP_CONTEXT ep;

bit32u cur_ep_ring_ptr;
bool   cur_ep_ring_cycle;

// we currently don't use any command line parameters, but still leave them here
int main(int argc, char *argv[]) {
  
  struct PCI_DEV pci_dev;
  struct PCI_POS pci_pos;
  
  // print header string
  printf("\n GD_xHCI -- xHCI: Get Device Descriptor.   v1.10.20" COPYRIGHT);
  
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
    if (pci_dev.p_interface == xHC_TYPE_XHCI) {
      // print that we found a controller at this 'address'
      printf("\n  Found xHCI controller at: 0x%08X", pci_dev.base0 & ~0xF);
      // call the function to see if there is a device attached
      process_xhci(&pci_dev, &pci_pos);
    }
    // increment the function for next time.
    pci_pos.func++;
  }
  
  printf("\n");
  return 0;
}

// reset the controller, create a few rings, set the address, and request the device descriptor.
bool process_xhci(struct PCI_DEV *pci_dev, struct PCI_POS *pos) {
  
  int timeout, ndp = 0, i, k;
  
  // Initialize the pci
  write_pci(pos->bus, pos->dev, pos->func, 0x04, 2, 0x0006);   // mem I/O access enable and bus master enable
  
  memset(port_info, 0, 16 * sizeof(struct S_XHCI_PORT_INFO));  // clear the port info
  
  // set up the memory access
  // The xHCI controller uses the dword at base0 and is memmapped access
  base_mi.address = pci_dev->base0 & ~0xF;
  base_mi.size = 65536;
  if (!get_physical_mapping(&base_mi, &base_selector)) {
    printf("\n Error 'allocating' physical memory for operational registers.");
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
  
  // Write to the FLADJ register incase the BIOS didn't
  // At the time of this writing, there wasn't a BIOS that supported xHCI yet :-)
  write_pci(pos->bus, pos->dev, pos->func, 0x61, 1, 0x20);
  
  // read the version register (just a small safety check)
  if (_farpeekw(base_selector, xHC_CAPS_IVersion) < 0x95)
    return FALSE;
  
  // if it is a Panther Point device, make sure sockets are xHCI controlled.
  if ((read_pci(pos->bus, pos->dev, pos->func, 0, sizeof(bit16u)) == 0x8086) && 
      (read_pci(pos->bus, pos->dev, pos->func, 2, sizeof(bit16u)) == 0x1E31) && 
      (read_pci(pos->bus, pos->dev, pos->func, 8, sizeof(bit8u)) == 4)) {
    write_pci(pos->bus, pos->dev, pos->func, 0xD8, sizeof(bit32u), 0xFFFFFFFF);
    write_pci(pos->bus, pos->dev, pos->func, 0xD0, sizeof(bit32u), 0xFFFFFFFF);
  }
  
  // calculate the operational base
  // must be before any xhci_read_op_reg() or xhci_write_op_reg() calls
  op_base_off = (bit32u) _farpeekb(base_selector, xHC_CAPS_CapLength);
  
  // reset the controller, returning false after 500mS if it doesn't reset
  // Be sure to read the section on reseting the xHCI controller in the book
  //  for the reason we wait 500ms instead of 50ms like USB 2.0 specifies
  timeout = 500;
  xhci_write_op_reg(xHC_OPS_USBCommand, (1<<1));
  while (xhci_read_op_reg(xHC_OPS_USBCommand) & (1<<1)) {
    mdelay(1);
    if (--timeout == 0)
      return FALSE;
  }
  
  /*
   * if we get here, we have a valid xHCI controller, so set it up.
   * First we need to find out which port access arrays are USB2 and which are USB3
   */
  
  // calculate the address of the Extended Capabilities registers and store the other registers
  hccparams1 = xhci_read_cap_reg(xHC_CAPS_HCCParams1);
  hccparams2 = xhci_read_cap_reg(xHC_CAPS_HCCParams2);
  hcsparams1 = xhci_read_cap_reg(xHC_CAPS_HCSParams1);
  hcsparams2 = xhci_read_cap_reg(xHC_CAPS_HCSParams2);
  rts_offset = xhci_read_cap_reg(xHC_CAPS_RTSOFF) & ~0x1F;  // bits 4:0 are reserved
  db_offset = xhci_read_cap_reg(xHC_CAPS_DBOFF) & ~0x03;    // bits 1:0 are reserved
  bit32u ext_caps_off = (((hccparams1 & 0xFFFF0000) >> 16) * 4);
  context_size = (hccparams1 & (1<<2)) ? 64 : 32;
  
  // Turn off legacy support for Keyboard and Mice
  if (!xhci_stop_legacy(ext_caps_off)) {
    printf("\n BIOS did not release Legacy support...");
    return FALSE;
  }
  
	// get num_ports from XHCI's HCSPARAMS1 register
	ndp = (bit8u) ((hcsparams1 & 0xFF000000) >> 24);
  printf("\n  Found %i (virtual) root hub ports.", ndp);
  
  // Get protocol of each port
  //  Each physical port will have a USB3 and a USB2 PortSC register set.
  //  Most likely a controller will only have one protocol item for each version.
  //   i.e.:  One for USB 3 and one for USB 2, they will not be fragmented.
  //  However, it doesn't state anywhere that it can't be fragmented, so the below
  //   code allows for fragmented protocol items
  bit32u next = ext_caps_off;
  bit16u flags;
  int cnt, offset, ports_usb2 = 0, ports_usb3 = 0;
  
  // find the USB 2.0 ports and mark the port_info byte as USB2 if found
  while (next) {
    next = xhci_get_proto_offset(next, 2, &offset, &cnt, &flags);
    if (cnt) {
      for (i=0; i<cnt; i++) {
        port_info[offset + i].offset = ports_usb2++;
        port_info[offset + i].flags = xHCI_PROTO_USB2;
        if (flags & 2)
          port_info[offset + i].flags |= xHCI_PROTO_HSO;
      }
    }
  }
  
  // find the USB 3.0 ports and mark the port_info byte as USB3 if found
  next = ext_caps_off;
  while (next) {
    next = xhci_get_proto_offset(next, 3, &offset, &cnt, &flags);
    if (cnt) {
      for (i=0; i<cnt; i++) {
        port_info[offset + i].offset = ports_usb3++;
        port_info[offset + i].flags = xHCI_PROTO_USB3;
      }
    }
  }
  
  // pair up each USB3 port with it's companion USB2 port
  for (i=0; i<ndp; i++) {
    for (k=0; k<ndp; k++) {
      if ((port_info[k].offset == port_info[i].offset) &&
          ((port_info[k].flags & xHCI_PROTO_INFO) != (port_info[i].flags & xHCI_PROTO_INFO))) {
        port_info[i].other_port_num = k;
        port_info[i].flags |= xHCI_PROTO_HAS_PAIR;
        port_info[k].other_port_num = i;
        port_info[k].flags |= xHCI_PROTO_HAS_PAIR;
      }
    }
  }
  
  // mark all USB3 ports and any USB2 only ports as active, deactivating any USB2 ports that have a USB3 companion
  for (i=0; i<ndp; i++) {
    if (xHCI_IS_USB3_PORT(i) ||
       (xHCI_IS_USB2_PORT(i) && !xHCI_HAS_PAIR(i)))
      port_info[i].flags |= xHCI_PROTO_ACTIVE;
  }
  
  /*
   * Now that we have the protocol for each port, let's set up the controller
   *  we need a command ring and a single endpoint ring with it's event ring.
   *  we also need a slot context area, which includes the pointer array
   */
  
  // get the page size of the controller
  page_size = (xhci_read_op_reg(xHC_OPS_USBPageSize) & 0xFFFF) << 12;
  const bit32u max_slots = hcsparams1 & 0xFF;
  
  // "allocate" the dcbaa and the slot contexts
  // set the scratch_buffer pointer to zero
  //  and clear out the rest of the buffer
  dcbaap_start = heap_alloc(2048, 64, page_size);
  
  // write the address to the controller
  xhci_write_op_reg64(xHC_OPS_USBDcbaap, dcbaap_start);
  
  // create the command ring, returning the physical address of the ring
  cmnd_ring_addr = cmnd_trb_addr = create_ring(CMND_RING_TRBS);
  cmnd_trb_cycle = TRB_CYCLE_ON;   // we start with a Cycle bit of 1 for our command ring
  
  // Command Ring Control Register
  xhci_write_op_reg64(xHC_OPS_USBCrcr, cmnd_ring_addr | TRB_CYCLE_ON);
  
  // Configure Register
  xhci_write_op_reg(xHC_OPS_USBConfig, max_slots);
  
  // Device Notification Control (only bit 1 is allowed)
  xhci_write_op_reg(xHC_OPS_USBDnctrl, (1 << 1));
  
  // Initialize the interrupters
  int max_event_segs = (1 << ((hcsparams2 & 0x000000F0) >> 4));
  int max_interrupters = ((hcsparams1 & 0x0007FF00) >> 8);
  bit32u event_ring_addr = create_event_ring(4096, &cur_event_ring_addr);
  cur_event_ring_cycle = 1;
  
  // write the registers
  xhci_write_primary_intr(xHC_INTERRUPTER_IMAN, (1 << 1) | (1 << 0));  // enable bit & clear pending bit
  xhci_write_primary_intr(xHC_INTERRUPTER_IMOD, 0);                    // disable throttling
  xhci_write_primary_intr(xHC_INTERRUPTER_TAB_SIZE, 1);                // count of segments (table size)
  xhci_write_primary_intr64(xHC_INTERRUPTER_DEQUEUE, (cur_event_ring_addr | (1 << 3)));
  xhci_write_primary_intr64(xHC_INTERRUPTER_ADDRESS, event_ring_addr);
  
  // clear the status register bits
  xhci_write_op_reg(xHC_OPS_USBStatus, (1<<10) | (1<<4) | (1<<3) | (1<<2));
  
  // start the interrupter
  if (set_interrupt_handler(pci_dev->irq, (int) xhci_irq) != 0) {
    printf("\n Error allocating irq.");
    return FALSE;
  }
  
  // set and start the Host Controllers schedule
  xhci_write_op_reg(xHC_OPS_USBCommand, (1<<3) | (1<<2) | (1<<0));
  mdelay(100);
  
  // loop through the ports, starting with the USB3 ports
  for (i=0; i<ndp; i++) {
    if (xHCI_IS_USB3_PORT(i) && xHCI_IS_ACTIVE(i)) {
      // power and reset the port
      if (xhci_reset_port(i))
        // if the reset was good, get the descriptor
        // if the reset was bad, the reset routine will mark this port as inactive,
        //  and mark the USB2 port as active.
        xhci_get_descriptor(i);
    }
  }
  
  // now the USB2 ports
  for (i=0; i<ndp; i++) {
    if (xHCI_IS_USB2_PORT(i) && xHCI_IS_ACTIVE(i)) {
      // power and reset the port
      if (xhci_reset_port(i))
        // if the reset was good, get the descriptor
        xhci_get_descriptor(i);
    }
  }
  
  // stop the interrupt
  stop_interrupt_handler(pci_dev->irq);
  
  // stop the controller  
  xhci_write_op_reg(xHC_OPS_USBCommand, 0x00000000);
  
  // free the "allocated" memory, and opregs selector
  __dpmi_free_physical_address_mapping(&heap_mi);
  __dpmi_free_physical_address_mapping(&base_mi);
  
  return 0;
}

/* allocates some memory in our heap on the alignment specified, rouding up to the nearest dword
 * doesn't allow the memory to cross the given boundary (unless boundary == 0)
 * Checks for errors, but does not return an error.  Simply "exits" if error found
 * will clear the memory to zero
 * returns physical address of memory found
 * alignment and boundary must be a power of 2
 * size must be <= boundary
 */
bit32u heap_alloc(bit32u size, const bit32u alignment, const bit32u boundary) {
  bit32u i;
  
  // align to the next alignment
  cur_heap_ptr = (cur_heap_ptr + (alignment - 1)) & ~(alignment - 1);
  
  // round up to the next dword size
  size = (size + 3) & ~3;
  
  // check to see if this will cross a boundary (unless boundary == 0)
  if (boundary > 0) {
    bit32u next_boundary = (cur_heap_ptr + (boundary - 1)) & ~(boundary - 1);
    if ((cur_heap_ptr + size) > boundary)
      cur_heap_ptr = next_boundary;
  }
  
  // check to see if we are out of bounds
  if ((((cur_heap_ptr + size) - 1) >= HEAP_SIZE) || ((boundary > 0) && (size > boundary))) {
    printf("\n Error in allocating memory within our heap");
    exit(-1);
  }
  
  // calculate physical address
  bit32u addr = heap_mi.address + cur_heap_ptr;
  
  // clear it to zeros
  for (i=0; i<size; i += 4)
    xhci_write_phy_mem(addr + i, 0);
  
  // update our pointer for next time
  cur_heap_ptr += size;
  
  // return the physical address
  return addr;
}

bool xhci_get_descriptor(const int port) {
  
  bit32u dword;
  bit32u HCPortStatusOff = xHC_OPS_USBPortSt + (port * 16);
  struct DEVICE_DESC dev_desc;
  
  // port has been reset, and is ready to be used
  // we have a port that has a device attached and is ready for data transfer.
  // so lets create our stack and send it along.
  dword = xhci_read_op_reg(HCPortStatusOff + xHC_Port_PORTSC);
  int speed = ((dword & (0xF << 10)) >> 10); // FULL = 1, LOW = 2, HI = 3, SS = 4
  /*
   * Some devices will only send the first 8 bytes of the device descriptor
   *  while in the default state.  We must request the first 8 bytes, then reset
   *  the port, set address, then request all 18 bytes.
   */
  
  // send the initialize and enable slot command
  int max_packet;
  
  // send the command and wait for it to return
  struct xHCI_TRB trb;
  trb.param = 0;
  trb.status = 0;
  trb.command = TRB_SET_STYPE(0) | TRB_SET_TYPE(ENABLE_SLOT);
  if (xhci_send_command(&trb, TRUE))
    return FALSE;
  
  // once we get the interrupt, we can get the slot_id
  bit32u slot_id = TRB_GET_SLOT(trb.command);
  
  // if the slot id > 0, we have a valid slot id
  if (slot_id > 0) {
    // calculate initial Max Packet Size (xHCI requires this or will not set address)
    switch (speed) {
      case xHCI_SPEED_LOW:
        max_packet = 8;
        break;
      case xHCI_SPEED_FULL:
      case xHCI_SPEED_HI:
        max_packet = 64;
        break;
      case xHCI_SPEED_SUPER:
        max_packet = 512;
        break;
    }
    
    // initialize the device/slot context
    bit32u slot_addr = xhci_initialize_slot(slot_id, port, speed, max_packet);
    // send the address_device command
    xhci_set_address(slot_addr, slot_id, TRUE);
    // now send the "get_descriptor" packet (get 8 bytes)
    xhci_control_in(&dev_desc, 8, slot_id, max_packet);
    
    // TODO: if the dev_desc.max_packet was different than what we have as max_packet,
    //       you would need to change it here and in the slot context by doing a
    //       evaluate_slot_context call.
    
    // reset the port
    xhci_reset_port(port);
    
    // send set_address_command again
    xhci_set_address(slot_addr, slot_id, FALSE);
    
    // get the whole packet.
    xhci_control_in(&dev_desc, 18, slot_id, max_packet);
    
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
  }
  
  return TRUE;
}

/* Returns offset and count of port register sets for a given version, including flags register
 * On Entry:
 *   list_off: offset to the start or current position of the Capability list
 *    version: 2 or 3.  Version of register set to find.
 * On Return:
 *  writes count of register sets found in *count.
 *  if *count > 0, *offset is written with zero based offset
 *  writes value of item->Protocol_defined in *flags
 *  returns offset of next item in list or 0 if no more
 * The following code assumes:
 * - Little-Endian storage
 * - there is a properly formatted list at this pointer
 */
bit32u xhci_get_proto_offset(bit32u list_off, const int version, int *offset, int *count, bit16u *flags) {
  
  bit32u next;
  
  *count = 0;  // mark that there isn't any to begin with
  
  do {
    // calculate next item position
    bit32u item_next = _farpeekb(base_selector, list_off + 1);
    next = (item_next) ? (list_off + (item_next * 4)) : 0;
    
    // is this a protocol item and if so, is it the version we are looking for?
    if ((_farpeekb(base_selector, list_off + 0) == xHC_xECP_ID_PROTO) && (_farpeekb(base_selector, list_off + 3) == version)) {
      *offset = _farpeekb(base_selector, list_off + 8) - 1;  // make it zero based
      *count = _farpeekb(base_selector, list_off + 9);
      *flags = _farpeekw(base_selector, list_off + 10) & 0x0FFF;
      return next;
    }
    
    // point to next item
    list_off = next;
  } while (list_off);
  
  // return no more
  return 0;
}

/* Release BIOS ownership of controller
 * On Entry:
 *   list: pointer to the start of the Capability list
 * On Return:
 *   TRUE if ownership released
 *
 * Set bit 24 to indicate to the BIOS to release ownership
 * The BIOS should clear bit 16 indicating that it has successfully done so
 * Ownership is released when bit 24 is set *and* bit 16 is clear.
 * This will wait xHC_xECP_LEGACY_TIMEOUT ms for the BIOS to release ownership.
 *   (It is unknown the exact time limit that the BIOS has to release ownership.)
 *
 * This assumes there is a Legacy entry *and* that it is the first entry.
 * You will need to code it so that this is not assumed...
 *
 */
bool xhci_stop_legacy(bit32u list_off) {
  
  // set bit 24 asking the BIOS to release ownership
  xhci_write_cap_reg(list_off, xhci_read_cap_reg(list_off) | xHC_xECP_LEGACY_OS_OWNED);
  
  // Timeout if bit 24 is not set and bit 16 is not clear after xHC_xECP_LEGACY_TIMEOUT milliseconds
  int timeout = xHC_xECP_LEGACY_TIMEOUT;
  while (timeout--) {
    if ((xhci_read_cap_reg(list_off) & xHC_xECP_LEGACY_OWNED_MASK) == xHC_xECP_LEGACY_OS_OWNED)
      return TRUE;
    mdelay(1);
  }
  return FALSE;
}

// 4.9: Initially when the TRB Ring is created in memory, or if it is ever re-initialized, 
//      all TRBs in the ring shall be cleared to ‘0’. This state represents an empty queue.
bit32u create_ring(const int trbs) {
  
  const bit32u addr = heap_alloc(trbs * sizeof(struct xHCI_TRB), 64, 65536);
  
  // make the last one a link TRB to point to the first one
  bit32u pos = addr + ((trbs - 1) * sizeof(struct xHCI_TRB));
  xhci_write_phy_mem64(pos +  0, addr);           // param
  xhci_write_phy_mem(pos +  8, (0 << 22) | 0);  // status
  xhci_write_phy_mem(pos + 12, TRB_LINK_CMND);  // command
  
  return addr;
}

// inserts a command into the command ring at the current command trb location
// returns TRUE if timed out
bool xhci_send_command(struct xHCI_TRB *trb, const bool ring_it) {
  
  // we monitor bit 31 in the command dword
  bit32u org_trb_addr = cmnd_trb_addr;
  
  // must write param and status fields to the ring before the command field.
  xhci_write_phy_mem64(cmnd_trb_addr, trb->param);                       // param
  xhci_write_phy_mem(cmnd_trb_addr +  8, trb->status);                   // status
  xhci_write_phy_mem(cmnd_trb_addr + 12, trb->command | cmnd_trb_cycle); // command
  
  cmnd_trb_addr += sizeof(struct xHCI_TRB);
  
  // if the next trb is the link trb, then move to the first TRB
  // ** for this example, we assume that we are moving to the first TRB in the command ring **
  bit32u cmnd = xhci_read_phy_mem(cmnd_trb_addr + 12);
  if (TRB_GET_TYPE(cmnd) == LINK) {
    xhci_write_phy_mem(cmnd_trb_addr + 12, (cmnd & ~1) | cmnd_trb_cycle);
    cmnd_trb_addr = cmnd_ring_addr;
    cmnd_trb_cycle ^= 1;
  }
  
  if (ring_it) {
    xhci_write_doorbell(0, 0);   // ring the doorbell
    
    // Now wait for the interrupt to happen
    // We use bit 31 of the command dword since it is reserved
    int timer = 2000;
    while (timer && (xhci_read_phy_mem(org_trb_addr + 8) & XHCI_IRQ_DONE) == 0) {
      mdelay(1);
      timer--;
    }
    if (timer == 0) {
      printf("\n USB xHCI Command Interrupt wait timed out.");
      return TRUE;
    } else {
      xhci_get_trb(trb, org_trb_addr);  // retrieve the trb data
      trb->status &= ~XHCI_IRQ_DONE;    // clear off the done bit
    }
  }
  
  return FALSE;
}

int xhci_wait_for_interrupt(bit32u status_addr) {
  
  int timer = 2000;
  while (timer) {
    if (xhci_read_phy_mem(status_addr) & XHCI_IRQ_DONE) {
      switch (TRB_GET_COMP_CODE(xhci_read_phy_mem(status_addr))) {
        case TRB_SUCCESS:
        case SHORT_PACKET:
          return TRB_SUCCESS;
        case STALL_ERROR:
        case DATA_BUFFER_ERROR:
        case BABBLE_DETECTION:
          return STALL_ERROR;
        default:
          printf("\n USB xHCI wait interrupt status = 0x%08X (%i)", xhci_read_phy_mem(status_addr), TRB_GET_COMP_CODE(xhci_read_phy_mem(status_addr)));
          return ERROR_UNKNOWN;
      }
    }
    
    timer--;
    mdelay(1);
  }
  
  printf("\n USB xHCI Interrupt wait timed out.");
  return ERROR_TIME_OUT;
}

void xhci_get_trb(struct xHCI_TRB *trb, const bit32u address) {
  trb->param =   xhci_read_phy_mem64(address);
  trb->status =  xhci_read_phy_mem(address +  8);
  trb->command = xhci_read_phy_mem(address + 12);
}

void xhci_set_trb(struct xHCI_TRB *trb, const bit32u address) {
  xhci_write_phy_mem64(address, trb->param);
  xhci_write_phy_mem(address +  8, trb->status);
  xhci_write_phy_mem(address + 12, trb->command);
}

bit32u create_event_ring(const int trbs, bit32u *ret_addr) {
  
  // Please note that 'trbs' should be <= 4096 or you will need to make multiple segments
  // I only use one here.
  
  const bit32u table_addr = heap_alloc(64, 64, 0);  // min 16 bytes on a 64 byte alignment, no boundary requirements
  const bit32u addr = heap_alloc((trbs * sizeof(struct xHCI_TRB)), 64, 65536); // 16 * trbs, 64 byte alignment, 64k boundary
  
  // we only use 1 segment for this example
  xhci_write_phy_mem64(table_addr, addr);
  xhci_write_phy_mem(table_addr +  8, trbs);  // count of TRB's
  xhci_write_phy_mem(table_addr + 12, 0);
  
  *ret_addr = addr;
  return table_addr;
}

bool xhci_reset_port(const int port) {
  
  bool ret = FALSE;
  bit32u HCPortStatusOff = xHC_OPS_USBPortSt + (port * 16);
  bit32u val;
  
  // power the port?
  if ((xhci_read_op_reg(HCPortStatusOff + xHC_Port_PORTSC) & (1<<9)) == 0) {
    xhci_write_op_reg(HCPortStatusOff + xHC_Port_PORTSC, (1<<9));
    mdelay(20);
    if ((xhci_read_op_reg(HCPortStatusOff + xHC_Port_PORTSC) & (1<<9)) == 0)
      return FALSE;  // return bad reset.
  }
  
  // we need to make sure that the status change bits are clear
  xhci_write_op_reg(HCPortStatusOff + xHC_Port_PORTSC, (1<<9) | xHC_PortUSB_CHANGE_BITS);
  
  // set bit 4 (USB2) or 31 (USB3) to reset the port
  if (xHCI_IS_USB3_PORT(port))
    xhci_write_op_reg(HCPortStatusOff + xHC_Port_PORTSC, (1<<9) | (1<<31));
  else
    xhci_write_op_reg(HCPortStatusOff + xHC_Port_PORTSC, (1<<9) | (1<<4));
  
  // wait for bit 21 to set
  int timeout = 500;
  while (timeout) {
    val = xhci_read_op_reg(HCPortStatusOff + xHC_Port_PORTSC);
    if (val & (1<<21))
      break;
    timeout--;
    mdelay(1);
  }
  
  // if we didn't time out
  if (timeout > 0) {
    // reset recovery time
    mdelay(USB_TRHRSI);
    
    // if after the reset, the enable bit is non zero, there was a successful reset/enable
    val = xhci_read_op_reg(HCPortStatusOff + xHC_Port_PORTSC);
    
    if (val & (1<<1)) {
      // clear the status change bit(s)
      xhci_write_op_reg(HCPortStatusOff + xHC_Port_PORTSC, (1<<9) | xHC_PortUSB_CHANGE_BITS);
      
      // success
      ret = TRUE;
    }
  } else
    printf("\n Reset Timed out");
  
  // if we have a successful USB2 reset, we need to make sure this port is marked active,
  //  and if it has a paired port, it is marked inactive
  if ((ret == SUCCESS_RESET) && xHCI_IS_USB2_PORT(port)) {
    port_info[port].flags |= xHCI_PROTO_ACTIVE;
    if (port_info[port].flags & xHCI_PROTO_HAS_PAIR)
      port_info[port_info[port].other_port_num].flags &= ~xHCI_PROTO_ACTIVE;
  }
  
  // if error resetting USB3 protocol, deactivate this port and activate the paired USB2 port.
  //  it will be paired since all USB3 ports must be USB2 compatible.
  if (!ret && xHCI_IS_USB3_PORT(port)) {
    port_info[port].flags &= ~xHCI_PROTO_ACTIVE;
    port_info[port_info[port].other_port_num].flags |= xHCI_PROTO_ACTIVE;
  }
  
  return ret;
}

/* Create a slot entry
 * - at this time, we don't know if the device is a hub or not, so we don't
 *   set the slot->hub, ->mtt, ->ttt, ->etc, items.
 */
bit32u xhci_initialize_slot(const int slot_id, const int port, const int speed, const int max_packet) {
  
  int i;
  
  bit32u slot_addr = heap_alloc(context_size * 2, 32, page_size);  // two contexts (slot and control_ep), 32 byte alignment, page_size boundary
  
  // write the address of the slot in the slot array
  xhci_write_phy_mem64(dcbaap_start + (slot_id * sizeof(bit64u)), slot_addr);
  
  // set the initial values
  slot.entries = 1;            // the control ep
  slot.speed = speed;          // speed
  slot.route_string = 0;
  slot.rh_port_num = port + 1; // root hub port number this device is downstream of
  slot.max_exit_latency = 0;   // calculated later
  slot.int_target = xHC_INTERRUPTER_PRIMARY;
  slot.slot_state = SLOT_STATE_DISABLED_ENABLED;
  slot.device_address = 0;
  
  // now write it to the controllers slot memory buffer
  write_to_slot(slot_addr, &slot);
  
  // initialize the control ep
  xhci_initialize_ep(slot_addr, slot_id, xHCI_CONTROL_EP, max_packet, CONTROL_EP, 0, speed, 0);
  
  return slot_addr;
}

// The Average TRB Length field is computed by dividing the average TD Transfer Size by 
//  the average number of TRBs that are used to describe a TD, including Link, No Op, and Event Data TRBs.
void xhci_initialize_ep(const bit32u slot_addr, const int slot_id, const int ep_num, const bit32u max_packet_size, 
                        const int type, const bool dir, const int speed, const int ep_interval) {
  
  // since we are only getting the device descriptor, we assume type will be CONTROL_EP
  if (type != CONTROL_EP)
    return;
  
  // allocate the EP's Transfer Ring
  ep.tr_dequeue_pointer = create_ring(TRBS_PER_RING);
  ep.dcs = TRB_CYCLE_ON;
  
  // save for the control_in stuff
  cur_ep_ring_ptr = ep.tr_dequeue_pointer;
  cur_ep_ring_cycle = ep.dcs;
  
  // set the initial values
  ep.max_packet_size = max_packet_size;
  ep.lsa = 0;
  ep.max_pstreams = 0;
  ep.mult = 0;
  ep.ep_state = EP_STATE_DISABLED;
  ep.hid = 0;
  ep.ep_type = 4;
  ep.average_trb_len = 8;  // All CONTROL EP's shall have '8' (page 325)
  ep.cerr = 3;
  ep.max_burst_size = 0;
  ep.interval = ep_interval;
  
  // now write it to the controllers ep memory block
  write_to_ep(slot_addr + (ep_num * context_size), &ep);
}

// write slot context to memory slot context buffer used by the controller
void write_to_slot(const bit32u offset, struct xHCI_SLOT_CONTEXT *slot) {
  xhci_write_phy_mem(offset +  0,
         (slot->entries << 27) | (slot->hub << 26) | (slot->mtt << 25) | (slot->speed << 20) | slot->route_string);
  xhci_write_phy_mem(offset +  4,
         (slot->num_ports << 24) | (slot->rh_port_num << 16) | slot->max_exit_latency);
  xhci_write_phy_mem(offset +  8,
         (slot->int_target << 22) | (slot->ttt << 16) | (slot->tt_port_num << 8) | slot->tt_hub_slot_id);
  xhci_write_phy_mem(offset + 12,
         (slot->slot_state << 27) | slot->device_address);
}

// write ep context to memory ep context buffer used by the controller
void write_to_ep(const bit32u offset, struct xHCI_EP_CONTEXT *ep) {
  xhci_write_phy_mem(offset +  0,
         (ep->interval << 16) | (ep->lsa << 15) | (ep->max_pstreams << 10) | (ep->mult << 8) | ep->ep_state);
  xhci_write_phy_mem(offset +  4,
         (ep->max_packet_size << 16) | (ep->max_burst_size << 8) | (ep->hid << 7) | (ep->ep_type << 3) | (ep->cerr << 1));
  xhci_write_phy_mem(offset +  8,
         (bit32u) (ep->tr_dequeue_pointer & 0xFFFFFFFF) | ep->dcs);
  xhci_write_phy_mem(offset + 12,
         (bit32u) (ep->tr_dequeue_pointer >> 32));
  xhci_write_phy_mem(offset + 16,
         (ep->max_esit_payload << 16) | ep->average_trb_len);
}

// read slot context from memory slot context buffer used by the controller
void read_from_slot(struct xHCI_SLOT_CONTEXT *slot, const bit32u offset) {
  slot->entries =      (xhci_read_phy_mem(offset +  0) & (0x1F << 27)) >> 27;
  slot->hub = (bit8u)  (xhci_read_phy_mem(offset +  0) & (0x01 << 26)) >> 26;
  slot->mtt = (bit8u)  (xhci_read_phy_mem(offset +  0) & (0x01 << 25)) >> 25;
  slot->speed =        (xhci_read_phy_mem(offset +  0) & (0x0F << 20)) >> 20;
  slot->route_string = (xhci_read_phy_mem(offset +  0) & 0xFFFFF);
  slot->num_ports =    (xhci_read_phy_mem(offset +  4) & (0xFF << 24)) >> 24;
  slot->rh_port_num =  (xhci_read_phy_mem(offset +  4) & (0xFF << 16)) >> 16;
  slot->max_exit_latency = (xhci_read_phy_mem(offset +  4) & 0xFFFF);
  slot->int_target =   (xhci_read_phy_mem(offset +  8) & (0x3FF << 22)) >> 22;
  slot->ttt =          (xhci_read_phy_mem(offset +  8) & (0x03 << 16)) >> 16;
  slot->tt_port_num =  (xhci_read_phy_mem(offset +  8) & (0xFF <<  8)) >>  8;
  slot->tt_hub_slot_id = (xhci_read_phy_mem(offset +  8) & 0xFF);
  slot->slot_state =   (xhci_read_phy_mem(offset + 12) & (0x1F << 27)) >> 27;
  slot->device_address = (xhci_read_phy_mem(offset + 12) & 0xFF);
}

// read ep context from memory ep context buffer used by the controller
void read_from_ep(struct xHCI_EP_CONTEXT *ep, const bit32u offset) {
  ep->interval =        (xhci_read_phy_mem(offset +  0) & (0xFF << 16)) >> 16;
  ep->lsa = (bit8u)     (xhci_read_phy_mem(offset +  0) & (0x01 << 15)) >> 15;
  ep->max_pstreams =    (xhci_read_phy_mem(offset +  0) & (0x1F << 10)) >> 10;
  ep->mult =            (xhci_read_phy_mem(offset +  0) & (0x03 <<  8)) >>  8;
  ep->ep_state =        (xhci_read_phy_mem(offset +  0) & 0x07);
  ep->max_packet_size = (xhci_read_phy_mem(offset +  4) & (0xFFFF << 16)) >> 16;
  ep->max_burst_size =  (xhci_read_phy_mem(offset +  4) & (0x00FF <<  8)) >>  8;
  ep->hid = (bit8u)     (xhci_read_phy_mem(offset +  4) & (0x01 <<  7)) >>  7;
  ep->ep_type =         (xhci_read_phy_mem(offset +  4) & (0x07 <<  3)) >>  3;
  ep->cerr =            (xhci_read_phy_mem(offset +  4) & (0x03 <<  1)) >>  1;
  ep->tr_dequeue_pointer = (xhci_read_phy_mem64(offset +  8) & ~0x0F);
  ep->dcs = (bit8u)     (xhci_read_phy_mem(offset +  8) & 0x01);
  ep->max_esit_payload = (xhci_read_phy_mem(offset + 16) & (0xFFFF << 16)) >> 16;
  ep->average_trb_len = (xhci_read_phy_mem(offset + 16) & 0xFFFF);
}

bool xhci_set_address(const bit32u slot_addr, const int slot_id, const bool block_it) {
  
  struct xHCI_SLOT_CONTEXT slot_context;
  struct xHCI_EP_CONTEXT ep_context;
  struct xHCI_TRB trb;
  
  bit32u address = heap_alloc(context_size * 32, 64, page_size);
  
  xhci_write_phy_mem(address + 0, 0x00);
  xhci_write_phy_mem(address + 4, 0x03);
  write_to_slot(address + context_size, &slot);
  write_to_ep(address + context_size + (xHCI_CONTROL_EP * context_size), &ep);
  
  trb.param = address;
  trb.status = 0;
  trb.command = TRB_SET_SLOT(slot_id) | TRB_SET_TYPE(ADDRESS_DEVICE) | (block_it << 9);
  if (xhci_send_command(&trb, TRUE))
    return FALSE;
  
  if (TRB_GET_COMP_CODE(trb.status) == TRB_SUCCESS) {
    read_from_slot(&slot_context, slot_addr);
    slot.slot_state = slot_context.slot_state;
    slot.device_address = slot_context.device_address;
    read_from_ep(&ep_context, slot_addr + (xHCI_CONTROL_EP * context_size));
    ep.ep_state = ep_context.ep_state;
    ep.max_packet_size = ep_context.max_packet_size;
    return TRUE;
  } else
    return FALSE;
}

bool xhci_control_in(void *targ, const int len, const int slot_id, const int max_packet) {

  bit8u dir = xHCI_DIR_IN;
  bit8u dirb = xHCI_DIR_IN_B;
  bit32u status_addr = heap_alloc(4, 16, 16);  // we need a dword status buffer with a physical address
  bit32u status;
  bit32u buffer_addr = heap_alloc(256, 1, 0);  // get a physical address buffer and then copy from it later
  
  static struct REQUEST_PACKET packet = { STDRD_GET_REQUEST, GET_DESCRIPTOR, ((DEVICE << 8) | 0), 0, 0 };
  packet.length = len;
  
  xhci_setup_stage(&packet, xHCI_DIR_IN);
  xhci_data_stage(buffer_addr, DATA_STAGE, len, xHCI_DIR_IN_B, max_packet, status_addr);
  
  // Now ring the doorbell and wait for the interrupt to happen
  xhci_write_doorbell(slot_id, xHCI_CONTROL_EP);   // ring the doorbell
  status = xhci_wait_for_interrupt(status_addr);
  
  if (status != TRB_SUCCESS)
    return 0;
  
  xhci_status_stage(xHCI_DIR_IN_B ^ 1, status_addr);
  
  // Now ring the doorbell and wait for the interrupt to happen
  xhci_write_doorbell(slot_id, xHCI_CONTROL_EP);   // ring the doorbell
  status = xhci_wait_for_interrupt(status_addr);
  
  if (status != TRB_SUCCESS)
    return 0;
  
  // now copy from the physical buffer to the specified buffer
  xhci_copy_from_phy_mem(targ, buffer_addr, len);
  
  return len;
}

int xhci_setup_stage(const struct REQUEST_PACKET *request, const bit8u dir) {
  
  bit64u param = (bit64u) (((request->value << 16) | (request->request << 8) | request->request_type) | 
                                  ((bit64u) request->length << 48) | ((bit64u) request->index << 32));

  xhci_write_phy_mem64(cur_ep_ring_ptr, param);
  xhci_write_phy_mem(cur_ep_ring_ptr +  8, (0 << 22) | 8);
  xhci_write_phy_mem(cur_ep_ring_ptr + 12, (dir << 16) | TRB_SET_TYPE(SETUP_STAGE) | (1 << 6) | (0 << 5) | cur_ep_ring_cycle);
  
  // this assumes that we will always have room in the ring for these TRB's
  cur_ep_ring_ptr += 16;
  
  return 1;
}

int xhci_data_stage(bit32u addr, bit8u trb_type, const bit32u size, bit8u direction, const bit16u max_packet, const bit32u status_addr) {
  
  int i = 0;
  int sz = (int) size;
  int remaining = (int) (((sz + (max_packet - 1)) / max_packet) - 1);
  
  if (remaining < 0)
    remaining = 0;
  
  while (sz > 0) {
    xhci_write_phy_mem64(cur_ep_ring_ptr, addr);  // physical address
    xhci_write_phy_mem(cur_ep_ring_ptr +  8, (0 << 22) | (remaining << 17) | ((sz < max_packet) ? sz : max_packet));
    xhci_write_phy_mem(cur_ep_ring_ptr + 12, (direction << 16) | TRB_SET_TYPE(trb_type) | (0 << 9) | (0 << 6) | (0 << 5) | 
                                       (1 << 4) | (0 << 3) | (0 << 2) | ((remaining == 0) << 1) | cur_ep_ring_cycle);
    addr += max_packet;
    // this assumes that we will always have room in the ring for these TRB's
    cur_ep_ring_ptr += sizeof(struct xHCI_TRB);
    i++;
    sz -= max_packet;
    remaining--;
    
    // if is a DATA_STAGE TRB, after the first trb, the remaining are NORMAL TRBs and direction is not used.
    trb_type = NORMAL;
    direction = 0;
  }
  
  xhci_write_phy_mem(status_addr, 0); // clear the status dword
  
  xhci_write_phy_mem64(cur_ep_ring_ptr, status_addr);
  xhci_write_phy_mem(cur_ep_ring_ptr +  8, (0 << 22));
  xhci_write_phy_mem(cur_ep_ring_ptr + 12, TRB_SET_TYPE(EVENT_DATA) | (1 << 5) | (0 << 4) | (0 << 1) | cur_ep_ring_cycle);
  
  // this assumes that we will always have room in the ring for these TRB's
  cur_ep_ring_ptr += sizeof(struct xHCI_TRB);
  
  return (i + 1);
}

int xhci_status_stage(const bit8u dir, const bit32u status_addr) {
  
  xhci_write_phy_mem64(cur_ep_ring_ptr, 0);
  xhci_write_phy_mem(cur_ep_ring_ptr +  8, (0 << 22));
  xhci_write_phy_mem(cur_ep_ring_ptr + 12, (dir << 16) | TRB_SET_TYPE(STATUS_STAGE) | (0 << 5) | (1 << 4) | (0 << 1) | cur_ep_ring_cycle);
  
  // this assumes that we will always have room in the ring for these TRB's
  cur_ep_ring_ptr += 16;
  
  xhci_write_phy_mem(status_addr, 0); // clear the status dword
  
  xhci_write_phy_mem64(cur_ep_ring_ptr, status_addr);
  xhci_write_phy_mem(cur_ep_ring_ptr +  8, (0 << 22));
  xhci_write_phy_mem(cur_ep_ring_ptr + 12, TRB_SET_TYPE(EVENT_DATA) | (1 << 5) | (0 << 4) | (0 << 1) | cur_ep_ring_cycle);
  
  // this assumes that we will always have room in the ring for these TRB's
  cur_ep_ring_ptr += 16;
  
  return 2;
}


void xhci_write_cap_reg(const bit32u offset, const bit32u val) {
  _farpokel(base_selector, offset, val);
}

void xhci_write_cap_reg64(const bit32u offset, const bit64u val) {
  _farpokel(base_selector, offset, (bit32u) val);
  if (hccparams1 & 1)
    _farpokel(base_selector, offset + 4, (bit32u) (val >> 32));
}

void xhci_write_op_reg(const bit32u offset, const bit32u val) {
  xhci_write_cap_reg(op_base_off + offset, val);
}

void xhci_write_op_reg64(const bit32u offset, const bit64u val) {
  xhci_write_cap_reg64(op_base_off + offset, val);
}


bit32u xhci_read_cap_reg(const bit32u offset) {
  return _farpeekl(base_selector, offset);
}

bit64u xhci_read_cap_reg64(const bit32u offset) {
  if (hccparams1 & 1)
    return (_farpeekl(base_selector, offset) |
      ((bit64u) _farpeekl(base_selector, offset + 4) << 32));
  else
    return _farpeekl(base_selector, offset);
}

bit32u xhci_read_op_reg(const bit32u offset) {
  return xhci_read_cap_reg(op_base_off + offset);
}

bit64u xhci_read_op_reg64(const bit32u offset) {
  return xhci_read_cap_reg64(op_base_off + offset);
}

void xhci_copy_from_phy_mem(void *targ, const bit32u address, const int len) {
  bit8u *t = (bit8u *) targ;
  int i;
  
  for (i=0; i<len; i++)
    t[i] = _farpeekb(heap_selector, get_linear(address + i));
}

void xhci_write_phy_mem(const bit32u address, bit32u val) {
  _farpokel(heap_selector, get_linear(address), val);
}

void xhci_write_phy_mem64(const bit32u address, bit64u val) {
  _farpokel(heap_selector, get_linear(address), (bit32u) val);
  if (hccparams1 & 1)
    _farpokel(heap_selector, get_linear(address) + 4, (bit32u) (val >> 32));
}

bit32u xhci_read_phy_mem(const bit32u address) {
  return _farpeekl(heap_selector, get_linear(address));
}

bit64u xhci_read_phy_mem64(const bit32u address) {
  if (hccparams1 & 1)
    return (_farpeekl(heap_selector, get_linear(address)) |
      ((bit64u) _farpeekl(heap_selector, get_linear(address) + 4) << 32));
  else
    return _farpeekl(heap_selector, get_linear(address));
}

void xhci_write_doorbell(const bit32u slot_id, const bit32u val) {
  _farpokel(base_selector, db_offset + (slot_id * sizeof(bit32u)), val); // ring a doorbell
}

void xhci_write_primary_intr(const bit32u offset, const bit32u val) {
  _farpokel(base_selector, (rts_offset + 0x20) + offset, val);
}

void xhci_write_primary_intr64(const bit32u offset, const bit64u val) {
  _farpokel(base_selector, (rts_offset + 0x20) + offset, (bit32u) val);
  if (hccparams1 & 1)
    _farpokel(base_selector, (rts_offset + 0x20) + offset + 4, (bit32u) (val >> 32));
}

bit32u xhci_read_primary_intr(const bit32u offset) {
  return _farpeekl(base_selector, (rts_offset + 0x20) + offset);
}

bit64u xhci_read_primary_intr64(const bit32u offset) {
  if (hccparams1 & 1)
    return (_farpeekl(base_selector, (rts_offset + 0x20) + offset) |
      ((bit64u) _farpeekl(base_selector, (rts_offset + 0x20) + offset + 4) << 32));
  else
    return _farpeekl(base_selector, (rts_offset + 0x20) + offset);
}


// we only "catch" the ones we need and ignore all of the rest.
// It is a big job to work on all returned events.  This will be something
//  I will leave for your efforts.
// Remember that this also assumes that all events will fit within the same segment and not wrap...
void xhci_irq() {
  
  // acknowledge interrupt (status register first)
  // clear the status register bits
  xhci_write_op_reg(xHC_OPS_USBStatus, xhci_read_op_reg(xHC_OPS_USBStatus));
  
  const bit32u dword = xhci_read_primary_intr(xHC_INTERRUPTER_IMAN);
  if ((dword & 3) == 3) {
    // acknowledge the interrupter's IP bit being set
    xhci_write_primary_intr(xHC_INTERRUPTER_IMAN, dword | 3);
    
    // do the work
    struct xHCI_TRB event, org;
    bit32u org_address;
    bit32u last_addr = cur_event_ring_addr;
    xhci_get_trb(&event, cur_event_ring_addr);
    
    while ((event.command & 1) == cur_event_ring_cycle) {
      if ((event.command & (1<<2)) == 0) {
        switch (TRB_GET_COMP_CODE(event.status)) {
          case TRB_SUCCESS:
            switch (TRB_GET_TYPE(event.command)) {
              // Command Completion Event
              case COMMAND_COMPLETION:
                org_address = (bit32u) event.param;
                xhci_get_trb(&org, org_address);
                switch (TRB_GET_TYPE(org.command)) {
                  case ENABLE_SLOT:
                    org.command &= 0x00FFFFFF;
                    org.command |= (event.command & 0xFF000000); // return slot ID (1 based)
                    org.status = event.status;
                    break;
                    
                  default:
                    org.status = event.status;
                    break;
                }
                
                // mark the command as done
                org.status |= XHCI_IRQ_DONE;
                // and write it back
                xhci_set_trb(&org, org_address);
                break;
            }
            break;
        }
        
        // mark the TRB as done
      } else {
        switch (TRB_GET_TYPE(event.command)) {
          case TRANS_EVENT: // If SPD was encountered in this TD, comp_code will be SPD, else it should be SUCCESS (specs 4.10.1.1)
            xhci_write_phy_mem((bit32u) event.param, (event.status | XHCI_IRQ_DONE)); // return code + bytes *not* transferred
            break;
            
          default:
            ;
        }
      }
      
      // get next one
      last_addr = cur_event_ring_addr;
      cur_event_ring_addr += sizeof(struct xHCI_TRB);
      xhci_get_trb(&event, cur_event_ring_addr);
    }
    
    // advance the dequeue pointer (clearing the busy bit)
    xhci_write_primary_intr64(xHC_INTERRUPTER_DEQUEUE, last_addr | (1<<3));
  }
  
  // this first one assumes your irq is 8 or above
 	outportb(0xA0, 0x20);      // end of interrupt on controller
 	outportb(0x20, 0x20);      // end of interrupt on controller
}

