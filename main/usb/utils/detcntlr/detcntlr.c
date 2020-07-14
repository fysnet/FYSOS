/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or binary forms with  or without modification,
 * are  permitted provided that the  following conditions are met.  Redistribution
 * in printed form must first acquire permission from copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 3. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * THIS SOFTWARE IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY   THEORY OF  LIABILITY, WHETHER  IN  CONTRACT, STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Let it be known that the purpose of this product, (hereby known as source code,
 * documentation, binary files, or other forms within this distribution), is to be
 * used as supplemental product for one or more of  the following mentioned books.
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
 * This product here is  included as a companion to one or more of these books and 
 * is not intended to be  self sufficient.  Each item within  this distribution is
 * part of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  compile using gcc (DJGPP)
 *   gcc -Os detcntlr.c -o detcntlr.exe -s
 *
 *  using:
 *    detcntlr /type
 *   displays the type of controller.
 *
 */

#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

#include "../include/ctype.h"
#include "../include/pci.h"

#include "detcntlr.h"

bit32u pci_mem_range(bit8u bus, bit8u dev, bit8u func, bit8u port);

int main(int argc, char *argv[]) {
  int i, index;
  bit8u pci_bus, pci_dev, pci_func;
  struct PCI_DEV data;
  bit32u *pcidata = (bit32u *) &data;
  char  type[6][10] = { "", "(UHCI)", "(OHCI)", "(EHCI)", "(xHCI)", "(Unknown)" };
  bit32u base = 0, size = 0;
  bool prt_type = FALSE;
  
  printf("\nDetect USB Controllers on a PCI Bus. v1.00.00"
         "\nForever Young Software -- Copyright 1984-2020\n");
  
  // check to see if the user added the "/type" parameter.  If so, set the flag
  if ((argc > 1) && (strcmp(argv[1], "/type") == 0))
    prt_type = TRUE;
  
  // scroll through PCI_MAX_BUS buses, PCI_MAX_DEV devices per bus, and PCI_MAX_FUNC functions per device
  for (pci_bus = 0; pci_bus < PCI_MAX_BUS; pci_bus++) {
    for (pci_dev=0; pci_dev < PCI_MAX_DEV; pci_dev++) {
      for (pci_func=0; pci_func < PCI_MAX_FUNC; pci_func++) {
        // read the first 16-bit word of the function
        // if it is not 0xFFFF, then we have a valid function at this 'address'
        if (pci_read_word(pci_bus, pci_dev, pci_func, 0x00) != 0xFFFF) {
          
          // read in the 256 bytes (64 dwords)
          for (i=0; i<64; i++)
            pcidata[i] = pci_read_dword(pci_bus, pci_dev, pci_func, (i<<2));
          
          // if the class == 0x0C and the subclass == 0x03, we have a USB controller.
          if ((data._class == 0x0C) && (data.sub_class == 0x03)) {
            // we use the upper nibble of the protocol interface field
            // it should be 0, 1, 2, or 3
            index = (data.p_interface >> 4);
            // check to be sure not more than 3.  If so, give '4' for (unknown) string.
            if (index > 3)
              index = 4;
            // Only the UHCI controller uses BAR 4.  The rest use BAR 0
            if (data.p_interface == 0x00) {
              base = data.base4;
              size = pci_mem_range(pci_bus, pci_dev, pci_func, 0x20);
            } else {
              base = data.base0;
              size = pci_mem_range(pci_bus, pci_dev, pci_func, 0x10);
            }
            
            // print the type of controller?
            if (prt_type == FALSE)
              // if not, simply print an empty string.
              index = 0;
            else
              // if so, skip to 2nd index of strings
              index++;
            
            // print the info
            printf("\n Found a USB compatible device entry. %s", type[index]);
            printf("\n Bus = %2i, device = %2i, function = %i, IO Base: 0x%08X, IRQ: %i, size = %i\n",
                   pci_bus, pci_dev, pci_func, base, data.irq, size);
          }
          
          // if bit 7 of the header type (of the first function of the device) is set,
          //  then this is a multi-function device.
          //  else, skip checking the rest of the functions.
          if (pci_func == 0)
            if ((pci_read_byte(pci_bus, pci_dev, pci_func, 0x0E) & 0x80) == 0)
              pci_func = PCI_MAX_FUNC;
          
        } else {
          // if first func of dev and is 0xFFFF, no more func's on this dev
          if (pci_func == 0)
            break;
        }
      }
    }
  }
  
  // return to host OS
  return 0;
}
