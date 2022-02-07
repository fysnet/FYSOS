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

#if !defined(AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_)
#define AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MbrEntry.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMbrEntry dialog

class CMbrEntry : public CPropertyPage {
  DECLARE_DYNCREATE(CMbrEntry)

// Construction
public:
  CMbrEntry();
  ~CMbrEntry();

// Dialog Data
  //{{AFX_DATA(CMbrEntry)
  enum { IDD = IDD_MBR_ENTRY };
  CString	m_begin_cyl;
  CString	m_begin_head;
  CString	m_begin_sect;
  CString	m_boot_id;
  CString	m_end_cyl;
  CString	m_end_head;
  CString	m_end_sect;
  CString m_size;
  CString m_start_lba;
  CString	m_sys_id;
  //}}AFX_DATA
  
  CString m_Title;
  int    m_index;
  BOOL   m_dirty;
  void  *m_parent;  // pointer to parent CMBR
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CMbrEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CMbrEntry)
  afx_msg void OnMbrBeginUpdate();
  afx_msg void OnMbrEndUpdate();
  afx_msg void OnMbrApply();
  afx_msg void OnChangeMbrSysId();
  virtual BOOL OnInitDialog();
  afx_msg void OnTabItemChange();
  afx_msg void OnTabItemChangeSize();
  afx_msg void OnSysId();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  void LBAtoCHS(const DWORD lba, WORD *cyl, BYTE *head, BYTE *sect);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MBRENTRY_H__CE4DA7DF_FB65_4EAE_974C_F70059D38CCB__INCLUDED_)
