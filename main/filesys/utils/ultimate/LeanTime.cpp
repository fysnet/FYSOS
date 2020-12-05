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

// LeanTime.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "LeanTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanTime dialog
CLeanTime::CLeanTime(CWnd* pParent /*=NULL*/)
  : CDialog(CLeanTime::IDD, pParent) {
  //{{AFX_DATA_INIT(CLeanTime)
  m_lean_time = _T("");
  //}}AFX_DATA_INIT
  m_divisor = 1;    // default to 1
  m_adjustment = 0; // default to no adjustment
}

void CLeanTime::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanTime)
  DDX_Text(pDX, IDC_LEAN_TIME, m_lean_time);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanTime, CDialog)
  //{{AFX_MSG_MAP(CLeanTime)
  ON_EN_CHANGE(IDC_LEAN_TIME, OnChangeLeanTime)
  ON_BN_CLICKED(IDC_TIME_NOW, OnTimeNow)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeanTime message handlers
BOOL CLeanTime::OnInitDialog() {
  CDialog::OnInitDialog();
  
  SetWindowText(m_title);
  
  OnChangeLeanTime();  
  
  return TRUE;
}

void CLeanTime::OnChangeLeanTime() {
  CString cs;
  
  GetDlgItemText(IDC_LEAN_TIME, cs);
  DWORD64 timestamp = (((INT64) convert64(cs) / m_divisor) + m_adjustment);  // convert from ? to Seconds
#if _MSC_VER <= 1310
  if (timestamp > 0xFFFFFFFF)  // VC++ 6.0 limited the value to 32-bits  (Crashes if larger)
    cs = "Error in Timestamp";
  else
#endif
  {
    CTime cTime((time_t) timestamp);  // converts seconds since 1 Jan 1970 to CTime
    cs.Format("%04i/%02i/%02i  %02i:%02i:%02i", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
  }
  SetDlgItemText(IDC_LEAN_TIME_DISP, cs);
}

void CLeanTime::OnTimeNow() {
  CString cs;
  CTime cTime;
  time_t now;
  
  cTime = CTime::GetCurrentTime();
  now = cTime.GetTime();
  cs.Format("%I64i", ((INT64) now * m_divisor) - m_adjustment);
  
  SetDlgItemText(IDC_LEAN_TIME, cs);
  
  OnChangeLeanTime();  
}
