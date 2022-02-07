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

#if !defined(AFX_LEANENTRY_H__4FFAF00C_12A4_4350_B5A4_8F7DA054DD6F__INCLUDED_)
#define AFX_LEANENTRY_H__4FFAF00C_12A4_4350_B5A4_8F7DA054DD6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeanEntry.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLeanEntry dialog

class CLeanEntry : public CDialog {
// Construction
public:
  CLeanEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CLeanEntry)
  enum { IDD = IDD_LEAN_ENTRY };
  CListBox	m_ext_start;
  CListBox	m_ext_size;
  CString	m_acc_time;
  CString	m_attribs;
  CString	m_cre_time;
  CString	m_ext_count;
  CString	m_filesize;
  CString	m_first_ind;
  CString	m_fork;
  CString	m_gid;
  CString	m_ind_count;
  CString	m_last_indirect;
  CString	m_links_count;
  CString	m_magic;
  CString	m_mod_time;
  CString	m_sch_time;
  CString	m_block_count;
  CString	m_uid;
  CString	m_entry_crc;
  CString	m_name;
  //}}AFX_DATA
  
  struct S_LEAN_INODE m_inode;
  CLean    *m_parent;
  HTREEITEM m_hItem;
  DWORD64 m_inode_num;
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CLeanEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CLeanEntry)
  virtual BOOL OnInitDialog();
  afx_msg void OnSelchangeExtStart();
  afx_msg void OnSelchangeExtSize();
  afx_msg void OnAttribute();
  afx_msg void OnCrcUpdate();
  afx_msg void OnAccTimeNow();
  afx_msg void OnCreTimeNow();
  afx_msg void OnModTimeNow();
  afx_msg void OnSchTimeNow();
  afx_msg void OnEas();
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEANENTRY_H__4FFAF00C_12A4_4350_B5A4_8F7DA054DD6F__INCLUDED_)
