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

// FYSFS.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "FYSFS.h"
#include "FYSFSEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFYSFS property page

IMPLEMENT_DYNCREATE(CFYSFS, CPropertyPage)

CFYSFS::CFYSFS() : CPropertyPage(CFYSFS::IDD) {
  //{{AFX_DATA_INIT(CFYSFS)
  m_base_lba = _T("");
  m_bitmap_flags = _T("");
  m_bitmap_lsn = _T("");
  m_bitmap_num = _T("");
  m_crc = _T("");
  m_data_lsn = _T("");
  m_data_sectors = _T("");
  m_encrypt_type = _T("");
  m_flags = _T("");
  m_last_chkdsk = _T("");
  m_last_optimize = _T("");
  m_guid = _T("");
  m_label = _T("");
  m_root_entries = _T("");
  m_root_lsn = _T("");
  m_spare_lsn = _T("");
  m_sect_cluster = _T("");
  m_sector_count = _T("");
  m_sig0 = _T("");
  m_sig1 = _T("");
  m_version = _T("");
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
}

CFYSFS::~CFYSFS() {
}

void CFYSFS::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFYSFS)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_BASE_LBA, m_base_lba);
  DDX_Text(pDX, IDC_BITMAP_FLAGS, m_bitmap_flags);
  DDX_Text(pDX, IDC_BITMAP_LSN, m_bitmap_lsn);
  DDX_Text(pDX, IDC_BITMAP_NUM, m_bitmap_num);
  DDX_Text(pDX, IDC_CRC, m_crc);
  DDX_Text(pDX, IDC_DATA_LSN, m_data_lsn);
  DDX_Text(pDX, IDC_DATA_SECTORS, m_data_sectors);
  DDX_Text(pDX, IDC_ENCRYPT_TYPE, m_encrypt_type);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_LAST_CHKDSK, m_last_chkdsk);
  DDX_Text(pDX, IDC_LAST_OPTIMIZE, m_last_optimize);
  DDX_Text(pDX, IDC_GUID, m_guid);
  DDX_Text(pDX, IDC_LABEL, m_label);
  DDX_Text(pDX, IDC_ROOT_ENTRIES, m_root_entries);
  DDX_Text(pDX, IDC_ROOT_LSN, m_root_lsn);
  DDX_Text(pDX, IDC_S_BITMAP_LSN, m_spare_lsn);
  DDX_Text(pDX, IDC_SECT_CLUST, m_sect_cluster);
  DDX_Text(pDX, IDC_SECTOR_COUNT, m_sector_count);
  DDX_Text(pDX, IDC_SIG0, m_sig0);
  DDX_Text(pDX, IDC_SIG1, m_sig1);
  DDX_Text(pDX, IDC_VERSION, m_version);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFYSFS, CPropertyPage)
  //{{AFX_MSG_MAP(CFYSFS)
  ON_WM_HELPINFO()
  ON_BN_CLICKED(ID_CHECK, OnFYSFSCheck)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_COPY, OnFYSFSCopy)
  ON_BN_CLICKED(ID_ENTRY, OnFYSFSEntry)
  ON_BN_CLICKED(ID_INSERT, OnFYSFSInsert)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(IDC_FLAGS_UPDATE, OnFlagsUpdate)
  ON_EN_CHANGE(IDC_VERSION, OnChangeVersion)
  ON_EN_KILLFOCUS(IDC_GUID, OnKillfocusGuid)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFYSFS message handlers
BOOL CFYSFS::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "FYSFSDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  return TRUE;
}

BOOL CFYSFS::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "fysfs.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CFYSFS::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_FYSFS_ITEMS *items = (struct S_FYSFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  
  *pResult = 0;
}


void CFYSFS::OnFYSFSApply() {
  
  
}

// clear out the root, fat, and data area, leaving the BPB as is
void CFYSFS::OnFYSFSClean() {
  /*
  int r = AfxMessageBox("This will erase the volume, leaving the BPB as is.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  FatFormat(FALSE);
  FatStart(m_lba, m_size, m_color, m_index, FALSE);
  */
}

void CFYSFS::OnFYSFSFormat() {
  /*
  int r = AfxMessageBox("This will erase the volume, using most of the current BPB values.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  FatFormat(r == IDYES);
  FatStart(m_lba, m_size, m_color, m_index, FALSE);
  */
}

void CFYSFS::OnFYSFSCheck() {
  AfxMessageBox("TODO:");
}

// SFS colors will have a green shade to them.
DWORD CFYSFS::GetNewColor(int index) {
  int r = ((236 - (index * 18)) > -1) ? (236 - (index * 18)) : 0;
  int g = ((217 - (index *  2)) > -1) ? (217 - (index *  2)) : 0;
  int b = ((151 - (index * 20)) > -1) ? (151 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CFYSFS::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  dlg->ReadFromFile(buffer, lba + 16, 1, FALSE);
  memcpy(&m_super, buffer, sizeof(struct S_FYSFS_SUPER));
  
  m_isvalid = TRUE;
  
  // 
  dlg->m_FYSNames[index] = "FYS FS";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_FYSNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->FYSFS[index].m_draw_index, (char *) (LPCTSTR) dlg->m_FYSNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog(&m_super);
  
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  if (m_isvalid) {
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    struct S_FYSFS_ROOT *root;
    DWORD64 entry_count = m_super.root_entries;
    root = (struct S_FYSFS_ROOT *) ReadFile(m_super.root, &entry_count, TRUE);
    if (root) {
      SaveItemInfo(m_hRoot, m_super.root, m_super.root_entries, NULL, 0, 0, FALSE);
      CWaitCursor wait; // display a wait cursor
      m_too_many = FALSE;
      ParseDir(root, m_super.root_entries, m_hRoot, TRUE);
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

void CFYSFS::ParseDir(struct S_FYSFS_ROOT *root, const unsigned entries, HTREEITEM parent, BOOL IsRoot) {
  struct S_FYSFS_ROOT *sub;
  HTREEITEM hItem;
  unsigned i = 0;
  CString name;
  DWORD64 start, filesize;
  DWORD attrb;
  //DWORD ErrorCode;
  //unsigned ErrorCount = 0;
  unsigned ErrorMax = AfxGetApp()->GetProfileInt("Settings", "MaxErrorCount", 10);
  
  while ((i<entries) && !m_too_many) {
    //ErrorCode = CheckRootEntry(&root[i]);
    switch (root[i].sig) {
      case S_FYSFS_ROOT_NEW:
        FysFSGetName(root, i, name, &attrb, &start, &filesize);
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, start, filesize, &root[i], S_FYSFS_ROOT_NEW, 0, TRUE);
        break;
        
      case S_FYSFS_ROOT_DEL:
        FysFSGetName(root, i, name, &attrb, &start, &filesize);
        name += " (deleted)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, start, filesize, &root[i], S_FYSFS_ROOT_DEL, 0, FALSE);
        break;
        
      case S_FYSFS_ROOT_SUB:
        FysFSGetName(root, i, name, &attrb, &start, &filesize);
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, start, filesize, &root[i], S_FYSFS_ROOT_SUB, 0, TRUE);
        start = FysFSGetFATEntry(root, i, 0);
        if (start != CLUSTER_NOT_FOUND) {
          sub = (struct S_FYSFS_ROOT *) ReadFile(start, &filesize, FALSE);
          if (sub) {
            ParseDir(sub, (unsigned) (filesize / sizeof(struct S_FYSFS_ROOT)), hItem, FALSE);
            free(sub);
          }
        }
        break;
        
      case S_FYSFS_ROOT_EMPTY:
      case S_FYSFS_CONT_NAME:
      case S_FYSFS_CONT_FAT:
        break;
        
      default:
        AfxMessageBox("error in slot");
        i = entries;
    }
    i++;
  }
}

// retrieve the (long) filename from the entry given
void CFYSFS::FysFSGetName(struct S_FYSFS_ROOT *root, unsigned index, CString &name, DWORD *attrb, DWORD64 *start, DWORD64 *filesize) {
  unsigned i;
  char str[(CONT_NAME_FAT_SPACE | NAME_FAT_SPACE) + 1];
  struct S_FYSFS_CONT *cont;
  
  name.Empty();
  memset(str, 0, NAME_FAT_SPACE + 1);
  memcpy(str, root[index].name_fat, root[index].namelen);
  name += str;
  i = root[index].name_continue;
  while (i) {
    cont = (struct S_FYSFS_CONT *) &root[i];
    if (cont->sig == S_FYSFS_CONT_NAME) {
      memset(str, 0, CONT_NAME_FAT_SPACE + 1);
      memcpy(str, cont->name_fat, cont->count);
      name += str;
      i = cont->next;
    } else
      break;
  }
  
  if (attrb) *attrb = root[index].attribute;
  if (filesize) *filesize = root[index].fsize;
  if (start) *start = FysFSGetFATEntry(root, index, 0);
}

DWORD64 CFYSFS::FysFSGetFATEntry(struct S_FYSFS_ROOT *root, unsigned root_index, unsigned index) {
  // assume error for now
  DWORD64 cluster = CLUSTER_NOT_FOUND;
  
  // if entry is within the root SLOT's fat entries, just get it there
  if (index < root[root_index].fat_entries) {
    DWORD *p = (DWORD *) (root[root_index].name_fat + ((root[root_index].namelen + 3) & ~0x03));
    return p[index];
  }
  
  // else, it must be in the chain of CONT entries
  index -= root[root_index].fat_entries;
  if (root[root_index].fat_continue) {
    struct S_FYSFS_CONT *cont = (struct S_FYSFS_CONT *) &root[root[root_index].fat_continue];
    // it would be an error for a FAT CONT root_index to have a count of zero, but check anyway.
    if (cont->count == 0)
      return CLUSTER_NOT_FOUND;
    
    // keep spanning the CONT slots until we find the one we need
    while (index >= cont->count) {
      index -= cont->count;
      // if there is not another slot and entry is still > this one, error
      if (cont->next == 0)
        return CLUSTER_NOT_FOUND;
      
      // get the next CONT slot
      cont = (struct S_FYSFS_CONT *) &root[cont->next];
      // it would be an error for a FAT CONT slot to have a count of zero, but check anyway.
      if (cont->count == 0)
        return CLUSTER_NOT_FOUND;
    }
    
    // we are now in the CONT slot that contains the entry we want
    if (cont->flags & CONT_FLAGS_LARGE) {
      DWORD64 *l = (DWORD64 *) cont->name_fat;
      return l[index];
    } else {
      DWORD *s = (DWORD *) cont->name_fat;
      return s[index];
    }
  }
  
  return CLUSTER_NOT_FOUND;
}

// the user change the status of the "Delete Clear" Check box
void CFYSFS::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "FYSFSDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void *CFYSFS::ReadFile(DWORD64 lsn, DWORD64 *size, BOOL IsRoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *ptr = NULL;
  unsigned pos = 0, mem_size = 0;
  
  if (IsRoot) {
    ptr = calloc(((DWORD) *size * sizeof(struct S_FYSFS_ROOT)) + (MAX_SECT_SIZE - 1), 1);
    if (ptr)
      dlg->ReadFromFile(ptr, m_lba + m_super.root, (((DWORD) *size * sizeof(struct S_FYSFS_ROOT)) + (512 - 1)) / 512, FALSE);
  } else {
    /*
    while (cluster != 0xFFFFFFFF) {
      if (mem_size < (pos + (bpb->bytes_per_sect * bpb->sect_per_clust))) {
        mem_size += (bpb->bytes_per_sect * bpb->sect_per_clust);
        ptr = realloc(ptr, mem_size);
      }
      dlg->ReadFromFile((BYTE *) ptr + pos, m_lba + m_datastart + ((cluster - 2) * bpb->sect_per_clust), bpb->sect_per_clust, FALSE);
      prev_cluster = cluster;  // catch an endless loop if next cluster == current cluster
      cluster = GetNextCluster(cluster);
      if (cluster == prev_cluster) {
        AfxMessageBox("Found Error: FAT0001");
        break;
      }
      pos += (bpb->bytes_per_sect * bpb->sect_per_clust);
    }
    if (size) *size = pos;
    */
  }
  
  return ptr;
}

void CFYSFS::SaveItemInfo(HTREEITEM hItem, DWORD64 Cluster, DWORD64 FileSize, struct S_FYSFS_ROOT *Entry, DWORD Sig, DWORD ErrorCode, BOOL CanCopy) {
  struct S_FYSFS_ITEMS *items = (struct S_FYSFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->Cluster = Cluster;
    items->FileSize = FileSize;
    items->ErrorCode = ErrorCode;
    items->CanCopy = CanCopy;
    items->Sig = Sig;
    if (Entry)
      memcpy(&items->Entry, Entry, sizeof(struct S_FYSFS_ROOT));
  }
}

// update all items in dialog
void CFYSFS::SendToDialog(struct S_FYSFS_SUPER *super) {
  CString cs;
  
  m_base_lba.Format("%I64i", super->base_lba);
  m_bitmap_flags.Format("0x%04X", super->bitmap_flag);
  m_bitmap_lsn.Format("%I64i", super->bitmap);
  m_bitmap_num.Format("%i", super->bitmaps);
  m_crc.Format("0x%08X", super->crc);
  m_data_lsn.Format("%I64i", super->data);
  m_data_sectors.Format("%I64i", super->data_sectors);
  m_encrypt_type.Format("0x%02X", super->encryption);
  m_flags.Format("0x%08X", super->flags);
  m_last_chkdsk.Format("%i", super->chkdsk);
  m_last_optimize.Format("%i", super->lastopt);
  m_root_entries.Format("%i", super->root_entries);
  m_root_lsn.Format("%I64i", super->root);
  m_spare_lsn.Format("%I64i", super->bitmapspare);
  m_sect_cluster.Format("%i", super->sect_clust);
  m_sector_count.Format("%I64i", super->sectors);
  m_sig0.Format("0x%08X", super->sig[0]);
  m_sig1.Format("0x%08X", super->sig[1]);
  m_version.Format("0x%04X", super->ver);
  cs.Format("%i.%02X", (super->ver >> 8), super->ver & 0xFF);
  SetDlgItemText(IDC_VERSION_DISP, cs);
  GUID_Format(m_guid, &super->guid);
  m_label.Format("%s", super->vol_label);
  
  UpdateData(FALSE); // send to Dialog
}

void CFYSFS::ReceiveFromDialog(struct S_FYSFS_SUPER *super) {
  UpdateData(TRUE); // receive from Dialog
  
  super->base_lba = convert64(m_base_lba);
  super->bitmap_flag = convert16(m_bitmap_flags);
  super->bitmap = convert64(m_bitmap_lsn);
  super->bitmaps = convert8(m_bitmap_num);
  super->crc = convert32(m_crc);
  super->data = convert64(m_data_lsn);
  super->data_sectors = convert64(m_data_sectors);
  super->encryption = convert8(m_encrypt_type);
  super->flags = convert32(m_flags);
  super->chkdsk = convert32(m_last_chkdsk);
  super->lastopt = convert32(m_last_optimize);
  super->root_entries = convert32(m_root_entries);
  super->root = convert64(m_root_lsn);
  super->bitmapspare = convert64(m_spare_lsn);
  super->sect_clust = convert16(m_sect_cluster);
  super->sectors = convert64(m_sector_count);
  super->sig[0] = convert32(m_sig0);
  super->sig[1] = convert32(m_sig1);
  super->ver = convert16(m_version);
  GUID_Retrieve(m_guid, &super->guid);
  strcpy((char *) super->vol_label, m_label);
}

void CFYSFS::OnFYSFSCopy() {
  char szPath[MAX_PATH];
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  if (!hItem) {
    AfxMessageBox("No Item");
    return;
  }
  
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

void CFYSFS::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
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
void CFYSFS::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CFYSFS::OnFYSFSInsert() {
  // TODO:  
}

void CFYSFS::OnSearch() {
  m_dir_tree.Search();
}

void CFYSFS::OnFYSFSEntry() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CFYSFSEntry FYSEntry;
  
  if (hItem) {
    struct S_FYSFS_ITEMS *items = (struct S_FYSFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      FYSEntry.m_attribute.Format("0x%08X", items->Entry.attribute);
      FYSEntry.m_crc.Format("0x%08X", items->Entry.crc);
      FYSEntry.m_created.Format("0x%08X", items->Entry.created);
      FYSEntry.m_fat_cont.Format("%i", items->Entry.fat_continue);
      FYSEntry.m_fat_entries.Format("%i", items->Entry.fat_entries);
      FYSEntry.m_file_size.Format("%I64i", items->Entry.fsize);
      FYSEntry.m_flags.Format("0x%08X", items->Entry.flags);
      FYSEntry.m_last_access.Format("0x%08X", items->Entry.lastaccess);
      FYSEntry.m_name_cont.Format("%i", items->Entry.name_continue);
      FYSEntry.m_name_fat = "TODO";
      FYSEntry.m_name_len.Format("%i", items->Entry.namelen);
      FYSEntry.m_scratch.Format("0x%02X", items->Entry.scratch);
      FYSEntry.m_slot_type.Format("0x%08X", items->Entry.sig);
      if (FYSEntry.DoModal() == IDOK) { // apply button pressed?
        // TODO:
      }
    }
  }
}

void CFYSFS::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CFYSFS::OnFlagsUpdate() {
  // TODO:  
}

void CFYSFS::OnChangeVersion() {
  // TODO:  
}

void CFYSFS::OnKillfocusGuid() {
  CString cs;
  GetDlgItemText(IDC_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    GetDlgItem(IDC_GUID)->SetFocus();
  }
}

void CFYSFS::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CFYSFS::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CFYSFS::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1, FALSE);
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}

void CFYSFS::OnUpdateCode() {
  AfxMessageBox("TO DO");
  
  /*
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  CString cs;
  
  if (m_super.resv_blocks == 0)
    m_super.resv_blocks = 1;
  unsigned reserved = m_super.resv_blocks * (1 << (m_super.block_size + 7));
  
  BYTE *existing = (BYTE *) malloc(reserved);
  BYTE *buffer = (BYTE *) calloc(reserved, 1);
  
  // first, read in what we already have
  dlg->ReadFromFile(existing, m_lba, m_super.resv_blocks, FALSE);
  
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
  odlg.m_ofn.lpstrTitle = "SFS Boot Sector File";
  if (odlg.DoModal() == IDOK) {
    POSITION pos = odlg.GetStartPosition();
    if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
      DWORD filesize = bsfile.GetLength();
      if (filesize > reserved) {
        cs.Format("Boot sector must be <= %i bytes", reserved);
        AfxMessageBox(cs);
        filesize = reserved;
      }
      bsfile.Read(buffer, filesize);
      bsfile.Close();
    }
    
    // copy the count of sectors from the new code to our buffer, skipping
    // the specific parts of the SuperBlock
    memcpy(existing, buffer, SFS_SUPER_LOC);  // the boot code before the superblock
    memcpy(existing + SFS_SUPER_LOC + sizeof(struct S_SFS_SUPER), buffer + SFS_SUPER_LOC + sizeof(struct S_SFS_SUPER), reserved - (SFS_SUPER_LOC + sizeof(struct S_SFS_SUPER)));
    
    // restore the FYSOS sig?
    if (AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE))
      memcpy(existing + S_FYSOSSIG_OFFSET, &s_sig, sizeof(struct S_FYSOSSIG));
    
    // write it back
    dlg->WriteToFile(existing, m_lba, m_super.resv_blocks, FALSE);
    
    AfxMessageBox("Updated Boot Code successfully");
  }
  
  free(buffer);
  free(existing);
  */
}
