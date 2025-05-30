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

#if !defined(AFX_ATTRIBUTE_H__5D0433D7_6136_4937_9034_53E1240E8037__INCLUDED_)
#define AFX_ATTRIBUTE_H__5D0433D7_6136_4937_9034_53E1240E8037__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Attribute.h : header file
//

struct S_ATTRIBUTES {
  DWORD64 attrb;
  DWORD64 mask;
  int   index;
  char  str[32];
  int   groups[10];  // up to 10 items can be grouped.  Increase if we need more.
};

/////////////////////////////////////////////////////////////////////////////
// CAttribute dialog

class CAttribute : public CDialog {
// Construction
public:
  CAttribute(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CAttribute)
  enum { IDD = IDD_ATTRIBUTE };
  CListBox	m_list;
  //}}AFX_DATA
  
  CString m_title;
  BOOL    m_single;
  DWORD64 m_attrib;
  struct S_ATTRIBUTES *m_attributes;
  
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAttribute)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
// Implementation
protected:
  
  // Generated message map functions
  //{{AFX_MSG(CAttribute)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnSelchangeList();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTRIBUTE_H__5D0433D7_6136_4937_9034_53E1240E8037__INCLUDED_)
