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

// Ext2.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "LeanTime.h"
#include "Attribute.h"

#include "Ext2.h"
#include "Ext2Dyn.h"
#include "Ext2Grp.h"
#include "Ext2Entry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExt2 property page

IMPLEMENT_DYNCREATE(CExt2, CPropertyPage)

CExt2::CExt2() : CPropertyPage(CExt2::IDD) {
  //{{AFX_DATA_INIT(CExt2)
  m_block_count = _T("");
  m_blocks_group = _T("");
  m_creator_os = _T("");
  m_error_b = _T("");
  m_first_data_block = _T("");
  m_frags_group = _T("");
  m_free_block_count = _T("");
  m_free_inode_count = _T("");
  m_gid = _T("");
  m_inode_count = _T("");
  m_inodes_group = _T("");
  m_last_check = _T("");
  m_last_check_int = _T("");
  m_log_block_size = _T("");
  m_log_frag_size = _T("");
  m_magic = _T("");
  m_max_mount_count = _T("");
  m_min_rev_level = _T("");
  m_mount_count = _T("");
  m_mount_time = _T("");
  m_r_block_count = _T("");
  m_rev_level = _T("");
  m_state = _T("");
  m_uid = _T("");
  m_write_time = _T("");
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_desc_table = NULL;
  m_cur_inode_table = NULL;
  m_block_table = NULL;
  m_cur_group = -1;
  m_block_table_inode = -1;
  m_inode_dirty = FALSE;
  m_hard_format = FALSE;
  m_free_blocks = 0;
}

CExt2::~CExt2() {
  if (m_desc_table)
    free(m_desc_table);
  if (m_cur_inode_table)
    free(m_cur_inode_table);
  if (m_block_table)
    free(m_block_table);
  
  m_desc_table = NULL;
  m_cur_inode_table = NULL;
  m_block_table = NULL;
  m_free_blocks = 0;
}

void CExt2::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExt2)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_BLOCK_COUNT, m_block_count);
  DDX_Text(pDX, IDC_BLOCKS_GROUP, m_blocks_group);
  DDX_Text(pDX, IDC_CREATOR_OS, m_creator_os);
  DDX_Text(pDX, IDC_ERROR_B, m_error_b);
  DDX_Text(pDX, IDC_FIRST_DATA_BLOCK, m_first_data_block);
  DDX_Text(pDX, IDC_FRAGS_GROUP, m_frags_group);
  DDX_Text(pDX, IDC_FREE_BLOCK_COUNT, m_free_block_count);
  DDX_Text(pDX, IDC_FREE_INODE_COUNT, m_free_inode_count);
  DDX_Text(pDX, IDC_GID, m_gid);
  DDX_Text(pDX, IDC_INODE_COUNT, m_inode_count);
  DDX_Text(pDX, IDC_INODES_GROUP, m_inodes_group);
  DDX_Text(pDX, IDC_LAST_CHECK, m_last_check);
  DDX_Text(pDX, IDC_LAST_CHECK_INT, m_last_check_int);
  DDX_Text(pDX, IDC_LOG_BLOCK_SIZE, m_log_block_size);
  DDX_Text(pDX, IDC_LOG_FRAG_SIZE, m_log_frag_size);
  DDX_Text(pDX, IDC_MAGIC, m_magic);
  DDX_Text(pDX, IDC_MAX_MOUNT_COUNT, m_max_mount_count);
  DDX_Text(pDX, IDC_MIN_REV_LEVEL, m_min_rev_level);
  DDX_Text(pDX, IDC_MOUNT_COUNT, m_mount_count);
  DDX_Text(pDX, IDC_MOUNT_TIME, m_mount_time);
  DDX_Text(pDX, IDC_R_BLOCK_COUNT, m_r_block_count);
  DDX_Text(pDX, IDC_REV_LEVEL, m_rev_level);
  DDX_Text(pDX, IDC_STATE, m_state);
  DDX_Text(pDX, IDC_UID, m_uid);
  DDX_Text(pDX, IDC_WRITE_TIME, m_write_time);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExt2, CPropertyPage)
  //{{AFX_MSG_MAP(CExt2)
  ON_WM_HELPINFO()
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(IDC_DYN_REV, OnDynRev)
  ON_BN_CLICKED(IDC_GROUPS, OnGroups)
  ON_BN_CLICKED(ID_APPLY, OnExt2Apply)
  ON_BN_CLICKED(ID_COPY, OnExt2Copy)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_ENTRY, OnExt2Entry)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_FORMAT, OnFormat)
  ON_BN_CLICKED(ID_CLEAN, OnClean)
  ON_BN_CLICKED(ID_CHECK, OnCheck)
  ON_BN_CLICKED(ID_MOUNT_TIME, OnMountTime)
  ON_BN_CLICKED(ID_LAST_CHECK, OnLastCheck)
  ON_BN_CLICKED(ID_WRITE_TIME, OnWriteTime)
  ON_BN_CLICKED(ID_LAST_CHECK_INT, OnLastCheckInt)
  ON_BN_CLICKED(ID_CREATOR_OS, OnCreatorOs)
  ON_BN_CLICKED(ID_STATE, OnState)
  ON_BN_CLICKED(ID_ERROR_B, OnErrorB)
  ON_EN_CHANGE(IDC_REV_LEVEL, OnChangeRevLevel)
  ON_BN_CLICKED(ID_MAGIC, OnMagic)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExt2 message handlers
BOOL CExt2::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    case IDC_EXPAND:
      strTipText = "Expand all Folders";
      break;
    case IDC_COLAPSE:
      strTipText = "Collapse all Folders";
      break;

    case ID_FORMAT:
      strTipText = "Format Partition";
      break;
    case ID_CLEAN:
      strTipText = "Clean Partition (Delete All)";
      break;
    case ID_CHECK:
      strTipText = "Check File System items";
      break;
    case ID_UPDATE_CODE:
      strTipText = "Write a new Boot Code Image to partition";
      break;
    
    case ID_ENTRY:
      strTipText = "View/Modify FAT Entry";
      break;
    case ID_COPY:
      strTipText = "Extract Folder/File from Partition to Host";
      break;
    case ID_INSERT:
      strTipText = "Insert Folder/File from Host to Partition";
      break;
    case ID_DELETE:
      strTipText = "Delete Folder/File from Partition";
      break;
    case ID_SEARCH:
      strTipText = "Search for Folder/File in Partition";
      break;
    case ID_VIEW:
      strTipText = "View File in Host's default Viewer";
      break;
    case IDC_SHOW_DEL:
      strTipText = "Show Deleted Entries";
      break;
    case IDC_DEL_CLEAR:
      strTipText = "Zero File Contents on Delete";
      break;

    //    case IDC_INSERT_VLABEL:
    //      //strTipText.Format("Insert Boot Code at LBA %i", GetDlgItemInt(IDC_DI_LBA, 0, FALSE));
    //      strTipText = "Insert Volume Label into Root Directory";
    //      break;

    case ID_ERASE:
      strTipText = "Erase whole Partition";
      break;
    case ID_FYSOS_SIG:
      strTipText = "Insert FYSOS Boot Signature";
      break;
    case ID_APPLY:
      strTipText = "Save modifications";
      break;
  }

  strncpy(pTTTA->szText, strTipText, 79);
  pTTTA->szText[79] = '\0';  // make sure it is null terminated

  return TRUE; // message was handled
}

BOOL CExt2::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "EXT2DelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  EnableToolTips(TRUE);

  return TRUE;
}

BOOL CExt2::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "ext4.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CExt2::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_EXT2_ITEMS *items = (struct S_EXT2_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow((hItem != NULL) && (m_dir_tree.GetParentItem(hItem) != NULL));
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE /*(hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0)*/);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE /*hItem != NULL*/);
  
  *pResult = 0;
}

// Lean colors will have a red shade to them.
DWORD CExt2::GetNewColor(int index) {
  int r = ((149 - (index * 2)) > -1) ? (149 - (index * 2)) : 0;
  int g = ((126 - (index * 18)) > -1) ? (126 - (index * 18)) : 0;
  int b = ((96 - (index * 20)) > -1) ? (96 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CExt2::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE * 2];
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  m_cur_group = -1;
  m_block_table_inode = -1;
  m_inode_dirty = FALSE;
  
  // read in the superblock (1024 bytes)
  dlg->ReadFromFile(buffer, m_lba + 2, 2);
  memcpy(&m_super, buffer, sizeof(struct S_EXT2_SUPER));
  
  // detect if Ext2, 3, or 4
  m_ext2_size = 2;
  if (m_super.feature_incompat & (EXT3_FEATURE_INCOMPAT_RECOVER | EXT3_FEATURE_INCOMPAT_JOURNAL_DEV | EXT3_FEATURE_INCOMPAT_EXTENTS))
    m_ext2_size = 3; 
  if (m_super.feature_compat & EXT3_FEATURE_COMPAT_HAS_JOURNAL)
    m_ext2_size = 3; 
  if (m_super.feature_incompat & (EXT4_FEATURE_INCOMPAT_64BIT | EXT4_FEATURE_INCOMPAT_MMP | EXT4_FEATURE_INCOMPAT_FLEX_BG))
    m_ext2_size = 4; 
  if (m_super.feature_ro_compat & (EXT4_FEATURE_RO_COMPAT_HUGE_FILE | EXT4_FEATURE_RO_COMPAT_GDT_CSUM | EXT4_FEATURE_RO_COMPAT_DIR_NLINK | EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE))
    m_ext2_size = 4;
  
  // get the inode size
  if (m_super.rev_level == EXT2_GOOD_OLD_REV)
    m_inode_size = 128;
  else
    m_inode_size = (m_super.inode_size <= 256) ? m_super.inode_size : 256;  // we only support up to a 256-byte inode

  // Read in the group descriptor table
  GetGroupDescTable();
  
  // Read in the first group's Inode Table
  GetGroupInodeTable(0);
  
  m_psp.dwFlags |= PSP_USETITLE;
  dlg->m_Ext2Names[index].Format("Ext %i", m_ext2_size);
  m_psp.pszTitle = dlg->m_Ext2Names[index];
  dlg->m_image_bar.UpdateTitle(dlg->Ext2[index].m_draw_index, (char *) (LPCTSTR) dlg->m_Ext2Names[index]);
  
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
  GetDlgItem(IDC_DYN_REV)->EnableWindow(m_super.rev_level == EXT2_DYNAMIC_REV);
  GetDlgItem(IDC_GROUPS)->EnableWindow(m_groups > 0);
  
  if (m_isvalid) {
    m_RootInode = FindRootInode(0);
    if (m_RootInode > -1) {
      GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
      
      // make sure the tree is emtpy
      m_dir_tree.DeleteAllItems();
      m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
      m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
      
      UpdateWindow();
      
      // fill the tree with the directory
      struct S_EXT2_INODE inode;
      unsigned group = Ext2GetInode(m_RootInode, &inode);
      void *root = Ext2ReadFile(&inode, m_RootInode, group);
      if (root) {
        CWaitCursor wait; // display a wait cursor
        SaveItemInfo(m_hRoot, (unsigned) m_RootInode, FALSE);
        m_too_many = FALSE;
        ParseDir(root, INODE_SIZE(&inode), m_hRoot);
        m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
        free(root);
        wait.Restore();
        GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
        GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
      }
    } else
      AfxMessageBox("Did not find root Inode");

    m_free_blocks = CalcFreeBlocks();
  }

  // display the free space
  DisplayFreeSpace();
}

void CExt2::ParseDir(void *root, DWORD64 root_size, HTREEITEM parent) {
  struct S_EXT2_DIR *cur = (struct S_EXT2_DIR *) root, *sub;
  struct S_EXT2_INODE inode;
  HTREEITEM hItem;
  unsigned group;
  CString name;
  int name_len;
  BYTE file_type;
  
  // if (EXT2_FEATURE_INCOMPAT_FILETYPE) use S_EXT2_DIR or S_EXT2_DIR_OLD
  const BOOL oldtype = (m_super.feature_incompat & EXT2_FEATURE_INCOMPAT_FILETYPE) == 0;
  
  while (((BYTE *) cur < ((BYTE *) root + root_size)) && !m_too_many) {
    if (cur->rec_len == 0)
      break;
    
    if (cur->inode > 0) {
      name_len = (oldtype) ? (* (WORD *) &cur->name_len) : cur->name_len;
    
      // retrieve the name.
      name.Empty();
      BYTE *p = (BYTE *) cur + sizeof(S_EXT2_DIR);
      for (int i=0; i<name_len; i++)
        name += p[i];
    
      // do we have to get the type from the inode?
      if (oldtype) {
        Ext2GetInode(cur->inode - 1, &inode);
        switch (inode.mode & EXT2_S_IFMT) {
          case EXT2_S_IFSOCK: file_type = EXT2_FT_SOCK; break;
          case EXT2_S_IFLNK:  file_type = EXT2_FT_SYMLINK; break;
          case EXT2_S_IFREG:  file_type = EXT2_FT_REG_FILE; break;
          case EXT2_S_IFBLK:  file_type = EXT2_FT_BLKDEV; break;
          case EXT2_S_IFDIR:  file_type = EXT2_FT_DIR; break;
          case EXT2_S_IFCHR:  file_type = EXT2_FT_CHRDEV; break;
          case EXT2_S_IFIFO:  file_type = EXT2_FT_FIFO; break;
          default: file_type = EXT2_FT_UNKNOWN;
        }
      } else
        file_type = cur->file_type;
      
      BOOL IsDot = ((name == ".") || (name == ".."));
    
      switch (file_type) {
        case EXT2_FT_UNKNOWN:  // File type: Empty
          break;
        case EXT2_FT_REG_FILE: // File type: regular file
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, TRUE);
          break;
        case EXT2_FT_DIR:    // File type: directory
          hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, !IsDot);
          if (!IsDot) {
            group = Ext2GetInode(cur->inode - 1, &inode);
            sub = (struct S_EXT2_DIR *) Ext2ReadFile(&inode, cur->inode - 1, group);
            if (sub) {
              ParseDir(sub, INODE_SIZE(&inode), hItem);
              free(sub);
            }
          }
          break;
        case EXT2_FT_CHRDEV:  // character device
          name += " (char dev)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DEVICE, IMAGE_DEVICE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
          break;
        case EXT2_FT_BLKDEV:
          name += " (block dev)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DEVICE, IMAGE_DEVICE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
          break;
        case EXT2_FT_FIFO:
          name += " (fifo)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DEVICE, IMAGE_DEVICE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
          break;
        case EXT2_FT_SOCK: // File type: socket
          name += " (socket)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DEVICE, IMAGE_DEVICE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
          break;
        case EXT2_FT_SYMLINK: // File type: symbolic link
          //name += " (Link)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FORK, IMAGE_SYMBOLIC, IMAGE_SYMBOLIC, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
          break;
        default:
          name.Format("TODO: type = 0x%02X", file_type);
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode - 1, FALSE);
      }
    }
    
    cur = (struct S_EXT2_DIR *) ((BYTE *) cur + cur->rec_len);
  }
}

void CExt2::OnSearch() {
  m_dir_tree.Search();
}

// Read in the group descriptor table
void CExt2::GetGroupDescTable(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  int group_desc_size = (m_ext2_size == 4) ? 64 : 32;
  m_block_size = (1024 << m_super.log_block_size);
  // TODO: what if   fdata->super->block_group_nr  != 0  (i.e: what if the super isn't in block 0?)
  DWORD group_desc_block = (m_block_size == 1024) ? 2 : 1;
  m_groups = (int) ((m_super.blocks_count + (m_super.blocks_per_group - 1)) / m_super.blocks_per_group);
  m_desc_table = (struct S_EXT2_GROUP_DESC *) realloc(m_desc_table, ((m_groups * group_desc_size) + (m_block_size - 1)) & ~(m_block_size - 1));
  m_sectors_per_block = (m_block_size / dlg->m_sect_size);
  int sectors_per_group_desc = (int) ((((m_groups * group_desc_size) + (m_block_size - 1)) / m_block_size) * m_sectors_per_block);
  dlg->ReadFromFile(m_desc_table, m_lba + (group_desc_block * m_sectors_per_block), sectors_per_group_desc);
}

// Write the group descriptor table
void CExt2::WriteGroupDescTable(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  int group_desc_size = (m_ext2_size == 4) ? 64 : 32;
  DWORD group_desc_block = (m_block_size == 1024) ? 2 : 1;
  int sectors_per_group_desc = (int) ((((m_groups * group_desc_size) + (m_block_size - 1)) / m_block_size) * m_sectors_per_block);
  dlg->WriteToFile(m_desc_table, m_lba + (group_desc_block * m_sectors_per_block), sectors_per_group_desc);
}

void CExt2::GetGroupInodeTable(const int group) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  const int size = (m_super.feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) ? 64 : 32;
  struct S_EXT2_GROUP_DESC *desc;
  
  // have we already read in this one?
  if (group == m_cur_group)
    return;

  // do we need to write the current buffer first?
  if (m_inode_dirty)
    WriteGroupInodeTable(m_cur_group);
  
  // now read in the desired group's inode table
  desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) m_desc_table + (group * size));
  m_inode_table_size = ((m_super.inodes_per_group * m_inode_size) + (dlg->m_sect_size - 1)) & ~(dlg->m_sect_size - 1);
  m_cur_group = group;
  if (m_cur_inode_table)
    free(m_cur_inode_table);
  m_cur_inode_table = malloc(m_inode_table_size);
  
  dlg->ReadFromFile(m_cur_inode_table, m_lba + (desc->inode_table * m_sectors_per_block), m_inode_table_size / dlg->m_sect_size);
}

void CExt2::WriteGroupInodeTable(const int group) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  const int size = (m_super.feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) ? 64 : 32;
  struct S_EXT2_GROUP_DESC *desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) m_desc_table + (group * size));
  
  dlg->WriteToFile(m_cur_inode_table, m_lba + (desc->inode_table * m_sectors_per_block), m_inode_table_size / dlg->m_sect_size);
  m_inode_dirty = FALSE;
}

int CExt2::FindRootInode(const int group) {
  GetGroupInodeTable(group);
  
  int i = 0;
  struct S_EXT2_INODE *inode = (struct S_EXT2_INODE *) m_cur_inode_table;
  while ((BYTE *) inode < ((BYTE *) m_cur_inode_table + m_inode_table_size)) {
    if ((inode->mode & EXT2_S_IFMT) == EXT2_S_IFDIR)
      return i;
    inode = (struct S_EXT2_INODE *) ((BYTE *) inode + m_inode_size);
    i++;
  }
  
  return -1;
}

void CExt2::GetInodeBlocks(unsigned InodeNum, struct S_EXT2_INODE *inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  unsigned i;
  
  // if we already have this one, just return
  if (m_block_table_inode == InodeNum)
    return;
  
  const unsigned entries_per_block = (m_block_size / 4);
  unsigned block_count = 
  m_block_tab_count = (unsigned) (INODE_SIZE(inode) + (m_block_size - 1)) / m_block_size;
  if (m_block_table) free(m_block_table);
  m_block_table = (DWORD64 *) malloc(m_block_tab_count * sizeof(DWORD64));
  m_block_table_inode = InodeNum;
  
  DWORD64 *p = m_block_table;
  if (inode->flags & EXT4_EXTENTS_FL) {
    if (inode->extents.extent_hdr.depth == 0) {
      struct S_EXT3_EXTENT *extent = (struct S_EXT3_EXTENT *) inode->extents.extents;
      for (i=0; i<inode->extents.extent_hdr.entries; i++)
        for (int j=0; j<extent[i].len; j++)
          p[extent[i].block + j] = ((DWORD64) extent[i].start_hi << 32) + extent[i].start + j;
    } else {
      struct S_EXT3_EXTENT_IDX *extent_idx = (struct S_EXT3_EXTENT_IDX *) inode->extents.extents;
      
      /// I am pretty sure these notes are correct
      ////  inode->extents.extent_hdr.depth tells you how deep to the extents.
      //  = 1:  inode contains 1 extent_idx that points to a block of extents
      //  = 2:  inode contains 1 extent_idx that points to a block of extent_idx's which each point to a block of extents
      //  = 3:  and so on
      ////// each block should have a header at it too.
      AfxMessageBox("TODO: S_EXT3_EXTENT_IDX");
      
    }
  } else { // uses blocks
    for (i=0; i<12 && block_count; i++) {
      *p++ = inode->block_array.block[i];
      block_count--;
    }
    
    // if block_count > 0, we need to read in the indirect blocks
    DWORD *indirect = (DWORD *) malloc(entries_per_block * 4);
    if (block_count) {
      dlg->ReadFromFile(indirect, m_lba + (inode->block_array.indirect_block * m_sectors_per_block), m_sectors_per_block);
      for (i=0; i<entries_per_block && block_count; i++) {
        *p++ = indirect[i];
        block_count--;
      }
    }
    
    // if block_count > 0, we need to read in the dbl indirect blocks
    if (block_count) {
      DWORD *dbl = (DWORD *) malloc(entries_per_block * 4);
      dlg->ReadFromFile(dbl, m_lba + (inode->block_array.dbl_indirect_block * m_sectors_per_block), m_sectors_per_block);
      i = 0;
      while (block_count && (i<entries_per_block)) {
        dlg->ReadFromFile(indirect, m_lba + (dbl[i] * m_sectors_per_block), m_sectors_per_block);
        for (unsigned j=0; j<entries_per_block && block_count; j++) {
          *p++ = indirect[j];
          block_count--;
        }
        i++;
      }
      free(dbl);
    }
    free(indirect);
    
    // if block_count > 0, we need to read in the trpl indirect blocks
    // TODO: Tripple indirect blocks
    // TODO: I think we can combine the above code to do all three types with one block of code.
    // TODO: Are Trpl indirect blocks only an Ext4 thing?
    if (block_count) {
      CString cs;
      cs.Format(" TODO: Tripple indirect blocks... (%i)", block_count);
      AfxMessageBox(cs);
    }
  }
}

DWORD64 CExt2::Ext2GetBlockNum(unsigned InodeNum, struct S_EXT2_INODE *inode, unsigned index) {
  GetInodeBlocks(InodeNum, inode);
  return m_block_table[index];
}

void *CExt2::Ext2ReadFile(struct S_EXT2_INODE *inode, const unsigned InodeNum, const unsigned group) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  const unsigned block_count = (unsigned) ((INODE_SIZE(inode) + (m_block_size - 1)) / m_block_size);
  void *buffer = malloc(block_count * m_block_size);
  BYTE *p = (BYTE *) buffer;
  for (unsigned i=0; i<block_count; i++) {
    DWORD64 block = Ext2GetBlockNum(InodeNum, inode, i);
    dlg->ReadFromFile(p, m_lba + (block * m_sectors_per_block), m_sectors_per_block);
    p += m_block_size;
  }
  return buffer;
}

void CExt2::Ext2PutInode(const unsigned InodeNum, struct S_EXT2_INODE *inode) {
  const unsigned group = InodeNum / m_super.inodes_per_group;
  const unsigned in_num = (InodeNum - (group * m_super.inodes_per_group));
  
  GetGroupInodeTable(group);
  memcpy((BYTE *) m_cur_inode_table + (in_num * m_inode_size), inode, m_inode_size);
  m_inode_dirty = TRUE;
}

unsigned CExt2::Ext2GetInode(const unsigned InodeNum, struct S_EXT2_INODE *inode) {
  const unsigned group = InodeNum / m_super.inodes_per_group;
  const unsigned in_num = (InodeNum - (group * m_super.inodes_per_group));

  GetGroupInodeTable(group);
  memcpy(inode, (BYTE *) m_cur_inode_table + (in_num * m_inode_size), m_inode_size);

  return group;
}

void CExt2::SendToDialog(struct S_EXT2_SUPER *super) {
  m_block_count.Format("%i", super->blocks_count);
  m_blocks_group.Format("%i", super->blocks_per_group);
  m_creator_os.Format("%i", super->creator_os);
  m_error_b.Format("%i", super->errors);
  m_first_data_block.Format("%i", super->first_data_block);
  m_frags_group.Format("%i", super->frags_per_group);
  m_free_block_count.Format("%i", super->free_blocks_count);
  m_free_inode_count.Format("%i", super->free_inodes_count);
  m_gid.Format("0x%04X", super->def_resgid);
  m_inode_count.Format("%i", super->inodes_count);
  m_inodes_group.Format("%i", super->inodes_per_group);
  m_last_check.Format("%i", super->lastcheck);
  m_last_check_int.Format("%i", super->checkinterval);
  m_log_block_size.Format("%i", super->log_block_size);
  m_log_frag_size.Format("%i", super->log_frag_size);
  m_magic.Format("0x%04X", super->magic);
  m_max_mount_count.Format("%i", super->max_mnt_count);
  m_min_rev_level.Format("%i", super->minor_rev_level);
  m_mount_count.Format("%i", super->mnt_count);
  m_mount_time.Format("%i", super->mtime);
  m_r_block_count.Format("%i", super->r_blocks_count);
  m_rev_level.Format("%i", super->rev_level);
  m_state.Format("0x%04X", super->state);
  m_uid.Format("0x%04X", super->def_resuid);
  m_write_time.Format("%i", super->wtime);
  
  UpdateData(FALSE); // send to Dialog
}

void CExt2::ReceiveFromDialog(struct S_EXT2_SUPER *super) {
  UpdateData(TRUE); // receive from Dialog
  
  super->blocks_count = convert32(m_block_count);
  super->blocks_per_group = convert32(m_blocks_group);
  super->creator_os = convert32(m_creator_os);
  super->errors = convert16(m_error_b);
  super->first_data_block = convert32(m_first_data_block);
  super->frags_per_group = convert32(m_frags_group);
  super->free_blocks_count = convert32(m_free_block_count);
  super->free_inodes_count = convert32(m_free_inode_count);
  super->def_resgid = convert16(m_gid);
  super->inodes_count = convert32(m_inode_count);
  super->inodes_per_group = convert32(m_inodes_group);
  super->lastcheck = convert32(m_last_check);
  super->checkinterval = convert32(m_last_check_int);
  super->log_block_size = convert32(m_log_block_size);
  super->log_frag_size = (INT32) convert32(m_log_frag_size);
  super->magic = convert16(m_magic);
  super->max_mnt_count = (INT16) convert16(m_max_mount_count);
  super->minor_rev_level = convert16(m_min_rev_level);
  super->mnt_count = convert16(m_mount_count);
  super->mtime = convert32(m_mount_time);
  super->r_blocks_count = convert32(m_r_block_count);
  super->rev_level = convert32(m_rev_level);
  super->state = convert16(m_state);
  super->def_resuid = convert16(m_uid);
  super->wtime = convert32(m_write_time);
}

void CExt2::OnExt2Entry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CExt2Entry Ext2Entry;
  
  if (hItem) {
    if (hItem == m_hRoot) {
      AfxMessageBox("No entry exists for the root");
      return;
    }
    struct S_EXT2_ITEMS *items = (struct S_EXT2_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      Ext2GetInode(items->InodeNum, &Ext2Entry.m_inode);
      Ext2Entry.m_sectors_per_block = m_sectors_per_block;
      if (Ext2Entry.DoModal() == IDOK)
        Ext2PutInode(items->InodeNum, &Ext2Entry.m_inode);
    }
  }
}

void CExt2::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CExt2::OnDynRev() {
  CExt2Dyn Dyn;
  
  Dyn.m_algo_usage_bitmap.Format("0x%08X", m_super.algorithm_usage_bitmap);
  Dyn.m_block_gr_num.Format("0x%04X", m_super.block_group_nr);
  GUID_Format(Dyn.m_ext2_guid, &m_super.uuid);
  Dyn.m_feature_comp.Format("0x%08X", m_super.feature_compat);
  Dyn.m_feature_incomp.Format("0x%08X", m_super.feature_incompat);
  Dyn.m_feature_ro_comp.Format("0x%08X", m_super.feature_ro_compat);
  Dyn.m_first_inode.Format("%i", m_super.first_ino);
  Dyn.m_inode_size.Format("%i", m_super.inode_size);
  Dyn.m_last_mount = m_super.last_mounted;
  Dyn.m_pre_alloc_blocks.Format("%i", m_super.prealloc_blocks);
  Dyn.m_pre_alloc_dir_blocks.Format("%i", m_super.prealloc_dir_blocks);
  Dyn.m_pre_alloc_gdt_blocks.Format("%i", m_super.reserved_gdt_blocks);
  Dyn.m_vol_name = m_super.volume_name;
  if (Dyn.DoModal() == IDOK) {
    m_super.algorithm_usage_bitmap = convert32(Dyn.m_algo_usage_bitmap);
    m_super.block_group_nr = convert16(Dyn.m_block_gr_num);
    GUID_Retrieve(Dyn.m_ext2_guid, &m_super.uuid);
    m_super.feature_compat = convert32(Dyn.m_feature_comp);
    m_super.feature_incompat = convert32(Dyn.m_feature_incomp);
    m_super.feature_ro_compat = convert32(Dyn.m_feature_ro_comp);
    m_super.first_ino = convert32(Dyn.m_first_inode);
    m_super.inode_size = convert16(Dyn.m_inode_size);
    strcpy(m_super.last_mounted, Dyn.m_last_mount);
    m_super.prealloc_blocks = convert8(Dyn.m_pre_alloc_blocks);
    m_super.prealloc_dir_blocks = convert8(Dyn.m_pre_alloc_dir_blocks);
    m_super.reserved_gdt_blocks = convert16(Dyn.m_pre_alloc_gdt_blocks);
    strcpy(m_super.volume_name, Dyn.m_vol_name);
  }
}

void CExt2::OnChangeRevLevel() {
  CString cs;
  
  GetDlgItemText(IDC_REV_LEVEL, cs);
  int r = convert32(cs);
  
  GetDlgItem(IDC_DYN_REV)->EnableWindow(r == EXT2_DYNAMIC_REV);
}

void CExt2::OnGroups() {
  CExt2Grp Grp;
  
  int group_desc_size = (m_ext2_size == 4) ? 64 : 32;
  int size = ((m_groups * group_desc_size) + (m_block_size - 1)) & ~(m_block_size - 1);
  Grp.m_desc_table = malloc(size);
  memcpy(Grp.m_desc_table, m_desc_table, size);
  Grp.m_groups = m_groups;
  Grp.m_ourflags = m_super.feature_incompat;
  if (Grp.DoModal() == IDOK) {
    memcpy(m_desc_table, Grp.m_desc_table, size);
  }
  free(Grp.m_desc_table);
}

void CExt2::SaveItemInfo(HTREEITEM hItem, const unsigned InodeNum, BOOL CanCopy) {
  struct S_EXT2_ITEMS *items = (struct S_EXT2_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->InodeNum = InodeNum;
    items->CanCopy = CanCopy;
    
  }
}

void CExt2::OnExt2Copy() {
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

void CExt2::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_EXT2_ITEMS *items = (struct S_EXT2_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  struct S_EXT2_INODE inode;
  int group;
  
  if (items) {
    group = Ext2GetInode(items->InodeNum, &inode);
    void *ptr = Ext2ReadFile(&inode, items->InodeNum, group);
    if (ptr) {
      CFile file;
      if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
        file.Write(ptr, (UINT) INODE_SIZE(&inode));
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
void CExt2::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

/*
void CExt2::OnFatInsert() {
  char szPath[MAX_PATH];
  BOOL IsDir, IsRoot, IsRootDir;
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
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsRootDir, csFName, csFPath, FALSE);
  
  // is it the root?
  IsRoot = (IsRootDir == -1);
  
  CWaitCursor wait;
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    if (IsDir)
      InsertFolder(items->Cluster, csName, csPath, IsRoot);
    else
      InsertFile(items->Cluster, csName, csPath, IsRoot);
  }
  
  // need to write the FAT back to the image file
  UpdateTheFATs();
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, m_fat_size, FALSE);
  
  wait.Restore();
  AfxMessageBox("Files transferred.");
}

// Cluster = starting cluster of folder to insert to
// csName = name of file to insert
// csPath = path on host of file to insert
void CExt2::InsertFile(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot) {
  struct S_FAT_ENTRIES fat_entries;
  void *buffer;
  DWORD size;
  CFile file;
  
  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return;
  }
  size = file.GetLength();
  struct S_FAT1216_BPB *bpb = (struct S_FAT1216_BPB *) m_bpb_buffer;
  buffer = malloc(size + (bpb->sect_per_clust * bpb->bytes_per_sect));  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, size);
  file.Close();
  
  // allocate the fat entries for it, returning a "struct S_FAT_ENTRIES"
  if (AllocateFAT(&fat_entries, size) == -1) {
    free(fat_entries.entries);
    free(buffer);
    return;
  }
  
  // create a root entry in the folder
  AllocateRoot(csName, Cluster, fat_entries.entries[0], size, FAT_ATTR_ARCHIVE, IsRoot);
  
  // copy the file to our system
  WriteFile(buffer, &fat_entries, size, FALSE);
  
  // free the buffers
  free(fat_entries.entries);
  free(buffer);
}

// Cluster = starting cluster of folder to insert in to
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CExt2::InsertFolder(DWORD Cluster, CString csName, CString csPath, BOOL IsRoot) {
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

*/

// the user change the status of the "Delete Clear" Check box
void CExt2::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "EXT2DelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

/*
void CExt2::OnFatDelete() {
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
  m_fat_buffer = FatLoadFAT(m_fat_buffer);
  
  wait.Restore();
  AfxMessageBox("File(s) deleted.");
}

void CExt2::DeleteFolder(HTREEITEM hItem) {
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
  
  // don't allow to delete root
  m_dir_tree.GetFullPath(hItem, &IsDir, csName, csPath, FALSE);
  if (IsDir != -1)
    DeleteFile(hItem);
}

void CExt2::DeleteFile(HTREEITEM hItem) {
  struct S_FAT_ITEMS *items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  struct S_FAT_ROOT *root;
  CString csName, csPath;
  int IsRoot, i;
  
  if (items == NULL)
    return;
  
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  if (hParent == NULL)
    return;
  
  struct S_FAT_ITEMS *root_items = (struct S_FAT_ITEMS *) m_dir_tree.GetDataStruct(hParent);
  if (root_items == NULL)
    return;
  
  m_dir_tree.GetFullPath(hParent, &IsRoot, csName, csPath, TRUE);
  IsRoot = (IsRoot == -1);
  DWORD rootsize = (IsRoot) ? m_rootsize : 0;
  root = (struct S_FAT_ROOT *) ReadFile(root_items->Cluster, &rootsize, IsRoot);
  if (root) {
    for (i=0; i<items->EntryCount; i++)
      root[items->Index + i].name[0] |= 0x80; // All LFN entries set the first bit in the first byte
    // save the first char for undelete (the first byte in the resv[] area is not used for both FAT12,16 and FAT32
    root[items->Index + i - 1].type.resv[0] = root[items->Index + i - 1].name[0];
    //root[items->Index + i - 1].strtclst = 0;
    root[items->Index + i - 1].name[0] = FAT_DELETED_CHAR;  // last entry is the SFN
    
    // TODO: we should update the CRC as well (for undelete?)
    
    struct S_FAT_ENTRIES ClusterList;
    if (!IsRoot || (m_fat_size == FS_FAT32))
      FatFillClusterList(&ClusterList, root_items->Cluster);
    WriteFile(root, &ClusterList, rootsize, IsRoot);
    
    free(root);
    
    // ClusterList holds the used clusters.  Free them.
    if (!IsRoot || (m_fat_size == FS_FAT32))
      for (i=0; i<ClusterList.entry_count; i++)
        MarkCluster(ClusterList.entries[i], 0);
  }
  
  m_dir_tree.DeleteItem(hItem);
}
*/

void CExt2::OnExt2Apply() {
  
  
  if (m_inode_dirty)
    WriteGroupInodeTable(m_cur_group);
  WriteGroupDescTable();
}

void CExt2::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CExt2::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CExt2::OnUpdateCode() {
  AfxMessageBox("TODO");
}

void CExt2::OnErase() {
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

void CExt2::OnClean() {
  int r = AfxMessageBox("This will erase the volume, leaving the Superblock as is.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  Ext2Format(FALSE);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CExt2::OnCheck() {

  AfxMessageBox("TODO");

}

void CExt2::OnFormat() {
  int r = AfxMessageBox("This will erase the volume, using most of the current Superblock values.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  Ext2Format(r == IDYES);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// https://www.nongnu.org/ext2-doc/ext2.html
// https://github.com/skeledrew/ext4-raw-reader

bool CExt2::Ext2Format(const BOOL AskForBoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CTime time = CTime::GetCurrentTime();
  struct S_EXT2_SUPER ext_super;
  CFile bsfile;
  BYTE buffer[MAX_SECT_SIZE * 2];
  DWORD64 l;
  int i;
  
  // items we support with this format
  DWORD rev_level;         // Revision level
  DWORD feature_compat;    // compatible feature set (The fs driver is free to support them or not without risk of damaging the meta-data.)
  DWORD feature_incompat;  // incompatible feature set (The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.)
  DWORD feature_ro_compat; // readonly-compatible feature set (see notes below)
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  // if not hard format (already formatted ext2), get some flags from the current super
  if (!m_hard_format) {
    ReceiveFromDialog(&ext_super);
    rev_level = ext_super.rev_level;
    feature_compat = ext_super.feature_compat;
    feature_incompat = ext_super.feature_incompat;
    feature_ro_compat = ext_super.feature_ro_compat;
  } else {
    // else, use some default flag values (TODO: or get some from the user)
    rev_level = 1;
    feature_compat = 0;
    feature_incompat = 0;
    feature_ro_compat = 0;
  }
  
  // first, clear out the volume
  memset(buffer, 0, MAX_SECT_SIZE * 2);
  for (l=0; l<m_size; l++)
    dlg->WriteToFile(buffer, m_lba + l, 1);
  
  /*
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
    odlg.m_ofn.lpstrTitle = "Ext2 Boot Sector File";
    odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
    if (odlg.DoModal() == IDOK) {
      POSITION pos = odlg.GetStartPosition();
      if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
        DWORD filesize = bsfile.GetLength();
        if (filesize <= (2 * dlg->m_sect_size)) {
          bsfile.Read(buffer, filesize);
          dlg->WriteToFile(buffer, m_lba, 2);  // buffer will still be zeros except where we read the boot code, so okay to just write 2 sectors
        } else
          AfxMessageBox("Boot sector must be <= 2 sectors");
        bsfile.Close();
      }
    }
  }
  */
  
  struct S_EXT2_SUPER *super = (struct S_EXT2_SUPER *) buffer;
  memset(super, 0, sizeof(struct S_EXT2_SUPER));
  
  super->inodes_count = 0;      // Inodes count
  super->blocks_count = (DWORD) (m_size / 2); // Blocks count (blocks are 1024 bytes each) //// TODO: what if sector size != 512
  super->r_blocks_count = 0;    // Reserved blocks count
  super->free_blocks_count = 0; // Free blocks count
  super->free_inodes_count = 0; // Free inodes count
  super->first_data_block = 1;  // First Data Block
  super->log_block_size = 0;    // Block size (0 = 1024)
  super->log_frag_size = 0;     // Fragment size
  super->blocks_per_group = 4096;  // # Blocks per group
  super->frags_per_group = 4096;   // # Fragments per group
  super->inodes_per_group = 1024;  // # Inodes per group
  super->mtime = 0;             // Mount time
  super->wtime = (DWORD) time.GetTime(); // Write time
  super->mnt_count = 0;         // Mount count
  super->max_mnt_count = 0;     // Maximal mount count
  super->magic = 0xEF53;        // Magic signature
  super->state = 1;             // File system state
  super->errors = 1;            // Behaviour when detecting errors
  super->minor_rev_level = 0;   // minor revision level
  super->lastcheck = 1309050591; // time of last check (seconds since Jan 01 1970)
  super->checkinterval = 15552000; // max. time between checks (15552000 = roughly 7 months)
  super->creator_os = 6;        // OS (6 = ?????)
  super->rev_level = 1;         // Revision level
  super->def_resuid = 0;        // Default uid for reserved blocks
  super->def_resgid = 0;        // Default gid for reserved blocks
  
  // These fields are for EXT2_DYNAMIC_REV (rev_level == 1) superblocks only.
  if (rev_level == 1) {
    super->first_ino = 0;         // First non-reserved inode
    super->inode_size = 128;      // size of inode structure
    super->block_group_nr = 0;    // block group # of this superblock
    super->feature_compat = feature_compat;       // compatible feature set (The fs driver is free to support them or not without risk of damaging the meta-data.)
    super->feature_incompat = feature_incompat;   // incompatible feature set (The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.)
    super->feature_ro_compat = feature_ro_compat; // readonly-compatible feature set (see notes below)
    GUID_Create(&super->uuid, GUID_TYPE_RANDOM); // 128-bit uuid for volume
    memcpy(super->volume_name, "A Volume Name\0\0\0", 16);  // volume name
    memset(super->last_mounted, 0, 64);  // directory where last mounted
    super->algorithm_usage_bitmap = 0; // For compression
    // Performance hints.  Directory preallocation should only happen if the EXT2_COMPAT_PREALLOC flag is on.
    super->prealloc_blocks = 0;     // Nr of blocks to try to preallocate
    super->prealloc_dir_blocks = 0; // Nr to preallocate for dirs
    super->reserved_gdt_blocks = 0; // per group table for online growth
    /*
    // Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
    struct S_GUID journal_uuid;  // uuid of journal superblock
    super->journal_inum;         // inode number of journal file
    super->journal_dev;          // device number of journal file
    super->last_orphan;          // start of list of inodes to delete
    super->hash_seed[4];         // HTREE hash seed
    super->def_hash_version;     // Default hash version to use
    super->jnl_backup_type;
    super->desc_size;            // Group descriptor size (INCOMPAT_64BIT)
    super->default_mount_opts;
    super->first_meta_bg;        // First metablock block group
    super->mkfs_time;            // When the filesystem was created
    super->jnl_blocks[17];       // Backup of the journal inode block_array [0 -> 14], [15] = unused?, [16] = inode->size;
    // 64bit support valid if EXT4_FEATURE_COMPAT_64BIT
    super->blocks_count_hi;      // Blocks count
    super->r_blocks_count_hi;    // Reserved blocks count
    super->free_blocks_count_hi; // Free blocks count
    super->min_extra_isize;      // All inodes have at least # bytes
    super->want_extra_isize;     // New inodes should reserve # bytes
    super->flags;                // Miscellaneous flags
    super->raid_stride;          // RAID stride
    super->mmp_interval;         // # seconds to wait in MMP checking
    DWORD64 mmp_block;           // Block for multi-mount protection
    super->raid_stripe_width;    // blocks on all data disks (N*stride)
    super->log_groups_per_flex;  // FLEX_BG group size    (1 << log_groups_per_flex) ?
    super->reserved_char_pad;
    super->reserved_pad;         // Padding to next 32bits
    super->kbytes_written;       // Number of KiB wri.en to this filesystem over its lifetime
    super->snapshot_inum;        //  inode number of active snapshot
    super->snapshot_id;          //  Sequential ID of active snapshot
    super->snapshot_r_blocks_count; //  Number of blocks reserved for active snapshot's future use
    super->snapshot_list;        //  inode number of the head of the on-disk snapshot list
    super->error_count;          //  Number of errors seen
    super->first_error_time;     //  First time an error happened, in seconds since the epoch
    super->first_error_ino;      //  inode involved in first error
    super->first_error_block;    //  Number of block involved of first error
    super->first_error_func[32]; //  Name of function where the error happened
    super->first_error_line;     //  Line number where error happened
    super->last_error_time;      //  Time of most recent error, in seconds since the epoch
    super->last_error_ino;       //  inode involved in most recent error
    super->last_error_line;      //  Line number where most recent error happened
    super->last_error_block;     //  Number of block involved in most recent error
    super->last_error_func[32];  //  Name of function where the most recent error happened
    super->mount_opts[64];       //  ASCIIZ string of mount options
    super->usr_quota_inum;       //  Inode number of user quota file
    super->grp_quota_inum;       //  Inode number of group quota file
    super->overhead_blocks;      //  Overhead blocks/clusters in fs (huh?)
    super->reserved[108];        //  Padding to the end of the block
    super->checksum;             //  Superblock checksum
    */
  }
  
  // write the super
  dlg->WriteToFile(buffer, m_lba + 2, 2);
  
  // create group descriptor table
  int group_desc_size = 32; //(m_ext2_size == 4) ? 64 : 32;
  int block_size = (1024 << super->log_block_size);
  DWORD group_desc_block = (block_size == 1024) ? 2 : 1;
  int groups = (int) ((super->blocks_count + (super->blocks_per_group - 1)) / super->blocks_per_group);
  void *desc_table_buffer = calloc((groups * group_desc_size) + block_size, 1);
  int sectors_per_block = (block_size / dlg->m_sect_size);
  int sectors_per_group_desc = (int) ((((groups * group_desc_size) + (block_size - 1)) / block_size) * sectors_per_block);
  DWORD64 cur_block_num = group_desc_block + sectors_per_group_desc;
  DWORD total_inodes = (groups * super->inodes_per_group); /////// may be less if last group isn't full
  const int block_bitmap_size = (((super->blocks_per_group + 7) / 8) + (block_size - 1)) / block_size;  // in blocks
  const int inode_bitmap_size = (((super->inodes_per_group + 7) / 8) + (block_size - 1)) / block_size;  // in blocks
  const int inode_table_size = ((super->inodes_per_group * super->inode_size) + (block_size - 1)) / block_size;  // in blocks
  
  for (i=0; i<groups; i++) {
    struct S_EXT2_GROUP_DESC *desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) desc_table_buffer + (i * group_desc_size));
    desc->block_bitmap = (DWORD) (cur_block_num & 0xFFFFFFFF);
    desc->inode_bitmap = (DWORD) ((desc->block_bitmap + block_bitmap_size) & 0xFFFFFFFF);
    desc->inode_table = (DWORD) ((desc->inode_bitmap + inode_bitmap_size) & 0xFFFFFFFF);
    desc->free_blocks_count = (WORD) (super->blocks_per_group - (block_bitmap_size + inode_bitmap_size + inode_table_size));
    desc->free_inodes_count = (WORD) ((total_inodes > super->inodes_per_group) ? super->inodes_per_group : total_inodes);
    desc->used_dirs_count = (WORD) (0);
    desc->flags = 4; // 4 = ??????
    //DWORD   bg_exclude_bitmap_lo;    // Lower 32-bits of location of snapshot exclusion bitmap
    //WORD    bg_block_bitmap_csum_lo; // Lower 16-bits of the block bitmap checksum
    //WORD    bg_inode_bitmap_csum_lo; // Lower 16-bits of the inode bitmap checksum
    desc->itable_unused = (WORD) super->inodes_per_group; // Unused inodes count ????
    
    if (feature_ro_compat & EXT4_FEATURE_RO_COMPAT_METADATA_CSUM) {
      //then UUID + group number + the entire descriptor
      desc->checksum = 0;      // crc16(s_uuid+grouo_num+group_desc)   ?????
    } else {
      desc->checksum = 0;
      // else if uninit_bg, then crc16(UUID + group number + the entire descriptor).  ?????
    }
    
    // ext4 stuff starts here
    if (feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) {
      desc->block_bitmap_hi = (DWORD) (cur_block_num >> 32);
      desc->inode_bitmap_hi = (DWORD) ((DWORD64) (desc->block_bitmap + block_bitmap_size) >> 32);
      desc->inode_table_hi = (DWORD) ((DWORD64) (desc->inode_bitmap + inode_bitmap_size) >> 32);
      desc->free_blocks_count_hi = (WORD) ((super->blocks_per_group - (block_bitmap_size + inode_bitmap_size + inode_table_size)) >> 16);
      desc->free_inodes_count_hi = (WORD) (((total_inodes > super->inodes_per_group) ? super->inodes_per_group : total_inodes) >> 16);
      desc->used_dirs_count_hi = (WORD) (0 >> 16);
//  WORD    bg_itable_unused_hi;     // Upper 16-bits of unused inode count.
//  DWORD   bg_exclude_bitmap_hi;    // Upper 32-bits of location of snapshot exclusion bitmap.
//  WORD    bg_block_bitmap_csum_hi; // Upper 16-bits of the block bitmap checksum.
//  WORD    bg_inode_bitmap_csum_hi; // Upper 16-bits of the inode bitmap checksum.
    }
    total_inodes -= super->inodes_per_group;
    cur_block_num += super->blocks_per_group;
  }
  dlg->WriteToFile(desc_table_buffer, m_lba + (group_desc_block * sectors_per_block), sectors_per_group_desc);
  
  // mark the blocks used for each group's block_bitmap, inode_bitmap, and inode_table
  // cannot use MarkBitmap() because we haven't "enumerated" the partition yet
  BYTE *sect_buffer = (BYTE *) calloc(block_bitmap_size * block_size, 1);
  for (i=0; i<(block_bitmap_size + inode_bitmap_size + inode_table_size); i++)
    sect_buffer[i/8] |= (0x80 >> (i % 8));
  for (i=0; i<groups; i++) {
    struct S_EXT2_GROUP_DESC *desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) desc_table_buffer + (i * group_desc_size));
    dlg->WriteToFile(sect_buffer, m_lba + ((((DWORD64) desc->block_bitmap_hi << 32) | (DWORD64) desc->block_bitmap) * sectors_per_block), block_bitmap_size * sectors_per_block);
  }
  free(sect_buffer);
  free(desc_table_buffer);
  
  if (!m_hard_format)
    Start(m_lba, m_size, m_color, m_index, FALSE);

  return TRUE;
}

void CExt2::DisplayFreeSpace(void) {
  CString csFree;
  
  //csFree.Format("Free Space: %s (bytes)", (LPCSTR) gFormatNum((size_t) m_free_blocks * block_size, FALSE, FALSE));
  csFree = "Not Implemented Yet";

  SetDlgItemText(IDC_FREE_SIZE_STR, csFree);
}

size_t CExt2::CalcFreeBlocks(void) {

  return 0;
}

// group  = group number
// bitmap = EXT2_BLOCK_BITMAP for block bitmap
//        = EXT2_INODE_BITMAP for inode bitmap
// start  = starting bit
// length = count of bits to set/clear
// mark   = TRUE to set, FALSE to clear
void CExt2::MarkBitmap(const int group, const int bitmap, const int start, const int length, const BOOL mark) {
  
}

void CExt2::OnMountTime() {
  CLeanTime dlg;
  
  dlg.m_title = "Mount Time";
  GetDlgItemText(IDC_MOUNT_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_MOUNT_TIME, dlg.m_lean_time);
}

void CExt2::OnWriteTime() {
  CLeanTime dlg;
  
  dlg.m_title = "Write Time";
  GetDlgItemText(IDC_WRITE_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_WRITE_TIME, dlg.m_lean_time);
}

void CExt2::OnLastCheck() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Checked";
  GetDlgItemText(IDC_LAST_CHECK, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_LAST_CHECK, dlg.m_lean_time);
}

void CExt2::OnLastCheckInt() {
  CLeanTime dlg;
  
  dlg.m_title = "Check Interval";
  GetDlgItemText(IDC_LAST_CHECK_INT	, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_LAST_CHECK_INT	, dlg.m_lean_time);
}

struct S_ATTRIBUTES ext2_creator_os[] = {
                                           //            |                               | <- max (col 67)
  { 0,                       0xFFFFFFFF,              0, "Linux"                          , {-1, } },
  { 1,                       0xFFFFFFFF,              1, "GNU HURD"                       , {-1, } },
  { 2,                       0xFFFFFFFF,              2, "MASIX"                          , {-1, } },
  { 3,                       0xFFFFFFFF,              3, "FreeBSD"                        , {-1, } },
  { 4,                       0xFFFFFFFF,              4, "Lites"                          , {-1, } },
  { 5,                       0xFFFFFFFF,              5, "Unknown"                        , {-1, } },
  { 6,                       0xFFFFFFFF,              6, "Unknown"                        , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2::OnCreatorOs() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_CREATOR_OS, cs);
  dlg.m_title = "Creator OS";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_creator_os;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_CREATOR_OS, cs);
  }
}

struct S_ATTRIBUTES ext2_current_state[] = {
                                           //            |                               | <- max (col 67)
  { 1,                       0xFFFFFFFF,              0, "Unmounted Cleanly"              , {-1, } },
  { 2,                       0xFFFFFFFF,              1, "Errors Detected"                , {-1, } },
  { 4,                       0xFFFFFFFF,              2, "Orphans being recovered ?"      , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2::OnState() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_STATE, cs);
  dlg.m_title = "Current State";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_current_state;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_STATE, cs);
  }
}

struct S_ATTRIBUTES ext2_error_b[] = {
                                           //            |                               | <- max (col 67)
  { 1,                       0xFFFFFFFF,              0, "Continue as nothing happened"   , {-1, } },
  { 2,                       0xFFFFFFFF,              1, "Remount as Read-Only"           , {-1, } },
  { 3,                       0xFFFFFFFF,              1, "Cause a kernel panic"           , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2::OnErrorB() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_ERROR_B, cs);
  dlg.m_title = "Error Behaviour";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_error_b;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_ERROR_B, cs);
  }
}

void CExt2::OnMagic() {
  SetDlgItemText(IDC_MAGIC, "0xEF53");
}
