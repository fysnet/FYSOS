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

// NewImage.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "NewPart.h"
#include "NewImage.h"

#include "Mbr.h"
#include "Embr.h"
#include "VDI.h"
#include "VHD.h"

#include "ISOPrimary.h"
#include "ISOBoot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// this is just enough code to place in the MBR to
//  print a string and halt
BYTE mbr_dummy_code[] = {
  0xFA, 0xB8, 0xC0, 0x07, 0x8E, 0xD8, 0x8E, 0xD0, 0xBC, 0x00, 0x40, 0xFB, 0xBE, 0x24, 0x00, 0xE8, 
  0x03, 0x00, 0xF4, 0xEB, 0xFD, 0xB4, 0x0E, 0x31, 0xDB, 0xFC, 0xAC, 0x08, 0xC0, 0x74, 0x04, 0xCD, 
  0x10, 0xEB, 0xF6, 0xC3, 0x0D, 0x0A, 0x07, 0x49, 0x20, 0x61, 0x6D, 0x20, 0x61, 0x6E, 0x20, 0x65, 
  0x6D, 0x70, 0x74, 0x79, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x63, 0x74, 0x6F, 0x72, 
  0x2E, 0x20, 0x20, 0x49, 0x20, 0x77, 0x69, 0x6C, 0x6C, 0x20, 0x6A, 0x75, 0x73, 0x74, 0x20, 0x68, 
  0x61, 0x6C, 0x74, 0x20, 0x68, 0x65, 0x72, 0x65, 0x2E, 0x00
};

/////////////////////////////////////////////////////////////////////////////
// CNewImage dialog
CNewImage::CNewImage(CWnd* pParent /*=NULL*/)
  : CDialog(CNewImage::IDD, pParent) {
  //{{AFX_DATA_INIT(CNewImage)
  m_cur_parts = 1;
  m_sect_size = 0;  // default to 512
  m_type = 0;
  m_options = 0;
  m_new_name = _T("");
  m_status = _T("");
  m_gpt_entry_start = 2;
  //}}AFX_DATA_INIT
  m_created = FALSE;
}

void CNewImage::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CNewImage)
  DDX_Control(pDX, IDC_BOUNDARY, m_boundary);
  DDX_Control(pDX, IDC_PROGRESS, m_progress);
  DDX_Control(pDX, IDC_COUNT_SPIN, m_count_spin);
  DDX_Control(pDX, IDC_PARTITIONS, m_partitions);
  DDX_Text(pDX, IDC_PART_COUNT, m_cur_parts);
  DDX_Radio(pDX, IDC_SECT_SIZE_512, m_sect_size);
  DDX_Radio(pDX, IDC_TYPE_PLAIN, m_type);
  DDX_Radio(pDX, IDC_RAW_FLAT, m_options);
  DDX_Text(pDX, IDC_NEW_NAME, m_new_name);
  DDX_Text(pDX, IDC_STATUS, m_status);
  DDX_Text(pDX, IDC_GPT_ENTRY_START, m_gpt_entry_start);
  DDV_MinMaxUInt(pDX, m_gpt_entry_start, 2, 65536);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewImage, CDialog)
  //{{AFX_MSG_MAP(CNewImage)
  ON_NOTIFY(UDN_DELTAPOS, IDC_COUNT_SPIN, OnDeltaposCountSpin)
  ON_MESSAGE(WM_DROPFILES, OnDropFiles)
  ON_BN_CLICKED(IDC_TYPE_PLAIN, OnTypeChange)
  ON_BN_CLICKED(IDC_TYPE_GPT, OnTypeChange)
  ON_BN_CLICKED(IDC_TYPE_MBR, OnTypeChange)
  ON_BN_CLICKED(IDC_TYPE_EMBR, OnTypeChange)
  ON_BN_CLICKED(IDC_TYPE_ISO9660, OnTypeChange)
  ON_BN_CLICKED(IDC_TYPE_ISO_UDF, OnTypeChange)
  ON_BN_CLICKED(IDC_SECT_SIZE_512, OnSectSizeChange)
  ON_BN_CLICKED(IDC_SECT_SIZE_1024, OnSectSizeChange)
  ON_BN_CLICKED(IDC_SECT_SIZE_2048, OnSectSizeChange)
  ON_BN_CLICKED(IDC_SECT_SIZE_4096, OnSectSizeChange)
  ON_BN_CLICKED(IDOK, OnCreateImage)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewImage message handlers
BOOL CNewImage::OnInitDialog() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  m_sector_size = dlg->m_sect_size;
  switch (m_sector_size) {
    case 1024:
      m_sect_size = 1;
      break;
    case 2048:
      m_sect_size = 2;
      break;
    case 4096:
      m_sect_size = 3;
      break;
    case 512:
    default:
      m_sect_size = 0;
      break;
  }

  CDialog::OnInitDialog();
  
  // Create at least one partition
  CreateNewPart(0);
  m_cur_parts = 1;
  m_max_parts = 1;
  
  // Create the Sheet
  CRect rect;
  m_Sheet.EnableStackedTabs(FALSE);
  m_Sheet.Create(this, WS_VISIBLE | WS_CHILD | WS_DLGFRAME);
  m_Sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
  m_partitions.GetWindowRect(&rect);
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
  
  m_count_spin.SetRange(1, 1); // currently set to "Plain"
  m_boundary.SetCurSel(0);
  
  SetDlgItemText(IDC_STATIC_NOTES, 
    "Simple string of sectors.  No partitioning scheme used.\n"
    "For a 1.44m floppy, set the count of sectors to 2880.\n"
  );
  
  return TRUE;
}

LRESULT CNewImage::OnDropFiles(WPARAM wParam, LPARAM lParam) {
  CString cs;
  char szDroppedFile[MAX_PATH];
  
  DragQueryFile((HDROP) wParam, // Struture Identifier
        0,                // Index (of 0xFFFFFFFF to get count of files dropped)
        szDroppedFile,    // Droped File Name
        MAX_PATH);        // Max char 
  
  int index = m_Sheet.GetActiveIndex();
  if (m_Parts[index].m_dirty) {
    cs.Format("Update partition with new file: %s", szDroppedFile);
    if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
      return 0;
  }
  
  cs = szDroppedFile;
  UpdateEntry(cs, index);
  
  return 0;
}

void CNewImage::UpdateEntry(CString cs, const int index) {
  m_Parts[index].UpdateData(TRUE); // bring from Dialog
  m_Parts[index].m_filename = cs;
  
  CFile file;
  if (file.Open(cs, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
    CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
    LARGE_INTEGER file_length = dlg->GetFileLength((HANDLE) file.m_hFile);
    m_Parts[index].m_sectors = (DWORD) ((file_length.QuadPart + (m_sector_size - 1)) / m_sector_size);
    file.Close();
  }
  
  m_Parts[index].UpdateData(FALSE); // Send to Dialog
  m_Parts[index].m_dirty = TRUE;
}

void CNewImage::CreateNewPart(const int index) {
  m_Parts[index].Construct(IDD_NEW_PART, 0);
  m_Parts[index].m_Title.Format("Entry %i", index);
  m_Parts[index].m_psp.dwFlags |= PSP_USETITLE;
  m_Parts[index].m_psp.pszTitle = m_Parts[index].m_Title;
  m_Parts[index].m_index = index;
  m_Parts[index].m_parent = this;
  
  m_Parts[index].m_filename.Empty();
  m_Parts[index].m_sectors = 10240;
  m_Parts[index].m_name.Empty();
  m_Parts[index].m_bootable = FALSE;
  m_Parts[index].m_dirty = FALSE;

  m_Sheet.AddPage(&m_Parts[index]);
  m_Sheet.SetActivePage(&m_Parts[index]);
}

BOOL CNewImage::RemovePart(const int index, const BOOL ask) {
  CString cs;
  
  if (ask) {
    if (m_Parts[index].m_dirty) {
      m_Sheet.SetActivePage(&m_Parts[index]);
      cs.Format("Remove modified entry #%i?", index);
      if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
        return FALSE;
    }
  }
  
  m_Sheet.RemovePage(&m_Parts[index]);
  m_cur_parts--;
  if (index >= m_cur_parts)
    m_Sheet.SetActivePage(m_cur_parts - 1);
  
  return TRUE;
}

void CNewImage::OnDeltaposCountSpin(NMHDR *pNMHDR, LRESULT *pResult) {
  NM_UPDOWN *pNMUpDown = (NM_UPDOWN *) pNMHDR;
  LRESULT result = 0; // 0 = allow the change to happen, 1 = don't change the spin control
  
  // pNMUpDown->iPos = current position
  // pNMUpDown->iDelta = 1 is increase, = -1 is decrease
  int newval = pNMUpDown->iPos + pNMUpDown->iDelta;
  if ((newval > 0) && (newval <= m_max_parts)) {
    if (newval > m_cur_parts) {
      CreateNewPart(m_cur_parts);

      // disable/enable the items depending on if we are a CDROM
      // these will be shown only on the first page
      m_Parts[m_cur_parts].GetDlgItem(IDC_NEW_NAME)->EnableWindow((m_type == 2) || (m_type == 3));
      m_Parts[m_cur_parts].GetDlgItem(IDC_BOOT_FRAME)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_BOOTABLE)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_ISO9660_NO_EMU)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_ISO9660_120)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_ISO9660_144)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_ISO9660_288)->EnableWindow(m_type == 4);
      m_Parts[m_cur_parts].GetDlgItem(IDC_ISO9660_HD)->EnableWindow(m_type == 4);

      m_cur_parts++;
    } else if (newval < m_cur_parts)
      if (!RemovePart(m_cur_parts - 1, TRUE))
        result = 1;
  }
  
  *pResult = result;
}

void CNewImage::OnTypeChange() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  UpdateData(TRUE);  // bring from Dialog
  BOOL isharddrive = TRUE;
  
  switch (m_type) {
    case 0:  // Plain
      m_max_parts = 1;
      //m_sector_size = dlg->m_sect_size;
      SetDlgItemText(IDC_STATIC_NOTES, 
        "Simple string of sectors.  No partitioning scheme used.\n"
        "For a 1.44m floppy, set the count of sectors to 2880.\n"
      );
      break;
    case 1:  // MBR
      m_max_parts = 4;
      //m_sector_size = dlg->m_sect_size;
      SetDlgItemText(IDC_STATIC_NOTES, 
        "MBR Partitioning Scheme, up to four partitions allowed.\n"
        "Use the Partition Count field to set how many partitions to use.\n"
        "Use a Starting Boundary value.  Choose a default or enter a number.\n"
      );
      break;
    case 2:  // eMBR
      m_max_parts = MAX_EMBR_ENTRIES;
      //m_sector_size = dlg->m_sect_size;
      SetDlgItemText(IDC_STATIC_NOTES, 
        "eMBR Partitioning Scheme, up to sixteen partitions allowed.\n"  // MAX_EMBR_ENTRIES = 16
        "Use the Partition Count field to set how many partitions to use.\n"
        "Use a Starting Boundary value.  Choose a default or enter a number.\n"
      );
      break;
    case 3:  // GPT
      m_max_parts = MAX_PARTS;
      //m_sector_size = dlg->m_sect_size;
      SetDlgItemText(IDC_STATIC_NOTES, 
        "UEFI Partitioning Scheme, up to sixteen partitions allowed.\n"   // MAX_GPT_ENTRIES = 16
        "Use the Partition Count field to set how many partitions to use.\n"
        "Use a Starting Boundary value.  Choose a default or enter a number.\n"
      );
      break;
    case 4:  // ISO 9660
    case 5:  // ISO UDF
      m_max_parts = 1;
      isharddrive = FALSE;
      m_sect_size = 2;  // set to 2048
      m_sector_size = 2048;
      SetDlgItemText(IDC_STATIC_NOTES, 
        "ISO 9660/UDF with no partitioning scheme used.\n"
        "If Bootable, this will ask for boot file at creation time.\n"
      );
      // don't allow a CDROM as a VDI file
      if (m_options != 0) {
        AfxMessageBox("The app doesn't allow a CDROM image to be in the VDI format,\r\n"
                      " or have a VHD footer.  Options have been changed to RAW.");
        m_options = 0;
      }
      break;
  }
  
  if (isharddrive && (m_sect_size == 2)) {
    m_sect_size = 0;  // set to 512
    m_sector_size = 512;
  }
  
  // enable/disable other items compared to what was selected
  GetDlgItem(IDC_WITH_VHD)->EnableWindow(isharddrive);
  GetDlgItem(IDC_VDI_DYNAMIC)->EnableWindow(isharddrive);
  GetDlgItem(IDC_VDI_FLAT)->EnableWindow(isharddrive);
  GetDlgItem(IDC_PART_COUNT)->EnableWindow(isharddrive);
  GetDlgItem(IDC_BOUNDARY)->EnableWindow(isharddrive);
  GetDlgItem(IDC_SECT_SIZE_512)->EnableWindow(isharddrive);
  GetDlgItem(IDC_SECT_SIZE_1024)->EnableWindow(isharddrive);
  //GetDlgItem(IDC_SECT_SIZE_2048)->EnableWindow(!isharddrive);  // always available
  GetDlgItem(IDC_SECT_SIZE_4096)->EnableWindow(isharddrive);
    
  GetDlgItem(IDC_GPT_ENTRY_START)->EnableWindow(m_type == 3);
  GetDlgItem(IDC_GPT_ENTRY_START_S)->EnableWindow(m_type == 3);
  GetDlgItem(IDC_BOUNDARY)->EnableWindow((m_type >= 1) && (m_type <= 3));

  // make sure the VHD checkbox is unchecked if ISO
  //if (!isharddrive)
  //  m_with_vhd = FALSE;
  
  int i = m_cur_parts;
  while (m_max_parts < i)
    RemovePart(--i, FALSE);
  
  // if type is plain image, no need for a name
  for (i=0; i<m_cur_parts; i++) {
    m_Parts[i].GetDlgItem(IDC_NEW_NAME)->EnableWindow((m_type == 2) || (m_type == 3));
    
    // disable/enable the items depending on if we are a CDROM
    // these will be shown only on the first page
    m_Parts[i].GetDlgItem(IDC_BOOT_FRAME)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_BOOTABLE)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_ISO9660_NO_EMU)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_ISO9660_120)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_ISO9660_144)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_ISO9660_288)->EnableWindow(m_type == 4);
    m_Parts[i].GetDlgItem(IDC_ISO9660_HD)->EnableWindow(m_type == 4);
  }
  
  if (m_cur_parts > m_max_parts) {
    AfxMessageBox("Too many partitions for this type of image...");
    m_max_parts = MAX_PARTS;
    m_count_spin.SetRange(1, MAX_PARTS);
    m_type = (m_cur_parts < 4) ? 1 : 2;
  }
  
  m_count_spin.SetRange(1, m_max_parts);
  m_count_spin.Invalidate(FALSE);
  
  UpdateData(FALSE);  // Send to Dialog
}

void CNewImage::OnSectSizeChange() {
  UpdateData(TRUE);  // bring from Dialog

  switch (m_sect_size) {
    case 1:
      m_sector_size = 1024;
      break;
    case 2:
      m_sector_size = 2048;
      break;
    case 3:
      m_sector_size = 4096;
      break;
    case 0:
    default:
      m_sector_size = 512;
      break;
  }

  // Since we changed the sector size, we need to update
  //  each partition's Meg Size display
  CNewPart *partition;
  int i, part_count = m_Sheet.GetPageCount();
  for (i=0; i<part_count; i++) {
    partition = (CNewPart *) m_Sheet.GetPage(i);
    partition->OnSizeChanged();
  }
}

void CNewImage::UpdateStatus(CString csStatus) {
  m_Status += csStatus;
  SetDlgItemText(IDC_STATUS, m_Status);
  CEdit *edit = (CEdit *) GetDlgItem(IDC_STATUS);
  edit->SetSel(m_Status.GetLength(), m_Status.GetLength(), FALSE);
  edit->Invalidate(FALSE);
}

#define EMBR_RESVED_SECTS  62

// Create the image
void CNewImage::OnCreateImage() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile srcfile;
  DWORD64 cur_lba;
  BYTE *buffer = NULL;
  CNewPart *partition;
  LARGE_INTEGER large_int, file_size;
  DWORD64 qword;
  UINT count;
  CString cs, csStatus;
  BOOL success = FALSE;
  BYTE head, sect;
  WORD cyl;
  int i, j;
  
  CWaitCursor wait; // display a wait cursor
  
  //  So that it makes it a lot easier to create a .VDI file,
  //   we use the dlg->m_file handle, set up some parameters 
  //   (VDI_members), and a few other things, then call dlg->WriteFile().
  //   This way the dlg->WriteFile() code will do the VDI Dynamic stuff
  //   for us.
  
  // set up some variables and default to a flat file type
  dlg->m_file_type = DLG_FILE_TYPE_FLAT;
  dlg->m_overwrite_okay = TRUE;
  dlg->m_sect_size = m_sector_size;
  
  // append ".vhd" to filename if VHD file and not already there
  if (m_options == 1)  // add VDH footer
    if (m_new_name.Right(4) != ".vhd")
      m_new_name += ".vhd";
  
  // append ".vdi" to filename if VDI file and not already there
  if ((m_options == 2) || (m_options == 3))  // Dynamic or Flat VDI file
    if (m_new_name.Right(4) != ".vdi")
      m_new_name += ".vdi";
  
  if (dlg->m_file.Open(m_new_name, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareExclusive, NULL) == 0) {
    AfxMessageBox("Error Creating Image File...");
    return;
  }
  
  // truncate the file
  dlg->m_file.SetLength(0);
  
  UpdateData(TRUE);  // Bring from Dialog
  
  // clear the status
  m_Status.Empty();
  csStatus.Format("Creating %s\r\n", m_new_name);
  UpdateStatus(csStatus);
  m_progress.SetRange(0, 110); // 110 so that we do 10 up to the writing of the partitions, then have 100% to work with
  m_progress.SetPos(0);
  //m_progress.SetMarquee(TRUE, 1000);  // use this instead?  ->    LRESULT lResult = ::SendMessage(m_progress, PBM_SETMARQUEE, (WPARAM) TRUE, (LPARAM) 1);

  // get the sector padding
  DWORD64 padding = 0;
  GetDlgItemText(IDC_BOUNDARY, cs);
  if (cs != "None")
    padding = convert64(cs);
  if (padding > 4096) {
    cs.Format("Partition Padding is > 4096: %i\r\nContinue?", padding);
    if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES) {
      dlg->m_file.Close();
      csStatus = "Aborting...\r\n";
      UpdateStatus(csStatus);
      free(buffer);
      return;
    }
  }

  // calculate last sector in image
  int part_count = m_Sheet.GetPageCount();
  DWORD64 lba_count = padding;
  for (i=0; i<part_count; i++) {
    partition = (CNewPart *) m_Sheet.GetPage(i);
    if (padding > 0)
      lba_count = ((lba_count + padding - 1) / padding) * padding;  // pad sectors to next boundary
    lba_count += partition->m_sectors;
  }
  
  // if type is GPT, we need to calculate the size of the entry(s),
  //  as well as add to the sector count
  int gpt_entries_size = 0;  // in sectors
  if (m_type == 3) {
    gpt_entries_size = ((((part_count > MAX_GPT_ENTRIES) ? MAX_GPT_ENTRIES : part_count) * sizeof(struct S_GPT_ENTRY)) + (m_sector_size - 1)) / m_sector_size;
    // must be at least 32 for 512-bytes sectors, or 4 for 4096-byte sectors....
    if ((m_sector_size == 512) && (gpt_entries_size < 32))
      gpt_entries_size = 32;
    if ((m_sector_size == 4096) && (gpt_entries_size < 4))
      gpt_entries_size = 4;
    if (padding < 1 + m_gpt_entry_start + gpt_entries_size)
      lba_count += ((1 + m_gpt_entry_start + gpt_entries_size) - padding);   // entries
    lba_count += gpt_entries_size; // backup entries
    lba_count++;                   // backup GPT header
  }
  
  // allocate a minumal buffer
  buffer = (BYTE *) malloc(MAX_SECT_SIZE);

  // if we are creating a VDI file, we need to set up a few things first.
  if ((m_options == 2) || (m_options == 3)) {  // Dynamic or Flat VDI file
    dlg->m_vdi_block_size = 1048576;  // 1 meg
    dlg->m_vdi_disk_size = ((lba_count + 1) * m_sector_size); 
    dlg->m_vdi_block_count = (DWORD) ((dlg->m_vdi_disk_size + (dlg->m_vdi_block_size - 1)) / dlg->m_vdi_block_size);
    dlg->m_vdi_image_type = VDI_IMAGE_TYPE_DYNAMIC;
    dlg->m_vdi_image_flags = 0;
    dlg->m_vdi_offset_blocks = m_sector_size;  // byte offset in file where the block table starts
    dlg->m_vdi_offset_data = m_sector_size + (((dlg->m_vdi_block_count * 4) + (m_sector_size - 1)) & ~(m_sector_size - 1));
    dlg->m_vdi_blocks_allocated = (m_options == 2) ? 0 : dlg->m_vdi_block_count;

    // before we change the file_type, write a VDI header to the file
    struct VDI_HEADER *header = (struct VDI_HEADER *) buffer;
    memset(header, 0, m_sector_size);
    strcpy((char *) header->cookie, "<<< Oracle VM VirtualBox Disk Image >>>\n");  // ? Unknown if it requres the \n at the end
    header->signature = 0xBEDA107F;
    header->version = 0x00010001;
    header->size_header = m_sector_size - 112;
    header->image_type = (m_options == 2) ? VDI_IMAGE_TYPE_DYNAMIC : VDI_IMAGE_TYPE_FLAT;
    header->image_flags = 0;
    //header->description
    header->offset_blocks = dlg->m_vdi_offset_blocks;
    header->offset_data = (DWORD) dlg->m_vdi_offset_data;
    header->cylinders = 0;
    header->heads = 0;
    header->sectors = 0;
    header->sector_size = m_sector_size;
    header->unused = 0;
    header->disk_size = dlg->m_vdi_disk_size;
    header->block_size = dlg->m_vdi_block_size;
    header->block_extra = 0;
    header->block_count = dlg->m_vdi_block_count;
    header->blocks_allocated = dlg->m_vdi_blocks_allocated;
    
    // write the header to the file
    dlg->WriteToFile(buffer, 0, 1);

    // allocate a block array
    //  (no need to write it to the file now, since we mark it as
    //   dirty, and the dlg->vdi_close_file() will write it for us)
    dlg->m_vdi_blocks = (DWORD *) malloc(dlg->m_vdi_block_count * sizeof(DWORD));
    if (header->image_type == VDI_IMAGE_TYPE_DYNAMIC) {
      for (size_t u=0; u<dlg->m_vdi_block_count; u++)
        dlg->m_vdi_blocks[u] = VDI_BLOCK_ID_UNALLOCATED;
    } else {
      void *zero_block = calloc(dlg->m_vdi_block_size, 1);
      for (size_t u=0; u<dlg->m_vdi_block_count; u++) {
        dlg->m_vdi_blocks[u] = (DWORD) u;
        // need to write the block to the file
        // we haven't changed to the DLG_FILE_TYPE_VB_VDI type yet,
        //  so we must calculate it as a raw flat file
        large_int.QuadPart = dlg->m_vdi_offset_data + (u * dlg->m_vdi_block_size);
        ::SetFilePointerEx((HANDLE) dlg->m_file.m_hFile, large_int, NULL, FILE_BEGIN);
        dlg->m_file.Write(zero_block, dlg->m_vdi_block_size);
      }
      free(zero_block);
    }
    dlg->m_vdi_table_dirty = TRUE;

    // make sure we change the file type last
    dlg->m_file_type = DLG_FILE_TYPE_VB_VDI;
  }

  // if type is Plain, MBR, EMBR, or GPT
  if (m_type <= 3) {
    BOOL hasMBR = (m_type > 0);
    BOOL hasEMBR = (m_type == 2);
    BOOL hasGPT = (m_type == 3);
    
    cur_lba = ((hasMBR)  ? 1 : 0) +
              ((hasEMBR) ? EMBR_RESVED_SECTS : 0) +
              ((hasGPT)  ? (m_gpt_entry_start + gpt_entries_size) : 0);
    
    // calculate the LBA's for each partition
    csStatus.Format("Calculating locations for %i partitions\r\n", part_count);
    UpdateStatus(csStatus);
    for (i=0; i<part_count; i++) {
      partition = (CNewPart *) m_Sheet.GetPage(i);
      partition->UpdateData(TRUE);  // bring from Dialog
      if (padding > 0)
        cur_lba = ((cur_lba + padding - 1) / padding) * padding;  // pad sectors to next boundary
      partition->m_lba = cur_lba;
      cur_lba += partition->m_sectors;
      csStatus.Format("Partition #%i at: %I64i\r\n", i, partition->m_lba);
      UpdateStatus(csStatus);
    }
    
    // we are "5%" done
    m_progress.SetPos(5);
    
    // make a MBR
    if (hasMBR) {
      csStatus = "Creating MBR\r\n";
      UpdateStatus(csStatus);
      memset(buffer, 0, MAX_SECT_SIZE);
      
      // place a dummy boot code in the MBR
      memcpy(buffer, mbr_dummy_code, sizeof(mbr_dummy_code));

      struct MBR_PART_ENTRY *part = (struct MBR_PART_ENTRY *) &buffer[512 - 2 - (4 * 16)];
      i = 0;
      if (hasEMBR) {
        csStatus = "Creating eMBR 'Encompassing' Entry\r\n";
        UpdateStatus(csStatus);
        part[0].boot_id = 0x80;
        part[0].start.cyl = 0;
        part[0].start.head = 0;
        part[0].start.sector = 0;
        part[0].sys_id = 0xE0;
        part[0].end.cyl = 0;
        part[0].end.head = 0;
        part[0].end.sector = 0;
        part[0].start_lba = 1;
        part[0].sectors = (cur_lba <= 0xFFFFFFFF) ? (DWORD) cur_lba : 0xFFFFFFFF;
        i = 1;
      } else
      if (hasGPT) {
        csStatus = "Creating GPT 'Encompassing' Entry\r\n";
        UpdateStatus(csStatus);
        // For EFI GPT to boot, the following values must be used
        part[0].boot_id = 0x00;      // must be zero
        part[0].start.cyl = 0;       //  CHS = 0, 0, 2
        part[0].start.head = 0;      //
        part[0].start.sector = 2;    // LBA 1
        part[0].sys_id = 0xEE;       // GPT EFI
        part[0].end.cyl = 0xFF;      // 
        part[0].end.head = 0xFF;     //
        part[0].end.sector = 0xFF;   //
        part[0].start_lba = 1;       // LBA 1
        part[0].sectors = /*(cur_lba <= 0xFFFFFFFF) ? (DWORD) cur_lba : */ 0xFFFFFFFF;
        i = 1;
      }
      if (part_count > 1) {
        csStatus.Format("Creating %i additional entries\r\n", ((part_count - 1) <= 3) ? (part_count - 1) : 3);
        UpdateStatus(csStatus);
      }
      for (j=0; j<part_count && i<4; i++, j++) {
        partition = (CNewPart *) m_Sheet.GetPage(j);
        part[i].boot_id = 0;
        gLBAtoCHS((DWORD) partition->m_lba, &cyl, &head, &sect, 63, 16);
        part[i].start.cyl = (BYTE) cyl;
        part[i].start.head = head;
        part[i].start.sector = sect;
        part[i].sys_id = 0;
        gLBAtoCHS((DWORD) (partition->m_lba + partition->m_sectors), &cyl, &head, &sect, 63, 16);
        part[i].end.cyl = (BYTE) cyl;
        part[i].end.head = head;
        part[i].end.sector = sect;
        part[i].start_lba = (DWORD) partition->m_lba;
        part[i].sectors = partition->m_sectors;
        csStatus.Format("Entry #%i at: %I64i\r\n", i, partition->m_lba);
        UpdateStatus(csStatus);
      }
      
      // sig
      * (WORD *) &buffer[512 - 2] = 0xAA55;
      
      // write the MBR
      csStatus = "Writing MBR to LBA 0\r\n";
      UpdateStatus(csStatus);
      dlg->WriteToFile(buffer, 0, 1);
    }
    
    if (hasEMBR) {
      csStatus.Format("Creating eMBR at LBA 1 with %i entries\r\n", (part_count <= MAX_EMBR_ENTRIES) ? part_count : MAX_EMBR_ENTRIES);
      UpdateStatus(csStatus);
      // (re)allocate a buffer to hold entries
      buffer = (BYTE *) realloc(buffer, EMBR_RESVED_SECTS * m_sector_size);
      memset(buffer, 0, EMBR_RESVED_SECTS * m_sector_size);
      struct S_EMBR_SIG *sig = (struct S_EMBR_SIG *) &buffer[0x1F2];
      sig->sig = EMBR_SIG_SIG;
      sig->offset = 20;
      sig->remaining = EMBR_RESVED_SECTS;
      sig->boot_sig = 0xAA55;
      
      struct S_EMBR_HDR *hdr = (struct S_EMBR_HDR *) (buffer + ((sig->offset - 1) * m_sector_size));
      hdr->sig0 = EMBR_HDR_SIG0;
      hdr->crc = 0;
      hdr->entry_count = (WORD) part_count;
      hdr->boot_delay = 20;
      hdr->version = 0x25;
      hdr->sig1 = EMBR_HDR_SIG1;
      
      struct S_EMBR_ENTRY *entries = (struct S_EMBR_ENTRY *) ((BYTE *) hdr + sizeof(struct S_EMBR_HDR));
      qword = 0;
      for (i=0; i<part_count && i<MAX_EMBR_ENTRIES; i++) {
        partition = (CNewPart *) m_Sheet.GetPage(i);
        entries[i].flags = EMBR_VALID_ENTRY;
        entries[i].signature = EMBR_ENTRY_SIG;
        entries[i].starting_sector = partition->m_lba;
        entries[i].sector_count = partition->m_sectors;
        if (qword < (partition->m_lba + partition->m_sectors))
          qword = (partition->m_lba + partition->m_sectors);
        strcpy((char *) entries[i].description, partition->m_name.Left(63));
        entries[i].date_created = 0;
        entries[i].date_last_booted = 0;
        entries[i].OS_signature = 0;
        csStatus.Format("Entry #%i at: %I64i\r\n", i, partition->m_lba);
        UpdateStatus(csStatus);
      }
      hdr->total_sectors = qword;
      
      // calculate CRC
      hdr->crc = crc32(hdr, sizeof(struct S_EMBR_HDR) + (i * sizeof(struct S_EMBR_ENTRY)));
      
      // write the eMBR
      csStatus.Format("Writing eMBR at LBA 1 with %i sectors\r\n", EMBR_RESVED_SECTS);
      UpdateStatus(csStatus);
      dlg->WriteToFile(buffer, 1, EMBR_RESVED_SECTS);
    }  
    
    if (hasGPT) {
      csStatus.Format("Creating GPT at LBA 1 with %i entries\r\n", (part_count <= MAX_GPT_ENTRIES) ? part_count : MAX_GPT_ENTRIES);
      UpdateStatus(csStatus);
      // (re)allocate a buffer to hold entries
      buffer = (BYTE *) realloc(buffer, (m_sector_size + (gpt_entries_size * m_sector_size)));
      memset(buffer, 0, (m_sector_size + (gpt_entries_size * m_sector_size)));
      
      struct S_GPT_HDR *hdr = (struct S_GPT_HDR *) buffer;
      memcpy(hdr->sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8);
      hdr->version = 0x00010000;
      hdr->hdr_size = 92;
      hdr->crc32 = 0;
      hdr->resv0 = 0;
      hdr->primary_lba = 1;
      hdr->last_usable = cur_lba - 1;
      hdr->backup_lba = (cur_lba += gpt_entries_size);
      hdr->first_usable = m_gpt_entry_start + gpt_entries_size; // LBA after entries
      GUID_Create(&hdr->guid, GUID_TYPE_RANDOM);
      hdr->entry_offset = m_gpt_entry_start;
      hdr->entries = 128; //part_count;
      hdr->entry_size = 128;
      hdr->crc32_entries = 0;
      
      struct S_GPT_ENTRY *entries = (struct S_GPT_ENTRY *) (buffer + m_sector_size);
      for (i=0; i<part_count && i<MAX_GPT_ENTRIES; i++) {
        partition = (CNewPart *) m_Sheet.GetPage(i);
        GUID_Create(&entries[i].guid_type, GUID_TYPE_MT);
        GUID_Create(&entries[i].guid, GUID_TYPE_RANDOM);
        //// TODO: partition->m_lba >= hdr->first_usable /////////////////////////////
        entries[i].first_lba = partition->m_lba;
        entries[i].last_lba = partition->m_lba + partition->m_sectors - 1;
        entries[i].attribute = 5;  // Required + Legacy
        MultiByteToWideChar(CP_ACP, 0, partition->m_name, -1, (LPWSTR) entries[i].name, 72);
        csStatus.Format("Entry #%i at: %I64i\r\n", i, partition->m_lba);
        UpdateStatus(csStatus);
      }
      
      // calculate Entries CRC
      hdr->crc32_entries = crc32(entries, (128 * sizeof(struct S_GPT_ENTRY))); //hdr->crc32_entries = crc32(entries, (i * sizeof(struct S_GPT_ENTRY)));
      // calculate CRC
      hdr->crc32 = crc32(hdr, hdr->hdr_size);
      
      // write the GPT
      csStatus.Format("Writing GPT at LBA 1 with %i sectors\r\n", gpt_entries_size + 1);
      UpdateStatus(csStatus);
      dlg->WriteToFile(buffer, 1, 1);  // write GPT header
      dlg->WriteToFile(buffer + m_sector_size, m_gpt_entry_start, gpt_entries_size); // write Entries
      
      // write backup to hdr->backup_lba
      csStatus.Format("Writing Backup GPT at LBA %I64i\r\n", hdr->backup_lba);
      UpdateStatus(csStatus);
      // we must swap the MyLBA and AlternateLBA values for the Backup Header
      DWORD64 temp = hdr->backup_lba;
      hdr->backup_lba = hdr->primary_lba;
      hdr->primary_lba = temp;
      hdr->entry_offset = temp - gpt_entries_size;  // entry offest points this (backup's) entries
      hdr->crc32 = 0;
      hdr->crc32 = crc32(hdr, hdr->hdr_size); // calculate new CRC (since we swapped values)
      //large_int.QuadPart = (temp * m_sector_size);
      //::SetFilePointerEx((HANDLE) dlg->m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      //dlg->m_file.Write(buffer, (1 * m_sector_size));  // write backup header
      dlg->WriteToFile(buffer, temp, 1); // write backup header
      //large_int.QuadPart = ((temp - gpt_entries_size) * m_sector_size);
      //::SetFilePointerEx((HANDLE) dlg->m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      //dlg->m_file.Write(buffer + m_sector_size, (gpt_entries_size * m_sector_size));
      dlg->WriteToFile(buffer + m_sector_size, (temp - gpt_entries_size), gpt_entries_size);
    }
    
    m_progress.SetPos(10);
    
    // now write the partitions
    // (we write them all, whether the MBR, EMBR, or GPT contain them)
    DWORD64 SectorsWritten = 0;
    buffer = (BYTE *) realloc(buffer, m_sector_size);
    for (i=0; i<part_count; i++) {
      partition = (CNewPart *) m_Sheet.GetPage(i);
      
      csStatus.Format("Writing Partition #%i at LBA %I64i\r\n", i, partition->m_lba);
      UpdateStatus(csStatus);
      
      qword = 0;
      //large_int.QuadPart = (partition->m_lba * m_sector_size);
      //::SetFilePointerEx((HANDLE) dlg->m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      if (!partition->m_filename.IsEmpty()) {
        if (srcfile.Open(partition->m_filename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
          file_size = dlg->GetFileLength((HANDLE) srcfile.m_hFile);
          if (((file_size.QuadPart + (m_sector_size - 1)) / m_sector_size) > partition->m_sectors)
            AfxMessageBox("Partition Image is larger than allotted space.  Truncating...");
          for (; qword<partition->m_sectors; qword++) {
            count = srcfile.Read(buffer, m_sector_size);
            if (count == 0)
              break;
            if (count < m_sector_size)
              memset(buffer + count, 0, m_sector_size - count);
            //dlg->m_file.Write(buffer, m_sector_size);
            dlg->WriteToFile(buffer, partition->m_lba + qword, 1);
            if (count < m_sector_size)
              break;
          }
          srcfile.Close();
        } else
          AfxMessageBox("Error Opening Partition Image File...\r\n"
                        "Writing zeros...");
      }
      
      // either continue to pad partition (or)
      //  no partition file was given, so fill partition with zeros.
      memset(buffer, 0, m_sector_size);
      LONG lIdle = 0;
      int percent;
      for (; qword<partition->m_sectors; qword++) {
        //dlg->m_file.Write(buffer, m_sector_size);
        dlg->WriteToFile(buffer, partition->m_lba + qword, 1);
        //AfxGetApp()->OnIdle(lIdle++);  // avoid "Not Responding" notice
        percent = (int) (((double) qword / (double) partition->m_sectors) * 100.0);
        m_progress.SetPos(10 + percent);
      }
      // update the progress bar
      SectorsWritten += partition->m_sectors;
      percent = (int) (((double) SectorsWritten / (double) cur_lba) * 100.0);
      m_progress.SetPos(10 + percent);
    }
    
    // VHD (all is Big Endian)
    if (m_options == 1) { // add vhd
      cur_lba++;
      
      // seconds between 1/1/2000 and 1/1/1970  (365.2425 * 24 * 60 * 60) * 30
      CTime cTime = CTime::GetCurrentTime();
      const DWORD seconds = (DWORD) (cTime.GetTime() - (time_t) 946708560);
      const WORD cyl = (WORD) (cur_lba / 63 / 16);

      csStatus.Format("Writing VHD at LBA %I64i\r\n", cur_lba);
      UpdateStatus(csStatus);
      //large_int.QuadPart = (cur_lba * m_sector_size);
      //::SetFilePointerEx((HANDLE) dlg->m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      
      memset(buffer, 0, m_sector_size);
      struct VHD_FOOTER *footer = (struct VHD_FOOTER *) buffer;
      memcpy(footer->cookie, "conectix", 8);
      footer->features = ENDIAN_32U(2);
      footer->version = ENDIAN_32U(0x00010000);
      footer->data_offset = ENDIAN_64U(0xFFFFFFFFFFFFFFFFULL);
      footer->time_stamp = ENDIAN_32U(seconds);
      memcpy(footer->creator_ap, "ULTM", 4);
      footer->creator_ver = ENDIAN_32U(0x002C0021);
      footer->creator_host_os = ENDIAN_32U(0x6B326957); // Windows = 'Wi2k'
      footer->original_size = 
        footer->current_size = ENDIAN_64U((cur_lba - 1) * m_sector_size);
      footer->disk_geometry.cylinder = ENDIAN_16U(cyl);
      footer->disk_geometry.heads = 16;
      footer->disk_geometry.spt = 63;
      footer->disk_type = ENDIAN_32U(2); // fixed
      GUID_Create(&footer->uuid, GUID_TYPE_RANDOM);
      footer->saved_state = 0;

      // calculate crc of this footer
      DWORD crc = 0;
      for (unsigned i=0; i<m_sector_size; i++)
        crc += buffer[i];
      footer->checksum = ENDIAN_32U(~crc);

      //dlg->m_file.Write(buffer, m_sector_size);
      dlg->WriteToFile(buffer, cur_lba, 1);
    }
    success = TRUE;
    
  // Create ISO9660
  // Note:  We do not call the dlg->Write function when creating an ISO
  } else if (m_type == 4) {
    unsigned u;
    size_t img_size = 0;
    m_progress.SetPos(0);
    CFile srcfile;
    BOOL blank = FALSE;
    
    // check that we are at least 1024 sectors in size
    partition = (CNewPart *) m_Sheet.GetPage(0);
    partition->UpdateData(TRUE);  // bring from Dialog
    // see if we have a boot image to add to it
    if (partition->m_bootable) {
      // get a boot file from the host
      CFileDialog odlg(
        TRUE,             // Create an open file dialog
        _T(".img"),       // Default file extension
        NULL,             // Default Filename
        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
        _T(".img files (.img)|*.img|")    // Filter string
        _T(".bin files (.bin)|*.bin|")    // Filter string
        _T("All Files (*.*)|*.*|")        // Filter string
        _T("|")
      );
      odlg.m_ofn.lpstrTitle = "CDROM Boot Sector File";
      if (odlg.DoModal() == IDOK) {
        POSITION pos = odlg.GetStartPosition();
        if (srcfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0)
          img_size = (size_t) ((srcfile.GetLength() + 511) / 512);
        else
          AfxMessageBox("Error opening boot source file.");
      } else {
        AfxMessageBox("Bootable is checked, but no image given.\r\n"
                      "Will leave 16 sectors for future boot image.");
        img_size = 16 * 4;  // 16 2048-byte sectors
        blank = TRUE;  // write 16 blank sectors
      }
    }
    
    // make sure we are the correct size
    if (partition->m_sectors >= (32 + ((img_size + 3) / 4))) {
      // allocate a minumal buffer
      buffer = (BYTE *) calloc(16 * MAX_SECT_SIZE, 1);
      // write all (blank) sectors
      dlg->m_file.SeekToBegin();
      for (u=0; u<partition->m_sectors; u++) {
        dlg->m_file.Write(buffer, 2048);
        int percent = (int) (((double) u / (double) partition->m_sectors) * 100.0);
        m_progress.SetPos(percent);
      }
      
      // create the PVD
      struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) buffer;
      pvd->type = 1;
      memcpy(pvd->ident, "CD001", 5);
      pvd->ver = 1;
      memcpy(pvd->sys_ident, "WINXP                           ", 32);
      memcpy(pvd->vol_ident, "Forever Young Software 1984-2020", 32);
      pvd->num_lbas = partition->m_sectors;
      pvd->num_lbas_b = ENDIAN_32U(pvd->num_lbas);
      pvd->set_size = 1;
      pvd->set_size_b = ENDIAN_16U(1);
      pvd->sequ_num = 1;
      pvd->sequ_num_b = ENDIAN_16U(1);
      pvd->lba_size = 2048;
      pvd->lba_size_b = ENDIAN_16U(2048);
      pvd->path_table_size = PATH_SECT_SIZE * 2048;
      pvd->path_table_size_b = ENDIAN_32U(PATH_SECT_SIZE * 2048);
      pvd->pathl_loc = (DWORD) (((img_size + 3) / 4) + BOOT_IMG_SECT);
      pvd->pathlo_loc = 0;
      pvd->pathm_loc = ENDIAN_32U(pvd->pathl_loc);
      pvd->pathmo_loc = 0;
      
      pvd->root.len = 34;
      pvd->root.e_attrib = 0;
      pvd->root.extent_loc = (DWORD) (((img_size + 3) / 4) + BOOT_IMG_SECT + PATH_SECT_SIZE);
      pvd->root.extent_loc_b = ENDIAN_32U(pvd->root.extent_loc);
      pvd->root.data_len = ROOT_SECT_SIZE * 2048;
      pvd->root.data_len_b = ENDIAN_32U(ROOT_SECT_SIZE * 2048);
      
      CTime time = CTime::GetCurrentTime();
      pvd->root.date.since_1900 = (BYTE) (time.GetYear() - 1900);
      pvd->root.date.month = (BYTE) time.GetMonth();
      pvd->root.date.day = (BYTE) time.GetDay();
      pvd->root.date.hour = (BYTE) time.GetHour();
      pvd->root.date.min = (BYTE) time.GetMinute();
      pvd->root.date.sec = (BYTE) time.GetSecond();
      pvd->root.date.gmt_off = 0; ////
      
      pvd->root.flags = 0x02;  // directory
      pvd->root.unit_size = 0;
      pvd->root.gap_size = 0;
      pvd->root.sequ_num = 1;
      pvd->root.sequ_num_b = ENDIAN_16U(1);
      pvd->root.fi_len = 1;
      pvd->root.ident[0] = 0;
      memset(pvd->set_ident, 0x20, 128);
      memset(pvd->pub_ident, 0x20, 128);
      memset(pvd->prep_ident, 0x20, 128);
      memset(pvd->app_ident, 0x20, 128);
      memcpy(pvd->app_ident, "Forever Young Software  ULTIMATE.EXE", 36);
      memset(pvd->copy_ident, 0x20, 37);
      memset(pvd->abs_ident, 0x20, 37);
      memset(pvd->bib_ident, 0x20, 37);
      
      //struct S_ISO_DATE_TIME vol_date;
      sprintf(pvd->vol_date.year, "%04i", time.GetYear());
      sprintf(pvd->vol_date.month, "%02i", time.GetMonth());
      sprintf(pvd->vol_date.day, "%02i", time.GetDay());
      sprintf(pvd->vol_date.hour, "%02i", time.GetHour());
      sprintf(pvd->vol_date.min, "%02i", time.GetMinute());
      sprintf(pvd->vol_date.sec, "%02i", time.GetSecond());
      sprintf(pvd->vol_date.jiffies, "%02i", 0);
      pvd->vol_date.gmt_off = 0;
      
      //struct S_ISO_DATE_TIME mod_date;
      sprintf(pvd->mod_date.year, "%04i", time.GetYear());
      sprintf(pvd->mod_date.month, "%02i", time.GetMonth());
      sprintf(pvd->mod_date.day, "%02i", time.GetDay());
      sprintf(pvd->mod_date.hour, "%02i", time.GetHour());
      sprintf(pvd->mod_date.min, "%02i", time.GetMinute());
      sprintf(pvd->mod_date.sec, "%02i", time.GetSecond());
      sprintf(pvd->mod_date.jiffies, "%02i", 0);
      pvd->mod_date.gmt_off = 0;
      
      //struct S_ISO_DATE_TIME exp_date;
      //sprintf(pvd->exp_date.year, "%04i", time.GetYear());
      //sprintf(pvd->exp_date.month, "%02i", time.GetMonth());
      //sprintf(pvd->exp_date.day, "%02i", time.GetDay());
      //sprintf(pvd->exp_date.hour, "%02i", time.GetHour());
      //sprintf(pvd->exp_date.min, "%02i", time.GetMinute());
      //sprintf(pvd->exp_date.sec, "%02i", time.GetSecond());
      //sprintf(pvd->exp_date.jiffies, "%02i", 0);
      //pvd->exp_date.gmt_off = 0;
      
      //struct S_ISO_DATE_TIME val_date;
      sprintf(pvd->val_date.year, "%04i", time.GetYear());
      sprintf(pvd->val_date.month, "%02i", time.GetMonth());
      sprintf(pvd->val_date.day, "%02i", time.GetDay());
      sprintf(pvd->val_date.hour, "%02i", time.GetHour());
      sprintf(pvd->val_date.min, "%02i", time.GetMinute());
      sprintf(pvd->val_date.sec, "%02i", time.GetSecond());
      sprintf(pvd->val_date.jiffies, "%02i", 0);
      pvd->val_date.gmt_off = 0;
      
      pvd->struct_ver = 1;
      
      // write the PVD
      dlg->m_file.Seek(PVD_SECT * 2048, CFile::begin);
      dlg->m_file.Write(buffer, 1 * 2048);
      
      // we are "100% + 1" done
      m_progress.SetPos(101);  // total of 110 for completely 100% done
      
      if (img_size > 0) {  // has boot image file
        // create Boot Descriptor
        memset(buffer, 0, 2048);
        struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) buffer;
        bvd->type = 0;
        memcpy(bvd->ident, "CD001", 5);
        bvd->ver = 1;
        memcpy(bvd->sys_ident, "EL TORITO SPECIFICATION", 23);
        bvd->boot_cat = BOOT_CAT_SECT;
        dlg->m_file.Seek(BVD_SECT * 2048, CFile::begin);
        dlg->m_file.Write(buffer, 1 * 2048);
        
        // advance to sector TVD_SECT and
        // write Term Volume Descriptor sector
        memset(buffer, 0, 2048);
        struct S_ISO_TERM *term = (struct S_ISO_TERM *) buffer;
        term->id = 255;
        memcpy(term->ident, "CD001", 5);
        term->ver = 1;
        dlg->m_file.Seek(TVD_SECT * 2048, CFile::begin);
        dlg->m_file.Write(buffer, 1 * 2048);
        
        // advance to sector BOOT_CAT_SECT and
        // write Boot Catalog sector
        memset(buffer, 0, 2048);
        struct S_ISO_BOOT_CAT *boot_cat = (struct S_ISO_BOOT_CAT *) buffer;
        boot_cat->val_entry.id = 1;
        boot_cat->val_entry.platform = 0;  // 0 = 80x86
        memcpy(boot_cat->val_entry.ident, "FYS ULTIMATE.EXE", 16);  // up to 24
        boot_cat->val_entry.key55 = 0x55;
        boot_cat->val_entry.keyAA = 0xAA;
        WORD crc = 0, *crc_p = (WORD *) &boot_cat->val_entry.id;
        for (u=0; u<16; u++) {
          if (u == 14) continue;  // skip current crc value
          crc += crc_p[u];
        }
        boot_cat->val_entry.crc = -crc;
        boot_cat->init_entry.bootable = (partition->m_bootable) ? 0x88 : 0x00; // 0x88 = bootable, 0x00 = non-bootable
        boot_cat->init_entry.media = partition->m_emulation;
        boot_cat->init_entry.load_seg = 0x0000;
        boot_cat->init_entry.sys_type = 0;
        boot_cat->init_entry.load_cnt = (WORD) img_size;  // load 'count' sectors for boot
        boot_cat->init_entry.load_rba = BOOT_IMG_SECT;
        boot_cat->end_entry.id = 0x91;  // no more entries follow
        dlg->m_file.Seek(BOOT_CAT_SECT * 2048, CFile::begin);
        dlg->m_file.Write(buffer, 1 * 2048);
        
        // advance to sector BOOT_IMG_SECT
        // and copy boot image
        memset(buffer, 0, 512);
        dlg->m_file.Seek(BOOT_IMG_SECT * 2048, CFile::begin);
        for (u=0; u<img_size; u++) {
          if (!blank)
            srcfile.Read(buffer, 1 * 512);
          dlg->m_file.Write(buffer, 1 * 512);
        }
        if (!blank)
          srcfile.Close();
      } else {
        // no boot image file
        memset(buffer, 0, 2048);
        dlg->m_file.Write(buffer, 1 * 2048);  // dummy boot descriptor
        dlg->m_file.Write(buffer, 1 * 2048);  // dummy term descriptor ?
        dlg->m_file.Write(buffer, 1 * 2048);  // dummy boot catalog
      }
      
      // path table
      memset(buffer, 0, 2048);
      // only one directory.  The ROOT.
      struct S_ISO_PATH *path_tab = (struct S_ISO_PATH *) buffer;
      path_tab->len_di = 1;
      path_tab->ext_attrib = 0;
      path_tab->loc = (DWORD) (((img_size + 3) / 4) + BOOT_IMG_SECT + PATH_SECT_SIZE);
      path_tab->parent = 1;
      memset(path_tab->ident, 0, 2);
      //dlg->m_file.Seek(TVD_SECT * 2048, CFile::begin);
      dlg->m_file.Write(buffer, 1 * 2048);
      
      // root table
      memset(buffer, 0, 2048);
      struct S_ISO_ROOT *root = (struct S_ISO_ROOT *) buffer;
      // two directory entrys.  One dir, one file (required?)
      root[0].len = 34;
      root[0].e_attrib = 0;
      root[0].extent_loc = (DWORD) (((img_size + 3) / 4) + BOOT_IMG_SECT + PATH_SECT_SIZE);
      root[0].extent_loc_b = ENDIAN_32U(root[0].extent_loc);
      root[0].data_len = 2048;
      root[0].data_len_b = ENDIAN_32U(2048);
      root[0].date.since_1900 = (BYTE) (time.GetYear() - 1900);
      root[0].date.month = (BYTE) time.GetMonth();
      root[0].date.day = (BYTE) time.GetDay();
      root[0].date.hour = (BYTE) time.GetHour();
      root[0].date.min = (BYTE) time.GetMinute();
      root[0].date.sec = (BYTE) time.GetSecond();
      root[0].date.gmt_off = 0; ////
      root[0].flags = 0x02;
      root[0].unit_size = 0;
      root[0].gap_size = 0;
      root[0].sequ_num = 0;
      root[0].sequ_num_b = 0;
      root[0].fi_len = 1;
      root[0].ident[0] = 0;
      // second entry only requires a difference of a 1 as the ident
      memcpy(&root[1], &root[0], sizeof(struct S_ISO_ROOT));
      root[1].ident[0] = 1;
      
      //dlg->m_file.Seek(TVD_SECT * 2048, CFile::begin);
      dlg->m_file.Write(buffer, 1 * 2048);
      
      m_progress.SetPos(110);  // total of 110 for completely 100% done
      success = TRUE;
    } else
      AfxMessageBox("Parition->Sectors value too small.");
    
  // Create ISO UDF
  } else if (m_type == 5) {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
  }
  
  // free the buffer
  if (buffer)
    free(buffer);
  
  if (success)
    csStatus = "Done...\r\n";
  else
    csStatus = "Aborting...\r\n";
  UpdateStatus(csStatus);
  
  // close the file
  dlg->vdi_close_file();
  dlg->m_file.Close();
  
  // restore the orginal mouse cursor
  wait.Restore();
  
  if (success) {
    AfxMessageBox("File created successfully");
    m_created = TRUE;
  } else
    m_created = FALSE;
}

void CNewImage::OnCancel() {
  CDialog::EndDialog((m_created) ? IDOK : IDCANCEL);
}
