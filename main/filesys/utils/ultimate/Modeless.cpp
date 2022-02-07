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

// Modeless.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModeless dialog
CModeless::CModeless(CWnd* pParent /*=NULL*/)
  : CDialog(CModeless::IDD, pParent) {
  //{{AFX_DATA_INIT(CModeless)
  m_edit = _T("");
  //}}AFX_DATA_INIT
  m_modeless = FALSE;
  m_Title = _T("");
}

void CModeless::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CModeless)
  DDX_Text(pDX, IDC_EDIT, m_edit);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CModeless, CDialog)
  //{{AFX_MSG_MAP(CModeless)
  ON_BN_CLICKED(IDC_DONE, OnDone)
  ON_BN_CLICKED(IDC_COPY, OnCopy)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModeless message handlers
BOOL CModeless::OnInitDialog() {
  CDialog::OnInitDialog();
  
  SetWindowText(m_Title);

  // only allow the DONE and COPY buttons to be active if
  //  we are done and now not a modeless window
  GetDlgItem(IDC_DONE)->EnableWindow(!m_modeless);
  GetDlgItem(IDC_COPY)->EnableWindow(!m_modeless);
  
  // scroll down to the last line
  if (!m_modeless) {
    CEdit *pEdit = (CEdit *) GetDlgItem(IDC_EDIT);
    pEdit->LineScroll(pEdit->GetLineCount());
  }
  
  // setting the focus to the button and 
  //  returning FALSE doesn't select the text
  //  in the Edit Control
  GetDlgItem(IDC_DONE)->SetFocus();
  return FALSE;
}

void CModeless::OnDone() {
  if (m_modeless)
    EndModalLoop(IDOK);
  else
    EndDialog(IDOK);
}

// copy the current contents of the string to the Windows clipboard
void CModeless::OnCopy() {

  if (!OpenClipboard())
    return;

  if (!EmptyClipboard())
    return;

  size_t size = m_edit.GetLength();
  HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, size);
  memcpy(hGlob, m_edit, size);
  ::SetClipboardData(CF_TEXT, hGlob);

  CloseClipboard();
}
