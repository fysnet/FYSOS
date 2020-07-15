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

#ifndef FYSOS_GD_EHCI
#define FYSOS_GD_EHCI


#pragma pack(1)


bool process_ehci(struct PCI_DEV *, struct PCI_POS *);
bit32u heap_alloc(bit32u, const bit32u);


void ehci_init_stack_frame(const bit32u);
bool ehci_stop_legacy(const struct PCI_POS *, const bit32u);

bool ehci_enable_async_list(const bool);
bool ehci_handshake(const bit32u, const bit32u, const bit32u, unsigned);

bool ehci_reset_port(const int);
bool ehci_get_descriptor(const int);

void ehci_clear_phy_mem(const bit32u, const int);
void ehci_copy_to_phy_mem(const bit32u, void *, const int);
void ehci_copy_from_phy_mem(void *, const bit32u, const int);
void ehci_write_phy_mem(const bit32u, bit32u);
bit32u ehci_read_phy_mem(const bit32u);

bool ehci_set_address(const bit8u, const bit8u);
bool ehci_control_in(void *, const int, const int, const bit8u);
void ehci_queue(bit32u, const bit32u, const bit8u, const bit16u, const bit8u);
int ehci_setup_packet(const bit32u, bit32u);
int ehci_packet(bit32u, const bit32u, bit32u, const bit32u, const bool, bit8u, const bit8u, const bit16u);
void ehci_insert_queue(bit32u, const bit8u);
bool ehci_remove_queue(bit32u);
int ehci_wait_interrupt(bit32u, const bit32u, bool *);

void ehci_write_cap_reg(const bit32u, const bit32u);
void ehci_write_op_reg(const bit32u, const bit32u);
bit32u ehci_read_cap_reg(const bit32u);
bit32u ehci_read_op_reg(const bit32u);


#endif  // FYSOS_GD_EHCI
