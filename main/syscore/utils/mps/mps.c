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
 *  MPS.EXE
 *
 *  Assumptions/prerequisites:
 *
 *  Last updated: 20 July 2020
 *
 *  This utility is compiled as a 16-bit DOS .EXE using a 16-bit compiler.
 *  This utility uses 16-bit segmented far pointers where a far pointer is
 *   made up as a 32-bit address of two 16-bit values:  seg:offset
 *
 *  Usage:
 *   MPS
 *   MPS > results.txt
 */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"

#include "mps.h"

char *assign_int_type[7] = { "INT", "NMI", "SMI", "ExtINT" };
char bus_names[256][7];

int main(int argc, char *argv[]) {
  struct S_MP_FLOATP_STRUCT far *fp_struct;
  struct S_MP_CONFIG_TABLE far *table;
  struct S_MP_PROCESSOR far *p_entry;
  struct S_MP_BUS far *bus_entry;
  struct S_MP_IOAPIC far *io_apic;
  struct S_MP_ASSIGN far *assign;
  bit32u dword;
  bit32u addr = 0;
  int i;
  
  // print start string
  printf("\nDump MPS  v00.10.00 (C)opyright   Forever Young Software 1984-2020\n\n");
  
  // first get the EBDA address from the BIOS
  // the BIOS should give us a 16-bit segment at 0x0040E
  // should be something like 0x9FC0
  dword = ((bit32u) (* (bit16u far *) 0x0000040E)) << 4;
  if (dword)
    addr = mp_find_sig(dword, (dword + 0x1000 - 1), MP_SIG);
  
  // else search from 511k to 512k
  if (addr == 0)
    addr = mp_find_sig(0x7FC00, (0x7FC00 + 0x1000 - 1), MP_SIG);
  
  // else search from 639k to 640k
  if (addr == 0)
    addr = mp_find_sig(0x9FC00, (0x9FC00 + 0x1000 - 1), MP_SIG);
  
  // else search from 0xF0000 to 0xFFFFF
  if (addr == 0)
    addr = mp_find_sig(0xF0000, 0xFFFFF, MP_SIG);
  
  // if we found it above, find the Config Table and extract
  //  the LAPIC address
  if (addr != 0) {
    fp_struct = (struct S_MP_FLOATP_STRUCT far *) MK_FP(addr);
    printf(" MP Floating Point Structure: 0x%08lX\n", addr);
    printf("             Signature: '%c%c%c%c' (0x%08lX)\n", 
      (char) ((bit32u) (fp_struct->sig & 0xFF000000) >> 24),
      (char) ((bit32u) (fp_struct->sig & 0x00FF0000) >> 16),
      (char) ((bit32u) (fp_struct->sig & 0x0000FF00) >>  8),
      (char) ((bit32u) (fp_struct->sig & 0x000000FF) >>  0),
      fp_struct->sig);
    printf("  Config Table Address: 0x%08lX\n", fp_struct->address);
    printf("          Table Length: %i\n", fp_struct->len);
    printf("           MPS Version: 1.%i\n", fp_struct->version);
    puts("");
    if (fp_struct->address) {  // physical address of MP Config Table
      table = (struct S_MP_CONFIG_TABLE far *) MK_FP(fp_struct->address);  // physical address of MP Config Table
      if ((table->sig == 0x504D4350) && (mp_crc_check(fp_struct->address, table->len) == 0)) {
        printf(" MP Configuration Table: 0x%08lX\n", fp_struct->address);
        printf("              Signature: '%c%c%c%c' (0x%08lX)\n", 
          (char) ((bit32u) (table->sig & 0xFF000000) >> 24),
          (char) ((bit32u) (table->sig & 0x00FF0000) >> 16),
          (char) ((bit32u) (table->sig & 0x0000FF00) >>  8),
          (char) ((bit32u) (table->sig & 0x000000FF) >>  0),
          table->sig);
        printf("           Table Length: %i\n", table->len);
        printf("            MPS Version: 1.%i\n", table->version);
        printf("                 OEM ID: %c%c%c%c%c%c%c%c\n",
          table->oem_id[0], table->oem_id[1], table->oem_id[2], table->oem_id[3],
          table->oem_id[4], table->oem_id[5], table->oem_id[6], table->oem_id[7]);
        printf("             Product ID: %c%c%c%c%c%c%c%c%c%c%c%c\n",
          table->prod_id[0], table->prod_id[1], table->prod_id[2], table->prod_id[3],
          table->prod_id[4], table->prod_id[5], table->prod_id[6], table->prod_id[7],
          table->prod_id[8], table->prod_id[9], table->prod_id[10], table->prod_id[11]);
        printf("      OEM Table Pointer: 0x%08lX\n", table->oem_table_ptr);
        printf("       OEM Table Length: %i\n", table->oem_len);
        printf("            Entry Count: %i\n", table->entry_count);
        printf("          LAPIC Address: 0x%08lX\n", table->lapic_addr);
        printf("     Extended Table Len: %i\n", table->ext_len);
        printf("     Extended Table CRC: %02X\n", table->ext_crc);
        printf("          Reserved Byte: %02X\n", table->resv);
        puts("");
        
        // start parsing the entries
        addr = fp_struct->address + sizeof(struct S_MP_CONFIG_TABLE);
        dword = table->entry_count;
        while ((addr < (fp_struct->address + sizeof(struct S_MP_CONFIG_TABLE) + table->len)) && (dword > 0)) {
          switch (* (bit8u far *) MK_FP(addr)) {
            case 0: // processor entry
              p_entry = (struct S_MP_PROCESSOR far *) MK_FP(addr);
              printf(" Processor Entry:\n");
              printf("        LAPIC ID: %i\n", p_entry->lapic_id);
              printf("       LAPIC ver: 0x%02X\n", p_entry->lapic_ver);
              printf("           Flags: 0x%02X (%susable, bsp = %s)\n", 
                p_entry->flags, ((p_entry->flags & 1) == 0) ? "un" : "", (p_entry->flags & 2) ? "yes" : "no");
              printf("   CPU Signature: 0x%08lX\n", p_entry->cpu_sig);
              printf("           Stepping: %i\n", (int) ((bit32u) (p_entry->cpu_sig & 0x0000000F) >> 0));
              printf("              Model: %i\n", (int) ((bit32u) (p_entry->cpu_sig & 0x000000F0) >> 4));
              printf("             Family: %i\n", (int) ((bit32u) (p_entry->cpu_sig & 0x00000F00) >> 8));
              printf("        Features: 0x%08lX\n", p_entry->features);
              puts("");
              addr += sizeof(struct S_MP_PROCESSOR);
              break;
              
            case 1: // bus entry
              bus_entry = (struct S_MP_BUS far *) MK_FP(addr);
              printf("       Bus Entry:\n");
              printf("          Bus ID: %i\n", bus_entry->bus_id);
              printf("            Type: %c%c%c%c%c%c\n",
                bus_entry->id[0], bus_entry->id[1], bus_entry->id[2],
                bus_entry->id[3], bus_entry->id[4], bus_entry->id[5]);
              puts("");
              addr += sizeof(struct S_MP_BUS);
              
              // copy the name for Assignment entries below
              // can't use memcpy() since we are using far pointers here
              //memcpy(bus_names[bus_entry->bus_id], bus_entry->id, 6);
              for (i=0; i<6 && (bus_entry->id[i] != 0x20); i++)
                bus_names[bus_entry->bus_id][i] = bus_entry->id[i];
              bus_names[bus_entry->bus_id][i] = '\0';
              break;
              
            case 2: // I/O APIC entry
              io_apic = (struct S_MP_IOAPIC far *) MK_FP(addr);
              printf("  I/O APIC Entry:\n");
              printf("     I/O APIC ID: %i\n", io_apic->apic_id);
              printf("    I/O APIC ver: 0x%02X\n", io_apic->apic_ver);
              printf("           Flags: 0x%02X (%susable)\n", 
                io_apic->flags, ((io_apic->flags & 1) == 0) ? "un" : "");
              printf("   Physical Addr: 0x%08lX\n", io_apic->address);
              puts("");
              addr += sizeof(struct S_MP_IOAPIC);
              break;
              
            case 3: // I/O Int Assignment entry
            case 4: // Local Int Assignment entry
              assign = (struct S_MP_ASSIGN far *) MK_FP(addr);
              if (assign->type == 3)
                printf("  I/O Assignment:\n");
              else
                printf("  Loc Assignment:\n");
              printf("  Interrupt type: %i (%s)\n", assign->int_type, assign_int_type[assign->int_type & 0x3]);
              printf("  Interrupt flag: 0x%04X ", assign->int_flag);
              switch ((assign->int_flag & (0x3 << 0)) >> 0) {
                case 0: printf("(conforms to bus) "); break;
                case 1: printf("(active high) "); break;
                case 2: printf("(reserved) "); break;
                case 3: printf("(active low) "); break;
              }
              switch ((assign->int_flag & (0x3 << 2)) >> 2) {
                case 0: printf("(conforms to bus) "); break;
                case 1: printf("(edge triggered) "); break;
                case 2: printf("(reserved) "); break;
                case 3: printf("(level triggered) "); break;
              }
              puts("");
              printf("      Source Bus: %i (%s)\n", assign->src_bus, bus_names[assign->src_bus]);
              if (!strcmp(bus_names[assign->src_bus], "ISA"))
                printf("      Source IRQ: %i\n", assign->src_irq);
              else if (!strcmp(bus_names[assign->src_bus], "PCI"))
                printf("      Source IRQ: INT_%c# on dev %i\n", 'A' + (assign->src_irq & 0x3), (assign->src_irq & 0x7C) >> 2);
              else
                printf("      Source IRQ: %i\n", assign->src_irq);
              printf("  Destination ID: %i\n", assign->dest_id);
              printf(" Destination Int: %i\n", assign->dest_int);
              puts("");
              addr += sizeof(struct S_MP_ASSIGN);
              break;
              
            default:
              dword = 1;  // mark end of table (-- below will be zero)
          }
          dword--;
        }
        
        return 1;
      }
    }
  }
  
  return 0;
}

// start is the physical address to start
// end is the physical address to stop
// sig is the 32-bit signature to look for
bit32u mp_find_sig(bit32u start, const  bit32u end, const bit32u sig) {
  bit32u far *s;
  while (start < end) {
    s = (bit32u far *) MK_FP(start);
    if ((*s == sig) && (mp_crc_check(start, 16) == 0))
      return start;
    start += 16;
  }
  return 0;
}

// crc check
// start is the physical address to start
bit8u mp_crc_check(const bit32u start, int len) {
  bit8u far *p = (bit8u far *) MK_FP(start);
  bit8u crc = 0;
  while (len--)
    crc += *p++;
  return crc;
}
