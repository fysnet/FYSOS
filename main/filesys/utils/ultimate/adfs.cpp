/*
 *                             Copyright (c) 1984-2026
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

/*
 * This code only supports Type-L ADFS filesystems:
 *   Old map format, no zones, old directory format, no boot block
 *   16 sectors per track, 256 byte sectors, 2 heads, 80 tracks, (image size = 640k)
 *   The Root is at 0x200.
 * 
 * Sample image files can be downloaded from:
 *   https://8bs.com/catalogue/tbi.htm
 * The ones marked A and MA
 * 
 *  A simplified specification is at:
 *    http://www.cowsarenotpurple.co.uk/bbccomputer/native/adfs.html
 *  A detailed specification for all types of this filesytem is at:
 *    https://www.geraldholdsworth.co.uk/documents/DiscImage.pdf
 */

// ADFS.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "MyImageList.h"
#include "MyTreeCtrl.h"

#include "ADFS.h"
#include "AdfsEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CADFS property page

IMPLEMENT_DYNCREATE(CADFS, CPropertyPage)

CADFS::CADFS() : CPropertyPage(CADFS::IDD) {
  //{{AFX_DATA_INIT(CADFS)
  m_adfs_sector_count = _T("");
  m_disk_name = _T("");
  //}}AFX_DATA_INIT
  m_free_blocks = 0;
  m_dir_buffer = NULL;
  m_hard_format = FALSE;
}

CADFS::~CADFS() {
  if (m_dir_buffer)
    free(m_dir_buffer);
  m_free_blocks = 0;
}

void CADFS::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CADFS)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_ADFS_BLOCK_COUNT, m_adfs_sector_count);
  DDX_Text(pDX, IDC_ADFS_CRC0, m_adfs_crc0);
  DDX_Text(pDX, IDC_ADFS_CRC1, m_adfs_crc1);
  DDX_Text(pDX, IDC_ADFS_DISK_ID, m_disk_id);
  DDX_Text(pDX, IDC_ADFS_MAP_END, m_map_end);
  DDX_Text(pDX, IDC_ADFS_DISK_NAME, m_disk_name);
  DDX_Check(pDX, IDC_SHOW_DEL, m_show_del);
  DDX_Control(pDX, IDC_FREE_LIST, m_free_list);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CADFS, CPropertyPage)
  //{{AFX_MSG_MAP(CADFS)
  ON_WM_HELPINFO()
  ON_BN_CLICKED(IDC_ADFS_CRC0_UPDATE, OnCrc0Update)
  ON_BN_CLICKED(IDC_ADFS_CRC1_UPDATE, OnCrc1Update)
  ON_BN_CLICKED(ID_CLEAN, OnAdfsFormat)
  ON_BN_CLICKED(ID_FORMAT, OnAdfsFormat)
  ON_BN_CLICKED(ID_CHECK, OnAdfsCheck)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_COPY, OnAdfsCopy)
  ON_BN_CLICKED(ID_VIEW, OnAdfsView)
  ON_BN_CLICKED(ID_ENTRY, OnAdfsEntry)
  ON_BN_CLICKED(ID_INSERT, OnAdfsInsert)
  ON_BN_CLICKED(ID_DELETE, OnAdfsDelete)
  ON_BN_CLICKED(ID_OPTIMIZE, OnAdfsOptimize)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_APPLY, OnAdfsApply)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CADFS message handlers
BOOL CADFS::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    
    case ID_ENTRY:
      strTipText = "View/Modify Entry";
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
    case ID_OPTIMIZE:
      strTipText = "Cleans and optimizes the volume";
      break;
    case ID_SEARCH:
      strTipText = "Search for Folder/File in Partition";
      break;
    case ID_VIEW:
      strTipText = "View File in Host's default Viewer";
      break;
    case IDC_DEL_CLEAR:
      strTipText = "Zero File Contents on Delete";
      break;

    case IDC_ADFS_CRC0:
      strTipText = "Recalculate CRC for Sector 0";
      break;
    case IDC_ADFS_CRC1:
      strTipText = "Recalculate CRC for Sector 1";
      break;

    case ID_ERASE:
      strTipText = "Erase whole Partition";
      break;
    case ID_APPLY:
      strTipText = "Save modifications";
      break;
  }
  
  strncpy(pTTTA->szText, strTipText, 79);
  pTTTA->szText[79] = '\0';  // make sure it is null terminated

  return TRUE; // message was handled
}

BOOL CADFS::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_show_del = AfxGetApp()->GetProfileInt("Settings", "ADFSShowDel", FALSE);
  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "ADFSDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  EnableToolTips(TRUE);

  return TRUE;
}

BOOL CADFS::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "adfs.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CADFS::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  GetDlgItem(ID_ENTRY)->EnableWindow((hItem != NULL) && (m_dir_tree.GetParentItem(hItem) != NULL));
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) /* && (m_dir_tree.IsDir(hItem) != 0)*/ );
  GetDlgItem(ID_VIEW)->EnableWindow((hItem != NULL) && (items->Flags & ITEM_IS_FILE));
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(TRUE);
  
  *pResult = 0;
}

void CADFS::OnAdfsApply() {
  ReceiveFromDialog(&m_fs_map); // Get From Dialog
  
  // if the CRCs need updated, ask to do so
  if (m_fs_map.crc0 != CalcCRC((BYTE *) m_fs_map.sector0))
    if (AfxMessageBox("Update Sector0 Check Sum?", MB_YESNO, 0) == IDYES)
      OnCrc0Update();
  if (m_fs_map.crc1 != CalcCRC((BYTE *) m_fs_map.sector1))
    if (AfxMessageBox("Update Sector1 Check Sum?", MB_YESNO, 0) == IDYES)
      OnCrc1Update();

  ReceiveFromDialog(&m_fs_map); // Get From Dialog
  WriteSectors(&m_fs_map, 0, 2);
}

void CADFS::OnAdfsFormat() {
  int r = AfxMessageBox("This will erase the volume.\r\n"
                        "Is this what you wish to do?", MB_YESNO, NULL);
  if (r != IDYES)
    return;
  
  Format();
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CADFS::OnAdfsCheck() {
  AfxMessageBox("TODO:");
}

// returns TRUE if successful
bool CADFS::Format(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD l;
  BYTE cycle = 0x27; // TODO
  
  // we must be at least 2560 256-byte sectors
  if ((m_size * dlg->m_sect_size) < (2560 * 256)) {
    AfxMessageBox("Partition must be at least 2,560 256-byte sectors");
    return FALSE;
  }
  
  // build a new freespace map (sectors 0 and 1)
  //  and create a new map
  memset(&m_fs_map, 0, sizeof(struct S_ADFS_FS_MAP));
  PutTriplet(&m_fs_map.sector0[0], 7);
  PutTriplet(&m_fs_map.sector1[0], (2560 - 2 - 5));
  PutTriplet(&m_fs_map.sectors, 2560);
  m_fs_map.disk_id = 0xA55A;
  m_fs_map.boot_option = 0x00;
  m_fs_map.map_end = sizeof(struct S_ADFS_TRIPLET);
  m_fs_map.crc0 = CalcCRC((BYTE *) m_fs_map.sector0);
  m_fs_map.crc1 = CalcCRC((BYTE *) m_fs_map.sector1);
  WriteSectors(&m_fs_map, 0, 2);
  
  // create the root directory
  // (it is empty)
  BYTE *buffer = (BYTE *) calloc(ADFS_SECT_SIZE, 5); // 0x500 bytes
  if (buffer != NULL) {
    // build the root directory
    buffer[0] = cycle;  // cycle
    * (DWORD *) &buffer[1] = ADFS_SIG_HUGO;
    // entries would go here (we have then cleared out)
    buffer[0x4CB] = 0;
    buffer[0x4CC] = '$'; // root directory
    PutTriplet((struct S_ADFS_TRIPLET *) &buffer[0x4D6], 2);
    buffer[0x4D9] = '$'; // root directory
    buffer[0x4FA] = cycle; // cycle number (repeat)
    * (DWORD *) &buffer[0x4FB] = ADFS_SIG_HUGO;
    WriteSectors(buffer, 2, 5);
    
    // clear out the remaining sectors
    for (l=7; l<m_size; l++)
      WriteSectors(buffer, l, 1);
    
    if (!m_hard_format)
      SendToDialog(&m_fs_map); // Send back to Dialog
    
    free(buffer);
    
    // If using a MBR, to make Ultimate find the partition, the partition entry's ID must be non-zero.
    AfxMessageBox("If using a MBR, remember to mark\r\nthe ID to something other than zero.");

    return TRUE;
  }
  
  return FALSE;
}

// Adfs colors will have a grey shade with yellow hint to them.
DWORD CADFS::GetNewColor(int index) {
  int r = ((160 - (index * 18)) > -1) ? (160 - (index * 18)) : 0;
  int g = ((160 - (index * 2)) > -1) ? (160 - (index * 2)) : 0;
  int b = ((60 - (index * 20)) > -1) ? (60 - (index * 20)) : 0;
  return RGB(r, g, b);
}

//  We only support:
//     Type L:  SPT = 16, BytePerSector = 256, Heads = 2, 80 Tracks = 640KB (Map = old, Dir's = old) (root at 0x200)
void CADFS::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  
  ReadSectors(buffer, 0, 2);
  memcpy(&m_fs_map, buffer, sizeof(struct S_ADFS_FS_MAP));
  
  // DetectAdfs() detected us, so we should be good
  m_isvalid = TRUE;

  // 
  dlg->m_AdfsNames[index] = "BBC: Advanced File System";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_AdfsNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->Adfs[index].m_draw_index, (char *) (LPCTSTR) dlg->m_AdfsNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  SendToDialog(&m_fs_map);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_VIEW)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  if (m_isvalid) {
    m_dir_size = 5; // root is 5 sectors
    m_dir_buffer = (BYTE *) realloc(m_dir_buffer, m_dir_size * ADFS_SECT_SIZE);
    ReadSectors(m_dir_buffer, 2, m_dir_size);
    
    m_free_blocks = CalcFreeBlocks();
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    
    UpdateWindow();
    
    // fill the tree with the directory
    SaveItemInfo(m_hRoot, 0, (struct S_ADFS_DIR *) m_dir_buffer, 2, 2, 0x0500, ADFS_ENTRY_D, ITEM_IS_FOLDER);
    CWaitCursor wait; // display a wait cursor
    ParseDir(m_dir_buffer, 2, m_dir_size, m_hRoot);
    m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
    wait.Restore();
    GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
    GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
    GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
    
    // select the root and set the focus to that root
    GetDlgItem(IDC_DIR_TREE)->SetFocus();
    m_dir_tree.SelectSetFirstVisible(m_hRoot);
  }
  
  // display the running total of free bytes left
  DisplayFreeSpace();
  
  Invalidate(TRUE);  // redraw the tab
}

// get the name and attribute from the dir entry
// the name can be up to 10 chars
// if the name is less than 10 chars, it is terminated with 0x0D or 0x00
// valid chars are 33 -> 127.
// the attribute bits are in the high bit of each byte starting at byte 0.
BYTE CADFS::GetName(CString &name, DWORD *start, DWORD *size, struct S_ADFS_DIR *dir) {
  BYTE attr = 0;
  int i = 0;

  name.Empty();
  while ((i < 10) && (dir->name[i] != 0x0D) && (dir->name[i] != 0x00)) {
    name += (CHAR) (dir->name[i] & 0x7F);
    i++;
  }
  for (i=0; i <= 4; i++)
    attr |= ((dir->name[i] & 0x80) > 0) ? (1 << i) : 0;

  if (name.IsEmpty())
    return 0xFF;
  
  if (start) *start = GetTriplet(&dir->start_block);
  if (size) *size = dir->file_length;

  return attr;
}

// sectors is count of 256 sectors for this buffer
// buffer is 'sectors' * 256 bytes of directory space
void CADFS::ParseDir(const BYTE *buffer, DWORD parent, const int sectors, HTREEITEM ParentItem) {
  HTREEITEM hItem;
  struct S_ADFS_DIR *dir = (struct S_ADFS_DIR *) &buffer[5];
  BYTE *sub;
  int i;
  BYTE attribute = 0;
  CString name;
  DWORD start = 0, size;

  // Bytes 1->4 will always be "Hugo"
  // (later versions is "Nick")
  if (* (DWORD *) &buffer[1] != ADFS_SIG_HUGO) {
    //AfxMessageBox("Directory doesn't start with 'Hugo'");
    return;
  }

  // up to 47 26-byte entries
  for (i=0; i<47; i++) {
    attribute = GetName(name, &start, &size, &dir[i]);
    if (attribute < 0xFF) {
      if (attribute & ADFS_ENTRY_D) {
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, ParentItem);
        if (hItem == NULL) return;
        SaveItemInfo(hItem, i, &dir[i], start, parent, size, attribute, ITEM_IS_FOLDER);
        sub = (BYTE *) ReadFile(start, size);
        if (sub != NULL) {
          ParseDir(sub, start, size, hItem);
          free(sub);
          //m_parse_depth_limit--;
        }
      } else {
        hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, ParentItem);
        if (hItem == NULL) return;
        SaveItemInfo(hItem, i, &dir[i], start, parent, size, attribute, ITEM_IS_FILE);
      }
    }
  }
}

/*
 * .adl image files store sectors in the increment order: sector - track - side. 
 * So track 0:side 0 is followed by track 0:side 1 then track 1:side 0 etc. 
 * For *.adl files, sectors 0x000 to 0x00F are followed by sectors 0x500 to 0x50F, 
 *   then sectors 0x010 to 0x01F, sectors &510 to &51F, etc.
 * 
 * This assumes 16 sectors per track (80 tracks per side, 2 sides, 256-byte sectors = 640k (655360 bytes)) 
 *   Which is Type L of the ADFS specs.
 */
void *CADFS::ReadFile(DWORD start, DWORD size) {
  DWORD lba, offset = 0; // side 0
  
  // is it the second half of the sectors?
  if (start > 0x500) {
    offset = 16;   // side 1
    start -= 0x500;
  }
  size = ((size + ADFS_SECT_SIZE - 1) / ADFS_SECT_SIZE); // convert to sectors

  void *buffer = malloc(size * ADFS_SECT_SIZE);
  if (buffer != NULL) {
    BYTE *ptr = (BYTE *) buffer;
    while (size--) {
      lba = ((start >> 4) * 32) + offset + (start & 0x0F);
      ReadSectors(ptr, lba, 1);
      ptr += ADFS_SECT_SIZE;
      start++;
    }
  }

  return buffer;
}

// the physical sector size might not be 256, so we need to
//  account for this.
void CADFS::WriteSectors(void *buffer, DWORD start, DWORD count) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *p, *buf;

  if (dlg->m_sect_size == ADFS_SECT_SIZE)
    dlg->WriteToFile(buffer, m_lba + start, count);
  else {
    buf = (BYTE *) malloc(dlg->m_sect_size);
    p = (BYTE *) buffer;
    while (count--) {
      DWORD lba = start / (dlg->m_sect_size / ADFS_SECT_SIZE);
      DWORD offset = start % (dlg->m_sect_size / ADFS_SECT_SIZE);
      dlg->ReadFromFile(buf, m_lba + lba, 1);
      memcpy(buf + (offset * ADFS_SECT_SIZE), p, ADFS_SECT_SIZE);
      dlg->WriteToFile(buf, m_lba + lba, 1);
      p += ADFS_SECT_SIZE;
      start++;
    }
    free(buf);
  }
}

// the physical sector size might not be 256, so we need to
//  account for this.
void CADFS::ReadSectors(void *buffer, DWORD start, DWORD count) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *p, *buf;

  if (dlg->m_sect_size == ADFS_SECT_SIZE)
    dlg->ReadFromFile(buffer, m_lba + start, count);
  else {
    buf = (BYTE *) malloc(dlg->m_sect_size);
    p = (BYTE *) buffer;
    while (count--) {
      DWORD lba = start / (dlg->m_sect_size / ADFS_SECT_SIZE);
      DWORD offset = start % (dlg->m_sect_size / ADFS_SECT_SIZE);
      dlg->ReadFromFile(buf, m_lba + lba, 1);
      memcpy(p, buf + (offset * ADFS_SECT_SIZE), ADFS_SECT_SIZE);
      p += ADFS_SECT_SIZE;
      start++;
    }
    free(buf);
  }
}

void CADFS::WriteFile(void *buffer, DWORD start, DWORD size) {
  DWORD lba, offset = 0; // side 0
  
  // is it the second half of the sectors?
  if (start > 0x500) {
    offset = 16;   // side 1
    start -= 0x500;
  }
  size = ((size + ADFS_SECT_SIZE - 1) / ADFS_SECT_SIZE); // convert to sectors

  BYTE *ptr = (BYTE *) buffer;
  while (size--) {
    lba = ((start >> 4) * 32) + offset + (start & 0x0F);
    WriteSectors(ptr, lba, 1);
    ptr += ADFS_SECT_SIZE;
    start++;
  }
}

void CADFS::SaveItemInfo(HTREEITEM hItem, int entry_num, struct S_ADFS_DIR *entry, DWORD start, DWORD parent, DWORD size, BYTE attribute, DWORD flags) {
  struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->ErrorCode = 0;
    items->Start = start;
    items->Parent = parent;
    items->Size = size;
    items->Attribute = attribute;
    items->EntryNum = entry_num;
    items->Flags = flags;
    memcpy(&items->Entry, entry, sizeof(struct S_ADFS_DIR));
  }
}

inline void CADFS::PutTriplet(struct S_ADFS_TRIPLET *triplet, DWORD value) {
  triplet->entry[0] = (BYTE) ((value >>  0) & 0xFF);
  triplet->entry[1] = (BYTE) ((value >>  8) & 0xFF);
  triplet->entry[2] = (BYTE) ((value >> 16) & 0xFF);
}

inline DWORD CADFS::GetTriplet(struct S_ADFS_TRIPLET *triplet) {
  return (DWORD) triplet->entry[0] | (triplet->entry[1] << 8) | (triplet->entry[2] << 16);
}

// update all items in dialog
void CADFS::SendToDialog(struct S_ADFS_FS_MAP *fs_map) {
  CString cs;
  int i, cnt;
  BYTE *p0, *p1;
  int sectors = GetTriplet(&fs_map->sectors);
  
  m_adfs_sector_count.Format("%i", sectors);
  m_adfs_crc0.Format("0x%02X", fs_map->crc0);
  m_adfs_crc1.Format("0x%02X", fs_map->crc1);
  m_disk_id.Format("0x%04X", fs_map->disk_id);
  m_map_end.Format("%i", fs_map->map_end);
  
  // Get the disk name
  m_disk_name.Empty();
  p0 = (BYTE *) fs_map->name0;
  p1 = (BYTE *) fs_map->name1;
  for (i=0; i<5; i++) {
    if ((*p0 == 0) || (*p0 == 0xD)) break;
    m_disk_name += *p0++;
    if ((*p1 == 0) || (*p1 == 0xD)) break;
    m_disk_name += *p1++;
  }

  // free block list
  cnt = m_free_list.GetCount();
  for (i=0; i<cnt; i++)
    m_free_list.DeleteString(0);
  cnt = fs_map->map_end / sizeof(struct S_ADFS_TRIPLET);
  for (i=0; i<cnt; i++) {
    cs.Format("%2i: Start: %i, Sectors: %i", i, GetTriplet(&fs_map->sector0[i]), GetTriplet(&fs_map->sector1[i]));
    m_free_list.AddString(cs);
  }

  UpdateData(FALSE); // send to Dialog
}

void CADFS::ReceiveFromDialog(struct S_ADFS_FS_MAP *fs_map) {
  int i, j;
  BYTE *p0, *p1;

  UpdateData(TRUE); // receive from Dialog

  int sectors = convert32(m_adfs_sector_count);
  PutTriplet(&fs_map->sectors, sectors);

  fs_map->crc0 = convert8(m_adfs_crc0);
  fs_map->crc1 = convert8(m_adfs_crc1);

  fs_map->disk_id = convert16(m_disk_id);

  fs_map->map_end = convert8(m_map_end);

  p0 = (BYTE *) fs_map->name0;
  p1 = (BYTE *) fs_map->name1;
  memset(p0, 0, 5);
  memset(p1, 0, 5);
  for (i=0,j=0; i<5 && j < m_disk_name.GetLength(); i++) {
    *p0++ = m_disk_name.GetAt(j++);
    if (j == m_disk_name.GetLength()) break;
    *p1++ = m_disk_name.GetAt(j++);
  }
}

BYTE CADFS::CalcCRC(BYTE *buffer) {
  unsigned crc = 0;
  
  // is byte 255 the bit8u(sum + carry) of byte 254->0
  // (don't carry the carry on the last one)
  for (int i=254; i>=0; i--) {
    crc += buffer[i];
    if (crc > 0xFF) {
      crc &= 0xFF;
      if (i > 0)
        crc++;
    }
  }

  return (BYTE) crc;
}

void CADFS::OnCrc0Update() {
  ReceiveFromDialog(&m_fs_map); // Get From Dialog
  m_fs_map.crc0 = CalcCRC((BYTE *) m_fs_map.sector0);

  CString cs;
  cs.Format("0x%02X", m_fs_map.crc0);
  SetDlgItemText(IDC_ADFS_CRC0, cs);
}

void CADFS::OnCrc1Update() {
  ReceiveFromDialog(&m_fs_map); // Get From Dialog
  m_fs_map.crc1 = CalcCRC((BYTE *) m_fs_map.sector1);

  CString cs;
  cs.Format("0x%02X", m_fs_map.crc1);
  SetDlgItemText(IDC_ADFS_CRC1, cs);
}

void CADFS::DisplayFreeSpace(void) {
  CString csFree;
  
  csFree.Format("Free Space: %s (bytes)", (LPCSTR) gFormatNum(m_free_blocks * ADFS_SECT_SIZE, FALSE, FALSE));
  SetDlgItemText(IDC_FREE_SIZE_STR, csFree);
}

DWORD CADFS::CalcFreeBlocks(void) {
  DWORD count = 0;

  const int cnt = m_fs_map.map_end / sizeof(struct S_ADFS_TRIPLET);
  for (int i=0; i<cnt; i++)
    count += GetTriplet(&m_fs_map.sector1[i]);

  return count;
}

void CADFS::OnAdfsCopy() {
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

void CADFS::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  void *ptr = ReadFile(items->Start, items->Size);
  if (ptr != NULL) {
    CFile file;
    if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
      file.Write(ptr, items->Size);
      file.Close();
    } else
      AfxMessageBox("Error Creating File...");
    free(ptr);
  }
}

// hItem = tree item of folder
// csPath = existing path to create folder in
// csName = name of folder to create
void CADFS::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CADFS::OnAdfsView() {
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

void CADFS::OnAdfsInsert() {
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
  struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (IsDir)
    InsertFolder(items->Start, csFPath, csName, csPath);
  else
    InsertFile(items->Start, csFPath, csName, csPath);
  wait.Restore();
  //AfxMessageBox("Files transferred.");

  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

// csFPath = path on guest image to place file
// csName = name of file to insert
// csPath = path on host of file to insert
void CADFS::InsertFile(DWORD Parent, CString csFPath, CString csName, CString csPath) {
  void *buffer;
  CFile file;
  DWORD size;

  // open and get size of file to insert
  if (file.Open(csPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening File...");
    return;
  }
  size = (DWORD) file.GetLength();
  buffer = malloc(size);
  if (buffer != NULL)
    file.Read(buffer, (UINT) size);
  file.Close();
  
  // create the entry (the call free's the buffer)
  CreateEntry(buffer, size, csName, Parent, ADFS_ENTRY_R | ADFS_ENTRY_W);
}

// csFPath = path on guest image to place file
// csName = name of folder to insert
// csPath = path on host of folder to insert
void CADFS::InsertFolder(DWORD Parent, CString csFPath, CString csName, CString csPath) {
  const unsigned size = 8192;
  char szPath[MAX_PATH];
  CString csNewPath;
  
  // new path to store the nested files (if any)
  csNewPath = csFPath;
  csNewPath += csName;

  // create the folder buffer
  BYTE cycle = 0x27; // TODO
  BYTE *buffer = (BYTE *) calloc(ADFS_SECT_SIZE, 5); // 0x500 bytes
  buffer[0] = cycle;  // cycle
  * (DWORD *) &buffer[1] = ADFS_SIG_HUGO;
  // entries would go here (we have then cleared out)
  buffer[0x4CB] = 0;
  CreateName(&buffer[0x4CC], csName, 10);
  PutTriplet((struct S_ADFS_TRIPLET *) &buffer[0x4D6], 2);
  CreateName(&buffer[0x4D9], csNewPath, 19);
  buffer[0x4FA] = cycle; // cycle number (repeat)
  * (DWORD *) &buffer[0x4FB] = ADFS_SIG_HUGO;

  // new parent starting sector
  Parent = CreateEntry(buffer, 0x500, csName, Parent, ADFS_ENTRY_R | ADFS_ENTRY_W | ADFS_ENTRY_D);
  if (Parent == -1)
    return;

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
      InsertFolder(Parent, csNewPath, filefind.GetFileName(), filefind.GetFilePath());
    else
      InsertFile(Parent, csNewPath, filefind.GetFileName(), filefind.GetFilePath());
  }
  filefind.Close();
  
  // restore the current directory
  SetCurrentDirectory(szPath);
}

// find and create a directory entry
//  Creates a directory entry at Parent
//  returns starting sector if successful, else -1
//  ** free's the buffer passed **
DWORD CADFS::CreateEntry(void *buffer, DWORD size, CString csName, DWORD Parent, DWORD attrib) {
  struct S_ADFS_FS_MAP map_copy;

  // since we have to check a few things before we can safely insert a file,
  //  we actually make a copy of the fs_map and use this copy until
  //  we are sure that we can insert safely.
  memcpy(&map_copy, &m_fs_map, sizeof(struct S_ADFS_FS_MAP));
    
  // first, we need to see if there is space on the disk to store the file
  DWORD start = InsertBlocks(&map_copy, size);
  if (start > 0) {
    // next see if we can insert an entry into the (sub)directory
    BYTE *sub = (BYTE *) ReadFile(Parent, 0x500);
    if (sub != NULL) {
      // need to find a free slot in the directory
      struct S_ADFS_DIR *dir = (struct S_ADFS_DIR *) &sub[5];
      for (int i=0; i<47; i++) {
        if ((dir[i].name[0] == 0) || (dir[i].name[0] == 0x0D)) {
          // found a free entry
          memset(&dir[i], 0, sizeof(struct S_ADFS_DIR));
          CreateName(dir[i].name, csName, 10);
          for (int j=0; j<5; j++)
            dir[i].name[j] |= ((attrib & (1<<j)) ? 0x80 : 0x00);
          dir[i].load_addr = 0;
          dir[i].exec_addr = 0;
          dir[i].file_length = size;
          PutTriplet(&dir[i].start_block, start);
          dir[i].cycle = 0; /// TODO:
          // write sub entry back
          WriteFile(sub, Parent, 0x500);
          // write the file to the volume
          WriteFile(buffer, start, size);
          // free the buffer
          free(buffer);
          // successfully inserted the file, so update everything else
          memcpy(&m_fs_map, &map_copy, sizeof(struct S_ADFS_FS_MAP));
          OnCrc0Update();
          OnCrc1Update();
          WriteSectors(&m_fs_map, 0, 2);
          return start;
        }
      }
    }
    AfxMessageBox("Could not insert into parent!");
  } else
    AfxMessageBox("Did not find enough free space in map!");
  
  // free the buffer
  free(buffer);
  
  return -1;
}

// places a name into the buffer given, terminating with 0x0D if needed
void CADFS::CreateName(BYTE *buffer, CString csName, int Len) {
  int i;
  
  for (i=0; i<Len && i<csName.GetLength(); i++)
    buffer[i] = csName.GetAt(i);
  for (; i<Len; i++)
    buffer[i] = 0x0D;
}

// the user change the status of the "Delete Clear" Check box
void CADFS::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "ADFSDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void CADFS::OnAdfsDelete() {
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  if (!hItem) {
    AfxMessageBox("No Item");
    return;
  }
  
  // ask before deleting the root directory
  // Ben: Todo:
  if (csName == "Root")
    if (AfxMessageBox("This will delete the root directory!  Continue?", MB_YESNO, 0) != IDYES)
      return;
  
  // get the parent of this file/dir so we can select it after the delete
  HTREEITEM hParent = m_dir_tree.GetParentItem(hItem);
  
  CWaitCursor wait;
  if (IsDir)
    DeleteFolder(hItem, csName);
  else
    DeleteFile(hItem, csName);
  
  // delete the item from the tree
  if (IsDlgButtonChecked(IDC_SHOW_DEL))
    Start(m_lba, m_size, m_color, m_index, FALSE);
  else
    m_dir_tree.DeleteItem(hItem);
  
  // select the parent item
  m_dir_tree.Select((hParent != NULL) ? hParent : TVI_ROOT, TVGN_CARET);
  
  // update the freespace display
  m_free_blocks = CalcFreeBlocks();
  DisplayFreeSpace();
  
  // update the fields and write the map back
  SendToDialog(&m_fs_map);
  OnCrc0Update();
  OnCrc1Update();
  WriteSectors(&m_fs_map, 0, 2);

  wait.Restore();
  //AfxMessageBox("File(s) deleted.");
}

void CADFS::DeleteFolder(HTREEITEM hItem, CString csOrgName) {
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
        DeleteFolder(hDeleteItem, csName);
      else 
        DeleteFile(hDeleteItem, csName);
    }
  }
  DeleteFile(hItem, csOrgName);
}

void CADFS::DeleteFile(HTREEITEM hItem, CString csName) {
  struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  CString name;
  DWORD start = 0, size;
  BYTE attribute;
  
  if (items == NULL)
    return;
  
  // load the directory
  BYTE *buffer = (BYTE *) ReadFile(items->Parent, 0x500);
  if (buffer) {
    struct S_ADFS_DIR *dir = (struct S_ADFS_DIR *) &buffer[5];
    // we have to search the directory to find the entry
    // (this will find the first entry that matches this name!)
    // up to 47 26-byte entries
    for (int i=0; i<47; i++) {
      attribute = GetName(name, &start, &size, &dir[i]);
      if (attribute < 0xFF) {
        if (name == csName) {
          // first remove it from the directory
          for (; i<46; i++)
            memcpy(&dir[i], &dir[i+1], sizeof(struct S_ADFS_DIR));
          memset(&dir[46], 0, sizeof(struct S_ADFS_DIR));
          WriteFile(buffer, items->Parent, 0x500);

          // mark the area as free
          FreeBlocks(&m_fs_map, start, size);
          
          // zero the space occupied?
          if (m_del_clear) {
            void *file = ReadFile(start, size);
            if (file) {
              memset(file, 0, size);
              WriteFile(file, start, size);
              free(file);
            }
          }
        }
      }
    }
    free(buffer);
  }
}

// this tries to find free space in the map and insert "size" into it
// returns starting block if successful, -1 if not
int CADFS::InsertBlocks(struct S_ADFS_FS_MAP *map, DWORD size) {
  // size is count of bytes needed, so adjust for sectors needed
  size = (size + ADFS_SECT_SIZE - 1) / ADFS_SECT_SIZE;

  DWORD len, start = -1;
  int cnt = map->map_end / sizeof(struct S_ADFS_TRIPLET);
  for (int i=0; i<cnt; i++) {
    len = GetTriplet(&map->sector1[i]);
    if (size == len) {
      // found an equal sized chunk, so remove from list
      start = GetTriplet(&map->sector0[i]);
      for (; i<cnt-1; i++) {
        PutTriplet(&map->sector0[i], GetTriplet(&map->sector0[i+1]));
        PutTriplet(&map->sector1[i], GetTriplet(&map->sector1[i+1]));
      }
      PutTriplet(&map->sector0[i], 0);
      PutTriplet(&map->sector1[i], 0);
      map->map_end -= sizeof(struct S_ADFS_TRIPLET);
      break;
    } else if (size < len) {
      // found a chunk larger than we need, so adjust it
      start = GetTriplet(&map->sector0[i]);
      PutTriplet(&map->sector0[i], start + size);
      PutTriplet(&map->sector1[i], len - size);
      break;
    }
  }
  
  return start;
}

// this tries to free space in the map
void CADFS::FreeBlocks(struct S_ADFS_FS_MAP *map, DWORD begin, DWORD size) {
  // size is count of bytes needed, so adjust for sectors needed
  size = (size + ADFS_SECT_SIZE - 1) / ADFS_SECT_SIZE;

  // optimize the list before we add to it
  OptimizeFreeList(map);

  // is there room to add it now?
  int cnt = map->map_end / sizeof(struct S_ADFS_TRIPLET);
  if (cnt < 82) {
    PutTriplet(&map->sector0[cnt], begin);
    PutTriplet(&map->sector1[cnt], size);
    map->map_end += sizeof(struct S_ADFS_TRIPLET);
    
    // optimize the list after we add to it
    OptimizeFreeList(map);
  }
}

// optimize the Free Space list
void CADFS::OptimizeFreeList(struct S_ADFS_FS_MAP *map) {
  int i, j, cnt = map->map_end / sizeof(struct S_ADFS_TRIPLET);
  DWORD start, len;

  // if there is only one, no need to optimize it
  if (cnt <= 1)
    return;

  // first, sort the list (a simple bubble sort)
  for (j=0; j<cnt; j++) {
    for (i=0; i<cnt-1; i++) {
      start = GetTriplet(&map->sector0[i]);
      len = GetTriplet(&map->sector1[i]);
      if (GetTriplet(&map->sector0[i+1]) < start) {
        PutTriplet(&map->sector0[i], GetTriplet(&map->sector0[i+1]));
        PutTriplet(&map->sector1[i], GetTriplet(&map->sector1[i+1]));
        PutTriplet(&map->sector0[i+1], start);
        PutTriplet(&map->sector1[i+1], len);
      }
    }
  }  
  
  // now combine any that can be combined
  for (i=0; i<cnt-1; i++) {
    start = GetTriplet(&map->sector0[i]);
    len = GetTriplet(&map->sector1[i]);
    if ((start + len) == GetTriplet(&map->sector0[i+1])) {
      len += GetTriplet(&map->sector1[i+1]);
      PutTriplet(&map->sector1[i], len);
      for (j=i+1; j<cnt-1; j++) {
        PutTriplet(&map->sector0[j], GetTriplet(&map->sector0[j+1]));
        PutTriplet(&map->sector1[j], GetTriplet(&map->sector1[j+1]));
      }
      PutTriplet(&map->sector0[j], 0);
      PutTriplet(&map->sector1[j], 0);
      cnt--;
    }
  }

  // update the end marker
  map->map_end = (BYTE) (cnt * sizeof(struct S_ADFS_TRIPLET));
}

// sorts the directory listing
void CADFS::SortDirListing(BYTE *buffer, DWORD lba) {
  struct S_ADFS_DIR *dir = (struct S_ADFS_DIR *) &buffer[5];
  struct S_ADFS_DIR temp;
  int i, j;
  BYTE attribute0, attribute1;
  CString name0, name1;
  DWORD start;

  // Bytes 1->4 will always be "Hugo"
  // (later versions is "Nick")
  if (* (DWORD *) &buffer[1] != ADFS_SIG_HUGO)
    return;

  // first recurse through any sub dirs
  for (i=0; i<47; i++) {
    attribute0 = GetName(name0, &start, NULL, &dir[i]);
    if ((attribute0 < 0xFF) && (attribute0 & ADFS_ENTRY_D)) {
      BYTE *sub = (BYTE *) malloc(0x500);
      ReadSectors(sub, start, 5);
      SortDirListing(sub, start);
      free(sub);
    }
  }

  // now sort the lising
  for (j=0; j<47; j++) {
    for (i=0; i<46; i++) {
      attribute0 = GetName(name0, NULL, NULL, &dir[i]);
      attribute1 = GetName(name1, NULL, NULL, &dir[i+1]);
      if ((attribute0 < 0xFF) && (attribute1 < 0xFF)) {
        if (name1 < name0) {
          memcpy(&temp, &dir[i], sizeof(struct S_ADFS_DIR));
          memcpy(&dir[i], &dir[i+1], sizeof(struct S_ADFS_DIR));
          memcpy(&dir[i+1], &temp, sizeof(struct S_ADFS_DIR));
        }
      }
    }
  }
  
  // now write it back
  WriteSectors(buffer, lba, 5);

  // refresh the "system"
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CADFS::OnAdfsOptimize() {
  
  // TODO: clear any freespace entries past end pointer
  // TODO: in SortDirListing(), clear any entries at end...
  // TODO: 
  // TODO: 
  // TODO: 
  // TODO: All "Cycle" fields


  // sort all the directory listings
  BYTE *buffer = (BYTE *) malloc(0x500);
  ReadSectors(buffer, 2, 5);
  SortDirListing(buffer, 2);
  free(buffer);

  // sort and shrink the free block list
  OptimizeFreeList(&m_fs_map);

  // update all fields
  SendToDialog(&m_fs_map);
  OnCrc0Update();
  OnCrc1Update();
  WriteSectors(&m_fs_map, 0, 2);
}

void CADFS::OnSearch() {
  m_dir_tree.Search();
}

void CADFS::OnAdfsEntry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CADFSEntry ADFSEntry;

  // Count how many are in this folder
  int count = m_dir_tree.GetCountOfChildren(m_dir_tree.GetParentItem(hItem));

  if (hItem) {
    struct S_ADFS_ITEMS *items = (struct S_ADFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      ADFSEntry.m_entry_num = items->EntryNum;
      ADFSEntry.m_entry_tot = count;
      memcpy(&ADFSEntry.m_entry, &items->Entry, sizeof(struct S_ADFS_DIR));
      if (ADFSEntry.DoModal() == IDOK) { // apply button pressed?
        BYTE *buffer = (BYTE *) malloc(0x500);
        struct S_ADFS_DIR *dir = (struct S_ADFS_DIR *) &buffer[5];
        ReadSectors(buffer, items->Parent, 5);
        memcpy(&dir[items->EntryNum], &ADFSEntry.m_entry, sizeof(struct S_ADFS_DIR));
        WriteSectors(buffer, items->Parent, 5);
        free(buffer);
        
        // refresh the "system"
        Start(m_lba, m_size, m_color, m_index, FALSE);
      }
    }
  }
}

void CADFS::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CADFS::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CADFS::OnErase() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  
  if (AfxMessageBox("This will erase the whole partition!  Continue?", MB_YESNO, 0) == IDYES) {
    memset(buffer, 0, MAX_SECT_SIZE);
    CWaitCursor wait; // display a wait cursor
    for (DWORD lba=0; lba<m_size; lba++)
      WriteSectors(buffer, lba, 1);
    wait.Restore(); // unnecassary since the 'destroy' code will restore it, but just to make sure.
    dlg->SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}
