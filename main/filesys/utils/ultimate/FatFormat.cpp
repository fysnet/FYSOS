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

// FatFormat.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "FatFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFatFormat dialog
CFatFormat::CFatFormat(CWnd* pParent /*=NULL*/)
  : CDialog(CFatFormat::IDD, pParent) {
  //{{AFX_DATA_INIT(CFatFormat)
  m_num_fats = 0;
  m_root_entries = 0;
  m_sect_cluster = 0;
  m_info = _T("");
  //}}AFX_DATA_INIT
}

void CFatFormat::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFatFormat)
  DDX_Text(pDX, IDC_NUM_FATS, m_num_fats);
  DDV_MinMaxInt(pDX, m_num_fats, 1, 2);
  DDX_Text(pDX, IDC_ROOT_ENTRIES, m_root_entries);
  DDX_Text(pDX, IDC_SECT_CLUSTER, m_sect_cluster);
  DDX_Text(pDX, IDC_INFO, m_info);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFatFormat, CDialog)
  //{{AFX_MSG_MAP(CFatFormat)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFatFormat message handlers
BOOL CFatFormat::OnInitDialog() {
  
  m_calc_spc = 
  m_sect_cluster = CalcSPC(m_sectors, m_fat_size);
  m_info.Format("For FAT%i with %I64i sectors, you must have a count of\r\n"
                "'sectors per cluster' of at least %i.\r\n"
                "(Must be a power of 2)",
                m_fat_size == FS_FAT12 ? 12 : (m_fat_size == FS_FAT16) ? 16 : 32,
                m_sectors,
                m_sect_cluster);
  
  CDialog::OnInitDialog();
  
  return TRUE;
}

void CFatFormat::OnOK() {
  CString cs;
  
  UpdateData(TRUE);  // bring from dialog
  
  if ((m_sect_cluster < m_calc_spc) || ((m_sect_cluster > 1) && !power_of_two(m_sect_cluster))) {
    cs.Format("Sectors per Cluster must be at least %i and a power of 2", m_calc_spc);
    AfxMessageBox(cs);
    SetDlgItemInt(IDC_SECT_CLUSTER, m_calc_spc, FALSE);
    return;
  }
  
  CDialog::OnOK();
}

// - cluster count <  4085 is FAT12
//                 < 65525 is FAT16
//                 else       FAT32
int CFatFormat::CalcSPC(DWORD64 sectors, int fat_size) {
  int spc = 1;
  
  while (spc <= 64) {
    if ((fat_size == FS_FAT12) && ((sectors / spc) < 4085))
      return spc;
    if ((fat_size == FS_FAT16) && ((sectors / spc) < 65525))
      return spc;
    if ((fat_size == FS_FAT32) && ((sectors / spc) >= 65525))
      return spc;
    spc = spc << 1;  // only powers of two are allowed
  }
  
  AfxMessageBox("Illegal combination of sectors per cluster, total sectors, and fat size");
  return 1;
}
