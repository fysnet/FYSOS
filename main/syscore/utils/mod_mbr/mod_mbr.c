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
 *  MOD_MBR.EXE
 *   This code is not meant to be a boot sector.  It is meant to be ran from
 *   the DOS prompt.  It will read the 1st sector of the 1st drive (0x80), or the
 *   drive specified on the command line, display the contents of the partition
 *   entries, then if an extended partition is found, load it and continue.
 *   
 *   It is intended to show you how to parse the partition entries, spanning
 *   extended partitions, and find the active partition so that you can boot
 *   that partition.  Once you have learned how to do that, you can then take
 *   this code and create your own bootable sector to do the same.
 *
 *  *** experimental code.  Not yet tested...
 *
 *  Assumptions/prerequisites:
 *   - You have at least a 80x386
 *   - You are running in a TRUE DOS environment, with a DPMI
 *   - You have complete access to all hardware
 *   - You have enough memory accessable to your program
 *
 *  Last updated: 20 July 2020
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os mod_mbr.c -o mod_mbr.exe -s
 *
 *  Usage:
 *    see parse_command() below
 */

#include <conio.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"
#include "..\include\mbr.h"
#include "mod_mbr.h"

int spt = 63;
int numheads = 16;

int main(int argc, char *argv[]) {
  struct COMMAND_LINE items;
  struct S_PART_ENTRY *entry;
  char filename[128];
  struct S_MBR mbr;
  FILE *fp;
  
  // print start string
  printf(strtstr);
  
  // we need to parse the command line and get the parameters found
  if (!parse_command(argc, argv, filename, &items))
    return -1;
  
  /*
  printf("     filename = [%s]\n"
         "        entry = %i\n"
         "   set active = %i\n"
         " set inactive = %i\n"
         "           id = 0x%02X (%i) - %i\n"
         "     base_lba = 0x%08X (%i) - %i\n"
         "         size = 0x%08X (%i) - %i\n",
         filename, 
         items.entry,
         items.set_active, items.set_inactive,
         items.id, items.id, items.id_given,
         items.base_lba, items.base_lba, items.base_given,
         items.size, items.size, items.size_given);
  return 0;
  */
  
  // try to open the image file
  if ((fp = fopen(filename, "r+b")) == NULL) {
    printf("\n Error opening '%s'...", filename);
    return -1;
  }
  
  // entry -> entry to modify
  // (simply to make the code easier to read)
  entry = &mbr.part_entry[items.entry];
  
  // read the mbr
  fread(&mbr, 1, 512, fp);
  
  if (items.set_active)
    entry->boot_id = 0x80;
  if (items.set_inactive)
    entry->boot_id = 0x00;
  if (items.id_given)
    entry->sys_id = items.id;
  if (items.base_given) {
    lba_to_chs(items.base_lba, &entry->start.cylinder, &entry->start.head, &entry->start.sector);
    entry->start_lba = items.base_lba;
  }
  if (items.size_given) {
    lba_to_chs(items.base_lba + items.size - 1, &entry->end.cylinder, &entry->end.head, &entry->end.sector);
    entry->size = items.size;
  }
  
  // back up and write it back
  rewind(fp);
  fwrite(&mbr, 1, 512, fp);
  fclose(fp);
  
  // we are done  
  return 0;
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

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the image file to modify (required)
 *  /0         - Modify the first entry (one of four required parameters)
 *  /1         - Modify the second entry (one of four required parameters)
 *  /2         - Modify the third entry (one of four required parameters)
 *  /3         - Modify the fourth entry (one of four required parameters)
 *  /A         - Set as active (if not given, not modified)
 *  /I         - Set as inactive (if not given, not modified)
 *  /ID:xx     - id to give partition
 *  /B:xxxxx   - Base LBA of partition (if not given, not modified)
 *  /S:xxxxx   - Size in sectors of partition (if not given, not modified)
 */
bool parse_command(int argc, char *argv[], char *filename, struct COMMAND_LINE *items) {
  int i;
  const char *s;
  bool ret = TRUE;
  
  strcpy(filename, "");
  items->entry = -1;
  items->set_active = FALSE;
  items->set_inactive = FALSE;
  items->id_given = FALSE;
  items->base_given = FALSE;
  items->size_given = FALSE;
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if (*s == '/') {
      s++;
      if (strcmp(s, "0") == 0)
        items->entry = 0;
      else if (strcmp(s, "1") == 0)
        items->entry = 1;
      else if (strcmp(s, "2") == 0)
        items->entry = 2;
      else if (strcmp(s, "3") == 0)
        items->entry = 3;
      else if ((strcmp(s, "A") == 0) ||
               (strcmp(s, "a") == 0))
        items->set_active = TRUE;
      else if ((strcmp(s, "I") == 0) ||
               (strcmp(s, "i") == 0))
        items->set_inactive = TRUE;
      else if ((memcmp(s, "ID:", 3) == 0) ||
               (memcmp(s, "id:", 3) == 0)) {
        items->id = strtol(s+3, NULL, 0);
        items->id_given = TRUE;
      }
      else if ((memcmp(s, "B:", 2) == 0) ||
               (memcmp(s, "b:", 2) == 0)) {
        items->base_lba = strtol(s+2, NULL, 0);
        items->base_given = TRUE;
      }
      else if ((memcmp(s, "S:", 2) == 0) ||
               (memcmp(s, "s:", 2) == 0)) {
        items->size = strtol(s+2, NULL, 0);
        items->size_given = TRUE;
      } else {
        printf(" Unknown switch parameter: /%s\n", s);
        ret = FALSE;
      }
    } else
      strcpy(filename, s);
  }
  
  if (items->entry == -1) {
    puts(" Must provide and entry number (0, 1, 2, or 3)");
    ret = FALSE;
  }
  if (items->set_active && items->set_inactive) {
    puts(" Set Active/Inactive set. Must provide one or the other.");
    ret = FALSE;
  }
  
  return ret;
}
