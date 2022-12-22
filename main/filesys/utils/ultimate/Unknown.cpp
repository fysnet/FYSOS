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

// Unknown.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "Unknown.h"

#include "Attribute.h"

#include "Lean.h"
#include "Fat.h"
#include "FatFormat.h"
#include "ExFat.h"
#include "FYSFS.h"
#include "SFS.h"
#include "Ext2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnknown property page
IMPLEMENT_DYNCREATE(CUnknown, CPropertyPage)

CUnknown::CUnknown() : CPropertyPage(CUnknown::IDD) {
  //{{AFX_DATA_INIT(CUnknown)
  m_dump = _T("");
  //}}AFX_DATA_INIT
}

CUnknown::~CUnknown() {
}

void CUnknown::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CUnknown)
  DDX_Text(pDX, IDC_DUMP, m_dump);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUnknown, CPropertyPage)
  //{{AFX_MSG_MAP(CUnknown)
  ON_BN_CLICKED(ID_CLEAN, OnClean)
  ON_BN_CLICKED(ID_FORMAT, OnFormat)
  ON_BN_CLICKED(ID_INSERT, OnInsert)
  ON_BN_CLICKED(ID_INSERT_FAT12, OnInsertFAT12BPB)
  ON_BN_CLICKED(ID_INSERT_FAT16, OnInsertFAT16BPB)
  ON_BN_CLICKED(IDC_DUMP_FIRST, OnDumpFirst)
  ON_BN_CLICKED(IDC_DUMP_PREV, OnDumpPrev)
  ON_BN_CLICKED(IDC_DUMP_NEXT, OnDumpNext)
  ON_BN_CLICKED(IDC_DUMP_LAST, OnDumpLast)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnknown message handlers
BOOL CUnknown::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  // set the font of the DUMP window to a fixed font
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_DUMP)->SetFont(&dlg->m_DumpFont);
  
  // display the base/size string
  CString cs;
  cs.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, cs);
  
  cs.Format("Detection Counts...\r\n"
            " (Filesystem: matched 'n' of 'n' tests)\r\n"
            "    Fat: %i of %i\r\n"
            "   Ext2: %i of %i\r\n"
            "  ExFat: %i of %i\r\n"
            "   Lean: not applicable\r\n"
            "   NTFS: %i of %i\r\n"
            "    SFS: %i of %i\r\n"
            "  FYSFS: %i of %i\r\n"
            "    FSZ: %i of %i",
    m_det_counts.FatC, m_det_counts.FatT,      // FAT
    m_det_counts.Ext2C, m_det_counts.Ext2T,    // Ext2
    m_det_counts.ExFatC, m_det_counts.ExFatT,  // ExFat
    //m_det_counts.LeanC, m_det_counts.LeanT,    // Lean (Lean cannot return a count due to it checks multiple sectors. Each sector could have different counts.)
    m_det_counts.NtfsC, m_det_counts.NtfsT,    // NTFS
    m_det_counts.SfsC, m_det_counts.SfsT,      // SFS
    m_det_counts.FysFsC, m_det_counts.FysFsT,  // FYSFS
    m_det_counts.FszC, m_det_counts.FszT       // FSZ
  );
  GetDlgItem(IDC_COUNT_LIST)->SetFont(&dlg->m_DumpFont);
  SetDlgItemText(IDC_COUNT_LIST, cs);

  OnDumpPrev();

  return TRUE;
}

void CUnknown::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_current = 0;
  
  // 
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = "Unknown";
  dlg->m_image_bar.UpdateTitle(dlg->Unknown[index].m_draw_index, "Unknown");
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
}

// Simply zero out the "partition"
void CUnknown::OnClean() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  DWORD64 sect;
  
  CString cs;
  cs.Format("This will clear the partition.\r\n"
            "  Start LBA = %I64i\r\n"
            "    End LBA = %I64i\r\n"
            "  Proceed?\r\n", m_lba, m_lba + m_size - 1);
  if (AfxMessageBox(cs, MB_YESNO, 0) == IDYES) {
    CWaitCursor wait; // display a wait cursor
    memset(buffer, 0, dlg->m_sect_size);
    for (sect=m_lba; sect<m_lba + m_size - 1; sect++)
      dlg->WriteToFile(buffer, sect, 1);
    Start(m_lba, m_size, m_color, m_index, FALSE);
    OnDumpPrev();
  }
}

struct S_ATTRIBUTES format_attrbs[] = {
                //            |                               | <- max (col 67)
  { FS_LEAN,    FS_LEAN,   0, "Lean FS"                        , {-1, } },
  { FS_EXT2,    FS_EXT2,   1, "Linux Ext 2"                    , {-1, } },
  { FS_SFS,     FS_SFS,    2, "Simple FS"                      , {-1, } },
  { FS_EXFAT,   FS_EXFAT,  3, "exFAT"                          , {-1, } },
  { FS_FAT12,   FS_FAT12,  4, "FAT 12"                         , {-1, } },
  { FS_FAT16,   FS_FAT16,  5, "FAT 16"                         , {-1, } },
  { FS_FYSFS,   FS_FYSFS,  6, "FYS FS"                         , {-1, } },
  { FS_FAT32,   FS_FAT32,  7, "FAT 32"                         , {-1, } },
  { 0,        (DWORD) -1, -1, NULL                             , {-1, } }
};

void CUnknown::OnFormat() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CAttribute adlg;
  
  adlg.m_title = "Choose File System:";
  adlg.m_attrib = 0;
  adlg.m_attributes = format_attrbs;
  adlg.m_single = TRUE;
  if (adlg.DoModal() == IDOK) {
    switch (adlg.m_attrib) {
      case FS_LEAN:
        FormatLean();
        break;
      case FS_EXT2:
        FormatExt2();
        break;
      case FS_SFS:
        FormatSFS();
        break;
      case FS_EXFAT:
        FormatExFat();
        break;
      case FS_FAT12:
        FormatFat(FS_FAT12);
        break;
      case FS_FAT16:
        FormatFat(FS_FAT16);
        break;
      case FS_FYSFS:
        FormatFYSFS();
        break;
      case FS_FAT32:
        FormatFat(FS_FAT32);
        break;
    }
    dlg->ReadFromFile(buffer, m_lba, 1);
    DumpIt(m_dump, buffer, 0, dlg->m_sect_size, FALSE);
    UpdateData(FALSE); // send to dialog
  }
}

void CUnknown::FormatLean(void) {
  CLean Lean;
  
  // build a "bogus" Lean partition
  Lean.m_lba = m_lba;
  Lean.m_size = m_size;
  Lean.m_hard_format = TRUE;
  
  // format it
  if (Lean.Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatFat(int Type) {
  CFat Fat;
  
  // build a "bogus" Fat partition
  Fat.m_lba = m_lba;
  Fat.m_size = m_size;
  Fat.m_fat_size = Type;
  Fat.m_bpb_buffer = calloc(MAX_SECT_SIZE, 1);
  Fat.m_hard_format = TRUE;
  
  // format it
  if (Fat.FatFormat(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatExFat(void) {
  CExFat ExFat;
  
  // build a "bogus" Fat partition
  ExFat.m_lba = m_lba;
  ExFat.m_size = m_size;
  ExFat.m_hard_format = TRUE;
  
  // format it
  if (ExFat.ExFatFormat(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatFYSFS(void) {
  CFYSFS FYSFS;
  
  // build a "bogus" Fat partition
  FYSFS.m_lba = m_lba;
  FYSFS.m_size = m_size;
  FYSFS.m_hard_format = TRUE;
  
  // format it
  if (FYSFS.FYSFSFormat(TRUE, TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatSFS(void) {
  CSFS SFS;
  
  SFS.m_lba = m_lba;
  SFS.m_size = m_size;
  
  // format it
  SFS.m_hard_format = TRUE;

  if (SFS.Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::FormatExt2(void) {
  CExt2 Ext2;
  
  Ext2.m_lba = m_lba;
  Ext2.m_size = m_size;
  
  // format it
  Ext2.m_hard_format = TRUE;

  if (Ext2.Ext2Format(TRUE))
    AfxMessageBox("Minimal format complete.  Close image file and re-open to parse correctly.");
  else
    AfxMessageBox("Format aborted.");
}

void CUnknown::OnInsert() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FYSOSSIG *s_sig;
  CString cs;
  CFile file;
  BOOL reload = FALSE;
  
  CFileDialog odlg (
    TRUE,             // Create an open file dialog
    _T(".img"),       // Default file extension
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
  cs = odlg.GetNextPathName(pos);
  
  if (file.Open(cs, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening Image File...");
    return;
  }
  
  LARGE_INTEGER file_length = dlg->GetFileLength((HANDLE) file.m_hFile);
  DWORD64 TotalBlocks = (file_length.QuadPart + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  
  if (TotalBlocks > m_size) {
    int ret = AfxMessageBox("Size of image to insert is larger than space in this partition.  Truncate?", MB_TOPMOST | MB_DEFBUTTON3 | MB_ICONEXCLAMATION | MB_YESNOCANCEL, 0);
    if (ret == IDCANCEL) {
      file.Close();
      return;
    } else if (ret == IDYES)
      TotalBlocks = m_size;
  }
  
  cs.Format("This will overwrite the partition.\r\n"
            "  Start LBA = %I64i\r\n"
            "       Size = %I64i\r\n"
            "  Proceed?\r\n", m_lba, TotalBlocks);
  if (AfxMessageBox(cs, MB_YESNO, 0) == IDYES) {
    CWaitCursor wait; // display a wait cursor
    BYTE buffer[MAX_SECT_SIZE];
    DWORD64 sect;
    LONG lIdle = 0;
    for (sect=0; sect<TotalBlocks; sect++) {
      file.Read(buffer, dlg->m_sect_size);
      // if the first sector and we are FYSOS specific, update the base signature
      if ((sect == 0) && AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE)) {
        s_sig = (struct S_FYSOSSIG *) (buffer + S_FYSOSSIG_OFFSET);
        s_sig->base = m_lba;
        s_sig->sig = (rand() << 16) | rand();
        s_sig->boot_sig =0xAA55;
      }
      dlg->WriteToFile(buffer, m_lba + sect, 1);
      AfxGetApp()->OnIdle(lIdle++);  // avoid "Not Responding" notice
    }
    reload = TRUE;
  }
  
  file.Close();

  if (reload)
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

void CUnknown::OnInsertFAT12BPB() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;

  cs.Format("Insert a FAT12 BPB at LSN %I64i?", m_current);
  if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
    return;

  // create a minimum FAT 16 BPB
  memset(buffer, 0x00, MAX_SECT_SIZE);

  CFat Fat;
  CFatFormat format;
  format.m_fat_size = FS_FAT12;
  format.m_sectors = m_size;
  format.m_num_fats = 2;
  format.m_root_entries = 16; // in sectors
  format.m_sect_cluster = 1;
  if (format.DoModal() != IDOK)
    return;

  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) buffer;
  bpb->jmp[0] = 0xEB;  bpb->jmp[1] = 0x3C; bpb->jmp[2] = 0x90;
  memcpy(bpb->oem_name, "OEM_NAME", 8);
  bpb->bytes_per_sect = dlg->m_sect_size;
  bpb->sect_per_clust = format.m_sect_cluster;
  bpb->sect_reserved = 1;
  bpb->fats = format.m_num_fats;
  bpb->root_entrys = (format.m_root_entries * dlg->m_sect_size) / 32;
  if (m_size <= 0xFFFF) {
    bpb->sectors = (WORD) m_size;
    bpb->sect_extnd = 0;
  } else {
    bpb->sectors = 0;
    bpb->sect_extnd = (DWORD) m_size;
  }
  if (m_size <= 2880)
    bpb->descriptor = 0xF0;  // if it is a floppy, we need descriptor to be 0xF0
  else
    bpb->descriptor = 0xF8;  // else, assume it is a hard drive and use 0xF8
  bpb->sect_per_fat = Fat.CalcSectPerFat(m_size - bpb->sect_reserved - format.m_root_entries, format.m_sect_cluster, dlg->m_sect_size, FS_FAT12);
  bpb->sect_per_trk = 0;
  bpb->heads = 0;
  bpb->hidden_sects = (DWORD) m_lba;
  bpb->drive_num = 0;
  bpb->resv = 0;
  bpb->sig = 0;
  bpb->serial = 0;
  memcpy(bpb->label, "NO LABEL   ", 11);
  memcpy(bpb->sys_type, "FAT12   ", 8);
  
  // write it to the image
  dlg->WriteToFile(buffer, m_lba + m_current, 1);
  
  // update the display
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

void CUnknown::OnInsertFAT16BPB() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;

  cs.Format("Insert a FAT16 BPB at LSN %I64i?", m_current);
  if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
    return;

  // create a minimum FAT 16 BPB
  memset(buffer, 0x00, MAX_SECT_SIZE);

  CFat Fat;
  CFatFormat format;
  format.m_fat_size = FS_FAT16;
  format.m_sectors = m_size;
  format.m_num_fats = 2;
  format.m_root_entries = 16; // in sectors
  format.m_sect_cluster = 1;
  if (format.DoModal() != IDOK)
    return;

  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) buffer;
  bpb->jmp[0] = 0xEB;  bpb->jmp[1] = 0x3C; bpb->jmp[2] = 0x90;
  memcpy(bpb->oem_name, "OEM_NAME", 8);
  bpb->bytes_per_sect = dlg->m_sect_size;
  bpb->sect_per_clust = format.m_sect_cluster;
  bpb->sect_reserved = 1;
  bpb->fats = format.m_num_fats;
  bpb->root_entrys = (format.m_root_entries * dlg->m_sect_size) / 32;
  if (m_size <= 0xFFFF) {
    bpb->sectors = (WORD) m_size;
    bpb->sect_extnd = 0;
  } else {
    bpb->sectors = 0;
    bpb->sect_extnd = (DWORD) m_size;
  }
  if (m_size <= 2880)
    bpb->descriptor = 0xF0;  // if it is a floppy, we need descriptor to be 0xF0
  else
    bpb->descriptor = 0xF8;  // else, assume it is a hard drive and use 0xF8
  bpb->sect_per_fat = Fat.CalcSectPerFat(m_size - bpb->sect_reserved - format.m_root_entries, format.m_sect_cluster, dlg->m_sect_size, FS_FAT16);
  bpb->sect_per_trk = 0;
  bpb->heads = 0;
  bpb->hidden_sects = (DWORD) m_lba;
  bpb->drive_num = 0;
  bpb->resv = 0;
  bpb->sig = 0;
  bpb->serial = 0;
  memcpy(bpb->label, "NO LABEL   ", 11);
  memcpy(bpb->sys_type, "FAT16   ", 8);
  
  // write it to the image
  dlg->WriteToFile(buffer, m_lba + m_current, 1);
  
  // update the display
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

void CUnknown::OnDumpFirst() {
  m_current = 1;
  OnDumpPrev();
}

void CUnknown::OnDumpPrev() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;
  
  if (m_current > 0)
    m_current--;
  
  GetDlgItem(IDC_DUMP_FIRST)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_PREV)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_NEXT)->EnableWindow(m_current < (m_size - 1));
  GetDlgItem(IDC_DUMP_LAST)->EnableWindow(m_current < (m_size - 1));
  cs.Format("LSN %I64i of %I64i", m_current, m_size - 1);
  SetDlgItemText(IDC_DUMP_STR, cs);
  
  dlg->ReadFromFile(buffer, m_lba + m_current, 1);
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

void CUnknown::OnDumpNext() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;
  
  if (m_current < (m_size - 1))
    m_current++;
  
  GetDlgItem(IDC_DUMP_FIRST)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_PREV)->EnableWindow(m_current > 0);
  GetDlgItem(IDC_DUMP_NEXT)->EnableWindow(m_current < (m_size - 1));
  GetDlgItem(IDC_DUMP_LAST)->EnableWindow(m_current < (m_size - 1));
  cs.Format("LSN %I64i of %I64i", m_current, m_size - 1);
  SetDlgItemText(IDC_DUMP_STR, cs);
  
  dlg->ReadFromFile(buffer, m_lba + m_current, 1);
  DumpIt(m_dump, buffer, (DWORD) (m_current * dlg->m_sect_size), dlg->m_sect_size, FALSE);
  SetDlgItemText(IDC_DUMP, m_dump);
}

void CUnknown::OnDumpLast() {
  m_current = m_size - 2;
  OnDumpNext();
}
