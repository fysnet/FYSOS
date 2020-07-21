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

#ifndef LIST_MBR
#define LIST_MBR

#pragma pack(1)

char strtstr[] = 
         "\nList MBR  v1.00.00  Forever Young Software -- (c) Copyright 1984-2020\n";

struct S_DISK_PACKET {
  bit8u  size;         // 10h or 18h
  bit8u  resv;         // 0
  bit16u num_blocks;   // max 7Fh
  bit16u offset;       // segmented address to transfer to
  bit16u segment;      //
  bit64u lba;          // lba
  bit64u phys_address; // (EDD-3.0, optional) 64-bit flat address of transfer buffer;
};                     // used if DWORD at 04h is FFFFh:FFFFh


bool check_big_disk(const bit8u);
bool read_sector(const bit8u, const bit64u, void *);
void display_new_line(const int);
void display_part_entrys(const int, const bit64u, const bit8u);


const char *fdisk_ids[] = {
  "Empty",
  "FAT12",
  "XENIX root",
  "XENIX usr",
  "FAT16 < 32M",
  "Extended",
  "FAT16",
  "HPFS/NTFS",
  "ATX",
  "ATX Bootable",
  "OS/2 Boot Manage",
  "W95 FAT32",
  "W95 FAT32 (LBA)",
  "*unknown*",
  "W95 FAT16 (LBA)",
  "W95 Ext'd (LBA)",
  "OPUS",
  "Hidden FAT12",
  "Compaq diagnostic",
  "*unknown*",
  "Hidden FAT16 < 32meg",
  "*unknown*",
  "Hidden FAT16",
  "Hidded HPFS/NTFS",
  "AST SmartSleep",
  "*unknown*",
  "*unknown*",
  "Hidded W95 FAT32",
  "Hidded W95 FAT32",
  "*unknown*",
  "Hidded W95 FAT36",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "NEC DOS",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Plan 9",
  "*unknown*",
  "*unknown*",
  "PartitionMagic",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Venix 80286",
  "PPC PRep Boot",
  "SFS",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "QNX4.x",
  "QNX4.x 2nd part",
  "QNX4.x 3rd part",
  "OnTrack DM",
  "OnTrack DM6 Aux",
  "CP/M",
  "OnTrack DM6 Aux",
  "OnTrackDM6",
  "EZ-DRIVE",
  "Golden Bow",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Priam Edisk",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "SpeedStor",
  "*unknown*",
  "GNU HURD or Sys",
  "Novell Netware",
  "Novell Netware",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "DiskSecure Mult",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "PC/IX",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Old Minix",
  "Minix / old Linux",
  "Linux swap",
  "Linux",
  "OS/2 Hidden C:",
  "Linux extended",
  "NTFS volume set",
  "NTFS volume set",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Linux LVM",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Amoeba",
  "Amoeba BBT",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "BSD/OS",
  "IBM Thinkpad hi",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "FreeBSD",
  "OpenBSD",
  "NeXTSTEP",
  "Darwin UFS",
  "NetBSD",
  "*unknown*",
  "Darwin boot",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "BSDI fs",
  "BSDI swap",
  "*unknown*",
  "*unknown*",
  "Boot Wizard hidden",
  "*unknown*",
  "*unknown*",
  "Solaris boot",
  "*unknown*",
  "*unknown*",
  "DRDOS/sec (FAT-",
  "*unknown*",
  "*unknown*",
  "DRDOS/sec (FAT-",
  "*unknown*",
  "DRDOS/sec (FAT-",
  "Syrinx",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Non-FS data",
  "*unknown*",
  "*unknown*",
  "CP/M / CTOS / ",
  "Dell Utility",
  "BootIt",
  "*unknown*",
  "DOS access",
  "*unknown*",
  "DOS R/O",
  "SpeedStor",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "BeOS fs",
  "EFI GPT",
  "*unknown*",
  "*unknown*",
  "EFI (FAT-12/16/",
  "Linux/PA-RISC b",
  "SpeedStor",
  "DOS secondary",
  "*unknown*",
  "SpeedStor",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Linux raid auto",
  "LANstep",
  "BBT"
};

#endif // LIST_MBR
