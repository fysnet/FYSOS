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

// ultimate.h : main header file for the ULTIMATE application
//

#if !defined(AFX_ULTIMATE_H__54500165_D296_435D_B65D_3F50E4759072__INCLUDED_)
#define AFX_ULTIMATE_H__54500165_D296_435D_B65D_3F50E4759072__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include <math.h>
#define LOG2(x) (int) (log((double) (x)) / log(2.0))  // log2(x) = log(x) / log(2.0)

#define MAX_SECT_SIZE   4096

// different FSs we support
#define FS_LEAN     1
#define FS_EXT2     2
#define FS_SFS      3
#define FS_NTFS     4
#define FS_EXFAT   11
#define FS_FAT12   12
#define FS_FAT16   16
#define FS_FYSFS   22
#define FS_FAT32   32

#define ENDIAN_16U(x)   ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_32U(x)   ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8) | (((x) & 0xFF000000) >> 24))
#define ENDIAN_64U(x)   (                                              \
                         ((DWORD64) ((x) & 0x00000000000000FF) << 56) | \
                         ((DWORD64) ((x) & 0x000000000000FF00) << 40) | \
                         ((DWORD64) ((x) & 0x0000000000FF0000) << 24) | \
                         ((DWORD64) ((x) & 0x00000000FF000000) <<  8) | \
                         ((DWORD64) ((x) & 0x000000FF00000000) >>  8) | \
                         ((DWORD64) ((x) & 0x0000FF0000000000) >> 24) | \
                         ((DWORD64) ((x) & 0x00FF000000000000) >> 40) | \
                         ((DWORD64) ((x) & 0xFF00000000000000) >> 56)   \
                        )

/////////////////////////////////////////////////////////////////////////////
// CUltimateApp:
// See ultimate.cpp for the implementation of this class
//

class CUltimateApp : public CWinApp {
public:
  CUltimateApp();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CUltimateApp)
  public:
  virtual BOOL InitInstance();
  //}}AFX_VIRTUAL
  
// Implementation
  
  //{{AFX_MSG(CUltimateApp)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

void DumpIt(CString &csTarg, const void *buffer, DWORD start_addr, unsigned size, BOOL Append);
int ConvertDumpToBuffer(CString csSrc, void *buffer, int max);
BOOL IsBufferEmpty(const void *buffer, const int count);

/////////////////////////////////////////////////////////////////////////////

DWORD64 convert64(CString &csSrc);
DWORD32 convert32(CString &csSrc);
WORD convert16(CString &csSrc);
BYTE convert8(CString &csSrc);

/////////////////////////////////////////////////////////////////////////////

#define CRC32_POLYNOMIAL 0x04C11DB7

void crc32_initialize(void);
DWORD crc32_reflect(DWORD reflect, char ch);
DWORD crc32(void *data, DWORD len);
void crc32_partial(DWORD *crc, void *ptr, DWORD len);

/////////////////////////////////////////////////////////////////////////////

BOOL BrowseForFolder(HWND hwnd, CString &csCurrent, LPTSTR szPath, UINT ulFlags);

/////////////////////////////////////////////////////////////////////////////

// ex: 3F2504E0-4F89-11D3-9A0C-0305E82C3301
//       data1    2    3    4     5[6]
struct S_GUID {
  DWORD  data1;
  WORD   data2;
  WORD   data3;
  WORD   data4;
  BYTE   data5[6];
};

void GUID_Format(CString &csguid, struct S_GUID *guid);
void GUID_Retrieve(CString &csguid, struct S_GUID *guid);
BOOL GUID_CheckFormat(CString csguid);

#define GUID_TYPE_MT         0  // Unused Entry 00000000-0000-0000-0000-000000000000
#define GUID_TYPE_EFI        1  // EFI System Partition C12A7328-F81F-11D2-BA4B-00A0C93EC93B
#define GUID_TYPE_LEGACY     2  // Partition containing a legacy MBR 024DEE41-33E7-11D3-9D69-0008C781F39F

#define GUID_TYPE_RANDOM   256  // create a random GUID

void GUID_Create(struct  S_GUID *guid, const DWORD type);

/////////////////////////////////////////////////////////////////////////////

#include "MyTreeCtrl.h"

void ExpandIt(CMyTreeCtrl *dir_tree, BOOL Expand);

/////////////////////////////////////////////////////////////////////////////

float GetUTCOffset(void);

/////////////////////////////////////////////////////////////////////////////

#define FNM_NOCASE      0x01
BOOL fnmatch(const char *wild_text, const char *tame_text, const DWORD flags);

/////////////////////////////////////////////////////////////////////////////
void gLBAtoCHS(const DWORD lba, WORD *cyl, BYTE *head, BYTE *sect, const DWORD spt, const DWORD heads);

/////////////////////////////////////////////////////////////////////////////
BOOL power_of_two(DWORD val);

/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ULTIMATE_H__54500165_D296_435D_B65D_3F50E4759072__INCLUDED_)
