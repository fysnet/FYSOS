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

#if !defined(AFX_SFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_)
#define AFX_SFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SFS.h : header file
//

#include "MyTreeCtrl.h"
#include "MyImageList.h"


#pragma pack(push, 1)

#define SFS_SUPER_MAGIC  0x00534653

#define SFS_ENTRY_SIZE   64

// at offset 0x18E in first part of partition (first sector on floppy, etc)
#define SFS_SUPER_LOC  0x18E
struct S_SFS_SUPER {
  INT64   time_stamp;         // time_stamp is 1/65536ths of a second since 1 Jan 1970
  DWORD64 data_block_count;   // Size of data area in blocks
  DWORD64 index_size;         // Size of index area *in bytes*
  DWORD   magic_version;      // Magic number (0x534653) + SFS version (0x10 for Version 1.0)
  DWORD64 total_blocks;       // Total number of blocks in volume
  DWORD   resv_blocks;        // Number of reserved blocks
  BYTE    block_size;         // Block size (2^(x+7) where x = 2 = 512)
  BYTE    crc;                // zero byte check sum of super block
};

// these all must be 0x1F or less so that it does not interfere with
//  valid filename chars 0x20+
#define SFS_ENTRY_VOL_ID    0x01  // volume ID
#define SFS_ENTRY_START     0x02  // start marker
#define SFS_ENTRY_UNUSED    0x10  // unused
#define SFS_ENTRY_DIR       0x11  // directory entry
#define SFS_ENTRY_FILE      0x12  // file entry
#define SFS_ENTRY_UNUSABLE  0x18  // unusable entry (bad sector(s))
#define SFS_ENTRY_DIR_DEL   0x19  // deleted directory
#define SFS_ENTRY_FILE_DEL  0x1A  // deleted file

#define VOLID_NAME_LEN  52
struct S_SFS_VOL_ID {
  BYTE    type;        // 0x01
  BYTE    crc;         // zero sum byte check sum
  WORD    resvd;       // reserved
  INT64   time_stamp;  // time of media format/volume creation
  BYTE    name[52];    // UTF-8 null terminated
};

struct S_SFS_START {
  BYTE    type;        // 0x02
  BYTE    crc;         // zero sum byte check sum
  BYTE    resvd[62];   // reserved
};

struct S_SFS_UNUSED {
  BYTE    type;        // 0x10
  BYTE    crc;         // zero sum byte check sum
  BYTE    resvd[62];   // reserved
};

#define DIR_NAME_LEN  53
struct S_SFS_DIR {
  BYTE    type;        // 0x11
  BYTE    crc;         // zero sum byte check sum
  BYTE    num_cont;    // number of cont slots
  INT64   time_stamp;  // 
  BYTE    name[DIR_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

#define FILE_NAME_LEN  29
struct S_SFS_FILE {
  BYTE    type;        // 0x12
  BYTE    crc;         // zero sum byte check sum
  BYTE    num_cont;    // number of cont slots
  INT64   time_stamp;  // 
  DWORD64 start_block; // starting block in data area
  DWORD64 end_block;   // end block in data area
  DWORD64 file_len;    // file length in bytes
  BYTE    name[FILE_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

struct S_SFS_UNUSABLE {
  BYTE    type;        // 0x18
  BYTE    crc;         // zero sum byte check sum
  BYTE    resv0[8];    // reserved
  DWORD64 start_block; // starting block in data area
  DWORD64 end_block;   // end block in data area
  BYTE    resv1[38];   // reserved
};

struct S_SFS_DIR_DEL {
  BYTE    type;        // 0x19
  BYTE    crc;         // zero sum byte check sum
  BYTE    num_cont;    // number of cont slots
  INT64   time_stamp;  // 
  BYTE    name[DIR_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

struct S_SFS_FILE_DEL {
  BYTE    type;        // 0x1A
  BYTE    crc;         // zero sum byte check sum
  BYTE    num_cont;    // number of cont slots
  INT64   time_stamp;  // 
  DWORD64 start_block; // starting block in data area
  DWORD64 end_block;   // end block in data area
  DWORD64 file_len;    // file length in bytes
  BYTE    name[FILE_NAME_LEN]; // UTF-8 null terminated (unless need continuation slot)
};

// cannot be > MAX_ITEM_SIZE bytes
struct S_SFS_ITEMS {
  DWORD ErrorCode;
  int   index;       // index number into index_block
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  BYTE  entry[SFS_ENTRY_SIZE];
  
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CSFS dialog
class CSFS : public CPropertyPage {
  DECLARE_DYNCREATE(CSFS)

// Construction
public:
  CSFS();
  ~CSFS();

// Dialog Data
  //{{AFX_DATA(CSFS)
  enum { IDD = IDD_SFS };
  CMyTreeCtrl	m_dir_tree;
  CString	m_sfs_block_count;
  CString	m_sfs_block_size;
  CString	m_sfs_crc;
  CString	m_sfs_index_size;
  CString	m_sfs_magic;
  CString	m_sfs_resv_blocks;
  CString	m_sfs_timestamp;
  CString	m_sfs_total_blocks;
  CString	m_sfs_version;
  //}}AFX_DATA

  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  DWORD GetNewColor(int index);
  
  void ParseDir(const BYTE *index, const int entries);
  HTREEITEM GetName(CString &name, void *entry, BOOL IsDir, BOOL *Error);
  void SaveItemInfo(HTREEITEM hItem, const BYTE *Entry, int Index, BOOL CanCopy);
  void *ReadFile(HTREEITEM hItem, DWORD *FileSize);
  void WriteFile(void *buffer, DWORD64 Block, DWORD FileSize);
  void ZeroFile(DWORD64 Start, DWORD64 End);
  BYTE CalculateCRC(const void *buffer, DWORD Size);
  INT64 CalculateTime(void);
  
  bool Format(const BOOL AskForBoot);
  
  void SendToDialog(struct S_SFS_SUPER *super);
  void ReceiveFromDialog(struct S_SFS_SUPER *super);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  void InsertFile(CString csFPath, CString csName, CString csPath);
  void InsertFolder(CString csFPath, CString csName, CString csPath);
  void DeleteFile(HTREEITEM hItem);
  void DeleteFolder(HTREEITEM hItem);
  BOOL IsFileOrFolder(HTREEITEM hItem);
  
  DWORD64 AppendFileToIndex(CString csFPath, CString csName, DWORD Size, BOOL IsDir);
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  struct S_SFS_SUPER m_super;
  BYTE   *m_indx_buffer;
  unsigned m_indx_size;  // in 512-byte sectors
  DWORD64 m_indx_start;  // lba of the first index sector
  int     m_block_size;
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->SFS[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;

  BOOL    m_show_del;
  BOOL    m_del_clear;
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CSFS)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CSFS)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnSFSClean();
  afx_msg void OnSFSFormat();
  afx_msg void OnSFSCheck();
  afx_msg void OnSFSCopy();
  afx_msg void OnSFSInsert();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnSFSEntry();
  afx_msg void OnChangeSfsBlockSize();
  afx_msg void OnChangeSfsVersion();
  afx_msg void OnCrcUpdate();
  afx_msg void OnChangeSfsTimestamp();
  afx_msg void OnSfsCopy();
  afx_msg void OnSfsEntry();
  afx_msg void OnSfsInsert();
  afx_msg void OnFysosSig();
  afx_msg void OnShowDeleted();
  afx_msg void OnDelClear();
  afx_msg void OnSfsDelete();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnUpdateCode();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  afx_msg void OnSFSApply();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_)
