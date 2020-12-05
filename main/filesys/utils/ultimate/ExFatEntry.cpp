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

// ExFatEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "ExFat.h"
#include "ViewBitmap.h"
#include "ExFatUCase.h"
#include "ExFatEntry.h"

#include "fat.h"
#include "FatCList.h"

#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExFatEntry dialog
CExFatEntry::CExFatEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CExFatEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CExFatEntry)
  m_attribute = _T("");
  m_crc = _T("");
  m_created = _T("");
  m_created_ms = _T("");
  m_created_tz = _T("");
  m_data_len = _T("");
  m_cluster = _T("");
  m_error_code = _T("");
  m_name = _T("");
  m_reserved0 = _T("");
  m_reserved1 = _T("");
  m_reserved2 = _T("");
  m_reserved3 = _T("");
  m_reserved4 = _T("");
  m_flags = _T("");
  m_last_acc = _T("");
  m_last_acc_tz = _T("");
  m_modified = _T("");
  m_modified_ms = _T("");
  m_modified_tz = _T("");
  m_name_hash = _T("");
  m_name_len = _T("");
  m_secondaries = _T("");
  m_valid_data_len = _T("");
  m_guid = _T("");
  //}}AFX_DATA_INIT
  m_entry_buffer = NULL;
  m_ucase_buffer = NULL;
}

void CExFatEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExFatEntry)
  DDX_Text(pDX, IDC_ATTRIB, m_attribute);
  DDX_Text(pDX, IDC_CRC, m_crc);
  DDX_Text(pDX, IDC_CREATED, m_created);
  DDX_Text(pDX, IDC_CREATED_MS, m_created_ms);
  DDX_Text(pDX, IDC_CREATED_TZ, m_created_tz);
  DDX_Text(pDX, IDC_DATA_LEN, m_data_len);
  DDX_Text(pDX, IDC_ENTRY_CLUSTER, m_cluster);
  DDX_Text(pDX, IDC_ENTRY_ERROR_CODE, m_error_code);
  DDX_Text(pDX, IDC_ENTRY_NAME, m_name);
  DDX_Text(pDX, IDC_ENTRY_RESERVED0, m_reserved0);
  DDX_Text(pDX, IDC_ENTRY_RESERVED1, m_reserved1);
  DDX_Text(pDX, IDC_ENTRY_RESERVED2, m_reserved2);
  DDX_Text(pDX, IDC_ENTRY_RESERVED3, m_reserved3);
  DDX_Text(pDX, IDC_ENTRY_RESERVED4, m_reserved4);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_LAST_ACC, m_last_acc);
  DDX_Text(pDX, IDC_LAST_ACC_TZ, m_last_acc_tz);
  DDX_Text(pDX, IDC_MODIFIED, m_modified);
  DDX_Text(pDX, IDC_MODIFIED_MS, m_modified_ms);
  DDX_Text(pDX, IDC_MODIFIED_TZ, m_modified_tz);
  DDX_Text(pDX, IDC_NAME_HASH, m_name_hash);
  DDX_Text(pDX, IDC_NAME_LEN, m_name_len);
  DDX_Text(pDX, IDC_SEC_COUNT, m_secondaries);
  DDX_Text(pDX, IDC_VALID_DATA_LEN, m_valid_data_len);
  DDX_Text(pDX, IDC_ENTRY_GUID, m_guid);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExFatEntry, CDialog)
  //{{AFX_MSG_MAP(CExFatEntry)
  ON_BN_CLICKED(IDDOATTRIB, OnDoattrib)
  ON_BN_CLICKED(IDFATENTRIES, OnFatentries)
  ON_BN_CLICKED(IDEXFATCONVERT, OnFatConvert)
  ON_BN_CLICKED(IDCREATED, OnCreated)
  ON_BN_CLICKED(IDLASTACC, OnLastacc)
  ON_BN_CLICKED(IDLASTMOD, OnLastmod)
  ON_BN_CLICKED(IDLFN_CLEAR2, OnClear2)
  ON_BN_CLICKED(IDLFN_CLEAR3, OnClear3)
  ON_BN_CLICKED(IDLFN_CLEAR4, OnClear4)
  ON_BN_CLICKED(IDSFN_CLEAR0, OnClear0)
  ON_BN_CLICKED(IDSFN_CLEAR1, OnClear1)
  ON_BN_CLICKED(IDC_CRC_UPDATE, OnCrcUpdate)
  ON_BN_CLICKED(IDFLAGS, OnFlags)
  ON_EN_CHANGE(IDC_CREATED, OnChangeCreated)
  ON_EN_CHANGE(IDC_MODIFIED, OnChangeModified)
  ON_EN_CHANGE(IDC_LAST_ACC, OnChangeLastAcc)
  ON_BN_CLICKED(IDDOUCASE, OnDoucase)
  ON_EN_CHANGE(IDC_CREATED_MS, OnChangeCreated)
  ON_EN_CHANGE(IDC_CREATED_TZ, OnChangeCreated)
  ON_EN_CHANGE(IDC_MODIFIED_MS, OnChangeModified)
  ON_EN_CHANGE(IDC_MODIFIED_TZ, OnChangeModified)
  ON_EN_CHANGE(IDC_LAST_ACC_TZ, OnChangeLastAcc)
  ON_EN_KILLFOCUS(IDC_ENTRY_GUID, OnKillfocusEntryGuid)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

int label_disables[] = {
  IDC_SEC_COUNT, IDC_CRC, IDC_CRC_UPDATE, IDDOATTRIB, IDC_ATTRIB,
  IDSFN_CLEAR0, IDCREATED, IDC_CREATED, IDC_CREATED_MS, IDC_CREATED_TZ,
  IDLASTMOD, IDC_MODIFIED, IDC_MODIFIED_MS, IDC_MODIFIED_TZ,
  IDLASTACC, IDC_LAST_ACC, IDC_LAST_ACC_TZ,
  IDFLAGS, IDC_FLAGS, IDLFN_CLEAR2, IDC_NAME_HASH,
  IDC_HASH_UPDATE, IDLFN_CLEAR3, IDC_VALID_DATA_LEN,
  IDLFN_CLEAR4, IDC_ENTRY_CLUSTER, IDC_DATA_LEN,
  -1
};

int ucase_disables[] = {
  IDC_SEC_COUNT, IDDOATTRIB, IDC_ATTRIB, IDC_NAME_LEN, IDC_ENTRY_NAME,
  IDCREATED, IDC_CREATED, IDC_CREATED_MS, IDC_CREATED_TZ,
  IDLASTMOD, IDC_MODIFIED, IDC_MODIFIED_MS, IDC_MODIFIED_TZ,
  IDLASTACC, IDC_LAST_ACC, IDC_LAST_ACC_TZ,
  IDFLAGS, IDC_FLAGS, IDLFN_CLEAR2, IDC_NAME_HASH,
  IDC_HASH_UPDATE, IDLFN_CLEAR3, IDC_VALID_DATA_LEN,
  IDLFN_CLEAR4,
  -1
};

int bitmap_disables[] = {
  IDC_SEC_COUNT, IDC_CRC, IDC_CRC_UPDATE, IDDOATTRIB, IDC_ATTRIB, 
  IDC_NAME_LEN, IDC_ENTRY_NAME, IDCREATED, IDC_CREATED, IDC_CREATED_MS,
  IDC_CREATED_TZ, IDLASTMOD, IDC_MODIFIED, IDC_MODIFIED_MS, IDC_MODIFIED_TZ,
  IDLASTACC, IDC_LAST_ACC, IDC_LAST_ACC_TZ, IDC_ENTRY_RESERVED0, IDSFN_CLEAR0,
  IDLFN_CLEAR2, IDC_NAME_HASH,
  IDC_HASH_UPDATE, IDLFN_CLEAR3, IDC_VALID_DATA_LEN,
  IDLFN_CLEAR4,
  -1
};

int guid_disables[] = {
  IDDOATTRIB, IDC_ATTRIB, IDC_NAME_LEN, IDC_ENTRY_NAME, IDCREATED, 
  IDC_CREATED, IDC_CREATED_MS, IDC_CREATED_TZ, IDLASTMOD, IDC_MODIFIED,
  IDC_MODIFIED_MS, IDC_MODIFIED_TZ, IDLASTACC, IDC_LAST_ACC, 
  IDC_LAST_ACC_TZ, IDC_ENTRY_RESERVED0, IDSFN_CLEAR0,
  IDLFN_CLEAR2, IDC_NAME_HASH, IDC_ENTRY_NAME,
  IDC_HASH_UPDATE, IDLFN_CLEAR3, IDC_VALID_DATA_LEN,
  IDLFN_CLEAR4, IDC_ENTRY_CLUSTER, IDC_DATA_LEN,
  -1
};

/////////////////////////////////////////////////////////////////////////////
// CExFatEntry message handlers
BOOL CExFatEntry::OnInitDialog() {
  struct S_EXFAT_ROOT *root;
  WORD namestr[256];
  int name_len, j, secondary_count;
  CString cs;
  
  if (LoadEntry()) {
    // fill dialog here
    root = (struct S_EXFAT_ROOT *) ((BYTE *) m_entry_buffer + m_root_offset);
    memset(namestr, 0, 256);
    m_Cluster = 0;
    m_DataLen = 0;
    m_EntryFlags = 0;
    
    if (root[0].entry_type == EXFAT_DIR_ENTRY) {
      m_secondaries.Format("%i", secondary_count = root[0].type.dir_entry.sec_count);
      m_crc.Format("0x%04X", root[0].type.dir_entry.crc);
      m_attribute.Format("0x%04X", root[0].type.dir_entry.attributes);
      m_reserved0.Format("0x%04X", root[0].type.dir_entry.resv1);
      m_created.Format("0x%08X", root[0].type.dir_entry.created);
      m_created_ms.Format("%03i", root[0].type.dir_entry.created_ms);
      m_created_tz.Format("%i", root[0].type.dir_entry.created_tz_offset);
      m_modified.Format("0x%08X", root[0].type.dir_entry.last_mod);
      m_modified_ms.Format("%03i", root[0].type.dir_entry.last_mod_ms);
      m_modified_tz.Format("%i", root[0].type.dir_entry.last_mod_tz_offset);
      m_last_acc.Format("0x%08X", root[0].type.dir_entry.last_acc);
      m_last_acc_tz.Format("%i", root[0].type.dir_entry.last_access_tz_offset);
      m_reserved1.Format("%02X %02X %02X %02X %02X %02X %02X", root[0].type.dir_entry.resv2[0], root[0].type.dir_entry.resv2[1], root[0].type.dir_entry.resv2[2],
        root[0].type.dir_entry.resv2[3], root[0].type.dir_entry.resv2[4], root[0].type.dir_entry.resv2[5], root[0].type.dir_entry.resv2[6]);
      
      m_flags.Format("0x%02X", m_EntryFlags = root[1].type.stream_ext.flags);
      m_reserved2.Format("0x%02X", root[1].type.stream_ext.resv1);
      m_name_len.Format("%i", name_len = root[1].type.stream_ext.name_len);
      m_name_hash.Format("0x%04X", root[1].type.stream_ext.name_hash);
      m_reserved3.Format("0x%04X", root[1].type.stream_ext.resv2);
      m_valid_data_len.Format("%I64i", root[1].type.stream_ext.valid_data_len);
      m_reserved4.Format("0x%08X", root[1].type.stream_ext.resv3);
      m_cluster.Format("0x%08X", m_Cluster = root[1].type.stream_ext.first_cluster);
      m_data_len.Format("%i", m_DataLen = root[1].type.stream_ext.data_len);
      // Get the name
      j = 0;
      for (int s=2; s<=secondary_count && s<(EXFAT_MAX_SECONDARY_COUNT - 3); s++) {
        for (int nn=0; nn<15 && j<255; nn++) {
          namestr[j++] = root[s].type.file_name_ext.name[nn];
          if (j == name_len) {
            s = secondary_count;
            break;
          }
        }
      }
      namestr[j] = '\0';
      WideCharToMultiByte(CP_ACP, 0, (LPCWCH) namestr, -1, m_name.GetBuffer(256), 256, NULL, NULL); m_name.ReleaseBuffer(256);
      GetDlgItem(IDFATENTRIES)->EnableWindow((m_Cluster > 0) && !(m_EntryFlags & EXFAT_FLAGS_NO_FAT));
      
    } else if (root[0].entry_type == EXFAT_DIR_LABEL) {
      j = 0;
      while (label_disables[j] > -1)
        GetDlgItem(label_disables[j++])->EnableWindow(FALSE);
      
      // get the name
      m_name_len.Format("%i", name_len = root[0].type.label.len);
      for (j=0; j<name_len && j<11; j++)
        namestr[j] = root[0].type.label.name[j];
      namestr[j] = '\0';
      WideCharToMultiByte(CP_ACP, 0, (LPCWCH) namestr, -1, m_name.GetBuffer(256), 256, NULL, NULL); m_name.ReleaseBuffer(256);
      m_reserved1.Format("%02X %02X %02X %02X %02X %02X %02X %02X", root[0].type.label.resv1[0], root[0].type.label.resv1[1], root[0].type.label.resv1[2],
        root[0].type.label.resv1[3], root[0].type.label.resv1[4], root[0].type.label.resv1[5], root[0].type.label.resv1[6], root[0].type.label.resv1[7]);
      
    } else if (root[0].entry_type == EXFAT_DIR_UCASE) {
      j = 0;
      while (ucase_disables[j] > -1)
        GetDlgItem(ucase_disables[j++])->EnableWindow(FALSE);
      
      m_reserved0.Format("0x%06X", (* (DWORD *) root[0].type.up_case_table.resv1) & 0x00FFFFFF);
      m_crc.Format("0x%08X", root[0].type.up_case_table.crc);
      m_reserved1.Empty();
      for (j=0; j<12; j++) {
        cs.Format("%02X ", root[0].type.up_case_table.resv2[j]);
        m_reserved1 += cs;
      }
      m_cluster.Format("0x%08X", m_Cluster = root[0].type.up_case_table.cluster_strt);
      m_data_len.Format("%I64i", m_DataLen = root[0].type.up_case_table.data_len);
      
      // load the ucase data
      m_ucase_buffer = m_Parent->ReadFile(m_Cluster, NULL, 0);
      
      GetDlgItem(IDDOUCASE)->SetWindowText("UCase");
      GetDlgItem(IDDOUCASE)->EnableWindow(TRUE);
      
    } else if (root[0].entry_type == EXFAT_DIR_BITMAP) {
      j = 0;
      while (bitmap_disables[j] > -1)
        GetDlgItem(bitmap_disables[j++])->EnableWindow(FALSE);
      
      m_flags.Format("0x%02X", root[0].type.bitmap.flags);
      m_reserved1.Empty();
      for (j=0; j<18; j++) {
        cs.Format("%02X ", root[0].type.bitmap.resv1[j]);
        m_reserved1 += cs;
      }
      m_cluster.Format("0x%08X", m_Cluster = root[0].type.bitmap.cluster_strt);
      m_data_len.Format("%I64i", m_DataLen = root[0].type.bitmap.data_len);
      
      // load the bitmap data
      m_ucase_buffer = m_Parent->ReadFile(m_Cluster, NULL, 0);
      
      GetDlgItem(IDDOUCASE)->SetWindowText("Bitmap");
      GetDlgItem(IDDOUCASE)->EnableWindow(TRUE);
      
    } else if (root[0].entry_type == EXFAT_DIR_GUID) {
      j = 0;
      while (guid_disables[j] > -1)
        GetDlgItem(guid_disables[j++])->EnableWindow(FALSE);
      
      m_secondaries.Format("%i", secondary_count = root[0].type.guid.sec_count);
      m_crc.Format("0x%04X", root[0].type.guid.crc);
      m_flags.Format("0x%04X", root[0].type.guid.flags);
      GUID_Format(m_guid, &root[0].type.guid.guid);
      m_reserved1.Empty();
      for (j=0; j<10; j++) {
        cs.Format("%02X ", root[0].type.guid.resv1[j]);
        m_reserved1 += cs;
      }
      GetDlgItem(IDC_ENTRY_GUID)->EnableWindow(TRUE);
      
    } else {
      GetDlgItem(IDC_ENTRY_NAME)->EnableWindow(FALSE);
    }
    
    m_error_code.Format("0x%04X", 0); // TODO
    
    CDialog::OnInitDialog();
    
    if (root[0].entry_type == EXFAT_DIR_ENTRY) {
      OnChangeCreated();
      OnChangeModified();
      OnChangeLastAcc();
    }
    
    return TRUE;
  } else {
    CDialog::OnCancel();
    return TRUE;
  }
}

void CExFatEntry::UpdateEntry(void) {
  struct S_EXFAT_ROOT *root;
  WORD namestr[256];
  CString csName;
  int j = 0, secondary_count, name_len;
  
  UpdateData(TRUE);
  root = (struct S_EXFAT_ROOT *) ((BYTE *) m_entry_buffer + m_root_offset);
  if (root[0].entry_type == EXFAT_DIR_ENTRY) {
    secondary_count = 
      root[0].type.dir_entry.sec_count = convert8(m_secondaries);
    root[0].type.dir_entry.crc = convert16(m_crc);
    root[0].type.dir_entry.attributes = convert16(m_attribute);
    root[0].type.dir_entry.resv1 = convert16(m_reserved0);
    root[0].type.dir_entry.created = convert32(m_created);
    root[0].type.dir_entry.created_ms = convert8(m_created_ms);
    root[0].type.dir_entry.created_tz_offset = convert8(m_created_tz);
    root[0].type.dir_entry.last_mod = convert32(m_modified);
    root[0].type.dir_entry.last_mod_ms = convert8(m_modified_ms);
    root[0].type.dir_entry.last_mod_tz_offset = convert8(m_modified_tz);
    root[0].type.dir_entry.last_acc = convert32(m_last_acc);
    root[0].type.dir_entry.last_access_tz_offset = convert8(m_last_acc_tz);
    ConvertDumpToBuffer(m_reserved1, root[0].type.dir_entry.resv2, 7);
    
    root[1].type.stream_ext.flags = convert8(m_flags);
    root[1].type.stream_ext.resv1 = convert8(m_reserved2);
    name_len =
      root[1].type.stream_ext.name_len = convert8(m_name_len);
    root[1].type.stream_ext.name_hash = convert16(m_name_hash);
    root[1].type.stream_ext.resv2 = convert16(m_reserved3);
    root[1].type.stream_ext.valid_data_len = convert64(m_valid_data_len);
    root[1].type.stream_ext.resv3 = convert32(m_reserved4);
    root[1].type.stream_ext.first_cluster = convert32(m_cluster);
    root[1].type.stream_ext.data_len = convert32(m_data_len);
    
    for (j=0; j<name_len && j<255; j++)
      namestr[j] = m_name.GetAt(j);
    namestr[j] = '\0';
    MultiByteToWideChar(CP_ACP, 0, csName, -1, (LPWSTR) namestr, 72);
    j = 0;
    for (int s=2; s<=secondary_count && s<(EXFAT_MAX_SECONDARY_COUNT - 3); s++) {
      for (int nn=0; nn<15 && j<255; nn++) {
        if (j >= csName.GetLength())
          break;
        root[s].type.file_name_ext.name[nn] = csName.GetAt(j);
        if (++j == name_len) {
          s = secondary_count;
          break;
        }
      }
    }
    ConvertDumpToBuffer(m_reserved1, root[0].type.label.resv1, 8);
  } else if (root[0].entry_type == EXFAT_DIR_LABEL) {
    // get the name
    name_len =
    root[0].type.label.len = convert8(m_name_len);
    for (j=0; j<name_len && j<11; j++)
      namestr[j] = m_name.GetAt(j);
    namestr[j] = '\0';
    MultiByteToWideChar(CP_ACP, 0, csName, -1, (LPWSTR) namestr, 72);
    for (j=0; j<name_len && j<11; j++)
      root[0].type.label.name[j] = csName.GetAt(j);
  
  } else if (root[0].entry_type == EXFAT_DIR_UCASE) {
    root[0].type.up_case_table.resv1[0] = (convert32(m_reserved0) & 0x0000FF) >>  0;
    root[0].type.up_case_table.resv1[1] = (convert32(m_reserved0) & 0x00FF00) >>  8;
    root[0].type.up_case_table.resv1[2] = (convert32(m_reserved0) & 0xFF0000) >> 16;
    root[0].type.up_case_table.crc = convert32(m_crc);
    ConvertDumpToBuffer(m_reserved1, root[0].type.up_case_table.resv2, 12);
    root[0].type.up_case_table.cluster_strt = convert32(m_cluster);
    root[0].type.up_case_table.data_len = convert64(m_data_len);
    
  } else if (root[0].entry_type == EXFAT_DIR_BITMAP) {
    root[0].type.bitmap.flags = convert8(m_flags);
    ConvertDumpToBuffer(m_reserved1, root[0].type.bitmap.resv1, 18);
    root[0].type.bitmap.cluster_strt = convert32(m_cluster);
    root[0].type.bitmap.data_len = convert64(m_data_len);
    
  } else if (root[0].entry_type == EXFAT_DIR_GUID) {
    root[0].type.guid.sec_count = convert8(m_secondaries);
    root[0].type.guid.crc = convert16(m_crc);
    root[0].type.guid.flags = convert16(m_flags);
    GUID_Retrieve(m_guid, &root[0].type.guid.guid);
    ConvertDumpToBuffer(m_reserved1, root[0].type.guid.resv1, 10);
  }
}

void CExFatEntry::OnOK() {
  UpdateEntry();
  SaveEntry();
  CDialog::OnOK();
}

void CExFatEntry::OnCancel() {
  if (m_entry_buffer)
    free(m_entry_buffer);
  if (m_ucase_buffer)
    free(m_ucase_buffer);
  CDialog::OnCancel();
}

BOOL CExFatEntry::LoadEntry(void) {
  m_entry_buffer = m_Parent->ReadFile(m_root_cluster, &m_buffer_size, m_Flags);
  if (m_entry_buffer)
    return TRUE;
  else {
    AfxMessageBox("Error reading parent root directory block.");
    return FALSE;
  }
}

void CExFatEntry::SaveEntry(void) {
  m_Parent->WriteFile(m_entry_buffer, m_root_cluster, m_buffer_size, m_Flags);
  if (m_entry_buffer)
    free(m_entry_buffer);
  
  // write UCase/Bitmap back?
  if (m_ucase_buffer) {
    m_Parent->WriteFile(m_ucase_buffer, m_Cluster, m_DataLen, 0);
    free(m_ucase_buffer);
  }
}

struct S_ATTRIBUTES exfat_attrbs[] = {
                                           //            |                               | <- max (col 67)
  { EXFAT_ATTR_READ_ONLY,    EXFAT_ATTR_READ_ONLY,    0, "Read Only"                      , {-1, } },
  { EXFAT_ATTR_HIDDEN,       EXFAT_ATTR_HIDDEN,       1, "Hidden"                         , {-1, } },
  { EXFAT_ATTR_SYSTEM,       EXFAT_ATTR_SYSTEM,       2, "System"                         , {-1, } },
  { EXFAT_ATTR_SUB_DIR,      EXFAT_ATTR_SUB_DIR,      3, "Directory"                      , {-1, } },
  { EXFAT_ATTR_ARCHIVE,      EXFAT_ATTR_ARCHIVE,      4, "Archive"                        , {-1, } },
  { 0,                       (DWORD)  -1,            -1, NULL                             , {-1, } }
};

void CExFatEntry::OnDoattrib() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_ATTRIB, cs);
  dlg.m_title = "Attribute";
  dlg.m_attrib = convert16(cs);
  dlg.m_attributes = exfat_attrbs;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_ATTRIB, cs);
  }
}

struct S_ATTRIBUTES exfat_dir_flags[] = {
                                           //            |                               | <- max (col 67)
  { EXFAT_FLAGS_ALL_POS,     EXFAT_FLAGS_ALL_POS,     0, "Allocation Possible"            , {-1, } },
  { EXFAT_FLAGS_NO_FAT,      EXFAT_FLAGS_NO_FAT,      1, "No FAT Chain"                   , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

struct S_ATTRIBUTES exfat_bp_flags[] = {
                                           //            |                               | <- max (col 67)
  { (1<<0),                  (1<<0),                  0, "Is 2nd Bitmap"                  , {-1, } },
  { 0,                   (DWORD) -1,                 -1, NULL                             , {-1, } }
};

void CExFatEntry::OnFlags() {
  struct S_EXFAT_ROOT *root = (struct S_EXFAT_ROOT *) ((BYTE *) m_entry_buffer + m_root_offset);
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FLAGS, cs);
  if ((root[0].entry_type == EXFAT_DIR_ENTRY) ||
      (root[0].entry_type == EXFAT_DIR_GUID)) {
    dlg.m_title = "Directory Flags";
    dlg.m_attrib = convert16(cs);
    dlg.m_attributes = exfat_dir_flags;
  } else if (root[0].entry_type == EXFAT_DIR_BITMAP) {
    dlg.m_title = "Bitmap Flags";
    dlg.m_attrib = convert8(cs);
    dlg.m_attributes = exfat_bp_flags;
  } else
    return;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_FLAGS, cs);
  }
}

void CExFatEntry::OnFatentries() {
  struct S_FAT_ENTRIES fat_entries;
  CFatCList list;
  
  m_Parent->ExFatFillClusterList(&fat_entries, m_Cluster, m_EntryFlags);
  if (fat_entries.entry_count > 0) {
    list.m_fat_size = FS_FAT32;
    list.m_entries = &fat_entries;
    list.DoModal();
  }
  
  if (fat_entries.entries)
    free(fat_entries.entries);
}

// convert the file from EXFAT_FLAGS_NO_FAT to FAT clusters, or visaversa
void CExFatEntry::OnFatConvert() {
  const DWORD64 bytes_per_cluster = ((DWORD64) 1 << convert8(m_Parent->m_log_bytes_cluster)) * ((DWORD64) 1 << convert8(m_Parent->m_log_bytes_sect));
  int Count = (int) ((m_DataLen + (bytes_per_cluster - 1)) / bytes_per_cluster);
  DWORD *fat = (DWORD *) m_Parent->m_fat_buffer;
  DWORD Cluster = m_Cluster;
  
  if (m_EntryFlags & EXFAT_FLAGS_NO_FAT) {
    // if already EXFAT_FLAGS_NO_FAT, we need to update the FAT
    while (Count--) {
      fat[Cluster] = Cluster + 1;
      Cluster++;
    }
    fat[Cluster - 1] = 0xFFFFFFFF;
  } else {
    // else, clear the FAT
    while (Count--) {
      fat[Cluster] = 0;
      Cluster++;
    }
  }
  m_Parent->ExFatWriteFAT(fat);
  m_EntryFlags ^= EXFAT_FLAGS_NO_FAT;
  
  CString cs;
  cs.Format("0x%04X", m_EntryFlags);
  SetDlgItemText(IDC_FLAGS, cs);
  
  if (AfxMessageBox("Update CRC?", MB_YESNO, NULL) == IDYES)
    OnCrcUpdate();
}

void CExFatEntry::OnClear0() {
  SetDlgItemText(IDC_ENTRY_RESERVED0, "0x0000");
}

void CExFatEntry::OnClear1() {
  SetDlgItemText(IDC_ENTRY_RESERVED1, "00 00 00 00 00 00 00");
}

void CExFatEntry::OnClear2() {
  SetDlgItemText(IDC_ENTRY_RESERVED2, "0x00");
}

void CExFatEntry::OnClear3() {
  SetDlgItemText(IDC_ENTRY_RESERVED3, "0x0000");
}

void CExFatEntry::OnClear4() {
  SetDlgItemText(IDC_ENTRY_RESERVED4, "0x00000000");
}

void CExFatEntry::OnCrcUpdate() {
  struct S_EXFAT_ROOT *root = (struct S_EXFAT_ROOT *) ((BYTE *) m_entry_buffer + m_root_offset);
  int secondary_count;
  WORD crc;
  
  UpdateEntry();
  if (root[0].entry_type == EXFAT_DIR_ENTRY) {
    secondary_count = convert8(m_secondaries);
    crc = m_Parent->ExFatCheckDirCRC((BYTE *) root, ((secondary_count + 1) * sizeof(struct S_EXFAT_ROOT)));
    m_crc.Format("0x%04X", crc);
    SetDlgItemText(IDC_CRC, m_crc);
    
  } else if (root[0].entry_type == EXFAT_DIR_UCASE) {
    BYTE *p = (BYTE *) m_ucase_buffer;
    DWORD crc = 0;
    for (int i=0; i<m_DataLen; i++)
      crc = ((crc << 31) | (crc >> 1)) + (DWORD) p[i];
    m_crc.Format("0x%08X", crc);
    SetDlgItemText(IDC_CRC, m_crc);
    
  } else if (root[0].entry_type == EXFAT_DIR_GUID) {
    // must copy to a temp buffer so we don't modify root[]
    //  by setting the crc to zero before the calculation
    BYTE entry[32];
    memcpy(entry, &root[0], 32);
    entry[1] = entry[2] = 0;
    WORD crc = 0;
    for (int i=0; i<32; i++)
      crc = ((crc << 15) | (crc >> 1)) + (WORD) entry[i];
    m_crc.Format("0x%04X", crc);
    SetDlgItemText(IDC_CRC, m_crc);
  }
}

void CExFatEntry::OnDoucase() {
  struct S_EXFAT_ROOT *root = (struct S_EXFAT_ROOT *) ((BYTE *) m_entry_buffer + m_root_offset);
  CString cs;
  
  if (root[0].entry_type == EXFAT_DIR_UCASE) {
    CExFatUCase UCase;
    UCase.m_data_len = (DWORD) m_DataLen;
    UCase.m_buffer = malloc(UCase.m_data_len);
    memcpy(UCase.m_buffer, m_ucase_buffer, UCase.m_data_len);
    UCase.m_count = UCase.m_data_len / sizeof(WORD);
    if (UCase.DoModal() == IDOK)
      memcpy(m_ucase_buffer, UCase.m_buffer, UCase.m_data_len);
    
  } else if (root[0].entry_type == EXFAT_DIR_BITMAP) {
    CViewBitmap Bitmap;
    Bitmap.m_data_len = (DWORD) m_DataLen;
    Bitmap.m_buffer = malloc(Bitmap.m_data_len);
    Bitmap.m_item_name = "Cluster";
    memcpy(Bitmap.m_buffer, m_ucase_buffer, Bitmap.m_data_len);
    Bitmap.m_count = m_Cluster_Count;
    if (Bitmap.DoModal() == IDOK)
      memcpy(m_ucase_buffer, Bitmap.m_buffer, Bitmap.m_data_len);
  }
}

struct S_TIME_ZONE {
  BYTE   hex;
  float  off;
} TimeZone[] = {
  { 0xB8,  14.00 },
  { 0xB4,  13.00 },
  { 0xB0,  12.00 },
  { 0xAC,  11.00 },
  { 0xA8,  10.00 },
  { 0xA6,   9.50 },
  { 0xA4,   9.00 },
  { 0xA0,   8.00 },
  { 0x9C,   7.00 },
  { 0x9A,   6.50 },
  { 0x98,   6.00 },
  { 0x97,   5.75 },
  { 0x96,   5.50 },
  { 0x94,   5.00 },
  { 0x92,   4.50 },
  { 0x90,   4.00 },
  { 0x8E,   3.50 },
  { 0x8C,   3.00 },
  { 0x88,   2.00 },
  { 0x84,   1.00 },  
  { 0x80,   0.00 },  // GMT (UTC)
  { 0xFC,  -1.00 },
  { 0xF8,  -2.00 },
  { 0xF4,  -3.00 },
  { 0xF2,  -3.50 },
  { 0xF0,  -4.00 },
  { 0xEC,  -5.00 },
  { 0xEC,  -5.00 },
  { 0xE8,  -6.00 },
  { 0xE4,  -7.00 },
  { 0xE0,  -8.00 },
  { 0xDC,  -9.00 },
  { 0xD8, -10.00 },
  { 0xD4, -11.00 },
  { 0xD0, -12.00 },
  { 0xFF, }
};

float CExFatEntry::GetUTCOff(BYTE val) {
  int i = 0;
  while (TimeZone[i].hex != 0xFF) {
    if (TimeZone[i].hex == val)
      return TimeZone[i].off;
    i++;
  }
  return 0.0;
}

BYTE CExFatEntry::GetUTCVal(float off) {
  int i = 0;
  while (TimeZone[i].hex != 0xFF) {
    if (TimeZone[i].off == off)
      return TimeZone[i].hex;
    i++;
  }
  return 0x80;
}

void CExFatEntry::OnCreated() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned t = ((((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay()) << 16) |
               ((time.GetHour() << 11) | (time.GetMinute() << 5) | (time.GetSecond() >> 1));
  cs.Format("%03i", ((time.GetSecond() & 1) * 100) + (rand() % 99));  // rand() just for fun since we don't have mS timing
  SetDlgItemText(IDC_CREATED_MS, cs);
  cs.Format("%i", GetUTCVal(GetUTCOffset()));
  SetDlgItemText(IDC_CREATED_TZ, cs);
  cs.Format("0x%08X", t);
  SetDlgItemText(IDC_CREATED, cs);
}

void CExFatEntry::OnLastmod() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned t = ((((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay()) << 16) |
               ((time.GetHour() << 11) | (time.GetMinute() << 5) | (time.GetSecond() >> 1));
  cs.Format("%03i", ((time.GetSecond() & 1) * 100) + (rand() % 99));  // rand() just for fun since we don't have mS timing
  SetDlgItemText(IDC_MODIFIED_MS, cs);
  cs.Format("%i", GetUTCVal(GetUTCOffset()));
  SetDlgItemText(IDC_MODIFIED_TZ, cs);
  cs.Format("0x%08X", t);
  SetDlgItemText(IDC_MODIFIED, cs);
}

void CExFatEntry::OnLastacc() {
  CString cs;
  CTime time = CTime::GetCurrentTime();
  unsigned t = ((((time.GetYear() - 1980) << 9) | (time.GetMonth() << 5) | time.GetDay()) << 16) |
               ((time.GetHour() << 11) | (time.GetMinute() << 5) | (time.GetSecond() >> 1));
  cs.Format("%i", GetUTCVal(GetUTCOffset()));
  SetDlgItemText(IDC_LAST_ACC_TZ, cs);
  cs.Format("0x%08X", t);
  SetDlgItemText(IDC_LAST_ACC, cs);
}

void CExFatEntry::OnChangeCreated() {
  CString cs, time, ms, tz;
  GetDlgItemText(IDC_CREATED, time);
  GetDlgItemText(IDC_CREATED_MS, ms);
  GetDlgItemText(IDC_CREATED_TZ, tz);
  
  unsigned odd = 0;
  float off = GetUTCOff(convert8(tz));
  unsigned m = convert8(ms);
  unsigned t = convert32(time);
  unsigned d = (t >> 16);
  if (m >= 100) { odd++; m -= 100; }
  cs.Format("%04i/%02i/%02i %02i:%02i:%02i.%03i %.2f", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0, (t & 0xF800) >> 11, (t & 0x07E0) >> 5, ((t & 0x001F) << 1) + odd, m * 10, off);
  
  SetDlgItemText(IDC_CREATED_DISP, cs);
}

void CExFatEntry::OnChangeModified() {
  CString cs, time, ms, tz;
  GetDlgItemText(IDC_MODIFIED, time);
  GetDlgItemText(IDC_MODIFIED_MS, ms);
  GetDlgItemText(IDC_MODIFIED_TZ, tz);
  
  unsigned odd = 0;
  float off = GetUTCOff(convert8(tz));
  unsigned m = convert32(ms);
  unsigned t = convert32(time);
  unsigned d = (t >> 16);
  if (m >= 100) { odd++; m -= 100; }
  cs.Format("%04i/%02i/%02i %02i:%02i:%02i.%03i %.2f", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0, (t & 0xF800) >> 11, (t & 0x07E0) >> 5, ((t & 0x001F) << 1) + odd, m * 10, off);
  
  SetDlgItemText(IDC_MODIFIED_DISP, cs);
}

void CExFatEntry::OnChangeLastAcc() {
  CString cs, time, ms, tz;
  GetDlgItemText(IDC_LAST_ACC, time);
  GetDlgItemText(IDC_LAST_ACC_TZ, tz);
  
  float off = GetUTCOff(convert8(tz));
  unsigned t = convert32(time);
  unsigned d = (t >> 16);
  cs.Format("%04i/%02i/%02i %02i:%02i:%02i.000 %.2f", (d >> 9) + 1980, (d & 0x01E0) >> 5, (d & 0x001F) >> 0, (t & 0xF800) >> 11, (t & 0x07E0) >> 5, ((t & 0x001F) << 1), off);
  
  SetDlgItemText(IDC_LAST_ACC_DISP, cs);
}

void CExFatEntry::OnKillfocusEntryGuid() {
  CString cs;
  GetDlgItemText(IDC_ENTRY_GUID, cs);
  if (!GUID_CheckFormat(cs)) {
    AfxMessageBox("GUID has illegal format\r\n"
                  "Must be in XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format\r\n"
                  "with only hexadecimal characters.");
    GetDlgItem(IDC_ENTRY_GUID)->SetFocus();
  }
}
