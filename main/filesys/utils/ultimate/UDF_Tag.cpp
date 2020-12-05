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

// UDF_Tag.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "UDF_Tag.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUDF_Tag dialog
CUDF_Tag::CUDF_Tag(CWnd* pParent /*=NULL*/)
  : CDialog(CUDF_Tag::IDD, pParent) {
  //{{AFX_DATA_INIT(CUDF_Tag)
  m_crc = _T("");
  m_desc_crc = _T("");
  m_desc_crc_len = _T("");
  m_id = _T("");
  m_location = _T("");
  m_resv = _T("");
  m_ser_num = _T("");
  m_ver = _T("");
  //}}AFX_DATA_INIT
}

void CUDF_Tag::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CUDF_Tag)
  DDX_Text(pDX, IDC_UDF_TAG_CRC, m_crc);
  DDX_Text(pDX, IDC_UDF_TAG_DESC_CRC, m_desc_crc);
  DDX_Text(pDX, IDC_UDF_TAG_DESC_CRC_LEN, m_desc_crc_len);
  DDX_Text(pDX, IDC_UDF_TAG_ID, m_id);
  DDX_Text(pDX, IDC_UDF_TAG_LOCATION, m_location);
  DDX_Text(pDX, IDC_UDF_TAG_RSVD, m_resv);
  DDX_Text(pDX, IDC_UDF_TAG_SER_NUM, m_ser_num);
  DDX_Text(pDX, IDC_UDF_TAG_VER, m_ver);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUDF_Tag, CDialog)
  //{{AFX_MSG_MAP(CUDF_Tag)
  ON_BN_CLICKED(IDC_UDF_TAG_ID_B, OnUdfTagIdB)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUDF_Tag message handlers
BOOL CUDF_Tag::OnInitDialog() {
  
  m_id.Format("0x%04X", m_tag.id);
  m_ver.Format("0x%04X", m_tag.ver);
  m_crc.Format("0x%02X", m_tag.crc);
  m_resv.Format("0x%02X", m_tag.resv);
  m_ser_num.Format("0x%04X", m_tag.tagsernum);
  m_desc_crc.Format("0x%04X", m_tag.desccrc);
  m_desc_crc_len.Format("%i", m_tag.desccrclen);
  m_location.Format("%i", m_tag.tagloc);
  
  CDialog::OnInitDialog();
  
  return TRUE;
}

void CUDF_Tag::OnOK() {
  
  m_tag.id = convert16(m_id);
  m_tag.ver = convert16(m_ver);
  m_tag.crc = convert8(m_crc);
  m_tag.resv = convert8(m_resv);
  m_tag.tagsernum = convert16(m_ser_num);
  m_tag.desccrc = convert16(m_desc_crc);
  m_tag.desccrclen = convert16(m_desc_crc_len);
  m_tag.tagloc = convert32(m_location);
  
  CDialog::OnOK();
}

struct S_ATTRIBUTES tag_id_vals[] = {
                                           //            |                               | <- max (col 67)
  { 1,                       0xFFFFFFFF,              0, "Primary Volume Descriptor"      , {-1, } },
  { 2,                       0xFFFFFFFF,              1, "Anchor Descriptor Pointer"      , {-1, } },
  { 3,                       0xFFFFFFFF,              2, "Volume Descriptor Pointer"      , {-1, } },
  { 4,                       0xFFFFFFFF,              3, "Implementation Use Volume Desc" , {-1, } },
  { 5,                       0xFFFFFFFF,              4, "Partition Descriptor"           , {-1, } },
  { 6,                       0xFFFFFFFF,              5, "Logical Volume Descriptor"      , {-1, } },
  { 7,                       0xFFFFFFFF,              6, "Unallocated Space Descriptor"   , {-1, } },
  { 8,                       0xFFFFFFFF,              7, "Terminating Descriptor"         , {-1, } },
  { 9,                       0xFFFFFFFF,              8, "Logical Volume Integrity Desc"  , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CUDF_Tag::OnUdfTagIdB() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_UDF_TAG_ID, cs);
  dlg.m_title = "Tag ID";
  dlg.m_attrib = convert16(cs);
  dlg.m_attributes = tag_id_vals;
  dlg.m_single = TRUE;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_UDF_TAG_ID, cs);
  }
}
