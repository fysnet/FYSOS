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

// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Make GPT Part  v00.10.00    Forever Young Software 1984-2020\n";
char usage_str[] = "\n Usage:\n   mgptpart /s:source_file_name /t:target_file_name\n";


#define MAX_LINE_LEN 1024

// 1 for the header
// (512 bytes per sector / 128 bytes per entry) * max entries)
#define MAX_ENTRIES   128   // must be an even divisor of 512 (i.e.: must evenly divide into 512)
#define GPT_SIZE      (1 + ((512 / 128) * MAX_ENTRIES))

struct S_PARTITION {
  char   filename[128];
  bit32u base;
  bit32u size;
  struct S_PART_TYPE {
    bit8u  system;
    bit8u  hidden;
    bit8u  legacy;
  } type;
  bit8u  sys_id;
  bit8u  boot_id;
};

// ex: 3F2504E0-4F89-11D3-9A0C-0305E82C3301
//       data1    2    3    4     5[6]
struct S_GUID {
  bit32u data1;
  bit16u data2;
  bit16u data3;
  bit16u data4;
  bit8u  data5[6];
};
void calc_guid(struct S_GUID *, const bit32u);

// CRC32
#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
bit32u crc32(void *, bit32u);
void crc32_partial(bit32u *, void *, bit32u);
bit32u crc32_reflect(bit32u, char);

// EFI GUID Partition Table
// page 11-9 of EFISpec_v110.pdf
struct S_GPT_HDR {
  bit8u  sig[8];
  bit32u version;
  bit32u hdr_size;
  bit32u crc32;   // only bytes 0 -> hdr_size are checked
  bit32u resv0;
  bit64u primary_lba;
  bit64u backup_lba;
  bit64u first_usable;
  bit64u last_usable;
  struct S_GUID guid;
  bit64u entry_offset;
  bit32u entries;
  bit32u entry_size;
  bit32u crc32_entries;
  bit8u  reserved[420];
};

void create_name(bit16u *, const int);

struct S_GPT_ENTRY {
  struct S_GUID guid_type;
  struct S_GUID guid;
  bit64u first_lba;
  bit64u last_lba;
  bit64u attribute;
  bit16u name[36];  // if entry is 128.  Could be more, though we don't allow for it in the code
};

int  parse_command_line(const int argc, char *argv[]);
int  get_a_line(FILE *fp, char *line, int *linenum);
int  get_filename(char *src, char *targ);
int  get_value(char *src, void *targ, const int size);
void lba_to_chs(const bit32u lba, bit8u *cyl, bit8u *head, bit8u *sector);
void create_header(const bit32u, const bit32u, const bit32u, const int, const bit32u, const bit32u, FILE *);
void create_partition(struct S_PARTITION *, FILE *);
bit32u create_entry(const bit32u, const int, struct S_PARTITION *, const bit32u, bit16u *, FILE *);

int check_image(const char *);
