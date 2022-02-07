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

// NTFSEntry.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "NTFS.h"
#include "NTFSEntry.h"

#include "LeanTime.h"
#include "Attribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNTFSEntry dialog
CNTFSEntry::CNTFSEntry(CWnd* pParent /*=NULL*/)
  : CDialog(CNTFSEntry::IDD, pParent) {
  //{{AFX_DATA_INIT(CNTFSEntry)
  m_alloc_size = _T("");
  m_attribs_off = _T("");
  m_base_file_ref = _T("");
  m_fixup_count = _T("");
  m_fixup_off = _T("");
  m_flags = _T("");
  m_hard_link_count = _T("");
  m_last_op_lsn = _T("");
  m_magic = _T("");
  m_next_attrib_id = _T("");
  m_rec_len = _T("");
  m_sequ_num = _T("");
  m_s_comp_flag = _T("");
  m_s_content_off = _T("");
  m_s_id = _T("");
  m_s_len = _T("");
  m_s_name_len = _T("");
  m_s_res_flag = _T("");
  m_s_type = _T("");
  m_s_class_id = _T("");
  m_s_creation = _T("");
  m_s_dos_perms = _T("");
  m_s_last_access = _T("");
  m_s_last_mod = _T("");
  m_s_last_mod_rec = _T("");
  m_s_max_version = _T("");
  m_s_owner_id = _T("");
  m_s_quota = _T("");
  m_s_security_id = _T("");
  m_s_usn = _T("");
  m_s_version = _T("");
  m_f_comp_flag = _T("");
  m_f_content_off = _T("");
  m_f_id = _T("");
  m_f_len = _T("");
  m_f_name_len = _T("");
  m_f_res_flag = _T("");
  m_f_type = _T("");
  m_f_alloc_size = _T("");
  m_f_creation = _T("");
  m_f_file_size = _T("");
  m_f_filename = _T("");
  m_f_filename_len = _T("");
  m_f_flags = _T("");
  m_f_last_access = _T("");
  m_f_last_mod = _T("");
  m_f_last_mod_rec = _T("");
  m_f_ref = _T("");
  m_f_sequ = _T("");
  m_d_comp_flag = _T("");
  m_d_content_off = _T("");
  m_d_id = _T("");
  m_d_len = _T("");
  m_d_name_len = _T("");
  m_d_res_flag = _T("");
  m_d_type = _T("");
  m_d_alloc_size = _T("");
  m_d_data_len = _T("");
  m_d_data_off = _T("");
  m_d_engine_num = _T("");
  m_d_index_flag = _T("");
  m_d_initial_size = _T("");
  m_d_last_vcn = _T("");
  m_d_padding = _T("");
  m_d_real_size = _T("");
  m_d_run_list = _T("");
  m_d_start_vcn = _T("");
  m_d_unused = _T("");
  //}}AFX_DATA_INIT
}

void CNTFSEntry::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CNTFSEntry)
  DDX_Text(pDX, IDC_ALLOC_SIZE, m_alloc_size);
  DDX_Text(pDX, IDC_ATTRIBS_OFF, m_attribs_off);
  DDX_Text(pDX, IDC_BASE_FILE_REF, m_base_file_ref);
  DDX_Text(pDX, IDC_FIXUP_COUNT, m_fixup_count);
  DDX_Text(pDX, IDC_FIXUP_OFF, m_fixup_off);
  DDX_Text(pDX, IDC_FLAGS, m_flags);
  DDX_Text(pDX, IDC_HARD_LINK_COUNT, m_hard_link_count);
  DDX_Text(pDX, IDC_LAST_OP_LSN, m_last_op_lsn);
  DDX_Text(pDX, IDC_MAGIC, m_magic);
  DDX_Text(pDX, IDC_NEXT_ATTRIB_ID, m_next_attrib_id);
  DDX_Text(pDX, IDC_REC_LEN, m_rec_len);
  DDX_Text(pDX, IDC_SEQU_NUM, m_sequ_num);
  DDX_Text(pDX, IDC_S_COMP_FLAG, m_s_comp_flag);
  DDX_Text(pDX, IDC_S_CONTENT_OFF, m_s_content_off);
  DDX_Text(pDX, IDC_S_ID, m_s_id);
  DDX_Text(pDX, IDC_S_LEN, m_s_len);
  DDX_Text(pDX, IDC_S_NAME_LEN, m_s_name_len);
  DDX_Text(pDX, IDC_S_RES_FLAG, m_s_res_flag);
  DDX_Text(pDX, IDC_S_TYPE, m_s_type);
  DDX_Text(pDX, IDC_S_CLASS_ID, m_s_class_id);
  DDX_Text(pDX, IDC_S_CREATION, m_s_creation);
  DDX_Text(pDX, IDC_S_DOS_PERM, m_s_dos_perms);
  DDX_Text(pDX, IDC_S_LAST_ACCESS, m_s_last_access);
  DDX_Text(pDX, IDC_S_LAST_MOD, m_s_last_mod);
  DDX_Text(pDX, IDC_S_LAST_MOD_REC, m_s_last_mod_rec);
  DDX_Text(pDX, IDC_S_MAX_VERSION, m_s_max_version);
  DDX_Text(pDX, IDC_S_OWNER_ID, m_s_owner_id);
  DDX_Text(pDX, IDC_S_QUOTA, m_s_quota);
  DDX_Text(pDX, IDC_S_SECURITY_ID, m_s_security_id);
  DDX_Text(pDX, IDC_S_USN, m_s_usn);
  DDX_Text(pDX, IDC_S_VERSION, m_s_version);
  DDX_Text(pDX, IDC_F_COMP_FLAG, m_f_comp_flag);
  DDX_Text(pDX, IDC_F_CONTENT_OFF, m_f_content_off);
  DDX_Text(pDX, IDC_F_ID, m_f_id);
  DDX_Text(pDX, IDC_F_LEN, m_f_len);
  DDX_Text(pDX, IDC_F_NAME_LEN, m_f_name_len);
  DDX_Text(pDX, IDC_F_RES_FLAG, m_f_res_flag);
  DDX_Text(pDX, IDC_F_TYPE, m_f_type);
  DDX_Text(pDX, IDC_F_ALLOC_SIZE, m_f_alloc_size);
  DDX_Text(pDX, IDC_F_CREATION, m_f_creation);
  DDX_Text(pDX, IDC_F_FILE_SIZE, m_f_file_size);
  DDX_Text(pDX, IDC_F_FILENAME, m_f_filename);
  DDX_Text(pDX, IDC_F_FILENAME_LEN, m_f_filename_len);
  DDX_Text(pDX, IDC_F_FLAGS, m_f_flags);
  DDX_Text(pDX, IDC_F_LAST_ACCESS, m_f_last_access);
  DDX_Text(pDX, IDC_F_LAST_MOD, m_f_last_mod);
  DDX_Text(pDX, IDC_F_LAST_MOD_REC, m_f_last_mod_rec);
  DDX_Text(pDX, IDC_F_REF, m_f_ref);
  DDX_Text(pDX, IDC_F_SEQU, m_f_sequ);
  DDX_Text(pDX, IDC_D_COMP_FLAG, m_d_comp_flag);
  DDX_Text(pDX, IDC_D_CONTENT_OFF, m_d_content_off);
  DDX_Text(pDX, IDC_D_ID, m_d_id);
  DDX_Text(pDX, IDC_D_LEN, m_d_len);
  DDX_Text(pDX, IDC_D_NAME_LEN, m_d_name_len);
  DDX_Text(pDX, IDC_D_RES_FLAG, m_d_res_flag);
  DDX_Text(pDX, IDC_D_TYPE, m_d_type);
  DDX_Text(pDX, IDC_D_ALLOC_SIZE, m_d_alloc_size);
  DDX_Text(pDX, IDC_D_DATA_LEN, m_d_data_len);
  DDX_Text(pDX, IDC_D_DATA_OFFSET, m_d_data_off);
  DDX_Text(pDX, IDC_D_ENGINE_NUM, m_d_engine_num);
  DDX_Text(pDX, IDC_D_INDEX_FLAG, m_d_index_flag);
  DDX_Text(pDX, IDC_D_INITIAL_SIZE, m_d_initial_size);
  DDX_Text(pDX, IDC_D_LAST_VCN, m_d_last_vcn);
  DDX_Text(pDX, IDC_D_PADDING, m_d_padding);
  DDX_Text(pDX, IDC_D_REAL_SIZE, m_d_real_size);
  DDX_Text(pDX, IDC_D_RUN_LIST, m_d_run_list);
  DDX_Text(pDX, IDC_D_START_VCN, m_d_start_vcn);
  DDX_Text(pDX, IDC_D_UNUSED, m_d_unused);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNTFSEntry, CDialog)
  //{{AFX_MSG_MAP(CNTFSEntry)
  ON_BN_CLICKED(IDC_FLAGS_B, OnFlags)
  ON_BN_CLICKED(IDC_D_RUN_LIST_B, OnDRunListB)
  ON_BN_CLICKED(IDC_S_CREATION_B, OnSCreationB)
  ON_BN_CLICKED(IDC_S_LAST_MOD_B, OnSLastModB)
  ON_BN_CLICKED(IDC_S_LAST_MOD_REC_B, OnSLastModRecB)
  ON_BN_CLICKED(IDC_S_LAST_ACCESS_B, OnSAccessB)
  ON_BN_CLICKED(IDC_F_CREATION_B, OnFCreationB)
  ON_BN_CLICKED(IDC_F_LAST_MOD_B, OnFLastModB)
  ON_BN_CLICKED(IDC_F_LAST_MOD_REC_B, OnFLastModRecB)
  ON_BN_CLICKED(IDC_F_LAST_ACCESS_B, OnFAccessB)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNTFSEntry message handlers
BOOL CNTFSEntry::OnInitDialog() {
  m_alloc_size.Format("%i", m_file_rec->alloc_size);
  m_attribs_off.Format("%i", m_file_rec->attribs_off);
  m_base_file_ref.Format("%I64i", m_file_rec->base_file_ref);
  m_fixup_count.Format("%i", m_file_rec->fixup_count);
  m_fixup_off.Format("%i", m_file_rec->fixup_off);
  m_flags.Format("0x%04X", m_file_rec->flags);
  m_hard_link_count.Format("%i", m_file_rec->hard_link_count);
  m_last_op_lsn.Format("%I64i", m_file_rec->last_op_lsn);
  m_magic.Format("0x%08X", m_file_rec->magic);
  m_next_attrib_id.Format("%i", m_file_rec->next_attrib_id);
  m_rec_len.Format("%i", m_file_rec->record_len);
  m_sequ_num.Format("%i", m_file_rec->sequ_number);
  
  struct S_NTFS_ATTR_STANDARD *stnds;
  struct S_NTFS_ATTR_RES *attrb_res;
  struct S_NTFS_ATTR_NONRES *attrb_nres;
  struct S_NTFS_ATTR_FILENAME *filename;
  struct S_NTFS_ATTR *attrb = (struct S_NTFS_ATTR *) ((BYTE *) m_file_rec + m_file_rec->attribs_off);
  char filename_str[256];
  BYTE filename_last_type = 0xFF;
  
  while (attrb->type != NTFS_ATTR_EOA) {
    attrb_res = (struct S_NTFS_ATTR_RES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
    attrb_nres = (struct S_NTFS_ATTR_NONRES *) ((BYTE *) attrb + sizeof(struct S_NTFS_ATTR));
    switch (attrb->type) {
      case NTFS_ATTR_STANDARD:
        m_s_comp_flag.Format("%i", attrb->comp_flag);
        m_s_content_off.Format("%i", attrb->content_off);
        m_s_id.Format("%i", attrb->id);
        m_s_len.Format("%i", attrb->len);
        m_s_name_len.Format("%i", attrb->name_len);
        m_s_res_flag.Format("%i", attrb->non_res_flag);
        m_s_type.Format("0x%02X", attrb->type);
        if (attrb->non_res_flag == 0) {
          stnds = (struct S_NTFS_ATTR_STANDARD *) ((BYTE *) attrb + attrb_res->data_off);
          m_s_class_id.Format("%i", stnds->class_id);
          m_s_creation.Format("%I64i", stnds->file_times.creation_time);
          m_s_dos_perms.Format("0x%08X", stnds->dos_permissions);
          m_s_last_access.Format("%I64i", stnds->file_times.last_access);
          m_s_last_mod.Format("%I64i", stnds->file_times.last_mod);
          m_s_last_mod_rec.Format("%I64i", stnds->file_times.last_mod_file_rec);
          m_s_max_version.Format("%i", stnds->max_ver_num);
          m_s_owner_id.Format("%i", stnds->owner_id);
          m_s_quota.Format("%I64i", stnds->quota_changed);
          m_s_security_id.Format("%i", stnds->security_id);
          m_s_usn.Format("%I64i", stnds->usn);
          m_s_version.Format("%i", stnds->ver_num);
        } else {
          // TODO:
        }
        break;
        
      case NTFS_ATTR_FILENAME:
        m_f_comp_flag.Format("%i", attrb->comp_flag);
        m_f_content_off.Format("%i", attrb->content_off);
        m_f_id.Format("%i", attrb->id);
        m_f_len.Format("%i", attrb->len);
        m_f_name_len.Format("%i", attrb->name_len);
        m_f_res_flag.Format("%i", attrb->non_res_flag);
        m_f_type.Format("0x%02X", attrb->type);
        if (attrb->non_res_flag) {
          //AfxMessageBox(" Filename: not resident");
        } else {
          filename = (struct S_NTFS_ATTR_FILENAME *) ((BYTE *) attrb + attrb_res->data_off);
          if ((filename_last_type != 1) && (filename_last_type != 3)) {
            filename_last_type = filename->filename_space;
            memset(filename_str, 0, 256);
            WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) ((BYTE *) filename + sizeof(struct S_NTFS_ATTR_FILENAME)), filename->filename_len, filename_str, 255, NULL, NULL);
          }
          m_f_alloc_size.Format("%I64i", filename->allocated_file_size);
          m_f_creation.Format("%I64i", filename->file_times.creation_time);
          m_f_file_size.Format("%I64i", filename->real_file_size);
          m_f_filename = filename_str;
          m_f_filename_len.Format("%i", filename->filename_len);
          m_f_flags.Format("%I64i", filename->flags);
          m_f_last_access.Format("%I64i", filename->file_times.last_access);
          m_f_last_mod.Format("%I64i", filename->file_times.last_mod);
          m_f_last_mod_rec.Format("%I64i", filename->file_times.last_mod_file_rec);
          m_f_ref.Format("%I64i", NTFS_GET_FILE_REF(filename->file_ref_parent));
          m_f_sequ.Format("%i", (unsigned) NTFS_GET_FILE_SEQ(filename->file_ref_parent));
        }
        break;
        
      case NTFS_ATTR_DATA:
        m_d_comp_flag.Format("%i", attrb->comp_flag);
        m_d_content_off.Format("%i", attrb->content_off);
        m_d_id.Format("%i", attrb->id);
        m_d_len.Format("%i", attrb->len);
        m_d_name_len.Format("%i", attrb->name_len);
        m_d_res_flag.Format("%i", attrb->non_res_flag);
        m_d_type.Format("0x%02X", attrb->type);
        if (!attrb->non_res_flag) {
          m_d_data_len.Format("%i", attrb_res->data_len);
          m_d_data_off.Format("%i", attrb_res->data_off);
          m_d_index_flag.Format("%i", attrb_res->indexed_flag);
          m_d_padding.Format("%i", attrb_res->padding);
        } else {
          m_d_alloc_size.Format("%I64i", attrb_nres->allocated_size);
          m_d_engine_num.Format("%i", attrb_nres->comp_engine_num);
          m_d_initial_size.Format("%I64i", attrb_nres->init_data_size);
          m_d_last_vcn.Format("%I64i", attrb_nres->last_vcn);
          m_d_real_size.Format("%I64i", attrb_nres->real_size);
          m_d_run_list.Format("%i", attrb_nres->run_list_off);
          m_d_start_vcn.Format("%I64i", attrb_nres->starting_vcn);
          m_d_unused.Format("0x%08X", attrb_nres->unused);
        }
        break;
      /*  
      default: {
        CString cs;
        cs.Format("%02X", attrb->type);
        AfxMessageBox(cs);
      }
      */
    }
    
    attrb = (struct S_NTFS_ATTR *) ((BYTE *) attrb + attrb->len);
    if (((BYTE *) attrb > (BYTE *) m_file_rec + m_file_rec->record_len) ||
       (attrb->len == 0))
      break;
  }
  
  CDialog::OnInitDialog();
  
  return TRUE;
}

struct S_ATTRIBUTES NTFS_flags[] = {
                                           //            |                               | <- max (col 67)
  { (1<<0),                  (1<<0),                  0, "non-resident"                   , {-1, } },
  { (1<<1),                  (1<<1),                  1, "directory"                      , {-1, } },
  { 0,                       (DWORD) -1,             -1, NULL                             , {-1, } }
};

void CNTFSEntry::OnFlags() {
  CAttribute dlg;
  CString cs;
  
  GetDlgItemText(IDC_FLAGS, cs);
  dlg.m_title = "Entry Flags";
  dlg.m_attrib = convert16(cs);
  dlg.m_attributes = NTFS_flags;
  if (dlg.DoModal() == IDOK) {
    cs.Format("0x%04X", dlg.m_attrib);
    SetDlgItemText(IDC_FLAGS, cs);
  }
}

void CNTFSEntry::OnDRunListB() {
  AfxMessageBox("TODO");
}

void CNTFSEntry::OnSCreationB() {
  CLeanTime dlg;
  
  dlg.m_title = "Creation Time";
  dlg.m_divisor = 100000000;
  //dlg.m_adjustment = (3 * 365 * 24 * 3600) + (4 * 30 * 24 * 3600) + (3 * 24 * 3600); // rough estimate of difference (3 years, 4 months, 3 days)
  GetDlgItemText(IDC_S_CREATION, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_S_CREATION, dlg.m_lean_time);
}

void CNTFSEntry::OnSLastModB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Mod Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_S_LAST_MOD, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_S_LAST_MOD, dlg.m_lean_time);
}

void CNTFSEntry::OnSLastModRecB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Record Mod Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_S_LAST_MOD_REC, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_S_LAST_MOD_REC, dlg.m_lean_time);
}

void CNTFSEntry::OnSAccessB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Access Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_S_LAST_ACCESS, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_S_LAST_ACCESS, dlg.m_lean_time);
}

void CNTFSEntry::OnFCreationB() {
  CLeanTime dlg;
  
  dlg.m_title = "Creation Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_F_CREATION, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_F_CREATION, dlg.m_lean_time);
}

void CNTFSEntry::OnFLastModB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Mod Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_F_LAST_MOD, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_F_LAST_MOD, dlg.m_lean_time);
}

void CNTFSEntry::OnFLastModRecB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Record Mod Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_F_LAST_MOD_REC, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_F_LAST_MOD_REC, dlg.m_lean_time);
}

void CNTFSEntry::OnFAccessB() {
  CLeanTime dlg;
  
  dlg.m_title = "Last Access Time";
  dlg.m_divisor = 100000000;
  GetDlgItemText(IDC_F_LAST_ACCESS, dlg.m_lean_time);
  if (dlg.DoModal() == IDOK)
    SetDlgItemText(IDC_F_LAST_ACCESS, dlg.m_lean_time);
}

