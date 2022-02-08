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

#if !defined(AFX_FYSFS_H__DFCCFA3D_8785_452B_B200_AB858F21B626__INCLUDED_)
#define AFX_FYSFS_H__DFCCFA3D_8785_452B_B200_AB858F21B626__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FYSFS.h : header file
//

#include "MyTreeCtrl.h"
#include "MyImageList.h"

#pragma pack(push, 1)

#define FYSFS_ATTR_ARCHIVE   0x00000001
#define FYSFS_ATTR_SUB_DIR   0x00000002
#define FYSFS_ATTR_LABEL     0x00000004
#define FYSFS_ATTR_SYSTEM    0x00000008
#define FYSFS_ATTR_HIDDEN    0x00000010
#define FYSFS_ATTR_READ_ONLY 0x00000020
#define FYSFS_ATTR_ALL       0x0000003B

// format of stored filename
#define FYSFS_FLAGS_ASCII    0
#define FYSFS_FLAGS_UTF8     1
#define FYSFS_FLAGS_UTF16    2

#define SUPER_BLOCK_LSN   16  // this leaves plenty enough room for a boot and loader to reside at the first of the disk

#define FYSFS_MAX_FILENAME_LEN  255      // 255 16-bit unicode chars plus a 16-bit null char at end
#define FYSFS_NEW_DIR_CLUST_CNT  10      // allocate 10 new clusters for a new directory

// bitmap_flag bits
#define BITMAP_USE_FIRST         0x0000
#define BITMAP_USE_SECOND        0x0001
#define BITMAP_UPDATE_INACTIVE   0x0002

// 2 bit bitmap entries (little-endian)
#define S_FYSOS_BITMAP_FREE     0x00  // 00xxxxxxb = unoccupide
#define S_FYSOS_BITMAP_OCC      0x40  // 01xxxxxxb = occupied
#define S_FYSOS_BITMAP_DEL      0x80  // 10xxxxxxb = deleted
#define S_FYSOS_BITMAP_SYS      0xC0  // 11xxxxxxb = system


#define CLUSTER_NOT_FOUND ((DWORD64) 0xFFFFFFFFFFFFFFFF)


#define S_FYSFS_BOOT_SIG        0x46595342  // 'FYSB'

#define SUPER_LABEL_MAX_LEN  64

#define SUPER_VOL_FLAGS_CASE_SEN  (1<<0)
#define SUPER_VOL_FLAGS_SUP_COPY  (1<<1)

// 512 byte block starting at sector SUPER_BLOCK_LSN
struct S_FYSFS_SUPER {
  DWORD  sig[2];        // signature   'FYSF' 'SUPR' (0x46595346 0x53555052)
  WORD   ver;           // version in BCD (0x0150 = 1.50)
  WORD   sect_clust;    // sectors per cluster (actual) must be a power of 2
  BYTE   encryption;    // type of encryption used (0 = none)
  BYTE   bitmaps;       // number of bitmaps (1 or 2)
  WORD   bitmap_flag;   // flags describing the bitmaps
  DWORD  root_entries;  // count of 'slots' in the root directory
  DWORD64 base_lba;      // physical sector LSN 0 occupies
  DWORD64 root;          // LSN pointer to root.  Must be in data block
  DWORD64 data;          // LSN pointer to the first sector of the data block
  DWORD64 data_sectors;  // count of sectors in data block
  DWORD64 sectors;       // total sectors in volume
  DWORD64 bitmap;        // LSN of list of bitmap sectors
  DWORD64 bitmapspare;   // LSN of list of bitmap sectors (second copy)
  DWORD  chkdsk;        // Seconds since 0:00 1-1-1980 when last chkdsk was ran
  DWORD  lastopt;       // Seconds since 0:00 1-1-1980 when last optimized
  DWORD  flags;         // volume flags
  DWORD  crc;           // crc of this (and copies) super block
  struct S_GUID guid;   // serial number in GUID format (see below)
  char   vol_label[64]; // asciiz volume label
  BYTE   filler[250];   // filler (to pad to 512 bytes)
  BYTE   enc_misc[10];  // encryption misc data area (RC4 10 byte IV) Should only be initialize once
  BYTE   enc_check[80]; // encryption check (updated each commit_super() to be sure it is correct
};

#define S_FYSFS_ROOT_EMPTY      0x00000000  // 
#define S_FYSFS_ROOT_NEW        0x534C4F54  // 'SLOT'
#define S_FYSFS_ROOT_DEL        0x444C5444  // 'DLDT'
#define S_FYSFS_ROOT_SUB        0x53554220  // 'SUB '
#define S_FYSFS_CONT_NAME       0x4E414D45  // 'NAME'
#define S_FYSFS_CONT_FAT        0x46415420  // 'FAT '

#define NAME_FAT_SPACE 80
#define FYSFS_ROOT_SCRATCH_DIRTY  0x01   // bit 0 in scratch field in root and cont structs

// 128 byte block
struct S_FYSFS_ROOT {
  DWORD  sig;           // signature of this slot type
  DWORD  attribute;     // file attributes
  BYTE   resv[5];       // 
  BYTE   fat_entries;   // fat entries in this directory entry
  BYTE   crc;           // crc of this slot.
  BYTE   scratch;       // OS scratch byte.  Should be zero when written to disk.
  DWORD  created;       // seconds since 00:00:00 1-1-80
  DWORD  lastaccess;    // seconds since 00:00:00 1-1-80  (read or written)
  DWORD64 fsize;         // file size
  DWORD  fat_continue;  // next entry that continues the FAT (0 if none)
  DWORD  name_continue; // next entry that continues the name (0 if none)
  WORD   flags;         // file flags (etc)
  BYTE   namelen;       // length of name in this slot
  BYTE   resv1[5];      // 
  // align to 32-bits
    // name starts here (up to 255 8-bit chars)
    // name is zero padded to 32-bits
    // fat starts here
  BYTE   name_fat[NAME_FAT_SPACE];
};

// 128 byte block sub directory marker
struct S_FYSFS_SUB {
  DWORD  sig;           // signature of this slot type (0x53554220)
  DWORD64 parent;       // starting cluster of parent directory (0 == root is parent)
  BYTE   resv;          // reserved
  BYTE   fat_entries;   // fat entries in this directory entry
  BYTE   crc;           // crc of this slot.
  BYTE   scratch;       // OS scratch byte.  Should be zero when written to disk.
  DWORD  created;       // seconds since 00:00:00 1-1-80
  DWORD  resv1;         // reserved
  DWORD64 fsize;        // sub_directory size in bytes (= clusters used * bytes per cluster)
  DWORD  fat_continue;  // next entry that continues the FAT (0 if none)
  DWORD  slot;          // slot number in parent's directory of this directories 'SLOT' entry
  WORD   resv2;         // reserved
  BYTE   namelen;       // length of name in this slot (always 0)
  BYTE   resv3[5];      // 
  DWORD  fat[20];       // up to 20 32-bit FAT entries
};

#define CONT_NAME_FAT_SPACE 112

// bits in the flags register for name_continue and fat_continue slots
#define CONT_FLAGS_LARGE    0x01   // ignored in the name_cont slot

// 128 byte block
struct S_FYSFS_CONT {
  DWORD  sig;           // ‘NAME’ for name, ‘FAT ’ for FAT
  DWORD  previous;      // pointer to the previous slot in this slot chain
  DWORD  next;          // next slot that continues the remainder (0 if none)
  BYTE   count;         // length of name or FAT entries in *this* slot
  BYTE   flags;         // see below
  BYTE   crc;           // crc of this slot.
  BYTE   scratch;       // scratch byte
  BYTE   name_fat[CONT_NAME_FAT_SPACE];
};

// cannot be > MAX_ITEM_SIZE bytes
struct S_FYSFS_ITEMS {
  DWORD64 Cluster;
  DWORD64 FileSize;
  DWORD ErrorCode;
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  DWORD Flags;
  DWORD Sig;
  struct S_FYSFS_ROOT Entry;
};

#pragma pack(pop)

// structure to hold all FAT entries (cluster numbers)
struct S_FYSFAT_ENTRIES {
  DWORD64 *entries;
  int    entry_size;
  int    entry_count;
  BOOL   was_error;
};


/////////////////////////////////////////////////////////////////////////////
// CFYSFS dialog

class CFYSFS : public CPropertyPage {
  DECLARE_DYNCREATE(CFYSFS)

// Construction
public:
  CFYSFS();
  ~CFYSFS();

// Dialog Data
  //{{AFX_DATA(CFYSFS)
  enum { IDD = IDD_FYSFS };
  CMyTreeCtrl m_dir_tree;
  CString m_base_lba;
  CString m_bitmap_flags;
  CString m_bitmap_lsn;
  CString m_bitmap_num;
  CString m_crc;
  CString m_data_lsn;
  CString m_data_sectors;
  CString m_encrypt_type;
  CString m_flags;
  CString m_last_chkdsk;
  CString m_last_optimize;
  CString m_guid;
  CString m_label;
  CString m_root_entries;
  CString m_root_lsn;
  CString m_spare_lsn;
  CString m_sect_cluster;
  CString m_sector_count;
  CString m_sig0;
  CString m_sig1;
  CString m_version;
  //}}AFX_DATA

  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  DWORD GetNewColor(int index);
  
  void ParseDir(struct S_FYSFS_ROOT *root, const unsigned entries, HTREEITEM parent, BOOL IsRoot);
  void *ReadFile(struct S_FYSFS_ROOT *root);
  void FysFSGetName(struct S_FYSFS_ROOT *root, unsigned index, CString &name, DWORD *attrb, DWORD64 *filesize);
  //DWORD64 FysFSGetFATEntry(struct S_FYSFS_ROOT *root, unsigned root_index, unsigned index);
  bool FYSFSFormat(const BOOL AskForBoot, const BOOL clean);
  void SaveItemInfo(HTREEITEM hItem, DWORD64 FileSize, struct S_FYSFS_ROOT *Entry, DWORD Sig, DWORD flags, DWORD ErrorCode, BOOL CanCopy);
  
  void SendToDialog(struct S_FYSFS_SUPER *super);
  void ReceiveFromDialog(struct S_FYSFS_SUPER *super);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  
  void DisplayFreeSpace(void);
  size_t CalcFreeBlocks(void);

  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  struct S_FYSFS_SUPER m_super;
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->FYSFS[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;

  BOOL    m_del_clear;

  size_t  m_free_blocks;

  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CFYSFS)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CFYSFS)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnFYSFSApply();
  afx_msg void OnFYSFSClean();
  afx_msg void OnFYSFSFormat();
  afx_msg void OnFYSFSCheck();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnFYSFSCopy();
  afx_msg void OnFYSFSView();
  afx_msg void OnFYSFSEntry();
  afx_msg void OnFYSFSInsert();
  afx_msg void OnFysosSig();
  afx_msg void OnFlagsUpdate();
  afx_msg void OnBitmapUpdate();
  afx_msg void OnChangeVersion();
  afx_msg void OnKillfocusGuid();
  afx_msg void OnDelClear();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  afx_msg void OnUpdateCode();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  int FillClusterList(struct S_FYSFAT_ENTRIES *EntryList, struct S_FYSFS_ROOT *root);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FYSFS_H__DFCCFA3D_8785_452B_B200_AB858F21B626__INCLUDED_)
