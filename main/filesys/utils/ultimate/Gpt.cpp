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

// Gpt.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Gpt.h"

#include "Modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGpt property page
IMPLEMENT_DYNCREATE(CGpt, CPropertyPage)

CGpt::CGpt() : CPropertyPage(CGpt::IDD) {
  //{{AFX_DATA_INIT(CGpt)
  m_GptDump = _T("");
  m_backup_lba = _T("");
  m_crc32 = _T("");
  m_entry_count = _T("");
  m_entry_crc32 = _T("");
  m_entry_offset = _T("");
  m_entry_size = _T("");
  m_first_lba = _T("");
  m_gpt_guid = _T("");
  m_hdr_size = _T("");
  m_last_lba = _T("");
  m_primary_lba = _T("");
  m_rsvd = _T("");
  m_gpt_sig = _T("");
  m_gpt_version = _T("");
  m_sig_chars = _T("");
  m_version_num = _T("");
  //}}AFX_DATA_INIT
  
  m_color = COLOR_GPT;
  m_entry_buffer = NULL;
  m_gpt_entries = 0;
}

CGpt::~CGpt() {
}

void CGpt::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CGpt)
  DDX_Control(pDX, IDC_GPT_PAGES, m_gpt_pages);
  DDX_Text(pDX, IDC_GPT_DUMP, m_GptDump);
  DDX_Text(pDX, IDC_GPT_BACKUP_LBA, m_backup_lba);
  DDV_MaxChars(pDX, m_backup_lba, 16);
  DDX_Text(pDX, IDC_GPT_CRC32, m_crc32);
  DDV_MaxChars(pDX, m_crc32, 16);
  DDX_Text(pDX, IDC_GPT_ENTRY_COUNT, m_entry_count);
  DDV_MaxChars(pDX, m_entry_count, 16);
  DDX_Text(pDX, IDC_GPT_ENTRY_CRC32, m_entry_crc32);
  DDV_MaxChars(pDX, m_entry_crc32, 16);
  DDX_Text(pDX, IDC_GPT_ENTRY_OFFSET, m_entry_offset);
  DDV_MaxChars(pDX, m_entry_offset, 16);
  DDX_Text(pDX, IDC_GPT_ENTRY_SIZE, m_entry_size);
  DDV_MaxChars(pDX, m_entry_size, 16);
  DDX_Text(pDX, IDC_GPT_FIRST_LBA, m_first_lba);
  DDV_MaxChars(pDX, m_first_lba, 16);
  DDX_Text(pDX, IDC_GPT_GUID, m_gpt_guid);
  DDV_MaxChars(pDX, m_gpt_guid, 48);
  DDX_Text(pDX, IDC_GPT_HDR_SIZE, m_hdr_size);
  DDV_MaxChars(pDX, m_hdr_size, 16);
  DDX_Text(pDX, IDC_GPT_LAST_LBA, m_last_lba);
  DDV_MaxChars(pDX, m_last_lba, 16);
  DDX_Text(pDX, IDC_GPT_PRIMARY_LBA, m_primary_lba);
  DDV_MaxChars(pDX, m_primary_lba, 16);
  DDX_Text(pDX, IDC_GPT_RSVD, m_rsvd);
  DDV_MaxChars(pDX, m_rsvd, 16);
  DDX_Text(pDX, IDC_GPT_SIG, m_gpt_sig);
  DDV_MaxChars(pDX, m_gpt_sig, 24);
  DDX_Text(pDX, IDC_GPT_VERSION, m_gpt_version);
  DDV_MaxChars(pDX, m_gpt_version, 12);
  DDX_Text(pDX, IDC_SIG_CHARS, m_sig_chars);
  DDV_MaxChars(pDX, m_sig_chars, 16);
  DDX_Text(pDX, IDC_GPT_VERSION_NUM, m_version_num);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGpt, CPropertyPage)
  //{{AFX_MSG_MAP(CGpt)
  ON_BN_CLICKED(ID_GPT_APPLY, OnGptApply)
  ON_BN_CLICKED(IDC_CRC_BUTTON, OnCrcButton)
  ON_BN_CLICKED(IDC_ECRC_BUTTON, OnEcrcButton)
  ON_EN_KILLFOCUS(IDC_GPT_GUID, OnKillfocusGptGuid)
  ON_BN_CLICKED(IDC_GUID_CREATE, OnGuidCreate)
  ON_BN_CLICKED(IDC_GPT_FROM_BACKUP, OnGPTFromBackup)
  ON_BN_CLICKED(IDC_GPT_BACKUP, OnGPTBackup)
  ON_BN_CLICKED(IDC_GPT_TOTAL_CHECK, OnGPTTotalCheck)
  ON_WM_HELPINFO()
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGpt message handlers
BOOL CGpt::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    case IDC_GPT_TOTAL_CHECK:
      strTipText = "Check the GPT Header and Entries";
      break;
    case IDC_GPT_FROM_BACKUP:
      strTipText = "Restore GPT Header and Entries from Backup";
      break;
    case IDC_GPT_BACKUP:
      strTipText = "Save GPT Header and Entries to Backup";
      break;
    case IDC_CRC_BUTTON:
      strTipText = "Recalculate and store the Header CRC";
      break;
    case IDC_GUID_CREATE:
      strTipText = "Create a new GUID";
      break;
    case IDC_ECRC_BUTTON:
      strTipText = "Recalculate and store the Entries CRC";
      break;

    case ID_GPT_APPLY:
      strTipText = "Save modifications";
      break;
   }

   strncpy(pTTTA->szText, strTipText, 79);
   pTTTA->szText[79] = '\0';  // make sure it is null terminated

   return TRUE; // message was handled
}

BOOL CGpt::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_GPT_DUMP)->SetFont(&dlg->m_DumpFont);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, End: %I64i", convert64(m_first_lba), convert64(m_last_lba));
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  EnableToolTips(TRUE);

  return TRUE;
}

BOOL CGpt::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "gpt.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

bool CGpt::Exists(DWORD64 LBA) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD dword;
  
  dlg->ReadFromFile(m_buffer, 1, 1);
  memcpy(&m_hdr, m_buffer, sizeof(struct S_GPT_HDR));
  
  // save the LBA
  m_lba = LBA;
  
  // check the signature
  m_exists = (memcmp(m_hdr.sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8) == 0);

  // check the CRC
  if (m_exists) {
    dword = m_hdr.crc32;
    m_hdr.crc32 = 0;
    if (dword != crc32(&m_hdr, m_hdr.hdr_size)) {
      if (AfxMessageBox("Found incorrect CRC on Header.  Continue?", MB_YESNO, 0) != IDYES) {
        m_exists = FALSE;
        return FALSE;
      }
    }
    m_hdr.crc32 = dword;
  }
  
  // if doesn't exist, but force_gpt_does, we need to set known values to GPT
  if (!m_exists && dlg->IsDlgButtonChecked(IDC_FORCE_GPT)) {
    
    
    m_exists = TRUE;
  }
  
  if (m_exists) {
    // allocate the memory for the entries
    m_entry_buffer = (struct S_GPT_ENTRY *) malloc((m_hdr.entries * m_hdr.entry_size) + (dlg->m_sect_size - 1));
    dlg->ReadFromFile(m_entry_buffer, m_hdr.entry_offset, ((m_hdr.entries * m_hdr.entry_size) + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
    
    m_gpt_sig.Format("%02X %02X %02X %02X %02X %02X %02X %02X", 
      m_hdr.sig[0], m_hdr.sig[1], m_hdr.sig[2], m_hdr.sig[3],
      m_hdr.sig[4], m_hdr.sig[5], m_hdr.sig[6], m_hdr.sig[7]);
    m_sig_chars.Format("%c%c%c%c%c%c%c%c",
      m_hdr.sig[0], m_hdr.sig[1], m_hdr.sig[2], m_hdr.sig[3],
      m_hdr.sig[4], m_hdr.sig[5], m_hdr.sig[6], m_hdr.sig[7]);
    m_gpt_version.Format("0x%08X", m_hdr.version);
    m_version_num.Format("%i.%i", m_hdr.version >> 16, m_hdr.version & 0xFFFF);
    m_hdr_size.Format("%i", m_hdr.hdr_size);
    m_crc32.Format("0x%08X", m_hdr.crc32); // only bytes 0 -> hdr_size are checked
    m_rsvd.Format("0x%08X", m_hdr.resv0);
    m_primary_lba.Format("%I64i", m_hdr.primary_lba);
    m_backup_lba.Format("%I64i", m_hdr.backup_lba);
    m_first_lba.Format("%I64i", m_hdr.first_usable);
    m_last_lba.Format("%I64i", m_hdr.last_usable);
    GUID_Format(m_gpt_guid, &m_hdr.guid);
    m_entry_offset.Format("%I64i", m_hdr.entry_offset);
    m_entry_count.Format("%i", m_hdr.entries);
    m_entry_crc32.Format("0x%08X", m_hdr.crc32_entries);
    m_entry_size.Format("%i", m_hdr.entry_size);
    DumpIt(m_GptDump, m_buffer + (512 - 420), (512 - 420), 420, FALSE);
    
    m_gpt_entries = 0;
    int p = 0;
    for (unsigned i=0; i<m_hdr.entries && i<MAX_GPT_ENTRIES; i++) {
      // ignore empty entries?
      if (dlg->IsDlgButtonChecked(IDC_IGNORE_MT_GPT_ENTRIES)) {
        struct S_GPT_ENTRY *entries = (struct S_GPT_ENTRY *) m_entry_buffer;
        if ((entries[i].first_lba == 0) && (entries[i].last_lba == 0))
          continue;
      }
      m_Pages[p].Construct(IDD_GPT_ENTRY, 0);
      m_Pages[p].m_Title.Format("Entry %i", p);
      if (CheckEntry(i))
        m_Pages[p].m_Title += " !";
      m_Pages[p].m_psp.dwFlags |= PSP_USETITLE;
      m_Pages[p].m_psp.pszTitle = m_Pages[p].m_Title;
      UpdateEntry(&m_Pages[p], p, m_hdr.entry_size);
      m_Pages[p].m_index = p;
      m_Sheet.AddPage(&m_Pages[p]);
      p++;
      m_gpt_entries++;
    }
  }
  
  return m_exists;
}

void CGpt::Begin(void) {
  CRect rect;
  
  // we can't adjust the pages if none were actually made
  // so only do this if we found any valid partitions
  if (m_gpt_entries > 0) {
    m_Sheet.EnableStackedTabs(FALSE);
    m_Sheet.Create(this, WS_VISIBLE | WS_CHILD | WS_DLGFRAME);
    m_Sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
    m_gpt_pages.GetWindowRect(&rect);
    ScreenToClient(&rect);
    m_Sheet.MoveWindow(&rect);
  
    // we need to resize the tabcontrol part
    m_Sheet.GetTabControl()->GetWindowRect(rect);
    m_Sheet.GetTabControl()->SetWindowPos(NULL, 0, 0,
      rect.Width() - 72,
      rect.Height(),
      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  
    // this updates the size of the active page as well
    m_Sheet.SetActivePage(0);
  }
}

void CGpt::UpdateEntry(CGptEntry *Entry, int index, const int entry_size) {
  struct S_GPT_ENTRY *entry = (struct S_GPT_ENTRY *) ((BYTE *) m_entry_buffer + (index * entry_size));
  
  GUID_Format(Entry->m_guid_type, &entry->guid_type);
  GUID_Format(Entry->m_guid, &entry->guid);
  Entry->m_start_lba.Format("%I64i", entry->first_lba);
  Entry->m_last_lba.Format("%I64i", entry->last_lba);
  Entry->m_attrib.Format("0x%016I64X", entry->attribute);
  
  char name[72];
  WideCharToMultiByte(CP_ACP, 0, (LPCWCH) entry->name, -1, &name[0], 72, NULL, NULL);
  Entry->m_name = name;
}

// Returns TRUE if entry is in error
BOOL CGpt::CheckEntry(int index) {
  struct S_GPT_ENTRY *entries = (struct S_GPT_ENTRY *) m_entry_buffer;
  BOOL ret = FALSE;
  CString cs;
  int i;
  
  // don't check if they are zero'd
  if ((entries[index].first_lba == 0) && (entries[index].last_lba == 0))
    return ret;
  
  // check for overlapping of previous entries
  for (i=0; i<index; i++) {
    if (((entries[index].first_lba >= entries[i].first_lba) && (entries[index].first_lba <= entries[i].last_lba)) ||
        ((entries[index].last_lba >= entries[i].first_lba) && (entries[index].last_lba <= entries[i].last_lba))) {
      cs.Format("GPT: Entry #%i overlaps Entry #%i!", index, i);
      AfxMessageBox(cs);
      ret = TRUE;
    }
  }
  
  return ret;
}

void CGpt::Destroy(void) {
  while (m_Sheet.GetPageCount())
    m_Sheet.RemovePage(0);
  m_Sheet.DestroyWindow();
  
  if (m_entry_buffer)
    free(m_entry_buffer);
  m_entry_buffer = NULL;
  
}

void CGpt::GetGPTHdr() {
  UpdateData(TRUE); // get from dialog
  
  m_hdr.version = convert32(m_gpt_version);
  m_hdr.hdr_size = convert32(m_hdr_size);
  m_hdr.crc32 = convert32(m_crc32);
  m_hdr.resv0 = convert32(m_rsvd);
  m_hdr.primary_lba = convert64(m_primary_lba);
  m_hdr.backup_lba = convert64(m_backup_lba);
  m_hdr.first_usable = convert64(m_first_lba);
  m_hdr.last_usable = convert64(m_last_lba);
  GUID_Retrieve(m_gpt_guid, &m_hdr.guid);
  m_hdr.entry_offset = convert64(m_entry_offset);
  m_hdr.entries = convert32(m_entry_count);
  m_hdr.crc32_entries = convert32(m_entry_crc32);
  m_hdr.entry_size = convert32(m_entry_size);
}

void CGpt::GetGPTEntries() {
  // update the entries
  for (int i=0; i<m_gpt_entries; i++) {
    struct S_GPT_ENTRY *entry = (struct S_GPT_ENTRY *) ((BYTE *) m_entry_buffer + (i * m_hdr.entry_size));
    
    if (m_Pages[i].m_dirty)
      m_Pages[i].OnGptApply();
    GUID_Retrieve(m_Pages[i].m_guid_type, &entry->guid_type);
    GUID_Retrieve(m_Pages[i].m_guid, &entry->guid);
    entry->first_lba = convert64(m_Pages[i].m_start_lba);
    entry->last_lba = convert64(m_Pages[i].m_last_lba);
    entry->attribute = convert64(m_Pages[i].m_attrib);
    memset(entry->name, 0, sizeof(entry->name)); // first clear it out
    MultiByteToWideChar(CP_ACP, 0, m_Pages[i].m_name, -1, (LPWSTR) entry->name, 72);
  }
}

// get the dialog items and write the buffer back to the disk
void CGpt::OnGptApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  // update the header items
  GetGPTHdr();
  memcpy(m_buffer, &m_hdr, sizeof(struct S_GPT_HDR));
  dlg->WriteToFile(m_buffer, m_lba, 1);
  
  // update the entries
  GetGPTEntries();
  //dlg->WriteToFile(m_entry_buffer, m_hdr.entry_offset, ((m_gpt_entries * m_hdr.entry_size) + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  dlg->WriteToFile(m_entry_buffer, m_hdr.entry_offset, ((m_hdr.entries * m_hdr.entry_size) + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
}

void CGpt::OnCrcButton() {
  CString cs;
  
  // update the header items
  GetGPTHdr();
  
  m_hdr.crc32 = 0;
  m_hdr.crc32 = crc32(&m_hdr, m_hdr.hdr_size);
  
  cs.Format("0x%08X", m_hdr.crc32);
  SetDlgItemText(IDC_GPT_CRC32, cs);
}

void CGpt::OnEcrcButton() {
  
  // update the entries
  GetGPTEntries();
  
  // update the header entry and the dialog
  //m_hdr.crc32_entries = crc32(m_entry_buffer, m_gpt_entries * m_hdr.entry_size);
  m_hdr.crc32_entries = crc32(m_entry_buffer, m_hdr.entries * m_hdr.entry_size);
  m_entry_crc32.Format("0x%08X", m_hdr.crc32_entries);
  SetDlgItemText(IDC_GPT_ENTRY_CRC32, m_entry_crc32);
  
  if (AfxMessageBox("Update Header CRC?", MB_YESNO, 0) == IDYES)
    OnCrcButton();
}

void CGpt::OnKillfocusGptGuid() {
  CString cs;
  GetDlgItemText(IDC_GPT_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    GetDlgItem(IDC_GPT_GUID)->SetFocus();
  }
}

void CGpt::OnGuidCreate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_GPT_GUID, cs);
}

// save to backup
void CGpt::OnGPTBackup() {
  
  if (AfxMessageBox("Updating the Backup GPT and entries will write\ncurrent values to primary locations too.\nContinue?", MB_YESNO, 0) != IDYES)
    return;

  // Make sure the CRCs are up to date and write the current values to the
  //  LBA 1 of the disk
  GetGPTHdr();

  // update the entries
  GetGPTEntries();
  
  // update the header entry and the dialog
  m_hdr.crc32_entries = crc32(m_entry_buffer, m_hdr.entries * m_hdr.entry_size);
  m_entry_crc32.Format("0x%08X", m_hdr.crc32_entries);
  SetDlgItemText(IDC_GPT_ENTRY_CRC32, m_entry_crc32);
  
  OnCrcButton();
  OnGptApply();

  // Now write these same values to the backup area
  // OnGptApply() above make sure that all members of m_hdr are updated correctly
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int gpt_entries_size;
  struct S_GPT_HDR *hdr = (struct S_GPT_HDR *) m_buffer;
  
  // update the header items
  memset(m_buffer, 0, dlg->m_sect_size);
  memcpy(m_buffer, &m_hdr, sizeof(struct S_GPT_HDR));

  // we must swap the MyLBA and AlternateLBA values for the Backup Header
  gpt_entries_size = ((hdr->entries * sizeof(struct S_GPT_ENTRY)) + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  hdr->backup_lba = m_hdr.primary_lba;
  hdr->primary_lba = m_hdr.backup_lba;
  hdr->entry_offset = m_hdr.backup_lba - gpt_entries_size;
  hdr->crc32 = 0;
  hdr->crc32 = crc32(hdr, hdr->hdr_size); // calculate new CRC (since we swapped values)
  dlg->WriteToFile(m_buffer, m_hdr.backup_lba, 1);
  
  // update the entries
  dlg->WriteToFile(m_entry_buffer, hdr->entry_offset, gpt_entries_size);
}

// restore from Backup
void CGpt::OnGPTFromBackup() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int gpt_entries_size;  // count of sectors used by entries
  DWORD64 temp;
  struct S_GPT_HDR *hdr = (struct S_GPT_HDR *) m_buffer;

  if (AfxMessageBox("Restore Primary GPT Header and Entries from Backup LBA?\n(Make sure 'BackupLBA' is valid)", MB_YESNO, 0) != IDYES)
    return;

  // get backuplba from the current dialog
  UpdateData(TRUE); // get from dialog
  m_hdr.backup_lba = convert64(m_backup_lba);

  // Check to see if the new data is valid (if not ask to continue)
  if (!CheckGPT(m_hdr.backup_lba, FALSE))
    if (AfxMessageBox("Backup GPT isn't valid. Continue?", MB_YESNO, 0) != IDYES)
      return;

  // read in the backup header
  dlg->ReadFromFile(hdr, m_hdr.backup_lba, 1);

  // need to swap the myLBA and alternateLBA values
  temp = hdr->backup_lba;
  hdr->backup_lba = hdr->primary_lba;
  hdr->primary_lba = temp;
  hdr->entry_offset = 2;
  hdr->crc32 = 0;
  hdr->crc32 = crc32(hdr, hdr->hdr_size); // calculate new CRC (since we swapped values)
  dlg->WriteToFile(hdr, 1, 1);

  // get the count of sectors used for the entries (from the newly read data)
  gpt_entries_size = ((hdr->entries * sizeof(struct S_GPT_ENTRY)) + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  temp = hdr->backup_lba;
  for (int i=0; i<gpt_entries_size; i++) {
    dlg->ReadFromFile(m_buffer, temp - gpt_entries_size + i, 1);
    dlg->WriteToFile(m_buffer, (DWORD64) (2 + i), 1);
  }

  // reload
  dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

// checks a GPT to see if it is valid
//  LBA = LBA of GPT to check.
//  CheckAlternate = TRUE if we check the other GPT as well
//  If LBA == 1, we know that it is the primary and the partition entries are after it
//  if LBA > 1, we know that it is the backup and the partition entries are directly before it.
BOOL CGpt::CheckGPT(DWORD64 LBA, BOOL CheckAlternate) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_GPT_HDR *hdr = (struct S_GPT_HDR *) m_buffer;

  dlg->ReadFromFile(m_buffer, LBA, 1);

  // ACPI, version 2.6, section 5.3.2, list of qualifications
  // check the signature
  if (memcmp(hdr->sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8) != 0)
    return FALSE;

  // check the Header CRC
  DWORD crc = hdr->crc32;
  hdr->crc32 = 0;
  if (crc != crc32(hdr, hdr->hdr_size))
    return FALSE;
  // restore the crc value
  hdr->crc32 = crc;

  // check that MyLBA points to a valid Partition Table
  // by checking the CRC of this table
  

  // if CheckAlternate, load and check the other GPT Header and partition table
  if (CheckAlternate) {
    



  }

  return TRUE;
}

// completely check the image for a valid 100% EFI GPT image
void CGpt::OnGPTTotalCheck() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_GPT_HDR *hdr = (struct S_GPT_HDR *) m_buffer;
  int i, j, error_cnt = 0;
  const DWORD64 last_lba = (DWORD64) dlg->GetFileSectCount() - 1 - ((dlg->m_hasVHD) ?  1 : 0);
  DWORD dword, dword1, cur_crc, org_crc, entry_size;
  BYTE *byte;

  CString cs, csInfo;
  CModeless modeless;
  modeless.m_Title = "Checking GPT Image";
  modeless.m_modeless = TRUE;
  modeless.Create(CModeless::IDD, this);
  modeless.ShowWindow(SW_SHOW);
  modeless.BringWindowToTop();
  
  csInfo = "Checking for 100% Valid EFI GPT image\r\n";
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  
  // first check that we have a valid GPT at LBA 1
  dlg->ReadFromFile(m_buffer, 1, 1);

  csInfo += "Checking for valid GPT at LBA 1: \r\n";
  if (memcmp(hdr->sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8) == 0)
    csInfo += "Checking Signature: Pass\r\n";
  else {
    csInfo += "*Checking Signature: Fail\r\n";
    error_cnt++;
  }
  //modeless.SetDlgItemText(IDC_EDIT, csInfo);
  //modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // check the Header CRC
  org_crc = hdr->crc32;
  hdr->crc32 = 0;
  cur_crc = crc32(hdr, hdr->hdr_size);
  hdr->crc32 = org_crc; // restore the crc value
  if (cur_crc == org_crc)
    cs.Format("Checking Header CRC: Pass (0x%08X)\r\n", cur_crc);
  else
    cs.Format("Checking Header CRC: Fail (0x%08X != 0x%08X)\r\n", org_crc, cur_crc);
  csInfo += cs;
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // if we make it this far with an error, there really isn't any need to go any further
  if (error_cnt > 0) {
    csInfo += "*Signature and/or CRC failed.  Stopping\r\n";
    goto TotalCheckDone;
  }

  // check that the Revision is 1.0
  if (hdr->version != 0x00010000) {
    cs.Format("*Error in Revision value: 0x%08X (%i.%i)\r\n", hdr->version, hdr->version >> 16, hdr->version & 0xFFFF);
    error_cnt++;
  } else
    cs.Format("Found Revision %i.%i\r\n", hdr->version >> 16, hdr->version & 0xFFFF);
  csInfo += cs;

  // check that the header size is (at lesat) 92
  if ((hdr->hdr_size < 92) || (hdr->hdr_size > dlg->m_sect_size)) {
    cs.Format("*Error in Header Size value: %i\r\n", hdr->hdr_size);
    error_cnt++;
  } else if (hdr->hdr_size != 92)
    cs.Format("Header Size = %i.  Valid, but should be 92.\r\n", hdr->hdr_size);
  else
    cs.Format("Header Size = %i\r\n", hdr->hdr_size);
  csInfo += cs;

  // the reserved field should be zero
  if (hdr->resv0) {
    cs.Format("*Reserved DWORD should be zero: (0x%08X)\r\n", hdr->resv0);
    error_cnt++;
  }
  
  // the MyLBA (Parimary) should = 1
  if (hdr->primary_lba != 1) {
    cs.Format("*Primary LBA doesn't = 1: (%I64i)\r\n", hdr->primary_lba);
    error_cnt++;
  }
  
  // Now let's check a few other things.
  // ACPI: Ver 2.6: Page 121:
  //  A minimum of 16,384 bytes of space must be reserved for the GPT Partition Entry Array.
  //  If the block size is 512, the First Usable LBA must be greater than or equal to 34 (allowing 1
  //   block for the Protective MBR, 1 block for the Partition Table Header, and 32 blocks for the GPT
  //   Partition Entry Array); if the logical block size is 4096, the First Useable LBA must be greater
  //   than or equal to 6 (allowing 1 block for the Protective MBR, 1 block for the GPT Header, and 4
  //   blocks for the GPT Partition Entry Array).
  if ((dlg->m_sect_size == 512) && (hdr->first_usable < 34)) {
    cs.Format("512-byte sectors:\r\n"
              "*The First Usable LBA must be at least 34\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current First Usable is %I64i\r\n", hdr->first_usable);
    error_cnt++;
  } else if ((dlg->m_sect_size == 4096) && (hdr->first_usable < 6)) {
    cs.Format("4096-byte sectors:\r\n"
              "*The First Usable LBA must be at least 6\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current First Usable is %I64i\r\n", hdr->first_usable);
    error_cnt++;
  } else
    cs.Format("First Usable value is within range: %I64i\r\n", hdr->first_usable);
  csInfo += cs;
  //modeless.SetDlgItemText(IDC_EDIT, csInfo);
  //modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // there must be enough room after the Last Usable LBA to store the backup
  //  entries and backup GPT
  if ((dlg->m_sect_size == 512) && (hdr->last_usable > (last_lba - 33))) {
    cs.Format("512-byte sectors:\r\n"
              "*The Last Usable LBA must be no more than %I64i\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current Last Usable is %I64i\r\n", last_lba - 33, hdr->last_usable);
    error_cnt++;
  } else if ((dlg->m_sect_size == 4096) && (hdr->last_usable > (last_lba - 5))) {
    cs.Format("4096-byte sectors:\r\n"
              "*The Last Usable LBA must be no more than %I64i\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current Last Usable is %I64i\r\n", last_lba - 5, hdr->last_usable);
    error_cnt++;
  } else
    cs.Format("Last Usable value is within range: %I64i\r\n", hdr->last_usable);
  csInfo += cs;
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  
  // if Backup GPT is not at end of disk, give error
  if (hdr->backup_lba != last_lba) {
    cs.Format("*Backup GPT is not at last LBA of disk.\r\n"
              " Should be at %I64i is at %I64i\r\n", last_lba, hdr->backup_lba);
    error_cnt++;
  } else
    cs.Format("Backup GPT LBA is at end of disk: %I64i\r\n", hdr->backup_lba);
  csInfo += cs;

  // check that the DiskGUID is not zeros
  if (IsBufferEmpty(&hdr->guid, 16)) {
    csInfo += "*Disk GUID should not be zeros...\r\n";
    error_cnt++;
  }

  // The Partition entry array must be (512-byte sectors:)32 or (4096-byte sectors:)4 less than First Usable
  if ((dlg->m_sect_size == 512) && (hdr->entry_offset > (hdr->first_usable - 32))) {
    cs.Format("512-byte sectors:\r\n"
              "*The Entry Offset LBA must be no more than %I64i\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current Entry Offset is %I64i\r\n", hdr->first_usable - 32, hdr->entry_offset);
    error_cnt++;
  } else if ((dlg->m_sect_size == 4096) && (hdr->entry_offset > (hdr->first_usable - 4))) {
    cs.Format("4096-byte sectors:\r\n"
              "*The Entry Offset LBA must be no more than %I64i\r\n"
              " allowing for 16384 bytes or 128 entries\r\n"
              " Current Entry Offset is %I64i\r\n", hdr->first_usable - 4, hdr->entry_offset);
    error_cnt++;
  } else if (hdr->entry_offset < 2) {
    cs.Format("*The Entry Offset LBA must be at least 2.\r\n"
              " Current Entry Offset is %I64i\r\n", hdr->entry_offset);
    error_cnt++;
  } else
    cs.Format("Entry Offset value is within range: %I64i\r\n", hdr->entry_offset);
  csInfo += cs;
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  
  // the number of partition entries must fit within the allotted space
  if (dlg->m_sect_size == 512)
    dword = (DWORD) ((hdr->first_usable - hdr->entry_offset) * 512);
  else
    dword = (DWORD) ((hdr->first_usable - hdr->entry_offset) * 4096);
  if ((hdr->entries * hdr->entry_size) > dword) {
    cs.Format("*Not enough room is allocated for count of entries (%i).\r\n"
              " Space needed = %i bytes, space allocated = %i bytes\r\n", hdr->entries * hdr->entry_size, dword);
    error_cnt++;
  } else
    cs.Format("Found %i bytes for partition entries.\r\n"
              " (%i / %i = room for up to %i entries).\r\n", dword, dword, hdr->entry_size, (dword > 0) ? (dword / hdr->entry_size) : 0);
  csInfo += cs;

  // the size of the partition entry must be a multiple of 128.
  //  * in case entry size is way out of portion, we use a sane value from here on out *
  if (hdr->entry_size & 127) {
    cs.Format("*Entry size of %i is not a multiple of 128.\r\n", hdr->entry_size);
    entry_size = 128;
    error_cnt++;
  } else {
    entry_size = hdr->entry_size;
    cs.Format("Found a valid entry size of %i.\r\n", entry_size);
  }
  csInfo += cs;

  // CRC of entries
  cur_crc = crc32(m_entry_buffer, m_hdr.entries * entry_size);
  if (cur_crc != m_hdr.crc32_entries) {
    cs.Format("*The Entry CRC is not correct.  Should be 0x%08X.\r\n", cur_crc);
    error_cnt++;
  } else
    cs.Format("Entry CRC is correct.\r\n");
  csInfo += cs;
  
  // TODO:
  // Check each entry:
  //  - check for overlapping entries
  //  - check that the name is null terminated
  //

  // The remaining of the sector must be zero
  byte = (BYTE *) &hdr->reserved;
  j = 0;
  for (i=0; i<(int) (dlg->m_sect_size - hdr->hdr_size); i++)
    j |= byte[i];
  if (j != 0) {
    csInfo += "*Remaining part of header should all be zeros...\r\n";
    error_cnt++;
  }
  
  // to check the backup GPT, simply check that it equals this one
  //  with exception of the MyLBA, and AlternateLBA, CRC, and Entry Start Loc
  struct S_GPT_HDR *back = (struct S_GPT_HDR *) malloc(dlg->m_sect_size);
  dlg->ReadFromFile(back, hdr->backup_lba, 1);
  // save and check the crc of the backup GPT
  org_crc = back->crc32;
  back->crc32 = 0;
  cur_crc = crc32(back, back->hdr_size);  // crc before we swap the LBA's
  // set the 4 backup items to the primary items so they aren't flagged as different
  // (only the MyLBA, AlternateLBA, CRC, and Partition Offset fields should be different)
  back->primary_lba = hdr->primary_lba;
  back->backup_lba = hdr->backup_lba;
  back->crc32 = hdr->crc32;
  back->entry_offset = hdr->entry_offset;
  if (memcmp(back, hdr, dlg->m_sect_size) != 0) {
    csInfo += "*Backup GPT doesn't match Primary GPT.\r\n";
    BYTE *s0 = (BYTE *) hdr;
    BYTE *s1 = (BYTE *) back;
    for (dword=0; dword<dlg->m_sect_size; dword++) {
      if (s0[dword] != s1[dword]) {
        cs.Format(" 0x%04X:  0x%02X 0x%02X\r\n", dword, s0[dword], s1[dword]);
        csInfo += cs;
      }
    }
    error_cnt++;
  } else if (org_crc != cur_crc) {
    csInfo += "*Backup GPT matches Primary GPT except for a bad CRC.\r\n";
    error_cnt++;
  } else
    csInfo += "Backup GPT matches Primary GPT\r\n";
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  
  // restore the backup hdr
  dlg->ReadFromFile(back, hdr->backup_lba, 1);

  // to check the backup entries, simply read in each sector and compare
  //  with the sector of the primary entries.
  dword1 = (hdr->entries < back->entries) ? hdr->entries : back->entries;
  cs.Format("Comparing %i Primary and Backup Partition Entries.\r\n", dword1);
  csInfo += cs;
  modeless.SetDlgItemText(IDC_EDIT, csInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  
  void *back_entries = malloc((dword1 * entry_size) + dlg->m_sect_size);
  dlg->ReadFromFile(back_entries, back->entry_offset, ((dword1 * entry_size) + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  BYTE *b = (BYTE *) back_entries;
  BYTE *p = (BYTE *) m_entry_buffer;
  j = 0;
  for (dword=0; dword<dword1; dword++) {
    if (memcmp(b, p, entry_size) != 0) {
      cs.Format("*Backup Entry #%i doesn't match Primary Entry.\r\n", dword);
      csInfo += cs;
      /*
      BYTE *s0 = (BYTE *) hdr;
      BYTE *s1 = (BYTE *) back;
      for (i=0; i<dlg->m_sect_size; i++) {
        if (s0[i] != s1[i]) {
          cs.Format(" 0x%04X:  0x%02X 0x%02X\r\n", i, s0[i], s1[i]);
          csInfo += cs;
        }
      }*/
      j++;
      error_cnt++;
    }
    b += entry_size;
    p += entry_size;
  }
  free(back_entries);
  free(back);
  if (j == 0)
    csInfo += "Backup Entries match Primary Entries\r\n";

  // check the PMBR to see if it only has one (the first) entry as 0xEE
  //  and that it encompasses all sectors correctly, etc.
  BYTE *pmbr = (BYTE *) malloc(dlg->m_sect_size);
  dlg->ReadFromFile(pmbr, 0, 1);
  struct MBR_PART_ENTRY *entry = (struct MBR_PART_ENTRY *) (pmbr + 0x1BE);
  // find the 0xEE entry
  j = 0; // count of 'other' used entries
  for (i=0; i<4; i++) {
    if (entry[i].sys_id == 0xEE) {
      cs.Format("Found EFI/GPT PMBR partition entry at index %i\r\n", i);
      csInfo += cs;
      if (i != 0)
        csInfo += "Warning: 0xEE partition entry should be index 0.\r\n";
      if (entry[i].boot_id != 0) {
        cs.Format("*Boot id != 0 (%02X)\r\n", entry[i].boot_id);
        csInfo += cs;
        error_cnt++;
      }
      if ((entry[i].start.cyl != 0) || (entry[i].start.head != 0) || (entry[i].start.sector != 2)) {
        cs.Format("*Starting CHS should be 00/00/02, is %02X/%02X/%02X\r\n", entry[i].start.cyl, entry[i].start.head, entry[i].start.sector);
        csInfo += cs;
        error_cnt++;
      }
  ///  why does Windows 10 use the values it does?????
      //if ((entry[i].end.cyl != 0xE9) || (entry[i].end.head != 0xFE) || (entry[i].end.sector != 0x7F)) {
      //  cs.Format("*Ending CHS should be 0xE9/0xFE/0x7F, is %02X/%02X/%02X\r\n", entry[i].end.cyl, entry[i].end.head, entry[i].end.sector);
      //  csInfo += cs;
      //  error_cnt++;
      //}
      if (entry[i].start_lba != 1) {
        cs.Format("*Starting LBA should be 1, is %i\r\n", entry[i].start_lba);
        csInfo += cs;
        error_cnt++;
      }
      //if (entry[i].sectors != 0xFFFFFFFF) {
      //  cs.Format("*Sector Count should be 0xFFFFFFFF, is 0x%08X\r\n", entry[i].sectors);
      //  csInfo += cs;
      //  error_cnt++;
      //}
    } else if (entry[i].sys_id != 0)
      j++;
  }
  // did we find other entries?
  if (j > 0) {
    cs.Format("Warning: Found %i 'other' used entries.\r\n", j);
    csInfo += cs;
    csInfo += "  Windows 10 will not recognize as GPT if a\r\n  hybrid GPT/MBR is used.\r\n";
  }
  free(pmbr);

  // done.  Display results
TotalCheckDone:
  if (error_cnt == 0)
    csInfo += "\r\nCompletely passed all checks...\r\n";
  else {
    cs.Format("\r\n **** Found %i Errors\r\n", error_cnt);
    csInfo += cs;
  }
  //modeless.SetDlgItemText(IDC_EDIT, csInfo);
  //modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // now "copy" the modeless to a modal and
  //  destroy the modeless and display the modal
  CModeless modal;
  modal.m_edit = csInfo;
  modal.m_Title = modeless.m_Title;
  modeless.DestroyWindow();
  modal.m_modeless = FALSE;
  modal.DoModal();
}
