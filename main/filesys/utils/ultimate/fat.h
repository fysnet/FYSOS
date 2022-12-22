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

#if !defined(AFX_FAT_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_)
#define AFX_FAT_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Fat.h : header file
//

#include "MyImageList.h"
#include "MyTreeCtrl.h"
#include "Modeless.h"

#define FAT_ATTR_LONG_FILE 0x0F
#define FAT_ATTR_ARCHIVE   0x20
#define FAT_ATTR_SUB_DIR   0x10
#define FAT_ATTR_VOLUME    0x08
#define FAT_ATTR_SYSTEM    0x04
#define FAT_ATTR_HIDDEN    0x02
#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_ALL       0x3F

#define FAT_DELETED_CHAR   0xE5

#pragma pack(push, 1)

struct S_FAT1216_BPB {
  BYTE   jmp[3];
  char   oem_name[8];
  WORD   bytes_per_sect;
  BYTE   sect_per_clust;
  WORD   sect_reserved;
  BYTE   fats;
  WORD   root_entrys;
  WORD   sectors;
  BYTE   descriptor;
  WORD   sect_per_fat;
  WORD   sect_per_trk;
  WORD   heads;
  DWORD  hidden_sects;
  DWORD  sect_extnd;
  BYTE   drive_num;  // not FAT specific
  BYTE   resv;
  BYTE   sig;
  DWORD  serial;
  char   label[11];
  char   sys_type[8];
};

struct S_FAT32_BPB {
  BYTE   jmp[3];
  char   oem_name[8];
  WORD   bytes_per_sect;
  BYTE   sect_per_clust;
  WORD   sect_reserved;
  BYTE   fats;
  WORD   root_entrys;
  WORD   sectors;
  BYTE   descriptor;
  WORD   sect_per_fat;
  WORD   sect_per_trk;
  WORD   heads;
  DWORD  hidden_sects;
  DWORD  sect_extnd;
  DWORD  sect_per_fat32;
  WORD   ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
  WORD   fs_version;
  DWORD  root_base_cluster; // ??????
  WORD   fs_info_sec;
  WORD   backup_boot_sec;
  BYTE   reserved[12];
  BYTE   drive_num;       // not FAT specific
  BYTE   resv;
  BYTE   sig;
  DWORD  serial;
  char   label[11];
  char   sys_type[8];
};

struct S_FAT32_BPB_INFO {
  DWORD  sig;               // 0x41615252 ("RRaA")
  BYTE   resv[480];         // reserved
  DWORD  struct_sig;        // 0x61417272 ("rrAa")
  DWORD  free_clust_cnt;    // -1 when the count is unknown
  DWORD  next_free_clust;   // most recent allocated cluster
  BYTE   resv0[12];         // reserved
  DWORD  trail_sig;         // 0xAA550000  (00 00 55 AA)
};

struct S_FAT_ROOT {
  BYTE   name[8];    // name
  BYTE   ext[3];     // ext
  BYTE   attrb;      // attribute
  union {
    BYTE   resv[10];   // reserved in fat12/16
    struct {
      BYTE   nt_resv;    // reserved for WinNT
      BYTE   crt_time_tenth; // millisecond stamp at creation time
      WORD   crt_time;   // time file was created
      WORD   crt_date;   // date file was created
      WORD   crt_last;   // date file was last accessed
      WORD   strtclst32; // hi word of FAT32 starting cluster
    } fat32;
  } type;
  WORD   time;       // time
  WORD   date;       // date
  WORD   strtclst;   // starting cluster number
  DWORD  filesize;   // file size in bytes
};

#define FAT_CHARS_PER_LFN  13

struct S_FAT_LFN_ROOT {
  BYTE   sequ_flags;
  WORD   name0[5];
  BYTE   attrb;
  BYTE   resv;
  BYTE   sfn_crc;
  WORD   name1[6];
  WORD   clust_zero;
  WORD   name2[2];
};

#define  FAT_NO_ERROR       0   // no error found
#define  FAT_BAD_ATTRIBUTE  1   // bad attribute value
#define  FAT_BAD_CHAR       2   // Found invalid char in SFN
#define  FAT_BAD_RESVD      3   // SFN reserved section is non-zero
#define  FAT_BAD_LFN_SEQU   4   // bad sequence number found in LFN
#define  FAT_BAD_LFN_DEL    5   // deleted entry
#define  FAT_BAD_LFN_CRC    6   // bad crc of SFN found in LFN
#define  FAT_BAD_LFN_CHAR   7   // Found invalid char in LFN



// cannot be > MAX_ITEM_SIZE bytes
#define ROOT_ENTRY_MAX   64
struct S_FAT_ITEMS {
  DWORD Cluster;
  DWORD FileSize;
  DWORD ErrorCode;
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  int   Index;          // index in root directory of first entry (LFN(s) or SFN)
  int   EntryCount;
  DWORD Flags;
  struct S_FAT_ROOT Entry[ROOT_ENTRY_MAX];  // A LFN can use up to 64 entries (64 * 32 = 2048 bytes)
};

#pragma pack(pop)

// structure to hold all FAT entries (cluster numbers)
struct S_FAT_ENTRIES {
  DWORD *entries;
  int    entry_size;
  int    entry_count;
  BOOL   was_error;
};

#define LASTERRORCODE   7
extern char *FatErrorCode[LASTERRORCODE+1];

/////////////////////////////////////////////////////////////////////////////
// CFat dialog

class CFat : public CPropertyPage {
  DECLARE_DYNCREATE(CFat)

// Construction
public:
  CFat();
  ~CFat();

// Dialog Data
  //{{AFX_DATA(CFat)
  enum { IDD = IDD_FAT };
  CMyTreeCtrl	m_dir_tree;
  CString	m_jmp0;
  CString	m_jmp1;
  CString	m_jmp2;
  CString	m_bytes_sect;
  CString	m_descriptor;
  CString	m_fat_type;
  CString	m_fats;
  CString	m_heads;
  CString	m_hidden_sects;
  CString	m_label;
  CString	m_oem_name;
  CString	m_clust_num;
  CString	m_root_entries;
  CString	m_sects_cluster;
  CString	m_sect_fat;
  CString	m_sect_fat32;
  CString m_ext_flags;
  CString	m_sect_track;
  CString	m_sectors;
  CString	m_sectors_ext;
  CString	m_fs_version;
  CString	m_backup_sector;
  CString	m_drv_num;
  CString	m_info_sector;
  CString	m_reserved;
  CString	m_sect_reserved;
  CString	m_serial_number;
  CString	m_fat_sig;
  //}}AFX_DATA
  
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, const int fs_type, BOOL IsNewTab);
  DWORD GetNewColor(int index);
  
  bool ParseDir(struct S_FAT_ROOT *root, const unsigned entries, HTREEITEM parent, BOOL IsRoot);
  DWORD CheckRootEntry(struct S_FAT_ROOT *root);
  unsigned FatCheckLFN(struct S_FAT_LFN_ROOT *lfn, DWORD *ErrorCode);
  unsigned FatGetName(struct S_FAT_ROOT *root, CString &name, BYTE *attrib, DWORD *start, DWORD *filesize, BOOL *IsDot);
  unsigned FatGetLFN(struct S_FAT_LFN_ROOT *lfn, CString &name);
  BOOL  FatIsValidChar(const BYTE ch, const bool is_sfn);
  DWORD GetNextCluster(void *FatBuffer, DWORD cluster);
  bool FatFormat(const BOOL AskForBoot);
  void SaveItemInfo(HTREEITEM hItem, DWORD Cluster, DWORD FileSize, struct S_FAT_ROOT *Entry, int Index, int count, DWORD flags, DWORD ErrorCode, BOOL CanCopy);
  
  void *ReadFile(DWORD Cluster, DWORD *Size, BOOL IsRoot);
  void WriteFile(void *buffer, struct S_FAT_ENTRIES *ClusterList, DWORD size, BOOL IsRoot);
  void ZeroCluster(DWORD Cluster);
  
  void SendToDialog(void *ptr);
  void ReceiveFromDialog(void *ptr);

  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  BOOL InsertFile(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  BOOL InsertFolder(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  void DeleteFolder(HTREEITEM hItem);
  void DeleteFile(HTREEITEM hItem);
  
  void *FatLoadFAT(void *fat_buffer);
  DWORD CalcFreeClusters(void *fat_buffer);
  DWORD CalcDataClusters(const void *bpb, const int fat_size);
  void DisplayFreeSpace(void);
  int CalcSectPerFat(DWORD64 size, int spc, int sect_size, int fat_size);
  int FatFillClusterList(struct S_FAT_ENTRIES *EntryList, DWORD StartCluster);
  int AllocateFAT(struct S_FAT_ENTRIES *EntryList, DWORD size);
  DWORD FindFreeCluster(DWORD Cluster);
  void UpdateTheFATs(void);
  void MarkCluster(DWORD Cluster, DWORD Value);
  void UpdateEntry(HTREEITEM hItem, struct S_FAT_ITEMS *entry_items);
  
  void AllocateRoot(CString csName, DWORD RootCluster, DWORD StartCluster, DWORD FileSize, BYTE Attribute, BOOL IsRoot);
  void AllocateRootSFN(CString csName, DWORD RootCluster, DWORD StartCluster, DWORD FileSize, BYTE Attribute, BOOL IsRoot);
  BYTE CalcCRCFromSFN(struct S_FAT_ROOT *entry);
  void CreateSFN(CString csLFN, int seq, BYTE name[8], BYTE ext[3]);
  WORD CreateDate(void);
  WORD CreateTime(void);
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  void *m_bpb_buffer;
  void *m_fat_buffer;
  DWORD m_clusters_in_data_area;
  DWORD m_free_clusters;
  BOOL  m_cluster_mismatch;
  int   m_fat_size;  // FS_FATxx
  DWORD m_datastart;
  DWORD m_rootstart;
  DWORD m_rootsize;  // size of root in bytes
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->Fat[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;

  BOOL    m_show_del;
  BOOL    m_del_clear;

  int     m_parse_depth_limit; // used to catch a repeating '..' or '.' recursive error
  
  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CFat)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  void FatCheckRoot(CModeless &modeless, struct S_FAT_ROOT *root, const unsigned entries, CString csPath);
  bool CheckForRecursion(DWORD cluster);

  // Generated message map functions
  //{{AFX_MSG(CFat)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnFatApply();
  afx_msg void OnFatClean();
  afx_msg void OnFatFormat();
  afx_msg void OnFatCheck();
  afx_msg void OnUpdateCode();
  afx_msg void OnFatCopy();
  afx_msg void OnFatView();
  afx_msg void OnInsertVLabel();
  afx_msg void OnFatInsert();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnFatEntry();
  afx_msg void OnFysosSig();
  afx_msg void OnFat32Info();
  afx_msg void OnChangeFatFsVersion();
  afx_msg void OnSerialUpdate();
  afx_msg void OnFatBackupSectUpdate();
  afx_msg void OnFatBackupSectRestore();
  afx_msg void OnOldFat();
  afx_msg void OnShowDeleted();
  afx_msg void OnDelClear();
  afx_msg void OnFatDelete();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAT_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_)
