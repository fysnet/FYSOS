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

// Settings.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog


CSettings::CSettings(CWnd* pParent /*=NULL*/)
  : CDialog(CSettings::IDD, pParent) {
  //{{AFX_DATA_INIT(CSettings)
  m_max_error_count = 0;
  m_mbr_path = _T("");
  m_embr_path = _T("");
  m_force_readonly = FALSE;
  m_clear_mru = FALSE;
  m_extract_path = _T("");
  m_help_path = _T("");
  m_viewer_path = _T("");
  m_force_fysos = FALSE;
  //}}AFX_DATA_INIT
}

void CSettings::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSettings)
  DDX_Text(pDX, IDC_SETTINGS_ERR_CNT, m_max_error_count);
  DDV_MinMaxInt(pDX, m_max_error_count, -1, 100);
  DDX_Text(pDX, IDC_MBR_PATH, m_mbr_path);
  DDX_Text(pDX, IDC_EMBR_PATH, m_embr_path);
  DDX_Check(pDX, IDC_FORCE_READONLY, m_force_readonly);
  DDX_Check(pDX, IDC_CLEAR_MRU, m_clear_mru);
  DDX_Text(pDX, IDC_EXTRACT_PATH, m_extract_path);
  DDX_Text(pDX, IDC_HELP_PATH, m_help_path);
  DDX_Text(pDX, IDC_VIEWER_PATH, m_viewer_path);
  DDX_Check(pDX, IDC_FORCE_FYSOS, m_force_fysos);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettings, CDialog)
  //{{AFX_MSG_MAP(CSettings)
  ON_BN_CLICKED(IDC_MBR_PATH_BRZ, OnMbrPathBrz)
  ON_BN_CLICKED(IDC_EMBR_PATH_BRZ, OnEmbrPathBrz)
  ON_BN_CLICKED(IDC_EXTRACT_PATH_BRZ, OnExtractPathBrz)
  ON_BN_CLICKED(IDC_VIEWER_PATH_BRZ, OnViewerPathBrz)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettings message handlers

void CSettings::OnMbrPathBrz() {
  CString csStart;
  char szPath[MAX_PATH];
  
  GetDlgItemText(IDC_MBR_PATH, csStart);
  if (BrowseForFolder(GetSafeHwnd(), csStart, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS))
    SetDlgItemText(IDC_MBR_PATH, szPath);
}

void CSettings::OnEmbrPathBrz() {
  CString csStart;
  char szPath[MAX_PATH];
  
  GetDlgItemText(IDC_MBR_PATH, csStart);
  if (BrowseForFolder(GetSafeHwnd(), csStart, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS))
    SetDlgItemText(IDC_EMBR_PATH, szPath);
}

void CSettings::OnExtractPathBrz() {
  CString csStart;
  char szPath[MAX_PATH];
  
  GetDlgItemText(IDC_EXTRACT_PATH, csStart);
  if (BrowseForFolder(GetSafeHwnd(), csStart, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS))
    SetDlgItemText(IDC_EXTRACT_PATH, szPath);
}

void CSettings::OnViewerPathBrz() {
  CString csStart;
  char szPath[MAX_PATH];
  
  GetDlgItemText(IDC_VIEWER_PATH, csStart);
  if (BrowseForFolder(GetSafeHwnd(), csStart, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS))
    SetDlgItemText(IDC_VIEWER_PATH, szPath);
}
