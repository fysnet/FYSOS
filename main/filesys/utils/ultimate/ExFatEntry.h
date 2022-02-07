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

#if !defined(AFX_EXFATENTRY_H__F0EA3CCC_467E_46D8_BBA0_54F3851261E1__INCLUDED_)
#define AFX_EXFATENTRY_H__F0EA3CCC_467E_46D8_BBA0_54F3851261E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExFatEntry.h : header file
//

#include "ExFat.h"

/////////////////////////////////////////////////////////////////////////////
// CExFatEntry dialog

class CExFatEntry : public CDialog {
// Construction
public:
  CExFatEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CExFatEntry)
  enum { IDD = IDD_EXFAT_ENTRY };
  CString m_attribute;
  CString m_crc;
  CString m_created;
  CString m_created_ms;
  CString m_created_tz;
  CString m_data_len;
  CString m_cluster;
  CString m_error_code;
  CString m_name;
  CString m_reserved0;
  CString m_reserved1;
  CString m_reserved2;
  CString m_reserved3;
  CString m_reserved4;
  CString m_flags;
  CString m_last_acc;
  CString m_last_acc_tz;
  CString m_modified;
  CString m_modified_ms;
  CString m_modified_tz;
  CString m_name_hash;
  CString m_name_len;
  CString m_secondaries;
  CString m_valid_data_len;
  CString	m_guid;
  //}}AFX_DATA
  
  BOOL LoadEntry(void);
  void SaveEntry(void);
  void UpdateEntry(void);
  float GetUTCOff(BYTE val);
  BYTE GetUTCVal(float off);
  
  CExFat *m_Parent;
  void *m_entry_buffer;  // holds the whole parent directory block
  DWORD64 m_buffer_size; // size of this directory block in bytes
  DWORD m_root_cluster;  // starting cluster of directory block holding this entry
  DWORD m_root_offset;   // offset within directory block holding this entry
  
  DWORD m_Cluster_Count; // count of clusters in data region
  DWORD m_Cluster;       // 
  DWORD64 m_DataLen;
  BYTE  m_Flags;         // flags sent from parent (usually the flags from the parent directory block)
  BYTE  m_EntryFlags;    // flags of current entry shown in dialog
  
  void *m_ucase_buffer;  // buffer to hold the ucase/bitmap data
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CExFatEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CExFatEntry)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnDoattrib();
  afx_msg void OnFatentries();
  afx_msg void OnFatConvert();
  afx_msg void OnCreated();
  afx_msg void OnLastacc();
  afx_msg void OnLastmod();
  afx_msg void OnClear2();
  afx_msg void OnClear3();
  afx_msg void OnClear4();
  afx_msg void OnClear0();
  afx_msg void OnClear1();
  afx_msg void OnCrcUpdate();
  afx_msg void OnFlags();
  afx_msg void OnChangeCreated();
  afx_msg void OnChangeModified();
  afx_msg void OnChangeLastAcc();
  afx_msg void OnDoucase();
  afx_msg void OnKillfocusEntryGuid();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXFATENTRY_H__F0EA3CCC_467E_46D8_BBA0_54F3851261E1__INCLUDED_)
