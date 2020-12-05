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

#if !defined(AFX_MBR_H__3FBE0389_F125_40C7_9B32_BBDD00200F19__INCLUDED_)
#define AFX_MBR_H__3FBE0389_F125_40C7_9B32_BBDD00200F19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Mbr.h : header file
//

#include "MbrEntry.h"

#pragma pack(push, 1)

struct MBR_CHS_ENTRY {
  BYTE  head;
  BYTE  sector;
  BYTE  cyl;
};

struct MBR_PART_ENTRY {
  BYTE  boot_id;
  struct MBR_CHS_ENTRY start;
  BYTE  sys_id;
  struct MBR_CHS_ENTRY end;
  DWORD start_lba;
  DWORD sectors;
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CMbr dialog

class CMbr : public CPropertyPage {
  DECLARE_DYNCREATE(CMbr)

// Construction
public:
  CMbr();
  ~CMbr();
  
  bool Exists(DWORD64 LBA);
  void Begin(void);
  void UpdateEntry(CMbrEntry *Entry, int index);
  void Destroy(void);
  bool IsPMBR(void);
  
  CString m_Title;
  int     m_index;   // index of MBR[x]
  
  bool  m_exists;
  DWORD m_color;
  DWORD64 m_lba;
  DWORD64 m_parent_lba;  // in extended MBRs, this is the LBA of the parent MBR
  int   m_draw_index;
  
  BYTE   m_buffer[MAX_SECT_SIZE];
  
  CPropertySheet m_Sheet;
  CMbrEntry  m_Pages[4];
  
// Dialog Data
  //{{AFX_DATA(CMbr)
  enum { IDD = IDD_MBR };
  CStatic	m_mbr_pages;
  CString	m_MbrDump;
  CString	m_spt;
  CString	m_head_count;
  CString	m_id_sig;
  CString	m_id_zero;
  //}}AFX_DATA


// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CMbr)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CMbr)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnMbrApply();
  afx_msg void OnExtractMbr();
  afx_msg void OnUpdateMbr();
  afx_msg void OnUpdateMbrSig();
  afx_msg void OnIdSigUpdate();
  afx_msg void OnIdZeroUpdate();
  afx_msg void OnPrependMBR();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MBR_H__3FBE0389_F125_40C7_9B32_BBDD00200F19__INCLUDED_)
