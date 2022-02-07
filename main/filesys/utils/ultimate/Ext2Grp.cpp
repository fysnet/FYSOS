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

// Ext2Grp.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "Ext2Grp.h"

#include "Ext2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExt2Grp dialog


CExt2Grp::CExt2Grp(CWnd* pParent /*=NULL*/)
  : CDialog(CExt2Grp::IDD, pParent)
{
  //{{AFX_DATA_INIT(CExt2Grp)
  m_block_bitmap = _T("");
  m_crc = _T("");
  m_flags = _T("");
  m_free_blocks_cnt = _T("");
  m_free_inodes_cnt = _T("");
  m_inode_bitmap = _T("");
  m_inode_table = _T("");
  m_unused_inode = _T("");
  m_used_dir_cnt = _T("");
  //}}AFX_DATA_INIT
  m_cur_group = 0;
}


void CExt2Grp::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExt2Grp)
  DDX_Text(pDX, IDC_BLOCK_BITMAP, m_block_bitmap);
  DDX_Text(pDX, IDC_CRC, m_crc);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_FREE_BLOCKS_CNT, m_free_blocks_cnt);
  DDX_Text(pDX, IDC_FREE_INODES_COUNT, m_free_inodes_cnt);
  DDX_Text(pDX, IDC_INODE_BITMAP, m_inode_bitmap);
  DDX_Text(pDX, IDC_INODE_TABLE, m_inode_table);
  DDX_Text(pDX, IDC_UNUSED_INODE_CNT, m_unused_inode);
  DDX_Text(pDX, IDC_USED_DIR_CNT, m_used_dir_cnt);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExt2Grp, CDialog)
  //{{AFX_MSG_MAP(CExt2Grp)
  ON_BN_CLICKED(IDC_NEXT, OnNext)
  ON_BN_CLICKED(IDC_PREV, OnPrev)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExt2Grp message handlers
BOOL CExt2Grp::OnInitDialog() {
  UpdateDialog();  
  
  CDialog::OnInitDialog();
  
  GetDlgItem(IDC_PREV)->EnableWindow(FALSE);
  GetDlgItem(IDC_NEXT)->EnableWindow(m_groups > 1);
  
  return TRUE;
}

void CExt2Grp::UpdateDialog(void) {
  int size = (m_ourflags & EXT4_FEATURE_INCOMPAT_64BIT) ? 64 : 32;
  struct S_EXT2_GROUP_DESC *desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) m_desc_table + (m_cur_group * size));
  CString cs;
  
  cs.Format("%i of %i", m_cur_group, m_groups - 1);
  SetDlgItemText(IDC_GROUP_NUM, cs);
  
  if (m_ourflags & EXT4_FEATURE_INCOMPAT_64BIT) {
    m_block_bitmap.Format("%I64i", ((DWORD64) desc->block_bitmap_hi << 32) | desc->block_bitmap);
    m_inode_bitmap.Format("%I64i", ((DWORD64) desc->inode_bitmap_hi << 32) | desc->inode_bitmap);
    m_inode_table.Format("%I64i", ((DWORD64) desc->inode_table_hi << 32) | desc->inode_table);
    m_free_blocks_cnt.Format("%i", ((DWORD) desc->free_blocks_count_hi << 16) | desc->free_blocks_count);
    m_free_inodes_cnt.Format("%i", ((DWORD) desc->free_inodes_count_hi << 16) | desc->free_inodes_count);
    m_used_dir_cnt.Format("%i", ((DWORD) desc->used_dirs_count_hi << 16) | desc->used_dirs_count);
  } else {
    m_block_bitmap.Format("%i", desc->block_bitmap);
    m_inode_bitmap.Format("%i", desc->inode_bitmap);
    m_inode_table.Format("%i", desc->inode_table);
    m_free_blocks_cnt.Format("%i", desc->free_blocks_count);
    m_free_inodes_cnt.Format("%i", desc->free_inodes_count);
    m_used_dir_cnt.Format("%i", desc->used_dirs_count);
  }
  m_crc.Format("0x%04X", desc->checksum);
  m_flags.Format("0x%04X", desc->flags);
  m_unused_inode.Format("%i", desc->itable_unused);
}

void CExt2Grp::UpdateGroup(void) {
  int size = (m_ourflags & EXT4_FEATURE_INCOMPAT_64BIT) ? 64 : 32;
  struct S_EXT2_GROUP_DESC *desc = (struct S_EXT2_GROUP_DESC *) ((BYTE *) m_desc_table + (m_cur_group * size));
  
  if (m_ourflags & EXT4_FEATURE_INCOMPAT_64BIT) {
    desc->block_bitmap_hi = (DWORD) (convert64(m_block_bitmap) >> 32);
    desc->inode_bitmap_hi = (DWORD) (convert64(m_inode_bitmap) >> 32);
    desc->inode_table_hi = (DWORD) (convert64(m_inode_table) >> 32);
    desc->free_blocks_count_hi = (WORD) (convert32(m_free_blocks_cnt) >> 16);
    desc->free_inodes_count_hi = (WORD) (convert32(m_free_inodes_cnt) >> 16);
    desc->used_dirs_count_hi = (WORD) (convert32(m_used_dir_cnt) >> 16);
  }
  desc->block_bitmap = (DWORD) (convert64(m_block_bitmap) & 0xFFFFFFFF);
  desc->inode_bitmap = (DWORD) (convert64(m_inode_bitmap) & 0xFFFFFFFF);
  desc->inode_table = (DWORD) (convert64(m_inode_table) & 0xFFFFFFFF);
  desc->free_blocks_count = (WORD) (convert32(m_free_blocks_cnt) & 0xFFFF);
  desc->free_inodes_count = (WORD) (convert32(m_free_inodes_cnt) & 0xFFFF);
  desc->used_dirs_count = (WORD) (convert32(m_used_dir_cnt) & 0xFFFF);
  
  desc->checksum = convert16(m_crc);
  desc->flags = convert16(m_flags);
  desc->itable_unused = convert16(m_unused_inode);
}

void CExt2Grp::OnNext() {
  if (m_cur_group < (m_groups - 1)) {
    UpdateGroup();
    m_cur_group++;
    UpdateDialog();
    UpdateData(FALSE); // send to dialog
    GetDlgItem(IDC_PREV)->EnableWindow(m_cur_group > 0);
    GetDlgItem(IDC_NEXT)->EnableWindow(m_cur_group < (m_groups - 1));
  }
}

void CExt2Grp::OnPrev() {
  if (m_cur_group > 0) {
    UpdateGroup();
    m_cur_group--;
    UpdateDialog();
    UpdateData(FALSE); // send to dialog
    GetDlgItem(IDC_PREV)->EnableWindow(m_cur_group > 0);
    GetDlgItem(IDC_NEXT)->EnableWindow(m_cur_group < (m_groups - 1));
  }
}

void CExt2Grp::OnOK() {
  UpdateGroup();
  CDialog::OnOK();
}
