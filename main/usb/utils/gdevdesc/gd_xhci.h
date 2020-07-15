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

#ifndef FYSOS_GD_XHCI
#define FYSOS_GD_XHCI


#pragma pack(1)


#define CMND_RING_TRBS   128  // not more than 4096

#define TRBS_PER_RING    256

// Port_info:
struct S_XHCI_PORT_INFO {
  bit8u  flags;                // port_info flags below
  bit8u  other_port_num;       // zero based offset to other speed port
  bit8u  offset;               // offset of this port within this protocol
  bit8u  reserved;
};

bool process_xhci(struct PCI_DEV *, struct PCI_POS *);
bool xhci_get_descriptor(const int);
bit32u heap_alloc(bit32u, const bit32u, const bit32u);

int xhci_wait_for_interrupt(bit32u);
bit32u xhci_get_proto_offset(bit32u, const int, int *, int *, bit16u *);
bool xhci_stop_legacy(bit32u);
bit32u create_ring(const int);
bit32u create_event_ring(const int, bit32u *);
bool xhci_reset_port(const int);
bool xhci_send_command(struct xHCI_TRB *, const bool);
void xhci_get_trb(struct xHCI_TRB *, const bit32u);
void xhci_set_trb(struct xHCI_TRB *, const bit32u);
bool xhci_set_address(const bit32u, const int, const bool);
bit32u xhci_initialize_slot(const int, const int, const int, const int );
void xhci_initialize_ep(const bit32u, const int, const int, const bit32u, 
                        const int, const bool, const int, const int);
void write_to_slot(const bit32u, struct xHCI_SLOT_CONTEXT *);
void write_to_ep(const bit32u, struct xHCI_EP_CONTEXT *);
void read_from_slot(struct xHCI_SLOT_CONTEXT *, const bit32u);
void read_from_ep(struct xHCI_EP_CONTEXT *, const bit32u);
bool xhci_control_in(void *, const int, const int, const int);

int xhci_setup_stage(const struct REQUEST_PACKET *, const bit8u);
int xhci_data_stage(bit32u, bit8u, const bit32u, bit8u, const bit16u, const bit32u);
int xhci_status_stage(const bit8u, const bit32u);

void xhci_write_cap_reg(const bit32u, const bit32u);
void xhci_write_cap_reg64(const bit32u, const bit64u);
void xhci_write_op_reg(const bit32u, const bit32u);
void xhci_write_op_reg64(const bit32u, const bit64u);

bit32u xhci_read_cap_reg(const bit32u);
bit64u xhci_read_cap_reg64(const bit32u);
bit32u xhci_read_op_reg(const bit32u);
bit64u xhci_read_op_reg64(const bit32u);

void xhci_copy_from_phy_mem(void *, const bit32u, const int);
void xhci_write_phy_mem(const bit32u, bit32u);
void xhci_write_phy_mem64(const bit32u, bit64u);
bit32u xhci_read_phy_mem(const bit32u);
bit64u xhci_read_phy_mem64(const bit32u);

void xhci_write_doorbell(const bit32u, const bit32u);

void xhci_write_primary_intr(const bit32u, const bit32u);
void xhci_write_primary_intr64(const bit32u, const bit64u);
bit32u xhci_read_primary_intr(const bit32u);
bit64u xhci_read_primary_intr64(const bit32u);

void xhci_irq();

#endif  // FYSOS_GD_XHCI
