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
 *  ACPI.EXE
 *   dumps the acpi tables
 *
 *  Assumptions/prerequisites:
 *   This utility is compiled as a 32-bit DOS .EXE using a DPMI.  It
 *    "allocates" a selector block to access memory above 1Meg.  Therefore,
 *    it does not work in anything but TRUE DOS.  i.e.: Does not work in
 *    a Windows DOS session.
 *   This utility assumes the Legacy BIOS is used, and we search for the
 *    'RSD PTR ' signature ourselves, rather than calling the BIOS to get
 *    the memory map.
 *
 *  Last updated: 20 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os acpi.c -o acpi.exe -s
 *
 *  Usage:
 *   ACPI > results.txt
 *
 * You can also give a command line of -o filename.bin to store the 
 *  AML code to disk.
 *
 *   ACPI -o filename.bin > results.txt
 */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/farptr.h>
#include <dpmi.h>
#include <go32.h>

#include "../include/ctype.h"

#include "acpi.h"

char out_file[128] = "\0";

// define this to zero if you do not want to prepend the DSDT Header
//  to the outfile before writing the AML code to the file
#define INCLUDE_DSDT_HDR  1  // set to 1 by default

int main(int argc, char *argv[]) {
  __dpmi_meminfo base_mi;
  int selector;
  bit32u dword, base;
  bit32u addr = 0;
  int i;
  
  // print start string
  printf("\nDump ACPI  v00.10.05 (C)opyright   Forever Young Software 1984-2020\n\n");
  
  // if -o outfile.bin is on the command line, save the filename
  if ((argc == 3) && (!strcmp(argv[1], "-o")))
    strcpy(out_file, argv[2]);
  
  // first get the EBDA address from the BIOS
  // the BIOS should give us a 16-bit segment at 0x0040E
  // should be something like 0x9FC0
  dword = _farpeekw(_dos_ds, 0x0000040E) << 4;
  if (dword)
    addr = acpi_find_sig(dword, (dword + 0x1000 - 1), RSD_SIG0, RSD_SIG1);
  
  // else search for the "RSD PTR " signature from 0xE0000 to 0xFFFFF
  if (addr == 0)
    addr = acpi_find_sig(0xE0000, 0xFFFFF, RSD_SIG0, RSD_SIG1);
  
  // did we find it?
  if (addr != 0) {
    printf(" Root System Descriptor Pointer Structure: 0x%08X\n", addr);
    printf("             Signature: '%c%c%c%c%c%c%c%c' (0x%08X 0x%08X)\n", 
      _farpeekb(_dos_ds, addr + 0), _farpeekb(_dos_ds, addr + 1), _farpeekb(_dos_ds, addr + 2), _farpeekb(_dos_ds, addr + 3),
      _farpeekb(_dos_ds, addr + 4), _farpeekb(_dos_ds, addr + 5), _farpeekb(_dos_ds, addr + 6), _farpeekb(_dos_ds, addr + 7),
      _farpeekl(_dos_ds, addr + 0), _farpeekl(_dos_ds, addr + 4));
    printf("               OEM sig: '%c%c%c%c%c%c'\n",
      _farpeekb(_dos_ds, addr + 9 + 0), _farpeekb(_dos_ds, addr + 9 + 1), _farpeekb(_dos_ds, addr + 9 + 2),
      _farpeekb(_dos_ds, addr + 9 + 3), _farpeekb(_dos_ds, addr + 9 + 4), _farpeekb(_dos_ds, addr + 9 + 5));
    printf("               Version: %i (%i.0)\n", _farpeekb(_dos_ds, addr + 15), 
      (_farpeekb(_dos_ds, addr + 15) == 0) ? 1 : _farpeekb(_dos_ds, addr + 15));
    printf("          RSDT Address: 0x%08X\n", _farpeekl(_dos_ds, addr + 16));
    puts("");
    
    // if version > 0, the rest of the table may be filled
    if (_farpeekb(_dos_ds, addr + 15) > 0) {
      if (acpi_crc_check(_dos_ds, addr + 0, _farpeekl(_dos_ds, addr + 0x14)) != 0) {
        printf("Rest of table does not check...\n");
        return 0;
      }
      printf("                Length: %i\n", _farpeekl(_dos_ds, addr + 20));
      printf("          xSDT Address: 0x%08X%08X\n", _farpeekl(_dos_ds, addr + 24 + 4), _farpeekl(_dos_ds, addr + 24 + 0));
      puts("");
    }
    
    // if rsdt < 0x100000, we can use the _dos_ds selector
    addr = _farpeekl(_dos_ds, addr + 16);
    if (addr < 0x100000) {
      selector = _dos_ds;
      base = addr;
    } else {
      base_mi.address = addr;
      base_mi.size = MAX_TABLE_SIZE;
      if (!get_physical_mapping(&base_mi, &selector)) {
        printf("Error 'allocating' physical memory.\n");
        return 0;
      }
      base = 0;
    }
    
    // check crc on rsdt
    if (acpi_crc_check(selector, base + 0, _farpeekl(selector, base + 4)) != 0) {
      printf("RSDT Checksum does not check...\n");
      return 0;
    }
    
    // print rsdt
    puts(" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf(" Root System Descriptor Table: 0x%08X\n", addr);
    printf("             Signature: '%c%c%c%c' (0x%08X)\n", 
      _farpeekb(selector, base + 0), _farpeekb(selector, base + 1), _farpeekb(selector, base + 2), _farpeekb(selector, base + 3),
      _farpeekl(selector, base + 0));
    printf("          Table Length: %i  (%i entries)\n", _farpeekl(selector, base + 4),
      (_farpeekl(selector, base + 4) - sizeof(struct S_ACPI_TBLE_HDR)) / sizeof(struct S_ACPI_TBLE_HDR *));
    printf("               Version: %i (%i.0)\n", _farpeekb(selector, base + 8), 
      (_farpeekb(selector, base + 8) == 0) ? 1 : _farpeekb(selector, base + 8));
    printf("               OEM sig: '%c%c%c%c%c%c'\n",
      _farpeekb(selector, base + 10 + 0), _farpeekb(selector, base + 10 + 1), _farpeekb(selector, base + 10 + 2),
      _farpeekb(selector, base + 10 + 3), _farpeekb(selector, base + 10 + 4), _farpeekb(selector, base + 10 + 5));
    printf("                OEM ID: '%c%c%c%c%c%c%c%c'\n",
      _farpeekb(selector, base + 16 + 0), _farpeekb(selector, base + 16 + 1), _farpeekb(selector, base + 16 + 2),
      _farpeekb(selector, base + 16 + 3), _farpeekb(selector, base + 16 + 4), _farpeekb(selector, base + 16 + 5),
      _farpeekb(selector, base + 16 + 6), _farpeekb(selector, base + 16 + 7));
    // if the version is > 256, it probably is a 4-digit string
    if (_farpeekl(selector, base + 24) > 0x100)
      printf("           OEM Version: '%c%c%c%c'\n",
        _farpeekb(selector, base + 24), _farpeekb(selector, base + 25), _farpeekb(selector, base + 26), _farpeekb(selector, base + 27));
    else
      printf("           OEM Version: %i\n", _farpeekl(selector, base + 24));
    printf("   Creator/ASL Compiler ID: '%c%c%c%c'\n",
      _farpeekb(selector, base + 28), _farpeekb(selector, base + 29), _farpeekb(selector, base + 30), _farpeekb(selector, base + 31));
    printf("  Creator/ASL Compiler Ver: 0x%08X\n", _farpeekl(selector, base + 32));
    puts("");
    
    // print table pointers
    dword = (_farpeekl(selector, base + 4) - sizeof(struct S_ACPI_TBLE_HDR)) / sizeof(struct S_ACPI_TBLE_HDR *);
    for (i=0; i<dword; i++) {
      puts(" -=-= Table Header Pointer =-=-=-=-=-=-=-=-=");
      printf("              Entry #%i: 0x%08X\n\n", i, _farpeekl(selector, base + sizeof(struct S_ACPI_TBLE_HDR) + (i * 4)));
      acpi_enum_tble(_farpeekl(selector, base + sizeof(struct S_ACPI_TBLE_HDR) + (i * 4)));
    }
    
    if (addr >= 0x100000)
      __dpmi_free_physical_address_mapping(&base_mi);
  }
  
  return 0;
}

// start is the physical address to start
// end is the physical address to stop
// sig is the 32-bit signature to look for
bit32u acpi_find_sig(bit32u start, const  bit32u end, const bit32u sig0, const bit32u sig1) {
  while (start < end) {
    if ((_farpeekl(_dos_ds, start + 0) == sig0) && 
        (_farpeekl(_dos_ds, start + 4) == sig1) && 
        (acpi_crc_check(_dos_ds, start, 20) == 0))
      return start;
    start += 16;
  }
  
  return 0;
}

// crc check
// start is the physical address to start
bit8u acpi_crc_check(const int sel, const bit32u start, const int len) {
  bit8u crc = 0;
  for (int i=0; i<len; i++)
    crc += _farpeekb(sel, start + i);
  return crc;
}

// This is the code the gathers data from each table
// For this example, we only print the info for the APIC
//  and the AML code (DSDT).  All other tables are simply
//  printed as found, and skipped.
// addr is a pointer to a table header
void acpi_enum_tble(const bit32u addr) {
  __dpmi_meminfo base_mi;
  int selector, i, len;
  char ch;
  bit8u *aml_code = NULL;
  bit32u base;
  
  if (addr < 0x100000) {
    selector = _dos_ds;
    base = addr;
  } else {
    base_mi.address = addr;
    base_mi.size = MAX_TABLE_SIZE;
    if (!get_physical_mapping(&base_mi, &selector)) {
      printf("Error 'allocating' physical memory.\n");
      return;
    }
    base = 0;
  }
  
  switch (_farpeekl(selector, base + 0)) {
    // must watch for FACP so we can recurse and get DSDT
    case 'PCAF':
      acpi_enum_tble(_farpeekl(selector, base + 40));
      break;
    
    case 'CIPA': {  // APIC
      puts(" -=-= Header -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
      printf("             Signature: '%c%c%c%c' (0x%08X)\n", 
        _farpeekb(selector, base + 0), _farpeekb(selector, base + 1), _farpeekb(selector, base + 2), _farpeekb(selector, base + 3),
        _farpeekl(selector, base + 0));
      printf("          Table Length: %i\n", _farpeekl(selector, base + 4));
      printf("               Version: %i (%i.0)\n", _farpeekb(selector, base + 8), 
        (_farpeekb(selector, base + 8) == 0) ? 1 : _farpeekb(selector, base + 8));
      printf("               OEM sig: '%c%c%c%c%c%c'\n",
        _farpeekb(selector, base + 10 + 0), _farpeekb(selector, base + 10 + 1), _farpeekb(selector, base + 10 + 2),
        _farpeekb(selector, base + 10 + 3), _farpeekb(selector, base + 10 + 4), _farpeekb(selector, base + 10 + 5));
      printf("                OEM ID: '%c%c%c%c%c%c%c%c'\n",
        _farpeekb(selector, base + 16 + 0), _farpeekb(selector, base + 16 + 1), _farpeekb(selector, base + 16 + 2),
        _farpeekb(selector, base + 16 + 3), _farpeekb(selector, base + 16 + 4), _farpeekb(selector, base + 16 + 5),
        _farpeekb(selector, base + 16 + 6), _farpeekb(selector, base + 16 + 7));
      // if the version is > 256, it probably is a 4-digit string
      if (_farpeekl(selector, base + 24) > 0x100)
        printf("           OEM Version: '%c%c%c%c'\n",
          _farpeekb(selector, base + 24), _farpeekb(selector, base + 25), _farpeekb(selector, base + 26), _farpeekb(selector, base + 27));
      else
        printf("           OEM Version: %i\n", _farpeekl(selector, base + 24));
      printf("   Creator/ASL Compiler ID: '%c%c%c%c'\n",
        _farpeekb(selector, base + 28), _farpeekb(selector, base + 29), _farpeekb(selector, base + 30), _farpeekb(selector, base + 31));
      printf("  Creator/ASL Compiler Ver: 0x%08X\n", _farpeekl(selector, base + 32));
      puts("");
      puts(" -=-= Body -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
      printf(" APIC: LAPIC Address: 0x%08X\n", _farpeekl(selector, base + ACPI_TBLE_HDR_SIZE + 0));
      printf(" APIC:     Dual-8259: %s\n", (_farpeekl(selector, base + ACPI_TBLE_HDR_SIZE + 4) & 1) ? "yes" : "no");
      puts("");
      puts(" -=-= Items -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
      bit32u current = ACPI_TBLE_HDR_SIZE + sizeof(bit32u) + sizeof(bit32u);
      while (current < _farpeekl(selector, base + 4)) {
        printf(" APIC sub-type %i found...\n", _farpeekb(selector, base + current + 0));
        switch (_farpeekb(selector, base + current + 0)) {
          // type = 0, len = 8 (table 5-46, p137)
          case 0:  // Processor Local APIC
            // each processor in this machine will have one of these entries.
            // each entry has an APIC ID.  We need to store this in the APIC
            //  driver (?) so that we can keep track of which is which.
            puts("  Processor Local APIC");
            printf(" APIC:  ACPI Processor ID: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:            APIC ID: %i\n", _farpeekb(selector, base + current + 3));
            printf(" APIC:             Usable: %s\n", (_farpeekl(selector, base + current + 4) & 1) ? "yes" : "no");
            puts("");
            break;
          // type = 1, len = 12 (table 5-48, p138)
          case 1: // I/O APIC entry
            puts("  I/O APIC");
            printf(" APIC:        I/O APIC ID: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:   I/O APIC Address: 0x%08X\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:     Interrupt Base: %i\n", _farpeekl(selector, base + current + 8));
            puts("");
            break;
          // type = 2, len = 10 (table 5-49, p139)
          case 2: // interrupt source overrides
            puts("  Interrupt Source Overrides");
            printf(" APIC:                Bus: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:             Source: %i\n", _farpeekb(selector, base + current + 3));
            printf(" APIC:   System Interrupt: %i\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:              Flags: 0x%04X\n", _farpeekw(selector, base + current + 8));
            switch ((_farpeekw(selector, base + current + 8) & (0x3 << 0)) >> 0) {
              case 0:
                printf("                 Polarity: Comforms to Specifications of the bus\n");
                break;
              case 1:
                printf("                 Polarity: Active High\n");
                break;
              case 2:
                printf("                 Polarity: Reserved.  Ooopps..\n");
                break;
              case 3:
                printf("                 Polarity: Active Low\n");
                break;
            }
            switch ((_farpeekw(selector, base + current + 8) & (0x3 << 2)) >> 2) {
              case 0:
                printf("             Trigger mode: Comforms to Specifications of the bus\n");
                break;
              case 1:
                printf("             Trigger mode: Edge Triggered\n");
                break;
              case 2:
                printf("             Trigger mode: Reserved.  Ooopps..\n");
                break;
              case 3:
                printf("             Trigger mode: Level Triggered\n");
                break;
            }
            puts("");
            break;
          // type = 3, len = 8 (table 5-51, p140)
          case 3: // NMI Source
            puts("  NMI Source");
            printf(" APIC:              Flags: 0x%04X\n", _farpeekw(selector, base + current + 2));
            printf(" APIC:   System Interrupt: %i\n", _farpeekl(selector, base + current + 4));
            puts("");
            break;
          // type = 4, len = 6 (table 5-52, p140)
          case 4: // local APIC NMI
            puts("  Local APIC NMI");
            printf(" APIC:  ACPI Processor ID: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:              Flags: 0x%04X\n", _farpeekw(selector, base + current + 3));
            printf(" APIC:        LAPIC LINT#: %i\n", _farpeekb(selector, base + current + 5));
            puts("");
            break;
          // type = 5, len = 12 (table 5-53, p141)
          case 5:  // local APIC Address Override
            puts("  Local APIC Address Override");
            printf(" APIC:     64-bit Address: 0x%08X%08X\n", _farpeekl(selector, base + current + 8), _farpeekl(selector, base + current + 4));
            puts("");
            break;
          // type = 6, len = 16 (table 5-54, p141)
          case 6:  // I/O SAPIC
            puts("  I/O SAPIC");
            printf(" APIC:       I/O SAPIC ID: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:     Interrupt Base: %i\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:     64-bit Address: 0x%08X%08X\n", _farpeekl(selector, base + current + 12), _farpeekl(selector, base + current + 8));
            puts("");
            break;
          // type = 7, len = ?  (table 5-55, p142)
          case 7:  // Local SAPIC
            puts("  Local SAPIC");
            printf(" APIC:  ACPI Processor ID: %i\n", _farpeekb(selector, base + current + 2));
            printf(" APIC:     Local SAPIC ID: %i\n", _farpeekb(selector, base + current + 3));
            printf(" APIC:    Local SAPIC EID: %i\n", _farpeekb(selector, base + current + 4));
            printf(" APIC:              Flags: 0x%08X\n", _farpeekl(selector, base + current + 8));
            printf(" APIC:                UID: 0x%08X\n", _farpeekl(selector, base + current + 12));
            printf(" APIC:         UID String: '\n");
            i = 0;
            while (ch = _farpeekb(selector, base + current + 16 + i)) {
              putch(ch);
              i++;
            }
            putch(39);
            puts("");
            break;
          // type = 8, len = 16  (table 5-56, p143)
          case 8: // Platform Interrupt Sources
            puts("  Platform Interrupt Sources");
            printf(" APIC:              Flags: 0x%04X\n", _farpeekw(selector, base + current + 2));
            printf(" APIC:     Interrupt Type: %i ", _farpeekb(selector, base + current + 4));
            switch (_farpeekb(selector, base + current + 4)) {
              case 1: puts("(PMI)"); break;
              case 2: puts("(INIT)"); break;
              case 3: puts("(Corrected Platform Error)"); break;
              default: puts("(unknown)");
            }
            printf(" APIC:       Processor ID: %i\n", _farpeekb(selector, base + current + 5));
            printf(" APIC:      Processor EID: %i\n", _farpeekb(selector, base + current + 6));
            printf(" APIC:   I/O SAPIC Vector: %i\n", _farpeekb(selector, base + current + 7));
            printf(" APIC:   System Interrupt: %i\n", _farpeekl(selector, base + current + 8));
            printf(" APIC:       Source Flags: 0x%08X\n", _farpeekl(selector, base + current + 12));
            puts("");
            break;
          // type = 9, len = 16  (table 5-58, p144)
          case 9: // Processor Local x2APIC
            puts("  Processor Local x2APIC");
            printf(" APIC:          x2APIC ID: %i\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:              Flags: %i\n", _farpeekl(selector, base + current + 8));
            printf(" APIC:                UID: 0x%08X\n", _farpeekl(selector, base + current + 12));
            puts("");
            break;
          // type = 10, len = 12  (table 5-59, p145)
          case 10:  // x2APIC NMI structure
            puts("  x2APIC NMI");
            printf(" APIC:              Flags: 0x%04X\n", _farpeekw(selector, base + current + 2));
            printf(" APIC:                UID: 0x%08X\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:       x2APIC LINT#: %i\n", _farpeekb(selector, base + current + 8));
            puts("");
            break;
          // type = 11, len = 40  (table 5-60, p146)
          case 11:  // GIC structure
            puts("  GIC");
            printf(" APIC:             GIC ID: 0x%04X\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:                UID: 0x%08X\n", _farpeekl(selector, base + current + 8));
            printf(" APIC:              Flags: 0x%08X\n", _farpeekl(selector, base + current + 12));
            printf(" APIC: P Protocol Version: 0x%08X\n", _farpeekl(selector, base + current + 16));
            printf(" APIC:     Pref. INT GSIV: 0x%08X\n", _farpeekl(selector, base + current + 20));
            printf(" APIC:  Parked: 64-bit Address: 0x%08X%08X\n", _farpeekl(selector, base + current + 28), _farpeekl(selector, base + current + 24));
            printf(" APIC:    Base: 64-bit Address: 0x%08X%08X\n", _farpeekl(selector, base + current + 36), _farpeekl(selector, base + current + 32));
            puts("");
            break;
          // type = 12, len = 24  (table 5-62, p147)
          case 12: // GICD structure
            puts("  GICD");
            printf(" APIC:            GICD ID: 0x%04X\n", _farpeekl(selector, base + current + 4));
            printf(" APIC:    Base: 64-bit Address: 0x%08X%08X\n", _farpeekl(selector, base + current + 12), _farpeekl(selector, base + current + 8));
            printf(" APIC: System Vector base: %i\n", _farpeekl(selector, base + current + 16));
            puts("");
            break;
          default:
            printf("ACPI unknown APIC type %i found...\n", _farpeekb(selector, base + current + 0));
            puts("");
        }
        
        // advanced to next item
        current += _farpeekb(selector, base + current + 1);
      }
    } break;
      
    // this is the AML Code stuff
    case 'TDSD':
#if INCLUDE_DSDT_HDR
      len = _farpeekl(selector, base + 4);
      printf(" ACPI: DSDT:  AML code size %i\n", len - ACPI_TBLE_HDR_SIZE);
#else
      len = _farpeekl(selector, base + 4) - ACPI_TBLE_HDR_SIZE;
      printf(" ACPI: DSDT:  AML code size %i\n", len);
#endif
      
      // check crc
      if (acpi_crc_check(selector, base + 0, _farpeekl(selector, base + 4)) != 0) {
        printf(" ACPI:DSDT table doesn't crc...\n");
      } else {
        aml_code = (bit8u *) malloc(len);
        if (aml_code != NULL) {
          // copy the aml code to our buffer
          for (i=0; i<len; i++)
#if INCLUDE_DSDT_HDR
            aml_code[i] = _farpeekb(selector, base + i);
#else
            aml_code[i] = _farpeekb(selector, base + ACPI_TBLE_HDR_SIZE + i);
#endif
          acpi_decode_aml(aml_code, len);
        } else
          puts("Error trying to allocate memory for AML code...");
      }
      puts("");
      break;
      
    default:
      printf("Found '%c%c%c%c' description table id.\n", 
        _farpeekb(selector, base + 0), _farpeekb(selector, base + 1), _farpeekb(selector, base + 2), _farpeekb(selector, base + 3));
      puts("");
  }
  
  if (addr >= 0x100000)
    __dpmi_free_physical_address_mapping(&base_mi);
}

void acpi_decode_aml(bit8u *aml_code, const int len) {
  FILE *fp;
  
  // remember that if INCLUDE_DSDT_HDR is not zero, there
  //  will be ACPI_TBLE_HDR_SIZE bytes of the DSDT header prepended
  //  to this buffer and the len field will include this header.
  printf("Decode %i bytes of aml here...\n", len);
  
  // if filename given on command line, write the AML code
  //  to this filename
  if (strlen(out_file)) {
    if ((fp = fopen(out_file, "w+b")) != NULL) {
      fwrite(aml_code, len, 1, fp);
      fclose(fp);
    }
  }
  
}

// 'Allocates' some physical memory
bool get_physical_mapping(__dpmi_meminfo *mi, int *selector) {
  int sel;
  
  if (__dpmi_physical_address_mapping(mi) == -1)
    return FALSE;
  sel = __dpmi_allocate_ldt_descriptors(1);
  if (sel == -1)
    return FALSE;
  if (__dpmi_set_segment_base_address(sel, mi->address) == -1)
    return FALSE;
  if (__dpmi_set_segment_limit(sel, mi->size - 1))
    return FALSE;
  
  *selector = sel;
  return TRUE;
}
