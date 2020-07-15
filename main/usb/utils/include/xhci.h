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

#ifndef FYSOS__XHCI
#define FYSOS__XHCI

#pragma pack(1)

#define xHC_CAPS_CapLength      0x00
#define xHC_CAPS_Reserved       0x01
#define xHC_CAPS_IVersion       0x02
#define xHC_CAPS_HCSParams1     0x04
#define xHC_CAPS_HCSParams2     0x08
#define xHC_CAPS_HCSParams3     0x0C
#define xHC_CAPS_HCCParams1     0x10
#define xHC_CAPS_DBOFF          0x14
#define xHC_CAPS_RTSOFF         0x18
#define xHC_CAPS_HCCParams2     0x1C

#define xHC_OPS_USBCommand      0x00
#define xHC_OPS_USBStatus       0x04
#define xHC_OPS_USBPageSize     0x08
#define xHC_OPS_USBDnctrl       0x14
#define xHC_OPS_USBCrcr         0x18
#define xHC_OPS_USBDcbaap       0x30
#define xHC_OPS_USBConfig       0x38

#define xHC_OPS_USBPortSt       0x400
#define xHC_Port_PORTSC             0
#define xHC_Port_PORTPMSC           4
#define xHC_Port_PORTLI             8
#define xHC_Port_PORTHLPMC         12

#define xHC_PortUSB_CHANGE_BITS  ((1<<17) | (1<<18) | (1<<20) | (1<<21) | (1<<22))
                             /// Quirk:  TI TUSB7340: sets bit 19 on USB2 ports  ??????????????

#define xHC_INTERRUPTER_PRIMARY      0

#define xHC_INTERRUPTER_IMAN      0x00
#define xHC_INTERRUPTER_IMOD      0x04
#define xHC_INTERRUPTER_TAB_SIZE  0x08
#define xHC_INTERRUPTER_RESV      0x0C
#define xHC_INTERRUPTER_ADDRESS   0x10
#define xHC_INTERRUPTER_DEQUEUE   0x18

// xHCI speed values
#define xHCI_SPEED_FULL   1
#define xHCI_SPEED_LOW    2
#define xHCI_SPEED_HI     3
#define xHCI_SPEED_SUPER  4

#define xHCI_DIR_NO_DATA  0
#define xHCI_DIR_OUT      2
#define xHCI_DIR_IN       3

#define xHCI_DIR_OUT_B  0
#define xHCI_DIR_IN_B   1

// End Point Doorbell numbers
#define xHCI_SLOT_CNTX   0
#define xHCI_CONTROL_EP  1
#define xHCI_EP1_OUT     2
#define xHCI_EP1_IN      3
#define xHCI_EP2_OUT     4
#define xHCI_EP2_IN      5
#define xHCI_EP3_OUT     6
#define xHCI_EP3_IN      7
#define xHCI_EP4_OUT     8
#define xHCI_EP4_IN      9
#define xHCI_EP5_OUT     10
#define xHCI_EP5_IN      11
#define xHCI_EP6_OUT     12
#define xHCI_EP6_IN      13
#define xHCI_EP7_OUT     14
#define xHCI_EP7_IN      15
#define xHCI_EP8_OUT     16
#define xHCI_EP8_IN      17
#define xHCI_EP9_OUT     18
#define xHCI_EP9_IN      19
#define xHCI_EP10_OUT    20
#define xHCI_EP10_IN     21
#define xHCI_EP11_OUT    22
#define xHCI_EP11_IN     23
#define xHCI_EP12_OUT    24
#define xHCI_EP12_IN     25
#define xHCI_EP13_OUT    26
#define xHCI_EP13_IN     27
#define xHCI_EP14_OUT    28
#define xHCI_EP14_IN     29
#define xHCI_EP15_OUT    30
#define xHCI_EP15_IN     31

// Port_info flags
#define xHCI_PROTO_INFO           (1<<0)  // bit 0 set = USB3, else USB2
#define xHCI_PROTO_HSO            (1<<1)  // bit 1 set = is USB 2 and High Speed Only
#define xHCI_PROTO_HAS_PAIR       (1<<2)  // bit 2 set = has a corresponding port. (i.e.: is a USB3 and has USB2 port (a must))
                                          //     clear = does not have a corr. port (i.e.: is a USB2 port and does not have a USB3 port)
#define xHCI_PROTO_ACTIVE         (1<<3)  // is the active port of the pair.

#define xHCI_PROTO_USB2  0
#define xHCI_PROTO_USB3  1

#define xHCI_IS_USB3_PORT(x)  ((port_info[(x)].flags & xHCI_PROTO_INFO) == xHCI_PROTO_USB3)
#define xHCI_IS_USB2_PORT(x)  ((port_info[(x)].flags & xHCI_PROTO_INFO) == xHCI_PROTO_USB2)
#define xHCI_IS_USB2_HSO(x)   ((port_info[(x)].flags & xHCI_PROTO_HSO) == xHCI_PROTO_HSO)
#define xHCI_HAS_PAIR(x)      ((port_info[(x)].flags & xHCI_PROTO_HAS_PAIR) == xHCI_PROTO_HAS_PAIR)
#define xHCI_IS_ACTIVE(x)     ((port_info[(x)].flags & xHCI_PROTO_ACTIVE) == xHCI_PROTO_ACTIVE)


#define xHC_TRB_ID_LINK         6
#define xHC_TRB_ID_NOOP         8

struct xHC_PORT_STATUS {
  bit32u portsc;
  bit32u portpmsc;
  bit32u portli;
  bit32u porthlpmc;
};

#define xHC_xECP_ID_NONE       0
#define xHC_xECP_ID_LEGACY     1
#define xHC_xECP_ID_PROTO      2
#define xHC_xECP_ID_POWER      3
#define xHC_xECP_ID_VIRT       4
#define xHC_xECP_ID_MESS       5
#define xHC_xECP_ID_LOCAL      6
#define xHC_xECP_ID_DEBUG     10
#define xHC_xECP_ID_EXT_MESS  17

#define xHC_xECP_LEGACY_TIMEOUT     10  // 10 milliseconds
#define xHC_xECP_LEGACY_BIOS_OWNED  (1<<16)
#define xHC_xECP_LEGACY_OS_OWNED    (1<<24)
#define xHC_xECP_LEGACY_OWNED_MASK  (xHC_xECP_LEGACY_BIOS_OWNED | xHC_xECP_LEGACY_OS_OWNED)
struct xHC_xECP_LEGACY {
  bit32u volatile id_next_owner_flags;
  bit32u volatile cntrl_status;
};

struct xHC_xECP_PROTO {
  bit8u  id;
  bit8u  next;
  bit8u  minor;
  bit8u  major;
  bit32u name;
  bit8u  offset;
  bit8u  count;
  bit16u flags;
};

#define MAX_CONTEXT_SIZE   64                               // Max Context size in bytes
#define MAX_SLOT_SIZE      (MAX_CONTEXT_SIZE * 32)          // Max Total Slot size in bytes

// Slot State
#define SLOT_STATE_DISABLED_ENABLED  0
#define SLOT_STATE_DEFAULT           1
#define SLOT_STATE_ADRESSED          2
#define SLOT_STATE_CONFIGURED        3

struct xHCI_SLOT_CONTEXT {
  unsigned entries;
  bit8u    hub;
  bit8u    mtt;
  unsigned speed;
  bit32u   route_string;
  unsigned num_ports;
  unsigned rh_port_num;
  unsigned max_exit_latency;
  unsigned int_target;
  unsigned ttt;
  unsigned tt_port_num;
  unsigned tt_hub_slot_id;
  unsigned slot_state;
  unsigned device_address;
};

// EP State
#define EP_STATE_DISABLED 0
#define EP_STATE_RUNNING  1
#define EP_STATE_HALTED   2
#define EP_STATE_STOPPED  3
#define EP_STATE_ERROR    4

struct xHCI_EP_CONTEXT {
  unsigned interval;
  bit8u    lsa;
  unsigned max_pstreams;
  unsigned mult;
  unsigned ep_state;
  unsigned max_packet_size;
  unsigned max_burst_size;
  bit8u    hid;
  unsigned ep_type;
  unsigned cerr;
  bit64u   tr_dequeue_pointer;
  bit8u    dcs;
  unsigned max_esit_payload;
  unsigned average_trb_len;
};

struct xHCI_TRB {
  bit64u param;
  bit32u status;
  bit32u command;
};

// event ring specification
struct xHCI_EVENT_SEG_TABLE {
  bit64u addr;
  bit32u size;
  bit32u resv;
};

#define XHCI_DIR_EP_OUT   0
#define XHCI_DIR_EP_IN    1
#define XHCI_GET_DIR(x)      (((x) & (1    <<  7)) >> 7)

#define TRB_GET_STYPE(x)     (((x) & (0x1F << 16)) >> 16)
#define TRB_SET_STYPE(x)     (((x) & 0x1F) << 16)
#define TRB_GET_TYPE(x)      (((x) & (0x3F << 10)) >> 10)
#define TRB_SET_TYPE(x)      (((x) & 0x3F) << 10)
#define TRB_GET_COMP_CODE(x) (((x) & (0x7F << 24)) >> 24)
#define TRB_SET_COMP_CODE(x) (((x) & 0x7F) << 24)
#define TRB_GET_SLOT(x)      (((x) & (0xFF << 24)) >> 24)
#define TRB_SET_SLOT(x)      (((x) & 0xFF) << 24)
#define TRB_GET_TDSIZE(x)    (((x) & (0x1F << 17)) >> 17)
#define TRB_SET_TDSIZE(x)    (((x) & 0x1F) << 17)
#define TRB_GET_EP(x)        (((x) & (0x1F << 16)) >> 16)
#define TRB_SET_EP(x)        (((x) & 0x1F) << 16)

#define TRB_GET_TARGET(x)    (((x) & (0x3FF << 22)) >> 22)
#define TRB_GET_TX_LEN(x)     ((x) & 0x1FFFF)
#define TRB_GET_TOGGLE(x)    (((x) & (1<<1)) >> 1)

#define TRB_DC(x)            (((x) & (1<<9)) >> 9)
#define TRB_IS_IMMED_DATA(x) (((x) & (1<<6)) >> 6)
#define TRB_IOC(x)           (((x) & (1<<5)) >> 5)
#define TRB_CHAIN(x)         (((x) & (1<<4)) >> 4)
#define TRB_SPD(x)           (((x) & (1<<2)) >> 2)
#define TRB_TOGGLE(x)        (((x) & (1<<1)) >> 1)

#define TRB_CYCLE_ON          (1<<0)
#define TRB_CYCLE_OFF         (0<<0)

#define TRB_TOGGLE_CYCLE_ON   (1<<1)
#define TRB_TOGGLE_CYCLE_OFF  (0<<1)

#define TRB_CHAIN_ON          (1<<4)
#define TRB_CHAIN_OFF         (0<<4)

#define TRB_IOC_ON            (1<<5)
#define TRB_IOC_OFF           (0<<5)

#define TRB_LINK_CMND         (TRB_SET_TYPE(LINK) | TRB_IOC_OFF | TRB_CHAIN_OFF | TRB_TOGGLE_CYCLE_OFF | TRB_CYCLE_ON)

// Common TRB types
enum { NORMAL=1, SETUP_STAGE, DATA_STAGE, STATUS_STAGE, ISOCH, LINK, EVENT_DATA, NO_OP,
       ENABLE_SLOT=9, DISABLE_SLOT, ADDRESS_DEVICE, CONFIG_EP, EVALUATE_CONTEXT, RESET_EP,
       STOP_EP=15, SET_TR_DEQUEUE, RESET_DEVICE, FORCE_EVENT, DEG_BANDWIDTH, SET_LAT_TOLERANCE,
       GET_PORT_BAND=21, FORCE_HEADER, NO_OP_CMD,  // 24 - 31 = reserved
       TRANS_EVENT=32, COMMAND_COMPLETION, PORT_STATUS_CHANGE, BANDWIDTH_REQUEST, DOORBELL_EVENT,
       HOST_CONTROLLER_EVENT=37, DEVICE_NOTIFICATION, MFINDEX_WRAP, 
       // 40 - 47 = reserved
       // 48 - 63 = Vendor Defined
};

// event completion codes
enum { TRB_SUCCESS=1, DATA_BUFFER_ERROR, BABBLE_DETECTION, TRANSACTION_ERROR, TRB_ERROR, STALL_ERROR,
       RESOURCE_ERROR=7, BANDWIDTH_ERROR, NO_SLOTS_ERROR, INVALID_STREAM_TYPE, SLOT_NOT_ENABLED, EP_NOT_ENABLED,
       SHORT_PACKET=13, RING_UNDERRUN, RUNG_OVERRUN, VF_EVENT_RING_FULL, PARAMETER_ERROR, BANDWITDH_OVERRUN,
       CONTEXT_STATE_ERROR=19, NO_PING_RESPONSE, EVENT_RING_FULL, INCOMPATIBLE_DEVICE, MISSED_SERVICE,
       COMMAND_RING_STOPPED=24, COMMAND_ABORTED, STOPPED, STOPPER_LENGTH_ERROR, RESERVED, ISOCH_BUFFER_OVERRUN,
       EVERN_LOST=32, UNDEFINED, INVALID_STREAM_ID, SECONDARY_BANDWIDTH, SPLIT_TRANSACTION
       /* 37 - 191 reserved */
       /* 192 - 223 vender defined errors */
       /* 224 - 225 vendor defined info */
};

#define XHCI_IRQ_DONE  (1<<31)

#endif // FYSOS__XHCI
