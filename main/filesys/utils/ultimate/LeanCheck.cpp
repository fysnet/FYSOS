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

// LeanCheck.cpp : implementation file

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Lean.h"

#include "Modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned lcErrorCount, lcDirCount, lcFileCount, lcDelFileCount,
         lcCluster_cnt, lcVolCount;
bool lc_entry_error;
DWORD *lcClusters;
CString lcInfo;

void CLean::OnLeanCheck() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int j;
  unsigned i, pos;
  CString cs;
  BYTE *buffer;
  BYTE superblock[MAX_SECT_SIZE];
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) superblock;
  BOOL super_dirty = FALSE;
  
  // disable the button
  GetDlgItem(ID_CHECK)->EnableWindow(FALSE);

  // clear the globals 
  // (if we don't, we will add to what was shown last time)
  lcErrorCount = 0;
  lcCluster_cnt = 0;
  lcDirCount = 0;
  lcFileCount = 0;
  lcDelFileCount = 0;
  lcVolCount = 0;
  lc_entry_error = FALSE;
  lcInfo.Empty();
  
  CModeless modeless;
  modeless.m_Title = "Checking LEAN";
  modeless.m_modeless = TRUE;
  modeless.Create(CModeless::IDD, this);
  modeless.ShowWindow(SW_SHOW);
  modeless.BringWindowToTop();
  
  // Check 1: // Check items in the SuperBlock
  lcInfo += "Checking the SuperBlock\r\n";

  // read in the superblock (we have to have at least a minimum
  //  compliant super block and m_super_lba value or we won't get
  //  to the point where we can click on the "CHECK" button.
  // therefore, we can rely on the fact that m_super_lba should be correct.
  // *** most of these checks are redundant.  We can't get to here ***
  // *** without these values being correct in the first place.    ***
  dlg->ReadFromFile(super, m_lba + m_super_lba, 1);
  
  // How about the check sum
  DWORD crc = 0, *p = (DWORD *) super;
  for (i=1; i<dlg->m_sect_size / sizeof(DWORD); i++)
    crc = (crc << 31) + (crc >> 1) + p[i];
  if (crc != super->checksum) {
    cs.Format("CRC is not correct.  Should be 0x%08X, is 0x%08X\r\n", crc, super->checksum);
    lcInfo += cs;
  }
  
  // the first entry should be 'LEAN'
  if (super->magic != LEAN_SUPER_MAGIC) {
    cs.Format("Magic value is not 0x%08X (0x%08X)\r\n", LEAN_SUPER_MAGIC, super->magic);
    lcInfo += cs;
  }
  
  // must be at least version 0.6
  if (super->fs_version < 0x0006) {
    cs.Format("Version must be at least 0.6. (0x%08X)\r\n", super->fs_version);
    lcInfo += cs;
  }
  
  // the preallocate field should be in the range of 0 to ?
  //if (super->pre_alloc_count > ?) {
  //  cs.Format("Preallocate count should be at least ?.  Is %i.\r\n", super->pre_alloc_count);
  //  lcInfo += cs;
  //}
  
  // the log2 entry should be at least 12 and not more than 31
  if ((super->log_sectors_per_band < 12) || (super->log_sectors_per_band > 31)) {
    cs.Format("Log Sectors Per Band must be at least 12 and no more than 31.  Is %i.\r\n", super->log_sectors_per_band);
    lcInfo += cs;
  }
  
  // all but bits 1:0 of state should be zero
  if (super->state & ~0x3) {
    cs.Format("State should have all high bits clear. (0x%08X)\r\n", super->state);
    lcInfo += cs;
  }
  
  // error if GUID is zero?
  if (IsBufferEmpty(&super->guid, 16)) {
    lcInfo += "Super GUID should not be zeros...\r\n";
    lcErrorCount++;
  }

  // error if label is empty?
  if (IsBufferEmpty(super->volume_label, 64)) {
    lcInfo += "Volume Label should not be zeros...\r\n";
    lcErrorCount++;
  }

  // is the total sectors count within range?
  if (super->sector_count != m_size) {
    cs.Format("Sector Count is %li, should be %li.\r\n", super->sector_count, m_size);
    lcInfo += cs;
  }
  
  // check the primarySuper field.  It should == m_super_lba
  if (super->primary_super != (DWORD64) m_super_lba) {
    cs.Format("Super Location is %li, should be %li.\r\n", super->primary_super, m_super_lba);
    lcInfo += cs;
  }
  
  // check if the sect size is 0, 1, 2, 3, 4 (etc) up to ???
  // TODO:
  
  // check if reserved is not zeros???
  if (!IsBufferEmpty(super->reserved, 351)) {
    lcInfo += "Reserved field should be zeros...\r\n";
    lcErrorCount++;
  }
  
  ////////////////////////////////////////////////////////////////////
  // Check 2: get free sector count from band bitmaps.
  lcInfo += "Checking the Bitmaps\r\n";
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_sectors_per_band); // sectors per band
  const unsigned bitmap_size = (unsigned) (band_size / dlg->m_sect_size / 8); // sectors per bitmap
  const unsigned bytes_bitmap = bitmap_size * dlg->m_sect_size;
  const unsigned tot_bands = (unsigned) ((m_super.sector_count + (band_size - 1)) / band_size);
  buffer = (BYTE *) malloc(bitmap_size * dlg->m_sect_size);
  DWORD64 bitmap_lba, FreeCount = 0;

  //CString css;
  //css.Format(" %i %i %i %i", (DWORD) band_size, bitmap_size, bytes_bitmap, tot_bands);
  //AfxMessageBox(css);
  
  for (i=0; i<tot_bands; i++) {
    // read in a bitmap
    bitmap_lba = (i==0) ? m_super.bitmap_start : (band_size * i);
    dlg->ReadFromFile(buffer, m_lba + bitmap_lba, bitmap_size);
    pos = 0;
    
    while (pos < bytes_bitmap) {
      if (buffer[pos] == 0)
        FreeCount += 8;
      else {
        for (j=0; j<8; j++) {
          if ((buffer[pos] & (1<<j)) == 0)
            FreeCount++;
        }
      }
      pos++;
    }
  }
  
  if (FreeCount != super->free_sector_count) {
    cs.Format("Found %li free sectors.  Super states %li.\r\n", FreeCount, super->free_sector_count);
    // ask to update super.  Remember to update m_super.free_sectors member
    //if (AfxMessageBox("FreeSector Count doesn't match.\r\nUpdate Super->FreeSectorCount field?", MB_YESNO, 0) == IDYES) {
    //  super->free_sector_count = FreeCount;
    //  super->CRC = 
    //  super_dirty = TRUE;
    //  m_free_sectors.Format("%I64i", FreeCount);
    //  UpdateData(FALSE); // send to Dialog
    //}
  } else
    cs.Format("Found %li free sectors.\r\n", FreeCount);
  lcInfo += cs;
  
  free(buffer);
  
  ////////////////////////////////////////////////////////////////////
  // Check 3: 
  //fcInfo += "Checking the FAT(s)\r\n";
  //modeless.SetDlgItemText(IDC_EDIT, fcInfo);
  //modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // Check 4: // check the directory entries
  //lcInfo += "Checking Root/Sub directory(s)\r\n";
  //modeless.SetDlgItemText(IDC_EDIT, lcInfo);
  //modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  /*
  struct S_FAT_ROOT *root = NULL;
  DWORD rootsize, rootcluster, cluster_cnt = 0;
  if (m_fat_size == FS_FAT32) {
    rootcluster = bpb32->root_base_cluster;
    if (rootcluster > 0)  // make sure there is actually a root to read
      root = (struct S_FAT_ROOT *) ReadFile(rootcluster, &rootsize, TRUE);
  } else {
    rootcluster = 0;
    rootsize = bpb12->root_entrys * sizeof(struct S_FAT_ROOT);
    root = (struct S_FAT_ROOT *) ReadFile(rootcluster, &rootsize, TRUE);
  }
  if (root) {
    // allocate a list of starting clusters, so that we can keep
    //  track of all clusters we check.  This way we don't loop
    //  indefinately if a root entry points to itself, or a parent.
    // we need to limit this to 65536 simply because we chose this value...
    fcClusters = (DWORD *) calloc(65536, sizeof(DWORD));
    fcClusters[fcCluster_cnt++] = rootcluster;
    fcDirCount = 1;
    FatCheckRoot(modeless, root, rootsize / sizeof(struct S_FAT_ROOT), "\\");
    free(fcClusters);
    free(root);
  }

  // show results
  cs.Format("Checked %u directory(s), %u file(s), %u deleted files, %u Volume Label(s).\r\n", fcDirCount, fcFileCount, fcDelFileCount, fcVolCount);
  fcInfo += cs;
  modeless.SetDlgItemText(IDC_EDIT, fcInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
  */

  // Check 5: // ?????
  
  
  
  
  
  // write back the super?
  if (super_dirty)
    dlg->WriteToFile(super, m_lba + m_super_lba, 1);
  
  cs.Format("\r\n  Found %i errors\r\n", lcErrorCount);
  lcInfo += cs;
  lcInfo += "\r\nUse the COPY button to copy all to the clipboard or the DONE button to exit\r\n";
  
  // now "copy" the modeless to a modal and
  //  destroy the modeless and display the modal
  CModeless modal;
  modal.m_edit = lcInfo;
  modal.m_Title = modeless.m_Title;
  modeless.DestroyWindow();
  modal.m_modeless = FALSE;
  modal.DoModal();
  
  // "free" the memory used by the CString
  lcInfo.Empty();
  
  // re-enable the button
  GetDlgItem(ID_CHECK)->EnableWindow(TRUE);
}

/*
void CFat::FatCheckRoot(CModeless &modeless, struct S_FAT_ROOT *root, const unsigned entries, CString csPath) {
  struct S_FAT_ROOT *sub;
  unsigned cnt, i = 0;
  DWORD start, filesize = 0, ErrorCode;
  BOOL IsDot = FALSE;
  CString cs, name;
  BYTE attrb;

  // print current path
  fcInfo += csPath;
  fcInfo += "\r\n";
  
  // if not the root, check that the first two entries are '.' and '..'
  if (csPath != "\\") {
    FatGetName(&root[0], name, &attrb, &start, &filesize, NULL);
    if (name != ".") {
      fcInfo += "First entry is not DOT entry\r\n";
      fcErrorCount++;
    }
    FatGetName(&root[1], name, &attrb, &start, &filesize, NULL);
    if (name != "..") {
      fcInfo += "Second entry is not DOT DOT entry\r\n";
      fcErrorCount++;
    }
  }
  
  // show the progress
  modeless.SetDlgItemText(IDC_EDIT, fcInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // parse the entries
  while (i<entries) {
    cs.Empty();
    if (root[i].name[0] == 0x00)
      break;
    ErrorCode = CheckRootEntry(&root[i]);
    if (ErrorCode == FAT_BAD_LFN_DEL) {
      cnt = FatGetName(&root[i], name, &attrb, &start, &filesize, NULL) - 1; // -1 so we display the deleted SFN
      cs.Format("Found Deleted Long File Name entry: %s (index %i)\r\n", csPath + name, i);
    } else if (ErrorCode != FAT_NO_ERROR) {
      cnt = 1;
      fcErrorCount++;
      cs.Format("Found Bad entry with error code %u* (index %i)\r\n", ErrorCode, i);
      if (!fc_entry_error) {
        fcInfo += cs;
        cs = "* Error Codes:\r\n"
             "   1 = Bad Attribute Value\r\n"
             "   2 = Invalid Char in SFN\r\n"
             "   3 = SFN Reserved section is non-zero\r\n"
             "   4 = bad sequence number found in LFN\r\n"
             "   5 = Deleted Entry\r\n"
             "   6 = Bad CRC of SFN found in LFN\r\n"
             "   7 = Found Invalid char in LFN\r\n";
        fc_entry_error = TRUE;
      }
    } else {
      // retrieve the name.
      cnt = FatGetName(&root[i], name, &attrb, &start, &filesize, &IsDot);
      if (root[i].name[0] == FAT_DELETED_CHAR) {
        fcDelFileCount++;
        cs.Format("Found Deleted entry: %s%s (index %i)\r\n", csPath, name, i);
      } else {
        if (attrb & FAT_ATTR_SUB_DIR) {
          if (!IsDot) {
            // check to see if we already parsed this cluster's directory
            if (CheckForRecursion(start)) {
              // found that we already did this directory....
              cs.Format("Directory with Cluster %u already checked.\r\n"
                        "Directory points to infinate loop...\r\n", start);
              fcErrorCount++;
            } else {
              sub = (struct S_FAT_ROOT *) ReadFile(start, &filesize, FALSE);
              if (sub) {
                FatCheckRoot(modeless, sub, (filesize + 31) / 32, csPath + name + "\\");
                free(sub);
                fcDirCount++;
              }
            }
          }
        } else if (attrb & FAT_ATTR_VOLUME) {
          // TODO: Check that label has only allowed chars????
          cs.Format("Volume Label: %s\r\n", name);
          fcVolCount++;
        } else {
          // found regular file
          fcFileCount++;
          cs.Format("%s%s\r\n", csPath, name);
        }
      }
    }
    
    // update display?
    if (cs.GetLength() > 0) {
      fcInfo += cs;
      modeless.SetDlgItemText(IDC_EDIT, fcInfo);
      modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
    }

    i += cnt;
  }
}

// check to see if the passed cluster is in our list
//  return TRUE if it is
bool CFat::CheckForRecursion(DWORD cluster) {
  unsigned i;

  for (i=0; i < fcCluster_cnt; i++)
    if (fcClusters[i] == cluster)
      return TRUE;

  return FALSE;
}
*/
