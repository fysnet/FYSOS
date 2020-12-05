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

// MbrEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "UltimateDlg.h"

#include "Mbr.h"
#include "MbrEntry.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMbrEntry property page

IMPLEMENT_DYNCREATE(CMbrEntry, CPropertyPage)

CMbrEntry::CMbrEntry() : CPropertyPage(CMbrEntry::IDD) {
  //{{AFX_DATA_INIT(CMbrEntry)
  m_begin_cyl = _T("");
  m_begin_head = _T("");
  m_begin_sect = _T("");
  m_boot_id = _T("");
  m_end_cyl = _T("");
  m_end_head = _T("");
  m_end_sect = _T("");
  m_size = _T("");
  m_start_lba = _T("");
  m_sys_id = _T("");
  //}}AFX_DATA_INIT
  m_dirty = FALSE;
}

CMbrEntry::~CMbrEntry() {
}

void CMbrEntry::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMbrEntry)
  DDX_Text(pDX, IDC_MBR_BEGIN_CYL, m_begin_cyl);
  DDV_MaxChars(pDX, m_begin_cyl, 10);
  DDX_Text(pDX, IDC_MBR_BEGIN_HEAD, m_begin_head);
  DDV_MaxChars(pDX, m_begin_head, 10);
  DDX_Text(pDX, IDC_MBR_BEGIN_SECT, m_begin_sect);
  DDV_MaxChars(pDX, m_begin_sect, 10);
  DDX_Text(pDX, IDC_MBR_BOOT_ID, m_boot_id);
  DDV_MaxChars(pDX, m_boot_id, 10);
  DDX_Text(pDX, IDC_MBR_END_CYL, m_end_cyl);
  DDV_MaxChars(pDX, m_end_cyl, 10);
  DDX_Text(pDX, IDC_MBR_END_HEAD, m_end_head);
  DDV_MaxChars(pDX, m_end_head, 10);
  DDX_Text(pDX, IDC_MBR_END_SECT, m_end_sect);
  DDV_MaxChars(pDX, m_end_sect, 10);
  DDX_Text(pDX, IDC_MBR_SIZE, m_size);
  DDV_MaxChars(pDX, m_size, 10);
  DDX_Text(pDX, IDC_MBR_START_LBA, m_start_lba);
  DDV_MaxChars(pDX, m_start_lba, 10);
  DDX_Text(pDX, IDC_MBR_SYS_ID, m_sys_id);
  DDV_MaxChars(pDX, m_sys_id, 10);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMbrEntry, CPropertyPage)
  //{{AFX_MSG_MAP(CMbrEntry)
  ON_BN_CLICKED(IDC_MBR_BEGIN_UPDATE, OnMbrBeginUpdate)
  ON_BN_CLICKED(IDC_MBR_END_UPDATE, OnMbrEndUpdate)
  ON_BN_CLICKED(ID_MBR_APPLY, OnMbrApply)
  ON_EN_CHANGE(IDC_MBR_SYS_ID, OnChangeMbrSysId)
  ON_EN_CHANGE(IDC_MBR_BEGIN_CYL, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_BEGIN_HEAD, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_BEGIN_SECT, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_BOOT_ID, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_END_CYL, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_END_HEAD, OnTabItemChange)
  ON_EN_CHANGE(IDC_MBR_END_SECT, OnTabItemChange)
  ON_EN_KILLFOCUS(IDC_MBR_SIZE, OnTabItemChangeSize)
  ON_EN_CHANGE(IDC_MBR_START_LBA, OnTabItemChange)
  ON_BN_CLICKED(IDC_SYS_ID, OnSysId)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMbrEntry message handlers

// **** not more than 31 chars each line ****
const char *fdisk_ids[] = {
  "Empty",
  "FAT12",
  "XENIX root",
  "XENIX usr",
  "FAT16 < 32M",
  "Extended",
  "FAT16",
  "HPFS/NTFS/exFAT",
  "ATX",
  "ATX Bootable",
  "OS/2 Boot Manage",
  "W95 FAT32",
  "W95 FAT32 (LBA)",
  "*unknown*",
  "W95 FAT16 (LBA)",
  "W95 Ext'd (LBA)",
  "OPUS",                      // 16
  "Hidden FAT12",
  "Compaq diagnostic",
  "*unknown*",
  "Hidden FAT16 < 32meg",
  "*unknown*",
  "Hidden FAT16",
  "Hidded HPFS/NTFS",
  "AST SmartSleep",
  "*unknown*",
  "*unknown*",
  "Hidded W95 FAT32",
  "Hidded W95 FAT32",
  "*unknown*",
  "Hidded W95 FAT36",
  "*unknown*",
  "*unknown*",                 // 32
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "NEC DOS",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",                 // 48
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Plan 9",
  "*unknown*",
  "*unknown*",
  "PartitionMagic",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Venix 80286",               // 64
  "PPC PRep Boot",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "QNX4.x",
  "QNX4.x 2nd part",
  "QNX4.x 3rd part",
  "OnTrack DM",
  "OnTrack DM6 Aux",
  "CP/M",
  "SFS",
  "OnTrackDM6",
  "EZ-DRIVE",
  "Golden Bow",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Priam Edisk",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",                 // 96
  "SpeedStor",
  "*unknown*",
  "GNU HURD or Sys",
  "Novell Netware",
  "Novell Netware",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "DiskSecure Mult",           // 112
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "PC/IX",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Old Minix",                 // 128
  "Minix / old Linux",
  "Linux swap",
  "Linux",
  "OS/2 Hidden C:",
  "Linux extended",
  "NTFS volume set",
  "NTFS volume set",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Linux LVM",
  "*unknown*",
  "*unknown*",                 // 144
  "*unknown*",
  "*unknown*",
  "Amoeba",
  "Amoeba BBT",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "BSD/OS",
  "IBM Thinkpad hi",           // 160
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "FreeBSD",
  "OpenBSD",
  "NeXTSTEP",
  "Darwin UFS",
  "NetBSD",
  "*unknown*",
  "Darwin boot",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",                 // 176
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "BSDI fs",
  "BSDI swap",
  "*unknown*",
  "*unknown*",
  "Boot Wizard hidden",
  "*unknown*",
  "*unknown*",
  "Solaris boot",
  "*unknown*",
  "*unknown*",                 // 192
  "DRDOS/sec (FAT-",
  "*unknown*",
  "*unknown*",
  "DRDOS/sec (FAT-",
  "*unknown*",
  "DRDOS/sec (FAT-",
  "Syrinx",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",                 // 208
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Non-FS data",
  "*unknown*",
  "*unknown*",
  "CP/M / CTOS / ",
  "Dell Utility",
  "BootIt",
  "eMBR",                      // 224
  "DOS access",
  "*unknown*",
  "DOS R/O",
  "SpeedStor",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "LeanFS",
  "BeOS fs",
  "*unknown*",
  "*unknown*",
  "EFI GPT Protect",
  "EFI (FAT-12/16/",
  "Linux/PA-RISC b",           // 240
  "SpeedStor",
  "DOS secondary",
  "*unknown*",
  "SpeedStor",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "*unknown*",
  "Linux raid auto",
  "LANstep",
  "BBT"
};

void CMbrEntry::OnMbrBeginUpdate() {
  UpdateData(TRUE);  // bring from Dialog
  DWORD lba;
  BYTE head, sect;
  WORD cyl;
  
  lba = convert32(m_start_lba);
  LBAtoCHS(lba, &cyl, &head, &sect);
  
  m_begin_cyl.Format("0x%02X", cyl);
  m_begin_head.Format("0x%02X", head);
  m_begin_sect.Format("0x%02X", sect);
  
  OnTabItemChange();
  UpdateData(FALSE);  // send to Dialog
}

void CMbrEntry::OnMbrEndUpdate() {
  UpdateData(TRUE);  // bring from Dialog
  DWORD lba;
  BYTE head, sect;
  WORD cyl;
  
  lba = convert32(m_start_lba);
  lba += convert32(m_size);
  LBAtoCHS(lba, &cyl, &head, &sect);
  
  m_end_cyl.Format("0x%02X", cyl);
  m_end_head.Format("0x%02X", head);
  m_end_sect.Format("0x%02X", sect);
  
  OnTabItemChange();
  UpdateData(FALSE);  // send to Dialog
}

void CMbrEntry::OnMbrApply() {
  struct MBR_PART_ENTRY *entry;
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CMbr *mbr = (CMbr *) m_parent;
  
  UpdateData(TRUE);  // bring from Dialog
  
  entry = (struct MBR_PART_ENTRY *) (&mbr->m_buffer[512-2-(4*16)] + (16 * m_index));
  entry->boot_id = convert8(m_boot_id);
  entry->start.cyl = convert8(m_begin_cyl);
  entry->start.head = convert8(m_begin_head);
  entry->start.sector = convert8(m_begin_sect);
  entry->sys_id = convert8(m_sys_id);
  entry->end.cyl = convert8(m_end_cyl);
  entry->end.head = convert8(m_end_head);
  entry->end.sector = convert8(m_end_sect);
  
  entry->start_lba = convert32(m_start_lba);
  entry->sectors = convert32(m_size);
  
  // Add the '*' to the tab title
  CPropertySheet *Sheet = (CPropertySheet *) GetParent();
  TC_ITEM ti;
  char szText[64];
  strcpy(szText, m_Title);
  ti.mask = TCIF_TEXT;
  ti.pszText = szText;
  Sheet->GetTabControl()->SetItem(m_index, &ti);
  m_dirty = FALSE;
}

void CMbrEntry::OnTabItemChangeSize() {
  CString cs;
  
  OnTabItemChange();
  
  GetDlgItemText(IDC_MBR_SIZE, cs);
  DWORD size = convert32(cs);
  if (size == 0) {
    SetDlgItemText(IDC_MBR_SYS_ID, "0x00");
    OnChangeMbrSysId();
    GetDlgItem(IDC_SYS_ID)->EnableWindow(FALSE);
    GetDlgItem(IDC_MBR_SYS_ID)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_SYS_ID)->EnableWindow(TRUE);
    GetDlgItem(IDC_MBR_SYS_ID)->EnableWindow(TRUE);
  }
}

void CMbrEntry::OnTabItemChange() {
  if (!m_dirty) {
    CPropertySheet *Sheet = (CPropertySheet *) GetParent();
    TC_ITEM ti;
    char szText[64];
    
    ti.mask = TCIF_TEXT;
    ti.pszText = szText;
    ti.cchTextMax = 64;
    Sheet->GetTabControl()->GetItem(m_index, &ti);
    strcat(szText, "*");
    Sheet->GetTabControl()->SetItem(m_index, &ti);
    m_dirty = TRUE;
  }
}

void CMbrEntry::OnChangeMbrSysId() {
  UpdateData(TRUE);  // bring from Dialog
  
  int i = convert8(m_sys_id);
  SetDlgItemText(IDC_SYS_STR, fdisk_ids[(i & 0xFF)]);	
  
  OnTabItemChange();
}

BOOL CMbrEntry::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  CString cs;
  DWORD size = convert32(m_size);
  if (size > 0) {
    GetDlgItem(IDC_SYS_ID)->EnableWindow(TRUE);
    GetDlgItem(IDC_MBR_SYS_ID)->EnableWindow(TRUE);
  }
  
  int i = convert8(m_sys_id);
  SetDlgItemText(IDC_SYS_STR, fdisk_ids[(i & 0xFF)]);	
  m_dirty = FALSE;
  
  return TRUE;
}

// Sector   = (LBA mod SPT) + 1       (SPT = Sectors per Track)
// Head     = (LBA  /  SPT) mod Heads
// Cylinder = (LBA  /  SPT)  /  Heads
void CMbrEntry::LBAtoCHS(const DWORD lba, WORD *cyl, BYTE *head, BYTE *sect) {
  CMbr *mbr = (CMbr *) m_parent;
  DWORD heads, spt;
  
  mbr->UpdateData(TRUE); // bring from Dialog
  
  spt = convert32(mbr->m_spt);
  heads = convert32(mbr->m_head_count);
  gLBAtoCHS(lba, cyl, head, sect, spt, heads);
}

void CMbrEntry::OnSysId() {
  CAttribute dlg;
  CString cs;
  int i, j = 0;
  
  // we create a list from fdisk_ids[] above
  struct S_ATTRIBUTES *mbr_entry_attrbs = (struct S_ATTRIBUTES *) malloc((256 + 1) * sizeof(struct S_ATTRIBUTES));
  for (i=0; i<256; i++) {
    if (strcmp(fdisk_ids[i], "*unknown*") != 0) {
      mbr_entry_attrbs[j].attrb = i;
      mbr_entry_attrbs[j].mask = (DWORD) -1;  // must equal the m_attrb value
      mbr_entry_attrbs[j].index = j;
      strcpy(mbr_entry_attrbs[j].str, fdisk_ids[i]);
      mbr_entry_attrbs[j].groups[0] = -1;
      j++;
    }
  }
  // mark the last one
  mbr_entry_attrbs[j].index = -1;
  
  GetDlgItemText(IDC_MBR_SYS_ID, cs);
  dlg.m_title = "MBR Sys ID";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = mbr_entry_attrbs;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%02X", dlg.m_attrib);
    SetDlgItemText(IDC_MBR_SYS_ID, cs);
    SetDlgItemText(IDC_SYS_STR, fdisk_ids[(dlg.m_attrib & 0xFF)]);	
    OnTabItemChange();
  }
  
  free(mbr_entry_attrbs);
}
