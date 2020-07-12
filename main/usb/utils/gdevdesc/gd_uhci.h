//////////////////////////////////////////////////////////////////////////
//  gd_uhci.h  v1.00
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS_GD_UHCI
#define FYSOS_GD_UHCI


#pragma pack(1)


bool process_uhci(struct PCI_DEV *, struct PCI_POS *);

bool uhci_port_pres(bit16u, bit8u);
bool uhci_port_reset(bit16u, bit8u);
bool uhci_get_descriptor(const bit16u, const bit32u, const int, struct DEVICE_DESC *, const bool, const int, const int, const int);
bool uhci_set_address(const bit16u, const bit32u, const int, const int, const bool);


#endif  // FYSOS_GD_UHCI
