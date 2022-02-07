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

// FYSOSSig.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "FYSOSSig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFYSOSSig dialog
CFYSOSSig::CFYSOSSig(CWnd* pParent /*=NULL*/)
  : CDialog(CFYSOSSig::IDD, pParent) {
  //{{AFX_DATA_INIT(CFYSOSSig)
  m_sig_base = _T("");
  m_boot_sig = _T("");
  m_fysos_sig = _T("");
  //}}AFX_DATA_INIT
}

void CFYSOSSig::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFYSOSSig)
  DDX_Text(pDX, IDC_FYSOS_SIG_BASE, m_sig_base);
  DDX_Text(pDX, IDC_FYSOS_SIG_BOOTSIG, m_boot_sig);
  DDX_Text(pDX, IDC_FYSOS_SIG_SIG, m_fysos_sig);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFYSOSSig, CDialog)
  //{{AFX_MSG_MAP(CFYSOSSig)
  ON_BN_CLICKED(IDC_UPDATE_SIG, OnUpdateSig)
  ON_BN_CLICKED(IDC_UPDATE_BOOT_SIG, OnUpdateBootSig)
  ON_BN_CLICKED(IDC_UPDATE_BASE, OnUpdateBase)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFYSOSSig message handlers
void CFYSOSSig::OnOK() {
  
  CDialog::OnOK();
}

void CFYSOSSig::OnUpdateSig() {
  CString cs;
  cs.Format("0x%08X", (rand() << 16) | rand());
  SetDlgItemText(IDC_FYSOS_SIG_SIG, cs);
}

void CFYSOSSig::OnUpdateBootSig() {
  SetDlgItemText(IDC_FYSOS_SIG_BOOTSIG, "0xAA55");
}

void CFYSOSSig::OnUpdateBase() {
  CString cs;
  cs.Format("%I64i", m_base_lba);
  SetDlgItemText(IDC_FYSOS_SIG_BASE, cs);
}
