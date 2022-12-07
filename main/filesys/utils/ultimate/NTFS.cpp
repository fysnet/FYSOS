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

// NTFS.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "FYSOSSig.h"

#include "NTFS.h"
#include "NTFSEntry.h"

#include "Modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNTFS property page

IMPLEMENT_DYNCREATE(CNTFS, CPropertyPage)

CNTFS::CNTFS() : CPropertyPage(CNTFS::IDD) {
  //{{AFX_DATA_INIT(CNTFS)
  m_cluster_index_block = _T("");
  m_cluster_rec_size = _T("");
  m_bytes_sect = _T("");
  m_descriptor = _T("");
  m_heads = _T("");
  m_hidden_sect = _T("");
  m_jmp0 = _T("");
  m_jmp1 = _T("");
  m_jmp2 = _T("");
  m_oem_name = _T("");
  m_sect_cluster = _T("");
  m_sect_reserved = _T("");
  m_sect_track = _T("");
  m_serial_num = _T("");
  m_mft_cluster = _T("");
  m_mft_mirror_cluster = _T("");
  m_sectors = _T("");
  m_del_clear = FALSE;
  //}}AFX_DATA_INIT
  m_last_mft = 0;
  m_mft_start = STARTING_MFT;
  m_bpb_buffer = NULL;
  m_mft_buffer = NULL;
}

CNTFS::~CNTFS() {
  if (m_bpb_buffer)
    free(m_bpb_buffer);
  if (m_mft_buffer)
    free(m_mft_buffer);
}

void CNTFS::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CNTFS)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_CLUSTER_INDEX_BLOCK, m_cluster_index_block);
  DDX_Text(pDX, IDC_CLUSTER_REC_SIZE, m_cluster_rec_size);
  DDX_Text(pDX, IDC_FAT_BYTES_SECT, m_bytes_sect);
  DDX_Text(pDX, IDC_FAT_DESCRIPTOR, m_descriptor);
  DDX_Text(pDX, IDC_FAT_HEADS, m_heads);
  DDX_Text(pDX, IDC_FAT_HIDDEN_SECTS, m_hidden_sect);
  DDX_Text(pDX, IDC_FAT_JMP0, m_jmp0);
  DDX_Text(pDX, IDC_FAT_JMP1, m_jmp1);
  DDX_Text(pDX, IDC_FAT_JMP2, m_jmp2);
  DDX_Text(pDX, IDC_FAT_OEM_NAME, m_oem_name);
  DDX_Text(pDX, IDC_FAT_SECT_CLUSTER, m_sect_cluster);
  DDX_Text(pDX, IDC_FAT_SECT_RESERVED, m_sect_reserved);
  DDX_Text(pDX, IDC_FAT_SECT_TRACK, m_sect_track);
  DDX_Text(pDX, IDC_FAT_SERIAL_NUM, m_serial_num);
  DDX_Text(pDX, IDC_MFT_CLUSTER, m_mft_cluster);
  DDX_Text(pDX, IDC_MFT_MIRROR_CLUST, m_mft_mirror_cluster);
  DDX_Text(pDX, IDC_SECTORS, m_sectors);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNTFS, CPropertyPage)
  //{{AFX_MSG_MAP(CNTFS)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_BN_CLICKED(ID_UPDATE_CODE, OnUpdateCode)
  ON_BN_CLICKED(ID_APPLY, OnApplyB)
  ON_BN_CLICKED(ID_CLEAN, OnClean)
  ON_BN_CLICKED(ID_FORMAT, OnFormat)
  ON_BN_CLICKED(ID_CHECK, OnCheck)
  ON_BN_CLICKED(ID_COPY, OnCopy)
  ON_BN_CLICKED(ID_VIEW, OnView)
  ON_BN_CLICKED(ID_INSERT, OnInsert)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ERASE, OnErase)
  ON_BN_CLICKED(ID_ENTRY, OnEntry)
  ON_BN_CLICKED(ID_FYSOS_SIG, OnFysosSig)
  ON_EN_CHANGE(IDC_CLUSTER_REC_SIZE, OnChangeClusterRecSize)
  ON_BN_CLICKED(IDC_SHOW_SYS_MFT, OnShowSysMft)
  ON_BN_CLICKED(IDC_DEL_CLEAR, OnDelClear)
  ON_WM_HELPINFO()
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNTFS message handlers
BOOL CNTFS::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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

BOOL CNTFS::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  // display the base/size string
  CString csBaseSize;
  csBaseSize.Format("Start: %I64i, Size: %I64i", m_lba, m_size);
  SetDlgItemText(IDC_BASE_SIZE_STR, csBaseSize);

  m_del_clear = AfxGetApp()->GetProfileInt("Settings", "NTFSDelClear", FALSE);
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
  
  EnableToolTips(TRUE);

  return TRUE;
}

BOOL CNTFS::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "ntfs.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CNTFS::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_NTFS_ITEMS *items = (struct S_NTFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  
  GetDlgItem(ID_ENTRY)->EnableWindow((hItem != NULL) && (m_dir_tree.GetParentItem(hItem) != NULL));
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && (items->mft_entry >= STARTING_MFT) && items->CanCopy);
  GetDlgItem(ID_VIEW)->EnableWindow((hItem != NULL) && (items->Flags & ITEM_IS_FILE));
  //GetDlgItem(ID_INSERT)->EnableWindow(FALSE /*(hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0)*/);
  //GetDlgItem(ID_DELETE)->EnableWindow(FALSE /*hItem != NULL*/);
  
  *pResult = 0;
}

// Lean colors will have a red shade to them.
DWORD CNTFS::GetNewColor(int index) {
  int r = ((250 - (index *  2)) > -1) ? (250 - (index *  2)) : 0;
  int g = ((100 - (index * 18)) > -1) ? (100 - (index * 18)) : 0;
  int b = (( 96 - (index * 20)) > -1) ? ( 96 - (index * 20)) : 0;
  return RGB(r, g, b);
}

void CNTFS::Start(const DWORD64 lba, const DWORD64 size, const DWORD color, const int index, BOOL IsNewTab) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CString cs;
  
  m_lba = lba;
  m_size = size;
  m_index = index;
  m_color = color;
  m_isvalid = TRUE;
  
  // read the bpb
  if (m_bpb_buffer) free(m_bpb_buffer);
  m_bpb_buffer = malloc(MAX_SECT_SIZE);
  dlg->ReadFromFile(m_bpb_buffer, lba, 1);
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  
  dlg->m_NTFSNames[index] = "NTFS";
  m_psp.dwFlags |= PSP_USETITLE;
  m_psp.pszTitle = dlg->m_NTFSNames[index];
  dlg->m_image_bar.UpdateTitle(dlg->NTFS[index].m_draw_index, (char *) (LPCTSTR) dlg->m_NTFSNames[index]);
  
  // Add the page to the control
  if (IsNewTab)
    dlg->m_TabControl.AddPage(this);
  dlg->m_TabControl.SetActivePage(this);
  
  // get the size of a record
  // (if val is positive, then val *= cluster size)
  m_rec_size = (int) (INT8) bpb->clust_file_rec_size;
  if (m_rec_size > -1)
    m_rec_size = m_rec_size * bpb->sect_per_clust * bpb->bytes_per_sect;
  // if val is negative, then byte size is 2 to the power of abs(val)
  else
    m_rec_size = (1 << -m_rec_size);
  
  // setup the MFT buffer, and read in the first cache size
  if (m_mft_buffer) free(m_mft_buffer);
  m_mft_buffer = malloc(NTFS_MFT_CACHE_SIZE * m_rec_size);
  m_mft_lba = m_lba + (bpb->MFT_cluster * bpb->sect_per_clust);
  m_mft_cur = 0;
  m_mft_dirty = FALSE;
  dlg->ReadFromFile(m_mft_buffer, m_mft_lba, ((NTFS_MFT_CACHE_SIZE * m_rec_size) / bpb->bytes_per_sect));
  
  // get the size of the table
  DWORD64 fsize = 0;
  struct S_NTFS_FILE_REC *file_rec = GetMFT(MFT_MFT);
  if (file_rec && GetFileAttrbs(file_rec, NULL, NULL, &fsize, NULL, NULL))
    m_last_mft = (unsigned) (fsize / m_rec_size);
  else
    m_isvalid = FALSE;
  
  SendToDialog((struct S_NTFS_BPB *) m_bpb_buffer);
  
  GetDlgItem(ID_ENTRY)->EnableWindow(FALSE);
  GetDlgItem(ID_COPY)->EnableWindow(FALSE);
  GetDlgItem(ID_VIEW)->EnableWindow(FALSE);
  GetDlgItem(ID_INSERT)->EnableWindow(FALSE);
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  GetDlgItem(ID_SEARCH)->EnableWindow(FALSE);
  
  // check that the sector size of the VBR matches our Dialog's sector size
  if (bpb->bytes_per_sect != dlg->m_sect_size) {
    cs.Format("BPB sector size (%i) doesn't equal our dialog sector size (%i)...", bpb->bytes_per_sect, dlg->m_sect_size);
    AfxMessageBox(cs);
    m_isvalid = FALSE;
  }
  
  if (m_isvalid) {
    GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
    
    // make sure the tree is emtpy
    m_dir_tree.DeleteAllItems();
    m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
    m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
    SaveItemInfo(m_hRoot, MFT_ROOT, 0, ITEM_IS_FOLDER, FALSE);
    
    UpdateWindow();
    
    // fill the tree with the directory
    CWaitCursor wait; // display a wait cursor
    
    // if going to take a little while, display a message saying so
    CModeless modeless;
    if (m_last_mft > 5000) {
      modeless.m_Title = "Parsing Directories";
      modeless.m_modeless = TRUE;
      modeless.Create(CModeless::IDD, this);
      modeless.ShowWindow(SW_SHOW);
      modeless.BringWindowToTop();
      modeless.SetDlgItemText(IDC_EDIT, "Working.  This might take a minute or two");
      modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
      modeless.GetDlgItem(IDC_DONE)->EnableWindow(FALSE);
    }
    
    m_too_many = FALSE;
    ParseDir(m_mft_start, MFT_ROOT, m_hRoot);
    m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
    
    // then kill the message (if we displayed it)
    if (m_last_mft > 5000)
      modeless.DestroyWindow();
    
    wait.Restore();
    GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
    GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
    GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
    GetDlgItem(IDC_SHOW_SYS_MFT)->EnableWindow(TRUE);
  }
}

void CNTFS::ParseDir(unsigned start, DWORD64 ref, HTREEITEM parent) {
  HTREEITEM hItem;
  DWORD attribs = 0;
  DWORD64 Parent;
  char filename[256];
  
  for (unsigned cur_mft=start; cur_mft<m_last_mft && !m_too_many; cur_mft++) {
    struct S_NTFS_FILE_REC *file_rec = GetMFT(cur_mft);
    if (file_rec) {
      if (GetFileAttrbs(file_rec, filename, NULL, NULL, &attribs, &Parent)) {
        if (NTFS_GET_FILE_REF(Parent) == ref) {
          if (attribs & NTFS_ATTR_SUB_DIR) {
            hItem = m_dir_tree.Insert(filename, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
            if (hItem == NULL) { m_too_many = TRUE; return; }
            SaveItemInfo(hItem, cur_mft, 0, ITEM_IS_FOLDER, TRUE);
            if (cur_mft != MFT_ROOT)
              ParseDir(cur_mft+1, cur_mft, hItem);
          } else {
            hItem = m_dir_tree.Insert(filename, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
            if (hItem == NULL) { m_too_many = TRUE; return; }
            SaveItemInfo(hItem, cur_mft, 0, ITEM_IS_FILE, TRUE);
          }
        }
      } else {
        //m_too_many = TRUE;
        //return;
      }
    } else {
      //m_too_many = TRUE;
      return;
    }
  }
}

void CNTFS::SaveItemInfo(HTREEITEM hItem, unsigned ref, DWORD ErrorCode, DWORD flags, BOOL CanCopy) {
  struct S_NTFS_ITEMS *items = (struct S_NTFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->mft_entry = ref;
    items->ErrorCode = ErrorCode;
    items->CanCopy = CanCopy;
    items->Flags = flags;
  }
}

void CNTFS::SendToDialog(struct S_NTFS_BPB *bpb) {
  m_cluster_index_block.Format("%i", bpb->clust_index_block);
  m_cluster_rec_size.Format("%i", bpb->clust_file_rec_size);
  m_bytes_sect.Format("%i", bpb->bytes_per_sect);
  m_descriptor.Format("0x%02X", bpb->descriptor);
  m_heads.Format("%i", bpb->heads);
  m_hidden_sect.Format("%i", bpb->hidden);
  m_jmp0.Format("0x%02X", bpb->jmp[0]);
  m_jmp1.Format("0x%02X", bpb->jmp[1]);
  m_jmp2.Format("0x%02X", bpb->jmp[2]);
  m_oem_name.Format("%c%c%c%c%c%c%c%c", bpb->oem_name[0], bpb->oem_name[1], bpb->oem_name[2], bpb->oem_name[3],
    bpb->oem_name[4], bpb->oem_name[5], bpb->oem_name[6], bpb->oem_name[7]);
  m_sect_cluster.Format("%i", bpb->sect_per_clust);
  m_sect_reserved.Format("%i", bpb->sect_resved);
  m_sect_track.Format("%i", bpb->sect_per_trk);
  m_serial_num.Format("0x%08X", bpb->serial_num);
  m_mft_cluster.Format("%I64i", bpb->MFT_cluster);
  m_mft_mirror_cluster.Format("%I64i", bpb->MFTMirr_cluster);
  m_sectors.Format("%I64i", bpb->tot_sectors);
  
  UpdateData(FALSE); // send to Dialog
  
  OnChangeClusterRecSize();
}

void CNTFS::ReceiveFromDialog(struct S_NTFS_BPB *bpb) {
  UpdateData(TRUE); // receive from Dialog

  AfxMessageBox("TODO");
  
}

// the user change the status of the "Delete Clear" Check box
void CNTFS::OnDelClear() {
  AfxGetApp()->WriteProfileInt("Settings", "NTFSDelClear", m_del_clear = IsDlgButtonChecked(IDC_DEL_CLEAR));
  if (m_del_clear)
    SetDlgItemText(ID_DELETE, "Delete/Zero");
  else
    SetDlgItemText(ID_DELETE, "Delete");
}

void *CNTFS::ReadFile(const unsigned ref, DWORD64 *FileSize) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  struct S_NTFS_FILE_REC *file_rec = GetMFT(ref);
  void *ret = NULL;
  
  if (FileSize) *FileSize = 0;
  
  if (file_rec && (file_rec->magic == FILE_REC_MAGIC)) {
    struct S_NTFS_ATTR *attrb = (struct S_NTFS_ATTR *) ((BYTE *) file_rec + file_rec->attribs_off);
    while (attrb->type != NTFS_ATTR_EOA) {
      if (attrb->type == NTFS_ATTR_DATA) {
        if (attrb->comp_flag == 0) {
          // if non resident, then it is a run list
          if (attrb->non_res_flag) {
            struct S_NTFS_ATTR_NONRES *attrb_nres = (struct S_NTFS_ATTR_NONRES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
            BYTE *list = (BYTE *) ((BYTE *) attrb + attrb_nres->run_list_off);
            // get the size of the file and allocate the buffer
            if (FileSize) *FileSize = attrb_nres->real_size;
            ret = malloc((size_t) (attrb_nres->real_size + (bpb->sect_per_clust * bpb->bytes_per_sect)));
            BYTE *buffer = (BYTE *) ret;
            INT64 cnt = (INT64) attrb_nres->real_size;
            DWORD64 cluster, lcn = 0;
            unsigned i, L, F, count;
            while (cnt > 0) {
              L = (list[0] >> 0) & 0x0F;  // TODO: we assume F <= 4
              F = (list[0] >> 4) & 0x0F;  // TODO: we assume F <= 8
              list++;
              count = 0;
              for (i=0; i<L; i++)
                count |= ((unsigned) *list++ << (i * 8));
              cluster = 0;
              for (i=0; i<F; i++)
                cluster |= ((DWORD64) *list++ << (i * 8));
              
              // can't just read in all of the run at once incase it overwrites our buffer.
              // our buffer is padded with only one (1) extra cluster space
              for (unsigned i=0; i<count && cnt>0; i++) {
                dlg->ReadFromFile(buffer, m_lba + ((lcn + cluster) * bpb->sect_per_clust) + (i * bpb->sect_per_clust), bpb->sect_per_clust);
                buffer += (bpb->sect_per_clust * bpb->bytes_per_sect);
                cnt -= (bpb->sect_per_clust * bpb->bytes_per_sect);
              }
              lcn = cluster;
            }
          // if it is resident, then the data is already in "data", so just extract what we need.
          } else {
            struct S_NTFS_ATTR_RES *attrb_res = (struct S_NTFS_ATTR_RES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
            if (attrb_res->data_len > 0) {
              // get the size of the file
              if (FileSize) *FileSize = attrb_res->data_len;
              ret = malloc(attrb_res->data_len + (bpb->sect_per_clust * bpb->bytes_per_sect));
              memcpy(ret, (void *) ((BYTE *) attrb + attrb_res->data_off), attrb_res->data_len);
            }
          }
        } else
          AfxMessageBox("TODO: File is compressed/encrypted/sparse");
        break;
      }
      attrb = (struct S_NTFS_ATTR *) ((BYTE *) attrb + attrb->len);
    }
  }
  
  return ret;
}

void CNTFS::OnEntry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_NTFS_ITEMS *items;
  CNTFSEntry NTFSEntry;
  
  if (hItem) {
    if (hItem == m_hRoot) {
      AfxMessageBox("No entry exists for the root");
      return;
    }
    items = (struct S_NTFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      struct S_NTFS_FILE_REC *file_rec = GetMFT(items->mft_entry);
      NTFSEntry.m_file_rec = (struct S_NTFS_FILE_REC *) malloc(file_rec->record_len);
      memcpy(NTFSEntry.m_file_rec, file_rec, file_rec->record_len);
      if (NTFSEntry.DoModal() == IDOK) { // apply button pressed?
        if (NTFSEntry.m_file_rec->record_len <= NTFSEntry.m_file_rec->alloc_size) {
          memcpy(file_rec, NTFSEntry.m_file_rec, NTFSEntry.m_file_rec->record_len);
          m_mft_dirty = TRUE;
          WriteMFT();
          // reload the list
          Start(m_lba, m_size, m_color, m_index, FALSE);
        } else
          AfxMessageBox("New record size > allocated size. Restoring original values.");
      }
    }
  }
}

void CNTFS::OnClean() {
  AfxMessageBox("TODO");
}

void CNTFS::OnFormat() {
  AfxMessageBox("TODO");
}

void CNTFS::OnCheck() {
  AfxMessageBox("TODO");
}

void CNTFS::OnCopy() {
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

void CNTFS::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_NTFS_ITEMS *items = (struct S_NTFS_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  DWORD64 FileSize = 0;
  
  if (items) {
    void *ptr = ReadFile(items->mft_entry, &FileSize);
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
void CNTFS::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CNTFS::OnView() {
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

void CNTFS::OnInsert() {
  AfxMessageBox("TODO");
}

void CNTFS::OnSearch() {
  m_dir_tree.Search();
}

void CNTFS::OnUpdateCode() {
  AfxMessageBox("TODO");
  
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
    dlg->WriteToFile(existing, m_lba, m_super.resv_blocks);
    
    AfxMessageBox("Updated Boot Code successfully");
  }
  
  free(buffer);
  free(existing);
*/
}

void CNTFS::OnFysosSig() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  dlg->UpdateSig(m_lba);
}

void CNTFS::OnApplyB() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  
  // write the MFT back?
  WriteMFT();
  
  ReceiveFromDialog((struct S_NTFS_BPB *) m_bpb_buffer);
  
  // write the sector
  dlg->WriteToFile(m_bpb_buffer, m_lba, 1);
}

void CNTFS::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CNTFS::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}

void CNTFS::OnChangeClusterRecSize() {
  CString cs;
  unsigned spc, bps;
  int val;
  
  GetDlgItemText(IDC_FAT_SECT_CLUSTER, cs);
  spc = convert8(cs);
  
  GetDlgItemText(IDC_FAT_BYTES_SECT, cs);
  bps = convert16(cs);
  
  GetDlgItemText(IDC_CLUSTER_REC_SIZE, cs);
  val = (int) convert32(cs);
  // (if val < 0x80, then val = cluster size)
  if (val > -1)
    val = val * spc * bps;
  // if val >= 0x80 (i.e. is negative), then byte size is 2 to the power of abs(val)
  else
    val = (1 << -val);
  
  cs.Format("%u", val);
  SetDlgItemText(IDC_CLUSTER_REC_SIZE_DISP, cs);
}

void CNTFS::WriteMFT(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  
  // write the MFT back?
  if (m_mft_dirty) {
    FixupMFT(FALSE); // create the fixups in the buffer
    dlg->WriteToFile(m_mft_buffer, m_mft_lba + ((m_mft_cur * m_rec_size) / bpb->bytes_per_sect), ((NTFS_MFT_CACHE_SIZE * m_rec_size) / bpb->bytes_per_sect));
    m_mft_dirty = FALSE;
  }
}

// we need to read the whole cluster and do the fixups.
//  then we can pick out the wanted mft
struct S_NTFS_FILE_REC *CNTFS::GetMFT(unsigned index) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  
  // is the requested index already in our cache?
  if ((index < m_mft_cur) || (index >= (m_mft_cur + NTFS_MFT_CACHE_SIZE))) {
    WriteMFT();
    m_mft_cur = (index / NTFS_MFT_CACHE_SIZE) * NTFS_MFT_CACHE_SIZE;
    dlg->ReadFromFile(m_mft_buffer, m_mft_lba + ((m_mft_cur * m_rec_size) / bpb->bytes_per_sect), ((NTFS_MFT_CACHE_SIZE * m_rec_size) / bpb->bytes_per_sect));
    FixupMFT(TRUE);
  }
  
  return (struct S_NTFS_FILE_REC *) ((BYTE *) m_mft_buffer + ((index - m_mft_cur) * m_rec_size));
}  

void CNTFS::FixupMFT(const BOOL read) {
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) m_bpb_buffer;
  struct S_NTFS_FILE_REC *file_rec;
  unsigned i = (m_mft_cur == 0) ? 1 : 0;  // skip the first MFT (MFT_MFT)
  WORD *fixup;
  
  if (read) {
    // reading it in, so fixup the buffer
    for (; i<NTFS_MFT_CACHE_SIZE; i++) {
      file_rec = (struct S_NTFS_FILE_REC *) ((BYTE *) m_mft_buffer + (i * m_rec_size));
      if (file_rec->magic == FILE_REC_MAGIC) {
        fixup = (WORD *) ((BYTE *) file_rec + file_rec->fixup_off);
        for (unsigned j=1; j<file_rec->fixup_count; j++) {
          WORD *eos = (WORD *) ((BYTE *) file_rec + ((j * bpb->bytes_per_sect) - 2));
          if (*eos == fixup[0])
            *eos = fixup[j];
          else
            return;
        }
      } else
        break;
    }
  } else {
    // writing it out, so create the fix ups
    for (; i<NTFS_MFT_CACHE_SIZE; i++) {
      file_rec = (struct S_NTFS_FILE_REC *) ((BYTE *) m_mft_buffer + (i * m_rec_size));
      if (file_rec->magic == FILE_REC_MAGIC) {
        fixup = (WORD *) ((BYTE *) file_rec + file_rec->fixup_off);
        for (unsigned j=1; j<file_rec->fixup_count; j++) {
          WORD *eos = (WORD *) ((BYTE *) file_rec + ((j * bpb->bytes_per_sect) - 2));
          *eos = fixup[0];
        }
      } else
        break;
    }
  }
}

BOOL CNTFS::GetFileAttrbs(struct S_NTFS_FILE_REC *file_rec, char *filename_str, DWORD64 *date_time, DWORD64 *fsize, DWORD *attribs, DWORD64 *parent) {
  struct S_NTFS_ATTR *attrb;
  struct S_NTFS_ATTR_RES *attrb_res;
  struct S_NTFS_ATTR_NONRES *attrb_nres;
  struct S_NTFS_ATTR_FILENAME *filename;
  struct S_NTFS_ATTR_STANDARD *stnds;
  
  if (file_rec->magic == FILE_REC_MAGIC) {
    attrb = (struct S_NTFS_ATTR *) ((BYTE *) file_rec + file_rec->attribs_off);
    BYTE filename_last_type = 0xFF;
    if ((attrb->type == NTFS_ATTR_EOA) || (attrb->len == 0))
      return FALSE;
    while ((attrb->type != NTFS_ATTR_EOA) && (attrb->len > 0)) {
      attrb_res = (struct S_NTFS_ATTR_RES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
      attrb_nres = (struct S_NTFS_ATTR_NONRES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
      switch (attrb->type) {
        case NTFS_ATTR_STANDARD:
          stnds = (struct S_NTFS_ATTR_STANDARD *) ((BYTE *) attrb + attrb_res->data_off);
          if (date_time) *date_time = stnds->file_times.last_mod;
          break;
          
        case NTFS_ATTR_FILENAME:
          if (attrb->non_res_flag) {
            AfxMessageBox(" Filename: not resident");
          } else {
            filename = (struct S_NTFS_ATTR_FILENAME *) ((BYTE *) attrb + attrb_res->data_off);
            if (parent) *parent = filename->file_ref_parent;
            if ((filename_last_type != 1) && (filename_last_type != 3)) {
              filename_last_type = filename->filename_space;
              if (attribs) *attribs = (DWORD) filename->flags;
              if (filename_str) {
                memset(filename_str, 0, 256);
                WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) ((BYTE *) filename + sizeof(struct S_NTFS_ATTR_FILENAME)), filename->filename_len, filename_str, 255, NULL, NULL);
              }
            }
          }
          break;
          
        case NTFS_ATTR_DATA:
          if (fsize) {
            if (!attrb->non_res_flag) *fsize = attrb_res->data_len;
            else                      *fsize = attrb_nres->real_size;
          }
          break;
      }
      
      attrb = (struct S_NTFS_ATTR *) ((BYTE *) attrb + attrb->len);
      if ((BYTE *) attrb > (BYTE *) file_rec + file_rec->record_len) {
        //AfxMessageBox(" Error:  Went passed end of filerec data...");
        break;
      }
    }
    return TRUE;
  }
  
  return FALSE;
}

void CNTFS::OnShowSysMft() {
  m_mft_start = IsDlgButtonChecked(IDC_SHOW_SYS_MFT) ? 0 : STARTING_MFT;
  Start(m_lba, m_size, m_color, m_index, FALSE);
}

void CNTFS::OnErase() {
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
