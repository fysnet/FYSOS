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

// ViewBitmap.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "ViewBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewBitmap dialog
CViewBitmap::CViewBitmap(CWnd* pParent /*=NULL*/)
  : CDialog(CViewBitmap::IDD, pParent) {
  //{{AFX_DATA_INIT(CViewBitmap)
  //}}AFX_DATA_INIT
  
  m_buffer = NULL;
}

void CViewBitmap::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CViewBitmap)
  DDX_Control(pDX, IDC_SCROLLBAR, m_scroll);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CViewBitmap, CDialog)
  //{{AFX_MSG_MAP(CViewBitmap)
  ON_WM_VSCROLL()
  ON_CONTROL_RANGE(BN_CLICKED, IDC_BITMAP_BIT0, IDC_BITMAP_BIT7, OnButtonClicked)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewBitmap message handlers
BOOL CViewBitmap::OnInitDialog() {
  
  SetValues(0);
  
  CDialog::OnInitDialog();
  
  if (m_count > 7)
    m_scroll.SetScrollRange(0, m_count - 8, TRUE);
  else
    m_scroll.SetScrollRange(0, 0, TRUE);
  m_scroll.SetScrollPos(0, TRUE);
  
  return TRUE;
}

void CViewBitmap::SetValues(const DWORD start) {
  BYTE *p = (BYTE *) m_buffer;
  CString cs;
  
  cs.Format("%ss %i to %i of %i", m_item_name, start, start+7, m_count);
  SetDlgItemText(IDC_CLUSTER_TITLE, cs);
  
  for (int i=0; i<8; i++) {
    cs.Format("%s %i", m_item_name, start + i);
    GetDlgItem(IDC_BITMAP_BIT0 + i)->SetWindowText(cs);
    UINT checked = ((p[(start + i) / 8] & (1 << ((start + i) % 8))) > 0) ? BST_CHECKED : BST_UNCHECKED;
    CheckDlgButton(IDC_BITMAP_BIT0 + i, checked);
  }
  
  m_current = start;
}

void CViewBitmap::OnButtonClicked(UINT nID) {
  BYTE *p = (BYTE *) m_buffer;
  int pos = m_current + (nID - IDC_BITMAP_BIT0);
  
  // this is called after the button is toggled
  if (!IsDlgButtonChecked(nID))
    p[pos / 8] &= ~(1 << (pos % 8));
  else
    p[pos / 8] |= (1 << (pos % 8));
}

void CViewBitmap::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {
  SCROLLINFO info = { sizeof(SCROLLINFO) };
  pScrollBar->GetScrollInfo(&info, SIF_ALL);
  
  //update scroller
  switch (nSBCode) {
    case SB_LEFT: info.nPos = info.nMin; break;
    case SB_RIGHT: info.nPos = info.nMax; break;
    case SB_LINELEFT: info.nPos--; break;
    case SB_LINERIGHT: info.nPos++;  break;
    case SB_PAGELEFT: info.nPos -= info.nPage; break;
    case SB_PAGERIGHT: info.nPos += info.nPage; break;
    case SB_THUMBPOSITION: info.nPos = info.nTrackPos; break;
    case SB_THUMBTRACK: info.nPos = info.nTrackPos; break;
  }
  
  if (info.nPos < 0)
    info.nPos = 0;
  if (info.nPos > (int) (m_count - 8))
    info.nPos = (int) (m_count - 8);
  
  pScrollBar->SetScrollInfo(&info, TRUE);
  
  SetValues(info.nPos);
}
