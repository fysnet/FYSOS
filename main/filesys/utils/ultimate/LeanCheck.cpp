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

int lcErrorCount, lcDiagCount, lcDirCount, lcFileCount;
CString lcInfo;

// for the link count stuff
struct CLeanCheckInodes {
  DWORD64 inode;
  int count;
  int reserved;  // to align next entry on 64-bit address
};
struct CLeanCheckInodes *lcInodes;
unsigned lcInodeArraySize, lcInodeCount;


void CLean::OnLeanCheck() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  int j;
  unsigned i, pos;
  CString cs;
  BYTE *superblock = (BYTE *) malloc(m_block_size);
  struct S_LEAN_SUPER *super = (struct S_LEAN_SUPER *) superblock;
  BOOL super_dirty = FALSE;
  
  // disable the button
  GetDlgItem(ID_CHECK)->EnableWindow(FALSE);

  // clear the globals 
  // (if we don't, we will add to what was shown last time)
  lcErrorCount = 0;
  lcDiagCount = 0;
  lcDirCount = 1;  // first will be the root
  lcFileCount = 0;
  lcInfo.Empty();
  lcInodes = NULL;
  lcInodeArraySize = 0;
  lcInodeCount = 0;
  
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
  // *** most of these checks are redundant.  We can't get to here
  //     without these values being correct in the first place.    ***
  dlg->ReadBlocks(superblock, m_lba, m_super_block_loc, m_block_size, 1);
  
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
  
  // must be version 1.0
  if (super->fs_version < 0x0100) {
    cs.Format("Version must be version 1.0. (0x%08X)\r\n", super->fs_version);
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
    lcDiagCount++;
  }
  // the Unmounted bit in the state field should be set since we are unmounted.
  if (super->state != 0x01) {
    cs.Format("State should be 0x00000001 (No error and Unmounted). (0x%08X)\r\n", super->state);
    lcInfo += cs;
    lcDiagCount++;
  }
  
  // diagnostic error if GUID is zero?
  if (IsBufferEmpty(&super->guid, 16)) {
    lcInfo += "Super GUID should not be zeros...\r\n";
    lcDiagCount++;
  }

  // diagnostic error if label is empty?
  if (IsBufferEmpty(super->volume_label, 64)) {
    lcInfo += "Volume Label should not be zeros...\r\n";
    lcDiagCount++;
  }

  // is the total blocks count within range?
  if (super->block_count != m_tot_blocks) {
    cs.Format("Block Count is %I64i, should be %I64i.\r\n", super->block_count, m_tot_blocks);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // check the primarySuper field.  It should == m_super_block_loc
  if (super->primary_super != m_super_block_loc) {
    cs.Format("Super Location is %I64i, should be %I64i.\r\n", super->primary_super, m_super_block_loc);
    lcInfo += cs;
    lcErrorCount++;
  }

  // the bitmapStart field should be no less than m_super_block_loc 
  if (super->bitmap_start <= m_super_block_loc) {
    cs.Format("Bitmap Start Location is %I64i, should be at least %I64i.\r\n", super->bitmap_start, m_super_block_loc + 1);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // the rootInode field should be no less than m_super_block_loc
  if (super->bitmap_start <= m_super_block_loc) {
    cs.Format("Root Start Location is %I64i, should be at least %I64i.\r\n", super->root_start, m_super_block_loc + 1);
    lcInfo += cs;
    lcErrorCount++;
  }

  // the capabilities register
  if (super->capabilities & ~1) {
    lcInfo += "Reserved bits in the capabilities member must be zeros...\r\n";
    lcErrorCount++;
  }
  
  // check if reserved is not zeros
  if (!IsBufferEmpty((BYTE *) super + sizeof(struct S_LEAN_SUPER), (int) ((size_t) m_super_block_loc - sizeof(struct S_LEAN_SUPER)))) {
    lcInfo += "Reserved field should be zeros...\r\n";
    lcDiagCount++;
  }
  
  // check the crc of the bitmap
  if (super->bitmap_checksum != BitmapChecksum(super)) {
    lcInfo += "Super Bitmap Checksum does not match actual...\r\n";
    lcErrorCount++;
  }

  ////////////////////////////////////////////////////////////////////
  // Check 2: get free block count from band bitmaps.
  lcInfo += "Checking the Bitmaps\r\n";
  const DWORD64 band_size = ((DWORD64) 1 << super->log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8); // blocks per bitmap
  const unsigned bytes_bitmap = bitmap_size * m_block_size;
  const unsigned tot_bands = (unsigned) ((super->block_count + (band_size - 1)) / band_size);
  BYTE *buffer = (BYTE *) malloc(bitmap_size * m_block_size);
  DWORD64 bitmap_block, FreeCount = 0;
  DWORD64 total_count = m_tot_blocks;  // only count up to total_blocks.  Don't count anything after that.

  for (i=0; i<tot_bands; i++) {
    // read in a bitmap
    bitmap_block = (i==0) ? super->bitmap_start : (band_size * i);
    dlg->ReadBlocks(buffer, m_lba, bitmap_block, m_block_size, bitmap_size);
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
    cs.Format("Error: Found %I64i free blocks.  Super states %I64i.\r\n", FreeCount, super->free_block_count);
    lcErrorCount++;
  } else
    cs.Format("Found %I64i free blocks.\r\n", FreeCount);
  lcInfo += cs;
  
  ////////////////////////////////////////////////////////////////////
  // Check 3: 
  // before we free the buffer, and since it will be plenty large enough to hold a single block,
  //  let's see if the backup superblock is valid and matches the first.
  struct S_LEAN_SUPER *backup = (struct S_LEAN_SUPER *) buffer;
  dlg->ReadBlocks(backup, m_lba, super->backup_super, m_block_size, 1);
  crc = 0; p = (DWORD *) backup;
  for (i=1; i<m_block_size / sizeof(DWORD); i++)
    crc = (crc << 31) + (crc >> 1) + p[i];
  if ((crc == backup->checksum) && (backup->magic == LEAN_SUPER_MAGIC)) {
    cs.Format("Found Backup Superblock at %I64i.\r\n", super->backup_super);
    lcInfo += cs;
    // now see if the backup matches the primary
    if (memcmp(super, backup, m_block_size) != 0) {
      lcInfo += "Error: Backup Superblock does not match Primary Superblock.\r\n";
      lcErrorCount++;
    }
  } else {
    cs.Format("Did not find valid Backup Superblock at %I64i.\r\n", super->backup_super);
    lcInfo += cs;
    lcErrorCount++;
  }
  
  // free the bitmap/backup buffer
  free(buffer);
  
  ////////////////////////////////////////////////////////////////////
  // Check 4: 
  //   Check all of the inodes in the system:
  
  // check the root's Inode and if it is good, parse the directory(s)
  lcInfo += "Checking the Root Inode\r\n";
  if (LeanCheckInode(super->root_start, TRUE, FALSE) == 0) {
    lcInfo += "Checking Inodes\r\n";
    struct S_LEAN_DIRENTRY *root;
    DWORD64 root_size = 0;
    root = (struct S_LEAN_DIRENTRY *) ReadFile(super->root_start, &root_size);
    if (root) {
      LeanCheckDir(root, root_size, "/");
      free(root);
    }
  }

  ////////////////////////////////////////////////////////////////////
  // Check 5: 
  // we now have a list of all the inodes used in the system
  // read back these inodes and check to see that their link count matches what we found
  lcInfo += "\r\nChecking Link Counts\r\n";
  LeanCheckLinkCount();
  if (lcInodes != NULL)
    free(lcInodes);

  // display count of files and directorys found
  cs.Format("\r\n  Found %i directories with %i files...\r\n", lcDirCount, lcFileCount);
  lcInfo += cs;
  
  // write back the super?
  if (super_dirty)
    dlg->WriteBlocks(super, m_lba, m_super_block_loc, m_block_size, 1);
  
  cs.Format("\r\n  Found %i errors\r\n"
                "  Found %i diagnostics\r\n", lcErrorCount, lcDiagCount);
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
  free(superblock);
  
  // re-enable the button
  GetDlgItem(ID_CHECK)->EnableWindow(TRUE);
}

BOOL CLean::LeanCheckDir(struct S_LEAN_DIRENTRY *root, DWORD64 root_size, CString path) {
  struct S_LEAN_DIRENTRY *cur = root, *sub;
  DWORD64 filesize;
  CString cs, name;
  BOOL IsDot, err = FALSE;
  
  while ((((unsigned char *) cur < ((unsigned char *) root + root_size))) && !err) {
    if (cur->rec_len == 0)
      break;
    
    // retrieve the name.
    GetName(cur->name, name, cur->name_len);
    
    IsDot = ((name == ".") || (name == ".."));
    
    switch (cur->type & 0x7) {
      case LEAN_FT_DIR:  // File type: directory
        lcErrorCount += LeanCheckInode(cur->inode, TRUE, TRUE);
        if (!IsDot) {
          sub = (struct S_LEAN_DIRENTRY *) ReadFile(cur->inode, &filesize);
          if (sub) {
            cs.Format("%s%s (LEAN_FT_DIR)\r\n", (LPCTSTR) path, (LPCTSTR) name);
            lcInfo += cs;
            cs.Format("%s%s/", (LPCTSTR) path, (LPCTSTR) name);
            LeanCheckDir(sub, filesize, cs);
            free(sub);
            lcDirCount++;
          }
        }
        break;
      case LEAN_FT_REG: // File type: regular file
        cs.Format("%s%s (LEAN_FT_REG)\r\n", (LPCTSTR) path, (LPCTSTR) name);
        lcInfo += cs;
        lcErrorCount += LeanCheckInode(cur->inode, TRUE, TRUE);
        lcFileCount++;
        break;
      case LEAN_FT_LNK: // File type: symbolic link
        cs.Format("%s%s (LEAN_FT_LNK)\r\n", (LPCTSTR) path, (LPCTSTR) name);
        lcInfo += cs;
        lcErrorCount += LeanCheckInode(cur->inode, TRUE, TRUE);
        lcFileCount++;
        break;
      // should not find a Fork in a directory listing
      //case LEAN_FT_FRK: // File type: fork
      //  LeanCheckInode(cur->inode, FALSE, TRUE);
      //  break;
      case LEAN_FT_DELETED: // File type: deleted for undelete
        cs.Format("%s%s (LEAN_FT_DELETED)\r\n", (LPCTSTR) path, (LPCTSTR) name);
        lcInfo += cs;
        lcErrorCount += LeanCheckInode(cur->inode, TRUE, TRUE);
        lcFileCount++;
        break;
      case LEAN_FT_MT:  // File type: Empty
        break;
      default:
        cs.Format("Unknown directory entry type found: %i\r\nStopping...", cur->type);
        lcInfo += cs;
        lcFileCount++;
        err = TRUE;  // break out of loop
    }
    
    cur = (struct S_LEAN_DIRENTRY *) ((BYTE *) cur + (cur->rec_len * 16));
  }

  return TRUE;
}

// returns TRUE if a valid Inode, returns FALSE if there was an error
int CLean::LeanCheckInode(DWORD64 block, const BOOL allow_fork, const BOOL add_to_list) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) buffer;
  int i, cnt, errors = 0;
  CString cs;
  DWORD dword;
  DWORD64 qword = 0, blocks_used = 0, blocks_needed;

  // read in the inode
  dlg->ReadBlocks(inode, m_lba, block, m_block_size, 1);

  // check to see if inode->magic = 'NODE';
  if (inode->magic != LEAN_INODE_MAGIC) {
    cs.Format("Inode %I64i: Magic value is not 0x%08X (0x%08X)\r\n", block, LEAN_INODE_MAGIC, inode->magic);
    lcInfo += cs;
    errors++;
  }
  
  // check to see if crc is correct
  dword = LeanCalcCRC(inode, LEAN_INODE_SIZE, TRUE);
  if (inode->checksum != dword) {
    cs.Format("Inode %I64i: CRC value is not 0x%08X (0x%08X)\r\n", block, dword, inode->checksum);
    lcInfo += cs;
    errors++;
  }

  // if either of the above is in error, we don't continue
  if (errors > 0) {
    free(buffer);
    return errors;
  }
  
  // add it to our inode array
  if (add_to_list)
    LeanCheckAddInode(block);
  
  // the extent count must be greater than zero and less than or equal to LEAN_INODE_EXTENT_CNT
  const int max_extents = (m_use_extended_extents) ? LEAN_INODE_EXTENT_CNT_EXT : LEAN_INODE_EXTENT_CNT;
  if ((inode->extent_count == 0) || (inode->extent_count > max_extents)) {
    cs.Format("Inode %I64i: Illegal Extent Count value %i\r\n", block, inode->extent_count);
    lcInfo += cs;
    errors++;
  }
  
  // No Error: Check that the reserved field is zero
  if (inode->reserved[0] || inode->reserved[1] || inode->reserved[2]) {
    cs.Format("Inode %I64i: reserved[3] member not zero\r\n", block);
    lcInfo += cs;
  }

  // No Error: The uid and gid fields shoule be zero
  if (inode->uid || inode->gid) {
    cs.Format("Inode %I64i: The uod or gid member(s) are not zero\r\n", block);
    lcInfo += cs;
  }
  
  // if any bit from 20 to 28 in the attributes member is non-zero, give error
  if ((inode->attributes & (0x1FF << 20)) || (inode->attributes & (1 << 12))) {
    cs.Format("Inode %I64i: Reserved bits in attribute are non-zero (0x%08X)\r\n", block, inode->attributes);
    lcInfo += cs;
    lcDirCount++;
  }

  // if any of the dates are zero, give error
  if (!inode->acc_time || !inode->sch_time || !inode->mod_time || !inode->cre_time) {
    cs.Format("Inode %I64i: One or more time stamps are zero\r\n", block);
    lcInfo += cs;
    errors++;
  }

  // check the Indirects?
  if (inode->first_indirect > 0) {
    // if the last indirect value is zero, we are in error
    if (inode->last_indirect == 0) {
      cs.Format("Inode %I64i: First Indirect is non-zero but Last Indirect is zero\r\n", block);
      lcInfo += cs;
      errors++;
    } else {
      errors += LeanCheckIndirect(block, inode->first_indirect, &dword, &qword);
      // does the found count of indirects match the inode->indirectCount field?
      if (inode->indirect_count != dword) {
        cs.Format("Inode %I64i: Indirect Count (%i) doesn't match Inode member of %i\r\n", block, dword, inode->indirect_count);
        lcInfo += cs;
        errors++;
      }
    }
  }
  
  // check that all blocks in the extent(s) are marked used
  cnt = LeanCheckExtents(&inode->extent_start[0], (unsigned int) inode->extent_count, max_extents, TRUE);
  if (cnt > 0) {
    cs.Format("Inode %I64i: Found %i used blocks marked free in bitmap(s)\r\n", block, cnt);
    lcInfo += cs;
    errors++;
  }

  //  Check that the actual count of blocks used (from direct extents and indirect extents) match inode->block_count
  DWORD32 *extent_size = (DWORD32 *) ((BYTE *) &inode->extent_start[0] + (max_extents * sizeof(DWORD64)));
  for (i=0; (i<max_extents) && (i<inode->extent_count); i++)
    blocks_used += extent_size[i];
  if (inode->block_count != (blocks_used + qword)) {
    cs.Format("Inode %I64i: Blocks Used Count (%I64i) doesn't match Inode (%I64i)\r\n", block, blocks_used + qword, inode->block_count);
    lcInfo += cs;
    errors++;
  }

  //  Check that the the inode->file_size field doesn't exceed the block count used
  if (inode->file_size > ((blocks_used + qword) * m_block_size)) {
    cs.Format("Inode %I64i: File size (%I64i) is larger than allocated blocks (%I64i) for inode\r\n", block, inode->file_size, ((blocks_used + qword) * m_block_size));
    lcInfo += cs;
    errors++;
  }

  //  Check that if the iaPrealloc flag is clear, did the driver dump the preallocated blocks off the end?
  blocks_needed = (((inode->attributes & LEAN_ATTR_EAS_IN_INODE) ? m_block_size : LEAN_INODE_SIZE) + inode->file_size + (m_block_size - 1)) / m_block_size;
  if (!(inode->attributes & LEAN_ATTR_PREALLOC) && (blocks_needed < blocks_used)) {
    cs.Format("Diag: Attributes::iaPrealloc == 0, but file has %i extra blocks appended to it.\r\n", (int) (blocks_used - blocks_needed));
    lcInfo += cs;
    lcDiagCount++;
  }
  
  //  Check the fork as a valid inode (must not contain a fork itself)
  if (allow_fork && inode->fork)
    errors += LeanCheckInode(inode->fork, FALSE, TRUE);
  if (!allow_fork && inode->fork) {
    cs.Format("Inode %I64i: Fork has fork\r\n", block);
    lcInfo += cs;
    errors++;
  }
  // a fork must not have extended attributes just after the inode
  if (!allow_fork && (inode->attributes & LEAN_ATTR_EAS_IN_INODE)) {
    cs.Format("Inode %I64i: Fork has LEAN_ATTR_EAS_IN_INODE set.\r\n", block);
    lcInfo += cs;
    errors++;
  }

  // free the buffer used
  free(buffer);

  return errors;
}

// returns TRUE if a valid Indirect, returns FALSE if there was an error
// check all indirect blocks until 'next' is zero or we find an invalid entry
int CLean::LeanCheckIndirect(DWORD64 inode_num, DWORD64 block, DWORD *ret_count, DWORD64 *blocks_used) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INDIRECT *indirect = (struct S_LEAN_INDIRECT *) buffer;
  int cnt, errors = 0;
  CString cs;
  DWORD dword, return_count = 0;
  DWORD64 prev = 0, block_count = 0;
  const unsigned extent_size = (m_use_extended_extents) ? 16 : 12;
  int extent_max = (m_block_size - LEAN_INDIRECT_SIZE) / extent_size;  // max count of extents an indirect can have
  int m_reserved2_size = m_block_size - LEAN_INDIRECT_SIZE - (extent_max * extent_size); // a count (if any) of bytes after the last indirect (a block size of 4096 will have a few)
  
  while ((block != 0) && (errors == 0)) {
    // read in the indirect
    dlg->ReadBlocks(indirect, m_lba, block, m_block_size, 1);

    // check to see if indirect->magic = 'INDX';
    if (indirect->magic != LEAN_INDIRECT_MAGIC) {
      cs.Format("Indirect %I64i: Magic value is not 0x%08X (0x%08X)\r\n", block, LEAN_INDIRECT_MAGIC, indirect->magic);
      lcInfo += cs;
      errors++;
    }
    
    // check to see if crc is correct
    dword = LeanCalcCRC(indirect, m_block_size, TRUE);
    if (indirect->checksum != dword) {
      cs.Format("Indirect %I64i: CRC value is not 0x%08X (0x%08X)\r\n", block, dword, indirect->checksum);
      lcInfo += cs;
      errors++;
    }

    // if either of the above is in error, we don't continue
    if (errors > 0)
      break;
    
    // check that the ExtentCount member is at least 1 and if NextIndirect is non-zero, it must be max_count of extents.
    if (indirect->extent_count == 0) {
      cs.Format("Indirect %I64i: ExtentCount = 0\r\n", block);
      lcInfo += cs;
      errors++;
    }
    if ((indirect->next_indirect > 0) && (indirect->extent_count != extent_max)) {
      cs.Format("Indirect %I64i: ExtentCount = %i, should be %i since this is not the last Indirect in list.\r\n", block, indirect->extent_count, extent_max);
      lcInfo += cs;
      errors++;
    }

    // does the number of extent blocks used match block_count?
    if (indirect->extent_count > 0) {
      DWORD32 *ptr = (DWORD32 *) ((BYTE *) indirect + LEAN_INDIRECT_SIZE + (extent_max * sizeof(DWORD64)));
      DWORD64 count = 0;
      for (WORD i=0; i<indirect->extent_count; i++)
        count += ptr[i];
      if (count != indirect->block_count) {
        cs.Format("Indirect %I64i: BlockCount = %I64i, Calculated = %I64i\r\n", block, indirect->block_count, count);
        lcInfo += cs;
        errors++;
      }
      block_count += count;

      // check that all blocks in the extent(s) are marked used
      cnt = LeanCheckExtents((BYTE *) indirect + LEAN_INDIRECT_SIZE, (unsigned int) indirect->extent_count, extent_max, FALSE);
      if (cnt > 0) {
        cs.Format("Indirect %I64i: Found %i used blocks marked free in bitmap(s)\r\n", block, cnt);
        lcInfo += cs;
        errors++;
      }
    }

    // check that the Inode member is correct
    if (indirect->inode != inode_num) {
      cs.Format("Indirect %I64i: Inode = %I64i, Found = %I64i\r\n", block, inode_num, indirect->inode);
      lcInfo += cs;
      errors++;
    }
    
    // check that the ThisBlock member is correct
    if (indirect->this_block != block) {
      cs.Format("Indirect %I64i: ThisBlock = %I64i, Found = %I64i\r\n", block, block, indirect->this_block);
      lcInfo += cs;
      errors++;
    }

    // check that the Prev member points to the last one we checked.
    if (indirect->prev_indirect != prev) {
      cs.Format("Indirect %I64i: PrevIndirect = %I64i, Found = %I64i\r\n", block, prev, indirect->prev_indirect);
      lcInfo += cs;
      errors++;
    }

    // NoError: Simply display if the reserved areas are not zero
    BYTE *resv2 = buffer + LEAN_INDIRECT_SIZE + (extent_max * extent_size);
    if ((indirect->reserved0[0] != 0) || (indirect->reserved0[1] != 0) || (indirect->reserved1 != 0) ||
       ((m_reserved2_size > 0) && !IsBufferEmpty(resv2, m_reserved2_size))) {
      cs.Format("Indirect %I64i: One or more reserved areas are not zero\r\n", block);
      lcInfo += cs;
    }

    // next indirect in list
    prev = block;
    block = indirect->next_indirect;
    return_count++;
  }

  if (ret_count)
    *ret_count = return_count;
  if (blocks_used)
    *blocks_used = block_count;

  free(buffer);

  return errors;
}

// checks a set of extents and returns the count of blocks used that are marked free in the bitmap(s)
int CLean::LeanCheckExtents(void *extents, unsigned int count, unsigned int extent_max, const BOOL is_inode) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *bitmap = NULL;
  DWORD last_band = 0xFFFFFFFF, band;
  DWORD64 lba, bitmap_block = 0;
  unsigned int i, extent;
  int fnd_count = 0;  // count of blocks used but marked free
  CString cs;
  
  const DWORD64 band_size = ((DWORD64) 1 << m_super.log_blocks_per_band); // blocks per band
  const unsigned bitmap_size = (unsigned) (band_size / m_block_size / 8); // blocks per bitmap

  // create the buffer
  bitmap = (BYTE *) malloc(bitmap_size * m_block_size);

  // point to the extent
  DWORD64 *extent_start = (DWORD64 *) extents;
  DWORD32 *extent_size = (DWORD32 *) ((BYTE *) extents + (extent_max * sizeof(DWORD64)));
  DWORD32 *extent_crc = (DWORD32 *) ((BYTE *) extents + (extent_max * (sizeof(DWORD64) + sizeof(DWORD32))));

  for (extent=0; extent<count; extent++) {
    for (i=0; i<extent_size[extent]; i++) {
      lba = extent_start[extent] + i;
      band = (DWORD) (lba >> m_super.log_blocks_per_band);
      // are we in the same band as the last one, or do we need to calculate it, load it
      if (band != last_band) {
        last_band = band;
        bitmap_block = (band==0) ? m_super.bitmap_start : (band * band_size);
        dlg->ReadBlocks(bitmap, m_lba, bitmap_block, m_block_size, bitmap_size);
      }
      // calculate block in this band, byte, and bit within bitmap for 'block'
      unsigned block_in_this_band = (unsigned) (lba & ((1 << m_super.log_blocks_per_band) - 1));
      unsigned byte = block_in_this_band / 8;
      unsigned bit = block_in_this_band % 8;
      if (!(bitmap[byte] & (1 << bit)))
        fnd_count++;
    }
    
    // check the CRC of the extent
    if (m_use_extended_extents) {
      DWORD crc = LeanCalcExtentCRC(extent_start[extent], extent_size[extent], is_inode && (extent == 0));
      if (extent_crc[extent] != crc) {
        cs.Format("Inode %I64i: Extent Index %i: CRC error 0x%08X: Should be 0x%08X.\r\n", extent_start[0], extent, extent_crc[extent], crc);
        lcInfo += cs;
        lcErrorCount++;
      }
    }
  }

  // free the buffer
  free(bitmap);

  return fnd_count;
}

// add an Inode to our array of inodes
//  we first check to see if the inode is already in the array.
//  if so, increment the count
//  else, add to the end of the array
void CLean::LeanCheckAddInode(DWORD64 block) {
  void *ptr = NULL;
  unsigned i;

  // is the inode already in the list?
  for (i=0; i<lcInodeCount; i++) {
    if (lcInodes[i].inode == block) {
      lcInodes[i].count++;
      return;
    }
  }

  // do we need to create/enlarge the array?
  if (lcInodeCount >= lcInodeArraySize) {
    lcInodeArraySize += 1024;
    ptr = realloc(lcInodes, lcInodeArraySize * sizeof(struct CLeanCheckInodes));
    if (ptr == NULL)
      return;
    lcInodes = (struct CLeanCheckInodes *) ptr;
  }

  // add the inode
  lcInodes[lcInodeCount].inode = block;
  lcInodes[lcInodeCount].count = 1;
  
  lcInodeCount++;
}

void CLean::LeanCheckLinkCount(void) {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  BYTE *buffer = (BYTE *) malloc(m_block_size);
  struct S_LEAN_INODE *inode = (struct S_LEAN_INODE *) buffer;
  CString cs;
  unsigned i;

  for (i=0; i<lcInodeCount; i++) {
    // read in the inode
    dlg->ReadBlocks(inode, m_lba, lcInodes[i].inode, m_block_size, 1);

    if (inode->links_count != lcInodes[i].count) {
      cs.Format("Inode %I64i: Inode's link count (%i) should be %i\r\n", lcInodes[i].inode, inode->links_count, lcInodes[i].count);
      lcInfo += cs;
      lcErrorCount++;
    }
  }

  // free the buffer used
  free(buffer);
}
