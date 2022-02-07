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

// Ext2Dyn.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "Attribute.h"

#include "Ext2.h"
#include "Ext2Dyn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExt2Dyn dialog
CExt2Dyn::CExt2Dyn(CWnd* pParent /*=NULL*/)
  : CDialog(CExt2Dyn::IDD, pParent) {
  //{{AFX_DATA_INIT(CExt2Dyn)
  m_algo_usage_bitmap = _T("");
  m_block_gr_num = _T("");
  m_ext2_guid = _T("");
  m_feature_comp = _T("");
  m_feature_incomp = _T("");
  m_feature_ro_comp = _T("");
  m_first_inode = _T("");
  m_inode_size = _T("");
  m_last_mount = _T("");
  m_pre_alloc_blocks = _T("");
  m_pre_alloc_dir_blocks = _T("");
  m_pre_alloc_gdt_blocks = _T("");
  m_vol_name = _T("");
  //}}AFX_DATA_INIT
}

void CExt2Dyn::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExt2Dyn)
  DDX_Text(pDX, IDC_ALGO_USAGE_BITMAP, m_algo_usage_bitmap);
  DDX_Text(pDX, IDC_BLOCK_GR_NUM, m_block_gr_num);
  DDX_Text(pDX, IDC_EXT2_GUID, m_ext2_guid);
  DDX_Text(pDX, IDC_FEATURE_COMP, m_feature_comp);
  DDX_Text(pDX, IDC_FEATURE_INCOMP, m_feature_incomp);
  DDX_Text(pDX, IDC_FEATURE_RO_INCOMP, m_feature_ro_comp);
  DDX_Text(pDX, IDC_FIRST_INODE, m_first_inode);
  DDX_Text(pDX, IDC_INODE_SIZE, m_inode_size);
  DDX_Text(pDX, IDC_LAST_MOUNT, m_last_mount);
  DDX_Text(pDX, IDC_PRE_ALLOC_BLOCKS, m_pre_alloc_blocks);
  DDX_Text(pDX, IDC_PRE_ALLOC_DIR_BLOCKS, m_pre_alloc_dir_blocks);
  DDX_Text(pDX, IDC_RESERVED_GDT_BLOCKS, m_pre_alloc_gdt_blocks);
  DDX_Text(pDX, IDC_VOLUME_NAME, m_vol_name);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExt2Dyn, CDialog)
  //{{AFX_MSG_MAP(CExt2Dyn)
  ON_BN_CLICKED(ID_FEATURE_COMP, OnFeatureComp)
  ON_BN_CLICKED(ID_FEATURE_INCOMP, OnFeatureIncomp)
  ON_BN_CLICKED(ID_FEATURE_RO_INCOMP, OnFeatureRoIncomp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExt2Dyn message handlers

struct S_ATTRIBUTES ext2_compat_attrbs[] = {
                                                                //            |                               | <- max
  { EXT2_FEATURE_COMPAT_DIR_PREALLOC,   EXT2_FEATURE_COMPAT_DIR_PREALLOC,  0, "Pre-allocation for new dirs"    , {-1, } },
  { EXT2_FEATURE_COMPAT_IMAGIC_INODES,  EXT2_FEATURE_COMPAT_IMAGIC_INODES, 0, "IMagic Inodes"                  , {-1, } },
  { EXT3_FEATURE_COMPAT_HAS_JOURNAL,    EXT3_FEATURE_COMPAT_HAS_JOURNAL,   0, "An Ext3 journal exists"         , {-1, } },
  { EXT2_FEATURE_COMPAT_EXT_ATTR,       EXT2_FEATURE_COMPAT_EXT_ATTR,      0, "Extended inode attributes"      , {-1, } },
  { EXT3_FEATURE_COMPAT_RESIZE_INO,     EXT3_FEATURE_COMPAT_RESIZE_INO,    0, "Non-standard inode size used"   , {-1, } },
  { EXT3_FEATURE_COMPAT_DIR_INDEX,      EXT3_FEATURE_COMPAT_DIR_INDEX,     0, "Directory indexing (HTree)"     , {-1, } },
  { EXT2_FEATURE_COMPAT_LAZY_BG,        EXT2_FEATURE_COMPAT_LAZY_BG,       0, "Lazy BG"                        , {-1, } },
  { EXT4_FEATURE_COMPAT_EXCLUDE_INODE,  EXT4_FEATURE_COMPAT_EXCLUDE_INODE, 0, "Exclude Inode"                  , {-1, } },
  { EXT4_FEATURE_COMPAT_EXCLUDE_BITMAP, EXT4_FEATURE_COMPAT_EXCLUDE_BITMAP,0, "Exclude Bitmap"                 , {-1, } },
  { 0,                                            (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2Dyn::OnFeatureComp() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FEATURE_COMP, cs);
  dlg.m_title = "Feature Compatible";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_compat_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_FEATURE_COMP, cs);
  }
}

// The fs driver should refuse to mount the fs if any of the indicated feature is unsupported.
struct S_ATTRIBUTES ext2_incompat_attrbs[] = {
                                                                         //            |                               | <- max (col 67)
  { EXT2_FEATURE_INCOMPAT_COMPRESSION,      EXT2_FEATURE_INCOMPAT_COMPRESSION,      0, "Disk/File compression is used"  , {-1, } },
  { EXT2_FEATURE_INCOMPAT_FILETYPE,         EXT2_FEATURE_INCOMPAT_FILETYPE,         0, "file type"                      , {-1, } },
  { EXT3_FEATURE_INCOMPAT_RECOVER,          EXT3_FEATURE_INCOMPAT_RECOVER,          0, "Recover"                        , {-1, } },
  { EXT3_FEATURE_INCOMPAT_JOURNAL_DEV,      EXT3_FEATURE_INCOMPAT_JOURNAL_DEV,      0, "journal device"                 , {-1, } },
  { EXT2_FEATURE_INCOMPAT_META_BG,          EXT2_FEATURE_INCOMPAT_META_BG,          0, "meta bg"                        , {-1, } },
  { EXT3_FEATURE_INCOMPAT_EXTENTS,          EXT3_FEATURE_INCOMPAT_EXTENTS,          0, "extents"                        , {-1, } },
  { EXT4_FEATURE_INCOMPAT_64BIT,            EXT4_FEATURE_INCOMPAT_64BIT,            0, "64-bit"                         , {-1, } },
  { EXT4_FEATURE_INCOMPAT_MMP,              EXT4_FEATURE_INCOMPAT_MMP,              0, "mmp"                            , {-1, } },
  { EXT4_FEATURE_INCOMPAT_FLEX_BG,          EXT4_FEATURE_INCOMPAT_FLEX_BG,          0, "flex bg"                        , {-1, } },
  { EXT4_FEATURE_INCOMPAT_EA_INODE,         EXT4_FEATURE_INCOMPAT_EA_INODE,         0, "EA Inode"                       , {-1, } },
  { EXT4_FEATURE_INCOMPAT_DIRDATA,          EXT4_FEATURE_INCOMPAT_DIRDATA,          0, "Dir Data"                       , {-1, } },
  { EXT4_FEATURE_INCOMPAT_BG_USE_META_CSUM, EXT4_FEATURE_INCOMPAT_BG_USE_META_CSUM, 0, "BG Use Meta CSUM"               , {-1, } },
  { EXT4_FEATURE_INCOMPAT_LARGEDIR,         EXT4_FEATURE_INCOMPAT_LARGEDIR,         0, "Large Dir"                      , {-1, } },
  { EXT4_FEATURE_INCOMPAT_INLINE_DATA,      EXT4_FEATURE_INCOMPAT_INLINE_DATA,      0, "Inline Data"                    , {-1, } },
  { 0,                                                     (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2Dyn::OnFeatureIncomp() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FEATURE_INCOMP, cs);
  dlg.m_title = "Feature Incompatible";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_incompat_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_FEATURE_INCOMP, cs);
  }
}

// If the fs driver doesn't support any of these, it still can read from the system, but shouldn't write to it.
struct S_ATTRIBUTES ext2_compat_ro_attrbs[] = {
                                                                     //            |                               | <- max (col 67)
  { EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER,  EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER,  0, "Sparse Superblock"              , {-1, } },
  { EXT2_FEATURE_RO_COMPAT_LARGE_FILE,    EXT2_FEATURE_RO_COMPAT_LARGE_FILE,    0, "Large files (64-bit file sizes)", {-1, } },
  { EXT2_FEATURE_RO_COMPAT_BTREE_DIR,     EXT2_FEATURE_RO_COMPAT_BTREE_DIR,     0, "Binary tree sorted dir files"   , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_HUGE_FILE,     EXT4_FEATURE_RO_COMPAT_HUGE_FILE,     0, "Huge File"                      , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_GDT_CSUM,      EXT4_FEATURE_RO_COMPAT_GDT_CSUM,      0, "GDT CSum"                       , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_DIR_NLINK,     EXT4_FEATURE_RO_COMPAT_DIR_NLINK,     0, "DIR NLINK"                      , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE,   EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE,   0, "Extra ISize"                    , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT,  EXT4_FEATURE_RO_COMPAT_HAS_SNAPSHOT,  0, "Has Snapshot"                   , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_QUOTA,         EXT4_FEATURE_RO_COMPAT_QUOTA,         0, "Quota"                          , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_BIGALLOC,      EXT4_FEATURE_RO_COMPAT_BIGALLOC,      0, "Big Alloc"                      , {-1, } },
  { EXT4_FEATURE_RO_COMPAT_METADATA_CSUM, EXT4_FEATURE_RO_COMPAT_METADATA_CSUM, 0, "Metadata csum"                  , {-1, } },
  { 0,                                                 (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CExt2Dyn::OnFeatureRoIncomp() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FEATURE_RO_INCOMP, cs);
  dlg.m_title = "Feature Read Only Compatible";
  dlg.m_attrib = convert32(cs);
  dlg.m_attributes = ext2_compat_ro_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("%i", dlg.m_attrib);
    SetDlgItemText(IDC_FEATURE_RO_INCOMP, cs);
  }
}
