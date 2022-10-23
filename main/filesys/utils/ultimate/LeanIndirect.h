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

#if !defined(AFX_LEANINDIRECT_H__121D5E71_F101_405D_98E2_E916BD5C2C27__INCLUDED_)
#define AFX_LEANINDIRECT_H__121D5E71_F101_405D_98E2_E916BD5C2C27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeanIndirect.h : header file
//

// CLeanIndirect dialog

class CLeanIndirect : public CDialog {
	//DECLARE_DYNAMIC(CLeanIndirect)

public:
	CLeanIndirect(CWnd* pParent = NULL);   // standard constructor
	~CLeanIndirect();

  //{{AFX_DATA(CLeanIndirect)
  enum { IDD = IDD_LEAN_INDIRECT };
  CListBox	m_ext_start;
  CListBox	m_ext_size;
  CString	m_magic;
  CString	m_crc;
  CString	m_block_count;
  CString	m_inode;
  CString	m_this_block;
  CString	m_prev_indirect;
  CString	m_next_indirect;
  CString	m_extent_count;
  CString	m_reserved0;
  CString	m_reserved1;
  CString	m_reserved2;
  //}}AFX_DATA

  struct S_LEAN_INDIRECT m_indirect;
  CLean    *m_parent;
  CLeanEntry *m_entry_parent;
  
  DWORD64 m_current_indirect;
  int m_cur_index;
  void *m_indirect_buffer;
  BOOL m_dirty;
  int  m_reserved2_size;
  int  m_extent_max;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CLeanIndirect)
  virtual BOOL OnInitDialog();
  afx_msg void OnSelchangeExtStart();
  afx_msg void OnSelchangeExtSize();
  afx_msg void OnCrcUpdate();
  afx_msg void OnMagicUpdate();
  afx_msg void OnBlockCountUpdate();
  afx_msg void OnThisBlockUpdate();
  afx_msg void OnPrev();
  afx_msg void OnNext();
  afx_msg void OnResv0Update();
  afx_msg void OnResv1Update();
  afx_msg void OnResv2Update();
  afx_msg void OnOkay();
  afx_msg void OnChangeItem();
  //}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  void UpdateStatus(const DWORD64 block);
  void UpdateIndirect(const DWORD64 block);
  void ApplyIndirect(const DWORD64 block);
};




#endif // !defined(AFX_LEANINDIRECT_H__121D5E71_F101_405D_98E2_E916BD5C2C27__INCLUDED_)
