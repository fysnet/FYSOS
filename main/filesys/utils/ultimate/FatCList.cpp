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

// FatCList.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "Fat.h"
#include "FatCList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFatCList dialog
CFatCList::CFatCList(CWnd* pParent /*=NULL*/)
  : CDialog(CFatCList::IDD, pParent) {
  //{{AFX_DATA_INIT(CFatCList)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

void CFatCList::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CFatCList)
  DDX_Control(pDX, IDC_LIST, m_list);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFatCList, CDialog)
  //{{AFX_MSG_MAP(CFatCList)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFatCList message handlers

BOOL CFatCList::OnInitDialog() {
  CDialog::OnInitDialog();
  
  CString cs;
  char ch;
  int i;
  CWaitCursor wait;  // creates and changes to a wait cursor (automatically changes back on destroy())

  for (i=0; i<m_entries->entry_count; i++) {
    ch = ((i > 0) && (m_entries->entries[i] != m_entries->entries[i-1] + 1)) ? '*' : ' ';
    switch (m_fat_size) {
      case FS_FAT12:
        cs.Format("0x%03X (%i)%c", m_entries->entries[i], m_entries->entries[i], ch);
        break;
      case FS_FAT16:
        cs.Format("0x%04X (%i)%c", m_entries->entries[i], m_entries->entries[i], ch);
        break;
      case FS_FAT32:
        cs.Format("0x%08X (%i)%c", m_entries->entries[i], m_entries->entries[i], ch);
        break;
    }
    m_list.AddString(cs);
  }
  
  // add a trailing entry to indicate how many entries there were
  cs.Format("%i entries", i);
  m_list.AddString(cs);

  return TRUE;
}
