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


// Mbr.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Mbr.h"
#include "MbrEntry.h"
#include "UltimageResize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMbr property page

IMPLEMENT_DYNCREATE(CMbr, CPropertyPage)

CMbr::CMbr() : CPropertyPage(CMbr::IDD) {
  //{{AFX_DATA_INIT(CMbr)
  m_MbrDump = _T("");
  m_spt = _T("63");
  m_head_count = _T("16");
  m_id_sig = _T("");
  m_id_zero = _T("");
  //}}AFX_DATA_INIT
  m_color = COLOR_MBR;
}

CMbr::~CMbr() {
}

void CMbr::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMbr)
  DDX_Control(pDX, IDC_MBR_PAGES, m_mbr_pages);
  DDX_Text(pDX, IDC_MBR_DUMP, m_MbrDump);
  DDX_Text(pDX, IDC_SPT, m_spt);
  DDV_MaxChars(pDX, m_spt, 10);
  DDX_Text(pDX, IDC_HEAD_COUNT, m_head_count);
  DDV_MaxChars(pDX, m_head_count, 10);
  DDX_Text(pDX, IDC_ID_SIG, m_id_sig);
  DDX_Text(pDX, IDC_ID_ZERO, m_id_zero);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMbr, CPropertyPage)
  //{{AFX_MSG_MAP(CMbr)
  ON_BN_CLICKED(ID_MBR_APPLY, OnMbrApply)
  ON_BN_CLICKED(IDEXTRACT, OnExtractMbr)
  ON_BN_CLICKED(IDUPDATE_MBR, OnUpdateMbr)
  ON_BN_CLICKED(IDUPDATE_MBR_SIG, OnUpdateMbrSig)
  ON_BN_CLICKED(IDC_ID_SIG_UPDATE, OnIdSigUpdate)
  ON_BN_CLICKED(IDC_ID_ZERO_UPDATE, OnIdZeroUpdate)
  ON_BN_CLICKED(IDPREPEND_MBR, OnPrependMBR)
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMbr message handlers
BOOL CMbr::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_MBR_DUMP)->SetFont(&dlg->m_DumpFont);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, (DWORD64) 1);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  return TRUE;
}

BOOL CMbr::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "mbr.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

bool CMbr::Exists(DWORD64 LBA) {
  struct MBR_PART_ENTRY *entry;
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int i;
  
  dlg->ReadFromFile(m_buffer, LBA, 1);
  
  // save the LBA
  m_lba = LBA;
  
  // check to see if 0xAA55 is present
  m_exists = (* (WORD *) &m_buffer[512-2] == 0xAA55);
  
  // now check to see if there are four entries
  entry = (struct MBR_PART_ENTRY *) &m_buffer[512-2-(4*16)];
  for (i=0; i<4; i++) {
    if ((entry[i].boot_id != 0x80) && (entry[i].boot_id != 0x00)) {
      m_exists = FALSE;
      break;
    }
    // TODO: Some more checking to detect if a partition table exists
  }
  
  // if all partition entries are zeros, then there is no MBR
  if (IsBufferEmpty(&m_buffer[512-2-(4*16)], 64))
    m_exists = FALSE;
  
  // 
  if (dlg->IsDlgButtonChecked(IDC_FORCE_NO_MBR))
    m_exists = FALSE;
  if (dlg->IsDlgButtonChecked(IDC_FORCE_MBR))
    m_exists = TRUE;
  
  if (m_exists) {
    for (i=0; i<4; i++) {
      m_Pages[i].Construct(IDD_MBR_ENTRY, 0);
      m_Pages[i].m_Title.Format("Entry %i", i);
      m_Pages[i].m_psp.dwFlags |= PSP_USETITLE;
      m_Pages[i].m_psp.pszTitle = m_Pages[i].m_Title;
      m_Pages[i].m_parent = this;
      UpdateEntry(&m_Pages[i], i);
      m_Pages[i].m_index = i;
      m_Sheet.AddPage(&m_Pages[i]);
    }
    
    DumpIt(m_MbrDump, m_buffer, 0, 512, FALSE);
  } else 
    m_MbrDump = "If the 0xAA55 signature is not found, or there wasn't any\r\n"
                " valid partition entries, there won't be any entries listed.\r\n"
                "Use the 'Update MBR Sig' button to add this signature.\r\n"
                "Then reload the image to try again.\r\n"
                "Don't forget to hit the 'Apply' button as well.\r\n"
                "\r\n"
                "If you know there is no MBR sector and wish to add one, use\r\n"
                " the 'Prepend MBR' button.  Be sure reload, then give a sys_id\r\n"
                " value so that the partition will then be recognized\r\n"
                "\r\n"
                "If this is a floppy image, then simply ignore this tab.";
  
  m_id_sig.Format("0x%08X", * (DWORD *) &m_buffer[512 - 2 - (4 * 16) - 2 - 4]);
  m_id_zero.Format("0x%04X", * (WORD *) &m_buffer[512 - 2 - (4 * 16) - 2]);
  
  // if we didn't find it, *but* the sector is zero'd, let's return true anyway, so we can create one
  if (IsBufferEmpty(m_buffer, 512))
    return TRUE;

  return m_exists;
}

void CMbr::Begin(void) {
  CRect rect;
  
  m_Sheet.EnableStackedTabs(FALSE);
  m_Sheet.Create(this, WS_VISIBLE | WS_CHILD | WS_DLGFRAME);
  m_Sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
  m_mbr_pages.GetWindowRect(&rect);
  ScreenToClient(&rect);
  m_Sheet.MoveWindow(&rect);
  
  // we need to resize the tabcontrol part
  m_Sheet.GetTabControl()->GetWindowRect(rect);
  m_Sheet.GetTabControl()->SetWindowPos(NULL, 0, 0,
    rect.Width() - 123,
    rect.Height(),
    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  
  // this updates the size of the active page as well
  m_Sheet.SetActivePage(0);
  
  GetDlgItem(IDC_RELATIVE)->ShowWindow((m_index > 0) ? SW_SHOW : SW_HIDE);
}

void CMbr::UpdateEntry(CMbrEntry *Entry, int index) {
  struct MBR_PART_ENTRY *entry;
  
  entry = (struct MBR_PART_ENTRY *) (&m_buffer[512-2-(4*16)] + (16 * index));
  Entry->m_boot_id.Format("0x%02X", entry->boot_id);
  Entry->m_begin_cyl.Format("0x%02X", entry->start.cyl);
  Entry->m_begin_head.Format("0x%02X", entry->start.head);
  Entry->m_begin_sect.Format("0x%02X", entry->start.sector);
  Entry->m_sys_id.Format("0x%02X", entry->sys_id);
  Entry->m_end_cyl.Format("0x%02X", entry->end.cyl);
  Entry->m_end_head.Format("0x%02X", entry->end.head);
  Entry->m_end_sect.Format("0x%02X", entry->end.sector);
  Entry->m_start_lba.Format("%u", entry->start_lba);
  if (entry->sectors < 0xFFFFFFFF)
    Entry->m_size.Format("%u", entry->sectors);
  else // if max sectors, show 0xFFFFFFFF.  It is easier to see than 4,294,967,295
    Entry->m_size.Format("0x%08X", entry->sectors);
}

void CMbr::Destroy(void) {
  while (m_Sheet.GetPageCount())
    m_Sheet.RemovePage(0);
  m_Sheet.DestroyWindow();
  
  m_exists = FALSE;
}

// simply write the buffer back to the disk
void CMbr::OnMbrApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  UpdateData(TRUE);  // get from dialog
  
  // update the buffer with the Disk ID fields
  * (DWORD *) &m_buffer[512 - 2 - (4 * 16) - 2 - 4] = convert32(m_id_sig);
  * (WORD *) &m_buffer[512 - 2 - (4 * 16) - 2] = convert16(m_id_zero);
  
  dlg->WriteToFile(m_buffer, m_lba, 1);
  
  // update the dump
  DumpIt(m_MbrDump, m_buffer, 0, 512, FALSE);
  UpdateData(FALSE);
}

void CMbr::OnExtractMbr() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile file;
  void *buffer;

  CFileDialog odlg (
    FALSE,            // Create a save_as file dialog
    _T(".bin"),       // Default file extension
    NULL,             // Default Filename
    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  odlg.m_ofn.lpstrTitle = "Extract MBR to file";
  if (odlg.DoModal() != IDOK)
    return;

  POSITION pos = odlg.GetStartPosition();
  if (file.Open(odlg.GetNextPathName(pos), CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
    buffer = calloc(1, MAX_SECT_SIZE);
    dlg->ReadFromFile(buffer, m_lba, 1);
    file.Write(buffer, dlg->m_sect_size);
    file.Close();
    free(buffer);
    AfxMessageBox("MBR Code Extracted Successfully...");
  } else
    AfxMessageBox("Error Creating File...");
}

// read in a file and update the current MBR buffer, overwriting partition table if needed
void CMbr::OnUpdateMbr() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile file;
  CString csPath;
  BYTE *buffer;
  size_t length, sectors, offset;
  
  CFileDialog odlg (
    TRUE,             // Create an open file dialog
    _T(".bin"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T(".img files (.img)|*.img|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  
  // use a default path?
  csPath = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
  if (!csPath.IsEmpty())
    odlg.m_ofn.lpstrInitialDir = csPath;
  odlg.m_ofn.lpstrTitle = "Update MBR Code Image File";
  
  if (odlg.DoModal() != IDOK)
    return;
  
  POSITION pos = odlg.GetStartPosition();
  if (file.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening Image File...");
    return;
  }
  
  // calculate the size and allocate the buffer
  length = (size_t) file.GetLength();
  sectors = (length + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  buffer = (BYTE *) calloc(length + dlg->m_sect_size, 1);  // extra padding to allow for a whole sector at the end
  
  // read the sector
  file.Read(buffer, (UINT) length);
  file.Close();
  
  // update the code section (first bytes up to the Disk Id fields)
  memcpy(m_buffer, buffer, 512 - 2 - (4 * 16) - 2 - 4);
  
  // if the overwrite disk id checkbox is checked,
  //  we need to update the fields in the dialog
  if (IsDlgButtonChecked(IDC_MBR_OVERWRITE_ID)) {
    memcpy(m_buffer + (512 - 2 - (4 * 16) - 2 - 4), buffer + (512 - 2 - (4 * 16) - 2 - 4), 4 + 2);
    m_id_sig.Format("0x%08X", * (DWORD *) &m_buffer[512 - 2 - (4 * 16) - 2 - 4]);
    m_id_zero.Format("0x%04X", * (WORD *) &m_buffer[512 - 2 - (4 * 16) - 2]);
  }
  
  // if the overwrite checkbox is checked, over write the partition entries
  if (IsDlgButtonChecked(IDC_MBR_OVERWRITE)) {
    memcpy(m_buffer + (512 - 2 - (4 * 16)), buffer + (512 - 2 - (4 * 16)), 4 * 16);
    for (int i=0; i<3; i++)
      UpdateEntry(&m_Pages[i], i);
  }
  
  // now write it to the image file
  dlg->WriteToFile(m_buffer, m_lba, 1);
  // the length of the file was more than one sector, let's write those too
  if (sectors > 1) {
    // if the is_legacy_gpt code box is checked, we need to read in the sector,
    //   update our buffer with the GPT header, then write it back
    if (dlg->Gpt.m_exists && IsDlgButtonChecked(IDC_MBR_LEGACY_GPT)) {
      dlg->ReadFromFile(buffer, m_lba + 1, 1);  // we can use the first sector of the buffer since we already wrote it
      memcpy(buffer + dlg->m_sect_size, buffer, 92);  // 92 is a standard GPT Header length (shouldn't be anything else)
    }
    // warn if the code will pass up the entry offset in the GPT
    offset = atoi(dlg->Gpt.m_entry_offset);
    if (dlg->Gpt.m_exists && (sectors > offset)) {
      if (AfxMessageBox("New MBR code will overwrite GPT Entries!  Continue?", MB_YESNO, 0) == IDYES) {
        for (unsigned i=1; i<sectors; i++)
          dlg->WriteToFile(buffer + (i * dlg->m_sect_size), m_lba + i, 1);
      }
    }
  }
  free(buffer);
  
  DumpIt(m_MbrDump, m_buffer, 0, 512, FALSE);
  UpdateData(FALSE);
}

// make sure the Sig is correct
void CMbr::OnUpdateMbrSig() {
  * (WORD *) &m_buffer[512-2] = 0xAA55;
  DumpIt(m_MbrDump, m_buffer, 0, 512, FALSE);
  UpdateData(FALSE);
}

// place a new sig in the ID Sig field
void CMbr::OnIdSigUpdate() {
  CString cs;
  cs.Format("0x%08X", (rand() << 16) | rand());
  SetDlgItemText(IDC_ID_SIG, cs);
}

// place a zero in the ID Zero field
void CMbr::OnIdZeroUpdate() {
  SetDlgItemText(IDC_ID_ZERO, "0x0000");
}

void CMbr::OnPrependMBR() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct MBR_PART_ENTRY *entry;
  DWORD64 lba, Size = dlg->GetFileSectCount();
  long count = 1;
  
  // first ask to be sure
  if (AfxMessageBox("This will move all sectors forward and write a MBR at LBA 0!  Continue?", MB_YESNO, 0) != IDYES)
    return;

  // Ask how many sectors to prepend (must be at least 1)
  CUltimageResize resize;
  resize.m_title = _T("Count of sectors to prepend");
  resize.m_cur_size = _T("0");
  resize.m_new_size = _T("1");
  if (resize.DoModal() != IDOK)
    return;
  count = (long) convert64(resize.m_new_size);
  if (count < 1) {
    AfxMessageBox("Count must be at least 1");
    return;
  }
  
  // simply 'insert' sector(s) at the front
  dlg->InsertSectors(0, count);

  // fill m_buffer with an empty/valid MBR
  memset(m_buffer, 0, MAX_SECT_SIZE);
  entry = (struct MBR_PART_ENTRY *) &m_buffer[446];
  
  entry[0].start_lba = count;
  entry[0].sectors = (Size <= 0xFFFFFFFF) ? (DWORD) Size : 0xFFFFFFFF;

  m_buffer[510] = 0x55;
  m_buffer[511] = 0xAA;

  // and write it to the file
  dlg->WriteToFile(m_buffer, 0, 1);

  // clear the inserted sectors
  if (--count > 0) {
    memset(m_buffer, 0, MAX_SECT_SIZE);
    lba = 1;
    while (count--)
      dlg->WriteToFile(m_buffer, lba++, 1);
  }

  // reload the image
  dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

// This verifies that the MBR is actually a valid Protective MBR.
//
//  TODO:  Do more validation....
//
bool CMbr::IsPMBR(void) {
  struct MBR_PART_ENTRY *entry;

  entry = (struct MBR_PART_ENTRY *) &m_buffer[446];
  return ((entry[0].sys_id == 0xEE) &&
          (entry[1].sys_id == 0x00) &&
          (entry[2].sys_id == 0x00) &&
          (entry[3].sys_id == 0x00));

}