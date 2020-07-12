/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Media Storage Devices, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  compile using gcc (DJGPP)
 *   gcc -Os detcntlr.c -o detcntlr.exe -s
 *
 *  usage:
 *    detcntlr [-v -d]
 *
 *    -v indicates verbose output
 *    -d indicates to dump the config space for each found (S)ATA controller
 *
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

//#define MDELAY(x) mdelay(x)  // use our mS delay
#define MDELAY(x) delay(x)  // use DJGPP's mS delay

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
         "Forever Young Software -- Copyright 1984-2015\n");
  
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
        if (read_pci(pci_bus, pci_dev, pci_func, 0x00, sizeof(bit16u)) != 0xFFFF) {
          
          // read in the 256 bytes (64 dwords)
          for (i=0; i<64; i++)
            pcidata[i] = read_pci(pci_bus, pci_dev, pci_func, (i<<2), sizeof(bit32u));
          
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
