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

#if !defined(AFX_NTFSENTRY_H__AA176824_9457_42A9_8278_2708F2EC7375__INCLUDED_)
#define AFX_NTFSENTRY_H__AA176824_9457_42A9_8278_2708F2EC7375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NTFSEntry.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNTFSEntry dialog

class CNTFSEntry : public CDialog
{
// Construction
public:
  CNTFSEntry(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CNTFSEntry)
  enum { IDD = IDD_NTFS_ENTRY };
  CString m_alloc_size;
  CString m_attribs_off;
  CString m_base_file_ref;
  CString m_fixup_count;
  CString m_fixup_off;
  CString m_flags;
  CString m_hard_link_count;
  CString m_last_op_lsn;
  CString m_magic;
  CString m_next_attrib_id;
  CString m_rec_len;
  CString m_sequ_num;
  CString m_s_comp_flag;
  CString m_s_content_off;
  CString m_s_id;
  CString m_s_len;
  CString m_s_name_len;
  CString m_s_res_flag;
  CString m_s_type;
  CString m_s_class_id;
  CString m_s_creation;
  CString m_s_dos_perms;
  CString m_s_last_access;
  CString m_s_last_mod;
  CString m_s_last_mod_rec;
  CString m_s_max_version;
  CString m_s_owner_id;
  CString m_s_quota;
  CString m_s_security_id;
  CString m_s_usn;
  CString m_s_version;
  CString m_f_comp_flag;
  CString m_f_content_off;
  CString m_f_id;
  CString m_f_len;
  CString m_f_name_len;
  CString m_f_res_flag;
  CString m_f_type;
  CString m_f_alloc_size;
  CString m_f_creation;
  CString m_f_file_size;
  CString m_f_filename;
  CString m_f_filename_len;
  CString m_f_flags;
  CString m_f_last_access;
  CString m_f_last_mod;
  CString m_f_last_mod_rec;
  CString m_f_ref;
  CString m_f_sequ;
  CString m_d_comp_flag;
  CString m_d_content_off;
  CString m_d_id;
  CString m_d_len;
  CString m_d_name_len;
  CString m_d_res_flag;
  CString m_d_type;
  CString m_d_alloc_size;
  CString m_d_data_len;
  CString m_d_data_off;
  CString m_d_engine_num;
  CString m_d_index_flag;
  CString m_d_initial_size;
  CString m_d_last_vcn;
  CString m_d_padding;
  CString m_d_real_size;
  CString m_d_run_list;
  CString m_d_start_vcn;
  CString m_d_unused;
  //}}AFX_DATA
  
  struct S_NTFS_FILE_REC *m_file_rec;
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CNTFSEntry)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CNTFSEntry)
  afx_msg void OnFlags();
  virtual BOOL OnInitDialog();
  afx_msg void OnDRunListB();
  afx_msg void OnSCreationB();
  afx_msg void OnSLastModB();
  afx_msg void OnSLastModRecB();
  afx_msg void OnSAccessB();
  afx_msg void OnFCreationB();
  afx_msg void OnFLastModB();
  afx_msg void OnFLastModRecB();
  afx_msg void OnFAccessB();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NTFSENTRY_H__AA176824_9457_42A9_8278_2708F2EC7375__INCLUDED_)
