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

// Ext2Entry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Ext2Entry.h"

#include "Ext2.h"
#include "Attribute.h"
#include "LeanTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExt2Entry dialog
CExt2Entry::CExt2Entry(CWnd* pParent /*=NULL*/)
  : CDialog(CExt2Entry::IDD, pParent) {
  //{{AFX_DATA_INIT(CExt2Entry)
  m_atime = _T("");
  m_blocks = _T("");
  m_ctime = _T("");
  m_dir_acl = _T("");
  m_dtime = _T("");
  m_f_addr = _T("");
  m_file_acl = _T("");
  m_flags = _T("");
  m_generation = _T("");
  m_gid = _T("");
  m_link_count = _T("");
  m_mode = _T("");
  m_mtime = _T("");
  m_osd1 = _T("");
  m_size = _T("");
  m_uid = _T("");
  m_sym_link = _T("");
  m_dbl_indirect = _T("");
  m_depth = _T("");
  m_entries = _T("");
  m_extent_gen = _T("");
  m_indirect = _T("");
  m_magic = _T("");
  m_max = _T("");
  m_trp_indirect = _T("");
  //}}AFX_DATA_INIT
}

void CExt2Entry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExt2Entry)
  DDX_Control(pDX, IDC_LIST, m_list);
  DDX_Text(pDX, IDC_ATIME, m_atime);
  DDX_Text(pDX, IDC_BLOCKS, m_blocks);
  DDX_Text(pDX, IDC_CTIME, m_ctime);
  DDX_Text(pDX, IDC_DIR_ACL, m_dir_acl);
  DDX_Text(pDX, IDC_DTIME, m_dtime);
  DDX_Text(pDX, IDC_F_ADDR, m_f_addr);
  DDX_Text(pDX, IDC_FILE_ACL, m_file_acl);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_GENERATION, m_generation);
  DDX_Text(pDX, IDC_GID, m_gid);
  DDX_Text(pDX, IDC_LINK_COUNT, m_link_count);
  DDX_Text(pDX, IDC_MODE, m_mode);
  DDX_Text(pDX, IDC_MTIME, m_mtime);
  DDX_Text(pDX, IDC_OSD1, m_osd1);
  DDX_Text(pDX, IDC_FSIZE, m_size);
  DDX_Text(pDX, IDC_UID, m_uid);
  DDX_Text(pDX, IDC_SYMBOLIC_LINK, m_sym_link);
  DDV_MaxChars(pDX, m_sym_link, 60);
  DDX_Text(pDX, IDC_DBL_INDIRECT, m_dbl_indirect);
  DDX_Text(pDX, IDC_DEPTH, m_depth);
  DDX_Text(pDX, IDC_ENTRIES, m_entries);
  DDX_Text(pDX, IDC_EXTENT_GEN, m_extent_gen);
  DDX_Text(pDX, IDC_INDIRECT, m_indirect);
  DDX_Text(pDX, IDC_MAGIC, m_magic);
  DDX_Text(pDX, IDC_MAX, m_max);
  DDX_Text(pDX, IDC_TRP_INDIRECT, m_trp_indirect);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExt2Entry, CDialog)
  //{{AFX_MSG_MAP(CExt2Entry)
  ON_BN_CLICKED(IDC_DOMODE, OnDomode)
  ON_BN_CLICKED(IDC_DOFLAGS, OnDoflags)
  ON_BN_CLICKED(IDC_ACC_TIME_NOW, OnAccTimeNow)
  ON_BN_CLICKED(IDC_CRE_TIME_NOW, OnCreTimeNow)
  ON_BN_CLICKED(IDC_MOD_TIME_NOW, OnModTimeNow)
  ON_BN_CLICKED(IDC_DEL_TIME_NOW, OnDelTimeNow)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExt2Entry message handlers
BOOL CExt2Entry::OnInitDialog() {
  CString cs;
  unsigned i;
  
  m_atime.Format("%i", m_inode.atime);
  m_blocks.Format("%I64i", ((DWORD64) m_inode.blocks_hi << 32) | m_inode.blocks);
  m_ctime.Format("%i", m_inode.ctime);
  m_dir_acl.Format("%i", m_inode.dir_acl);
  m_dtime.Format("%i", m_inode.dtime);
  m_f_addr.Format("%i", m_inode.faddr);
  m_file_acl.Format("%I64i", ((DWORD64) m_inode.file_acl_high << 32) | m_inode.file_acl);
  m_flags.Format("0x%08X", m_inode.flags);
  m_generation.Format("%i", m_inode.generation);
  m_gid.Format("%i", ((DWORD) m_inode.gid_high << 16) | m_inode.gid);
  m_link_count.Format("%i", m_inode.links_count);
  m_mode.Format("0x%04X", m_inode.mode);
  m_mtime.Format("%i", m_inode.mtime);
  m_osd1.Format("%i", m_inode.osd1);
  m_size.Format("%i", m_inode.size);
  m_uid.Format("%i", ((DWORD) m_inode.uid_high << 16) | m_inode.uid);

  if ((m_inode.mode & EXT2_S_IFMT) == EXT2_S_IFLNK) {
    GetDlgItem(IDC_SYMBOLIC_LINK)->EnableWindow(TRUE);
    // if m_inode.size > 60, it is in the data portion of the file????
    m_sym_link = m_inode.symbolic_link;
  } else {
    if (m_inode.flags & EXT4_EXTENTS_FL) {
      GetDlgItem(IDC_BLOCKS_FRAME)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_MAGIC)->EnableWindow(TRUE);
      GetDlgItem(IDC_ENTRIES)->EnableWindow(TRUE);
      GetDlgItem(IDC_MAX)->EnableWindow(TRUE);
      GetDlgItem(IDC_DEPTH)->EnableWindow(TRUE);
      GetDlgItem(IDC_EXTENT_GEN)->EnableWindow(TRUE);
      // extents
      m_magic.Format("0x%04X", m_inode.extents.extent_hdr.magic);
      m_entries.Format("%i", m_inode.extents.extent_hdr.entries);
      m_max.Format("%i", m_inode.extents.extent_hdr.max);
      m_depth.Format("%i", m_inode.extents.extent_hdr.depth);
      m_extent_gen.Format("%i", m_inode.extents.extent_hdr.generation);
    } else {
      GetDlgItem(IDC_EXTENTS_FRAME)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_INDIRECT)->EnableWindow(TRUE);
      GetDlgItem(IDC_DBL_INDIRECT)->EnableWindow(TRUE);
      GetDlgItem(IDC_TRP_INDIRECT)->EnableWindow(TRUE);
      m_indirect.Format("%i", m_inode.block_array.indirect_block);
      m_dbl_indirect.Format("%i", m_inode.block_array.dbl_indirect_block);
      m_trp_indirect.Format("%i", m_inode.block_array.trpl_indirect_block);
    }
  }
  
  CDialog::OnInitDialog();
  
  // we have to add the strings to m_list after OnInitDialog()
  //  because the list object is not made until after OnInitDialog()
  if ((m_inode.mode & EXT2_S_IFMT) == EXT2_S_IFLNK) {
    m_list.ShowWindow(SW_HIDE);
  } else {
    if (m_inode.flags & EXT4_EXTENTS_FL) {
      if (m_inode.extents.extent_hdr.depth == 0) {
        struct S_EXT3_EXTENT *extent = (struct S_EXT3_EXTENT *) m_inode.extents.extents;
        for (i=0; i<m_inode.extents.extent_hdr.entries; i++) {
          cs.Format("%I64i, %i (%i)", 0, ((DWORD64) extent[i].start_hi << 32) + extent[i].start, extent[i].len, extent[i].block);
          m_list.AddString(cs);
        }
      } else {
        //struct S_EXT3_EXTENT_IDX *extent_idx = (struct S_EXT3_EXTENT_IDX *) m_inode.extents.extents;
        m_list.AddString("TODO: S_EXT3_EXTENT_IDX");
      }    
    } else {
      unsigned cnt = m_inode.blocks / m_sectors_per_block;
      for (i=0; i<cnt && i<12; i++) {
        cs.Format("%i", m_inode.block_array.block[i]);
        m_list.AddString(cs);
      }
    }
  }
  
  return TRUE;
}

void CExt2Entry::OnOK() {
  
  m_inode.atime = convert32(m_atime);
  m_inode.blocks = (DWORD) (convert64(m_blocks) & 0xFFFFFFFF);
  m_inode.blocks_hi = (WORD) (convert64(m_blocks) >> 32);
  m_inode.ctime = convert32(m_ctime);
  m_inode.dir_acl = convert32(m_dir_acl);
  m_inode.dtime = convert32(m_dtime);
  m_inode.faddr = convert32(m_f_addr);
  m_inode.file_acl = (DWORD) (convert64(m_file_acl) & 0xFFFFFFFF);
  m_inode.file_acl_high = (WORD) (convert64(m_file_acl) >> 32);
  m_inode.flags = convert32(m_flags);
  m_inode.generation = convert32(m_generation);
  m_inode.gid = (WORD) (convert32(m_gid) & 0xFFFF);
  m_inode.gid_high = (WORD) (convert32(m_gid) >> 16);
  m_inode.links_count = convert16(m_link_count);
  m_inode.mode = convert16(m_mode);
  m_inode.mtime = convert32(m_mtime);
  m_inode.osd1 = convert32(m_osd1);
  m_inode.size = convert32(m_size);
  m_inode.uid = (WORD) (convert32(m_uid) & 0xFFFF);
  m_inode.uid_high = (WORD) (convert32(m_uid) >> 16);
  
  if ((m_inode.mode & EXT2_S_IFMT) == EXT2_S_IFLNK) {
    strcpy((char *) m_inode.symbolic_link, m_sym_link.Left(59));
  } else {
    if (m_inode.flags & EXT4_EXTENTS_FL) {
      m_inode.extents.extent_hdr.magic = convert16(m_magic);
      m_inode.extents.extent_hdr.entries = convert16(m_entries);
      m_inode.extents.extent_hdr.max = convert16(m_max);
      m_inode.extents.extent_hdr.depth = convert16(m_depth);
      m_inode.extents.extent_hdr.generation = convert32(m_extent_gen);
    } else {
      m_inode.block_array.indirect_block = convert32(m_indirect);
      m_inode.block_array.dbl_indirect_block = convert32(m_dbl_indirect);
      m_inode.block_array.trpl_indirect_block = convert32(m_trp_indirect);
    }
  }
  
  CDialog::OnOK();
}

struct S_ATTRIBUTES ext2_inode_modes[] = {
                        //            |                               | <- max
  { EXT2_S_IFSOCK, EXT2_S_IFMT,    0, "*socket"                   , { 1, 2, 3, 4, 5, 6, -1 } },
  { EXT2_S_IFLNK,  EXT2_S_IFMT,    1, "*symbolic link"            , { 0, 2, 3, 4, 5, 6, -1 } },
  { EXT2_S_IFREG,  EXT2_S_IFMT,    2, "*regular file"             , { 0, 1, 3, 4, 5, 6, -1 } },
  { EXT2_S_IFBLK,  EXT2_S_IFMT,    3, "*block device"             , { 0, 1, 2, 4, 5, 6, -1 } },
  { EXT2_S_IFDIR,  EXT2_S_IFMT,    4, "*directory"                , { 0, 1, 2, 3, 5, 6, -1 } },
  { EXT2_S_IFCHR,  EXT2_S_IFMT,    5, "*character device"         , { 0, 1, 2, 3, 4, 6, -1 } },
  { EXT2_S_IFIFO,  EXT2_S_IFMT,    6, "*fifo"                     , { 0, 1, 2, 3, 4, 5, -1 } },
  { EXT2_S_ISUID,  EXT2_S_ISUID,   7, "SUID"                      , { 8, 9, -1 } },
  { EXT2_S_ISGID,  EXT2_S_ISGID,   8, "SGID"                      , { 7, 9, -1 } },
  { EXT2_S_ISVTX,  EXT2_S_ISVTX,   9, "sticky bit"                , { 7, 8, -1 } },
  { EXT2_S_IRUSR,  EXT2_S_IRUSR,  10, "read"                      , {   } },
  { EXT2_S_IWUSR,  EXT2_S_IWUSR,  11, "write"                     , {   } },
  { EXT2_S_IXUSR,  EXT2_S_IXUSR,  12, "execute"                   , {   } },
  { EXT2_S_IRGRP,  EXT2_S_IRGRP,  13, "read"                      , {   } },
  { EXT2_S_IWGRP,  EXT2_S_IWGRP,  14, "write"                     , {   } },
  { EXT2_S_IXGRP,  EXT2_S_IXGRP,  15, "execute"                   , {   } },
  { EXT2_S_IROTH,  EXT2_S_IROTH,  16, "read"                      , {   } },
  { EXT2_S_IWOTH,  EXT2_S_IWOTH,  17, "write"                     , {   } },
  { EXT2_S_IXOTH,  EXT2_S_IXOTH,  18, "execute"                   , {   } },
  { 0,               (DWORD) -1,  -1, NULL                        , {-1, } }
};

void CExt2Entry::OnDomode() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_MODE, cs);
  dlg.m_title = "Mode Flags";
  dlg.m_attrib = convert16(cs);
  dlg.m_attributes = ext2_inode_modes;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_MODE, cs);
  }
}

struct S_ATTRIBUTES ext2_inode_flags[] = {
                                      //            |                               | <- max
  { EXT2_SECRM_FL,      EXT2_SECRM_FL,           0, "Secure deletion"                , {-1, } },
  { EXT2_UNRM_FL,       EXT2_UNRM_FL,            1, "Undelete"                       , {-1, } },
  { EXT2_COMPR_FL,      EXT2_COMPR_FL,           2, "Compress file"                  , {-1, } },
  { EXT2_SYNC_FL,       EXT2_SYNC_FL,            3, "Synchronous updates"            , {-1, } },
  { EXT2_IMMUTABLE_FL,  EXT2_IMMUTABLE_FL,       4, "Immutable file"                 , {-1, } },
  { EXT2_APPEND_FL,     EXT2_APPEND_FL,          5, "writes to file may only append" , {-1, } },
  { EXT2_NODUMP_FL,     EXT2_NODUMP_FL,          6, "do not dump file"               , {-1, } },
  { EXT2_NOATIME_FL,    EXT2_NOATIME_FL,         7, "do not update atime"            , {-1, } },
  { EXT2_DIRTY_FL,      EXT2_DIRTY_FL,           8, "Dirty"                          , {-1, } },
  { EXT2_COMPRBLK_FL,   EXT2_COMPRBLK_FL,        9, "One/more compressed clusters"   , {-1, } },
  { EXT2_NOCOMP_FL,     EXT2_NOCOMP_FL,         10, "Don't compress"                 , {-1, } },
  { EXT2_ECOMPR_FL,     EXT2_ECOMPR_FL,         11, "Compression error"              , {-1, } },
  { EXT2_BTREE_FL,      EXT2_BTREE_FL,          12, "btree format dir"               , {-1, } },
  { EXT2_IMAGIC_FL,     EXT2_IMAGIC_FL,         13, "IMagic"                         , {-1, } },
  { EXT3_JOURNAL_DATA_FL, EXT3_JOURNAL_DATA_FL, 14, "file data should be journaled"  , {-1, } },
  { EXT2_NOTAIL_FL,     EXT2_NOTAIL_FL,         15, "file tail should not be merged" , {-1, } },
  { EXT2_DIRSYNC_FL,    EXT2_DIRSYNC_FL,        16, "Synchronous dir modifications"  , {-1, } },
  { EXT2_TOPDIR_FL,     EXT2_TOPDIR_FL,         17, "Top of directory hierarchies"   , {-1, } },
  { EXT4_HUGE_FILE_FL,  EXT4_HUGE_FILE_FL,      18, "Set to each huge file"          , {-1, } },
  { EXT4_EXTENTS_FL,    EXT4_EXTENTS_FL,        19, "Inode uses extents"             , {-1, } },
  { EXT2_RESERVED_FL,   EXT2_RESERVED_FL,       20, "reserved for ext2 lib"          , {-1, } },
  { 0,                        (DWORD) -1,       -1, NULL                             , {-1, } }
};

void CExt2Entry::OnDoflags() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FLAGS, cs);
  dlg.m_title = "Ext2 Flags";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_inode_flags;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%08X", dlg.m_attrib);
    SetDlgItemText(IDC_FLAGS, cs);
  }
}

void CExt2Entry::OnAccTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Access Time";
  dlg.m_divisor = 1;
  GetDlgItemText(IDC_ATIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_ATIME, dlg.m_lean_time);
}

void CExt2Entry::OnCreTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Create Time";
  dlg.m_divisor = 1;
  GetDlgItemText(IDC_CTIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_CTIME, dlg.m_lean_time);
}

void CExt2Entry::OnModTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Mod Time";
  dlg.m_divisor = 1;
  GetDlgItemText(IDC_MTIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_MTIME, dlg.m_lean_time);
}

void CExt2Entry::OnDelTimeNow() {
  CLeanTime dlg;
  
  dlg.m_title = "Deleted Time";
  dlg.m_divisor = 1;
  GetDlgItemText(IDC_DTIME, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_DTIME, dlg.m_lean_time);
}
