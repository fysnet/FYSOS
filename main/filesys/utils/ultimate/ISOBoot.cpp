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

// ISOBoot.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "ISOBoot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CISOBoot property page

IMPLEMENT_DYNCREATE(CISOBoot, CPropertyPage)

CISOBoot::CISOBoot() : CPropertyPage(CISOBoot::IDD) {
  //{{AFX_DATA_INIT(CISOBoot)
  m_type = 0;
  m_boot_cat_lba = 0;
  m_cd001 = _T("");
  m_version = _T("");
  m_sys_id = _T("");
  m_sys_use = _T("");
  m_boot_id = _T("");
  m_val_crc = _T("");
  m_val_id = _T("");
  m_val_ident = _T("");
  m_val_key55 = _T("");
  m_val_keyaa = _T("");
  m_val_reserved = _T("");
  m_di_count = _T("");
  m_di_lba = _T("");
  m_di_load_seg = _T("");
  m_di_reserved = _T("");
  m_di_sys_type = _T("");
  //}}AFX_DATA_INIT
}

CISOBoot::~CISOBoot() {
  m_is_valid = FALSE;
}

void CISOBoot::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CISOBoot)
  DDX_Control(pDX, IDC_DI_MEDIA, m_di_media);
  DDX_Control(pDX, IDC_DI_BOOTABLE, m_di_bootable);
  DDX_Control(pDX, IDC_VAL_PLATFORM, m_val_platform);
  DDX_Text(pDX, IDC_TYPE, m_type);
  DDX_Text(pDX, IDC_BOOT_CAT_LBA, m_boot_cat_lba);
  DDX_Text(pDX, IDC_CD001, m_cd001);
  DDX_Text(pDX, IDC_VERSION, m_version);
  DDX_Text(pDX, IDC_SYS_ID, m_sys_id);
  DDX_Text(pDX, IDC_SYS_USE, m_sys_use);
  DDX_Text(pDX, IDC_BOOT_ID, m_boot_id);
  DDX_Text(pDX, IDC_VAL_CRC, m_val_crc);
  DDX_Text(pDX, IDC_VAL_ID, m_val_id);
  DDX_Text(pDX, IDC_VAL_IDENT, m_val_ident);
  DDX_Text(pDX, IDC_VAL_KEY55, m_val_key55);
  DDX_Text(pDX, IDC_VAL_KEYAA, m_val_keyaa);
  DDX_Text(pDX, IDC_VAL_RESERVED, m_val_reserved);
  DDX_Text(pDX, IDC_DI_COUNT, m_di_count);
  DDX_Text(pDX, IDC_DI_LBA, m_di_lba);
  DDX_Text(pDX, IDC_DI_LOAD_SEG, m_di_load_seg);
  DDX_Text(pDX, IDC_DI_RESERVED, m_di_reserved);
  DDX_Text(pDX, IDC_DI_SYS_TYPE, m_di_sys_type);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CISOBoot, CPropertyPage)
  //{{AFX_MSG_MAP(CISOBoot)
  ON_BN_CLICKED(ID_APPLY, OnApplyB)
  ON_BN_CLICKED(IDC_VAL_CRC_UPDATE, OnValCrcUpdate)
  ON_BN_CLICKED(IDC_VAL_KEY_UPDATE, OnValKeyUpdate)
  ON_BN_CLICKED(IDC_DI_INSERT, OnDiInsert)
  ON_BN_CLICKED(IDC_DI_EXTRACT, OnDiExtract)
  ON_BN_CLICKED(IDC_CLEAR_RESERVED, OnClearReserved)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CISOBoot::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    case IDC_DI_INSERT:
      strTipText.Format("Insert Boot Code at LBA %i", GetDlgItemInt(IDC_DI_LBA, 0, FALSE));
      break;
    case IDC_DI_EXTRACT:
      strTipText.Format("Extract %i sectors of Boot Code from LBA %i", GetDlgItemInt(IDC_DI_COUNT, 0, FALSE), GetDlgItemInt(IDC_DI_LBA, 0, FALSE));
      break;
    case IDC_DI_COUNT:
      strTipText = "Count of 512-byte sectors to load";
      break;
    case IDC_DI_LBA:
      strTipText = "2048-byte LBA of boot code";
      break;
    case IDC_CLEAR_RESERVED:
      strTipText = "Clear the Reserved Area";
      break;
    case IDC_VAL_CRC_UPDATE:
      strTipText = "Update the CRC";
      break;
    case IDC_VAL_KEY_UPDATE:
      strTipText = "Update the Key";
      break;
    case IDC_DI_LOAD_SEG:
      strTipText = "Usually 0x0000, can be 0x07C0";
      break;
    case IDC_DI_MEDIA:
      strTipText = "Emulated Media type";
      break;
    case IDC_DI_BOOTABLE:
      strTipText = "Bootable";
      break;

    case ID_APPLY:
      strTipText = "Save modifications";
      break;
   }

   strncpy(pTTTA->szText, strTipText, 79);
   pTTTA->szText[79] = '\0';  // make sure it is null terminated

   return TRUE; // message was handled
}

/////////////////////////////////////////////////////////////////////////////
// CISOBoot message handlers
BOOL CISOBoot::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_SYS_USE)->SetFont(&dlg->m_DumpFont);
  
  // we have to update controls *after* the dialog is created so we put it here
  SendToDialog();

  EnableToolTips(TRUE);

  return TRUE;
}

void CISOBoot::Start(const DWORD64 lba, DWORD color, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) m_descriptor;
  
  m_is_valid = TRUE;
  m_lba = lba;
  m_size = 1; ////
  m_color = color;
  
  m_boot_cat_lba = bvd->boot_cat;   // sector containing boot catalog.
  if (m_boot_cat_lba > 0)
    dlg->ReadFromFile(m_boot_cat, m_boot_cat_lba, 1);
  
  DWORD offset = (DWORD) ((BYTE *) bvd->sys_use - (BYTE *) bvd);
  DumpIt(m_sys_use, bvd->sys_use, offset, 2048 - offset, FALSE);
}

void CISOBoot::SendToDialog(void) {
  struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) m_descriptor;
  
  m_type = 0;
  m_cd001 = "CD001";
  m_version.Format("%i", bvd->ver);
  
  memcpy(m_sys_id.GetBuffer(33), bvd->sys_ident, 32); m_sys_id.ReleaseBuffer(32);
  memcpy(m_boot_id.GetBuffer(33), bvd->boot_ident, 32); m_boot_id.ReleaseBuffer(32);
  
  if (m_boot_cat_lba > 0) {
    struct S_ISO_BC_VALIDATION *validation = (struct S_ISO_BC_VALIDATION *) m_boot_cat;
    m_val_crc.Format("0x%04X", validation->crc);
    m_val_id.Format("%i", validation->id);
    memcpy(m_val_ident.GetBuffer(25), validation->ident, 24); m_val_ident.ReleaseBuffer(24);
    m_val_key55.Format("0x%02X", validation->key55);
    m_val_keyaa.Format("0x%02X", validation->keyAA);
    m_val_reserved.Format("0x%04X", validation->resv0);
    if ((validation->platform <= 2) || (validation->platform == 0xEF)) {
      if (validation->platform == 0xEF)
        validation->platform = 3;
      m_val_platform.SetCurSel(validation->platform);
    }
    
    struct S_ISO_BC_ENTRY_EXT *entry = (struct S_ISO_BC_ENTRY_EXT *) ((BYTE *) m_boot_cat + sizeof(struct S_ISO_BC_VALIDATION));
    m_di_count.Format("%i", entry->load_cnt);
    m_di_lba.Format("%i", entry->load_rba);
    m_di_load_seg.Format("0x%04X", entry->load_seg);
    m_di_reserved.Format("0x%02X", entry->resv0);
    m_di_sys_type.Format("0x%02X", entry->sys_type);
    if (entry->bootable == 0x88)
      m_di_bootable.SetCurSel(1);
    else if (entry->bootable == 0x00)
      m_di_bootable.SetCurSel(0);
    if (entry->media <= 4)
      m_di_media.SetCurSel(entry->media);
  }
  
  UpdateData(FALSE);
}

void CISOBoot::ReceiveFromDialog(void) {
  struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) m_descriptor;
  UpdateData(TRUE);
  
  bvd->ver = convert8(m_version);
  
  memset(bvd->sys_ident, 0x20, 32);
  memcpy(bvd->sys_ident, m_sys_id, (m_sys_id.GetLength() < 32) ? m_sys_id.GetLength() : 32);
  memset(bvd->boot_ident, 0x20, 32);
  memcpy(bvd->boot_ident, m_boot_id, (m_boot_id.GetLength() < 32) ? m_boot_id.GetLength() : 32);
  
  bvd->boot_cat = m_boot_cat_lba;
  if (m_boot_cat_lba > 0) {
    struct S_ISO_BC_VALIDATION *validation = (struct S_ISO_BC_VALIDATION *) m_boot_cat;
    validation->crc = convert16(m_val_crc);
    validation->id = convert8(m_val_id);
    memset(validation->ident, 0x20, 24);
    memcpy(validation->ident, m_val_ident, (m_val_ident.GetLength() < 24) ? m_val_ident.GetLength() : 24);
    validation->key55 = convert8(m_val_key55);
    validation->keyAA = convert8(m_val_keyaa);
    validation->resv0 = convert16(m_val_reserved);
    validation->platform = m_val_platform.GetCurSel();
    if (validation->platform == 3)
      validation->platform = 0xEF;
    
    struct S_ISO_BC_ENTRY_EXT *entry = (struct S_ISO_BC_ENTRY_EXT *) ((BYTE *) m_boot_cat + sizeof(struct S_ISO_BC_VALIDATION));
    entry->load_cnt = convert16(m_di_count);
    entry->load_rba = convert32(m_di_lba);
    entry->load_seg = convert16(m_di_load_seg);
    entry->resv0 = convert8(m_di_reserved);
    entry->sys_type = convert8(m_di_sys_type);
    if (m_di_bootable.GetCurSel() == 0)
      entry->bootable = 0;
    else if (m_di_bootable.GetCurSel() == 1)
      entry->bootable = 0x88;
    entry->media = m_di_media.GetCurSel();
  }
}

void CISOBoot::OnApplyB() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  ReceiveFromDialog();
  
  dlg->WriteToFile(m_descriptor, m_lba, 1);
  if (m_boot_cat_lba > 0)
    dlg->WriteToFile(m_boot_cat, m_boot_cat_lba, 1);
}

// Calculate and update the CRC field
void CISOBoot::OnValCrcUpdate() {
  
  // Update m_boot_cat with dialog data
  ReceiveFromDialog();
  
  WORD crc = 0, *crc_p = (WORD *) m_boot_cat;
  for (int i=0; i<16; i++) {
    if (i == 14) continue;  // skip current crc value
    crc += crc_p[i];
  }
  
  CString cs;
  cs.Format("0x%04X", (WORD) -crc);
  SetDlgItemText(IDC_VAL_CRC, cs);
}

void CISOBoot::OnValKeyUpdate() {
  SetDlgItemText(IDC_VAL_KEY55, "0x55");
  SetDlgItemText(IDC_VAL_KEYAA, "0xAA");
}

void CISOBoot::OnDiInsert() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  CFileDialog odlg (
    TRUE,             // Create an open file dialog
    _T(".bin"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  if (odlg.DoModal() != IDOK)
    return;
  POSITION pos = odlg.GetStartPosition();
  CString csPath = odlg.GetNextPathName(pos);
  
  UpdateData(TRUE); // bring from dialog
  
  BYTE buffer[MAX_SECT_SIZE];
  UINT count = (convert32(m_di_count) + 3) / 4;  // count is count of 512-byte sectors (we convert to 2048-byte sectors)
  UINT lba = convert32(m_di_lba);
  CFile file;
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
    if (file.GetLength() > (count * 2048))
      AfxMessageBox("File to insert is larger than 'count' sectors.  File will be truncated");
    while (count--) {
      memset(buffer, 0, MAX_SECT_SIZE);  // clear the buffer incase we don't read a full 2048 bytes
      file.Read(buffer, 2048);           // this could happen if the file to insert is smaller than 'count' sectors
      dlg->WriteToFile(buffer, lba++, 1);
    }
    file.Close();
    AfxMessageBox("Boot image inserted successfully.");
  } else
    AfxMessageBox("Error Opening File...");
}

void CISOBoot::OnDiExtract() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  CFileDialog odlg (
    FALSE,            // Create an saveas file dialog
    NULL,             // Default file extension
    "bootfile.img",   // Default Filename
    OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT, // flags
    NULL
  );
  if (odlg.DoModal() != IDOK)
    return;
  POSITION pos = odlg.GetStartPosition();
  CString csPath = odlg.GetNextPathName(pos);
  
  UpdateData(TRUE); // bring from dialog
  
  BYTE buffer[MAX_SECT_SIZE];
  UINT count = convert32(m_di_count);  // count is count of 512-byte sectors
  UINT lba = convert32(m_di_lba);
  CFile file;
  int i;
  if (file.Open(csPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
    while (count) {
      dlg->ReadFromFile(buffer, lba++, 1);
      for (i=0; i<4 && count; i++, count--)
        file.Write(&buffer[i*512], 512);
    }
    file.Close();
    AfxMessageBox("Boot image extracted successfully.");
  } else
    AfxMessageBox("Error Creating File...");
}

void CISOBoot::OnClearReserved() {
  struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) m_descriptor;
  
  DWORD offset = (DWORD) ((BYTE *) bvd->sys_use - (BYTE *) bvd);
  memset(bvd->sys_use, 0, 2048 - offset);
  DumpIt(m_sys_use, bvd->sys_use, offset, 2048 - offset, FALSE);
  SetDlgItemText(IDC_SYS_USE, m_sys_use);
}
