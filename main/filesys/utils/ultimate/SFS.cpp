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

// SFS.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#include "SFS.h"
#include "SFSEntry.h"
#include "SFSFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSFS property page

IMPLEMENT_DYNCREATE(CSFS, CPropertyPage)

CSFS::CSFS() : CPropertyPage(CSFS::IDD) {
  //{{AFX_DATA_INIT(CSFS)
  m_sfs_block_count = _T("");
  m_sfs_block_size = _T("");
  m_sfs_crc = _T("");
  m_sfs_index_size = _T("");
  m_sfs_magic = _T("");
  m_sfs_resv_blocks = _T("");
  m_sfs_timestamp = _T("");
  m_sfs_total_blocks = _T("");
  m_sfs_version = _T("");
  m_show_del = FALSE;
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_indx_buffer = NULL;
  m_hard_format = FALSE;
}

CSFS::~CSFS() {
  if (m_indx_buffer)
    free(m_indx_buffer);
}

void CSFS::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSFS)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_SFS_BLOCK_COUNT, m_sfs_block_count);
  DDX_Text(pDX, IDC_SFS_BLOCK_SIZE, m_sfs_block_size);
  DDX_Text(pDX, IDC_SFS_CRC, m_sfs_crc);
  DDX_Text(pDX, IDC_SFS_INDEX_SIZE, m_sfs_index_size);
  DDX_Text(pDX, IDC_SFS_MAGIC, m_sfs_magic);
  DDX_Text(pDX, IDC_SFS_RESERVED_BLOCKS, m_sfs_resv_blocks);
  DDX_Text(pDX, IDC_SFS_TIMESTAMP, m_sfs_timestamp);
  DDX_Text(pDX, IDC_SFS_TOTAL_BLOCKS, m_sfs_total_blocks);
  DDX_Text(pDX, IDC_SFS_VERSION, m_sfs_version);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSFS, CPropertyPage)
  //{{AFX_MSG_MAP(CSFS)
  ON_WM_HELPINFO()
  ON_BN_CLICKED(ID_CLEAN, OnSFSClean)
  ON_BN_CLICKED(ID_FORMAT, OnSFSFormat)
  ON_BN_CLICKED(ID_CHECK, OnSFSCheck)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_EN_CHANGE(IDC_SFS_BLOCK_SIZE, OnChangeSfsBlockSize)
  ON_EN_CHANGE(IDC_SFS_VERSION, OnChangeSfsVersion)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  ON_EN_CHANGE(IDC_SFS_TIMESTAMP, OnChangeSfsTimestamp)
  ON_BN_CLICKED(ID_COPY, OnSfsCopy)
  ON_BN_CLICKED(ID_ENTRY, OnSfsEntry)
  ON_BN_CLICKED(ID_INSERT, OnSfsInsert)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(ID_DELETE, OnSfsDelete)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_APPLY, OnSFSApply)
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSFS message handlers
BOOL CSFS::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_show_del = AfxGetApp()->GetProfileInt("Settings", "SFSShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "SFSDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  return TRUE;
}

BOOL CSFS::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "sfs.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CSFS::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow(IsFileOrFolder(hItem) || ((hItem != NULL) && (m_dir_tree.IsDir(hItem) == -1)) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(IsFileOrFolder(hItem));
  
  *pResult = 0;
}

void CSFS::OnSFSApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  ReceiveFromDialog(&m_super); // Get From Dialog
  
  // make sure the super is up to date
  BYTE buffer[MAX_SECT_SIZE];
  dlg->ReadFromFile(buffer, m_lba, 1);
  memcpy(buffer + SFS_SUPER_LOC, &m_super, sizeof(struct S_SFS_SUPER));
  dlg->WriteToFile(buffer, m_lba, 1);
  
  // make sure the index is up to date
  dlg->WriteToFile(m_indx_buffer, m_lba + m_indx_start, m_indx_size);
}

// clear out the index area, creating the start and label entries, etc.
void CSFS::OnSFSClean() {
  int r = AfxMessageBox("This will erase the volume.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  Format(FALSE);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CSFS::OnSFSFormat() {
  int r = AfxMessageBox("This will erase the volume.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  Format(r == IDYES);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CSFS::OnSFSCheck() {
  AfxMessageBox("TODO:");
}

// returns TRUE if successful
bool CSFS::Format(const BOOL AskForBoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile bsfile;
  BYTE *buffer;
  size_t reserved = 1;
  unsigned block_size;
  DWORD64 l;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  // default the blocks size to the given sector size
  CSFSFormat SFSFormat;
  SFSFormat.m_block_size = dlg->m_sect_size;
  SFSFormat.m_info = "Please choose a block size:\r\n"
                     "512, 1024, 2048, 4096";
  if (SFSFormat.DoModal() == IDOK)
    block_size = SFSFormat.m_block_size;
  else
    return FALSE;
  
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
    odlg.m_ofn.lpstrTitle = "SFS Boot Sector File";
    if (odlg.DoModal() == IDOK) {
      POSITION pos = odlg.GetStartPosition();
      if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
        size_t filesize = (size_t) bsfile.GetLength();
        if (filesize <= 8192) {
          reserved = (filesize + (block_size - 1)) / block_size;
          if (filesize > MAX_SECT_SIZE) // must check manually so we don't "shrink" the buffer with realloc()
            buffer = (BYTE *) realloc(buffer, filesize);
          bsfile.Read(buffer, (UINT) filesize);
          dlg->WriteToFile(buffer, m_lba, (UINT) reserved);
        } else
          AfxMessageBox("Boot sector must be <= 8192 bytes");
        bsfile.Close();
      }
    }
  }
  
  // create a new superblock
  m_super.time_stamp = CalculateTime();
  m_super.data_block_count = 0;
  m_super.index_size = (SFS_ENTRY_SIZE * 2); // Start + Label
  m_super.magic_version = (0x1A << 24) | SFS_SUPER_MAGIC;  // version 1.10  (0x01.0x0A)
  m_super.total_blocks = m_size;
  m_super.resv_blocks = (DWORD) reserved;
  m_super.block_size = LOG2(block_size) - 7; // Block size (2^(x+7) where x = 2 = 512)
  m_super.crc = -CalculateCRC(&m_super.magic_version, 17);
  
  // write the super back to the volume
  // if a bootsector was loaded, the first 512 bytes will still contain that code
  // however, lets update the super. 
  memcpy(buffer + SFS_SUPER_LOC, &m_super, sizeof(struct S_SFS_SUPER));
  dlg->WriteToFile(buffer, m_lba, 1);
  free(buffer);
  
  // now the index
  m_indx_size = 1;
  m_indx_start = m_size - m_indx_size;
  m_indx_buffer = (BYTE *) realloc(m_indx_buffer, m_indx_size * 512);
  memset(m_indx_buffer, 0, m_indx_size * 512);
  struct S_SFS_START *start = (struct S_SFS_START *) (m_indx_buffer + (6 * SFS_ENTRY_SIZE));
  start->type = SFS_ENTRY_START;
  start->crc = -CalculateCRC(start, SFS_ENTRY_SIZE);
  struct S_SFS_VOL_ID *id = (struct S_SFS_VOL_ID *) (m_indx_buffer + (7 * SFS_ENTRY_SIZE));
  id->type = SFS_ENTRY_VOL_ID;
  id->time_stamp = CalculateTime();
  strcpy((char *) id->name, "A Clean SFS Volume");
  id->crc = -CalculateCRC(id, SFS_ENTRY_SIZE);
  dlg->WriteToFile(m_indx_buffer, m_lba + m_indx_start, m_indx_size);
  
  if (!m_hard_format)
    SendToDialog(&m_super); // Send back to Dialog

  return TRUE;
}

void CSFS::OnUpdateCode() {
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
  dlg->ReadFromFile(existing, m_lba, m_super.resv_blocks);
  
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
      size_t filesize = (size_t) bsfile.GetLength();
      if (filesize > reserved) {
        cs.Format("Boot sector must be <= %i bytes", reserved);
        AfxMessageBox(cs);
        filesize = reserved;
      }
      bsfile.Read(buffer, (UINT) filesize);
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
    dlg->WriteToFile(existing, m_lba, m_super.resv_blocks);
    
    AfxMessageBox("Updated Boot Code successfully");
  }
  
  free(buffer);
  free(existing);
}

// SFS colors will have a green shade to them.
DWORD CSFS::GetNewColor(int index) {
  int r = ((126 - (index * 18)) > -1) ? (126 - (index * 18)) : 0;
  int g = ((249 - (index * 2)) > -1) ? (249 - (index * 2)) : 0;
  int b = ((96 - (index * 20)) > -1) ? (96 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CSFS::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  dlg->ReadFromFile(buffer, lba, 1);
  memcpy(&m_super, buffer + SFS_SUPER_LOC, sizeof(struct S_SFS_SUPER));
  
  // DetectSFS() detected us, so we should be good
  m_isvalid = TRUE;
  
  // 
  dlg->m_SFSNames[index] = "SimpleFS";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_SFSNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->SFS[index].m_draw_index, (char *) (LPCTSTR) dlg->m_SFSNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog(&m_super);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  if (m_isvalid) {
    m_indx_size = (unsigned) ((m_super.index_size + 511) / 512);
    m_indx_start = size - m_indx_size;
    
    m_indx_buffer = (BYTE *) realloc(m_indx_buffer, m_indx_size * 512);
    dlg->ReadFromFile(m_indx_buffer, m_lba + m_indx_start, m_indx_size);
    
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    SaveItemInfo(m_hRoot, NULL, -1, FALSE);
    CWaitCursor wait; // display a wait cursor
    const unsigned offset = (m_indx_size * 512) - (unsigned) m_super.index_size;
    m_too_many = FALSE;
    ParseDir(m_indx_buffer + offset, (int) (m_super.index_size / SFS_ENTRY_SIZE));
    m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
    wait.Restore();
    GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
    GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
    GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
    
    // select the root and set the focus to that root
    GetDlgItem(IDC_DIR_TREE)->SetFocus();
    m_dir_tree.SelectSetFirstVisible(m_hRoot);
  }
  Invalidate(TRUE);  // redraw the tab
}

void CSFS::ParseDir(const BYTE *index, const int entries) {
  HTREEITEM hItem, hParent;
  int cnt, i;
  CString name;
  BOOL NameError;
  
  if ((index[0] != SFS_ENTRY_START) || (CalculateCRC(index, SFS_ENTRY_SIZE) != 0)) {
    AfxMessageBox("First entry is not the Start entry");
    m_isvalid = FALSE;
    return;
  }
  
  i = 0;
  while ((i<entries) && !m_too_many) {
    switch (index[0]) {
      case SFS_ENTRY_VOL_ID: {  // volume ID
        struct S_SFS_VOL_ID *vol_id = (struct S_SFS_VOL_ID *) index;
        name.Format("Vol ID: %s", vol_id->name);
        if (CalculateCRC(index, SFS_ENTRY_SIZE) != 0) name += " (Bad CRC)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, m_hRoot);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, index, i, FALSE);
        cnt = 1;
      } break;
      case SFS_ENTRY_START:    // start marker
        hItem = m_dir_tree.Insert("(Start Entry)", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, m_hRoot);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, index, i, FALSE);
        cnt = 1;
        break;
      case SFS_ENTRY_UNUSED:   // unused
        //hItem = m_dir_tree.Insert("Unused", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, m_hRoot);
        //if (hItem == NULL) { m_too_many = TRUE; return; }
        cnt = 1;
        break;
      case SFS_ENTRY_DIR: {    // directory entry
        struct S_SFS_DIR *eDir = (struct S_SFS_DIR *) index;
        hParent = GetName(name, eDir, TRUE, &NameError);
        if (NameError) name += " (Error in name)";
        // can't add string to end of dir name since we check the name when adding to the list
        //if (CalculateCRC(index, SFS_ENTRY_SIZE + (eDir->num_cont * SFS_ENTRY_SIZE)) != 0) name += " (Bad CRC)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, hParent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, index, i, TRUE);
        cnt = 1 + eDir->num_cont;
      } break;
      case SFS_ENTRY_FILE: {   // file entry
        struct S_SFS_FILE *eFile = (struct S_SFS_FILE *) index;
        hParent = GetName(name, eFile, FALSE, &NameError);
        if (NameError) name += " (Error in name)";
        if (CalculateCRC(index, SFS_ENTRY_SIZE + (eFile->num_cont * SFS_ENTRY_SIZE)) != 0) name += " (Bad CRC)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, hParent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, index, i, TRUE);
        cnt = 1 + eFile->num_cont;
      } break;
      case SFS_ENTRY_UNUSABLE: // unusable entry (bad sector(s))
        name = "Unusable Entry";
        if (CalculateCRC(index, SFS_ENTRY_SIZE) != 0) name += " (Bad CRC)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, m_hRoot);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, index, i, FALSE);
        cnt = 1;
        break;
      case SFS_ENTRY_DIR_DEL: { // deleted directory
        struct S_SFS_DIR *eDir = (struct S_SFS_DIR *) index;
        if (IsDlgButtonChecked(IDC_SHOW_DEL)) {
          hParent = GetName(name, eDir, TRUE, &NameError);
          name += " (Deleted)";
          // can't add string to end of dir name since we check the name when adding to the list
          //if (CalculateCRC(index, SFS_ENTRY_SIZE + (eDir->num_cont * SFS_ENTRY_SIZE)) != 0) name += " (Bad CRC)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_DELETE, IMAGE_DELETE, hParent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, index, i, FALSE);
        }
        cnt = 1 + eDir->num_cont;
      } break;
      case SFS_ENTRY_FILE_DEL: { // deleted file
        struct S_SFS_FILE *eFile = (struct S_SFS_FILE *) index;
        if (IsDlgButtonChecked(IDC_SHOW_DEL)) {
          hParent = GetName(name, eFile, FALSE, &NameError);
          name += " (Deleted)";
          if (CalculateCRC(index, SFS_ENTRY_SIZE + (eFile->num_cont * SFS_ENTRY_SIZE)) != 0) name += " (Bad CRC)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, hParent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, index, i, FALSE);
        }
        cnt = 1 + eFile->num_cont;
      } break;
      default:
        hItem = m_dir_tree.Insert("Error", ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, m_hRoot);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        cnt = 1;
        break;
    }
    index += (cnt * SFS_ENTRY_SIZE);
    i += cnt;
  }
}

HTREEITEM CSFS::GetName(CString &csname, void *entry, BOOL IsDir, BOOL *Error) {
  BYTE *buffer = (BYTE *) calloc(16373+1, 1); // 16,373 = largest name that can be had, +1 ensures that it is null terminated
  BOOL NameError = FALSE;
  
  if (IsDir) {
    struct S_SFS_DIR *eDir = (struct S_SFS_DIR *) entry;
    memcpy(buffer, eDir->name, DIR_NAME_LEN + (eDir->num_cont * SFS_ENTRY_SIZE));
  } else {
    struct S_SFS_FILE *eFile = (struct S_SFS_FILE *) entry;
    memcpy(buffer, eFile->name, FILE_NAME_LEN + (eFile->num_cont * SFS_ENTRY_SIZE));
  }
  
  // now we need to parse the name and see if we find a '/'.  If so, see if we find
  //  a folder already entered in the list and use that folder item.
  BYTE *name = (BYTE *) calloc(16373+1, 1);
  HTREEITEM hParent = m_hRoot, hItem;
  BYTE *s = buffer;
  BYTE *t = name;
  while (*s) {
    if ((*s < 0x20) || 
       ((*s >= 0x80) && (*s <= 0x9F)) ||
        (*s == 0x22) ||
        (*s == 0x2A) ||
        (*s == 0x3A) ||
        (*s == 0x3C) ||
        (*s == 0x3E) ||
        (*s == 0x3F) ||
        (*s == 0x5C) ||
        (*s == 0x7F) ||
        (*s == 0xA0)) {
      NameError = TRUE;
      *s = '?';
    }
    if (*s == '/') {
      *t = '\0';
      if ((hItem = m_dir_tree.FindFirst(hParent, (LPCTSTR) name)) != NULL) {
        // TODO: get item to see if it is a dir ?????
        hParent = hItem;
        s++;  // skip '/'
        t = name;
      } else {
        *t++ = *s++; // error (name includes '/')
        NameError = TRUE;
      }
    } else
      *t++ = *s++;
  }
  *t = '\0';
  
  csname = name;
  
  free(name);
  free(buffer);
  
  if (Error) *Error = NameError;
  return hParent;
}

void CSFS::SaveItemInfo(HTREEITEM hItem, const BYTE *Entry, int Index, BOOL CanCopy) {
  struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    if (Entry) memcpy(items->entry, Entry, SFS_ENTRY_SIZE);
    else       memset(items->entry, 0, SFS_ENTRY_SIZE);
    items->index = Index;
    items->CanCopy = CanCopy;
    
  }
}

// update all items in dialog
void CSFS::SendToDialog(struct S_SFS_SUPER *super) {
  CString cs;
  
  m_sfs_block_count.Format("%I64i", super->data_block_count);
  m_sfs_block_size.Format("%i", super->block_size);
  m_sfs_crc.Format("0x%02X", super->crc);
  m_sfs_index_size.Format("%I64i", super->index_size);
  m_sfs_magic.Format("0x%06X", super->magic_version & 0x00FFFFFF);
  m_sfs_resv_blocks.Format("%i", super->resv_blocks);
  m_sfs_timestamp.Format("%I64i", super->time_stamp);
  m_sfs_total_blocks.Format("%I64i", super->total_blocks);
  m_sfs_version.Format("0x%02X", super->magic_version >> 24);
  
  // convert timestamp from 1/65536ths to seconds
  DWORD64 timestamp = (super->time_stamp / 65536);
  if (timestamp <= 0xFFFFFFFF) {  // VC++ 6.0 limited the value to 32-bits  (Crashes if larger)
    CTime cTime((time_t) timestamp);  // converts seconds since 1 Jan 1970 to CTime
    cs.Format("%04i/%02i/%02i  %02i:%02i:%02i", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
  } else
    cs = "Error in Timestamp";
  SetDlgItemText(IDC_SFS_TIMESTAMP_DISP, cs);
  
  cs.Format("%i.%i", (super->magic_version >> 28) & 0x0F, (super->magic_version >> 24) & 0x0F);
  SetDlgItemText(IDC_SFS_VERSION_DISP, cs);
  
  cs.Format("%i", 1 << (super->block_size + 7));
  SetDlgItemText(IDC_SFS_BLOCK_SIZE_DISP, cs);
  
  UpdateData(FALSE); // send to Dialog
}

void CSFS::ReceiveFromDialog(struct S_SFS_SUPER *super) {
  UpdateData(TRUE); // receive from Dialog
  
  super->data_block_count = convert64(m_sfs_block_count);
  super->block_size = convert8(m_sfs_block_size);
  super->crc = convert8(m_sfs_crc);
  super->index_size = convert64(m_sfs_index_size);
  super->magic_version = (convert32(m_sfs_magic) & 0x00FFFFFF) | (convert8(m_sfs_version) << 24);
  super->resv_blocks = convert32(m_sfs_resv_blocks);
  super->time_stamp = convert64(m_sfs_timestamp);
  super->total_blocks = convert64(m_sfs_total_blocks);
}

void CSFS::OnChangeSfsBlockSize() {
  CString cs;
  
  GetDlgItemText(IDC_SFS_BLOCK_SIZE, cs);
  if (!cs.IsEmpty())
    cs.Format("%i", 1 << (convert8(cs) + 7));
  SetDlgItemText(IDC_SFS_BLOCK_SIZE_DISP, cs);
}

void CSFS::OnChangeSfsVersion() {
  CString cs;
  int v;
  
  GetDlgItemText(IDC_SFS_VERSION, cs);
  v = convert8(cs);
  if (!cs.IsEmpty())
    cs.Format("%i.%i", (v >> 4) & 0x0F, v & 0x0F);
  SetDlgItemText(IDC_SFS_VERSION_DISP, cs);
}

void CSFS::OnChangeSfsTimestamp() {
  CString cs;
  
  GetDlgItemText(IDC_SFS_TIMESTAMP, cs);
  DWORD64 timestamp = ((INT64) convert64(cs) / 65536);
  if (timestamp <= 0xFFFFFFFF) {  // VC++ 6.0 limited the value to 32-bits  (Crashes if larger)
    CTime cTime((time_t) timestamp);  // converts seconds since 1 Jan 1970 to CTime
    cs.Format("%04i/%02i/%02i  %02i:%02i:%02i", cTime.GetYear(), cTime.GetMonth(), cTime.GetDay(), cTime.GetHour(), cTime.GetMinute(), cTime.GetSecond());
  } else
    cs = "Error in Timestamp";
  SetDlgItemText(IDC_SFS_TIMESTAMP_DISP, cs);
}

void CSFS::OnCrcUpdate() {
  ReceiveFromDialog(&m_super); // Get From Dialog
  m_super.crc = -CalculateCRC(&m_super.magic_version, 17);
  
  CString cs;
  cs.Format("0x%02X", m_super.crc);
  SetDlgItemText(IDC_SFS_CRC, cs);
}

void *CSFS::ReadFile(HTREEITEM hItem, DWORD *FileSize) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *buffer = NULL;
  
  struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    struct S_SFS_FILE *eFile = (struct S_SFS_FILE *) items->entry;
    if (eFile->type == SFS_ENTRY_FILE) {
      buffer = malloc((DWORD) eFile->file_len + MAX_SECT_SIZE);
      DWORD start_lsn = (DWORD) (eFile->start_block * ((DWORD64) 1 << (m_super.block_size + 7))) / dlg->m_sect_size; // convert eFile->start_block to lsn
      dlg->ReadFromFile(buffer, m_lba + start_lsn, (long) ((eFile->file_len + (dlg->m_sect_size - 1)) / dlg->m_sect_size));
      if (FileSize) *FileSize = (DWORD) eFile->file_len;
    }
  }
  
  return buffer;
}

void CSFS::WriteFile(void *buffer, DWORD64 Block, DWORD FileSize) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  DWORD64 start_lsn = (DWORD64) (Block * ((DWORD64) 1 << (m_super.block_size + 7))) / dlg->m_sect_size; // convert eFile->start_block to lsn
  dlg->WriteToFile(buffer, m_lba + start_lsn, (long) ((FileSize + (dlg->m_sect_size - 1)) / dlg->m_sect_size));
}

// write zeros to this file
void CSFS::ZeroFile(DWORD64 Start, DWORD64 End) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *zero = (void *) calloc(dlg->m_sect_size, 1);

  while (Start < End) {
    dlg->WriteToFile((BYTE *) zero, m_lba + Start, 1);
    Start++;
  }
  
  free(zero);
}

BYTE CSFS::CalculateCRC(const void *buffer, DWORD Size) {
  BYTE crc = 0;
  const BYTE *p = (const BYTE *) buffer;
  
  while (Size--)
    crc += *p++;
  
  return crc;
}

// covert time to 1/65536ths of a second since 1 Jan 1970
// GetTime() will return the number of seconds between the current CTime object and January 1, 1970.
INT64 CSFS::CalculateTime(void) {
  CTime time = CTime::GetCurrentTime();
  return ((INT64) time.GetTime()) * 65536;
}

void CSFS::OnSfsCopy() {
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

void CSFS::CopyFile(HTREEITEM hItem, CString csName) {
  DWORD FileSize;
  void *ptr = ReadFile(hItem, &FileSize);
  if (ptr) {
    CFile file;
    if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
      file.Write(ptr, FileSize);
      file.Close();
    } else
      AfxMessageBox("Error Creating File...");
    free(ptr);
  }
}

// hItem = tree item of folder
// csPath = existing path to create folder in
// csName = name of folder to create
void CSFS::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CSFS::OnSfsInsert() {
  char szPath[MAX_PATH];
  BOOL IsDir;
  CString csName, csPath;
  
  // get a file/directory from the host
  csName = AfxGetApp()->GetProfileString("Settings", "DefaultExtractPath", NULL);
  if (!BrowseForFolder(GetSafeHwnd(), csName, szPath, BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES))
    return;
  csPath = szPath;
  AfxGetApp()->WriteProfileString("Settings", "DefaultExtractPath", csPath);
  // at this point:
  //  csPath = full path to file/folder selected
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
  if (IsDir)
    InsertFolder(csFPath, csName, csPath);
  else
    InsertFile(csFPath, csName, csPath);
  wait.Restore();
  //AfxMessageBox("Files transferred.");
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// csFPath = path on guest image to place file
// csName = name of file to insert
// csPath = path on host of file to insert
void CSFS::InsertFile(CString csFPath, CString csName, CString csPath) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *buffer;
  size_t size;
  CFile file;
  
  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return;
  }
  size = (size_t) file.GetLength();
  buffer = malloc(size + dlg->m_sect_size);  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, (UINT) size);
  file.Close();
  
  // append a file entry to the index
  DWORD64 Block = AppendFileToIndex(csFPath, csName, (DWORD) size, FALSE);
  
  // append file data to the data blocks
  WriteFile(buffer, Block, (DWORD) size);
  
  // free the buffer
  free(buffer);
}

// csFPath = path on guest image to place file
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CSFS::InsertFolder(CString csFPath, CString csName, CString csPath) {
  const unsigned size = 8192;
  char szPath[MAX_PATH];
  CString csNewPath;
  
  // new path to store the nested files (if any)
  csNewPath = csFPath + "/";
  csNewPath += csName;
  
  // append a dir entry to the index
  AppendFileToIndex(csFPath, csName, 0, TRUE);
  
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
      InsertFolder(csNewPath, filefind.GetFileName(), filefind.GetFilePath());
    else
      InsertFile(csNewPath, filefind.GetFileName(), filefind.GetFilePath());
  }
  filefind.Close();
  
  // restore the current directory
  SetCurrentDirectory(szPath);
}

// the user change the status of the "Show Deleted" Check box
void CSFS::OnShowDeleted() {
  AfxGetApp()->WriteProfileInt("Settings", "SFSShowDel", m_show_del = IsDlgButtonChecked(IDC_SHOW_DEL));
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// the user change the status of the "Delete Clear" Check box
void CSFS::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "SFSDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void CSFS::OnSfsDelete() {
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
  
  // delete the item from the tree
  if (IsDlgButtonChecked(IDC_SHOW_DEL))
    Start(m_lba, m_size, m_color, m_index, FALSE);
  else
    m_dir_tree.DeleteItem(hItem);
  
  // select the parent item
  m_dir_tree.Select((hParent != NULL) ? hParent : TVI_ROOT, TVGN_CARET);
  
  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CSFS::DeleteFolder(HTREEITEM hItem) {
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
      if (IsDir == 1)
        DeleteFolder(hDeleteItem);
      else 
        DeleteFile(hDeleteItem);
    }
  }
  DeleteFile(hItem);
}

void CSFS::DeleteFile(HTREEITEM hItem) {
  struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  
  if (items == NULL)
    return;
  
  const unsigned offset = (m_indx_size * 512) - (unsigned) m_super.index_size;
  BYTE *p = m_indx_buffer + offset + (items->index * SFS_ENTRY_SIZE);
  
  switch (p[0]) {
    case SFS_ENTRY_VOL_ID:
      AfxMessageBox("Should not delete the Volume ID");
      return;
    case SFS_ENTRY_START:
      AfxMessageBox("Should not delete the Start Entry");
      return;
    case SFS_ENTRY_UNUSABLE:
      AfxMessageBox("Should not delete the 'Unusable' Entry");
      return;
    case SFS_ENTRY_DIR:
      p[0] = SFS_ENTRY_DIR_DEL;
      p[1] = 0;
      p[1] = -CalculateCRC(p, SFS_ENTRY_SIZE + (p[2] * SFS_ENTRY_SIZE));
      break;
    case SFS_ENTRY_FILE:
      p[0] = SFS_ENTRY_FILE_DEL;
      p[1] = 0;
      p[1] = -CalculateCRC(p, SFS_ENTRY_SIZE + (p[2] * SFS_ENTRY_SIZE));
      if (m_del_clear) {
        struct S_SFS_FILE *eFile = (struct S_SFS_FILE *) p;
        ZeroFile(eFile->start_block, eFile->end_block);
      }
      break;
    case SFS_ENTRY_DIR_DEL:
    case SFS_ENTRY_FILE_DEL:
      return;
    default:
      AfxMessageBox("Internal Error: Invalid selection...");
      return;
  }
  
  // write back the index
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->WriteToFile(m_indx_buffer, m_lba + m_indx_start, m_indx_size);
  
  m_dir_tree.DeleteItem(hItem);
}

void CSFS::OnSearch() {
  m_dir_tree.Search();
}

BOOL CSFS::IsFileOrFolder(HTREEITEM hItem) {
  BOOL ret = FALSE;
  
  if (hItem == NULL)
    return FALSE;
  
  struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items == NULL)
    return FALSE;
  
  switch (items->entry[0]) {
    case SFS_ENTRY_DIR:
    case SFS_ENTRY_FILE:
      ret = TRUE;
      break;
    /*
    case SFS_ENTRY_UNUSABLE:
    case SFS_ENTRY_START:
    case SFS_ENTRY_VOL_ID:
    case SFS_ENTRY_DIR_DEL:
    case SFS_ENTRY_FILE_DEL:
    default:
      ret = FALSE;
    */
  }
  
  return ret;
}

// appends a File Entry to the end of the index, then writes
//  the index back to the image
// csFPath = path this file goes into (not counting ending '/')
// csName = name of this file
// Size = size of file to insert
DWORD64 CSFS::AppendFileToIndex(CString csFPath, CString csName, DWORD Size, BOOL IsDir) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CString csNewName;
  DWORD64 ret = 0, fnd_ret = 0;
  unsigned offset;
  void *ptr = NULL;

  // csNewName = new entry name
  csFPath.Replace('\\', '/');
  csNewName = csFPath;
  csNewName += "/";
  csNewName += csName;
  while (csNewName.GetAt(0) == '/')
    csNewName = csNewName.Right(csNewName.GetLength()-1);
  
  // cnt = count of entries needed
  int cnt = (((SFS_ENTRY_SIZE - FILE_NAME_LEN) + csNewName.GetLength() + 1) + (SFS_ENTRY_SIZE - 1)) / SFS_ENTRY_SIZE;  // + 1 for the NULL at the end of the name
  
  // let's search the current entries and see if there is a deleted block that we can use
  bool fnd = FALSE;
  offset = (m_indx_size * 512) - (unsigned) m_super.index_size;
  while (!fnd && (offset < (m_indx_size * 512))) {
    int j = 1 + m_indx_buffer[offset + 2];
    if (j == 0) break;  // catch an error (if any)
    switch (m_indx_buffer[offset]) {
      case SFS_ENTRY_FILE_DEL:
        if (j >= cnt) {
          ptr = (void *) &m_indx_buffer[offset];
          // if original entry was larger, we have to divide it to make two: 
          //  the new one and a smaller original one.
          if (j > cnt) {
            int extra = cnt - j;
            m_indx_buffer[offset + (extra * SFS_ENTRY_SIZE)] = m_indx_buffer[offset];
            m_indx_buffer[offset + (extra * SFS_ENTRY_SIZE) + 2] = extra - 1;
          }
          fnd = TRUE;
          // if the space the deleted file occupied is large enough to hold the new one, use it
          struct S_SFS_FILE_DEL *file = (struct S_SFS_FILE_DEL *) ptr;
          if (file->file_len >= Size)
            fnd_ret = file->start_block;
          break;
        } // else, fall through
      case SFS_ENTRY_DIR_DEL:
      case SFS_ENTRY_DIR:
      case SFS_ENTRY_FILE:
        offset += (j * SFS_ENTRY_SIZE);
        break;
      default:
        offset += SFS_ENTRY_SIZE;
    }
  }
  
  if (!fnd) {
    // there should not be any empty entries at the end of the list,
    //  and the last entry must be the Volume Label entry.
    // therefore, any entry appended to the list, must be after all other
    //  entries and before the Label entry.
    offset = (m_indx_size * 512) - (unsigned) m_super.index_size;
    for (int j=0; j<cnt; j++) {
      if (offset > 0) {
        memmove(m_indx_buffer + offset - SFS_ENTRY_SIZE, m_indx_buffer + offset, (size_t) (m_super.index_size - SFS_ENTRY_SIZE));
        offset -= SFS_ENTRY_SIZE;
      } else {
        if ((m_indx_start - 1) > (m_super.resv_blocks + m_super.data_block_count)) {
          m_indx_start--;
          m_indx_size++;   // increment the count of sectors it has
          m_indx_buffer = (BYTE *) realloc(m_indx_buffer, m_indx_size * 512);
          offset = 512 - SFS_ENTRY_SIZE;
          // move all entries (including the label) forward
          memmove(offset + m_indx_buffer, m_indx_buffer, (size_t) m_super.index_size);
          // move the label entry to the end of the buffer
          memmove(offset + m_indx_buffer + m_super.index_size, offset + m_indx_buffer + m_super.index_size - SFS_ENTRY_SIZE, SFS_ENTRY_SIZE);
          // clear the "unused" part at the first so that we don't mistakenly find the start before we should
          memset(m_indx_buffer, 0, offset);
        } else {
          AfxMessageBox("Index start will crash into blocks.  No more room.");
          return 0;
        }
      }
      m_super.index_size += SFS_ENTRY_SIZE;
    }
    ptr = (offset + (m_indx_buffer + m_super.index_size - (cnt * SFS_ENTRY_SIZE) - SFS_ENTRY_SIZE));
  }
  memset(ptr, 0, (cnt * SFS_ENTRY_SIZE));
  
  const unsigned block_size = (1 << (m_super.block_size + 7));
  if (IsDir) {
    struct S_SFS_DIR *dir = (struct S_SFS_DIR *) ptr;
    dir->type = SFS_ENTRY_DIR;
    dir->num_cont = cnt - 1;
    dir->time_stamp = CalculateTime();
    strcpy((char *) dir->name, csNewName);
    dir->crc = -CalculateCRC(dir, (cnt * SFS_ENTRY_SIZE));
  } else {  
    // add the new file entry
    struct S_SFS_FILE *file = (struct S_SFS_FILE *) ptr;
    file->type = SFS_ENTRY_FILE;
    file->num_cont = cnt - 1;
    file->time_stamp = CalculateTime();
    // if we haven't already chosen a position
    ret = (fnd_ret == 0) ? (m_super.resv_blocks + m_super.data_block_count) : fnd_ret;
    file->start_block = ret;
    file->end_block = file->start_block + ((Size + (block_size - 1)) / block_size) - 1;
    file->file_len = Size;
    strcpy((char *) file->name, csNewName);
    file->crc = -CalculateCRC(file, (cnt * SFS_ENTRY_SIZE));
  }
  
  // write back the index
  dlg->WriteToFile(m_indx_buffer, m_lba + m_indx_start, m_indx_size);
  
  // update the block count
  if (fnd_ret == 0) {
    m_super.data_block_count += ((Size + (block_size - 1)) / block_size);
    // make sure we didn't "crash" into Index
    if ((m_super.resv_blocks + m_super.data_block_count) > m_indx_start) {
      AfxMessageBox("Block Count will crash into index.  No more room.");
      return 0;
    }
  }
  
  // write back the super
  BYTE buffer[MAX_SECT_SIZE];
  dlg->ReadFromFile(buffer, m_lba, 1);
  memcpy(buffer + SFS_SUPER_LOC, &m_super, sizeof(struct S_SFS_SUPER));
  dlg->WriteToFile(buffer, m_lba, 1);
  
  // return the block to write the file to (if a file), else 0
  return ret;
}

void CSFS::OnSfsEntry() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CSFSEntry SFSEntry;
  
  if (hItem) {
    struct S_SFS_ITEMS *items = (struct S_SFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      SFSEntry.m_type = items->entry[0];
      SFSEntry.m_index_block = m_indx_buffer;
      SFSEntry.m_index_size = (int) (m_super.index_size / SFS_ENTRY_SIZE);
      SFSEntry.m_index = items->index;
      memcpy(&SFSEntry.m_entry, items->entry, SFS_ENTRY_SIZE);
      if (SFSEntry.DoModal() == IDOK) { // apply button pressed?
        // TODO:
      }
    }
  }
}

void CSFS::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CSFS::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CSFS::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CSFS::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1);
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}
