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

#ifndef FYSOS_USB
#define FYSOS_USB

#pragma pack(1)

// Controller Types
#define xHC_TYPE_UHCI  0x00
#define xHC_TYPE_OHCI  0x10
#define xHC_TYPE_EHCI  0x20
#define xHC_TYPE_XHCI  0x30

enum { CONTROL_EP=0, ISOCHRONOUS_EP, BULK_EP, INTERRUPT_EP };  // transfer types (Endpoint types) (USB 2.0 page 270)

// Reset wait times.  USB 2.0 specs, page 153, section 7.1.7.5, paragraph 3
#define USB_TDRSTR   50   // reset on a root hub
#define USB_TDRST    10   // minimum delay for a reset
#define USB_TRHRSI    3   // No more than this between resets for root hubs
#define USB_TRSTRCY  10   // reset recovery

struct DEVICE_DESC {
  bit8u  len;
  bit8u  type;
  bit16u usb_ver;
  bit8u  _class;
  bit8u  subclass;
  bit8u  protocol;
  bit8u  max_packet_size;
  bit16u vendorid;
  bit16u productid;
  bit16u device_rel;
  bit8u  manuf_indx;   // index value
  bit8u  prod_indx;    // index value
  bit8u  serial_indx;  // index value
  bit8u  configs;      // Number of configurations
};

struct REQUEST_PACKET {
  bit8u  request_type;
  bit8u  request;
  bit16u value;
  bit16u index;
  bit16u length;
};

// config descriptor
struct CONFIG_DESC {
  bit8u  len;
  bit8u  type;
  bit16u tot_len;
  bit8u  num_interfaces;
  bit8u  config_val;
  bit8u  config_indx;
  bit8u  bm_attrbs;
  bit8u  max_power;
};

struct STRING_DESC {
  bit8u  len;         // length of whole desc in bytes
  bit8u  type;
  bit16u string[127];
};

struct INTERFACE_ASSOSIATION_DESC {
  bit8u  len;             // len of this desc (8)
  bit8u  type;            // type = 11
  bit8u  interface_num;   // first interface number to start association
  bit8u  count;           // count of continuous interfaces for association
  bit8u  _class;          //
  bit8u  subclass;        //
  bit8u  protocol;        //
  bit8u  function_indx;   // string id of this association
};

// interface descriptor
struct INTERFACE_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  interface_num;
  bit8u  alt_setting;
  bit8u  num_endpoints;
  bit8u  interface_class;
  bit8u  interface_sub_class;
  bit8u  interface_protocol;
  bit8u  interface_indx;
};

// endpoint descriptor
struct ENDPOINT_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  end_point;        // 6:0 end_point number, 7 = IN (set) or OUT (clear)
  bit8u  bm_attrbs;        // 
  bit16u max_packet_size;  // 10:0 = max_size, 12:11 = max transactions, 15:13 = reserved
  bit8u  interval;
};

// setup packets
#define DEV_TO_HOST     0x80
#define HOST_TO_DEV     0x00
#define REQ_TYPE_STNDRD 0x00
#define REQ_TYPE_CLASS  0x20
#define REQ_TYPE_VENDOR 0x40
#define REQ_TYPE_RESV   0x60
#define RECPT_DEVICE    0x00
#define RECPT_INTERFACE 0x01
#define RECPT_ENDPOINT  0x02
#define RECPT_OTHER     0x03
#define STDRD_GET_REQUEST   (DEV_TO_HOST | REQ_TYPE_STNDRD | RECPT_DEVICE)
#define STDRD_SET_REQUEST   (HOST_TO_DEV | REQ_TYPE_STNDRD | RECPT_DEVICE)
#define STDRD_SET_INTERFACE (HOST_TO_DEV | REQ_TYPE_STNDRD | RECPT_INTERFACE)

// device requests
enum { GET_STATUS=0, CLEAR_FEATURE, SET_FEATURE=3, SET_ADDRESS=5, GET_DESCRIPTOR=6, SET_DESCRIPTOR,
        GET_CONFIGURATION, SET_CONFIGURATION,
// interface requests
        GET_INTERFACE, SET_INTERFACE,
// standard endpoint requests
        SYNCH_FRAME,
// Device specific
        GET_MAX_LUNS = 0xFE, BULK_ONLY_RESET
};

// Descriptor types
enum { 
  DEVICE=1, 
  CONFIG, 
  STRING, 
  INTERFACE, 
  ENDPOINT, 
  DEVICE_QUALIFIER,
  OTHER_SPEED_CONFIG, 
  INTERFACE_POWER, 
  OTG, 
  DEBUG, 
  INTERFACE_ASSOSIATION,
  
  HID=0x21,
  HID_REPORT, 
  HID_PHYSICAL, 
  
  INTERFACE_FUNCTION = 0x24,
  ENDPOINT_FUNCTION,
  
  HUB=0x29
};

#endif // FYSOS_USB
