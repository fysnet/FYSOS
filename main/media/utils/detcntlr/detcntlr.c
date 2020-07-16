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
 *  DETCNTLR.EXE
 *   Will enumerate through the PCI, finding all (S)ATA controllers.
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS envirnment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *   - This code assumes the attached device is a high-speed device.  If a full-
 *     or low-speed device is attached, the device will not be found by this code.
 *     Use GD_UHCI or GD_OHCI for full- and low-speed devices.
 *
 *  Last updated: 15 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os detcntlr.c -o detcntlr.exe -s
 *
 *  Usage:
 *    detcntlr
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

#include "..\include\ctype.h"
#include "..\include\pci.h"
#include "..\include\misc.h"

#include "detcntlr.h"

bool verbose = FALSE;
bool verbose_dump = FALSE;

int main(int argc, char *argv[]) {
  
  int i, cnt = 0;
  bit8u pci_bus, pci_dev, pci_func;
  struct PCI_DEV data;
  bit32u *pcidata = (bit32u *) &data;
  bool is_multi;
  
  printf("Detect ATA Controllers on a PCI Bus. v1.00.00\n"
         "Forever Young Software -- Copyright 1984-2020\n");
  
  // parse the command line parameters
  if (!get_parameters(argc, argv))
    return -1;
  
  // scroll through PCI_MAX_BUS buses, PCI_MAX_DEV devices per bus, and PCI_MAX_FUNC functions per device
  for (pci_bus = 0; pci_bus < PCI_MAX_BUS; pci_bus++) {
    for (pci_dev=0; pci_dev < PCI_MAX_DEV; pci_dev++) {
      is_multi = TRUE;
      for (pci_func=0; is_multi && (pci_func < PCI_MAX_FUNC); pci_func++) {
        // read the first 16-bit word of the function
        // if it is not 0xFFFF, then we have a valid function at this 'address'
        if (pci_read_word(pci_bus, pci_dev, pci_func, 0x00) != 0xFFFF) {
          
          // read in the 256 bytes (64 dwords)
          for (i=0; i<64; i++)
            pcidata[i] = pci_read_dword(pci_bus, pci_dev, pci_func, (i<<2));
          
          if (verbose) {
            printf("\nFound Device:\n");
            printf("  Vendor 0x%04X, Device 0x%04X, rev = 0x%02X\n"
                   "  Class = %02X, Subclass = %02X, Protocol = %02X\n"
                   "  Bus = %i, Dev = %i, Func = %i\n",
                   data.vendorid, data.deviceid, data.revid,
                   data._class, data.sub_class, data.p_interface,
                   pci_bus, pci_dev, pci_func);
          }
          
          // if the class == PCI_CLASS_ATA, we have an ATA controller of some type.
          if (data._class == PCI_CLASS_ATA) {
            cnt++;
            
            // print the info
            printf("\nFound an ATA compatible device entry.\n");
            printf("  BAR0: 0x%08X, BAR1: 0x%08X, BAR2: 0x%08X\n"
                   "  BAR3: 0x%08X, BAR4: 0x%08X, BAR5: 0x%08X\n",
                   data.base0, data.base1, data.base2, 
                   data.base3, data.base4, data.base5);
            
            if (verbose_dump)
              dump(&data, 256);
            
            switch (data.sub_class) {
              case PCI_ATA_SUB_SCSI:  //   SCSI
                printf(" Found SCSI ATA Device\n");
                break;
              case PCI_ATA_SUB_IDE:   //   IDE
                printf(" Found IDE ATA Device with:\n  Primary in %s mode\n  Secondary in %s mode.\n", 
                  (data.p_interface & (1 << 0)) ? "native" : "compatibility (0x01F0)",
                  (data.p_interface & (1 << 2)) ? "native" : "compatibility (0x0170)"
                  );
                break;
              case PCI_ATA_SUB_FDC:   //   FDC
                printf(" Found FDC ATA Device\n");
                break;
              case PCI_ATA_SUB_IPI:   //   IPI
                printf(" Found IPI ATA Device\n");
                break;
              case PCI_ATA_SUB_RAID:  //   RAID
                printf(" Found RAID ATA Device\n");
                break;
              case PCI_ATA_SUB_ATA:   //   ATA controller w/single DMA (20h) w/chained DMA (30h)
                printf(" Found SUB ATA Device\n");
                break;
              case PCI_ATA_SUB_SATA:   //   Serial ATA
                switch (data.p_interface) {
                  case 1:  // AHCI version 1.xx.xx
                    printf(" Found SATA Device with AHCI interface.\n");
                    break;
                  default:
                    printf(" Found SATA Device\n");
                }
                break;
              case PCI_ATA_SUB_SAS:    //   Serial Attached SCSI
                printf(" Found Serial Attached SCSI Device\n");
                break;
              case PCI_ATA_SUB_SSS:    //   Solid State Storage
                printf(" Found Solid State Storage Device\n");
                break;
              case PCI_ATA_SUB_OTHER:  //   Other
                printf(" Found Other ATA Device\n");
                break;
              default:
                printf(" Found Unknown ATA Device\n");
            }
          }
          // if bit 7 of the header type (of func = 0) is set, then this is a multi-function device
          if (pci_func == 0)
            is_multi = ((data.hdrtype & 0x80) > 0);
        }
      }
    }
  }
  
  printf("\nFound %i PCI ATA Controllers\n", cnt);
  
  // return to host OS
  return 0;
}

// simply parses the command line parameters for specific values
bool get_parameters(int argc, char *argv[]) {
  int i;
  
  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "v") == 0)
        verbose = TRUE;
      else if (strcmp(&argv[i][1], "d") == 0)
        verbose_dump = TRUE;
      else {
        printf("Unknown parameter: %s\n", argv[i]);
        return FALSE;
      }
    } else {
      printf("Unknown parameter: %s\n", argv[i]);
      return FALSE;
    }
  }
  
  return TRUE;
}
