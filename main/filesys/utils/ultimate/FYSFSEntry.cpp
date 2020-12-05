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

// FYSFSEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "FYSFSEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFYSFSEntry dialog
CFYSFSEntry::CFYSFSEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CFYSFSEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CFYSFSEntry)
  m_attribute = _T("");
  m_crc = _T("");
  m_created = _T("");
  m_fat_cont = _T("");
  m_fat_entries = _T("");
  m_file_size = _T("");
  m_flags = _T("");
  m_last_access = _T("");
  m_name_cont = _T("");
  m_name_fat = _T("");
  m_name_len = _T("");
  m_scratch = _T("");
  m_slot_type = _T("");
  //}}AFX_DATA_INIT
}

void CFYSFSEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFYSFSEntry)
  DDX_Text(pDX, IDC_ATTRIBUTE, m_attribute);
  DDX_Text(pDX, IDC_CRC, m_crc);
  DDX_Text(pDX, IDC_CREATED, m_created);
  DDX_Text(pDX, IDC_FAT_CONT, m_fat_cont);
  DDX_Text(pDX, IDC_FAT_ENTRIES, m_fat_entries);
  DDX_Text(pDX, IDC_FILE_SIZE, m_file_size);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_LAST_ACC, m_last_access);
  DDX_Text(pDX, IDC_NAME_CONT, m_name_cont);
  DDX_Text(pDX, IDC_NAME_FAT, m_name_fat);
  DDX_Text(pDX, IDC_NAME_LEN, m_name_len);
  DDX_Text(pDX, IDC_SCRATCH, m_scratch);
  DDX_Text(pDX, IDC_SLOT_TYPE, m_slot_type);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFYSFSEntry, CDialog)
  //{{AFX_MSG_MAP(CFYSFSEntry)
    // NOTE: the ClassWizard will add message map macros here
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFYSFSEntry message handlers
