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
