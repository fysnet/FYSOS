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
 *  Last updated: 20 July 2020
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
