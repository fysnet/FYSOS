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

// ImageBar.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "ImageBar.h"
#include "Mbr.h"
#include "VHD.h"

#include "Fat.h"
#include "Lean.h"
#include "Ext2.h"
#include "ExFat.h"
#include "NTFS.h"
#include "FSZ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageBar

CImageBar::CImageBar() {
}

CImageBar::~CImageBar() {
}

BEGIN_MESSAGE_MAP(CImageBar, CWnd)
  //{{AFX_MSG_MAP(CImageBar)
    // NOTE - the ClassWizard will add and remove mapping macros here.
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// recursive Check for MBR (and extended Boot Records)
void CImageBar::CheckForMBRRecusive(DWORD64 LBA, DWORD64 TotalBlocks) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int i, j;
  
  // check to make sure we don't loop indefinately via "ill-connected" extended partitions
  for (i=0; i<dlg->m_MBRCount; i++) {
    if (dlg->Mbr[i].m_lba == LBA) {
      AfxMessageBox("Extended MBR pointing to already found (ext) MBR...");
      return;
    }
  }
  
  // "insert" a new (extended) MBR entry?
  if (dlg->m_MBRCount < MAX_SUB_VOLUMES) {
    if (dlg->Mbr[dlg->m_MBRCount].Exists(LBA)) {
      dlg->Mbr[dlg->m_MBRCount].m_draw_index = DrawBox(LBA, 1, TotalBlocks, dlg->Mbr[dlg->m_MBRCount].m_color, TRUE, "MBR", &dlg->Mbr[dlg->m_MBRCount]);
      dlg->Mbr[dlg->m_MBRCount].m_index = dlg->m_MBRCount;
      dlg->Mbr[dlg->m_MBRCount].m_parent_lba = LBA;
      
      // set the title of the Tab
      if (dlg->m_MBRCount == 0)
        dlg->Mbr[dlg->m_MBRCount].m_Title = "MBR";
      else
        dlg->Mbr[dlg->m_MBRCount].m_Title = "ext MBR";
      dlg->Mbr[dlg->m_MBRCount].m_psp.dwFlags |= PSP_USETITLE;
      dlg->Mbr[dlg->m_MBRCount].m_psp.pszTitle = dlg->Mbr[dlg->m_MBRCount].m_Title;
      
      // add the page
      dlg->m_TabControl.AddPage(&dlg->Mbr[dlg->m_MBRCount]);
      
      // check to see if any of there are extended partitions
      j = dlg->m_MBRCount++;
      for (i=0; i<4; i++) {
        DWORD64 lba = convert32(dlg->Mbr[j].m_Pages[i].m_start_lba);
        BYTE type = convert8(dlg->Mbr[j].m_Pages[i].m_sys_id);
        if ((type == 0x05) || (type == 0x0F))
          CheckForMBRRecusive(LBA + lba, TotalBlocks);
      }
    }
  } else
    AfxMessageBox("Too many MBRs");
}

/////////////////////////////////////////////////////////////////////////////
// CImageBar message handlers
void CImageBar::ImageParse(CFile *file) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  CPropertyPage *page = NULL;
  CString cs;
  int i, j;
  
  // make sure dlg members are updated first
  dlg->UpdateData(TRUE); // bring from Dialog
  
  LARGE_INTEGER file_length = dlg->GetFileLength((HANDLE) file->m_hFile);
  
  // ben: this assumes 512-byte blocks  (If we are a ISO (only) image, this will not work)
  DWORD64 TotalBlocks = (file_length.QuadPart + (dlg->m_sect_size - 1)) / dlg->m_sect_size;
  
  // update display numbers
  dlg->SetDlgItemText(IDC_START_DISP, "0");
  cs.Format("%i", TotalBlocks - 1);
  dlg->SetDlgItemText(IDC_END_DISP, cs);
  cs.Format("%i", TotalBlocks / 2);
  dlg->SetDlgItemText(IDC_MID_DISP, cs);

  // check to see if a MBR exists
  CheckForMBRRecusive(0, TotalBlocks);

  // if there was not a valid MBR, there won't be a valid GPT  
  if (dlg->Mbr[0].m_exists) {
    if (dlg->Gpt.Exists(1ULL)) {
      dlg->Gpt.m_draw_index = DrawBox(1, 2, TotalBlocks, dlg->Gpt.m_color, TRUE, "GPT", &dlg->Gpt);
      dlg->m_TabControl.AddPage(&dlg->Gpt);
    }
  }
  
  // encompass the GPT sectors
  if (dlg->Gpt.m_exists) {
    DrawBox(1, dlg->Gpt.m_hdr.last_usable, TotalBlocks, dlg->Gpt.m_color, FALSE, "GPT", &dlg->Gpt);
    DrawBox(1, dlg->Gpt.m_hdr.first_usable - 1, TotalBlocks, dlg->Gpt.m_color, TRUE, "GPT", &dlg->Gpt);
    if (dlg->Gpt.m_hdr.backup_lba && (dlg->Gpt.m_hdr.backup_lba > 63))
      DrawBox(dlg->Gpt.m_hdr.backup_lba - 63, dlg->Gpt.m_hdr.backup_lba, TotalBlocks, dlg->Gpt.m_color, TRUE, "GPT Backup", &dlg->Gpt);
  }

  // if no MBR, then add an "Empty" one
  // (for some reason, the system needs one before the Create() or
  //  it crashes on the first time we add a page)
  if (dlg->m_MBRCount == 0)
    dlg->m_TabControl.AddPage(&dlg->Mbr[0]);

  // if the MBR is a PMBR, change the tab's title
  CPropertyPage *p = dlg->m_TabControl.GetPage(dlg->m_TabControl.GetPageIndex(&dlg->Mbr[0]));
  if (dlg->Gpt.m_exists && dlg->Mbr[0].IsPMBR())
    p->m_psp.pszTitle = "PMBR";
  else
    p->m_psp.pszTitle = "MBR";
  
  // If we have an ISO image as well, parse it
  if (dlg->m_isISOImage) {
    if (dlg->ISO.Start()) {
      if (dlg->ISO.m_BVD.m_is_valid)
        dlg->ISO.m_BVD.m_draw_index = DrawBox(dlg->ISO.m_BVD.m_lba, dlg->ISO.m_BVD.m_lba + dlg->ISO.m_BVD.m_size - 1, TotalBlocks, dlg->ISO.m_BVD.m_color, TRUE, "ISO", &dlg->ISO.m_BVD);
      if (dlg->ISO.m_PVD.m_is_valid)
        dlg->ISO.m_PVD.m_draw_index = DrawBox(dlg->ISO.m_PVD.m_lba, dlg->ISO.m_PVD.m_lba + dlg->ISO.m_PVD.m_size - 1, TotalBlocks, dlg->ISO.m_PVD.m_color, TRUE, "ISO", &dlg->ISO.m_PVD);
      //if (dlg->ISO.m_SVD.m_is_valid)
      //  dlg->ISO.m_SVD.m_draw_index = DrawBox(dlg->ISO.m_SVD.m_lba, dlg->ISO.m_SVD.m_lba + dlg->ISO.m_SVD.m_size - 1, TotalBlocks, dlg->ISO.m_SVD.m_color, TRUE, "ISO", &dlg->ISO.m_SVD);
      if (dlg->ISO.m_BEA.m_is_valid)
        dlg->ISO.m_BEA.m_draw_index = DrawBox(dlg->ISO.m_BEA.m_lba, dlg->ISO.m_BEA.m_lba + dlg->ISO.m_BEA.m_size - 1, TotalBlocks, dlg->ISO.m_BEA.m_color, TRUE, "ISO", &dlg->ISO.m_BEA);
      if (dlg->ISO.m_NSR.m_is_valid)
        dlg->ISO.m_NSR.m_draw_index = DrawBox(dlg->ISO.m_NSR.m_lba, dlg->ISO.m_NSR.m_lba + dlg->ISO.m_NSR.m_size - 1, TotalBlocks, dlg->ISO.m_NSR.m_color, TRUE, "ISO", &dlg->ISO.m_NSR);
    }
  }
  
  // move the Tabs to the correct position
  dlg->m_TabControl.Create(dlg, WS_VISIBLE | WS_CHILD | WS_DLGFRAME);
  dlg->m_TabControl.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
  CRect rect;
  dlg->m_StaticTabs.GetWindowRect(&rect);
  dlg->ScreenToClient(&rect);
  dlg->m_TabControl.MoveWindow(&rect);
  
  // update the items
  for (i=0; i<dlg->m_MBRCount; i++) {
    if (dlg->Mbr[i].m_exists) {
      dlg->m_TabControl.SetActivePage(&dlg->Mbr[i]);
      dlg->Mbr[i].Begin();
      if (!dlg->Gpt.m_exists || !dlg->IsDlgButtonChecked(IDC_FORCE_GPT_ENUM)) {
        page = &dlg->Mbr[i];
        for (j=0; j<4; j++) {
          DWORD64 lba = dlg->Mbr[i].m_parent_lba + convert32(dlg->Mbr[i].m_Pages[j].m_start_lba);
          DWORD64 size = convert32(dlg->Mbr[i].m_Pages[j].m_size);
          BYTE type = convert8(dlg->Mbr[i].m_Pages[j].m_sys_id);
          if ((type > 0) && (size > 0)) {
            if ((type == 0x05) || (type == 0x0F))
              ;
            // detect if we have an Embr.  If not, try to parse the entry anyway
            else if ((type != 0xE0) || !dlg->Embr.Exists(0ULL)) {
              ImageParseVolume(type, lba, size, TotalBlocks);
            }
          }
        }
      }
    }
  }
  
  if (dlg->Gpt.m_exists) {
    dlg->Mbr[0].CheckDlgButton(IDC_MBR_LEGACY_GPT, BST_CHECKED);  // is GPT, so set the Legacy Button on the MBR dialog
    dlg->m_TabControl.SetActivePage(&dlg->Gpt);
    dlg->Gpt.Begin();
    if (dlg->IsDlgButtonChecked(IDC_FORCE_GPT_ENUM)) {
      page = &dlg->Gpt;
      for (i=0; i<dlg->Gpt.m_gpt_entries; i++) {
        DWORD64 lba = convert64(dlg->Gpt.m_Pages[i].m_start_lba);
        DWORD64 size = (convert64(dlg->Gpt.m_Pages[i].m_last_lba) - lba) + 1;
        BYTE type = 0xFF;
        ImageParseVolume(type, lba, size, TotalBlocks);
      }
    }
  } else
    dlg->Mbr[0].GetDlgItem(IDC_MBR_LEGACY_GPT)->EnableWindow(FALSE);
    
  if (dlg->Embr.m_exists) {
    DrawBox(1, dlg->Embr.m_total_sectors - 1, TotalBlocks, dlg->Embr.m_color, FALSE, "eMBR", &dlg->Embr);
    DrawBox(1, 1 + convert16(dlg->Embr.m_remaining) + 1, TotalBlocks, dlg->Embr.m_color, TRUE, "eMBR", &dlg->Embr);
    dlg->m_TabControl.AddPage(&dlg->Embr);
    dlg->m_TabControl.SetActivePage(&dlg->Embr);
    dlg->Embr.Begin();
    page = &dlg->Embr;
      
    for (i=0; i<dlg->Embr.m_embr_entries; i++) {
      DWORD lba = convert32(dlg->Embr.m_Pages[i].m_start_lba);
      DWORD size = convert32(dlg->Embr.m_Pages[i].m_sectors);
      BYTE type = 0xFF;
      ImageParseVolume(type, lba, size, TotalBlocks);
    }
  }
  
  // if there was no MBR, it could be a floppy
  //  or a volume starting at LBA 0
  if (!dlg->m_isISOImage) {
    if (!dlg->Mbr[0].m_exists) {
      ImageParseVolume(0x01, 0, TotalBlocks, TotalBlocks);
      dlg->m_TabControl.SetActivePage(0);
    } else
      dlg->m_TabControl.SetActivePage(page);
  }
  
  // if we found an ISO image as well, parse it too
  if (dlg->m_isISOImage) {
    // do the SVD first
    if (dlg->ISO.m_SVD.m_is_valid) {
      // We have to set the active page so that the framework builds the dialog
      dlg->m_TabControl.SetActivePage(&dlg->ISO.m_SVD);
      dlg->ISO.m_SVD.DoRoot();
    }
    // then the PVD second
    if (dlg->ISO.m_PVD.m_is_valid) {
      // We have to set the active page so that the framework builds the dialog
      dlg->m_TabControl.SetActivePage(&dlg->ISO.m_PVD);
      dlg->ISO.m_PVD.DoRoot();
    }
  }
  
  // if this is an .VHD file, load that as well.
  // (don't try if we already detected a VDI file)
  if (dlg->m_file_type != DLG_FILE_TYPE_VB_VDI) {
    if (dlg->VHD.Exists(TotalBlocks - 1)) {
      dlg->VHD.m_draw_index = DrawBox(dlg->VHD.m_lba, dlg->VHD.m_lba + 1, TotalBlocks, dlg->VHD.m_color, TRUE, "VHD", &dlg->VHD);
      dlg->m_TabControl.AddPage(&dlg->VHD);
      dlg->VHD.Begin();
      dlg->m_hasVHD = TRUE;
    }
  }
  
  dlg->m_TabControl.Invalidate(FALSE);
}

void CImageBar::ImageParseVolume(const BYTE type, const DWORD64 lba, const DWORD64 size, const DWORD64 TotalBlocks) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  DWORD color;
  int fs_type;
  
  // detect file system...
  fs_type = DetectFileSystem(lba, size);
  switch (fs_type) {
    case FS_FAT12:
    case FS_FAT16:
    case FS_FAT32:
      if (dlg->m_FatCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many Fat Volumes...");
        return;
      }
      color = dlg->Fat[dlg->m_FatCount].GetNewColor(dlg->m_FatCount);
      dlg->Fat[dlg->m_FatCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "Fat", &dlg->Fat[dlg->m_FatCount]);
      dlg->Fat[dlg->m_FatCount].Start(lba, size, color, dlg->m_FatCount, fs_type, TRUE);
      dlg->m_FatCount++;
      break;
    case FS_EXT2:
      if (dlg->m_Ext2Count >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many Ext2 Volumes...");
        return;
      }
      color = dlg->Ext2[dlg->m_Ext2Count].GetNewColor(dlg->m_Ext2Count);
      dlg->Ext2[dlg->m_Ext2Count].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "Ext2", &dlg->Ext2[dlg->m_Ext2Count]);
      dlg->Ext2[dlg->m_Ext2Count].Start(lba, size, color, dlg->m_Ext2Count, TRUE);
      dlg->m_Ext2Count++;
      break;
      
    case FS_EXFAT:
      if (dlg->m_ExFatCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many ExFat Volumes...");
        return;
      }
      color = dlg->ExFat[dlg->m_ExFatCount].GetNewColor(dlg->m_ExFatCount);
      dlg->ExFat[dlg->m_ExFatCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "ExFat", &dlg->ExFat[dlg->m_ExFatCount]);
      dlg->ExFat[dlg->m_ExFatCount].Start(lba, size, color, dlg->m_ExFatCount, TRUE);
      dlg->m_ExFatCount++;
      break;
      
    case FS_LEAN:
      if (dlg->m_LeanCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many Lean Volumes...");
        return;
      }
      color = dlg->Lean[dlg->m_LeanCount].GetNewColor(dlg->m_LeanCount);
      dlg->Lean[dlg->m_LeanCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "Lean", &dlg->Lean[dlg->m_LeanCount]);
      dlg->Lean[dlg->m_LeanCount].Start(lba, size, color, dlg->m_LeanCount, TRUE);
      dlg->m_LeanCount++;
      break;
      
    case FS_NTFS:
      if (dlg->m_NTFSCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many NTFS Volumes...");
        return;
      }
      color = dlg->NTFS[dlg->m_NTFSCount].GetNewColor(dlg->m_NTFSCount);
      dlg->NTFS[dlg->m_NTFSCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "NTFS", &dlg->NTFS[dlg->m_NTFSCount]);
      dlg->NTFS[dlg->m_NTFSCount].Start(lba, size, color, dlg->m_NTFSCount, TRUE);
      dlg->m_NTFSCount++;
      break;
      
    case FS_SFS:
      if (dlg->m_SFSCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many SFS Volumes...");
        return;
      }
      color = dlg->SFS[dlg->m_SFSCount].GetNewColor(dlg->m_SFSCount);
      dlg->SFS[dlg->m_SFSCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "SFS", &dlg->SFS[dlg->m_SFSCount]);
      dlg->SFS[dlg->m_SFSCount].Start(lba, size, color, dlg->m_SFSCount, TRUE);
      dlg->m_SFSCount++;
      break;
      
    case FS_FYSFS:
      if (dlg->m_FYSCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many FYSFS Volumes...");
        return;
      }
      color = dlg->FYSFS[dlg->m_FYSCount].GetNewColor(dlg->m_FYSCount);
      dlg->FYSFS[dlg->m_FYSCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "FYSFS", &dlg->FYSFS[dlg->m_FYSCount]);
      dlg->FYSFS[dlg->m_FYSCount].Start(lba, size, color, dlg->m_FYSCount, TRUE);
      dlg->m_FYSCount++;
      break;
      
    case FS_FSZ:
      if (dlg->m_FSZCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many FSZ Volumes...");
        return;
      }
      color = dlg->FSZ[dlg->m_FSZCount].GetNewColor(dlg->m_FSZCount);
      dlg->FSZ[dlg->m_FSZCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "FSZ", &dlg->FSZ[dlg->m_FSZCount]);
      dlg->FSZ[dlg->m_FSZCount].Start(lba, size, color, dlg->m_FSZCount, TRUE);
      dlg->m_FSZCount++;
      break;
      
    default:
      color = COLOR_UNKN;
      if (dlg->m_UCount >= MAX_SUB_VOLUMES) {
        AfxMessageBox("Too many Unknown Volumes...");
        return;
      }
      dlg->Unknown[dlg->m_UCount].m_draw_index = DrawBox(lba, lba+size-1, TotalBlocks, color, TRUE, "Unknown", &dlg->Unknown[dlg->m_UCount]);
      dlg->Unknown[dlg->m_UCount].Start(lba, size, color, dlg->m_UCount, TRUE);
      dlg->m_UCount++;
      break;
  }
}

int CImageBar::DrawBox(DWORD64 start, DWORD64 end, DWORD64 total, DWORD color, BOOL filled, char *title, CPropertyPage *page) {
  int ret;
  
  // fix a few things?
  if (end > total) end = total;
  
  // get a pointer to the main dialog
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  
  RECT ItemRect;
  dlg->m_image_bar.GetWindowRect(&ItemRect);
  
  int WidthOfBox = ItemRect.right - ItemRect.left - 2;
  const double s = (double) (((double) start * 100.0) / (double) total) / 100.0;  // s = percent
  const double e = (double) (((double) end * 100.0) / (double) total) / 100.0;    // e = percent
  ItemRect.left = (long) ((double) WidthOfBox * s);
  ItemRect.right = (long) ((double) WidthOfBox * e);
  
  if (ItemRect.right <= ItemRect.left)
    ItemRect.right = ItemRect.left + 1;
  if ((ItemRect.left == 0) && (start > 0))
    ItemRect.left = 1;
  
  ret = dlg->m_image_bar.AddItem(ItemRect.left, ItemRect.right, color, filled, title, page);
  dlg->m_image_bar.Invalidate(FALSE);
  
  return ret;
}

void CImageBar::Clear(void) {
  // get a pointer to the main dialog
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int i;
  
  for (i=0; i<dlg->m_MBRCount; i++) {
    if (dlg->Mbr[i].m_exists)
      dlg->Mbr[i].Destroy();
    dlg->Mbr[i].m_exists = FALSE;
  }
  if (dlg->Gpt.m_exists)
    dlg->Gpt.Destroy();
  dlg->Gpt.m_exists = FALSE;
  if (dlg->Embr.m_exists)
    dlg->Embr.Destroy();
  dlg->Embr.m_exists = FALSE;
  dlg->VHD.m_exists = FALSE;
  
  dlg->SetDlgItemText(IDC_START_DISP, "start");
  dlg->SetDlgItemText(IDC_MID_DISP, "mid");
  dlg->SetDlgItemText(IDC_END_DISP, "end");
  
  // clear the image bar
  dlg->m_image_bar.Clear();
  
  // remove the pages
  while (dlg->m_TabControl.GetPageCount())
    dlg->m_TabControl.RemovePage(0);
  dlg->m_TabControl.DestroyWindow();
}

// when creating titles on the CTab items, make the text backgroud of the tab title match the color of the ****
/*
(2) I did the following in my CMyPropertySheet; Basicaly I painted the "tab" portion of my ACTIVE tab page 
with the active color (I can color non-active tabs with another color too):

<< CMyPropertySheet.h/cpp >>
afx_msg void OnDrawItem(---)

::OnInitDialog()
CPropertySheet::OnInitDialog();
GetTabControl()->ModifyStyle(0, TCS_OWNERDRAWFIXED);

::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpd)
CDC dc;

dc.Attach(lpd->hDC); RECT rect = lpd->rcItem;

CTabCtrl* tab = GetTabControl();
 TCHAR buffer[256] = {0};

TC_ITEM tabItem;

tabItem.pszText = buffer;

tabItem.cchTextMax = 256;

tabItem.mask = TCIF_TEXT;

int nTabIndex = lpd->itemID;

if (! tab->GetItem(nTabIndex, &tabItem)) return;

//-----------------(1)------------------ Tab Background Color

BOOL bCurrent = (nTabIndex == iTabCur);

CBrush brush;

if (bCurrent) {

brush.CreateSolidBrush(YELLOW); // active tab color

dc.SelectObject(&brush);

dc.FillRect(&lpd->rcItem, &brush);
} else {
brush.CreateSolidBrush(RED);
// Non-active tab color
}

//-----------------(2)------------------ Tab Text & Color
dc.SetBkMode(TRANSPARENT); 
dc.SetTextColor(BLACK);
dc.DrawText(tabItem.pszText, &lpd->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
dc.Detach();
*/

// Detect the file system
int CImageBar::DetectFileSystem(const DWORD64 lba, const DWORD64 size) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  void *buffer;
  int fs_type = -1;
  
  buffer = malloc(65 * MAX_SECT_SIZE);
  dlg->ReadFromFile(buffer, lba, 65);
  
  // detect a FAT file system
  fs_type = DetectFat((struct S_FAT1216_BPB *) buffer, dlg->m_sect_size);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a Ext2 file system
  fs_type = DetectExt2(buffer, size, dlg->m_sect_size);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a ExFat file system
  fs_type = DetectExFat(buffer);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a LeanFS file system
  fs_type = DetectLean(lba, dlg->m_sect_size);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a NTFS FS file system
  fs_type = DetectNTFS(buffer);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a SFS file system
  fs_type = DetectSFS(buffer, dlg->m_sect_size);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a FYSFS file system
  fs_type = DetectFYSFS(buffer, dlg->m_sect_size);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // detect a FSZ file system
  fs_type = DetectFSZ(lba);
  if (fs_type > -1) {
    free(buffer);
    return fs_type;
  }
  
  // No (Known) File System detected
  free(buffer);
  return -1;
}

const BYTE media[11] = { 0x00, 0x01, 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

int CImageBar::DetectFat(struct S_FAT1216_BPB *bpb, const unsigned sect_size) {
  //struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) bpb;
  // check the jmp instruction
  if (!(((bpb->jmp[0] == 0xEB) && (bpb->jmp[2] == 0x90)) ||
        ((bpb->jmp[0] == 0xE9) && ((*(WORD *) &bpb->jmp[1]) < 0x1FE))))
    return -1;
  // check that bytes_per_sec is a power of 2 from 128 -> 4096
  if ((bpb->bytes_per_sect < 128) || (bpb->bytes_per_sect > 4096) || !power_of_two(bpb->bytes_per_sect))
    return -1;
  // check that sect_per_clust is a power of two with a range of 1 -> 128
  if ((bpb->sect_per_clust != 1) && (
    (bpb->sect_per_clust < 1) || (bpb->sect_per_clust > 128) || !power_of_two(bpb->sect_per_clust))
    ) return -1;
  // check that sec_reserved > 0
  if (!bpb->sect_reserved)
    return -1;
  // check that num_fats > 0
  if (!bpb->fats)
    return -1;
  // check that bpb->num_lbas or sectors_ext_lbas is non zero
  // each can contain a value, though they must be identical if they do
  if (((bpb->sectors && bpb->sect_extnd) && (bpb->sectors != bpb->sect_extnd)) || (!bpb->sectors && !bpb->sect_extnd))
    return -1;
  // check that media descriptor is good
  if (memchr(media, bpb->descriptor, 11) == NULL)
    return -1;
  
  // check that bpb->sec_per_fat is > 0
  // FAT32: bpb->sec_per_fat can == 0 if fsVersion == 0 ????
  // The Compaq Diag (FAT compatible) partition has this as zero also
  //if (!bpb->sect_per_fat && bpb32->fs_version)
  //  return -1;

  // now check to makes sure that the devices block size == bpb->bytes_per_sec ?????
  
  // if we made it this far, we must be a FAT File System
  return DetectFatSize(bpb);
}

/* detects that FAT size of the fat fs.
 * returns FAT_12, FAT_16, or FAT_32
 * The issue with this code is that if the FAT partition was created correctly per the
 *  MS FAT specs, the simple check that are in that specification would be sufficient.
 *  However, the problem is, almost any sized media can be formatted to any one of the
 *  three fat types.  For example, a 1.44meg floppy can be formatted FAT12 or FAT16 without
 *  any errors.  However, if that floppy is formatted to FAT16, even though it would work
 *  just fine per the specification, it would not be a valid format.
 * The case is that even though that 1.44meg floppy is formatted FAT16, any Microsoft operating
 *  system would do the size check per their specification and would see the floppy as FAT12
 *  and read/write to it as FAT12 completely destroying any data that was on there before.
 * Therefore, this code does multiple checks not neccassarily in this order:
 *  1. checks the size per the MS specification.
 *  2. checks the type field in the BPB.
 *  3. checks the first two DWORDS in the FAT(s)
 *     if the value is: Fi FF FF xx xx xx xx xx = FAT12
 *     if the value is: Fi FF FF FF xx xx xx xx = FAT16
 *     if the value is: Fi FF FF 0F FF FF FF 0F = FAT32
 *     This check will only work for a FAT32 system.  The xx's in a fat16 or fat12 may be eof markers
 *      and be 0xFF's, where there would be no way of telling if it was a fat12 or fat16 fat table.
 */
unsigned CImageBar::DetectFatSize(struct S_FAT1216_BPB *bpb) {
  // We will test using each technique incrementing a corresponding value for each type.
  // At end of the tests, if only one field non zero, then we have a good size detection.
  unsigned fat12 = 0, fat16 = 0, fat32 = 0;
  
  // This is per Microsoft's calculations
  const unsigned root_dir_sects = (((bpb->root_entrys * 32) + (bpb->bytes_per_sect - 1)) / bpb->bytes_per_sect);
  const unsigned fat_size = (bpb->sect_per_fat != 0) ? bpb->sect_per_fat : ((struct S_FAT32_BPB *) bpb)->sect_per_fat32;
  const unsigned total_secs = (bpb->sectors != 0) ? bpb->sectors : bpb->sect_extnd;
  const unsigned data_sec = total_secs - (bpb->sect_reserved + (bpb->fats * fat_size) + root_dir_sects);
  const unsigned clusters = (data_sec / bpb->sect_per_clust);
  //unsigned nibbles_fat_entry = 0;
  if (clusters < 4085) {
    fat12++;
    //nibbles_fat_entry = 3;
  } else if (clusters < 65525) {
    fat16++;
    //nibbles_fat_entry = 4;
  } else {
    fat32++;
    //nibbles_fat_entry = 8;
  }
  
  // Due to compatibility with a DOS5 bug, you must also compute clusters_describable_by_fat and
  //  must use clusters_describable_by_fat if it is < clusters
  //const unsigned clusters_describable_by_fat = ((fat_size * bpb->bytes_per_sect * 2 ) / nibbles_fat_entry) - 2;
  //if (clusters_describable_by_fat < clusters)
  //  clusters = clusters_describable_by_fat;
  // **** However, unless we are going to save this value in the DPB, we have no need for it here ****
  
  // now let's check bpb->fs_type[8]
  if (memcmp((void *) bpb->sys_type, "FAT12   ", 8) == 0)
    fat12++;
  else if (memcmp((void *) bpb->sys_type, "FAT16   ", 8) == 0)
    fat16++;
  else if (memcmp(((struct S_FAT32_BPB *) bpb)->sys_type, "FAT32   ", 8) == 0)
    fat32++;
  
  // ;;;;;;;;;;;;;; ?????
  if ((fat12 > 0) && (memcmp((void *) bpb->sys_type, "FAT12   ", 8) != 0))
    fat12--;
  if ((fat16 > 0) && (memcmp((void *) bpb->sys_type, "FAT16   ", 8) != 0))
    fat16--;
  if ((fat32 > 0) && (memcmp(((struct S_FAT32_BPB *) bpb)->sys_type, "FAT32   ", 8) != 0))
    fat32--;

  /*
  // Load the first FAT of this partition and compare the first two dwords with known values of a fat32 type fat.
  bit32u *sector = (bit32u *) malloc(bpb->bytes_per_sect, "fs_detect_fat_size");
  if (dpb->dev->driver(dpb->dev, BLOCK_DEV_READ, dpb->base_lba + bpb->sect_reserved, 1, sector)) {
    if (((sector[0] & 0xFFFFFFF0) == 0x0FFFFFF0) && (sector[1] == 0x0FFFFFFF))
      fat32++;
  }
  mfree(sector);
  */
  
  // TODO: place other checks here.
  //printf(" ****** (%i) (%i) (%i)\n", fat12, fat16, fat32);
  
  if (fat12 && !fat16 && !fat32)
    return FS_FAT12;
  if (!fat12 && fat16 && !fat32)
    return FS_FAT16;
  if (!fat12 && !fat16 && fat32)
    return FS_FAT32;
  
  CString cs;
  cs.Format(" Found inconsistant FAT type while detecting FAT type determination.\r\n"
            "               (fat12 = %i) (fat16 = %i) (fat32 = %i)\r\n"
            "                         Assume FAT 12?\r\n"
            "               (Yes = Fat 12, No = Fat 16, or Cancel = 32)", fat12, fat16, fat32);
  int ret = AfxMessageBox(cs, MB_YESNOCANCEL, 0);
  if (ret == IDYES)
    return FS_FAT12;
  if (ret == IDNO)
    return FS_FAT16;
  return FS_FAT32;
}

int CImageBar::DetectExFat(void *buffer) {
  struct S_EXFAT_VBR *vbr = (struct S_EXFAT_VBR *) buffer;
  
  // first three bytes are 0xEB, 0x76, 0x90
  // TODO: does the second byte have to be 0x76?????
  if ((vbr->jmp[0] != 0xEB) || (vbr->jmp[1] != 0x76) || (vbr->jmp[2] != 0x90))
    return -1;
  
  // the must_be_zero field must be zeros
  //if (IsBufferEmpty(vbr->reserved0, 53))
  //  return -1;
  
  // fs version should be 1.00
  if (vbr->fs_version != 0x0100)
    return -1;
  
  // the log_bytes_per_sector + log_sects_per_track must be less than or equal to 25
  // and log_bytes_per_sector cannot be more than 12 (4096 bytes per sector)
  if ((vbr->log_bytes_per_sect > 12) || ((vbr->log_bytes_per_sect + vbr->log_sects_per_clust) > 25))
    return -1;
  
  // num fats must be 1 or 2
  if ((vbr->num_fats < 1) || (vbr->num_fats > 2))
    return -1;
  
  // percentage of heap should be 0 -> 100
  if (vbr->percent_heap > 100)
    return -1;
  
  // bit 3 of flags should be zero
  if (vbr->flags & (1<<3))
    return -1;
  
  // boot sig should be 0xAA55
  //if (vbr->boot_sig != 0xAA55)
  //  return -1;
  
  return FS_EXFAT;
}

int CImageBar::DetectExt2(void *buffer, const DWORD64 Size, const unsigned sect_size) {
  struct S_EXT2_SUPER *super = (struct S_EXT2_SUPER *) ((BYTE *) buffer + (2 * sect_size));
  
  // currently, the log block size can only be 0, 1, or 2 (1k, 2k, or 4k)
  if (super->log_block_size > 2)
    return -1;
  
  if ((super->blocks_count * (1024 << super->log_block_size) >> 9) > Size)
    return -1;
  
  if (super->magic != 0xEF53)
    return -1;
  
  // if the rev_level isn't 0 or 1, error
  if (super->rev_level > 1)
    return -1;
  
  return FS_EXT2;
}

int CImageBar::DetectLean(DWORD64 lba, const unsigned sect_size) {
  CLean Lean;

  Lean.m_lba = lba;

  return (Lean.DetectLeanFS()) ? FS_LEAN : -1;
}

int CImageBar::DetectNTFS(void *buffer) {
  struct S_NTFS_BPB *bpb = (struct S_NTFS_BPB *) buffer;
  
  DWORD crc, *p = (DWORD *) bpb;
  
  // check the jmp instruction
  if ((bpb->jmp[0] != 0xEB) || (bpb->jmp[2] != 0x90))
    return -1;
  
  // check the checksum
  crc = 0;
  for (int i=0; i<22; i++)
    crc += p[i];
  if (bpb->crc && (crc != bpb->crc))
    return -1;
  
  // check the oem for "NTFS    "
  if (memcmp(bpb->oem_name, "NTFS    ", 8) != 0)
    return -1;
  
  // check that bytes_per_sec is a power of 2 from 128 -> 4096
  if ((bpb->bytes_per_sect < 128) || (bpb->bytes_per_sect > 4096) || !power_of_two(bpb->bytes_per_sect))
    return -1;
  
  // check that sect_per_clust is a power of two with a range of 1 -> 128
  if ((bpb->sect_per_clust != 1) && (
    (bpb->sect_per_clust < 1) || (bpb->sect_per_clust > 128) || !power_of_two(bpb->sect_per_clust))
    ) return -1;
  
  // check that media descriptor is good
  if (memchr(media, bpb->descriptor, 11) == NULL)
    return -1;
  
  // Check the cluster size is not above 65536 bytes. 
  if ((bpb->bytes_per_sect * bpb->sect_per_clust) > 65536)
    return -1;
  
  // if clust_file_rec_size is 225->247 or 1, 2, 4, 8, 16, 32, or 64, then valid ntfs
  if (((bpb->clust_file_rec_size < 0xE1) || (bpb->clust_file_rec_size > 0xF7)) && (
    (bpb->clust_file_rec_size != 1) && (bpb->clust_file_rec_size != 2) && (bpb->clust_file_rec_size != 4) && 
    (bpb->clust_file_rec_size != 8) && (bpb->clust_file_rec_size != 16) && (bpb->clust_file_rec_size != 32) &&
    (bpb->clust_file_rec_size != 64)))
    return -1;
  
  // if clust_index_block is 225->247 or 1, 2, 4, 8, 16, 32, or 64, then valid ntfs
  if (((bpb->clust_index_block < 0xE1) || (bpb->clust_index_block > 0xF7)) && (
    (bpb->clust_index_block != 1) && (bpb->clust_index_block != 2) && (bpb->clust_index_block != 4) && 
    (bpb->clust_index_block != 8) && (bpb->clust_index_block != 16) && (bpb->clust_index_block != 32) &&
    (bpb->clust_index_block != 64)))
    return -1;
  
  // if reserved are zero, it is a NTFS
  if (memcmp(bpb->resv0, "\0\0\0", 3) || memcmp(bpb->resv5, "\0\0\0", 3) || memcmp(bpb->resv6, "\0\0\0", 3) ||
    bpb->resv1 || bpb->resv2 || bpb->resv3 || bpb->resv4)
    return -1;

  // other checks go here
  
  // if we made it this far, we must be a NTFS File System
  return FS_NTFS;
}

int CImageBar::DetectSFS(void *buffer, const unsigned sect_size) {
  struct S_SFS_SUPER *super = (struct S_SFS_SUPER *) ((BYTE *) buffer + SFS_SUPER_LOC);
  BYTE crc = 0;
  
  if ((super->magic_version & 0x00FFFFFF) != SFS_SUPER_MAGIC)
    return -1;
  
  if (((super->magic_version & 0xFF000000) >> 24) != 0x1A)  // 0x1A = v1.10
    return -1;
  
  // check the crc of bytes 0x1A6 -> 0x1B7
  for (int i = SFS_SUPER_LOC + 24; i<=SFS_SUPER_LOC + 41; i++)
    crc += ((BYTE *) buffer)[i];
  if (crc > 0)
    return -1;
  
  if ((super->block_size < 2) || (super->block_size > 5))     // x = 2 = (2 ^ (x + 7))
    return -1;
  
  return FS_SFS;
}

int CImageBar::DetectFYSFS(void *buffer, const unsigned sect_size) {
  struct S_FYSFS_SUPER *super = (struct S_FYSFS_SUPER *) ((BYTE *) buffer + (16 * sect_size));
  
  if ((super->sig[0] != 'FYSF') || (super->sig[1] != 'SUPR'))
    return -1;
  // check that sect_per_clust is a power of two with a range of 1 -> 512
  if ((super->sect_clust != 1) && (
    (super->sect_clust < 1) || (super->sect_clust > 512) || !power_of_two(super->sect_clust))
    ) return -1;
  // check that root entries >= 128 and < 65532 and a multiple of 4
  if ((super->root_entries < 128) || (super->root_entries > 65532) || (super->root_entries % 4))
    return -1;
  // TODO: make sure that all the reserved entries are actually zero.
  if (super->encryption > 1)
    return -1;
  // if the bitmap count is not 1 or 2
  if ((super->bitmaps < 1) || (super->bitmaps > 2))
    return -1;
  
  // here we have pretty much successfully detected a FYSFS FS.  
  // Let's check some values to make sure it is a valid FS
  if ((super->bitmap_flag & BITMAP_USE_SECOND) && (super->bitmaps != 2))
    AfxMessageBox(" Bitmap flag says to use second bitmap, but only one bitmap found...");
  if ((super->root < super->data) || (super->root > (super->data + super->data_sectors)))
    AfxMessageBox(" Root is not within the Data block...");
  
  // if the version of the FS is less than 1.50, we don't support it.
  if (super->ver < 0x0150)
    AfxMessageBox(" ***** Version of volume is less than 01.50. *****");
  
  // if we made it this far, we must be a FAT File System
  return FS_FYSFS;
}

int CImageBar::DetectFSZ(DWORD64 lba) {
  CFSZ fsz;
  fsz.m_lba = lba;

  return (fsz.DetectFSZ()) ? FS_FSZ : -1;
}
