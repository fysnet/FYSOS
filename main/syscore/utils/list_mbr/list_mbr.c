/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
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
 * File Version: 1.00.00
 * Last mod: 04 Feb 2012
 *
 * This code is not meant to be a boot sector.  It is meant to be ran from
 * the DOS prompt.  It will read the 1st sector of the 1st drive (0x80), or the
 * drive specified on the command line, display the contents of the partition
 * entries, then if an extended partition is found, load it and continue.
 * 
 * It is intended to show you how to parse the partition entries, spanning
 * extended partitions, and find the active partition so that you can boot
 * that partition.  Once you have learned how to do that, you can then take
 * this code and create your own bootable sector to do the same.
 *
 * This code assumes the following:
 * - You have at least a 80x386
 * - You are running in a TRUE DOS environment, with a DPMI
 * - You have complete access to all hardware
 * - You have enough memory accessable to your program
 *
 */

#include <conio.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\ctype.h"
#include "..\include\mbr.h"
#include "list_mbr.h"

#include <dpmi.h>
#include <go32.h>


int main(int argc, char *argv[]) {
  
  unsigned drv = 0x80;
  int i;
  
  // print start string
  printf(strtstr);
  
  // get BIOS drive value from the command line
  //  valid parameters:
  //   /d:0xDD where DD is a hex value from 80 to FF.
  for (i=1; i<argc; i++) {
    if (memcmp(argv[i], "/d:0x", 5) == 0) {
      sscanf(&argv[i][3], "%x", &drv);
      break;
    }
  }
  
  // check to see if drv is a valid number
  // should be at least 0x80 and no more than 0xFF
  if ((drv < 0x80) || (drv > 0xFF)) {
    printf("\n Invalid value for drive:  0x%X", drv);
    return -1;
  }
  
  // now check to make sure that the disk supports the Extended Read function
  if (!check_big_disk(drv)) {
    printf("\n The disk does not support the Ext Read function, or no disk found.");
    return -2;
  }
  
  // now call the routine to display the MBR entries
  // we actually call it here so that we can call it again using recursion when
  //  we find an extended partition entry
  printf("\n \xDA Drive: 0x%02X", drv);
  display_part_entrys(0, 0, drv);
  
  // an empty line to space the next command line
  printf("\n");
  
  // we are done  
  return 0;
}

/* This function checks the BIOS to see if it supports the Extended BIOS Read function
 * on entry:
 *  drv = 0x80 -> 0xFF.  BIOS drive number of disk
 * on exit:
 *  returns TRUE if Extended Read is supported
 */
bool check_big_disk(const bit8u drv) {
  
  __dpmi_regs regs;
  
  // call the Disk BIOS to check to see if the extended read function is supported
  regs.h.dl = drv;
  regs.h.ah = 0x41;
  regs.x.bx = 0x55AA;
  __dpmi_int(0x13, &regs);
  
  // if the carry flag is set, function is not allowed, therefore
  //  the extended read function is not allowed
  if (regs.x.flags & 0x01)
    return FALSE;
  
  // the extended read function (0x42) is allowed if bx = 0xAA55 and bit 0 of CX is set.
  return ((regs.x.bx == 0xAA55) && (regs.x.cx & 1));
}

/* Use the BIOS Disk Extended Read Service to read a single sector
 * on entry:
 *  drv = 0x80 -> 0xFF.  BIOS drive number of disk
 *  lba = 64-bit lba to read
 *  ptr -> 512 byte buffer to read it to
 * on exit:
 *  returns TRUE if Extended Read was successful
 */
bool read_sector(const bit8u drv, const bit64u lba, void *ptr) {
  
  struct S_DISK_PACKET disk_packet;
  __dpmi_regs regs;
  
  // allocate DOS Conventional memory (below 1meg)
  int sel;
  bit16u buf_seg = __dpmi_allocate_dos_memory(((512+15)>>4), &sel);
  if (buf_seg == -1) {
    printf("\n Error allocating memory");
    return FALSE;
  }
  
  // set up the data packet to send
  disk_packet.size = 0x10;
  disk_packet.resv = 0x00;
  disk_packet.num_blocks = 1;
  disk_packet.offset = 0x0000;
  disk_packet.segment = buf_seg;
  disk_packet.lba = lba;
  disk_packet.phys_address = 0;
  // copy disk_packet to DOS conventional memory using DJGPP's transfer buffer
  dosmemput(&disk_packet, sizeof(struct S_DISK_PACKET), __tb);
  
  // call the INT 13h service
  regs.h.ah = 0x42;
  regs.h.dl = drv;
  regs.x.ds = __tb >> 4;
  regs.x.si = __tb & 0x0f;
  __dpmi_int(0x13, &regs);
  
  // if carry is clear on return, copy the 512 byte sector to our buffer
  if ((regs.x.flags & 0x01) == 0) {
    dosmemget(buf_seg << 4, 512, ptr);
    __dpmi_free_dos_memory(sel); // free the memory
    return TRUE;
  }
  
  // free the memory and return FALSE
  __dpmi_free_dos_memory(sel);
  return FALSE;
}

/* This function is re-entrant (recursive) and will display the values of the four partition
 *  entries in the MBR table at LBA.  If one of these entries points to an Extended MBR entry,
 *  it will recurse and call this function again to display those partition entries, again,
 *  allowing for another Extended MBR entry.
 *
 * Please note that LBA is passed as a 64-bit value even though MBR entries only support
 *  32-bit entries.  However, since Extended Partitions are zero based from the partition
 *  entry, not the base of the disk, adding the current LBA to the Extended partition's
 *  offset could result in more than a 32-bit number.  Therefore, 64-bits are used.
 */
void display_part_entrys(const int seq, const bit64u lba, const bit8u drv) {
  
  int i;
  struct S_MBR mbr;
  
  // read the mbr sector
  if (!read_sector(drv, lba, &mbr)) {
    printf("\n Error reading from sector 0x%016llX", lba);
    return;
  }
  
  // if the mbr's signature word does not equal 0xAA55, then don't parse partition entries
  if (mbr.sig != 0xAA55) {
    printf("\n mbr sig != 0xAA55  (%04X)", mbr.sig);
    return;
  }
  
  // loop 4 times for 4 partition entries
  for (i=0; i<4; i++) {
    display_new_line(seq);
    printf("%c\xC4 boot_id = 0x%02X  sys_id = 0x%02X (%s)",
      (i < 3) ? 0xC3 : 0xC0,
      mbr.part_entry[i].boot_id, mbr.part_entry[i].sys_id, fdisk_ids[mbr.part_entry[i].sys_id]);
    
    // only print remaining items if not an empty partition type (type == 0 if empty)
    if (mbr.part_entry[i].sys_id > 0) {
      display_new_line(seq);
      printf("%c  start = %i (%li) sectors = %i",
        (i < 3) ? 0xB3 : 0x20,
        mbr.part_entry[i].start_lba, lba + mbr.part_entry[i].start_lba, mbr.part_entry[i].size);
      
      display_new_line(seq);
      printf("%c  start: head = %i, sector = %i, cylinder = %i",
        (i < 3) ? 0xB3 : 0x20,
        mbr.part_entry[i].start.head, mbr.part_entry[i].start.sector, mbr.part_entry[i].start.cylinder);
      
      display_new_line(seq);
      printf("%c    end: head = %i, sector = %i, cylinder = %i",
        (i < 3) ? 0xB3 : 0x20,
        mbr.part_entry[i].end.head, mbr.part_entry[i].end.sector, mbr.part_entry[i].end.cylinder);
      
      // if the partition is an extended partition, then parse it too.
      if (mbr.part_entry[i].sys_id == 0x05)
        display_part_entrys(seq + 1, lba + mbr.part_entry[i].start_lba, drv);
    }
  }
}

/* This function simply prints a new line with a give number of
 *  vertical lines after it to represent the depth of the partition
 */
void display_new_line(const int seq) {
  int i;
  
  printf("\n ");
  for (i=0; i<seq; i++)
    printf("\xB3  ");
}

