/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The System Core, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef FYSOS_MPS
#define FYSOS_MPS

#pragma pack(1)

#define MAX_TABLE_SIZE    32768

#define RSD_SIG0 0x20445352   // 'RSD ' in little endian
#define RSD_SIG1 0x20525450   // 'PTR ' in little endian

struct S_ACPI_RSDP {
  // version 1.0+
  bit8u  sig[8];
  bit8u  crc;        // of bytes 0-19 only
  bit8u  oem_id[6];
  bit8u  version;    // 0 = ACPI 1.0  --  2 = ACPI 2+
  bit32u rsdt_addr;
  // version 2.0+
  bit32u len;        // length of entire table
  bit32u xsdt_addr[2];
  bit8u  xcrc;       // of entire table including both crc's
  bit8u  resv[3];
};

#define ACPI_TBLE_HDR_SIZE  36
struct S_ACPI_TBLE_HDR {
  bit32u sig;
  bit32u length;
  bit8u  rev;
  bit8u  crc;
  bit8u  oem_id[6];
  bit8u  oem_tble_id[8];
  bit32u oem_rev;
  bit32u creator_id;
  bit32u creater_rev;
};



bit32u acpi_find_sig(bit32u, const bit32u, const bit32u, const bit32u);
bit8u acpi_crc_check(const int, const bit32u, const int);
void acpi_enum_tble(const bit32u);
void acpi_decode_aml(bit8u *, const int);

bool get_physical_mapping(__dpmi_meminfo *, int *);


#endif // FYSOS_MPS
