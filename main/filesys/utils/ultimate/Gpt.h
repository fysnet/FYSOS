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

#if !defined(AFX_GPT_H__A4B5F5F6_FC0E_4DE0_BBB3_E9C5A9C583A6__INCLUDED_)
#define AFX_GPT_H__A4B5F5F6_FC0E_4DE0_BBB3_E9C5A9C583A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Gpt.h : header file
//

#include "GptEntry.h"

#pragma pack(push, 1)

// EFI GUID Partition Table
// page 11-9 of EFISpec_v110.pdf
struct S_GPT_HDR {
  BYTE    sig[8];
  DWORD   version;
  DWORD   hdr_size;
  DWORD   crc32;   // only bytes 0 -> hdr_size are checked
  DWORD   resv0;
  DWORD64 primary_lba;  // my LBA
  DWORD64 backup_lba;   // alternate LBA
  DWORD64 first_usable;
  DWORD64 last_usable;
  struct  S_GUID guid;
  DWORD64 entry_offset;
  DWORD   entries;
  DWORD   entry_size;
  DWORD   crc32_entries;
  BYTE    reserved[420];
};

struct S_GPT_ENTRY {
  struct S_GUID guid_type;
  struct S_GUID guid;
  DWORD64 first_lba;
  DWORD64 last_lba;
  DWORD64 attribute;
  WORD    name[36];
};

#pragma pack(pop)

#define MAX_GPT_ENTRIES  16

/////////////////////////////////////////////////////////////////////////////
// CGpt dialog

class CGpt : public CPropertyPage
{
  DECLARE_DYNCREATE(CGpt)

// Construction
public:
  CGpt();
  ~CGpt();
  
  bool Exists(DWORD64 LBA);
  void Begin(void);
  void UpdateEntry(CGptEntry *Entry, int index, const int entry_size);
  BOOL CheckEntry(int index);
  void Destroy(void);
  
  void GetGPTHdr(void);
  void GetGPTEntries(void);
  BOOL CheckGPT(DWORD64 LBA, BOOL CheckAlternate);

  bool  m_exists;
  DWORD m_color;
  DWORD64 m_lba;
  int   m_draw_index;
  
  BYTE   m_buffer[MAX_SECT_SIZE];
  struct S_GPT_HDR m_hdr;
  struct S_GPT_ENTRY *m_entry_buffer;
  int    m_gpt_entries;
  
  CPropertySheet m_Sheet;
  CGptEntry  m_Pages[MAX_GPT_ENTRIES];
  
// Dialog Data
  //{{AFX_DATA(CGpt)
  enum { IDD = IDD_GPT };
  CStatic	m_gpt_pages;
  CString	m_GptDump;
  CString	m_backup_lba;
  CString	m_crc32;
  CString	m_entry_count;
  CString	m_entry_crc32;
  CString	m_entry_offset;
  CString	m_entry_size;
  CString	m_first_lba;
  CString	m_gpt_guid;
  CString	m_hdr_size;
  CString	m_last_lba;
  CString	m_primary_lba;
  CString	m_rsvd;
  CString	m_gpt_sig;
  CString	m_gpt_version;
  CString	m_sig_chars;
  CString	m_version_num;
  //}}AFX_DATA


// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CGpt)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CGpt)
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnCrcButton();
  afx_msg void OnKillfocusGptGuid();
  afx_msg void OnGuidCreate();
  afx_msg void OnGPTBackup();
  afx_msg void OnGPTFromBackup();
  afx_msg void OnGPTTotalCheck();
public:
  afx_msg void OnGptApply();
  afx_msg void OnEcrcButton();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GPT_H__A4B5F5F6_FC0E_4DE0_BBB3_E9C5A9C583A6__INCLUDED_)
