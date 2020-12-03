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
 *  Last updated: 2 Dec 2020
 */

#ifndef FYSOS_MISC
#define FYSOS_MISC

// huge file seek and ftell
#ifdef _MSC_VER
  #define FSEEK(f, o, s) _fseeki64(f, (o), s)
  #define FTELL(f) _ftelli64(f)
#elif defined DJGPP
  #define _FILE_OFFSET_BITS  64  
  #define FSEEK(f, o, s) fseek(f, (o), s)
  #define FTELL(f) ftell(f)
#else
# error 64 bit ftell(), fseek() not defined
#endif

// 64-bit display
#ifdef _MSC_VER
  #define LL64BIT "I64"
#elif defined DJGPP
  #define LL64BIT "ll"
#endif

// set it to 1 (align on byte)
#pragma pack (push, 1)

#define PART_START      63  // assuming non existing image to modify.

struct PART_TBLE_CHS {
  bit8u  head;    // 8 bit head count
  bit8u  sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  bit8u  cyl;     // bottom 8 bits
};

struct PART_TBLE {
  bit8u  bi;
  struct PART_TBLE_CHS start_chs;
  bit8u  si;
  struct PART_TBLE_CHS end_chs;
  bit32u startlba;
  bit32u size;
};

#pragma pack (pop)

// Convert LBA to CHS.  If LBA >= (1024 * spt * heads) use max values */
#define _SECTOR ((lba % 63) + 1)
#define _HEAD   ((lba / 63) % 16)
#define _CYL    ((lba / 63) / 16)
// Converts LBA to the BIOS form of CHS with the high 2 bits of the cylinder
//  in the high 2 bits of the sector field
void lba_to_chs(struct PART_TBLE_CHS *chs, const size_t lba) {
  if (lba < (1024 * 16 * 63)) {
    chs->head = (bit8u) _HEAD;
    chs->sector = (bit8u) (((_CYL & 0x300) >> 2) | (_SECTOR & 0x3F));
    chs->cyl = (bit8u) (_CYL & 0x0FF);
  } else {
    chs->head = 0xFE;
    chs->sector = 0xFF;
    chs->cyl = 0xFF;
  }
}

// ex: 3F2504E0-4F89-11D3-9A0C-0305E82C3301
//       data1    2    3    4     5[6]
struct S_GUID {
  bit32u data1;
  bit16u data2;
  bit16u data3;
  bit16u data4;
  bit8u  data5[6];
};

// Sometimes the data4 member is set to the machines MAC address.
//  However, this allows a user to track a guid to the machine that
//  made the guid.
void calc_guid(struct S_GUID *guid) {
  
  // time() only returns 32-bits, so we need the cast below
  bit64u timestamp;
  
  time((time_t *) &timestamp);
  guid->data1 = (bit32u) timestamp;
  guid->data2 = (bit16u) (timestamp >> 32);
  guid->data3 = (bit16u) (timestamp >> 48);
  guid->data4 = (bit16u) rand();
  * ((bit32u *) &guid->data5[0]) = rand();
  * ((bit16u *) &guid->data5[4]) = (bit16u) rand();
}


#define NAME_LEN_MAX  512

// resource file contents (after we parse it)
struct S_RESOURCE {
    char mbr_filename[NAME_LEN_MAX];
    char boot_filename[NAME_LEN_MAX];
    char targ_filename[NAME_LEN_MAX];
  bit32u file_cnt;
  bit64u base_lba;
  bit64u tot_sectors;
  int    param0;
  int    param1;
  int    heads;
  int    spt;
  struct {
    char path_filename[NAME_LEN_MAX];
    char filename[NAME_LEN_MAX];
    bit32u param;
  } files[1];  // allocated later
};

void get_a_string(FILE *fp, char *str) {
  char ch, *t = str;
  int c = NAME_LEN_MAX - 1;
  
  // skip all leading spaces
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch != ' ')
      break;
  }
  
  while (1) {
    if (feof(fp))
      break;
    if (ch == 13) {  // skip CR character
      ch = fgetc(fp);
      continue;
    }
    if (strchr("\xA,=#", ch))
      break;
    *t++ = ch;
    if (--c == 0)
      break;
    ch = fgetc(fp);
  }
  
  // if it was the '#' char, skip to eol
  if (ch == '#') {
    while (!feof(fp)) {
      ch = fgetc(fp);
      if (ch == 10)
        break;
    }
  }
  
  // kill all trailing spaces
  while (t > str) {
    t--;
    if (*t != ' ') {
      t++;
      break;
    }
  }
  
  // asciiz it
  *t = 0;
}

/*
 *  # line is a comment
 *  mbrfile=filename   - This is the path/filename of the mbr file to use 
 *  bootfile=filename  - This is the path/filename of the boot code file to use
 *  imgfile=filename   - This is the path/filename of the target file to create/modify
 *  base_lba=0         - Base LBA to write the Super Block to
 *  tot_sects=0        - Total sectors to "allocate"
 *  path=path_to_source_file, filename=filename_to_use, param
 *
 */
struct S_RESOURCE *parse_resource(const char *filename) {
  struct S_RESOURCE *r;
  int cnt = 0;
  int limit = 10;  // start with allowing 10 pairs
  char str[NAME_LEN_MAX];
  FILE *fp;
  
  errno_t err;

  err = fopen_s(&fp, filename, "r");
  if (err != 0) {
    printf("\nError opening resource file.");
    return NULL;
  }
  
  r = (struct S_RESOURCE *) calloc(sizeof(struct S_RESOURCE) - sizeof(r->files) + (limit * sizeof(r->files)), 1);
  r->base_lba = PART_START;       // assuming none given
  r->tot_sectors = (20*16*63);    // default size
  r->param0 = 12;                 // 
  
  while (!feof(fp)) {
    get_a_string(fp, str);
    if (strcmp(str, "mbrfile") == 0) {
      get_a_string(fp, str);
      strcpy_s(r->mbr_filename, str);
    } else if (strcmp(str, "bootfile") == 0) {
      get_a_string(fp, str);
      strcpy_s(r->boot_filename, str);
    } else if (strcmp(str, "imgfile") == 0) {
      get_a_string(fp, str);
      strcpy_s(r->targ_filename, str);
    } else if (strcmp(str, "base_lba") == 0) {
      get_a_string(fp, str);
      r->base_lba = strtoul(str, NULL, 0);
    } else if (strcmp(str, "tot_sects") == 0) {
      get_a_string(fp, str);
      r->tot_sectors = strtoul(str, NULL, 0);
    } else if (strcmp(str, "param0") == 0) {
      get_a_string(fp, str);
      r->param0 = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "param1") == 0) {
      get_a_string(fp, str);
      r->param1 = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "heads") == 0) {
      get_a_string(fp, str);
      r->heads = (bit8u) strtoul(str, NULL, 0);
    } else if (strcmp(str, "spt") == 0) {
      get_a_string(fp, str);
      r->spt = (bit8u) strtoul(str, NULL, 0);
    } else if (strlen(str) > 0) {
      strcpy_s(r->files[cnt].path_filename, str);
      get_a_string(fp, r->files[cnt].filename);
      get_a_string(fp, str);
      r->files[cnt].param = strtoul(str, NULL, 0);
      cnt++;
      if (cnt == limit) {
        limit += 10;
        r = (struct S_RESOURCE *) realloc(r, sizeof(struct S_RESOURCE) - sizeof(r->files) + (limit * sizeof(r->files)));
      }
    }
  }
  
  // close the file
  fclose(fp);
  
  r->file_cnt = cnt;
  return r;
}

#endif // FYSOS_MISC
