/*
 *                             Copyright (c) 1984-2021
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

#if !defined(AFX_FSZ_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_)
#define AFX_FSZ_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// fsz.h : header file
//

#include "MyImageList.h"
#include "MyTreeCtrl.h"
#include "Modeless.h"

/*
#define FAT_ATTR_LONG_FILE 0x0F
#define FAT_ATTR_ARCHIVE   0x20
#define FAT_ATTR_SUB_DIR   0x10
#define FAT_ATTR_VOLUME    0x08
#define FAT_ATTR_SYSTEM    0x04
#define FAT_ATTR_HIDDEN    0x02
#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_ALL       0x3F

#define FAT_DELETED_CHAR   0xE5
*/

// feature flags
#define FSZ_SB_BIGINODE         (1<<0)  // indicates inode size is 2048 (ACL size 96 instead of 32)
#define FSZ_SB_JOURNAL_DATA     (1<<1)  // also put file content records in journal file, not just metadata
#define FSZ_SB_SOFTRAID         (1<<2)  // single disk when not set
#define FSZ_SB_ACCESSDATE       (1<<3)  // store last access timestamp in i-nodes


#pragma pack(push, 1)

// 128-bit unsigned double quadword
#ifndef DQWORD
  typedef struct _DQWORD {
    DWORD64 LowPart;
    DWORD64 HighPart;
  } DQWORD;
#endif

struct S_FSZ_SUPER {
  BYTE   loader[512];
  DWORD  magic;
  BYTE   version_major;
  BYTE   version_minor;
  BYTE   logsec;
  BYTE   enctype;
  DWORD  flags;
  WORD   maxmounts;
  WORD   currmounts;
  DQWORD numsec;        // total logical sectors on volume minus 1
  DQWORD freesec;
  DQWORD rootdirfid;
  DQWORD freesecfid;
  DQWORD badsecfid;
  DQWORD indexfid;
  DQWORD metafid;
  DQWORD journalfid;
  DWORD64 journalhead;
  DWORD64 journaltail;
  DWORD64 journalmax;
  BYTE   encrypt[28];
  DWORD  enchash;
  DWORD64 createdate;
  DWORD64 lastmountdate;
  DWORD64 lastunmountdate;
  DWORD64 lastcheckdate;
  struct S_GUID uuid;
  BYTE   reserved[256];
  DWORD  magic2;
  DWORD  checksum;
  BYTE   raidspecific[1024];
};

/*
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
  BYTE   name0[10];
  BYTE   attrb;
  BYTE   resv;
  BYTE   sfn_crc;
  BYTE   name1[12];
  WORD   clust_zero;
  BYTE   name2[4];
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
  struct S_FAT_ROOT Entry[ROOT_ENTRY_MAX];  // A LFN can use up to 64 entries (64 * 32 = 2048 bytes)
  
};
*/

#pragma pack(pop)

/*
// structure to hold all FAT entries (cluster numbers)
struct S_FAT_ENTRIES {
  DWORD *entries;
  int    entry_size;
  int    entry_count;
  BOOL   was_error;
};
*/

/////////////////////////////////////////////////////////////////////////////
// CFSZ dialog

class CFSZ : public CPropertyPage {
  DECLARE_DYNCREATE(CFSZ)

// Construction
public:
  CFSZ();
  ~CFSZ();

// Dialog Data
  //{{AFX_DATA(CFSZ)
  enum { IDD = IDD_FSZ };
  CMyTreeCtrl	m_dir_tree;
  CString	m_magic;
  CString	m_version_major;
  CString	m_version_minor;
  CString	m_logsec;
  CString	m_enctype;
  CString	m_flags;
  CString	m_maxmounts;
  CString	m_currmounts;
  CString	m_numsec;
  CString	m_freesec;
  CString	m_freesecfid;
  CString	m_rootfid;
  CString	m_badsecfid;
  CString	m_indexfid;
  CString	m_metafid;
  CString	m_journalfid;
  CString	m_journalhead;
  CString	m_journaltail;
  CString m_journalmax;
  CString	m_encrypt;
  CString	m_enchash;
  CString	m_createdate;
  CString	m_lastmountdate;
  CString	m_lastunmountdate;
  CString	m_lastcheckdate;
  CString	m_uuid;
  CString	m_magic2;
  CString	m_checksum;
  //}}AFX_DATA

  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  DWORD crc32c_calc(void *buffer, size_t length);
  BOOL DetectFSZ(void);
  DWORD GetNewColor(int index);
  
  /*
  bool ParseDir(struct S_FAT_ROOT *root, const unsigned entries, HTREEITEM parent, BOOL IsRoot);
  DWORD CheckRootEntry(struct S_FAT_ROOT *root);
  unsigned FatCheckLFN(struct S_FAT_LFN_ROOT *lfn, DWORD *ErrorCode);
  unsigned FatGetName(struct S_FAT_ROOT *root, CString &name, BYTE *attrib, DWORD *start, DWORD *filesize, BOOL *IsDot);
  unsigned FatGetLFN(struct S_FAT_LFN_ROOT *lfn, CString &name);
  BOOL  FatIsValidChar(const char ch);
  DWORD GetNextCluster(void *FatBuffer, DWORD cluster);
  bool FatFormat(const BOOL AskForBoot);
  void SaveItemInfo(HTREEITEM hItem, DWORD Cluster, DWORD FileSize, struct S_FAT_ROOT *Entry, int Index, int count, DWORD ErrorCode, BOOL CanCopy);
  
  void *ReadFile(DWORD Cluster, DWORD *Size, BOOL IsRoot);
  void WriteFile(void *buffer, struct S_FAT_ENTRIES *ClusterList, DWORD size, BOOL IsRoot);
  void ZeroCluster(DWORD Cluster);
  */
  void SendToDialog(void *ptr);
  //void ReceiveFromDialog(void *ptr);
  /*
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  BOOL InsertFile(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  void InsertFolder(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  void DeleteFolder(HTREEITEM hItem);
  void DeleteFile(HTREEITEM hItem);
  
  void *FatLoadFAT(void *fat_buffer);
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
  */
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;

  void *m_super;
  /*
  int   m_fat_size;  // FS_FATxx
  DWORD m_datastart;
  DWORD m_rootstart;
  DWORD m_rootsize;  // size of root in bytes
  */
  BOOL    m_isvalid;
  int     m_index; // index into dlg->FSZ[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;
  DWORD   m_block_size; // must be at least 2048
  BOOL    m_big_inode;  // 0 = Inodes are 1024 bytes in size, 1 = 2048 bytes in size

  BOOL    m_show_del;
  BOOL    m_del_clear;

  //int     m_parse_depth_limit; // used to catch a repeating '..' or '.' recursive error
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CFSZ)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  /*
  void FatCheckRoot(CModeless &modeless, struct S_FAT_ROOT *root, const unsigned entries, CString csPath);
  bool CheckForRecursion(DWORD cluster);
  */
  // Generated message map functions
  //{{AFX_MSG(CFSZ)
  virtual BOOL OnInitDialog();
  /*
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnFatApply();
  afx_msg void OnFatClean();
  afx_msg void OnFatFormat();
  afx_msg void OnFatCheck();
  afx_msg void OnUpdateCode();
  afx_msg void OnFatCopy();
  afx_msg void OnInsertVLabel();
  afx_msg void OnFatInsert();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  */
  afx_msg void OnChangeVersion();
  afx_msg void OnChangeLogSecSize();
  afx_msg void OnChangeHashType();
  afx_msg void OnChangeEncryptType();
  /*
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
  */
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FSZ_H__D10F989D_088D_449D_8E72_E3E414D0E5BA__INCLUDED_)
