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

#if !defined(AFX_EXT2ENTRY_H__E6C0906E_1857_40F2_9C96_3930B8789565__INCLUDED_)
#define AFX_EXT2ENTRY_H__E6C0906E_1857_40F2_9C96_3930B8789565__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ext2Entry.h : header file
//

#include "Ext2.h"

/////////////////////////////////////////////////////////////////////////////
// CExt2Entry dialog

class CExt2Entry : public CDialog {
// Construction
public:
  CExt2Entry(CWnd* pParent = NULL);   // standard constructor
  
// Dialog Data
  //{{AFX_DATA(CExt2Entry)
  enum { IDD = IDD_EXT2_ENTRY };
  CListBox  m_list;
  CString m_atime;
  CString m_blocks;
  CString m_ctime;
  CString m_dir_acl;
  CString m_dtime;
  CString m_f_addr;
  CString m_file_acl;
  CString m_flags;
  CString m_generation;
  CString m_gid;
  CString m_link_count;
  CString m_mode;
  CString m_mtime;
  CString m_osd1;
  CString m_size;
  CString m_uid;
  CString m_sym_link;
  CString m_dbl_indirect;
  CString m_depth;
  CString m_entries;
  CString m_extent_gen;
  CString m_indirect;
  CString m_magic;
  CString m_max;
  CString m_trp_indirect;
  //}}AFX_DATA
  
  struct S_EXT2_INODE m_inode;
  unsigned m_sectors_per_block;
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CExt2Entry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CExt2Entry)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnDomode();
  afx_msg void OnDoflags();
  afx_msg void OnAccTimeNow();
  afx_msg void OnCreTimeNow();
  afx_msg void OnModTimeNow();
  afx_msg void OnDelTimeNow();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXT2ENTRY_H__E6C0906E_1857_40F2_9C96_3930B8789565__INCLUDED_)
