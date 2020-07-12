//////////////////////////////////////////////////////////////////////////
//  gd_ehci.h  v01.10.00
//////////////////////////////////////////////////////////////////////////

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
