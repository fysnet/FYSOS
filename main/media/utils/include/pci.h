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

/*
 *  Last updated: 15 July 2020
 */

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

// in port byte (8-bit)
__inline__ bit8u inpb(const bit16u port) {
  bit8u rv;
  __asm__ __volatile__ ("inb %1, %0"
    : "=a" (rv)
    : "dN" (port));
  return rv;
}

// out port byte (8-bit)
__inline__ void outpb(const bit16u port, const bit8u data) {
  __asm__ __volatile__ ("outb %1, %0"
    :
    : "dN" (port),
      "a" (data));
}

// in port word (16-bit)
__inline__ bit16u inpw(const bit16u port) {
  bit16u rv;
  __asm__ __volatile__ ("inw %1, %0"
    : "=a" (rv)
    : "dN" (port));
  return rv;
}

// out port word (16-bit)
__inline__ void outpw(const bit16u port, const bit16u data) {
  __asm__ __volatile__ ("outw %1, %0"
    :
    : "dN" (port),
      "a" (data));
}

// in port double (32-bit)
__inline__ bit32u inpd(const bit16u port) {
  bit32u rv;
  __asm__ __volatile__ ("inl %1, %0"
    : "=a" (rv)
    : "dN" (port));
  return rv;
}

// out port double (32-bit)
__inline__ void outpd(const bit16u port, const bit32u data) {
  __asm__ __volatile__ ("outl %1, %0"
    :
    : "dN" (port),
      "a" (data));
}

// port is the byte offset from 0x00
bit8u pci_read_byte(const int bus, const int dev, const int func, const int port) {
  const int shift = ((port & 3) * 8);
  const bit32u val = 0x80000000 |
    (bus << 16) |
    (dev << 11) |
    (func << 8) |
    (port & 0xFC);
  outpd(PCI_ADDR, val);
  return (inpd(PCI_DATA) >> shift) & 0xFF;
}

// port is the byte offset from 0x00
bit16u pci_read_word(const int bus, const int dev, const int func, const int port) {
  if ((port & 3) <= 2) {
    const int shift = ((port & 3) * 8);
    const bit32u val = 0x80000000 |
      (bus << 16) |
      (dev << 11) |
      (func << 8) |
      (port & 0xFC);
    outpd(PCI_ADDR, val);
    return (inpd(PCI_DATA) >> shift) & 0xFFFF;
  } else
    return (pci_read_byte(bus, dev, func, port + 1) << 8) | pci_read_byte(bus, dev, func, port);
}

// port is the byte offset from 0x00
bit32u pci_read_dword(const int bus, const int dev, const int func, const int port) {
  if ((port & 3) == 0) {
    const bit32u val = 0x80000000 |
      (bus << 16) |
      (dev << 11) |
      (func << 8) |
      (port & 0xFC);
    outpd(PCI_ADDR, val);
    return inpd(PCI_DATA);
  } else
    return (pci_read_word(bus, dev, func, port + 2) << 16) | pci_read_word(bus, dev, func, port);
}

// port is the byte offset from 0x00
void pci_write_byte(const int bus, const int dev, const int func, const int port, const bit8u value) {
  const int shift = ((port & 3) * 8);
  const bit32u val = 0x80000000 |
    (bus << 16) |
    (dev << 11) |
    (func << 8) |
    (port & 0xFC);
  outpd(PCI_ADDR, val);
  outpd(PCI_DATA, (inpd(PCI_DATA) & ~(0xFF << shift)) | (value << shift));
}

// port is the byte offset from 0x00
void pci_write_word(const int bus, const int dev, const int func, const int port, const bit16u value) {
  if ((port & 3) <= 2) {
    const int shift = ((port & 3) * 8);
    const bit32u val = 0x80000000 |
      (bus << 16) |
      (dev << 11) |
      (func << 8) |
      (port & 0xFC);
    outpd(PCI_ADDR, val);
    outpd(PCI_DATA, (inpd(PCI_DATA) & ~(0xFFFF << shift)) | (value << shift));
  } else {
    pci_write_byte(bus, dev, func, port + 0, (bit8u) (value & 0xFF));
    pci_write_byte(bus, dev, func, port + 1, (bit8u) (value >> 8));
  }
}

// port is the byte offset from 0x00
void pci_write_dword(const int bus, const int dev, const int func, const int port, const bit32u value) {
  if ((port & 3) == 0) {
    const bit32u val = 0x80000000 |
      (bus << 16) |
      (dev << 11) |
      (func << 8) |
      (port & 0xFC);
    outpd(PCI_ADDR, val);
    outpd(PCI_DATA, value);
  } else {
    pci_write_word(bus, dev, func, port + 0, (bit16u) (value & 0xFFFF));
    pci_write_word(bus, dev, func, port + 2, (bit16u) (value >> 16));
  }
}

bool pci_get_base(bit8u *pci_bus, bit8u *pci_dev, bit8u *pci_func, bit32u *ret_type) {
  bit32u type, base;
  
  for (*pci_bus; *pci_bus < PCI_MAX_BUS; (*pci_bus)++) {
    for (*pci_dev; *pci_dev < PCI_MAX_DEV; (*pci_dev)++) {
      for (*pci_func; *pci_func < PCI_MAX_FUNC; (*pci_func)++) {
        if (pci_read_word(*pci_bus, *pci_dev, *pci_func, 0x00) != 0xFFFF) {
          type = pci_read_dword(*pci_bus, *pci_dev, *pci_func, (2<<2)) >> 8;
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
  cmnd = pci_read_word(bus, dev, func, 0x04);
  pci_write_word(bus, dev, func, 0x04, cmnd & ~0x07);
  
  // read the original value(s)
  org0 = pci_read_dword(bus, dev, func, port);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    org1 = pci_read_dword(bus, dev, func, port+4);
  
  // write 0xFFFFFFFF
  pci_write_dword(bus, dev, func, port, 0xFFFFFFFF);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    pci_write_dword(bus, dev, func, port+4, 0xFFFFFFFF);
  
  // read it back.
  range = pci_read_dword(bus, dev, func, port);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    range |= ((bit64u) pci_read_dword(bus, dev, func, port+4) << 32);
  
  // write back the original value
  pci_write_dword(bus, dev, func, port, org0);
  if ((org0 & 0x07) == 0x04)  // 64 bit and mem I/O
    pci_write_dword(bus, dev, func, port+4, org1);
  
  // restore the command register
  pci_write_word(bus, dev, func, 0x04, cmnd);
  
  if (org0 & 1) { // port I/O
    org0 = (bit32u) range;
    if ((org0 & 0xFFFF0000) == 0)    // upper 16-bits are reserved if read as zero
      org0 |= 0xFFFF0000;
    return (bit32u) (~(org0 & ~0x1) + 1);
  } else
    return (bit32u) (~(range & (bit64u) ~0xF) + 1);
}


//#define MDELAY(x) mdelay(x)  // use our mS delay

#include <dos.h>
#define MDELAY(x) delay(x)  // use DJGPP's mS delay

// Find the PCI Extended Capabilities List, see if there is a PM Entry, then
//  if so, make sure the device is in the D0 state, transitioning it if needed.
bool pci_set_D0_state(const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func) {
  
  bit8u  addr;
  bit32u dword;
  
  // get start of list
  addr = pci_read_byte(pci_bus, pci_dev, pci_func, 0x34);
  
  // if addr = 0, no list
  while (addr) {
    dword = pci_read_dword(pci_bus, pci_dev, pci_func, addr);
    
    // if the id = 1 (power management) we are here.
    if ((dword & 0xFF) == 1) {
      dword = pci_read_dword(pci_bus, pci_dev, pci_func, addr + 4);
      if ((dword & 0x03) != 0x00) {
        printf("\n PCI PM state = D%i.  Transitioning to D0...", (dword & 0x03) + '0');
        pci_write_dword(pci_bus, pci_dev, pci_func, addr + 4, (dword & ~0x03) | 0x8000);
        MDELAY(25); // PCI PM, v2.1, section 5.4 states a minimum of 10ms.  We wait 25ms for "faulty" devices.
        dword = pci_read_dword(pci_bus, pci_dev, pci_func, addr + 4);
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
