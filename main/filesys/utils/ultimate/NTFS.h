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

#if !defined(AFX_NTFS_H__10789641_6A20_4C8C_B938_CF35BDC45E53__INCLUDED_)
#define AFX_NTFS_H__10789641_6A20_4C8C_B938_CF35BDC45E53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NTFS.h : header file
//
#include "MyImageList.h"
#include "MyTreeCtrl.h"

#pragma pack(push, 1)

#define NTFS_MFT_CACHE_SIZE  32   // count of MFT entries we cache  (smaller values is actually faster)

// cluster sizes can be 512 bytes, 1k, 2k, 4k, 8k, 16k, 32k, and 64k
//        sectors =       1        2   4   8   16  32   64       128

#define NTFS_ATTR_SUB_DIR   0x10000000
#define NTFS_ATTR_SUB_COMP  0x00000800
#define NTFS_ATTR_ARCHIVE   0x00000020
#define NTFS_ATTR_SYSTEM    0x00000004
#define NTFS_ATTR_HIDDEN    0x00000002
#define NTFS_ATTR_READ_ONLY 0x00000001

#define STARTING_MFT 16

enum {
  MFT_MFT = 0,
  MFT_MIRROR,
  MFT_LOGFILE,
  MFT_VOLUME,
  MFT_ATTRDEF,
  MFT_ROOT,
  MFT_BITMAP_,   // MFC already defines MFT_BITMAP
  MFT_BOOT,
  MFT_BADCLUS,
  MFT_SECURE,
  MFT_UPCASE,
  MFT_EXTEND
};

#define NTFS_TEMP_BUF_SIZE  (512*8)

struct S_NTFS_BPB {
  BYTE     jmp[3];
  char     oem_name[8];
  WORD     bytes_per_sect;
  BYTE     sect_per_clust;
  WORD     sect_resved;  // must be zero
  BYTE     resv0[3];     // must be zero
  WORD     resv1;        // must be zero
  BYTE     descriptor;   // F8
  WORD     resv2;        // must be zero
  WORD     sect_per_trk; // not used by ntfs
  WORD     heads;        // not used by ntfs
  DWORD    hidden;       // not used by ntfs
  DWORD    resv3;        // must be zero
  BYTE     phy_drv;      // not used by ntfs
  BYTE     cur_head;     // not used by ntfs
  BYTE     xboot_sig;    // not used by ntfs
  BYTE     resv4;        // not used by ntfs
  DWORD64  tot_sectors;  
  DWORD64  MFT_cluster;  
  DWORD64  MFTMirr_cluster; 
  BYTE     clust_file_rec_size;
  BYTE     resv5[3];     // not used by ntfs
  BYTE     clust_index_block;
  BYTE     resv6[3];     // not used by ntfs
  DWORD64  serial_num;   //
  DWORD    crc;          // not used by ntfs (who said it was a crc?)
};

#define NTFS_TIME_ADJ ((DWORD64 )(379 * 365 + 91) * 24 * 3600 * 10000000)  // 1A8E79FE1D58000
struct S_NTFS_FILE_TIMES {
  DWORD64  creation_time;
  DWORD64  last_mod;
  DWORD64  last_mod_file_rec;
  DWORD64  last_access;
};

#define FILE_REC_MAGIC  0x454C4946

struct S_NTFS_FILE_REC {
  DWORD    magic;                // 'FILE' in that order
  WORD     fixup_off;
  WORD     fixup_count;
  DWORD64  last_op_lsn;          // LSN of last suboperation affecting this
  WORD     sequ_number;
  WORD     hard_link_count;
  WORD     attribs_off;
  WORD     flags;                // bit0 = nonresident attribs, bit1= directory
  DWORD    record_len;           // rounded up to 8 bytes (len = file_rec + all attribs)
  DWORD    alloc_size;
  DWORD64  base_file_ref;
  WORD     next_attrib_id;
};

// File Attribute
#define NTFS_ATTR_STANDARD   0x00000010
#define NTFS_ATTR_LIST       0x00000020
#define NTFS_ATTR_FILENAME   0x00000030
#define NTFS_ATTR_VOL_VER    0x00000040
#define NTFS_ATTR_SECURITY   0x00000050
#define NTFS_ATTR_VOL_NAME   0x00000060
#define NTFS_ATTR_VOL_INFO   0x00000070
#define NTFS_ATTR_DATA       0x00000080
#define NTFS_ATTR_INDX_ROOT  0x00000090
#define NTFS_ATTR_INDX_ALLOC 0x000000A0
#define NTFS_ATTR_BITMAP     0x000000B0
#define NTFS_ATTR_SYMBOLIC   0x000000C0
#define NTFS_ATTR_EA_INFO    0x000000D0
#define NTFS_ATTR_EA         0x000000E0
#define NTFS_ATTR_EOA        0xFFFFFFFF

// Attribute header
struct S_NTFS_ATTR {
  DWORD    type;              // type above
  DWORD    len;               // length of attribute
  BYTE     non_res_flag;      // 0 = resident, 1 = non resident
  BYTE     name_len;          // 
  WORD     content_off;       //
  WORD     comp_flag;         //
  WORD     id;                // 
};

// Attribute header (resident)
struct S_NTFS_ATTR_RES {
  DWORD    data_len;
  WORD     data_off;
  BYTE     indexed_flag;
  BYTE     padding;
};

// Attribute header (non-resident)
struct S_NTFS_ATTR_NONRES {
  DWORD64  starting_vcn;
  DWORD64  last_vcn;
  WORD     run_list_off;
  WORD     comp_engine_num;
  DWORD    unused;
  DWORD64  allocated_size;
  DWORD64  real_size;
  DWORD64  init_data_size;
};

// standard attribute struct
struct S_NTFS_ATTR_STANDARD {
  struct S_NTFS_FILE_TIMES file_times;
  DWORD    dos_permissions;
  DWORD    max_ver_num;
  DWORD    ver_num;
  DWORD    class_id;
  DWORD    owner_id;
  DWORD    security_id;
  DWORD64  quota_changed;
  DWORD64  usn;
};

#define NTFS_FILENAME_ATTRB_DIR     0x0000000010000000
#define NTFS_FILENAME_ATTRB_COMP    0x0000000000000800
#define NTFS_FILENAME_ATTRB_ARCH    0x0000000000000020
#define NTFS_FILENAME_ATTRB_SYS     0x0000000000000004
#define NTFS_FILENAME_ATTRB_HIDDEN  0x0000000000000002
#define NTFS_FILENAME_ATTRB_READ_O  0x0000000000000001

//filename space value
// 0  POSIX space: any Unicode character but 0 (null) and '/'. Names are case sensitive. 
// 1  Win32 space: sub-space of the POSIX space, characters '"' '*' '/' ':' '<' '>' '?' '\' '|' are forbidden.
//    Names can't end with a '.' or a ' ' and are case insensitive.
// 2  DOS® space: sub-space of the Win32 space, allowing only 8 bit characters greater than ' ' and different 
//    from '"' '*' '+' ',' ':' ';' '<' '=' '>' '?' '\'. Names must match the following pattern: 
//    1 to 8 characters, then '.', then 1 to 3 characters. Letters must be in uppercase.
// 3  Both Win32 and DOS® spaces: The provided Win32 name already belongs to the DOS® space, so there is no 
//    need to generate an additional DOS® name.

#define NTFS_GET_FILE_REF(r)  (r & 0x0000FFFFFFFFFFFFL)
#define NTFS_GET_FILE_SEQ(r) ((r & 0xFFFF000000000000L) >> 48)

// filename attribute struct
struct S_NTFS_ATTR_FILENAME {
  DWORD64  file_ref_parent;   // high 16-bit = sequ, low 48-bit = reference (use NTFS_GET_FILE_xxx() above)
  struct S_NTFS_FILE_TIMES file_times;
  DWORD64  allocated_file_size;
  DWORD64  real_file_size;
  DWORD64  flags;
  BYTE     filename_len;
  BYTE     filename_space;
};

// vol_ver attribute struct
struct S_NTFS_ATTR_VOL_VER {
  BYTE    unknown[16];
};

// vol_info attribute struct
struct S_NTFS_ATTR_VOL_INFO {
  BYTE     zeros0[8];
  BYTE     major;
  BYTE     minor;
  WORD     flags;
  DWORD    pad;
};

// index_root attribute struct
struct S_NTFS_ATTR_INDX_ROOT {
  DWORD    thirty;
  DWORD    one;
  DWORD    size_indx_rec;
  DWORD    clusters_rec;   // clusters per index record
  DWORD    ten;
  DWORD    entry_size;
  DWORD    again;
  WORD     indx_alloc_flag;  // 0 = no, 1 = yes
  WORD     flags;
};

// index_alloc attribute struct
struct S_NTFS_ATTR_INDX_ALLOC {
  DWORD    magic;  // 'INDX'
  WORD     fixup_off;
  WORD     fixup_cnt;
  DWORD64  lsn;
  DWORD64  vcn_of_buffer;
  WORD     hdr_size;
  WORD     unused0;
  DWORD    inuse_len;
  DWORD    tot_len;
  DWORD    unused1;
};

// cannot be > 240 bytes
struct S_NTFS_ITEMS {
  //HTREEITEM hParent;    // parent directory so we can load it for this entry
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  unsigned mft_entry;
  DWORD ErrorCode;
  DWORD Flags;
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CNTFS dialog
class CNTFS : public CPropertyPage {
  DECLARE_DYNCREATE(CNTFS)

// Construction
public:
  CNTFS();
  ~CNTFS();

// Dialog Data
  //{{AFX_DATA(CNTFS)
  enum { IDD = IDD_NTFS };
  CMyTreeCtrl m_dir_tree;
  CString m_cluster_index_block;
  CString m_cluster_rec_size;
  CString m_bytes_sect;
  CString m_descriptor;
  CString m_heads;
  CString m_hidden_sect;
  CString m_jmp0;
  CString m_jmp1;
  CString m_jmp2;
  CString m_oem_name;
  CString m_sect_cluster;
  CString m_sect_reserved;
  CString m_sect_track;
  CString m_serial_num;
  CString m_mft_cluster;
  CString m_mft_mirror_cluster;
  CString m_sectors;
  //}}AFX_DATA

  DWORD GetNewColor(int index);
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  void ParseDir(unsigned start, DWORD64 ref, HTREEITEM parent);
  struct S_NTFS_FILE_REC *GetMFT(unsigned index);
  void FixupMFT(const BOOL read);
  void WriteMFT(void);
  BOOL GetFileAttrbs(struct S_NTFS_FILE_REC *file_rec, char *filename_str, DWORD64 *date_time, DWORD64 *fsize, DWORD *attribs, DWORD64 *parent);
  
  void SendToDialog(struct S_NTFS_BPB *bpb);
  void ReceiveFromDialog(struct S_NTFS_BPB *bpb);
  void SaveItemInfo(HTREEITEM hItem, unsigned ref, DWORD ErrorCode, DWORD flags, BOOL CanCopy);
  void *ReadFile(const unsigned ref, DWORD64 *FileSize);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  //void InsertFile(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  //void InsertFolder(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot);
  //void DeleteFile(HTREEITEM hItem);
  //void DeleteFolder(HTREEITEM hItem);
  
  void *m_bpb_buffer;
  int   m_rec_size;
  
  void   *m_mft_buffer;
  DWORD64 m_mft_lba;
  unsigned m_mft_start;
  unsigned m_last_mft;
  unsigned m_mft_cur;
  BOOL     m_mft_dirty;
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  BOOL      m_too_many;
  
  BOOL    m_isvalid;
  int     m_index; // index into dlg->NTFS[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  BOOL    m_hard_format;
  
  BOOL    m_del_clear;

// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CNTFS)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CNTFS)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnUpdateCode();
  afx_msg void OnApplyB();
  afx_msg void OnClean();
  afx_msg void OnFormat();
  afx_msg void OnCheck();
  afx_msg void OnCopy();
  afx_msg void OnView();
  afx_msg void OnInsert();
  afx_msg void OnEntry();
  afx_msg void OnDelClear();
  afx_msg void OnFysosSig();
  afx_msg void OnChangeClusterRecSize();
  afx_msg void OnShowSysMft();
  afx_msg void OnSearch();
  afx_msg void OnErase();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NTFS_H__10789641_6A20_4C8C_B938_CF35BDC45E53__INCLUDED_)
