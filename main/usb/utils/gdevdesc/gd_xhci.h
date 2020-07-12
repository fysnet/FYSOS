//////////////////////////////////////////////////////////////////////////
//  gd_xhci.h  v1.00
//////////////////////////////////////////////////////////////////////////

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
