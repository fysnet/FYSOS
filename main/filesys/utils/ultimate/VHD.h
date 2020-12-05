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

#if !defined(AFX_VHD_H__05E1ABFD_D61F_47B2_A6AF_92EF054CAEAD__INCLUDED_)
#define AFX_VHD_H__05E1ABFD_D61F_47B2_A6AF_92EF054CAEAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VHD.h : header file
//

#pragma pack(push, 1)

struct VHD_CHS {
  WORD   cylinder;
  BYTE   heads;
  BYTE   spt;
};

struct VHD_FOOTER {
  BYTE   cookie[8];          // conectix
  DWORD  features;           // 0x00000002
  DWORD  version;            // 0x00010000
  DWORD64 data_offset;       // offset to next structure from start of file (0xFFFFFFFF if fixed disk)
  DWORD  time_stamp;         // num of seconds since 1, 1, 2000, 12:00:00 am in UTC/GMT
  BYTE   creator_ap[4];      // four byte string of creator ("vpc " for vpc 2004)
  DWORD  creator_ver;        // version of creator app (0x00050000 for vpc 2004)
  DWORD  creator_host_os;    // 0x5769326B for Windows or 0x4D616320 for Mac
  DWORD64 original_size;     // size of disk from the perspective of the guest (image size)
  DWORD64 current_size;      // current size of the disk (image size)
  struct VHD_CHS disk_geometry;  // 
  DWORD  disk_type;          // type of disk, 0 = none, 2 = fixed, 3 = dynamic, 4 = differencing
  DWORD  checksum;           // 1's compliment of the sum of all bytes in footer not counting this field
  struct S_GUID uuid;        // UUID
  BYTE   saved_state;        // 1 = in saved state
  BYTE   reserved[427];      // must be zero's
};


#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CVHD dialog

class CVHD : public CPropertyPage
{
  DECLARE_DYNCREATE(CVHD)

// Construction
public:
  CVHD();
  ~CVHD();
  
  bool Exists(DWORD64 LBA);
  void Begin(void);
  
  bool  m_exists;
  DWORD m_color;
  DWORD64 m_lba;
  int   m_draw_index;
  
  BYTE   m_buffer[MAX_SECT_SIZE];
  
// Dialog Data
  //{{AFX_DATA(CVHD)
  enum { IDD = IDD_VHD };
  CString m_check_sum;
  CString m_cookie;
  CString m_creator;
  CString m_creator_host;
  CString m_creator_version;
  CString m_currect_size;
  CString m_data_offset;
  CString m_disk_type;
  CString m_features;
  CString m_guid;
  CString m_org_size;
  CString m_staved_state;
  CString m_time_stamp;
  CString m_time_show;
  CString m_version;
  CString m_dump;
  CString	m_cylinder;
  CString	m_head;
  CString	m_sect_track;
  //}}AFX_DATA


// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CVHD)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CVHD)
  afx_msg void OnVhdApply();
  virtual BOOL OnInitDialog();
  afx_msg void OnKillfocusGuid();
  afx_msg void OnDiskTypeSet();
  afx_msg void OnCRCSet();
  afx_msg void OnGUIDUpdate();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VHD_H__05E1ABFD_D61F_47B2_A6AF_92EF054CAEAD__INCLUDED_)
