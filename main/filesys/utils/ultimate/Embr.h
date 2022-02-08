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

#if !defined(AFX_EMBR_H__4692C7EB_8C3C_4273_A1BB_A40B6A0CB407__INCLUDED_)
#define AFX_EMBR_H__4692C7EB_8C3C_4273_A1BB_A40B6A0CB407__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Embr.h : header file
//

#include "EmbrEntry.h"

#define MAX_EMBR_ENTRIES  16

#pragma pack(push, 1)

#define EMBR_SIG_SIG  0x456D627272626D45ULL
struct S_EMBR_SIG {
  DWORD64  sig;
  WORD     offset;     // sectors to S_EMBR_HDR (LBA of S_EMBR_HDR)
  WORD     remaining;
  WORD     boot_sig;
};

#define EMBR_HDR_SIG0   0x52424D45  // EMBR
#define EMBR_HDR_SIG1   0x454D4252  // RBME

struct S_EMBR_HDR {
  DWORD   sig0;          // 'EMBR'
  DWORD   crc;           // crc of this sector
  WORD    entry_count;   // total entries in EMBR (reserved in all but first sector)
  BYTE    boot_delay;    // seconds to delay before booting last booted (reserved in all but first sector)
  BYTE    version;       //  1.05 = 001_00101b
  DWORD64 total_sectors; // total sectors this eMBR encompasses
  BYTE    resv1[8];      // 
  DWORD   sig1;          // 'RBME'
};

#define EMBR_ENTRY_SIG   0x52424D65
#define EMBR_VALID_ENTRY  (1<<0)
#define EMBR_HIDDN_ENTRY  (1<<1)

struct S_EMBR_ENTRY {
  DWORD   flags;       // bit 0 = 1 = valid, bit 1 = 1 = hidden (rest are preserved)
  DWORD   signature;   // 'eMBR'
  DWORD64 starting_sector;
  DWORD64 sector_count;
  BYTE    description[64];
  DWORD64 date_created;      // Secs since 01Jan1980
  DWORD64 date_last_booted;  // Secs since 01Jan1980
  DWORD64 OS_signature;
  BYTE    reserved[16];
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CEmbr dialog

class CEmbr : public CPropertyPage
{
  DECLARE_DYNCREATE(CEmbr)

// Construction
public:
  CEmbr();
  ~CEmbr();
  
  bool Exists(DWORD64 LBA);
  void Begin(void);
  void UpdateEntry(CEmbrEntry *Entry, int index);
  BOOL CheckEntry(int index);
  void Destroy(void);
  
  bool  m_exists;
  DWORD m_color;
  DWORD64 m_lba;
  int   m_draw_index;
  
  bool    m_hdr_valid;
  void   *m_entry_buffer;
  int     m_embr_entries;
  DWORD64 m_total_sectors;
  
  CPropertySheet m_Sheet;
  CEmbrEntry  m_Pages[MAX_EMBR_ENTRIES];
  
// Dialog Data
  //{{AFX_DATA(CEmbr)
  enum { IDD = IDD_EMBR };
  CStatic m_embr_pages;
  CString m_boot_sig;
  CString m_entry_offset;
  CString m_remaining;
  CString m_embr_sig;
  CString m_embr_dump;
  CString m_entry_crc;
  CString m_entries;
  CString m_reserved;
  CString m_sig0;
  CString m_sig1;
  int		  m_boot_delay;
  CString	m_tot_sectors;
  CString	m_version;
  //}}AFX_DATA

  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CEmbr)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CEmbr)
  afx_msg void OnEmbrApply();
  virtual BOOL OnInitDialog();
  afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
  afx_msg void OnBootSigUpdate();
  afx_msg void OnCrcSet();
  afx_msg void OnResvClear();
  afx_msg void OnSig0Set();
  afx_msg void OnSig1Set();
  afx_msg void OnSignatureSet();
  afx_msg void OnUpdateCode();
  afx_msg void OnChangeEmbrVersion();
  afx_msg void OnUpdateTotSects();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMBR_H__4692C7EB_8C3C_4273_A1BB_A40B6A0CB407__INCLUDED_)
