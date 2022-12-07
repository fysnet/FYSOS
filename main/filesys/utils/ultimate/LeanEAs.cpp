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

// LeanEAs.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Lean.h"
#include "LeanEAs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanEAs dialog
CLeanEAs::CLeanEAs(CWnd* pParent /*=NULL*/)
  : CDialog(CLeanEAs::IDD, pParent) {
  //{{AFX_DATA_INIT(CLeanEAs)
  m_as_utf8 = -1;
  m_name = _T("");
  m_attrib_as_byte = _T("");
  m_attrib_as_utf8 = _T("");
  m_attrib_as_value = _T("");
  m_force_fork = FALSE;
  //}}AFX_DATA_INIT
}

void CLeanEAs::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanEAs)
  DDX_Control(pDX, IDC_LIST, m_list);
  DDX_Radio(pDX, IDC_AS_UTF8, m_as_utf8);
  DDX_Text(pDX, IDC_NAME, m_name);
  DDV_MaxChars(pDX, m_name, 255);
  DDX_Text(pDX, IDC_ATTRIB_AS_BYTE, m_attrib_as_byte);
  DDX_Text(pDX, IDC_ATTRIB_AS_UTF8, m_attrib_as_utf8);
  DDX_Text(pDX, IDC_ATTRIB_AS_VALUE, m_attrib_as_value);
  DDX_Check(pDX, IDC_FORCE_FORK, m_force_fork);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanEAs, CDialog)
  //{{AFX_MSG_MAP(CLeanEAs)
  ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
  ON_BN_CLICKED(IDC_ADD, OnAdd)
  ON_BN_CLICKED(IDC_DELETE, OnDelete)
  ON_BN_CLICKED(IDDONE, OnDone)
  ON_BN_CLICKED(IDC_AS_UTF8, OnAsChange)
  ON_BN_CLICKED(IDC_AS_VALUE, OnAsChange)
  ON_BN_CLICKED(IDC_AS_BYTES, OnAsChange)
  ON_BN_CLICKED(IDC_APPLY, OnApply)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeanEAs message handlers

BOOL CLeanEAs::OnInitDialog() {
  int i;
  
  m_as_utf8 = 0;
  
  CDialog::OnInitDialog();
  
  if (m_count > 0) {
    for (i=0; i<m_count; i++)
      m_list.AddString(m_ea_struct[i].csName);
  } else
    m_list.AddString("No Entries");
  
  return TRUE;
}

void CLeanEAs::UpdateMembers(const int index) {
  int i, j;
  DWORD64 qword;
  CString cs;
  BYTE *p;
  
  m_name = m_ea_struct[index].csName;
  m_as_utf8 = m_ea_struct[index].Type;
  if (m_ea_struct[index].AttribLen > 0) {
    switch (m_as_utf8) {
      case 0:  // UTF8
        p = (BYTE *) m_attrib_as_utf8.GetBufferSetLength(m_ea_struct[index].AttribLen + 1);
        memcpy(p, m_ea_struct[index].Data, m_ea_struct[index].AttribLen);
        p[m_ea_struct[index].AttribLen] = 0;
        m_attrib_as_utf8.ReleaseBuffer(-1);
        break;
      case 1:  // Value
        if (m_ea_struct[index].AttribLen < 8) {
          qword = * (DWORD64 *) m_ea_struct[index].Data;
          m_attrib_as_value.Format("%I64i", qword);
        } else
          m_attrib_as_value = "Too Big";
        break;
      case 2:  // Data
        m_attrib_as_byte.Empty();
        for (i=0, j=0; i<m_ea_struct[index].AttribLen; i++) {
          cs.Format("0x%02X ", m_ea_struct[index].Data[i]);
          m_attrib_as_byte += cs;
          if (++j == 8) {
            m_attrib_as_byte += "\r\n";
            j = 0;
          }
        }
        break;
    }
  } else {
    m_attrib_as_utf8 = "";
    m_attrib_as_value = "";
    m_attrib_as_byte = "";
  }
  
  UpdateData(FALSE);  // push to dialog
}

void CLeanEAs::OnSelchangeList() {
  int index = m_list.GetCurSel();
  BOOL enabled = ((index != LB_ERR) && (m_count > 0));
  
  GetDlgItem(IDC_DELETE)->EnableWindow(enabled);
  GetDlgItem(IDC_APPLY)->EnableWindow(enabled);
  GetDlgItem(IDC_NAME)->EnableWindow(enabled);
  GetDlgItem(IDC_AS_UTF8)->EnableWindow(enabled);
  GetDlgItem(IDC_AS_VALUE)->EnableWindow(enabled);
  GetDlgItem(IDC_AS_BYTES)->EnableWindow(enabled);
  GetDlgItem(IDC_ATTRIB_AS_UTF8)->SendMessage(EM_SETREADONLY, !enabled, 0);
  GetDlgItem(IDC_ATTRIB_AS_VALUE)->SendMessage(EM_SETREADONLY, !enabled, 0);
  GetDlgItem(IDC_ATTRIB_AS_BYTE)->SendMessage(EM_SETREADONLY, !enabled, 0);
  
  if (!enabled) {
    SetDlgItemText(IDC_NAME, "");
    SetDlgItemText(IDC_ATTRIB_AS_UTF8, "");
    SetDlgItemText(IDC_ATTRIB_AS_VALUE, "");
    SetDlgItemText(IDC_ATTRIB_AS_BYTE, "");
  } else {
    UpdateMembers(index);
    OnAsChange();
  }
}

void CLeanEAs::OnAdd() {
  // if we don't have any yet, make sure and delete
  //  the "No Items" entry.
  if (m_count == 0)
    m_list.ResetContent();
  
  // don't allow any more than MAX_EA_STRUCT_ENTRIES
  if (m_count < MAX_EA_STRUCT_ENTRIES) {
    m_ea_struct[m_count].csName = "New Name";
    m_ea_struct[m_count].AttribLen = 0;
    memset(m_ea_struct[m_count].Data, 0, MAX_EA_STRUCT_DATA_SIZE);
    
    m_list.AddString(m_ea_struct[m_count].csName);
    m_list.SetCurSel(m_count);
    m_count++;
    OnSelchangeList();
  } else
    AfxMessageBox("We only allow a count of MAX_EA_STRUCT_ENTRIES entries.");
}

void CLeanEAs::OnDelete() {
  int i, index = m_list.GetCurSel();
  
  if ((index == LB_ERR) || (m_count == 0))
    return;
  
  m_list.DeleteString(index);
  
  for (i=index; i<m_count-1; i++) {
    m_ea_struct[i].csName = m_ea_struct[i+1].csName;
    m_ea_struct[i].AttribLen = m_ea_struct[i+1].AttribLen;
    if (m_ea_struct[i+1].AttribLen > 0)
      memcpy(m_ea_struct[i].Data, m_ea_struct[i+1].Data, m_ea_struct[i].AttribLen);
    m_ea_struct[i].AttribLen = m_ea_struct[i+1].AttribLen;
  }
  
  if (--m_count == 0)
    m_list.AddString("No Entries");
  
  m_list.SetCurSel(index);
  OnSelchangeList();
}

void CLeanEAs::OnAsChange() {
  int index = m_list.GetCurSel();
  
  if ((index == LB_ERR) || (m_count == 0))
    return;
  
  int id = GetCheckedRadioButton(IDC_AS_UTF8, IDC_AS_BYTES);
  switch (id) {
    case IDC_AS_UTF8:  // UTF8
      GetDlgItem(IDC_ATTRIB_AS_UTF8)->SendMessage(EM_SETREADONLY, FALSE, 0);
      GetDlgItem(IDC_ATTRIB_AS_VALUE)->SendMessage(EM_SETREADONLY, TRUE, 0);
      GetDlgItem(IDC_ATTRIB_AS_BYTE)->SendMessage(EM_SETREADONLY, TRUE, 0);
      SetDlgItemText(IDC_ATTRIB_AS_VALUE, "");
      SetDlgItemText(IDC_ATTRIB_AS_BYTE, "");
      break;
    case IDC_AS_VALUE:  // Value
      GetDlgItem(IDC_ATTRIB_AS_UTF8)->SendMessage(EM_SETREADONLY, TRUE, 0);
      GetDlgItem(IDC_ATTRIB_AS_VALUE)->SendMessage(EM_SETREADONLY, FALSE, 0);
      GetDlgItem(IDC_ATTRIB_AS_BYTE)->SendMessage(EM_SETREADONLY, TRUE, 0);
      SetDlgItemText(IDC_ATTRIB_AS_UTF8, "");
      SetDlgItemText(IDC_ATTRIB_AS_BYTE, "");
      break;
    case IDC_AS_BYTES:  // Data
      GetDlgItem(IDC_ATTRIB_AS_UTF8)->SendMessage(EM_SETREADONLY, TRUE, 0);
      GetDlgItem(IDC_ATTRIB_AS_VALUE)->SendMessage(EM_SETREADONLY, TRUE, 0);
      GetDlgItem(IDC_ATTRIB_AS_BYTE)->SendMessage(EM_SETREADONLY, FALSE, 0);
      SetDlgItemText(IDC_ATTRIB_AS_UTF8, "");
      SetDlgItemText(IDC_ATTRIB_AS_VALUE, "");
      break;
  }
}

void CLeanEAs::OnApply() {
  int index = m_list.GetCurSel();
  DWORD64 qword;
  
  if ((index == LB_ERR) || (m_count == 0))
    return;
  
  UpdateData(TRUE);
  
  m_list.DeleteString(index);
  m_ea_struct[index].csName = m_name;
  m_list.InsertString(index, m_name);
  m_ea_struct[index].Type = m_as_utf8;
  
  switch (m_as_utf8) {
    case 0:  // UTF8
      m_ea_struct[index].AttribLen = m_attrib_as_utf8.GetLength();
      if (m_ea_struct[index].AttribLen > MAX_EA_STRUCT_DATA_SIZE)
        m_ea_struct[index].AttribLen = MAX_EA_STRUCT_DATA_SIZE;
      memcpy(m_ea_struct[index].Data, m_attrib_as_utf8, m_ea_struct[index].AttribLen);
      break;
    case 1:  // Value
      qword = convert64(m_attrib_as_value);
      * (DWORD64 *) m_ea_struct[index].Data = qword;
      if (qword <= 0xFF)
        m_ea_struct[index].AttribLen = 1;
      else if (qword <= 0xFFFF)
        m_ea_struct[index].AttribLen = 2;
      else if (qword <= 0xFFFFFFFF)
        m_ea_struct[index].AttribLen = 4;
      else
        m_ea_struct[index].AttribLen = 8;
      break;
    case 2:  // Data
      m_ea_struct[index].AttribLen = ConvertDumpToBuffer(m_attrib_as_byte, m_ea_struct[index].Data, MAX_EA_STRUCT_DATA_SIZE);
      break;
  }
  
  m_list.SetCurSel(index);
}

void CLeanEAs::OnDone() {
  CDialog::OnOK();
}
