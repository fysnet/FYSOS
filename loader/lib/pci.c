
#include "ctype.h"

#include "paraport.h"
#include "pci.h"
#include "string.h"
#include "sys.h"
#include "windows.h"

void get_pci_info(struct S_BIOS_PCI *info) {
  struct REGS regs;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear the buffer first
  memset(info, 0, sizeof(struct S_BIOS_PCI));
  
  if (spc_key_F2)
    para_printf("Getting PCI information.\n");
  
  // call the service
  regs.eax = 0x0000B101;
  regs.edi = 0x00000000;
  if (!intx(0x1A, &regs) && !(regs.eax & 0x0000FF00)) {
    info->sig = regs.edx;
    info->flags = (bit8u) ((regs.eax >> 0) & 0xFF);
    info->major = (bit8u) ((regs.ebx >> 8) & 0xFF);
    info->minor = (bit8u) ((regs.ebx >> 0) & 0xFF);
    info->last  = (bit8u) ((regs.ecx >> 0) & 0xFF);

    if (spc_key_F2)
      para_printf("Found version %i.%i.\n", info->major, info->minor);
  }
}
