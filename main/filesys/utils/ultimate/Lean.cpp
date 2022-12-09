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

// Lean.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#include "Lean.h"
#include "LeanEntry.h"
#include "LeanFormat.h"
#include "LeanJournal.h"
#include "LeanSymbolic.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLean property page

IMPLEMENT_DYNCREATE(CLean, CPropertyPage)

CLean::CLean() : CPropertyPage(CLean::IDD) {
  //{{AFX_DATA_INIT(CLean)
  m_backup_lba = _T("");
  m_bad_lba = _T("");
  m_bitmap_lba = _T("");
  m_bitmap_crc = _T("");
  m_crc = _T("");
  m_cur_state = _T("");
  m_free_blocks = _T("");
  m_next_free = _T("");
  m_guid = _T("");
  m_label = _T("");
  m_magic = _T("");
  m_pre_alloc = _T("");
  m_primary_lba = _T("");
  m_root_lba = _T("");
  m_blocks_band = _T("");
  m_block_count = _T("");
  m_version = _T("");
  m_journal_lba = _T("");
  m_capabilities = _T("");
  m_log_block_size = _T("");
  m_show_hidden = FALSE;
  m_show_del = FALSE;
  m_del_clear = FALSE;
  m_EAs_in_Inode = FALSE;
  //}}AFX_DATA_INIT
  m_hard_format = FALSE;
  m_free_block_cnt = 0;
  m_use_extended_extents = FALSE;
}

CLean::~CLean(){
  m_free_block_cnt = 0;
}

void CLean::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLean)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_LEAN_BACKUP_LBA, m_backup_lba);
  DDV_MaxChars(pDX, m_backup_lba, 32);
  DDX_Text(pDX, IDC_LEAN_BAD_LBA, m_bad_lba);
  DDV_MaxChars(pDX, m_bad_lba, 32);
  DDX_Text(pDX, IDC_LEAN_BITMAP_LBA, m_bitmap_lba);
  DDV_MaxChars(pDX, m_bitmap_lba, 32);
  DDX_Text(pDX, IDC_LEAN_BITMAP_CRC, m_bitmap_crc);
  DDV_MaxChars(pDX, m_bitmap_crc, 32);
  DDX_Text(pDX, IDC_LEAN_CRC, m_crc);
  DDV_MaxChars(pDX, m_crc, 16);
  DDX_Text(pDX, IDC_LEAN_CUR_STATE, m_cur_state);
  DDV_MaxChars(pDX, m_cur_state, 16);
  DDX_Text(pDX, IDC_LEAN_FREE_BLOCKS, m_free_blocks);
  DDV_MaxChars(pDX, m_free_blocks, 32);
  DDX_Text(pDX, IDC_LEAN_NEXT_FREE, m_next_free);
  DDV_MaxChars(pDX, m_next_free, 32);
  DDX_Text(pDX, IDC_LEAN_GUID, m_guid);
  DDV_MaxChars(pDX, m_guid, 64);
  DDX_Text(pDX, IDC_LEAN_LABEL, m_label);
  DDV_MaxChars(pDX, m_label, 64);
  DDX_Text(pDX, IDC_LEAN_MAGIC, m_magic);
  DDV_MaxChars(pDX, m_magic, 16);
  DDX_Text(pDX, IDC_LEAN_PRE_ALLOC, m_pre_alloc);
  DDV_MaxChars(pDX, m_pre_alloc, 8);
  DDX_Text(pDX, IDC_LEAN_PRIMARY_LBA, m_primary_lba);
  DDV_MaxChars(pDX, m_primary_lba, 32);
  DDX_Text(pDX, IDC_LEAN_ROOT_LBA, m_root_lba);
  DDV_MaxChars(pDX, m_root_lba, 32);
  DDX_Text(pDX, IDC_LEAN_BLOCKS_BAND, m_blocks_band);
  DDV_MaxChars(pDX, m_blocks_band, 8);
  DDX_Text(pDX, IDC_LEAN_BLOCK_SIZE, m_log_block_size);
  DDV_MaxChars(pDX, m_log_block_size, 32);
  DDX_Text(pDX, IDC_LEAN_BLOCK_COUNT, m_block_count);
  DDV_MaxChars(pDX, m_block_count, 32);
  DDX_Text(pDX, IDC_LEAN_VERSION, m_version);
  DDV_MaxChars(pDX, m_version, 16);
  DDX_Text(pDX, IDC_JOURNAL_LBA, m_journal_lba);
  DDX_Text(pDX, IDC_LEAN_CAPABILITIES, m_capabilities);
  DDX_Check(pDX, IDC_SHOW_HIDDEN, m_show_hidden);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  DDX_Check(pDX, IDC_DEL_CLEAR, m_del_clear);
  DDX_Check(pDX, IDC_EAS_IN_INODE, m_EAs_in_Inode);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLean, CPropertyPage)
  //{{AFX_MSG_MAP(CLean)
  ON_WM_HELPINFO()
  ON_BN_CLICKED(ID_APPLY, OnLeanApply)
  ON_BN_CLICKED(ID_CLEAN, OnLeanClean)
  ON_BN_CLICKED(ID_FORMAT, OnLeanFormat)
  ON_BN_CLICKED(ID_CHECK, OnLeanCheck)
  ON_EN_CHANGE(IDC_JOURNAL_LBA, OnChangeLeanJournal)
  ON_EN_CHANGE(IDC_LEAN_VERSION, OnChangeLeanVersion)
  ON_EN_CHANGE(IDC_LEAN_PRE_ALLOC, OnChangeLeanPreAlloc)
  ON_EN_CHANGE(IDC_LEAN_BLOCKS_BAND, OnChangeLeanBlockBand)
  ON_EN_CHANGE(IDC_LEAN_BLOCK_SIZE, OnChangeLeanBlockSize)
  ON_BN_CLICKED(ID_COPY, OnLeanCopy)
  ON_BN_CLICKED(ID_VIEW, OnLeanView)
  ON_BN_CLICKED(ID_INSERT, OnLeanInsert)
  ON_BN_CLICKED(ID_INSERT_SYMBOLIC, OnLeanInsertSymbolic)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_ENTRY, OnLeanEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(IDC_LEAN_CRC_UPDATE, OnLeanCrcUpdate)
  ON_BN_CLICKED(IDC_LEAN_MAGIC_UPDATE, OnLeanMagicUpdate)
  ON_BN_CLICKED(IDC_LEAN_CURRENT_STATE, OnLeanCurrentState)
  ON_BN_CLICKED(IDC_FREE_BLOCKS_UPDATE, OnLeanFreeUpdate)
  ON_BN_CLICKED(IDC_BITMAP_CRC_UPDATE, OnLeanBitmapCRC)
  ON_EN_KILLFOCUS(IDC_LEAN_GUID, OnKillfocusLeanGuid)
  ON_BN_CLICKED(IDC_GUID_CREATE, OnGuidCreate)
  ON_BN_CLICKED(ID_DELETE, OnLeanDelete)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_VIEW_JOURNAL, OnViewJournal)
  ON_BN_CLICKED(ID_JOURNAL_INODE, OnJournalInode)
  ON_BN_CLICKED(IDC_SHOW_HIDDEN, OnShowHidden)
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_BN_CLICKED(IDC_EAS_IN_INODE, OnEAsInInode)
  ON_BN_CLICKED(ID_SAVE_SUPER, OnSaveSuper)
  ON_BN_CLICKED(ID_RESTORE_SUPER, OnRestoreSuper)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLean message handlers
BOOL CLean::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    case IDC_LEAN_CRC_UPDATE:
      strTipText = "Recalculate CRC";
      break;
    case IDC_LEAN_MAGIC_UPDATE:
      strTipText = "Set to Magic: '0x4E41454C'";
      break;
    case IDC_LEAN_CURRENT_STATE:
      strTipText = "Modify Current State Flags";
      break;
    case IDC_FREE_BLOCKS_UPDATE:
      strTipText = "Update Free Blocks to calculated free count";
      break;
    case IDC_BITMAP_CRC_UPDATE:
      strTipText = "Update the Checksum of the bitmap";
      break;
    case ID_JOURNAL_INODE:
      strTipText = "View Journal Inode";
      break;
    case ID_VIEW_JOURNAL:
      strTipText = "View Journal contents";
      break;

    case IDC_GUID_CREATE:
      strTipText = "Create a new GUID";
      break;
    case IDC_EAS_IN_INODE:
      strTipText = "When clear, the file's contents start in Inode block";
      break;

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
    case IDC_SHOW_HIDDEN:
      strTipText = "Show Hidden Entries";
      break;
    case IDC_DEL_CLEAR:
      strTipText = "Zero File Contents on Delete";
      break;

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

BOOL CLean::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_show_hidden = AfxGetApp()->GetProfileInt("Settings", "LEANShowHidden", FALSE);
  m_show_del = AfxGetApp()->GetProfileInt("Settings", "LEANShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "LEANDelClear", FALSE);
  m_EAs_in_Inode = AfxGetApp()->GetProfileInt("Settings", "LEANEAsInInode", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  EnableToolTips(TRUE);

  return TRUE;
}

BOOL CLean::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "lean.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CLean::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow((hItem != NULL) && (m_dir_tree.GetParentItem(hItem) != NULL));
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_VIEW)->EnableWindow((hItem != NULL) && (items->Flags & (ITEM_IS_FILE | ITEM_IS_SYMBOLIC)));
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(hItem != NULL);
  
  *pResult = 0;
}

void CLean::OnLeanApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_block_size);
  DWORD crc;
  int ret;
  
  ReceiveFromDialog(&m_super); // bring from Dialog
  
  memset(buffer, 0, m_block_size);
  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));
  
  crc = LeanCalcCRC(buffer, m_block_size, TRUE);
  if (m_super.checksum != crc) {
    ret = AfxMessageBox("CRC is not correct!  Update before Write?", MB_YESNOCANCEL, NULL);
    if (ret == IDCANCEL)
      return;
    if (ret == IDYES) {
      m_super.checksum = crc;
      m_crc.Format("0x%08X", m_super.checksum);
      SetDlgItemText(IDC_LEAN_CRC, m_crc);
    }
  }
  
  // we updated the super, so copy it back to the buffer
  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));

  // write the buffer to the file
  dlg->WriteBlocks(buffer, m_lba, m_super_block_loc, m_block_size, 1);
}

void CLean::OnLeanClean() {
  //int r = AfxMessageBox("This will erase the volume, leaving the SuperBlock as is.\r\n"
  //                      "Is this what you wish to do?", MB_YESNO, NULL);
  int r = AfxMessageBox("This will erase the volume, doing a format on the volume.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  Format(FALSE);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

/* Calculate how many blocks each band will use
 *   returns the log value.  i.e.: returns (x) of  2^x 
 */
BYTE CLean::lean_calc_log_band_size(const DWORD block_size, const DWORD64 tot_blocks) {
  BYTE ret = 12;
  int i;

  for (i=63; i>16; i--) {
    if (tot_blocks & ((DWORD64) 1 << i)) {
      ret = (BYTE) (i - 4);
      break;
    }
  }

  // A band must be large enough to occupy all bits in a bitmap block.
  //  therefore, 512-byte blocks must return at least a log2 of 12.
  //             1024-byte blocks must return at least a log2 of 13.
  //             2048-byte blocks must return at least a log2 of 14.
  //             4096-byte blocks must return at least a log2 of 15, etc
  if (ret < (LOG2(block_size) + 3))
    return (LOG2(block_size) + 3);
  
  return ret;
}

void CLean::OnLeanFormat() {
  int r = AfxMessageBox("This will erase the volume, using most of the current Super Block values.\r\n"
                        "Do you wish to specify a boot sector file?\r\n", MB_YESNOCANCEL, NULL);
  if (r == IDCANCEL)
    return;
  
  Format(r == IDYES);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// returns TRUE if a successful format
bool CLean::Format(const BOOL AskForBoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CFile bsfile;
  CString cs;
  unsigned u;
  size_t reserved;
  DWORD64 lba;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  // restore our SuperBlock info and get the specs to format the volume with
  CLeanFormat format;
  format.m_journal = FALSE;
  format.m_pre_alloc_count = (dlg->m_sect_size == 4096) ? (4-1) : (8-1); // 4096-byte blocks use 4 prealloc's, else 8 prealloc's
  format.m_block_size = dlg->m_sect_size;
  if (!m_hard_format) {
    //format.m_journal = TRUE;
    format.m_pre_alloc_count = m_super.pre_alloc_count;
  }
  if (format.DoModal() != IDOK)
    return FALSE;
  
  m_use_extended_extents = format.m_extended_extents;
  m_block_size = format.m_block_size;
  m_tot_blocks = (m_size * dlg->m_sect_size) / m_block_size;
  
  // Up to the first 33 blocks are reserved for the boot code and the super block.
  // The super block can be located in blocks 1 to 32 (zero based)
  // We place the superblock just after the boot code, in this case block 31
  // (we leave a max of 16k for boot code)
  if (m_block_size < 16384) {
    m_super_block_loc = (16384 / m_block_size);
    if (m_super_block_loc > 32)
      m_super_block_loc = 32;
  } else
    m_super_block_loc = 1;
  BYTE *buffer = (BYTE *) calloc(m_block_size * ((size_t) m_super_block_loc + 1), 1);

  if (!m_hard_format)
    ReceiveFromDialog(&m_super); // bring from Dialog

  // first, clear out the volume
  for (lba=0; lba<m_size; lba++)
    dlg->WriteToFile(buffer, m_lba + lba, 1);
  
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
    odlg.m_ofn.lpstrTitle = "Lean Boot Sector File";
    odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
    if (odlg.DoModal() == IDOK) {
      POSITION pos = odlg.GetStartPosition();
      if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
        size_t filesize = (size_t) bsfile.GetLength();
        if (filesize <= 16384) {
          reserved = (filesize + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
          bsfile.Read(buffer, (UINT) filesize);
          dlg->WriteToFile(buffer, m_lba, (UINT) reserved);
        } else
          AfxMessageBox("Boot sector must be <= 16384 bytes");
        bsfile.Close();
      }
    }
  } // TODO: else write a generic boot....
  
  // Calculate the bitmap size for a single band, which will also calculate the band size.
  // A band can be 2^12 to 2^31 blocks.
  // A band must be large enough to occupy all bits in a bitmap block.
  //  therefore, 512-byte blocks must return at least a log2 of 12.
  //             1024-byte blocks must return at least a log2 of 13.
  //             2048-byte blocks must return at least a log2 of 14.
  //             4096-byte blocks must return at least a log2 of 15, etc
  const BYTE log_band_size = lean_calc_log_band_size(m_block_size, m_tot_blocks);
  const DWORD band_size = (1 << log_band_size);
  unsigned bitmap_size = (band_size / m_block_size / 8);
  const unsigned tot_bands = (unsigned) (m_tot_blocks + (band_size - 1)) / band_size;
  
  // now create a super block
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) (buffer + (m_super_block_loc * m_block_size));
  memset(super, 0, m_block_size);
  super->magic = LEAN_SUPER_MAGIC;
  super->fs_version = 0x0100;  // 1.0
  super->log_blocks_per_band = log_band_size;
  super->pre_alloc_count = format.m_pre_alloc_count;
  super->state = (0<<1) | (1<<0);  // clean unmount
  GUID_Create(&super->guid, GUID_TYPE_RANDOM);
  PutName(super->volume_label, "A label goes here.", 64);
  super->block_count = m_tot_blocks;
  super->free_block_count = (m_tot_blocks - m_super_block_loc - 1 - (tot_bands * bitmap_size) - (super->pre_alloc_count + 1) - 1) - ((format.m_journal) ? 3 : 0);
  super->next_free_block = 0;
  super->primary_super = m_super_block_loc;
  super->backup_super = ((band_size - 1) < m_tot_blocks) ? (band_size - 1) : (m_tot_blocks - 1);   // last block in first band
  super->bitmap_start = m_super_block_loc + 1;
  super->resv0 = 0;
  super->root_start = super->bitmap_start + bitmap_size;
  super->bad_start = 0;  // no bad blocks (yet?)
  super->journal = (format.m_journal) ? super->backup_super - JOURNAL_SIZE - 1: 0;   // -1 for the inode block
  super->capabilities = (format.m_extended_extents) ? LEAN_CAPABILITIES_EXT_EXTENTS : 0;
  super->log_block_size = LOG2(m_block_size);
  super->resv1[0] = super->resv1[1] = super->resv1[2] = 0;
  
  // create a buffer for the bitmap(s), and mark the first few bits as used.
  BYTE *bitmap = (BYTE *) calloc(bitmap_size * m_block_size, 1);
  for (u=0; u<(((super->root_start + (super->pre_alloc_count + 1)) - 1) / 8); u++)
    bitmap[u] = 0xFF;
  bitmap[u] = (0xFF >> (8 - ((super->root_start + (super->pre_alloc_count + 1)) % 8)));
  bitmap[((band_size - 1) / 8)] = (BYTE) ((DWORD) 0xFF << (8 - (1 + ((format.m_journal) ? (1 + JOURNAL_SIZE) : 0))));  // mark one for the backup, 1 for the inode, and JOURNAL_SIZE bits for the Journal

  // create a root directory
  // (for an empty directory, we need at most two blocks.  However, we write (super->pre_alloc_count + 1) blocks to the disk)
  u = ((super->pre_alloc_count + 1) > 2) ? (super->pre_alloc_count + 1) : 2;
  struct S_LEAN_INODE *root = (struct S_LEAN_INODE *) calloc(u * m_block_size, 1);
  root->magic = LEAN_INODE_MAGIC;
  root->extent_count = 1;
  //memset(root->reserved, 0, 3);
  root->links_count = 2;  // the "." and ".." entries
  root->uid = 0;
  root->gid = 0;
  root->attributes = LEAN_ATTR_IXUSR | LEAN_ATTR_IRUSR | LEAN_ATTR_IWUSR | LEAN_ATTR_ARCHIVE | LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC;
  if (format.m_eas_after_inode)
    root->attributes |= LEAN_ATTR_EAS_IN_INODE;
  root->file_size = 0;  // currently zero
  root->block_count = (super->pre_alloc_count + 1);
  CTime now = CTime::GetCurrentTime();
  root->acc_time = 
    root->cre_time =
    root->sch_time =
    root->mod_time = ((INT64) now.GetTime()) * 1000000;  // uS from 1 Jan 1970
  root->first_indirect = 0;
  root->last_indirect = 0;
  root->fork = 0;

  // create the one and only extent
  unsigned max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  DWORD64 *p_extent_start = (DWORD64 *) &root->extent_start[0];
  DWORD *p_extent_size = (DWORD *) ((BYTE *) &root->extent_start[0] + (max_extents * sizeof(DWORD64)));
  DWORD *p_extent_crc = (DWORD *) ((BYTE *) &root->extent_start[0] + (max_extents * (sizeof(DWORD64) + sizeof(DWORD))));
  p_extent_start[0] = super->root_start;
  p_extent_size[0] = u;
  if (m_use_extended_extents)
    p_extent_crc[0] = LeanCalcCRC(root, u * m_block_size, TRUE);
  
  // the directory entry's
  // (for now, just the dot and dotdot entries)
  struct S_LEAN_DIRENTRY *entry;
  if (format.m_eas_after_inode) {
    * (DWORD *) ((BYTE *) root + LEAN_INODE_SIZE) = (m_block_size - LEAN_INODE_SIZE - sizeof(DWORD));  // padding extended attribute
    entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + m_block_size);
  } else
    entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + LEAN_INODE_SIZE);

  // The "." and ".." entries
  root->file_size = CreateRootEntry(entry, super->root_start, LEAN_FA_HIDDEN | LEAN_FT_DIR, ".");
  entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) entry + root->file_size);  // move to next entry
  root->file_size += CreateRootEntry(entry, super->root_start, LEAN_FA_HIDDEN | LEAN_FT_DIR, "..");

  // create an empty Journal?
  if (format.m_journal) {
    // Make an INODE, then write inode and journal to the image
    unsigned Size = m_block_size * JOURNAL_SIZE;
    
    struct S_LEAN_BLOCKS extents;
    AllocateExtentBuffer(&extents, 1);
    extents.extent_count = 1;
    extents.extent_size[0] = 1 + JOURNAL_SIZE;
    extents.extent_start[0] = m_super.journal;
    BuildInode(&extents, Size, LEAN_ATTR_SYSTEM | LEAN_ATTR_IMMUTABLE_FL | LEAN_ATTR_EAS_IN_INODE);
    
    BYTE *journal = (BYTE *) calloc(Size, 1);
    struct S_LEAN_JOURNAL *hdr = (struct S_LEAN_JOURNAL *) journal;
    struct S_LEAN_JOURNAL_ENTRY *entry = (struct S_LEAN_JOURNAL_ENTRY *) ((BYTE *) hdr + sizeof(struct S_LEAN_JOURNAL));
    hdr->magic = JOURNAL_MAGIC;
    hdr->entry_cnt = (Size - sizeof(struct S_LEAN_JOURNAL)) / sizeof(S_LEAN_JOURNAL_ENTRY);
    for (u=0; u<hdr->entry_cnt; u++)
      entry[u].flags = JOURNAL_ENTRY_INVALID;
    hdr->checksum = LeanCalcCRC(hdr, sizeof(struct S_LEAN_JOURNAL) + (hdr->entry_cnt * sizeof(S_LEAN_JOURNAL_ENTRY)), TRUE);
    WriteFile(journal, &extents, Size);
    FreeExtentBuffer(&extents);
    free(journal);
  }
  
  // now write the first band to the disk
  dlg->WriteBlocks(buffer, m_lba, 0, m_block_size, (long) m_super_block_loc); // write everything up to the superblock
  dlg->WriteBlocks(bitmap, m_lba, super->bitmap_start, m_block_size, bitmap_size);
  dlg->WriteBlocks(root, m_lba, super->root_start, m_block_size, (super->pre_alloc_count + 1));

  // now we can do the crc of the root
  UpdateFileExtents(root, FALSE);
  root->checksum = LeanCalcCRC(root, LEAN_INODE_SIZE, TRUE);
  dlg->WriteBlocks(root, m_lba, super->root_start, m_block_size, 1);

  // now create and write each remaining band (just the bitmap)
  if (tot_bands > 1) {
    memset(bitmap, 0, bitmap_size * m_block_size);
    // mark the bitmap block(s) as used
    for (u=0; u<(bitmap_size / 8); u++)
      bitmap[u] = 0xFF;
    bitmap[u] = (0xFF >> (8 - (bitmap_size % 8)));
  }
  
  lba = 0;
  for (u=1; u<tot_bands; u++) {
    lba += band_size;
    // don't write past end of disk (volume)
    if (((band_size * u) + bitmap_size) > m_tot_blocks)
      bitmap_size = (unsigned) (m_tot_blocks - (band_size * u));
    // write the bitmap to current band location
    dlg->WriteBlocks(bitmap, m_lba, lba, m_block_size, bitmap_size);
  }

  // before we leave, set/clear EAs_in_Inode per format.m_eas_after_inode
  m_EAs_in_Inode = format.m_eas_after_inode;
  CheckDlgButton(IDC_EAS_IN_INODE, format.m_eas_after_inode ? BST_CHECKED : BST_UNCHECKED);

  // update the bitmap's checksum and write the superblock to the disk
  super->bitmap_checksum = 0;
  super->checksum = LeanCalcCRC(super, m_block_size, TRUE);
  dlg->WriteBlocks(super, m_lba, super->primary_super, m_block_size, 1);
  dlg->WriteBlocks(super, m_lba, super->backup_super, m_block_size, 1);
  
  // done, cleanup and return
  free(root);
  free(bitmap);
  free(buffer);

  return TRUE;
}

// create a directory entry at 'entry'
// return byte size of entry (bytes used)
DWORD CLean::CreateRootEntry(struct S_LEAN_DIRENTRY *entry, const DWORD64 inode_num, const BYTE type, CString name) {
  WORD length = 0;

  entry->inode = inode_num;
  entry->type = type;
  entry->name_len = name.GetLength(); // count of chars used
  length = PutName(entry->name, name, LEAN_NAME_LEN_MAX);
  entry->rec_len = (((LEAN_DIRENTRY_NAME + length) + 15) / 16);

  // return the count of bytes used
  return entry->rec_len * 16;
}

void CLean::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  CString cs;
  
  BYTE *existing = (BYTE *) calloc((size_t) m_super_block_loc * m_block_size, 1);
  
  // first, read in what we already have (at least the first block)
  dlg->ReadBlocks(existing, m_lba, 0, m_block_size, 1);
  
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
  odlg.m_ofn.lpstrTitle = "Lean Boot Sector File";
  odlg.m_ofn.lpstrInitialDir = AfxGetApp()->GetProfileString("Settings", "DefaultMBRPath", NULL);
  if (odlg.DoModal() == IDOK) {
    POSITION pos = odlg.GetStartPosition();
    if (bsfile.Open(odlg.GetNextPathName(pos), CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) != 0) {
      size_t filesize = (size_t) bsfile.GetLength();
      if (filesize > (m_super_block_loc * m_block_size)) {
        cs.Format("Boot sector must be <= %i bytes", (m_super_block_loc * m_block_size));
        AfxMessageBox(cs);
        filesize = ((size_t) m_super_block_loc * m_block_size);
      }
      bsfile.Read(existing, (UINT) filesize);
      bsfile.Close();
    }
    
    // restore the FYSOS sig?
    if (AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE))
      memcpy(existing + S_FYSOSSIG_OFFSET, &s_sig, sizeof(struct S_FYSOSSIG));
    
    // write it
    dlg->WriteBlocks(existing, m_lba, 0, m_block_size, (long) m_super_block_loc);

    AfxMessageBox("Update Successful...");
  }
  
  free(existing);
}

void CLean::OnLeanCrcUpdate() {
  BYTE *buffer = (BYTE *) calloc(m_block_size, 1);

  ReceiveFromDialog(&m_super);

  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));
  
  m_super.checksum = LeanCalcCRC(buffer, m_block_size, TRUE);
  m_crc.Format("0x%08X", m_super.checksum);
  SetDlgItemText(IDC_LEAN_CRC, m_crc);

  free(buffer);
}

void CLean::OnLeanMagicUpdate() {
  SetDlgItemText(IDC_LEAN_MAGIC, "0x4E41454C");
}

struct S_ATTRIBUTES lean_cur_state[] = {
                                           //            |                               | <- max (col 67)
  { (1<<0),                  (1<<0),                  0, "Unmounted"                      , {-1, } },
  { (1<<1),                  (1<<1),                  1, "Error"                          , {-1, } },
  { 0,                   (DWORD) -1,                 -1, NULL                             , {-1, } }
};

void CLean::OnLeanCurrentState() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_LEAN_CUR_STATE, cs);
  dlg.m_title = "Current State";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = lean_cur_state;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%08X", dlg.m_attrib);
    SetDlgItemText(IDC_LEAN_CUR_STATE, cs);
  }
}

void CLean::OnLeanFreeUpdate() {
  m_super.free_block_count = m_free_block_cnt;
  m_free_blocks.Format("%I64i", m_super.free_block_count);
  SetDlgItemText(IDC_LEAN_FREE_BLOCKS, m_free_blocks);
}

void CLean::OnLeanBitmapCRC() {
  DWORD crc = BitmapChecksum(&m_super);
  m_bitmap_crc.Format("0x%08X", crc);
  SetDlgItemText(IDC_LEAN_BITMAP_CRC, m_bitmap_crc);
}

void CLean::OnChangeLeanJournal() {
  CString cs;
  DWORD64 inode;
  
  GetDlgItemText(IDC_JOURNAL_LBA, cs);
  inode = convert64(cs);
  
  GetDlgItem(ID_JOURNAL_INODE)->EnableWindow(inode > 0);
  GetDlgItem(ID_VIEW_JOURNAL)->EnableWindow(inode > 0);
}

void CLean::OnChangeLeanVersion() {
  CString cs;
  WORD version;
  
  GetDlgItemText(IDC_LEAN_VERSION, cs);
  version = convert16(cs);
  cs.Format("%i.%i", version >> 8, version & 0xFF);
  SetDlgItemText(IDC_LEAN_VERSION_DISP, cs);
}

void CLean::OnChangeLeanPreAlloc() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_PRE_ALLOC, cs);
  byte = convert8(cs);
  
  cs.Format("%i", byte + 1);
  SetDlgItemText(IDC_LEAN_PRE_ALLOC_DISP, cs);
}

void CLean::OnChangeLeanBlockBand() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_BLOCKS_BAND, cs);
  byte = convert8(cs);
  
  cs.Format("%i", 1 << byte);
  SetDlgItemText(IDC_LEAN_BLOCKS_BAND_DISP, cs);
}

void CLean::OnChangeLeanBlockSize() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_BLOCK_SIZE, cs);
  byte = convert8(cs);
  
  if (byte > 0)
    cs.Format("%i", 1 << byte);
  else
    cs = "?";
  SetDlgItemText(IDC_LEAN_BLOCK_SIZE_DISP, cs);
}

// Lean colors will have a red shade to them.
DWORD CLean::GetNewColor(int index) {
  int r = ((249 - (index * 2)) > -1) ? (249 - (index * 2)) : 0;
  int g = ((126 - (index * 18)) > -1) ? (126 - (index * 18)) : 0;
  int b = ((96 - (index * 20)) > -1) ? (96 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CLean::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;

  m_hard_format = FALSE;
  
  if (!DetectLeanFS()) {
    AfxMessageBox("Did not find a valid LeanFS volume");
    m_isvalid = FALSE;
  }

  // check the bitmap checksum
  if (m_super.bitmap_checksum != BitmapChecksum(&m_super))
    if (AfxMessageBox("Lean: Super Bitmap Checksum does not match actual. Ignore?", MB_YESNO, 0) != IDYES)
      m_isvalid = FALSE;
  
  m_psp.dwFlags |= PSP_USETITLE;
  dlg->m_LeanNames[index] = "LeanFS";
  m_psp.pszTitle = dlg->m_LeanNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->Lean[index].m_draw_index, (char *) (LPCTSTR) dlg->m_LeanNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog(&m_super);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_VIEW)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  if (m_isvalid) {
    m_free_block_cnt = CalcFreeBlocks();

    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    struct S_LEAN_DIRENTRY *root;
    DWORD64 root_size = 0;
    root = (struct S_LEAN_DIRENTRY *) ReadFile(m_super.root_start, &root_size);
    if (root) {
      CWaitCursor wait; // display a wait cursor
      SaveItemInfo(m_hRoot, m_super.root_start, root_size, ITEM_IS_FOLDER, 0, FALSE);
      m_too_many = FALSE;
      ParseDir(root, root_size, m_hRoot);
      m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
      free(root);
      wait.Restore();
      GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
      GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
      GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
      
      // select the root and set the focus to that root
      GetDlgItem(IDC_DIR_TREE)->SetFocus();
      m_dir_tree.SelectSetFirstVisible(m_hRoot);
    } else 
      AfxMessageBox("Found illegal Root Inode.");
    
    // display the running total of free bytes left
    DisplayFreeSpace();
  }
  Invalidate(TRUE);  // redraw the tab
}

void CLean::ParseDir(struct S_LEAN_DIRENTRY *root, DWORD64 root_size, HTREEITEM parent) {
  struct S_LEAN_DIRENTRY *cur = root, *sub;
  HTREEITEM hItem;
  DWORD64 filesize;
  CString name;
  BOOL IsDot;

  while ((((unsigned char *) cur < ((unsigned char *) root + root_size))) && !m_too_many) {
    if (cur->rec_len == 0)
      break;
    
    // retrieve the name.
    GetName(cur->name, name, cur->name_len);

    IsDot = ((name == ".") || (name == ".."));
    
    switch (cur->type & LEAN_FT_FMT) {
      case LEAN_FT_MT:  // File type: Empty
        break;
      case LEAN_FT_REG: // File type: regular file
        if (cur->type & LEAN_FA_HIDDEN) {
          if (!m_show_hidden)
            break;
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE_HIDDEN, IMAGE_FILE_HIDDEN, parent);
        } else
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, cur->inode, 0, ITEM_IS_FILE, (DWORD) ((BYTE *) cur - (BYTE *) root), TRUE);
        break;
      case LEAN_FT_DIR:  // File type: directory
        if (cur->type & LEAN_FA_HIDDEN) {
          if (!m_show_hidden)
            break;
          hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER_HIDDEN, IMAGE_FOLDER_HIDDEN, parent);
        } else
          hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        sub = (struct S_LEAN_DIRENTRY *) ReadFile(cur->inode, &filesize);
        SaveItemInfo(hItem, cur->inode, filesize, ITEM_IS_FOLDER, (DWORD) ((BYTE *) cur - (BYTE *) root), !IsDot);
        if (!IsDot && sub) {
          ParseDir(sub, filesize, hItem);
          free(sub);
        }
        break;
      case LEAN_FT_LNK: // File type: symbolic link
        name += " (Link)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_SYMBOLIC, IMAGE_SYMBOLIC, IMAGE_SYMBOLIC, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, cur->inode, 0, ITEM_IS_SYMBOLIC, (DWORD) ((BYTE *) cur - (BYTE *) root), FALSE);
        break;
      // should not encounter a fork.  A fork should not be referenced in a directory
      //case LEAN_FT_FRK: // File type: fork
      //  break;
      case LEAN_FT_DELETED:
        if (m_show_del) {
          name += " (Deleted)";
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
          if (hItem == NULL) { m_too_many = TRUE; return; }
          SaveItemInfo(hItem, cur->inode, 0, ITEM_IS_FILE, (DWORD) ((BYTE *) cur - (BYTE *) root), FALSE);
        }
        break;
      default:
        name.Format("Unknown directory entry type found: %i\r\nStopping...", cur->type);
        AfxMessageBox(name);
        m_too_many = TRUE; // break out of the while loop
    }
    
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
  }
}

void *CLean::ReadFile(DWORD64 block, DWORD64 *Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *buffer = NULL;
  struct S_LEAN_INODE *inode;
  BYTE *ptr;
  CString cs;
  DWORD64 count;
  
  inode = (struct S_LEAN_INODE *) calloc(m_block_size, 1);
  dlg->ReadBlocks(inode, m_lba, block, m_block_size, 1);
  
  // check to make sure the inode is valid
  if (!ValidInode(inode)) {
    cs.Format("Read: Found invalid Inode at %I64i", block);
    AfxMessageBox(cs);
    free(inode);
    return NULL;
  }
  
  if (Size) *Size = inode->file_size;
  
  // safety catch
  if (inode->block_count == 0)
    return NULL;
  
  // this allocates at least 1 block, even if the file size is zero
  if (inode->file_size < m_block_size)
    buffer = (struct S_LEAN_DIRENTRY *) calloc(m_block_size, 1);
  else
    buffer = (struct S_LEAN_DIRENTRY *) calloc((size_t) inode->file_size, 1);
  ptr = (BYTE *) buffer;

  // if the file size is zero, just return
  if (inode->file_size == 0)
    return ptr;
  
  // does the file start in the inode?
  count = inode->file_size;
  if (!(inode->attributes & LEAN_ATTR_EAS_IN_INODE)) {
    if (count > (m_block_size - LEAN_INODE_SIZE)) {
      memcpy(ptr, (BYTE *) inode + LEAN_INODE_SIZE, (m_block_size - LEAN_INODE_SIZE));
      ptr += (m_block_size - LEAN_INODE_SIZE);
      count -= (m_block_size - LEAN_INODE_SIZE);
    } else {
      memcpy(ptr, (BYTE *) inode + LEAN_INODE_SIZE, (size_t) count);
      return ptr;
    }
  }

  // check that the starting extent value matches our inode value
  if (block != inode->extent_start[0]) {
    free(buffer);
    return NULL;
  }

  struct S_LEAN_BLOCKS extents;
  if (ReadFileExtents(&extents, inode->extent_start[0]) < 1) {
    free(buffer);
    return NULL;
  }

  if (extents.extent_count > 0) {
    BYTE *block = (BYTE *) malloc(m_block_size);
    unsigned i = 0, j = 1; // The first read needs to skip the inode (j = 1)
    while (count > 0) {
      if (j >= extents.extent_size[i])
        i++, j = 0;
      dlg->ReadBlocks(block, m_lba, extents.extent_start[i] + j, m_block_size, 1);
      if (count > m_block_size) {
        memcpy(ptr, block, m_block_size);
        ptr += m_block_size;
        count -= m_block_size;
      } else {
        memcpy(ptr, block, (size_t) count);
        count = 0;
      }
      j++;
    }
    free(block);
  }
  FreeExtentBuffer(&extents);
  
  free(inode);
  return buffer;
}

void CLean::WriteFile(void *buffer, struct S_LEAN_BLOCKS *extents, DWORD64 Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode;
  BYTE *ptr = (BYTE *) buffer;
  DWORD64 count;
  
  inode = (struct S_LEAN_INODE *) calloc(m_block_size, 1);
  dlg->ReadBlocks(inode, m_lba, extents->extent_start[0], m_block_size, 1);
  
  // check to make sure the inode is valid
  if (!ValidInode(inode)) {
    CString cs;
    cs.Format("Write: Found invalid Inode at %I64i", extents->extent_start[0]);
    AfxMessageBox(cs);
    free(inode);
    return;
  }

  // actual count of bytes allocated for the file
  DWORD64 allocated_space = extents->block_count * m_block_size;
  
  // does the file start in the inode?
  count = Size;
  if (!(inode->attributes & LEAN_ATTR_EAS_IN_INODE)) {
    if (count > (m_block_size - LEAN_INODE_SIZE)) {
      memcpy((BYTE *) inode + LEAN_INODE_SIZE, ptr, (m_block_size - LEAN_INODE_SIZE));
      ptr += (m_block_size - LEAN_INODE_SIZE);
      count -= (m_block_size - LEAN_INODE_SIZE);
    } else {
      if (m_del_clear)
        memset((BYTE *) inode + LEAN_INODE_SIZE, 0, (m_block_size - LEAN_INODE_SIZE));
      memcpy((BYTE *) inode + LEAN_INODE_SIZE, ptr, (size_t) count);
      count = 0;
    }
    allocated_space -= LEAN_INODE_SIZE;
  } else
    allocated_space -= m_block_size;

  // before we write the extents, do we need to append any?
  if (Size > allocated_space)
    AppendToExtents(extents, Size - allocated_space, 0, TRUE);
  // TODO: if (Size < allocated_space)
  //  TruncateExtents(extents, allocated_space - Size, 0, TRUE);
  WriteFileExtents(extents, inode);

  // update the inode (just incase we need to)
  inode->file_size = Size;
  inode->block_count = extents->block_count;

  BYTE *block = (BYTE *) malloc(m_block_size);
  unsigned i = 0, j = 1; // The first write needs to skip the inode (j = 1)
  while (count > 0) {
    if (j >= extents->extent_size[i])
      i++, j = 0;
    if (count > m_block_size) {
      memcpy(block, ptr, m_block_size);
      ptr += m_block_size;
      count -= m_block_size;
    } else {
      if (m_del_clear)
        memset(block, 0, m_block_size);
      memcpy(block, ptr, (size_t) count);
      count = 0;
    }
    dlg->WriteBlocks(block, m_lba, extents->extent_start[i] + j, m_block_size, 1);
    j++;
  }
  free(block);
  
  // update the extended extent's crc?
  dlg->WriteBlocks(inode, m_lba, extents->extent_start[0], m_block_size, 1);
  UpdateFileExtents(inode, FALSE);

  // update the inode's Timestamps
  CTime time = CTime::GetCurrentTime();
  inode->acc_time = 
    inode->mod_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970

  // update the inode's check sum and write it back
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);
  dlg->WriteBlocks(inode, m_lba, extents->extent_start[0], m_block_size, 1);

  free(inode);
}

void CLean::ZeroExtent(DWORD64 ExtentStart, DWORD ExtentSize) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *zero = calloc(m_block_size, 1);

  for (DWORD i=0; i<ExtentSize; i++)
    dlg->WriteBlocks(zero, m_lba, ExtentStart + i, m_block_size, 1);

  free(zero);
}

// A driver must use the magic, checksum and primarySuper fields of the superblock to identify a valid superblock.
// the super must be between byte offset 512 and byte offset 131072, inclusively.
BOOL CLean::DetectLeanFS(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(131072 + 65536);
  BOOL fnd = FALSE;
  
  // mark the block as "not found"
  m_super_block_loc = 0xFFFFFFFF;
  
  // since we have an unknown block size, the super must be on a 512-byte boundary
  //  from 512 to and including 131072.  That is 256+1 512-byte sectors.
  // since we support up to 64k block sizes, we read an extra 128 512-byte sectors so we can
  //  check the checksum as well.
  // We also (temporarily) change the sector size to 512, so we only read 256+128 512-byte sectors.
  DWORD64 lba = (m_lba * dlg->m_sect_size) / 512;
  unsigned org_size = dlg->m_sect_size;
  dlg->m_sect_size = 512;
  dlg->ReadFromFile(buffer, lba, 256+128);
  dlg->m_sect_size = org_size;

  for (unsigned sector=1; sector<=256; sector++) {
    struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) (buffer + (sector * 512));

    // the first entry should be 'LEAN'
    if (super->magic != LEAN_SUPER_MAGIC)
      continue;

    // block / block size
    // (we don't support anything over 64k)
    if ((super->log_block_size < 8) || (super->log_block_size > 16))
      continue;
    unsigned block_size = (1 << super->log_block_size);
    // the superblock must be block aligned
    if (((sector * 512) % block_size) > 0)
      continue;
    unsigned block = ((sector * 512) / block_size);

    // How about the check sum
    DWORD crc = 0, *p = (DWORD *) super;
    for (unsigned i=1; i<(block_size / sizeof(DWORD)); i++)
      crc = (crc << 31) + (crc >> 1) + p[i];
    if (crc != super->checksum)
      continue;
    
    // check the primarySuper field.  It should == block
    if (super->primary_super != (DWORD64) block)
      continue;
    
    // We make a few more checks along the way, here.
    
    // the log2 entry should be at least 12 and not more than 31
    // 12 for 512-byte blocks, 13 for 1024-byte blocks, etc...
    if (super->log_blocks_per_band > 31)
      continue;
    if ((super->log_blocks_per_band - 3) < super->log_block_size)
      continue;
    
    // all but bits 1:0 of state should be zero
    if (super->state & ~0x3)
      continue;
    
    // must be version 1.0 (we don't support backward compatibility)
    if (super->fs_version != 0x0100)
      continue;

    // else we may have a valid LEAN FS super
    m_super_block_loc = block;
    m_block_size = block_size;
    m_tot_blocks = (m_size * dlg->m_sect_size) / m_block_size;
    memcpy(&m_super, super, sizeof(struct S_LEAN_SUPER));
    fnd = TRUE;
  }

  free(buffer);
  return fnd;
}

// if skip is TRUE, skip first dword (is inode, indirect, or superblock)
DWORD CLean::LeanCalcCRC(const void *data, size_t count, const BOOL skip) {
  DWORD crc = 0;
  DWORD *p = (DWORD *) data;
  size_t i = (skip) ? 1 : 0;
  
  count /= 4;
  for (; i<count; i++)
    crc = (crc << 31) + (crc >> 1) + p[i];
  
  return crc;
}

// read in an extent, calculate the crc, and return the crc
// we check to make sure that the extent isn't 'erroneously' long just as a check.
DWORD CLean::LeanCalcExtentCRC(const DWORD64 start, const DWORD size, const BOOL first) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  size_t length = (size_t) size * m_block_size;
  DWORD crc = 0;

  // check that size is less than 64 meg, just so we don't allocate a huge buffer, etc.
  if (length > 0x04000000ULL)
    return 0;

  void *ptr = malloc(length);
  if (ptr == NULL)
    return 0;

  dlg->ReadBlocks(ptr, m_lba, start, m_block_size, size);
  if (first)
    crc = LeanCalcCRC((BYTE *) ptr + LEAN_INODE_SIZE, length - LEAN_INODE_SIZE, FALSE);
  else
    crc = LeanCalcCRC(ptr, length, FALSE);
  
  free(ptr);
  
  return crc;
}

BOOL CLean::ValidInode(const struct S_LEAN_INODE *inode) {
  
  // check to see if inode->magic = 'NODE';
  if (inode->magic != LEAN_INODE_MAGIC)
    return FALSE;
  
  // check to see if crc is correct
  if (inode->checksum != LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE))
    return FALSE;
  
  // if links_count == 0, file should have been deleted
  if (inode->links_count == 0)
    return FALSE;
  
  return TRUE;  
}

BOOL CLean::ValidIndirect(const struct S_LEAN_INDIRECT *indirect) {
  
  // check to see if indirect->magic = 'INDX';
  if (indirect->magic != LEAN_INDIRECT_MAGIC)
    return FALSE;
  
  // check to see if crc is correct
  if (indirect->checksum != LeanCalcCRC(indirect, m_block_size, TRUE))
    return FALSE;
  
  // other checks here

  return TRUE;
}

void CLean::SendToDialog(struct S_LEAN_SUPER *super) {
  
  m_crc.Format("0x%08X", super->checksum);
  m_magic.Format("0x%08X", super->magic);
  m_version.Format("0x%04X", super->fs_version);
  m_pre_alloc.Format("%i", super->pre_alloc_count);
  m_blocks_band.Format("%i", super->log_blocks_per_band);
  m_cur_state.Format("0x%08X", super->state);
  GUID_Format(m_guid, &super->guid);
  GetName(super->volume_label, m_label, 64);
  m_block_count.Format("%I64i", super->block_count);
  m_free_blocks.Format("%I64i", super->free_block_count);
  m_next_free.Format("%I64i", super->next_free_block);
  m_primary_lba.Format("%I64i", super->primary_super);
  m_backup_lba.Format("%I64i", super->backup_super);
  m_bitmap_lba.Format("%I64i", super->bitmap_start);
  m_bitmap_crc.Format("0x%08X", super->bitmap_checksum);
  m_root_lba.Format("%I64i", super->root_start);
  m_bad_lba.Format("%I64i", super->bad_start);
  m_log_block_size.Format("%i", super->log_block_size);
  m_journal_lba.Format("%I64i", super->journal);
  m_capabilities.Format("0x%08X", super->capabilities);

  m_use_extended_extents = (super->capabilities & (1 << 0)) > 0;

  UpdateData(FALSE); // send to Dialog
  
  OnChangeLeanJournal();
  OnChangeLeanVersion();
  OnChangeLeanPreAlloc();
  OnChangeLeanBlockBand();
  OnChangeLeanBlockSize();
}

void CLean::ReceiveFromDialog(struct S_LEAN_SUPER *super) {
  UpdateData(TRUE); // receive from Dialog
  
  super->checksum = convert32(m_crc);
  super->magic = convert32(m_magic);
  super->fs_version = convert16(m_version);
  super->pre_alloc_count = convert8(m_pre_alloc);
  super->log_blocks_per_band = convert8(m_blocks_band);
  super->state = convert32(m_cur_state);
  GUID_Retrieve(m_guid, &super->guid);
  PutName(super->volume_label, m_label, 64);
  super->block_count = convert64(m_block_count);
  super->free_block_count = convert64(m_free_blocks);
  super->next_free_block = convert64(m_next_free);
  super->primary_super = convert64(m_primary_lba);
  super->backup_super = convert64(m_backup_lba);
  super->bitmap_start = convert64(m_bitmap_lba);
  super->bitmap_checksum = convert32(m_bitmap_crc);
  super->root_start = convert64(m_root_lba);
  super->bad_start = convert64(m_bad_lba);
  super->log_block_size = convert8(m_log_block_size);
  super->journal = convert64(m_journal_lba);
  super->capabilities = convert32(m_capabilities);
}

void CLean::SaveItemInfo(HTREEITEM hItem, DWORD64 Inode, DWORD64 FileSize, DWORD flags, DWORD Offset, BOOL CanCopy) {
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->Inode = Inode;
    items->FileSize = FileSize;
    items->Offset = Offset;
    items->CanCopy = CanCopy;
    items->Flags = flags;
  }
}

void CLean::OnLeanCopy() {
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
  //AfxMessageBox("Files transferred.");
}

void CLean::CopyFile(HTREEITEM hItem, CString csName) {
  DWORD64 FileSize;
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    void *ptr = ReadFile(items->Inode, &FileSize);
    if (ptr) {
      CFile file;
      if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
        file.Write(ptr, (UINT) FileSize);
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
void CLean::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CLean::OnLeanView() {
  char szPath[MAX_PATH];
  char szName[MAX_PATH];
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  if (!hItem)
    return;

  CWaitCursor wait;
  
  if (IsDir)  // we shouldn't have found this
    return;

  if ((GetTempPath(MAX_PATH, szPath) == 0) ||
      (GetTempFileName(szPath, "ULT", 0, szName) == 0)) {
    AfxMessageBox("Error creating temp file...");
    return;
  }

  // copy the file to the temp file
  csPath = szName;
  CopyFile(hItem, csPath);

  // open the file using the default/specified app
  csName = AfxGetApp()->GetProfileString("Settings", "DefaultViewerPath", "notepad.exe");
  ShellExecute(NULL, _T("open"), csName, szName, _T(""), SW_SHOWNORMAL);
  
  wait.Restore();
}

void CLean::OnLeanInsert() {
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
  
  CWaitCursor wait;
  
  // get the item from the tree control
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    if (IsDir)
      InsertFolder(items->Inode, csName, csPath);
    else
      InsertFile(items->Inode, csName, csPath);
  }
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
  
  wait.Restore();
  //AfxMessageBox("Files transferred.");
}

// insert an Inode in the currently selected directory, giving it a Symbolic type and data
void CLean::OnLeanInsertSymbolic() {
  struct S_LEAN_BLOCKS extents;
  DWORD64 Size, TotSize;
  DWORD Attrib = IsDlgButtonChecked(IDC_EAS_IN_INODE) ? LEAN_ATTR_EAS_IN_INODE : 0;  // Allow user to specify if EAS_IN_INODE or not on new creation.

  CLeanSymbolic Symbolic;
  if (Symbolic.DoModal() == IDCANCEL)
    return;

  // length of the symbolic name (file data)
  Size = GetNameLen(Symbolic.m_symbolic_name);

  // get the item from the tree control
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    // if we don't have enough room on the image, error and return
    DWORD64 blocks_needed = ((((Attrib & LEAN_ATTR_EAS_IN_INODE) ? m_block_size : LEAN_INODE_SIZE) + Size + (m_block_size - 1)) / m_block_size);
    if (blocks_needed > m_free_block_cnt) {
      CString cs;
      cs.Format("Not enough room left in image. Blocks needed = %i", blocks_needed - m_free_block_cnt);
      AfxMessageBox(cs);
      return;
    }
    
    // allocate the extents for it, returning a "struct S_LEAN_BLOCKS"
    AllocateExtentBuffer(&extents, LEAN_DEFAULT_COUNT);
    // Calculate how many actual bytes we will use
    TotSize = Size + ((Attrib & LEAN_ATTR_EAS_IN_INODE) ? m_block_size : LEAN_INODE_SIZE); 
    if (AppendToExtents(&extents, TotSize, 0, TRUE) == -1) {
      FreeExtentBuffer(&extents);
      return;
    }
    
    // create a root entry in the folder
    int r = AllocateRoot(Symbolic.m_file_name, items->Inode, extents.extent_start[0], LEAN_FT_LNK);
    if (r == -1) {
      FreeExtentBuffer(&extents);
      return;
    }
    
    // create an Inode at extents.extent_start[0]
    BuildInode(&extents, Size, LEAN_ATTR_IFLNK | Attrib);
    
    // copy the file to our system
    BYTE *buffer = (BYTE *) malloc((size_t) Size);
    PutName(buffer, Symbolic.m_symbolic_name, (int) Size);
    WriteFile(buffer, &extents, Size);
    free(buffer);
    
    // free the buffers
    FreeExtentBuffer(&extents);
    
    // update our running count of free clusters
    m_free_block_cnt = CalcFreeBlocks();
  }
  
  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// Inode = starting inode of folder to insert to
// csName = name of file to insert
// csPath = path on host of file to insert
BOOL CLean::InsertFile(DWORD64 Inode, CString csName, CString csPath) {
  struct S_LEAN_BLOCKS extents;
  void *buffer;
  DWORD64 Size, TotSize;
  CFile file;
  DWORD Attrib = IsDlgButtonChecked(IDC_EAS_IN_INODE) ? LEAN_ATTR_EAS_IN_INODE : 0;  // Allow user to specify if EAS_IN_INODE or not on new creation.
  
  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return FALSE;
  }
  Size = file.GetLength();  // TODO: use long version

  // if we don't have enough room on the image, error and return
  DWORD64 blocks_needed = ((((Attrib & LEAN_ATTR_EAS_IN_INODE) ? m_block_size : LEAN_INODE_SIZE) + Size + (m_block_size - 1)) / m_block_size);
  if (blocks_needed > m_free_block_cnt) {
    file.Close();
    CString cs;
    cs.Format("Not enough room left in image. Blocks needed = %i", blocks_needed - m_free_block_cnt);
    AfxMessageBox(cs);
    return FALSE;
  }

  buffer = calloc((size_t) Size + m_block_size, 1);  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, (UINT) Size);
  file.Close();
  
  // allocate the extents for it, returning a "struct S_LEAN_BLOCKS"
  AllocateExtentBuffer(&extents, LEAN_DEFAULT_COUNT);
  // Calculate how many actual bytes we will use
  TotSize = Size + ((Attrib & LEAN_ATTR_EAS_IN_INODE) ? m_block_size : LEAN_INODE_SIZE); 
  if (AppendToExtents(&extents, TotSize, 0, TRUE) == -1) {
    FreeExtentBuffer(&extents);
    free(buffer);
    return FALSE;
  }
  
  // create a root entry in the folder
  int r = AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_REG);
  if (r == -1) {
    FreeExtentBuffer(&extents);
    free(buffer);
    return FALSE;
  }

  // create an Inode at extents.extent_start[0]
  BuildInode(&extents, Size, LEAN_ATTR_IFREG | Attrib);
  
  // copy the file to our system
  WriteFile(buffer, &extents, Size);
  
  // free the buffers
  FreeExtentBuffer(&extents);
  free(buffer);
  
  // update our running count of free clusters
  m_free_block_cnt = CalcFreeBlocks();

  return TRUE;
}

// Inode = starting Inode of folder to insert in to
// csName = name of folder to insert
// csPath = path on host of folder to insert
BOOL CLean::InsertFolder(DWORD64 Inode, CString csName, CString csPath) {
  struct S_LEAN_BLOCKS extents;
  char szPath[MAX_PATH];
  DWORD Attrib = IsDlgButtonChecked(IDC_EAS_IN_INODE) ? LEAN_ATTR_EAS_IN_INODE : 0;  // Allow user to specify if EAS_IN_INODE or not on new creation.
  
  // allocate the extents for the folder, returning a "struct S_LEAN_BLOCKS"
  AllocateExtentBuffer(&extents, LEAN_INODE_EXTENT_CNT);
  if (AppendToExtents(&extents, ((size_t) m_super.pre_alloc_count + 1) * m_block_size, 0, TRUE) == -1) {
    FreeExtentBuffer(&extents);
    return FALSE;
  }
  
  // create a root entry in the folder for the folder
  int r = AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_DIR);
  if (r == -1) {
    FreeExtentBuffer(&extents);
    return FALSE;
  }

  // create an Inode at extents.extent_start[0]
  BuildInode(&extents, 0, LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC | Attrib);

  // add the . and .. entries
  CString csDots = ".";
  if (AllocateRoot(csDots, extents.extent_start[0], extents.extent_start[0], LEAN_FT_DIR) != 1) {
    FreeExtentBuffer(&extents);
    return FALSE;
  }
  IncrementLinkCount(extents.extent_start[0]);

  csDots = "..";
  if (AllocateRoot(csDots, extents.extent_start[0], Inode, LEAN_FT_DIR) != 1) {
    FreeExtentBuffer(&extents);
    return FALSE;
  }
  IncrementLinkCount(Inode);

  // save the current directory
  GetCurrentDirectory(MAX_PATH, szPath);
  
  // Set to the directory of the host's path
  SetCurrentDirectory(csPath);
  
  // now scan the host for files within this folder, recursing folders if needed
  CFileFind filefind;
  BOOL b = TRUE, fnd = filefind.FindFile(NULL);
  while (fnd) {
    fnd = filefind.FindNextFile();
    
    // if '.' or '..', skip so we don't recurse indefinately
    if (filefind.IsDots())
      continue;
    
    if (filefind.IsDirectory())
      b = InsertFolder(extents.extent_start[0], filefind.GetFileName(), filefind.GetFilePath());
    else
      b = InsertFile(extents.extent_start[0], filefind.GetFileName(), filefind.GetFilePath());
  }
  filefind.Close();
  
  // free the extent buffer
  FreeExtentBuffer(&extents);
  
  // restore the current directory
  SetCurrentDirectory(szPath);

  return b;
}

// the user change the status of the "Show Hidden" Check box
void CLean::OnShowHidden() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANShowHidden", m_show_hidden = IsDlgButtonChecked(IDC_SHOW_HIDDEN));
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// the user change the status of the "Show Deleted" Check box
void CLean::OnShowDeleted() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANShowDel", m_show_del = IsDlgButtonChecked(IDC_SHOW_DEL));
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CLean::OnEAsInInode() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANEAsInInode", m_EAs_in_Inode = IsDlgButtonChecked(IDC_EAS_IN_INODE));
}

// copy the Primary Superblock to the Backup Superblock
void CLean::OnSaveSuper() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  // first apply current values
  OnLeanApply();  // updates m_super for us

  // do a few checks just to make sure we are okay
  if ((m_super.backup_super <= m_super_block_loc) || 
      (m_super.backup_super >= m_tot_blocks)) {
    AfxMessageBox("Backup Superblock location is in error.");
    return;
  }

  BYTE *buffer = (BYTE *) calloc(m_block_size, 1);
  
  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));
  
  // write the block
  dlg->WriteBlocks(buffer, m_lba, m_super.backup_super, m_block_size, 1);

  free(buffer);

  // make sure the bitmap is marked for this block
  MarkBlock(m_super.backup_super, TRUE);
  m_free_block_cnt = CalcFreeBlocks();

  AfxMessageBox("Backup Superblock stored.");
}

// copy the Backup Superblock to the Primary Superblock
// ** May need to reload the image since the Root Inode might be different **
void CLean::OnRestoreSuper() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;

  if (AfxMessageBox("This will copy the Backup Superblock to the Primary Superblock"
                    " and reload the image.  Continue?", MB_YESNO, 0) != IDYES)
    return;
  
  ReceiveFromDialog(&m_super); // bring from Dialog

  // do a few checks just to make sure we are okay
  if ((m_super.backup_super <= m_super_block_loc) || 
      (m_super.backup_super >= m_tot_blocks)) {
    AfxMessageBox("Backup Superblock location is in error.");
    return;
  }

  BYTE *buffer = (BYTE *) malloc(m_block_size);
  
  // read the block
  dlg->ReadBlocks(buffer, m_lba, m_super.backup_super, m_block_size, 1);

  // check that the block size is the same
  struct S_LEAN_SUPER *backup = (struct S_LEAN_SUPER *) buffer;
  if (backup->log_block_size != m_super.log_block_size) {
    if (AfxMessageBox("The log_block_size fields do not match. Continue?", MB_YESNO, 0) != IDYES) {
      free(buffer);
      return;
    }
  }
  
  // write the block
  dlg->WriteBlocks(buffer, m_lba, m_super.primary_super, m_block_size, 1);

  free(buffer);

  AfxMessageBox("Primary Superblock stored.\r\n"
                "Ready to Reload image...");
  
  // reload the file
  SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

// the user change the status of the "Delete Clear" Check box
void CLean::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void CLean::OnLeanDelete() {
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
  m_dir_tree.DeleteItem(hItem);

  // select the parent item
  m_dir_tree.Select((hParent != NULL) ? hParent : TVI_ROOT, TVGN_CARET);
  
  // update our running count of free clusters
  m_free_block_cnt = CalcFreeBlocks();
  
  // update the freespace display
  DisplayFreeSpace();
  
  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CLean::DeleteFolder(HTREEITEM hItem) {
  HTREEITEM hChildItem, hDeleteItem;
  struct S_LEAN_ITEMS *items;
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
  hDeleteItem = m_dir_tree.GetParentItem(hItem);
  if (hDeleteItem != NULL) {
    items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hDeleteItem);
    DecrementLinkCount(items->Inode);
    DeleteFile(hItem);
  }
}

void CLean::DeleteFile(HTREEITEM hItem) {
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  struct S_LEAN_DIRENTRY *root, *cur;
  struct S_LEAN_BLOCKS extents;
  DWORD64 RootSize;
  DWORD Offset;
  
  if (items == NULL)
    return;
  
  if (m_del_clear) {
    if (ReadFileExtents(&extents, items->Inode) > 0) {
      for (unsigned int i=0; i<extents.extent_count; i++)
        ZeroExtent(extents.extent_start[i], extents.extent_size[i]);
      FreeExtentBuffer(&extents);
    }
  }
  DeleteInode(items->Inode);
  Offset = items->Offset;
  
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  if (hParent == NULL)
    return;
  
  items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hParent);
  if (items == NULL)
    return;
  
  root = (struct S_LEAN_DIRENTRY *) ReadFile(items->Inode, &RootSize);
  if (root) {
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + Offset);
    cur->type = LEAN_FT_MT;
    if (m_del_clear) {
      cur->inode = 0;
      cur->name_len = 0;
      memset(cur->name, 0, (cur->rec_len * 16) - LEAN_DIRENTRY_NAME);
    }
    if (ReadFileExtents(&extents, items->Inode) > 0) {
      WriteFile(root, &extents, RootSize);
      FreeExtentBuffer(&extents);
    }
    free(root);
  }
}

void CLean::OnSearch() {
  m_dir_tree.Search();
}

void CLean::DisplayFreeSpace(void) {
  CString csFree;
  
  csFree.Format("Free Space: %s (bytes)", (LPCSTR) gFormatNum((size_t) m_free_block_cnt * (1ULL << m_super.log_block_size), FALSE, FALSE));
  SetDlgItemText(IDC_FREE_SIZE_STR, csFree);
}

// calculate the count of free clusters left in the data area
DWORD CLean::CalcFreeBlocks(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD Free = 0, Cluster = 0;
  BYTE *bitmap = NULL;
  DWORD64 bitmap_block, cur_block = 0;

  const unsigned band_size = (1ULL << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8);     // blocks per bitmap
  const unsigned tot_bands = (unsigned) ((m_tot_blocks + (band_size - 1)) / band_size);

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * m_block_size);

  for (unsigned band=0; band<tot_bands; band++) {
    bitmap_block = (band==0) ? m_super.bitmap_start : (band * band_size);
    dlg->ReadBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);

    for (unsigned block=0; block<band_size; block++) {
      unsigned byte = block / 8;
      unsigned bit = block % 8;
      if (!(bitmap[byte] & (1 << bit)))
        Free++;
      // make sure we don't go past the last block in the volume
      cur_block++;
      if (cur_block >= m_tot_blocks) {
        band = tot_bands;  // so we exit the outer for() loop as well.
        break;
      }
    }
  }

  free(bitmap);

  return Free;
}

// S_LEAN_BLOCKS is a list of extents.  Does not know about indirects or anything.
//  is simply a list of extents.  It is up to the other functions to read/write the actual extents to the disk
void CLean::FreeExtents(const struct S_LEAN_BLOCKS *Extents) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bitmap = NULL;
  DWORD last_band = 0xFFFFFFFF, band;
  DWORD64 lba, bitmap_block = 0;
  unsigned int i, extent;
  unsigned free_count = 0;  // count of blocks free'd
  
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8);     // blocks per bitmap

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * m_block_size);

  for (extent=0; extent<Extents->extent_count; extent++) {
    for (i=0; i<Extents->extent_size[extent]; i++) {
      lba = Extents->extent_start[extent] + i;
      band = (DWORD) (lba >> m_super.log_blocks_per_band);
      // are we in the same band as the last one, or do we need to calculate it, load it
      if (band != last_band) {
        // need to write the last one before we read a new one?
        if (last_band != 0xFFFFFFFF)
          dlg->WriteBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);
        last_band = band;
        bitmap_block = (band==0) ? m_super.bitmap_start : (band * band_size);
        dlg->ReadBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);
      }
      // clear the bit in this band
      // calculate block in this band, byte, and bit within bitmap for 'block'
      unsigned block_in_this_band = (unsigned) (lba & ((1 << m_super.log_blocks_per_band) - 1));
      unsigned byte = block_in_this_band / 8;
      unsigned bit = block_in_this_band % 8;
      bitmap[byte] &= ~(1 << bit);
      free_count++;
    }
  }

  // do we need to write the last modified band?
  if (last_band != 0xFFFFFFFF)
    dlg->WriteBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);

  // we also need to update the FreeCount in the super.
  m_super.free_block_count += free_count;
  // TODO: update super:CRC and write the super to the disk

  // free the buffer
  free(bitmap);
}

// returns a free block (or zero if none found)
DWORD64 CLean::GetFreeBlock(DWORD64 Start, BOOL MarkIt) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer;
  DWORD64 bitmap_block;
  
  int j;
  unsigned i, pos;
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8); // blocks per bitmap
  const unsigned bytes_bitmap = bitmap_size * m_block_size;
  const unsigned tot_bands = (unsigned) ((m_super.block_count + (band_size - 1)) / band_size);
  buffer = (BYTE *) malloc(bitmap_size * m_block_size);
  DWORD64 block = 0;
  
  // TODO: Start  // start with specified block.  Will have to skip to next band and what not
  
  for (i=0; i<tot_bands; i++) {
    // read in a bitmap
    bitmap_block = (i==0) ? m_super.bitmap_start : (band_size * i);
    dlg->ReadBlocks(buffer, m_lba, bitmap_block, m_block_size, bitmap_size);
    pos = 0;
    
    while (pos < bytes_bitmap) {
      for (j=0; j<8; j++) {
        if ((buffer[pos] & (1<<j)) == 0) {
          if (MarkIt) {
            buffer[pos] |= (1<<j);  // mark it
            dlg->WriteBlocks(buffer, m_lba, bitmap_block, m_block_size, bitmap_size);
            // we also need to update the FreeCount in the super.
            m_super.free_block_count--;
            // TODO: update super:CRC and write the super to the disk
          }
          free(buffer);
          return block;
        }
        block++;
      }
      pos++;
    }
  }
  
  free(buffer);
  return 0;
}

// Mark a Block as used/free in the corresponding band's bitmap
void CLean::MarkBlock(DWORD64 Block, BOOL MarkIt) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bitmap = NULL;
  DWORD64 bitmap_block;
  DWORD band;
  
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8); // blocks per bitmap

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * m_block_size);
  
  band = (DWORD) (Block >> m_super.log_blocks_per_band);
  bitmap_block = (band==0) ? m_super.bitmap_start : (band * band_size);
  dlg->ReadBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);

  // clear/set the bit in this band
  // calculate block in this band, byte, and bit within bitmap for 'block'
  unsigned block_in_this_band = (unsigned) (Block & ((1 << m_super.log_blocks_per_band) - 1));
  unsigned byte = block_in_this_band / 8;
  unsigned bit = block_in_this_band % 8;
  if (MarkIt)
    bitmap[byte] |= (1 << bit);
  else
    bitmap[byte] &= ~(1 << bit);
  
  dlg->WriteBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);
  
  free(bitmap);
}

// Load in the bitmap, calculate, and return a checksum
DWORD CLean::BitmapChecksum(struct S_LEAN_SUPER *super) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *bitmap;
  BYTE *ptr;
  DWORD64 bitmap_block;
  DWORD band, crc = 0;
  
  unsigned block_size = (1 << super->log_block_size);
  const DWORD64 band_size = ((DWORD64) 1 << super->log_blocks_per_band); // blocks per band
  unsigned bitmap_size = (unsigned) (band_size / block_size / 8);       // in blocks
  const unsigned tot_bands = (unsigned) ((super->block_count + (band_size - 1)) / band_size);

  // create the buffer
  bitmap = malloc(bitmap_size * tot_bands * block_size);
  ptr = (BYTE *) bitmap;
  
  for (band=0; band<tot_bands; band++) {
    bitmap_block = (band==0) ? super->bitmap_start : (band * band_size);
    dlg->ReadBlocks(ptr, m_lba, bitmap_block, block_size, bitmap_size);
    ptr += (bitmap_size * block_size);
  }

  crc = LeanCalcCRC(bitmap, (tot_bands * bitmap_size * block_size), FALSE);
  
  free(bitmap);

  return crc;
}

// This extracts a string from a UTF-8 buffer ('ptr') and puts it in a CString 'name'
void CLean::GetName(void *ptr, CString &name, const int len) {
  name.Empty();
  
  
  // TODO: Currently assumes English and the first 128 code points
  // A proper conversion from UTF-8 to MFC's CString should be written
  void *p = name.GetBuffer(len + 1);
  memcpy(p, ptr, len);
  name.ReleaseBuffer(len);


}

// returns the count of bytes needed to store a name
WORD CLean::GetNameLen(CString name) {
  int ret = 0;

  
  // TODO: Currently assumes English and the first 128 code points
  // A proper routine to get the count of bytes needed should be written
  ret = name.GetLength();

  
  return ret;
}

// This extracts a string from a CString 'name' and encodes it to a UTF-8 buffer ('ptr')
// max_len is the max character length to place
// if max_len == -1, get the length of the CString as the max count of chars to encode
WORD CLean::PutName(void *ptr, CString name, int max_len) {
  WORD ret = 0;
  if (max_len == -1)
    max_len = name.GetLength();

  // TODO: Currently assumes English and the first 128 code points
  // A proper conversion from MFC's CString to UTF-8 should be written
  int len = name.GetLength();
  if (len > max_len) len = max_len;
  memcpy(ptr, (LPCSTR) name, len);
  ret = len;
  

  return ret;
}

// read in the current root (Inode) and add to it
// returns  1 on good return
//         -1 on error
//         -2 on not enough room in root
int CLean::AllocateRoot(CString csName, DWORD64 Inode, DWORD64 Start, BYTE Attrib) {
  struct S_LEAN_DIRENTRY *root, *cur, *next;
  struct S_LEAN_BLOCKS extents;
  const WORD len = GetNameLen(csName);
  DWORD64 root_size;
  BYTE byte;

  root = (struct S_LEAN_DIRENTRY *) ReadFile(Inode, &root_size);
  if (root) {
    cur = root;
    if (ReadFileExtents(&extents, Inode) < 1)
      return -1;
    while ((unsigned char *) cur < ((unsigned char *) root + root_size)) {
      if (cur->rec_len == 0)
        break;
      if (cur->type == LEAN_FT_MT) {
        if (((cur->rec_len * 16) - LEAN_DIRENTRY_NAME) >= len) {
          // see if there is enough room to divide this record in to two
          if (cur->rec_len > ((((len + LEAN_DIRENTRY_NAME) + 15) / 16) + 1)) {  // +1 is only 1 extra???
            byte = cur->rec_len;
            cur->rec_len = ((len + LEAN_DIRENTRY_NAME + 15) / 16);
            next = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
            next->rec_len = byte - cur->rec_len;
            next->inode = 0;
            next->name_len = 0;
            next->type = LEAN_FT_MT;
          }
          CreateRootEntry(cur, Start, Attrib, csName);
          // write it back to the root
          WriteFile(root, &extents, root_size);
          FreeExtentBuffer(&extents);
          free(root);
          return 1;
        }
      }
      cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
    }
    
    // did not find an empty entry in the current buffer,
    //  so lets add to the file length and put it there
    unsigned extended_size = ((LEAN_DIRENTRY_NAME + len) + 15) & ~15;
    root = (struct S_LEAN_DIRENTRY *) realloc(root, (size_t) (root_size + extended_size));
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + root_size);

    CreateRootEntry(cur, Start, Attrib, csName);

    // WriteFile will add to the extents if needed
    WriteFile(root, &extents, root_size + extended_size);
    FreeExtentBuffer(&extents);
    free(root);
    return 1;
  }
  AfxMessageBox("Error adding to directory");
  return -1;
}

void CLean::BuildInode(struct S_LEAN_BLOCKS *extents, DWORD64 Size, DWORD Attrib) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode;
  const DWORD64 pre_alloc_count = ((Attrib & LEAN_ATTR_PREALLOC) ? (m_super.pre_alloc_count + 1) : 0) + 1; // +1 for the Inode
  
  CTime time = CTime::GetCurrentTime();
  const INT64 now = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  
  inode = (struct S_LEAN_INODE *) calloc(m_block_size, 1);
  
  const unsigned max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  inode->magic = LEAN_INODE_MAGIC;
  inode->extent_count = (extents->extent_count > max_extents) ? max_extents : (BYTE) extents->extent_count;
  inode->indirect_count = 0;
  inode->links_count = 1;
  inode->attributes = Attrib;
  inode->file_size = Size;
  if (Attrib & LEAN_ATTR_EAS_IN_INODE)
    inode->block_count = pre_alloc_count + ((Size + (m_block_size - 1)) / m_block_size);
  else {
    if (Size > (m_block_size - LEAN_INODE_SIZE)) {
      DWORD64 s = Size - (m_block_size - LEAN_INODE_SIZE);
      inode->block_count = pre_alloc_count + ((s + (m_block_size - 1)) / m_block_size);
    } else
      inode->block_count = pre_alloc_count;
  }
  inode->acc_time = 
    inode->sch_time = 
    inode->mod_time = 
    inode->cre_time = now;
  inode->first_indirect = 0;
  inode->last_indirect = 0;
  inode->fork = 0;

  WriteFileExtents(extents, inode);  // must write at least the first 6 so the checksum is correct
  dlg->WriteBlocks(inode, m_lba, extents->extent_start[0], m_block_size, 1);

  free(inode);
}

void CLean::IncrementLinkCount(DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) malloc(m_block_size);
  
  dlg->ReadBlocks(inode, m_lba, Inode, m_block_size, 1);

  inode->links_count++;
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);
  
  dlg->WriteBlocks(inode, m_lba, Inode, m_block_size, 1);
  free(inode);
}

// TODO: does not delete the Inode... Should it, or leave it to the user?
void CLean::DecrementLinkCount(DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) malloc(m_block_size);
  
  dlg->ReadBlocks(inode, m_lba, Inode, m_block_size, 1);

  if (inode->links_count > 1) {
    inode->links_count--;
    inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);
    
    dlg->WriteBlocks(inode, m_lba, Inode, m_block_size, 1);
  }
  free(inode);
}

void CLean::DeleteInode(DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) malloc(m_block_size);
  struct S_LEAN_BLOCKS extents;
  
  dlg->ReadBlocks(inode, m_lba, Inode, m_block_size, 1);

  if (inode->links_count > 1) {
    inode->links_count--;
    inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);
  } else {
    if (inode->fork)
      DeleteInode(inode->fork);
    
    // free the extents
    if (ReadFileExtents(&extents, Inode) > 0) {
      FreeExtents(&extents);
      FreeExtentBuffer(&extents);
    }

    // we need to free the indirects as well.
    if (inode->first_indirect > 0)
      DeleteIndirects(inode->first_indirect);
    
    // (For debugging purposes) also clear the inode or parts of it
    memset(inode, 0, LEAN_INODE_SIZE);
  }
  
  dlg->WriteBlocks(inode, m_lba, Inode, m_block_size, 1);
  free(inode);
}

void CLean::DeleteIndirects(DWORD64 block) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INDIRECT *indirect = (struct S_LEAN_INDIRECT *) malloc(m_block_size);
  DWORD64 block_num = block;

  while (block_num > 0) {
    MarkBlock(block_num, FALSE);
    
    // read in the indirect to get the next one.
    dlg->ReadBlocks(indirect, m_lba, block_num, m_block_size, 1);
    if (!ValidIndirect(indirect))
      break;
    block = indirect->next_indirect;
    
    // (For debugging purposes) also clear the indirect
    memset(indirect, 0, m_block_size);
    dlg->WriteBlocks(indirect, m_lba, block_num, m_block_size, 1);

    // next block number or zero
    block_num = block;
  }

  free(indirect);
}

void CLean::OnLeanEntry() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CLeanEntry LeanEntry;
  struct S_LEAN_INODE *inode_buff = (struct S_LEAN_INODE *) malloc(m_block_size);
  
  if (hItem) {
    struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      // read the inode
      dlg->ReadBlocks(inode_buff, m_lba, items->Inode, m_block_size, 1);
      memcpy(&LeanEntry.m_inode, inode_buff, LEAN_INODE_SIZE);
      LeanEntry.m_hItem = hItem;
      LeanEntry.m_parent = this;
      LeanEntry.m_inode_num = items->Inode;
      LeanEntry.DoModal();
    }
  }
  
  // for some reason, sometimes, the dir tree doesn't get re-drawn after this, so we do it here
  m_dir_tree.Invalidate(FALSE);
}

void CLean::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CLean::OnKillfocusLeanGuid() {
  CString cs;
  GetDlgItemText(IDC_LEAN_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    GetDlgItem(IDC_LEAN_GUID)->SetFocus();
  }
}

void CLean::OnGuidCreate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_LEAN_GUID, cs);
}

void CLean::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CLean::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CLean::OnViewJournal() {
  CLeanJournal Journal;
  
  if (m_super.journal == 0) {
    AfxMessageBox("SuperBlock.Journal == 0");
    return;
  }
  
  DWORD64 size = 0;
  Journal.m_buffer = ReadFile(m_super.journal, &size);
  Journal.m_parent = this;
  if (Journal.m_buffer) {
    if (Journal.DoModal() == IDOK) {
      struct S_LEAN_BLOCKS extents;
      AllocateExtentBuffer(&extents, 1);
      extents.extent_count = 1;
      extents.extent_size[0] = 1 + JOURNAL_SIZE;
      extents.extent_start[0] = m_super.journal;
      WriteFile(Journal.m_buffer, &extents, size);
      FreeExtentBuffer(&extents);
    }
    free(Journal.m_buffer);
  }
}

void CLean::OnJournalInode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_block_size);
  CLeanEntry LeanEntry;
  
  // read the inode
  dlg->ReadBlocks(buffer, m_lba, m_super.journal, m_block_size, 1);
  memcpy(&LeanEntry.m_inode, buffer, LEAN_INODE_SIZE);
  LeanEntry.m_hItem = NULL;
  LeanEntry.m_parent = this;
  LeanEntry.m_inode_num = m_super.journal;
  if (LeanEntry.DoModal() == IDOK) { // apply button pressed?
    // must read it back in so we don't "destroy" the EA's we might have updated in LeanEntry
    dlg->ReadBlocks(buffer, m_lba, m_super.journal, m_block_size, 1);
    memcpy(buffer, &LeanEntry.m_inode, LEAN_INODE_SIZE);
    dlg->WriteBlocks(buffer, m_lba, m_super.journal, m_block_size, 1);
  }
  free(buffer);
}

void CLean::OnErase() {
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

// allocate blocks, building extents for a Size count of blocks
// S_LEAN_BLOCKS is a list of extents.  Does not know about indirects or anything.
//  is simply a list of extents.  It is up to the other functions to read/write the actual extents to the disk
// Size = bytes we need appended
int CLean::AppendToExtents(struct S_LEAN_BLOCKS *extents, DWORD64 Size, DWORD64 Start, BOOL MarkIt) {
  unsigned i, cnt = extents->extent_count;
  unsigned count = (unsigned) ((Size + (m_block_size - 1)) / m_block_size);
  DWORD64 block = Start;

  for (i=0; i<count; i++) {
    block = GetFreeBlock(block, MarkIt);
    
    // if incrementing Extents->extent_size[cnt] will be greater than will fit in a DWORD sized then move to next one
    // Ben: For debugging purposes, we can change the 0xFFFF to 32 to create a few Indirects for each (large) file.
    if (extents->extent_size[cnt] == MAXDWORD /* 32 */) {
      cnt++;
      if (cnt >= extents->allocated_count)
        ReAllocateExtentBuffer(extents, extents->allocated_count + LEAN_DEFAULT_COUNT);
    }
    
    if (extents->extent_size[cnt] == 0) {
      extents->extent_start[cnt] = block;
      extents->extent_size[cnt]++;
    } else if (block == (extents->extent_start[cnt] + extents->extent_size[cnt])) {
      extents->extent_size[cnt]++;
    } else {
      cnt++;
      if (cnt >= extents->allocated_count)
        ReAllocateExtentBuffer(extents, extents->allocated_count + LEAN_DEFAULT_COUNT);
      extents->extent_start[cnt] = block;
      extents->extent_size[cnt] = 1;
    }
  }
  
  // update the count of extents used
  extents->extent_count = cnt + 1;
  extents->block_count += count;

  // return this count
  return extents->extent_count;
}

// get the extents of the file
int CLean::ReadFileExtents(struct S_LEAN_BLOCKS *extents, DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *inode_buffer = (BYTE *) malloc(m_block_size);
  BYTE *indirect_buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INODE *inode;
  struct S_LEAN_INDIRECT *indirect;
  DWORD64 ind_block;
  unsigned i;
  
  // initialize our pointers
  inode = (struct S_LEAN_INODE *) inode_buffer;
  indirect = (struct S_LEAN_INDIRECT *) indirect_buffer;

  DWORD64 *p_extent_start;
  DWORD32 *p_extent_size, *p_extent_crc;
  
  // read the inode
  dlg->ReadBlocks(inode, m_lba, Inode, m_block_size, 1);
  if (!ValidInode(inode)) {
    free(inode_buffer);
    free(indirect_buffer);
    return -1;
  }

  // allocate at least LEAN_INODE_EXTENT_CNT extents to hold the direct extents
  unsigned max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  AllocateExtentBuffer(extents, max_extents);
  
  // do the direct extents first
  p_extent_start = (DWORD64 *) &inode->extent_start[0];
  p_extent_size = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (max_extents * sizeof(DWORD64)));
  p_extent_crc = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (max_extents * (sizeof(DWORD64) + sizeof(DWORD32))));
  for (i=0; i<inode->extent_count; i++) {
    extents->extent_start[extents->extent_count] = p_extent_start[i];
    extents->extent_size[extents->extent_count] = p_extent_size[i];
    if (m_use_extended_extents)
      extents->extent_crc[extents->extent_count] = p_extent_crc[i];
    extents->block_count += p_extent_size[i];
    extents->extent_count++;
  }
  
  // are there any indirect extents?
  unsigned extent_size = (m_use_extended_extents) ? 16 : 12;
  max_extents = (m_block_size - LEAN_INDIRECT_SIZE) / extent_size;
  p_extent_start = (DWORD64 *) (indirect_buffer + LEAN_INDIRECT_SIZE);
  p_extent_size = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * sizeof(DWORD64)));
  p_extent_crc = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD32))));
  if (inode->first_indirect > 0) {
    ind_block = inode->first_indirect;
    while (ind_block > 0) {
      // read in the indirect block
      dlg->ReadBlocks(indirect, m_lba, ind_block, m_block_size, 1);
      if (!ValidIndirect(indirect))
        break;

      // increase the size of our list
      ReAllocateExtentBuffer(extents, extents->extent_count + indirect->extent_count);

      // retrieve the extents in this indirect block
      for (i=0; i<indirect->extent_count; i++) {
        extents->extent_start[extents->extent_count] = p_extent_start[i];
        extents->extent_size[extents->extent_count] = p_extent_size[i];
        if (m_use_extended_extents)
          extents->extent_crc[extents->extent_count] = p_extent_crc[i];
        extents->block_count += p_extent_size[i];
        extents->extent_count++;
      }

      // get next indirect (0 == no more)
      ind_block = indirect->next_indirect;
    }
  }
  
  free(inode_buffer);
  free(indirect_buffer);
  return extents->extent_count;
}

// write the extents of the file to the file
// The count of extents could actually extend past the current inode count.
//  therefore, we add indirects if needed
// (We can't update the extended extent crc here since the file may not have been written to yet.)
int CLean::WriteFileExtents(const struct S_LEAN_BLOCKS *extents, struct S_LEAN_INODE *inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *indirect_buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INDIRECT *indirect;
  DWORD64 prev_block, this_block, next_block, remaining;
  unsigned i, count = 0;
  
  // initialize our pointer
  indirect = (struct S_LEAN_INDIRECT *) indirect_buffer;

  // calculate the max count of extents we can have per this block size
  unsigned max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  DWORD64 *p_extent_start;
  DWORD32 *p_extent_size, *p_extent_crc;
  
  // do the direct extents first
  p_extent_start = (DWORD64 *) ((BYTE *) &inode->extent_start[0]);
  p_extent_size = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (max_extents * sizeof(DWORD64)));
  p_extent_crc = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (max_extents * (sizeof(DWORD64) + sizeof(DWORD32))));
  for (i=0; (i<max_extents) && (count<extents->extent_count); i++) {
    p_extent_start[i] = extents->extent_start[count];
    p_extent_size[i] = extents->extent_size[count];
    if (m_use_extended_extents)
      p_extent_crc[i] = 0;
    count++;
  }

  this_block = 0;
  prev_block = 0;
  remaining = 0;
  BOOL used = FALSE;
  
  inode->indirect_count = 0;

  unsigned extent_size = (m_use_extended_extents) ? 16 : 12;
  max_extents = (m_block_size - LEAN_INDIRECT_SIZE) / extent_size;
  p_extent_start = (DWORD64 *) (indirect_buffer + LEAN_INDIRECT_SIZE);
  p_extent_size = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * sizeof(DWORD64)));
  p_extent_crc = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD32))));

  // are there any indirect extents needed?
  if (count < extents->extent_count) {
    if (inode->first_indirect > 0) {
      this_block = inode->first_indirect;
      used = TRUE;
    } else {
      this_block = GetFreeBlock(0, TRUE);
      inode->first_indirect = this_block;
      used = FALSE;
    }
    
    while (count < extents->extent_count) {
      if (!used) {
        // initialize the indirect block
        memset(indirect, 0, m_block_size);
        indirect->magic = LEAN_INDIRECT_MAGIC;
        indirect->inode = extents->extent_start[0];
        indirect->this_block = this_block;
        indirect->prev_indirect = prev_block;
        indirect->next_indirect = 0;
      } else
        dlg->ReadBlocks(indirect_buffer, m_lba, this_block, m_block_size, 1);
      
      // write to the indirect extents
      indirect->block_count = 0;
      for (i=0; (i<max_extents) && (count<extents->extent_count); i++) {
        p_extent_start[i] = extents->extent_start[count];
        p_extent_size[i] = extents->extent_size[count];
        if (m_use_extended_extents)
          p_extent_crc[i] = 0;
        indirect->block_count += extents->extent_size[count];
        count++;
      }

      indirect->extent_count = i;
      inode->indirect_count++;

      // will there be more?
      if (count < extents->extent_count) {
        if (used && (indirect->next_indirect > 0)) {
          next_block = indirect->next_indirect;
        } else {
          next_block = GetFreeBlock(0, TRUE);
          indirect->next_indirect = next_block;
          used = FALSE;
        }
      } else {
        next_block = this_block;
        if (used)
          remaining = indirect->next_indirect;
        indirect->next_indirect = 0;
      }
      indirect->checksum = LeanCalcCRC(indirect, m_block_size, TRUE);
      
      // write the indirect back
      dlg->WriteBlocks(indirect_buffer, m_lba, this_block, m_block_size, 1);
      
      // initialize for next round
      prev_block = this_block;
      this_block = next_block;
    }
  }
  
  // if we don't need any more indirects, yet there are some allocated,
  //  we need to free each remaining indirect block
  while (remaining > 0) {
    MarkBlock(remaining, FALSE);
    dlg->ReadBlocks(indirect_buffer, m_lba, remaining, m_block_size, 1);
    remaining = indirect->next_indirect;
  }
  
  // update the inode structure
  max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  inode->last_indirect = this_block;
  inode->extent_count = (count > max_extents) ? max_extents : (BYTE) count;
  CTime time = CTime::GetCurrentTime();
  inode->sch_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);

  free(indirect_buffer);
  return extents->extent_count;
}

// update the Extended Extent CRC of the file
// to save time and when we know we only updated the data after the inode, we can specify to only update the inode
void CLean::UpdateFileExtents(struct S_LEAN_INODE *inode, const BOOL inode_only) {

  // only have to do if extended extents are used
  if (!m_use_extended_extents)
    return;

  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *indirect_buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INDIRECT *indirect;
  DWORD64 next_block;
  unsigned i;
  
  // initialize our pointer
  indirect = (struct S_LEAN_INDIRECT *) indirect_buffer;

  // calculate the max count of extents we can have per this block size
  DWORD64 *p_extent_start;
  DWORD32 *p_extent_size, *p_extent_crc;
  
  // do the direct extents first
  p_extent_start = (DWORD64 *) ((BYTE *) &inode->extent_start[0]);
  p_extent_size = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (LEAN_INODE_EXTENT_CNT_EXT * sizeof(DWORD64)));
  p_extent_crc = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (LEAN_INODE_EXTENT_CNT_EXT * (sizeof(DWORD64) + sizeof(DWORD32))));
  for (i=0; i<inode->extent_count; i++)
    p_extent_crc[i] = LeanCalcExtentCRC(p_extent_start[i], p_extent_size[i], i == 0);

  if (!inode_only) {
    unsigned max_extents = (m_block_size - LEAN_INDIRECT_SIZE) / 16;
    p_extent_start = (DWORD64 *) (indirect_buffer + LEAN_INDIRECT_SIZE);
    p_extent_size = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * sizeof(DWORD64)));
    p_extent_crc = (DWORD32 *) (indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD32))));
    
    next_block = inode->first_indirect;
    while (next_block > 0) {
      dlg->ReadBlocks(indirect_buffer, m_lba, next_block, m_block_size, 1);
    
      // update the crc in this indirect block
      for (i=0; i<indirect->extent_count; i++)
        p_extent_crc[i] = LeanCalcExtentCRC(p_extent_start[i], p_extent_size[i], FALSE);
    
      // update the indirect's crc
      indirect->checksum = LeanCalcCRC(indirect, m_block_size, TRUE);
    
      // write the indirect back
      dlg->WriteBlocks(indirect_buffer, m_lba, next_block, m_block_size, 1);
    
      // get the next one
      next_block = indirect->next_indirect;
    }
  }
  
  // update the inode structure
  CTime time = CTime::GetCurrentTime();
  inode->sch_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);

  free(indirect_buffer);
}

// allocate memory for the extents
void CLean::AllocateExtentBuffer(struct S_LEAN_BLOCKS *extents, const unsigned count) {
  extents->extent_count = 0;
  extents->allocated_count = count;
  extents->block_count = 0;
  extents->extent_start = (DWORD64 *) calloc(count, sizeof(DWORD64));
  extents->extent_size = (DWORD32 *) calloc(count, sizeof(DWORD32));
  if (m_use_extended_extents)
    extents->extent_crc = (DWORD32 *) calloc(count, sizeof(DWORD32));
  else
    extents->extent_crc = NULL;
}

// allocate memory for the extents
void CLean::ReAllocateExtentBuffer(struct S_LEAN_BLOCKS *extents, const unsigned count) {
  void *ptr;

  // we don't use realloc() to enlarge the buffers so that we
  //  can clear the (new) unused part
  if (count > extents->allocated_count) {
    // first the 'start'
    ptr = calloc(count, sizeof(DWORD64));
    memcpy(ptr, extents->extent_start, extents->allocated_count * sizeof(DWORD64));
    free(extents->extent_start);
    extents->extent_start = (DWORD64 *) ptr;
    
    // then the 'size'
    ptr = calloc(count, sizeof(DWORD32));
    memcpy(ptr, extents->extent_size, extents->allocated_count * sizeof(DWORD32));
    free(extents->extent_size);
    extents->extent_size = (DWORD32 *) ptr;

    // then the 'crc' ?
    if (m_use_extended_extents) {
      ptr = calloc(count, sizeof(DWORD32));
      memcpy(ptr, extents->extent_crc, extents->allocated_count * sizeof(DWORD32));
      free(extents->extent_crc);
      extents->extent_crc = (DWORD32 *) ptr;
    }

  } else {
    realloc(extents->extent_start, count * sizeof(DWORD64));
    realloc(extents->extent_size, count * sizeof(DWORD32));
    if (m_use_extended_extents)
      realloc(extents->extent_crc, count * sizeof(DWORD32));
  }
  
  // update the new size
  extents->allocated_count = count;
}

// free the memory used by the extents
void CLean::FreeExtentBuffer(struct S_LEAN_BLOCKS *extents) {
  extents->extent_count = 0;
  if (extents->extent_start)
    free(extents->extent_start);
  extents->extent_start = NULL;
  if (extents->extent_size)
    free(extents->extent_size);
  extents->extent_size = NULL;
  if (m_use_extended_extents) {
    if (extents->extent_crc)
      free(extents->extent_crc);
    extents->extent_crc = NULL;
  }
}
