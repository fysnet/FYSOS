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

#if !defined(AFX_ISOBOOT_H__ABC3C8F3_34B3_4500_80A3_74B9AED8E6DA__INCLUDED_)
#define AFX_ISOBOOT_H__ABC3C8F3_34B3_4500_80A3_74B9AED8E6DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ISOBoot.h : header file
//
#pragma pack(push, 1)

struct S_ISO_BVD {
  BYTE   type;      // 0
  char   ident[5];  // CD001
  BYTE   ver;       // 
  char   sys_ident[32];
  char   boot_ident[32];
  DWORD  boot_cat;
  BYTE   sys_use[1973];
};

struct S_ISO_BC_VALIDATION {
  BYTE   id;
  BYTE   platform;
  WORD   resv0;
  char   ident[24];
  WORD   crc;         // word offset 14
  BYTE   key55;
  BYTE   keyAA;
};

struct S_ISO_BC_ENTRY_EXT {
  BYTE   bootable;
  BYTE   media;               // 0 = no boot image, standard image only
                              // 1 = 1.22m
                              // 2 = 1.44m
                              // 3 = 2.88m
                              // 4 = hard drive
  WORD   load_seg;            // 0 = 0x07C0, else segment to use
  BYTE   sys_type;
  BYTE   resv0;
  WORD   load_cnt;
  DWORD  load_rba;
  BYTE   resv1[20];
};

struct S_ISO_BC_END_ENTRY {
  BYTE   id;
  BYTE   platform;
  WORD   num;
  BYTE   resv1[28];
};

struct S_ISO_BOOT_CAT {
  struct S_ISO_BC_VALIDATION val_entry;
  struct S_ISO_BC_ENTRY_EXT init_entry;
  struct S_ISO_BC_END_ENTRY end_entry;
  BYTE   filler[1952];
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CISOBoot dialog

class CISOBoot : public CPropertyPage
{
  DECLARE_DYNCREATE(CISOBoot)

// Construction
public:
  CISOBoot();
  ~CISOBoot();

// Dialog Data
  //{{AFX_DATA(CISOBoot)
  enum { IDD = IDD_ISO_BVD };
  CComboBox	m_di_media;
  CComboBox	m_di_bootable;
  CComboBox m_val_platform;
  int     m_type;
  int     m_boot_cat_lba;
  CString	m_cd001;
  CString	m_version;
  CString	m_sys_id;
  CString	m_sys_use;
  CString	m_boot_id;
  CString m_val_crc;
  CString m_val_id;
  CString m_val_ident;
  CString m_val_key55;
  CString m_val_keyaa;
  CString m_val_reserved;
  CString	m_di_count;
  CString	m_di_lba;
  CString	m_di_load_seg;
  CString	m_di_reserved;
  CString	m_di_sys_type;
  //}}AFX_DATA

  void Start(const DWORD64 lba, DWORD color, BOOL IsNewTab);
  void SendToDialog(void);
  void ReceiveFromDialog(void);
  
  bool    m_is_valid;
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  
  BYTE  m_descriptor[MAX_SECT_SIZE];
  BYTE  m_boot_cat[MAX_SECT_SIZE];
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CISOBoot)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CISOBoot)
  afx_msg void OnApplyB();
  virtual BOOL OnInitDialog();
  afx_msg void OnValCrcUpdate();
  afx_msg void OnValKeyUpdate();
  afx_msg void OnDiInsert();
  afx_msg void OnDiExtract();
  afx_msg void OnClearReserved();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISOBOOT_H__ABC3C8F3_34B3_4500_80A3_74B9AED8E6DA__INCLUDED_)
