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
 *
 *  
 *  To compile using DJGPP:  (http://www.delorie.com/djgpp/)
 *    gcc -Os mgptpart.c -o mgptpart.exe -s  (DOS .EXE requiring DPMI)
 *  
 *  Usage:
 *    mgptpart /s:filename.txt /t:filename.bin
 *  
 *  Where filename.txt is an input text file with a list of partitions.
 *   See samples.txt for a few samples and instructions
 *  Where filename.bin (optional) is the image file name to create  
 *  
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "..\include\ctype.h"
#include "..\include\mbr.h"
#include "mgptpart.h"

/* Buffers to hold filenames.
 * Be careful not to exceed 128 bytes.
 */
char src_filename[128] = "";
char targ_filename[128] = "outfile.img";
char mbr_filename[128] = "";
bit16u wname[36];
FILE *src, *targ;
bool use_legacy_mbr = TRUE;  // if true, use Legacy MBR, else use Protected MBR

// if the /c parameter is used, this code will then take the 'src_filename' file as
//  an existing GPT image file and check it for correctness.
bool checking = FALSE;

// default values for CHS conversion
bit32u spt = 63, numheads = 16, last_lba = 0, crc_entries;

/* Buffer to hold a single line from source file.
 * Be careful not to exceed the given length limit.
 */
char line[MAX_LINE_LEN + 1];

/* Buffer to hold each partition from text file
 * Be careful not to exceed 128 entries
 */
struct S_PARTITION partitions[MAX_ENTRIES];
int part_cnt = 0;


int main(int argc, char *argv[]) {
  int linelen, linenum = 0, pos, errors = 0;
  char tempstr[128];
  
  // print start string
  printf(strtstr);
  
  // parse the command line
  if (parse_command_line(argc, argv) < 1) {
    printf(usage_str);
    return -1;
  }
  
  // initialize the crc32 stuff
  crc32_initialize();
  srand(time(NULL));
  
  // if /c given on command line, we are checking an existing image
  if (checking)
    return check_image(src_filename);
  
  // try to open the source file
  if ((src = fopen(src_filename, "rb")) == NULL) {
    printf(" Could not open source file: %s\n", src_filename);
    return -2;
  }
  
  // clear out the partition stuff
  memset(partitions, 0, sizeof(struct S_PARTITION) * MAX_ENTRIES);
  part_cnt = 0;
  
  // loop until no more lines in the file
  while (!feof(src) && (part_cnt < MAX_ENTRIES)) {
    linelen = get_a_line(src, line, &linenum);
    if (linelen > 0) {
      pos = 0;
      /*
       * OUT: file="filename.bin"
       */
      if (memcmp(line, "out:", 4) == 0) {
        pos += 4;
        while (pos < linelen) {
          if (memcmp(&line[pos], "file=\"", 6) == 0) {
            pos += 6;
            pos += get_filename(&line[pos], targ_filename);
          } else if (memcmp(&line[pos], "spt=", 4) == 0) {
            pos += 4;
            pos += get_value(&line[pos], &spt, sizeof(bit32u));
          } else if (memcmp(&line[pos], "heads=", 6) == 0) {
            pos += 6;
            pos += get_value(&line[pos], &numheads, sizeof(bit32u));
          } else {
            printf("Line %i: unknown parameter for 'out': '%s'\n", linenum, &line[pos]);
            errors++;
          }
        }
      /*
       * MBR: file="filename.bin"
       */
      } else if (memcmp(line, "mbr:", 4) == 0) {
        pos += 4;
        if (memcmp(&line[pos], "file=\"", 6) == 0) {
          pos += 6;
          pos += get_filename(&line[pos], mbr_filename);
        } else {
          printf("Line %i: unknown parameter for 'mbr': '%s'\n", linenum, &line[pos]);
          errors++;
        }
      /*
       * TYPE: Legacy or EFI
       */
      } else if (memcmp(line, "type:", 5) == 0) {
        pos += 5;
        if (memcmp(&line[pos], "legacy", 6) == 0) {
          pos += 6;
          use_legacy_mbr = TRUE;
        } else if (memcmp(&line[pos], "efi", 3) == 0) {
          pos += 3;
          use_legacy_mbr = FALSE;
        } else {
          printf("Line %i: unknown parameter for 'type': '%s'\n", linenum, &line[pos]);
          errors++;
        }
      /*
       * PART: file="filename.bin", base=123456, size=54678, type="system", type="hidden", type="legacy", sys_id=4, boot_id=0x80
       */
      } else if (memcmp(line, "part:", 5) == 0) {
        pos += 5;
        while (pos < linelen) {
          if (memcmp(&line[pos], "file=\"", 6) == 0) {
            pos += 6;
            pos += get_filename(&line[pos], partitions[part_cnt].filename);
          } else if (memcmp(&line[pos], "base=", 5) == 0) {
            pos += 5;
            pos += get_value(&line[pos], &partitions[part_cnt].base, sizeof(bit32u));
            if (partitions[part_cnt].base < (GPT_SIZE + 1)) {
              printf("Base must be past partition entries.  Min base = %i\n", (GPT_SIZE + 1));
              errors++;
            }
          } else if (memcmp(&line[pos], "size=", 5) == 0) {
            pos += 5;
            pos += get_value(&line[pos], &partitions[part_cnt].size, sizeof(bit32u));
          } else if (memcmp(&line[pos], "type=\"", 6) == 0) {
            pos += 6;
            pos += get_filename(&line[pos], tempstr);
            if (strcmp(tempstr, "system") == 0)
              partitions[part_cnt].type.system = TRUE;
            else if (strcmp(tempstr, "hidden") == 0)
              partitions[part_cnt].type.hidden = TRUE;
            else if (strcmp(tempstr, "legacy") == 0)
              partitions[part_cnt].type.legacy = TRUE;
            else {
              printf("Unknown partition type: %s\n", tempstr);
              errors++;
            }
          } else if (memcmp(&line[pos], "sys_id=", 7) == 0) {
            pos += 7;
            pos += get_value(&line[pos], &partitions[part_cnt].sys_id, sizeof(bit8u));
          } else if (memcmp(&line[pos], "boot_id=", 8) == 0) {
            pos += 8;
            pos += get_value(&line[pos], &partitions[part_cnt].boot_id, sizeof(bit8u));
          } else {
            printf("Line %i: unknown parameter for 'part': '%s'\n", linenum, &line[pos]);
            errors++;
            break;
          }
        }
        part_cnt++;
      } else {
        printf("Line %i: unknown line content: '%s'\n", linenum, line);
        errors++;
      }
    }
  }
  
  // done with the file, so close it
  fclose(src);
  
  // if there was an error, print error message and abort
  if (errors > 0) {
    printf(" Found %i errors.  Aborting...\n", errors);
    return -2;
  }
  
  // now that we have all of the information, we can now print it to make sure
  printf("    Create image file: %s\n", targ_filename);
  if (strlen(mbr_filename))
    printf(" Using mbr image file: %s\n", mbr_filename);
  else  
    puts(" Writing nulls to mbr code area.");
  printf(" Writing %i partitions:\n", part_cnt);
  
  // create the out file
  if ((targ = fopen(targ_filename, "w+b")) == NULL) {
    printf(" Could not create target file: %s\n", targ_filename);
    return -3;
  }
  
  // write the partitions
  for (pos=0; pos<part_cnt; pos++) {
    printf("  %i: base = %i, size = %i, system = %i, hidden = %i, legacy = %i\n", pos,
      partitions[pos].base, partitions[pos].size, partitions[pos].type.system, partitions[pos].type.hidden, partitions[pos].type.legacy);
    if (strlen(partitions[pos].filename))
      printf("       Writing image file: %s\n", partitions[pos].filename);
    create_partition(&partitions[pos], targ);
    if (((partitions[pos].base + partitions[pos].size) - 1) > last_lba)
      last_lba = ((partitions[pos].base + partitions[pos].size) - 1);
  }
  
  // we need to add enough sectors to the out file to hold the backup header and entries
  last_lba += GPT_SIZE;
  
  // now we can start to write the partition entries
  struct S_MBR mbr;
  memset(&mbr, 0, sizeof(struct S_MBR));
  
  // if a mbr image was given, try to load it into mbr
  if (strlen(mbr_filename)) {
    FILE *fp;
    if ((fp = fopen(mbr_filename, "rb")) == NULL) {
      printf(" Could not open mbr file: %s\n", mbr_filename);
    } else {
      fread(&mbr, 1, 512, fp);
      fclose(fp);
    }
  }
  
  // come back to the first of the file
  rewind(targ);
  
  // modify and write the MBR
  if (use_legacy_mbr) {
    for (pos=0; pos<part_cnt && pos<4; pos++) {
      mbr.part_entry[pos].boot_id = partitions[pos].boot_id; // boot ID can be 0x80 for bootable
      mbr.part_entry[pos].sys_id = partitions[pos].sys_id;   // ex: 4 = FAT16 < 32meg
      mbr.part_entry[pos].start_lba = partitions[pos].base;  // start LBA of actual partition within GPT
      mbr.part_entry[pos].size = partitions[pos].size;       // size of partition in sectors
      lba_to_chs(mbr.part_entry[pos].start_lba, &mbr.part_entry[pos].start.cylinder, &mbr.part_entry[pos].start.head, &mbr.part_entry[pos].start.sector);
      lba_to_chs(mbr.part_entry[pos].start_lba + mbr.part_entry[pos].size - 1, &mbr.part_entry[pos].end.cylinder, &mbr.part_entry[pos].end.head, &mbr.part_entry[pos].end.sector);
    }
  } else {
    mbr.part_entry[0].boot_id = 0x00;      // boot ID must be 0x00;
    mbr.part_entry[0].sys_id = 0xEE;       // sys ID is 0xEE
    mbr.part_entry[0].start_lba = 1;       // start LBA is the GPT Header (LBA 1)
    mbr.part_entry[0].size = last_lba + 1 - 1;  // + 1 to get 1 based, - 1 to not count MBR
    mbr.part_entry[0].start.cylinder = 0;  // the start CHS must be 0,0,1 even though we have LBA 2
    mbr.part_entry[0].start.head = 0;      //
    mbr.part_entry[0].start.sector = 1;    //
    mbr.part_entry[0].end.cylinder = 0xFF; // the end CHS must be 0xFF, 0xFE, 0xFF no matter the ending LBA
    mbr.part_entry[0].end.head = 0xFE;     //
    mbr.part_entry[0].end.sector = 0xFF;   //
    pos = 1;
  }
  
  // clear out remaining partition entries (if any)
  for (; pos<4; pos++)
    memset(&mbr.part_entry[pos], 0, 16);
  
  mbr.sig = 0xAA55;
  fwrite(&mbr, 1, 512, targ);
  
  // create the partition entries
  crc_entries = 0xFFFFFFFF;
  for (pos=0; pos<MAX_ENTRIES; pos++) {
    create_name(wname, pos);
    if (pos < part_cnt) {
      crc_entries = create_entry(2, pos, &partitions[pos], crc_entries, wname, targ);
      create_entry(last_lba - GPT_SIZE, pos, &partitions[pos], 0, wname, targ);
    } else {
      create_entry(2, pos, NULL, 0, wname, targ);
      create_entry(last_lba - GPT_SIZE, pos, NULL, 0, wname, targ);
    }
  }
  crc_entries ^= 0xFFFFFFFF;
  
  // create partition header
  create_header(1, last_lba, 2, part_cnt, last_lba + 1, crc_entries, targ);
  create_header(last_lba, 1, last_lba - GPT_SIZE, part_cnt, last_lba + 1, crc_entries, targ);
  
  // close the image file
  fclose(targ);  
}

/*
 * Parse command line.
 *  Looking for:
 *   /s:source_filename.txt
 *   /t:target_image_filename.img
 *
 */
int parse_command_line(const int argc, char *argv[]) {
  
  int i;
  
  for (i=1; i<argc; i++) {
    if (memcmp(argv[i], "/s:", 3) == 0)
      strncpy(src_filename, &argv[i][3], 128);
    else if (memcmp(argv[i], "/t:", 3) == 0)
      strncpy(targ_filename, &argv[i][3], 128);
    else if (strcmp(argv[i], "/c") == 0)
      checking = TRUE;
    else
      printf(" Unknown paramter given on command line: %s\n", argv[i]);
  }
  
  return argc;
}


/*
 * Get a line of text from the stream in fp.
 *  This gets a line of text, skipping white_chars[] chars and stopping at the
 *   first CR.  It does not include the CR in the target string.
 *  This will also lower case all chars unless it is within a set of ""s
 */
char white_chars[] = { 0x20, 0x09, 0x0A };

int get_a_line(FILE *fp, char *line, int *linenum) {
  
  char ch;
  int i = 0;
  bool inquote = FALSE;
  
  // first, clear out the buffer
  memset(line, 0, MAX_LINE_LEN + 1);
  
  // get the whole line, or until a comment is reached
  while ((ch = fgetc(fp)) > 0) {
    // ignore all characters in our white_chars string
    if (strchr(white_chars, ch))
      continue;
    // if a comment is reached, or the line is longer 
    //  than 1024 chars, truncate line here
    if ((ch == '#') || (i == MAX_LINE_LEN)) {
      while (!feof(fp) && (fgetc(fp) != 13))
        ;
      (*linenum)++;
      break;
    }
    // end of line
    if (ch == 13) {
      (*linenum)++;
      break;
    }
    
    // toggle the inquote flag if " found
    if (ch == 34)
      inquote ^= 1;
    
    // lower case the char unless we are within a quote
    if (!inquote)
      ch = tolower(ch);
    
    // else, add to the string
    line[i++] = ch;
  }
  
  // make sure it is asciiz'd
  line[i] = 0;
  
  return i;
}

/*
 * This returns the filename within the ""s
 * The src should point to the first part of the string.
 * This will skip the comma after the last " if it exists.
 */
int get_filename(char *src, char *targ) {
  int i=0;
  
  while (*src && (*src != 34)) {
    *targ++ = *src++;
    i++;
  }
  
  // make the target asciiz
  *targ = 0;
  
  // skip past the double quote
  if (*src == 34) {
    src++;
    i++;
  }
  
  // skip past the comma
  if (*src == 44) {
    src++;
    i++;
  }
  
  // return the amount to add to the position
  return i;
}

/*
 * This returns the a value from the digits at src
 * The src should point to the first part of the value.
 * All digits should be from 0-9, A-F (i.e.: Decimal and Hex only)
 * This will skip the comma after the last " if it exists.
 */
int get_value(char *src, void *targ, const int size) {
  int i=0;
  bit32u val = 0;
  
  // is it hex
  if ((src[0] == '0') && (tolower(src[1]) == 'x')) {
    src += 2;
    i += 2;
    while (*src && isxdigit(*src)) {
      val *= 16;
      if (isdigit(*src))
        val += (*src - '0');
      else
        val += (toupper(*src) - 'A');
      i++;
      src++;
    }
  } else {  
    while (*src && isdigit(*src)) {
      val *= 10;
      val += (*src - '0');
      i++;
      src++;
    }
  }
  
  switch (size) {
    case 4:
      * (bit32u *) targ = (bit32u) val;
      break;
    case 2:
      * (bit16u *) targ = (bit16u) val;
      break;
    case 1:
      * (bit8u *) targ = (bit8u) val;
      break;
  }
  
  // skip past the comma
  if (*src == 44) {
    src++;
    i++;
  }
  
  return i;
}

/*
 * Convert LBA to CHS.  If LBA >= (1024 * spt * heads) use max values */
#define SECTOR ((lba % spt) + 1)
#define HEAD   ((lba / spt) % numheads)
#define CYL    ((lba / spt) / numheads)
/* Converts LBA to the BIOS form of CHS with the high 2 bits of the cylinder
 *  in the high 2 bits of the sector field
 */
void lba_to_chs(const bit32u lba, bit8u *cyl, bit8u *head, bit8u *sector) {
  if (lba < (1024 * spt * numheads)) {
    *head = (bit8u) HEAD;
    *sector = (bit8u) (((CYL & 0x300) >> 2) | (SECTOR & 0x3F));
    *cyl = (bit8u) (CYL & 0x0FF);
  } else {
    *head = 0xFE;
    *sector = 0xFF;
    *cyl = 0xFF;
  }
}

// Sometimes the data4 member is set to the machines MAC address.
//  However, this allows a user to track a guid to the machine that
//  made the guid.
void calc_guid(struct S_GUID *guid, const bit32u dword) {
  time_t timestamp = time(NULL);
  
  guid->data1 = (bit32u) timestamp;
  guid->data2 = (bit16u) rand();
  guid->data3 = (bit16u) rand();
  guid->data4 = (bit16u) rand();
  *((bit32u *) &guid->data5[0]) = dword;
  *((bit16u *) &guid->data5[4]) = (bit16u) rand();
}

// CRC32
bit32u crc32_table[256]; // CRC lookup table array.

void crc32_initialize(void) {
  int i, j;
  
  memset(crc32_table, 0, sizeof(crc32_table));
  
  // 256 values representing ASCII character codes.
  for (i=0; i<=0xFF; i++) {
    crc32_table[i] = crc32_reflect(i, 8) << 24;
    
    for (j=0; j<8; j++)
      crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
    
    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
}

// Reflection is a requirement for the official CRC-32 standard.
//  You can create CRCs without it, but they won't conform to the standard.
bit32u crc32_reflect(bit32u reflect, char ch) {
  bit32u ret = 0;
  int i;
  
  // Swap bit 0 for bit 7 bit 1 For bit 6, etc....
  for (i=1; i<(ch + 1); i++) {
    if (reflect & 1)
      ret |= 1 << (ch - i);
    reflect >>= 1;
  }
  
  return ret;
}

bit32u crc32(void *data, bit32u len) {
  bit32u crc = 0xFFFFFFFF;
  crc32_partial(&crc, data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(bit32u *crc, void *ptr, bit32u len) {
  bit8u *data = (bit8u *) ptr;
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}

/*
 * This creates a gpt header at current "base"
 */
void create_header(const bit32u base, const bit32u backup, const bit32u entry_offset, const int count, 
                   const bit32u size, const bit32u crc_entries, FILE *out) {
  struct S_GPT_HDR hdr;
  
  memset(&hdr, 0, sizeof(struct S_GPT_HDR));
  memcpy(hdr.sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8);
  hdr.version = 0x00010000;
  hdr.hdr_size = 92;
  hdr.primary_lba = base;
  hdr.backup_lba = backup;
  hdr.first_usable = GPT_SIZE;
  hdr.last_usable = size - GPT_SIZE;
  calc_guid(&hdr.guid, rand());
  hdr.entry_offset = entry_offset;
  hdr.entries = count;
  hdr.entry_size = 128;
  hdr.crc32_entries = crc_entries;
  hdr.crc32 = crc32(&hdr, 92);
  
  printf(" Writing GPT Header to LBA %i\n", base);
  fseek(out, (base * 512), SEEK_SET);
  fwrite(&hdr, 1, 512, out);
}

// first word in wname is the length of the string
void create_name(bit16u *wname, const int pos) {
  char name[35];
  int i;
  
  memset(name, 0, 35);
  memset(wname, 0, 36 * sizeof(bit16u));
  sprintf(name, "partition #%i", pos);
  wname[0] = strlen(name);
  for (i=0; i<35; i++)
    wname[i+1] = name[i];
}

/*
 * This creates a partition entry at current "base[pos]"
 */
bit32u create_entry(const bit32u base, const int pos, struct S_PARTITION *part, const bit32u crc_entries, bit16u *wname, FILE *out) {
  struct S_GPT_ENTRY entry;
  bit32u crc = crc_entries;
  
  memset(&entry, 0, sizeof(struct S_GPT_ENTRY));
  if (part != NULL) {
    if (part->type.system)
                               // C12A7328-F81F-11D2-4BBA-00A0C93EC93B
                               // little-endian   little-endian  little-endian  little-endian     single bytes (a string)
      memcpy(&entry.guid_type, "\x28\x73\x2A\xC1"   "\x1F\xF8"     "\xD2\x11"    "\xBA\x4B"     "\x00\xA0\xC9\x3E\xC9\x3B", 16);
    else
      calc_guid(&entry.guid_type, rand());
    entry.attribute = (((part->type.system) ? (1 << 0) : 0) |
                       ((part->type.hidden) ? (1 << 1) : 0) |
                       ((part->type.legacy) ? (1 << 2) : 0));
    calc_guid(&entry.guid, rand());
    entry.first_lba = part->base;
    entry.last_lba = part->base + part->size - 1;
    memcpy(entry.name, &wname[1], wname[0] * sizeof(bit16u));
  }
  crc32_partial(&crc, &entry, sizeof(struct S_GPT_ENTRY));
  
  //printf(" Writing GPT Entry to LBA %i offset %i\n", (base + ((pos * 128) / 512)), (pos % 4) * 128);
  fseek(out, (base * 512) + (pos * 128), SEEK_SET);
  fwrite(&entry, 1, 128, out);
  
  return crc;
}

/*
 * This creates a partition at current "base"
 */
void create_partition(struct S_PARTITION *part, FILE *out) {
  bit8u buffer[512];
  unsigned int i;
  FILE *fp;
  
  // if a filename was given for this partition, write it, else write blank sectors 
  fseek(out, part->base * 512, SEEK_SET);
  fp = NULL;
  if (strlen(part->filename)) {
    fp = fopen(part->filename, "rb");
    if (fp == NULL)
      printf(" Could not open partition file: %s\n", part->filename);
  }
  if (strlen(part->filename) && fp) {
    for (i=0; i<part->size; i++) {
      memset(buffer, 0, 512);  // this way the padding is always zero's
      fread(buffer, 1, 512, fp);
      fwrite(buffer, 1, 512, out);
    }
    fclose(fp);
  } else {
    memset(buffer, 0, 512);
    for (i=0; i<part->size; i++)
      fwrite(buffer, 1, 512, out);
  }
}

// ex: 3F2504E0-4F89-11D3-9A0C-0305E82C3301
//       data1    2    3    4     5[6]
void print_guid(struct S_GUID *guid) {
  printf("%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X\n",
    guid->data1,
    guid->data2,
    guid->data3,
    guid->data4,
    guid->data5[0],
    guid->data5[1],
    guid->data5[2],
    guid->data5[3],
    guid->data5[4],
    guid->data5[5]);
}

// check an existing image for correctness.

int check_image(const char *filename) {
  int i, j;
  bit32u dword0, dword1, last_lba, last_usable;
  bit8u  buffer[512];
  struct S_MBR mbr;
  struct S_GPT_HDR hdr;
  struct S_GPT_ENTRY entry[128];
  
  printf("\nChecking '%s'...\n\n", filename);
  
  // try to open the source file
  if ((src = fopen(filename, "rb")) == NULL) {
    printf(" Could not open source image file: %s\n", src_filename);
    return -2;
  }
  
  // get length of file (last LBA number)
  fseek(src, 0, SEEK_END);
  last_lba = (ftell(src) / 512) - 1;
  rewind(src);
  
  // read in the first sector as the MBR
  puts("Reading in the MBR:");
  fread(&mbr, 1, 512, src);
  for (i=0; i<4; i++)
    printf("  Partition Entry #%i:\n"
           "      Boot ID: %02X\n"
           "    System ID: %02X\n"
           "    Start LBA: %i\n"
           "         Size: %i\n", 
           i,
           mbr.part_entry[i].boot_id,
           mbr.part_entry[i].sys_id,
           mbr.part_entry[i].start_lba,
           mbr.part_entry[i].size);
  
  // read in the header
  puts("\nReading in the GPT Header:");
  fread(&hdr, 1, 512, src);
  last_usable = (last_lba - 1 - (((hdr.entries * 128) + 511) / 512));
  
  memcpy(buffer, hdr.sig, 8);
  buffer[8] = 0;
  printf("        Signature: '%s'", buffer);
  if (strcmp(buffer, "EFI PART"))
    puts(" <-- Should be 'EFI PART'.");
  else
    puts("");
  
  printf("          Version: 0x%08X", hdr.version);
  if (hdr.version != 0x00010000)
    puts(" <-- Should be 0x00010000.");
  else
    puts("");
  
  printf("      Header size: %i", hdr.hdr_size);
  if (hdr.hdr_size != 92)
    puts(" <-- Should be 92.");
  else
    puts("");
  
  printf("       Header CRC: 0x%08X", hdr.crc32);
  dword0 = hdr.crc32;
  hdr.crc32 = 0;
  dword1 = crc32(&hdr, 92);
  hdr.crc32 = dword0;
  if (hdr.crc32 != dword1)
    printf(" <-- Should be 0x%08X.\n", dword1);
  else
    puts("");
  
  printf("         Reserved: 0x%08X", hdr.resv0);
  if (hdr.resv0 != 0)
    puts(" <-- Should be 0x00000000.");
  else
    puts("");
  
  printf("      Primary LBA: %i", hdr.primary_lba);
  if (hdr.primary_lba != 1)
    puts(" <-- Should be 1.");
  else
    puts("");
  
  printf("       Backup LBA: %i", hdr.backup_lba);
  if (hdr.backup_lba != last_lba)
    printf(" <-- Should be %i.\n", last_lba);
  else
    puts("");
  
  printf(" First Usable LBA: %i\n", hdr.first_usable);
  printf("  Last Usable LBA: %i", hdr.last_usable);
  if (hdr.last_usable > last_usable)
    printf(" <-- Should be %i.\n", last_usable);
  else
    printf(" (Calculated %i)\n", last_usable);
  
  printf("               GUID: "); print_guid(&hdr.guid);
  
  printf("        Entry LBA: %i", hdr.entry_offset);
  if (hdr.entry_offset != 2)
    puts(" <-- Should be 2.");
  else
    puts("");
  
  printf("Number of entries: %i", hdr.entries);
  if ((hdr.entries < 1) || (hdr.entries > 128)) {
    puts(" <-- Should be 0 < entries < 129.");
    if (hdr.entries > 128)  // catch for below
      hdr.entries = 128;
  } else
    puts("");
  
  printf("       Entry Size: %i", hdr.entry_size);
  if (hdr.entry_size != 128)
    puts(" <-- Should be 128.");
  else
    puts("");
  
  printf("      Entries CRC: 0x%08X\n", hdr.crc32_entries);
  
  // read in the entries
  if (hdr.entries < 1)
    return -1;
  
  puts("\nReading in the GPT Entries:");
  fread(entry, hdr.entries, 128, src);
  
  dword0 = crc32(entry, hdr.entries * 128);
  if (hdr.crc32_entries != dword0)
    printf(" Header Entries CRC32 value should be: 0x%08X.\n", dword0);
  
  for (i=0; i<hdr.entries; i++) {
    printf("     Entry #%i\n", i);
    
    printf("          GUID Type: "); print_guid(&entry[i].guid_type);
    printf("               GUID: "); print_guid(&entry[i].guid);
    
    printf("       Starting LBA: %lli\n", entry[i].first_lba);
    printf("         Ending LBA: %lli\n", entry[i].last_lba);

    printf("          Attribute: %lli\n", entry[i].attribute);
    
    printf("               Name: '");
    for (j=0; j<36; j++) {
      if (!isprint(entry[i].name[j]))
        break;
      putchar(entry[i].name[j]);
    }
    puts("'");
  }
  
  fclose(src);
  
  return 0;
}
