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

// GetImage.cpp : implementation file
//

#include "stdafx.h"
#include "pch.h"

#include "ultimate.h"

#include "winioctl.h"
#include "GetImage.h"

#include "ntddcdrm.h"
#include "ISOPrimary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma pack(push, 1)

#define CDROM_SECT_SIZE  2048

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

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// CGetImage dialog
CGetImage::CGetImage(CWnd* pParent /*=NULL*/)
  : CDialog(CGetImage::IDD, pParent) {
  //{{AFX_DATA_INIT(CGetImage)
  m_new_name = _T("");
  m_type = (IDC_TYPE_LOG_DRIVE - IDC_TYPE_160K);
  //}}AFX_DATA_INIT
  m_disk_info = NULL;
  m_volumes = NULL;
  m_vol_count = 0;
}

void CGetImage::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CGetImage)
  DDX_Control(pDX, IDC_STATUS, m_status);
  DDX_Control(pDX, IDC_VOLUME_LIST, m_vol_list);
  DDX_Control(pDX, IDC_PROGRESS, m_progress);
  DDX_Text(pDX, IDC_NEW_NAME, m_new_name);
  DDX_Radio(pDX, IDC_TYPE_160K, m_type);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGetImage, CDialog)
  //{{AFX_MSG_MAP(CGetImage)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_TYPE_160K, IDC_TYPE_LOG_DRIVE, OnButtonClicked)
  ON_EN_KILLFOCUS(IDC_SECTOR_COUNT, OnKillfocusSectorCount)
  ON_LBN_SELCHANGE(IDC_VOLUME_LIST, OnSelchangeVolumeList)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetImage message handlers
BOOL CGetImage::OnInitDialog() {
  CDialog::OnInitDialog();
  
  OnButtonClicked(IDC_TYPE_LOG_DRIVE);
  
  m_volumes = (struct VOLUMES *) calloc(MAX_VOLUMES * sizeof(struct VOLUMES), 1);
  m_vol_count = GetDrives(m_volumes, MAX_VOLUMES);
  FillDrives(m_volumes, m_vol_count);
  
  return TRUE;
}

struct DISK_TYPE disk_info[] = {
  {  320, 40,  8, 1,  160},  // 160k
  {  360, 40,  9, 1,  180},  // 180k
  {  640, 40,  8, 2,  320},  // 320k
  {  720, 40,  9, 2,  360},  // 360k
  { 2400, 80, 15, 2, 2400},  // 1.20Meg
  { 1440, 80,  9, 2,  720},  // 720k
  { 1440, 80,  9, 2,  720},  // 720k
  { 2880, 80, 18, 2, 1440},  // 1.44Meg
  { 3360, 80, 21, 2, 1720},  // 1.72Meg
  { 5760, 80, 36, 2, 2880},  // 2.88Meg
  {    0,  0, 63, 16,   0}
};

void CGetImage::OnButtonClicked(UINT nID) {
  const int index = (nID - IDC_TYPE_160K);
  
  m_disk_info = &disk_info[index];
  
  SetDlgItemInt(IDC_SPT, m_disk_info->sec_per_track, FALSE);
  SetDlgItemInt(IDC_HEAD_COUNT, m_disk_info->num_heads, FALSE);
  if (index < (IDC_TYPE_LOG_DRIVE - IDC_TYPE_160K)) {
    GetDlgItem(IDC_VOLUME_LIST)->EnableWindow(FALSE);
    GetDlgItem(IDC_SECTOR_COUNT)->SendMessage(EM_SETREADONLY, TRUE, 0);
    SetDlgItemInt(IDC_CYL_COUNT, m_disk_info->cylinders, FALSE);
    SetDlgItemInt(IDC_SECTOR_COUNT, (UINT) m_disk_info->total_sects, FALSE);
  } else {
    GetDlgItem(IDC_VOLUME_LIST)->EnableWindow(TRUE);
    GetDlgItem(IDC_SECTOR_COUNT)->SendMessage(EM_SETREADONLY, FALSE, 0);
    OnSelchangeVolumeList();
  }
  OnKillfocusSectorCount();
}

void CGetImage::OnKillfocusSectorCount() {
  CString cs;
  UINT val = GetDlgItemInt(IDC_SECTOR_COUNT, NULL, FALSE);
  UINT cyls = 0;
  UINT spt = GetDlgItemInt(IDC_SPT, NULL, FALSE);
  UINT heads = GetDlgItemInt(IDC_HEAD_COUNT, NULL, FALSE);
  if ((spt * heads) > 0)
    cyls = (val + ((spt * heads) - 1)) / (spt * heads);
  SetDlgItemInt(IDC_CYL_COUNT, cyls, FALSE);
}

BOOL CGetImage::GetDrvGeometry(DISK_GEOMETRY *pdg, const TCHAR drv) {
  TCHAR szDrive[7] = TEXT("\\\\.\\x:");
  HANDLE hDevice;    // handle to the drive to be examined
  BOOL bResult;      // results flag
  DWORD dword;       // discard results
  
  szDrive[4] = drv;
  hDevice = CreateFile(szDrive,   // drive to open
      0,                          // don't need any access to the drive
      FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
      NULL,                       // default security attributes
      OPEN_EXISTING,              // disposition
      0,                          // file attributes
      NULL);                      // don't copy any file's attributes
  if (hDevice == INVALID_HANDLE_VALUE)
    return FALSE;
  
  bResult = DeviceIoControl(hDevice, // device we are querying
      IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
      NULL, 0,                       // no input buffer
      pdg, sizeof(DISK_GEOMETRY),    // output buffer
      &dword,                        // count of bytes returned
      (LPOVERLAPPED) NULL);          // synchronous I/O
  
  CloseHandle(hDevice); // we're done with the handle

  return bResult;
}

BOOL CGetImage::GetDrvGeometryEx(DISK_GEOMETRY_EX *pdg, const TCHAR drv, LARGE_INTEGER *liSize) {
  TCHAR szDrive[7] = TEXT("\\\\.\\x:");
  HANDLE hDevice;    // handle to the drive to be examined
  BOOL bResult;      // results flag
  DWORD dword;       // discard results
  
  szDrive[4] = drv;
  hDevice = CreateFile(szDrive,   // drive to open
      0,                          // don't need any access to the drive
      FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
      NULL,                       // default security attributes
      OPEN_EXISTING,              // disposition
      0,                          // file attributes
      NULL);                      // don't copy any file's attributes
  if (hDevice == INVALID_HANDLE_VALUE)
    return FALSE;
  
  bResult = DeviceIoControl(hDevice, // device we are querying
      IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, // operation to perform
      NULL, 0,                       // no input buffer
      pdg, sizeof(DISK_GEOMETRY_EX),    // output buffer
      &dword,                        // count of bytes returned
      (LPOVERLAPPED) NULL);          // synchronous I/O
  
  CloseHandle(hDevice); // we're done with the handle

  return bResult;
}

void CGetImage::OnSelchangeVolumeList() {
  DISK_GEOMETRY pdg;
  DISK_GEOMETRY_EX pdgex;
  int index = m_vol_list.GetCurSel();
  UINT count = 0;
  CString cs;
  UINT heads = 0, spt = 0;
  
  if (index >= 0) {
    // this assumes (and it should be) that the first char is the drive letter.
    m_vol_list.GetText(index, cs);
    LARGE_INTEGER liSize;
    if (!GetDrvGeometryEx(&pdgex, cs.GetAt(0), &liSize)) {
      // GetDrvGeometryEx does not work on floppies, so we need to use the old version
      if (GetDrvGeometry(&pdg, cs.GetAt(0))) {
        const DWORD64 TotalSectors = pdg.Cylinders.QuadPart * pdg.TracksPerCylinder * pdg.SectorsPerTrack;
        count = (UINT) TotalSectors;
        if (m_volumes[index].iType != DRIVE_CDROM) {
          heads = (UINT) (TotalSectors / pdg.Cylinders.QuadPart / pdg.SectorsPerTrack);
          spt = pdg.SectorsPerTrack;
        }
      } else {
        AfxMessageBox("Attempt to get drive geometry failed.");
        return;
      }
    } else {
      count = (UINT) (pdgex.DiskSize.QuadPart / pdgex.Geometry.BytesPerSector);
      if (m_volumes[index].iType != DRIVE_CDROM) {
        spt = pdgex.Geometry.SectorsPerTrack;
        heads = (UINT) ((pdgex.DiskSize.QuadPart / pdgex.Geometry.BytesPerSector) / pdgex.Geometry.Cylinders.QuadPart / pdgex.Geometry.SectorsPerTrack);
      }
    }
    
    SetDlgItemInt(IDC_SPT, spt, FALSE);
    SetDlgItemInt(IDC_HEAD_COUNT, heads);
    
    OnKillfocusSectorCount();
  }
  
  SetDlgItemInt(IDC_SECTOR_COUNT, count, FALSE);
}

#define BUFSIZE 512

int CGetImage::GetDrives(struct VOLUMES *volumes, const int max_count) {
  TCHAR szTemp[BUFSIZE];
  TCHAR szName[BUFSIZE];
  szTemp[0] = '\0';
  int count = 0;
  
  CWaitCursor wait; // display a wait cursor
  if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) {
    TCHAR szDrive[3] = TEXT(" :");
    TCHAR szDriveRoot[4] = TEXT(" :\\");
    TCHAR *p = szTemp;
    
    do {
      // Copy the drive letter to the template string
      szDrive[0] = *p;
      szDriveRoot[0] = *p;
      
      strcpy(volumes[count].szDriveName, szDrive);
      volumes[count].iType = GetDriveType(szDriveRoot);
      
      DWORD MaxComp, Flags;
      if (GetVolumeInformation(szDriveRoot, szName, 64, NULL, &MaxComp, &Flags, NULL, 0))
        strcpy(volumes[count].szName, szName);
      else
        strcpy(volumes[count].szName, "Unknown");
      
      count++;
      
      // Go to the next NULL character.
      while (*p++);
      
    } while ((count < max_count) && *p); // end of string?
  }
  wait.Restore();
  
  return count;
}

void CGetImage::FillDrives(struct VOLUMES *volumes, const int count) {
  CString cs;
  
  for (int i=0; i<count; i++) {
    switch (volumes[i].iType) {
      case DRIVE_REMOVABLE:
        cs.Format("%s %s (Removable)", volumes[i].szDriveName, volumes[i].szName);
        break;
      case DRIVE_FIXED:
        cs.Format("%s %s (Fixed)", volumes[i].szDriveName, volumes[i].szName);
        break;
      case DRIVE_CDROM:
        cs.Format("%s %s (CD-ROM)", volumes[i].szDriveName, volumes[i].szName);
        break;
      default:
        continue;
    }
    m_vol_list.AddString(cs);
  }
}

void CGetImage::OnCancel() {
  if (m_volumes)
    free(m_volumes);
  
  CDialog::OnCancel();
}

void CGetImage::OnOK() {
  DWORD dword;
  int index, ret;
  UINT cnt = 0;
  CString cs, csStatus;
  TCHAR szDrive[7] = TEXT("\\\\.\\x:");
  BYTE *buffer;
  CFile file;
  HANDLE logical_drv;
  LONG lIdle = 0;
  BOOL bRet;
  int trys;
  
  // get the desired count
  GetDlgItemText(IDC_SECTOR_COUNT, cs);
  cnt = convert32(cs);
  
  // get the type selected
  if (m_type < (IDC_TYPE_LOG_DRIVE - IDC_TYPE_160K)) {
    //
    AfxMessageBox("TODO");
    //
  } else {
    index = m_vol_list.GetCurSel();
    if (index < 0) {
      AfxMessageBox("Must select a volume");
      return;
    }
    
    // this assumes (and it should be) that the first char is the drive letter.
    m_vol_list.GetText(index, cs);
    szDrive[4] = cs.GetAt(0);
    
    // open the logical drive
    logical_drv = CreateFile(szDrive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (logical_drv == INVALID_HANDLE_VALUE) {
      AfxMessageBox("Error opening logical drive");
      return;
    }
    
    // create the target file
    if (file.Open(m_new_name, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareExclusive, NULL) == 0) {
      AfxMessageBox("Error Creating File...");
      return;
    }
    
    // do the read
    CWaitCursor wait; // display a wait cursor
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    
    ret = 0;
    csStatus.Empty();
    if (m_volumes[index].iType == DRIVE_CDROM) {
      csStatus += "Found CDROM\r\n";
      m_status.SetWindowText(csStatus);
      m_status.UpdateWindow();
      
      // Lock the compact disc in the CD-ROM drive to prevent accidental removal while reading from it.
      PREVENT_MEDIA_REMOVAL pmrLockCDROM;
      pmrLockCDROM.PreventMediaRemoval = TRUE;
      DeviceIoControl(logical_drv, IOCTL_CDROM_MEDIA_REMOVAL, &pmrLockCDROM, sizeof(pmrLockCDROM), NULL, 0, &dword, NULL);
      
      // Get sector size of compact disc
      DISK_GEOMETRY dgCDROM;
      DeviceIoControl(logical_drv, IOCTL_CDROM_GET_DRIVE_GEOMETRY, NULL, 0, &dgCDROM, sizeof(dgCDROM), &dword, NULL);
      if (dgCDROM.BytesPerSector != CDROM_SECT_SIZE) {
        cs.Format("Error: Disc Sector size isn't %i (%i)", CDROM_SECT_SIZE, dgCDROM.BytesPerSector);
        AfxMessageBox(cs);
        ret = 0;
        goto GetImageDone;
      }
      
      // allocate our buffer
      //buffer = (BYTE *) VirtualAlloc(NULL, CDROM_SECT_SIZE * 32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
      buffer = (BYTE *) malloc(CDROM_SECT_SIZE * 32);
      
      CDROM_READ_TOC_EX toc;
      memset(&toc, 0, sizeof(toc));
      toc.Format = CDROM_READ_TOC_EX_FORMAT_SESSION;
      toc.Msf = 0;
      toc.SessionTrack = 0;
      
      _CDROM_TOC_SESSION_DATA *data = (_CDROM_TOC_SESSION_DATA *) buffer;
      DeviceIoControl(logical_drv, IOCTL_CDROM_READ_TOC_EX, &toc, sizeof(toc), data, 804, &dword, NULL);
      
      int tot_sessions = data->LastCompleteSession - data->FirstCompleteSession + 1;
      cs.Format("Found %i session(s)\r\n", tot_sessions);
      csStatus += cs;
      m_status.SetWindowText(csStatus);
      m_status.UpdateWindow();
      
      LARGE_INTEGER pos, last_session_start;
      last_session_start.QuadPart = (data->TrackData[0].Address[0] << 24) | (data->TrackData[0].Address[1] << 16) | 
        (data->TrackData[0].Address[2] << 8) | (data->TrackData[0].Address[3] << 0);
      
      cs.Format("Last session starts at %I64i\r\n", last_session_start.QuadPart);
      csStatus += cs;
      m_status.SetWindowText(csStatus);
      m_status.UpdateWindow();
      
      pos.QuadPart = ((last_session_start.QuadPart + 16) * CDROM_SECT_SIZE);
      pos.LowPart = SetFilePointer(logical_drv, pos.LowPart, &pos.HighPart, FILE_BEGIN);
      
      if (pos.QuadPart != ((last_session_start.QuadPart + 16) * CDROM_SECT_SIZE)) {
        cs.Format("Could not seek to %I64i.  Seeked to %I64i", ((last_session_start.QuadPart + 16) * CDROM_SECT_SIZE), pos.QuadPart);
        AfxMessageBox(cs);
        ret = 0;
        goto GetImageDone;
      }
      
      // now read in the descriptors
      ReadFile(logical_drv, buffer, CDROM_SECT_SIZE, &dword, NULL);
      if (dword < CDROM_SECT_SIZE) {
        AfxMessageBox("Error reading the first sector of the disc");
        ret = 0;
        goto GetImageDone;
      }
      
      if (memcmp(&buffer[1], "CD001", 5) == 0) {
        struct S_ISO_PVD *pvd = (struct S_ISO_PVD *) buffer;
        cnt = pvd->num_lbas;
        
        cs.Format("Found ISO 9660 disc: lbas = %i  size = %i\r\n", pvd->num_lbas, pvd->set_size);
        csStatus += cs;
        m_status.SetWindowText(csStatus);
        m_status.UpdateWindow();
      } else if (memcmp(&buffer[1], "BEA01", 5) == 0) {
        // read in the next sector
        bRet = FALSE;
        for (trys=0; trys<3 && !bRet; trys++)
          bRet = ReadFile(logical_drv, buffer, CDROM_SECT_SIZE, &dword, NULL);
        if (!bRet) {
          AfxMessageBox("Error Reading from CDROM. (1)");
          ret = 0;
          goto GetImageDone;
        }
        
        // should be NSR02 or NSR03 for UDF discs
        if ((memcmp(&buffer[1], "NSR02", 5) == 0) ||
            (memcmp(&buffer[1], "NSR03", 5) == 0)) {
          
          // Anchor Volume Descriptor Pointer (always at sector 256)
          //printf("\n Found Anchor Volume Descriptor Pointer at LBA 256");
          pos.QuadPart = (256 * CDROM_SECT_SIZE);
          SetFilePointer(logical_drv, pos.LowPart, &pos.HighPart, FILE_BEGIN);
          
          bRet = FALSE;
          for (trys=0; trys<3 && !bRet; trys++)
            bRet = ReadFile(logical_drv, buffer, CDROM_SECT_SIZE, &dword, NULL);
          if (!bRet) {
            AfxMessageBox("Error Reading from CDROM. (2)");
            ret = 0;
            goto GetImageDone;
          }
          
          // read Main Volume Descriptor Sequence
          struct AVDP *avdp = (struct AVDP *) buffer;
          int len = avdp->main_vds.length;
          //printf("\n Main Volume Descriptor Sequence at LBA %i (len = %i)", avdp->main_vds.location, avdp->main_vds.length);
          pos.QuadPart = (avdp->main_vds.location * CDROM_SECT_SIZE);
          SetFilePointer(logical_drv, pos.LowPart, &pos.HighPart, FILE_BEGIN);
          // TODO:  what if (avdp->main_vds.length > (CDROM_SECT_SIZE * 32))???
          
          bRet = FALSE;
          for (trys=0; trys<3 && !bRet; trys++)
            bRet = ReadFile(logical_drv, buffer, avdp->main_vds.length, &dword, NULL);
          if (!bRet) {
            AfxMessageBox("Error Reading from CDROM. (3)");
            ret = 0;
            goto GetImageDone;
          }
          
          // buffer now contains the Main Volume Descriptor Sequence
          struct DESC_TAG *tag = (struct DESC_TAG *) buffer;
          struct DESC_PART *part;
          //struct DESC_LVD *lvd;
          //struct LVD_MAP_1 *map;
          //struct LVD_MAP_2 *map2;
          while (len > 0) {
            switch (tag->id) {
              /*
              case 0x0001:  // Primary Volume Descriptor
                AfxMessageBox("Found Primary Volume Descriptor");
                break;
              case 0x0002:  // AVDP (shouldn't find this one)
                AfxMessageBox("Found AVDP");
                break;
              case 0x0003:  // Volume Descriptor Pointer
                AfxMessageBox("Found VDP");
                break;
              case 0x0004:  // Implement Use Volume Descriptor
                AfxMessageBox("Found Implement Use Volume Descriptor");
                break;
              */
              case 0x0005:  // Partition Descriptor
                //AfxMessageBox("Found Partition Descriptor");
                part = (struct DESC_PART *) tag;
                //printf("\n  num: %i  sequ %i", part->number, part->sequ_num);
                //printf("\n  Starting LBA: %i, sectors: %i, allocated: %s", part->start_lba, part->sectors, yes_no_str[part->flags & 1]);
                //printf("\n  [%c%c%c%c%c]", part->contents[1], part->contents[2], part->contents[3], part->contents[4], part->contents[4]);
                //printf("\n  access type: %s", access_type_str[part->access_type & 0x07]);  // 0x07 to make sure not above 7
                //printf("\n  Implementation: [%s] [%s]", part->implement, part->implement_use);
                // if is allocated and last sector is above our count, then make our count above last sector
                if ((part->flags & 1) &&
                   ((part->start_lba + part->sectors) > cnt))
                  cnt = (part->start_lba + part->sectors);
                break;
              /*
              case 0x0006:  // Logical Volume Descriptor
                AfxMessageBox("Found Logical Volume Descriptor");
                lvd = (struct DESC_LVD *) tag;
                printf("\n   block size = %i", lvd->block_size);
                printf("\n   number of partition map entries = %i", lvd->partition_maps);
                map = (struct LVD_MAP_1 *) lvd->maps;
                for (UINT i=0; i<lvd->partition_maps; i++) {
                  if (map->type == 1)
                    printf("\n    Map %i: type 1:  sequ = %i, part = %i", ul, map->sequ_num, map->part_num);
                  else if (map->type == 2) {
                    map2 = (struct LVD_MAP_2 *) map;
                    printf("\n    Map %i: type 2:  [%02X %02X %02X %02X]", ul, map2->id[0], map2->id[1], map2->id[2], map2->id[3]);
                  } else 
                    break;
                  map = (struct LVD_MAP_1 *) ((bit32u) map + map->len);
                }
                if (lvd->block_size != CDROM_SECT_SIZE)
                  printf("\n ***** Block size doesn't equal %i *****", SECT_SIZE);
                break;
              case 0x0007:  // Unallocated Space Descriptor
                AfxMessageBox("Found Unallocated Space Descriptor");
                break;
              */
              case 0x0008:  // Terminating Descriptor
                AfxMessageBox("Found Terminating Descriptor");
                len = CDROM_SECT_SIZE;
                break;
              /*
              case 0x0009:  // Logical Volume Integrity Descriptor
                AfxMessageBox("Found Logical Volume Integrity Descriptor");
                break;
              */
              default:
                //AfxMessageBox("Found unknown Volume Descriptor (0x%04X)", tag->id);
                len = CDROM_SECT_SIZE;
            }
            len -= CDROM_SECT_SIZE;
            tag = (struct DESC_TAG *) ((BYTE *) tag + CDROM_SECT_SIZE);
          }
        } else {
          //printf("\n Did not find NSR0x descriptor...");
          //return -1;
        }
        //printf("\n Found a size of %i sectors (%i meg)...Continue (Y|N): [N]", cnt, (cnt * SECT_SIZE) / 1000000);
        //gets(temp);
        //if (strlen(temp) && (strcmp(temp, "Y") != 0))
        //  return 0;
      } else {
        cs.Format("Did not find ""CD001"" or ""BEA01"" signature at found session start\r\n"
                  "Found [%c%c%c%c%c] instead.", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
        AfxMessageBox(cs);
        ret = 0;
        goto GetImageDone;
      }
      
      // rewind(logical_drv)
      SetFilePointer(logical_drv, last_session_start.LowPart, &last_session_start.HighPart, FILE_BEGIN);
      
      // read in the blocks
      m_progress.SetRange32(0, cnt-1);
      m_progress.SetPos(0);
      UINT i;
      for (i=0; i<cnt; ) {
        cs.Format("%s"
                  " *** Hit ESC to abort ***\r\n"
                  " Sector %i of %i", csStatus, i, cnt-1);
        m_status.SetWindowText(cs);
        m_status.UpdateWindow();
        bRet = FALSE;
        for (trys = 0; trys<3 && !bRet; trys++)
          bRet = ReadFile(logical_drv, buffer, CDROM_SECT_SIZE, &dword, NULL);
        if (!bRet || (dword != CDROM_SECT_SIZE))
          break;
        file.Write(buffer, CDROM_SECT_SIZE);
        i++;
        m_progress.SetPos(i);
        //m_progress.UpdateWindow();
        if (GetAsyncKeyState(VK_ESCAPE) < 0) {
          ret = -1;
          break;
        }
        AfxGetApp()->OnIdle(lIdle++);  // avoid "Not Responding" notice
      }
      if (ret != -1)
        ret = (i == cnt) ? 1 : 0;
      
      // Unlock the disc in the CD-ROM drive.
      pmrLockCDROM.PreventMediaRemoval = FALSE;
      DeviceIoControl(logical_drv, IOCTL_CDROM_MEDIA_REMOVAL, &pmrLockCDROM, sizeof(pmrLockCDROM), NULL, 0, &dword, NULL);
      
      //VirtualFree(buffer, 0, MEM_RELEASE);
      free(buffer);
      
    } else {
      UINT count, i;
      const UINT spt = GetDlgItemInt(IDC_SPT, NULL, FALSE);
      cnt = GetDlgItemInt(IDC_SECTOR_COUNT, NULL, FALSE);
      buffer = (BYTE *) malloc(spt * 512);
      m_progress.SetRange32(0, cnt-1);
      m_progress.SetPos(0);
      for (i=0; i<cnt; ) {
        cs.Format(" *** Hit ESC to abort ***\r\n"
                  " Sector %i of %i", i, cnt-1);
        m_status.SetWindowText(cs);
        m_status.UpdateWindow();
        count = ((cnt - i) >= spt) ? spt : (cnt - i);
        ReadFile(logical_drv, buffer, count * 512, (DWORD *) &dword, NULL);
        if (dword != (count * 512))
          break;
        file.Write(buffer, count * 512);
        i += count;
        m_progress.SetPos(i);
        //m_progress.UpdateWindow();
        if (GetAsyncKeyState(VK_ESCAPE) < 0) {
          ret = -1;
          break;
        }
        AfxGetApp()->OnIdle(lIdle++);  // avoid "Not Responding" notice
      }
      free(buffer);
      if (ret != -1)
        ret = (i == cnt) ? 1 : 0;
    }
    
    // TODO:  If you hit the ESC key above to abort, the ESC is still
    //  in the buffer and the MessageBox below instantly gets closed...

GetImageDone:
    m_progress.SetPos(0);
    GetDlgItem(IDOK)->EnableWindow(TRUE);
    GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
    wait.Restore();
    file.Close();
    CloseHandle(logical_drv);
    
    if (ret == 0) {
      AfxMessageBox("Error reading from volume.");
      return;
    } else if (ret == -1) {
      AfxMessageBox("User Abort.");
      return;
    } else {
      if (AfxMessageBox("Successfully read from device. Exit?", MB_YESNO, 0) == IDYES) {
        if (m_volumes)
          free(m_volumes);
        CDialog::OnOK();
      }
    }
  }
}
