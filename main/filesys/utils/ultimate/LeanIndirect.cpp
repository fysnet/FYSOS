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

// LeanIndirect.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Lean.h"
#include "LeanEntry.h"
#include "LeanIndirect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeanIndirect dialog

//IMPLEMENT_DYNAMIC(CLeanIndirect, CDialog)

CLeanIndirect::CLeanIndirect(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LEAN_INDIRECT, pParent) {
  //{{AFX_DATA_INIT(CLeanIndirect)
  m_magic = _T("");
  m_crc = _T("");
  m_block_count = _T("");
  m_inode = _T("");
  m_this_block = _T("");
  m_prev_indirect = _T("");
  m_next_indirect = _T("");
  m_extent_count = _T("");
  m_reserved0 = _T("");
  m_reserved1 = _T("");
  m_reserved2 = _T("");
  //}}AFX_DATA_INIT

  m_current_indirect = 0;
  m_cur_index = 0;
  m_indirect_buffer = NULL;
  m_dirty = FALSE;
  m_reserved2_size = 0;
  m_extent_max = 0;
}

CLeanIndirect::~CLeanIndirect() {
  if (m_indirect_buffer != NULL)
    free(m_indirect_buffer);
}

void CLeanIndirect::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CLeanEntry)
  DDX_Control(pDX, IDC_EXT_START, m_ext_start);
  DDX_Control(pDX, IDC_EXT_SIZE, m_ext_size);
  DDX_Control(pDX, IDC_EXT_CRC, m_ext_crc);
  DDX_Text(pDX, IDC_ENTRY_MAGIC, m_magic);
  DDX_Text(pDX, IDC_ENTRY_CRC, m_crc);
  DDX_Text(pDX, IDC_ENTRY_BLOCK_COUNT, m_block_count);
  DDX_Text(pDX, IDC_ENTRY_INODE, m_inode);
  DDX_Text(pDX, IDC_ENTRY_THIS_BLOCK, m_this_block);
  DDX_Text(pDX, IDC_ENTRY_PREV_INDIRECT, m_prev_indirect);
  DDX_Text(pDX, IDC_ENTRY_NEXT_INDIRECT, m_next_indirect);
  DDX_Text(pDX, IDC_ENTRY_EXTENT_COUNT, m_extent_count);
  DDX_Text(pDX, IDC_ENTRY_RESERVED0, m_reserved0);
  DDX_Text(pDX, IDC_ENTRY_RESERVED1, m_reserved1);
  DDX_Text(pDX, IDC_ENTRY_RESERVED2, m_reserved2);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLeanIndirect, CDialog)
  //{{AFX_MSG_MAP(CLeanIndirect)
  ON_LBN_SELCHANGE(IDC_EXT_START, OnSelchangeExtStart)
  ON_LBN_SELCHANGE(IDC_EXT_SIZE, OnSelchangeExtSize)
  ON_LBN_SELCHANGE(IDC_EXT_CRC, OnSelchangeExtCrc)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  ON_BN_CLICKED(IDC_MAGIC_UPDATE, OnMagicUpdate)
  ON_BN_CLICKED(IDC_BLOCK_UPDATE, OnBlockCountUpdate)
  ON_BN_CLICKED(IDC_THIS_BLOCK_UPDATE, OnThisBlockUpdate)
  ON_BN_CLICKED(IDPREV, OnPrev)
  ON_BN_CLICKED(IDNEXT, OnNext)
  ON_BN_CLICKED(IDC_RESV0_UPDATE, OnResv0Update)
  ON_BN_CLICKED(IDC_RESV1_UPDATE, OnResv1Update)
  ON_BN_CLICKED(IDC_RESV2_UPDATE, OnResv2Update)
  ON_BN_CLICKED(IDOK, OnOkay)
  
  ON_EN_CHANGE(IDC_ENTRY_CRC, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_MAGIC, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_BLOCK_COUNT, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_INODE, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_THIS_BLOCK, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_PREV_INDIRECT, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_NEXT_INDIRECT, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_EXTENT_COUNT, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_RESERVED0, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_RESERVED1, OnChangeItem)
  ON_EN_CHANGE(IDC_ENTRY_RESERVED2, OnChangeItem)

  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeanEntry message handlers
BOOL CLeanIndirect::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult) {
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
      strTipText = "Recalculate CRC of Indirect";
      break;
    case IDC_MAGIC_UPDATE:
      strTipText = "Set to the correct Magic value";
      break;
    case IDC_BLOCK_UPDATE:
      strTipText = "Recalculate Used Blocks";
      break;
    case IDC_THIS_BLOCK_UPDATE:
      strTipText = "Set to This Block value";
      break;
    case IDC_RESV0_UPDATE:
    case IDC_RESV1_UPDATE:
    case IDC_RESV2_UPDATE:
      strTipText = "Zero this Reserved Area";
      break;

    case IDPREV:
      strTipText = "Previous Indirect in this linked list";
      break;
    case IDNEXT:
      strTipText = "Next Indirect in this linked list";
      break;

    case IDC_EXT_START:
      strTipText = "Extent Starts for this Indirect";
      break;
    case IDC_EXT_SIZE:
      strTipText = "Extent Sizes for this Indirect";
      break;

    case IDOK:
      strTipText = "Save modifications and exit";
      break;
    case IDCANCEL:
      strTipText = "Disregard modifications and exit";
      break;
  }

  strncpy(pTTTA->szText, strTipText, 79);
  pTTTA->szText[79] = '\0';  // make sure it is null terminated

  return TRUE; // message was handled
}

// CLeanIndirect message handlers
BOOL CLeanIndirect::OnInitDialog() {
  
  CDialog::OnInitDialog();

  unsigned extent_size = (m_parent->m_use_extended_extents) ? 16 : 12;
  m_extent_max = (m_parent->m_block_size - LEAN_INDIRECT_SIZE) / extent_size;
  m_reserved2_size = m_parent->m_block_size - LEAN_INDIRECT_SIZE - (m_extent_max * extent_size);
  m_indirect_buffer = malloc(m_parent->m_block_size);
  UpdateIndirect(m_current_indirect);
  //UpdateStatus(m_current_indirect); // called by UpdateIndirect() above
  
  GetDlgItem(IDC_ENTRY_RESERVED2_STATIC)->ShowWindow(m_reserved2_size > 0);
  GetDlgItem(IDC_ENTRY_RESERVED2)->ShowWindow(m_reserved2_size > 0);
  GetDlgItem(IDC_RESV2_UPDATE)->ShowWindow(m_reserved2_size > 0);

  EnableToolTips(TRUE);

  return TRUE;
}

void CLeanIndirect::UpdateStatus(const DWORD64 block) {
  CString cs;

  cs.Format("Indirect at %I64i -- index %i of %i", block, m_cur_index, m_entry_parent->m_inode.indirect_count);
  SetDlgItemText(IDC_INDIRECT_STATUS, cs);
}

void CLeanIndirect::UpdateIndirect(const DWORD64 block) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *byte;
  CString cs;
  int i;

  unsigned extent_size = (m_parent->m_use_extended_extents) ? 16 : 12;
  unsigned max_extents = (m_parent->m_block_size - LEAN_INDIRECT_SIZE) / extent_size;
  DWORD64 *p_extent_start = (DWORD64 *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE);
  DWORD *p_extent_size = (DWORD *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * sizeof(DWORD64)));
  DWORD *p_extent_crc = (DWORD *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD))));
  if (m_parent->m_use_extended_extents)
    byte = (BYTE *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD) + sizeof(DWORD))));
  else
    byte = (BYTE *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE + (max_extents * (sizeof(DWORD64) + sizeof(DWORD))));

  dlg->ReadBlocks(m_indirect_buffer, m_parent->m_lba, block, m_parent->m_block_size, 1);

  memcpy(&m_indirect, m_indirect_buffer, LEAN_INDIRECT_SIZE);
  
  m_crc.Format("0x%08X", m_indirect.checksum);
  m_magic.Format("0x%08X", m_indirect.magic);
  m_block_count.Format("%I64i", m_indirect.block_count);
  m_inode.Format("%I64i", m_indirect.inode);
  m_this_block.Format("%I64i", m_indirect.this_block);
  m_prev_indirect.Format("%I64i", m_indirect.prev_indirect);
  m_next_indirect.Format("%I64i", m_indirect.next_indirect);
  m_extent_count.Format("%i", m_indirect.extent_count);
  
  m_reserved0.Format("%02X %02X", m_indirect.reserved0[0], m_indirect.reserved0[1]);
  m_reserved1.Format("0x%08X", m_indirect.reserved1);
  if (m_reserved2_size > 0) {
    m_reserved2 = "";
    for (i=0; i<m_reserved2_size; i++) {
      cs.Format("%02X ", byte[i]);
      m_reserved2 += cs;
    }
  }

  m_ext_start.ResetContent();
  m_ext_size.ResetContent();
  m_ext_crc.ResetContent();
  for (i=0; i<m_indirect.extent_count; i++) {
    cs.Format("%I64i", p_extent_start[i]);
    m_ext_start.AddString(cs);
    cs.Format("%i", p_extent_size[i]);
    m_ext_size.AddString(cs);
    if (m_parent->m_use_extended_extents) {
      cs.Format("0x%08X", p_extent_crc[i]);
      m_ext_crc.AddString(cs);
    }
  }

  // send to the dialog
  UpdateData(FALSE);

  GetDlgItem(IDPREV)->EnableWindow(m_indirect.prev_indirect != 0);
  GetDlgItem(IDNEXT)->EnableWindow(m_indirect.next_indirect != 0);
  GetDlgItem(IDC_EXT_CRC)->ShowWindow(m_parent->m_use_extended_extents);
  GetDlgItem(IDC_EXT_CRC)->SetFont(&dlg->m_DumpFont);

  UpdateStatus(block);
  m_dirty = FALSE;
  GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CLeanIndirect::OnSelchangeExtStart() {
  m_ext_size.SetCurSel(m_ext_start.GetCurSel());
  m_ext_crc.SetCurSel(m_ext_start.GetCurSel());
}

void CLeanIndirect::OnSelchangeExtSize() {
  m_ext_start.SetCurSel(m_ext_size.GetCurSel());
  m_ext_crc.SetCurSel(m_ext_size.GetCurSel());
}

void CLeanIndirect::OnSelchangeExtCrc() {
  m_ext_start.SetCurSel(m_ext_crc.GetCurSel());
  m_ext_size.SetCurSel(m_ext_crc.GetCurSel());
}

void CLeanIndirect::OnCrcUpdate() {
  UpdateData(TRUE);
  
  m_indirect.checksum = convert32(m_crc);
  m_indirect.magic = convert32(m_magic);
  m_indirect.block_count = convert64(m_block_count);
  m_indirect.inode = convert64(m_inode);
  m_indirect.this_block = convert64(m_this_block);
  m_indirect.prev_indirect = convert64(m_prev_indirect);
  m_indirect.next_indirect = convert64(m_next_indirect);
  m_indirect.extent_count = convert32(m_extent_count);

  memcpy(m_indirect_buffer, &m_indirect, LEAN_INDIRECT_SIZE);
  DWORD crc = m_parent->computeChecksum(m_indirect_buffer, m_parent->m_block_size, TRUE);
  m_crc.Format("0x%08X", crc);
  SetDlgItemText(IDC_ENTRY_CRC, m_crc);
}

void CLeanIndirect::ApplyIndirect(const DWORD64 block) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;

  UpdateData(TRUE);
  
  m_indirect.checksum = convert32(m_crc);
  m_indirect.magic = convert32(m_magic);
  m_indirect.block_count = convert64(m_block_count);
  m_indirect.inode = convert64(m_inode);
  m_indirect.this_block = convert64(m_this_block);
  m_indirect.prev_indirect = convert64(m_prev_indirect);
  m_indirect.next_indirect = convert64(m_next_indirect);
  m_indirect.extent_count = convert32(m_extent_count);

  memcpy(m_indirect_buffer, &m_indirect, LEAN_INDIRECT_SIZE);
  dlg->WriteBlocks(m_indirect_buffer, m_parent->m_lba, block, m_parent->m_block_size, 1);
}

void CLeanIndirect::OnMagicUpdate() {
  SetDlgItemText(IDC_ENTRY_MAGIC, "0x58444E49");
}

void CLeanIndirect::OnBlockCountUpdate() {
  CString cs;
  DWORD64 count = 0;
  DWORD32 *dword;
  int extent_count, i;

  GetDlgItemText(IDC_ENTRY_EXTENT_COUNT, cs);
  extent_count = convert32(cs);
  
  dword = (DWORD32 *) ((BYTE *) m_indirect_buffer + LEAN_INDIRECT_SIZE + (m_extent_max * sizeof(DWORD64)));
  for (i=0; i<extent_count; i++)
    count += dword[i];

  cs.Format("%I64i", count);
  SetDlgItemText(IDC_ENTRY_BLOCK_COUNT, cs);
}

void CLeanIndirect::OnThisBlockUpdate() {
  CString cs;
  
  cs.Format("%I64i", m_current_indirect);
  SetDlgItemText(IDC_ENTRY_THIS_BLOCK, cs);
}

void CLeanIndirect::OnPrev() {
  int ret;

  // if dirty, ask to update
  if (m_dirty) {
    ret = AfxMessageBox("Write Current Indirect?", MB_YESNOCANCEL, 0);
    if (ret == IDCANCEL)
      return;
    if (ret == IDYES)
      ApplyIndirect(m_current_indirect);
  }

  m_cur_index--;
  m_current_indirect = convert64(m_prev_indirect);
  UpdateIndirect(m_current_indirect);
}

void CLeanIndirect::OnNext() {
  int ret;

  // if dirty, ask to update
  if (m_dirty) {
    ret = AfxMessageBox("Write Current Indirect?", MB_YESNOCANCEL, 0);
    if (ret == IDCANCEL)
      return;
    if (ret == IDYES)
      ApplyIndirect(m_current_indirect);
  }
  
  m_cur_index++;
  m_current_indirect = convert64(m_next_indirect);
  UpdateIndirect(m_current_indirect);
}

void CLeanIndirect::OnResv0Update() {
  m_indirect.reserved0[0] = 0;
  m_indirect.reserved0[1] = 0;

  SetDlgItemText(IDC_ENTRY_RESERVED0, "00 00");
}

void CLeanIndirect::OnResv1Update() {
  m_indirect.reserved1 = 0;

  SetDlgItemText(IDC_ENTRY_RESERVED1, "0x00000000");
}

void CLeanIndirect::OnResv2Update() {
  CString cs;

  cs = "";
  for (int i=0; i<m_reserved2_size; i++)
    cs += "00 ";
  
  SetDlgItemText(IDC_ENTRY_RESERVED2, cs);
}

void CLeanIndirect::OnOkay() {
  ApplyIndirect(m_current_indirect);
  m_dirty = FALSE;
  GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CLeanIndirect::OnChangeItem() {
  m_dirty = TRUE;
  
  UpdateStatus(m_current_indirect);
  GetDlgItem(IDOK)->EnableWindow(TRUE);
}
