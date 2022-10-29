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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING INtrying  ANY WAY  OUT OF THE USE OF THIS
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

// ultimateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"
#include "UltimageResize.h"

#include "afxadv.h"

#include "FYSOSSig.h"
#include "Settings.h"
#include "VDI.h"

#include "NewImage.h"
#include "GetImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog {
public:
  CAboutDlg();
  
// Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD) {
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CAboutDlg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUltimateDlg dialog

CUltimateDlg::CUltimateDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CUltimateDlg::IDD, pParent) {
  //{{AFX_DATA_INIT(CUltimateDlg)
  m_force_gpt_presence = FALSE;
  m_force_mbr_presence = FALSE;
  m_force_gpt_enum = TRUE;
  m_ignore_empty_gpt_entries = TRUE;
  m_force_readonly = TRUE;
  m_sect_size = 512;
  m_sect_size_option = 0; // default to 512-byte sectors
  m_force_no_mbr_presence = FALSE;
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_file_type = DLG_FILE_TYPE_FLAT;
  m_dflt_sect_size = 512;
  m_isISOImage = FALSE;
  m_hasVHD = FALSE;
  m_MBRCount = 0;
  m_FatCount = 0;
  m_Ext2Count = 0;
  m_ExFatCount = 0;
  m_LeanCount = 0;
  m_NTFSCount = 0;
  m_SFSCount = 0;
  m_FYSCount = 0;
  m_UCount = 0;
  m_overwrite_okay = FALSE;
  m_MaxErrorCount = 0;

  m_vdi_blocks = NULL;
}

void CUltimateDlg::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CUltimateDlg)
  DDX_Control(pDX, IDC_IMAGE_BAR, m_image_bar);
  DDX_Control(pDX, IDC_TABS, m_StaticTabs);
  DDX_Check(pDX, IDC_FORCE_GPT, m_force_gpt_presence);
  DDX_Check(pDX, IDC_FORCE_MBR, m_force_mbr_presence);
  DDX_Check(pDX, IDC_FORCE_NO_MBR, m_force_no_mbr_presence);
  DDX_Check(pDX, IDC_FORCE_GPT_ENUM, m_force_gpt_enum);
  DDX_Check(pDX, IDC_IGNORE_MT_GPT_ENTRIES, m_ignore_empty_gpt_entries);
  DDX_Check(pDX, IDC_FORCE_READONLY, m_force_readonly);
  DDX_Radio(pDX, IDC_SECT_SIZE_512, m_sect_size_option);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUltimateDlg, CDialog)
  //{{AFX_MSG_MAP(CUltimateDlg)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_COMMAND(ID_FILE_NEW, OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
  ON_COMMAND(ID_TOOLS_RESIZE, OnToolsResize)
  ON_COMMAND(ID_TOOLS_ERASE_IMAGE, OnToolsErase)
  ON_COMMAND(ID_GET_DISK_IMAGE, OnToolsGetDisk)
  ON_COMMAND(ID_APPEND_VHD, OnToolsAppendVHD)
  ON_COMMAND(ID_TOOLS_ADDHYBRIDCDROM, OnToolsHybridCDROM)
  ON_COMMAND(ID_TOOLS_VIEWVDIHEADER, OnViewVDIHeader)
  ON_COMMAND(ID_FILE_RELOAD, OnFileReload)
  ON_COMMAND(ID_FILE_EXIT, OnFileExit)
  ON_COMMAND(ID_HELP_INDEX, OnHelpHelp)
  ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
  ON_COMMAND(ID_TOOLS_CREATE_INFO, OnCreateInfoFile)
  ON_BN_CLICKED(IDC_SECT_SIZE_512, OnChangeSectSize)
  ON_BN_CLICKED(IDC_SECT_SIZE_1024, OnChangeSectSize)
  ON_BN_CLICKED(IDC_SECT_SIZE_2048, OnChangeSectSize)
  ON_BN_CLICKED(IDC_SECT_SIZE_4096, OnChangeSectSize)
  ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnRecentFileMenu)
  ON_WM_INITMENUPOPUP()
  ON_COMMAND(ID_SETTINGS, OnAppSettings)
  ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateFileMRUFile1)
  ON_WM_LBUTTONUP()
  ON_BN_CLICKED(IDC_FORCE_MBR, OnForceMbr)
  ON_BN_CLICKED(IDC_FORCE_NO_MBR, OnForceNoMbr)
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MRL_MAX  16  // this can't be more than ID_FILE_MRU_FILE16/ID_FILE_MRU_LAST

// our sector sizes.  Index starts at 0.  If we add any more, must remain in accending order
unsigned int g_sector_sizes[4] = { 512, 1024, 2048, 4096 };

/////////////////////////////////////////////////////////////////////////////
// CUltimateDlg message handlers
BOOL CUltimateDlg::OnInitDialog() {
  
  m_force_readonly = AfxGetApp()->GetProfileInt("Settings", "ForceReadOnly", -1);
  if (m_force_readonly == -1) {
    AfxMessageBox("ForceReadOnly is set by default.\r\n"
                  "Use the Settings Menu option to change.");
    m_force_readonly = TRUE;
  }
  
  // Sector Size
  m_sect_size_option = AfxGetApp()->GetProfileInt("Settings", "DefaultSectSize", 0);
  m_sect_size = m_dflt_sect_size = g_sector_sizes[m_sect_size_option];
  
  m_TabControl.EnableStackedTabs(FALSE);
  
  CDialog::OnInitDialog();
  
  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);     // Set big icon
  SetIcon(m_hIcon, FALSE);    // Set small icon
  
  m_bIsOpen = FALSE;
  
  m_rfl = new CRecentFileList(0, _T("Recent Files"), _T("File%d"), MRL_MAX, AFX_ABBREV_FILENAME_LEN);
  m_rfl->ReadList();
  
  // Create a fixed font for "DUMP" windows
  m_DumpFont.Detach();
  m_DumpFont.CreateFont(14,            // nHeight
            0,                         // nWidth
            0,                         // nEscapement
            0,                         // nOrientation
            FW_NORMAL,                 // nWeight
            FALSE,                     // bItalic
            FALSE,                     // bUnderline
            0,                         // cStrikeOut
            ANSI_CHARSET,              // nCharSet
            OUT_DEFAULT_PRECIS,        // nOutPrecision
            CLIP_DEFAULT_PRECIS,       // nClipPrecision
            DEFAULT_QUALITY,           // nQuality
            DEFAULT_PITCH,             // nPitchAndFamily
            _T("Courier New"));        // lpszFacename
  
  return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CUltimateDlg::OnHelpInfo(HELPINFO *pHelpInfo) {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "main.html",
    NULL, NULL, SW_SHOWNORMAL);
  return TRUE;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CUltimateDlg::OnPaint() {
  if (IsIconic()) {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  } else {
    CDialog::OnPaint();
  }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUltimateDlg::OnQueryDragIcon() {
  return (HCURSOR) m_hIcon;
}

void CUltimateDlg::OnFileNew() {
  CNewImage NewImage;
  
  // see if we already have a file open
  if (m_bIsOpen) {
    
    // do the save/close of existing

    vdi_close_file();
    m_file.Close();
    m_ImageBar.Clear();
    m_bIsOpen = FALSE;
  }
  
  CFileDialog odlg (
    FALSE,            // Create a save_as file dialog
    _T(".img"),       // Default file extension
    NULL,             // Default Filename
    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T(".vdi files (.vdi)|*.vdi|")    // Filter string
    _T(".vhd files (.vhd)|*.vhd|")    // Filter string
    _T(".iso files (.iso)|*.iso|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  odlg.m_ofn.lpstrTitle = "Create New Image File";
  if (odlg.DoModal() != IDOK)
    return;
  
  POSITION pos = odlg.GetStartPosition();
  NewImage.m_new_name = odlg.GetNextPathName(pos);
  if (NewImage.DoModal() == IDOK)
    if (AfxMessageBox("Open newly created image?", MB_YESNO, 0) == IDYES) {
      // we need to set the sector size to the size used in the NewImage dlg.
      m_sect_size_option = NewImage.m_sect_size;
      m_sect_size = m_dflt_sect_size = g_sector_sizes[m_sect_size_option];
      AfxGetApp()->WriteProfileInt("Settings", "DefaultSectSize", m_sect_size_option);
      UpdateData(FALSE);  // send to Dialog
      // then open the file
      FileOpen(NewImage.m_new_name);
    }
}

// detects what type file we are using:
//   Flat = sector for sector
//   Growing = 
//    etc.
//
// VMDK images:
//   https://en.wikipedia.org/wiki/VMDK
//   https://www.vmware.com/support/developer/vddk/vmdk_50_technote.pdf
//
//  VDI images:
//   https://forums.virtualbox.org/viewtopic.php?t=8046
//
// 
int CUltimateDlg::DetFileType(void) {
  BYTE buffer[MAX_SECT_SIZE];
  int type = DLG_FILE_TYPE_FLAT;
  struct VDI_HEADER *vdi_header = (struct VDI_HEADER *) buffer;
  
  // set to sect for sect so we can read from the file
  // also, (temporaritly) set the length so the check won't fail
  m_file_length.QuadPart = MAX_SECT_SIZE;  // need to be at least MAX_SECT_SIZE incase we are using MAX_SECT_SIZE sized sectors
  m_file_type = DLG_FILE_TYPE_FLAT;
  ReadFromFile(buffer, 0, 1);
  
  // try a Bochs Virtual disk image
  if (memcmp(&buffer[0], "Bochs Virtual HD Image\0\0\0\0\0\0\0\0\0\0", 32) == 0) {
    // found bochs virtual file
    if (memcmp(&buffer[32], "Redolog\0\0\0\0\0\0\0\0\0", 16) == 0) {
      // found redolog file
      if (memcmp(&buffer[48], "Undoable\0\0\0\0\0\0\0\0", 16) == 0)
        type = DLG_FILE_TYPE_UNDOABLE;
      else if (memcmp(&buffer[48], "Volatile\0\0\0\0\0\0\0\0", 16) == 0)
        type = DLG_FILE_TYPE_VOLATILE;
      else if (memcmp(&buffer[48], "Growing\0\0\0\0\0\0\0\0\0", 16) == 0)
        type = DLG_FILE_TYPE_UNKWN;
      else {
        // unknown Redolog type file
        AfxMessageBox("Found Bochs Virtual HD Image: Redolog: \"Unknown\" file...");
        return DLG_FILE_TYPE_UNKWN;
      }
      // type = Redolog : subtype
      
      // http://bochs.sourceforge.net/doc/docbook/development/harddisk-redologs.html
      
      /*
      DWORD cat_count = * (DWORD *) &buffer[72];
      DWORD bitmap_size = * (DWORD *) &buffer[76];
      DWORD ext_size = * (DWORD *) &buffer[80];
      DWORD64 disk_size = * (DWORD64 *) &buffer[88];
      
      cs.Format("%i %i %i %I64i", cat_count, bitmap_size, ext_size, disk_size);
      AfxMessageBox(cs);
      */
      
      AfxMessageBox("Found Bochs Virtual HD Image: Redolog: [Undoable|Volatile|Growing] file...\r\n"
                    "I currently don't support this type of file...");
      
      return DLG_FILE_TYPE_UNKWN; ///////////////////
    } else {
      // unknown Bochs V HD Image type file
      AfxMessageBox("Found Bochs Virtual HD Image: \"Unknown\" file...");
      return DLG_FILE_TYPE_UNKWN;
    }
    
  // try a VirtualBox Virtual disk image
  } else if ((memcmp(vdi_header, "<<<", 3) == 0) && 
             (vdi_header->signature == 0xBEDA107F) &&
             (vdi_header->version == 0x00010001)) {
    return vdi_open_file(vdi_header);
  }
  
  // default to sector for sector (flat)
  return DLG_FILE_TYPE_FLAT;
}

// found a VirtualBox VDI image.
int CUltimateDlg::vdi_open_file(struct VDI_HEADER *vdi_header) {
  LARGE_INTEGER large_int;
  
  // change to the sector size given
  if (vdi_header->sector_size == 1024) {
    m_sect_size = m_dflt_sect_size = 1024;
    CheckRadioButton(IDC_SECT_SIZE_512, IDC_SECT_SIZE_4096, IDC_SECT_SIZE_1024); // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
  } else if (vdi_header->sector_size == 2048) {
    m_sect_size = m_dflt_sect_size = 2048;
    CheckRadioButton(IDC_SECT_SIZE_512, IDC_SECT_SIZE_4096, IDC_SECT_SIZE_2048); // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
  } else if (vdi_header->sector_size == 4096) {
    m_sect_size = m_dflt_sect_size = 4096;
    CheckRadioButton(IDC_SECT_SIZE_512, IDC_SECT_SIZE_4096, IDC_SECT_SIZE_4096); // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
  } else {
    m_sect_size = m_dflt_sect_size = 512;
    CheckRadioButton(IDC_SECT_SIZE_512, IDC_SECT_SIZE_4096, IDC_SECT_SIZE_512); // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
  }

  // we need to set a few items
  m_vdi_image_type = vdi_header->image_type;        // either flat or dynamic
  m_vdi_image_flags = vdi_header->image_flags;      // flags
  m_vdi_offset_blocks = vdi_header->offset_blocks;  // byte offset in file where the block table starts
  m_vdi_offset_data = (DWORD64) vdi_header->offset_data; // byte offset in file where the data blocks start
  m_vdi_disk_size = vdi_header->disk_size;          // size of image file in bytes (if all blocks were used)
  m_vdi_block_size = vdi_header->block_size;        // block size (should be 1Meg blocks)
  m_vdi_block_count = vdi_header->block_count;      // count of blocks used
  m_vdi_blocks_allocated = vdi_header->blocks_allocated;  // current allocated blocks
  m_vdi_table_dirty = FALSE;
      
  // allocate and read in the block table
  m_vdi_blocks = (DWORD *) calloc(m_vdi_block_count * sizeof(DWORD), 1);

  large_int.QuadPart = m_vdi_offset_blocks;
  ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
  m_file.Read(m_vdi_blocks, m_vdi_block_count * sizeof(DWORD));
    
  return DLG_FILE_TYPE_VB_VDI;
}

// we need to close the VDI file
void CUltimateDlg::vdi_close_file() {
  LARGE_INTEGER large_int;
  BYTE buffer[MAX_SECT_SIZE];
  struct VDI_HEADER *vdi_header = (struct VDI_HEADER *) buffer;
  
  if (m_file_type == DLG_FILE_TYPE_VB_VDI) {
    // if we changed the block table, we need to write it back
    if (m_vdi_table_dirty) {
      large_int.QuadPart = m_vdi_offset_blocks;
      ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      m_file.Write(m_vdi_blocks, m_vdi_block_count * sizeof(DWORD));
      m_vdi_table_dirty = FALSE;
      
      // now update the allocated count, etc.
      large_int.QuadPart = 0;
      ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      m_file.Read(buffer, 512);
      vdi_header->blocks_allocated = m_vdi_blocks_allocated;
      ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      m_file.Write(buffer, 512);
    }
    
    if (m_vdi_blocks)
      free(m_vdi_blocks);

    m_vdi_blocks = NULL;
  }
}

void CUltimateDlg::FileOpen(CString csPath) {
  BYTE buffer[MAX_SECT_SIZE];
  CString cs;
  
  // see if we already have a file open
  if (m_bIsOpen) {
    
    // do the save/close of existing
    
    vdi_close_file();
    m_file.Close();
    m_ImageBar.Clear();
    m_bIsOpen = FALSE;
  }
  
  m_sect_size = m_dflt_sect_size;
  m_isISOImage = FALSE;
  m_hasVHD = FALSE;
  m_MBRCount = 0;
  m_FatCount = 0;
  m_LeanCount = 0;
  m_NTFSCount = 0;
  m_SFSCount = 0;
  m_FYSCount = 0;
  m_Ext2Count = 0;
  m_ExFatCount = 0;
  m_UCount = 0;
  
  if (m_file.Open(csPath, CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    if (AfxMessageBox("Error Opening Image File...\r\nDo you want to remove it from the MRU list?", MB_YESNO, 0) == IDYES)
      RemoveFromMRUList(csPath);
    return;
  }
  
  // detect file type (flat, growing, etc.)
  m_file_type = DetFileType();
  if (m_file_type == DLG_FILE_TYPE_UNKWN) {
    AfxMessageBox("Found unknown type of image file...");
    m_file.Close();
    return;
  }
  
  // get and save the length of the image file
  m_file_length = GetFileLength((HANDLE) m_file.m_hFile);
  
  // see if there is a companion ".info" file for this image
  // for example, if the image file is named "new_image.img" the
  //   info file will be named "new_image.img.info".
  // this info file will hold a few items for this image, one being
  //   if this file is default 4k sectors or not.
  if (!FileOpenInfo(csPath)) {
    m_file.Close();
    vdi_close_file();
    return;
  }

  // check to see if it is an ISO image
  m_sect_size = m_dflt_sect_size;
  // Need to use the ReadFromFile() so we can read from physical device (eventually!)
  ReadFromFile(buffer, 16, 1);
  if ((memcmp(buffer + 1, "CD001", 5) == 0) ||
      (memcmp(buffer + 1, "BEA01", 5) == 0)) {
    m_isISOImage = TRUE;
  } else if (m_sect_size != 2048) {
    // if the sector size isn't 2048, try again with that sector size
    m_sect_size = 2048;  // temporarily set the sector size to 2k
    ReadFromFile(buffer, 16, 1);
    m_sect_size = m_dflt_sect_size;  // restore the sector size
    if ((memcmp(buffer + 1, "CD001", 5) == 0) ||
        (memcmp(buffer + 1, "BEA01", 5) == 0)) {
      AfxMessageBox("Found CDROM but Sector Size is not 2048!\r\n"
                    " Change Sector Size to 2048 and try again");
    }
  }
  
  // now don't allow the user to change the sector size and a few other things
  GetDlgItem(IDC_FORCE_MBR)->EnableWindow(FALSE);
  GetDlgItem(IDC_FORCE_NO_MBR)->EnableWindow(FALSE);
  GetDlgItem(IDC_FORCE_GPT)->EnableWindow(FALSE);
  GetDlgItem(IDC_FORCE_GPT_ENUM)->EnableWindow(FALSE);
  GetDlgItem(IDC_SECT_SIZE_512)->EnableWindow(FALSE);
  GetDlgItem(IDC_SECT_SIZE_1024)->EnableWindow(FALSE);
  GetDlgItem(IDC_SECT_SIZE_2048)->EnableWindow(FALSE);
  GetDlgItem(IDC_SECT_SIZE_4096)->EnableWindow(FALSE);

  m_image_bar.m_item_count = 0;
  m_image_bar.m_TabControl = &m_TabControl;
  m_ImageBar.ImageParse(&m_file);
  
  m_bIsOpen = TRUE;
  m_cur_file = csPath;
  
  AddToMRUList(csPath);
  
  cs.Format("Ultimate Imager -- %s", csPath);
  SetWindowText(cs);
}

int CUltimateDlg::GetString(char *buffer, CString &str) {
  int p = 0, c = 256;  // don't go more than 256 chars (just a safety catch)
  char ch = 0;
  
  str.Empty();
  
  // skip all leading spaces
  while (buffer[p]) {
    ch = buffer[p++];
    if (ch != ' ')
      break;
  }
  
  while (ch) {
    if (ch != 13) {  // this skips all CRs
      if (strchr("\xA,=#", ch))
        break;
      str += ch;
      if (--c == 0)
        break;
    }
    ch = buffer[p++];
  }
  
  // if it was the '#' char, skip to eol
  if (ch == '#') {
    while (buffer[p]) {
      if (buffer[p++] == 10)
        break;
    }
  }
  
  // kill all trailing spaces
  str.TrimRight();

  return p;
}

// if there is a text file with the same name + ".info", parse the file
//  to get some parameters
// ( https://www.fysnet.net/ultimate/help/infofile.html )
BOOL CUltimateDlg::FileOpenInfo(CString csPath) {
  CFile file;
  char *buffer, *pos;
  CString str, cs;
  UINT length;
  int len, i;
  BOOL ret = TRUE;

  if (file.Open(csPath + ".info", CFile::modeRead | CFile::typeText | CFile::shareDenyWrite, NULL) == 0)
    return TRUE;

  // don't allow the file to be larger than 64k in size (just in case...)
  //  64k is just a number I picked.  No specific reason for this size...
  length = (UINT) file.GetLength();
  if (length > 65535)  // 64k - 1
    length = 65535;
  
  // allocate the buffer, read the file, close the file
  buffer = (char *) malloc(length + 1);
  file.Read(buffer, length);
  file.Close();
  
  // make sure it is null terminated
  buffer[length] = '\0';
  
  // parse the file
  pos = buffer;
  while (*pos) {
    // get a parameter
    if ((len = GetString(pos, str)) == 0)
      break;
    pos += len;
    
    len = 0;
    // sector-size=  field
    if (str == "sector-size") {
      len = GetString(pos, str);
      i = -1;
      if (str == "512")
        i = 0;
      else if (str == "1024")
        i = 1;
      else if (str == "2048")
        i = 2;
      else if (str == "4096")
        i = 3;
      else {
        cs.Format("Found illegal value for 'sector-size=' parameter in .info file: %s", str);
        AfxMessageBox(cs);
        ret = FALSE;
      }
      if (i > -1) {
        m_sect_size_option = i;
        m_sect_size = m_dflt_sect_size = g_sector_sizes[m_sect_size_option];
        CheckRadioButton(IDC_SECT_SIZE_512, IDC_SECT_SIZE_4096, IDC_SECT_SIZE_512 + m_sect_size_option); // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
      }
    }
    // read-only= field
    else if (str == "read-only") {
      len = GetString(pos, str);
      if (str == "1") {
        m_force_readonly = TRUE;
        CheckDlgButton(IDC_FORCE_READONLY, TRUE);  // can't call UpdateData() because it will call the ON_BN_CLICKED handler...
      } else {
        cs.Format("Found illegal value for 'read-only=' parameter in .info file: %s", str);
        AfxMessageBox(cs);
        ret = FALSE;
      }
    }

    // move to the next position to start parsing parameters
    pos += len;
  }

  free(buffer);

  return ret;
}

// create a new .info file for this image file.
//  *** will overwrite any existing .info file ***
void CUltimateDlg::OnCreateInfoFile() {
  CFile file;
  CString cs, csText;

  if (AfxMessageBox("This will create an .info settings file for this image file.\r\n"
                    " If the .info file already exists, it will be truncated!\r\n"
                    " Proceed?", MB_YESNO, 0) != IDYES) {
    return;
  }

  csText  = "# This is the .info Companion File for the image: ";
  csText += m_cur_file;
  csText += "\r\n\n";

  csText += "# First parameter we will specify is the sector size parameter.\r\n";
  cs.Format("sector-size = %i  # may use 512, 1024, 2048, or 4096 only\r\n", m_sect_size);
  csText += cs;
  csText += "\r\n";
  
  csText += "# Set/Clear the Readonly flag.\r\n";
  if (IsDlgButtonChecked(IDC_FORCE_READONLY))
    csText += "read-only = 1      # 1 = set = read only  (anything else is an error)\r\n";
  else
    csText += "# read-only = 0      # 1 = set = read only  (anything else is an error)\r\n";
  csText += "\r\n";

  // now write the text to the file
  if (file.Open(m_cur_file + ".info", CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive, NULL) != 0) {
    file.Write((LPCTSTR) csText, csText.GetLength());
    file.Close();
  }
}

void CUltimateDlg::OnFileOpen() {
  CFileDialog dlg (
    TRUE,             // Create an open file dialog
    _T(".img"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T(".vdi files (.vdi)|*.vdi|")    // Filter string
    _T(".vhd files (.vhd)|*.vhd|")    // Filter string
    _T(".iso files (.iso)|*.iso|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  if (dlg.DoModal() != IDOK)
    return;
  
  POSITION pos = dlg.GetStartPosition();
  FileOpen(dlg.GetNextPathName(pos));
}

void CUltimateDlg::OnFileClose() {
  // first see if we already have a file open (should have
  if (m_bIsOpen) {
    vdi_close_file();
    
    m_file.Close();
    m_ImageBar.Clear();
    m_bIsOpen = FALSE;

    // reset the sector size field
    m_sect_size_option = AfxGetApp()->GetProfileInt("Settings", "DefaultSectSize", 0);
    m_sect_size = m_dflt_sect_size = g_sector_sizes[m_sect_size_option];
    UpdateData(FALSE);  // send to Dialog

    // allow the user to change the sector size
    GetDlgItem(IDC_FORCE_MBR)->EnableWindow(TRUE);
    GetDlgItem(IDC_FORCE_NO_MBR)->EnableWindow(TRUE);
    GetDlgItem(IDC_FORCE_GPT)->EnableWindow(TRUE);
    GetDlgItem(IDC_FORCE_GPT_ENUM)->EnableWindow(TRUE);
    GetDlgItem(IDC_SECT_SIZE_512)->EnableWindow(TRUE);
    GetDlgItem(IDC_SECT_SIZE_1024)->EnableWindow(TRUE);
    GetDlgItem(IDC_SECT_SIZE_2048)->EnableWindow(TRUE);
    GetDlgItem(IDC_SECT_SIZE_4096)->EnableWindow(TRUE);
    
    m_isISOImage = FALSE;
    m_hasVHD = FALSE;
    m_MBRCount = 0;
    m_FatCount = 0;
    m_LeanCount = 0;
    m_NTFSCount = 0;
    m_SFSCount = 0;
    m_FYSCount = 0;
    m_ExFatCount = 0;
    m_UCount = 0;
  }
  
  SetWindowText("Ultimate Imager");
}

void CUltimateDlg::OnFileReload() {
  if (m_bIsOpen)
    FileOpen(m_cur_file);
}

void CUltimateDlg::OnFileExit() {
  // first see if we already have a file open
  if (m_bIsOpen) {
    
    // do the save/close of existing
    
    vdi_close_file();
    m_file.Close();
    m_ImageBar.Clear();
    m_bIsOpen = FALSE;
  }
  
  m_rfl->WriteList();
  delete[] m_rfl;
  
  EndDialog(ID_FILE_EXIT);  
}

void CUltimateDlg::OnToolsResize() {
  CUltimageResize resize;
  DWORD64 cur_size = (m_file_length.QuadPart + (m_sect_size - 1)) / m_sect_size;
  DWORD64 new_size;
  BOOL ret = FALSE;
  CString cs;
  
  resize.m_cur_size.Format("%I64i", cur_size);
  resize.m_new_size.Format("%I64i", cur_size);
  if (resize.DoModal() == IDOK) {
    new_size = convert64(resize.m_new_size);
    // if they are the same, then just return
    if (new_size == cur_size)
      return;
    
    // ask first
    cs.Format("Resize the file from %I64i sectors to %I64i sectors?", cur_size, new_size);
    if (AfxMessageBox(cs, MB_YESNO, 0) != IDYES)
      return;
    
    // if new size is greater, simply add "sectors" to file
    // if new size is less than, ask first, then if okay, change size of file.
    if (new_size < cur_size)
      if (AfxMessageBox("You are truncating some of the file.  Are you sure?", MB_YESNO, 0) != IDYES)
        return;
    
    // TODO: Fix me:
    // if the image has a VHD at the end, we need to move it, not delete it.
    if (AfxMessageBox("If this image has a VHD Footer, it will be deleted.\n"
                      "This is a known issue and is on my TODO list.  Continue?", MB_YESNO, 0) != IDYES)
      return;
    
    // see if we can change the size
    if (SetFileLength((HANDLE) m_file.m_hFile, new_size * m_sect_size))
      // reload the file
      SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
    else
      AfxMessageBox("Error modifying size of file");
  }
}

void CUltimateDlg::OnToolsGetDisk() {
  CGetImage GetImage;
  
  CFileDialog odlg (
    FALSE,            // Create a save_as file dialog
    _T(".img"),       // Default file extension
    NULL,             // Default Filename
    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".img files (.img)|*.img|")    // Filter string
    _T(".bin files (.bin)|*.bin|")    // Filter string
    _T(".vhd files (.vhd)|*.vhd|")    // Filter string
    _T(".iso files (.iso)|*.iso|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  odlg.m_ofn.lpstrTitle = "Create New Image File";
  if (odlg.DoModal() != IDOK)
    return;
  
  POSITION pos = odlg.GetStartPosition();
  GetImage.m_new_name = odlg.GetNextPathName(pos);
  if (GetImage.DoModal() == IDOK)
    if (AfxMessageBox("Open image?", MB_YESNO, 0) == IDYES)
      FileOpen(GetImage.m_new_name);
}

void CUltimateDlg::OnToolsAppendVHD() {
  LARGE_INTEGER large_int, large_size;
  
  if (AfxMessageBox("Append a VHD footer to this image file?", MB_YESNO, 0) != IDYES)
    return;

  // seconds between 1/1/2000 and 1/1/1970  (365.2425 * 24 * 60 * 60) * 30
  CTime cTime = CTime::GetCurrentTime();
  const DWORD seconds = (DWORD) (cTime.GetTime() - (time_t) 946708560);

  // move to the end of the file
  large_int.QuadPart = 0;
  if (::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, &large_size, FILE_END) == 0) {
    AfxMessageBox("Error moving to end of file.");
    return;
  }

  const WORD cyl = (WORD) ((large_size.QuadPart / m_sect_size) / 63 / 16);
  BYTE *buffer = (BYTE *) calloc(m_sect_size, 1);
  struct VHD_FOOTER *footer = (struct VHD_FOOTER *) buffer;

  memcpy(footer->cookie, "conectix", 8);
  footer->features = ENDIAN_32U(2);
  footer->version = ENDIAN_32U(0x00010000);
  footer->data_offset = ENDIAN_64U(0xFFFFFFFFFFFFFFFFULL);
  footer->time_stamp = ENDIAN_32U(seconds);
  memcpy(footer->creator_ap, "ULTM", 4);
  footer->creator_ver = ENDIAN_32U(0x002C0021);
  footer->creator_host_os = ENDIAN_32U(0x6B326957); // Windows = 'Wi2k'
  footer->original_size = 
    footer->current_size = ENDIAN_64U(large_size.QuadPart);
  footer->disk_geometry.cylinder = ENDIAN_16U(cyl);
  footer->disk_geometry.heads = 16;
  footer->disk_geometry.spt = 63;
  footer->disk_type = ENDIAN_32U(2); // fixed
  GUID_Create(&footer->uuid, GUID_TYPE_RANDOM);
  footer->saved_state = 0;

  // calculate crc of this footer
  DWORD crc = 0;
  for (unsigned i=0; i<m_sect_size; i++)
    crc += buffer[i];
  footer->checksum = ENDIAN_32U(~crc);

  // write the footer
  // (no need to update m_file_length or anything else, since
  //  the reload below will update all this for us)
  m_file.Write(buffer, m_sect_size);
  
  // free the buffer
  free(buffer);

  // reload the file
  SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

void CUltimateDlg::OnToolsErase() {
  LARGE_INTEGER large_int;
  DWORD64 Size;
  
  if (AfxMessageBox("This will erase the whole image file!  Continue?", MB_YESNO, 0) == IDYES) {
    BYTE *buffer = (BYTE *) calloc(m_sect_size, 1);
    Size = m_file_length.QuadPart / m_sect_size;
    large_int.QuadPart = 0;
    ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
    for (DWORD64 lba=0; lba<Size; lba++)
      m_file.Write(buffer, m_sect_size);
    free(buffer);
    
    SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
  }
}

// View/Edit a VDI header
void CUltimateDlg::OnViewVDIHeader() {
  CFile vdi_file;
  CVDI vdi;

  CFileDialog dlg (
    TRUE,             // Create an open file dialog
    _T(".vdi"),       // Default file extension
    NULL,             // Default Filename
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, // flags
    _T(".vdi files (.vdi)|*.vdi|")    // Filter string
    _T("All Files (*.*)|*.*|")        // Filter string
    _T("|")
  );
  if (dlg.DoModal() != IDOK)
    return;
  
  POSITION pos = dlg.GetStartPosition();
  if (vdi_file.Open(dlg.GetNextPathName(pos), CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyWrite, NULL) == 0) {
    AfxMessageBox("Error Opening Image File...");
    return;
  }

  vdi_file.Read(vdi.m_buffer, 512);
  if (vdi.DoModal() == IDOK) {
    vdi_file.SeekToBegin();
    vdi_file.Write(vdi.m_buffer, 512);
  }
  vdi_file.Close();  
}

// this will place a CDROM El Torito "image" at LBA 64 (CDROM LBA 16).
// it will copy the existing MBR partition entries, not counting the 0xEE,
//  and place another MBR at the sector just before the first partition.
// The emulation of the El Torito should show a disk starting at this new MBR
//  and ending at the last sector of the last partition.
//
// This assumes the user has already made sure there is enough unused room
//  from LBA 16 to LBA 16+x
void CUltimateDlg::OnToolsHybridCDROM() {
  struct MBR_PART_ENTRY entries[4] /*, *p_entries */;
  DWORD sector_count = 0;  // count of sectors used (1 for MBR + (last sector of last partition - LBA of MBR))
  DWORD first_usable = 0xFFFFFFFF;  // need to find the first LBA of the closest partition to zero.
  int i, entry_count = 0;
  BYTE sys_id = 0;
  
  // need to get the entries from the existing MBR
  // if none are found (i.e.: Starting and size are zero), abort.
  if (m_MBRCount == 0) {
    AfxMessageBox("Did not find the primary MBR to extract partition information.  Aborting!");
    return;
  }

  // this only searches the primary MBR at LBA 0
  for (i=0; i<4; i++) {
    if (convert32(Mbr[0].m_Pages[i].m_sys_id) != 0xEE) {
      if ((convert32(Mbr[0].m_Pages[i].m_start_lba) > 0) && (convert32(Mbr[0].m_Pages[i].m_size) > 0)) {
        entries[entry_count].boot_id = convert32(Mbr[0].m_Pages[i].m_boot_id);
        if (entries[entry_count].boot_id == 0x80)
          sys_id = convert32(Mbr[0].m_Pages[i].m_sys_id);
        //entries[entry_count].start.cyl = convert32(Mbr[0].m_Pages[i].m_begin_cyl);
        //entries[entry_count].start.head = convert32(Mbr[0].m_Pages[i].m_begin_head);
        //entries[entry_count].start.sector = convert32(Mbr[0].m_Pages[i].m_begin_sect);
        //entries[entry_count].sys_id = convert32(Mbr[0].m_Pages[i].m_sys_id);
        //entries[entry_count].end.cyl = convert32(Mbr[0].m_Pages[i].m_end_cyl);
        //entries[entry_count].end.head = convert32(Mbr[0].m_Pages[i].m_end_head);
        //entries[entry_count].end.sector = convert32(Mbr[0].m_Pages[i].m_end_sect);
        entries[entry_count].start_lba = convert32(Mbr[0].m_Pages[i].m_start_lba);
        entries[entry_count].sectors = convert32(Mbr[0].m_Pages[i].m_size);
        // find the count of sectors used (not counting MBR)
        if ((entries[entry_count].start_lba + entries[entry_count].sectors) > sector_count)
          sector_count = (entries[entry_count].start_lba + entries[entry_count].sectors);
        // find the first (in consecutive order) LBA used
        if (entries[entry_count].start_lba < first_usable)
          first_usable = entries[entry_count].start_lba;
        entry_count++;
      }
    }
  }
  
  // must find at least one partition entry
  if (entry_count == 0) {
    AfxMessageBox("Did not find any available entries in primary MBR.  Aborting!");
    return;
  }

  // Count of sectors is last sector found, minus ((BOOT_CAT_SECT + 1) * 4)
  if (sector_count <= ((BOOT_CAT_SECT + 1) * 4)) {
    AfxMessageBox("Did not find enough partition sectors.  Aborting!");
    return;
  }

  // if we will overwrite the first partition, give error
  if (((BOOT_CAT_SECT + 1) * 4) >= first_usable) {
    AfxMessageBox("Not enough room between disc sector 16 and first partition found.  Aborting!");
    return;
  }
  
  // allocate a minumal buffer
  BYTE *buffer = (BYTE *) malloc(MAX_SECT_SIZE);

  // create the PVD
  struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) buffer;
  pvd->type = 1;
  memcpy(pvd->ident, "CD001", 5);
  pvd->ver = 1;
  memcpy(pvd->sys_ident, "WINXP                           ", 32);
  memcpy(pvd->vol_ident, "Forever Young Software 1984-2022", 32);
  pvd->num_lbas = (sector_count + 3) / 4;  // convert to 2048-byte sectors (+ 1 for the MBR)
  pvd->num_lbas_b = ENDIAN_32U(pvd->num_lbas);
  pvd->set_size = 1;
  pvd->set_size_b = ENDIAN_16U(1);
  pvd->sequ_num = 1;
  pvd->sequ_num_b = ENDIAN_16U(1);
  pvd->lba_size = 2048;
  pvd->lba_size_b = ENDIAN_16U(2048);
  pvd->path_table_size = 0;
  pvd->path_table_size_b = 0;
  pvd->pathl_loc = 0;
  pvd->pathlo_loc = 0;
  pvd->pathm_loc = 0;
  pvd->pathmo_loc = 0;

  pvd->root.len = 34;
  pvd->root.e_attrib = 0;
  pvd->root.extent_loc = BOOT_CAT_SECT + 1;
  pvd->root.extent_loc_b = ENDIAN_32U(BOOT_CAT_SECT + 1);
  pvd->root.data_len = 2048;
  pvd->root.data_len_b = ENDIAN_32U(2048);
      
  CTime time = CTime::GetCurrentTime();
  pvd->root.date.since_1900 = (BYTE) (time.GetYear() - 1900);
  pvd->root.date.month = (BYTE) time.GetMonth();
  pvd->root.date.day = (BYTE) time.GetDay();
  pvd->root.date.hour = (BYTE) time.GetHour();
  pvd->root.date.min = (BYTE) time.GetMinute();
  pvd->root.date.sec = (BYTE) time.GetSecond();
  pvd->root.date.gmt_off = 0; ////
      
  pvd->root.flags = 0x02;  // directory
  pvd->root.unit_size = 0;
  pvd->root.gap_size = 0;
  pvd->root.sequ_num = 1;
  pvd->root.sequ_num_b = ENDIAN_16U(1);
  pvd->root.fi_len = 1;
  pvd->root.ident[0] = 0;
  memset(pvd->set_ident, 0x20, 128);
  memset(pvd->pub_ident, 0x20, 128);
  memset(pvd->prep_ident, 0x20, 128);
  memset(pvd->app_ident, 0x20, 128);
  memcpy(pvd->app_ident, "Forever Young Software  ULTIMATE.EXE", 36);
  memset(pvd->copy_ident, 0x20, 37);
  memset(pvd->abs_ident, 0x20, 37);
  memset(pvd->bib_ident, 0x20, 37);
      
  //struct S_ISO_DATE_TIME vol_date;
  sprintf(pvd->vol_date.year, "%04i", time.GetYear());
  sprintf(pvd->vol_date.month, "%02i", time.GetMonth());
  sprintf(pvd->vol_date.day, "%02i", time.GetDay());
  sprintf(pvd->vol_date.hour, "%02i", time.GetHour());
  sprintf(pvd->vol_date.min, "%02i", time.GetMinute());
  sprintf(pvd->vol_date.sec, "%02i", time.GetSecond());
  sprintf(pvd->vol_date.jiffies, "%02i", 0);
  pvd->vol_date.gmt_off = 0;
      
  //struct S_ISO_DATE_TIME mod_date;
  sprintf(pvd->mod_date.year, "%04i", time.GetYear());
  sprintf(pvd->mod_date.month, "%02i", time.GetMonth());
  sprintf(pvd->mod_date.day, "%02i", time.GetDay());
  sprintf(pvd->mod_date.hour, "%02i", time.GetHour());
  sprintf(pvd->mod_date.min, "%02i", time.GetMinute());
  sprintf(pvd->mod_date.sec, "%02i", time.GetSecond());
  sprintf(pvd->mod_date.jiffies, "%02i", 0);
  pvd->mod_date.gmt_off = 0;
      
  //struct S_ISO_DATE_TIME exp_date;
  //sprintf(pvd->exp_date.year, "%04i", time.GetYear());
  //sprintf(pvd->exp_date.month, "%02i", time.GetMonth());
  //sprintf(pvd->exp_date.day, "%02i", time.GetDay());
  //sprintf(pvd->exp_date.hour, "%02i", time.GetHour());
  //sprintf(pvd->exp_date.min, "%02i", time.GetMinute());
  //sprintf(pvd->exp_date.sec, "%02i", time.GetSecond());
  //sprintf(pvd->exp_date.jiffies, "%02i", 0);
  //pvd->exp_date.gmt_off = 0;
      
  //struct S_ISO_DATE_TIME val_date;
  sprintf(pvd->val_date.year, "%04i", time.GetYear());
  sprintf(pvd->val_date.month, "%02i", time.GetMonth());
  sprintf(pvd->val_date.day, "%02i", time.GetDay());
  sprintf(pvd->val_date.hour, "%02i", time.GetHour());
  sprintf(pvd->val_date.min, "%02i", time.GetMinute());
  sprintf(pvd->val_date.sec, "%02i", time.GetSecond());
  sprintf(pvd->val_date.jiffies, "%02i", 0);
  pvd->val_date.gmt_off = 0;
      
  pvd->struct_ver = 1;
      
  // write the PVD
  WriteToFile(buffer, PVD_SECT, 1);
  
  
  // create Boot Descriptor
  memset(buffer, 0, 2048);
  struct S_ISO_BVD *bvd = (struct S_ISO_BVD *) buffer;
  bvd->type = 0;
  memcpy(bvd->ident, "CD001", 5);
  bvd->ver = 1;
  memcpy(bvd->sys_ident, "EL TORITO SPECIFICATION", 23);
  bvd->boot_cat = BOOT_CAT_SECT;
  WriteToFile(buffer, BVD_SECT, 1);
  
  // write Termination Volume Descriptor sector
  memset(buffer, 0, 2048);
  struct S_ISO_TERM *term = (struct S_ISO_TERM *) buffer;
  term->id = 255;
  memcpy(term->ident, "CD001", 5);
  term->ver = 1;
  WriteToFile(buffer, TVD_SECT, 1);
  
  // write Boot Catalog sector
  memset(buffer, 0, 2048);
  struct S_ISO_BOOT_CAT *boot_cat = (struct S_ISO_BOOT_CAT *) buffer;

  // validation entry
  boot_cat->val_entry.id = 1;        // Header ID = 1
  boot_cat->val_entry.platform = 0;  // 0 = 80x86
  memcpy(boot_cat->val_entry.ident, "FYS ULTIMATE.EXE", 16);  // up to 24
  boot_cat->val_entry.key55 = 0x55;  // must be 0x55
  boot_cat->val_entry.keyAA = 0xAA;  // must be 0xAA
  WORD crc = 0, *crc_p = (WORD *) &boot_cat->val_entry.id;
  for (i=0; i<16; i++) {
    if (i == 14) continue;  // skip current crc value
    crc += crc_p[i];
  }
  boot_cat->val_entry.crc = -crc;

  // Initial/Default entry
  boot_cat->init_entry.bootable = 0x88;   // 0x88 = bootable, 0x00 = non-bootable
  boot_cat->init_entry.media = 4;         // 4 = hard drive image
  boot_cat->init_entry.load_seg = 0x0000; // 0x0000 = use default address of 0x07C0
  boot_cat->init_entry.sys_type = sys_id; // system type = needs to be the same ID as the partition sys_id value
  boot_cat->init_entry.load_cnt = 4;      // load 'count' sectors for boot (Since these are 512-byte sectors, we need to read 4 to get the 2048 sector)
  boot_cat->init_entry.load_rba = 0;      // Start address of virtual disk
  
  // end entry
  boot_cat->end_entry.id = 0x91;          // no more entries follow
  WriteToFile(buffer, BOOT_CAT_SECT, 1);
  
  // root table
  memset(buffer, 0, 2048);
  WriteToFile(buffer, BOOT_CAT_SECT + 1, 1);
  
  // free the buffer
  free(buffer);

  // Success.  Reload the image
  AfxMessageBox("Successfully added the Hybrid CDROM contents.");
  SendMessage(WM_COMMAND, ID_FILE_RELOAD, 0);
}

void CUltimateDlg::OnRecentFileMenu(UINT uID) {
  if ((uID >= ID_FILE_MRU_FILE1) && (uID <= ID_FILE_MRU_FILE16)) {
    int i = uID - ID_FILE_MRU_FILE1;
    FileOpen(m_rfl->m_arrNames[i]);
  } else
    OnFileClose();
}

// "manually" adds/sorts MRL list
void CUltimateDlg::AddToMRUList(CString csPath) {
  int i, j, max = m_rfl->GetSize();
  CString csNames[MRL_MAX];
  
  for (i=0; i<max && i<MRL_MAX; i++)
    csNames[i] = m_rfl->m_arrNames[i];
  
  m_rfl->m_arrNames[0] = csPath;
  for (i=0, j=1; j<max && j<MRL_MAX && i<max && i<MRL_MAX; i++) {
    if (csNames[i] == csPath)
      continue;
    m_rfl->m_arrNames[j] = csNames[i];
    j++;
  }
}

// Remove csPath from the MRU list (if it exists)
void CUltimateDlg::RemoveFromMRUList(CString csPath) {
  int i, max = m_rfl->GetSize();
  
  for (i=0; i<max && i<MRL_MAX; i++) {
    if (m_rfl->m_arrNames[i] == csPath) {
      for (; i<(max-1) && i<(MRL_MAX-1); i++)
        m_rfl->m_arrNames[i] = m_rfl->m_arrNames[i+1];
      m_rfl->m_arrNames[i] = "";
      break;
    }
  }
}

void CUltimateDlg::OnHelpHelp() {
  CString url = AfxGetApp()->GetProfileString("Settings", "DefaultHelpURL", NULL);
  ShellExecute(AfxGetApp()->m_pMainWnd->m_hWnd, "open", 
    url + "main.html",
    NULL, NULL, SW_SHOWNORMAL);
}

void CUltimateDlg::OnHelpAbout() {
  CAboutDlg dlgAbout;
  dlgAbout.DoModal();
}

void CUltimateDlg::OnUpdateFileMRUFile1(CCmdUI* pCmdUI) {
  // must have this so that the "core" doesn't get called...
}

// Computer\HKEY_CURRENT_USER\Software\Forever Young Software\ultimate\Recent Files
void CUltimateDlg::OnInitMenuPopup(CMenu *pMenu, UINT nIndex, BOOL bSysMenu) {
  CString csPath;
  CCmdUI cmdUI;
  int max, i;

  // update the menu items
  pMenu->EnableMenuItem(ID_FILE_CLOSE, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_FILE_RELOAD, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_TOOLS_RESIZE, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_TOOLS_ERASE_IMAGE, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_GET_DISK_IMAGE, !m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_APPEND_VHD, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_TOOLS_ADDHYBRIDCDROM, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_TOOLS_VIEWVDIHEADER, !m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  pMenu->EnableMenuItem(ID_TOOLS_CREATE_INFO, m_bIsOpen ? MF_ENABLED : MF_GRAYED);
  
  if (!bSysMenu) {
    // get the first string in the list
    // if there is nothing in the list, the UpdateMenu() call below, crashes this app
    csPath = m_rfl->operator[](0);
    if (csPath.GetLength() > 0) {
      max = pMenu->GetMenuItemCount();
      for (i=0; i<max; i++) {
        pMenu->GetMenuString(i, csPath, MF_BYPOSITION);
        if (csPath == "Recent &Files") {
          CMenu *pSubMenu = pMenu->GetSubMenu(i);
          if (pSubMenu != NULL) {
            cmdUI.m_nIndex = nIndex;
            cmdUI.m_nID = ID_FILE_MRU_FILE1;
            cmdUI.m_pMenu = pSubMenu;
            m_rfl->UpdateMenu(&cmdUI);
          }
          break;
        }
      }
    }
  }
}

LARGE_INTEGER CUltimateDlg::GetFileLength(HANDLE hFile) {
  LARGE_INTEGER large;
  
  large.QuadPart = 0;
  switch (m_file_type) {
    case DLG_FILE_TYPE_FLAT:
      ::GetFileSizeEx(hFile, &large);
      break;
    case DLG_FILE_TYPE_VB_VDI:
      large.QuadPart = m_vdi_disk_size;
      break;
    
    default:
      AfxMessageBox("Error: Unknown DLF_FILE_TYPE...");
  }
  
  return large;
}

// returns file size in sectors
// assumes not a CDROM
// will include the VHD if one is attached
DWORD64 CUltimateDlg::GetFileSectCount(void) {
  return (DWORD64) (GetFileLength((HANDLE) m_file.m_hFile).QuadPart / m_sect_size);
}

BOOL CUltimateDlg::SetFileLength(HANDLE hFile, const DWORD64 Size) {
  BOOL ret = FALSE;
  LARGE_INTEGER large;
  
  large.QuadPart = Size;
  switch (m_file_type) {
    case DLG_FILE_TYPE_FLAT:
      ret = ::SetFilePointerEx(hFile, large, NULL, FILE_BEGIN);
      if (ret != 0)
        ret = ::SetEndOfFile(hFile);
      break;
    case DLG_FILE_TYPE_VB_VDI:
      AfxMessageBox("VDI images are not resizable...");
      break;
    default:
      AfxMessageBox("Error: Unknown DLF_FILE_TYPE...");
  }
  
  return ret;
}

// only display an error if we are under the max allowed to display from the Settings menu
int CUltimateDlg::DoError(CString csStr) {

  int max_count = AfxGetApp()->GetProfileInt("Settings", "MaxErrorCount", 10);
  int ret = 0;

  if (m_MaxErrorCount < max_count)
    ret = AfxMessageBox(csStr);
  
  m_MaxErrorCount++;
  
  if (m_MaxErrorCount == max_count)
    AfxMessageBox("Error Count has reached Settings:MaxCount.\n"
                  "This error will not be displayed further.");

  return ret;
}

long CUltimateDlg::ReadFromFile(void *buffer, DWORD64 lba, long count) {
  LARGE_INTEGER large_int;
  CString cs;
  long cnt;
  BYTE *ptr = (BYTE *) buffer;

  if (count == 0)
    return 0;
  
  switch (m_file_type) {
    case DLG_FILE_TYPE_FLAT: {
      large_int.QuadPart = (lba * m_sect_size);
      
      // if we will be reading from past end of file, give an error
      if ((large_int.QuadPart + (count * m_sect_size)) > m_file_length.QuadPart) {
        cs.Format("ReadFromFile: Error trying to read outside of image file...(%I64i  %i)", lba, count);
        DoError(cs);
        return 0;
      }
      
      ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
      if (m_file.Read(buffer, count * m_sect_size) != count * m_sect_size) {
        cs.Format("Error reading %i sectors from LBA %I64i", count, lba);
        AfxMessageBox(cs);
        return 0;
      }
    } break;
    
    case DLG_FILE_TYPE_VB_VDI: {
      //  ?? reading from a flat type image should be identical to reading from a dynamic,
      //  ??  only all blocks will already be allocated
      BOOL display_error = TRUE;
      DWORD64 ByteOffset = lba * m_sect_size;
      for (cnt=0; cnt<count; cnt++) {
        // check to see if we are out of bounds
        if ((ByteOffset + m_sect_size) > m_vdi_disk_size) {
          cs.Format("Error reading %i sectors from LBA %I64i (out of bounds)", count, lba);
          AfxMessageBox(cs);
          return 0;
        }
        
        // get the block value to use
        DWORD block = m_vdi_blocks[ByteOffset / m_vdi_block_size];
        if (block == VDI_BLOCK_ID_UNALLOCATED) { // unallocated block
          // we give an error on the first sector try only
          if (display_error) {
            cs.Format("VDI File: Trying to read from an unallocated block (%I64i  %i)", lba, count);
            AfxMessageBox(cs);
            display_error = FALSE;
          }
          // so that there is no confusion or so that we don't get errornous values,
          //  we return all zeros for the sector(s) read
          memset(ptr, 0, m_sect_size);
        } else if (block == VDI_BLOCK_ID_ZERO_BLOCK) { // zero block
          memset(ptr, 0, m_sect_size);
        } else if (block < m_vdi_blocks_allocated) {
          // get the block and extract the sector
          large_int.QuadPart = m_vdi_offset_data + (block * m_vdi_block_size) + (ByteOffset % m_vdi_block_size);
          ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
          if (m_file.Read(ptr, m_sect_size) != m_sect_size) {
            cs.Format("Error reading %i sectors from LBA %I64i", count, lba);
            AfxMessageBox(cs);
            return 0;
          }
        } else {
          // if the block number is more than the currently allocated blocks, we have an error
          cs.Format("Trying to read from an unallocated block (%I64i  %i)", lba, count);
          AfxMessageBox(cs);
          return 0;
        }
        ptr += m_sect_size;
        ByteOffset += m_sect_size;
      }
    } break;
      
    default:
      AfxMessageBox("Error: Unknown DLF_FILE_TYPE...");
  }
  
  return count;
}

// GUI Throws and exception if in error
void CUltimateDlg::WriteToFile(void *buffer, DWORD64 lba, long count) {
  if (!IsDlgButtonChecked(IDC_FORCE_READONLY)) {
    LARGE_INTEGER large_int;
    CString cs;
    long cnt;
    BYTE *ptr = (BYTE *) buffer;
    
    if (count == 0)
      return;
    
    switch (m_file_type) {
      case DLG_FILE_TYPE_FLAT: {
        LARGE_INTEGER large_int;
        large_int.QuadPart = (lba * m_sect_size);
        
        // if we will be writing past end of file, give an error
        if (!m_overwrite_okay)
          if ((large_int.QuadPart + (count * m_sect_size)) > m_file_length.QuadPart) {
            cs.Format("WriteToFile: Error trying to write outside of image file...(%I64i  %i)", lba, count);
            AfxMessageBox(cs);
            return;
          }
        
        ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
        m_file.Write(buffer, count * m_sect_size);
      } break;
        
      case DLG_FILE_TYPE_VB_VDI: {
        // Flat files will always be allocated...
        //   m_vdi_image_type = VDI_IMAGE_TYPE_DYNAMIC or VDI_IMAGE_TYPE_FLAT
        DWORD64 ByteOffset = lba * m_sect_size;
        for (cnt=0; cnt<count; cnt++) {
          // check to see if we are out of bounds
          if ((ByteOffset + m_sect_size) > m_vdi_disk_size) {
            cs.Format("Error trying to write %i sectors to LBA %I64i (out of bounds)", count, lba);
            AfxMessageBox(cs);
            return;
          }
          
          // get the block value to use
          DWORD block = m_vdi_blocks[ByteOffset / m_vdi_block_size];
          // if the sector data is not zeros, and
          //        block is unallocated or block is zeros, 
          //        then allocate a new block
          if ((block == VDI_BLOCK_ID_UNALLOCATED) || (block == VDI_BLOCK_ID_ZERO_BLOCK)) {
            // calculate if the sector is all zeros
            // This will slow down the writes considerably. (but only when the block has yet to be allocated)
            if (!IsBufferEmpty(ptr, m_sect_size)) {
              // get next available block to use
              block = m_vdi_blocks_allocated++;
              
              // move to that block area and write a full block of zeros (plus this sector's data)
              large_int.QuadPart = m_vdi_offset_data + (block * m_vdi_block_size);
              ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
              BYTE *zeros = (BYTE *) calloc(m_vdi_block_size, 1);
              memcpy(zeros + (ByteOffset % m_vdi_block_size), ptr, m_sect_size);
              m_file.Write(zeros, m_vdi_block_size);
              free(zeros);
              
              // mark the table
              m_vdi_blocks[ByteOffset / m_vdi_block_size] = block;
              m_vdi_table_dirty = TRUE;
            } else {
              // if sector data is zeros but block was unallocated, mark it as a zero block
              if (block == VDI_BLOCK_ID_UNALLOCATED) {
                // mark the table
                m_vdi_blocks[ByteOffset / m_vdi_block_size] = VDI_BLOCK_ID_ZERO_BLOCK;
                m_vdi_table_dirty = TRUE;
              }
            }
          } else if (block < m_vdi_blocks_allocated) {
            // get the block and extract the sector
            large_int.QuadPart = m_vdi_offset_data + (block * m_vdi_block_size) + (ByteOffset % m_vdi_block_size);
            ::SetFilePointerEx((HANDLE) m_file.m_hFile, large_int, NULL, FILE_BEGIN);
            m_file.Write(ptr, m_sect_size);
          } else {
            // if the block number is more than the currently allocated blocks, we have an error
            cs.Format("Trying to write to an unallocated block (%I64i  %i)", lba, count);
            AfxMessageBox(cs);
            return;
          }
          ptr += m_sect_size;
          ByteOffset += m_sect_size;
        }
      } break;
      
      default:
        AfxMessageBox("Error: Unknown DLF_FILE_TYPE...");
    }
  }
}

void CUltimateDlg::ReadBlocks(void *buffer, DWORD64 base, DWORD64 block, DWORD block_size, long count) {
  
  // we need to calculate the correct sector offset and count
  if (block_size < m_sect_size) {
    // This isn't very efficient, but how many times will the block size
    //  be less than the sector size?
    BYTE buff[MAX_SECT_SIZE];
    BYTE *p = (BYTE *) buffer;
    while (count--) {
      DWORD64 sector = (block * block_size) / m_sect_size;
      DWORD64 offset = (block * block_size) % m_sect_size;
      ReadFromFile(buff, base + sector, 1);
      memcpy(p, buff + offset, block_size);
      p += block_size;
      block++;
    }
  } else if (block_size == m_sect_size)
    ReadFromFile(buffer, base + block, count);
  else  //if (block_size > m_sect_size)
    ReadFromFile(buffer, base + (block * (block_size / m_sect_size)), (count * (block_size / m_sect_size)));
}

void CUltimateDlg::WriteBlocks(void *buffer, DWORD64 base, DWORD64 block, DWORD block_size, long count) {
  
  // we need to calculate the correct sector offset and count
  // possibly reading in a sector since block size is less than sector size
  if (block_size < m_sect_size) {
    // This isn't very efficient, but how many times will the block size
    //  be less than the sector size?
    BYTE buff[MAX_SECT_SIZE];
    BYTE *p = (BYTE *) buffer;
    while (count--) {
      DWORD64 sector = (block * block_size) / m_sect_size;
      DWORD64 offset = (block * block_size) % m_sect_size;
      ReadFromFile(buff, base + sector, 1);
      memcpy(buff + offset, p, block_size);
      WriteToFile(buff, base + sector, 1);
      p += block_size;
      block++;
    }
  } else if (block_size == m_sect_size)
    WriteToFile(buffer, base + block, count);
  else  //if (block_size > m_sect_size)
    WriteToFile(buffer, base + (block * (block_size / m_sect_size)), (count * (block_size / m_sect_size)));
}

// insert sectors starting at lba
// assumes not a CDROM
void CUltimateDlg::InsertSectors(const DWORD64 start_lba, const long count) {
  DWORD64 lba = (DWORD64) GetFileSectCount() - 1;
  BYTE buffer[MAX_SECT_SIZE];

  m_overwrite_okay = TRUE;  // okay to write past eof
  
  while (lba >= start_lba) {
    if (ReadFromFile(buffer, lba, 1) == 0)
      break;
    WriteToFile(buffer, lba + count, 1);
    if (lba == 0) break; // catch zero since we are unsigned lba value
    lba--;
  }
  
  m_overwrite_okay = FALSE;  // not okay to write past eof

  // update the member with the new file size
  m_file_length = GetFileLength((HANDLE) m_file.m_hFile);
}

// remove sectors starting at lba
// assumes not a CDROM
void CUltimateDlg::RemoveSectors(const DWORD64 start_lba, const long count) {
  DWORD64 lba = start_lba;
  DWORD64 size = m_file_length.QuadPart / m_sect_size;
  BYTE buffer[MAX_SECT_SIZE];

  for (long i=0; i<count; i++) {
    if (lba >= size) 
      break;
    if (ReadFromFile(buffer, lba, 1) == 0)
      break;
    WriteToFile(buffer, lba - count, 1);
    lba++;
  }

  // update the member with the new file size
  m_file_length = GetFileLength((HANDLE) m_file.m_hFile);
  m_file_length.QuadPart -= ((unsigned) count * m_sect_size);
  SetFileLength((HANDLE) m_file.m_hFile, m_file_length.QuadPart);
}

void CUltimateDlg::OnChangeSectSize() {
  UpdateData(TRUE);  // bring from Dialog
  
  // update the member(s) as well
  m_sect_size = m_dflt_sect_size = g_sector_sizes[m_sect_size_option];

  // write it to the registry
  AfxGetApp()->WriteProfileInt("Settings", "DefaultSectSize", m_sect_size_option);
}

// Called from a volume/partition file system to update the
//  FYSOS signature in the boot sector of that volume
// (We place it here because it is the same for all file systems)
// assumes not a CDROM
BOOL CUltimateDlg::UpdateSig(DWORD64 lba) {
  BYTE buffer[MAX_SECT_SIZE];
  struct S_FYSOSSIG *s_sig = (struct S_FYSOSSIG *) (buffer + S_FYSOSSIG_OFFSET);
  CFYSOSSig sig;
  
  ReadFromFile(buffer, lba, 1);
  sig.m_fysos_sig.Format("0x%08X", s_sig->sig);
  sig.m_sig_base.Format("%I64i", s_sig->base);
  sig.m_boot_sig.Format("0x%04X", s_sig->boot_sig);
  sig.m_base_lba = lba;
  if (sig.DoModal() == IDOK) {
    s_sig->sig = convert32(sig.m_fysos_sig);
    s_sig->base = convert64(sig.m_sig_base);
    s_sig->boot_sig = convert16(sig.m_boot_sig);
    WriteToFile(buffer, lba, 1);
    return TRUE;
  }
  return FALSE;
}

// Settings Dialog
void CUltimateDlg::OnAppSettings() {
  CWinApp *App = AfxGetApp();
  CSettings dlg;
  
  dlg.m_force_readonly = App->GetProfileInt("Settings", "ForceReadOnly", TRUE);
  dlg.m_force_fysos = App->GetProfileInt("Settings", "ForceFYSOS", TRUE);
  dlg.m_max_error_count = App->GetProfileInt("Settings", "MaxErrorCount", 10);
  dlg.m_help_path = App->GetProfileString("Settings", "DefaultHelpURL", NULL);
  dlg.m_mbr_path = App->GetProfileString("Settings", "DefaultMBRPath", NULL);
  dlg.m_embr_path = App->GetProfileString("Settings", "DefaultEMBRPath", NULL);
  dlg.m_extract_path = App->GetProfileString("Settings", "DefaultExtractPath", NULL);
  dlg.m_viewer_path = App->GetProfileString("Settings", "DefaultViewerPath", "notepad.exe");
  dlg.m_clear_mru = FALSE;
  
  if (dlg.DoModal() == IDOK) {
    App->WriteProfileInt("Settings", "ForceReadOnly", dlg.m_force_readonly);
    App->WriteProfileInt("Settings", "ForceFYSOS", dlg.m_force_fysos);
    App->WriteProfileInt("Settings", "MaxErrorCount", dlg.m_max_error_count);
    App->WriteProfileString("Settings", "DefaultHelpURL", dlg.m_help_path);
    App->WriteProfileString("Settings", "DefaultMBRPath", dlg.m_mbr_path);
    App->WriteProfileString("Settings", "DefaultEMBRPath", dlg.m_embr_path);
    App->WriteProfileString("Settings", "DefaultExtractPath", dlg.m_extract_path);
    App->WriteProfileString("Settings", "DefaultViewerPath", dlg.m_viewer_path);
    if (dlg.m_clear_mru) {
      int i = m_rfl->GetSize();
      while (i--)
        m_rfl->Remove(0);
      m_rfl->WriteList();
    }
    UpdateData(TRUE);   // get from dialog
    m_force_readonly = dlg.m_force_readonly;
    UpdateData(FALSE);   // send to dialog
  }
}

// don't allow one to be checked if the other is checked
void CUltimateDlg::OnForceMbr() {
  if (IsDlgButtonChecked(IDC_FORCE_MBR))
    if (IsDlgButtonChecked(IDC_FORCE_NO_MBR))
      CheckDlgButton(IDC_FORCE_NO_MBR, BST_UNCHECKED);
}

// don't allow one to be checked if the other is checked
void CUltimateDlg::OnForceNoMbr() {
  if (IsDlgButtonChecked(IDC_FORCE_NO_MBR))
    if (IsDlgButtonChecked(IDC_FORCE_MBR))
      CheckDlgButton(IDC_FORCE_MBR, BST_UNCHECKED);
}
