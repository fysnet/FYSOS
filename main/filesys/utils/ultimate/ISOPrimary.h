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

#if !defined(AFX_ISOPRIMARY_H__588672E1_1003_41C7_B8F0_45300AC10AFC__INCLUDED_)
#define AFX_ISOPRIMARY_H__588672E1_1003_41C7_B8F0_45300AC10AFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyImageList.h"
#include "MyTreeCtrl.h"


// Build ISO9660
#define PVD_SECT         16  // 
#define BVD_SECT         17  // 
#define TVD_SECT         18  // Terminator Descriptor is at 18
#define BOOT_CAT_SECT    19  // our boot catalog sector is at 19
#define BOOT_IMG_SECT    20  // our boot image is at 20
#define PATH_SECT_SIZE    1  // path size in sectors
#define ROOT_SECT_SIZE    1  // root size in sectors



#pragma pack(push, 1)

// Since CDROM's are "universal" to all platforms, if a value stored
//  in one of the following structures is more than a byte, the value
//  is stored twice.  The first being little_endian, the second, big_endian.

struct S_VOL_DATE {
  BYTE   since_1900;
  BYTE   month;
  BYTE   day;
  BYTE   hour;
  BYTE   min;
  BYTE   sec;
  INT8   gmt_off;
};

// this struct is in character format.  For example, year[4] is stored as "2011"
struct S_ISO_DATE_TIME {
  char   year[4];
  char   month[2];
  char   day[2];
  char   hour[2];
  char   min[2];
  char   sec[2];
  char   jiffies[2];
  INT8   gmt_off;
};

#define ISO_ROOT_FLAGS_EXISTS  (1<<0) // If set to ZERO, shall mean that the existence of the file shall be made known to the user upon an inquiry by the user.
#define ISO_ROOT_FLAGS_DIR     (1<<1) // If set to ONE, shall mean that the Directory Record identifies a directory.
#define ISO_ROOT_FLAGS_ASSOC   (1<<2) // If set to ONE, shall mean that the file is an Associated File.
#define ISO_ROOT_FLAGS_RECORD  (1<<3) // If set to ONE, shall mean that the structure of the information in the file has a record format specified by a number 
                                      //   other than zero in the Record Format Field of the Extended Attribute Record (see 9.5.8).
#define ISO_ROOT_FLAGS_PROT    (1<<4) // If set to ONE, shall mean that:
                                      //  - an Owner Identification and a Group Identification are specified for the file (see 9.5.1 and 9.5.2);
                                      //  - at least one of the even-numbered bits or bit 0 in the Permissions field of the associated Extended Attribute Record is set to ONE (see 9.5.3).
#define ISO_ROOT_FLAGS_MULTI_EXT (1<<7) // If set to ZERO, shall mean that this is the final Directory Record for the file.

#define  ISO_NO_ERROR       0   // no error found
#define  ISO_BAD_ATTRIBUTE  1   // bad attribute value
#define  ISO_BAD_ID_LEN     2   // id len of zero or more than 30
#define  ISO_BAD_IDENT      3   // bad identifier found

struct S_ISO_ROOT {
  BYTE   len;
  BYTE   e_attrib;
  DWORD  extent_loc;
  DWORD  extent_loc_b;
  DWORD  data_len;
  DWORD  data_len_b;
  struct S_VOL_DATE date;
  BYTE   flags;
  BYTE   unit_size;
  BYTE   gap_size;
  WORD   sequ_num;
  WORD   sequ_num_b;
  BYTE   fi_len;
  BYTE   ident[1];  // d-characters (A-Z, 0-9, _  only) + '.' and ';' (at most 30 chars not counting ECMA-119 specs)
};

/*
struct S_ISO_PATH {
  BYTE   len;
  BYTE   e_attrib;
  WORD   extent_loc;
  WORD   extent_loc_b;
  BYTE   parent;
  BYTE   parent_b;
  struct S_ISO_ROOT dir_rec;
};
*/

struct S_ISO_PATH {
  BYTE   len_di;
  BYTE   ext_attrib;
  DWORD  loc;
  WORD   parent;
  BYTE   ident[16];
};

struct S_ISO_PVD {
  BYTE   type;      // 1
  char   ident[5];  // CD001
  BYTE   ver;
  BYTE   resv0;     // unused
  char   sys_ident[32];
  char   vol_ident[32];
  BYTE   resv1[8];
  DWORD  num_lbas;
  DWORD  num_lbas_b;
  BYTE   resv2[32];        // type 1: unused
  WORD   set_size;
  WORD   set_size_b;
  WORD   sequ_num;
  WORD   sequ_num_b;
  WORD   lba_size;   // sector size in bytes
  WORD   lba_size_b;
  DWORD  path_table_size;
  DWORD  path_table_size_b;
  DWORD  pathl_loc;
  DWORD  pathlo_loc;
  DWORD  pathm_loc;
  DWORD  pathmo_loc;
  struct S_ISO_ROOT root; 
  char   set_ident[128];
  char   pub_ident[128];
  char   prep_ident[128];
  char   app_ident[128];
  char   copy_ident[37];
  char   abs_ident[37];
  char   bib_ident[37];
  struct S_ISO_DATE_TIME vol_date;
  struct S_ISO_DATE_TIME mod_date;
  struct S_ISO_DATE_TIME exp_date;
  struct S_ISO_DATE_TIME val_date;
  BYTE   struct_ver;
  BYTE   resv3;
  BYTE   app_use[512];
  BYTE   resv4[653];
};

struct S_ISO_TERM {
  BYTE   id;
  char   ident[5];
  BYTE   ver;
  BYTE   resv1[2041];
};

// cannot be > MAX_ITEM_SIZE bytes
struct S_ISO_ITEMS {
  BOOL  CanCopy;        // the entry is not a deleted/invalid/other that we can copy out to the host
  BYTE RootEntry[34 + 256];
  DWORD Flags;
  //struct S_ISO_ROOT root;
  //BYTE rest_of_ident[256];  
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CISOPrimary dialog

class CISOPrimary : public CPropertyPage {
  DECLARE_DYNCREATE(CISOPrimary)

// Construction
public:
  CISOPrimary();
  ~CISOPrimary();

// Dialog Data
  //{{AFX_DATA(CISOPrimary)
  enum { IDD = IDD_ISO_PVD };
  CMyTreeCtrl m_dir_tree;
  int   m_type;
  CString m_cd001;
  CString m_version;
  CString m_num_lbas;
  CString m_opath_table_loc;
  CString m_path_table_loc;
  CString m_path_table_size;
  CString m_sequence_num;
  CString m_set_size;
  CString m_sys_id;
  CString m_vol_id;
  CString m_block_size;
  CString m_abs_id;
  CString m_app_id;
  CString m_bib_id;
  CString m_copy_id;
  CString m_prep_id;
  CString m_pub_id;
  CString m_set_id;
  CString m_struct_ver;
  //}}AFX_DATA
  
  void Start(const DWORD64 lba, DWORD color, BOOL IsNewTab);
  void DoRoot(void);
  void *ReadFile(DWORD extent, DWORD size, BYTE Flags, BOOL IsRoot);
  DWORD CheckRootEntry(struct S_ISO_ROOT *r);
  bool ParseDir(struct S_ISO_ROOT *root, DWORD datalen, HTREEITEM parent, BOOL IsRoot);
  void SaveItemInfo(HTREEITEM hItem, struct S_ISO_ROOT *root, DWORD flags, DWORD ErrorCode, BOOL CanCopy);
  
  void SendToDialog(const BOOL update);
  void ReceiveFromDialog(void);
  
  void CopyFile(HTREEITEM hItem, CString csName);
  void CopyFolder(HTREEITEM hItem, CString csPath, CString csName);
  
  bool    m_is_valid;
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  int     m_parse_depth_limit; // used to catch a repeating '..' or '.' recursive error
  
  BYTE  m_descriptor[2048];
  
  CMyImageList m_TreeImages;
  HTREEITEM m_hRoot;
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CISOPrimary)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CISOPrimary)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnApplyB();
  afx_msg void OnVolDate();
  afx_msg void OnModDate();
  afx_msg void OnExpDate();
  afx_msg void OnEffectDate();
  afx_msg void OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnEntry();
  afx_msg void OnCopy();
  afx_msg void OnView();
  afx_msg void OnInsert();
  afx_msg void OnISOCheck();
  afx_msg void OnExpand();
  afx_msg void OnCollapse();
  afx_msg void OnSearch();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISOPRIMARY_H__588672E1_1003_41C7_B8F0_45300AC10AFC__INCLUDED_)
