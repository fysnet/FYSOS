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

// Embr.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Embr.h"
#include "EmbrEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmbr property page

IMPLEMENT_DYNCREATE(CEmbr, CPropertyPage)

CEmbr::CEmbr() : CPropertyPage(CEmbr::IDD){
  //{{AFX_DATA_INIT(CEmbr)
  m_boot_sig = _T("");
  m_entry_offset = _T("");
  m_remaining = _T("");
  m_embr_sig = _T("");
  m_embr_dump = _T("");
  m_entry_crc = _T("");
  m_entries = _T("");
  m_reserved = _T("");
  m_sig0 = _T("");
  m_sig1 = _T("");
  m_boot_delay = 0;
  m_tot_sectors = _T("");
  m_version = _T("");
  //}}AFX_DATA_INIT
  
  m_color = COLOR_EMBR;
  m_entry_buffer = NULL;
  m_embr_entries = 0;
  m_hdr_valid = FALSE;
}

CEmbr::~CEmbr() {
}

void CEmbr::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CEmbr)
  DDX_Control(pDX, IDC_EMBR_PAGES, m_embr_pages);
  DDX_Text(pDX, IDC_EMBR_BOOT_SIG, m_boot_sig);
  DDX_Text(pDX, IDC_EMBR_ENTRY_OFFSET, m_entry_offset);
  DDX_Text(pDX, IDC_EMBR_REMAINING, m_remaining);
  DDX_Text(pDX, IDC_EMBR_SIG, m_embr_sig);
  DDX_Text(pDX, IDC_EMBR_DUMP, m_embr_dump);
  DDX_Text(pDX, IDC_EMBR_CRC, m_entry_crc);
  DDX_Text(pDX, IDC_EMBR_ENTRIES, m_entries);
  DDX_Text(pDX, IDC_EMBR_RESV, m_reserved);
  DDX_Text(pDX, IDC_EMBR_SIG0, m_sig0);
  DDX_Text(pDX, IDC_EMBR_SIG1, m_sig1);
  DDX_Text(pDX, IDC_EMBR_BOOT_DELAY, m_boot_delay);
  DDV_MinMaxInt(pDX, m_boot_delay, 0, 120);
  DDX_Text(pDX, IDC_EMBR_TOT_SECTORS, m_tot_sectors);
  DDX_Text(pDX, IDC_EMBR_VERSION, m_version);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEmbr, CPropertyPage)
  //{{AFX_MSG_MAP(CEmbr)
  ON_BN_CLICKED(ID_EMBR_APPLY, OnEmbrApply)
  ON_BN_CLICKED(IDC_BOOT_SIG_UPDATE, OnBootSigUpdate)
  ON_BN_CLICKED(IDC_CRC_SET, OnCrcSet)
  ON_BN_CLICKED(IDC_RESV_CLEAR, OnResvClear)
  ON_BN_CLICKED(IDC_SIG0_SET, OnSig0Set)
  ON_BN_CLICKED(IDC_SIG1_SET, OnSig1Set)
  ON_BN_CLICKED(IDC_SIGNATURE_SET, OnSignatureSet)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_EN_CHANGE(IDC_EMBR_VERSION, OnChangeEmbrVersion)
  ON_BN_CLICKED(IDC_UPDATE_TOT_SECTS, OnUpdateTotSects)
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmbr message handlers

BOOL CEmbr::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_EMBR_DUMP)->SetFont(&dlg->m_DumpFont);
  
  OnChangeEmbrVersion();
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, convert64(m_tot_sectors));
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  return TRUE;
}

BOOL CEmbr::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "embr.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

bool CEmbr::Exists(DWORD64 LBA) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD org_crc;
  int i;
  
  BYTE buffer[MAX_SECT_SIZE];
  struct S_EMBR_SIG *sig = (struct S_EMBR_SIG *) &buffer[0x1F2];
  
  dlg->ReadFromFile(buffer, LBA + 1, 1);
  
  // save the LBA
  m_lba = LBA;
  
  // does the signature match?
  m_exists = (memcmp(&sig->sig, "EmbrrbmE", 8) == 0);
  
  if (m_exists) {
    m_embr_sig = "EmbrrbmE";
    m_entry_offset.Format("%i", sig->offset);
    m_remaining.Format("%i", sig->remaining);
    m_boot_sig.Format("0x%04X", sig->boot_sig);
    DumpIt(m_embr_dump, buffer, 0, 512, FALSE);
    
    // allocate the memory for the entries
    m_entry_buffer = malloc((sig->remaining - sig->offset - 1) * dlg->m_sect_size);
    dlg->ReadFromFile(m_entry_buffer, m_lba + sig->offset, sig->remaining - sig->offset - 1);
    struct S_EMBR_HDR *hdr = (struct S_EMBR_HDR *) m_entry_buffer;
    
    // check the CRC of the entries
    org_crc = hdr->crc;
    hdr->crc = 0;
    if (crc32(hdr, sizeof(struct S_EMBR_HDR) + (hdr->entry_count * sizeof(struct S_EMBR_ENTRY))) != org_crc) {
      if (AfxMessageBox("eMBR: Found invalid hdr->crc.  Ignore?", MB_YESNO, 0) != IDYES) {
        free(m_entry_buffer);
        return (m_exists = FALSE);
      }
    }
    hdr->crc = org_crc;
    
    m_boot_delay = hdr->boot_delay;
    m_version.Format("0x%02X", hdr->version);
    m_tot_sectors.Format("%I64i", hdr->total_sectors);
    m_entry_crc.Format("0x%08X", hdr->crc);
    m_entries.Format("%i", hdr->entry_count);
    
    m_reserved.Empty();
    CString cs;
    for (i=0; i<8; i++) {
      cs.Format("%02X ", hdr->resv1[i]);
      m_reserved += cs;
    }
    
    m_sig0.Format("0x%08X", hdr->sig0);
    m_sig1.Format("0x%08X", hdr->sig1);
    if ((hdr->sig0 == EMBR_HDR_SIG0) && (hdr->sig1 == EMBR_HDR_SIG1)) {
      m_hdr_valid = TRUE;
      m_embr_entries = 0;
      m_total_sectors = hdr->total_sectors;
      
      for (i=0; i<hdr->entry_count && i<MAX_EMBR_ENTRIES; i++) {
        m_Pages[i].Construct(IDD_EMBR_ENTRY, 0);
        m_Pages[i].m_Title.Format("Entry %i", i);
        if (CheckEntry(i))
          m_Pages[i].m_Title += " !";
        m_Pages[i].m_psp.dwFlags |= PSP_USETITLE;
        m_Pages[i].m_psp.pszTitle = m_Pages[i].m_Title;
        UpdateEntry(&m_Pages[i], i);
        m_Pages[i].m_index = i;
        m_Pages[i].m_entry_buffer = m_entry_buffer;
        m_Sheet.AddPage(&m_Pages[i]);
        m_embr_entries++;
      }
    }
  }
  
  return m_exists;
}

void CEmbr::Begin(void) {
  CRect rect;
  
  // if the header is valid, enable those controls
  if (m_hdr_valid) {
    GetDlgItem(IDC_EMBR_BOOT_DELAY)->EnableWindow(TRUE);
    GetDlgItem(IDC_RESV_CLEAR)->EnableWindow(TRUE);
    GetDlgItem(IDC_EMBR_ENTRIES)->EnableWindow(TRUE);
  }
  
  m_Sheet.EnableStackedTabs(FALSE);
  m_Sheet.Create(this, WS_VISIBLE | WS_CHILD | WS_DLGFRAME);
  m_Sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
  m_embr_pages.GetWindowRect(&rect);
  ScreenToClient(&rect);
  m_Sheet.MoveWindow(&rect);
  
  // we need to resize the tabcontrol part
  if (m_Sheet.GetTabControl()) {
    m_Sheet.GetTabControl()->GetWindowRect(rect);
    m_Sheet.GetTabControl()->SetWindowPos(NULL, 0, 0,
      rect.Width() - 72,
      rect.Height(),
      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }
  
  // this updates the size of the active page as well
  m_Sheet.SetActivePage(0);
}

void CEmbr::UpdateEntry(CEmbrEntry *Entry, int index) {
  struct S_EMBR_ENTRY *entry = (struct S_EMBR_ENTRY *) ((BYTE *) m_entry_buffer + sizeof(struct S_EMBR_HDR) + (index * sizeof(S_EMBR_ENTRY)));
  
  Entry->m_flags.Format("0x%08X", entry->flags);
  Entry->m_signature.Format("0x%08X", entry->signature);
  Entry->m_start_lba.Format("%I64i", entry->starting_sector);
  Entry->m_sectors.Format("%I64i", entry->sector_count);
  Entry->m_created.Format("%I64i", entry->date_created);
  Entry->m_last_booted.Format("%I64i", entry->date_last_booted);
  Entry->m_os_sig.Format("0x%08X", entry->OS_signature);
  Entry->m_description = entry->description;
  
  Entry->m_reserved.Empty();
  CString cs;
  for (int i=0; i<16; i++) {
    cs.Format("%02X ", entry->reserved[i]);
    Entry->m_reserved += cs;
  }
}

// Returns TRUE if entry is in error
BOOL CEmbr::CheckEntry(int index) {
  struct S_EMBR_ENTRY *entries = (struct S_EMBR_ENTRY *) ((BYTE *) m_entry_buffer + sizeof(struct S_EMBR_HDR));
  DWORD64 start0, last0, last1;
  BOOL ret = FALSE;
  CString cs;
  int i;
  
  // check for overlapping of previous entries
  for (i=0; i<index; i++) {
    start0 = entries[i].starting_sector;
    last0 = entries[i].starting_sector + entries[i].sector_count - 1;
    last1 = entries[index].starting_sector + entries[index].sector_count - 1;
    if (((entries[index].starting_sector >= start0) && (entries[index].starting_sector <= last0)) ||
        ((last1 >= start0) && (last1 <= last0))) {
      cs.Format("eMBR: Entry #%i overlaps Entry #%i!", index, i);
      AfxMessageBox(cs);
      ret = TRUE;
    }
  }
  
  return ret;
}

void CEmbr::Destroy(void) {
  while (m_Sheet.GetPageCount())
    m_Sheet.RemovePage(0);
  m_Sheet.DestroyWindow();
  
  if (m_entry_buffer)
    free(m_entry_buffer);
  m_entry_buffer = NULL;
  
  m_exists = FALSE;
  m_hdr_valid = FALSE;
  m_embr_entries = 0;
}

void CEmbr::OnEmbrApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  int i;
  
  UpdateData(TRUE); // bring from Dialog
  
  // update the Signature in LBA 1 (LSN 0)
  struct S_EMBR_SIG *sig = (struct S_EMBR_SIG *) &buffer[0x1F2];
  dlg->ReadFromFile(buffer, m_lba + 1, 1);
  sig->offset = convert16(m_entry_offset);
  sig->remaining = convert16(m_remaining);
  dlg->WriteToFile(buffer, m_lba + 1, 1);
  
  struct S_EMBR_HDR *hdr = (struct S_EMBR_HDR *) m_entry_buffer;
  hdr->boot_delay = m_boot_delay;
  hdr->version = convert8(m_version);
  hdr->total_sectors = convert64(m_tot_sectors);
  hdr->crc = convert32(m_entry_crc);
  hdr->entry_count = convert16(m_entries);
  
  // get the reserved field
  CString cs;
  for (i=0; i<8; i++) {
    // the format will be "00 00 00 00 ..."
    cs = "0x" + m_reserved.Mid(i * 3, 2);
    hdr->resv1[i] = convert8(cs);
  }
  
  hdr->sig0 = convert32(m_sig0);
  hdr->sig1 = convert32(m_sig1);
  
  // update the entries
  for (i=0; i<m_embr_entries; i++) {
    if (m_Pages[i].m_dirty)
      m_Pages[i].OnEmbreApply();
  }
  dlg->WriteToFile(m_entry_buffer, m_lba + sig->offset, sig->remaining - sig->offset - 1);
}

void CEmbr::OnSignatureSet() {
  SetDlgItemText(IDC_SIGNATURE_SET, "EmbrrbmE");
}

void CEmbr::OnBootSigUpdate() {
  SetDlgItemText(IDC_EMBR_BOOT_SIG, "0xAA55");
}

// we get the count of entries from the Dialog, *not* the saved value of the dialog *or* the hdr->entry value
void CEmbr::OnCrcSet() {
  CString cs;
  
  // get and check the entry count
  GetDlgItemText(IDC_EMBR_ENTRIES, cs);
  const unsigned entry_count = convert32(cs);
  if ((entry_count == 0) || (entry_count > MAX_EMBR_ENTRIES)) {
    AfxMessageBox("Error with Entry Count...");
    return;
  }
  
  const unsigned size = sizeof(struct S_EMBR_HDR) + (entry_count * sizeof(struct S_EMBR_ENTRY));
  struct S_EMBR_HDR *hdr = (struct S_EMBR_HDR *) m_entry_buffer;
  
  hdr->crc = 0;
  hdr->crc = crc32(hdr, size);
  
  cs.Format("0x%08X", hdr->crc);
  SetDlgItemText(IDC_EMBR_CRC, cs);
}

void CEmbr::OnResvClear() {
  SetDlgItemText(IDC_EMBR_RESV, "00 00 00 00 00 00 00 00");
}

void CEmbr::OnSig0Set() {
  SetDlgItemText(IDC_EMBR_SIG0, "0x52424D45");
}

void CEmbr::OnSig1Set() {
  SetDlgItemText(IDC_EMBR_SIG1, "0x454D4252");
}

// allow the user to update LSN 0 -> (LSN of entries - 1)
//  not overwriting the sig in LSN 0
// if the code is larger than the space allowed (LSN 0 -> LSN of entries - 1),
//  give error and quit.
void CEmbr::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString csPath;
  CFile file;
  
  CFileDialog odlg (
    TRUE,             // Create an open file dialog
    _T(".bin"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  
  // use a default path?
  csPath = AfxGetApp()->GetProfileString("Settings", "DefaultEMBRPath", NULL);
  if (!csPath.IsEmpty())
    odlg.m_ofn.lpstrInitialDir = csPath;
  odlg.m_ofn.lpstrTitle = "Update Code Image File";
  
  if (odlg.DoModal() != IDOK)
    return;
  
  POSITION pos = odlg.GetStartPosition();
  csPath = odlg.GetNextPathName(pos);
  
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening Binary File...");
    return;
  }
  
  // apply all changes made (if any)
  OnEmbrApply();
  
  // get the size of the code
  UINT file_sectors = (UINT) ((file.GetLength() + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  
  // read in the Signature in LBA 1 (LSN 0)
  dlg->ReadFromFile(buffer, m_lba + 1, 1);
  struct S_EMBR_SIG *sig = (struct S_EMBR_SIG *) &buffer[0x1F2];
  if (file_sectors > (unsigned) (sig->offset - 1)) {
    AfxMessageBox("Code file is larger than space allocated prior to partition entries.\r\n"
                  "Update the Offset field and try again.");
    file.Close();
    return;
  }
  
  // read in the code file
  BYTE *code = (BYTE *) malloc(file_sectors * dlg->m_sect_size);
  file.Read(code, file_sectors * dlg->m_sect_size);
  file.Close();
  
  // Copy the current S_EMBR_SIG to the new code buffer
  memcpy(&code[0x1F2], sig, sizeof(struct S_EMBR_SIG));
  
  // write the new code
  dlg->WriteToFile(code, m_lba + 1, file_sectors);
  
  // update the dump display box
  DumpIt(m_embr_dump, code, 0, 512, FALSE);
  SetDlgItemText(IDC_EMBR_DUMP, m_embr_dump);
  
  free(code);
}

void CEmbr::OnChangeEmbrVersion() {
  CString cs;
  GetDlgItemText(IDC_EMBR_VERSION, cs);
  BYTE byte = convert8(cs);
  if (byte > 0)
    cs.Format("%i.%02i", byte >> 5, byte & 0x1F);
  else
    cs = "org";
  SetDlgItemText(IDC_EMBR_VERSION_DISP, cs);
}

void CEmbr::OnUpdateTotSects() {
  struct S_EMBR_ENTRY *entry = (struct S_EMBR_ENTRY *) ((BYTE *) m_entry_buffer + sizeof(struct S_EMBR_HDR));
  struct S_EMBR_HDR *hdr = (struct S_EMBR_HDR *) m_entry_buffer;
  int i;
  
  for (i=0; i<m_embr_entries; i++) {
    if (m_Pages[i].m_dirty)
      m_Pages[i].OnEmbreApply();
    if ((entry[i].starting_sector + entry[i].sector_count) > m_total_sectors)
      m_total_sectors = (entry[i].starting_sector + entry[i].sector_count);
  }
  
  CString cs;
  cs.Format("%I64i", m_total_sectors);
  SetDlgItemText(IDC_EMBR_TOT_SECTORS, cs);
}
