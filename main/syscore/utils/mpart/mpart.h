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
 */

// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Make Part  v00.10.00    Forever Young Software 1984-2015\n";
char usage_str[] = "\n Usage:\n   mpart /s:source_file_name /t:target_file_name\n";


#define MAX_LINE_LEN 1024

struct S_PARTITION {
  char   filename[128];
  bit32u base;
  bit32u size;
  bit8u  type;
  bool   active;
  bool   last;  
};


int  parse_command_line(const int argc, char *argv[]);
int  get_a_line(FILE *fp, char *line);
int  get_filename(char *src, char *targ);
int  get_value(char *src, void *targ, const int size);
void lba_to_chs(const bit32u lba, bit8u *cyl, bit8u *head, bit8u *sector);
void create_table(struct S_MBR *buffer, bit32u base, FILE *out);

