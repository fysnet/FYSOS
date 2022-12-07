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

// LeanFormat.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Lean.h"
#include "LeanFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanFormat dialog


CLeanFormat::CLeanFormat(CWnd* pParent /*=NULL*/)
  : CDialog(CLeanFormat::IDD, pParent)
{
  //{{AFX_DATA_INIT(CLeanFormat)
  m_block_size = 512;
  m_pre_alloc_count = 0;
  m_encoding = ceUTF8;
  m_eas_after_inode = TRUE;
  m_extended_extents = FALSE;
  m_journal = FALSE;
  //}}AFX_DATA_INIT
}

void CLeanFormat::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanFormat)
  DDX_Text(pDX, IDC_BLOCK_SIZE, m_block_size);
  DDV_MinMaxInt(pDX, m_block_size, 256, 65536);
  DDX_Text(pDX, IDC_PRE_ALLOC_COUNT, m_pre_alloc_count);
  DDV_MinMaxInt(pDX, m_pre_alloc_count, 1, 12);
  DDX_CBIndex(pDX, IDC_ENCODING, m_encoding);
  DDX_Check(pDX, IDC_EAS_IN_INODE, m_eas_after_inode);
  DDX_Check(pDX, IDC_EXT_EXTENTS, m_extended_extents);
  DDX_Check(pDX, IDC_JOURNAL, m_journal);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanFormat, CDialog)
  //{{AFX_MSG_MAP(CLeanFormat)
  ON_BN_CLICKED(IDOK, OnOkay)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CLeanFormat::OnInitDialog() {
  CDialog::OnInitDialog();

  // fill the Encoding combo box
  CComboBox *cb = (CComboBox *) GetDlgItem(IDC_ENCODING);
  cb->AddString("Ascii");
  cb->AddString("UTF-8");
  cb->AddString("UTF-16");
  cb->SetCurSel(m_encoding);

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLeanFormat message handlers
void CLeanFormat::OnOkay() {

  UpdateData(TRUE);

  if (!power_of_two(m_block_size)) {
    AfxMessageBox("Block Size must be a power of two from 256 to 65536.");
    return;
  }

  CDialog::OnOK();
}

