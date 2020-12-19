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

// FatCheck.cpp : implementation file

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"
#include "ultimateDlg.h"

#include "Fat.h"

#include "Modeless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned fcErrorCount, fcDirCount, fcFileCount, fcDelFileCount,
         fcCluster_cnt, fcVolCount;
bool fc_entry_error;
DWORD *fcClusters;
CString fcInfo;

void CFat::OnFatCheck() {
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  struct S_FAT32_BPB *bpb32 = (struct S_FAT32_BPB *) m_bpb_buffer;
  struct S_FAT1216_BPB *bpb12 = (struct S_FAT1216_BPB *) m_bpb_buffer;
  void *alt_buffer;
  unsigned i;
  int ret;
  CString cs;
  BOOL alert = TRUE, error, fat_dirty = FALSE;
  DWORD cluster_count;
  
  // disable the button
  GetDlgItem(ID_CHECK)->EnableWindow(FALSE);

  // clear the globals 
  // (if we don't, we will add to what was shown last time)
  fcErrorCount = 0;
  fcCluster_cnt = 0;
  fcDirCount = 0;
  fcFileCount = 0;
  fcDelFileCount = 0;
  fcVolCount = 0;
  fc_entry_error = FALSE;
  fcInfo.Empty();
  
  CModeless modeless;
  modeless.m_Title = "Checking FAT";
  modeless.m_modeless = TRUE;
  modeless.Create(CModeless::IDD, this);
  modeless.ShowWindow(SW_SHOW);
  modeless.BringWindowToTop();
  
  // calculate max count of cluster entries there are in a single FAT
  const unsigned sect_per_fat = (m_fat_size == FS_FAT32) ? bpb32->sect_per_fat32 : bpb12->sect_per_fat;
  if (m_fat_size == FS_FAT12)
    cluster_count = (DWORD) ((float) (sect_per_fat * bpb12->bytes_per_sect) / (float) 1.5);
  else if (m_fat_size == FS_FAT16)
    cluster_count = (sect_per_fat * bpb12->bytes_per_sect) / 2;
  else
    cluster_count = (sect_per_fat * bpb12->bytes_per_sect) / 4;

  // Check 1: // Check items in the BPB
  fcInfo += "Checking the BPB\r\n";
  if (bpb12->jmp[2] != 0x90)
    fcInfo += "[Diag] BPB.JMP[2] != 0x90\r\n";
  //if (bpb12->bytes_per_sect != 512) {
  //  fcInfo += "BPB.BytesPerSector != 512\r\n";
  //  fcErrorCount++;
  //}
  if (bpb12->sect_reserved == 0) {
    fcInfo += "BPB.SectorsReserved == 0\r\n";
    fcErrorCount++;
  }
  if (bpb12->fats != 2)
    fcInfo += "[Diag] BPB.Fats != 2 (Is usually 2, but can be 1)\r\n";
  if ((bpb12->descriptor > 0x01) && (bpb12->descriptor < 0xF0)) {
    fcInfo += "BPB.Descriptor must be 0x00, 0x01, or 0xF0 to 0xFF.\r\n";
    fcErrorCount++;
  }
  if (bpb12->hidden_sects != m_lba) {
    cs.Format("[Diag] BPB.HiddenSectors != Current Base of 0x%08X\r\n", (DWORD) m_lba);
    fcInfo += cs;
  }
  
  // Check 2: Cluster Count
  // A Fat-12 must be less than 4085 clusters
  // A Fat-16 must be at least 4085 and less than 65525 clusters
  // A Fat-32 must be at least 65525 clusters
  if (m_fat_size == FS_FAT12) {
    if (cluster_count >= 4085) {
      cs.Format("FAT-12 should have a cluster count less than 4085. Has %u\r\n", cluster_count);
      fcInfo += cs;
      fcErrorCount++;
    }
  } else if (m_fat_size == FS_FAT16) {
    if ((cluster_count < 4085) || (cluster_count >= 65525)) {
      cs.Format("FAT-16 should have a cluster count of at least 4085\r\n"
                "  and less than 65525. Has %u\r\n", cluster_count);
      fcInfo += cs;
      fcErrorCount++;
    }
  } else {
    if (cluster_count < 65525) {
      cs.Format("FAT-32 should have a cluster count of at least 65525. Has %u\r\n", cluster_count);
      fcInfo += cs;
      fcErrorCount++;
    }
  }
  
  // Check 3: // Does each FAT match the first FAT?
  fcInfo += "Checking the FAT(s)\r\n";
  modeless.SetDlgItemText(IDC_EDIT, fcInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

  // first two FAT entries must be 0x0FFFFFFF & 0x0FFFFFF8
  // fat specs 1.03 state: (page 18)
  //  "The first reserved cluster, FAT[0], contains the BPB_Media byte value in its low 8 bits, and all other bits are set to 1.
  //  For example, if the BPB_Media value is 0xF8, for FAT12 FAT[0] = 0x0FF8, for FAT16 FAT[0] = 0xFFF8, and for FAT32 FAT[0] = 0x0FFFFFF8.
  //  The second reserved cluster, FAT[1], is set by FORMAT to the EOC mark. On FAT12 volumes, it is not used and is simply always contains an EOC mark.
  //  For FAT16 and FAT32, the file system driver may use the high two bits of the FAT[1] entry for dirty volume flags (all other bits, are always left set to 1)

  // TODO: high bits in -16 and -32 have different meaning

  DWORD *dword = (DWORD *) m_fat_buffer;
  if (m_fat_size == FS_FAT12) {
    if ((dword[0] & 0x00FFFFFF) != (0x00FFFF00 | bpb12->descriptor)) {
      cs.Format("First two cluster entries are not F%02X FFF. (0x%03X 0x%03X)", bpb12->descriptor, dword[0] & 0xFFF, (dword[0] >> 12) & 0xFFF);
      fcInfo += cs;
      fcErrorCount++;
      if (AfxMessageBox(cs + "\r\nFix?", MB_YESNO, 0) == IDYES) {
        dword[0] = (0x00FFFF00 | bpb12->descriptor);
        fat_dirty = TRUE;
        fcInfo += " -- Fixed!";
      }
      fcInfo += "\r\n";
    }
  } else if (m_fat_size == FS_FAT16) {
    if (dword[0] != (0xFFFFFF00 | bpb12->descriptor)) {
      cs.Format("First two cluster entries are not FF%02X FFFF. (0x%04X 0x%04X)", bpb12->descriptor, dword[0] & 0xFFFF, (dword[0] >> 16) & 0xFFFF);
      fcInfo += cs;
      fcErrorCount++;
      if (AfxMessageBox(cs + "\r\nFix?", MB_YESNO, 0) == IDYES) {
        dword[0] = (0xFFFFFF00 | bpb12->descriptor);
        fat_dirty = TRUE;
        fcInfo += " -- Fixed!";
      }
      fcInfo += "\r\n";
    }
  } else {
    if ((dword[0] != (0x0FFFFF00 | bpb12->descriptor)) && (dword[1] != 0xFFFFFFFF)) {
      cs.Format("First two cluster entries are not 0FFF_FF%02X 0FFF_FFFF. (0x%08X 0x%08X)", bpb12->descriptor, dword[0], dword[1]);
      fcInfo += cs;
      fcErrorCount++;
      if (AfxMessageBox(cs + "\r\nFix?", MB_YESNO, 0) == IDYES) {
        dword[0] = (0xFFFFFF00 | bpb12->descriptor);
        dword[1] = 0xFFFFFFFF;
        fat_dirty = TRUE;
        fcInfo += " -- Fixed!";
      }
      fcInfo += "\r\n";
    }
  }
  
  if (bpb12->fats > 1) {
    for (i=1; i<bpb12->fats; i++) {
      cs.Format("Comparing FAT #%i with #0:\r\n", i);
      fcInfo += cs;
      modeless.SetDlgItemText(IDC_EDIT, fcInfo);
      modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
      
      alt_buffer = malloc(sect_per_fat * bpb12->bytes_per_sect);
      dlg->ReadFromFile(alt_buffer, m_lba + bpb12->sect_reserved + (sect_per_fat * i), sect_per_fat);
      
      // now compare the fat with the original fat
      error = FALSE;
      DWORD j, Cluster0, ClusterJ;
      cs.Format("Found %i FAT entries\r\n", cluster_count);
      fcInfo += cs;
      modeless.SetDlgItemText(IDC_EDIT, fcInfo);
      modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
      for (j=0; j<cluster_count; j++) {
        Cluster0 = GetNextCluster(m_fat_buffer, j);
        ClusterJ = GetNextCluster(alt_buffer, j);
        if (Cluster0 != ClusterJ) {
          cs.Format("* Cluster index %u in FAT #%u doesn't match 1st FAT.\r\n"
                    "    (0x%08X != 0x%08X)  Continue?", j, i, Cluster0, ClusterJ);
          fcInfo += cs;
          fcInfo += "\r\n";
          modeless.SetDlgItemText(IDC_EDIT, fcInfo);
          modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
          if (alert) {
            if (AfxMessageBox(cs, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2, 0) == IDNO)
              break;
            alert = FALSE;
          }
          error = TRUE;
        }
      }
      
      // if error == TRUE, ask if we wish to copy the indexed FAT to FAT 0 and/or visa-versa
      if (error) {
        cs.Format("There are differences between FAT #%i and the 1st FAT.\r\n"
                  "Do you wish to copy the 1st fat to FAT #%i?\r\n", i, i);
        fcInfo += cs;
        ret = AfxMessageBox(cs, MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_DEFBUTTON3, 0);
        if (ret == IDYES) {
          fcInfo += "Yes\r\n";
          cs.Format("Writing FAT #0 to FAT #%i\r\n", i); 
          fcInfo += cs;
          dlg->WriteToFile(m_fat_buffer, m_lba + bpb12->sect_reserved + (sect_per_fat * i), sect_per_fat);
          fcInfo += "Changes applied\r\n";
          AfxMessageBox("Changes applied.");
        } else if (ret == IDNO) {
          fcInfo += "No\r\n";
          cs.Format("Do you wish to copy FAT #%i to the 1st fat?\r\n", i);
          fcInfo += cs;
          if (AfxMessageBox(cs, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2, 0) == IDYES) {
            fcInfo += "Yes\r\n";
            cs.Format("Copying FAT #%i to FAT #0\r\n", i);
            fcInfo += cs;
            memcpy(m_fat_buffer, alt_buffer, sect_per_fat * bpb12->bytes_per_sect);
            AfxMessageBox("Changes will not be made until you use the Apply button.");
          } else
            fcInfo += "No\r\n";
        } else
          fcInfo += "Cancel\r\n";
        fcErrorCount++;
      }
      
      modeless.SetDlgItemText(IDC_EDIT, fcInfo);
      modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();
      free(alt_buffer);
    }
  }
  
  // Check 4: // check the directory entries
  fcInfo += "Checking Root/Sub directory(s)\r\n";
  modeless.SetDlgItemText(IDC_EDIT, fcInfo);
  modeless.GetDlgItem(IDC_EDIT)->UpdateWindow();

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

  // Check 5: // ?????
  
  
  
  
  
  
  
  // write the fat(s)?
  if (fat_dirty) {
    fcInfo += "Updating the FAT(s)\r\n";
    UpdateTheFATs();
  }
  
  cs.Format("\r\n  Found %i errors\r\n", fcErrorCount);
  fcInfo += cs;
  fcInfo += "\r\nUse the COPY button to copy all to the clipboard or the DONE button to exit\r\n";
  
  // now "copy" the modeless to a modal and
  //  destroy the modeless and display the modal
  CModeless modal;
  modal.m_edit = fcInfo;
  modal.m_Title = modeless.m_Title;
  modeless.DestroyWindow();
  modal.m_modeless = FALSE;
  modal.DoModal();
  
  // "free" the memory used by the CString
  fcInfo.Empty();
  
  // re-enable the button
  GetDlgItem(ID_CHECK)->EnableWindow(TRUE);
}

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

