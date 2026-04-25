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

// AdfsEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "Adfs.h"
#include "AdfsEntry.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CADFSEntry dialog
CADFSEntry::CADFSEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CADFSEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CADFSEntry)
  m_canberead = FALSE;
  m_canbewritten = FALSE;
  m_locked = FALSE;
  m_isdir = FALSE;
  m_execute_only = FALSE;
  m_entry_num = 0;
  m_entry_tot = 0;
  m_name = _T("");
  m_load_addr = _T("");
  m_exec_addr = _T("");
  m_filelen = 0;
  m_start = 0;
  m_cycle_num = 0;
  //}}AFX_DATA_INIT
}

void CADFSEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CADFSEntry)
  DDX_Check(pDX, IDC_ADFS_CANREAD, m_canberead);
  DDX_Check(pDX, IDC_ADFS_CANWRITE, m_canbewritten);
  DDX_Check(pDX, IDC_ADFS_LOCKED, m_locked);
  DDX_Check(pDX, IDC_ADFS_DIR, m_isdir);
  DDX_Check(pDX, IDC_ADFS_EXECUTE, m_execute_only);
  DDX_Text(pDX, IDC_ENTRY_NUM, m_entry_num);
  DDX_Text(pDX, IDC_ENTRY_TOT, m_entry_tot);
  DDX_Text(pDX, IDC_ENTRY_NAME, m_name);
  DDV_MaxChars(pDX, m_name, 10);
  DDX_Text(pDX, IDC_ENTRY_LOAD_ADDR, m_load_addr);
  DDX_Text(pDX, IDC_ENTRY_EXEC_ADDR, m_exec_addr);
  DDX_Text(pDX, IDC_ENTRY_FILELEN, m_filelen);
  DDX_Text(pDX, IDC_ENTRY_START, m_start);
  DDX_Text(pDX, IDC_ENTRY_CYCLE, m_cycle_num);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CADFSEntry, CDialog)
  //{{AFX_MSG_MAP(CADFSEntry)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CADFSEntry message handlers

BOOL CADFSEntry::OnInitDialog() {

  m_attribute = CADFS::GetName(m_name, &m_start, &m_filelen, &m_entry);
  m_load_addr.Format("0x%08X", m_entry.load_addr);
  m_exec_addr.Format("0x%08X", m_entry.exec_addr);
  m_cycle_num = m_entry.cycle;

  m_canberead = (m_attribute & ADFS_ENTRY_R) > 0;
  m_canbewritten = (m_attribute & ADFS_ENTRY_W) > 0;
  m_locked = (m_attribute & ADFS_ENTRY_L) > 0;
  m_isdir = (m_attribute & ADFS_ENTRY_D) > 0;
  m_execute_only = (m_attribute & ADFS_ENTRY_E) > 0;

  CDialog::OnInitDialog();
  
  return TRUE;
}

void CADFSEntry::OnOK() {
  UpdateData(TRUE); // receive from Dialog
  int i;
  
  // attribute
  m_attribute  = m_canberead ? ADFS_ENTRY_R : 0;
  m_attribute |= m_canbewritten ? ADFS_ENTRY_W : 0;
  m_attribute |= m_locked ? ADFS_ENTRY_L : 0;
  m_attribute |= m_isdir ? ADFS_ENTRY_D : 0;
  m_attribute |= m_execute_only ? ADFS_ENTRY_E : 0;

  // name
  BYTE *p = m_entry.name;
  memset(p, 0x0D, 10);
  for (i=0; i<10 && i < m_name.GetLength(); i++)
    *p++ = m_name.GetAt(i) | ((m_attribute & (1<<i)) ? 0x80 : 0x00);
  for (; i<5; i++)
    *p++ |= ((m_attribute & (1<<i)) ? 0x80 : 0x00);

  m_entry.load_addr = convert32(m_load_addr);
  m_entry.exec_addr = convert32(m_exec_addr);
  m_entry.cycle = m_cycle_num;
  
  m_entry.file_length = m_filelen;
  m_entry.start_block.entry[0] = (BYTE) ((m_start >>  0) & 0xFF);
  m_entry.start_block.entry[1] = (BYTE) ((m_start >>  8) & 0xFF);
  m_entry.start_block.entry[2] = (BYTE) ((m_start >> 16) & 0xFF);

  CDialog::OnOK();
}
