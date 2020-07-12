//////////////////////////////////////////////////////////////////////////
//  gd_ohci.h  v1.00
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS_GD_OHCI
#define FYSOS_GD_OHCI


#pragma pack(1)


bool process_ohci(struct PCI_DEV *, struct PCI_POS *);


bool ohci_reset_port(int, int);
void ohci_create_stack(struct OHCI_FRAME *, const bit32u, const int, int, const bool, const int);
void ohci_set_address(struct OHCI_FRAME *, bit32u, int, bool);
bool ohci_request_desc(struct OHCI_FRAME *, __dpmi_meminfo *, int, int, int);

void ohci_write_op_reg(const bit32u, const bit32u);
bit32u ohci_read_op_reg(const bit32u);


#endif  // FYSOS_GD_OHCI
