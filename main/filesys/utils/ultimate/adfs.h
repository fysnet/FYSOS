/*
 *                             Copyright (c) 1984-2026
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

#if !defined(AFX_ADFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_)
#define AFX_ADFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ADFS.h : header file
//

#include "MyTreeCtrl.h"
#include "MyImageList.h"


#define ADFS_SECT_SIZE  256
#define ADFS_SIG_HUGO   0x6F677548


#pragma pack(push, 1)

struct S_ADFS_TRIPLET {
  BYTE entry[3];
};

// first two sectors of the partition (disk)
struct S_ADFS_FS_MAP {
  // sector 0
  struct S_ADFS_TRIPLET sector0[82];
  BYTE zero;
  BYTE name0[5];
  struct S_ADFS_TRIPLET sectors;
  BYTE crc0;

  // sector 1
  struct S_ADFS_TRIPLET sector1[82];
  BYTE name1[5];
  WORD disk_id;
  BYTE boot_option;
  BYTE map_end;
  BYTE crc1;
};

#define ADFS_ENTRY_R    1  // can be read
#define ADFS_ENTRY_W    2  // can be writtten to
#define ADFS_ENTRY_L    4  // file is locked
#define ADFS_ENTRY_D    8  // is a directory
#define ADFS_ENTRY_E   16  // execute only

struct S_ADFS_DIR {
  BYTE    name[10];     // name and attribute
  DWORD   load_addr;    // 
  DWORD   exec_addr;    // 
  DWORD   file_length;  //
  struct S_ADFS_TRIPLET start_block; // starting block
  BYTE    cycle;        // 
};

// cannot be > MAX_ITEM_SIZE bytes
struct S_ADFS_ITEMS {
  DWORD ErrorCode;
  BYTE  Attribute;
  DWORD Start;
  DWORD Parent;
  DWORD Size;
  DWORD Flags;
  int   EntryNum;
  int   EntryTotal;
  struct S_ADFS_DIR Entry;
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CADFS dialog
class CADFS : public CPropertyPage {
  DECLARE_DYNCREATE(CADFS)

// Construction
public:
  CADFS();
  ~CADFS();

// Dialog Data
  //{{AFX_DATA(CADFS)
  enum { IDD = IDD_ADFS };
  CMyTreeCtrl	m_dir_tree;
  CString	m_adfs_sector_count;
  CString	m_adfs_crc0;
  CString	m_adfs_crc1;
  CString m_disk_id;
  CString	m_map_end;
  CString m_disk_name;
  CListBox m_free_list;
  //}}AFX_DATA

  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  DWORD GetNewColor(int index);
  
  void ParseDir(const BYTE *buffer, DWORD Parent, const int sectors, HTREEITEM ParentItem);
  void *ReadFile(DWORD start, DWORD size);
  static BYTE GetName(CString &name, DWORD *start, DWORD *size, struct S_ADFS_DIR *dir);
  void SaveItemInfo(HTREEITEM hItem, int entry_num, struct S_ADFS_DIR *entry, DWORD start, DWORD Parent, DWORD size, BYTE attribute, DWORD flags);

  void WriteFile(void *buffer, DWORD start, DWORD size);
  void DisplayFreeSpace(void);
  DWORD CalcFreeBlocks(void);

  bool Format(void);
  BYTE CalcCRC(BYTE *buffer);
  
  void PutTriplet(struct S_ADFS_TRIPLET *triplet, DWORD value);
  static DWORD GetTriplet(struct S_ADFS_TRIPLET *triplet);
  void SendToDialog(struct S_ADFS_FS_MAP *map);
  void ReceiveFromDialog(struct S_ADFS_FS_MAP *map);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  void InsertFile(DWORD Parent, CString csFPath, CString csName, CString csPath);
  void InsertFolder(DWORD Parent, CString csFPath, CString csName, CString csPath);
  DWORD CreateEntry(void *Buffer, DWORD Size, CString csName, DWORD Parent, DWORD Attrib);
  void CreateName(BYTE *Buffer, CString csName, int Len);
  void DeleteFile(HTREEITEM hItem, CString csName);
  void DeleteFolder(HTREEITEM hItem, CString csName);
  
  int InsertBlocks(struct S_ADFS_FS_MAP *map, DWORD size);
  void FreeBlocks(struct S_ADFS_FS_MAP *map, DWORD begin, DWORD size);
  void OptimizeFreeList(struct S_ADFS_FS_MAP *map);
  void SortDirListing(BYTE *buffer, DWORD lba);

  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  
  struct S_ADFS_FS_MAP m_fs_map;
  BYTE   *m_dir_buffer;
  unsigned m_dir_size;  // in 256-byte sectors
  BOOL    m_isvalid;
  int     m_index; // index into dlg->ADFS[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;

  BOOL    m_show_del;
  BOOL    m_del_clear;
  DWORD   m_free_blocks;

  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CADFS)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CADFS)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnAdfsFormat();
  afx_msg void OnAdfsCheck();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnCrc0Update();
  afx_msg void OnCrc1Update();
  afx_msg void OnAdfsCopy();
  afx_msg void OnAdfsView();
  afx_msg void OnAdfsEntry();
  afx_msg void OnAdfsInsert();
  //afx_msg void OnShowDeleted();
  afx_msg void OnDelClear();
  afx_msg void OnAdfsDelete();
  afx_msg void OnAdfsOptimize();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  afx_msg void OnAdfsApply();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADFS_H__C3EBDBEB_368D_4DB1_9351_09DAC58549E1__INCLUDED_)
