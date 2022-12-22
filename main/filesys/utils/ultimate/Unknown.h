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

#if !defined(AFX_UNKNOWN_H__51679AF6_0C29_4B6C_9E5A_8287B836030F__INCLUDED_)
#define AFX_UNKNOWN_H__51679AF6_0C29_4B6C_9E5A_8287B836030F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Unknown.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUnknown dialog

class CUnknown : public CPropertyPage
{
  DECLARE_DYNCREATE(CUnknown)

// Construction
public:
  CUnknown();
  ~CUnknown();

// Dialog Data
  //{{AFX_DATA(CUnknown)
  enum { IDD = IDD_UNKNOWN };
  CString	m_dump;
  //}}AFX_DATA
  
  void Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab);
  void FormatLean(void);
  void FormatFat(int Type);
  void FormatExFat(void);
  void FormatFYSFS(void);
  void FormatSFS(void);
  void FormatExt2(void);
  
  int     m_index; // index into dlg->Unknown[]
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  
  DWORD64 m_current;  // current LSN of dump sector

  // the list of file system counts
  struct S_DET_COUNTS m_det_counts;

  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CUnknown)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CUnknown)
  virtual BOOL OnInitDialog();
  afx_msg void OnClean();
  afx_msg void OnFormat();
  afx_msg void OnInsert();
  afx_msg void OnInsertFAT16BPB();
  afx_msg void OnInsertFAT12BPB();
  afx_msg void OnDumpFirst();
  afx_msg void OnDumpPrev();
  afx_msg void OnDumpNext();
  afx_msg void OnDumpLast();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNKNOWN_H__51679AF6_0C29_4B6C_9E5A_8287B836030F__INCLUDED_)
