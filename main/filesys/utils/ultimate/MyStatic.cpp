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

// MyStatic.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "MyStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyStatic
IMPLEMENT_DYNCREATE(CMyStatic, CStatic)

CMyStatic::CMyStatic() : CStatic() {
  m_item_count = 0;
  m_TabControl = NULL;
}

CMyStatic::~CMyStatic() {
}

BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
  //{{AFX_MSG_MAP(CMyStatic)
  ON_WM_PAINT()
  ON_WM_LBUTTONUP()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CMyStatic::AddItem(int left, int right, DWORD color, BOOL filled, char *title, CPropertyPage *page) {
  int ret = -1;
  
  if (m_item_count < IMAGE_BAR_MAX) {
    m_items[m_item_count].page = page;
    m_items[m_item_count].left = left;
    m_items[m_item_count].right = right;
    m_items[m_item_count].color = color;
    m_items[m_item_count].filled = filled;
    strcpy(m_items[m_item_count].title, title);
    ret = m_item_count++;
  }
  
  return ret;
}

void CMyStatic::UpdateTitle(int index, char *title) {
  if ((index > -1) && (index < IMAGE_BAR_MAX))
    strcpy(m_items[index].title, title);
  Invalidate(TRUE);
}

void CMyStatic::Clear() {
  m_item_count = 0;
  Invalidate(TRUE);
}

void CMyStatic::OnPaint() {
  int i;
  CPaintDC dc(this); // device context for painting
  
  CRect rect;
  GetClientRect(&rect);
  
  CBrush Brush;
  
  // back color first
  Brush.CreateSolidBrush(COLOR_MBR_EMPTY);
  dc.FillRect(&rect, &Brush);
  
  // So that a regular rectangle (non-filled) will
  // "encompass" other filled rectangles, we need to
  // draw them last.
  for (i=0; i<m_item_count; i++) {
    if (m_items[i].filled) {
      Brush.CreateSolidBrush(m_items[i].color);
      rect.left = m_items[i].left;
      rect.right = m_items[i].right;
      dc.FillRect(&rect, &Brush);
      
      // draw a title
      dc.SetTextColor(RGB(255,255,255));
      dc.SetBkColor(m_items[i].color);
      dc.DrawText(m_items[i].title, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
  }
  
  // now the (non-filled) rectangles
  // no text involved
  for (i=0; i<m_item_count; i++) {
    if (!m_items[i].filled) {
      Brush.CreateSolidBrush(m_items[i].color);
      rect.left = m_items[i].left;
      rect.right = m_items[i].right;
      dc.FrameRect(&rect, &Brush);
    }
  }
}

void CMyStatic::OnLButtonUp(UINT nFlags, CPoint point) {
  int i;
  
  for (i=0; i<m_item_count; i++) {
    if ((point.x >= m_items[i].left) && (point.x <= m_items[i].right)) {
      if (m_TabControl && m_items[i].page) {
        m_TabControl->SetActivePage(m_items[i].page);
        //break; // ??? If we break, it doesn't work right ?????
      }
    }
  }
  
  CStatic::OnLButtonUp(nFlags, point);
}
