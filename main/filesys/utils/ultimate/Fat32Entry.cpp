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

// Fat32Entry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Fat32Entry.h"

#include "Fat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFat32Entry dialog
CFat32Entry::CFat32Entry(CWnd* pParent /*=NULL*/)
  : CDialog(CFat32Entry::IDD, pParent) {
  //{{AFX_DATA_INIT(CFat32Entry)
  m_cluster = _T("");
  m_date = _T("");
  m_last_acc = _T("");
  m_nt_resv = _T("");
  m_tenth = _T("");
  m_time = _T("");
  //}}AFX_DATA_INIT
}

void CFat32Entry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFat32Entry)
  DDX_Text(pDX, IDC_FAT32_ENTRY_CLUST, m_cluster);
  DDX_Text(pDX, IDC_FAT32_ENTRY_DATE, m_date);
  DDX_Text(pDX, IDC_FAT32_ENTRY_LAST_ACC, m_last_acc);
  DDX_Text(pDX, IDC_FAT32_ENTRY_RESV, m_nt_resv);
  DDX_Text(pDX, IDC_FAT32_ENTRY_TENTH, m_tenth);
  DDX_Text(pDX, IDC_FAT32_ENTRY_TIME, m_time);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFat32Entry, CDialog)
  //{{AFX_MSG_MAP(CFat32Entry)
  ON_BN_CLICKED(IDC_UPDATE_DATE, OnUpdateDate)
  ON_BN_CLICKED(IDC_UPDATE_LAST, OnUpdateLast)
  ON_BN_CLICKED(IDC_UPDATE_TIME, OnUpdateTime)
  ON_EN_CHANGE(IDC_FAT32_ENTRY_TIME, OnChangeFat32EntryTime)
  ON_EN_CHANGE(IDC_FAT32_ENTRY_DATE, OnChangeFat32EntryDate)
  ON_EN_CHANGE(IDC_FAT32_ENTRY_LAST_ACC, OnChangeFat32EntryLastAcc)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFat32Entry message handlers
BOOL CFat32Entry::OnInitDialog() {
  CDialog::OnInitDialog();
  
  OnChangeFat32EntryTime();
  OnChangeFat32EntryDate();
  OnChangeFat32EntryLastAcc();
  
  return TRUE;
}

void CFat32Entry::OnUpdateDate() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned d = ((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay();
  cs.Format("0x%04X", d);
  SetDlgItemText(IDC_FAT32_ENTRY_DATE, cs);
}

void CFat32Entry::OnUpdateLast() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned d = ((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay();
  cs.Format("0x%04X", d);
  SetDlgItemText(IDC_FAT32_ENTRY_LAST_ACC, cs);
}

void CFat32Entry::OnUpdateTime() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned t = (time.GetHour() << 11) | (time.GetMinute() << 5) | time.GetSecond();
  cs.Format("0x%04X", t);
  SetDlgItemText(IDC_FAT32_ENTRY_TIME, cs);
}

void CFat32Entry::OnChangeFat32EntryTime() {
  CString cs, time;
  GetDlgItemText(IDC_FAT32_ENTRY_TIME, time);
  
  unsigned t = convert16(time);
  cs.Format("%02i:%02i:%02i", t >> 11, (t & 0x07E0) >> 5, (t & 0x001F) >> 0);
  
  SetDlgItemText(IDC_FAT32_ENTRY_TIME_DISP, cs);
}

void CFat32Entry::OnChangeFat32EntryDate() {
  CString cs, date;
  GetDlgItemText(IDC_FAT32_ENTRY_DATE, date);
  
  unsigned d = convert16(date);
  cs.Format("%04i/%02i/%02i", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0);
  
  SetDlgItemText(IDC_FAT32_ENTRY_DATE_DISP, cs);
}

void CFat32Entry::OnChangeFat32EntryLastAcc() {
  CString cs, date;
  GetDlgItemText(IDC_FAT32_ENTRY_LAST_ACC, date);
  
  unsigned d = convert16(date);
  cs.Format("%04i/%02i/%02i", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0);
  
  SetDlgItemText(IDC_FAT32_ENTRY_LAST_DISP, cs);
}

