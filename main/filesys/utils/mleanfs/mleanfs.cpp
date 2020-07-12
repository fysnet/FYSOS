/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2019
 *
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that
 *   uses this code must have written permission from the author.
 *
 * Last update:  25 Apr 2019
 *
 * usage:
 *   mleanfs filename.ext /e /v
 *
 * See the included files.txt file for an example of the resource file
 *
 * This utility will take a list of filenames and include those
 *    files in the image, creating root entries.
 *
 * This utility will allow sub-directories to be used.  For example,
 *  if the filename is given as
 *    first/second/filename.txt
 *  this utility will create a sub-directory called 'first' in the root
 *  directory, then create a sub-directory called 'second' in the 'first'
 *  directory, and then create the 'filename.txt' entry.
 * If you include another filename using the same directory structure,
 *  such as
 *    first/filename.txt
 *  this utility won't need to create a new directory, because it already
 *  exists.
 *
 *  Thank you for your purchase and interest in my work.
 *
 *  This code is designed for a 64-bit only platform.
 *
 */

#pragma warning(disable: 4996)  // disable the _s warning for sprintf(), etc.

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/ctype.h"
#include "../include/misc.h"

#include "mleanfs.h"

struct S_FOLDERS *folders = NULL;
size_t cur_folder = 0;
size_t cur_folder_size = 0;

size_t cur_sector = 0;
bit8u *bitmaps = NULL;
size_t sectors_used = 0;
size_t band_size, bitmap_size, tot_bands;
size_t tot_sects;

int main(int argc, char *argv[]) {
	size_t super_loc = LEAN_SUPER_LOCATION;
	size_t boot_size, size;
  unsigned int u;
	//int i;

	struct S_RESOURCE *resources;
	bool existing_image = FALSE;
	char filename[MAX_PATH + 1];
	FILE *src, *targ;
	char label[256] = "A Label for a Lean FS volume.";
	
	time_t timestamp;
	srand((unsigned) time(&timestamp)); // seed the randomizer

	// print start string
	printf(strtstr);

	// we need to parse the command line and get the parameters found
	parse_command(argc, argv, filename, &existing_image, label);

	// now retrieve the resource file's contents
	resources = parse_resource(filename);
	if (!resources) {
		printf(" Error with Resource file. '%s'\n", filename);
		if (resources) free(resources);
		return -1;
	}

	// do we need to add sectors to end on a cylinder boundary?
	size_t cylinders = (size_t) (resources->tot_sectors + ((16 * 63) - 1)) / (16 * 63);    // cylinders used
	size_t add = (size_t) ((cylinders * (16 * 63)) - resources->tot_sectors); // sectors to add to boundary on cylinder
	if (add && (resources->tot_sectors > 2880)) {  // don't add if floppy image
		printf(" Total Sectors does not end on cylinder boundary. Expand to %I64i? [Y|N] ", resources->tot_sectors + add);
		if (toupper(_getche()) == 'Y') {
			resources->tot_sectors += add;
			// need to calculate again since we added sectors
			cylinders = (size_t) ((resources->tot_sectors + ((16 * 63) - 1)) / (16 * 63));
		}
		puts("");
	}
	tot_sects = (size_t) resources->tot_sectors;

	// make sure log_band_size is within boundaries
	if ((resources->param0 < 12) || (resources->param0 > 31)) {
		printf(" Log Bands (2^%i) must be within boundaries.  Default to 12? [Y|N] ", resources->param0);
		if (toupper(_getche()) == 'Y')
			resources->param0 = 12;
		else
			return -1;
		puts("");
	}

	if (!existing_image) {
		// create target file
    if ((targ = fopen(resources->targ_filename, "w+b")) == NULL) {
			printf(" Error creating target file: '%s'\n", resources->targ_filename);
			return -1;
		}
	}
	else {
		// open existing target file
    if ((targ = fopen(resources->targ_filename, "r+b")) == NULL) {
			printf(" Error opening target file: '%s'\n", resources->targ_filename);
			return -1;
		}
	}

	// allocate our temp buffer
	// must be at least 33 sectors in length
	bit8u *buffer = (bit8u *) calloc(33 * SECT_SIZE, 1);

	// if we are not working on an existing image file, 
	if (!existing_image) {
		// create the MBR and padding sectors
		u = 0;
		if (strlen(resources->mbr_filename)) {
      if ((src = fopen(resources->mbr_filename, "rb")) == NULL) {
				printf(" Error opening mbr.bin file.\n");
				fclose(targ);
				free(buffer);
				return -2;
			}
			fread(buffer, SECT_SIZE, 1, src);

			// create and write the Disk Indentifier
			// We call rand() multiple times.
			* (bit32u *) &buffer[0x01B8] = (bit32u) ((rand() << 20) | (rand() << 10) | (rand() << 0));
			* (bit16u *) &buffer[0x01BC] = 0x0000;

			// create a single partition entry pointing to our partition
			struct PART_TBLE *pt = (struct PART_TBLE *) &buffer[0x01BE];
			pt->bi = 0x80;
			lba_to_chs(&pt->start_chs, (size_t) resources->base_lba);
			pt->si = 0xEA;  //lEAn
			lba_to_chs(&pt->end_chs, (size_t) ((cylinders * resources->heads * resources->spt) - 1 - resources->base_lba));  // last sector - 1 for the MBR
			pt->startlba = (bit32u) resources->base_lba;
			pt->size = (bit32u) ((cylinders * resources->heads * resources->spt) - resources->base_lba);
			printf(" Writing MBR to LBA %I64i\n", FTELL(targ) / SECT_SIZE);
			fwrite(buffer, SECT_SIZE, 1, targ);
			memset(buffer, 0, SECT_SIZE);
			u = 1;
		}

		if (u < resources->base_lba)
			printf(" Writing Padding between MBR and Base LBA...\n");
		for (; u < resources->base_lba; u++)
			fwrite(buffer, SECT_SIZE, 1, targ);
		//resources->tot_sectors -= resources->base_lba;
		tot_sects -= (size_t) resources->base_lba;

		// create the boot code sectors (up to 32)
		if (strlen(resources->boot_filename)) {
      if ((src = fopen(resources->boot_filename, "rb")) == NULL) {
				printf(" Error opening boot file.\n");
				fclose(targ);
				free(buffer);
				return -3;
			}

			// clearing the buffer first makes sure that if we don't read 33 sectors
			//  of boot file (leanfs.bin), the last part will be zeros.
			memset(buffer, 0, SECT_SIZE * 32);
			boot_size = fread(buffer, SECT_SIZE, 32, src);  // boot_size = sectors
			fclose(src);
		} else {
			boot_size = 1;  // must reserve at least 1 sector for boot
			memset(buffer, 0, SECT_SIZE * 32); // but can be up to 32 with default LEAN_SUPER_LOCATION = 32
		}

		// update the sig and base lba within the given boot code
		//  this assumes that the coder of the boot code knew that this area was
		//  reserved for this data...
		// this reserved area is the last few bytes of the first sector of the partition/disk
		//  sig       dword  (unique id for partition/disk)
		//  base_lba  qword  (0 for floppy, could be 63 for partitions)
		//  boot sig  word   (0xAA55)
		* (bit32u *) &buffer[498] = ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
		//if (* (bit64u *) &buffer[502] == 0)  // we only update the base if the .bin file had it as zeros already.
		* (bit64u *) &buffer[502] = FTELL(targ) / SECT_SIZE;

		// calculate the location of the super block
		if (boot_size > (super_loc - 1)) {
			super_loc = boot_size;
			if (super_loc > 32) {
				printf(" Boot code exceeds allowed boot sector area.\n"
					"  Boot = %zi sectors (LBA 0 to LBA %zi), while Super is at LBA %zi\n", boot_size, boot_size - 1, super_loc);
				fclose(targ);
				free(buffer);
				return -1;
			}
			if (super_loc != LEAN_SUPER_LOCATION)
				printf(" Moving superblock to LBA %zi (boot code = %zi sectors)\n", super_loc, boot_size);
		}

		// write the first (up to 32) sectors
		buffer[510] = 0x55; buffer[511] = 0xAA;
		printf(" Writing Boot Sector(s) to LBA %I64i\n", FTELL(targ) / SECT_SIZE);
		fwrite(buffer, SECT_SIZE, super_loc, targ);
	} // !existing_image

	// A band can be 2^12 to 2^31 sectors.
	band_size = (1ULL << resources->param0);  // in sectors
	bitmap_size = (band_size >> 12);       // in sectors
	tot_bands = (size_t) (resources->tot_sectors + (band_size - 1)) / band_size;

	// now create a super block
	struct S_LEAN_SUPER* super = (struct S_LEAN_SUPER*) buffer;
	super->magic = LEAN_SUPER_MAGIC;
	super->fs_version = 0x0006;  // 0.6
	super->log_sectors_per_band = resources->param0;
	super->pre_alloc_count = (8 - 1);
	super->state = (0 << 1) | (1 << 0);  // clean unmount
	calc_guid(&super->guid);
	strncpy((char *) super->volume_label, label, 63);
	super->volume_label[63] = 0;  // make sure label is null terminated
	super->sector_count = resources->tot_sectors;     // 32 = boot, 1 for super, bitmaps, root size, 1 backup super  (70 sectors)
	super->free_sector_count = 0; // we update this later
	super->primary_super = super_loc;
	super->backup_super = ((resources->tot_sectors < band_size) ? resources->tot_sectors : band_size) - 1; // last sector in first band
	super->bitmap_start = super_loc + 1;
	super->root_start = super->bitmap_start + bitmap_size;
	super->bad_start = 0;  // no bad sectors (yet?)
	memset(super->reserved, 0, 360);

	// create a buffer for the bitmap(s), and mark the first few bits as used.
	bitmaps = (bit8u *) calloc(tot_bands * (bitmap_size * SECT_SIZE), 1);
	bitmap_mark(0, 0, (int) super->root_start, TRUE);
	bitmap_mark(0, (int) band_size - 1, 1, TRUE);  // backup super
	for (u = 1; u < tot_bands; ++u)
		bitmap_mark(u, 0, (int) bitmap_size, TRUE);
	// mark the end of the last bitmap for safety
	bitmap_mark((int) (tot_bands - 1), (int) (resources->tot_sectors - ((tot_bands - 1) * band_size)), (int) ((tot_bands * band_size) - (bit32u) resources->tot_sectors), FALSE);

	// allocate the folders data
	folders = (struct S_FOLDERS *) calloc(DEF_FOLDER_CNT * sizeof(struct S_FOLDERS), 1);
	cur_folder_size = DEF_FOLDER_CNT;

	// mark where to start looking for free sectors
	cur_sector = (size_t) super->root_start;

	// create the root
	strcpy(folders[0].name, "/");
	folders[0].root = calloc(FOLDER_SIZE * SECT_SIZE, 1);
	root_start((struct S_LEAN_DIRENTRY *) folders[0].root, super->root_start, super->root_start);
	folders[0].cur_rec = 2;
	folders[0].links_count = 2;   // self ('.' link & '..' link)
	folders[0].buf_size = FOLDER_SIZE;
	folders[0].ext_cnt = get_next_extent(FOLDER_SIZE + 1, &folders[0].extent);
	folders[0].parent = NULL;
	++cur_folder;

	/*  Since we have an empty disk and root, we know that we can start with the
	 *  first sector and write files consecutively.  There is no need to find
	 *  free sectors when we create new entries.  Therefore, we will start with
	 *  the first sector after the root and the next entry within the root.
	 *  We will also keep track of how many sectors we use so that we can
	 *  update the bitmap.
	 */
	size_t read;
	bit64u file_size;
	struct S_LEAN_EXTENT extent;
	size_t ext_cnt;
	for (u = 0; u < resources->file_cnt; u++) {
		// get the file to write to the image
    if ((src = fopen(resources->files[u].path_filename, "rb")) == NULL) {
			printf(" Error opening %s file.\n", resources->files[u].path_filename);
			continue;
		}
		FSEEK(src, 0, SEEK_END);
		file_size = FTELL(src);
		rewind(src);

		// create the root's entry
#ifdef INODE_HAS_EAS
		size = (file_size + (SECT_SIZE - 1)) / SECT_SIZE;
#else
		size = (size_t) (file_size - (SECT_SIZE - S_LEAN_INODE_SIZE) + (SECT_SIZE - 1)) / SECT_SIZE;
#endif
		ext_cnt = get_next_extent(size + 1, &extent);
		create_root_entry(0, resources->files[u].filename, 0, extent.start[0], super->root_start);

		// create the inode
		printf(" % 2i: Writing %s to LBA %I64i\n", u, resources->files[u].filename, resources->base_lba + extent.start[0]);
		FSEEK(targ, (resources->base_lba + extent.start[0]) * SECT_SIZE, SEEK_SET);
		create_inode(targ, file_size, file_size, LEAN_ATTR_ARCHIVE | LEAN_ATTR_IFREG, 1, ext_cnt, &extent);

#ifndef INODE_HAS_EAS
		memset(buffer + SECT_SIZE, 0, (SECT_SIZE - S_LEAN_INODE_SIZE));
		read = fread(buffer + SECT_SIZE, 1, (SECT_SIZE - S_LEAN_INODE_SIZE), src);
		fwrite(buffer + SECT_SIZE, (SECT_SIZE - S_LEAN_INODE_SIZE), 1, targ);
#endif

		unsigned int uu = 0;
		++extent.start[0];  // skip over inode
		--extent.size[0];   // skip over inode
		do {
			// by clearing the buffer first, we make sure that the "padding" bytes are all zeros
			//  (buffer is used by the super, we need to move to next sector)
			memset(buffer + SECT_SIZE, 0, SECT_SIZE);
			read = fread(buffer + SECT_SIZE, 1, SECT_SIZE, src);
			if (read == 0)
				break;
			FSEEK(targ, (resources->base_lba + extent.start[uu]) * SECT_SIZE, SEEK_SET);
			fwrite(buffer + SECT_SIZE, SECT_SIZE, 1, targ);
			++extent.start[uu];
			if (--extent.size[uu] == 0)
				++uu;
		} while (read == SECT_SIZE);
		fclose(src);
	}

	// write the folders
	for (u = 0; u < cur_folder; u++) {
		printf(" Writing folder '%s' to LBA %I64i\n", folders[u].name, resources->base_lba + folders[u].extent.start[0]);
		// if we added to the folder size, we need to allocate some more sectors
		// this works since we have an empty canvas, it won't have more than two extents allocated,
		//  and we only had at most two already allocated, totaling less than the limit we allow.
		if (folders[u].buf_size > FOLDER_SIZE) {
			ext_cnt = get_next_extent(folders[u].buf_size - FOLDER_SIZE, &extent);
			for (size_t uu = 0; uu < ext_cnt; ++uu) {
				folders[u].extent.start[folders[u].ext_cnt] = extent.start[uu];
				folders[u].extent.size[folders[u].ext_cnt] = extent.size[uu];
				++folders[u].ext_cnt;
			}
		}
		FSEEK(targ, (resources->base_lba + folders[u].extent.start[0]) * SECT_SIZE, SEEK_SET);
		create_inode(targ, folders[u].cur_rec * sizeof(struct S_LEAN_DIRENTRY), folders[u].buf_size * SECT_SIZE,
			LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC, folders[u].links_count, folders[u].ext_cnt, &folders[u].extent);
		void *p = folders[u].root;
#ifndef INODE_HAS_EAS
		fwrite(p, (SECT_SIZE - S_LEAN_INODE_SIZE), 1, targ);
		p = (void *) ((bit8u *) p + (SECT_SIZE - S_LEAN_INODE_SIZE));
#endif
		unsigned uu = 0;
		for (unsigned int f = 0; f < folders[u].buf_size; f++) {
			++folders[u].extent.start[uu];
			FSEEK(targ, (resources->base_lba + folders[u].extent.start[uu]) * SECT_SIZE, SEEK_SET);
			fwrite(p, SECT_SIZE, 1, targ);
			if (--folders[u].extent.size[uu] == 0)
				++uu;
			p = (void *) ((bit8u *) p + SECT_SIZE);
		}
		free(folders[u].root);
	}
	free(folders);

	// now write the first band to the disk
	FSEEK(targ, (resources->base_lba + super_loc) * SECT_SIZE, SEEK_SET);
	printf(" Writing Super Block to LBA %I64i\n", FTELL(targ) / SECT_SIZE);
	super->free_sector_count = resources->tot_sectors - sectors_used;
	super->checksum = lean_calc_crc(super, sizeof(struct S_LEAN_SUPER));
	fwrite(super, SECT_SIZE, 1, targ);
	FSEEK(targ, (resources->base_lba + super->backup_super) * SECT_SIZE, SEEK_SET);
	printf(" Writing Backup Super Block to LBA %I64i\n", FTELL(targ) / SECT_SIZE);
	fwrite(super, SECT_SIZE, 1, targ);
	--tot_sects;

	// now create and write each remaining band (just the bitmap)
	bit8u *b = bitmaps;
	for (u = 0; u < tot_bands; ++u) {
		if (u == 0)
			FSEEK(targ, (resources->base_lba + super_loc + 1) * SECT_SIZE, SEEK_SET);
		else
			FSEEK(targ, (resources->base_lba + (band_size * u)) * SECT_SIZE, SEEK_SET);
		printf(" Writing Bitmap #%i to LBA %I64i\n", (int) (u + 1), FTELL(targ) / SECT_SIZE);
		fwrite(b, SECT_SIZE, bitmap_size, targ);
		b += (bitmap_size * SECT_SIZE);
		if (u < (tot_bands - 1))
			tot_sects -= band_size;
		else
			tot_sects = (size_t) (resources->tot_sectors - (FTELL(targ) / SECT_SIZE));
	}

	// if there was only one band, we have written all sectors
	//  due to the fact that we wrote the backup super to the last
	//  sector of the band,
	// else we need to write remaining sectors (as zeros)
	if (tot_bands > 1) {
		memset(buffer, 0, SECT_SIZE);
		while (tot_sects--)
			fwrite(buffer, SECT_SIZE, 1, targ);
	}

	// print space used so we know how much more we have
	puts("");
	printf("     Total space used: %i%%\n", (int) ((cur_sector * 100) / resources->tot_sectors));
	printf(" Total byte remaining: %I64i  (%i Meg)\n", (resources->tot_sectors - cur_sector) * 512,
		(int) (((resources->tot_sectors - cur_sector) + (2048 - 1)) >> 11));

	// done, cleanup and return
	free(buffer);
	free(bitmaps);

	fclose(targ);

	return 0;
}

/* This always skips the first dword since it is the crc field.
 *  The CRC is calculated as:
 *     crc = 0;
 *     loop (n times)
 *       crc = ror(crc) + dword[x]
 */
bit32u lean_calc_crc(const void *ptr, unsigned int size) {
	bit32u crc = 0;
	const bit32u *p = (const bit32u *) ptr;
	unsigned int i;

	size /= sizeof(bit32u);
	for (i = 1; i < size; ++i)
		crc = (crc << 31) + (crc >> 1) + p[i];

	return crc;
}

/* *********************************************************************************************************
 * returns count of micro-seconds since 1 Jan 1970
 * accounts for leap days, but not leap seconds
 * (we can't just use time(NULL) since that is not portable,
 *   and could be different on each compiler)
 */
bit64u get_useconds(void) {
	__time64_t rawtime;
	struct tm timeinfo;

	_time64(&rawtime);
	_localtime64_s(&timeinfo, &rawtime);

	// credit to: http://howardhinnant.github.io/date_algorithms.html
	const int d = timeinfo.tm_mday;
	const int m = timeinfo.tm_mon + 1;
	const int y = (timeinfo.tm_year + 1900) - (m <= 2);
	const int era = (y >= 0 ? y : y - 399) / 400;
	const unsigned yoe = (unsigned) (y - era * 400);                  // [0, 399]
	const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
	const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;           // [0, 146096]
	const unsigned days = era * 146097 + (int)doe - 719468;          // 719468 = days between 0000/03/01 and 1970/1/1.

	// then we need seconds
	const unsigned hours = (days * 24) + timeinfo.tm_hour;
	const unsigned mins = (hours * 60) + timeinfo.tm_min;
	return (bit64u) (((bit64u) mins * 60) + (bit64u) timeinfo.tm_sec) * (bit64u) 1000000;

	// 1493751050000000 = 2 May 2017, ~6:45p
}

// band = band to mark
// start = first bit in band
// count = count of consecutive bits to mark
// used = true = these need to be counted as used
void bitmap_mark(const int band, int start, int count, const bool used) {
	// count the sectors used
	if (used)
		sectors_used += count;

	// point to the correct starting bitmap
	bit8u *p = bitmaps + (band * (bitmap_size * SECT_SIZE));
	while (count--) {
		p[start / 8] |= (1 << (start % 8));
		++start;
	}
}

bit64u bitmap_find(void) {
	while (bitmaps[cur_sector / 8] & (1 << (cur_sector % 8))) {
		++cur_sector;
		if (cur_sector >= (tot_sects - 1)) {  // -1 for the last sector (in the band) being the backup super (we just do -1 for ease)
			printf("Image size too small.  No more available sectors...\n");
			exit(-1);
		}
	}

	// mark it
	bitmaps[cur_sector / 8] |= (1 << (cur_sector % 8));
	++sectors_used;

	return cur_sector++;
}

// In theory, this should only need a maximum of two extents
//  most of the time, only one, since we have a blank canvas.
//  however, when we get to the next band, we need to skip over the
//   bitmap (and super backup), so this will need two extents.
// size = bytes (remember to subtract if EA's not used)
size_t get_next_extent(size_t size, struct S_LEAN_EXTENT* extent) {
	size_t i = 0;
	bit64u sect, c = 1;

	extent->start[0] = bitmap_find();
	extent->size[0] = 1;
	while (--size) {
		sect = bitmap_find();
		if (sect == (extent->start[i] + c)) {
			++extent->size[i];
			++c;
		}
		else {
			++i;
			if (i == LEAN_INODE_EXTENT_CNT) {
				printf("Extent size reached more than %i...\n", LEAN_INODE_EXTENT_CNT);
				exit(-1);
			}
			extent->start[i] = sect;
			extent->size[i] = 1;
			c = 1;
		}
	}

	// return count of extents used
	return i + 1;
}

void create_root_entry(const size_t folder, char *filename, const size_t pos, bit64u sector, const bit64u parent) {
	char name[512], *p;
	size_t i;
	bool is_folder = FALSE, fnd = FALSE;

	// check to see if the file name given is part of a path (i.e.: has directory names first)
	if (p = strchr(filename + pos, '/')) {
		memcpy(name, filename, (p - filename));
		name[(p - filename)] = '\0';

		// now see if we have already made this folders block
		for (i = 0; i < cur_folder; i++) {
			if (strcmp(folders[i].name, name) == 0) {
				fnd = TRUE;
				break;
			}
		}

		// if we didn't find an existing folder, create one
		if (!fnd) {
			if (cur_folder == DEF_FOLDER_CNT) {
				cur_folder_size += DEF_FOLDER_CNT;
				folders = (struct S_FOLDERS *) realloc(folders, cur_folder_size * sizeof(struct S_FOLDERS));
			}
			// i == cur_folder from for() loop above
			strcpy(folders[i].name, name);
			folders[i].buf_size = FOLDER_SIZE;
			folders[i].root = calloc(FOLDER_SIZE * SECT_SIZE, 1);
			folders[i].ext_cnt = get_next_extent(FOLDER_SIZE + 1, &folders[i].extent);
			root_start((struct S_LEAN_DIRENTRY*) folders[i].root, folders[i].extent.start[0], parent);
			folders[i].cur_rec = 2;
			folders[i].links_count = 1;   // self ('.' link)
			folders[i].parent = &folders[folder];
			is_folder = TRUE;
			++cur_folder;
		}

		create_root_entry(i, filename, strlen(name) + 1, sector, folders[i].extent.start[0]);
		sector = folders[i].extent.start[0];
		strcpy(name, name + pos);
	}	else
		strcpy(name, filename + pos);

	// if it was not already found as an existing directory, we need to make the entry
	if (!fnd) {
		const size_t rec_len = ((strlen(name) + 12 + 15) / 16);
		// if we are near the end of the buffer, we need to add to the buffer size
		if (((folders[folder].cur_rec + rec_len) * sizeof(struct S_LEAN_DIRENTRY)) > (folders[folder].buf_size * SECT_SIZE)) {
			folders[i].buf_size += FOLDER_SIZE;
			folders[folder].root = (void *) realloc(folders[folder].root, (folders[i].buf_size * SECT_SIZE));
		}
		struct S_LEAN_DIRENTRY* entry = (struct S_LEAN_DIRENTRY*)
			((bit8u*)folders[folder].root + (folders[folder].cur_rec * sizeof(struct S_LEAN_DIRENTRY)));
		entry->name_len = (bit16u) strlen(name);
		entry->inode = sector;
		entry->type = (is_folder) ? LEAN_FT_DIR : LEAN_FT_REG;
		entry->rec_len = (bit8u) rec_len;
		memcpy(entry->name, name, entry->name_len);

		// move to next record
		folders[folder].cur_rec += entry->rec_len;

		if (is_folder) {
			++folders[i].links_count;
			if (folders[i].parent)
				++folders[i].parent->links_count;
		}
	}
}

// create the first two entries of a root directory
void root_start(struct S_LEAN_DIRENTRY *record, const bit64u self, const bit64u parent) {
	// since '.' and '..' will be less than 4 chars each, we can cheat and use the record index [0] and [1]
	// The "." entry
	record[0].inode = self;
	record[0].type = LEAN_FT_DIR;
	record[0].rec_len = 1;
	record[0].name_len = 1;
	record[0].name[0] = '.';
	// The ".." entry
	record[1].inode = parent;
	record[1].type = LEAN_FT_DIR;
	record[1].rec_len = 1;
	record[1].name_len = 2;
	record[1].name[0] = '.';
	record[1].name[1] = '.';
}

void create_inode(FILE *fp, const bit64u file_size, const bit64u allocation_size, const bit32u attrib,
	const int link_count, const size_t ext_cnt, struct S_LEAN_EXTENT *extent) {

	bit8u buffer[SECT_SIZE];
	struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) buffer;
	memset(buffer, 0, SECT_SIZE);

	inode->magic = LEAN_INODE_MAGIC;
	inode->extent_count = (bit8u) ext_cnt;
	memset(inode->reserved, 0, 3);
	inode->indirect_count = 0;
	inode->links_count = link_count;
	inode->attributes = LEAN_ATTR_IXUSR | LEAN_ATTR_IRUSR | LEAN_ATTR_IWUSR |
#ifdef INODE_HAS_EAS
		LEAN_ATTR_INLINEXTATTR |   // make the data start on the next sector boundary
#endif                         
		attrib;
	inode->file_size = file_size;
#ifdef INODE_HAS_EAS
	inode->sector_count = ((allocation_size + (SECT_SIZE - 1)) / SECT_SIZE) + 1;
#else
	if (allocation_size > (SECT_SIZE - S_LEAN_INODE_SIZE))
		inode->sector_count = (((allocation_size - (SECT_SIZE - S_LEAN_INODE_SIZE)) + (SECT_SIZE - 1)) / SECT_SIZE) + 1;
	else
		inode->sector_count = 1;  // everything fits in the INODE sector
#endif
	inode->acc_time =
		inode->sch_time =
		inode->mod_time =
		inode->cre_time = get_useconds();
	inode->first_indirect = 0;
	inode->last_indirect = 0;
	inode->fork = 0;
	for (size_t e = 0; e < ext_cnt; ++e) {
		inode->extent_start[e] = extent->start[e];
		inode->extent_size[e] = extent->size[e];
	}
	inode->checksum = lean_calc_crc((bit32u *) inode, S_LEAN_INODE_SIZE);

#ifdef INODE_HAS_EAS
	// extended attributes
	bit32u *ea = (bit32u *) ((bit32u) inode + S_LEAN_INODE_SIZE);
	ea[0] = SECT_SIZE - S_LEAN_INODE_SIZE - sizeof(bit32u);
#endif

	// write the inode to the disk
#ifdef INODE_HAS_EAS
	fwrite(buffer, SECT_SIZE, 1, fp);
#else
	fwrite(buffer, S_LEAN_INODE_SIZE, 1, fp);
#endif
}

/* Parse command line.  We are looking for the following items
 *  filename   - This is the path/filename of the resource file to open
 *  /E         - Tell this app to use an existing image to write the fs to.
 *  /V         - Volume Name
 */
void parse_command(int argc, char *argv[], char *filename, bool *existing_image, char *label) {
	int i;
	const char *s;

	strcpy(filename, "");

	for (i = 1; i < argc; i++) {
		s = argv[i];
		if (*s == '/') {
			s++;
			if ((strcmp(s, "E") == 0) ||
				(strcmp(s, "e") == 0))
				* existing_image = TRUE;
			else if ((memcmp(s, "v:", 2) == 0) ||
				(memcmp(s, "V:", 2) == 0)) {
				strncpy(label, s + 2, NAME_LEN_MAX - 1);
				label[NAME_LEN_MAX - 1] = 0;  // make sure null terminated
			}
			else
				printf(" Unknown switch parameter: /%s\n", s);
		}
		else
			strcpy(filename, s);
	}
}
