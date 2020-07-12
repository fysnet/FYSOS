//////////////////////////////////////////////////////////////////////////
//  pci.h  v1.00
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS__PCI
#define FYSOS__PCI

#pragma pack(1)

#define  PCI_ADDR     0x0CF8
#define  PCI_DATA     0x0CFC

#define  PCI_MAX_BUS      64  // PCI specs say 256 (we go to 64)
#define  PCI_MAX_DEV      32  // limit
#define  PCI_MAX_FUNC      8  // limit

#define  PCI_CLASS_ATA     0x01
#define    PCI_ATA_SUB_SCSI    0x00  //   SCSI
#define    PCI_ATA_SUB_IDE     0x01  //   IDE
#define    PCI_ATA_SUB_FDC     0x02  //   FDC
#define    PCI_ATA_SUB_IPI     0x03  //   IPI
#define    PCI_ATA_SUB_RAID    0x04  //   RAID
#define    PCI_ATA_SUB_ATA     0x05  //   ATA controller w/single DMA (20h) w/chained DMA (30h)
#define    PCI_ATA_SUB_SATA    0x06  //   Serial ATA
#define    PCI_ATA_SUB_SAS     0x07  //   Serial Attached SCSI
#define    PCI_ATA_SUB_SSS     0x08  //   Solid State Storage
#define    PCI_ATA_SUB_OTHER   0x80  //   Other


struct PCI_POS {
  bit8u  bus;
  bit8u  dev;
  bit8u  func;
};

struct PCI_DEV {
  bit16u vendorid;         // Vendor ID = FFFFh if not 'a valid entry'
  bit16u deviceid;         // Device ID
  bit16u command;          // Command
  bit16u status;           // Status
  bit8u  revid;            // Revision ID
  bit8u  p_interface;      //
  bit8u  sub_class;        // 
  bit8u  _class;           //
  bit8u  cachelinesz;      // Cache Line Size
  bit8u  latencytmr;       // Latency Timer
  bit8u  hdrtype;          // Header Type (0,1,2, if bit 7 set, multifunction device)
  bit8u  selftest;
  bit32u base0;            // 
  bit32u base1;            //
  bit32u base2;            //
  bit32u base3;            //
  bit32u base4;            // 
  bit32u base5;            //
  bit32u cardbus;          //
  bit16u subsysvid;        //
  bit16u subsysid;         //
  bit32u rombase;          //
  bit8u  capsoff;          //
  bit8u  resv0[3];         //
  bit32u resv1;            //
  bit8u  irq;              //
  bit8u  intpin;           //
  bit8u  min_time;         //
  bit8u  max_time;         //
  bit32u varies[48];       // varies by device.
};

// read from the pci config space
bit32u read_pci(const bit8u bus, const bit8u dev, const bit8u func, const bit8u port, const bit8u len) {
  
  bit32u ret;
  
  const bit32u val = 0x80000000 |
    (bus << 16) |
    (dev << 11) |
    (func << 8) |
    (port & 0xFC);
  outportl(PCI_ADDR, val);
  ret = (inportl(PCI_DATA) >> ((port & 3) * 8)) & (0xFFFFFFFF >> ((4-len) * 8));
  
  return ret;
}

// write to the pci config space
void write_pci(const bit8u bus, const bit8u dev, const bit8u func, const bit8u port, const bit8u len, bit32u value) {
  
  bit32u val = 0x80000000 |
    (bus << 16) |
    (dev << 11) |
    (func << 8) |
    (port & 0xFC);
  outportl(PCI_ADDR, val);

  // get current value
  val = inportl(PCI_DATA);
  
  // make sure value is of 'len' size
  value &= (0xFFFFFFFF >> ((4-len) * 8));
  
  // mask out new section
  if (len < 4) {
    val &= (0xFFFFFFFF << (len * 8));
    val |= value;
  } else
    val = value;
  
  outportl(PCI_DATA, val);
}

bool pci_get_base(bit8u *pci_bus, bit8u *pci_dev, bit8u *pci_func, bit32u *ret_type) {
  bit32u type, base;
  
  for (*pci_bus; *pci_bus < PCI_MAX_BUS; (*pci_bus)++) {
    for (*pci_dev; *pci_dev < PCI_MAX_DEV; (*pci_dev)++) {
      for (*pci_func; *pci_func < PCI_MAX_FUNC; (*pci_func)++) {
        if (read_pci(*pci_bus, *pci_dev, *pci_func, 0x00, sizeof(bit16u)) != 0xFFFF) {
          type = read_pci(*pci_bus, *pci_dev, *pci_func, (2<<2), sizeof(bit32u)) >> 8;
          if ((type & 0x00FF0000) == (*ret_type << 16)) {
            *ret_type = type;
            return TRUE;
          }
        } else {
          // if func0 == 0xFFFF, then no devices on this device, move to next
          if (*pci_func == 0)
            break;
        }
      }
      *pci_func = 0;
    }
    *pci_dev = 0;
  }
  
  return FALSE;
}

/* PCI specs v3.0, section 6.2.5.1 (page 227)
   Decode (I/O or memory) of a register is disabled via the command register before sizing a
   Base Address register. Software saves the original value of the Base Address register, writes
   0 FFFF FFFFh to the register, then reads it back. Size calculation can be done from the
   32-bit value read by first clearing encoding information bits (bit 0 for I/O, bits 0-3 for
   memory), inverting all 32 bits (logical NOT), then incrementing by 1. The resultant 32-bit
   value is the memory/I/O range size decoded by the register. Note that the upper 16 bits of
   the result is ignored if the Base Address register is for I/O and bits 16-31 returned zero
   upon read. The original value in the Base Address register is restored before re-enabling
   decode in the command register of the device.
   
   64-bit (memory) Base Address registers can be handled the same, except that the second
   32-bit register is considered an extension of the first; i.e., bits 32-63. Software writes
   0FFFFFFFFh to both registers, reads them back, and combines the result into a 64-bit value.
   Size calculation is done on the 64-bit value.
*/
bit32u pci_mem_range(const bit8u bus, const bit8u dev, const bit8u func, const bit8u port) {
  bit32u org0, org1, cmnd;
  bit64u range;
  
  // get the command register, save it, and disable I/O
  cmnd = read_pci(bus, dev, func, 0x04, 2);
  write_pci(bus, dev, func, 0x04, 2, cmnd & ~0x07);
  
  // read the original value(s)
  org0 = read_pci(bus, dev, func, port, 4);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    org1 = read_pci(bus, dev, func, port+4, 4);
  
  // write 0xFFFFFFFF
  write_pci(bus, dev, func, port, 4, 0xFFFFFFFF);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    write_pci(bus, dev, func, port+4, 4, 0xFFFFFFFF);
  
  // read it back.
  range = read_pci(bus, dev, func, port, 4);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    range |= ((bit64u) read_pci(bus, dev, func, port+4, 4) << 32);
  
  // write back the original value
  write_pci(bus, dev, func, port, 4, org0);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    write_pci(bus, dev, func, port+4, 4, org1);
  
  // restore the command register
  write_pci(bus, dev, func, 0x04, 2, cmnd);
  
  if (org0 & 1) { // port I/O
    org0 = (bit32u) range;
    if ((org0 & 0xFFFF0000) == 0)    // upper 16-bits are reserved if read as zero
      org0 |= 0xFFFF0000;
    return (bit32u) (~(org0 & ~0x1) + 1);
  } else
    return (bit32u) (~(range & (bit64u) ~0xF) + 1);
}

// Find the PCI Extended Capabilities List, see if there is a PM Entry, then
//  if so, make sure the device is in the D0 state, transitioning it if needed.
bool pci_set_D0_state(const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func) {
  
  bit8u  addr;
  bit32u dword;
  
  // get start of list
  addr = read_pci(pci_bus, pci_dev, pci_func, 0x34, sizeof(bit8u));
  
  // if addr = 0, no list
  while (addr) {
    dword = read_pci(pci_bus, pci_dev, pci_func, addr, sizeof(bit32u));
    
    // if the id = 1 (power management) we are here.
    if ((dword & 0xFF) == 1) {
      dword = read_pci(pci_bus, pci_dev, pci_func, addr + 4, sizeof(bit32u));
      if ((dword & 0x03) != 0x00) {
        printf("\n PCI PM state = D%i.  Transitioning to D0...", (dword & 0x03) + '0');
        write_pci(pci_bus, pci_dev, pci_func, addr + 4, sizeof(bit32u), (dword & ~0x03) | 0x8000);
        MDELAY(25); // PCI PM, v2.1, section 5.4 states a minimum of 10ms.  We wait 25ms for "faulty" devices.
        dword = read_pci(pci_bus, pci_dev, pci_func, addr + 4, sizeof(bit32u));
        if ((dword & 0x03) != 0x00) {
          printf("\n Could not transition to D0 state...");
          return FALSE;
        }
      }
      return TRUE;
    }
    
    if (dword & 0x0000FF00)
      addr = ((dword & 0x0000FF00) >> 8);
    else
      addr = 0;
  }
  
  return TRUE;
}

#endif // FYSOS__PCI
