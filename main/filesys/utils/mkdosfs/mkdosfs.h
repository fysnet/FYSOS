/*
 *                             Copyright (c) 1984-2022
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
 *  Last updated: 5 Feb 2022
 */

// set it to 1 (align on byte)
#pragma pack (push, 1)

char strtstr[] = "\nMake DOS Image  v02.00.00    Forever Young Software 1984-2022\n\n";

#define SECT_SIZE  512

#define FS_INFO_SECT    1
#define FS_BACKUP_SECT  6

#define ROOT_SIZE       7   // default root size in sectors

#define FAT_TYPE resources->param0
#define SPCLUST  resources->param1

struct S_FAT1216_BPB {
  bit8u  jmps[2];       // The jump short instruction
  bit8u  nop;           // nop instruction;
  char   oemname[8];    // OEM name
  bit16u nBytesPerSec;  // Bytes per sector
  bit8u  nSecPerClust;  // Sectors per cluster
  bit16u nSecRes;       // Sectors reserved for Boot Record
  bit8u  nFATs;         // Number of FATs
  bit16u nRootEnts;     // Max Root Directory Entries allowed
  bit16u nSecs;         // Number of Logical Sectors (0B40h)
  bit8u  mDesc;         // Medium Descriptor Byte
  bit16u nSecPerFat;    // Sectors per FAT
  bit16u nSecPerTrack;  // Sectors per Track
  bit16u nHeads;        // Number of Heads
  bit32u nSecHidden;    // Number of Hidden Sectors
  bit32u nSecsExt;      // This value used when there are more
  bit8u  DriveNum;      // Physical drive number
  bit8u  nResByte;      // Reserved (we use for FAT type (12- 16-bit)
  bit8u  sig;           // Signature for Extended Boot Record
  bit32u SerNum;        // Volume Serial Number
  char   VolName[11];   // Volume Label
  char   FSType[8];     // File system type
};

#define   SECT_RES32  32   // sectors reserved

struct S_FAT32_BPB {
  bit8u  jmps[2];
  bit8u  nop;           // nop instruction;
  char   oemname[8];
  bit16u nBytesPerSec;
  bit8u  nSecPerClust;
  bit16u nSecRes;
  bit8u  nFATs;
  bit16u nRootEnts;
  bit16u nSecs;
  bit8u  mDesc;
  bit16u nSecPerFat;
  bit16u nSecPerTrack;
  bit16u nHeads;
  bit32u nSecHidden;
  bit32u nSecsExt;
  bit32u sect_per_fat32; // offset 36 (24h)
  bit16u ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
  bit16u fs_version;
  bit32u root_base_cluster; //
  bit16u fs_info_sec;
  bit16u backup_boot_sec;
  bit8u  reserved[12];
  bit8u  DriveNum;  // not FAT specific
  bit8u  nResByte;
  bit8u  sig;
  bit32u SerNum;
  char   VolName[11];
  char   FSType[8];
};

struct S_FAT32_FSINFO {
  bit32u sig0;              // 0x41615252 ("RRaA")
  bit8u  resv[480];
  bit32u sig1;              // 0x61417272 ("rrAa")
  bit32u free_clust_cnt;    // 0xFFFFFFFF when the count is unknown
  bit32u next_free_clust;   // most recent allocated cluster  + 1
  bit8u  resv1[12];
  bit32u trail_sig;         // 0xAA550000
};

struct S_FAT_ROOT {
  bit8u  name[8];    // name
  bit8u  ext[3];     // ext
  bit8u  attrb;      // attribute
  union {
    bit8u  resv[10];   // reserved in fat12/16
    struct {
      bit8u  nt_resv;    // reserved for WinNT
      bit8u  crt_time_tenth; // millisecond stamp at creation time
      bit16u crt_time;   // time file was created
      bit16u crt_date;   // date file was created
      bit16u last_acc;   // date file was last accessed
      bit16u strtclst32; // hi word of FAT32 starting cluster
    } fat32;
  } type;
  bit16u time;       // time
  bit16u date;       // date
  bit16u strtclst;   // starting cluster number
  bit32u filesize;   // file size in bytes
};

struct S_FAT_LFN_ROOT {
  bit8u  sequ_flags;
  bit8u  name0[10];
  bit8u  attrb;
  bit8u  resv;
  bit8u  sfn_crc;
  bit8u  name1[12];
  bit16u clust_zero;
  bit8u  name2[4];
};

void parse_command(int, char *[], char *, bool *, bool *, char *);
bit8u media_descriptor(const bool, const int, const int, const int);
void create_root_entry(struct S_FAT_ROOT *, char *, const bit32u, bit32u *, bit8u *, bit32u *, const bit8u, 
                       const int, const int, const bit32u);
void create_label_entry(char *, const char *);
bit8u ror_byte(bit8u);
bit32u fat_build_serial_num();
bit16u fat_time_word(struct tm *);
bit16u fat_date_word(struct tm *);

#pragma pack (pop)
