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

// ISOPrimary.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "ISOImage.h"
#include "ISOPrimary.h"
#include "ISOTime.h"
#include "ISOEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CISOPrimary property page

IMPLEMENT_DYNCREATE(CISOPrimary, CPropertyPage)

CISOPrimary::CISOPrimary() : CPropertyPage(CISOPrimary::IDD) {
  //{{AFX_DATA_INIT(CISOPrimary)
  m_num_lbas = _T("");
  m_opath_table_loc = _T("");
  m_path_table_loc = _T("");
  m_path_table_size = _T("");
  m_sequence_num = _T("");
  m_set_size = _T("");
  m_sys_id = _T("");
  m_vol_id = _T("");
  m_block_size = _T("");
  m_abs_id = _T("");
  m_app_id = _T("");
  m_bib_id = _T("");
  m_copy_id = _T("");
  m_prep_id = _T("");
  m_pub_id = _T("");
  m_set_id = _T("");
  m_struct_ver = _T("");
  //}}AFX_DATA_INIT
}

CISOPrimary::~CISOPrimary() {
}

void CISOPrimary::DoDataExchange(CDataExchange* pDX) {
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CISOPrimary)
  DDX_Control(pDX, IDC_DIR_TREE, m_dir_tree);
  DDX_Text(pDX, IDC_TYPE, m_type);
  DDX_Text(pDX, IDC_CD001, m_cd001);
  DDX_Text(pDX, IDC_VERSION, m_version);
  DDX_Text(pDX, IDC_NUM_LBAS, m_num_lbas);
  DDX_Text(pDX, IDC_OPATH_TABLE_LOC, m_opath_table_loc);
  DDX_Text(pDX, IDC_PATH_TABLE_LOC, m_path_table_loc);
  DDX_Text(pDX, IDC_PATH_TABLE_SIZE, m_path_table_size);
  DDX_Text(pDX, IDC_SEQUENCE_NUM, m_sequence_num);
  DDX_Text(pDX, IDC_SET_SIZE, m_set_size);
  DDX_Text(pDX, IDC_SYS_ID, m_sys_id);
  DDX_Text(pDX, IDC_VOL_ID, m_vol_id);
  DDX_Text(pDX, IDC_BLOCK_SIZE, m_block_size);
  DDX_Text(pDX, IDC_ABS_ID, m_abs_id);
  DDX_Text(pDX, IDC_APP_ID, m_app_id);
  DDX_Text(pDX, IDC_BIB_ID, m_bib_id);
  DDX_Text(pDX, IDC_COPY_ID, m_copy_id);
  DDX_Text(pDX, IDC_PREP_ID, m_prep_id);
  DDX_Text(pDX, IDC_PUB_ID, m_pub_id);
  DDX_Text(pDX, IDC_SET_ID, m_set_id);
  DDX_Text(pDX, IDC_STRUCT_VER, m_struct_ver);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CISOPrimary, CPropertyPage)
  //{{AFX_MSG_MAP(CISOPrimary)
  ON_BN_CLICKED(ID_APPLY, OnApplyB)
  ON_BN_CLICKED(IDC_VOL_DATE, OnVolDate)
  ON_BN_CLICKED(IDC_MOD_DATE, OnModDate)
  ON_BN_CLICKED(IDC_EXP_DATE, OnExpDate)
  ON_BN_CLICKED(IDC_EFFECT_DATE, OnEffectDate)
  ON_NOTIFY(TVN_SELCHANGED, IDC_DIR_TREE, OnSelchangedDirTree)
  ON_BN_CLICKED(ID_ENTRY, OnEntry)
  ON_BN_CLICKED(ID_COPY, OnCopy)
  ON_BN_CLICKED(ID_VIEW, OnView)
  ON_BN_CLICKED(ID_INSERT, OnInsert)
  ON_BN_CLICKED(ID_SEARCH, OnSearch)
  ON_BN_CLICKED(ID_ISO_CHECK, OnISOCheck)
  ON_BN_CLICKED(IDC_EXPAND, OnExpand)
  ON_BN_CLICKED(IDC_COLAPSE, OnCollapse)
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CISOPrimary message handlers
BOOL CISOPrimary::OnInitDialog() {
  CPropertyPage::OnInitDialog();
  
  m_TreeImages.DoCreate();
  m_dir_tree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
  
  return TRUE;
}

BOOL CISOPrimary::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "iso_primary.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

void CISOPrimary::OnSelchangedDirTree(NMHDR* pNMHDR, LRESULT* pResult) {
  NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *) pNMHDR;
  
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  struct S_ISO_ITEMS *items = (struct S_ISO_ITEMS *) m_dir_tree.GetDataStruct(hItem);

  GetDlgItem(ID_ENTRY)->EnableWindow(hItem != NULL);
  GetDlgItem(ID_COPY)->EnableWindow((hItem != NULL) && items->CanCopy);
  GetDlgItem(ID_VIEW)->EnableWindow((hItem != NULL) && (items->Flags & ITEM_IS_FILE));
  GetDlgItem(ID_INSERT)->EnableWindow((hItem != NULL) && (m_dir_tree.IsDir(hItem) != 0));
  GetDlgItem(ID_DELETE)->EnableWindow(FALSE);
  
  *pResult = 0;
}

void CISOPrimary::Start(const DWORD64 lba, DWORD color, BOOL IsNewTab) {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  
  m_is_valid = TRUE;
  m_lba = lba;
  m_size = pvd->num_lbas;
  m_color = color;

  SendToDialog(FALSE);
}

void CISOPrimary::OnVolDate() {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  CISOTime time;
  
  time.Convert(&pvd->vol_date, TRUE);
  if (time.DoModal() == IDOK) {
    time.Convert(&pvd->vol_date, FALSE);
  }
}

void CISOPrimary::OnModDate() {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  CISOTime time;
  
  time.Convert(&pvd->mod_date, TRUE);
  if (time.DoModal() == IDOK) {
    time.Convert(&pvd->mod_date, FALSE);
  }
}

void CISOPrimary::OnExpDate() {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  CISOTime time;
  
  time.Convert(&pvd->exp_date, TRUE);
  if (time.DoModal() == IDOK) {
    time.Convert(&pvd->exp_date, FALSE);
  }
}

void CISOPrimary::OnEffectDate() {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  CISOTime time;
  
  time.Convert(&pvd->val_date, TRUE);
  if (time.DoModal() == IDOK) {
    time.Convert(&pvd->val_date, FALSE);
  }
}

void CISOPrimary::DoRoot(void) {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  
  GetDlgItem(IDC_DIR_TREE)->EnableWindow(TRUE);
  
  // make sure the tree is emtpy
  m_dir_tree.DeleteAllItems();
  m_hRoot = m_dir_tree.Insert("\\", ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, TVI_ROOT);
  m_dir_tree.SetItemState(m_hRoot, TVIS_BOLD, TVIS_BOLD);
  
  UpdateWindow();
  
  // fill the tree with the directory
  struct S_ISO_ROOT *root;
  root = (struct S_ISO_ROOT *) ReadFile(pvd->root.extent_loc, pvd->root.data_len, pvd->root.flags, TRUE);
  if (root) {
    SaveItemInfo(m_hRoot, &pvd->root, ITEM_IS_FOLDER, 0, FALSE);
    CWaitCursor wait; // display a wait cursor
    m_parse_depth_limit = 0;  // start new
    ParseDir(root, pvd->root.data_len, m_hRoot, TRUE);
    m_dir_tree.Expand(m_hRoot, TVE_EXPAND);
    free(root);
    wait.Restore();
    GetDlgItem(IDC_EXPAND)->EnableWindow(TRUE);
    GetDlgItem(IDC_COLAPSE)->EnableWindow(TRUE);
    GetDlgItem(ID_SEARCH)->EnableWindow(TRUE);
  }
}

// d-characters (A-Z, 0-9, _) + '.' and ';' (at most 30 chars not counting ECMA-119 specs)
//const char iso_valid_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.;~";  // we added the '~' char (d1-chars ????)

// ISO/IEC 646 characters ( https://en.wikipedia.org/wiki/ISO/IEC_646 )
// we added the '~' char
const char iso_valid_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!%&'()*+,-./:;<=>?_~\"";

DWORD CISOPrimary::CheckRootEntry(struct S_ISO_ROOT *r) {
  int i;
  
  // first check to see if it is '.' or '..' (the codes for anyway)
  if ((r->fi_len == 1) && (r->ident[0] == 0)) // '.'
    return ISO_NO_ERROR;
  if ((r->fi_len == 1) && (r->ident[0] == 1)) // '..'
    return ISO_NO_ERROR;
  
  // now see if it is the actual . or .. entry.
  // The ISO 9660 specs, section 6.8.2.2 state it is a 0 and a 1, not a . and a ..
  if ((r->fi_len == 1) && (r->ident[0] == '.')) // '.'
      return ISO_BAD_IDENT;
  if ((r->fi_len == 2) && (r->ident[0] == '.') && (r->ident[1] == '.')) // '..'
      return ISO_BAD_IDENT;
  
  // if bad id length, return error
  // I don't know why we limit it to 30 chars..., so I commented this out for now.
  if ((r->fi_len == 0) /*|| (r->fi_len > 30) */)
    return ISO_BAD_ID_LEN;
  
  // if we find a null in the char stream before length of id is found, error
  // or if we find a non-d char in name, return error
  for (i=0; i<r->fi_len; i++) {
    if (r->ident[i] == 0)  // the NULL byte (if present) is not included in the length field
      break;
    if (strchr(iso_valid_chars, toupper(r->ident[i])) == NULL)
      return ISO_BAD_IDENT;
  }
  if (i != r->fi_len)
    return ISO_BAD_IDENT;
  
  return ISO_NO_ERROR;
}

bool CISOPrimary::ParseDir(struct S_ISO_ROOT *root, DWORD datalen, HTREEITEM parent, BOOL IsRoot) {
  struct S_ISO_ROOT *r = root, *sub;
  HTREEITEM hItem;
  char szName[256+1];
  CString name;
  DWORD ErrorCode;
  unsigned ErrorCount = 0;
  unsigned ErrorMax = AfxGetApp()->GetProfileInt("Settings", "MaxErrorCount", 10);

  // catch to make sure we don't simply repeatedly recurse on '.' and '..' entries
  if (++m_parse_depth_limit > MAX_DIR_PARSE_DEPTH) {
    AfxMessageBox("Recursive ParseDir() too deep.  Stopping");
    return FALSE;
  }
  
  while ((BYTE *) r < ((BYTE *) root + datalen)) {
    if (r->len == 0)
      break;

    ErrorCode = CheckRootEntry(r);
    if (ErrorCode != ISO_NO_ERROR) {
      hItem = m_dir_tree.Insert("Invalid Entry", ITEM_IS_FILE, IMAGE_DELETE, IMAGE_DELETE, parent);
      if (hItem == NULL) { m_parse_depth_limit--; return TRUE; }
      SaveItemInfo(hItem, r, 0, 0, FALSE);
      if (++ErrorCount >= ErrorMax)
        break;
    } else {
      // retrieve the name.
      if ((r->fi_len == 1) && (r->ident[0] == 0)) {
        name = ".";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
        if (hItem == NULL) { m_parse_depth_limit--; return TRUE; }
        SaveItemInfo(hItem, r, ITEM_IS_FOLDER, 0, FALSE);
      } else if ((r->fi_len == 1) && (r->ident[0] == 1)) {
        name = "..";
        hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
        if (hItem == NULL) { m_parse_depth_limit--; return TRUE; }
        SaveItemInfo(hItem, r, ITEM_IS_FOLDER, 0, FALSE);
      } else {
        // stop at ';'
        bool b = FALSE;
        for (int i=0; i<r->fi_len; i++) {
          b = (!b && (r->ident[i] == 0x3B));
          if (b)
            r->ident[i] = 0x00;
        }
        
        memcpy(szName, r->ident, r->fi_len);
        szName[r->fi_len] = '\0';
        name = szName;
        if (r->flags & ISO_ROOT_FLAGS_EXISTS)
          name += " (hidden)";
        
        if (r->flags & ISO_ROOT_FLAGS_DIR) {
          hItem = m_dir_tree.Insert(name, ITEM_IS_FOLDER, IMAGE_FOLDER, IMAGE_FOLDER, parent);
          if (hItem == NULL) { m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, r, ITEM_IS_FOLDER, 0, TRUE);
          sub = (struct S_ISO_ROOT *) ReadFile(r->extent_loc, r->data_len, r->flags, FALSE);
          if (sub) {
            if (!ParseDir(sub, r->data_len, hItem, FALSE)) {
              free(sub);
              m_parse_depth_limit--;
              return FALSE;
            }
            free(sub);
          }
        } else {
          hItem = m_dir_tree.Insert(name, ITEM_IS_FILE, IMAGE_FILE, IMAGE_FILE, parent);
          if (hItem == NULL) { m_parse_depth_limit--; return TRUE; }
          SaveItemInfo(hItem, r, ITEM_IS_FILE, 0, TRUE);
        }
      }
    }
    
    r = (struct S_ISO_ROOT *) ((BYTE *) r + r->len);
  }
  
  m_parse_depth_limit--;
  return TRUE;
}

// size = size in bytes
void *CISOPrimary::ReadFile(DWORD extent, DWORD size, BYTE Flags, BOOL IsRoot) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  if (Flags & ISO_ROOT_FLAGS_MULTI_EXT) {
    AfxMessageBox("Is Multi Extent...");
    return NULL;
  }
  
  void *ptr = calloc(size + (MAX_SECT_SIZE - 1), 1);
  if (ptr)
    dlg->ReadFromFile(ptr, extent, (size + (2048 - 1)) / 2048);
  
  return ptr;
}

void CISOPrimary::SaveItemInfo(HTREEITEM hItem, struct S_ISO_ROOT *root, DWORD flags, DWORD ErrorCode, BOOL CanCopy) {
  struct S_ISO_ITEMS *items = (struct S_ISO_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    items->CanCopy = CanCopy;
    items->Flags = flags;
    memcpy(items->RootEntry, root, root->len);
  }
}

void CISOPrimary::OnCopy() {
  char szPath[MAX_PATH];
  CString csPath, csName;
  BOOL IsDir = FALSE;
  
  // get the path from the tree control
  HTREEITEM hItem = m_dir_tree.GetFullPath(NULL, &IsDir, csName, csPath, TRUE);
  if (!hItem) {
    AfxMessageBox("No Item");
    return;
  }
  
  // remove everything from the ';' and after from the name
  int pos = csName.Find(';', 0);
  if (pos > -1)
    csName = csName.Left(pos);
  if (csName.IsEmpty())
    return;
  
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

void CISOPrimary::CopyFile(HTREEITEM hItem, CString csName) {
  struct S_ISO_ITEMS *items = (struct S_ISO_ITEMS *) m_dir_tree.GetDataStruct(hItem);
  if (items) {
    struct S_ISO_ROOT *root = (struct S_ISO_ROOT *) items->RootEntry;
    void *ptr = ReadFile(root->extent_loc, root->data_len, root->flags, FALSE);
    if (ptr) {
      CFile file;
      if (file.Open(csName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) != 0) {
        file.Write(ptr, root->data_len);
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
void CISOPrimary::CopyFolder(HTREEITEM hItem, CString csPath, CString csName) {
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

void CISOPrimary::OnView() {
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

void CISOPrimary::OnInsert() {
  
}

void CISOPrimary::OnSearch() {
  m_dir_tree.Search();
}

void CISOPrimary::OnEntry() {
  HTREEITEM hItem = m_dir_tree.GetSelectedItem();
  CISOEntry ISOEntry;
  
  if (hItem) {
    struct S_ISO_ITEMS *items = (struct S_ISO_ITEMS *) m_dir_tree.GetDataStruct(hItem);
    if (items) {
      struct S_ISO_ROOT *root = (struct S_ISO_ROOT *) items->RootEntry;
      ISOEntry.m_length.Format("%i", root->len);
      ISOEntry.m_attribute.Format("0x%02X", root->e_attrib);
      ISOEntry.m_extent_loc.Format("%i", root->extent_loc);
      ISOEntry.m_data_len.Format("%i", root->data_len);
      ISOEntry.m_flags.Format("0x%02X", root->flags);
      ISOEntry.m_unit_size.Format("%i", root->unit_size);
      ISOEntry.m_gap_size.Format("%i", root->gap_size);
      ISOEntry.m_sequ_num.Format("%i", root->sequ_num);
      ISOEntry.m_fi_len.Format("%i", root->fi_len);
      if ((root->fi_len == 1) && ((root->ident[0] == 0) || (root->ident[0] == 1))) {
        if (root->ident[0] == 0)
          ISOEntry.m_ident = "(zero)";
        else
          ISOEntry.m_ident = "(one)";
      } else {
        memcpy(ISOEntry.m_ident.GetBuffer(root->fi_len + 1), root->ident, root->fi_len);
        ISOEntry.m_ident.ReleaseBuffer(root->fi_len);
        //ISOEntry.m_ident.SetAt(root->fi_len, 0);
      }
      
      ISOEntry.m_year = root->date.since_1900 + 1900;
      ISOEntry.m_month = root->date.month;
      ISOEntry.m_day = root->date.day;
      ISOEntry.m_hour = root->date.hour;
      ISOEntry.m_minute = root->date.min;
      ISOEntry.m_second = root->date.sec;
      ISOEntry.m_gmt_off = root->date.gmt_off;
      
      const int padding = ((root->fi_len % 2) == 0) ? 1 : 0;
      ISOEntry.m_ident_extra_cnt = (root->len - 33 - root->fi_len - padding);
      if (ISOEntry.m_ident_extra_cnt > 0)
        DumpIt(ISOEntry.m_ident_extra, root->ident + root->fi_len + padding, 0, ISOEntry.m_ident_extra_cnt, FALSE);
      if (ISOEntry.DoModal() == IDOK) {
        
      }
    }
  }
}

void CISOPrimary::SendToDialog(const BOOL update) {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  
  m_type = 0;
  m_cd001 = "CD001";
  m_version.Format("%i", pvd->ver);
  
  m_sys_id = pvd->sys_ident;
  m_vol_id = pvd->vol_ident;
  
  m_num_lbas.Format("%i", pvd->num_lbas);
  m_path_table_loc.Format("%i", pvd->pathl_loc);
  m_opath_table_loc.Format("%i", pvd->pathlo_loc);
  m_path_table_size.Format("%i", pvd->path_table_size);
  m_sequence_num.Format("%i", pvd->sequ_num);
  m_block_size.Format("%i", pvd->lba_size);
  m_set_size.Format("%i", pvd->set_size);
  m_struct_ver.Format("%i", pvd->struct_ver);

  memcpy(m_abs_id.GetBuffer(33), pvd->abs_ident, 32); m_abs_id.ReleaseBuffer(32);
  memcpy(m_app_id.GetBuffer(33), pvd->app_ident, 32); m_app_id.ReleaseBuffer(32);
  memcpy(m_bib_id.GetBuffer(33), pvd->bib_ident, 32); m_bib_id.ReleaseBuffer(32);
  memcpy(m_copy_id.GetBuffer(33), pvd->copy_ident, 32); m_copy_id.ReleaseBuffer(32);
  memcpy(m_prep_id.GetBuffer(33), pvd->prep_ident, 32); m_prep_id.ReleaseBuffer(32);
  memcpy(m_pub_id.GetBuffer(33), pvd->pub_ident, 32); m_pub_id.ReleaseBuffer(32);
  memcpy(m_set_id.GetBuffer(33), pvd->set_ident, 32); m_set_id.ReleaseBuffer(32);

  if (update)
    UpdateData(FALSE);
}

void CISOPrimary::ReceiveFromDialog(void) {
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) m_descriptor;
  
  UpdateData(TRUE);
  
  pvd->ver = convert8(m_version);
  
  memset(pvd->sys_ident, 0x20, 32);
  memcpy(pvd->sys_ident, m_sys_id, (m_sys_id.GetLength() < 32) ? m_sys_id.GetLength() : 32);
  memset(pvd->vol_ident, 0x20, 32);
  memcpy(pvd->vol_ident, m_vol_id, (m_vol_id.GetLength() < 32) ? m_vol_id.GetLength() : 32);
  
  pvd->num_lbas = convert32(m_num_lbas); pvd->num_lbas_b = ENDIAN_32U(pvd->num_lbas);
  // TODO: do we need to add pathm_loc to this, sendtodialog, and the actual dialog????
  pvd->pathl_loc = convert32(m_path_table_loc);
  pvd->pathlo_loc = convert32(m_opath_table_loc);
  pvd->path_table_size = convert32(m_path_table_size); pvd->path_table_size_b = ENDIAN_32U(pvd->path_table_size);
  pvd->sequ_num = convert16(m_sequence_num); pvd->sequ_num_b = ENDIAN_16U(pvd->sequ_num);
  pvd->lba_size = convert16(m_block_size); pvd->lba_size_b = ENDIAN_16U(pvd->lba_size);
  pvd->set_size = convert16(m_set_size); pvd->set_size_b = ENDIAN_16U(pvd->set_size);
  pvd->struct_ver = convert8(m_struct_ver);
  
  memset(pvd->abs_ident, 0x20, 32);
  memcpy(pvd->abs_ident, m_abs_id, (m_abs_id.GetLength() < 32) ? m_abs_id.GetLength() : 32);
  
  memset(pvd->app_ident, 0x20, 32);
  memcpy(pvd->app_ident, m_app_id, (m_app_id.GetLength() < 32) ? m_app_id.GetLength() : 32);
  memset(pvd->bib_ident, 0x20, 32);
  memcpy(pvd->bib_ident, m_bib_id, (m_bib_id.GetLength() < 32) ? m_bib_id.GetLength() : 32);
  memset(pvd->copy_ident, 0x20, 32);
  memcpy(pvd->copy_ident, m_copy_id, (m_copy_id.GetLength() < 32) ? m_copy_id.GetLength() : 32);
  memset(pvd->prep_ident, 0x20, 32);
  memcpy(pvd->prep_ident, m_prep_id, (m_prep_id.GetLength() < 32) ? m_prep_id.GetLength() : 32);
  memset(pvd->pub_ident, 0x20, 32);
  memcpy(pvd->pub_ident, m_pub_id, (m_pub_id.GetLength() < 32) ? m_pub_id.GetLength() : 32);
  memset(pvd->set_ident, 0x20, 32);
  memcpy(pvd->set_ident, m_set_id, (m_set_id.GetLength() < 32) ? m_set_id.GetLength() : 32);
}

void CISOPrimary::OnApplyB() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  ReceiveFromDialog();
  
  dlg->WriteToFile(m_descriptor, m_lba, 1);
}

void CISOPrimary::OnISOCheck() {
  AfxMessageBox("TODO:");
}

void CISOPrimary::OnExpand() {
  ExpandIt(&m_dir_tree, TRUE);
}

void CISOPrimary::OnCollapse() {
  ExpandIt(&m_dir_tree, FALSE);
}
