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
 *  VIDEOCAM.EXE
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS environment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *   - The camera is plugged into a High Speed port (EHCI)
 *   - you have at least a 486 with the CPUID instruction.
 *   - no external hubs have any devices attached.
 *      (this code won't get the device descriptor of any devices plugged
 *       in to external hubs)
 *   - all memory above 1meg is available for use with this code
 *
 *  Last updated: 5 Jan 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os videocam.c -o videocam.exe -s
 *
 *  Usage:
 *    videocam
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

#include "videocam.h"

// include the helper functions
#include "heap.h"
#include "timer.h"

bit32u op_base_off;

int main(int argc, char *argv[]) {
  
  struct PCI_DEV pci_dev;
  struct PCI_POS pci_pos;
  
  // print header string
  printf("\n VIDEOCAM -- EHCI: Get Video Camera Feed.   v1.00.00"
         "\n Forever Young Software        (C)opyright 1984-2022\n");
  
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
  
  int timeout, i, dev_address = 1, num_ports;
  struct DEVICE_DESC dev_desc;
  bit32u hccparams, hcsparams;
  
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
  heap_mi.address = HEAP_START;
  heap_mi.size = HEAP_SIZE;
  if (!get_physical_mapping(&heap_mi, &heap_selector)) {
    printf("\n Error 'allocating' physical memory for our heap.");
    __dpmi_free_physical_address_mapping(&base_mi);
    return FALSE;
  }
  heap_init(HEAP_START, HEAP_SIZE);
  
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
  num_ports = (int) (hcsparams & 0x0F);  // at least 1 and no more than 15
  printf("\n  Found %i root hub ports.", num_ports);
  
  // allocate then initialize the async queue list (Control and Bulk TD's)
  async_base = heap_alloc(16 * EHCI_QUEUE_HEAD_SIZE);
  ehci_init_stack_frame(async_base);
  
  // allocate then initialize the periodic list (ISO's and Interrupts)
  periodic_base = heap_alloc(1024 * sizeof(bit32u));
  ehci_init_periodic(periodic_base);
  
  // set and start the Host Controllers schedule
  if ((hccparams & (1<<0)) == 1)
    ehci_write_op_reg(EHC_OPS_CtrlDSSegemnt, 0x00000000);     // we use only 32-bit addresses
  ehci_write_op_reg(EHC_OPS_PeriodicListBase, periodic_base); // physical address
  ehci_write_op_reg(EHC_OPS_AsyncListBase, async_base);       // physical address
  ehci_write_op_reg(EHC_OPS_FrameIndex, 0);                   // start at (micro)frame 0
  ehci_write_op_reg(EHC_OPS_USBInterrupt, 0);                 // disallow interrupts
  ehci_write_op_reg(EHC_OPS_USBStatus, 0x3F);                 // clear any pending interrupts
  
  // start the host controller: 8 micro-frames, start schedule (frame list size = 1024)
  ehci_write_op_reg(EHC_OPS_USBCommand, (8<<16) | (1<<0));
  
  // enable the asynchronous list
  if (!ehci_enable_list(TRUE, 5)) {
    printf("\n Did not enable the Ascynchronous List");
    return FALSE;
  }
  
  // enable the periodic list
  if (!ehci_enable_list(TRUE, 4)) {
    printf("\n Did not enable the Periodic List");
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
    if (ehci_reset_port(i)) {
      // if the reset was good, get the descriptor
      if (ehci_get_descriptor(i, &dev_desc, dev_address)) {
        // if this is the camera we are looking for, call the rest of the code
        if ((dev_desc.vendorid == VENDOR_ID) && (dev_desc.productid == PRODUCT_ID)) {
          ehci_do_video(&dev_desc, dev_address);
          break;
        }
        
        // the device attached was not the camera we are looking for.
        //  increment to the next device address number and try the next port.
        dev_address++;
      }
    }
  }
  
  // stop the controller  
  ehci_write_op_reg(EHC_OPS_USBCommand, 0x00000000);
  
  // free the async_base (don't really need to do this)
  heap_free(async_base);
  
  // free the "allocated" memory, and opregs selector
  __dpmi_free_physical_address_mapping(&heap_mi);
  __dpmi_free_physical_address_mapping(&base_mi);
  
  return 0;
}

// finds another controller, or returns FALSE if no more found
// pos = bus/dev/func to start with
bool get_next_cntrlr(struct PCI_DEV *device, struct PCI_POS *pos) {
  
  bit32u type;
  bit32u *pcidata = (bit32u *) device;
  int i;
  
  for (; pos->bus < PCI_MAX_BUS; pos->bus++) {
    for (; pos->dev < PCI_MAX_DEV; pos->dev++) {
      for (; pos->func < PCI_MAX_FUNC; pos->func++) {
				if (pci_read_word(pos->bus, pos->dev, pos->func, 0x00) != 0xFFFF) {
          type = pci_read_word(pos->bus, pos->dev, pos->func, (2<<2)+2);
          if (type == 0x0C03) {
            printf("\n PCI: Found a USB controller entry: Bus = %i, device = %i, function = %i ", pos->bus, pos->dev, pos->func);
            // read in the 256 bytes (64 dwords)
            for (i=0; i<64; i++)
              pcidata[i] = pci_read_dword(pos->bus, pos->dev, pos->func, (i<<2));
            return TRUE;
          }
				}
			}
      pos->func = 0;
		}
    pos->dev = 0;
	}
  
  // no more devices found
	return FALSE;
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

// creates an 8 byte request packet
void usb_request_packet(struct REQUEST_PACKET *packet, const bit8u type, const bit8u request, const bit8u value_hi, 
                        const bit8u value_lo, const bit16u index, const bit16u len) {
	packet->request_type = type;
	packet->request = request;
	packet->value = (value_hi << 8) | value_lo;
	packet->index = index;
	packet->length = len;
}

bool ehci_get_descriptor(const int port, struct DEVICE_DESC *dev_desc, const int dev_address) {
  
  /*
   * Since most high-speed devices will only work with a max packet size of 64,
   *  we don't request the first 8 bytes, then set the address, and request
   *  the all 18 bytes like the uhci/ohci controllers.  However, I have included
   *  the code below just to show how it could be done.
   */
  
  struct REQUEST_PACKET packet;
  usb_request_packet(&packet, STDRD_GET_REQUEST, GET_DESCRIPTOR, DEVICE, 0, 0, 18);
  
  // send the "get_descriptor" packet (get 18 bytes)
  if (!ehci_control_in(dev_desc, &packet, 18, 64, 0))
    return FALSE;
  
  // reset the port
  if (!ehci_reset_port(port))
    return FALSE;
  
  // set address
  if (!ehci_set_address(dev_desc->max_packet_size, dev_address))
    return FALSE;
  
  // get the whole packet.
  memset(dev_desc, 0, 18);
  if (!ehci_control_in(dev_desc, &packet, 18, 64, dev_address))
    return FALSE;
  
  // print the descriptor
  printf("\n  Found Device Descriptor:"
         "\n                 len: %i"
         "\n                type: %i"
         "\n             version: %01X.%02X"
         "\n               class: 0x%02X"
         "\n            subclass: 0x%02X"
         "\n            protocol: 0x%02X"
         "\n     max packet size: %i"
         "\n           vendor id: 0x%04X"
         "\n          product id: 0x%04X"
         "\n         release ver: %i%i.%i%i"
         "\n   manufacture index: %i (index to a string)"
         "\n       product index: %i"
         "\n        serial index: %i"
         "\n   number of configs: %i",
         dev_desc->len, dev_desc->type, dev_desc->usb_ver >> 8, dev_desc->usb_ver & 0xFF, dev_desc->_class, dev_desc->subclass, 
         dev_desc->protocol, dev_desc->max_packet_size, dev_desc->vendorid, dev_desc->productid, 
         (dev_desc->device_rel & 0xF000) >> 12, (dev_desc->device_rel & 0x0F00) >> 8,
         (dev_desc->device_rel & 0x00F0) >> 4,  (dev_desc->device_rel & 0x000F) >> 0,
         dev_desc->manuf_indx, dev_desc->prod_indx, dev_desc->serial_indx, dev_desc->configs
  );
  
  return TRUE;
}

bool ehci_get_string(char *str, const int dev_address, const int max_packet_size, const int index) {
  struct REQUEST_PACKET packet;
  int i, j;
  
  if (index > 0) {
    usb_request_packet(&packet, STDRD_GET_REQUEST, GET_DESCRIPTOR, STRING, index, 0x0409, 255);  // 0x0409 = English
    
    // send the "get_descriptor" packet
    struct STRING_DESC str_desc;
    if (!ehci_control_in(&str_desc, &packet, 255, max_packet_size, dev_address))
      return FALSE;
    
    // convert 16-bit Unicode to 8-bit char
    // (we assume ascii here. You will need to call your actual conversion routine)
    j = (str_desc.len - 2);
    for (i=0; i<j; i++)
      str[i] = (char) str_desc.string[i];
    str[i] = 0;
    
    return TRUE;
  }
  
  return FALSE;
}

bool ehci_get_config(struct CONFIG_DESC *config_desc, const int dev_address, const int max_packet_size) {
  struct REQUEST_PACKET packet;
  
  // send the "get_descriptor" packet (get the first 9 bytes)
  usb_request_packet(&packet, STDRD_GET_REQUEST, GET_DESCRIPTOR, CONFIG, 0, 0, 9);
  if (!ehci_control_in(config_desc, &packet, 9, max_packet_size, dev_address))
    return FALSE;
  
  // send the "get_descriptor" packet (get all of the descriptor)
  usb_request_packet(&packet, STDRD_GET_REQUEST, GET_DESCRIPTOR, CONFIG, 0, 0, config_desc->tot_len);
  if (!ehci_control_in(config_desc, &packet, config_desc->tot_len, max_packet_size, dev_address))
    return FALSE;
  
  return TRUE;
}

// we found our camera, so do the rest of the demonstration
void ehci_do_video(struct DEVICE_DESC *dev_desc, const int dev_address) {
  struct REQUEST_PACKET packet;
  int i, j, k;
  bit8u byte;
  
  /*
   * at this point, we have gotten the whole device descriptor, reset the port,
   *  set the address to 'dev_address', and retrieved the whole descriptor again.
   */
  
  // get and print the manufacturer and product string descriptors (if present)
  char str[128];
  if (ehci_get_string(str, dev_address, dev_desc->max_packet_size, dev_desc->manuf_indx))
    printf("\n Manufacturer String: '%s'", str);
  if (ehci_get_string(str, dev_address, dev_desc->max_packet_size, dev_desc->prod_indx))
    printf("\n Product String: '%s'", str);
  
  // get the configuration descriptor
  bit8u config_buffer[1024];
  struct CONFIG_DESC *config_desc = (struct CONFIG_DESC *) config_buffer;
  if (!ehci_get_config(config_desc, dev_address, dev_desc->max_packet_size)) {
    printf("\n Error getting the device's Configuration Descriptor.");
    return;
  }
  
  // get and print the configuration string descriptor (if present)
  if (ehci_get_string(str, dev_address, dev_desc->max_packet_size, config_desc->config_indx))
    printf("\n Configuration String: '%s'", str);
  
  // DEBUG: Dump the configuration descriptor to the screen
  //dump(config_desc, config_desc->tot_len);
  
  // now parse the configuration descriptor for the interface associations, interface, and endpoint descriptors.
  // ignore the interfaces/functions we don't want (sound for example)
  struct INTERFACE_DESC *interface_desc;
  struct VIDCAM_INFO_S info;
  info.video_cntrl = FALSE;
  info.video_in = FALSE;
  info.video_out = FALSE;
  info.video_proc = FALSE;
  info.interface_assoc_index = -1;
  info.interface_count = 0;
  info.control_int = -1;
  info.stream_int = -1;
  info.format_cnt = 0;
  
  /* This code is written for and assumes the camera mentioned in the book.
   * Therefore, it is written so that it assumes a certain order of the following
   *   items.  You may need to change the code to accomidate your camera.
   */
  int iad = 0;
  bit8u *p = (bit8u *) ((bit8u *) config_desc + sizeof(struct CONFIG_DESC));
  while ((p < ((bit8u *) config_desc + config_desc->tot_len)) && p[0]) {
    if (p[1] == INTERFACE_ASSOSIATION) {
      iad++;
      struct INTERFACE_ASSOSIATION_DESC *interface_assoc_desc = (struct INTERFACE_ASSOSIATION_DESC *) p;
      if (interface_assoc_desc->_class == 0x0E) {
        if (info.interface_assoc_index > -1)
          printf("\n We already found a Video Interface Association Descriptor.  Why is there another?");
        info.interface_assoc_index = iad;
        info.first_int_num = interface_assoc_desc->interface_num;
        info.last_int_num = interface_assoc_desc->interface_num + interface_assoc_desc->count - 1;
      }
      
      p += p[0];
      continue;
    }
    
    // We only use items within the video association descriptor
    if (info.interface_assoc_index == iad) {
      switch (p[1]) {
        case INTERFACE:
          interface_desc = (struct INTERFACE_DESC *) p;
          // if this is the first interface descriptor, set up the structure
          if (interface_desc->alt_setting == 0) {
            info.interface_count++;
            if (interface_desc->interface_sub_class == 1)
              info.control_int = info.interface_count - 1;
            if (interface_desc->interface_sub_class == 2)
              info.stream_int = info.interface_count - 1;
            info.interfaces[info.interface_count - 1].int_count = 0;
            info.interfaces[info.interface_count - 1].ep_count = 0;
          } else {
            // if we did not find an endpoint for the first interface (x:0), then skip the first endpoint
            if (info.interfaces[info.interface_count - 1].ep_count == 0) {
              memset(&info.interfaces[info.interface_count - 1].pairs[info.interfaces[info.interface_count - 1].ep_count].endpt_desc, 0x00, sizeof(struct ENDPOINT_DESC));
              info.interfaces[info.interface_count - 1].ep_count++;
            }
          }
          // sanity check
          if (info.interface_count > 0) {
            memcpy(&info.interfaces[info.interface_count - 1].pairs[info.interfaces[info.interface_count - 1].int_count].intr_desc, p, sizeof(struct INTERFACE_DESC));
            info.interfaces[info.interface_count - 1].int_count++;
          }
          break;
        
        case ENDPOINT:
          // sanity check
          if (info.interface_count > 0) {
            memcpy(&info.interfaces[info.interface_count - 1].pairs[info.interfaces[info.interface_count - 1].ep_count].endpt_desc, p, sizeof(struct ENDPOINT_DESC));
            info.interfaces[info.interface_count - 1].ep_count++;
          }
          break;
          
        case INTERFACE_FUNCTION:
          // are we in the control interface or the streaming interface
          if ((info.interface_count - 1) == info.control_int) {
            switch (p[2]) {
              case 1:  // video control
                memcpy(&info.control_func, p, sizeof(struct VIDEO_CONTROL_FUNC_DESC));
                info.video_cntrl = TRUE;
                break;
              
              case 2:  // video input terminal
                memcpy(&info.vc_input_func, p, sizeof(struct VIDEO_INPUT_TERM_FUNC_DESC));  // not going to hurt to copy more
                info.video_in = TRUE;
                break;
              
              case 3:  // video output terminal
                memcpy(&info.vc_output_func, p, sizeof(struct VIDEO_OUTPUT_TERM_FUNC_DESC));
                info.video_out = TRUE;
                break;
              
              case 5:  // video processing unit
                memcpy(&info.vc_proc_func, p, sizeof(struct VIDEO_PROC_FUNC_DESC));
                info.video_proc = TRUE;
                break;
            }
          }
          // are we in the control interface or the streaming interface
          if ((info.interface_count - 1) == info.stream_int) {
            switch (p[2]) {
              case 1:  // video stream input header
                memcpy(&info.vs_header, p, sizeof(struct VIDEO_STREAM_HEADER_DESC));
                info.video_stream_hdr = TRUE;
                break;
                
              case 3:  // still frame
                break;
                
              case 4:  // format: uncompressed
                info.format_cnt++;
                memcpy(&info.formats[info.format_cnt - 1].format, p, sizeof(struct VS_FORMAT));
                info.formats[info.format_cnt - 1].frame_cnt = 0;
                break;
                
              case 5:  // frame: uncompressed
                info.formats[info.format_cnt - 1].frame_cnt++;
                memcpy(&info.formats[info.format_cnt - 1].frame[info.formats[info.format_cnt - 1].frame_cnt - 1], p, sizeof(struct VS_FRAME));
                break;
                
              case 13: // color matching
                break;
            }
            break;
          }
          break;
          
        case ENDPOINT_FUNCTION:
          switch (p[2]) {
            case 3:  // VC Processing Unit
              memcpy(&info.vc_proc_epfunc, p, sizeof(struct VIDEO_PROC_EPFUNC_DESC));
              info.video_out = TRUE;
              break;
          }
          break;
      }
    }
    
    p += p[0];
  }

  printf("\n\n **********************************************************************");
  printf("\n Interface Association index:  %i", info.interface_assoc_index);
  printf("\n            Found Interfaces:  %i", info.interface_count);
  printf("\n     Control Interface Index:  %i", info.control_int);
  printf("\n   Streaming Interface Index:  %i", info.stream_int);
  
  printf("\n\n Streaming Interface:");
  printf("\n      Alternate Interfaces:  %2i", info.interfaces[info.stream_int].int_count - 1);
  printf("\n       and their Endpoints:  %2i", info.interfaces[info.stream_int].ep_count - 1);
  for (i=1; i<info.interfaces[info.stream_int].int_count; i++) {
    printf("\n Interface %i.%2i:", info.stream_int, i);
    if (i < info.interfaces[info.stream_int].ep_count)
      printf(" ep = 0x%02X,  mps =%5i", info.interfaces[info.stream_int].pairs[i].endpt_desc.end_point, info.interfaces[info.stream_int].pairs[i].endpt_desc.max_packet_size);
  }
  
  printf("\n\n      Found Formats:  %i", info.format_cnt);
  for (i=0; i<info.format_cnt; i++) {
    printf("\n  Format: %i   Bits per Pixel: %i", i+1, info.formats[i].format.bits_pixel);
    printf("\n          Frame Count: %i", info.formats[i].frame_cnt);
    for (j=0; j<info.formats[i].frame_cnt; j++) {
      printf("\n          %i.%i:", i+1, j+1);
      printf("  %i x %i", info.formats[i].frame[j].width, info.formats[i].frame[j].height);
    }
  }
  
  // A few checks
  if (info.control_int == -1) {
    printf("\n Did not find Control Interface...");
    return;
  }

  if (info.stream_int == -1) {
    printf("\n Did not find Streaming Interface...");
    return;
  }

  if (!info.video_cntrl || (info.control_func.version != 0x0100)) {
    printf("\n No Video Control or wrong version found...");
    return;
  }
  
  if (!info.video_in || (info.vc_input_func.term_type != 0x0201)) {
    printf("\n No Video Control In or terminal type not a camera...");
    return;
  }
  
  if (!info.video_out || (info.vc_output_func.term_type != 0x0101)) {
    printf("\n No Video Control Out or Out terminal type not streaming...");
    return;
  }
  
  if (!info.video_proc) {
    printf("\n No Video Processing Unit Control found...");
    return;
  }
  
  i = 0;
  printf("\n");
  while ((i < 1) || (i > info.format_cnt)) {
    printf("Choose a Format (1 to %i): ", info.format_cnt);
    scanf("%i", &i);
  }
  
  j = 0;
  while ((j < 1) || (j > info.formats[i-1].frame_cnt)) {
    printf("Choose a Frame (1 to %i): ", info.formats[i-1].frame_cnt);
    scanf("%i", &j);
  }
  
  //  Set Config to 1
  usb_request_packet(&packet, STDRD_SET_REQUEST, SET_CONFIGURATION, 0, 1, 0, 0);
  if (!ehci_control_out(NULL, &packet, 0, dev_desc->max_packet_size, dev_address))
    return;
  
  //  Set Interface to 1.0 (Streaming Interface, alt = 0)
  usb_request_packet(&packet, STDRD_SET_INTERFACE, SET_INTERFACE, 0, 0, info.stream_int, 0);
  if (!ehci_control_out(NULL, &packet, 0, dev_desc->max_packet_size, dev_address))
    return;
  
  // DEBUG: Dump the information to the screen
  //printf("\n **** %i.%i   (%i x %i)", i, j, info.formats[i-1].frame[j-1].width, info.formats[i-1].frame[j-1].height);
  //printf("\n **** format indx = %i, frame index = %i", info.formats[i-1].format.index, info.formats[i-1].frame[j-1].index);
  //printf("\n **** frame interval = 0x%08X", info.formats[i-1].frame[j-1].def_frame_interval);
  
  /*
   * now get the capabilities
   */
  
  // brightness
  struct PROC_UNIT_ITEM_VALUES brightness;
  get_control_info(&brightness, info.vc_proc_func.unit_addr, PU_BRIGHTNESS_CONTROL, dev_desc->max_packet_size, dev_address);

  // contrast
  struct PROC_UNIT_ITEM_VALUES contrast;
  get_control_info(&contrast, info.vc_proc_func.unit_addr, PU_CONTRAST_CONTROL, dev_desc->max_packet_size, dev_address);
  
  // hue
  struct PROC_UNIT_ITEM_VALUES hue;
  get_control_info(&hue, info.vc_proc_func.unit_addr, PU_HUE_CONTROL, dev_desc->max_packet_size, dev_address);

  // saturation
  struct PROC_UNIT_ITEM_VALUES saturation;
  get_control_info(&saturation, info.vc_proc_func.unit_addr, PU_SATURATION_CONTROL, dev_desc->max_packet_size, dev_address);

  // sharpness
  struct PROC_UNIT_ITEM_VALUES sharpness;
  get_control_info(&sharpness, info.vc_proc_func.unit_addr, PU_SHARPNESS_CONTROL, dev_desc->max_packet_size, dev_address);

  // gamma
  struct PROC_UNIT_ITEM_VALUES gamma;
  get_control_info(&gamma, info.vc_proc_func.unit_addr, PU_GAMMA_CONTROL, dev_desc->max_packet_size, dev_address);

  // gain
  struct PROC_UNIT_ITEM_VALUES gain;
  get_control_info(&gain, info.vc_proc_func.unit_addr, PU_GAIN_CONTROL, dev_desc->max_packet_size, dev_address);
  
  // power_line
  struct PROC_UNIT_ITEM_VALUES power_line;
  get_control_info(&power_line, info.vc_proc_func.unit_addr, PU_POWER_LINE_FREQ_CONTROL, dev_desc->max_packet_size, dev_address);
  
  /*
   * Get the current settings by probing the contol
   */

  // (really don't need to do this first one, but we will for the book sake)
  struct VS_PROBE_COMMIT_CONTROL probe;
  memset(&probe, 0, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_CUR, VS_PROBE_CONTROL, 0, 
    (0 << 8) | info.stream_int, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  if (!ehci_control_in(&probe, &packet, sizeof(struct VS_PROBE_COMMIT_CONTROL), dev_desc->max_packet_size, dev_address))
    return;
  // DEBUG: Dump the information to the screen
  // printf("\n Receiving Probe:"); dump(&probe, 26);
  // 000917DA  00 00 01 01 15 16 05 00-00 00 00 00 00 00 00 00
  // 000917EA  00 00 00 60 09 00 B8 0B-00 00
  
  // send the probe
  memset(&probe, 0, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  probe.format_index = info.formats[i-1].format.index;
  probe.frame_index = info.formats[i-1].frame[j-1].index;
  probe.frame_interval = info.formats[i-1].frame[j-1].def_frame_interval;
  probe.max_payload_transer_size = 0;
  
  usb_request_packet(&packet, HOST_TO_DEV | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_SET_CUR, VS_PROBE_CONTROL, 0, 
    (0 << 8) | info.stream_int, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  if (!ehci_control_out(&probe, &packet, sizeof(struct VS_PROBE_COMMIT_CONTROL), dev_desc->max_packet_size, dev_address))
    return;
  
  // request it back
  usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_CUR, VS_PROBE_CONTROL, 0, 
    (0 << 8) | info.stream_int, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  if (!ehci_control_in(&probe, &packet, sizeof(struct VS_PROBE_COMMIT_CONTROL), dev_desc->max_packet_size, dev_address))
    return;
  // DEBUG: Dump the information to the screen
  //printf("\n Receiving Probe:"); dump(&probe, 26);

  // set the Power Line Frequency control
  byte = 1; // (1 = 50 Hz) (2 = 60 Hz)
  usb_request_packet(&packet, HOST_TO_DEV | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_SET_CUR, PU_POWER_LINE_FREQ_CONTROL, 0, 
    (info.vc_proc_func.unit_addr << 8) | 0, 1);
  if (!ehci_control_out(&byte, &packet, 1, dev_desc->max_packet_size, dev_address))
    printf("\n Error setting PU_POWER_LINE_FREQ_CONTROL to 1");
  
  // send the commit
  usb_request_packet(&packet, HOST_TO_DEV | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_SET_CUR, VS_COMMIT_CONTROL, 0, 
    (0 << 8) | info.stream_int, sizeof(struct VS_PROBE_COMMIT_CONTROL));
  if (!ehci_control_out(&probe, &packet, sizeof(struct VS_PROBE_COMMIT_CONTROL), dev_desc->max_packet_size, dev_address))
    return;
  
  // Since we know that we will only be using this camera, for this example anyway, we have all of the bandwidth
  //  of the USB bus.  Therefore, find alternate interface with the highest value max packet size, but a number 
  //  of transactions to 1 (value of zero)
  // must not assume that the alternates are in ascending order
  struct ENDPOINT_DESC *iso_endpt = NULL;
  int alt_int = -1;
  int current = -1;
  for (k=1; k<info.interfaces[info.stream_int].ep_count; k++) {
    int mps = (info.interfaces[info.stream_int].pairs[k].endpt_desc.max_packet_size & 0x7FF);
    int trns = (info.interfaces[info.stream_int].pairs[k].endpt_desc.max_packet_size >> 11) & 0x3;
    if ((mps > current) && (trns == 0)) {
      current = mps;
      alt_int = info.interfaces[info.stream_int].pairs[k].intr_desc.alt_setting;
      iso_endpt = &info.interfaces[info.stream_int].pairs[k].endpt_desc;
    }
  }
  if (alt_int == -1) {
    printf("\n Did not find an alternate interface with an endpoint's maxpacketsize large enough...");
    return;
  }
  
  // DEBUG: Dump the information to the screen
  // printf("\n  %i %i 0x%04X", alt_int, current,
  //   info.interfaces[info.stream_int].pairs[alt_int].endpt_desc.max_packet_size);
  
  //  Set Interface to info.stream_int:alt_int (Streaming Interface, alt = alt_int)
  usb_request_packet(&packet, STDRD_SET_INTERFACE, SET_INTERFACE, 0, alt_int, info.stream_int, 0);
  if (!ehci_control_out(NULL, &packet, 0, dev_desc->max_packet_size, dev_address))
    return;

  /*
   * Here is where you are ready to request the packets.
   * You should request them (for this camera anyway) every millisecond.
   * Therefore, place a single ISO request in each frame up to the amount
   *  you wish to collect.
   *
   * This is where you should actually set up a revolving set of buffers.
   * For example, set up 10 buffers and insert 10 ISO requests, 1 per frame
   *  each in consecuative frames.  When the first buffer is filled, retrieve
   *  the data from that buffer, then place another ISO request after the
   *  last one inserted, and point it to this buffer.  Continue on using
   *  a round robin type schedule parsing these 10 buffers and ISO requests.
   */

  /*
   * For the sake of this code, I am going to retrieve the first three to
   *  show you have it is done.  See Part 5 of the book on the format of
   *  this data, its header, and more information on what to do with the
   *  retrieved data.
   * 
   * It is not my intent to show you how to display video to the screen.
   *  It is my intent to show you how to retrieve the data from the camera
   *  and I have done that to this point.  The job of displaying the data
   *  to the screen is now yours.
   */

  bit8u payload[1024];
  
  // first one returns zero bytes (could be only this camera, could be all cameras)
  memset(payload, 0, 1024);
  int len = ehci_iso_in(payload, iso_endpt, current, dev_address);
  printf("\n Length returned: %i", len);
  if (len > 0)
    dump(payload, len);
  
  // second one returns what it needs to
  memset(payload, 0, 1024);
  len = ehci_iso_in(payload, iso_endpt, current, dev_address);
  printf("\n Length returned: %i", len);
  if (len > 0)
    dump(payload, len);

  // third one too
  memset(payload, 0, 1024);
  len = ehci_iso_in(payload, iso_endpt, current, dev_address);
  printf("\n Length returned: %i", len);
  if (len > 0)
    dump(payload, len);
  
  // At this point, and for the sake of this code and the book, I am
  //  simply returning to the caller, which then stops the controller,
  //  and returns to DOS.
  
}

// enable/disable one of the lists.
// if the async member is set, it disables/enables the asynchronous list, else the periodic list
// bit = 4 = periodic list
// bit = 5 = async list
bool ehci_enable_list(const bool enable, const int bit) {
  
  bit32u command;
  
  // first make sure that both bits are the same
  // should not modify the enable bit unless the status bit has the same value
  command = ehci_read_op_reg(EHC_OPS_USBCommand);
  if (ehci_handshake(EHC_OPS_USBStatus, (1<<(bit + 10)), (command & (1<<(bit + 0))) ? (1<<(bit + 10)) : 0, 100)) {
    if (enable) {
      if (!(command & (1<<(bit + 0))))
        ehci_write_op_reg(EHC_OPS_USBCommand, command | (1<<(bit + 0)));
      return ehci_handshake(EHC_OPS_USBStatus, (1<<(bit + 10)), (1<<(bit + 10)), 100);
    } else {
      if (command & (1<<(bit + 0)))
        ehci_write_op_reg(EHC_OPS_USBCommand, command & ~(1<<(bit + 0)));
      return ehci_handshake(EHC_OPS_USBStatus, (1<<(bit + 10)), 0, 100);
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
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_ENDPT_CAPS, (0 << 16) | ((i==0) ? (1<<15) : 0) | QH_HS_EPS_HS | (0<<8) | 0);
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_HUB_INFO, (1<<30));
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_NEXT_QTD_PTR, QH_HS_T1);
    ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_ALT_NEXT_QTD_PTR, QH_HS_T1);
    cur_addr += EHCI_QUEUE_HEAD_SIZE;
  }
  
  // backup and point the last one at the first one
  cur_addr -= EHCI_QUEUE_HEAD_SIZE;
  ehci_write_phy_mem(cur_addr + EHCI_QH_OFF_HORZ_PTR, (async_base | QH_HS_TYPE_QH | QH_HS_T0));
}

// initialize the periodic list (ISO's and Interrupts)
void ehci_init_periodic(const bit32u periodic_base) {
  int i;
  
  // the periodic list is a round robin set of (256, 512, or) 1204 list pointers.
  for (i=0; i<1024; i++)
    ehci_write_phy_mem(periodic_base + (i * sizeof(bit32u)), QH_HS_TYPE_ISO | QH_HS_T1);
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
  const bit32u packet = heap_alloc(64 + EHCI_QUEUE_HEAD_SIZE + (2 * EHCI_TD_SIZE));
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
  
  heap_free(packet);
  
  return (ret == SUCCESS);
}

bool ehci_control_in(const void *targ, struct REQUEST_PACKET *tpacket, const int len, const int max_packet, const bit8u address) {
  bool ret = FALSE;
  
  // allocate enough memory to hold the packet, queue, and the TD's
  const bit32u packet = heap_alloc(64 + EHCI_QUEUE_HEAD_SIZE + (16 * EHCI_TD_SIZE));
  bit32u queue = packet + 64;
  bit32u td0 = queue + EHCI_QUEUE_HEAD_SIZE;
  
  const bit32u buffer_addr = heap_alloc(len);  // get a physical address buffer and then copy from it later
  
  // copy the request packet from local memory to physical address memory
  ehci_copy_to_phy_mem(packet, tpacket, sizeof(struct REQUEST_PACKET));
  
  bool spd = 0;
  const int last = 1 + ((len + (max_packet-1)) / max_packet);
  ehci_queue(queue, td0, 0, max_packet, address);
  ehci_setup_packet(td0, packet);
  ehci_packet(td0 + EHCI_TD_SIZE, td0 + (last * EHCI_TD_SIZE), buffer_addr, len, FALSE, 1, EHCI_TD_PID_IN, max_packet);
  ehci_packet(td0 + (last * EHCI_TD_SIZE), NULL, NULL, 0, TRUE, 1, EHCI_TD_PID_OUT, max_packet);
  
  ehci_insert_queue(queue, QH_HS_TYPE_QH);
  int int_ret = ehci_wait_interrupt(td0, 2000, &spd);
  ehci_remove_queue(queue);
  
  if (int_ret == SUCCESS) {
    // now copy from the physical buffer to the specified buffer
    ehci_copy_from_phy_mem(targ, buffer_addr, len);
    ret = TRUE;
  }
  
  heap_free(packet);
  heap_free(buffer_addr);
  return ret;
}

bool ehci_control_out(void *src, struct REQUEST_PACKET *tpacket, const int len, const int max_packet, const bit8u address) {
  bool ret = FALSE;
  
  // allocate enough memory to hold the packet, queue, and the TD's
  const bit32u packet = heap_alloc(64 + EHCI_QUEUE_HEAD_SIZE + (16 * EHCI_TD_SIZE));
  bit32u queue = packet + 64;
  bit32u td0 = queue + EHCI_QUEUE_HEAD_SIZE;
  
  // copy the request packet from local memory to physical address memory
  ehci_copy_to_phy_mem(packet, tpacket, sizeof(struct REQUEST_PACKET));
  
  // copy from the specified buffer to the physical buffer
  const bit32u buffer_addr = heap_alloc(len);  // get a physical address buffer and copy to it
  ehci_copy_to_phy_mem(buffer_addr, src, len);
  
  bool spd = 0;
  const int last = 1 + ((len + (max_packet-1)) / max_packet);
  ehci_queue(queue, td0, 0, max_packet, address);
  ehci_setup_packet(td0, packet);
  if (len > 0)
    ehci_packet(td0 + EHCI_TD_SIZE, td0 + (last * EHCI_TD_SIZE), buffer_addr, len, FALSE, 1, EHCI_TD_PID_OUT, max_packet);
  ehci_packet(td0 + (last * EHCI_TD_SIZE), NULL, NULL, 0, TRUE, 1, EHCI_TD_PID_IN, max_packet);
  
  ehci_insert_queue(queue, QH_HS_TYPE_QH);
  int int_ret = ehci_wait_interrupt(td0, 2000, &spd);
  ehci_remove_queue(queue);
  
  if (int_ret == SUCCESS)
    ret = TRUE;
  
  heap_free(packet);
  heap_free(buffer_addr);
  return ret;
}

int ehci_iso_in(const void *targ, const struct ENDPOINT_DESC *endpt, const int len, const bit8u address) {
  const bit32u td0 = heap_alloc(EHCI_TD_SIZE);
  const bit32u buffer_addr = heap_alloc(len);  // get a physical address buffer and then copy from it later
  
  ehci_iso_packet(td0, buffer_addr, endpt, len, EHCI_TD_PID_IN, address);
  ehci_insert_periodic(td0);
  
  int ret_len = 0;
  int int_ret = ehci_wait_iso(td0, 2000, &ret_len);
  
  if ((int_ret == SUCCESS) && (ret_len > 0))
    // now copy from the physical buffer to the specified buffer
    ehci_copy_from_phy_mem(targ, buffer_addr, ret_len);
  
  heap_free(td0);
  heap_free(buffer_addr);
  return ret_len;
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

void ehci_iso_packet(const bit32u addr, const bit32u buffer_addr, const struct ENDPOINT_DESC *endpt, const int len, const bit8u dir, const bit8u device_addr) {
  int i, last_page, remaining, length;
  bit32u last_page_ptr, curr;
  
  curr = buffer_addr;
  remaining = len;
  last_page = -1;  // so that first increment = 0
  last_page_ptr = 0xFFFFFFFF;
  
  // clear it to zeros
  ehci_clear_phy_mem(addr, EHCI_TD_SIZE);
  
  // nextptr: no more after this one
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_NEXT_TD_PTR, 0 | QH_HS_TYPE_ISO | QH_HS_T1);
  
  // characteristics first (endpt, device address, etc)
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_0, ((endpt->end_point & 0x0F) << 8) | (device_addr << 0) );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_1, (dir << 11) | (endpt->max_packet_size & 0x7FF) );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_2, (1 << 0) );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_3, 0 );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_4, 0 );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_5, 0 );
  ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_6, 0 );
  
  // now the buffers and lengths
  for (i=0; i<8; i++) {
    if (remaining > 0) {
      if ((curr & 0xFFFFF000) != last_page_ptr) {
        last_page++;
        last_page_ptr = ehci_read_phy_mem(addr + EHCI_ISO_OFF_BUFFER_0 + (last_page * sizeof(bit32u)));
        last_page_ptr &= 0x00000FFF;  // make sure it is zero
        ehci_write_phy_mem(addr + EHCI_ISO_OFF_BUFFER_0 + (last_page * sizeof(bit32u)), (curr & 0xFFFFF000) | last_page_ptr);
        last_page_ptr = curr & 0xFFFFF000;
      }
      
      // calculate the length
      length = remaining;
      if (length > (endpt->max_packet_size & 0x7FF))
        length = (endpt->max_packet_size & 0x7FF);
      if (length > 1024)
        length = 1024;
      if (last_page_ptr != ((curr + length) & 0xFFFFF000))
        length = (last_page_ptr + 4096) - curr;
      
      ehci_write_phy_mem(addr + EHCI_ISO_OFF_STATUS_0 + (i * sizeof(bit32u)), 
             (1<<31) | 
             (0x0FFF0000 & (length << 16)) |
             (0x00007000 & (last_page << 12)) |
             (0x00000FFF & curr));
      
      // increment for next one (if any)
      remaining -= length;
      curr += length;
    } else
      ehci_write_phy_mem(addr + EHCI_ISO_OFF_STATUS_0 + (i * sizeof(bit32u)), 0x00000000);
  }

  // DEBUG: Dump the information to the screen
  // printf("\n ISO dump: (0x%08X)", buffer_addr);
  // for (i=0; i<16; i++)
  //   printf("\n %i: 0x%08X", i, ehci_read_phy_mem(addr + (i * 4)));
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

// we assume we are using a 1024 size perodic list
// will need to modify if using a smaller list
void ehci_insert_periodic(const bit32u addr) {
  
  bit32u indx, frame_index = ehci_read_op_reg(EHC_OPS_FrameIndex);
  
  // bits 31:14 = reserved
  //         13 = toggle
  //      12:03 = frame index
  //      02:00 = microframe index
  indx = ((frame_index >> 3) & 0x3FF) + 1;
  
  // make sure we didn't go past end of list
  indx &= 0x3FF;
  
  // this assumes that we are not overwritting anything (as in an un executed iTD)
  // normally, you should check to make sure, but for the example here, we assume we are not.
  ehci_write_phy_mem(periodic_base + (indx * sizeof(bit32u)), addr | QH_HS_TYPE_ISO | QH_HS_T0);
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

// timeout = milliseconds
int ehci_wait_iso(bit32u addr, const bit32u timeout, int *ret_len) {
  
  int timer = timeout * 100, i = 0, ret = -1;
  bit32u status, len = 0;
  
  if (ret_len) *ret_len = 0;
  
  while (timer) {
    status = ehci_read_phy_mem(addr + (EHCI_ISO_OFF_STATUS_0 + (i * sizeof(bit32u))));
    if ((status & 0x80000000) == 0x00) {
      if ((status & 0x70000000) != 0x00) {
        if (status & (1<<30))
          ret = ERROR_DATA_BUFFER_ERROR;
        else if (status & (1<<29))
          ret = ERROR_BABBLE_DETECTED;
        else if (status & (1<<28))
          ret = ERROR_NAK;
        return ret;
      }
      len += ((status & 0x0FFF0000) >> 16);
      i++;
      if (i == 8) {
        if (ret_len) *ret_len = len;
        return SUCCESS;
      }
    } else {
      udelay(10);
      timer--;
    }
  }
  
  if (ret == -1) {
    printf("\n USB EHCI ISO Interrupt wait timed out.");
    ret = ERROR_TIME_OUT;
  }
  
  return ret;
}

void ehci_clear_phy_mem(const bit32u address, const int len) {
  int i;
  
  for (i=0; i<len; i++)
    _farpokeb(heap_selector, get_linear(address) + i, 0);
}

void ehci_copy_to_phy_mem(const bit32u address, void *src, const int len) {
  bit8u *s = (bit8u *) src;
  int i;
  
  for (i=0; i<len; i++)
    _farpokeb(heap_selector, get_linear(address) + i, s[i]);
}

void ehci_copy_from_phy_mem(const void *targ, const bit32u address, const int len) {
  bit8u *t = (bit8u *) targ;
  int i;
  
  for (i=0; i<len; i++)
    t[i] = _farpeekb(heap_selector, get_linear(address) + i);
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

void get_control_info(struct PROC_UNIT_ITEM_VALUES *item, const int unit, const int control, const bit8u mps, const int addr) {
  struct REQUEST_PACKET packet;
  bit16u buff;
  
  memset(item, 0, sizeof(struct PROC_UNIT_ITEM_VALUES));
  
  // first GET_INFO to see if we can do get and set
  usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_INFO, control, 0, (unit << 8) | 0, 1);
  if (!ehci_control_in(&buff, &packet, 1, mps, addr))
    return;
  item->info = (bit8u) buff;
  
  // allows GET_ ?
  if (buff & (1<<0)) {
    // then get length
    usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_LEN, control, 0, (unit << 8) | 0, 2);
    if (!ehci_control_in(&buff, &packet, 2, mps, addr))
      return;
    item->len = buff;
    
    // make sure len is either a byte or a word
    if ((item->len < 1) || (item->len > 2))
      return;
    
    // then get min
    buff = 0; // (incase len = 1)
    usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_MIN, control, 0, (unit << 8) | 0, item->len);
    if (!ehci_control_in(&buff, &packet, item->len, mps, addr))
      return;
    item->min = buff;

    // then max
    buff = 0; // (incase len = 1)
    usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_MAX, control, 0, (unit << 8) | 0, item->len);
    if (!ehci_control_in(&buff, &packet, item->len, mps, addr))
      return;
    item->max = buff;

    // res
    buff = 0; // (incase len = 1)
    usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_RES, control, 0, (unit << 8) | 0, item->len);
    if (!ehci_control_in(&buff, &packet, item->len, mps, addr))
      return;
    item->res = buff;

    // and finally default
    buff = 0; // (incase len = 1)
    usb_request_packet(&packet, DEV_TO_HOST | REQ_TYPE_CLASS | RECPT_INTERFACE, VID_GET_DEF, control, 0, (unit << 8) | 0, item->len);
    if (!ehci_control_in(&buff, &packet, item->len, mps, addr))
      return;
    item->def = buff;
  }
}
