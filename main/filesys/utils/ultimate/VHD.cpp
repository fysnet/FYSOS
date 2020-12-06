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

// VHD.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "VHD.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVHD property page

IMPLEMENT_DYNCREATE(CVHD, CPropertyPage)

CVHD::CVHD() : CPropertyPage(CVHD::IDD) {
  //{{AFX_DATA_INIT(CVHD)
  m_check_sum = _T("");
  m_cookie = _T("");
  m_creator = _T("");
  m_creator_host = _T("");
  m_creator_version = _T("");
  m_currect_size = _T("");
  m_data_offset = _T("");
  m_disk_type = _T("");
  m_features = _T("");
  m_guid = _T("");
  m_org_size = _T("");
  m_staved_state = _T("");
  m_time_stamp = _T("");
  m_time_show = _T("");
  m_version = _T("");
  m_dump = _T("");
  m_cylinder = _T("");
  m_head = _T("");
  m_sect_track = _T("");
  //}}AFX_DATA_INIT
  m_exists = FALSE;
  m_color = COLOR_MBR;
}

CVHD::~CVHD() {
}

void CVHD::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CVHD)
  DDX_Text(pDX, IDC_CHECK_SUM, m_check_sum);
  DDX_Text(pDX, IDC_COOKIE, m_cookie);
  DDX_Text(pDX, IDC_CREATOR, m_creator);
  DDX_Text(pDX, IDC_CREATOR_HOST, m_creator_host);
  DDX_Text(pDX, IDC_CREATOR_VERSION, m_creator_version);
  DDX_Text(pDX, IDC_CURRENT_SIZE, m_currect_size);
  DDX_Text(pDX, IDC_DATA_OFFSET, m_data_offset);
  DDX_Text(pDX, IDC_DISK_TYPE, m_disk_type);
  DDX_Text(pDX, IDC_FEATURES, m_features);
  DDX_Text(pDX, IDC_GUID, m_guid);
  DDX_Text(pDX, IDC_ORIGINAL_SIZE, m_org_size);
  DDX_Text(pDX, IDC_SAVED_STATE, m_staved_state);
  DDX_Text(pDX, IDC_TIME_STAMP, m_time_stamp);
  DDX_Text(pDX, IDC_TIME_SHOW, m_time_show);
  DDX_Text(pDX, IDC_VERSION, m_version);
  DDX_Text(pDX, IDC_DUMP, m_dump);
  DDX_Text(pDX, IDC_CYL, m_cylinder);
  DDX_Text(pDX, IDC_HEAD, m_head);
  DDX_Text(pDX, IDC_SECT, m_sect_track);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVHD, CPropertyPage)
  //{{AFX_MSG_MAP(CVHD)
  ON_BN_CLICKED(ID_VHD_APPLY, OnVhdApply)
  ON_EN_KILLFOCUS(IDC_GUID, OnKillfocusGuid)
  ON_BN_CLICKED(IDC_DISK_TYPE_SET, OnDiskTypeSet)
  ON_BN_CLICKED(IDC_VHD_CHECK_SUM, OnCRCSet)
  ON_BN_CLICKED(IDC_VHD_GUID_UPDATE, OnGUIDUpdate)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVHD message handlers
BOOL CVHD::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_DUMP)->SetFont(&dlg->m_DumpFont);
  
  return TRUE;
}

bool CVHD::Exists(DWORD64 LBA) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  if (LBA == 0)
    return FALSE;
  
  dlg->ReadFromFile(m_buffer, LBA, 1);
  struct VHD_FOOTER *footer = (struct VHD_FOOTER *) m_buffer;
  
  if (memcmp(footer->cookie, "conectix", 8) != 0)
    return FALSE;
  
  m_exists = TRUE;
  m_lba = LBA;
  
  return m_exists;
}

void CVHD::Begin(void) {
  struct VHD_FOOTER *footer = (struct VHD_FOOTER *) m_buffer;
  DWORD64 timestamp = ((INT64) ENDIAN_32U(footer->time_stamp) + (time_t) 946708560);
  CTime cTime((time_t) timestamp);  // converts seconds since 1 Jan 1970 to CTime

  m_check_sum.Format("0x%08X", ENDIAN_32U(footer->checksum));
  m_cookie = "conectix";
  m_creator.Format("%c%c%c%c", footer->creator_ap[0], footer->creator_ap[1], footer->creator_ap[2], footer->creator_ap[3]);
  m_creator_host.Format("0x%08X", ENDIAN_32U(footer->creator_host_os));
  m_creator_version.Format("0x%08X", ENDIAN_32U(footer->creator_ver));
  m_currect_size.Format("%I64i", ENDIAN_64U(footer->current_size));
  m_data_offset.Format("%I64i", ENDIAN_64U(footer->data_offset));
  m_disk_type.Format("0x%08X", ENDIAN_32U(footer->disk_type));
  m_features.Format("0x%08X", ENDIAN_32U(footer->features));
  GUID_Format(m_guid, &footer->uuid);
  m_org_size.Format("%I64i", ENDIAN_64U(footer->original_size));
  m_staved_state.Format("0x%02X", footer->saved_state);
  m_time_stamp.Format("0x%08X", ENDIAN_32U(footer->time_stamp));
  m_time_show = cTime.Format(_T("%Y %b %d %H:%M:%S"));
  m_version.Format("0x%08X", ENDIAN_32U(footer->version));
  m_cylinder.Format("%i", ENDIAN_16U(footer->disk_geometry.cylinder));
  m_head.Format("%i", footer->disk_geometry.heads);
  m_sect_track.Format("%i", footer->disk_geometry.spt);
  DumpIt(m_dump, footer->reserved, 512-427, 427, FALSE);
}

void CVHD::OnVhdApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct VHD_FOOTER *footer = (struct VHD_FOOTER *) m_buffer;
  DWORD64 qword;
  DWORD dword;
  WORD word;
  
  UpdateData(TRUE); // bring from Dialog
  
  dword = convert32(m_check_sum); footer->checksum = ENDIAN_32U(dword);
  footer->creator_ap[0] = m_creator.GetAt(0);
  footer->creator_ap[1] = m_creator.GetAt(1);
  footer->creator_ap[2] = m_creator.GetAt(2);
  footer->creator_ap[3] = m_creator.GetAt(3);
  dword = convert32(m_creator_host); footer->creator_host_os = ENDIAN_32U(dword);
  dword = convert32(m_creator_version); footer->creator_ver = ENDIAN_32U(dword);
  qword = convert64(m_currect_size); footer->current_size = ENDIAN_64U(qword);
  qword = convert64(m_data_offset); footer->data_offset = ENDIAN_64U(qword);
  dword = convert32(m_disk_type); footer->disk_type = ENDIAN_32U(dword);
  dword = convert32(m_features); footer->features = ENDIAN_32U(dword);
  GUID_Retrieve(m_guid, &footer->uuid);
  qword = convert64(m_org_size); footer->original_size = ENDIAN_64U(qword);
  footer->saved_state = convert8(m_staved_state);
  dword = convert32(m_time_stamp); footer->time_stamp = ENDIAN_32U(dword);
  dword = convert32(m_version); footer->version = ENDIAN_32U(dword);
  word = convert16(m_cylinder); footer->disk_geometry.cylinder = ENDIAN_16U(word);
  footer->disk_geometry.heads = convert8(m_head);
  footer->disk_geometry.spt = convert8(m_sect_track);
  
  dlg->WriteToFile(m_buffer, m_lba, 1);
}

void CVHD::OnKillfocusGuid() {
  CString cs;
  GetDlgItemText(IDC_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("UUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    GetDlgItem(IDC_GUID)->SetFocus();
  }
}

struct S_ATTRIBUTES vhd_disk_types[] = {
             //           |                               | <- max (col 67)
  { 0, (DWORD) -1,     0, "None"                           , {-1, } },
  { 2, (DWORD) -1,     1, "Fixed"                          , {-1, } },
  { 3, (DWORD) -1,     2, "Dynamic"                        , {-1, } },
  { 4, (DWORD) -1,     3, "Differencing"                   , {-1, } },
  { 0, (DWORD) -1,    -1, NULL                             , {-1, } }
};

void CVHD::OnDiskTypeSet() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_DISK_TYPE, cs);
  dlg.m_title = "VHD Disk Type";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = vhd_disk_types;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%08X", dlg.m_attrib);
    SetDlgItemText(IDC_DISK_TYPE, cs);
  }
}

void CVHD::OnCRCSet() {

  // we will have to "pull" all items from the dialog,
  //  save to a 512-byte buffer, then calculate the CRC
  AfxMessageBox("To Do");

}

// create new random GUID for this footer
void CVHD::OnGUIDUpdate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_GUID, cs);
}
