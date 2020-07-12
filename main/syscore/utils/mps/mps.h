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

#define MK_FP(a) ((((a) & 0xFFFFFFF0) << 12) | ((a) & 0x000F))

#define MP_SIG  0x5F504D5F  // '_PM_'  ('_MP_' in little endian)

struct S_MP_FLOATP_STRUCT {
  bit32u sig;         // 0x5F504D5F
  bit32u address;     // physical address of config table
  bit8u  len;         // len of this struct
  bit8u  version;     // 1.x where version == the x
  bit8u  crc;         //
  bit8u  features[5];
};

struct S_MP_CONFIG_TABLE {
  bit32u sig;         // 0x504D4350
  bit16u len;         // length of this table
  bit8u  version;     // 1.x where version == the x
  bit8u  crc;         //
  bit8u  oem_id[8];   // manufacture id string
  bit8u  prod_id[12]; // product id string
  bit32u oem_table_ptr; // pointer to an optional oem table
  bit16u oem_len;     // length of that table (if present)
  bit16u entry_count; // count of entries after this table
  bit32u lapic_addr;  // address of LAPIC (0xFEE0000 by default)
  bit16u ext_len;     // length of extended table
  bit8u  ext_crc;     // crc of extended table
  bit8u  resv;
};

struct S_MP_PROCESSOR {
  bit8u  type;      // 0
  bit8u  lapic_id;  // 
  bit8u  lapic_ver; // 
  bit8u  flags;     //
  bit32u cpu_sig;   // 
  bit32u features;  //
  bit32u resv[2];   //
};

struct S_MP_BUS {
  bit8u  type;      // 1
  bit8u  bus_id;    // 
  bit8u  id[6];     //
};

struct S_MP_IOAPIC {
  bit8u  type;      // 2
  bit8u  apic_id;   // 
  bit8u  apic_ver;  // 
  bit8u  flags;     //
  bit32u address;   //
};

struct S_MP_ASSIGN {
  bit8u  type;      // 3 & 4
  bit8u  int_type;  // 
  bit16u int_flag;  //
  bit8u  src_bus;   //
  bit8u  src_irq;   //
  bit8u  dest_id;   //
  bit8u  dest_int;  //
};

bit32u mp_find_sig(bit32u, const bit32u, const bit32u);
bit8u mp_crc_check(const bit32u, int);


#endif // FYSOS_MPS
