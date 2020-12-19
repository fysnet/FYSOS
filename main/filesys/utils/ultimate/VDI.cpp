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

// VDI.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "VDI.h"

//#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVDI property page
CVDI::CVDI(CWnd* pParent /*=NULL*/)
  : CDialog(CVDI::IDD, pParent) {
  //{{AFX_DATA_INIT(CVDI)
  m_cookie = _T("");
  m_signature = _T("");
  m_version = _T("");
  m_size_header = _T("");
  m_image_type = _T("");
  m_image_flags = _T("");
  m_description = _T("");
  m_offset_blocks = _T("");
  m_offset_data = _T("");
  m_sector_size = _T("");
  m_unused = _T("");
  m_disk_size = _T("");
  m_block_size = _T("");
  m_block_extra = _T("");
  m_block_count = _T("");
  m_block_allocated = _T("");
  m_uuid = _T("");
  m_snap = _T("");
  m_link = _T("");
  m_parent = _T("");
  m_cylinder = _T("");
  m_head = _T("");
  m_sect_track = _T("");
  //}}AFX_DATA_INIT
}

void CVDI::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CVDI)
  DDX_Text(pDX, IDC_COOKIE, m_cookie);
  DDX_Text(pDX, IDC_SIG0, m_signature);
  DDX_Text(pDX, IDC_VERSION, m_version);
  DDX_Text(pDX, IDC_HEADER_SIZE, m_size_header);
  DDX_Text(pDX, IDC_IMAGE_TYPE, m_image_type);
  DDX_Text(pDX, IDC_FLAGS, m_image_flags);
  DDX_Text(pDX, IDC_DESCRIPTION, m_description);
  DDV_MaxChars(pDX, m_description, 256);
  DDX_Text(pDX, IDC_OFFSET_BLOCKS, m_offset_blocks);
  DDX_Text(pDX, IDC_OFFSET_DATA, m_offset_data);
  DDX_Text(pDX, IDC_SECT_SIZE, m_sector_size);
  DDX_Text(pDX, IDC_UNUSED, m_unused);
  DDX_Text(pDX, IDC_DISK_SIZE, m_disk_size);
  DDX_Text(pDX, IDC_BLOCK_SIZE, m_block_size);
  DDX_Text(pDX, IDC_BLOCK_EXTRA, m_block_extra);
  DDX_Text(pDX, IDC_BLOCK_COUNT, m_block_count);
  DDX_Text(pDX, IDC_BLOCKS_ALLOC, m_block_allocated);
  DDX_Text(pDX, IDC_UUID, m_uuid);
  DDX_Text(pDX, IDC_SNAP, m_snap);
  DDX_Text(pDX, IDC_LINK, m_link);
  DDX_Text(pDX, IDC_PARENT, m_parent);
  DDX_Text(pDX, IDC_CYL, m_cylinder);
  DDX_Text(pDX, IDC_HEAD, m_head);
  DDX_Text(pDX, IDC_SECT, m_sect_track);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVDI, CDialog)
  //{{AFX_MSG_MAP(CVDI
  ON_BN_CLICKED(IDOK, OnApply)
  ON_BN_CLICKED(IDC_VDI_SIG_UPDATE, OnSigUpdate)
  ON_BN_CLICKED(IDC_VDI_UUID_UPDATE, OnUUIDUpdate)
  ON_BN_CLICKED(IDC_VDI_SNAP_UPDATE, OnSnapUpdate)
  ON_BN_CLICKED(IDC_VDI_LINK_UPDATE, OnLinkUpdate)
  ON_BN_CLICKED(IDC_VDI_PARENT_UPDATE, OnParentUpdate)
  ON_EN_CHANGE(IDC_DISK_SIZE, OnChangeSize)
  ON_EN_CHANGE(IDC_IMAGE_TYPE, OnChangeType)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVDI message handlers
BOOL CVDI::OnInitDialog() {
  struct VDI_HEADER *header = (struct VDI_HEADER *) m_buffer;

  memcpy(m_cookie.GetBuffer(65), header->cookie, 64);
  m_cookie.ReleaseBuffer();
  memcpy(m_description.GetBuffer(257), header->description, 256);
  m_description.ReleaseBuffer();

  m_signature.Format("0x%08X", header->signature);
  m_version.Format("0x%08X", header->version);
  m_size_header.Format("%i", header->size_header);
  m_image_type.Format("0x%08X", header->image_type);
  m_image_flags.Format("0x%08X", header->image_flags);
  m_offset_blocks.Format("0x%08X", header->offset_blocks);
  m_offset_data.Format("0x%08X", header->offset_data);
  m_sector_size.Format("%i", header->sector_size);
  m_unused.Format("0x%08X", header->unused);
  m_disk_size.Format("%I64i", header->disk_size);
  m_block_size.Format("%i", header->block_size);
  m_block_extra.Format("0x%08X", header->block_extra);
  m_block_count.Format("%i", header->block_count);
  m_block_allocated.Format("%i", header->blocks_allocated);
  GUID_Format(m_uuid, &header->uuid);
  GUID_Format(m_snap, &header->snap);
  GUID_Format(m_link, &header->link);
  GUID_Format(m_parent, &header->parent);
  m_cylinder.Format("%i", header->cylinders);
  m_head.Format("%i", header->heads);
  m_sect_track.Format("%i", header->sectors);

  CDialog::OnInitDialog();

  OnChangeSize();
  OnChangeType();

  return TRUE;
}

void CVDI::OnApply() {
  struct VDI_HEADER *header = (struct VDI_HEADER *) m_buffer;
  
  UpdateData(TRUE);  // Bring from Dialog
  
  memcpy(header->cookie, m_cookie.GetBuffer(65), 64);
  m_cookie.ReleaseBuffer();
  memcpy(header->description, m_description.GetBuffer(257), 256);
  m_description.ReleaseBuffer();

  header->signature = convert32(m_signature);
  header->version = convert32(m_version);
  header->size_header = convert32(m_size_header);
  header->image_type = convert32(m_image_type);
  header->image_flags = convert32(m_image_flags);
  header->offset_blocks = convert32(m_offset_blocks);
  header->offset_data = convert32(m_offset_data);
  header->sector_size = convert32(m_sector_size);
  header->unused = convert32(m_unused);
  header->disk_size = convert64(m_disk_size);
  header->block_size = convert32(m_block_size);
  header->block_extra = convert32(m_block_extra);
  header->block_count = convert32(m_block_count);
  header->blocks_allocated = convert32(m_block_allocated);
  GUID_Retrieve(m_uuid, &header->uuid);
  GUID_Retrieve(m_snap, &header->snap);
  GUID_Retrieve(m_link, &header->link);
  GUID_Retrieve(m_parent, &header->parent);
  header->cylinders = convert32(m_cylinder);
  header->heads = convert32(m_head);
  header->sectors = convert32(m_sect_track);

  CDialog::OnOK();
}

void CVDI::OnSigUpdate() {
  SetDlgItemText(IDC_SIG0, "0xBEDA107F");
}

// create new random GUID for this header item
void CVDI::OnUUIDUpdate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_UUID, cs);
}

// create new random GUID for this header item
void CVDI::OnSnapUpdate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_SNAP, cs);
}

// create new random GUID for this header item
void CVDI::OnLinkUpdate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_LINK, cs);
}

// create new random GUID for this header item
void CVDI::OnParentUpdate() {
  CString cs;
  struct S_GUID guid;
  
  GUID_Create(&guid, GUID_TYPE_RANDOM);
  GUID_Format(cs, &guid);
  SetDlgItemText(IDC_PARENT, cs);
}

void CVDI::OnChangeSize() {
  CString cs;
  DWORD64 size;

  GetDlgItemText(IDC_DISK_SIZE, cs);
  size = convert64(cs);
  cs.Format("(%.3f Meg)", (float) ((double) size / (double) (1024 * 1024)) );
  SetDlgItemText(IDC_DISK_SIZE_DISP, cs);
}

void CVDI::OnChangeType() {
  CString cs;
  DWORD type;

  GetDlgItemText(IDC_IMAGE_TYPE, cs);
  type = convert32(cs);
  if (type == VDI_IMAGE_TYPE_DYNAMIC)
    SetDlgItemText(IDC_TYPE_DISP, "(Dynamic)");
  else if (type == VDI_IMAGE_TYPE_FLAT)
    SetDlgItemText(IDC_TYPE_DISP, "(Flat)");
  else
    SetDlgItemText(IDC_TYPE_DISP, "(Unknown)");
}

