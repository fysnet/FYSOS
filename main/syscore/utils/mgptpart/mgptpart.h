/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
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

// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Make GPT Part  v00.10.00    Forever Young Software 1984-2016\n";
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

