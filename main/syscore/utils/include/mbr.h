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

#ifndef MBR
#define MBR

#pragma pack(push, 1)

struct S_PART_ENTRY {
	bit8u  boot_id;
	struct {
		bit8u  head;       // 8 bits
		bit8u  sector;     // hi 2 bits = hi 2 bits of cyl, low 6 bits = sector
		bit8u  cylinder;   // low 8 bits of 10 bit cyl
	} start;
	bit8u  sys_id;
	struct {
		bit8u  head;       // 8 bits
		bit8u  sector;     // hi 2 bits = hi 2 bits of cyl, low 6 bits = sector
		bit8u  cylinder;   // low 8 bits of 10 bit cyl
	} end;
	bit32u start_lba;
	bit32u size;
};

struct S_MBR {
	bit8u  boot[446];
	struct S_PART_ENTRY part_entry[4];
	bit16u sig;
};

#pragma pack(pop)

#endif  // MBR
