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

// ISOEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "ISOEntry.h"
#include "ISOPrimary.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CISOEntry dialog


CISOEntry::CISOEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CISOEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CISOEntry)
  m_attribute = _T("");
  m_data_len = _T("");
  m_extent_loc = _T("");
  m_fi_len = _T("");
  m_flags = _T("");
  m_gap_size = _T("");
  m_ident = _T("");
  m_length = _T("");
  m_sequ_num = _T("");
  m_unit_size = _T("");
  m_day = 0;
  m_gmt_off = 0;
  m_hour = 0;
  m_minute = 0;
  m_month = 0;
  m_second = 0;
  m_year = 0;
  m_ident_extra = _T("");
  //}}AFX_DATA_INIT
}

void CISOEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CISOEntry)
  DDX_Text(pDX, IDC_ATTRIBUTE, m_attribute);
  DDX_Text(pDX, IDC_DATA_LEN, m_data_len);
  DDX_Text(pDX, IDC_EXTENT_LOC, m_extent_loc);
  DDX_Text(pDX, IDC_FILENGTH, m_fi_len);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_GAP_SIZE, m_gap_size);
  DDX_Text(pDX, IDC_IDENT, m_ident);
  DDX_Text(pDX, IDC_LENGTH, m_length);
  DDX_Text(pDX, IDC_SEQU_NUM, m_sequ_num);
  DDX_Text(pDX, IDC_UNIT_SIZE, m_unit_size);
  DDX_Text(pDX, IDC_DAY, m_day);
  DDV_MinMaxInt(pDX, m_day, 1, 31);
  DDX_Text(pDX, IDC_GMT_OFF, m_gmt_off);
  DDV_MinMaxInt(pDX, m_gmt_off, -22, 22);
  DDX_Text(pDX, IDC_HOUR, m_hour);
  DDV_MinMaxInt(pDX, m_hour, 0, 23);
  DDX_Text(pDX, IDC_MINUTE, m_minute);
  DDV_MinMaxInt(pDX, m_minute, 0, 59);
  DDX_Text(pDX, IDC_MONTH, m_month);
  DDV_MinMaxInt(pDX, m_month, 1, 12);
  DDX_Text(pDX, IDC_SECOND, m_second);
  DDV_MinMaxInt(pDX, m_second, 0, 59);
  DDX_Text(pDX, IDC_YEAR, m_year);
  DDV_MinMaxInt(pDX, m_year, 1900, 2500);
  DDX_Text(pDX, IDC_IDENT_EXTRA, m_ident_extra);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CISOEntry, CDialog)
  //{{AFX_MSG_MAP(CISOEntry)
  ON_BN_CLICKED(IDC_FLAGS_B, OnFlagsB)
  ON_BN_CLICKED(IDC_EXTRA_COPY, OnExtraCopy)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CISOEntry message handlers
BOOL CISOEntry::OnInitDialog() {
  CDialog::OnInitDialog();
  
  if (m_ident_extra_cnt > 0) {
    // set the font of the DUMP window to a fixed font
    CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
    GetDlgItem(IDC_IDENT_EXTRA)->SetFont(&dlg->m_DumpFont);
    GetDlgItem(IDC_IDENT_EXTRA)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_IDENT_EXTRA_S)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_EXTRA_COPY)->ShowWindow(SW_SHOW);
  } else {
    GetDlgItem(IDC_IDENT_EXTRA)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_IDENT_EXTRA_S)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_EXTRA_COPY)->ShowWindow(SW_HIDE);
  }
  
  return TRUE;
}

struct S_ATTRIBUTES iso_attrbs[] = {
                    //            |                               | <- max (col 67)
  { ISO_ROOT_FLAGS_EXISTS,     ISO_ROOT_FLAGS_EXISTS,    0, "Hidden"                         , {-1, } },
  { ISO_ROOT_FLAGS_DIR,        ISO_ROOT_FLAGS_DIR,       1, "Directory"                      , {2, 3, 5, -1, } },
  { ISO_ROOT_FLAGS_ASSOC,      ISO_ROOT_FLAGS_ASSOC,     2, "Associated File"                , {1, -1, } },
  { ISO_ROOT_FLAGS_RECORD,     ISO_ROOT_FLAGS_RECORD,    3, "Record"                         , {1, -1, } },
  { ISO_ROOT_FLAGS_PROT,       ISO_ROOT_FLAGS_PROT,      4, "Protection"                     , {-1, } },
  { ISO_ROOT_FLAGS_MULTI_EXT,  ISO_ROOT_FLAGS_MULTI_EXT, 5, "Multi-Extent"                   , {1, -1, } },
  { 0,                          (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CISOEntry::OnFlagsB() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FLAGS, cs);
  dlg.m_title = "Entry Flags";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = iso_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%02X", dlg.m_attrib);
    SetDlgItemText(IDC_FLAGS, cs);
  }
}

// copy the contents of Extra to the clipboard
void CISOEntry::OnExtraCopy() {
  CEdit *edit = (CEdit *) GetDlgItem(IDC_IDENT_EXTRA);
  edit->SetSel(0, -1, TRUE);
  edit->Copy();
}
