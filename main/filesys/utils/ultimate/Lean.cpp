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

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//// TODO:
////   - Apply
////   - Starting sector in AppendToExtents
////   - 
////   - 
////   - 


/////////////////////////////////////////////////////////////////////////////
// CLean property page

IMPLEMENT_DYNCREATE(CLean, CPropertyPage)

CLean::CLean() : CPropertyPage(CLean::IDD) {
  //{{AFX_DATA_INIT(CLean)
  m_backup_lba = _T("");
  m_bad_lba = _T("");
  m_bitmap_lba = _T("");
  m_crc = _T("");
  m_cur_state = _T("");
  m_free_sectors = _T("");
  m_guid = _T("");
  m_label = _T("");
  m_magic = _T("");
  m_pre_alloc = _T("");
  m_primary_lba = _T("");
  m_root_lba = _T("");
  m_sect_band = _T("");
  m_sect_count = _T("");
  m_version = _T("");
  m_journal_lba = _T("");
  m_sect_size = _T("");
  m_show_del = FALSE;
  m_del_clear = FALSE;
  m_ESs_in_Inode = FALSE;
  //}}AFX_DATA_INIT
  m_hard_format = FALSE;
}

CLean::~CLean(){
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
  DDX_Text(pDX, IDC_LEAN_CRC, m_crc);
  DDV_MaxChars(pDX, m_crc, 16);
  DDX_Text(pDX, IDC_LEAN_CUR_STATE, m_cur_state);
  DDV_MaxChars(pDX, m_cur_state, 16);
  DDX_Text(pDX, IDC_LEAN_FREE_SECTORS, m_free_sectors);
  DDV_MaxChars(pDX, m_free_sectors, 32);
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
  DDX_Text(pDX, IDC_LEAN_SECT_BAND, m_sect_band);
  DDV_MaxChars(pDX, m_sect_band, 8);
  DDX_Text(pDX, IDC_LEAN_SECT_SIZE, m_sect_size);
  DDV_MaxChars(pDX, m_sect_size, 32);
  DDX_Text(pDX, IDC_LEAN_SECT_COUNT, m_sect_count);
  DDV_MaxChars(pDX, m_sect_count, 32);
  DDX_Text(pDX, IDC_LEAN_VERSION, m_version);
  DDV_MaxChars(pDX, m_version, 16);
  DDX_Text(pDX, IDC_JOURNAL_LBA, m_journal_lba);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  DDX_Check(pDX, IDC_DEL_CLEAR, m_del_clear);
  DDX_Check(pDX, IDC_EAS_IN_INODE, m_ESs_in_Inode);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLean, CPropertyPage)
  //{{AFX_MSG_MAP(CLean)
  ON_WM_HELPINFO()
  ON_BN_CLICKED(ID_APPLY, OnLeanApply)
  ON_BN_CLICKED(ID_CLEAN, OnLeanClean)
  ON_BN_CLICKED(ID_FORMAT, OnLeanFormat)
  ON_BN_CLICKED(ID_CHECK, OnLeanCheck)
  ON_EN_CHANGE(IDC_JOURNAL_LBA, OnChangeLeanVersion)
  ON_EN_CHANGE(IDC_LEAN_VERSION, OnChangeLeanVersion)
  ON_EN_CHANGE(IDC_LEAN_PRE_ALLOC, OnChangeLeanPreAlloc)
  ON_EN_CHANGE(IDC_LEAN_SECT_BAND, OnChangeLeanSectBand)
  ON_EN_CHANGE(IDC_LEAN_SECT_SIZE, OnChangeLeanSectSize)
  ON_BN_CLICKED(ID_COPY, OnLeanCopy)
  ON_BN_CLICKED(ID_INSERT, OnLeanInsert)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_ENTRY, OnLeanEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_BN_CLICKED(IDC_LEAN_CRC_UPDATE, OnLeanCrcUpdate)
  ON_BN_CLICKED(IDC_LEAN_MAGIC_UPDATE, OnLeanMagicUpdate)
  ON_BN_CLICKED(IDC_LEAN_CURRENT_STATE, OnLeanCurrentState)
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
  ON_BN_CLICKED(IDC_SHOW_DEL, OnShowDeleted)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_BN_CLICKED(IDC_EAS_IN_INODE, OnEAsInInode)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLean message handlers
BOOL CLean::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_show_del = AfxGetApp()->GetProfileInt("Settings", "LEANShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "LEANDelClear", FALSE);
  m_ESs_in_Inode = AfxGetApp()->GetProfileInt("Settings", "LEANEAsInInode", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
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

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(hItem != NULL);
  
  *pResult = 0;
}

void CLean::OnLeanApply() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  DWORD crc;
  int ret;
  
  ReceiveFromDialog(&m_super); // bring from Dialog
  
  memset(buffer, 0, MAX_SECT_SIZE);
  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));

  crc = LeanCalcCRC(buffer, dlg->m_sect_size);
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
  dlg->WriteToFile(buffer, m_lba + m_super_lba, 1, FALSE);
}

void CLean::OnLeanClean() {
  int r = AfxMessageBox("This will erase the volume, leaving the SuperBlock as is.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  Format(FALSE);
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

/* Calculate how many sectors each band will use
 *   returns the log value.  i.e.: returns (x) of  2^x 
 * TODO: can't we just use LOG() ????
 */
BYTE CLean::lean_calc_log_band_size(const DWORD sect_size, const DWORD64 tot_sectors) {
  BYTE ret = 12;
  int i;

  for (i=63; i>16; i--) {
    if (tot_sectors & ((DWORD64) 1 << i)) {
      ret = (BYTE) (i - 4);
      break;
    }
  }

  // A band must be large enough to occupy all bits in a bitmap sector.
  //  therefore, 512-byte sectors must return at least a log2 of 12.
  //             1024-byte sectors must return at least a log2 of 13.
  //             2048-byte sectors must return at least a log2 of 14.
  //             4096-byte sectors must return at least a log2 of 15, etc
  if (ret < (LOG2(sect_size) + 3))
    return (LOG2(sect_size) + 3);
  
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
  unsigned u, k;
  size_t reserved;
  DWORD64 lba;
  
  // m_size must be at least 1024 sectors
  if (m_size < 1024) {
    AfxMessageBox("Partition must be at least 1024 sectors");
    return FALSE;
  }
  
  // Up to the first 33 sectors are reserved for the boot code and the super block.
  // The super block can be located in sectors 1 to 32 (zero based)
  // We place the superblock just after the boot code, in this case LBA 31
  BYTE *buffer = (BYTE *) calloc(dlg->m_sect_size * (32 + 1), 1);
  
  if (!m_hard_format)
    ReceiveFromDialog(&m_super); // bring from Dialog
  
  // first, clear out the volume
  for (lba=0; lba<m_size; lba++)
    dlg->WriteToFile(buffer, m_lba + lba, 1, FALSE);
  
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
        if (filesize <= (31 * dlg->m_sect_size)) {
          reserved = (filesize + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
          bsfile.Read(buffer, (UINT) filesize);
          dlg->WriteToFile(buffer, m_lba, (UINT) reserved, FALSE);
        } else {
          cs.Format("Boot sector must be <= %i bytes", (31 * dlg->m_sect_size));
          AfxMessageBox(cs);
        }
        bsfile.Close();
      }
    }
  } // TODO: else write a generic boot....
  
  // restore our SuperBlock info and get the specs to format the volume with
  CLeanFormat format;
  format.m_journal = FALSE;
  format.m_eas_after_inode = FALSE;
  format.m_pre_alloc_count = (8-1);
  format.m_root_sectors = 32;
  if (!m_hard_format) {
    format.m_journal = (m_super.fs_version == 0x0007);
    format.m_pre_alloc_count = m_super.pre_alloc_count;
  }
  if (format.DoModal() != IDOK) {
    free(buffer);
    return FALSE;
  }
  
  // Calculate the bitmap size for a single band, which will also calculate the band size.
  // A band can be 2^12 to 2^31 sectors.
  // A band must be large enough to occupy all bits in a bitmap sector.
  //  therefore, 512-byte sectors must return at least a log2 of 12.
  //             1024-byte sectors must return at least a log2 of 13.
  //             2048-byte sectors must return at least a log2 of 14.
  //             4096-byte sectors must return at least a log2 of 15, etc
  const BYTE log_band_size = lean_calc_log_band_size(dlg->m_sect_size, m_size);
  const DWORD band_size = (1 << log_band_size);
  unsigned bitmap_size = (band_size / dlg->m_sect_size / 8);
  const unsigned tot_bands = (unsigned) (m_size + (band_size - 1)) / band_size;
  
  // now create a super block and place it at sector 31
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) (buffer + (31 * dlg->m_sect_size));
  memset(super->reserved, 0, dlg->m_sect_size);
  super->magic = LEAN_SUPER_MAGIC;
  if (format.m_journal || (dlg->m_sect_size > 512))
    super->fs_version = 0x0007;  // 0.7
  else
    super->fs_version = 0x0006;  // 0.6
  super->log_sectors_per_band = log_band_size;
  super->pre_alloc_count = format.m_pre_alloc_count;
  super->state = (0<<1) | (1<<0);  // clean unmount
  GUID_Create(&super->guid, GUID_TYPE_RANDOM);
  strcpy((char *) super->volume_label, "A label goes here.");
  super->sector_count = m_size;     // 32 = boot, 1 for super, bitmap(s), root size, 1 backup super
  super->free_sector_count = (m_size - 32 - 1 - (tot_bands * bitmap_size) - LEAN_ROOT_SIZE - 1);
  super->primary_super = 31;
  super->backup_super = ((band_size - 1) < m_size) ? (band_size - 1) : (m_size - 1);   // last sector in first band
  super->bitmap_start = 31 + 1;
  super->root_start = super->bitmap_start + bitmap_size;
  super->bad_start = 0;  // no bad sectors (yet?)
  super->journal = (format.m_journal) ? super->backup_super - JOURNAL_SIZE - 1: 0;   // -1 for the inode sector
  super->log_sector_size = LOG2(dlg->m_sect_size) - 9;
  super->checksum = LeanCalcCRC(super, dlg->m_sect_size);
  
  // create a buffer for the bitmap(s), and mark the first few bits as used.
  BYTE *bitmap = (BYTE *) calloc(bitmap_size * dlg->m_sect_size, 1);
  for (u=0; u<(((super->root_start + LEAN_ROOT_SIZE) - 1) / 8); u++)
    bitmap[u] = 0xFF;
  bitmap[u] = (0xFF >> (8 - ((super->root_start + LEAN_ROOT_SIZE) % 8)));
  bitmap[((band_size - 1) / 8)] = (BYTE) ((DWORD) 0xFF << (8 - (1 + 1 + JOURNAL_SIZE)));  // mark one for the backup, 1 for the inode, and JOURNAL_SIZE bits for the Journal
  
  // create a root directory
  struct S_LEAN_INODE *root = (struct S_LEAN_INODE *) calloc(LEAN_ROOT_SIZE * dlg->m_sect_size, 1);
  root->magic = LEAN_INODE_MAGIC;
  root->extent_count = 1;
  memset(root->reserved, 0, 3);
  root->links_count = 2;  // the "." and ".." entries
  root->uid = 0;
  root->gid = 0;
  if (format.m_eas_after_inode) {
    root->attributes = LEAN_ATTR_IXUSR | LEAN_ATTR_IRUSR | LEAN_ATTR_IWUSR | LEAN_ATTR_ARCHIVE | LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC | LEAN_ATTR_EAS_IN_INODE;
    root->file_size = ((LEAN_ROOT_SIZE - 1) * dlg->m_sect_size);
  } else {
    root->attributes = LEAN_ATTR_IXUSR | LEAN_ATTR_IRUSR | LEAN_ATTR_IWUSR | LEAN_ATTR_ARCHIVE | LEAN_ATTR_IFDIR | LEAN_ATTR_PREALLOC;
    root->file_size = ((LEAN_ROOT_SIZE - 1) * dlg->m_sect_size) + (dlg->m_sect_size - LEAN_INODE_SIZE);
  }
  root->sector_count = LEAN_ROOT_SIZE;
  CTime now = CTime::GetCurrentTime();
  root->acc_time = 
  root->cre_time =
  root->sch_time =
  root->mod_time = ((INT64) now.GetTime()) * 1000000;  // uS from 1 Jan 1970
  root->first_indirect = 0;
  root->last_indirect = 0;
  root->fork = 0;
  root->extent_start[0] = super->root_start;
  root->extent_size[0] = LEAN_ROOT_SIZE;
  root->checksum = LeanCalcCRC(root, LEAN_INODE_SIZE);
  
  // the directory entry's
  struct S_LEAN_DIRENTRY *entry;
  if (format.m_eas_after_inode) {
    *(DWORD *) ((BYTE *) root + LEAN_INODE_SIZE) = (dlg->m_sect_size - LEAN_INODE_SIZE - sizeof(DWORD));  // padding extended attribute
    entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + dlg->m_sect_size);
  } else
    entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + LEAN_INODE_SIZE);
  // The "." entry
  entry[0].inode = super->root_start;
  entry[0].type = LEAN_FT_DIR;
  entry[0].rec_len = 1;
  entry[0].name_len = 1;
  entry[0].name[0] = '.';
  // The ".." entry
  entry[1].inode = super->root_start;
  entry[1].type = LEAN_FT_DIR;
  entry[1].rec_len = 1;
  entry[1].name_len = 2;
  entry[1].name[0] = '.';
  entry[1].name[1] = '.';
  // The emtpy entry for this sector
  entry[2].inode = 0;
  entry[2].type = LEAN_FT_MT;
  if (format.m_eas_after_inode) {
    entry[2].rec_len = (dlg->m_sect_size - 16 - 16) >> 4;
    u = 2; // next start at 2: 1 for the Inode, 1 for these entries
  } else {
    entry[2].rec_len = (dlg->m_sect_size - LEAN_INODE_SIZE - 16 - 16) >> 4;
    u = 1; // next start at 1: 1 for the Inode
  }
  // fill the remaining sectors of the root with MT's
  for (; u<LEAN_ROOT_SIZE; u++) {
    entry = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + (u * dlg->m_sect_size));
    entry->inode = 0;
    entry->type = LEAN_FT_MT;
    entry->rec_len = dlg->m_sect_size >> 4;
  }
  
  // create an empty Journal?
  if (format.m_journal) {
    // Make an INODE, then write inode and journal to the image
    unsigned Size = dlg->m_sect_size * JOURNAL_SIZE;
    
    struct S_LEAN_SECTORS extents;
    AllocateExtentBuffer(&extents, LEAN_INODE_EXTENT_CNT);
    extents.extent_count = 1;
    extents.extent_size[0] = 1 + JOURNAL_SIZE;
    extents.extent_start[0] = m_super.journal;
    BuildInode(&extents, Size, LEAN_ATTR_HIDDEN | LEAN_ATTR_SYSTEM | LEAN_ATTR_IMMUTABLE_FL | LEAN_ATTR_EAS_IN_INODE);
    
    BYTE *journal = (BYTE *) calloc(Size, 1);
    struct S_LEAN_JOURNAL *hdr = (struct S_LEAN_JOURNAL *) journal;
    struct S_LEAN_JOURNAL_ENTRY *entry = (struct S_LEAN_JOURNAL_ENTRY *) ((BYTE *) hdr + sizeof(struct S_LEAN_JOURNAL));
    hdr->magic = JOURNAL_MAGIC;
    hdr->entry_cnt = (Size - sizeof(struct S_LEAN_JOURNAL)) / sizeof(S_LEAN_JOURNAL_ENTRY);
    for (u=0; u<hdr->entry_cnt; u++)
      entry[u].flags = JOURNAL_ENTRY_INVALID;
    hdr->checksum = LeanCalcCRC(hdr, sizeof(struct S_LEAN_JOURNAL) + (hdr->entry_cnt * sizeof(S_LEAN_JOURNAL_ENTRY)));
    WriteFile(journal, &extents, Size);
    FreeExtentBuffer(&extents);
    free(journal);
  }
  
  // now write the first band to the disk
  dlg->WriteToFile(buffer, m_lba + 0, 31 + 1, FALSE); // + 1 to include the super
  dlg->WriteToFile(bitmap, m_lba + super->bitmap_start, bitmap_size, FALSE);
  dlg->WriteToFile(root, m_lba + super->root_start, LEAN_ROOT_SIZE, FALSE);
  dlg->WriteToFile(super, m_lba + super->backup_super, 1, FALSE);
  
  // now create and write each remaining band (just the bitmap)
  if (tot_bands > 1) {
    memset(bitmap, 0, bitmap_size * dlg->m_sect_size);
    // mark the bitmap sector(s) as used
    for (u=0; u<(bitmap_size / 8); u++)
      bitmap[u] = 0xFF;
    bitmap[u] = (0xFF >> (8 - (bitmap_size % 8)));
  }
  
  lba = 0;
  for (u=1; u<tot_bands; u++) {
    lba += band_size;
    // don't write past end of disk (volume)
    if (((band_size * u) + bitmap_size) > m_size)
      bitmap_size = (unsigned) (m_size - (band_size * u));
    // write the bitmap to current band location
    dlg->WriteToFile(bitmap, m_lba + lba, bitmap_size, FALSE);
  }

  // set all bits in last bitmap that are past end of volume
  //  we are either at LBA l, or there was only 1 band.
  k = (unsigned) ((m_size - lba) / 8);
  bitmap[k] = (0xFF >> ((m_size - lba) % 8));
  for (k++; k<dlg->m_sect_size; k++)
    bitmap[k] = 0xFF;
  if (tot_bands == 1)
    lba = super->bitmap_start;
  dlg->WriteToFile(bitmap, m_lba + lba, bitmap_size, FALSE);
  
  // before we leave, set/clear EAs_in_Inode per format.m_eas_after_inode
  m_ESs_in_Inode = format.m_eas_after_inode;
  CheckDlgButton(IDC_EAS_IN_INODE, format.m_eas_after_inode ? BST_CHECKED : BST_UNCHECKED);

  // done, cleanup and return
  free(root);
  free(bitmap);
  free(buffer);

  return TRUE;
}

void CLean::OnUpdateCode() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FYSOSSIG s_sig;
  CFile bsfile;
  CString cs;
  
  BYTE *existing = (BYTE *) calloc(m_super_lba * dlg->m_sect_size, 1);
  
  // first, read in what we already have (at least the first sector)
  dlg->ReadFromFile(existing, m_lba, 1, FALSE);
  
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
      if (filesize > (m_super_lba * dlg->m_sect_size)) {
        cs.Format("Boot sector must be <= %i bytes", (m_super_lba * dlg->m_sect_size));
        AfxMessageBox(cs);
        filesize = (m_super_lba * dlg->m_sect_size);
      }
      bsfile.Read(existing, (UINT) filesize);
      bsfile.Close();
    }
    
    // restore the FYSOS sig?
    if (AfxGetApp()->GetProfileInt("Settings", "ForceFYSOS", TRUE))
      memcpy(existing + S_FYSOSSIG_OFFSET, &s_sig, sizeof(struct S_FYSOSSIG));
    
    // write it
    dlg->WriteToFile(existing, m_lba, m_super_lba, FALSE);
  }
  
  free(existing);
}

void CLean::OnLeanCrcUpdate() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];

  ReceiveFromDialog(&m_super);

  memset(buffer, 0, MAX_SECT_SIZE);
  memcpy(buffer, &m_super, sizeof(struct S_LEAN_SUPER));
  
  m_super.checksum = LeanCalcCRC(buffer, dlg->m_sect_size);
  m_crc.Format("0x%08X", m_super.checksum);
  SetDlgItemText(IDC_LEAN_CRC, m_crc);
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

void CLean::OnChangeLeanVersion() {
  CString cs;
  WORD version;
  DWORD64 JournalInode;
  BOOL enable;
  
  GetDlgItemText(IDC_LEAN_VERSION, cs);
  version = convert16(cs);
  
  cs.Format("%i.%i", version >> 8, version & 0xFF);
  SetDlgItemText(IDC_LEAN_VERSION_DISP, cs);

  GetDlgItemText(IDC_JOURNAL_LBA, cs);
  JournalInode = convert64(cs);

  enable = (version == 0x0007) && (JournalInode > 0);
  GetDlgItem(ID_VIEW_JOURNAL)->EnableWindow(enable);
  GetDlgItem(ID_JOURNAL_INODE)->EnableWindow(enable);

  GetDlgItem(IDC_JOURNAL_LBA)->EnableWindow(version == 0x0007);
  GetDlgItem(IDC_LEAN_SECT_SIZE)->EnableWindow(version == 0x0007);
}

void CLean::OnChangeLeanPreAlloc() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_PRE_ALLOC, cs);
  byte = convert8(cs);
  
  cs.Format("%i", byte + 1);
  SetDlgItemText(IDC_LEAN_PRE_ALLOC_DISP, cs);
}

void CLean::OnChangeLeanSectBand() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_SECT_BAND, cs);
  byte = convert8(cs);
  
  cs.Format("%i", 1 << byte);
  SetDlgItemText(IDC_LEAN_SECT_BAND_DISP, cs);
}

void CLean::OnChangeLeanSectSize() {
  CString cs;
  int byte;
  
  GetDlgItemText(IDC_LEAN_SECT_SIZE, cs);
  byte = convert8(cs);
  
  cs.Format("%i", 1 << (byte + 9));
  SetDlgItemText(IDC_LEAN_SECT_SIZE_DISP, cs);
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
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  if (m_isvalid) {
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
      SaveItemInfo(m_hRoot, m_super.root_start, root_size, 0, FALSE);
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
    }
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
    name.Empty();
    for (int i=0; i<cur->name_len; i++)
      name += cur->name[i];
    
    IsDot = ((name == ".") || (name == ".."));
    
    switch (cur->type) {
      case LEAN_FT_DIR:  // File type: directory
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        sub = (struct S_LEAN_DIRENTRY *) ReadFile(cur->inode, &filesize);
        SaveItemInfo(hItem, cur->inode, filesize, (DWORD) ((BYTE *) cur - (BYTE *) root), !IsDot);
        if (!IsDot && sub) {
          ParseDir(sub, filesize, hItem);
          free(sub);
        }
        break;
      case LEAN_FT_REG: // File type: regular file
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, cur->inode, 0, (DWORD) ((BYTE *) cur - (BYTE *) root), TRUE);
        break;
      case LEAN_FT_LNK: // File type: symbolic link
        name += " (Link)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FORK, IMAGE_FORKED, IMAGE_FORKED, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, cur->inode, 0, (DWORD) ((BYTE *) cur - (BYTE *) root), FALSE);
        break;
      case LEAN_FT_FRK: // File type: fork
        name += " (Fork)";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FORK, IMAGE_FORKED, IMAGE_FORKED, parent);
        if (hItem == NULL) { m_too_many = TRUE; return; }
        SaveItemInfo(hItem, cur->inode, 0, (DWORD) ((BYTE *) cur - (BYTE *) root), FALSE);
        break;
      case LEAN_FT_MT:  // File type: Empty
        break;
    }
    
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
  }
}

void *CLean::ReadFile(DWORD64 lba, DWORD64 *Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *buffer = NULL;
  struct S_LEAN_INODE *inode;
  BYTE *ptr;
  unsigned i;
  CString cs;
  
  inode = (struct S_LEAN_INODE *) calloc(MAX_SECT_SIZE, 1);
  dlg->ReadFromFile(inode, m_lba + lba, 1, FALSE);
  
  // check to make sure the inode is valid
  if (!ValidInode(inode)) {
    cs.Format("Read: Found invalid Inode at %I64i", lba);
    AfxMessageBox(cs);
    free(inode);
    return NULL;
  }
  
  if (Size) *Size = inode->file_size;
  buffer = (struct S_LEAN_DIRENTRY *) malloc((DWORD) inode->sector_count * dlg->m_sect_size);
  ptr = (BYTE *) buffer;
  
  // does the file start in the inode?
  if (!(inode->attributes & LEAN_ATTR_EAS_IN_INODE)) {
    memcpy(ptr, (BYTE *) inode + sizeof(struct S_LEAN_INODE), (dlg->m_sect_size - sizeof(struct S_LEAN_INODE)));
    ptr += (dlg->m_sect_size - sizeof(struct S_LEAN_INODE));
  }
  
  struct S_LEAN_SECTORS extents;
  ReadFileExtents(&extents, inode->extent_start[0]);
  if (extents.extent_count > 0) {
    // The first write needs to skip the inode
    if (extents.extent_size[0] > 1) {
      dlg->ReadFromFile(ptr, m_lba + extents.extent_start[0] + 1, extents.extent_size[0] - 1, FALSE);
      ptr += ((extents.extent_size[0] - 1) * dlg->m_sect_size);
    }
    // do the remaining extents (if any)
    for (i=1; i<extents.extent_count; i++) {
      // TODO: make sure we don't read past end of buffer
      if (extents.extent_size[i] > 0) {
        dlg->ReadFromFile(ptr, m_lba + extents.extent_start[i], extents.extent_size[i], FALSE);
        ptr += (extents.extent_size[i] * dlg->m_sect_size);
      }
    }
  }
  FreeExtentBuffer(&extents);
  
  free(inode);
  return buffer;
}

void CLean::WriteFile(void *buffer, const struct S_LEAN_SECTORS *extents, DWORD64 Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode;
  BYTE *ptr = (BYTE *) buffer;
  unsigned i;
  
  inode = (struct S_LEAN_INODE *) calloc(MAX_SECT_SIZE, 1);
  dlg->ReadFromFile(inode, m_lba + extents->extent_start[0], 1, FALSE);
  
  // check to make sure the inode is valid
  if (!ValidInode(inode)) {
    CString cs;
    cs.Format("Write: Found invalid Inode at %I64i", extents->extent_start[0]);
    AfxMessageBox(cs);
    free(inode);
    return;
  }
  
  // update the inode (just incase we need to)
  inode->file_size = Size;
  WriteFileExtents(extents, inode);
  
  // does the file start in the inode?
  if (!(inode->attributes & LEAN_ATTR_EAS_IN_INODE)) {
    memcpy((BYTE *) inode + sizeof(struct S_LEAN_INODE), ptr, (dlg->m_sect_size - sizeof(struct S_LEAN_INODE)));
    ptr += (dlg->m_sect_size - sizeof(struct S_LEAN_INODE));
  }
  
  if (extents->extent_count > 0) {
    // The first write needs to skip the inode
    if (extents->extent_size[0] > 1) {
      dlg->WriteToFile(ptr, m_lba + extents->extent_start[0] + 1, extents->extent_size[0] - 1, FALSE);
      ptr += ((extents->extent_size[0] - 1) * dlg->m_sect_size);
    }
    // do the remaining extents (if any)
    for (i=1; i<extents->extent_count; i++) {
      // TODO: make sure we don't write past end of buffer
      if (extents->extent_size[i] > 0) {
        dlg->WriteToFile(ptr, m_lba + extents->extent_start[i], extents->extent_size[i], FALSE);
        ptr += (extents->extent_size[i] * dlg->m_sect_size);
      }
    }
  }
  
  // update the inode's Timestamps
  CTime time = CTime::GetCurrentTime();
  inode->acc_time = 
  inode->mod_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  
  // update the inode's check sum and write it back
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE);
  dlg->WriteToFile(inode, m_lba + extents->extent_start[0], 1, FALSE);
  
  free(inode);
}

void CLean::ZeroExtent(DWORD64 ExtentStart, DWORD ExtentSize) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *zero = calloc(dlg->m_sect_size, 1);

  for (DWORD i=0; i<ExtentSize; i++)
    dlg->WriteToFile(zero, m_lba + ExtentStart, 1, FALSE);

  free(zero);
}

// http://freedos-32.sourceforge.net/lean/specification.php
// A driver must use the magic, checksum and primarySuper fields of the superblock to identify a valid superblock.
BOOL CLean::DetectLeanFS(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) buffer;
  
  // mark the sector as "not found"
  m_super_lba = 0xFFFFFFFF;
  
  // the lean specs say that the super can be in sector 1 - 32 (zero based)
  // count is 32 when finding primary, and 1 when finding backup
  for (unsigned sector=1; sector<=32; sector++) {
    dlg->ReadFromFile(buffer, m_lba + sector, 1, FALSE);
    
    // the first entry should be 'LEAN'
    if (super->magic != LEAN_SUPER_MAGIC)
      continue;
    
    // How about the check sum
    DWORD crc = 0, *p = (DWORD *) super;
    for (unsigned i=1; i<(dlg->m_sect_size / sizeof(DWORD)); i++)
      crc = (crc << 31) + (crc >> 1) + p[i];
    if (crc != super->checksum)
      continue;
    
    // check the primarySuper field.  It should == sector
    if (super->primary_super != (DWORD64) sector)
      continue;
    
    // We make a few more checks along the way, here.
    
    // the log2 entry should be at least 12 and not more than 31
    if ((super->log_sectors_per_band < 12) || (super->log_sectors_per_band > 31))
      continue;
    
    // all but bits 1:0 of state should be zero
    if (super->state & ~0x3)
      continue;
    
    // must be at least version 0.6
    if (super->fs_version < 0x0006)
      continue;
    
    // else we have a valid LEAN FS super
    m_super_lba = sector;
    memcpy(&m_super, super, sizeof(struct S_LEAN_SUPER));
    return TRUE;
  }
  
  // if we checked lba 1 - 32 and didn't find anything, no lean fs found
  return FALSE;
}

DWORD CLean::LeanCalcCRC(const void *data, unsigned count) {
  DWORD crc = 0;
  DWORD *p = (DWORD *) data;
  
  count /= 4;
  for (unsigned i=1; i<count; i++)
    crc = (crc << 31) + (crc >> 1) + p[i];
  
  return crc;
}

BOOL CLean::ValidInode(const struct S_LEAN_INODE *inode) {
  
  // check to see if inode->magic = 'NODE';
  if (inode->magic != LEAN_INODE_MAGIC)
    return FALSE;
  
  // check to see if crc is correct
  if (inode->checksum != LeanCalcCRC(inode, LEAN_INODE_SIZE))
    return FALSE;
  
  // if links_count == 0, file should have been deleted
  if (inode->links_count == 0)
    return FALSE;
  
  return TRUE;  
}

BOOL CLean::ValidIndirect(const struct S_LEAN_INDIRECT *indirect) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  // check to see if indirect->magic = 'INDX';
  if (indirect->magic != LEAN_INDIRECT_MAGIC)
    return FALSE;
  
  // check to see if crc is correct
  if (indirect->checksum != LeanCalcCRC(indirect, dlg->m_sect_size))
    return FALSE;
  
  // other checks here

  return TRUE;
}

void CLean::SendToDialog(struct S_LEAN_SUPER *super) {
  
  m_crc.Format("0x%08X", super->checksum);
  m_magic.Format("0x%08X", super->magic);
  m_version.Format("0x%04X", super->fs_version);
  m_pre_alloc.Format("%i", super->pre_alloc_count);
  m_sect_band.Format("%i", super->log_sectors_per_band);
  m_cur_state.Format("0x%08X", super->state);
  GUID_Format(m_guid, &super->guid);
  m_label.Format("%s", super->volume_label);
  m_sect_count.Format("%I64i", super->sector_count);
  m_free_sectors.Format("%I64i", super->free_sector_count);
  m_primary_lba.Format("%I64i", super->primary_super);
  m_backup_lba.Format("%I64i", super->backup_super);
  m_bitmap_lba.Format("%I64i", super->bitmap_start);
  m_root_lba.Format("%I64i", super->root_start);
  m_bad_lba.Format("%I64i", super->bad_start);
  m_sect_size.Format("%i", super->log_sector_size);
  m_journal_lba.Format("%I64i", super->journal);
  
  UpdateData(FALSE); // send to Dialog
  
  OnChangeLeanVersion();
  OnChangeLeanPreAlloc();
  OnChangeLeanSectBand();
  OnChangeLeanSectSize();
}

void CLean::ReceiveFromDialog(struct S_LEAN_SUPER *super) {
  UpdateData(TRUE); // receive from Dialog
  
  super->checksum = convert32(m_crc);
  super->magic = convert32(m_magic);
  super->fs_version = convert16(m_version);
  super->pre_alloc_count = convert8(m_pre_alloc);
  super->log_sectors_per_band = convert8(m_sect_band);
  super->state = convert32(m_cur_state);
  GUID_Retrieve(m_guid, &super->guid);
  strcpy((char *) super->volume_label, m_label);
  super->sector_count = convert64(m_sect_count);
  super->free_sector_count = convert64(m_free_sectors);
  super->primary_super = convert64(m_primary_lba);
  super->backup_super = convert64(m_backup_lba);
  super->bitmap_start = convert64(m_bitmap_lba);
  super->root_start = convert64(m_root_lba);
  super->bad_start = convert64(m_bad_lba);
  super->log_sector_size = convert8(m_sect_size);
  super->journal = convert64(m_journal_lba);
}

void CLean::SaveItemInfo(HTREEITEM hItem, DWORD64 Inode, DWORD64 FileSize, DWORD Offset, BOOL CanCopy) {
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->Inode = Inode;
    items->FileSize = FileSize;
    items->Offset = Offset;
    items->CanCopy = CanCopy;
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

// Inode = starting inode of folder to insert to
// csName = name of file to insert
// csPath = path on host of file to insert
void CLean::InsertFile(DWORD64 Inode, CString csName, CString csPath) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_SECTORS extents;
  void *buffer;
  DWORD64 Size, TotSize;
  CFile file;
  DWORD Attrib = IsDlgButtonChecked(IDC_EAS_IN_INODE) ? LEAN_ATTR_EAS_IN_INODE : 0;  // Allow user to specify if EAS_IN_INODE or not on new creation.
  
  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return;
  }
  Size = file.GetLength();  // TODO: use long version
  buffer = malloc((size_t) Size + dlg->m_sect_size);  // to prevent buffer overrun in WriteFile()
  file.Read(buffer, (UINT) Size);
  file.Close();
  
  // allocate the extents for it, returning a "struct S_LEAN_SECTORS"
  AllocateExtentBuffer(&extents, LEAN_DEFAULT_COUNT);
  // Calculate how many actual bytes we will use
  TotSize = Size + ((Attrib & LEAN_ATTR_EAS_IN_INODE) ? dlg->m_sect_size : sizeof(struct S_LEAN_INODE));
  if (AppendToExtents(&extents, TotSize, 0, TRUE) == -1) {
    FreeExtentBuffer(&extents);
    free(buffer);
    return;
  }
  
  // create a root entry in the folder
  int r = AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_REG);
  if (r == -1) {
    FreeExtentBuffer(&extents);
    free(buffer);
    return;
  }
  
  // did it return "not enough room"?
  if (r == -2) {
    // append room to the end of the directory
    if (AppendToDir(Inode, 4096) != 1) {
      FreeExtentBuffer(&extents);
      free(buffer);
      return;
    }
    // now try it again
    if (AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_REG) != 1) {
      FreeExtentBuffer(&extents);
      free(buffer);
      return;
    }
  }
  
  // create an Inode at extents.extent_start[0]
  BuildInode(&extents, Size, LEAN_ATTR_IFREG | Attrib);
  
  // copy the file to our system
  WriteFile(buffer, &extents, Size);
  
  // free the buffers
  FreeExtentBuffer(&extents);
  free(buffer);
}

// Inode = starting Inode of folder to insert in to
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CLean::InsertFolder(DWORD64 Inode, CString csName, CString csPath) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_SECTORS extents;
  const unsigned size = 8192;  // must be a multiple of sect_size
  char szPath[MAX_PATH];
  DWORD Attrib = IsDlgButtonChecked(IDC_EAS_IN_INODE) ? LEAN_ATTR_EAS_IN_INODE : 0;  // Allow user to specify if EAS_IN_INODE or not on new creation.
  
  // allocate the extents for the folder, returning a "struct S_LEAN_SECTORS"
  AllocateExtentBuffer(&extents, LEAN_INODE_EXTENT_CNT);
  if (AppendToExtents(&extents, size, 0, TRUE) == -1) {
    FreeExtentBuffer(&extents);
    return;
  }
  
  // create a root entry in the folder for the folder
  int r = AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_DIR);
  if (r == -1) {
    FreeExtentBuffer(&extents);
    return;
  }
  if (r == -2) {
    // append room to the end of the directory
    if (AppendToDir(Inode, 4096) != 1) {
      FreeExtentBuffer(&extents);
      return;
    }
    if (AllocateRoot(csName, Inode, extents.extent_start[0], LEAN_FT_DIR) != 1) {
      FreeExtentBuffer(&extents);
      return;
    }
  }
  
  // create an Inode at extents.extent_start[0]
  BuildInode(&extents, size, LEAN_ATTR_IFDIR | Attrib);
  
  // create the directory in the image
  void *buffer = malloc(size + dlg->m_sect_size);  // to prevent buffer overrun in WriteFile()
  CreateEmptyDir(buffer, size);
  WriteFile(buffer, &extents, size);
  free(buffer);
  
  // add the . and .. entries
  CString csDots = ".";
  if (AllocateRoot(csDots, extents.extent_start[0], extents.extent_start[0], LEAN_FT_DIR) != 1) {
    FreeExtentBuffer(&extents);
    return;
  }
  csDots = "..";
  if (AllocateRoot(csDots, extents.extent_start[0], Inode, LEAN_FT_DIR) != 1) {
    FreeExtentBuffer(&extents);
    return;
  }
  
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
      InsertFolder(extents.extent_start[0], filefind.GetFileName(), filefind.GetFilePath());
    else
      InsertFile(extents.extent_start[0], filefind.GetFileName(), filefind.GetFilePath());
  }
  filefind.Close();
  
  // free the extent buffer
  FreeExtentBuffer(&extents);
  
  // restore the current directory
  SetCurrentDirectory(szPath);
}

// the user change the status of the "Show Deleted" Check box
void CLean::OnShowDeleted() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANShowDel", m_show_del = IsDlgButtonChecked(IDC_SHOW_DEL));
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CLean::OnEAsInInode() {
  AfxGetApp()->WriteProfileInt("Settings", "LEANEAsInInode", m_ESs_in_Inode = IsDlgButtonChecked(IDC_EAS_IN_INODE));
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
  
  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CLean::DeleteFolder(HTREEITEM hItem) {
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

void CLean::DeleteFile(HTREEITEM hItem) {
  struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  struct S_LEAN_DIRENTRY *root, *cur;
  struct S_LEAN_SECTORS extents;
  DWORD64 RootSize;
  DWORD Offset;
  
  if (items == NULL)
    return;
  
  if (m_del_clear) {
    ReadFileExtents(&extents, items->Inode);
    for (unsigned int i=0; i<extents.extent_count; i++)
      ZeroExtent(extents.extent_start[i], extents.extent_size[i]);
    FreeExtentBuffer(&extents);
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
    ReadFileExtents(&extents, items->Inode);
    WriteFile(root, &extents, RootSize);
    FreeExtentBuffer(&extents);
    free(root);
  }
}

void CLean::OnSearch() {
  m_dir_tree.Search();
}

// S_LEAN_SECTORS is a list of extents.  Does not know about indirects or anything.
//  is simply a list of extents.  It is up to the other functions to read/write the actual extents to the disk
void CLean::FreeExtents(const struct S_LEAN_SECTORS *Extents) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bitmap = NULL;
  DWORD last_band = 0xFFFFFFFF, band;
  DWORD64 lba, bitmap_lba = 0;
  unsigned int i, extent;
  unsigned free_count = 0;  // count of sectors free'd
  
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_sectors_per_band); // sectors per band
  const unsigned bitmap_size = (unsigned) (band_size >> 12);     // sectors per bitmap

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * dlg->m_sect_size);

  for (extent=0; extent<Extents->extent_count; extent++) {
    for (i=0; i<Extents->extent_size[extent]; i++) {
      lba = Extents->extent_start[extent] + i;
      band = (DWORD) (lba >> m_super.log_sectors_per_band);
      // are we in the same band as the last one, or do we need to calculate it, load it
      if (band != last_band) {
        // need to write the last one before we read a new one?
        if (last_band != 0xFFFFFFFF)
          dlg->WriteToFile(bitmap, m_lba + bitmap_lba, bitmap_size, FALSE);
        last_band = band;
        bitmap_lba = (band==0) ? m_super.bitmap_start : (band * band_size);
        dlg->ReadFromFile(bitmap, m_lba + bitmap_lba, bitmap_size, FALSE);
      }
      // clear the bit in this band
      // calculate sector in this band, byte, and bit within bitmap for 'sector'
      unsigned sector_in_this_band = (unsigned) (lba & ((1 << m_super.log_sectors_per_band) - 1));
      unsigned byte = sector_in_this_band / 8;
      unsigned bit = sector_in_this_band % 8;
      bitmap[byte] &= ~(1 << bit);
      free_count++;
    }
  }

  // do we need to write the last modified band?
  if (last_band != 0xFFFFFFFF)
    dlg->WriteToFile(bitmap, m_lba + bitmap_lba, bitmap_size, FALSE);

  // we also need to update the FreeCount in the super.
  m_super.free_sector_count += free_count;
  // TODO: update super:CRC and write the super to the disk

  // free the buffer
  free(bitmap);
}

// returns a free sector (or zero if none found)
DWORD64 CLean::GetFreeSector(DWORD64 Start, BOOL MarkIt) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer;
  DWORD64 bitmap_lba;
  
  int j;
  unsigned i, pos;
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_sectors_per_band); // sectors per band
  const unsigned bitmap_size = (unsigned) (band_size >> 12);     // sectors per bitmap
  const unsigned bytes_bitmap = bitmap_size * dlg->m_sect_size;
  const unsigned tot_bands = (unsigned) ((m_super.sector_count + (band_size - 1)) / band_size);
  buffer = (BYTE *) malloc(bitmap_size * dlg->m_sect_size);
  DWORD64 Lba = 0;
  
  // TODO: Start  // start with specified sector.  Will have to skip to next band and whatnot
  
  for (i=0; i<tot_bands; i++) {
    // read in a bitmap
    bitmap_lba = (i==0) ? m_super.bitmap_start : (band_size * i);
    dlg->ReadFromFile(buffer, m_lba + bitmap_lba, bitmap_size, FALSE);
    pos = 0;
    
    while (pos < bytes_bitmap) {
      for (j=0; j<8; j++) {
        if ((buffer[pos] & (1<<j)) == 0) {
          if (MarkIt) {
            buffer[pos] |= (1<<j);  // mark it
            dlg->WriteToFile(buffer, m_lba + bitmap_lba, bitmap_size, FALSE);
            // we also need to update the FreeCount in the super.
            m_super.free_sector_count--;
            // TODO: update super:CRC and write the super to the disk
          }
          free(buffer);
          return Lba;
        }
        Lba++;
      }
      pos++;
    }
  }
  
  free(buffer);
  return 0;
}

// Mark a Sector as used/free in the corresponding band's bitmap
void CLean::MarkSector(DWORD64 Sector, BOOL MarkIt) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bitmap = NULL;
  DWORD64 bitmap_lba;
  DWORD band;
  
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_sectors_per_band); // sectors per band
  const unsigned bitmap_size = (unsigned) (band_size >> 12);     // sectors per bitmap

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * dlg->m_sect_size);
  
  band = (DWORD) (Sector >> m_super.log_sectors_per_band);
  bitmap_lba = (band==0) ? m_super.bitmap_start : (band * band_size);
  dlg->ReadFromFile(bitmap, m_lba + bitmap_lba, bitmap_size, FALSE);

  // clear/set the bit in this band
  // calculate sector in this band, byte, and bit within bitmap for 'sector'
  unsigned sector_in_this_band = (unsigned) (Sector & ((1 << m_super.log_sectors_per_band) - 1));
  unsigned byte = sector_in_this_band / 8;
  unsigned bit = sector_in_this_band % 8;
  if (MarkIt)
    bitmap[byte] |= (1 << bit);
  else
    bitmap[byte] &= ~(1 << bit);
  
  dlg->WriteToFile(bitmap, m_lba + bitmap_lba, bitmap_size, FALSE);
  
  free(bitmap);
}

// allocate sectors, building extents for a Size count of sectors
// S_LEAN_SECTORS is a list of extents.  Does not know about indirects or anything.
//  is simply a list of extents.  It is up to the other functions to read/write the actual extents to the disk
int CLean::AppendToExtents(struct S_LEAN_SECTORS *extents, DWORD64 Size, DWORD64 Start, BOOL MarkIt) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  unsigned i, cnt = extents->extent_count;
  unsigned count = (unsigned) ((Size + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  DWORD64 Lba = Start;

  for (i=0; i<count; i++) {
    Lba = GetFreeSector(Lba, MarkIt);
    if (extents->extent_size[cnt] == 0) {
      extents->extent_start[cnt] = Lba;
      extents->extent_size[cnt]++;
    } else if (Lba == (extents->extent_start[cnt] + extents->extent_size[cnt])) {
      extents->extent_size[cnt]++;
      // TODO: if Extents->extent_size[cnt] > DWORD sized then cnt++, etc
    } else {
      cnt++;
      if (cnt >= extents->allocated_count)
        ReAllocateExtentBuffer(extents, extents->allocated_count + LEAN_DEFAULT_COUNT);
      extents->extent_start[cnt] = Lba;
      extents->extent_size[cnt] = 1;
    }
  }
  
  // update the count of extents used
  extents->extent_count = cnt + 1;

  // return this count
  return extents->extent_count;
}

// get the extents of the file
int CLean::ReadFileExtents(struct S_LEAN_SECTORS *extents, DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE inode_buffer[MAX_SECT_SIZE];
  BYTE indirect_buffer[MAX_SECT_SIZE];
  struct S_LEAN_INODE *inode;
  struct S_LEAN_INDIRECT *indirect;
  DWORD64 ind_lba;
  unsigned i;
  
  // initialize our pointers
  inode = (struct S_LEAN_INODE *) inode_buffer;
  indirect = (struct S_LEAN_INDIRECT *) indirect_buffer;
  
  // read the inode
  dlg->ReadFromFile(inode, m_lba + Inode, 1, FALSE);
  if (!ValidInode(inode))
    return -1;
  
  // allocate at least LEAN_INODE_EXTENT_CNT extents to hold the direct extents
  AllocateExtentBuffer(extents, LEAN_INODE_EXTENT_CNT);
  
  // do the direct extents first
  for (i=0; i<inode->extent_count; i++) {
    extents->extent_start[extents->extent_count] = inode->extent_start[i];
    extents->extent_size[extents->extent_count] = inode->extent_size[i];
    extents->extent_count++;
  }
  
  // are there any indirect extents?
  if (inode->first_indirect > 0) {
    ind_lba = inode->first_indirect;
    while (ind_lba > 0) {
      // read in the indirect sector
      dlg->ReadFromFile(indirect, m_lba + ind_lba, 1, FALSE);
      if (!ValidIndirect(indirect))
        break;

      // increase the size of our list
      ReAllocateExtentBuffer(extents, extents->extent_count + indirect->extent_count);

      // retrieve the extents in this indirect block
      for (i=0; i<indirect->extent_count; i++) {
        extents->extent_start[extents->extent_count] = indirect->extent_start[i];
        extents->extent_size[extents->extent_count] = indirect->extent_size[i];
        extents->extent_count++;
      }

      // get next indirect (0 == no more)
      ind_lba = indirect->next_indirect;
    }
  }
  
  return extents->extent_count;
}

// write the extents of the file to the file
// The count of extents could actually extend past the current inode count.
//  therefore, we add indirects if needed
int CLean::WriteFileExtents(const struct S_LEAN_SECTORS *extents, struct S_LEAN_INODE *inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE indirect_buffer[MAX_SECT_SIZE];
  struct S_LEAN_INDIRECT *indirect;
  DWORD64 prev_lba, this_lba, next_lba, remaining;
  unsigned i, count = 0, max_extents;
  
  // initialize our pointer
  indirect = (struct S_LEAN_INDIRECT *) indirect_buffer;

  // do the direct extents first
  for (i=0; (i<LEAN_INODE_EXTENT_CNT) && (count<extents->extent_count); i++) {
    inode->extent_start[i] = extents->extent_start[count];
    inode->extent_size[i] = extents->extent_size[count];
    count++;
  }

  this_lba = 0;
  prev_lba = 0;
  remaining = 0;
  BOOL used = FALSE;
  
  inode->indirect_count = 0;

  // are there any indirect extents needed?
  if (count < extents->extent_count) {
    if (inode->first_indirect > 0) {
      this_lba = inode->first_indirect;
      used = TRUE;
    } else {
      this_lba = GetFreeSector(0, TRUE);
      inode->first_indirect = this_lba;
      used = FALSE;
    }
    
    indirect->sector_count = 0;
    
    // calculate the max count of extents we can have per this sector size
    if (dlg->m_sect_size == 1024)
      max_extents = LEAN_INDIRECT_EXTENT_CNT_1024;
    else if (dlg->m_sect_size == 2048)
      max_extents = LEAN_INDIRECT_EXTENT_CNT_2048;
    else if (dlg->m_sect_size == 4096)
      max_extents = LEAN_INDIRECT_EXTENT_CNT_4096;
    else
      max_extents = LEAN_INDIRECT_EXTENT_CNT_512;
    
    while (count < extents->extent_count) {
      if (!used) {
        // initialize the indirect block
        memset(indirect, 0, MAX_SECT_SIZE);
        indirect->magic = LEAN_INDIRECT_MAGIC;
        indirect->inode = extents->extent_start[0];
        indirect->this_sector = this_lba;
        indirect->prev_indirect = prev_lba;
        indirect->next_indirect = 0;
      } else
        dlg->ReadFromFile(indirect_buffer, m_lba + this_lba, 1, FALSE);
      
      // write to the indirect extents
      for (i=0; (i<max_extents) && (count<extents->extent_count); i++) {
        indirect->extent_start[i] = extents->extent_start[count];
        indirect->extent_size[i] = extents->extent_size[count];
        indirect->sector_count += extents->extent_size[count];
        count++;
      }
      indirect->extent_count = i;
      inode->indirect_count++;

      // will there be more?
      if (count < extents->extent_count) {
        if (used && (indirect->next_indirect > 0)) {
          next_lba = indirect->next_indirect;
        } else {
          next_lba = GetFreeSector(0, TRUE);
          indirect->next_indirect = next_lba;
          used = FALSE;
        }
      } else {
        next_lba = this_lba;
        if (used)
          remaining = indirect->next_indirect;
        indirect->next_indirect = 0;
      }
      indirect->checksum = LeanCalcCRC(indirect, dlg->m_sect_size);
      
      // write the indirect back
      dlg->WriteToFile(indirect_buffer, m_lba + this_lba, 1, FALSE);
      
      // initialize for next round
      prev_lba = this_lba;
      this_lba = next_lba;
    }
  }
  
  // if we don't need any more indirects, yet there are some allocated,
  //  we need to free each remaining indirect block
  while (remaining > 0) {
    MarkSector(remaining, FALSE);
    dlg->ReadFromFile(indirect_buffer, m_lba + remaining, 1, FALSE);
    // TODO: Test indirect
    remaining = indirect->next_indirect;
  }
  
  // update the inode structure
  inode->last_indirect = this_lba;
  inode->extent_count = (count > LEAN_INODE_EXTENT_CNT) ? LEAN_INODE_EXTENT_CNT : (BYTE) count;
  CTime time = CTime::GetCurrentTime();
  inode->sch_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE);

  return extents->extent_count;
}

// read in the current root (Inode) and add to it
// returns  1 on good return
//         -1 on error
//         -2 on not enough room in root
int CLean::AllocateRoot(CString csName, DWORD64 Inode, DWORD64 Start, BYTE Attrib) {
  struct S_LEAN_DIRENTRY *root, *cur, *next;
  struct S_LEAN_SECTORS extents;
  DWORD64 root_size;
  BYTE byte;
  
  root = (struct S_LEAN_DIRENTRY *) ReadFile(Inode, &root_size);
  if (root) {
    cur = root;
    ReadFileExtents(&extents, Inode);
    while ((unsigned char *) cur < ((unsigned char *) root + root_size)) {
      if (cur->rec_len == 0)
        break;
      if (cur->type == LEAN_FT_MT) {
        WORD len = csName.GetLength();
        if (((cur->rec_len * 16) - 12) >= len) {
          // see if there is enough room to divide this record in to two
          if (cur->rec_len > ((((len + 12) + 15) / 16) + 1)) {  // +1 is only 1 extra???
            byte = cur->rec_len;
            cur->rec_len = ((len + 12 + 15) / 16);
            next = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
            next->rec_len = byte - cur->rec_len;
            next->inode = 0;
            next->name_len = 0;
            next->type = LEAN_FT_MT;
          }
          cur->name_len = len;
          cur->inode = Start;
          cur->type = Attrib;
          memcpy(cur->name, csName, len);
          // write it back to the root
          WriteFile(root, &extents, root_size);
          FreeExtentBuffer(&extents);
          free(root);
          return 1;
        }
      }
      cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
    }
    free(root);
    FreeExtentBuffer(&extents);
    return -2;
  }
  AfxMessageBox("Error adding to directory");
  return -1;
}

// appends to the end of a directory either by increasing FileSize due to
//  the fact that we already have sectors allocated, or
// allocates more sectors and then increases FileSize
// Clears the newly allocated area and places empty DIR entries in it.
// Size should be a multiple of sect_size
// returns 1 if successful
int CLean::AppendToDir(DWORD64 Inode, DWORD Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode;
  
  // make sure Size is a multiple of sect_size
  Size = (Size + (dlg->m_sect_size - 1)) & ~(dlg->m_sect_size - 1);
  
  inode = (struct S_LEAN_INODE *) calloc(MAX_SECT_SIZE, 1);
  dlg->ReadFromFile(inode, m_lba + Inode, 1, FALSE);
  
  // check to make sure the inode is valid
  if (!ValidInode(inode)) {
    CString cs;
    cs.Format("Append: Found invalid Inode at %I64i", Inode);
    AfxMessageBox(cs);
    free(inode);
    return -1;
  }
  
  // get the file's extents
  struct S_LEAN_SECTORS extents;
  ReadFileExtents(&extents, Inode);
  
  // first see if we already have the sectors allocated for this amount
  // if not, we will have to allocate some.
  if (((inode->sector_count * dlg->m_sect_size) - inode->file_size) < Size) {
    DWORD64 Start = 0;
    if (extents.extent_count > 0)
      Start = extents.extent_start[extents.extent_count - 1] + extents.extent_size[extents.extent_count - 1];
    AppendToExtents(&extents, Size, Start, TRUE);
    // copy them back to the inode
    WriteFileExtents(&extents, inode);
    dlg->WriteToFile(inode, m_lba + Inode, 1, FALSE);
  } 
  
  // Read in the new set of sectors
  DWORD64 root_size;
  struct S_LEAN_DIRENTRY *root, *cur;
  root = (struct S_LEAN_DIRENTRY *) ReadFile(Inode, &root_size);
  if (root) {
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + root_size);
    CreateEmptyDir(cur, Size);
    WriteFile(root, &extents, root_size + Size);
    free(root);
  } else {
    free(inode);
    return -1;
  }
  free(inode);
  FreeExtentBuffer(&extents);
  
  return 1;
}

// create an empty directory listing in passed buffer
// size must be a multiple of sect_size
void CLean::CreateEmptyDir(void *buffer, DWORD Size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  unsigned sect_size = dlg->m_sect_size;
  struct S_LEAN_DIRENTRY *root;
  
  memset(buffer, 0, Size);
  
  sect_size -= LEAN_INODE_SIZE; // the first one is sect size - size of inode since we start in the inode
  root = (struct S_LEAN_DIRENTRY *) buffer;
  while ((BYTE *) root < ((BYTE *) buffer + Size)) {
    root->rec_len = (sect_size / 16);
    Size -= sect_size;
    root = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + sect_size);
    sect_size = dlg->m_sect_size;  // restore the sector size
  }
}

void CLean::BuildInode(struct S_LEAN_SECTORS *extents, DWORD64 Size, DWORD Attrib) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_INODE *inode;
  
  CTime time = CTime::GetCurrentTime();
  const INT64 now = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  
  inode = (struct S_LEAN_INODE *) calloc(MAX_SECT_SIZE, 1);
  
  inode->magic = LEAN_INODE_MAGIC;
  inode->extent_count = (extents->extent_count > LEAN_INODE_EXTENT_CNT) ? LEAN_INODE_EXTENT_CNT : (BYTE) extents->extent_count;
  inode->indirect_count = 0;
  inode->links_count = 1;
  inode->attributes = Attrib;
  inode->file_size = Size;
  if (Attrib & LEAN_ATTR_EAS_IN_INODE)
    inode->sector_count = 1 + ((Size + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  else {
    DWORD64 s = Size - (dlg->m_sect_size - (sizeof(struct S_LEAN_INODE)));
    inode->sector_count = 1 + ((s + (dlg->m_sect_size - 1)) / dlg->m_sect_size);
  }
  inode->acc_time = now;
  inode->sch_time = now;
  inode->mod_time = now;
  inode->cre_time = now;
  inode->first_indirect = 0;
  inode->last_indirect = 0;
  inode->fork = 0;

  WriteFileExtents(extents, inode);  // most write at least the first 6 so the checksum is correct
  dlg->WriteToFile(inode, m_lba + extents->extent_start[0], 1, FALSE);
  free(inode);
}

void CLean::DeleteInode(DWORD64 Inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) buffer;
  struct S_LEAN_SECTORS extents;
  
  dlg->ReadFromFile(buffer, m_lba + Inode, 1, FALSE);

  if (inode->links_count > 1) {
    inode->links_count--;
    inode->checksum = LeanCalcCRC(inode, LEAN_INODE_SIZE);
    dlg->WriteToFile(buffer, m_lba + Inode, 1, FALSE);
  } else {
    if (inode->fork)
      DeleteInode(inode->fork);
    
    // free the extents
    ReadFileExtents(&extents, Inode);
    FreeExtents(&extents);
    FreeExtentBuffer(&extents);
    
    // also clear the inode or parts of it???
    memset(inode, 0, sizeof(struct S_LEAN_INODE));
    dlg->WriteToFile(buffer, m_lba + Inode, 1, FALSE);
  }
}

void CLean::OnLeanEntry() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CLeanEntry LeanEntry;
  BYTE buffer[MAX_SECT_SIZE];
  struct S_LEAN_INODE *inode_buff = (struct S_LEAN_INODE *) buffer;
  
  if (hItem) {
    struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      // read the inode
      dlg->ReadFromFile(buffer, m_lba + items->Inode, 1, FALSE);
      memcpy(&LeanEntry.m_inode, buffer, sizeof(struct S_LEAN_INODE));
      LeanEntry.m_hItem = hItem;
      LeanEntry.m_parent = this;
      LeanEntry.m_inode_num = items->Inode;
      if (LeanEntry.DoModal() == IDOK) { // apply button pressed?
      //  // must read it back in so we don't "destroy" the EA's we might have updated in LeanEntry
      //  dlg->ReadFromFile(buffer, m_lba + items->Inode, 1, FALSE);
      //  memcpy(buffer, &LeanEntry.m_inode, sizeof(struct S_LEAN_INODE));
      //  //inode_buff->checksum = LeanCalcCRC(buffer, LEAN_INODE_SIZE);
      //  dlg->WriteToFile(buffer, m_lba + items->Inode, 1, FALSE);
      }
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
      struct S_LEAN_SECTORS extents;
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
  BYTE buffer[MAX_SECT_SIZE];
  CLeanEntry LeanEntry;
  
  // read the inode
  dlg->ReadFromFile(buffer, m_lba + m_super.journal, 1, FALSE);
  memcpy(&LeanEntry.m_inode, buffer, sizeof(struct S_LEAN_INODE));
  LeanEntry.m_hItem = NULL;
  LeanEntry.m_parent = this;
  LeanEntry.m_inode_num = m_super.journal;
  if (LeanEntry.DoModal() == IDOK) { // apply button pressed?
    // must read it back in so we don't "destroy" the EA's we might have updated in LeanEntry
    dlg->ReadFromFile(buffer, m_lba + m_super.journal, 1, FALSE);
    memcpy(buffer, &LeanEntry.m_inode, sizeof(struct S_LEAN_INODE));
    dlg->WriteToFile(buffer, m_lba + m_super.journal, 1, FALSE);
  }
}

void CLean::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    for (DWORD64 lba=0; lba<m_size; lba++)
      dlg->WriteToFile(buffer, m_lba + lba, 1, FALSE);
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}

// allocate memory for the extents
void CLean::AllocateExtentBuffer(struct S_LEAN_SECTORS *extents, const unsigned count) {
  extents->was_error = FALSE;
  extents->extent_count = 0;
  extents->allocated_count = count;
  extents->extent_start = (DWORD64 *) calloc(count, sizeof(DWORD64));
  extents->extent_size = (DWORD *) calloc(count, sizeof(DWORD));
}

// allocate memory for the extents
void CLean::ReAllocateExtentBuffer(struct S_LEAN_SECTORS *extents, const unsigned count) {
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
    ptr = calloc(count, sizeof(DWORD));
    memcpy(ptr, extents->extent_size, extents->allocated_count * sizeof(DWORD));
    free(extents->extent_size);
    extents->extent_size = (DWORD *) ptr;
  } else {
    realloc(extents->extent_start, count * sizeof(DWORD64));
    realloc(extents->extent_size, count * sizeof(DWORD));
  }
  
  // update the new size
  extents->allocated_count = count;
}

// free the memory used by the extents
void CLean::FreeExtentBuffer(struct S_LEAN_SECTORS *extents) {
  extents->extent_count = 0;
  if (extents->extent_start)
    free(extents->extent_start);
  extents->extent_start = NULL;
  if (extents->extent_size)
    free(extents->extent_size);
  extents->extent_size = NULL;
}

