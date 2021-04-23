/*
 *                             Copyright (c) 1984-2021
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

// NewPart.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "NewImage.h"
#include "NewPart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewPart property page

IMPLEMENT_DYNCREATE(CNewPart, CPropertyPage)

CNewPart::CNewPart() : CPropertyPage(CNewPart::IDD) {
  //{{AFX_DATA_INIT(CNewPart)
  m_filename = _T("");
  m_sectors = 10240;
  m_name = _T("");
  m_bootable = FALSE;
  m_emulation = 0;
  //}}AFX_DATA_INIT
  m_dirty = FALSE;
}

CNewPart::~CNewPart() {
}

void CNewPart::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CNewPart)
  DDX_Text(pDX, IDC_NEW_PART_NAME, m_filename);
  DDX_Text(pDX, IDC_SECTORS, m_sectors);
  DDX_Text(pDX, IDC_NEW_NAME, m_name);
  DDX_Check(pDX, IDC_BOOTABLE, m_bootable);
  DDX_Radio(pDX, IDC_ISO9660_NO_EMU, m_emulation);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewPart, CPropertyPage)
  //{{AFX_MSG_MAP(CNewPart)
  ON_EN_CHANGE(IDC_NEW_PART_NAME, OnPartChanged)
  ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
  ON_EN_CHANGE(IDC_SECTORS, OnPartChanged)
  ON_MESSAGE(WM_DROPFILES, OnDropFiles)
  ON_EN_CHANGE(IDC_NEW_NAME, OnPartChanged)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewPart::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // only show the CDROM stuff on the first page
  CPropertySheet *Sheet = (CPropertySheet *) GetParent();
  if (Sheet->GetPageIndex(this) > 0) {
    GetDlgItem(IDC_BOOTABLE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BOOT_FRAME)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ISO9660_NO_EMU)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ISO9660_120)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ISO9660_144)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ISO9660_288)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_ISO9660_HD)->ShowWindow(SW_HIDE);
  }
  OnSizeChanged();

  return TRUE;
}

void CNewPart::OnPartChanged() {
  if (!m_dirty) {
    CPropertySheet *Sheet = (CPropertySheet *) GetParent();
    TC_ITEM ti;
    char szText[64];
    ti.mask = TCIF_TEXT;
    ti.pszText = szText;
    ti.cchTextMax = 64;
    Sheet->GetTabControl()->GetItem(m_index, &ti);
    strcat(szText, "*");
    Sheet->GetTabControl()->SetItem(m_index, &ti);
    m_dirty = TRUE;
  }
  
  OnSizeChanged();
}

void CNewPart::OnSizeChanged() {
  CNewImage *parent = (CNewImage *) m_parent;
  CString megs;

  UpdateData(TRUE); // bring from Dialog
  if (((size_t) m_sectors * parent->m_sector_size) < (size_t) (1024 * 1024 * 1024))
    megs.Format("%.3f Meg", (float) ((double) ((size_t) m_sectors * parent->m_sector_size) / (double) (size_t) (1024 * 1024)));
  else
    megs.Format("%.3f Gig", (float) ((double) ((size_t) m_sectors * parent->m_sector_size) / (double) (size_t) (1024 * 1024 * 1024)));
  SetDlgItemText(IDC_SIZE_MEGS, megs);
}

void CNewPart::OnBrowse() {
  CString csPath;
  
  CFileDialog dlg (
    TRUE,             // Create an open file dialog
    _T(".img"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  if (dlg.DoModal() != IDOK)
    return;
  
  POSITION pos = dlg.GetStartPosition();
  csPath = dlg.GetNextPathName(pos);
  
  UpdateData(TRUE); // bring from Dialog
  m_filename = csPath;
  
  CFile file;
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
    CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
    LARGE_INTEGER file_length = dlg->GetFileLength((HANDLE) file.m_hFile);
    CNewImage *parent = (CNewImage *) m_parent;
    DWORD sectors_needed = (DWORD) ((file_length.QuadPart + (parent->m_sector_size - 1)) / parent->m_sector_size);
    if (sectors_needed > m_sectors)
      m_sectors = sectors_needed;
    file.Close();
  }
  
  UpdateData(FALSE); // Send to Dialog
  m_dirty = TRUE;
}

LRESULT CNewPart::OnDropFiles(WPARAM wParam, LPARAM lParam) {
  /*
  CString cs;
  char szDroppedFile[MAX_PATH];
  
  DragQueryFile((HDROP) wParam, // Struture Identifier
        0,                // Index (of 0xFFFFFFFF to get count of files dropped)
        szDroppedFile,    // Droped File Name
        MAX_PATH);        // Max char 
  
  int index = m_Sheet.GetActiveIndex();
  if (m_Parts[index].m_dirty) {
    cs.Format("Update partition with new file: %s", szDroppedFile);
    if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
      return 0;
  }
  
  cs = szDroppedFile;
  UpdateEntry(cs, index);
  */

  AfxMessageBox("Todo");
  
  return 0;
}
