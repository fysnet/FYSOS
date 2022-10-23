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

#if !defined(AFX_LEAN_H__BCBDA211_31D9_4B7D_B24A_283ADD3337DD__INCLUDED_)
#define AFX_LEAN_H__BCBDA211_31D9_4B7D_B24A_283ADD3337DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Lean.h : header file
//

#include "MyImageList.h"
#include "MyTreeCtrl.h"
#include "Gpt.h"

#define LEAN_SUPER_MAGIC     0x4E41454C  // 'LEAN'
#define LEAN_INODE_MAGIC     0x45444F4E  // 'NODE'
#define LEAN_INDIRECT_MAGIC  0x58444E49  // 'INDX'

#pragma pack(push, 1)

struct S_LEAN_SUPER {
  DWORD  checksum;                // DWORD sum of all fields.
  DWORD  magic;                   // 0x4E41454C ('LEAN')
  WORD   fs_version;              // 0x0007 = 0.7
  BYTE   pre_alloc_count;         // count minus one of contiguous blocks that driver should try to preallocate
  BYTE   log_blocks_per_band;     // 1 << log_blocks_per_band = blocks_per_band. Valid values are 12, 13, 14, ...
  DWORD  state;                   // bit 0 = unmounted?, bit 1 = error?
  struct S_GUID guid;             // Globally Unique IDentifier
  BYTE   volume_label[64];        // can be modified by the LABEL command
  DWORD64 block_count;            // The total number of blocks that form a file system volume
  DWORD64 free_block_count;       // The number of free blocks in the volume. A value of zero means disk full.
  DWORD64 primary_super;          // block number of primary super block
  DWORD64 backup_super;           // block number of backup super block
  DWORD64 bitmap_start;           // This is the address of the block where the first bands' bitmap starts
  DWORD64 root_start;             // This is the address of the block where the root directory of the volume starts, the inode number of the root directory.
  DWORD64 bad_start;              // This is the address of the block where the pseudo-file to track bad blocks starts.
  DWORD64 journal;                // if not zero, inode number of journal file (version 0.7+ only)
  BYTE   log_block_size;          // (1 << log_block_size) = block size (9 = 512, 10 = 1024, etc)
  //BYTE   reserved[];            // zeros
};

#define LEAN_FS_TIME_ADJUST  ((DWORD64) 0x00011EF9B4758000) // additional uS from 01-01-1970 to 01-01-1980

#define LEAN_INODE_SIZE       176
#define LEAN_INODE_EXTENT_CNT   6

// 176 bytes each
struct S_LEAN_INODE {
  DWORD  checksum;                // DWORD  sum of all fields in structure
  DWORD  magic;                   // 0x45444F4E  ('NODE')
  BYTE   extent_count;            // count of extents in this inode struct.
  BYTE   reserved[3];             // reserved
  DWORD  indirect_count;          // number of indirect blocks owned by file
  DWORD  links_count;             // The number of hard links (the count of directory entries) referring to this file, at least 1
  DWORD  uid;                     // currently reserved, set to 0
  DWORD  gid;                     // currently reserved, set to 0
  DWORD  attributes;              // see table below
  DWORD64 file_size;              // file size
  DWORD64 block_count;            // count of blocks used
  INT64  acc_time;                // last accessed: number of uS elapsed since midnight of 1970-01-01
  INT64  sch_time;                // status change: number of uS elapsed since midnight of 1970-01-01
  INT64  mod_time;                // last modified: number of uS elapsed since midnight of 1970-01-01
  INT64  cre_time;                //       created: number of uS elapsed since midnight of 1970-01-01
  DWORD64 first_indirect;         // address of the first indirect block of the file.
  DWORD64 last_indirect;          // address of the last indirect block of the file.
  DWORD64 fork;                   // if non-zero, contains an Inode number for a file holding extended attributes
  DWORD64 extent_start[LEAN_INODE_EXTENT_CNT]; // The array of extents
  DWORD  extent_size[LEAN_INODE_EXTENT_CNT]; 
};

//attributes:
#define  LEAN_ATTR_IXOTH        (1 << 0)  // Other: execute permission 
#define  LEAN_ATTR_IWOTH        (1 << 1)  // Other: write permission 
#define  LEAN_ATTR_IROTH        (1 << 2)  // Other: read permission 
#define  LEAN_ATTR_IXGRP        (1 << 3)  // Group: execute permission 
#define  LEAN_ATTR_IWGRP        (1 << 4)  // Group: write permission 
#define  LEAN_ATTR_IRGRP        (1 << 5)  // Group: read permission 
#define  LEAN_ATTR_IXUSR        (1 << 6)  // Owner: execute permission 
#define  LEAN_ATTR_IWUSR        (1 << 7)  // Owner: write permission 
#define  LEAN_ATTR_IRUSR        (1 << 8)  // Owner: read permission 
//       LEAN_ATTR_             (1 << 9)  // reserved
#define  LEAN_ATTR_ISUID        (1 << 10) // Other: execute as user id
#define  LEAN_ATTR_ISGID        (1 << 11) // Other: execute as group id 
#define  LEAN_ATTR_HIDDEN       (1 << 12) // Don't show in directory listing 
#define  LEAN_ATTR_SYSTEM       (1 << 13) // Warn that this is a system file 
#define  LEAN_ATTR_ARCHIVE      (1 << 14) // File changed since last backup 
#define  LEAN_ATTR_SYNC_FL      (1 << 15) // Synchronous updates 
#define  LEAN_ATTR_NOATIME_FL   (1 << 16) // Don't update last access time 
#define  LEAN_ATTR_IMMUTABLE_FL (1 << 17) // Don't move file blocks 
#define  LEAN_ATTR_PREALLOC     (1 << 18) // Keep any preallocated blocks beyond fileSize when the file is closed
#define  LEAN_ATTR_EAS_IN_INODE (1 << 19) // Remaining bytes after the inode structure are reserved for inline extended attributes
//       LEAN_ATTR_             (1 << 20)  // reserved
//       LEAN_ATTR_             (1 << 21)  // reserved
//       LEAN_ATTR_             (1 << 22)  // reserved
//       LEAN_ATTR_             (1 << 23)  // reserved
//       LEAN_ATTR_             (1 << 24)  // reserved
//       LEAN_ATTR_             (1 << 25)  // reserved
//       LEAN_ATTR_             (1 << 26)  // reserved
//       LEAN_ATTR_             (1 << 27)  // reserved
//       LEAN_ATTR_             (1 << 28)  // reserved
#define  LEAN_ATTR_IFMT         (7 << 29) // Bit mask to extract the file type 
#define  LEAN_ATTR_IFREG        (1 << 29) // File type: regular file 
#define  LEAN_ATTR_IFDIR        (2 << 29) // File type: directory 
#define  LEAN_ATTR_IFLNK        (3 << 29) // File type: symbolic link 
#define  LEAN_ATTR_IFFRK        (4 << 29) // File type: fork 

#define LEAN_INDIRECT_SIZE   56
struct S_LEAN_INDIRECT {
  DWORD  checksum;                // DWORD  sum of all fields before this one.
  DWORD  magic;                   // 0x58444E49 ('INDX')
  DWORD64 block_count;            // total number of blocks addressable using this indirect block.
  DWORD64 inode;                  // the inode number of this file this indirect block belongs to.
  DWORD64 this_block;             // The address of the block storing this indirect block.
  DWORD64 prev_indirect;          // the address of the previous indirect block.
  DWORD64 next_indirect;          // the address of the next indirect block.
  WORD   extent_count;            // The number of valid extent specifications storing in the indirect struct.
  BYTE   reserved0[2];            // reserved
  DWORD  reserved1;               // reserved
  //DWORD64 extent_start[]; // The array of extents
  //DWORD  extent_size[];
};

#define LEAN_NAME_LEN_MAX  4068   // 255 is largest value allowed in rec_len * 16 bytes per record - 12 bytes for header
#define LEAN_DIRENTRY_NAME   12
struct S_LEAN_DIRENTRY {
  DWORD64 inode;     // The inode number of the file linked by this directory entry, the address of the first cluster of the file.
  BYTE   type;      // see table below (0 = deleted)
  BYTE   rec_len;   // len of total record in 16 byte units.
  WORD   name_len;  // total length of name.
  BYTE   name[4];   // (UTF-8) must *not* be null terminated, remaining bytes undefined if not a multiple of 8.  UTF-8
};                  // 4 to make it 16 bytes for first para.  Not limited to 4 bytes.

// type:
#define LEAN_FT_MT    0 // File type: Empty
#define LEAN_FT_REG   1 // File type: regular file
#define LEAN_FT_DIR   2 // File type: directory
#define LEAN_FT_LNK   3 // File type: symbolic link
#define LEAN_FT_FRK   4 // File type: fork

///////////////////////////////////////////////////////////////////////////////////
// Journals
#define JOURNAL_MAGIC   0x4C4E524A
#define JOURNAL_SIZE             2  // in blocks (should not be more than 6 for this formatter)

#define JOURNAL_ENTRY_INVALID  (0<<0)
#define JOURNAL_ENTRY_VALID    (1<<0)

// If exist, is an Inode itself.
struct S_LEAN_JOURNAL {
  DWORD   checksum;      // bit32u sum of all fields in structure *including all entries that follow...
  DWORD   magic;         // 0x4C4E524A  ('JRNL')
  DWORD   entry_cnt;     // count of journal entries in this structure (file)
  DWORD   padding;       // for alignment
  BYTE    reserved[16];  // for future expansion
};

struct S_LEAN_JOURNAL_ENTRY {
  DWORD64 new_inode;
  DWORD64 org_inode;
  DWORD   flags;       // valid entry, etc.
  DWORD   resv[3];     // for future expansion
};


///////////////////////////////////////////////////////////////////////////////////
// cannot be > 240 bytes
struct S_LEAN_ITEMS {
  DWORD64 Inode;
  DWORD64 FileSize;
  DWORD ErrorCode;
  BOOL  CanCopy;    // the entry is not a deleted/invalid/other that we can copy out to the host
  DWORD Offset;     // byte offset in directory where this entry starts
  DWORD Flags;
};

#pragma pack(pop)

// used with EA's
#define MAX_EA_STRUCT_ENTRIES    256
#define MAX_EA_STRUCT_DATA_SIZE  1024
struct S_EA_STRUCT {
  int   NameLen;
  int   AttribLen;
  int   Type;
  CString csName;
  BYTE  Data[MAX_EA_STRUCT_DATA_SIZE];
};

// structure to hold all LEAN blocks
#define LEAN_DEFAULT_COUNT   64
struct S_LEAN_BLOCKS {
  BOOL   was_error;
  unsigned extent_count;
  unsigned allocated_count;
  DWORD64 block_count;
  DWORD64 *extent_start; // The array of extents
  DWORD  *extent_size; 
};

/////////////////////////////////////////////////////////////////////////////
// CLean dialog
class CLean : public CPropertyPage {
  DECLARE_DYNCREATE(CLean)

// Construction
public:
  CLean();
  ~CLean();

// Dialog Data
  //{{AFX_DATA(CLean)
  enum { IDD = IDD_LEAN };
  CMyTreeCtrl	m_dir_tree;
  CString	m_backup_lba;
  CString	m_bad_lba;
  CString	m_bitmap_lba;
  CString	m_crc;
  CString	m_cur_state;
  CString	m_free_blocks;
  CString	m_guid;
  CString	m_label;
  CString	m_magic;
  CString	m_pre_alloc;
  CString	m_primary_lba;
  CString	m_root_lba;
  CString	m_blocks_band;
  CString	m_block_count;
  CString	m_version;
  CString	m_journal_lba;
  CString	m_log_block_size;
  //}}AFX_DATA
  
  BYTE lean_calc_log_band_size(const DWORD sect_size, const DWORD64 tot_blocks);
  
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  BOOL DetectLeanFS(void);
  BOOL DetectLeanFSOld(void);
  DWORD GetNewColor(int index);
  
  void ParseDir(struct S_LEAN_DIRENTRY *root, DWORD64 root_size, HTREEITEM parent);
  void *ReadFile(DWORD64 block, DWORD64 *Size);
  void WriteFile(void *buffer, struct S_LEAN_BLOCKS *Extents, DWORD64 Size);
  void ZeroExtent(DWORD64 ExtentStart, DWORD ExtentSize);
  BOOL ValidInode(const struct S_LEAN_INODE *inode);
  BOOL ValidIndirect(const struct S_LEAN_INDIRECT *indirect);
  DWORD LeanCalcCRC(const void *data, unsigned count);
  void SaveItemInfo(HTREEITEM hItem, DWORD64 Inode, DWORD64 FileSize, DWORD flags, DWORD Offset, BOOL CanCopy);
  bool Format(const BOOL AskForBoot);
  
  void SendToDialog(struct S_LEAN_SUPER *super);
  void ReceiveFromDialog(struct S_LEAN_SUPER *super);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  BOOL InsertFile(DWORD64 Inode, CString csName, CString csPath);
  BOOL InsertFolder(DWORD64 Inode, CString csName, CString csPath);
  void DeleteFile(HTREEITEM hItem);
  void DeleteFolder(HTREEITEM hItem);
  
  DWORD CalcFreeBlocks(void);
  void DisplayFreeSpace(void);
  DWORD64 GetFreeBlock(DWORD64 Start, BOOL MarkIt);
  void MarkBlock(DWORD64 Block, BOOL MarkIt);
  int AppendToExtents(struct S_LEAN_BLOCKS *Extents, DWORD64 Size, DWORD64 Start, BOOL MarkIt);
  void FreeExtents(const struct S_LEAN_BLOCKS *Extents);
  int ReadFileExtents(struct S_LEAN_BLOCKS *Extents, DWORD64 Inode);
  int WriteFileExtents(const struct S_LEAN_BLOCKS *Extents, struct S_LEAN_INODE *inode);
  int AllocateRoot(CString csName, DWORD64 Inode, DWORD64 Start, BYTE Attrib);
  void BuildInode(struct S_LEAN_BLOCKS *Extents, DWORD64 Size, DWORD Attrib);
  void IncrementLinkCount(DWORD64 Inode);
  void DecrementLinkCount(DWORD64 Inode);
  void DeleteInode(DWORD64 Inode);

  void AllocateExtentBuffer(struct S_LEAN_BLOCKS *extents, const unsigned count);
  void ReAllocateExtentBuffer(struct S_LEAN_BLOCKS *extents, const unsigned count);
  void FreeExtentBuffer(struct S_LEAN_BLOCKS *extents);
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  struct S_LEAN_SUPER m_super;
  DWORD64 m_super_block_loc;  // zero based block of super block within this partition
  DWORD   m_free_block_cnt;
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->Lean[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition *in sectors*
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;
  DWORD   m_block_size;
  DWORD64 m_tot_blocks;

  BOOL    m_show_del;
  BOOL    m_del_clear;
  BOOL    m_ESs_in_Inode;

  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

  // Check Lean stuff
  BOOL LeanCheckDir(struct S_LEAN_DIRENTRY *root, DWORD64 root_size, CString path);
  int  LeanCheckInode(DWORD64 block, const BOOL allow_fork);
  int  LeanCheckIndirect(DWORD64 inode_num, DWORD64 block, DWORD *ret_count, DWORD64 *blocks_used);
  void LeanCheckAddInode(DWORD64 block);
  void LeanCheckLinkCount(void);

// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CLean)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CLean)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnLeanApply();
  afx_msg void OnLeanClean();
  afx_msg void OnLeanFormat();
  afx_msg void OnLeanCheck();
  afx_msg void OnChangeLeanJournal();
  afx_msg void OnChangeLeanVersion();
  afx_msg void OnChangeLeanPreAlloc();
  afx_msg void OnChangeLeanBlockBand();
  afx_msg void OnChangeLeanBlockSize();
  afx_msg void OnLeanCopy();
  afx_msg void OnLeanView();
  afx_msg void OnLeanInsert();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnLeanEntry();
  afx_msg void OnFysosSig();
  afx_msg void OnLeanCrcUpdate();
  afx_msg void OnLeanMagicUpdate();
  afx_msg void OnLeanCurrentState();
  afx_msg void OnLeanFreeUpdate();
  afx_msg void OnKillfocusLeanGuid();
  afx_msg void OnGuidCreate();
  afx_msg void OnShowDeleted();
  afx_msg void OnEAsInInode();
  afx_msg void OnDelClear();
  afx_msg void OnLeanDelete();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnUpdateCode();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  afx_msg void OnViewJournal();
  afx_msg void OnJournalInode();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEAN_H__BCBDA211_31D9_4B7D_B24A_283ADD3337DD__INCLUDED_)
