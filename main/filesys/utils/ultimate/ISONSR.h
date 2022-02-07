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

#if !defined(AFX_ISONSR_H__B7DC3890_2AF2_4001_AAE9_86D6F9F73735__INCLUDED_)
#define AFX_ISONSR_H__B7DC3890_2AF2_4001_AAE9_86D6F9F73735__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ISONSR.h : header file
//

#pragma pack(push, 1)

#define  CDROM_SECT_SIZE  2048

/*
TAG:ID:
    1 Primary Volume Descriptor (3/10.1)
    2 Anchor Volume Descriptor Pointer (3/10.2)
    3 Volume Descriptor Pointer (3/10.3)
    4 Implementation Use Volume Descriptor (3/10.4)
    5 Partition Descriptor (3/10.5)
    6 Logical Volume Descriptor (3/10.6)
    7 Unallocated Space Descriptor (3/10.8)
    8 Terminating Descriptor (3/10.9 and 4/14.2)
    9 Logical Volume Integrity Descriptor (3/10.10)
  256 File Set Descriptor (4/14.1)
  257 File Identifier Descriptor (4/14.4)
  258 Allocation Extent Descriptor (4/14.5)
  259 Indirect Entry (4/14.7)
  260 Terminal Entry (4/14.8)
  261 File Entry (4/14.9)
  262 Extended Attribute Header Descriptor (4/14.10.1)
  263 Unallocated Space Entry (4/14.11)
  264 Space Bitmap Descriptor (4/14.12)
  265 Partition Integrity Entry (4/14.13)
  266 Extended File Entry (4/14.17)
*/

struct DESC_TAG {
  WORD   id;
  WORD   ver;
  BYTE   crc;
  BYTE   resv;
  WORD   tagsernum;
  WORD   desccrc;
  WORD   desccrclen;
  DWORD  tagloc;
};

struct EXTENT {
  DWORD  length;
  DWORD  location;
};

struct AVDP {
  struct DESC_TAG tag;
  struct EXTENT main_vds;
  struct EXTENT resv_vds;
  BYTE   resv[480];
};

struct CHARSPEC {
  BYTE  CharacterSetType;
  BYTE  CharacterSetInfo[63];
};

struct TIMESTAMP {
  WORD  TypeAndTimezone;
  INT16 Year;
  BYTE  Month;
  BYTE  Day;
  BYTE  Hour;
  BYTE  Minute;
  BYTE  Second;
  BYTE  Centiseconds;
  BYTE  HundredsofMicroseconds;
  BYTE  Microseconds;
};
 
struct DESC_PVD { /* ECMA 167 3/10.1 */
  struct DESC_TAG tag;
  DWORD  VolumeDescriptorSequenceNumber;
  DWORD  PrimaryVolumeDescriptorNumber;
  BYTE   VolumeIdentifier[32];
  WORD   VolumeSequenceNumber;
  WORD   MaximumVolumeSequenceNumber;
  WORD   InterchangeLevel;
  WORD   MaximumInterchangeLevel;
  DWORD  CharacterSetList;
  DWORD  MaximumCharacterSetList;
  BYTE   VolumeSetIdentifier[128];
  struct CHARSPEC DescriptorCharacterSet;
  struct CHARSPEC ExplanatoryCharacterSet;
  struct EXTENT VolumeAbstract;
  struct EXTENT VolumeCopyrightNotice;
  BYTE   ApplicationIdentifier[32];
  struct TIMESTAMP RecordingDateandTime;
  BYTE   ImplementationIdentifier[32];
  BYTE   ImplementationUse[64];
  DWORD  PredecessorVolumeDescriptorSequenceLocation;
  WORD   Flags;
  BYTE   Reserved[22];
};

struct DESC_LVD {
  struct DESC_TAG tag;
  DWORD  seq_num;
  BYTE   char_set[64];
  BYTE   log_id[128];
  DWORD  block_size;
  BYTE   domain_id[32];
  BYTE   content_use[16];
  DWORD  map_table_len;
  DWORD  partition_maps;
  BYTE   implementation_id[32];
  BYTE   implementation_use[128];
  struct EXTENT integrity_seq;
  BYTE   maps[1608];
};

struct LVD_MAP_1 {
  BYTE   type;
  BYTE   len;
  WORD   sequ_num;
  WORD   part_num;
};

struct LVD_MAP_2 {
  BYTE   type;
  BYTE   len;
  BYTE   id[62];
};

struct DESC_PART {
  struct DESC_TAG tag;
  DWORD  sequ_num;
  WORD   flags;
  WORD   number;
  BYTE   contents[32];
  BYTE   content_use[128];
  DWORD  access_type;
  DWORD  start_lba;
  DWORD  sectors;
  BYTE   implement[32];
  BYTE   implement_use[128];
  BYTE   resv[156];
};

struct IDENTIFIER {
  BYTE    flags;
  BYTE    ident[23];
  BYTE    suffix[8];
};

struct REC_ADDR {
  DWORD   block;
  WORD    part_ref;
};

struct LONG_EXTENT {
  DWORD  length;
  struct REC_ADDR location;
  BYTE   implementation_use[6];
};

struct ICB_TAG {
  DWORD prior_rec_num;
  WORD  strategy_type;
  WORD  strategy_param;
  WORD  max_num_entries;
  BYTE  resv;
  BYTE  file_type;
  struct REC_ADDR parent;
  WORD  flags;
};

struct EX_FILE_ENTRY {
  struct DESC_TAG tag;
  struct ICB_TAG icbtag;
  DWORD  uid;
  DWORD  gid;
  DWORD  permissions;
  WORD   link_count;
  BYTE   rec_format;
  BYTE   rec_display_attrbs;
  DWORD  rec_length;
  DWORD64 info_length;
  DWORD64 object_size;
  DWORD64 log_blocks;
  struct TIMESTAMP access_time;
  struct TIMESTAMP modification_time;
  struct TIMESTAMP creation_time;
  struct TIMESTAMP attrib_time;
  DWORD  check_point;
  DWORD  resv;
  struct LONG_EXTENT ext_attrib;
  struct LONG_EXTENT stream_directory;
  struct IDENTIFIER implementation_id;
  DWORD64 unique_id;
  DWORD  ex_attribs_len;  // =L_EA
  DWORD  alloc_descp_len; // =L_AD
  //  L_EA Extended Attribute s bytes
  //  L_AD Allocation descriptors bytes
};

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CISONSR dialog

class CISONSR : public CPropertyPage {
  DECLARE_DYNCREATE(CISONSR)

// Construction
public:
  CISONSR();
  ~CISONSR();

// Dialog Data
  //{{AFX_DATA(CISONSR)
  enum { IDD = IDD_ISO_NSR };
  CString	m_avdp_lba;
  CString	m_avdp_len;
  CString	m_avdp_lba_r;
  CString	m_avdp_len_r;
  CString m_part_access_type;
  CString m_part_flags;
  CString m_part_number;
  CString m_part_sectors;
  CString m_part_sequ_num;
  CString m_part_start_lba;
  CString	m_part_contents;
  CString	m_part_implement;
  CString m_log_block_size;
  CString m_log_int_lba;
  CString m_log_int_len;
  CString m_log_map_table_len;
  CString m_log_part_maps;
  CString m_log_seq_num;
  CString	m_log_log_id;
  CString m_log_domain_id;
  CString m_log_implementation_id;
  //}}AFX_DATA
  
  void Start(const DWORD64 lba, DWORD color, BOOL IsNewTab);
  
  bool    m_is_valid;
  DWORD64 m_lba;   // starting lba of this partition
  DWORD64 m_size;  // size of this partition in sectors
  DWORD   m_color; // color used in image bar
  int     m_draw_index;
  
  BYTE  m_descriptor[MAX_SECT_SIZE];
  BYTE  m_avdp_desc[MAX_SECT_SIZE];
  void *m_descriptors;
  struct DESC_PART *m_part_desc;
  struct DESC_PVD *m_pvd;
  struct DESC_LVD *m_lvd;
  struct LVD_MAP_1 *m_map;
  struct LVD_MAP_2 *m_map2;
  
// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(CISONSR)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CISONSR)
  afx_msg void OnApplyB();
  virtual BOOL OnInitDialog();
  afx_msg void OnAvdpTag();
  afx_msg void OnPartFlagsB();
  afx_msg void OnPartTag();
  afx_msg void OnPartAccessTypeB();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISONSR_H__B7DC3890_2AF2_4001_AAE9_86D6F9F73735__INCLUDED_)
