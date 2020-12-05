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

// Fat.cpp : implementation file

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#include "Fat.h"
#include "FatEntry.h"
#include "Fat32Info.h"
#include "FatFormat.h"

#include "Modeless.h"
#include "InsertVName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFat property page

IMPLEMENT_DYNCREATE(CFat, CPropertyPage)

CFat::CFat() : CPropertyPage(CFat::IDD) {
  //{{AFX_DATA_INIT(CFat)
  m_jmp0 = _T("");
  m_jmp1 = _T("");
  m_jmp2 = _T("");
  m_bytes_sect = _T("");
  m_descriptor = _T("");
  m_fat_type = _T("");
  m_fats = _T("");
  m_heads = _T("");
  m_hidden_sects = _T("");
  m_label = _T("");
  m_oem_name = _T("");
  m_clust_num = _T("");
  m_root_entries = _T("");
  m_sects_cluster = _T("");
  m_sect_fat = _T("");
  m_sect_fat32 = _T("");
  m_sect_track = _T("");
  m_sectors = _T("");
  m_sectors_ext = _T("");
  m_fs_version = _T("");
  m_backup_sector = _T("");
  m_drv_num = _T("");
  m_info_sector = _T("");
  m_reserved = _T("");
  m_sect_reserved = _T("");
  m_serial_number = _T("");
  m_fat_sig = _T("");
  m_show_del = FALSE;
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_fat_buffer = NULL;
  m_bpb_buffer = NULL;
  m_hard_format = FALSE;
}

CFat::~CFat() {
  if (m_fat_buffer)
    free(m_fat_buffer);
  if (m_bpb_buffer)
    free(m_bpb_buffer);
  
  m_fat_buffer = NULL;
  m_bpb_buffer = NULL;
}

void CFat::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFat)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_FAT_JMP0, m_jmp0);
  DDV_MaxChars(pDX, m_jmp0, 4);
  DDX_Text(pDX, IDC_FAT_JMP1, m_jmp1);
  DDV_MaxChars(pDX, m_jmp1, 4);
  DDX_Text(pDX, IDC_FAT_JMP2, m_jmp2);
  DDV_MaxChars(pDX, m_jmp2, 4);
  DDX_Text(pDX, IDC_FAT_BYTES_SECT, m_bytes_sect);
  DDV_MaxChars(pDX, m_bytes_sect, 16);
  DDX_Text(pDX, IDC_FAT_DESCRIPTOR, m_descriptor);
  DDV_MaxChars(pDX, m_descriptor, 8);
  DDX_Text(pDX, IDC_FAT_FAT_TYPE, m_fat_type);
  DDV_MaxChars(pDX, m_fat_type, 8);
  DDX_Text(pDX, IDC_FAT_FATS, m_fats);
  DDV_MaxChars(pDX, m_fats, 8);
  DDX_Text(pDX, IDC_FAT_HEADS, m_heads);
  DDV_MaxChars(pDX, m_heads, 8);
  DDX_Text(pDX, IDC_FAT_HIDDEN_SECTS, m_hidden_sects);
  DDV_MaxChars(pDX, m_hidden_sects, 16);
  DDX_Text(pDX, IDC_FAT_LABEL, m_label);
  DDV_MaxChars(pDX, m_label, 16);
  DDX_Text(pDX, IDC_FAT_OEM_NAME, m_oem_name);
  DDV_MaxChars(pDX, m_oem_name, 16);
  DDX_Text(pDX, IDC_FAT_CLUST_NUM, m_clust_num);
  DDV_MaxChars(pDX, m_clust_num, 16);
  DDX_Text(pDX, IDC_FAT_ROOT_ENTRIES, m_root_entries);
  DDV_MaxChars(pDX, m_root_entries, 16);
  DDX_Text(pDX, IDC_FAT_SECT_CLUSTER, m_sects_cluster);
  DDV_MaxChars(pDX, m_sects_cluster, 16);
  DDX_Text(pDX, IDC_FAT_SECT_FAT, m_sect_fat);
  DDV_MaxChars(pDX, m_sect_fat, 16);
  DDX_Text(pDX, IDC_FAT_SECT_FAT32, m_sect_fat32);
  DDV_MaxChars(pDX, m_sect_fat32, 32);
  DDX_Text(pDX, IDC_FAT_EXT_FLAGS, m_ext_flags);
  DDV_MaxChars(pDX, m_ext_flags, 32);
  DDX_Text(pDX, IDC_FAT_SECT_TRACK, m_sect_track);
  DDV_MaxChars(pDX, m_sect_track, 8);
  DDX_Text(pDX, IDC_FAT_SECTORS, m_sectors);
  DDV_MaxChars(pDX, m_sectors, 16);
  DDX_Text(pDX, IDC_FAT_SECTORS_EXT, m_sectors_ext);
  DDV_MaxChars(pDX, m_sectors_ext, 16);
  DDX_Text(pDX, IDC_FAT_FS_VERSION, m_fs_version);
  DDV_MaxChars(pDX, m_fs_version, 8);
  DDX_Text(pDX, IDC_FAT_BACKUP_SECT, m_backup_sector);
  DDV_MaxChars(pDX, m_backup_sector, 32);
  DDX_Text(pDX, IDC_FAT_DRV_NUM, m_drv_num);
  DDV_MaxChars(pDX, m_drv_num, 8);
  DDX_Text(pDX, IDC_FAT_INFO_SECTOR, m_info_sector);
  DDV_MaxChars(pDX, m_info_sector, 16);
  DDX_Text(pDX, IDC_FAT_RESERVED, m_reserved);
  DDV_MaxChars(pDX, m_reserved, 8);
  DDX_Text(pDX, IDC_FAT_SECT_RESERVED, m_sect_reserved);
  DDV_MaxChars(pDX, m_sect_reserved, 32);
  DDX_Text(pDX, IDC_FAT_SERIAL_NUM, m_serial_number);
  DDV_MaxChars(pDX, m_serial_number, 32);
  DDX_Text(pDX, IDC_FAT_SIG, m_fat_sig);
  DDV_MaxChars(pDX, m_fat_sig, 8);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  DDX_Check(pDX, IDC_DEL_CLEAR, m_del_clear);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFat, CPropertyPage)
  //{{AFX_MSG_MAP(CFat)
  ON_BN_CLICKED(ID_APPLY, OnFatApply)
  ON_BN_CLICKED(ID_CLEAN, OnFatClean)
  ON_BN_CLICKED(ID_FORMAT, OnFatFormat)
  ON_BN_CLICKED(ID_CHECK, OnFatCheck)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_COPY, OnFatCopy)
  ON_BN_CLICKED(IDC_INSERT_VLABEL, OnInsertVLabel)
  ON_BN_CLICKED(ID_INSERT, OnFatInsert)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_ENTRY, OnFatEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(ID_FAT32_INFO, OnFat32Info)
  ON_EN_CHANGE(IDC_FAT_FS_VERSION, OnChangeFatFsVersion)
  ON_BN_CLICKED(IDC_SERIAL_UPDATE, OnSerialUpdate)
  ON_BN_CLICKED(IDC_FAT_BACKUP_SECT_UPDATE, OnFatBackupSectUpdate)
  ON_BN_CLICKED(IDC_FAT_BACKUP_SECT_RESTORE, OnFatBackupSectRestore)
  ON_BN_CLICKED(IDC_OLD_FAT, OnOldFat)
  ON_BN_CLICKED(ID_DELETE, OnFatDelete)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFat message handlers
BOOL CFat::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  OnChangeFatFsVersion();
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  if (m_fat_size == FS_FAT12)
    GetDlgItem(IDC_OLD_FAT)->EnableWindow(TRUE);
  
  m_show_del = AfxGetApp()->GetProfileInt("Settings", "FATShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "FATDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  return TRUE;
}

BOOL CFat::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "fat.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CFat::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(hItem != NULL);
  
  *pResult = 0;
}

void CFat::OnFatApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  ReceiveFromDialog(m_bpb_buffer); // bring from Dialog
  
  // update the BPB
  dlg->ReadFromFile(buffer, m_lba, 1, FALSE);
  if (m_fat_size == FS_FAT32)
    memcpy(buffer, m_bpb_buffer, sizeof(struct S_FAT32_BPB));
  else
    memcpy(buffer, m_bpb_buffer, sizeof(struct S_FAT1216_BPB));
  dlg->WriteToFile(buffer, m_lba, 1, FALSE);
  
  // need to write the FAT back to the image file
  if (AfxMessageBox("Update the FAT(s)?", MB_YESNO, NULL) == IDYES)
    UpdateTheFATs();
  
  // TODO: write stuff from items->Entry[0] to items->Entry[sfn_index] to disk.
  
}

// clear out the root, fat, and data area, leaving the BPB as is
void CFat::OnFatClean() {
  int r = AfxMessageBox("This will erase the volume, leaving the BPB as is.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  if (FatFormat(FALSE))
    Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
}

void CFat::OnFatFormat() {
  int r = AfxMessageBox("This will erase the volume, using most of the current BPB values.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  if (FatFormat(r == IDYES))
    Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
}

// returns TRUE is successful format
bool CFat::FatFormat(const BOOL AskForBoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile bsfile;
  BYTE *buffer;
  size_t reserved = 1;
  int i, j, sect_per_fat;
  DWORD64 l;
  bool has_boot_sector = FALSE;
  unsigned info_sect, backup_sect;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  if (!m_hard_format)
    ReceiveFromDialog(m_bpb_buffer); // bring from Dialog
  
  // first, clear out the volume
  buffer = (BYTE *) calloc(MAX_SECT_SIZE, 1);
  for (l=0; l<m_size; l++)
    dlg->WriteToFile(buffer, m_lba + l, 1, FALSE);
  
  // did we request a boot sector?
  if (AskForBoot) {
    CFileDialog odlg(
      TRUE,             // Create an open file dialog
      _T(".bin"),       // Default file extension
      NULL,             // Default Filename
      OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
      _T(".bin files (.bin)|*.bin|")    // Filter string
      _T(".img files (.img)|*.img|")    // Filter string
      _T("All Files (*.*)|*.*|")        // Filter string
      _T("|")
    );
    odlg.m_ofn.lpstrTitle = "FAT Boot Sector File";
    odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
    if (odlg.DoModal() == IDOK) {
      POSITION pos = odlg.GetStartPosition();
      if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
        size_t filesize = (size_t) bsfile.GetLength();
        if (filesize <= 8192) {
          reserved = (filesize + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
          if (filesize > MAX_SECT_SIZE) // must check manually so we don't "shrink" the buffer with realloc()
            buffer = (BYTE *) realloc(buffer, filesize);
          bsfile.Read(buffer, (UINT) filesize);
          dlg->WriteToFile(buffer, m_lba, (UINT) reserved, FALSE);
          has_boot_sector = TRUE;
        } else
          AfxMessageBox("Boot sector must be <= 8192 bytes");
        bsfile.Close();
      }
    }
  }
  
  // restore our bpb info and get the specs to format the volume with
  CFatFormat format;
  format.m_fat_size = m_fat_size;
  format.m_sectors = m_size - reserved;  // we should also subtract fat sectors (and root sectors (if < fat23)), but how do we know until after the calculation
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB *bpb = (struct S_FAT32_BPB *) buffer;
    if (!has_boot_sector)
      bpb->sect_reserved = 32;
    reserved = bpb->sect_reserved;
    format.m_num_fats = (bpb->fats > 0) ? bpb->fats : 2;
    format.m_root_entries = 16;  // in sectors
    format.m_sect_cluster = (bpb->sect_per_clust > 0) ? bpb->sect_per_clust : 2;
    if (format.DoModal() != IDOK) {
      free(buffer);
      return FALSE;
    }

    // check that the fat size is compatible with the cluster count
    // Microsoft's EFI FAT32 specification states that any FAT file system with less than 4085 clusters is FAT12,
    // any FAT file system with less than 65525 clusters is FAT16, and otherwise it is FAT32. 
    const int clusters = (int) (format.m_sectors / format.m_sect_cluster);
    if (clusters < 65525) {
      CString csTemp;
      csTemp.Format(" Incompatible FAT size found for count of clusters.\n"
                    " FAT32 must have a cluster count of at least 65525.\n"
                    " Current cluster count = %i\n"
                    " Continue with format?", clusters);
      if (AfxMessageBox(csTemp, MB_YESNO) != IDYES) {
        free(buffer);
        return FALSE;
      }
    }

    memcpy(bpb->oem_name, "OEM_NAME", 8);
    memcpy(bpb->label, "NO LABEL   ", 11);
    bpb->fats = format.m_num_fats;
    bpb->descriptor = 0xF8;
    bpb->sig = 0x29; // 0x29 means the VolID, VolLab, and FilSysType members are valid
    bpb->root_entrys = 0;
    bpb->root_base_cluster = 2;
    bpb->sect_per_clust = format.m_sect_cluster;
    sect_per_fat = bpb->sect_per_fat32 = CalcSectPerFat(m_size - reserved, format.m_sect_cluster, dlg->m_sect_size, FS_FAT32);
    if (bpb->jmp[0] == 0) { bpb->jmp[0] = 0xEB;  bpb->jmp[1] = 0x3C; bpb->jmp[2] = 0x90; }
    bpb->bytes_per_sect = dlg->m_sect_size;
    bpb->sectors = 0; // for FAT32 volumes, the 16-bit sectors entry must be zero
    bpb->sect_extnd = (DWORD) m_size;
    bpb->fs_info_sec = info_sect = 1;
    bpb->backup_boot_sec = backup_sect = 6;
    memcpy(bpb->sys_type, "FAT32   ", 8);
    bpb->hidden_sects = (DWORD) m_lba;
  } else {
    struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) buffer;
    bpb->sect_reserved = (WORD) reserved;
    format.m_num_fats = (bpb->fats > 0) ? bpb->fats : 2;
    format.m_root_entries = (bpb->root_entrys > 0) ? (((bpb->root_entrys * 32) + (dlg->m_sect_size - 1)) / dlg->m_sect_size) : 16;  // in sectors
    format.m_sect_cluster = (bpb->sect_per_clust > 0) ? bpb->sect_per_clust : 2;
    if (format.DoModal() != IDOK) {
      free(buffer);
      return FALSE;
    }

    // check that the fat size is compatible with the cluster count
    // Microsoft's EFI FAT32 specification states that any FAT file system with less than 4085 clusters is FAT12,
    // any FAT file system with less than 65525 clusters is FAT16, and otherwise it is FAT32. 
    const int clusters = (int) (format.m_sectors / format.m_sect_cluster);
    if (m_fat_size == FS_FAT16) {
      if ((clusters < 4085) || (clusters >= 65525)) {
        CString csTemp;
        csTemp.Format(" Incompatible FAT size found for count of clusters.\n"
                      " FAT16 must have a cluster count of at least 4085 and less than 65525.\n"
                      " Current cluster count = %i\n"
                      " Continue with format?", clusters);
        if (AfxMessageBox(csTemp, MB_YESNO) != IDYES) {
          free(buffer);
          return FALSE;
        }
      }
    } else {
      if (clusters >= 4085) {
        CString csTemp;
        csTemp.Format(" Incompatible FAT size found for count of clusters.\n"
                      " FAT12 must have a cluster count less than 4085.\n"
                      " Current cluster count = %i\n"
                      " Continue with format?", clusters);
        if (AfxMessageBox(csTemp, MB_YESNO) != IDYES) {
          free(buffer);
          return FALSE;
        }
      }
    }

    memcpy(bpb->oem_name, "OEM_NAME", 8);
    memcpy(bpb->label, "NO LABEL   ", 11);
    bpb->fats = format.m_num_fats;
    if (m_size <= 2880)
      bpb->descriptor = 0xF0;  // if it is a floppy, we need descriptor to be 0xF0
    else
      bpb->descriptor = 0xF8;  // else, assume it is a hard drive and use 0xF8
    bpb->root_entrys = (format.m_root_entries * dlg->m_sect_size) / 32;
    bpb->sect_per_clust = format.m_sect_cluster;
    sect_per_fat = bpb->sect_per_fat = CalcSectPerFat(m_size - reserved - format.m_root_entries, format.m_sect_cluster, dlg->m_sect_size, m_fat_size);
    if (bpb->jmp[0] == 0) { bpb->jmp[0] = 0xEB;  bpb->jmp[1] = 0x3C; bpb->jmp[2] = 0x90; }
    bpb->bytes_per_sect = dlg->m_sect_size;
    bpb->sig = 0x29; // 0x29 means the VolID, VolLab, and FilSysType members are valid
    if (m_size <= 0xFFFF) {
      bpb->sectors = (WORD) m_size;
      bpb->sect_extnd = 0;
    } else {
      bpb->sectors = 0;
      bpb->sect_extnd = (DWORD) m_size;
    }
    if (m_fat_size == FS_FAT16)
      memcpy(bpb->sys_type, "FAT16   ", 8);
    else
      memcpy(bpb->sys_type, "FAT12   ", 8);
    bpb->hidden_sects = (DWORD) m_lba;
  }
  
  // write the bpb back to the volume
  // if a bootsector was loaded, the first 512 bytes will still contain that code
  // however, lets update the bpb. 
  //   (skipping the jump incase the new code jumps differently than normal ???)
  dlg->WriteToFile(buffer, m_lba, 1, FALSE);
  
  // now the fat(s)
  for (j=0; j<format.m_num_fats; j++) {
    memset(buffer, 0, MAX_SECT_SIZE);
    DWORD *dword = (DWORD *) buffer;
    switch (m_fat_size) {
      case FS_FAT12:
        dword[0] = 0x00FFFFF8;
        break;
      case FS_FAT16:
        dword[0] = 0xFFFFFFF8;
        break;
      case FS_FAT32:
        dword[0] = 0x0FFFFFF8;
        dword[1] = 0x0FFFFFFF;
        for (i=0; i<(format.m_root_entries / format.m_sect_cluster); i++)
          dword[2+i] = 3 + i;
        dword[2+i-1] = 0x0FFFFFFF;
        break;
    }
    dlg->WriteToFile(buffer, m_lba + reserved, 1, FALSE);
    memset(buffer, 0, MAX_SECT_SIZE);
    for (i=1; i<sect_per_fat; i++)
      dlg->WriteToFile(buffer, m_lba + reserved + i, 1, FALSE);
  }

  // Now the root.  It is written to just after the last fat
  //  no matter the fat type.
  l = reserved + (format.m_num_fats * sect_per_fat);
  memset(buffer, 0, MAX_SECT_SIZE);
  for (i=0; i<format.m_root_entries; i++)  // format.m_root_entries is in sectors
    dlg->WriteToFile(buffer, m_lba + l, 1, FALSE);

  // if a FAT32, we need an info sector and a backup sector
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB_INFO *info = (struct S_FAT32_BPB_INFO *) buffer;
    memset(info, 0, dlg->m_sect_size);
    info->sig = 0x41615252;
    info->struct_sig = 0x61417272;
    info->free_clust_cnt = -1;
    info->next_free_clust = (format.m_root_entries / format.m_sect_cluster) + 1;
    info->trail_sig = 0xAA550000;
    dlg->WriteToFile(buffer, m_lba + info_sect, 1, FALSE);
    
    // backup_sect = convert16(m_backup_sector);
  }

  free(buffer);
  
  if (!m_hard_format)
    SendToDialog(m_bpb_buffer); // Send back to Dialog

  return TRUE;
}

void CFat::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_bpb_buffer;
  struct S_FAT1216_BPB *bpb12 = (struct S_FAT1216_BPB *) m_bpb_buffer;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  CString cs;
  unsigned extra = 0;
  unsigned reserved = (bpb12->resv > 0) ? bpb12->sect_reserved : 1;
  unsigned i, info_sect, backup_sect;

  BYTE *existing = (BYTE *) malloc(reserved * dlg->m_sect_size);
  BYTE *buffer = (BYTE *) calloc(reserved * dlg->m_sect_size, 1);

  UpdateData(TRUE); // receive from Dialog
  info_sect = convert16(m_info_sector);
  backup_sect = convert16(m_backup_sector);

  // first, read in what we already have
  dlg->ReadFromFile(existing, m_lba, reserved, FALSE);
  
  // save the FYSOS signature block incase we restore it below
  memcpy(&s_sig, existing + S_FYSOSSIG_OFFSET, sizeof(struct S_FYSOSSIG));
  
  CFileDialog odlg(
    TRUE,             // Create an open file dialog
    _T(".bin"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T(".img files (.img)|*.img|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  odlg.m_ofn.lpstrTitle = "FAT Boot Sector File";
  odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
  if (odlg.DoModal() == IDOK) {
    POSITION pos = odlg.GetStartPosition();
    if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
      size_t filesize = (size_t) bsfile.GetLength();
      if (filesize > (reserved * dlg->m_sect_size)) {
        cs.Format("New boot sector = %i bytes.  'Reserved' allows %i bytes. Expand boot sectors and 'reserved'?.", filesize, (reserved * dlg->m_sect_size));
        if (AfxMessageBox(cs, MB_YESNO) == IDYES) {
          extra = (unsigned) ((filesize + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
          buffer = (BYTE *) realloc(buffer, extra * dlg->m_sect_size);
        } else
          filesize = (reserved * dlg->m_sect_size);
      }
      bsfile.Read(buffer, (UINT) filesize);
      bsfile.Close();
    }
    
    // copy the count of sectors from the new code to our buffer, skipping
    //  the specific parts of the BPB
    memcpy(existing, buffer, 3);  // the jump instruction
    if (m_fat_size == FS_FAT32) {
      // the first sector then the remaining sectors, skipping the Info Sector and Backup Sector
      memcpy(existing + sizeof(struct S_FAT32_BPB), buffer + sizeof(struct S_FAT32_BPB), dlg->m_sect_size - sizeof(struct S_FAT32_BPB));
      for (i=1; i<reserved; i++) {
        if ((i == info_sect) || (i == backup_sect))
          continue;
        memcpy(existing + (i * dlg->m_sect_size), buffer + (i * dlg->m_sect_size), dlg->m_sect_size);
      }
    } else
      memcpy(existing + sizeof(struct S_FAT1216_BPB), buffer + sizeof(struct S_FAT1216_BPB), (reserved * dlg->m_sect_size) - sizeof(struct S_FAT1216_BPB));
    
    // restore the FYSOS sig?
    if (AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE))
      memcpy(existing + S_FYSOSSIG_OFFSET, &s_sig, sizeof(struct S_FYSOSSIG));
    
    // update the reserved member?
    if (extra > 0) {
      ((struct S_FAT1216_BPB *) existing)->sect_reserved = extra;
      m_sect_reserved.Format("%i", extra);
      SetDlgItemText(IDC_FAT_SECT_RESERVED, m_sect_reserved);
    }

    // write it back
    dlg->WriteToFile(existing, m_lba, reserved, FALSE);

    // was there any extra?
    if (extra > 0)
      dlg->WriteToFile(buffer + (reserved * dlg->m_sect_size), m_lba + reserved, extra - reserved, FALSE);
  }
  
  free(buffer);
  free(existing);
}

// Fat colors will have a blue shade to them.
DWORD CFat::GetNewColor(int index) {
  int r = ((106 - (index * 20)) > -1) ? (106 - (index * 20)) : 0;
  int g = ((126 - (index * 18)) > -1) ? (126 - (index * 18)) : 0;
  int b = ((239 - (index *  2)) > -1) ? (239 - (index *  2)) : 0;
  return RGB(r, g, b);
}

void CFat::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, const int fs_type, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD rootcluster;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  m_hard_format = FALSE;
  
  m_bpb_buffer = malloc(MAX_SECT_SIZE);
  dlg->ReadFromFile(m_bpb_buffer, lba, 1, FALSE);
  struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_bpb_buffer;
  struct S_FAT1216_BPB *bpb12 = (struct S_FAT1216_BPB *) m_bpb_buffer;
  
  if (dlg->m_sect_size != bpb12->bytes_per_sect) {
    AfxMessageBox("FAT:\r\n"
                  "BPB Bytes per Sector doesn't equal specified bytes per sector.\r\n"
                  "Using BPB Bytes per Sector for this volume.");
  }
  
  m_fat_size = fs_type;
  if (fs_type == FS_FAT12)
    dlg->m_FatNames[index] = "FAT 12";
  else if (fs_type == FS_FAT16)
    dlg->m_FatNames[index] = "FAT 16";
  else
    dlg->m_FatNames[index] = "FAT 32";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_FatNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->Fat[index].m_draw_index, (char *) (LPCTSTR) dlg->m_FatNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  // if not FAT32, disable the FAT32 specific items
  if (fs_type != FS_FAT32) {
    GetDlgItem(IDC_FAT_SECT_FAT32)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_EXT_FLAGS)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_FS_VERSION)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_CLUST_NUM)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_INFO_SECTOR)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_BACKUP_SECT)->EnableWindow(FALSE);
    GetDlgItem(IDC_FAT_BACKUP_SECT_UPDATE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_FAT_BACKUP_SECT_RESTORE)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_FAT_FS_VERSION_DISP)->ShowWindow(SW_HIDE);
    GetDlgItem(ID_FAT32_INFO)->ShowWindow(SW_HIDE);
  }
  SendToDialog(m_bpb_buffer);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  // load the fat
  if (m_isvalid) {
    m_fat_buffer = FatLoadFAT(m_fat_buffer);
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);

    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    struct S_FAT_ROOT *root = NULL;
    if (m_fat_size == FS_FAT32) {
      rootcluster = bpb32->root_base_cluster;
      m_datastart = bpb32->sect_reserved + (bpb32->fats * bpb32->sect_per_fat32);
      if (rootcluster > 0)  // make sure there is actually a root to read
        root = (struct S_FAT_ROOT *) ReadFile(rootcluster, &m_rootsize, TRUE);
    } else {
      rootcluster = 0;
      m_rootsize = bpb12->root_entrys * sizeof(struct S_FAT_ROOT);
      m_rootstart = bpb12->sect_reserved + (bpb12->fats * bpb12->sect_per_fat);
      root = (struct S_FAT_ROOT *) ReadFile(rootcluster, &m_rootsize, TRUE);
      m_datastart = m_rootstart + (((bpb12->root_entrys * 32) + (bpb12->bytes_per_sect - 1)) / bpb12->bytes_per_sect);
    }
    if (root) {
      SaveItemInfo(m_hRoot, rootcluster, m_rootsize, NULL, 0, 0, 0, FALSE);
      CWaitCursor wait; // display a wait cursor
      m_too_many = FALSE;
      m_parse_depth_limit = FALSE;
      ParseDir(root, m_rootsize / sizeof(struct S_FAT_ROOT), m_hRoot, TRUE);
      m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
      free(root);
      wait.Restore();
      GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
      GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
      GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
      
      // select the root and set the focus to that root
      GetDlgItem(IDC_DIR_TREE)->SetFocus();
      m_dir_tree.SelectSetFirstVisible(m_hRoot);
    }
  }
  Invalidate(TRUE);  // redraw the tab
}

bool CFat::ParseDir(struct S_FAT_ROOT *root, const unsigned entries, HTREEITEM parent, BOOL IsRoot) {
  struct S_FAT_ROOT *sub;
  HTREEITEM hItem;
  unsigned cnt, i = 0;
  CString name;
  DWORD start, filesize = 0, ErrorCode;
  BYTE attrb;
  BOOL IsDot = FALSE;
  unsigned ErrorCount = 0;
  unsigned ErrorMax = AfxGetApp()->GetProfileInt("Settings", "MaxErrorCount", 10);

  // catch to make sure we don't simply repeatedly recurse on '.' and '..' entries
  if (++m_parse_depth_limit > MAX_DIR_PARSE_DEPTH) {
    AfxMessageBox("Recursive ParseDir() too deep.  Stopping");
    return FALSE;
  }
  
  while ((i<entries) && !m_too_many) {
    if (root[i].name[0] == 0x00)
      break;
    ErrorCode = CheckRootEntry(&root[i]);
    if (ErrorCode == FAT_BAD_LFN_DEL)
      cnt = FatGetName(&root[i], name, &attrb, &start, &filesize, NULL) - 1; // -1 so we display the deleted SFN ????
    else if (ErrorCode != FAT_NO_ERROR) {
      hItem = m_dir_tree.Insert("Invalid Entry", ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
      if (hItem == NULL) { m_too_many = TRUE; return TRUE; }
      SaveItemInfo(hItem, 0, filesize, &root[i], i, 1, ErrorCode, FALSE);
      cnt = 1;
      if (++ErrorCount >= ErrorMax)
        break;
    } else {
      // retrieve the name.
      cnt = FatGetName(&root[i], name, &attrb, &start, &filesize, &IsDot);
      if (root[i].name[0] == FAT_DELETED_CHAR) {
        if (IsDlgButtonChecked(IDC_SHOW_DEL)) {
          name = "(deleted entry)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return TRUE; }
          SaveItemInfo(hItem, 0, filesize, &root[i], i, cnt, 0, FALSE);
        }
      } else {
        if (attrb & FAT_ATTR_SUB_DIR) {
          if (attrb & FAT_ATTR_HIDDEN)
            hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER_HIDDEN, IMAGE_FOLDER_HIDDEN, parent);
          else
            hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
          if (hItem == NULL) { m_too_many = TRUE; return TRUE; }
          SaveItemInfo(hItem, start, filesize, &root[i], i, cnt, 0, !IsDot);
          if (!IsDot) {
            sub = (struct S_FAT_ROOT *) ReadFile(start, &filesize, FALSE);
            if (sub) {
              if (!ParseDir(sub, (filesize + 31) / 32, hItem, FALSE)) {
                free(sub);
                return FALSE;
              }
              free(sub);
            }
          }
        } else if (attrb & FAT_ATTR_VOLUME) {
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_LABEL, IMAGE_LABEL, parent);
          if (hItem == NULL) { m_too_many = TRUE; return TRUE; }
          SaveItemInfo(hItem, 0, filesize, &root[i], i, cnt, 0, FALSE);
        } else {
          if (attrb & FAT_ATTR_HIDDEN)
            hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE_HIDDEN, IMAGE_FILE_HIDDEN, parent);
          else
            hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return TRUE; }
          SaveItemInfo(hItem, start, filesize, &root[i], i, cnt, 0, TRUE);
        }
      }
    }
    i += cnt;
  }

  return TRUE;
}

void *CFat::ReadFile(DWORD cluster, DWORD *size, BOOL IsRoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *ptr = NULL;
  unsigned pos = 0, mem_size = 0;
  
  if (IsRoot && (m_fat_size != FS_FAT32)) {
    ptr = calloc(*size + (MAX_SECT_SIZE - 1), 1);
    if (ptr)
      dlg->ReadFromFile(ptr, m_lba + m_rootstart, (*size + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect, FALSE);
  } else {
    struct S_FAT_ENTRIES ClusterList;
    FatFillClusterList(&ClusterList, cluster);
    for (int i=0; i<ClusterList.entry_count; i++) {
      if (mem_size < (pos + (bpb->bytes_per_sect * bpb->sect_per_clust))) {
        mem_size += (bpb->bytes_per_sect * bpb->sect_per_clust);
        ptr = realloc(ptr, mem_size);
      }
      dlg->ReadFromFile((BYTE *) ptr + pos, m_lba + m_datastart + ((ClusterList.entries[i] - 2) * bpb->sect_per_clust), bpb->sect_per_clust, FALSE);
      pos += (bpb->bytes_per_sect * bpb->sect_per_clust);
    }
    free(ClusterList.entries);
    if (size) *size = pos;
  }
  
  return ptr;
}

// if IsRoot:
//  buffer holds root directory
//  size is size in bytes to write
//  ClusterList is ignored
// If not IsRoot:
//  buffer holds file data to write
//  size is size in bytes to write
//  ClusterList holds the list of clusters to use
void CFat::WriteFile(void *buffer, struct S_FAT_ENTRIES *ClusterList, DWORD size, BOOL IsRoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  unsigned pos = 0;
  
  if (IsRoot && (m_fat_size != FS_FAT32)) {
    dlg->WriteToFile(buffer, m_lba + m_rootstart, (size + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect, FALSE);
  } else {
    for (int i=0; i<ClusterList->entry_count; i++) {
      dlg->WriteToFile((BYTE *) buffer + pos, m_lba + m_datastart + ((ClusterList->entries[i] - 2) * bpb->sect_per_clust), bpb->sect_per_clust, FALSE);
      pos += (bpb->bytes_per_sect * bpb->sect_per_clust);
    }
  }
}

// write zeros to this cluster
void CFat::ZeroCluster(DWORD Cluster) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *zero = (void *) calloc((size_t) bpb->sect_per_clust * (size_t) bpb->bytes_per_sect, 1);

  dlg->WriteToFile((BYTE *) zero, m_lba + m_datastart + ((Cluster - 2) * (size_t) bpb->sect_per_clust), bpb->sect_per_clust, FALSE);
  
  free(zero);
}

// checks a ROOT entry
DWORD CFat::CheckRootEntry(struct S_FAT_ROOT *root) {
  unsigned i, sfn = 0;
  
  // first check the attribute value.
  // should be one of/a combination of
  //   FAT_ATTR_ARCHIVE   0x20
  //   FAT_ATTR_SUB_DIR   0x10
  //   FAT_ATTR_VOLUME    0x08
  //   FAT_ATTR_SYSTEM    0x04
  //   FAT_ATTR_HIDDEN    0x02
  //   FAT_ATTR_READ_ONLY 0x01
  if (root->attrb & ~FAT_ATTR_ALL)
    return FAT_BAD_ATTRIBUTE;
  if (root->attrb != FAT_ATTR_LONG_FILE) {
    // if sub and any other bits are set, error (except for Hidden?)
    if ((root->attrb & FAT_ATTR_SUB_DIR) && (root->attrb & ~(FAT_ATTR_SUB_DIR | FAT_ATTR_HIDDEN)))
      return FAT_BAD_ATTRIBUTE;
    // if vol label and any other bits are set, error
    if ((root->attrb & FAT_ATTR_VOLUME) && (root->attrb & ~FAT_ATTR_VOLUME))
      return FAT_BAD_ATTRIBUTE;
  }
  
  // if a LFN entry, check it first, returning index to SFN in sfn
  if (root->attrb == FAT_ATTR_LONG_FILE) {
    DWORD ErrorCode;
    sfn = FatCheckLFN((struct S_FAT_LFN_ROOT *) root, &ErrorCode);
    if (ErrorCode != FAT_NO_ERROR)
      return ErrorCode;
  }
  
  // Check that there are no invalid chars in the name
  if ((root[sfn].name[0] != FAT_DELETED_CHAR) && (root[sfn].name[0] != 0)) {
    for (i=0; i<8; i++) {
      if (!FatIsValidChar(root[sfn].name[i]))
        return FAT_BAD_CHAR;
    }
    for (i=0; i<3; i++) {
      if (!FatIsValidChar(root[sfn].ext[i]))
        return FAT_BAD_CHAR;
    }
  }

  // if the first char is non-zero but any remaining chars in the name are,
  //  this is an error
  if (root[sfn].name[0]) {
    for (i=1; i<8; i++) {
      if (root[sfn].name[i] == 0)
        return FAT_BAD_CHAR;
    }
    for (i=0; i<3; i++) {
      if (root[sfn].ext[i] == 0)
        return FAT_BAD_CHAR;
    }
  }
  
  // check the the reserved section is all zeros...
  //for (i=0; i<10; i++)
  //  if (root[sfn].type.resv[i] != 0)
  //    return FAT_BAD_RESVD;
  
  return FAT_NO_ERROR;
}

// retrieve the (long) filename from the entry given
unsigned CFat::FatGetName(struct S_FAT_ROOT *root, CString &name, BYTE *attrb, DWORD *start, DWORD *filesize, BOOL *IsDot) {
  unsigned i = 0;
  CString ext;
  
  if (root->attrb == FAT_ATTR_LONG_FILE)
    i = FatGetLFN((struct S_FAT_LFN_ROOT *) root, name);
  else {
    name.Format("%c%c%c%c%c%c%c%c", root->name[0], root->name[1], root->name[2], root->name[3],
      root->name[4], root->name[5], root->name[6], root->name[7]);
    name.TrimRight(0x20);
    ext.Format("%c%c%c", root->ext[0], root->ext[1], root->ext[2]);
    ext.TrimRight(0x20);
    if (ext.GetLength())
      name += ".";
    name += ext;
  }
  
  if (attrb) *attrb = root[i].attrb;
  if (start) {
    *start = root[i].strtclst;
    if (m_fat_size == FS_FAT32)
      *start |= (root[i].type.fat32.strtclst32 << 16);
  }
  if (filesize) *filesize = root[i].filesize;
  if (IsDot) {
    *IsDot = ((memcmp(root[i].name, ".       ", 8) == 0) ||
              (memcmp(root[i].name, "..      ", 8) == 0)) &&
              (memcmp(root[i].ext, "   ", 3) == 0);
  }
  
  return i + 1;
}

unsigned CFat::FatGetLFN(struct S_FAT_LFN_ROOT *lfn, CString &name) {
  int i, j, cnt = lfn->sequ_flags & ~0xC0;
  char *t, str[1024];
  
  if (lfn->sequ_flags & 0x80)
    return cnt;
  
  i = cnt - 1;
  memset(str, 0, 1024);
  t = str;
  
  while (i>=0) {
    BYTE *s = (BYTE *) lfn[i].name0;
    for (j=0; j<13; j++) {
      if (j==5)  s = (BYTE *) lfn[i].name1;
      if (j==11) s = (BYTE *) lfn[i].name2;
      *t++ = *s++;
      s++;
    }
    i--;
  }
  
  name = str;
  return cnt;
}

unsigned CFat::FatCheckLFN(struct S_FAT_LFN_ROOT *lfn, DWORD *ErrorCode) {
  int i, j, cnt = lfn->sequ_flags & ~0xC0;
  char *t, str[1024];
  
  // if bit 7 set, this is a deleted entry
  if (lfn->sequ_flags & 0x80) {
    if (ErrorCode) *ErrorCode = FAT_BAD_LFN_DEL;
    return cnt;
  }
  
  // the first LFN entry should be (0x40 | n)
  if (!(lfn->sequ_flags & 0x40)) {
    if (ErrorCode) *ErrorCode = FAT_BAD_LFN_SEQU;
    return cnt;
  }
  
  // check the CRC of the SFN entry
  struct S_FAT_ROOT *root = (struct S_FAT_ROOT *) ((BYTE *) lfn + (cnt * 32));
  BYTE *p = (BYTE *) root->name;
  BYTE Sum = 0;
  for (i=11; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  if (Sum != lfn->sfn_crc) {
    if (ErrorCode) *ErrorCode = FAT_BAD_LFN_CRC;
    return cnt;
  }
  
  // now get the name and check it for illegal values
  i = cnt - 1;
  memset(str, 0, 1024); // 63 total entries * 26 w_chars each (13 bytes each) = 819 total byte chars
  t = str;
  while (i>=0) {
    BYTE *s = (BYTE *) lfn[i].name0;
    for (j=0; j<13; j++) {
      if (j==5)  s = (BYTE *) lfn[i].name1;
      if (j==11) s = (BYTE *) lfn[i].name2;
      *t++ = *s++;
      s++;
    }
    i--;
  }
  
  t = str;
  while (*t) {
    if (!FatIsValidChar(*t)) {
      if (ErrorCode) *ErrorCode = FAT_BAD_LFN_CHAR;
      return cnt;
    }
    t++;
  }
  
  if (ErrorCode) *ErrorCode = FAT_NO_ERROR;
  return cnt;
}

const char fat_valid_chars[] = "$%'-@{}~!#()&_^ .";

// returns TRUE if a valid char for FAT
BOOL CFat::FatIsValidChar(const char ch) {
  return (isalpha(ch) || isdigit(ch) || (strchr(fat_valid_chars, ch) ? TRUE : FALSE));
}

// Load the first FAT to fat_buffer
void *CFat::FatLoadFAT(void *fat_buffer) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_bpb_buffer;
  struct S_FAT1216_BPB *bpb12 = (struct S_FAT1216_BPB *) m_bpb_buffer;
  if (fat_buffer) free(fat_buffer);
  if (m_fat_size == FS_FAT32) {
    fat_buffer = malloc(bpb32->sect_per_fat32 * bpb32->bytes_per_sect);
    dlg->ReadFromFile(fat_buffer, m_lba + bpb32->sect_reserved, bpb32->sect_per_fat32, FALSE);
  } else {
    fat_buffer = malloc(bpb12->sect_per_fat * bpb12->bytes_per_sect);
    dlg->ReadFromFile(fat_buffer, m_lba + bpb12->sect_reserved, bpb12->sect_per_fat, FALSE);
  }
  return fat_buffer;
}

// calculate sectors needed for a FAT Table
// size = total sectors (minus reserved space (and root sectors for FAT12, 16))
// spc = sectors per cluster
// sect_size = sector size in bytes
// fat_size = FS_FAT12, FS_FAT16, or FS_FAT32
int CFat::CalcSectPerFat(DWORD64 size, int spc, int sect_size, int fat_size) {
  
  size /= spc;   // size == fat entries
  switch (fat_size) {
    case FS_FAT12:
      size = size + (size / 2);  // 1 1/2 bytes per entry
      size /= sect_size;         // divide by bytes per sector
      break;
    case FS_FAT16:
      size *= 2;                 // 2 bytes per entry
      size /= sect_size;         // divide by bytes per sector
      break;
    case FS_FAT32:
      size *= 4;                 // 4 bytes per entry
      size /= sect_size;         // divide by bytes per sector
      break;
  }
  
  // make sure not zero
  if (size == 0)
    size = 1;
  
  return (int) size;
}

DWORD CFat::GetNextCluster(void *FatBuffer, DWORD cluster) {
  BYTE *fat = (BYTE *) FatBuffer;
  DWORD newclust;
  
  switch (m_fat_size) {
    case FS_FAT12:
      fat += cluster;
      fat += (cluster / 2);
      newclust = * ((WORD *) fat);
      if (cluster & 1) newclust >>= 4;
      else             newclust &= 0x0FFF;
      if (newclust >= 0x0FF8) newclust = 0xFFFFFFFF;
      break;
    case FS_FAT16:
      fat += (cluster << 1);
      newclust = * ((WORD *) fat);
      if (newclust >= 0xFFF8) newclust = 0xFFFFFFFF;
      break;
    case FS_FAT32:
      fat += (cluster << 2);
      newclust = * ((DWORD *) fat) & 0x0FFFFFFF;
      if (newclust >= 0x0FFFFFF8) newclust = 0xFFFFFFFF;
      break;
    default:
      newclust = 0xFFFFFFFF;
  }
  
  return newclust;
}

int CFat::FatFillClusterList(struct S_FAT_ENTRIES *EntryList, DWORD Cluster) {
  EntryList->was_error = FALSE;
  EntryList->entry_count = 0;
  EntryList->entry_size = 128;
  EntryList->entries = (DWORD *) malloc(128 * sizeof(DWORD));
  while (Cluster < 0xFFFFFFFF) {
    // check to see if this cluster number is already in the list.
    // if so, we have an error.
    for (int i=0; i<EntryList->entry_count; i++) {
      if (EntryList->entries[i] == Cluster) {
        EntryList->was_error = TRUE;
        AfxMessageBox("Bad Cluster list... (Error: FAT0001)");
        return EntryList->entry_count;
      }
    }
    if (EntryList->entry_count >= EntryList->entry_size) {
      EntryList->entry_size += 128;
      EntryList->entries = (DWORD *) realloc(EntryList->entries, EntryList->entry_size * sizeof(DWORD));
    }
    EntryList->entries[EntryList->entry_count] = Cluster;
    EntryList->entry_count++;
    Cluster = GetNextCluster(m_fat_buffer, Cluster);
  }
  return EntryList->entry_count;
}

// allocate some FAT entries, writing the FAT back to the file
int CFat::AllocateFAT(struct S_FAT_ENTRIES *EntryList, DWORD size) {
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  unsigned cnt = (((size + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect) + (bpb->sect_per_clust - 1)) / bpb->sect_per_clust;
  unsigned i;
  
  // allocate the entry list
  EntryList->was_error = FALSE;
  EntryList->entry_count = 0;
  EntryList->entry_size = cnt;
  EntryList->entries = (DWORD *) malloc(cnt * sizeof(DWORD));
  
  DWORD cluster = 2;
  for (i=0; i<cnt; i++) {
    cluster = FindFreeCluster(cluster);
    if (cluster == 0xFFFFFFFF) {
      AfxMessageBox("Image is full");
      return -1;
    }
    EntryList->entries[i] = cluster;
    EntryList->entry_count++;
  }
  
  // now mark these clusters
  for (i=0; i<cnt-1; i++)
    MarkCluster(EntryList->entries[i], EntryList->entries[i + 1]);
  MarkCluster(EntryList->entries[i], 0xFFFFFFFF);
  
  return cnt;
}

DWORD CFat::FindFreeCluster(DWORD Cluster) {
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  DWORD newclust;
  BYTE *fat;
  
  while (Cluster < (DWORD) (m_size / bpb->sect_per_clust)) {
    Cluster++;
    fat = (BYTE *) m_fat_buffer;
    switch (m_fat_size) {
      case FS_FAT12:
        fat += Cluster;
        fat += (Cluster / 2);
        newclust = * ((WORD *) fat);
        if (Cluster & 1) newclust >>= 4;
        else             newclust &= 0x0FFF;
        if (newclust == 0)
          return Cluster;
        break;
      case FS_FAT16:
        fat += (Cluster << 1);
        newclust = *((WORD *) fat);
        if (newclust == 0)
          return Cluster;
        break;
      case FS_FAT32:
        fat += (Cluster << 2);
        newclust = *((DWORD *) fat) & 0x0FFFFFFF;
        if (newclust == 0)
          return Cluster;
        break;
      default:
        return 0xFFFFFFFF;
    }
  }
  
  return 0xFFFFFFFF;
}

void CFat::UpdateTheFATs(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  unsigned l = 0;
  
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_bpb_buffer;
    for (int i=0; i<bpb32->fats; i++) {
      dlg->WriteToFile(m_fat_buffer, m_lba + bpb32->sect_reserved + l, bpb32->sect_per_fat32, FALSE);
      l += bpb32->sect_per_fat32;
    }
  } else {
    struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
    for (int i=0; i<bpb->fats; i++) {
      dlg->WriteToFile(m_fat_buffer, m_lba + bpb->sect_reserved + l, bpb->sect_per_fat, FALSE);
      l += bpb->sect_per_fat;
    }
  }
}

void CFat::MarkCluster(DWORD Cluster, DWORD Value) {
  BYTE *fat = (BYTE *) m_fat_buffer;
  WORD word;
  
  switch (m_fat_size) {
    case FS_FAT12:
      fat += Cluster;
      fat += (Cluster / 2);
      word = * ((WORD *) fat);
      if (Cluster & 1) *((WORD *) fat) = ((word & ~0xFFF0) | ((WORD) (Value & 0xFFF) << 4));
      else             *((WORD *) fat) = ((word & ~0x0FFF) | ((WORD) (Value & 0xFFF) << 0));
      break;
    case FS_FAT16:
      fat += (Cluster << 1);
      * ((WORD *) fat) = (WORD) (Value & 0xFFFF);
      break;
    case FS_FAT32:
      fat += (Cluster << 2);
      * ((DWORD *) fat) = Value & 0x0FFFFFFF;
      break;
  }
}

void CFat::AllocateRoot(CString csName, DWORD RootCluster, DWORD StartCluster, DWORD FileSize, BYTE Attribute, BOOL IsRoot) {
  struct S_FAT_ROOT *root, *r, *sfn = NULL;
  struct S_FAT_LFN_ROOT *lfn;
  DWORD rootsize = (IsRoot) ? m_rootsize : 0;
  BOOL fnd = FALSE;
  int i, cnt = 1 + ((csName.GetLength() + (FAT_CHARS_PER_LFN - 1)) / FAT_CHARS_PER_LFN);  // count of consecutive entries needed
  
  root = (struct S_FAT_ROOT *) ReadFile(RootCluster, &rootsize, IsRoot);
  if (root) {
    r = root;
    i = 0;
    lfn = NULL;
    while ((BYTE *) r < ((BYTE *) root + rootsize)) {
      if ((r->name[0] == 0) || (r->name[0] == FAT_DELETED_CHAR) || (r->name[0] & 0x80) ) {
        if (lfn == NULL) {
          lfn = (struct S_FAT_LFN_ROOT *) r;
          i = 1;
        } else {
          i++;
          if (i == cnt) {
            sfn = r;
            fnd = TRUE;
            break;
          }
        }
      } else {
        lfn = NULL;
        i = 0;
      }
      r++;
    }
    
    // if fnd == TRUE, we found enough consective entries
    if (fnd && sfn) {
      memset(lfn, 0, (cnt * 32));
      
      // Short Filename entry
      CreateSFN(csName, 1, sfn->name, sfn->ext);
      sfn->attrb = Attribute;
      sfn->date = CreateDate();
      sfn->filesize = FileSize;
      sfn->strtclst = (WORD) (StartCluster & 0xFFFF);
      sfn->time = CreateTime();
      if (m_fat_size == FS_FAT32) {
        sfn->type.fat32.crt_time_tenth = 0;
        sfn->type.fat32.crt_time = CreateTime();
        sfn->type.fat32.crt_date = CreateDate();
        sfn->type.fat32.crt_last = CreateDate();
        sfn->type.fat32.strtclst32 = (WORD) (StartCluster >> 16);
      }
      
      // Long Filename entry(s)
      int j, k = 1, p = 0;
      int l = csName.GetLength();
      for (i=cnt-2; i>=0; i--) {
        lfn[i].sequ_flags = ((i == 0) ? 0x40 : 0) | k++;
        for (j=0; j<10; j+=2) {
          lfn[i].name0[j] = (p == l) ? 0 : csName.GetAt(p++);
          lfn[i].name0[j+1] = 0;
        }
        lfn[i].attrb = 0x0F;
        lfn[i].sfn_crc = CalcCRCFromSFN(sfn);
        for (j=0; j<12; j+=2) {
          lfn[i].name1[j] = (p == l) ? 0 : csName.GetAt(p++);
          lfn[i].name1[j+1] = 0;
        }
        lfn[i].clust_zero = 0;
        for (j=0; j<4; j+=2) {
          lfn[i].name2[j] = (p == l) ? 0 : csName.GetAt(p++);
          lfn[i].name2[j+1] = 0;
        }
      }
      
      // write the root back to the file image
      struct S_FAT_ENTRIES ClusterList;
      if (!IsRoot || (m_fat_size == FS_FAT32))
        FatFillClusterList(&ClusterList, RootCluster);
      WriteFile(root, &ClusterList, rootsize, IsRoot);
      if (!IsRoot || (m_fat_size == FS_FAT32))
        free(ClusterList.entries);
    } else {
      AfxMessageBox("Directory is full");
    }
  }
}

void CFat::AllocateRootSFN(CString csName, DWORD RootCluster, DWORD StartCluster, DWORD FileSize, BYTE Attribute, BOOL IsRoot) {
  struct S_FAT_ROOT *root, *sfn;
  DWORD rootsize = (IsRoot) ? m_rootsize : 0;
  BOOL fnd = FALSE;
  
  root = (struct S_FAT_ROOT *) ReadFile(RootCluster, &rootsize, IsRoot);
  if (root) {
    sfn = root;
    while ((BYTE *) sfn < ((BYTE *) root + rootsize)) {
      if (sfn->name[0] == 0) {
        memset(sfn, 0, 32);
        // Short Filename entry
        CreateSFN(csName, 1, sfn->name, sfn->ext);
        sfn->attrb = Attribute;
        sfn->date = CreateDate();
        sfn->filesize = FileSize;
        sfn->strtclst = (WORD) (StartCluster & 0xFFFF);
        sfn->time = CreateTime();
        if (m_fat_size == FS_FAT32) {
          sfn->type.fat32.crt_time_tenth = 0;
          sfn->type.fat32.crt_time = CreateTime();
          sfn->type.fat32.crt_date = CreateDate();
          sfn->type.fat32.crt_last = CreateDate();
          sfn->type.fat32.strtclst32 = (WORD) (StartCluster >> 16);
        }
        fnd = TRUE;
        break;
      }
      sfn++;
    }
    
    if (fnd) {
      // write the root back to the file image
      struct S_FAT_ENTRIES ClusterList;
      if (!IsRoot || (m_fat_size == FS_FAT32))
        FatFillClusterList(&ClusterList, RootCluster);
      WriteFile(root, &ClusterList, rootsize, IsRoot);
      if (!IsRoot || (m_fat_size == FS_FAT32))
        free(ClusterList.entries);
    } else {
      AfxMessageBox("Directory is full");
    }
  }
}

BYTE CFat::CalcCRCFromSFN(struct S_FAT_ROOT *entry) {
  BYTE *p = (BYTE *) entry->name;
  BYTE Sum = 0;
  for (int i=11; i!=0; i--)
    Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *p++;
  return Sum;
}

void CFat::CreateSFN(CString csLFN, int seq, BYTE name[8], BYTE ext[3]) {
  CString csName, csExt, cs;
  
  memset(name, ' ', 8);
  memset(ext, ' ', 3);
  
  // check to see if it is the '.' or '..' entries
  if (csLFN == ".") {
    name[0] = '.';
    return;
  }
  if (csLFN == "..") {
    name[0] =
    name[1] = '.';
    return;
  }
  
  // extract the two: name and ext
  int i = csLFN.Find('.', 0);
  int l = csLFN.GetLength();
  if (i > -1) {
    csName = csLFN.Left(i);
    if ((l - i - 1) > 0)
      csExt = csLFN.Right(l - i - 1);
  } else
    csName = csLFN;
  
  BOOL dotilde = (csName.GetLength() == 0) || (csName.GetLength() > 8) || (csExt.GetLength() > 3);
  
  // check for any illegal chars
  csName = csName.Left(8);
  csName.MakeUpper();
  l = csName.GetLength();
  for (i=0; i<l; i++)
    if (!FatIsValidChar(csName.GetAt(i)) || ((csName.GetAt(i) == '~') && dotilde))
      csName.SetAt(i, '_');
  
  csExt = csExt.Left(3);
  csExt.MakeUpper();
  l = csExt.GetLength();
  for (i=0; i<l; i++)
    if (!FatIsValidChar(csExt.GetAt(i)))
      csExt.SetAt(i, '_');
  
  if (dotilde) {
    cs.Format("~%i", seq);
    csName += cs;
    csName = csName.Right(8);
  }
  
  l = csName.GetLength();
  memcpy(name, csName, l);
  
  l = csExt.GetLength();
  memcpy(ext, csExt, l);
}

// 15-9 Year (0 = 1980, 119 = 2099 supported under DOS/Windows, theoretically up to 127 = 2107)
// 8-5  Month (1–12)
// 4-0  Day (1–31) 
WORD CFat::CreateDate(void) {
  CTime time = CTime::GetCurrentTime();
  WORD word;
  
  word = ((time.GetYear() - 1980) << 9) |
          (time.GetMonth() << 5) |
          (time.GetDay() << 0);
  
  return word;
}

// 15-11  Hours (0-23)
// 10-5   Minutes (0-59)
// 4-0  Seconds/2 (0-29) 
WORD CFat::CreateTime(void) {
  CTime time = CTime::GetCurrentTime();
  WORD word;
  
  word = (time.GetHour() << 11) |
         (time.GetMinute() << 5) |
         (time.GetSecond() << 0);
  
  return word;
}

// update all items in dialog
void CFat::SendToDialog(void *ptr) {
  if (ptr == NULL)
    return;
  
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB *bpb = (struct S_FAT32_BPB *) ptr;
    m_jmp0.Format("0x%02X", bpb->jmp[0]);
    m_jmp1.Format("0x%02X", bpb->jmp[1]);
    m_jmp2.Format("0x%02X", bpb->jmp[2]);
    m_oem_name.Format("%c%c%c%c%c%c%c%c", bpb->oem_name[0], bpb->oem_name[1], bpb->oem_name[2], bpb->oem_name[3],
      bpb->oem_name[4], bpb->oem_name[5], bpb->oem_name[6], bpb->oem_name[7]);
    m_bytes_sect.Format("%i", bpb->bytes_per_sect);
    m_sects_cluster.Format("%i", bpb->sect_per_clust);
    m_sect_reserved.Format("%i", bpb->sect_reserved);
    m_fats.Format("%i", bpb->fats);
    m_root_entries.Format("%i", bpb->root_entrys);
    m_sectors.Format("%i", bpb->sectors);
    m_descriptor.Format("0x%02X", bpb->descriptor);
    m_sect_fat.Format("%i", bpb->sect_per_fat);
    m_sect_track.Format("%i", bpb->sect_per_trk);
    m_heads.Format("%i", bpb->heads);
    m_hidden_sects.Format("%i", bpb->hidden_sects);
    m_sectors_ext.Format("%i", bpb->sect_extnd);
    m_sect_fat32.Format("%i", bpb->sect_per_fat32);
    m_ext_flags.Format("0x%04X", bpb->ext_flags);
    m_fs_version.Format("0x%04X", bpb->fs_version);
    m_clust_num.Format("%i", bpb->root_base_cluster);
    m_info_sector.Format("%i", bpb->fs_info_sec);
    m_backup_sector.Format("%i", bpb->backup_boot_sec);
    m_drv_num.Format("0x%02X", bpb->drive_num);
    m_reserved.Format("0x%02X", bpb->resv);
    m_fat_sig.Format("0x%02X", bpb->sig);
    m_serial_number.Format("0x%08X", bpb->serial);
    m_label.Format("%c%c%c%c%c%c%c%c%c%c%c", bpb->label[0], bpb->label[1], bpb->label[2], bpb->label[3],
      bpb->label[4], bpb->label[5], bpb->label[6], bpb->label[7], bpb->label[8], bpb->label[9], bpb->label[10]);
    m_fat_type.Format("%c%c%c%c%c%c%c%c", bpb->sys_type[0], bpb->sys_type[1], bpb->sys_type[2], bpb->sys_type[3],
      bpb->sys_type[4], bpb->sys_type[5], bpb->sys_type[6], bpb->sys_type[7]);
  } else {
    struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) ptr;
    m_jmp0.Format("0x%02X", bpb->jmp[0]);
    m_jmp1.Format("0x%02X", bpb->jmp[1]);
    m_jmp2.Format("0x%02X", bpb->jmp[2]);
    m_oem_name.Format("%c%c%c%c%c%c%c%c", bpb->oem_name[0], bpb->oem_name[1], bpb->oem_name[2], bpb->oem_name[3],
      bpb->oem_name[4], bpb->oem_name[5], bpb->oem_name[6], bpb->oem_name[7]);
    m_bytes_sect.Format("%i", bpb->bytes_per_sect);
    m_sects_cluster.Format("%i", bpb->sect_per_clust);
    m_sect_reserved.Format("%i", bpb->sect_reserved);
    m_fats.Format("%i", bpb->fats);
    m_root_entries.Format("%i", bpb->root_entrys);
    m_sectors.Format("%i", bpb->sectors);
    m_descriptor.Format("0x%02X", bpb->descriptor);
    m_sect_fat.Format("%i", bpb->sect_per_fat);
    m_sect_track.Format("%i", bpb->sect_per_trk);
    m_heads.Format("%i", bpb->heads);
    if (!IsDlgButtonChecked(IDC_OLD_FAT)) {
      m_hidden_sects.Format("%i", bpb->hidden_sects);
      m_sectors_ext.Format("%i", bpb->sect_extnd);
      m_drv_num.Format("0x%02X", bpb->drive_num);
      m_reserved.Format("0x%02X", bpb->resv);
      m_fat_sig.Format("0x%02X", bpb->sig);
      m_serial_number.Format("0x%08X", bpb->serial);
      m_label.Format("%c%c%c%c%c%c%c%c%c%c%c", bpb->label[0], bpb->label[1], bpb->label[2], bpb->label[3],
        bpb->label[4], bpb->label[5], bpb->label[6], bpb->label[7], bpb->label[8], bpb->label[9], bpb->label[10]);
      m_fat_type.Format("%c%c%c%c%c%c%c%c", bpb->sys_type[0], bpb->sys_type[1], bpb->sys_type[2], bpb->sys_type[3],
        bpb->sys_type[4], bpb->sys_type[5], bpb->sys_type[6], bpb->sys_type[7]);
    } else
      m_hidden_sects.Format("%i", * (WORD *) &bpb->hidden_sects);
    
    // need to clear out the FAT32 stuff
    m_sect_fat32.Empty();
    m_backup_sector.Empty();
    m_clust_num.Empty();
    m_info_sector.Empty();
    m_fs_version.Empty();
  }
  
  UpdateData(FALSE); // send to Dialog
}

void CFat::ReceiveFromDialog(void *ptr) {
  if (ptr == NULL)
    return;
  
  UpdateData(TRUE); // receive from Dialog
  
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB *bpb = (struct S_FAT32_BPB *) ptr;
    bpb->jmp[0] = convert8(m_jmp0);
    bpb->jmp[1] = convert8(m_jmp1);
    bpb->jmp[2] = convert8(m_jmp2);
    memcpy(bpb->oem_name, m_oem_name, 8);
    bpb->bytes_per_sect = convert16(m_bytes_sect);
    bpb->sect_per_clust = convert8(m_sects_cluster);
    bpb->sect_reserved = convert16(m_sect_reserved);
    bpb->fats = convert8(m_fats);
    bpb->root_entrys = convert16(m_root_entries);
    bpb->sectors = convert16(m_sectors);
    bpb->descriptor = convert8(m_descriptor);
    bpb->sect_per_fat = convert16(m_sect_fat);
    bpb->sect_per_trk = convert16(m_sect_track);
    bpb->heads = convert16(m_heads);
    bpb->hidden_sects = convert32(m_hidden_sects);
    bpb->sect_extnd = convert32(m_sectors_ext);
    bpb->sect_per_fat32 = convert32(m_sect_fat32);
    bpb->ext_flags = convert16(m_ext_flags);
    bpb->fs_version = convert16(m_fs_version);
    bpb->root_base_cluster = convert32(m_clust_num);
    bpb->fs_info_sec = convert16(m_info_sector);
    bpb->backup_boot_sec = convert16(m_backup_sector);
    bpb->drive_num = convert8(m_drv_num);
    bpb->resv = convert8(m_reserved);
    bpb->sig = convert8(m_fat_sig);
    bpb->serial = convert32(m_serial_number);
    memcpy(bpb->label, m_label, 11);
    memcpy(bpb->sys_type, m_fat_type, 8);
  } else {
    struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) ptr;
    bpb->jmp[0] = convert8(m_jmp0);
    bpb->jmp[1] = convert8(m_jmp1);
    bpb->jmp[2] = convert8(m_jmp2);
    memcpy(bpb->oem_name, m_oem_name, 8);
    bpb->bytes_per_sect = convert16(m_bytes_sect);
    bpb->sect_per_clust = convert8(m_sects_cluster);
    bpb->sect_reserved = convert16(m_sect_reserved);
    bpb->fats = convert8(m_fats);
    bpb->root_entrys = convert16(m_root_entries);
    bpb->sectors = convert16(m_sectors);
    bpb->descriptor = convert8(m_descriptor);
    bpb->sect_per_fat = convert16(m_sect_fat);
    bpb->sect_per_trk = convert16(m_sect_track);
    bpb->heads = convert16(m_heads);
    if (!IsDlgButtonChecked(IDC_OLD_FAT)) {
      bpb->hidden_sects = convert32(m_hidden_sects);
      bpb->sect_extnd = convert32(m_sectors_ext);
      bpb->drive_num = convert8(m_drv_num);
      bpb->resv = convert8(m_reserved);
      bpb->sig = convert8(m_fat_sig);
      bpb->serial = convert32(m_serial_number);
      memcpy(bpb->label, m_label, 11);
      memcpy(bpb->sys_type, m_fat_type, 8);
    } else
      * (WORD *) &bpb->hidden_sects = convert16(m_hidden_sects);
  }
}

void CFat::SaveItemInfo(HTREEITEM hItem, DWORD Cluster, DWORD FileSize, struct S_FAT_ROOT *Entry, int Index, int count, DWORD ErrorCode, BOOL CanCopy) {
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->Cluster = Cluster;
    items->FileSize = FileSize;
    items->Index = Index;
    items->ErrorCode = ErrorCode;
    items->CanCopy = CanCopy;
    
    if (count > 64) count = 64;
    items->EntryCount = count;
    if (Entry)
      memcpy(items->Entry, Entry, (count * sizeof(struct S_FAT_ROOT)));
  }
}

void CFat::OnFatCopy() {
  char szPath[MAX_PATH];
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  CWaitCursor wait;
  if (IsDir) {
    csPath = AfxGetApp()->GetProfileString("Settings", "DefaultExtractPath", NULL);
    if (!BrowseForFolder(GetSafeHwnd(), csPath, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS))
      return;
    csPath = szPath;
    AfxGetApp()->WriteProfileString("Settings", "DefaultExtractPath", csPath);
    CopyFolder(hItem, csPath, csName);
  } else {
    CFileDialog dlg (
      FALSE,            // Create an saveas file dialog
      NULL,             // Default file extension
      csName,           // Default Filename
      OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT, // flags
      NULL
    );
    if (dlg.DoModal() != IDOK)
      return;
    
    POSITION pos = dlg.GetStartPosition();
    csPath = dlg.GetNextPathName(pos);
    CopyFile(hItem, csPath);
  }
  
  wait.Restore();
  AfxMessageBox("Files transferred.");
}

void CFat::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    // don't allow the label to be copied out
    // TODO: this is the LFN attribute which *DOES* have the VOLUME flag set
    //if (items->Entry[0].attrb & FAT_ATTR_VOLUME)
    //  return;
    void *ptr = ReadFile(items->Cluster, NULL, FALSE);
    if (ptr) {
      CFile file;
      if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
        file.Write(ptr, items->FileSize);
        file.Close();
      } else
        AfxMessageBox("Error Creating File...");
      free(ptr);
    }
  }
}

// hItem = tree item of folder
// csPath = existing path to create folder in
// csName = name of folder to create
void CFat::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
  char szCurPath[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, szCurPath);
  BOOL IsDir;
  CString csFName, csFPath;
  
  // change to the directory we passed here
  SetCurrentDirectory(csPath);
  
  // create the new directory
  if (CreateDirectory(csName, NULL) == 0) {
    DWORD Error = GetLastError();
    if (Error == ERROR_ALREADY_EXISTS) {
      if (AfxMessageBox("Directory Already Exists.  Overwrite?", MB_YESNO, 0) != IDYES) {
        SetCurrentDirectory(szCurPath);
        return;
      }
    } else {
      SetCurrentDirectory(szCurPath);
      return;
    }
  }
  
  // change to the newly created path
  csPath += "\\";
  csPath += csName;
  SetCurrentDirectory(csPath);
  
  // it should have children, but let's be sure
  if (m_dir_tree.ItemHasChildren(hItem)) {
    HTREEITEM hChildItem = m_dir_tree.GetChildItem(hItem);
    while (hChildItem != NULL) {
      m_dir_tree.GetFullPath(hChildItem, &IsDir, csFName, csFPath, FALSE);
      if (IsDir)
        CopyFolder(hChildItem, csPath, csFName);
      else
        CopyFile(hChildItem, csFName);
      hChildItem = m_dir_tree.GetNextItem(hChildItem, TVGN_NEXT);
    }
  }
  
  // restore the current directory
  SetCurrentDirectory(szCurPath);
}

// insert a volume label entry into the root directory.
//  does not check to see if there already is one
void CFat::OnInsertVLabel() {
  CInsertVName vname;

  if (vname.DoModal() == IDOK) {
    HTREEITEM hItem = m_dir_tree.GetRootItem();
    struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);

    // create a root entry in the folder
    AllocateRoot(vname.m_volume_name, items->Cluster, 0, 0, FAT_ATTR_VOLUME, TRUE);

    // refresh the "system"
    Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
  }
}

void CFat::OnFatInsert() {
  char szPath[MAX_PATH];
  BOOL IsDir, IsRoot, IsRootDir, bError = FALSE;
  CString csName, csPath;
  
  // get a file/directory from the host
  csName = AfxGetApp()->GetProfileString("Settings", "DefaultExtractPath", NULL);
  if (!BrowseForFolder(GetSafeHwnd(), csName, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES))
    return;
  csPath = szPath;
  AfxGetApp()->WriteProfileString("Settings", "DefaultExtractPath", csPath);
  // at this point:
  //  csPath = full path to file/folder selected (including filename if given)
  //  csName = name of file/folder selected
  
  // save the current directory
  GetCurrentDirectory(MAX_PATH, szPath);
  
  // as far as I know, there is no way to tell if that returned path is a file or a directory.
  // so we try to change to that path.  If it fails, it must be a file.
  IsDir = SetCurrentDirectory(csPath);
  // and then restore it
  SetCurrentDirectory(szPath);
  
  // get the item from the tree control
  CString csFName, csFPath;
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsRootDir, csFName, csFPath, FALSE);
  
  // is it the root?
  IsRoot = (IsRootDir == -1);
  
  CWaitCursor wait;
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    if (IsDir)
      InsertFolder(items->Cluster, csName, csPath, IsRoot);
    else
      bError = (InsertFile(items->Cluster, csName, csPath, IsRoot) != TRUE);
  }
  
  // need to write the FAT back to the image file
  UpdateTheFATs();
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
  
  wait.Restore();
  //if (!bError)
  //  AfxMessageBox("Files transferred.");
}

// Cluster = starting cluster of folder to insert to
// csName = name of file to insert
// csPath = path on host of file to insert
BOOL CFat::InsertFile(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot) {
  struct S_FAT_ENTRIES fat_entries;
  void *buffer;
  size_t size;
  CFile file;
  
  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return FALSE;
  }
  size = (size_t) file.GetLength();
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  buffer = malloc(size + (bpb->sect_per_clust * bpb->bytes_per_sect));  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, (UINT) size);
  file.Close();
  
  // allocate the fat entries for it, returning a "struct S_FAT_ENTRIES"
  if (AllocateFAT(&fat_entries, (DWORD) size) == -1) {
    free(fat_entries.entries);
    free(buffer);
    return FALSE;
  }
  
  // create a root entry in the folder
  AllocateRoot(csName, Cluster, fat_entries.entries[0], (DWORD) size, FAT_ATTR_ARCHIVE, IsRoot);
  
  // copy the file to our system
  WriteFile(buffer, &fat_entries, (DWORD) size, FALSE);
  
  // free the buffers
  free(fat_entries.entries);
  free(buffer);

  return TRUE;
}

// Cluster = starting cluster of folder to insert in to
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CFat::InsertFolder(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot) {
  struct S_FAT_ENTRIES fat_entries;
  const unsigned size = 8192;
  char szPath[MAX_PATH];
  
  // allocate the fat entries for the folder, returning a "struct S_FAT_ENTRIES"
  if (AllocateFAT(&fat_entries, size) == -1) {
    free(fat_entries.entries);
    return;
  }
  
  // create a root entry in the folder for the folder
  AllocateRoot(csName, Cluster, fat_entries.entries[0], 0, FAT_ATTR_SUB_DIR, IsRoot);
  
  // create the directory in our image
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *buffer = calloc(size + (bpb->sect_per_clust * bpb->bytes_per_sect), 1);  // to prevent buffer overrun in WriteFile()
  WriteFile(buffer, &fat_entries, size, FALSE);
  free(buffer);
  
  // add the . and .. entries
  CString csDots = ".";
  AllocateRootSFN(csDots, fat_entries.entries[0], fat_entries.entries[0], 0, FAT_ATTR_SUB_DIR, FALSE);
  csDots = "..";
  AllocateRootSFN(csDots, fat_entries.entries[0], Cluster, 0, FAT_ATTR_SUB_DIR, FALSE);
  
  // save the current directory
  GetCurrentDirectory(MAX_PATH, szPath);
  
  // Set to the directory of the host's path
  SetCurrentDirectory(csPath);
  
  // now scan the host for files within this folder, recursing folders if needed
  CFileFind filefind;
  BOOL fnd = filefind.FindFile(NULL);
  while (fnd) {
    fnd = filefind.FindNextFile();
    
    // if '.' or '..', skip so we don't recurse indefinately
    if (filefind.IsDots())
      continue;
    
    if (filefind.IsDirectory())
      InsertFolder(fat_entries.entries[0], filefind.GetFileName(), filefind.GetFilePath(), FALSE);
    else
      InsertFile(fat_entries.entries[0], filefind.GetFileName(), filefind.GetFilePath(), FALSE);
  }
  filefind.Close();
  
  // restore the current directory
  SetCurrentDirectory(szPath);
  
  free(fat_entries.entries);
}

// the user change the status of the "Show Deleted" Check box
void CFat::OnShowDeleted() {
  AfxGetApp()->WriteProfileInt("Settings", "FATShowDel", m_show_del = IsDlgButtonChecked(IDC_SHOW_DEL));
  Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
}

// the user change the status of the "Delete Clear" Check box
void CFat::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "FATDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void CFat::OnFatDelete() {
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  if (!hItem) {
    AfxMessageBox("No Item");
    return;
  }
  
  // ask before deleting the root directory
  if (csName == "Root")
    if (AfxMessageBox("This will delete the root directory!  Continue?", MB_YESNO, 0) != IDYES)
      return;
  
  // get the parent of this file/dir so we can select it after the delete
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);

  CWaitCursor wait;
  if (IsDir)
    DeleteFolder(hItem);
  else
    DeleteFile(hItem);
  
  // since we changed the FAT, we need to reload the fat buffer
  m_fat_buffer = FatLoadFAT(m_fat_buffer);
  
  // delete the item from the tree
  if (IsDlgButtonChecked(IDC_SHOW_DEL))
    Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
  else
    m_dir_tree.DeleteItem(hItem);

  // select the parent item
  m_dir_tree.Select((hParent != NULL) ? hParent : TVI_ROOT, TVGN_CARET);

  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CFat::DeleteFolder(HTREEITEM hItem) {
  HTREEITEM hChildItem, hDeleteItem;
  CString csPath, csName;
  int IsDir;
  
  // it should have children, but let's be sure
  if (m_dir_tree.ItemHasChildren(hItem)) {
    hChildItem = m_dir_tree.GetChildItem(hItem);
    while (hChildItem != NULL) {
      hDeleteItem = hChildItem;
      hChildItem = m_dir_tree.GetNextItem(hChildItem, TVGN_NEXT);
      m_dir_tree.GetFullPath(hDeleteItem, &IsDir, csName, csPath, FALSE);
      if (IsDir == 1) {
        if ((csName != ".") && (csName != ".."))
          DeleteFolder(hDeleteItem);
      } else 
        DeleteFile(hDeleteItem);
    }
  }
  
  // don't allow to delete root
  if (m_dir_tree.GetParentItem(hItem) != NULL)
    DeleteFile(hItem);
}

void CFat::DeleteFile(HTREEITEM hItem) {
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  struct S_FAT_ROOT *root;
  CString csName, csPath;
  int i;
  
  if (items == NULL)
    return;
  
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  if (hParent == NULL)
    return;
  BOOL IsRoot = (m_dir_tree.GetParentItem(hParent) == NULL);

  struct S_FAT_ITEMS *root_items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hParent);
  if (root_items == NULL)
    return;
  
  m_dir_tree.GetFullPath(hParent, NULL, csName, csPath, TRUE);
  DWORD rootsize = (IsRoot) ? m_rootsize : 0, cluster;
  root = (struct S_FAT_ROOT *) ReadFile(root_items->Cluster, &rootsize, IsRoot);
  if (root) {
    for (i=0; i<items->EntryCount; i++)
      root[items->Index + i].name[0] |= 0x80; // All LFN entries set the first bit in the first byte
    cluster = root[items->Index + i - 1].strtclst;
    // save the first char for undelete (the first byte in the resv[] area is not used for both FAT12,16 and FAT32
    root[items->Index + i - 1].type.resv[0] = root[items->Index + i - 1].name[0];
    //root[items->Index + i - 1].strtclst = 0;
    root[items->Index + i - 1].name[0] = FAT_DELETED_CHAR;  // last entry is the SFN
    
    // TODO: we should update the CRC as well (for undelete?)

    // write back the new root
    struct S_FAT_ENTRIES ClusterList;
    if (!IsRoot || (m_fat_size == FS_FAT32)) {
      FatFillClusterList(&ClusterList, root_items->Cluster);
      WriteFile(root, &ClusterList, rootsize, FALSE);
      free(ClusterList.entries);
    } else
      WriteFile(root, NULL, rootsize, IsRoot);
    free(root);

    // Free the clusters of the file
    FatFillClusterList(&ClusterList, cluster);
    for (i=0; i<ClusterList.entry_count; i++) {
      MarkCluster(ClusterList.entries[i], 0);
      if (m_del_clear)
        ZeroCluster(ClusterList.entries[i]);
    }
    free(ClusterList.entries);
    UpdateTheFATs(); // need to write the FAT back to the image file
  }
}

void CFat::OnSearch() {
  m_dir_tree.Search();
}

// write back to the directory record the modified changes in entry_items->Entry
void CFat::UpdateEntry(HTREEITEM hItem, struct S_FAT_ITEMS *entry_items) {
  struct S_FAT_ROOT *root;
  CString csName, csPath;
  int i;
  
  if (entry_items == NULL)
    return;
  
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  if (hParent == NULL)
    return;

  struct S_FAT_ITEMS *root_items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hParent);
  if (root_items == NULL)
    return;
  
  m_dir_tree.GetFullPath(hParent, NULL, csName, csPath, TRUE);
  BOOL IsRoot = (m_dir_tree.GetParentItem(hParent) == NULL);
  DWORD rootsize = (IsRoot) ? m_rootsize : 0;
  root = (struct S_FAT_ROOT *) ReadFile(root_items->Cluster, &rootsize, IsRoot);
  if (root) {
    for (i=0; i<entry_items->EntryCount; i++)
      memcpy(&root[entry_items->Index + i], &entry_items->Entry[i], sizeof(struct S_FAT_ROOT));
    
    struct S_FAT_ENTRIES ClusterList;
    if (!IsRoot || (m_fat_size == FS_FAT32))
      FatFillClusterList(&ClusterList, root_items->Cluster);
    WriteFile(root, &ClusterList, rootsize, IsRoot);
    if (!IsRoot || (m_fat_size == FS_FAT32))
      free(ClusterList.entries);
    
    free(root);
  }
}

void CFat::OnFatEntry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CFatEntry FatEntry;
  int sfn_index, i;
  
  if (hItem) {
    if (hItem == m_hRoot) {
      AfxMessageBox("No entry exists for the root");
      return;
    }
    struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      sfn_index = items->EntryCount - 1;
      
      // SFN first (the dialog does the LFN)
      FatEntry.m_name.Format("%c%c%c%c%c%c%c%c", 
        items->Entry[sfn_index].name[0], items->Entry[sfn_index].name[1], items->Entry[sfn_index].name[2], items->Entry[sfn_index].name[3],
        items->Entry[sfn_index].name[4], items->Entry[sfn_index].name[5], items->Entry[sfn_index].name[6], items->Entry[sfn_index].name[7]);
      FatEntry.m_ext.Format("%c%c%c", items->Entry[sfn_index].ext[0], items->Entry[sfn_index].ext[1], items->Entry[sfn_index].ext[2]);
      FatEntry.m_attrib.Format("0x%02X", items->Entry[sfn_index].attrb);
      FatEntry.m_resvd.Format("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
        items->Entry[sfn_index].type.resv[0], items->Entry[sfn_index].type.resv[1], items->Entry[sfn_index].type.resv[2], items->Entry[sfn_index].type.resv[3],
        items->Entry[sfn_index].type.resv[4], items->Entry[sfn_index].type.resv[5], items->Entry[sfn_index].type.resv[6], items->Entry[sfn_index].type.resv[7],
        items->Entry[sfn_index].type.resv[8], items->Entry[sfn_index].type.resv[9]);
      FatEntry.m_time.Format("0x%04X", items->Entry[sfn_index].time);
      FatEntry.m_date.Format("0x%04X", items->Entry[sfn_index].date);
      FatEntry.m_cluster.Format("0x%04X", items->Entry[sfn_index].strtclst);
      FatEntry.m_filesize.Format("%i", items->Entry[sfn_index].filesize);
      FatEntry.m_error_code.Format("%i", items->ErrorCode);
      FatEntry.m_lfns = malloc(ROOT_ENTRY_MAX * sizeof(struct S_FAT_ROOT));
      memcpy(FatEntry.m_lfns, &items->Entry, ROOT_ENTRY_MAX * sizeof(struct S_FAT_ROOT));
      FatEntry.m_lfn_count = items->EntryCount - 1;
      FatEntry.m_fat_size = m_fat_size;
      memcpy(&FatEntry.m_sfn, &items->Entry[sfn_index], sizeof(struct S_FAT_ROOT));
      
      // fill fat entry list
      FatEntry.m_fat_entries.entries = NULL;
      FatEntry.m_fat_entries.entry_count = 0;
      if (items->Cluster > 0)
        FatFillClusterList(&FatEntry.m_fat_entries, items->Cluster);
      if (FatEntry.DoModal() == IDOK) { // apply button pressed?
        memcpy(&items->Entry, FatEntry.m_lfns, ROOT_ENTRY_MAX * sizeof(struct S_FAT_ROOT));
        //items->EntryCount = FatEntry.m_lfn_count + 1;  // must keep the same amount of entries so we write them all back
        for (i=0; i<8; i++)
          items->Entry[sfn_index].name[i] = FatEntry.m_name.GetAt(i);
        for (i=0; i<3; i++)
          items->Entry[sfn_index].ext[i] = FatEntry.m_ext.GetAt(i);
        items->Entry[sfn_index].attrb = convert8(FatEntry.m_attrib);
        ConvertDumpToBuffer(FatEntry.m_resvd, items->Entry[sfn_index].type.resv, 10);
        items->Entry[sfn_index].time = convert16(FatEntry.m_time);
        items->Entry[sfn_index].date = convert16(FatEntry.m_date);
        items->Entry[sfn_index].strtclst = convert16(FatEntry.m_cluster);
        items->Entry[sfn_index].filesize = convert32(FatEntry.m_filesize);
        UpdateEntry(hItem, items);
        // reload the list
        Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
      }
      if (FatEntry.m_fat_entries.entries)
        free(FatEntry.m_fat_entries.entries);
      free(FatEntry.m_lfns);
    }
  }
}

void CFat::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CFat::OnFat32Info() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  struct S_FAT32_BPB_INFO *info = (struct S_FAT32_BPB_INFO *) buffer;
  CFat32Info Info;
  CString cs;
  
  GetDlgItemText(IDC_FAT_INFO_SECTOR, cs);
  unsigned info_sector = convert16(cs);
  dlg->ReadFromFile(buffer, m_lba + info_sector, 1, FALSE);
  
  Info.m_sig0.Format("0x%08X", info->sig);
  Info.m_sig1.Format("0x%08X", info->struct_sig);
  Info.m_free_count.Format("%i", info->free_clust_cnt);
  Info.m_next_free.Format("%i", info->next_free_clust);
  Info.m_sig2.Format("0x%08X", info->trail_sig);
  Info.m_reserved = "Reserved 0:\r\n";
  DumpIt(Info.m_reserved, &info->resv, (DWORD) ((BYTE *) &info->resv - (BYTE *) info), 480, TRUE);
  Info.m_reserved += "Reserved 1:\r\n";
  DumpIt(Info.m_reserved, &info->resv0, (DWORD) ((BYTE *) &info->resv0 - (BYTE *) info), 12, TRUE);
  if (Info.DoModal() == IDOK) {
    info->sig = convert32(Info.m_sig0);
    info->struct_sig = convert32(Info.m_sig1);
    info->free_clust_cnt = convert32(Info.m_free_count);
    info->next_free_clust = convert32(Info.m_next_free);
    info->trail_sig = convert32(Info.m_sig2);
    if (Info.m_clear_reserved) {
      memset(info->resv, 0, 480);
      memset(info->resv0, 0, 12);
    }
    dlg->WriteToFile(buffer, m_lba + info_sector, 1, FALSE);
  }
}

void CFat::OnChangeFatFsVersion() {
  CString cs;
  
  GetDlgItemText(IDC_FAT_FS_VERSION, cs);
  WORD val = convert16(cs);
  cs.Format("%i.%i", val >> 8, val & 0xFF);
  SetDlgItemText(IDC_FAT_FS_VERSION_DISP, cs);
}

void CFat::OnSerialUpdate() {
  CString cs;
  cs.Format("0x%08X", (rand() << 16) | rand());
  SetDlgItemText(IDC_FAT_SERIAL_NUM, cs);
}

void CFat::OnFatBackupSectUpdate() {
  // update backup
  // write backup from current items in dialog.
  //  (ask if we should update current to these as well???)
  // if BackupSector number is not within range (1 -> reserved_sects), give error
  AfxMessageBox("TODO:  Update Backup with Current.");
}

void CFat::OnFatBackupSectRestore() {
  // restore current from backup
  // First ask if we are sure.
  // Then update members.  If the Apply button isn't set, it won't write it
  //  We will need to call Start() again ????
  // if BackupSector number is not within range (1 -> reserved_sects), give error
  AfxMessageBox("TODO:  Update Current with Backup.");
}

// before version 3.0 of DOS, the FAT12 BPB only went to a 2-byte Hidden Sectors field
void CFat::OnOldFat() {
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  BOOL show = !IsDlgButtonChecked(IDC_OLD_FAT);
  CString cs;
  
  GetDlgItem(IDC_FAT_SECTORS_EXT)->EnableWindow(show);
  GetDlgItem(IDC_FAT_SECT_FAT32)->EnableWindow(show);
  GetDlgItem(IDC_FAT_EXT_FLAGS)->EnableWindow(show);
  GetDlgItem(IDC_FAT_LABEL)->EnableWindow(show);
  GetDlgItem(IDC_FAT_FAT_TYPE)->EnableWindow(show);
  GetDlgItem(IDC_FAT_DRV_NUM)->EnableWindow(show);
  GetDlgItem(IDC_FAT_RESERVED)->EnableWindow(show);
  GetDlgItem(IDC_FAT_SIG)->EnableWindow(show);
  GetDlgItem(IDC_FAT_SERIAL_NUM)->EnableWindow(show);
  GetDlgItem(IDC_SERIAL_UPDATE)->EnableWindow(show);
  
  cs.Format("%i", (show) ? bpb->hidden_sects : (* (WORD *) &bpb->hidden_sects));
  SetDlgItemText(IDC_FAT_HIDDEN_SECTS, cs);
  
  if (show) {
    cs.Format("%i", bpb->sect_extnd);
    SetDlgItemText(IDC_FAT_SECTORS_EXT, cs);
    cs.Format("0x%02X", bpb->drive_num);
    SetDlgItemText(IDC_FAT_DRV_NUM, cs);
    cs.Format("0x%02X", bpb->resv);
    SetDlgItemText(IDC_FAT_RESERVED, cs);
    cs.Format("0x%02X", bpb->sig);
    SetDlgItemText(IDC_FAT_SIG, cs);
    cs.Format("0x%08X", bpb->serial);
    SetDlgItemText(IDC_FAT_SERIAL_NUM, cs);
    cs.Format("%c%c%c%c%c%c%c%c%c%c%c", bpb->label[0], bpb->label[1], bpb->label[2], bpb->label[3],
      bpb->label[4], bpb->label[5], bpb->label[6], bpb->label[7], bpb->label[8], bpb->label[9], bpb->label[10]);
    SetDlgItemText(IDC_FAT_LABEL, cs);
    cs.Format("%c%c%c%c%c%c%c%c", bpb->sys_type[0], bpb->sys_type[1], bpb->sys_type[2], bpb->sys_type[3],
      bpb->sys_type[4], bpb->sys_type[5], bpb->sys_type[6], bpb->sys_type[7]);
    SetDlgItemText(IDC_FAT_FAT_TYPE, cs);
  } else {
    SetDlgItemText(IDC_FAT_SECTORS_EXT, "");
    SetDlgItemText(IDC_FAT_DRV_NUM, "");
    SetDlgItemText(IDC_FAT_RESERVED, "");
    SetDlgItemText(IDC_FAT_SIG, "");
    SetDlgItemText(IDC_FAT_SERIAL_NUM, "");
    SetDlgItemText(IDC_FAT_LABEL, "");
    SetDlgItemText(IDC_FAT_FAT_TYPE, "");
  }
}

void CFat::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CFat::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CFat::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1, FALSE);
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}
