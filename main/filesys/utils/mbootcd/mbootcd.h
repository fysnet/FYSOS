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
 *  Last updated: 15 July 2020
 */


// set it to 1 (align on byte)
#pragma pack (push, 1)

#define VERSION_STR  "02.00.00"

char strtstr[] = "\nMake Bootable CDROM Image  v" VERSION_STR "  Forever Young Software 1984-2022\n";

#define DESC_TYPE_BOOT    0
#define DESC_TYPE_PVD     1
#define DESC_TYPE_SVD     2
#define DESC_TYPE_PART    3
#define DESC_TYPE_TERM  255

// Since CDROM's are "universal" to all platforms, if a value stored
//  in one of the following structures is more than a byte, the value
//  is stored twice.  The first being little_endian, the second, big_endian.
struct VOL_DATE {
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char min[2];
    char sec[2];
    char sec100[2];
   bit8s gmt_off;
};

struct DIR_DATE {
   bit8u since_1900;
   bit8u month;
   bit8u day;
   bit8u hour;
   bit8u min;
   bit8u sec;
   bit8s gmt_off;
};

struct PATH_TAB {
   bit8u len_di;
   bit8u ext_attrib;
  bit32u loc;
  bit16u parent;
   bit8u ident[16];
};

struct ROOT {
   bit8u len;
   bit8u e_attrib;
  bit32u extent_loc;
  bit32u extent_loc_b;
  bit32u data_len;
  bit32u data_len_b;
  struct DIR_DATE date;
   bit8u flags;
   bit8u unit_size;
   bit8u gap_size;
  bit16u sequ_num;
  bit16u sequ_num_b;
   bit8u fi_len;
   bit8u ident;
};

struct PVD {
   bit8u id;
    char ident[5];
   bit8u ver;
   bit8u flags;        // PCD = reserved, SVD = flags
    char sys_ident[32];
    char vol_ident[32];
   bit8u resv1[8];
  bit32u num_lbas;
  bit32u num_lbas_b;
   bit8u escape_sequ[32];
  bit16u set_size;
  bit16u set_size_b;
  bit16u sequ_num;
  bit16u sequ_num_b;
  bit16u lba_size;
  bit16u lba_size_b;
  bit32u path_table_size;
  bit32u path_table_size_b;
  bit32u PathL_loc;
  bit32u PathLO_loc;
  bit32u PathM_loc;
  bit32u PathMO_loc;
  struct ROOT root;  
    char set_ident[128];
    char pub_ident[128];
    char prep_ident[128];
    char app_ident[128];
    char copy_ident[37];
    char abs_ident[37];
    char bib_ident[37];
  struct VOL_DATE vol_date;
  struct VOL_DATE mod_date;
  struct VOL_DATE exp_date;
  struct VOL_DATE val_date;
   bit8u struct_ver;
   bit8u resv3;
   bit8u app_use[512];
   bit8u resv4[653];
};

struct BRVD {
   bit8u id;
    char ident[5];
   bit8u ver;
    char bsident[32];
   bit8u resv0[32];
  bit32u boot_cat;
   bit8u resv1[1973];
};

struct TERM {
   bit8u id;
    char ident[5];
   bit8u ver;
   bit8u resv1[2041];
};

struct CAT {
  struct VAL_ENTRY {
     bit8u id;
     bit8u platform;
    bit16u resv0;
      char ident[24];
    bit16u crc;
     bit8u key55;
     bit8u keyAA;
  } val_entry;
  struct INIT_ENTRY {
     bit8u bootable;
     bit8u media;
    bit16u load_seg;
     bit8u sys_type;
     bit8u resv0;
    bit16u load_cnt;
    bit32u load_rba;
     bit8u resv1[20];
  } init_entry;
};

struct SECT_HEADER {
   bit8u id;
   bit8u platform;
  bit16u num;
   bit8u ident[28];
};

struct SECT_ENTRY {
   bit8u bootable;
   bit8u media;
  bit16u load_seg;
   bit8u sys_type;
   bit8u resv0;
  bit16u load_cnt;
  bit32u load_rba;
   bit8u criteria;
   bit8u vendor[19];
};

struct SECT_ENTRY_X {
   bit8u id;
   bit8u final;
   bit8u vendor[30];
};

void fill_date(struct DIR_DATE *);
void fill_e_date(struct VOL_DATE *);
void ascii2utf16(void *, char *, int);
void fill_utf16(void *, bit16u, int);
void parse_command(int, char *[], char *);

#pragma pack (pop)
