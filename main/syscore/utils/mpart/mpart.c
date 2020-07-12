/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
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
 *    gcc -Os mpart.c -o mpart.exe -s  (DOS .EXE requiring DPMI)
 *  
 *  Usage:
 *    mpart /s:filename.txt /t:filename.bin
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

#include "..\include\ctype.h"
#include "..\include\mbr.h"
#include "mpart.h"

/* Buffers to hold filenames.
 * Be careful not to exceed 128 bytes.
 */
char src_filename[128] = "";
char targ_filename[128] = "outfile.img";
char mbr_filename[128] = "";
FILE *src, *targ;

// default values for CHS conversion
bit32u spt = 63, numheads = 16;

/* Buffer to hold a single line from source file.
 * Be careful not to exceed the given length limit.
 */
char line[MAX_LINE_LEN + 1];

/* Buffer to hold each partition from text file
 * Be careful not to exceed 64 entries
 */
struct S_PARTITION partitions[64];
int part_cnt = 0, cur_rec = 0;


int main(int argc, char *argv[]) {
  int linelen, linenum = 0, pos, errors = 0;
  
  // print start string
  printf(strtstr);
  
  // parse the command line
  if (parse_command_line(argc, argv) < 1) {
    printf(usage_str);
    return -1;
  }
  
  // try to open the source file
  if ((src = fopen(src_filename, "rb")) == NULL) {
    printf("\n Could not open source file: %s", src_filename);
    return -2;
  }
  
  // clear out the partition stuff
  memset(partitions, 0, sizeof(struct S_PARTITION) * 64);
  part_cnt = 0;
  
  // loop until no more lines in the file
  while (!feof(src)) {
    linelen = get_a_line(src, line);
    if (linelen > 0) {
      linenum++;
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
            printf("\nLine %i: unknown parameter for 'out': '%s'", linenum, &line[pos]);
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
          printf("\nLine %i: unknown parameter for 'mbr': '%s'", linenum, &line[pos]);
          errors++;
        }
      /*
       * PART: file="filename.bin", base=123456, size=54678, type=8, active=0, last=0
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
          } else if (memcmp(&line[pos], "size=", 5) == 0) {
            pos += 5;
            pos += get_value(&line[pos], &partitions[part_cnt].size, sizeof(bit32u));
          } else if (memcmp(&line[pos], "type=", 5) == 0) {
            pos += 5;
            pos += get_value(&line[pos], &partitions[part_cnt].type, sizeof(bit8u));
          } else if (memcmp(&line[pos], "active=", 7) == 0) {
            pos += 7;
            pos += get_value(&line[pos], &partitions[part_cnt].active, sizeof(bit8u));
          } else if (memcmp(&line[pos], "last=", 5) == 0) {
            pos += 5;
            pos += get_value(&line[pos], &partitions[part_cnt].last, sizeof(bit8u));
          } else {
            printf("\nLine %i: unknown parameter for 'part': '%s'", linenum, &line[pos]);
            errors++;
            break;
          }
        }
        part_cnt++;
      } else {
        printf("\nLine %i: unknown line content: '%s'", linenum, line);
        errors++;
      }
    }
  }
  
  // done with the file, so close it
  fclose(src);
  
  // if there was an error, print error message and abort
  if (errors > 0) {
    printf("\n Found %i errors.  Aborting...", errors);
    return -2;
  }
  
  // now that we have all of the information, we can now print it to make sure
  printf("\n Create image file: %s", targ_filename);
  if (strlen(mbr_filename))
    printf("\n Using mbr image file: %s", mbr_filename);
  else  
    printf("\n Writing nulls to mbr code area.");
  printf("\n Writing %i (ext)partitions:", part_cnt);
  
  // print the partitions
  for (pos=0; pos<part_cnt; pos++) {
    printf("\n  %i: base = %i, size = %i, type = %i, active = %i, last = %i", pos,
      partitions[pos].base, partitions[pos].size, partitions[pos].type, partitions[pos].active, partitions[pos].last);
    if (strlen(partitions[pos].filename))
      printf("\n       Writing image file: %s", partitions[pos].filename);
  }
  
  // now we can start to write the partitions
  struct S_MBR mbr;
  memset(&mbr, 0, sizeof(struct S_MBR));
  
  // if a mbr image was given, try to load it into mbr
  if (strlen(mbr_filename)) {
    FILE *fp;
  if ((fp = fopen(mbr_filename, "rb")) == NULL) {
      printf("\n Could not open mbr file: %s", mbr_filename);
    } else {
      fread(&mbr, 512, 1, fp);
      fclose(fp);
    }
  }
  
  // create the out file
  if ((targ = fopen(targ_filename, "w+b")) == NULL) {
    printf("\n Could not create target file: %s", targ_filename);
    return -3;
  }
  
  // create the partition tables
  create_table(&mbr, 0, targ);
  
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
    else
      printf("\n Unknown paramter given on command line: %s", argv[i]);
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

int get_a_line(FILE *fp, char *line) {
  
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
      break;
    }
    // end of line
    if (ch == 13)
      break;
    
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
 * All digits should be from 0-9 (i.e.: Decimal only)
 * This will skip the comma after the last " if it exists.
 */
int get_value(char *src, void *targ, const int size) {
  
  int i=0;
  bit32u val = 0;
  
  while (*src && isdigit(*src)) {
    val *= 10;
    val += (*src - '0');
    i++;
    src++;
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

/*
 * This creates a partition table at current "base".
 * This will write the sector at base when it encouters "last".
 * This will watch for Extended Partitions and recurse when found.
 */
void create_table(struct S_MBR *mbr, bit32u base, FILE *out) {
  struct S_MBR *next;
  int cur_entry = 0;
  unsigned i;
  bool last;
  FILE *fp;
  
  while (1) {
    mbr->part_entry[cur_entry].boot_id = (partitions[cur_rec].active) ? 0x80 : 0x00;
    lba_to_chs(partitions[cur_rec].base, 
      &mbr->part_entry[cur_entry].start.cylinder,
      &mbr->part_entry[cur_entry].start.head,
      &mbr->part_entry[cur_entry].start.sector);
    mbr->part_entry[cur_entry].sys_id = partitions[cur_rec].type;
    lba_to_chs(partitions[cur_rec].base + partitions[cur_rec].size - 1,
      &mbr->part_entry[cur_entry].end.cylinder,
      &mbr->part_entry[cur_entry].end.head,
      &mbr->part_entry[cur_entry].end.sector);
    // Remember that the 'base' value from the text file is zero based from start
    //  of disk.  We need to make it zero based from MBR or last ext partition entry.
    mbr->part_entry[cur_entry].start_lba = partitions[cur_rec].base - base;
    mbr->part_entry[cur_entry].size = partitions[cur_rec].size;
    
    // we need to make the 'last' flag be on the stack, not a static variable
    last = partitions[cur_rec].last;
    
    // if this partition record was the last one (in that partition), write it to 'base'
    if (last) {
      mbr->sig = 0xAA55;
      fseek(out, base * 512, SEEK_SET);
      fwrite(mbr, 512, 1, out);
    }
    
    // type 5 and 15 are extended partition types
    if ((partitions[cur_rec].type == 5) || (partitions[cur_rec].type == 15)) {
      next = (struct S_MBR *) malloc(sizeof(struct S_MBR));
      memset(next, 0, sizeof(struct S_MBR));
      create_table(next, partitions[cur_rec++].base, out);
      free(next);
    } else {
      // if a filename was given for this partition, write it, else write blank sectors 
      bit8u *buffer = (bit8u *) malloc(512);
      fseek(out, partitions[cur_rec].base * 512, SEEK_SET);
      fp = NULL;
      if (strlen(partitions[cur_rec].filename)) {
        fp = fopen(partitions[cur_rec].filename, "rb");
        if (fp == NULL)
          printf("\n Could not open partition file: %s", partitions[cur_rec].filename);
      }
      if (strlen(partitions[cur_rec].filename) && fp) {
        for (i=0; i<partitions[cur_rec].size; i++) {
          memset(buffer, 0, 512);  // this way the padding is always zero's
          fread(buffer, 512, 1, fp);
          fwrite(buffer, 512, 1, out);
        }
        fclose(fp);
      } else {
        memset(buffer, 0, 512);
        for (i=0; i<partitions[cur_rec].size; i++)
          fwrite(buffer, 512, 1, out);
      }
      free(buffer);
    }
    
    if (last)
      break;
    
    // move to next partition record
    cur_rec++;
    if (cur_rec >= part_cnt)
      break;
    
    // move to next partition entry in this sector
    cur_entry++;
    if (cur_entry > 3) {
      printf("\n Error: Can only have 4 entries per mbr");
      break;
    }
  }
}

