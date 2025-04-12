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

// LeanEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Lean.h"
#include "LeanEntry.h"
#include "LeanTime.h"
#include "LeanEAs.h"
#include "LeanIndirect.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanEntry dialog
CLeanEntry::CLeanEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CLeanEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CLeanEntry)
  m_acc_time = _T("");
  m_attribs = _T("");
  m_cre_time = _T("");
  m_ext_count = _T("");
  m_filesize = _T("");
  m_first_ind = _T("");
  m_fork = _T("");
  m_gid = _T("");
  m_ind_count = _T("");
  m_last_indirect = _T("");
  m_links_count = _T("");
  m_magic = _T("");
  m_mod_time = _T("");
  m_sch_time = _T("");
  m_block_count = _T("");
  m_uid = _T("");
  m_entry_crc = _T("");
  m_name = _T("");
  m_rec_len = _T("");
  m_name_len = _T("");
  m_hidden = 0;
  m_undelete = 0;
  //}}AFX_DATA_INIT
  m_force_fork = FALSE;
}

void CLeanEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanEntry)
  DDX_Control(pDX, IDC_EXT_START, m_ext_start);
  DDX_Control(pDX, IDC_EXT_SIZE, m_ext_size);
  DDX_Control(pDX, IDC_EXT_CRC, m_ext_crc);
  DDX_Text(pDX, IDC_ENTRY_ACC_TIME, m_acc_time);
  DDX_Text(pDX, IDC_ENTRY_ATTRBS, m_attribs);
  DDX_Text(pDX, IDC_ENTRY_CRE_TIME, m_cre_time);
  DDX_Text(pDX, IDC_ENTRY_EXT_COUNT, m_ext_count);
  DDX_Text(pDX, IDC_ENTRY_FILESIZE, m_filesize);
  DDX_Text(pDX, IDC_ENTRY_FIRST_IND, m_first_ind);
  DDX_Text(pDX, IDC_ENTRY_FORK, m_fork);
  DDX_Text(pDX, IDC_ENTRY_GID, m_gid);
  DDX_Text(pDX, IDC_ENTRY_IND_COUNT, m_ind_count);
  DDX_Text(pDX, IDC_ENTRY_LAST_IND, m_last_indirect);
  DDX_Text(pDX, IDC_ENTRY_LINKS_COUNT, m_links_count);
  DDX_Text(pDX, IDC_ENTRY_MAGIC, m_magic);
  DDX_Text(pDX, IDC_ENTRY_MOD_TIME, m_mod_time);
  DDX_Text(pDX, IDC_ENTRY_SCH_TIME, m_sch_time);
  DDX_Text(pDX, IDC_ENTRY_SECT_COUNT, m_block_count);
  DDX_Text(pDX, IDC_ENTRY_UID, m_uid);
  DDX_Text(pDX, IDC_ENTRY_CRC, m_entry_crc);
  DDX_Text(pDX, IDC_ENTRY_REC_LEN, m_rec_len);
  DDX_Text(pDX, IDC_ENTRY_NAME_LEN, m_name_len);
  DDX_Text(pDX, IDC_ENTRY_NAME, m_name);
  DDX_Check(pDX, IDC_HIDDEN, m_hidden);
  DDX_Check(pDX, IDC_UNDELETE, m_undelete);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanEntry, CDialog)
  //{{AFX_MSG_MAP(CLeanEntry)
  ON_LBN_SELCHANGE(IDC_EXT_START, OnSelchangeExtStart)
  ON_LBN_SELCHANGE(IDC_EXT_SIZE, OnSelchangeExtSize)
  ON_LBN_SELCHANGE(IDC_EXT_CRC, OnSelchangeExtCrc)
  ON_BN_CLICKED(IDC_ATTRIBUTE, OnAttribute)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  ON_BN_CLICKED(IDC_RSVD_UPDATE, OnRsvdUpdate)
  ON_BN_CLICKED(IDC_ACC_TIME_NOW, OnAccTimeNow)
  ON_BN_CLICKED(IDC_CRE_TIME_NOW, OnCreTimeNow)
  ON_BN_CLICKED(IDC_MOD_TIME_NOW, OnModTimeNow)
  ON_BN_CLICKED(IDC_SCH_TIME_NOW, OnSchTimeNow)
  ON_BN_CLICKED(IDEAS, OnEas)
  ON_BN_CLICKED(IDINDIRECTS, OnIndirects)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeanEntry message handlers
BOOL CLeanEntry::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
    case IDC_CRC_UPDATE:
      strTipText = "Recalculate CRC of Inode";
      break;
    case IDC_ATTRIBUTE:
      strTipText = "Modify Attributes";
      break;

    case IDC_ACC_TIME_NOW:
    case IDC_SCH_TIME_NOW:
    case IDC_MOD_TIME_NOW:
    case IDC_CRE_TIME_NOW:
      strTipText = "Adjust Timestamp";
      break;

    case IDC_HIDDEN:
      strTipText = "Mark entry as hidden";
      break;

    case IDC_UNDELETE:
      strTipText = "Mark entry deleted (for undelete)";
      break;

    case IDEAS:
      strTipText = "View/Modify Extended Attributes";
      break;

    case IDINDIRECTS:
      strTipText = "View Indirect Structure(s)";
      break;

    case ID_APPLY:
      strTipText = "Save modifications";
      break;
  }

  strncpy(pTTTA->szText, strTipText, 79);
  pTTTA->szText[79] = '\0';  // make sure it is null terminated

  return TRUE; // message was handled
}

BOOL CLeanEntry::OnInitDialog() {
  CDialog::OnInitDialog();

  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_LEAN_BLOCKS extents;
  CString cs;
  unsigned int i;
  
  m_entry_crc.Format("0x%08X", m_inode.checksum);
  m_magic.Format("0x%08X", m_inode.magic);
  m_ext_count.Format("%i", m_inode.extent_count);
  m_ind_count.Format("%i", m_inode.indirect_count);
  m_links_count.Format("%i", m_inode.links_count);
  m_uid.Format("0x%08X", m_inode.uid);
  m_gid.Format("0x%08X", m_inode.gid);
  m_attribs.Format("0x%08X", m_inode.attributes);
  m_filesize.Format("%I64i", m_inode.file_size);
  m_block_count.Format("%I64i", m_inode.block_count);
  m_acc_time.Format("%I64i", m_inode.acc_time);
  m_sch_time.Format("%I64i", m_inode.sch_time);
  m_mod_time.Format("%I64i", m_inode.mod_time);
  m_cre_time.Format("%I64i", m_inode.cre_time);
  m_first_ind.Format("%I64i", m_inode.first_indirect);
  m_last_indirect.Format("%I64i", m_inode.last_indirect);
  m_fork.Format("%I64i", m_inode.fork);

  cs.Format("%02X %02X %02X", m_inode.reserved[0], m_inode.reserved[1], m_inode.reserved[2]);
  SetDlgItemText(IDC_ENTRY_RESVD, (LPCTSTR) cs);
  
  // set the spin control of the LinksCount for up button goes positive, down button goes negative.
  CSpinButtonCtrl *spin = (CSpinButtonCtrl *) GetDlgItem(IDC_SPIN1);
  spin->SetRange(0, UD_MAXVAL);
  
  const unsigned max_extents = (m_parent->m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  m_ext_start.ResetContent();
  m_ext_size.ResetContent();
  m_ext_crc.ResetContent();
  if (m_parent->ReadFileExtents(&extents, m_inode_num) > 0) {
    for (i=0; i<m_inode.extent_count && i<max_extents; i++) {
      cs.Format("%I64i", extents.extent_start[i]);
      m_ext_start.AddString(cs);
      cs.Format("%i", extents.extent_size[i]);
      m_ext_size.AddString(cs);
      if (m_parent->m_use_extended_extents) {
        cs.Format("0x%08X", extents.extent_crc[i]);
        m_ext_crc.AddString(cs);
      }
    }
    m_parent->FreeExtentBuffer(&extents);
  }
  
  // Get the filename of this entry
  if (m_hItem) {
    struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_parent->m_dir_tree.GetDataStruct(m_hItem);
    if (items != NULL) {
      struct S_LEAN_DIRENTRY *root, *cur;
      DWORD64 RootSize;
      DWORD Offset = items->Offset;
      HTREEITEM hParent = m_parent->m_dir_tree.GetParentItem(m_hItem);
      if (hParent != NULL) {
        items = (struct S_LEAN_ITEMS *) m_parent->m_dir_tree.GetDataStruct(hParent);
        if (items != NULL) {
          root = (struct S_LEAN_DIRENTRY *) m_parent->ReadFile(items->Inode, &RootSize);
          if (root) {
            cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + Offset);
            m_rec_len.Format("%i bytes", cur->rec_len * 16);
            m_name_len.Format("%i chars", cur->name_len);
            m_parent->GetName(cur->name, m_name, cur->name_len);
            m_hidden = (cur->type & LEAN_FA_HIDDEN) > 0;
            m_undelete = (cur->type & 0x7F) == LEAN_FT_DELETED;
            free(root);
          }
        }
      }
    }
  } else {
    GetDlgItem(IDC_ENTRY_REC_LEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_NAME_LEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY_NAME)->EnableWindow(FALSE);
  }
  
  // if there are no indirects, don't enable the button
  GetDlgItem(IDINDIRECTS)->EnableWindow(m_inode.first_indirect != 0);
  
  // if extended extents is not used, disable the listbox
  GetDlgItem(IDC_EXT_CRC)->ShowWindow(m_parent->m_use_extended_extents);
  GetDlgItem(IDC_EXT_CRC)->SetFont(&dlg->m_DumpFont);

  // send to the dialog
  UpdateData(FALSE);
  cs.Format("Lean Inode: Block %I64i", m_inode_num);
  SetWindowText(cs);

  EnableToolTips(TRUE);

  return TRUE;
}

void CLeanEntry::OnSelchangeExtStart() {
  m_ext_size.SetCurSel(m_ext_start.GetCurSel());
  m_ext_crc.SetCurSel(m_ext_start.GetCurSel());
}

void CLeanEntry::OnSelchangeExtSize() {
  m_ext_start.SetCurSel(m_ext_size.GetCurSel());
  m_ext_crc.SetCurSel(m_ext_size.GetCurSel());
}

void CLeanEntry::OnSelchangeExtCrc() {
  m_ext_start.SetCurSel(m_ext_crc.GetCurSel());
  m_ext_size.SetCurSel(m_ext_crc.GetCurSel());
}

struct S_ATTRIBUTES lean_attrbs[] = {
                                           //            |                               | <- max (col 67)
  { LEAN_ATTR_IXOTH,         LEAN_ATTR_IXOTH,         0, "Other: execute permission"      , {-1, } },
  { LEAN_ATTR_IWOTH,         LEAN_ATTR_IWOTH,         1, "Other: write permission"        , {-1, } },
  { LEAN_ATTR_IROTH,         LEAN_ATTR_IROTH,         2, "Other: read permission"         , {-1, } },
  { LEAN_ATTR_IXGRP,         LEAN_ATTR_IXGRP,         3, "Group: execute permission"      , {-1, } },
  { LEAN_ATTR_IWGRP,         LEAN_ATTR_IWGRP,         4, "Group: write permission"        , {-1, } },
  { LEAN_ATTR_IRGRP,         LEAN_ATTR_IRGRP,         5, "Group: read permission"         , {-1, } },
  { LEAN_ATTR_IXUSR,         LEAN_ATTR_IXUSR,         6, "Owner: execute permission"      , {-1, } },
  { LEAN_ATTR_IWUSR,         LEAN_ATTR_IWUSR,         7, "Owner: write permission"        , {-1, } },
  { LEAN_ATTR_IRUSR,         LEAN_ATTR_IRUSR,         8, "Owner: read permission"         , {-1, } },
  { LEAN_ATTR_ISVTX,         LEAN_ATTR_ISVTX,         9, "*Other: restrict rename/delete" , {-1, } },
  { LEAN_ATTR_ISUID,         LEAN_ATTR_ISUID,        10, "*Other: execute as user id"     , {10, -1, } },
  { LEAN_ATTR_ISGID,         LEAN_ATTR_ISGID,        11, "*Other: execute as group id"    , { 9, -1, } },
  {        (1 << 12),              (1 << 12),        12, "reserved"                       , {-1, } },
  { LEAN_ATTR_SYSTEM,        LEAN_ATTR_SYSTEM,       13, "Warn that this is a system file", {-1, } },
  { LEAN_ATTR_ARCHIVE,       LEAN_ATTR_ARCHIVE,      14, "File changed since last backup" , {-1, } },
  { LEAN_ATTR_SYNC_FL,       LEAN_ATTR_SYNC_FL,      15, "Synchronous updates"            , {-1, } },
  { LEAN_ATTR_NOATIME_FL,    LEAN_ATTR_NOATIME_FL,   16, "Don't update last access time"  , {-1, } },
  { LEAN_ATTR_IMMUTABLE_FL,  LEAN_ATTR_IMMUTABLE_FL, 17, "Don't move file sectors"        , {-1, } },
  { LEAN_ATTR_PREALLOC,      LEAN_ATTR_PREALLOC,     18, "Keep any preallocated sectors"  , {-1, } },
  { LEAN_ATTR_EAS_IN_INODE,  LEAN_ATTR_EAS_IN_INODE, 19, "bytes after inode are EA's"     , {-1, } },
  { LEAN_ATTR_IFREG,         LEAN_ATTR_IFMT,        20, "*File type: regular file"       , {21, 22, 23, -1, } },
  { LEAN_ATTR_IFDIR,         LEAN_ATTR_IFMT,        21, "*File type: directory"          , {20, 22, 23, -1, } },
  { LEAN_ATTR_IFLNK,         LEAN_ATTR_IFMT,        22, "*File type: symbolic link"      , {20, 21, 23, -1, } },
  { LEAN_ATTR_IFFRK,         LEAN_ATTR_IFMT,        23, "*File type: fork"               , {20, 21, 22, -1, } },
  { 0,                            (DWORD) -1,        -1, NULL                             , {-1, } }
};

// display attributes
void CLeanEntry::OnAttribute() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_ENTRY_ATTRBS, cs);
  dlg.m_title = "Entry Attribute";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = lean_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%08X", (DWORD) dlg.m_attrib);
    SetDlgItemText(IDC_ENTRY_ATTRBS, cs);
  }
}

void CLeanEntry::OnCrcUpdate() {
  UpdateData(TRUE);
  
  m_inode.checksum = convert32(m_entry_crc);
  m_inode.magic = convert32(m_magic);
  m_inode.extent_count = convert32(m_ext_count);
  m_inode.indirect_count = convert32(m_ind_count);
  m_inode.links_count = convert32(m_links_count);
  m_inode.uid = convert32(m_uid);
  m_inode.gid = convert32(m_gid);
  m_inode.attributes = convert32(m_attribs);
  m_inode.file_size = convert64(m_filesize);
  m_inode.block_count = convert64(m_block_count);
  m_inode.acc_time = convert64(m_acc_time);
  m_inode.sch_time = convert64(m_sch_time);
  m_inode.mod_time = convert64(m_mod_time);
  m_inode.cre_time = convert64(m_cre_time);
  m_inode.first_indirect = convert64(m_first_ind);
  m_inode.last_indirect = convert64(m_last_indirect);
  m_inode.fork = convert64(m_fork);
  
  DWORD crc = m_parent->computeChecksum(&m_inode, LEAN_INODE_SIZE, TRUE);
  m_entry_crc.Format("0x%08X", crc);
  SetDlgItemText(IDC_ENTRY_CRC, m_entry_crc);
}

void CLeanEntry::OnRsvdUpdate() {
  memset(m_inode.reserved, 0, 3);
  SetDlgItemText(IDC_ENTRY_RESVD, "00 00 00");
}

void CLeanEntry::OnAccTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Access Time";
  dlg.m_divisor = 1000000;
  GetDlgItemText(IDC_ENTRY_ACC_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_ENTRY_ACC_TIME, dlg.m_lean_time);
}

void CLeanEntry::OnCreTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Creation Time";
  dlg.m_divisor = 1000000;
  GetDlgItemText(IDC_ENTRY_CRE_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_ENTRY_CRE_TIME, dlg.m_lean_time);
}

void CLeanEntry::OnModTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Modification Time";
  dlg.m_divisor = 1000000;
  GetDlgItemText(IDC_ENTRY_MOD_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_ENTRY_MOD_TIME, dlg.m_lean_time);
}

void CLeanEntry::OnSchTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "SCH Time";
  dlg.m_divisor = 1000000;
  GetDlgItemText(IDC_ENTRY_SCH_TIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_ENTRY_SCH_TIME, dlg.m_lean_time);
}

// load and display the Inode's EAs
// We use the 'Implementation Specific' field in the signature dword to store a type
//  so that we can display and use this type within this app.
void CLeanEntry::OnEas() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) calloc(m_parent->m_block_size, 1);
  DWORD64 Size = 0, More;
  BYTE *p;
  int pos, len, buff_len, i;

  // Read in the whole Inode Block
  struct S_LEAN_INODE *inode_buf = (struct S_LEAN_INODE *) malloc(m_parent->m_block_size);
  dlg->ReadBlocks(inode_buf, m_parent->m_lba, m_inode_num, m_parent->m_block_size, 1);
  
  // does the inode contain the EA's
  if (inode_buf->attributes & LEAN_ATTR_EAS_IN_INODE) {
    Size = (m_parent->m_block_size - LEAN_INODE_SIZE);
    memcpy(buffer, (BYTE *) inode_buf + LEAN_INODE_SIZE, (size_t) Size);
  }
  
  // are there any (more) in the fork?
  if (inode_buf->fork > 0) {
    p = (BYTE *) m_parent->ReadFile(inode_buf->fork, &More);
    if (p) {
      buffer = (BYTE *) realloc(buffer, (size_t) (Size + More));
      memcpy(buffer + Size, p, (size_t) More);
      free(p);
    }
    Size += More;
  }
  
  // All of the Extended Attributes are now in 'buffer' with 'Size' bytes in length

  // retrieve the items
  p = buffer;
  DWORD dword;  // NNRVVVVV where NN = name length, R= type, and VVVVV = length of attribute, in bytes

  // allocate the items we use (including constructing the CStrings)
  struct S_EA_STRUCT *ea_struct = (struct S_EA_STRUCT *) calloc(MAX_EA_STRUCT_ENTRIES * sizeof(struct S_EA_STRUCT), 1);
  if (ea_struct == NULL) {
    AfxMessageBox("Not enough memory"); // Kind of redundant. If we don't have enough memory, Windows won't be able to create a MESSAGE_BOX anyway...
    free(buffer);
    free(inode_buf);
    return;
  }
  
  // initialize the CString csName in the allocated memory above
  for (i=0; i<MAX_EA_STRUCT_ENTRIES; i++)
    CString::Construct(&ea_struct[i].csName);

  int EntryCount = 0;
  DWORD NameLen;
  while (p < (buffer + Size)) {
    dword = * (DWORD *) p;
    NameLen = (dword & 0xFF000000) >> 24;
    ea_struct[EntryCount].Type = (BYTE) ((dword & 0x00F00000) >> 20);
    ea_struct[EntryCount].AttribLen = (dword & 0x000FFFFF);
    len = 0;

    // if NN == 0, this is an empty entry and VVVVV = length of whole entry EXCLUDING dword header
    if (NameLen > 0) {
      m_parent->GetName(&p[4], ea_struct[EntryCount].csName, NameLen);
      len = m_parent->GetNameLen(ea_struct[EntryCount].csName);
      if (ea_struct[EntryCount].AttribLen > 0) {
        if (ea_struct[EntryCount].AttribLen > MAX_EA_STRUCT_DATA_SIZE) {
          AfxMessageBox("Entry has more than MAX_EA_STRUCT_DATA_SIZE bytes in data");
          ea_struct[EntryCount].AttribLen = MAX_EA_STRUCT_DATA_SIZE;
        }
        memcpy(ea_struct[EntryCount].Data, &p[4] + len, ea_struct[EntryCount].AttribLen);
      }
      len += ea_struct[EntryCount].AttribLen;
      if (++EntryCount == MAX_EA_STRUCT_ENTRIES) {
        AfxMessageBox("We only support up to MAX_EA_STRUCT_ENTRIES entries...");
        break;
      }
    } else if (ea_struct[EntryCount].AttribLen == 0)
      break;
    else
      len = ea_struct[EntryCount].AttribLen;

    p += (sizeof(DWORD) + len + 3) & ~3;
  }
  free(buffer);
  
  // display the entries, allowing changes if needed
  CLeanEAs LeanEAs;
  LeanEAs.m_ea_struct = ea_struct;
  LeanEAs.m_count = EntryCount;
  LeanEAs.m_force_fork = m_force_fork;
  LeanEAs.DoModal();
  m_force_fork = LeanEAs.m_force_fork;

  // write back the entries
  i = 0;
  if (inode_buf->attributes & LEAN_ATTR_EAS_IN_INODE) {
    buff_len = m_parent->m_block_size - LEAN_INODE_SIZE;
    buffer = (BYTE *) calloc(buff_len, 1);
    p = buffer;
    if (!LeanEAs.m_force_fork) {
      for (; i<LeanEAs.m_count; i++) {
        NameLen = ea_struct[i].csName.GetLength();
        if (NameLen > 255) NameLen = 255; // make sure we are not over the limit
        dword = m_parent->GetNameLen(ea_struct[i].csName);
        len = (sizeof(DWORD) + dword + ea_struct[i].AttribLen + 3) & ~3;
        if (len <= buff_len) {
          * (DWORD *) p = (NameLen << 24) | (ea_struct[i].Type << 20) | ea_struct[i].AttribLen;
          m_parent->PutName(&p[4], ea_struct[i].csName, -1);
          memcpy(p + sizeof(DWORD) + dword, ea_struct[i].Data, ea_struct[i].AttribLen);
          p += len;
          buff_len -= len;
        } else
          break;
      }
    }
    if (buff_len > 0)
      * (DWORD *) p = buff_len - sizeof(DWORD);
    memcpy((BYTE *) inode_buf + LEAN_INODE_SIZE, buffer, m_parent->m_block_size - LEAN_INODE_SIZE);
    free(buffer);
  }
  
  // do we still have any after (possibly) storing in the Inode's block?
  if (i < LeanEAs.m_count) {
    pos = 0;
    buff_len = m_parent->m_block_size;  // default to one block. Will resize below if needed.
    buffer = (BYTE *) calloc(buff_len, 1);
    for (; i<LeanEAs.m_count; i++) {
      NameLen = ea_struct[i].csName.GetLength();
      dword = m_parent->GetNameLen(ea_struct[i].csName); // update the NameLen field
      len = sizeof(DWORD) + (((dword + ea_struct[i].AttribLen) + 3) & ~3);
      if (pos + len > buff_len)
        buffer = (BYTE *) realloc(buffer, buff_len = pos + len);
      * (DWORD *) &buffer[pos] = (NameLen << 24) | (ea_struct[i].Type << 20) | ea_struct[i].AttribLen;
      m_parent->PutName(&buffer[pos] + sizeof(DWORD), ea_struct[i].csName, -1);
      memcpy(&buffer[pos] + sizeof(DWORD) + dword, ea_struct[i].Data, ea_struct[i].AttribLen);
      pos += len;
    }
    
    struct S_LEAN_BLOCKS extents;
    // allocate the extents for the fork, returning a "struct S_LEAN_BLOCKS"
    m_parent->AllocateExtentBuffer(&extents, LEAN_DEFAULT_COUNT);
    if (inode_buf->fork == 0) {
      if (m_parent->AppendToExtents(&extents, pos, 0, TRUE) == -1) {
        free(buffer);
        free(inode_buf);
        return;
      }
      m_parent->BuildInode(&extents, pos, LEAN_ATTR_IFFRK);
      inode_buf->fork = extents.extent_start[0];
      m_fork.Format("%I64i", inode_buf->fork); // update the Dialog member
    } else
      m_parent->ReadFileExtents(&extents, inode_buf->fork);
    
    m_parent->WriteFile(buffer, &extents, pos);
    m_parent->FreeExtentBuffer(&extents);
    free(buffer);
  } else if (inode_buf->fork > 0) {
    // Delete Fork
    m_parent->DeleteInode(inode_buf->fork);
    inode_buf->fork = 0;
    m_fork = "0"; // update the Dialog member
  }
  
  // We need to update the Inodes Time Stamps
  CTime time = CTime::GetCurrentTime();
  inode_buf->acc_time = 
  inode_buf->mod_time = ((INT64) time.GetTime()) * 1000000;  // uS from 1 Jan 1970
  m_acc_time.Format("%I64i", inode_buf->acc_time); // update the dialog members
  m_mod_time.Format("%I64i", inode_buf->mod_time);
  inode_buf->checksum = m_parent->computeChecksum(inode_buf, LEAN_INODE_SIZE, TRUE);
  dlg->WriteBlocks(inode_buf, m_parent->m_lba, m_inode_num, m_parent->m_block_size, 1);

  // update the extended extent's crcs
  if (m_parent->m_use_extended_extents) {
    m_parent->UpdateFileExtents(inode_buf, TRUE);
    dlg->WriteBlocks(inode_buf, m_parent->m_lba, m_inode_num, m_parent->m_block_size, 1);
  }

  // copy to our local inode buffer
  memcpy(&m_inode, inode_buf, LEAN_INODE_SIZE);
  free(inode_buf);

  // free the array and CStrings
  for (i=0; i<MAX_EA_STRUCT_ENTRIES; i++)
    ea_struct[i].csName.~CString();
  free(ea_struct);

  UpdateData(FALSE); // Send to dialog
}

void CLeanEntry::OnIndirects() {
  CLeanIndirect Indirect;
  
  Indirect.m_current_indirect = m_inode.first_indirect;
  Indirect.m_parent = m_parent;
  Indirect.m_entry_parent = this;
  Indirect.DoModal();
}

// "Apply" button was pressed
void CLeanEntry::OnOK() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_parent->m_block_size);
  DWORD crc;
  
  UpdateData(TRUE);
  
  m_inode.checksum = convert32(m_entry_crc);
  m_inode.magic = convert32(m_magic);
  m_inode.extent_count = convert32(m_ext_count);
  m_inode.indirect_count = convert32(m_ind_count);
  m_inode.links_count = convert32(m_links_count);
  m_inode.uid = convert32(m_uid);
  m_inode.gid = convert32(m_gid);
  m_inode.attributes = convert32(m_attribs);
  m_inode.file_size = convert64(m_filesize);
  m_inode.block_count = convert64(m_block_count);
  m_inode.acc_time = convert64(m_acc_time);
  m_inode.sch_time = convert64(m_sch_time);
  m_inode.mod_time = convert64(m_mod_time);
  m_inode.cre_time = convert64(m_cre_time);
  m_inode.first_indirect = convert64(m_first_ind);
  m_inode.last_indirect = convert64(m_last_indirect);
  m_inode.fork = convert64(m_fork);
  
  // check the CRC
  crc = m_parent->computeChecksum(&m_inode, LEAN_INODE_SIZE, TRUE);
  if (crc != m_inode.checksum)
    if (AfxMessageBox("Checksum not correct. Update?", MB_YESNO, 0) == IDYES)
      m_inode.checksum = crc;
  
  // Save the filename of this entry (and the new attributes?)
  if (m_hItem) {
    struct S_LEAN_ITEMS *items = (struct S_LEAN_ITEMS *) m_parent->m_dir_tree.GetDataStruct(m_hItem);
    if (items != NULL) {
      struct S_LEAN_DIRENTRY *root, *cur;
      DWORD64 RootSize;
      DWORD Offset = items->Offset;
      HTREEITEM hParent = m_parent->m_dir_tree.GetParentItem(m_hItem);
      if (hParent != NULL) {
        items = (struct S_LEAN_ITEMS *) m_parent->m_dir_tree.GetDataStruct(hParent);
        if (items != NULL) {
          struct S_LEAN_BLOCKS extents;
          m_parent->ReadFileExtents(&extents, items->Inode);
          root = (struct S_LEAN_DIRENTRY *) m_parent->ReadFile(items->Inode, &RootSize);
          if (root) {
            cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) root + Offset);
            int len = m_parent->GetNameLen(m_name);
            if (len <= ((cur->rec_len * 16) - LEAN_DIRENTRY_NAME)) {
              // TODO: can we divide it into two entries now??
              m_parent->PutName(cur->name, m_name, len);
              cur->name_len = m_name.GetLength();
            } else {
              // name is larger than what will fit in this current spot
              //  need to expand existing spot (i.e.: enlarge file and move all data toward the new end)
              if (len > 4068) // rare, but catch anyway.
                len = 4068;
              int old_len = cur->rec_len;
              int new_len = ((len + LEAN_DIRENTRY_NAME + 15) / 16);
              int tail_len = (int) (RootSize - Offset - (old_len * 16));
              RootSize += ((new_len - old_len) * 16);
              void *new_root = malloc((size_t) RootSize);
              memcpy(new_root, root, Offset + LEAN_DIRENTRY_NAME);
              
              cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) new_root + Offset);
              m_parent->PutName(cur->name, m_name, len);
              cur->rec_len = (BYTE) new_len;
              cur->name_len = m_name.GetLength();
              
              if (tail_len > 0)
                memcpy((BYTE *) new_root + Offset + (new_len * 16), (BYTE *) root + Offset + (old_len * 16), tail_len);
              
              free(root);
              root = (struct S_LEAN_DIRENTRY *) new_root;
            }

            // update the directory type from the attributes
            // the user could have possibly changed the attributes in the inode, so we need to update the respected values for the directory
            if (m_undelete)
              cur->type = LEAN_FT_DELETED;
            else
              cur->type = (BYTE) (((m_hidden) ? LEAN_FA_HIDDEN : 0) |
                                  ((m_inode.attributes & LEAN_ATTR_IFMT) >> 29));

            // write the file back to the image
            m_parent->WriteFile(root, &extents, RootSize);
            free(root);
          }
          m_parent->FreeExtentBuffer(&extents);
        }
      }
    }
  }
  
  // write the new inode data to the disk
  dlg->ReadBlocks(buffer, m_parent->m_lba, m_inode_num, m_parent->m_block_size, 1);
  memcpy(buffer, &m_inode, LEAN_INODE_SIZE);
  dlg->WriteBlocks(buffer, m_parent->m_lba, m_inode_num, m_parent->m_block_size, 1);
  free(buffer);

  // to refresh the display tree (the user may have updated the filename and/or attribute)
  m_parent->Start(m_parent->m_lba, m_parent->m_size, m_parent->m_color, m_parent->m_index, FALSE);

  CDialog::OnOK();
}
