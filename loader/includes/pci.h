
#ifndef _PCI_H
#define _PCI_H

struct S_BIOS_PCI {
  bit32u sig;
  bit8u  flags;
  bit8u  major;
  bit8u  minor;
  bit8u  last;
};

void get_pci_info(struct S_BIOS_PCI *);



#endif   // _PCI_H
