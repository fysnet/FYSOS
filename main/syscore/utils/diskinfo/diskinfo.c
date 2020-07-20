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
 *  DISKINFO.EXE
 *   Dumps information about the BIOS found disks.
 *
 *  Assumptions/prerequisites:
 *  - Must be ran from TRUE DOS, no windows/linux sessions.
 *
 *  Last updated: 20 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os diskinfo.c -o diskinfo.exe -s
 *
 *  Usage:
 *    diskinfo
 *    diskinfo 80
 *  If parameter is given, it is a hexadecimal value of drive to get.
 *  If no parameter is given, diskinfo checks all drives attached to system.
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dpmi.h>
#include <go32.h>

#include "../include/ctype.h"
#include "diskinfo.h"


int main(int argc, char *argv[]) {
  __dpmi_regs regs;
  
  int drv_id_start = 0x00;
  int drv_id_end = 0xFF;
  int drv_id_spec = -1, drv_id;

  printf("\nTest: Get Disk Info. v1.00.00"
         "\nForever Young Software -- (c) Copyright 1984-2015");
  
  // Get the drive from the command line.
  if (argc == 2) {
    sscanf(argv[1], "%X", &drv_id_spec);
    
    // check that  0 <= drv <= FF
    if ((drv_id_spec < 0) || (drv_id_spec > 0xFF)) {
      printf("\n Invalid Drive Number:  0 <= drv <= FF\n");
      return -2;
    } else
      drv_id_start = drv_id_end = drv_id_spec;
  }
  
  for (drv_id = drv_id_start; drv_id <= drv_id_end; drv_id++) {
    // we skip 0x04 -> 0x7F (only if drive given on command line != 0x04)
    if ((drv_id == 0x04) && (drv_id_spec != 0x04))
      drv_id = 0x80;

    // check for a keypress and exit if so.
    if (kbhit())
      break;
    
    bit32u sectors = 0;
    
    // reset the disk system
    regs.h.ah = 0x00;
    regs.h.dl = (bit8u) drv_id;
    __dpmi_int(0x13, &regs);
    
    // get disk type.  This will ensure that there is or is not a drive
    //  at this ID number
    regs.x.ax = 0x15FF;
    regs.x.cx = 0xFFFF;
    regs.h.dl = (bit8u) drv_id;
    __dpmi_int(0x13, &regs);
    if ((regs.x.flags & 1) ||
      (((regs.x.flags & 1) == 0) && (regs.h.ah == 0))) {
      if (drv_id == drv_id_spec)
        printf("\n No disk drive found at: 0x%02X", drv_id);
      continue;
    } else {
      printf("\n\n=-=- %02Xh =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-", drv_id);
      printf("\n Drive has type: (%i)", regs.h.ah);
      switch (regs.h.ah) {
        case 01:  // 01h floppy without change-line support
          printf(" floppy without change-line support");
          break;
        case 02:
          printf(" floppy (or other removable drive) with change-line support");
          break;
        case 03:
          sectors = (((bit32u) regs.x.cx << 16) | regs.x.dx);
          printf(" hard disk with %i sectors (from function 15h)", sectors);
          break;
        default:
          printf(" unknown");
      }
    }
    
    // get standard parameters
    standard_params(&regs, drv_id);
    
    // check to see if we can use the IBM/MS INT 13 Extensions
    regs.h.ah = 0x41;
    regs.x.bx = 0x55AA;
    regs.h.dl = (bit8u) drv_id;
    __dpmi_int(0x13, &regs);
    if (!(regs.x.flags & 1) && (regs.x.bx == 0xAA55))
      int13_extentions(&regs, drv_id, sectors);
    
    // check to see if we can get the IDENT info
    struct S_IDENTIFY identify;
    memset(&identify, 0, sizeof(struct S_IDENTIFY));
    regs.h.ah = 0x25;
    regs.h.dl = (bit8u) drv_id;
    regs.x.es = (__tb >> 4);
    regs.x.di = (__tb & 0xF);
    __dpmi_int(0x13, &regs);
    if (!(regs.x.flags & 1) && (regs.h.ah == 0)) {
      dosmemget(__tb, 512, &identify);
      int13_identification(&identify);
    } else
      printf("\n Could not retrieve the Identify information.");
    
  }  
}

int standard_params(__dpmi_regs *regs, const int drv_id) {
  
  regs->h.ah = 0x08;
  regs->h.dl = (bit8u) drv_id;
  regs->x.di = 0;
  regs->x.es = 0;
  __dpmi_int(0x13, regs);
  if ((regs->x.flags & 1) || (regs->h.ah != 0))
    return -4;
  
  printf("\nStandard BIOS disk Get Parameters:");
  printf("\n            Drive Type (%02X): ", regs->h.bl);
  switch (regs->h.bl) {
    case 0x01:
      printf("360K");
      break;
    case 0x02:
      printf("1.2M");
      break;
    case 0x03:
      printf("720K");
      break;
    case 0x04:
      printf("1.44M");
      break;
    case 0x05:
      printf("2.88M ???");
      break;
    case 0x06:
      printf("2.88M");
      break;
    case 0x10:
      printf("ATAPI Removable Media Device");
      break;
    default:
      printf("Unknown");
  }
  
  printf("\n                  Cylinders: %i", ((regs->h.cl >> 6) | regs->h.cl) + 1);
  printf("\n                      Heads: %i", (regs->h.dh + 1));
  printf("\n          Sectors per Track: %i", regs->h.cl & 0x3F);
  printf("\n   Number of drives of type: %i", regs->h.dl);
  
  printf("\n  Drive Parameter Table Ptr: %04X:%04Xh", regs->x.es, regs->x.di);
  if (regs->x.es && regs->x.di) {
    struct S_1E_DISK_PARAMS params;
    memset(&params, 0, sizeof(struct S_1E_DISK_PARAMS));
    dosmemget((regs->x.es << 4) + regs->x.di, sizeof(struct S_1E_DISK_PARAMS), &params);
    printf("\n                    Step Rate: %i", params.specify0 >> 4);
    printf("\n                  Head Unload: %i", params.specify0 & 0xF);
    printf("\n                    Head Load: %i (%i)", params.specify1 >> 1, (params.specify1 >> 1) * 4);
    printf("\n                 Non DMA Mode: %i", params.specify1 & 1);
    printf("\n              Motor Off Delay: %i", params.delay);
    printf("\n             Bytes per Sector: %i", 128 << params.bytes_sector);
    printf("\n            Sectors per Track: %i", params.sects_track);
    printf("\n                   Gap Length: %02Xh", params.gap_len);
    printf("\n                  Data Length: %02Xh", params.data_len);
    printf("\n        Formatting Gap Length: %02Xh", params.gap_len_f);
    printf("\n           Format Filler Byte: %02Xh", params.filler);
    printf("\n             Head Settle Time: %i", params.head_settle);
    printf("\n             Motor Start Time: %i", params.motor_start);
  }
  
  return 0;
}

int int13_extentions(__dpmi_regs *regs, const int drv_id, const bit32u sectors) {
  struct S_DRV_PARAMS params;
  
  // clear our buffer, setting the length field
  memset(&params, 0, sizeof(struct S_DRV_PARAMS));
  
  printf("\n\nFound BIOS Disk Extensions version: ");
  switch (regs->h.ah) {
    case 0x01:  // version 1.x
      printf("1.x (dh = 0x%02X)", regs->h.dh);
      params.size = 26;
      break;
    case 0x20:  // version 2.0 / EDD-1.0
      printf("2.0 / EDD - 1.0 (dh = 0x%02X)", regs->h.dh);
      params.size = 30;
      break;
    case 0x21:  // version 2.1 / EDD-1.1
      printf("2.1 / EDD - 1.1 (dh = 0x%02X)", regs->h.dh);
      params.size = 30;
      break;
    case 0x30:  //  EDD-3.0
      printf("EDD - 3.0 (dh = 0x%02X)", regs->h.dh);
      params.size = 66;
      break;
    default:
      printf(" unknown version found: 0x%02X", regs->h.ah);
  }
  printf(" for drv ID: 0x%02X", drv_id);
  
  if (regs->x.cx & (1<<0))
    printf("\n Functions 42h, 43h, 44h, 47h, and 48h are supported.");
  if (regs->x.cx & (1<<1))
    printf("\n Removable Device Functions 45h, 46h, 48h, 49h, and INT 15h/AH=52h are supported.");
  if (regs->x.cx & (1<<2))
    printf("\n EDD functions 48h and 4Eh are supported."
           "\n  The Drive Parameter Table and the Phoenix Enhanced Disk Drive Spec"
           "\n     Fixed Disk Parameter Table is supported.");
  if (regs->x.cx & ~0x0007)
    printf("\n CX has reserved bits set:  0x%04X", regs->x.cx);
  
  // now if bit 2 in CX was set, continue
  if (regs->x.cx & (1<<2)) {
    // copy our buffer to conventional memory and call the BIOS
    dosmemput(&params, params.size, __tb);
    regs->h.ah = 0x48;
    regs->h.dl = (bit8u) drv_id;
    regs->x.si = __tb & 0x0F;
    regs->x.ds = __tb >> 4;
    __dpmi_int(0x13, regs);
    if (regs->x.flags & 1) {
      printf("\n Get Parameters (ah = 48h) returned Carry Set (error = 0x%02X)", regs->h.ah);
      return -4;
    }
    // copy the data to our buffer
    dosmemget(__tb, params.size, &params);
  
    printf("\n Parameters returned:");
    printf("\n      Size returned: %i", params.size);
    printf("\n  Information Flags: 0x%02X", params.flags);
    printf("\n     DMA Boundary errors handled transparently: %s", (params.flags & (1<<0)) ? "TRUE" : "FALSE");
    printf("\n    Cylinder/head/sectors per track info valid: %s", (params.flags & (1<<1)) ? "TRUE" : "FALSE");
    printf("\n                               Removable Drive: %s", (params.flags & (1<<2)) ? "TRUE" : "FALSE");
    printf("\n                   Write with verify supported: %s", (params.flags & (1<<3)) ? "TRUE" : "FALSE");
    printf("\n                 Drive has Change Line Support: %s", (params.flags & (1<<2)) ? ((params.flags & (1<<4)) ? "TRUE" : "FALSE") : "Bit not valid");
    printf("\n                           Drive can be locked: %s", (params.flags & (1<<5)) ? "TRUE" : "FALSE");
    printf("\n         CHS information set to maximum values: %s", (params.flags & (1<<2)) ? ((params.flags & (1<<6)) ? "TRUE" : "FALSE") : "Bit not valid");
    if (params.flags & ~0x007F)
      printf("\n         Bits 15:7 are not zero: 0x%04X", params.flags);
    printf("\n    Total Cylinders: %i%c", params.cylinders, (params.flags & (1<<1)) ? ' ' : '*');
    printf("\n        Total Heads: %i%c", params.heads, (params.flags & (1<<1)) ? ' ' : '*');
    printf("\n  Sectors per Track: %i%c", params.sects_per_trk, (params.flags & (1<<1)) ? ' ' : '*');
    printf("\n      Total Sectors: %lli", params.tot_sectors);
    if ((sectors != 0) && (params.tot_sectors != sectors))
      printf(" (doesn't match what function 15h reported.)");
    if (!(params.flags & (1<<1)))
      printf("\n *If Cyl/Head/Sect info above is FALSE, these values are not valid.");
    printf("\n   Bytes per Sector: %i", params.sector_size);
    if (params.size >= 30) {
      bit16u seg = (params.config_params_ptr >> 16) & 0xFFFF;
      bit16u off = params.config_params_ptr & 0xFFFF;
      printf("\n Pointer to EDD Config Table: %04X:%04Xh", seg, off);
      if (params.config_params_ptr != 0xFFFFFFFF) {
        struct S_FIXED_DISK_PARAMS dpte;
        memset(&dpte, 0, sizeof(struct S_FIXED_DISK_PARAMS));
        dosmemget((seg << 4) + off, sizeof(struct S_FIXED_DISK_PARAMS), &dpte);
        if ((dpte.prog_io_cntrl & 0xFF) == 0xA0)
          printf("\n Use Translated Table????"); // Table 00277  ????
        ///////////////////////////////////
        // Table 00278
        printf("\n             Port IO Address: 0x%04X", dpte.port_base);
        printf("\n          Port Cntrl Address: 0x%04X", dpte.prog_io_cntrl);
        printf("\n                 Drive Flags: 0x%02X (%s) (%s)", dpte.flags, (dpte.flags & (1<<4)) ? "slave" : "master", (dpte.flags & (1<<6)) ? "lba" : "");
        printf("\n            Proprietary Info: 0x%04X", dpte.propty_info);
        printf("\n                         irq: %i (0x%02X)", dpte.irq & 0x0F, dpte.irq);
        printf("\n  Cnt for multi-sect transfs: %i", dpte.multi_sect_cnt);
        printf("\n        DMA Type and channel: %i, %i", dpte.dma_cntrl >> 4, dpte.dma_cntrl & 0xF);
        printf("\n         Programmed IO Cntrl: %i (0x%02X)", dpte.prog_io_cntrl & 0x0F, dpte.prog_io_cntrl);
        printf("\n               Drive Options: 0x%04X", dpte.options);
        printf("\n               Fast PIO Enabled: %i", (dpte.options & (1<<0)) ? 1 : 0);
        printf("\n               Fast DMA Enabled: %i", (dpte.options & (1<<1)) ? 1 : 0);
        printf("\n                      Block PIO: %i", (dpte.options & (1<<2)) ? 1 : 0);
        printf("\n                CHS Translation: %i", (dpte.options & (1<<3)) ? 1 : 0);
        printf("\n                LBA translation: %i", (dpte.options & (1<<4)) ? 1 : 0);
        printf("\n                Removable Media: %i", (dpte.options & (1<<5)) ? 1 : 0);
        printf("\n                   ATAPI Device: %i", (dpte.options & (1<<6)) ? 1 : 0);
        printf("\n           32-bit transfer mode: %i", (dpte.options & (1<<7)) ? 1 : 0);
        printf("\n          ATAPI uses DRQ signal: %i", (dpte.options & (1<<8)) ? 1 : 0);
        printf("\n               Translation Type: %i", (dpte.options & (3<<9)) >> 9);
        if (!(dpte.options & (1<<3)) && (((dpte.options & (3<<9)) >> 9) != 0))
          printf(" (bit three above is clear and this field > 0)");
        if (params.size == 66) { // ver 3.0?
          printf("\n                      Ultra DMA: %i", (dpte.options & (1<<11)) ? 1 : 0);
          printf("\n                       Reserved: 0x%X", (dpte.options & (0xF<<12)) >> 12);
        }
        printf("\n                    Reserved: %02X %02X", dpte.resv[0], dpte.resv[1]);
        printf("\n               Ext. Revision: %i.%i", dpte.ext_revision >> 4, dpte.ext_revision & 0xF);
        printf("\n                         crc: %02X ", dpte.crc);
        if (calc_crc(&dpte, sizeof(struct S_FIXED_DISK_PARAMS)) == 0)
          printf("(correct)");
        else
          printf("(incorrect)");
      }
    }
    if (params.size >= 66) {
      printf("\n       Device Path Signature: 0x%04X", params.bedd_sig);
      if (params.bedd_sig == 0xBEDD) {
        printf("\n          Device Path Length: %i", params.bedd_len);
        printf("\n              Reserved Bytes: %02X %02X %02X", params.resv0[0], params.resv0[1], params.resv0[2]);
        printf("\n               Host Bus Name: %c%c%c", params.host_bus_name[0], params.host_bus_name[1], params.host_bus_name[2]);
        printf("\n              Interface Type: %c%c%c%c%c%c%c", params.interface_type[0], params.interface_type[1], params.interface_type[2],
                                                                 params.interface_type[3], params.interface_type[4], params.interface_type[5],
                                                                 params.interface_type[6]);
        // interface path
        if (strcmp(params.host_bus_name, "ISA") == 0) {
          printf("\n              ISA:   Base Address: 0x%04X", params.int_path.isa.base_addr);
          printf("\n              ISA: Reserved Bytes: %02X %02X %02X %02X %02X %02X", params.int_path.isa.resv[0], params.int_path.isa.resv[1],
                                                                                       params.int_path.isa.resv[2], params.int_path.isa.resv[3],
                                                                                       params.int_path.isa.resv[4], params.int_path.isa.resv[5]);
        } else if (strcmp(params.host_bus_name, "PCI") == 0) {
          printf("\n              PCI:     Bus Number: %i", params.int_path.pci.bus);
          printf("\n              PCI:     Dev Number: %i", params.int_path.pci.dev);
          printf("\n              PCI:    Func Number: %i", params.int_path.pci.func);
          printf("\n              PCI: Reserved Bytes: %02X %02X %02X %02X %02X", params.int_path.pci.resv[0], params.int_path.pci.resv[1],
                                                                                  params.int_path.pci.resv[2], params.int_path.pci.resv[3],
                                                                                  params.int_path.pci.resv[4]);
        }
        // device path
        if (strcmp(params.interface_type, "ATA") == 0) {
          printf("\n              ATA: Master or Slave: 0x%02X", params.dev_path.ata.flag);
          printf("\n              ATA:  Reserved Bytes: %02X %02X %02X %02X %02X %02X %02X", params.dev_path.ata.resv[0], params.dev_path.ata.resv[1],
                                                                                             params.dev_path.ata.resv[2], params.dev_path.ata.resv[3],
                                                                                             params.dev_path.ata.resv[4], params.dev_path.ata.resv[5],
                                                                                             params.dev_path.ata.resv[6]);
        } else if (strcmp(params.interface_type, "ATAPI") == 0) {
          printf("\n            ATAPI: Master or Slave: 0x%02X", params.dev_path.atapi.flag);
          printf("\n            ATAPI:  Logical Unit #: %i", params.dev_path.atapi.lun);
          printf("\n            ATAPI:  Reserved Bytes: %02X %02X %02X %02X %02X %02X", params.dev_path.atapi.resv[0], params.dev_path.atapi.resv[1],
                                                                                        params.dev_path.atapi.resv[2], params.dev_path.atapi.resv[3],
                                                                                        params.dev_path.atapi.resv[4], params.dev_path.atapi.resv[5]);
        } else if (strcmp(params.interface_type, "SCSI") == 0) {
          printf("\n             SCSI:  Logical Unit #: %i", params.dev_path.scsi.lun);
          printf("\n             SCSI:  Reserved Bytes: %02X %02X %02X %02X %02X %02X %02X", params.dev_path.scsi.resv[0], params.dev_path.scsi.resv[1],
                                                                                             params.dev_path.scsi.resv[2], params.dev_path.scsi.resv[3],
                                                                                             params.dev_path.scsi.resv[4], params.dev_path.scsi.resv[5],
                                                                                             params.dev_path.scsi.resv[6]);
        } else if (strcmp(params.interface_type, "USB") == 0) {
          printf("\n              USB:   Unknown value: 0x%02X", params.dev_path.usb.unknown);
          printf("\n              USB:  Reserved Bytes: %02X %02X %02X %02X %02X %02X %02X", params.dev_path.usb.resv[0], params.dev_path.usb.resv[1],
                                                                                             params.dev_path.usb.resv[2], params.dev_path.usb.resv[3],
                                                                                             params.dev_path.usb.resv[4], params.dev_path.usb.resv[5],
                                                                                             params.dev_path.usb.resv[6]);
        } else if (strcmp(params.interface_type, "1394") == 0) {
          printf("\n         IEEE1394:           GUID: %016llX", params.dev_path.ieee.guid);
        } else if (strcmp(params.interface_type, "FIBRE") == 0) {
          printf("\n            Fibre:            WWN: %16lli", params.dev_path.fibre.wwn);
        }
        printf("\n               Reserved Byte: %02X", params.resv1);
        printf("\n               Checksum Byte: %02X", params.crc);
        if (calc_crc(&params, sizeof(struct S_DRV_PARAMS)) == 0)
          printf("(correct)");
        else
          printf("(incorrect)");
      }
    }
  }
  
  return 0;
}

// calculates the crc (2's compliment)
bit8u calc_crc(void *ptr, int cnt) {
  bit8u crc = 0;
  bit8u *p = (bit8u *) ptr;
  
  while (cnt--)
    crc += *p++;
  
  return crc;
}

int int13_identification(struct S_IDENTIFY *identify) {
  int i;
  
  printf("\n INT 13h/25h Identify Command.");
  printf("\n");
  printf("\n      General Drive Config:"
         "\n                                 reserved: %i"
         "\n      Format speed tolerance gap required: %i"
         "\n            Track offset option available: %i"
         "\n      Data strobe offset option available: %i"
         "\n     Rotational speed tolerance is > 0.5%: %i"
         "\n              Disk transfer rate > 10 Mbs: %i"
         "\n   Disk transfer rate > 5Mbs but <= 10Mbs: %i"
         "\n               Disk transfer rate <= 5Mbs: %i"
         "\n                Removable cartridge drive: %i"
         "\n                              Fixed drive: %i"
         "\n Spindle motor control option implemented: %i"
         "\n               Head switch time > 15 usec: %i"
         "\n                          Not MFM encoded: %i"
         "\n                            Soft sectored: %i"
         "\n                            Hard sectored: %i"
         "\n                                 reserved: %i",
         identify->get_conf & (1<<15),
         identify->get_conf & (1<<14),
         identify->get_conf & (1<<13),
         identify->get_conf & (1<<12),
         identify->get_conf & (1<<11),
         identify->get_conf & (1<<10),
         identify->get_conf & (1<< 9),
         identify->get_conf & (1<< 8),
         identify->get_conf & (1<< 7),
         identify->get_conf & (1<< 6),
         identify->get_conf & (1<< 5),
         identify->get_conf & (1<< 4),
         identify->get_conf & (1<< 3),
         identify->get_conf & (1<< 2),
         identify->get_conf & (1<< 1),
         identify->get_conf & (1<< 0));
  printf("\n                 Cylinders: %i", identify->cylinders);
  printf("\n                  reserved: %02X", identify->resv0);
  printf("\n                     Heads: %i", identify->heads);
  printf("\n   Unformatted bytes/track: %i", identify->un_bytes_trck);
  printf("\n  Unformatted bytes/sector: %i", identify->un_bytes_sect);
  printf("\n         Sectors per Track: %i", identify->sects_track);
  printf("\n          Vendor Unique ID: %04X%04X%04X", 
    ENDIAN_16(identify->vendor_unique0[0]), ENDIAN_16(identify->vendor_unique0[1]), ENDIAN_16(identify->vendor_unique0[2]));
  printf("\n             Serial Number: ");
  if (* (bit16u *) &identify->serial_num[0] != 0x0000)
    for (i=0; i<10; i++)
      printf("%c", identify->serial_num[i]);
  else
    printf("Not Specified.");
  printf("\n               Buffer type: %04X", identify->buffer_type);
  printf("\n               Buffer size: %i (* 512)", identify->buff_size);
  printf("\n       Number of EEC bytes: %i", identify->num_eec_bytes);
  printf("\n         Firmware reserved: ");
  for (i=0; i<8; i++)
    printf("%02X ", identify->firmware_rev[i]);
  printf("\n              Model Number: ");
  if (* (bit16u *) &identify->model_num[0] != 0x0000)
    for (i=0; i<40; i++)
      printf("%c", identify->model_num[i]);
  else
    printf("Not Specified.");
  printf("\n          Vendor Unique ID: %04X", identify->vendor_unique1);
  printf("\n              Double Words: %i", identify->double_words);
  printf("\n              Capabilities:"
         "\n                            ATAPI present: (%i%i) = ",
         (identify->caps & (1<<15)) >> 15, (identify->caps & (1<<14)) >> 14);
  if ((identify->caps & ((1<<15) | (1<<14))) == 0)
    printf("no ATA(PI)");
  else if ((identify->caps & ((1<<15) | (1<<14))) == (1<<14))
    printf("ATA (at least version 3)");
  else if ((identify->caps & ((1<<15) | (1<<14))) == (1<<15))
    printf("ATAPI");
  else 
    printf("unknown");
  printf("\n                                 Supports: %s",
    ((identify->caps & (1<<13)) == 0) ? "CHS support only" : "LBA support");
  printf("\n           Doubleword read/write support?: %s",
    ((identify->caps & (1<<12)) == 0) ? "no" : "yes");
  printf("\n                           Optical Drive?: %s");
  if ((identify->caps & ((1<<11) | (1<<10) | (1<<9))) == 0)
    printf("not optical");
  else if ((identify->caps & ((1<<11) | (1<<10) | (1<<9))) == (1<<9))
    printf("CD-ROM");
  else if ((identify->caps & ((1<<11) | (1<<10) | (1<<9))) == (1<<10))
    printf("CD-R/CD-RW");
  else if ((identify->caps & ((1<<11) | (1<<10) | (1<<9))) == ((1<<10) | (1<<9)))
    printf("DVD");
  else 
    printf("unknown");
  printf("\n                                 reserved: %i", (identify->caps & (1<<8)) >> 8);
  printf("\n                            DMA supported: %s",
    ((identify->caps & (1<<7)) == 0) ? "no" : "yes");
  printf("\n                                 DMA used: %i", (identify->caps & ((1<<6) | (1<<5) | (1<<4)) >> 4));
  printf("\n                                 reserved: %02X", (identify->caps & ((1<<3) | (1<<2) | (1<<1) | (1<<0))));
  printf("\n              Capabilities: %04X", identify->caps2);
  printf("\n                PIO Timing: %i", identify->PIO_timing);
  printf("\n                DMA Timing: %i", identify->DMA_timing);
  printf("\n               Rest valid?: %i", identify->rest_valid);
  printf("\n         Current Cylinders: %i", identify->num_cur_cylinders);
  printf("\n             Current Heads: %i", identify->num_cur_heads);
  printf("\n     Current sectors/track: %i", identify->num_cur_sect_track);
  printf("\n          Current Capacity: %i (%i)", identify->cur_capacity, identify->cur_capacity * (identify->buff_size * 512));
  printf("\n                  reserved: %04X", identify->resv2);
  printf("\n              LBA Capacity: %i (%i)", identify->lba_capacity, identify->lba_capacity * (identify->buff_size * 512));
  printf("\n                  reserved: %04X", identify->resv3);
  printf("\n            Multi-word DMA: %i", identify->multiword_dma);
  printf("\n                 PIO modes: %i", identify->pio_modes);
  printf("\n        Min multi-word DMA: %i", identify->min_multi_dma);
  printf("\nManufacture multi-word DMA: %i", identify->manuf_min_multi_dma);
  printf("\n            Min PIO Cycles: %i %i", identify->min_pio_cycle0, identify->min_pio_cycle1);
  printf("\n                  reserved: %04X %04X", identify->resv4[0], identify->resv4[1]);
  printf("\n                  reserved: %04X %04X %04X %04X", identify->resv5[0], identify->resv5[1], identify->resv5[2], identify->resv5[3]);
  printf("\n               Queue Depth: %i", identify->queue_depth);
  printf("\n                  reserved: %04X %04X %04X %04X", identify->resv6[0], identify->resv6[1], identify->resv6[2], identify->resv6[3]);
  printf("\n  Major Versions supported: ");
  for (i=0; i<15; i++) {
    if (identify->major_ver & (1<<i)) {
      if (i < 4)
        printf("\n                             ATA%i", i);
      else
        printf("\n                             ATAPI%i", i);
    }
  }  
  printf("\n             Minor Version: %i", identify->minor_ver);
  printf("\n             Command Set 1: %04X", identify->command_set1);
  printf("\n             Command Set 2: %04X", identify->command_set2);
  printf("\n                     CFSSE: %04X", identify->cfsse);
  printf("\n              CFS Enable 1: %04X", identify->cfs_enable_1);
  printf("\n              CFS Enable 2: %04X", identify->cfs_enable_2);
  printf("\n               CFS Default: %04X", identify->csf_default);
  printf("\n                 DMA Ultra: %04X", identify->dma_ultra);
  printf("\n                    TRSEUC: %04X %04X", identify->trseuc, identify->trsEuc);
  printf("\n        Current APM values: %04X", identify->cur_apm_values);
  printf("\n                      MPRC: %04X", identify->mprc);
  printf("\n           Hardware Config: %04X", identify->hw_config);
  printf("\n                  Acoustic: %04X", identify->acoustic);
  printf("\n                     MSRQS: %04X", identify->msrqs);
  printf("\n                    SXFERT: %04X", identify->sxfert);
  printf("\n                       SAL: %04X", identify->sal);
  printf("\n                       SPG: %04X", identify->spg);
  printf("\n       48-bit # of Sectors: %" LL64BIT "i (%" LL64BIT "i)", identify->lba_capacity2, (identify->lba_capacity2 * ((bit64u) identify->buff_size * 512)));
  printf("\n                  reserved: ");
  for (i=0; i<22; i++) {
    if ((i % 10) == 0)
      printf("\n");
    printf("%04X ", identify->resv7[i]);
  }
  printf("\n                  Last LUN: %i", identify->last_lun);
  printf("\n                  reserved: %04X", identify->word127);
  printf("\n                       DLF: %04X", identify->dlf);
  printf("\n                      CSFO: %04X", identify->csfo);
  printf("\n                  reserved: ");
  for (i=0; i<8; i++)
    printf("%04X ", identify->resv8[i]);
  printf("\n                 CFA Power: %04X", identify->cfa_power);
  printf("\n                  reserved: ");
  for (i=0; i<15; i++) {
    if ((i % 10) == 0)
      printf("\n");
    printf("%04X ", identify->resv9[i]);
  }
  printf("\n  Current Media Serial Num:");
  if (* (bit16u *) &identify->cur_media_sn[0] != 0x0000)
    for (i=0; i<60; i++)
      printf("%c", identify->cur_media_sn[i]);
  else
    printf("Not Specified.");
  printf("\n                  reserved: ");
  for (i=0; i<49; i++) {
    if ((i % 10) == 0)
      printf("\n");
    printf("%04X ", identify->resvA[i]);
  }
  printf("\n                 Integrity: %04X", identify->integrity);
  
  return 0;
}
