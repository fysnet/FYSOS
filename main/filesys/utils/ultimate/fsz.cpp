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

// fsz.cpp : implementation file

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#include "fsz.h"
//#include "FatEntry.h"
//#include "Fat32Info.h"
//#include "FatFormat.h"

#include "Modeless.h"
//#include "InsertVName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFSZ property page

IMPLEMENT_DYNCREATE(CFSZ, CPropertyPage)

CFSZ::CFSZ() : CPropertyPage(CFSZ::IDD) {
  //{{AFX_DATA_INIT(CFSZ)
  m_magic = _T("");
  m_version_major = _T("");
  m_version_minor = _T("");
  m_logsec = _T("");
  m_enctype = _T("");
  m_flags = _T("");
  m_maxmounts = _T("");
  m_currmounts = _T("");
  m_numsec = _T("");
  m_freesec = _T("");
  m_freesecfid = _T("");
  m_rootfid = _T("");
  m_badsecfid = _T("");
  m_indexfid = _T("");
  m_metafid = _T("");
  m_journalfid = _T("");
  m_journalhead = _T("");
  m_journaltail = _T("");
  m_journalmax = _T("");
  m_encrypt = _T("");
  m_enchash = _T("");
  m_createdate = _T("");
  m_lastmountdate = _T("");
  m_lastunmountdate = _T("");
  m_lastcheckdate = _T("");
  m_uuid = _T("");
  m_magic2 = _T("");
  m_checksum = _T("");
  m_show_del = FALSE;
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_super = NULL;
  //m_hard_format = FALSE;
}

CFSZ::~CFSZ() {
  if (m_super)
    free(m_super);
  
  m_super = NULL;
}

void CFSZ::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFSZ)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_FSZ_MAGIC, m_magic);
  DDX_Text(pDX, IDC_FSZ_VER_MAJ, m_version_major);
  DDX_Text(pDX, IDC_FSZ_VER_MIN, m_version_minor);
  DDX_Text(pDX, IDC_FSZ_LOGSEC, m_logsec);
  DDX_Text(pDX, IDC_FSZ_ENCTYPE, m_enctype);
  DDX_Text(pDX, IDC_FSZ_FLAGS, m_flags);
  DDX_Text(pDX, IDC_FSZ_MAXMOUNTS, m_maxmounts);
  DDX_Text(pDX, IDC_FSZ_CURRMOUNTS, m_currmounts);
  DDX_Text(pDX, IDC_FSZ_NUMSEC, m_numsec);
  DDX_Text(pDX, IDC_FSZ_FREESEC, m_freesec);
  DDX_Text(pDX, IDC_FSZ_ROOTFID, m_rootfid);
  DDX_Text(pDX, IDC_FSZ_FREESECFID, m_freesecfid);
  DDX_Text(pDX, IDC_FSZ_BADSECFID, m_badsecfid);
  DDX_Text(pDX, IDC_FSZ_INDEXFID, m_indexfid);
  DDX_Text(pDX, IDC_FSZ_METAFID, m_metafid);
  DDX_Text(pDX, IDC_FSZ_JOURNALFID, m_journalfid);
  DDX_Text(pDX, IDC_FSZ_JOURNALHEAD, m_journalhead);
  DDX_Text(pDX, IDC_FSZ_JOURNALTAIL, m_journaltail);
  DDX_Text(pDX, IDC_FSZ_JOURNALMAX, m_journalmax);
  //DDX_Text(pDX, IDC_FSZ_ENCRYPT, m_encrypt);
  DDX_Text(pDX, IDC_FSZ_ENCHASH, m_enchash);
  DDX_Text(pDX, IDC_FSZ_CREATEDATE, m_createdate);
  DDX_Text(pDX, IDC_FSZ_LASTMOUNTDATE, m_lastmountdate);
  DDX_Text(pDX, IDC_FSZ_LASTUNMOUNTDATE, m_lastunmountdate);
  DDX_Text(pDX, IDC_FSZ_LASTCHECKDATE, m_lastcheckdate);
  DDX_Text(pDX, IDC_FSZ_UUID, m_uuid);
  DDX_Text(pDX, IDC_FSZ_MAGIC2, m_magic2);
  DDX_Text(pDX, IDC_FSZ_CHECKSUM, m_checksum);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  DDX_Check(pDX, IDC_DEL_CLEAR, m_del_clear);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFSZ, CPropertyPage)
  //{{AFX_MSG_MAP(CFSZ)
  /*
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
  */
  ON_EN_CHANGE(IDC_FSZ_VER_MAJ, OnChangeVersion)
  ON_EN_CHANGE(IDC_FSZ_VER_MIN, OnChangeVersion)
  ON_EN_CHANGE(IDC_FSZ_LOGSEC, OnChangeLogSecSize)
  ON_EN_CHANGE(IDC_FSZ_ENCHASH, OnChangeHashType)
  ON_EN_CHANGE(IDC_FSZ_ENCTYPE, OnChangeEncryptType)
  /*
  ON_BN_CLICKED(ID_ENTRY, OnFatEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(ID_FAT32_INFO, OnFat32Info)
  ON_EN_CHANGE(IDC_FAT_FS_VERSION, OnChangeFatFsVersion)
  ON_BN_CLICKED(IDC_FAT_BACKUP_SECT_UPDATE, OnFatBackupSectUpdate)
  ON_BN_CLICKED(IDC_FAT_BACKUP_SECT_RESTORE, OnFatBackupSectRestore)
  ON_BN_CLICKED(IDC_OLD_FAT, OnOldFat)
  ON_BN_CLICKED(ID_DELETE, OnFatDelete)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_WM_HELPINFO()
  */
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFSZ message handlers
BOOL CFSZ::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  //OnChangeFatFsVersion();
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);
  
  m_show_del = AfxGetApp()->GetProfileInt("Settings", "FSZShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "FSZDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");

  return TRUE;
}

void CFSZ::OnChangeVersion() {
  CString cs_maj, cs_min;
  int maj, min;
  
  GetDlgItemText(IDC_FSZ_VER_MAJ, cs_maj);
  GetDlgItemText(IDC_FSZ_VER_MIN, cs_min);
  maj = convert8(cs_maj);
  min = convert8(cs_min);

  cs_maj.Format("%i.%i", maj, min);
  SetDlgItemText(IDC_FSZ_VERSION, cs_maj);
}

void CFSZ::OnChangeLogSecSize() {
  CString cs;
  int i;
  
  GetDlgItemText(IDC_FSZ_LOGSEC, cs);
  i = 1 << (convert8(cs) + 11);

  cs.Format("%i", i);
  SetDlgItemText(IDC_FSZ_SECTSIZE, cs);
}

void CFSZ::OnChangeHashType() {
  CString cs;
  int i;
  
  GetDlgItemText(IDC_FSZ_ENCHASH, cs);
  i = convert32(cs);
  GetDlgItem(IDC_FSZ_ENCTYPE)->EnableWindow(i > 0);
  GetDlgItem(IDC_FSZ_ENCRYPT_NAME)->EnableWindow(i > 0);
  GetDlgItem(IDC_FSZ_ENC_MASK_DO)->EnableWindow(i > 0);
  
  OnChangeEncryptType();
}

void CFSZ::OnChangeEncryptType() {
  CString cs;
  int i;

  GetDlgItemText(IDC_FSZ_ENCHASH, cs);
  i = convert32(cs);
  if (i > 0) {
    GetDlgItemText(IDC_FSZ_ENCTYPE, cs);
    i = convert8(cs);

    if (cs.GetLength() == 0)
      i = -1;

    switch (i) {
      case 0:
        SetDlgItemText(IDC_FSZ_ENCRYPT_NAME, "SHA256");
        break;
      case 1:
        SetDlgItemText(IDC_FSZ_ENCRYPT_NAME, "AES 256");
        break;
      default:
        SetDlgItemText(IDC_FSZ_ENCRYPT_NAME, "?????");
    }
  }
}

/*
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

  GetDlgItem(ID_ENTRY)->EnableWindow((hItem != NULL) && (m_dir_tree.GetParentItem(hItem) != NULL));
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(hItem != NULL);
  
  *pResult = 0;
}

void CFat::OnFatApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  ReceiveFromDialog(m_super); // bring from Dialog
  
  // update the BPB
  dlg->ReadFromFile(buffer, m_lba, 1);
  if (m_fat_size == FS_FAT32)
    memcpy(buffer, m_super, sizeof(struct S_FAT32_BPB));
  else
    memcpy(buffer, m_super, sizeof(struct S_FAT1216_BPB));
  dlg->WriteToFile(buffer, m_lba, 1);
  
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
  CString cs;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  if (!m_hard_format)
    ReceiveFromDialog(m_super); // bring from Dialog
  
  // if there are a lot of sectors, give a warning
  if (m_size > 1000000) {
    cs.Format("Found a count of %I64i sectors.  This may take a while!  Continue?", m_size);
    if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
      return FALSE;
  }
  
  // first, clear out the volume
  buffer = (BYTE *) calloc(MAX_SECT_SIZE, 1);
  for (l=0; l<m_size; l++)
    dlg->WriteToFile(buffer, m_lba + l, 1);
  
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
          dlg->WriteToFile(buffer, m_lba, (UINT) reserved);
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
  dlg->WriteToFile(buffer, m_lba, 1);
  
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
        dword[0] = 0x0FFFFFF8;  // TODO: F8 should be the media descriptor value given above. (bpb->descriptor).  We currently assume it will be F8.
        dword[1] = 0x0FFFFFFF;
        for (i=0; i<(format.m_root_entries / format.m_sect_cluster); i++)
          dword[2+i] = 3 + i;
        dword[2+i-1] = 0x0FFFFFFF;
        break;
    }
    dlg->WriteToFile(buffer, m_lba + reserved, 1);
    memset(buffer, 0, MAX_SECT_SIZE);
    for (i=1; i<sect_per_fat; i++)
      dlg->WriteToFile(buffer, m_lba + reserved + i, 1);
  }

  // Now the root.  It is written to just after the last fat
  //  no matter the fat type.
  l = reserved + (format.m_num_fats * sect_per_fat);
  memset(buffer, 0, MAX_SECT_SIZE);
  for (i=0; i<format.m_root_entries; i++)  // format.m_root_entries is in sectors
    dlg->WriteToFile(buffer, m_lba + l, 1);

  // if a FAT32, we need an info sector and a backup sector
  if (m_fat_size == FS_FAT32) {
    struct S_FAT32_BPB_INFO *info = (struct S_FAT32_BPB_INFO *) buffer;
    memset(info, 0, dlg->m_sect_size);
    info->sig = 0x41615252;
    info->struct_sig = 0x61417272;
    info->free_clust_cnt = -1;
    info->next_free_clust = (format.m_root_entries / format.m_sect_cluster) + 1;
    info->trail_sig = 0xAA550000;
    dlg->WriteToFile(buffer, m_lba + info_sect, 1);
    
    // backup_sect = convert16(m_backup_sector);
  }

  free(buffer);
  
  if (!m_hard_format)
    SendToDialog(m_super); // Send back to Dialog

  return TRUE;
}

void CFat::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_super;
  struct S_FAT1216_BPB *bpb12 = (struct S_FAT1216_BPB *) m_super;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  CString cs;
  unsigned extra = 0;
  unsigned reserved = (bpb12->sect_reserved > 0) ? bpb12->sect_reserved : 1;
  unsigned i, info_sect, backup_sect;

  BYTE *existing = (BYTE *) malloc(reserved * dlg->m_sect_size);
  BYTE *buffer = (BYTE *) calloc(reserved * dlg->m_sect_size, 1);

  UpdateData(TRUE); // receive from Dialog
  info_sect = convert16(m_info_sector);
  backup_sect = convert16(m_backup_sector);

  // first, read in what we already have
  dlg->ReadFromFile(existing, m_lba, reserved);
  
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
    dlg->WriteToFile(existing, m_lba, reserved);

    // was there any extra?
    if (extra > 0)
      dlg->WriteToFile(buffer + (reserved * dlg->m_sect_size), m_lba + reserved, extra - reserved);
  }
  
  free(buffer);
  free(existing);
}
*/

// Fat colors will have a blue shade to them.
DWORD CFSZ::GetNewColor(int index) {
  int r = ((255 - (index * 20)) > -1) ? (255 - (index * 20)) : 0;
  int g = ((51 - (index * 18)) > -1) ? (51 - (index * 18)) : 0;
  int b = ((0 - (index *  2)) > -1) ? (0 - (index *  2)) : 0;
  return RGB(r, g, b);
}

void CFSZ::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;

  //DWORD rootcluster;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  m_hard_format = FALSE;

  if (DetectFSZ() != 3) {
    AfxMessageBox("Did not find a valid FS/Z volume");
    m_isvalid = FALSE;
  }
  
  // set some flags for us
  struct S_FSZ_SUPER *super = (struct S_FSZ_SUPER *) m_super;
  m_big_inode = (super->flags & FSZ_SB_BIGINODE) > 0;
  
  // set the tab display
  dlg->m_FSZNames[index] = "FSZ";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_FSZNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->FSZ[index].m_draw_index, (char *) (LPCTSTR) dlg->m_FSZNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog(m_super);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  // load the (root) directory
  if (m_isvalid) {
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);

    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    /*
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
      m_parse_depth_limit = 0;  // start new
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
    */
  }

  Invalidate(TRUE);  // redraw the tab
}

static const DWORD crc32c_lookup[256] = {
  0x00000000, 0xf26b8303, 0xe13b70f7, 0x1350f3f4, 0xc79a971f, 0x35f1141c, 0x26a1e7e8, 0xd4ca64eb, 0x8ad958cf,
  0x78b2dbcc, 0x6be22838, 0x9989ab3b, 0x4d43cfd0, 0xbf284cd3, 0xac78bf27, 0x5e133c24, 0x105ec76f, 0xe235446c,
  0xf165b798, 0x030e349b, 0xd7c45070, 0x25afd373, 0x36ff2087, 0xc494a384, 0x9a879fa0, 0x68ec1ca3, 0x7bbcef57,
  0x89d76c54, 0x5d1d08bf, 0xaf768bbc, 0xbc267848, 0x4e4dfb4b, 0x20bd8ede, 0xd2d60ddd, 0xc186fe29, 0x33ed7d2a,
  0xe72719c1, 0x154c9ac2, 0x061c6936, 0xf477ea35, 0xaa64d611, 0x580f5512, 0x4b5fa6e6, 0xb93425e5, 0x6dfe410e,
  0x9f95c20d, 0x8cc531f9, 0x7eaeb2fa, 0x30e349b1, 0xc288cab2, 0xd1d83946, 0x23b3ba45, 0xf779deae, 0x05125dad,
  0x1642ae59, 0xe4292d5a, 0xba3a117e, 0x4851927d, 0x5b016189, 0xa96ae28a, 0x7da08661, 0x8fcb0562, 0x9c9bf696,
  0x6ef07595, 0x417b1dbc, 0xb3109ebf, 0xa0406d4b, 0x522bee48, 0x86e18aa3, 0x748a09a0, 0x67dafa54, 0x95b17957,
  0xcba24573, 0x39c9c670, 0x2a993584, 0xd8f2b687, 0x0c38d26c, 0xfe53516f, 0xed03a29b, 0x1f682198, 0x5125dad3,
  0xa34e59d0, 0xb01eaa24, 0x42752927, 0x96bf4dcc, 0x64d4cecf, 0x77843d3b, 0x85efbe38, 0xdbfc821c, 0x2997011f,
  0x3ac7f2eb, 0xc8ac71e8, 0x1c661503, 0xee0d9600, 0xfd5d65f4, 0x0f36e6f7, 0x61c69362, 0x93ad1061, 0x80fde395,
  0x72966096, 0xa65c047d, 0x5437877e, 0x4767748a, 0xb50cf789, 0xeb1fcbad, 0x197448ae, 0x0a24bb5a, 0xf84f3859,
  0x2c855cb2, 0xdeeedfb1, 0xcdbe2c45, 0x3fd5af46, 0x7198540d, 0x83f3d70e, 0x90a324fa, 0x62c8a7f9, 0xb602c312,
  0x44694011, 0x5739b3e5, 0xa55230e6, 0xfb410cc2, 0x092a8fc1, 0x1a7a7c35, 0xe811ff36, 0x3cdb9bdd, 0xceb018de,
  0xdde0eb2a, 0x2f8b6829, 0x82f63b78, 0x709db87b, 0x63cd4b8f, 0x91a6c88c, 0x456cac67, 0xb7072f64, 0xa457dc90,
  0x563c5f93, 0x082f63b7, 0xfa44e0b4, 0xe9141340, 0x1b7f9043, 0xcfb5f4a8, 0x3dde77ab, 0x2e8e845f, 0xdce5075c,
  0x92a8fc17, 0x60c37f14, 0x73938ce0, 0x81f80fe3, 0x55326b08, 0xa759e80b, 0xb4091bff, 0x466298fc, 0x1871a4d8,
  0xea1a27db, 0xf94ad42f, 0x0b21572c, 0xdfeb33c7, 0x2d80b0c4, 0x3ed04330, 0xccbbc033, 0xa24bb5a6, 0x502036a5,
  0x4370c551, 0xb11b4652, 0x65d122b9, 0x97baa1ba, 0x84ea524e, 0x7681d14d, 0x2892ed69, 0xdaf96e6a, 0xc9a99d9e,
  0x3bc21e9d, 0xef087a76, 0x1d63f975, 0x0e330a81, 0xfc588982, 0xb21572c9, 0x407ef1ca, 0x532e023e, 0xa145813d,
  0x758fe5d6, 0x87e466d5, 0x94b49521, 0x66df1622, 0x38cc2a06, 0xcaa7a905, 0xd9f75af1, 0x2b9cd9f2, 0xff56bd19,
  0x0d3d3e1a, 0x1e6dcdee, 0xec064eed, 0xc38d26c4, 0x31e6a5c7, 0x22b65633, 0xd0ddd530, 0x0417b1db, 0xf67c32d8,
  0xe52cc12c, 0x1747422f, 0x49547e0b, 0xbb3ffd08, 0xa86f0efc, 0x5a048dff, 0x8ecee914, 0x7ca56a17, 0x6ff599e3,
  0x9d9e1ae0, 0xd3d3e1ab, 0x21b862a8, 0x32e8915c, 0xc083125f, 0x144976b4, 0xe622f5b7, 0xf5720643, 0x07198540,
  0x590ab964, 0xab613a67, 0xb831c993, 0x4a5a4a90, 0x9e902e7b, 0x6cfbad78, 0x7fab5e8c, 0x8dc0dd8f, 0xe330a81a,
  0x115b2b19, 0x020bd8ed, 0xf0605bee, 0x24aa3f05, 0xd6c1bc06, 0xc5914ff2, 0x37faccf1, 0x69e9f0d5, 0x9b8273d6,
  0x88d28022, 0x7ab90321, 0xae7367ca, 0x5c18e4c9, 0x4f48173d, 0xbd23943e, 0xf36e6f75, 0x0105ec76, 0x12551f82,
  0xe03e9c81, 0x34f4f86a, 0xc69f7b69, 0xd5cf889d, 0x27a40b9e, 0x79b737ba, 0x8bdcb4b9, 0x988c474d, 0x6ae7c44e,
  0xbe2da0a5, 0x4c4623a6, 0x5f16d052, 0xad7d5351
};

// Calculate CRC using the Castagnoli method, with polynomial 0x1EDC6F41
DWORD CFSZ::crc32c_calc(void *buffer, size_t length) {
  BYTE *src = (BYTE *) buffer;

  DWORD crc32_val=0;
  while (length--)
    crc32_val = (crc32_val >> 8) ^ crc32c_lookup[ (crc32_val & 0xFF) ^ *src++];
  
  return crc32_val;
}

// return count of tests passed (should be 3, currently)
int CFSZ::DetectFSZ(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[2048];
  CString cs;
  int i, count = 0;

  // since we have an unknown block size, we need to read 2048 bytes, only.
  // We (temporarily) change the sector size to 512, so we only read 4 512-byte sectors.
  unsigned org_size = dlg->m_sect_size;
  dlg->m_sect_size = 512;
  dlg->ReadFromFile(buffer, m_lba, 4);
  dlg->m_sect_size = org_size;
  
  // now detect the file system
  struct S_FSZ_SUPER *super = (struct S_FSZ_SUPER *) buffer;

  if (super->magic == 0x5A2F5346)
    count++;

  if (super->magic2 == 0x5A2F5346)
    count++;

  if (crc32c_calc((char *) &super->magic, 508) == super->checksum)
    count++;

  // check that any of the high 8-byte parts of the 16-byte values are all zeros.
  // if any of these 16-byte values are greater than 64-bits, give an error.
  // * we don't support anything larger than 64-bit values *
  if (count == 3) {
    DQWORD *val = &super->numsec;
    for (i=0; i<8; i++) {
      if (val[i].HighPart != 0) {
        AfxMessageBox("FSZ: A 128-bit super member is larger than 64-bits.");
        break;
      }
    }

  //  DWORD t = crc32c_calc((char *) &super->magic, 508);
  //  cs.Format(" 0x%08X   0x%08X ", super->checksum, t);
  //  AfxMessageBox(cs);

    // found valid FS/Z volume
    if (super->logsec <= 5)
      m_block_size = 1 << (super->logsec + 11);
    else {
      cs.Format("Log Sector Size is out of range at %i.\nShould be 0 -> 5. Setting to a default of 4096.", super->logsec);
      AfxMessageBox(cs);
      m_block_size = 4096;  // default to 4096 if out of range
      super->logsec = 1;
    }

    m_super = malloc(2048);
    memcpy(m_super, buffer, 2048);
  }

  return count;
}

/*
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
      if (hItem == NULL) { m_too_many = TRUE; m_parse_depth_limit--; return TRUE; }
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
          if (hItem == NULL) { m_too_many = TRUE; m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, 0, filesize, &root[i], i, cnt, 0, FALSE);
        }
      } else {
        if (attrb & FAT_ATTR_SUB_DIR) {
          if (attrb & FAT_ATTR_HIDDEN)
            hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER_HIDDEN, IMAGE_FOLDER_HIDDEN, parent);
          else
            hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
          if (hItem == NULL) { m_too_many = TRUE; m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, start, filesize, &root[i], i, cnt, 0, !IsDot);
          if (!IsDot) {
            sub = (struct S_FAT_ROOT *) ReadFile(start, &filesize, FALSE);
            if (sub) {
              if (!ParseDir(sub, (filesize + 31) / 32, hItem, FALSE)) {
                free(sub);
                m_parse_depth_limit--;
                return FALSE;
              }
              free(sub);
            }
          }
        } else if (attrb & FAT_ATTR_VOLUME) {
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_LABEL, IMAGE_LABEL, parent);
          if (hItem == NULL) { m_too_many = TRUE; m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, 0, filesize, &root[i], i, cnt, 0, FALSE);
        } else {
          if (attrb & FAT_ATTR_HIDDEN)
            hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE_HIDDEN, IMAGE_FILE_HIDDEN, parent);
          else
            hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
          if (hItem == NULL) { m_too_many = TRUE; m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, start, filesize, &root[i], i, cnt, 0, TRUE);
        }
      }
    }
    i += cnt;
  }

  m_parse_depth_limit--;
  return TRUE;
}

// 'size' is only valid if IsRoot > 0
void *CFat::ReadFile(DWORD cluster, DWORD *size, BOOL IsRoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *ptr = NULL;
  unsigned pos = 0, mem_size = 0;
  
  if (IsRoot && (m_fat_size != FS_FAT32)) {
    ptr = calloc(*size + (MAX_SECT_SIZE - 1), 1);
    if (ptr)
      dlg->ReadFromFile(ptr, m_lba + m_rootstart, (*size + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect);
  } else {
    struct S_FAT_ENTRIES ClusterList;
    FatFillClusterList(&ClusterList, cluster);
    ptr = malloc(ClusterList.entry_count * (bpb->bytes_per_sect * bpb->sect_per_clust));
    for (int i=0; i<ClusterList.entry_count; i++) {
      dlg->ReadFromFile((BYTE *) ptr + pos, m_lba + m_datastart + ((ClusterList.entries[i] - 2) * bpb->sect_per_clust), bpb->sect_per_clust);
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
    dlg->WriteToFile(buffer, m_lba + m_rootstart, (size + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect);
  } else {
    for (int i=0; i<ClusterList->entry_count; i++) {
      dlg->WriteToFile((BYTE *) buffer + pos, m_lba + m_datastart + ((ClusterList->entries[i] - 2) * bpb->sect_per_clust), bpb->sect_per_clust);
      pos += (bpb->bytes_per_sect * bpb->sect_per_clust);
    }
  }
}

// write zeros to this cluster
void CFat::ZeroCluster(DWORD Cluster) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *zero = (void *) calloc((size_t) bpb->sect_per_clust * (size_t) bpb->bytes_per_sect, 1);

  dlg->WriteToFile((BYTE *) zero, m_lba + m_datastart + ((Cluster - 2) * (size_t) bpb->sect_per_clust), bpb->sect_per_clust);
  
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
    dlg->ReadFromFile(fat_buffer, m_lba + bpb32->sect_reserved, bpb32->sect_per_fat32);
  } else {
    fat_buffer = malloc(bpb12->sect_per_fat * bpb12->bytes_per_sect);
    dlg->ReadFromFile(fat_buffer, m_lba + bpb12->sect_reserved, bpb12->sect_per_fat);
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
  CWaitCursor wait; // display a wait cursor
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
      dlg->WriteToFile(m_fat_buffer, m_lba + bpb32->sect_reserved + l, bpb32->sect_per_fat32);
      l += bpb32->sect_per_fat32;
    }
  } else {
    struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
    for (int i=0; i<bpb->fats; i++) {
      dlg->WriteToFile(m_fat_buffer, m_lba + bpb->sect_reserved + l, bpb->sect_per_fat);
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
*/

// update all items in dialog
void CFSZ::SendToDialog(void *ptr) {
  if (ptr == NULL)
    return;
  
  struct S_FSZ_SUPER *super = (struct S_FSZ_SUPER *) ptr;

  m_magic.Format("%c%c%c%c", (super->magic & 0x000000FF) >> 0, (super->magic & 0x0000FF00) >> 8, (super->magic & 0x00FF0000) >> 16, (super->magic & 0xFF000000) >> 24);
  m_version_major.Format("%i", super->version_major);
  m_version_minor.Format("%i", super->version_minor);
  m_logsec.Format("%i", super->logsec);
  m_flags.Format("0x%08X", super->flags);
  m_maxmounts.Format("%i", super->maxmounts);
  m_currmounts.Format("%i", super->currmounts);
  m_numsec.Format("%I64i", super->numsec.LowPart);
  m_freesec.Format("%I64i", super->freesec.LowPart);
  m_freesecfid.Format("%I64i", super->freesecfid.LowPart);
  m_rootfid.Format("%I64i", super->rootdirfid.LowPart);
  m_badsecfid.Format("%I64i", super->badsecfid.LowPart);
  m_indexfid.Format("%I64i", super->indexfid.LowPart);
  m_metafid.Format("%I64i", super->metafid.LowPart);
  m_journalfid.Format("%I64i", super->journalfid.LowPart);
  m_journalhead.Format("%I64i", super->journalhead);
  m_journaltail.Format("%I64i", super->journaltail);
  m_journalmax.Format("%I64i", super->journalmax);
  m_enctype.Format("%i", super->enctype);
  m_enchash.Format("0x%08X", super->enchash);
  m_createdate.Format("%I64i", super->createdate);
  m_lastmountdate.Format("%I64i", super->lastmountdate);
  m_lastunmountdate.Format("%I64i", super->lastunmountdate);
  m_lastcheckdate.Format("%I64i", super->lastcheckdate);
  GUID_Format(m_uuid, &super->uuid);
  m_magic2.Format("%c%c%c%c", (super->magic2 & 0x000000FF) >> 0, (super->magic2 & 0x0000FF00) >> 8, (super->magic2 & 0x00FF0000) >> 16, (super->magic2 & 0xFF000000) >> 24);
  m_checksum.Format("0x%08X", super->checksum);
  
  UpdateData(FALSE); // send to Dialog

  OnChangeVersion();
  OnChangeLogSecSize();
  OnChangeHashType();
  OnChangeEncryptType();
}

/*
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
      FALSE,            // Create a saveas file dialog
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
    if (m_fat_size == FS_FAT32)
      cluster |= (root[items->Index + i - 1].type.fat32.strtclst32 << 16);

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
      if (items->Cluster > 0) {
        CWaitCursor wait;  // creates and changes to a wait cursor (automatically changes back on destroy())
        FatFillClusterList(&FatEntry.m_fat_entries, items->Cluster);
      }
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
        items->Entry[sfn_index].strtclst = convert16(FatEntry.m_cluster) & 0xFFFF;
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
  dlg->ReadFromFile(buffer, m_lba + info_sector, 1);
  
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
    dlg->WriteToFile(buffer, m_lba + info_sector, 1);
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
    CWaitCursor wait; // display a wait cursor
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1);
    wait.Restore(); // unnecassary since the 'destroy' code will restore it, but just to make sure.
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}
*/
