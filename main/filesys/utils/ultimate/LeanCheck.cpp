/*
 *                             Copyright (c) 1984-2021
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
  int j;
  unsigned i, pos;
  CString cs;
  BYTE *buffer;
  BYTE *superblock = (BYTE *) malloc(m_block_size);
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
  //  compliant super block and m_super_block_loc value or we won't get
  //  to the point where we can click on the "CHECK" button.
  // therefore, we can rely on the fact that m_super_block_loc should be correct.
  // *** most of these checks are redundant.  We can't get to here ***
  // *** without these values being correct in the first place.    ***
  LeanReadBlocks(super, m_super_block_loc, 1);
  
  // How about the check sum
  DWORD crc = 0, *p = (DWORD *) super;
  for (i=1; i<m_block_size / sizeof(DWORD); i++)
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
  
  // must be version 0.7
  if (super->fs_version < 0x0007) {
    cs.Format("Version must be version 0.7. (0x%08X)\r\n", super->fs_version);
    lcInfo += cs;
  }
  
  // the preallocate field should be in the range of 0 to ?
  //if (super->pre_alloc_count > ?) {
  //  cs.Format("Preallocate count should be at least ?.  Is %i.\r\n", super->pre_alloc_count);
  //  lcInfo += cs;
  //}
  
  // the log2 entry should be at least 12 and not more than 31
  if ((super->log_blocks_per_band < 12) || (super->log_blocks_per_band > 31)) {
    cs.Format("Log Blocks Per Band must be at least 12 and no more than 31.  Is %i.\r\n", super->log_blocks_per_band);
    lcInfo += cs;
  }
  if ((super->log_blocks_per_band - 3) < super->log_block_size) {
    cs.Format("Log Blocks Per Band must be at least %i.  Is %i.\r\n", super->log_block_size + 3, super->log_blocks_per_band);
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

  // is the total blocks count within range?
  if (super->block_count != m_tot_blocks) {
    cs.Format("Block Count is %li, should be %li.\r\n", super->block_count, m_tot_blocks);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // check the primarySuper field.  It should == m_super_block_loc
  if (super->primary_super != (DWORD64) m_super_block_loc) {
    cs.Format("Super Location is %li, should be %li.\r\n", super->primary_super, m_super_block_loc);
    lcInfo += cs;
    lcErrorCount++;
  }

  // the bitmapStart field should be no less than m_super_block_loc 
  if (super->bitmap_start <= m_super_block_loc) {
    cs.Format("Bitmap Start Location is %li, should be at least %li.\r\n", super->bitmap_start, m_super_block_loc + 1);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // the rootInode field should be no less than m_super_block_loc
  if (super->bitmap_start <= m_super_block_loc) {
    cs.Format("Root Start Location is %li, should be at least %li.\r\n", super->root_start, m_super_block_loc + 1);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // check if reserved is not zeros???
  // the size depends on log_block_size
  //if (!IsBufferEmpty(super->reserved, 351)) {
  //  lcInfo += "Reserved field should be zeros...\r\n";
  //  lcErrorCount++;
  //}
  
  ////////////////////////////////////////////////////////////////////
  // Check 2: get free block count from band bitmaps.
  lcInfo += "Checking the Bitmaps\r\n";
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8); // blocks per bitmap
  const unsigned bytes_bitmap = bitmap_size * m_block_size;
  const unsigned tot_bands = (unsigned) ((m_super.block_count + (band_size - 1)) / band_size);
  buffer = (BYTE *) malloc(bitmap_size * m_block_size);
  DWORD64 bitmap_block, FreeCount = 0;
  DWORD64 total_count = m_tot_blocks;  // only count up to total_blocks.  Don't count anything after that.

  for (i=0; i<tot_bands; i++) {
    // read in a bitmap
    bitmap_block = (i==0) ? m_super.bitmap_start : (band_size * i);
    LeanReadBlocks(buffer, bitmap_block, bitmap_size);
    pos = 0;
    
    while ((pos < bytes_bitmap) && (total_count > 0)) {
      if ((total_count > 8) && (buffer[pos] == 0)) {
        FreeCount += 8;
        total_count -= 8;
      } else {
        for (j=0; (j<8 && (total_count > 0)); j++) {
          if ((buffer[pos] & (1<<j)) == 0)
            FreeCount++;
          total_count--;
        }
      }
      pos++;
    }
  }
  
  if (FreeCount != super->free_block_count) {
    cs.Format("Found %li free blocks.  Super states %li.\r\n", FreeCount, super->free_block_count);
    // ask to update super.  Remember to update m_super.free_sectors member
    //if (AfxMessageBox("Freeblock Count doesn't match.\r\nUpdate Super->FreeblockCount field?", MB_YESNO, 0) == IDYES) {
    //  super->free_block_count = FreeCount;
    //  super->CRC = 
    //  super_dirty = TRUE;
    //  m_free_blocks.Format("%I64i", FreeCount);
    //  UpdateData(FALSE); // send to Dialog
    //}
    lcErrorCount++;
  } else
    cs.Format("Found %li free blocks.\r\n", FreeCount);
  lcInfo += cs;
  
  free(buffer);
  
  ////////////////////////////////////////////////////////////////////
  // Check 3: 
  //   TODO:
  //
  
  
  //  when transversing through the files, keep an array of Inodes and their link counts.
  //  when an inode references an inode in this array, update the link count.
  //  then transverse through the files again, checking against the inode->link_count field.
  
  
  
  // write back the super?
  if (super_dirty)
    LeanWriteBlocks(super, m_super_block_loc, 1);
  
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
