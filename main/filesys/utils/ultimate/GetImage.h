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

#if !defined(AFX_GETIMAGE_H__42ED13CE_4B80_4726_8FE1_8A5A775FFEE5__INCLUDED_)
#define AFX_GETIMAGE_H__42ED13CE_4B80_4726_8FE1_8A5A775FFEE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GetImage.h : header file
//

#include "winioctl.h"

struct DISK_TYPE {
  DWORD64 total_sects;
  DWORD   cylinders;
  DWORD   sec_per_track;
  DWORD   num_heads;
  DWORD64 size;
};

#define MAX_VOLUMES  16
//  DRIVE_REMOVABLE
//  DRIVE_FIXED
//  DRIVE_REMOTE
//  DRIVE_CDROM
//  DRIVE_RAMDISK
struct VOLUMES {
  TCHAR szDriveName[3]; // = TEXT(" :");
  TCHAR szName[65];
  UINT  iType;
};

/////////////////////////////////////////////////////////////////////////////
// CGetImage dialog

class CGetImage : public CDialog {
// Construction
public:
  CGetImage(CWnd* pParent = NULL);   // standard constructor
  
// Dialog Data
  //{{AFX_DATA(CGetImage)
  enum { IDD = IDD_GET_IMAGE };
  CEdit	m_status;
  CListBox	m_vol_list;
  CProgressCtrl	m_progress;
  CString m_new_name;
  int		m_type;
  //}}AFX_DATA
  
  struct DISK_TYPE *m_disk_info;
  struct VOLUMES *m_volumes;
  int   m_vol_count;
  
  BOOL GetDrvGeometry(DISK_GEOMETRY *pdg, const TCHAR drv);
  BOOL GetDrvGeometryEx(DISK_GEOMETRY_EX *pdg, const TCHAR drv, LARGE_INTEGER *liSize);
  int GetDrives(struct VOLUMES *volumes, const int max);
  void FillDrives(struct VOLUMES *volumes, const int count);
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGetImage)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CGetImage)
  virtual BOOL OnInitDialog();
  afx_msg void OnButtonClicked(UINT nID);
  virtual void OnCancel();
  virtual void OnOK();
  afx_msg void OnKillfocusSectorCount();
  afx_msg void OnSelchangeVolumeList();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETIMAGE_H__42ED13CE_4B80_4726_8FE1_8A5A775FFEE5__INCLUDED_)
