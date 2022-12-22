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

// ISOImage.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "ISOImage.h"

#include "ISOBoot.h"
#include "ISOPrimary.h"
#include "ISOSupple.h"

#include "ISOBEA.h"
#include "ISONSR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CISOImage property page

CISOImage::CISOImage() {
  //{{AFX_DATA_INIT(CISOImage)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

CISOImage::~CISOImage() {
}

/////////////////////////////////////////////////////////////////////////////
// CISOImage message handlers

// ISO colors will have a ???? shade to them.
DWORD CISOImage::GetNewColor(int index) {
  int r = ((106 - (index * 20)) > -1) ? (106 - (index * 20)) : 0;
  int g = ((126 - (index * 18)) > -1) ? (126 - (index * 18)) : 0;
  int b = ((239 - (index *  2)) > -1) ? (239 - (index *  2)) : 0;
  return RGB(r, g, b);
}

// LBA = lba of volume descriptor (16 -> end)
bool CISOImage::Start(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE buffer[MAX_SECT_SIZE];
  DWORD64 lba = 16;
  int     cindex = 0;
  BOOL    cont = TRUE;
  
  while (cont) {
    dlg->ReadFromFile(buffer, lba, 1);
    if (memcmp(buffer + 1, "CD001", 5) == 0) {
      switch (buffer[0]) {
        case VD_TYPE_BOOT:
          if (!m_BVD.m_is_valid) {
            memcpy(m_BVD.m_descriptor, buffer, 2048);
            m_BVD.Start(lba, GetNewColor(cindex), TRUE);
            dlg->m_TabControl.AddPage(&m_BVD);
          }
          break;
        case VD_TYPE_PRIM:
          if (!m_PVD.m_is_valid) {
            memcpy(m_PVD.m_descriptor, buffer, 2048);
            m_PVD.Start(lba, GetNewColor(cindex), TRUE);
            dlg->m_TabControl.AddPage(&m_PVD);
          }
          break;
        case VD_TYPE_SUMP:
          if (!m_SVD.m_is_valid) {
            memcpy(m_SVD.m_descriptor, buffer, 2048);
            m_SVD.Start(lba, GetNewColor(cindex), TRUE);
            dlg->m_TabControl.AddPage(&m_SVD);
          } else
            AfxMessageBox("Already found supplement entry.\r\nTODO: Add ability to have multiple...");
          break;
        case VD_TYPE_VOL:
          //dlg->m_ISONames[index] = "Volume Partition";
          AfxMessageBox("Found Volume Desc");
          break;
        case VD_TYPE_END:
          //AfxMessageBox("Found End");
          cont = FALSE;
          break;
        default:
          //dlg->m_ISONames[index] = "Unknown";
          AfxMessageBox("Found Other");
          cont = FALSE;
          break;
      }
    } else if (memcmp(buffer + 1, "BEA01", 5) == 0) {
      if (!m_BEA.m_is_valid) {
        memcpy(m_BEA.m_descriptor, buffer, 2048);
        m_BEA.Start(lba, GetNewColor(cindex), TRUE);
        dlg->m_TabControl.AddPage(&m_BEA);
      }
    } else if ((memcmp(buffer + 1, "NSR02", 5) == 0) ||
               (memcmp(buffer + 1, "NSR03", 5) == 0)) {
      if (!m_NSR.m_is_valid) {
        memcpy(m_NSR.m_descriptor, buffer, 2048);
        m_NSR.Start(lba, GetNewColor(cindex), TRUE);
        dlg->m_TabControl.AddPage(&m_NSR);
      }
    } else
      break;
    lba++;
    cindex++;
  }
  
  return TRUE;
}
