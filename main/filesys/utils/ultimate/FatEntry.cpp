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

// FatEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "FatEntry.h"
#include "Fat32Entry.h"

#include "Fat.h"
#include "FatCList.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFatEntry dialog


CFatEntry::CFatEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CFatEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CFatEntry)
  m_data_disp = _T("");
  m_attrib = _T("");
  m_cluster = _T("");
  m_date = _T("");
  m_ext = _T("");
  m_filesize = _T("");
  m_lfn_attrb = _T("");
  m_lfn_cluster = _T("");
  m_lfn_resv = _T("");
  m_name = _T("");
  m_lfn_name = _T("");
  m_resvd = _T("");
  m_sequ = _T("");
  m_sfn_crc = _T("");
  m_time = _T("");
  m_sequ_disp = _T("");
  m_time_disp = _T("");
  m_error_code = _T("");
  //}}AFX_DATA_INIT
  m_lfns = NULL;
  m_lfn_count = 0;
  m_fat_size = 0;
  m_lfn_deleted = FALSE;
  m_show_bytes = FALSE;
}

void CFatEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFatEntry)
  DDX_Text(pDX, IDC_DATE_DISP, m_data_disp);
  DDX_Text(pDX, IDC_ENTRY_ATTRIB, m_attrib);
  DDV_MaxChars(pDX, m_attrib, 16);
  DDX_Text(pDX, IDC_ENTRY_CLUSTER, m_cluster);
  DDV_MaxChars(pDX, m_cluster, 16);
  DDX_Text(pDX, IDC_ENTRY_DATE, m_date);
  DDV_MaxChars(pDX, m_date, 16);
  DDX_Text(pDX, IDC_ENTRY_EXT, m_ext);
  DDV_MaxChars(pDX, m_ext, 4);
  DDX_Text(pDX, IDC_ENTRY_FILESIZE, m_filesize);
  DDV_MaxChars(pDX, m_filesize, 16);
  DDX_Text(pDX, IDC_ENTRY_LFN_ATTRIB, m_lfn_attrb);
  DDV_MaxChars(pDX, m_lfn_attrb, 16);
  DDX_Text(pDX, IDC_ENTRY_LFN_CLUST, m_lfn_cluster);
  DDV_MaxChars(pDX, m_lfn_cluster, 16);
  DDX_Text(pDX, IDC_ENTRY_LFN_RESV, m_lfn_resv);
  DDV_MaxChars(pDX, m_lfn_resv, 16);
  DDX_Text(pDX, IDC_ENTRY_NAME, m_name);
  DDX_Text(pDX, IDC_ENTRY_LFN_NAME, m_lfn_name);
  DDV_MaxChars(pDX, m_lfn_name, 64);
  DDX_Text(pDX, IDC_ENTRY_RESERVED, m_resvd);
  DDX_Text(pDX, IDC_ENTRY_SEQU, m_sequ);
  DDV_MaxChars(pDX, m_sequ, 16);
  DDX_Text(pDX, IDC_ENTRY_SFN_CRC, m_sfn_crc);
  DDV_MaxChars(pDX, m_sfn_crc, 16);
  DDX_Text(pDX, IDC_ENTRY_TIME, m_time);
  DDV_MaxChars(pDX, m_time, 16);
  DDX_Text(pDX, IDC_SEQU_DISP, m_sequ_disp);
  DDX_Text(pDX, IDC_TIME_DISP, m_time_disp);
  DDX_Text(pDX, IDC_ENTRY_ERROR_CODE, m_error_code);
  DDX_Check(pDX, IDC_SHOW_BYTES, m_show_bytes);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFatEntry, CDialog)
  //{{AFX_MSG_MAP(CFatEntry)
  ON_BN_CLICKED(IDDATE, OnDate)
  ON_BN_CLICKED(IDDELLFN, OnDellfn)
  ON_BN_CLICKED(IDLFN_CLEAR, OnLFNRClear)
  ON_BN_CLICKED(IDNEXT, OnNext)
  ON_BN_CLICKED(IDPREV, OnPrev)
  ON_BN_CLICKED(IDTIME, OnTime)
  ON_BN_CLICKED(IDSFN_CLEAR, OnSFNRClear)
  ON_EN_CHANGE(IDC_ENTRY_DATE, OnChangeEntryDate)
  ON_EN_CHANGE(IDC_ENTRY_TIME, OnChangeEntryTime)
  ON_BN_CLICKED(ID_FAT_ATTRIB, OnAttribute)
  ON_BN_CLICKED(IDFATENTRIES, OnFatentries)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  ON_BN_CLICKED(IDC_SHOW_BYTES, OnShowBytes)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFatEntry message handlers
BOOL CFatEntry::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
  UNREFERENCED_PARAMETER(id);
  UNREFERENCED_PARAMETER(pResult);

  // need to handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA *pTTTA = (TOOLTIPTEXTA *) pNMHDR;
  CString strTipText;
  UINT_PTR nID = pNMHDR->idFrom;  // idFrom is actually the HWND of the tool

  if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND))
    nID = ::GetDlgCtrlID((HWND)nID);

  // Make sure that all strings are less than 80 chars
  switch (nID) {
    case ID_FAT_ATTRIB:
      strTipText = "Modify Attributes";
      break;
    case IDSFN_CLEAR:
    case IDLFN_CLEAR:
      strTipText = "Clear Reserved Area";
      break;
    case IDTIME:
    case IDDATE:
      strTipText = "Set to now";
      break;
    case IDC_CRC_UPDATE:
      strTipText = "Set to CRC of SFN";
      break;

    case IDPREV:
      strTipText = "Previous LFN entry in this slot";
      break;
    case IDNEXT:
      strTipText = "Next LFN entry in this slot";
      break;
    
    case IDFATENTRIES:
      strTipText = "View all of the FAT entries for this file";
      break;

    case IDOK:
      strTipText = "Save modifications";
      break;
  }

  strncpy(pTTTA->szText, strTipText, 79);
  pTTTA->szText[79] = '\0';  // make sure it is null terminated

  return TRUE; // message was handled
}

BOOL CFatEntry::OnInitDialog() {
  CDialog::OnInitDialog();
  
  m_lfn_cur = -1;
  if (m_lfn_count > 0)
    OnNext();
  else {
    GetDlgItem(IDC_ENTRY_SEQU)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_ATTRIB)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_SFN_CRC)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_CLUST)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_NAME)->EnableWindow(FALSE);
    GetDlgItem(IDLFN_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDPREV)->EnableWindow(FALSE);
    GetDlgItem(IDNEXT)->EnableWindow(FALSE);
    GetDlgItem(IDDELLFN)->EnableWindow(FALSE);
    SetDlgItemText(IDC_SEQU_DISP, "None");
  }
  OnChangeEntryDate();
  OnChangeEntryTime();
  
  if (m_fat_size == FS_FAT32) {
    SetDlgItemText(IDC_RESERVED_DISP, "Fat 32:");
    SetDlgItemText(IDSFN_CLEAR, "Fat 32");
  } else {
    SetDlgItemText(IDC_RESERVED_DISP, "Reserved:");
    SetDlgItemText(IDSFN_CLEAR, "Clear");
  }
  
  GetDlgItem(IDFATENTRIES)->EnableWindow(m_fat_entries.entries > 0);
  
  EnableToolTips(TRUE);

  return TRUE;
}

void CFatEntry::OnOK() {
  CString cs;
  int i;
  
  if (!m_lfn_deleted)
    DialogtoLFN();
  UpdateData(TRUE);
  
  BYTE *p = (BYTE *) m_name.GetBuffer(8);
  BYTE Sum = 0;
  for (i=8; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  m_name.ReleaseBuffer(8);
  p = (BYTE *) m_ext.GetBuffer(3);
  for (i=3; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  m_ext.ReleaseBuffer(3);
  
  struct S_FAT_LFN_ROOT *lfn = (struct S_FAT_LFN_ROOT *) m_lfns;
  for (i=0; i<m_lfn_count; i++) {
    if (lfn[i].sfn_crc != Sum) {
      cs.Format("Update LFN check sum value for entry %i", i);
      if (AfxMessageBox(cs, MB_YESNO, 0) == IDYES)
        lfn[i].sfn_crc = Sum;
    }
  }
  
  CDialog::OnOK();
}

void CFatEntry::OnDellfn() {
  if (AfxMessageBox("Are you sure?", MB_YESNO, 0) == IDYES) {  
    struct S_FAT_LFN_ROOT *lfn = (struct S_FAT_LFN_ROOT *) m_lfns;
    for (int i=0; i<m_lfn_count; i++)
      lfn[i].sequ_flags |= 0x80;
    m_lfn_deleted = TRUE;
    GetDlgItem(IDC_ENTRY_SEQU)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_ATTRIB)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_SFN_CRC)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_CLUST)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_LFN_NAME)->EnableWindow(FALSE);
    GetDlgItem(IDLFN_CLEAR)->EnableWindow(FALSE);
    GetDlgItem(IDPREV)->EnableWindow(FALSE);
    GetDlgItem(IDNEXT)->EnableWindow(FALSE);
    GetDlgItem(IDDELLFN)->EnableWindow(FALSE);
    SetDlgItemText(IDC_SEQU_DISP, "None");
  }
}

void CFatEntry::OnLFNRClear() {
  SetDlgItemText(IDC_ENTRY_LFN_RESV, "00");
}

void CFatEntry::OnSFNRClear() {
  if (m_fat_size == FS_FAT32) {
    CFat32Entry Fat32Entry;
    
    Fat32Entry.m_nt_resv.Format("0x%02X", m_sfn.type.fat32.nt_resv);
    Fat32Entry.m_tenth.Format("0x%02X", m_sfn.type.fat32.crt_time_tenth);
    Fat32Entry.m_time.Format("0x%04X", m_sfn.type.fat32.crt_time);
    Fat32Entry.m_date.Format("0x%04X", m_sfn.type.fat32.crt_date);
    Fat32Entry.m_last_acc.Format("0x%04X", m_sfn.type.fat32.crt_last);
    Fat32Entry.m_cluster.Format("0x%04X", m_sfn.type.fat32.strtclst32);
    if (Fat32Entry.DoModal() == IDOK) {
      m_sfn.type.fat32.nt_resv = convert8(Fat32Entry.m_nt_resv);
      m_sfn.type.fat32.crt_time_tenth = convert8(Fat32Entry.m_tenth);
      m_sfn.type.fat32.crt_time = convert16(Fat32Entry.m_time);
      m_sfn.type.fat32.crt_date = convert16(Fat32Entry.m_date);
      m_sfn.type.fat32.crt_last = convert16(Fat32Entry.m_last_acc);
      m_sfn.type.fat32.strtclst32 = convert16(Fat32Entry.m_cluster);
    }    
  } else
    SetDlgItemText(IDC_ENTRY_RESERVED, "00 00 00 00 00 00 00 00 00 00");
}

void CFatEntry::OnNext() {
  CString cs;
  
  DialogtoLFN();
  
  if ((m_lfn_cur + 1) < m_lfn_count) {
    m_lfn_cur++;
    LFNtoDialog();
  }
  
  GetDlgItem(IDNEXT)->EnableWindow(((m_lfn_cur + 1) < m_lfn_count));
  GetDlgItem(IDPREV)->EnableWindow(m_lfn_cur > 0);
  
  cs.Format("%i of %i", m_lfn_cur + 1, m_lfn_count);
  SetDlgItemText(IDC_SEQU_DISP, cs);
}

void CFatEntry::OnPrev() {
  CString cs;
  
  DialogtoLFN();
  
  if (m_lfn_cur > 0) {
    m_lfn_cur--;
    LFNtoDialog();
  }
  
  GetDlgItem(IDNEXT)->EnableWindow(((m_lfn_cur + 1) < m_lfn_count));
  GetDlgItem(IDPREV)->EnableWindow(m_lfn_cur > 0);
  
  cs.Format("%i of %i", m_lfn_cur + 1, m_lfn_count);
  SetDlgItemText(IDC_SEQU_DISP, cs);
}

void CFatEntry::OnTime() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned t = (time.GetHour() << 11) | (time.GetMinute() << 5) | (time.GetSecond() >> 1);
  cs.Format("0x%04X", t);
  SetDlgItemText(IDC_ENTRY_TIME, cs);
}

void CFatEntry::OnDate() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned d = ((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay();
  cs.Format("0x%04X", d);
  SetDlgItemText(IDC_ENTRY_DATE, cs);
}

void CFatEntry::OnChangeEntryDate() {
  CString cs, date;
  GetDlgItemText(IDC_ENTRY_DATE, date);
  
  unsigned d = convert16(date);
  cs.Format("%04i/%02i/%02i", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0);
  
  SetDlgItemText(IDC_DATE_DISP, cs);
}

void CFatEntry::OnChangeEntryTime() {
  CString cs, time;
  GetDlgItemText(IDC_ENTRY_TIME, time);
  
  unsigned t = convert16(time);
  cs.Format("%02i:%02i:%02i", t >> 11, (t & 0x07E0) >> 5, (t & 0x001F) << 1);
  
  SetDlgItemText(IDC_TIME_DISP, cs);
}

void CFatEntry::LFNtoDialog() {
  struct S_FAT_LFN_ROOT *lfn = (struct S_FAT_LFN_ROOT *) m_lfns;
  CString cs;
  
  cs.Format("0x%02X", lfn[m_lfn_cur].sequ_flags);
  SetDlgItemText(IDC_ENTRY_SEQU, cs);
  cs.Format("0x%02X", lfn[m_lfn_cur].attrb);
  SetDlgItemText(IDC_ENTRY_LFN_ATTRIB, cs);
  cs.Format("%02X", lfn[m_lfn_cur].resv);
  SetDlgItemText(IDC_ENTRY_LFN_RESV, cs);
  cs.Format("0x%02X", lfn[m_lfn_cur].sfn_crc);
  SetDlgItemText(IDC_ENTRY_SFN_CRC, cs);
  cs.Format("0x%04X", lfn[m_lfn_cur].clust_zero);
  SetDlgItemText(IDC_ENTRY_LFN_CLUST, cs);
  
  char *t, str[16];
  memset(str, 0, 16);
  t = str;
  WORD *s = lfn[m_lfn_cur].name0;
  for (int j=0; j<13; j++) {
    if (j==5)  s = lfn[m_lfn_cur].name1;
    if (j==11) s = lfn[m_lfn_cur].name2;
    *t++ = *s & 0x00FF;
    s++;
  }
  SetDlgItemText(IDC_ENTRY_LFN_NAME, str);
}

void CFatEntry::DialogtoLFN() {
  struct S_FAT_LFN_ROOT *lfn = (struct S_FAT_LFN_ROOT *) m_lfns;
  CString cs;
  
  if (m_lfn_cur > -1) {
    GetDlgItemText(IDC_ENTRY_SEQU, cs);
    lfn[m_lfn_cur].sequ_flags = convert8(cs);
    GetDlgItemText(IDC_ENTRY_LFN_ATTRIB, cs);
    lfn[m_lfn_cur].attrb = convert8(cs);
    GetDlgItemText(IDC_ENTRY_LFN_RESV, cs);
    lfn[m_lfn_cur].resv = convert8(cs);
    GetDlgItemText(IDC_ENTRY_SFN_CRC, cs);
    lfn[m_lfn_cur].sfn_crc = convert8(cs);
    GetDlgItemText(IDC_ENTRY_LFN_CLUST, cs);
    lfn[m_lfn_cur].clust_zero = convert16(cs);
    
    GetDlgItemText(IDC_ENTRY_LFN_NAME, cs);
    memset(lfn[m_lfn_cur].name0, 0, 10);
    memset(lfn[m_lfn_cur].name1, 0, 12);
    memset(lfn[m_lfn_cur].name2, 0, 4);
    WORD *t = lfn[m_lfn_cur].name0;
    int j, l = cs.GetLength();
    for (j=0; j<13; j++) {
      if (j==5)  t = lfn[m_lfn_cur].name1;
      if (j==11) t = lfn[m_lfn_cur].name2;
      if (j < l) *t = cs.GetAt(j);
      else if (j == l) *t = 0x0000;
      else *t = 0xFFFF;
      t++;
    }
  }
}

struct S_ATTRIBUTES fat_attrbs[] = {
                                 //            |                               | <- max (col 67)
  { FAT_ATTR_ARCHIVE,   FAT_ATTR_ARCHIVE,   0, "Archive"                        , {-1, } },
  { FAT_ATTR_SUB_DIR,   FAT_ATTR_SUB_DIR,   1, "Sub Directory"                  , {-1, } },
  { FAT_ATTR_VOLUME,    FAT_ATTR_VOLUME,    2, "Volume Label"                   , {-1, } },
  { FAT_ATTR_SYSTEM,    FAT_ATTR_SYSTEM,    3, "System File"                    , {-1, } },
  { FAT_ATTR_HIDDEN,    FAT_ATTR_HIDDEN,    4, "Hidden File"                    , {-1, } },
  { FAT_ATTR_READ_ONLY, FAT_ATTR_READ_ONLY, 5, "Read-Only"                      , {-1, } },
  { 0,                         (DWORD) -1, -1, NULL                             , {-1, } }
};

// display attributes
void CFatEntry::OnAttribute() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_ENTRY_ATTRIB, cs);
  dlg.m_title = "Entry Attribute";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = fat_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%02X", dlg.m_attrib);
    SetDlgItemText(IDC_ENTRY_ATTRIB, cs);
  }
}

void CFatEntry::OnFatentries() {
  CFatCList list;
  
  if (m_fat_entries.entry_count > 0) {
    list.m_fat_size = m_fat_size;
    list.m_entries = &m_fat_entries;
    list.DoModal();
  }
}

void CFatEntry::OnCrcUpdate() {
  struct S_FAT_LFN_ROOT *lfn = (struct S_FAT_LFN_ROOT *) m_lfns;
  CString cs;
  int i;
  
  UpdateData(TRUE);
  
  BYTE *p = (BYTE *) m_name.GetBuffer(8);
  BYTE Sum = 0;
  for (i=8; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  m_name.ReleaseBuffer(8);
  p = (BYTE *) m_ext.GetBuffer(3);
  for (i=3; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  m_ext.ReleaseBuffer(3);
  
  cs.Format("0x%02X", Sum);
  SetDlgItemText(IDC_ENTRY_SFN_CRC, cs);
}

void CFatEntry::OnShowBytes() {
  m_show_bytes = !m_show_bytes;
  CString string, cs;
  int i;
  
  if (m_show_bytes) {
    // because the name retrieved could possibly be null terminated,
    //  we must account for this because using GetAt() past the null
    //  will cause a "crash".  Therefore, we must parse up to the null
    //  padding with nulls ourselves
    GetDlgItemText(IDC_ENTRY_NAME, m_name);  // save it incase we have changed it in this dialog
    string = "";
    for (i=0; i<8 && i<m_name.GetLength(); i++) {
      cs.Format("%02X ", m_name.GetAt(i));
      string += cs;
    }
    for (; i<8; i++)
      string += "00 ";
    string.TrimRight();
    SetDlgItemText(IDC_ENTRY_NAME, string);

    GetDlgItemText(IDC_ENTRY_EXT, m_ext);  // save it incase we have changed it in this dialog
    string = "";
    for (i=0; i<3 && i<m_ext.GetLength(); i++) {
      cs.Format("%02X ", m_ext.GetAt(i));
      string += cs;
    }
    for (; i<3; i++)
      string += "00 ";
    string.TrimRight();
    SetDlgItemText(IDC_ENTRY_EXT, string);

  } else {
    SetDlgItemText(IDC_ENTRY_NAME, m_name);
    SetDlgItemText(IDC_ENTRY_EXT, m_ext);
  }
  GetDlgItem(IDC_ENTRY_NAME)->EnableWindow(!m_show_bytes);
  GetDlgItem(IDC_ENTRY_EXT)->EnableWindow(!m_show_bytes);
}
