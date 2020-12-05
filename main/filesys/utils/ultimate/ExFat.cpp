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

// ExFat.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "ExFat.h"
#include "ExFatEntry.h"
#include "ExFatUcaseTab.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExFat property page

IMPLEMENT_DYNCREATE(CExFat, CPropertyPage)

CExFat::CExFat() : CPropertyPage(CExFat::IDD) {
  //{{AFX_DATA_INIT(CExFat)
  m_data_region_lba = _T("");
  m_data_region_size = _T("");
  m_drive_sel = _T("");
  m_fats = _T("");
  m_first_fat = _T("");
  m_flags = _T("");
  m_fs_version = _T("");
  m_jmp0 = _T("");
  m_jmp1 = _T("");
  m_jmp2 = _T("");
  m_log_bytes_sect = _T("");
  m_log_bytes_cluster = _T("");
  m_oem_name = _T("");
  m_part_offset = _T("");
  m_percent_heap = _T("");
  m_reserved0 = _T("");
  m_reserved1 = _T("");
  m_root_cluster = _T("");
  m_sect_fat = _T("");
  m_serial_number = _T("");
  m_total_sectors = _T("");
  m_show_del = FALSE;
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_fat_buffer = NULL;
  m_vbr_buffer = NULL;

  m_bp_buffer = NULL;
  m_bp_cluster = 0;
  m_bp_size = 0;
  m_bp_flags = 0;

  m_hard_format = FALSE;
}

CExFat::~CExFat() {
  if (m_fat_buffer)
    free(m_fat_buffer);
  if (m_vbr_buffer)
    free(m_vbr_buffer);
  if (m_bp_buffer)
    free(m_bp_buffer);
  
  m_fat_buffer = NULL;
  m_vbr_buffer = NULL;
  m_bp_buffer = NULL;
}

void CExFat::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExFat)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_EXFAT_DATA_REGION_LBA, m_data_region_lba);
  DDX_Text(pDX, IDC_EXFAT_DATA_REGION_SIZE, m_data_region_size);
  DDX_Text(pDX, IDC_EXFAT_DRIVE_SEL, m_drive_sel);
  DDX_Text(pDX, IDC_EXFAT_FATS, m_fats);
  DDX_Text(pDX, IDC_EXFAT_FIRST_FAT, m_first_fat);
  DDX_Text(pDX, IDC_EXFAT_FLAGS, m_flags);
  DDX_Text(pDX, IDC_EXFAT_FS_VERSION, m_fs_version);
  DDX_Text(pDX, IDC_EXFAT_JMP0, m_jmp0);
  DDX_Text(pDX, IDC_EXFAT_JMP1, m_jmp1);
  DDX_Text(pDX, IDC_EXFAT_JMP2, m_jmp2);
  DDX_Text(pDX, IDC_EXFAT_LOG_BYTES_SECT, m_log_bytes_sect);
  DDX_Text(pDX, IDC_EXFAT_LOG_SECT_CLUSTER, m_log_bytes_cluster);
  DDX_Text(pDX, IDC_EXFAT_OEM_NAME, m_oem_name);
  DDV_MaxChars(pDX, m_oem_name, 8);
  DDX_Text(pDX, IDC_EXFAT_PART_OFFSET, m_part_offset);
  DDX_Text(pDX, IDC_EXFAT_PERCENT_HEAP, m_percent_heap);
  DDX_Text(pDX, IDC_EXFAT_RESERVED0, m_reserved0);
  DDX_Text(pDX, IDC_EXFAT_RESERVED1, m_reserved1);
  DDX_Text(pDX, IDC_EXFAT_ROOT_CLUSTER, m_root_cluster);
  DDX_Text(pDX, IDC_EXFAT_SECT_FAT, m_sect_fat);
  DDX_Text(pDX, IDC_EXFAT_SERIAL_NUM, m_serial_number);
  DDX_Text(pDX, IDC_EXFAT_TOTAL_SECTORS, m_total_sectors);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExFat, CPropertyPage)
  //{{AFX_MSG_MAP(CExFat)
  ON_WM_HELPINFO()
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_APPLY, OnExFatApply)
  ON_BN_CLICKED(ID_CLEAN, OnExFatClean)
  ON_BN_CLICKED(ID_FORMAT, OnExFatFormat)
  ON_BN_CLICKED(ID_CHECK, OnExFatCheck)
  ON_BN_CLICKED(ID_COPY, OnExFatCopy)
  ON_BN_CLICKED(ID_INSERT, OnExFatInsert)
  ON_BN_CLICKED(ID_ENTRY, OnExFatEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_EN_CHANGE(IDC_EXFAT_FS_VERSION, OnChangeExFatFsVersion)
  ON_BN_CLICKED(IDC_SERIAL_UPDATE, OnSerialUpdate)
  ON_BN_CLICKED(ID_DELETE, OnExFatDelete)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_EN_CHANGE(IDC_EXFAT_LOG_BYTES_SECT, OnChangeExfatLogBytesSect)
  ON_EN_CHANGE(IDC_EXFAT_LOG_SECT_CLUSTER, OnChangeExfatLogSectCluster)
  ON_BN_CLICKED(IDFLAGS, OnFlags)
  ON_BN_CLICKED(ID_EXFAT_RESTORE_BACKUP, OnExfatRestoreBackup)
  ON_BN_CLICKED(ID_EXFAT_UPDATE_BACKUP, OnExfatUpdateBackup)
  ON_BN_CLICKED(ID_EXFAT_PARAM_SECT, OnExfatParamSect)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExFat message handlers
BOOL CExFat::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_show_del = AfxGetApp()->GetProfileInt("Settings", "exFATShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "exFATDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  return TRUE;
}

BOOL CExFat::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "exfat.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CExFat::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_EXFAT_ITEMS *items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(hItem != NULL);
  
  *pResult = 0;
}

// Lean colors will have a red shade to them.
DWORD CExFat::GetNewColor(int index) {
  int r = ((150 - (index *  2)) > -1) ? (150 - (index *  2)) : 0;
  int g = ((100 - (index * 18)) > -1) ? (100 - (index * 18)) : 0;
  int b = (( 96 - (index * 20)) > -1) ? ( 96 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CExFat::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  unsigned vbr_sect_size, i;
  DWORD crc, *p;
  CString cs;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  // read 12 sectors so we get all of the Primary VBR
  m_vbr_buffer = malloc(MAX_SECT_SIZE * 12);
  dlg->ReadFromFile(m_vbr_buffer, lba, 12, FALSE);
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  
  dlg->m_ExFatNames[index] = "ExFAT";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_ExFatNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->ExFat[index].m_draw_index, (char *) (LPCTSTR) dlg->m_ExFatNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog((struct S_EXFAT_VBR *) m_vbr_buffer);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  // check that the sector size of the VBR matches our Dialog's sector size
  vbr_sect_size = (1 << vbr->log_bytes_per_sect);
  if (vbr_sect_size != dlg->m_sect_size) {
    cs.Format("VBR sector size (%i) doesn't equal our dialog sector size (%i)...", vbr_sect_size, dlg->m_sect_size);
    AfxMessageBox(cs);
    m_isvalid = FALSE;
  }  
  
  // check the CRC for the Primary VBR
  crc = VBRChecksum(m_vbr_buffer, 11 * vbr_sect_size);
  p = (DWORD *) ((BYTE *) m_vbr_buffer + (11 * vbr_sect_size));
  BOOL notequal = FALSE;
  for (i=0; i<vbr_sect_size / 4; i++)
    notequal |= (p[i] != crc);
  if (notequal) {
    AfxMessageBox(" ExFAT:\r\n*** CRC sector does not check ***\r\nThe Apply button will write a new crc, then reload the image.");
    m_isvalid = FALSE;
  }
  
  if (m_isvalid) {
    // load the fat
    m_fat_buffer = ExFatLoadFAT(m_fat_buffer);
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    
    // load the bitmap
    m_bp_buffer = ExFatLoadBP(m_bp_buffer);
    
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    struct S_EXFAT_ROOT *root;
    DWORD64 rootsize = 0;
    root = (struct S_EXFAT_ROOT *) ReadFile(vbr->root_dir_cluster, &rootsize, 0);
    if (root) {
      SaveItemInfo(m_hRoot, NULL, vbr->root_dir_cluster, rootsize, 0, 0, 0, FALSE);
      CWaitCursor wait; // display a wait cursor
      m_too_many = FALSE;
      ParseDir(root, rootsize, m_hRoot);
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

void CExFat::ParseDir(struct S_EXFAT_ROOT *root, DWORD64 root_size, HTREEITEM parent) {
  struct S_EXFAT_ROOT *cur = root;
  int  name_len, secondary_count, j;
  DWORD64 filesize;
  DWORD cluster, CurOffset;
  BYTE flags;
  HTREEITEM hItem;
  WORD namestr[256];
  CString name;
  
  while (((BYTE *) cur < ((BYTE *) root + root_size)) && !m_too_many) {
    CurOffset = (DWORD) ((BYTE *) cur - (BYTE *) root);
    switch (cur->entry_type) {
      case EXFAT_DIR_EOD:  // Entry type: End of Directory
        return;
      case EXFAT_DIR_BITMAP: // Entry type: Bitmap
        hItem = m_dir_tree.Insert("Bitmap Entry", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, cur[0].type.bitmap.cluster_strt, cur[0].type.bitmap.data_len, cur[0].type.bitmap.flags, CurOffset, 0, FALSE);
        break;
      case EXFAT_DIR_UCASE:  // Entry type: UCase Table
        hItem = m_dir_tree.Insert("UCase Entry", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, cur[0].type.up_case_table.cluster_strt, cur[0].type.up_case_table.data_len, 0, CurOffset, 0, FALSE);
        break;
      case EXFAT_DIR_LABEL:  // Entry type: Label
        name_len = cur->type.label.len;
        for (j=0; j<name_len && j<11; j++)
          namestr[j] = cur->type.label.name[j];
        namestr[j] = '\0';
        WideCharToMultiByte(CP_ACP, 0, (LPCWCH) namestr, -1, name.GetBuffer(256), 256, NULL, NULL); name.ReleaseBuffer(256);
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_LABEL, IMAGE_LABEL, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, 0, 0, 0, CurOffset, 0, FALSE);
        break;
      case EXFAT_DIR_DELETED_S: // deleted entry
      case EXFAT_DIR_DELETED: // deleted entry
      case EXFAT_DIR_ENTRY:  // Entry type: Directory Entry
        secondary_count = cur->type.dir_entry.sec_count;
        if (((secondary_count >= EXFAT_MIN_SECONDARY_COUNT) && (secondary_count <= EXFAT_MAX_SECONDARY_COUNT)) &&
            (ExFatCheckDirCRC((BYTE *) cur, ((secondary_count + 1) * sizeof(struct S_EXFAT_ROOT))) == cur->type.dir_entry.crc)) {
          if (cur[1].entry_type == EXFAT_DIR_STRM_EXT) {
            cluster = cur[1].type.stream_ext.first_cluster;
            filesize = cur[1].type.stream_ext.valid_data_len;
            flags = cur[1].type.stream_ext.flags;
            name_len = cur[1].type.stream_ext.name_len;
            memset(namestr, 0, 256);
            int npos = 0;
            // Get the name
            for (int s=2; s<=secondary_count && s<(EXFAT_MAX_SECONDARY_COUNT - 3); s++) {
              if (cur[s].entry_type == EXFAT_DIR_NAME_EXT) {
                for (int nn=0; nn<15 && npos<255; nn++) {
                  namestr[npos++] = cur[s].type.file_name_ext.name[nn];
                  if (npos == name_len) {
                    s = secondary_count;
                    break;
                  }
                }
              //} else {
              //  AfxMessageBox("Didn't find valid filename extension entry(s) after stream entry");
              //  return;
              }
            }
            namestr[npos] = '\0';
            WideCharToMultiByte(CP_ACP, 0, (LPCWCH) namestr, -1, name.GetBuffer(256), 256, NULL, NULL); name.ReleaseBuffer(256);
            if (cur->entry_type == EXFAT_DIR_DELETED_S) {
              if (IsDlgButtonChecked(IDC_SHOW_DEL)) {
                hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
                if (hItem == NULL) { m_too_many = TRUE; return; }
                SaveItemInfo(hItem, parent, cluster, filesize, flags, CurOffset, 0, FALSE);
              }
            } else {
              if (cur->type.dir_entry.attributes & EXFAT_ATTR_SUB_DIR) {
                hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
                if (hItem == NULL) { m_too_many = TRUE; return; }
                BOOL IsDot = ((name == ".") || (name == ".."));
                SaveItemInfo(hItem, parent, cluster, filesize, flags, CurOffset, 0, !IsDot);
                if (!IsDot) {
                  DWORD64 size = filesize;
                  struct S_EXFAT_ROOT *sub = (struct S_EXFAT_ROOT *) ReadFile(cluster, &size, flags);
                  if (sub) {
                    ParseDir(sub, size, hItem);
                    free(sub);
                  }
                }
              } else {
                hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
                if (hItem == NULL) { m_too_many = TRUE; return; }
                SaveItemInfo(hItem, parent, cluster, filesize, flags, CurOffset, 0, TRUE);
              }
            }
          } //else
            //AfxMessageBox("Didn't find stream extension entry after directory entry");
          cur += secondary_count;
        }
        break;
      case EXFAT_DIR_WINCE_ACC:  // Entry type: 
        hItem = m_dir_tree.Insert("TODO: WinCE_ACC", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, 0, 0, 0, CurOffset, 0, FALSE);
        break;
      case EXFAT_DIR_GUID:   // Entry type: 
        hItem = m_dir_tree.Insert("GUID Entry", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, 0, 0, 0, CurOffset, 0, FALSE);
        break;
      case EXFAT_DIR_TEXFAT: // Entry type: 
        hItem = m_dir_tree.Insert("TODO: TEXFAT", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, 0, 0, 0, CurOffset, 0, FALSE);
        break;
      default:
        name.Format("TODO: type = 0x%02X", cur->entry_type);
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, parent, 0, 0, 0, 0, 0, FALSE);
    }
    cur++;
  }
}

// if flags & EXFAT_FLAGS_NO_FAT, *size = size of file/root
// else, *size = buffer to store size read
void *CExFat::ReadFile(DWORD cluster, DWORD64 *size, BYTE flags) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  DWORD clust = cluster;
  DWORD root_size = 0;  // in clusters
  void *buffer = NULL;
  
  // if no fat, it starts at 'cluster' and then is consecutive on the media
  if (flags & EXFAT_FLAGS_NO_FAT) {
    buffer = malloc((size_t) (*size + (bytes_per_sect - 1)));
    root_size = (DWORD) ((*size + (bytes_per_sect - 1)) / bytes_per_sect);
    dlg->ReadFromFile(buffer, m_lba + ((cluster-2) * sect_per_clust) + vbr->data_region_lba, root_size, FALSE);
  } else {
    // find size of file
    do {
      clust = ExfatGetNextSect(clust);
      root_size++;
    } while (clust < EXFAT_FAT_BAD);
    
    buffer = malloc(root_size * sect_per_clust * bytes_per_sect);
    BYTE *ptr = (BYTE *) buffer;
    clust = cluster;
    do {
      dlg->ReadFromFile(ptr, m_lba + ((clust-2) * sect_per_clust) + vbr->data_region_lba, sect_per_clust, FALSE);
      ptr += (sect_per_clust * bytes_per_sect);
      clust = ExfatGetNextSect(clust);
    } while (clust < EXFAT_FAT_BAD);
    if (size) *size = (root_size * sect_per_clust * bytes_per_sect);
  }
  
  return buffer;
}

void CExFat::ZeroCluster(DWORD Cluster) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  void *zero = calloc(sect_per_clust * bytes_per_sect, 1);
  
  dlg->WriteToFile(zero, m_lba + ((Cluster-2) * sect_per_clust) + vbr->data_region_lba, sect_per_clust, FALSE);

  free(zero);
}

void CExFat::WriteFile(void *buffer, DWORD cluster, DWORD64 size, BYTE flags) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  
  // if no fat, it starts at 'cluster' and then is consecutive on the media
  if (flags & EXFAT_FLAGS_NO_FAT) {
    DWORD count = (DWORD) ((size + (bytes_per_sect - 1)) / bytes_per_sect);
    dlg->WriteToFile(buffer, m_lba + ((cluster-2) * sect_per_clust) + vbr->data_region_lba, count, FALSE);
  } else {
    BYTE *ptr = (BYTE *) buffer;
    do {
      dlg->WriteToFile(ptr, m_lba + ((cluster-2) * sect_per_clust) + vbr->data_region_lba, sect_per_clust, FALSE);
      ptr += (sect_per_clust * bytes_per_sect);
      cluster = ExfatGetNextSect(cluster);
    } while (cluster < EXFAT_FAT_BAD);
  }
}

DWORD CExFat::ExfatGetNextSect(DWORD cluster) {
  BYTE *fat = (BYTE *) m_fat_buffer;
  
  fat += (cluster * sizeof(DWORD));
  DWORD newclust = * ((DWORD *) fat);
  if (newclust >= EXFAT_FAT_BAD) newclust = EXFAT_FAT_LAST;
  
  return newclust;
}

// calculate the CRC of the first 11 sectors
DWORD CExFat::VBRChecksum(const void *buffer, const int len) {
  const BYTE *ptr = (const BYTE *) buffer;
  DWORD crc = 0;
  
  for (int i=0; i<len; i++) {
    if ((i == 106) || (i == 107) || (i == 112))
      continue;
    crc = ((crc << 31) | (crc >> 1)) + (DWORD) ptr[i];
  }
  return crc;
}

WORD CExFat::ExFatCheckDirCRC(BYTE *buffer, const int len) {
  WORD crc = 0;
  
  for (int i=0; i < len; i++) {
    // skip crc
    if ((i == 2) || (i == 3))
      continue;
    crc = ((crc << 15) | (crc >> 1)) + (WORD) buffer[i];
  }
  
  return crc;
}

// allocate a list of clusters or one cluster and so many bits in the bitmap
int CExFat::AllocateFAT(struct S_FAT_ENTRIES *EntryList, DWORD size, BYTE *flags) {
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  const unsigned bytes_per_clust = sect_per_clust * bytes_per_sect;
  const DWORD total_clusters = vbr->data_region_size;
  BYTE retflags = 0;
  
  if (!m_bp_buffer)
    return FALSE;

  // see if we can find enough consecutive bits in the bitmap
  int count = (size + (bytes_per_clust - 1)) / bytes_per_clust;

  EntryList->was_error = FALSE;
  EntryList->entry_count = 0;
  EntryList->entry_size = count;
  EntryList->entries = (DWORD *) malloc(count * sizeof(DWORD));

  BYTE *bp = (BYTE *) m_bp_buffer;
  DWORD pos = 2;
  int i = 0;
  while (pos < total_clusters) {
    if ((bp[pos / 8] & (1 << (pos % 8))) == 0) {
      EntryList->entries[i] = pos;
      i++;
    }
    pos++;
    if (i >= count)
      break;
  }

  // if at end of count they are all consecutive, simply take the first one and return flags = EXFAT_FLAGS_NO_FAT
  bool consecutive = TRUE;
  for (i=1; i<count; i++) {
    if (EntryList->entries[i] != (EntryList->entries[i-1] + 1)) {
      consecutive = FALSE;
      break;
    }
  }
  if (consecutive) {
    retflags = EXFAT_FLAGS_NO_FAT;
    for (i=0; i<count; i++)
      ExFATMarkBP(EntryList->entries[i], TRUE);
    count = 1;
  } else {
    // Mark the FAT
    for (i=0; i<count; i++) {
      ExFATMarkFAT(EntryList->entries[i], EntryList->entries[i+1]);
      ExFATMarkBP(EntryList->entries[i], TRUE);
    }
    ExFATMarkFAT(EntryList->entries[i-1], EXFAT_FAT_LAST);
  }
  
  if (flags) *flags = retflags;
  EntryList->entry_count = count;

  return FALSE;
}

int CExFat::ExFatFillClusterList(struct S_FAT_ENTRIES *EntryList, DWORD StartCluster, BYTE Flags) {
  EntryList->was_error = FALSE;
  EntryList->entry_count = 0;
  EntryList->entry_size = 128;
  EntryList->entries = (DWORD *) malloc(128 * sizeof(DWORD));
  
  if (Flags & EXFAT_FLAGS_NO_FAT) {
    EntryList->entries[0] = StartCluster;
    EntryList->entry_count = 1;
  } else {
    while (StartCluster < EXFAT_FAT_LAST) {
      // check to see if this cluster number is already in the list.
      // if so, we have an error.
      for (int i=0; i<EntryList->entry_count; i++) {
        if (EntryList->entries[i] == StartCluster) {
          EntryList->was_error = TRUE;
          AfxMessageBox("Bad Cluster list... (Error: ExFAT0001)");
          return EntryList->entry_count;
        }
      }
      if (EntryList->entry_count >= EntryList->entry_size) {
        EntryList->entry_size += 128;
        EntryList->entries = (DWORD *) realloc(EntryList->entries, EntryList->entry_size * sizeof(DWORD));
      }
      EntryList->entries[EntryList->entry_count] = StartCluster;
      EntryList->entry_count++;
      StartCluster = ExfatGetNextSect(StartCluster);
    }
  }
  return EntryList->entry_count;
}

// returns TRUE if we modified the m_fat_buffer and/or bitmap
bool CExFat::ExFatClearClusterChain(DWORD Cluster, DWORD64 datalen, BYTE Flags) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bp = (BYTE *) m_bp_buffer;
  bool ret = FALSE;

  if (Flags & EXFAT_FLAGS_NO_FAT) {
    if (m_bp_buffer) {
      int count = (int) ((datalen + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
      for (int i=0; i<count; i++, Cluster++) {
        if (m_del_clear)
          ZeroCluster(Cluster);
        bp[Cluster / 8] &= ~(1 << (Cluster % 8));
      }
      ret = TRUE;
    }
  } else {
    while (Cluster < EXFAT_FAT_BAD) {
      if (m_del_clear)
        ZeroCluster(Cluster);
      if (m_bp_buffer)
        bp[Cluster / 8] &= ~(1 << (Cluster % 8));
      BYTE *fat = (BYTE *) m_fat_buffer;
      fat += (Cluster * sizeof(DWORD));
      Cluster = * ((DWORD *) fat);
      * ((DWORD *) fat) = 0;
    }
    ret = TRUE;
  }

  return ret;
}

void CExFat::ExFATMarkFAT(DWORD Cluster, DWORD Value) {
  if (m_fat_buffer) {
    BYTE *fat = (BYTE *) m_fat_buffer;
    fat += (Cluster * sizeof(DWORD));
    * ((DWORD *) fat) = Value;
  }
}

void CExFat::ExFATMarkBP(DWORD Cluster, bool Mark) {
  BYTE *bp = (BYTE *) m_bp_buffer;
  if (m_bp_buffer) {
    if (Mark)
      bp[Cluster / 8] |= (1 << (Cluster % 8));
    else
      bp[Cluster / 8] &= ~(1 << (Cluster % 8));
  }
}

void CExFat::AllocateRoot(CString csName, struct S_EXFAT_ITEMS *RootItems, DWORD StartCluster, DWORD FileSize, BYTE flags, bool isFolder) {
  struct S_EXFAT_ROOT *entry;
  DWORD64 rootsize = RootItems->FileSize;
  bool eod = FALSE;
  
  // count of consecutive entries needed
  // 1 for stream_ext
  // 1 for every 15 chars of name (file_name_ext)
  int secondary_count = 1 + ((csName.GetLength() + (EXFAT_MAX_NAME_PER_ENTRY - 1)) / EXFAT_MAX_NAME_PER_ENTRY);
  int i, w;
  
  void *root = (struct S_EXFAT_ROOT *) ReadFile(RootItems->Cluster, &rootsize, RootItems->Flags);
  if (root) {
    entry = (struct S_EXFAT_ROOT *) root;
    while ((BYTE *) entry < ((BYTE *) root + rootsize)) {
      if (entry->entry_type == EXFAT_DIR_EOD) {
        eod = TRUE;
        break;
      }
      if ((entry->entry_type == EXFAT_DIR_DELETED_S) && (entry->type.dir_entry.sec_count >= secondary_count))
        break;
      entry++;
    }
    
    CTime time = CTime::GetCurrentTime();
    unsigned t = ((((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay()) << 16) |
                 ((time.GetHour() << 11) | (time.GetMinute() << 5) | (time.GetSecond() >> 1));

    // entry now points to an available set of entries
    memset(&entry[0], 0, 32);
    entry[0].entry_type = EXFAT_DIR_ENTRY;
    entry[0].type.dir_entry.sec_count = secondary_count;
    entry[0].type.dir_entry.attributes = (isFolder) ? EXFAT_ATTR_SUB_DIR : EXFAT_ATTR_ARCHIVE;
    entry[0].type.dir_entry.created = t;
    entry[0].type.dir_entry.last_mod = t;
    entry[0].type.dir_entry.last_acc = t;
    entry[0].type.dir_entry.created_ms = 0;
    entry[0].type.dir_entry.last_mod_ms = 0;
    entry[0].type.dir_entry.created_tz_offset = 0xE4;  // -7
    entry[0].type.dir_entry.last_mod_tz_offset = 0xE4;  // -7
    entry[0].type.dir_entry.last_access_tz_offset = 0xE4;  // -7

    WORD namestr[256];
    MultiByteToWideChar(CP_ACP, 0, csName, -1, (LPWSTR) namestr, 256);
    
    memset(&entry[1], 0, 32);
    entry[1].entry_type = EXFAT_DIR_STRM_EXT;
    entry[1].type.stream_ext.flags = flags;
    entry[1].type.stream_ext.name_len = csName.GetLength();
    entry[1].type.stream_ext.name_hash = exFATNameHash(namestr, csName.GetLength());
    entry[1].type.stream_ext.valid_data_len = FileSize;
    entry[1].type.stream_ext.first_cluster = StartCluster;
    entry[1].type.stream_ext.data_len = FileSize;

    i = 2;
    w = 0;
    while (i < (secondary_count + 1)) {
      memset(&entry[i], 0, 32);
      entry[i].entry_type = EXFAT_DIR_NAME_EXT;
      entry[i].type.file_name_ext.flags = 0;
      for (int j=0; j<EXFAT_MAX_NAME_PER_ENTRY; j++) {
        entry[i].type.file_name_ext.name[j] = namestr[w];
        w++;
        if (w >= csName.GetLength())
          break;
      }
      i++;
    }

    // do the CRC for the entry(s)
    entry[0].type.dir_entry.crc = ExFatCheckDirCRC((BYTE *) &entry[0], ((secondary_count + 1) * sizeof(struct S_EXFAT_ROOT)));

    // if eod, then we need to add eod to end of our entries
    if (eod) {
      memset(&entry[i], 0, 32);
      entry[i].entry_type = EXFAT_DIR_EOD;
    }
    
    // write the root back to the file image
    WriteFile(root, RootItems->Cluster, rootsize, RootItems->Flags);
    free(root);
  }
}

// https://docs.microsoft.com/en-us/windows/win32/fileio/exfat-specification#764-namehash-field
WORD CExFat::exFATNameHash(WORD *Name, int Length) {
  UCHAR *Buffer = (UCHAR *) Name;
  int    NumberOfBytes = Length * 2;
  WORD   Hash = 0;

  for (int Index=0; Index<NumberOfBytes; Index++)
    Hash = ((Hash & 1) ? 0x8000 : 0x0000) + (Hash >> 1) + (WORD) Buffer[Index];
  
  return Hash;
}

void CExFat::SendToDialog(struct S_EXFAT_VBR *vbr) {
  m_data_region_lba.Format("%i", vbr->data_region_lba);
  m_data_region_size.Format("%i", vbr->data_region_size);
  m_drive_sel.Format("0x%02X", vbr->drive_sel);
  m_fats.Format("%i", vbr->num_fats);
  m_first_fat.Format("%i", vbr->first_fat);
  m_flags.Format("0x%04X", vbr->flags);
  m_fs_version.Format("0x%04X", vbr->fs_version);
  m_jmp0.Format("0x%02X", vbr->jmp[0]);
  m_jmp1.Format("0x%02X", vbr->jmp[1]);
  m_jmp2.Format("0x%02X", vbr->jmp[2]);
  m_log_bytes_sect.Format("%i", vbr->log_bytes_per_sect);
  m_log_bytes_cluster.Format("%i", vbr->log_sects_per_clust);
  m_oem_name.Format("%c%c%c%c%c%c%c%c", vbr->oem_name[0], vbr->oem_name[1], vbr->oem_name[2], vbr->oem_name[3],
    vbr->oem_name[4], vbr->oem_name[5], vbr->oem_name[6], vbr->oem_name[7]);
  m_part_offset.Format("%I64i", vbr->part_offset);
  m_percent_heap.Format("%i", vbr->percent_heap);
  //m_reserved0.Format("%i", );
  //m_reserved1.Format("%i", );
  m_root_cluster.Format("%i", vbr->root_dir_cluster);
  m_sect_fat.Format("%i", vbr->sect_per_fat);
  m_serial_number.Format("0x%08X", vbr->serial);
  m_total_sectors.Format("%I64i", vbr->total_sectors);
  //BYTE    reserved0[53];
  //BYTE    reserved1[7];
  
  UpdateData(FALSE); // send to Dialog
  
  OnChangeExfatLogBytesSect();
  OnChangeExfatLogSectCluster();
  OnChangeExFatFsVersion();  
}

void CExFat::ReceiveFromDialog(struct S_EXFAT_VBR *vbr) {
  UpdateData(TRUE); // receive from Dialog
  
  vbr->data_region_lba = convert32(m_data_region_lba);
  vbr->data_region_size = convert32(m_data_region_size);
  vbr->drive_sel = convert8(m_drive_sel);
  vbr->num_fats = convert8(m_fats);
  vbr->first_fat = convert32(m_first_fat);
  vbr->flags = convert16(m_flags);
  vbr->fs_version = convert16(m_fs_version);
  vbr->jmp[0] = convert8(m_jmp0);
  vbr->jmp[1] = convert8(m_jmp1);
  vbr->jmp[2] = convert8(m_jmp2);
  vbr->log_bytes_per_sect = convert8(m_log_bytes_sect);
  vbr->log_sects_per_clust = convert8(m_log_bytes_cluster);
  memcpy(vbr->oem_name, m_oem_name, 8);
  vbr->part_offset = convert64(m_part_offset);
  vbr->percent_heap = convert8(m_percent_heap);
  //m_reserved0.Format("%i", );
  //m_reserved1.Format("%i", );
  vbr->root_dir_cluster = convert32(m_root_cluster);
  vbr->sect_per_fat = convert32(m_sect_fat);
  vbr->serial = convert32(m_serial_number);
  vbr->total_sectors = convert64(m_total_sectors);
  //BYTE    reserved0[53];
  //BYTE    reserved1[7];
}

// Load the first FAT to fat_buffer
void *CExFat::ExFatLoadFAT(void *fat_buffer) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  if (fat_buffer) free(fat_buffer);
  fat_buffer = malloc(vbr->sect_per_fat * (1 << vbr->log_bytes_per_sect));
  dlg->ReadFromFile(fat_buffer, m_lba + vbr->first_fat, vbr->sect_per_fat, FALSE);
  return fat_buffer;
}

void *CExFat::ExFatLoadBP(void *bp_buffer) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  struct S_EXFAT_ROOT *entry;
  DWORD64 rootsize = 0;

  if (bp_buffer) free(bp_buffer);
  bp_buffer = NULL;
  m_bp_cluster = 0;
  m_bp_size = 0;
  m_bp_flags = 0;

  void *root = ReadFile(vbr->root_dir_cluster, &rootsize, 0);
  if (root) {
    entry = (struct S_EXFAT_ROOT *) root;
    while ((BYTE *) entry < ((BYTE *) root + rootsize)) {
      if (entry->entry_type == EXFAT_DIR_BITMAP) {
        m_bp_cluster = entry->type.bitmap.cluster_strt;
        m_bp_size = entry->type.bitmap.data_len;
        m_bp_flags = entry->type.bitmap.flags;
        bp_buffer = malloc((size_t) m_bp_size);
        if (bp_buffer)
          bp_buffer = ReadFile(m_bp_cluster, &m_bp_size, m_bp_flags);
        break;
      }
      entry++;
    }

    free(root);
  }
  
  return bp_buffer;
}

void CExFat::ExFatWriteBP(void *bp_buffer) {

  if (!bp_buffer || !m_bp_size)
    return;
  
  WriteFile(bp_buffer, m_bp_cluster, m_bp_size, m_bp_flags);
}

// Write fat_buffer to the first FAT
void CExFat::ExFatWriteFAT(void *fat_buffer) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  dlg->WriteToFile(fat_buffer, m_lba + vbr->first_fat, vbr->sect_per_fat, FALSE);
}

void CExFat::SaveItemInfo(HTREEITEM hItem, HTREEITEM hParent, DWORD Cluster, DWORD64 FileSize, BYTE Flags, DWORD EntryOffset, DWORD ErrorCode, BOOL CanCopy) {
  struct S_EXFAT_ITEMS *items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->hParent = hParent;
    items->Cluster = Cluster;
    items->FileSize = FileSize;
    items->Flags = Flags;
    items->EntryOffset = EntryOffset;
    items->ErrorCode = ErrorCode;
    items->CanCopy= CanCopy;
  }
}

void CExFat::OnExFatEntry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_EXFAT_ITEMS *items, *parent_items = NULL;
  CExFatEntry ExFatEntry;
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  
  if (hItem) {
    if (hItem == m_hRoot) {
      AfxMessageBox("No entry exists for the root");
      return;
    }
    items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      // if (parent_items == NULL) we will need to calcute the root cluster number from the vbr
      if (items->hParent) {
        parent_items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(items->hParent);
        ExFatEntry.m_root_cluster = parent_items->Cluster;
        ExFatEntry.m_Flags = parent_items->Flags;
        ExFatEntry.m_buffer_size = parent_items->FileSize;
      } else {
        struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
        ExFatEntry.m_root_cluster = vbr->root_dir_cluster;
        ExFatEntry.m_Flags = 0;
        ExFatEntry.m_buffer_size = 0;
      }
      ExFatEntry.m_root_offset = items->EntryOffset;
      ExFatEntry.m_Cluster_Count = vbr->data_region_size;
      ExFatEntry.m_Parent = this;
      if (ExFatEntry.DoModal() == IDOK) { // apply button pressed?
        // reload the list
        Start(m_lba, m_size, m_color, m_index, FALSE);
      }
    }
  }
}

void CExFat::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
  
  // we have to update m_vbr_buffer or the Apply button will destroy the updates
  BYTE buffer[MAX_SECT_SIZE];
  dlg->ReadFromFile(buffer, m_lba, 1, FALSE);
  memcpy((BYTE *) m_vbr_buffer + S_FYSOSSIG_OFFSET, buffer + S_FYSOSSIG_OFFSET, sizeof(struct S_FYSOSSIG));
  OnExFatApply();
}

void CExFat::OnExFatApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  ReceiveFromDialog((struct S_EXFAT_VBR *) m_vbr_buffer);
  
  // update the CRC sector
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  unsigned vbr_sect_size = (1 << vbr->log_bytes_per_sect);
  DWORD crc = VBRChecksum(m_vbr_buffer, 11 * vbr_sect_size);
  DWORD *p = (DWORD *) ((BYTE *) m_vbr_buffer + (11 * vbr_sect_size));
  for (unsigned i=0; i<vbr_sect_size / 4; i++)
    p[i] = crc;
  
  // write 12 sectors
  dlg->WriteToFile(m_vbr_buffer, m_lba, 12, FALSE);
}

void CExFat::OnExfatRestoreBackup() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  // read 12 sectors
  dlg->ReadFromFile(m_vbr_buffer, m_lba + 12, 12, FALSE);  // backup starts at 12
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CExFat::OnExfatUpdateBackup() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  ReceiveFromDialog((struct S_EXFAT_VBR *) m_vbr_buffer);
  
  // update the CRC sector
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  unsigned vbr_sect_size = (1 << vbr->log_bytes_per_sect);
  DWORD crc = VBRChecksum(m_vbr_buffer, 11 * vbr_sect_size);
  DWORD *p = (DWORD *) ((BYTE *) m_vbr_buffer + (11 * vbr_sect_size));
  for (unsigned i=0; i<vbr_sect_size / 4; i++)
    p[i] = crc;
  
  // write 12 sectors
  dlg->WriteToFile(m_vbr_buffer, m_lba + 12, 12, FALSE);  // backup starts at 12
}

void CExFat::OnExFatClean() {
  int r = AfxMessageBox("This will erase the volume, leaving the VBR as is.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  ExFatFormat(FALSE);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CExFat::OnExFatFormat() {
  int r = AfxMessageBox("This will erase the volume, using most of the current VBR values.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  ExFatFormat(r == IDYES);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// returns TRUE if successful
bool CExFat::ExFatFormat(const BOOL AskForBoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile bsfile;
  BYTE *buffer;
  unsigned i, j, k, bitmap_size, root_size = 2, off = 0;
  DWORD64 l;
  DWORD crc, *p;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  if (!m_hard_format)
    ReceiveFromDialog((struct S_EXFAT_VBR *) m_vbr_buffer);
  
  // first, clear out the volume
  buffer = (BYTE *) calloc(12 * dlg->m_sect_size, 1);
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
    odlg.m_ofn.lpstrTitle = "ExFAT Boot Sector File";
    odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
    if (odlg.DoModal() == IDOK) {
      POSITION pos = odlg.GetStartPosition();
      if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
        size_t filesize = (size_t) bsfile.GetLength();
        if (filesize <= (9 * dlg->m_sect_size)) {
          bsfile.Read(buffer, (UINT) filesize);
          off = 3;
        } else
          AfxMessageBox("Boot sector must be <= 9 sectors");
        bsfile.Close();
      }
    }
  }
  
  // restore our vbr info and setup the new VBR
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) buffer;
  if (!m_hard_format)
    memcpy((BYTE *) vbr + off, (BYTE *) m_vbr_buffer + off, sizeof(struct S_EXFAT_VBR) - off);
  
  vbr->num_fats = 1;
  vbr->first_fat = 128;  ///  128 is a little high ???
  
  // calculate sectors per cluster
  if (m_hard_format || (vbr->log_sects_per_clust == 0)) {
    if (m_size < 10000)
      vbr->log_sects_per_clust = 0;  // 0 = 1 spc
    else if (m_size < 50000)
      vbr->log_sects_per_clust = 1;  // 1 = 2 spc
    else if (m_size < 100000)
      vbr->log_sects_per_clust = 2;  // 2 = 4 spc
    else if (m_size < 500000)
      vbr->log_sects_per_clust = 3;  // 3 = 8 spc
    else
      vbr->log_sects_per_clust = 4;  // 4 = 16 spc
  }
  
  // calculate count of clusters
  vbr->data_region_size = (DWORD) ((m_size - vbr->first_fat) / ((DWORD64) 1 << vbr->log_sects_per_clust));
  
  if (off == 0) {
    vbr->jmp[0] = 0xEB;
    vbr->jmp[1] = 0x76;
    vbr->jmp[2] = 0x90;
  }
  memset(vbr->reserved0, 0, 53);
  vbr->part_offset = m_lba;
  vbr->total_sectors = m_size;
  vbr->sect_per_fat = ((vbr->data_region_size * sizeof(DWORD)) + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  vbr->data_region_lba = vbr->first_fat + vbr->sect_per_fat;
  vbr->serial = (rand() << 16) | rand();
  vbr->fs_version = 0x0100;
  vbr->flags = (0<<3) | (0<<2) | (0<<1) | (0<<0);
  vbr->log_bytes_per_sect = LOG2(dlg->m_sect_size);
  vbr->drive_sel = 0x80;
  memset(vbr->reserved1, 0, 7);
  
  // calculate bitmap size
  bitmap_size = (((vbr->data_region_size + 7) / 8) + (dlg->m_sect_size - 1)) / dlg->m_sect_size;  // sectors needed
  bitmap_size = (bitmap_size + ((1 << vbr->log_sects_per_clust) - 1)) / (1 << vbr->log_sects_per_clust); // clusters needed
  vbr->root_dir_cluster = 2 + bitmap_size;
  
  // make sure sector 10 and 11 are clear
  memset(buffer + (10 * dlg->m_sect_size), 0, 2 * dlg->m_sect_size);
  
  // make sure each sector up to 8 has "00 00 55 AA" as the last dword
  // (the first sector only needs "55 AA")
  WORD *w = (WORD *) (buffer + 510);
  *w = 0xAA55;
  for (i=1; i<8; i++) {
    p = (DWORD *) (buffer + (i * dlg->m_sect_size) + (dlg->m_sect_size - 4));
    *p = 0xAA550000;
  }
  
  // calculate crc of first 11 sectors and write it to the 12th
  crc = VBRChecksum(buffer, 11 * dlg->m_sect_size);
  p = (DWORD *) (buffer + (11 * dlg->m_sect_size));
  for (i=0; i<dlg->m_sect_size / 4; i++)
    p[i] = crc;
  
  // calculate the size and crc of the ucase table
  DWORD ucase_cluster = 0;
  const unsigned ucase_bsize = 5836;  // 2918 WORD-sized chars
  unsigned ucase_size = (ucase_bsize + (dlg->m_sect_size - 1)) / dlg->m_sect_size;  // sectors needed
  ucase_size = (ucase_size + ((1 << vbr->log_sects_per_clust) - 1)) / (1 << vbr->log_sects_per_clust); // clusters needed
  BYTE *b = (BYTE *) ucase_table;
  DWORD ucase_crc = 0;
  for (i=0; i<ucase_bsize; i++)
    ucase_crc = ((ucase_crc << 31) | (ucase_crc >> 1)) + (DWORD) b[i];
  
  // calculate percentage of heap
  const unsigned used = (bitmap_size + ucase_size + root_size) * 100;
  vbr->percent_heap = (BYTE) (int) ((double) used / (double) vbr->data_region_size);
  
  // write the VBR and backup VBR
  dlg->WriteToFile(buffer, m_lba, 12, FALSE);
  dlg->WriteToFile(buffer, m_lba + 12, 12, FALSE);
  
  // create and write fat(s)
  BYTE *sect_buff = (BYTE *) malloc(dlg->m_sect_size);
  for (i=0; i<vbr->num_fats; i++) {
    memset(sect_buff, 0, dlg->m_sect_size);
    p = (DWORD *) sect_buff;
    p[0] = 0xFFFFFFF8;
    p[1] = 0xFFFFFFFF;
    // fat entries for the bitmap
    j = 2;
    for (k=0; k<bitmap_size-1; k++, j++)
      p[j] = j+1;
    p[j] = EXFAT_FAT_LAST;
    // fat entries for the root
    j++;
    for (k=0; k<root_size-1; k++, j++)
      p[j] = j+1;
    p[j] = EXFAT_FAT_LAST;
    // fat entries for the UCase table
    ucase_cluster = ++j;
    for (k=0; k<ucase_size-1; k++, j++)
      p[j] = j+1;
    p[j] = EXFAT_FAT_LAST;
    for (j=0; j<vbr->sect_per_fat; j++) {
      dlg->WriteToFile(sect_buff, m_lba + vbr->first_fat + (i * vbr->sect_per_fat) + j, 1, FALSE);
      if (j == 0)
        memset(sect_buff, 0, dlg->m_sect_size);
    }
  }
  
  // create and write bitmap
  // (this assumes that the set bits in the bitmap will all fit in the first sector)
  // (for a clean format, this should be the case)
  memset(sect_buff, 0, dlg->m_sect_size);
  for (i=0; i<bitmap_size + root_size; i++)
    sect_buff[i/8] |= 1 << (i % 8);
  for (i=0; i<bitmap_size * (1 << vbr->log_sects_per_clust); i++) {
    dlg->WriteToFile(sect_buff, m_lba + vbr->data_region_lba + i, 1, FALSE);
    if (i == 0)
      memset(sect_buff, 0, dlg->m_sect_size);
  }
  
  // create and write root
  // root_size = size of root in clusters
  // we need the label, the bitmap entry, and the ucase entry.  fill all remaining as zeros...
  // (this assumes the two entries needed fits in the first sector)
  // (3 * 32-bytes each < 512)
  memset(sect_buff, 0, dlg->m_sect_size);
  struct S_EXFAT_ROOT *root = (struct S_EXFAT_ROOT *) sect_buff;
  // label entry
  root[0].entry_type = EXFAT_DIR_LABEL;
  memcpy(root[0].type.label.name, "A\0 \0L\0a\0b\0e\0l\0", 14);
  root[0].type.label.len = 7;
  // bitmap entry
  root[1].entry_type = EXFAT_DIR_BITMAP;
  root[1].type.bitmap.cluster_strt = 2;
  root[1].type.bitmap.data_len = vbr->data_region_size * sizeof(DWORD);
  root[1].type.bitmap.flags = 0;
  // ucase entry
  root[2].entry_type = EXFAT_DIR_UCASE;
  root[2].type.up_case_table.cluster_strt = ucase_cluster;
  root[2].type.up_case_table.data_len = ucase_bsize;
  root[2].type.up_case_table.crc = ucase_crc;
  for (i=0; i<root_size * (1 << vbr->log_sects_per_clust); i++) {
    dlg->WriteToFile(sect_buff, m_lba + vbr->data_region_lba + ((vbr->root_dir_cluster - 2) * (1 << vbr->log_sects_per_clust)) + i, 1, FALSE);
    if (i == 0)
      memset(sect_buff, 0, dlg->m_sect_size);
  }
  
  // write the UCase data
  dlg->WriteToFile(ucase_table, m_lba + vbr->data_region_lba + ((ucase_cluster - 2) * (1 << vbr->log_sects_per_clust)), ucase_size, FALSE);
  
  if (!m_hard_format) {
    memcpy(m_vbr_buffer, vbr, sizeof(struct S_EXFAT_VBR));
    SendToDialog((struct S_EXFAT_VBR *) m_vbr_buffer);
  }
  
  free(sect_buff);
  free(buffer);

  return TRUE;
}

void CExFat::OnExFatCheck() {
  AfxMessageBox("TODO");
  
  // check that sectors 0 -> 11 all have 0x000055AA at offset 508 ????
  
}

void CExFat::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  unsigned i;
  
  BYTE *existing = (BYTE *) malloc(12 * dlg->m_sect_size);
  BYTE *buffer = (BYTE *) calloc(9 * dlg->m_sect_size, 1);
  
  // first, read in what we already have
  dlg->ReadFromFile(existing, m_lba, 12, FALSE);
  
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
  odlg.m_ofn.lpstrTitle = "ExFAT Boot Sector File";
  odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
  if (odlg.DoModal() == IDOK) {
    POSITION pos = odlg.GetStartPosition();
    if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
      size_t filesize = (size_t) bsfile.GetLength();
      if (filesize > (9 * dlg->m_sect_size)) {
        AfxMessageBox("Boot sector must be <= 9 sectors");
        filesize = (9 * dlg->m_sect_size);
      }
      bsfile.Read(buffer, (UINT) filesize);
      bsfile.Close();
    }
    
    // copy the 9 sectors from the new code to our buffer, skipping
    // the specific parts of the VBR
    memcpy(existing, buffer, 3);  // the jump instruction
    memcpy(existing + sizeof(struct S_EXFAT_VBR), buffer + sizeof(struct S_EXFAT_VBR), (9 * dlg->m_sect_size) - sizeof(struct S_EXFAT_VBR));
    
    // make sure sector 10 and 11 are clear
    memset(existing + (10 * dlg->m_sect_size), 0, 2 * dlg->m_sect_size);
    
    // make sure each sector up to 8 has "00 00 55 AA" as the last dword
    // (the first sector only needs "55 AA")
    DWORD *p;
    WORD *w = (WORD *) (existing + 510);
    *w = 0xAA55;
    for (i=1; i<8; i++) {
      p = (DWORD *) (existing + (i * dlg->m_sect_size) + (dlg->m_sect_size - 4));
      *p = 0xAA550000;
    }
    
    // restore the FYSOS sig?
    if (AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE))
      memcpy(existing + S_FYSOSSIG_OFFSET, &s_sig, sizeof(struct S_FYSOSSIG));
    
    // calculate crc of first 11 sectors and write it to the 12th
    DWORD crc = VBRChecksum(existing, 11 * dlg->m_sect_size);
    p = (DWORD *) (existing + (11 * dlg->m_sect_size));
    for (i=0; i<dlg->m_sect_size / 4; i++)
      p[i] = crc;
    
    // write it back
    dlg->WriteToFile(existing, m_lba, 12, FALSE);
    
    // need to update the m_vbr_buffer buffer as well
    memcpy(m_vbr_buffer, existing, 3);  // the jump instruction
    memcpy((BYTE *) m_vbr_buffer + sizeof(struct S_EXFAT_VBR), existing + sizeof(struct S_EXFAT_VBR), (12 * dlg->m_sect_size) - sizeof(struct S_EXFAT_VBR));
    
    AfxMessageBox("Boot code updated successfully.");
  }
  
  free(buffer);
  free(existing);
}

void CExFat::OnExfatParamSect() {
  // struct S_EXFAT_PARAMS
  // TODO
  AfxMessageBox("TODO");
  //
  // why would each of the 10 be the same format?  
  // Wouldn't you want 10 different formatted parameters, with the
  //  GUID specifying the type and format of parameter?
  //
}

void CExFat::OnExFatCopy() {
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

void CExFat::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_EXFAT_ITEMS *items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    // don't allow the label to be copied out
    if (items->Cluster == 0)
      return;
    DWORD64 size = items->FileSize;
    void *ptr = ReadFile(items->Cluster, &size, items->Flags);
    if (ptr) {
      CFile file;
      if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
        file.Write(ptr, (UINT) items->FileSize);
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
void CExFat::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CExFat::OnExFatInsert() {
  // TODO:
  char szPath[MAX_PATH];
  BOOL IsDir, bError = FALSE;
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
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, NULL, csFName, csFPath, FALSE);
  
  CWaitCursor wait;
  struct S_EXFAT_ITEMS *items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    if (IsDir)
      InsertFolder(items, csName, csPath);
    else
      bError = (InsertFile(items, csName, csPath) != TRUE);
  }
  
  // need to write the FAT and Bitmap back to the image file
  ExFatWriteFAT(m_fat_buffer);
  ExFatWriteBP(m_bp_buffer);
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
  
  wait.Restore();
  //if (!bError)
  //  AfxMessageBox("Files transferred.");
}

// Cluster = starting cluster of folder to insert in to
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CExFat::InsertFolder(struct S_EXFAT_ITEMS *RootItems, CString csName, CString csPath) {
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  struct S_FAT_ENTRIES fat_entries;
  const unsigned size = 2 * (sect_per_clust * bytes_per_sect);  // size of our new folder
  char szPath[MAX_PATH];
  BYTE flags;
  
  // allocate the fat entries for it, returning a "struct S_FAT_ENTRIES"
  if (AllocateFAT(&fat_entries, (DWORD) size, &flags) == -1) {
    if (fat_entries.entries) free(fat_entries.entries);
    return;
  }

  // create a root entry in the folder
  AllocateRoot(csName, RootItems, fat_entries.entries[0], (DWORD) size, flags, TRUE);
  
  // create and write an empty directory
  void *buffer = calloc(size, 1);
  WriteFile(buffer, fat_entries.entries[0], (DWORD) size, flags);
  free(buffer);

  // save the 'items' for below
  struct S_EXFAT_ITEMS Items;
  Items.Cluster = fat_entries.entries[0];
  Items.FileSize = size;
  Items.Flags = flags;
  
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
      InsertFolder(&Items, filefind.GetFileName(), filefind.GetFilePath());
    else
      InsertFile(&Items, filefind.GetFileName(), filefind.GetFilePath());
  }
  filefind.Close();
  
  // restore the current directory
  SetCurrentDirectory(szPath);
  
  if (fat_entries.entries)
    free(fat_entries.entries);
}

// Cluster = starting cluster of folder to insert to
// csName = name of file to insert
// csPath = path on host of file to insert
BOOL CExFat::InsertFile(struct S_EXFAT_ITEMS *RootItems, CString csName, CString csPath) {
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) m_vbr_buffer;
  const unsigned sect_per_clust = (1 << vbr->log_sects_per_clust);
  const unsigned bytes_per_sect = (1 << vbr->log_bytes_per_sect);
  struct S_FAT_ENTRIES fat_entries;
  void *buffer;
  size_t size;
  CFile file;
  BYTE flags;

  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return FALSE;
  }
  size = (size_t) file.GetLength();
  buffer = malloc(size + (sect_per_clust * bytes_per_sect));  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, (UINT) size);
  file.Close();
  
  // allocate the fat entries for it, returning a "struct S_FAT_ENTRIES"
  if (AllocateFAT(&fat_entries, (DWORD) size, &flags) == -1) {
    if (fat_entries.entries) free(fat_entries.entries);
    if (buffer) free(buffer);
    return FALSE;
  }

  // create a root entry in the folder
  AllocateRoot(csName, RootItems, fat_entries.entries[0], (DWORD) size, flags, FALSE);
  
  // copy the file to our system
  WriteFile(buffer, fat_entries.entries[0], (DWORD) size, flags);
  
  // free the buffers
  if (fat_entries.entries)
    free(fat_entries.entries);
  if (buffer)
    free(buffer);

  return TRUE;
}

// the user change the status of the "Show Deleted" Check box
void CExFat::OnShowDeleted() {
  AfxGetApp()->WriteProfileInt("Settings", "exFATShowDel", m_show_del = IsDlgButtonChecked(IDC_SHOW_DEL));
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// the user change the status of the "Delete Clear" Check box
void CExFat::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "exFATDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void CExFat::OnExFatDelete() {
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
  
  CWaitCursor wait;
  if (IsDir)
    DeleteFolder(hItem);
  else
    DeleteFile(hItem);
  
  // since we changed the FAT, we need to reload the fat buffer
  m_fat_buffer = ExFatLoadFAT(m_fat_buffer);
  
  // delete the item from the tree
  if (IsDlgButtonChecked(IDC_SHOW_DEL))
    Start(m_lba, m_size, m_color, m_index, FALSE);
  else
    m_dir_tree.DeleteItem(hItem);

  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CExFat::DeleteFolder(HTREEITEM hItem) {
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

void CExFat::DeleteFile(HTREEITEM hItem) {
  struct S_EXFAT_ITEMS *items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  
  void *root;
  CString csName, csPath;
  
  if (items == NULL)
    return;
  
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  if (hParent == NULL)
    return;

  struct S_EXFAT_ITEMS *root_items = (struct S_EXFAT_ITEMS *) m_dir_tree.GetDataStruct(hParent);
  if (root_items == NULL)
    return;
  
  m_dir_tree.GetFullPath(hParent, NULL, csName, csPath, TRUE);
  
  bool root_mod = FALSE;
  DWORD64 rootsize = 0;
  root = ReadFile(root_items->Cluster, &rootsize, root_items->Flags);
  if (root) {
    struct S_EXFAT_ROOT *entry = (struct S_EXFAT_ROOT *) ((BYTE *) root + items->EntryOffset);
    int secondary_count;
    switch (entry->entry_type) {
      case EXFAT_DIR_BITMAP: // Entry type: Bitmap
        AfxMessageBox("Should not delete the Bitmap Entry");
        break;
      case EXFAT_DIR_UCASE:  // Entry type: UCase Table
        AfxMessageBox("Should not delete the UCase Entry");
        break;
      case EXFAT_DIR_LABEL:  // Entry type: Label
        AfxMessageBox("TODO: Delete Label Entry");
        break;
      case EXFAT_DIR_DELETED: // deleted entry
        AfxMessageBox("Already a Deleted Entry");
        break;
      case EXFAT_DIR_ENTRY:  // Entry type: Directory Entry
        entry->entry_type = EXFAT_DIR_DELETED_S;
        secondary_count = entry->type.dir_entry.sec_count;
        if (entry[1].entry_type == EXFAT_DIR_STRM_EXT) {
          // Free the clusters of the file
          if (ExFatClearClusterChain(entry[1].type.stream_ext.first_cluster, entry[1].type.stream_ext.data_len, entry[1].type.stream_ext.flags)) {
            ExFatWriteFAT(m_fat_buffer);
            ExFatWriteBP(m_bp_buffer);
          }
        }
        // update the CRC
        entry->type.dir_entry.crc = ExFatCheckDirCRC((BYTE *) entry, ((secondary_count + 1) * sizeof(struct S_EXFAT_ROOT)));
        // mark that we need to write back the root
        root_mod = TRUE;
        break;
      case EXFAT_DIR_WINCE_ACC:  // Entry type: 
        AfxMessageBox("WinCE_ACC Entry");
        break;
      case EXFAT_DIR_GUID:   // Entry type: 
        AfxMessageBox("GUID Entry");
        break;
      case EXFAT_DIR_TEXFAT: // Entry type: 
        AfxMessageBox("TEXFAT Entry");
        break;
      default:
        AfxMessageBox("Unknown Entry");
    }
    
    // write back the new root
    if (root_mod)
      WriteFile(root, root_items->Cluster, rootsize, root_items->Flags);
    free(root);
  }
}

void CExFat::OnSearch() {
  m_dir_tree.Search();
}

void CExFat::OnChangeExFatFsVersion() {
  CString cs;
  
  GetDlgItemText(IDC_EXFAT_FS_VERSION, cs);
  WORD val = convert16(cs);
  cs.Format("%i.%i", val >> 8, val & 0xFF);
  SetDlgItemText(IDC_EXFAT_FS_VERSION_DISP, cs);
}

void CExFat::OnSerialUpdate() {
  CString cs;
  cs.Format("0x%08X", (rand() << 16) | rand());
  SetDlgItemText(IDC_EXFAT_SERIAL_NUM, cs);
}

void CExFat::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CExFat::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CExFat::OnChangeExfatLogBytesSect() {
  CString cs;
  
  GetDlgItemText(IDC_EXFAT_LOG_BYTES_SECT, cs);
  DWORD val = convert8(cs);
  cs.Format("%i", 1 << val);
  SetDlgItemText(IDC_EXFAT_LOG_BYTES_SECT_DISP, cs);
}

void CExFat::OnChangeExfatLogSectCluster() {
  CString cs;
  
  GetDlgItemText(IDC_EXFAT_LOG_SECT_CLUSTER, cs);
  DWORD val = convert8(cs);
  cs.Format("%i", 1 << val);
  SetDlgItemText(IDC_EXFAT_LOG_SECT_CLUSTER_DISP, cs);
}

struct S_ATTRIBUTES exfat_flags0[] = {
                                           //            |                               | <- max (col 67)
  { (1<<0),                  (1<<0),                  0, "Active FAT"                     , {-1, } },
  { (1<<1),                  (1<<1),                  1, "Volume Dirty"                   , {-1, } },
  { (1<<2),                  (1<<2),                  1, "Media Failure"                  , {-1, } },
  { (1<<3),                  (1<<3),                  1, "Clear to Zero"                  , {-1, } },
  {      0,              (DWORD) -1,                 -1, NULL                             , {-1, } }
};

void CExFat::OnFlags() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_EXFAT_FLAGS, cs);
  dlg.m_title = "Flags";
  dlg.m_attrib = convert16(cs);
  dlg.m_attributes = exfat_flags0;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_EXFAT_FLAGS, cs);
  }
}

void CExFat::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1, FALSE);
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}
