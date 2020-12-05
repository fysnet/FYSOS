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

#if !defined(AFX_FATENTRY_H__5170C875_4DE7_442A_9FAF_0AA5E03CC2A7__INCLUDED_)
#define AFX_FATENTRY_H__5170C875_4DE7_442A_9FAF_0AA5E03CC2A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FatEntry.h : header file
//

#include "Fat.h"

/////////////////////////////////////////////////////////////////////////////
// CFatEntry dialog

class CFatEntry : public CDialog
{
// Construction
public:
  CFatEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CFatEntry)
  enum { IDD = IDD_FAT_ENTRY };
  CString	m_data_disp;
  CString	m_attrib;
  CString	m_cluster;
  CString	m_date;
  CString	m_ext;
  CString	m_filesize;
  CString	m_lfn_attrb;
  CString	m_lfn_cluster;
  CString	m_lfn_resv;
  CString	m_name;
  CString	m_lfn_name;
  CString	m_resvd;
  CString	m_sequ;
  CString	m_sfn_crc;
  CString	m_time;
  CString	m_sequ_disp;
  CString	m_time_disp;
  CString	m_error_code;
  //}}AFX_DATA
  
  void LFNtoDialog();
  void DialogtoLFN();
  
  struct S_FAT_ROOT m_sfn;
  void *m_lfns;
  int   m_lfn_count;
  int   m_lfn_cur;
  int   m_fat_size;
  BOOL  m_lfn_deleted;
  BOOL  m_show_bytes;
  
  struct S_FAT_ENTRIES m_fat_entries;
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CFatEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CFatEntry)
  afx_msg void OnDate();
  afx_msg void OnDellfn();
  afx_msg void OnLFNRClear();
  afx_msg void OnNext();
  afx_msg void OnPrev();
  afx_msg void OnTime();
  afx_msg void OnSFNRClear();
  afx_msg void OnChangeEntryDate();
  afx_msg void OnChangeEntryTime();
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnAttribute();
  afx_msg void OnFatentries();
  afx_msg void OnCrcUpdate();
  afx_msg void OnShowBytes();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FATENTRY_H__5170C875_4DE7_442A_9FAF_0AA5E03CC2A7__INCLUDED_)
