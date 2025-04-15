// ReadTOC.cpp : implementation file
//

#include "pch.h"
#include "ultimate.h"
#include "ultimateDlg.h"
#include <winioctl.h>
#include "ReadTOC.h"
#include "afxdialogex.h"

// CReadTOC dialog

IMPLEMENT_DYNAMIC(CReadTOC, CDialogEx)

CReadTOC::CReadTOC(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_READ_TOC, pParent) {
}

CReadTOC::~CReadTOC() {
}

void CReadTOC::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_DRIVE_LETTER, m_drive_letter);
  DDX_Control(pDX, IDC_FORMAT, m_format);
  DDX_Check(pDX, IDC_MSF, m_msf);
  DDX_Text(pDX, IDC_STARTING_TRACK, m_starting_track);
  DDV_MinMaxInt(pDX, m_starting_track, 1, 255);
}

BEGIN_MESSAGE_MAP(CReadTOC, CDialogEx)
  ON_BN_CLICKED(IDCOPY, &CReadTOC::OnClickedCopy)
  ON_CBN_SELCHANGE(IDC_FORMAT, &CReadTOC::OnSelchangeFormat)
END_MESSAGE_MAP()

// CReadTOC message handlers
void CReadTOC::OnOK() {
  HANDLE hFile;
  char drive[32];
  char path[32];
  UCHAR format, msf, track;
  CString csTemp;

  UpdateData(TRUE);
  
  m_drive_letter.GetLBText(m_drive_letter.GetCurSel(), path);
  format = m_format.GetCurSel();  // format
  msf = (format <= 1) ? m_msf : 0;
  track = ((format == 0) || (format == 2)) ? (UCHAR) m_starting_track : 0;
  
  sprintf(drive, "\\\\.\\%s", path);
  hFile = CreateFile(drive, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    DWORD lpBytesReturned;
    DeviceIoControl(hFile, IOCTL_STORAGE_LOAD_MEDIA, NULL, 0, NULL, 0, &lpBytesReturned, NULL);

    // the implementation below is the platform-dependent code required
    // to read the TOC from a physical cdrom.
    // This only works with WinXP or newer
    CDROM_READ_TOC_EX input;
    memset(&input, 0, sizeof(input));
    input.Format = format;
    input.Msf = msf;
    input.SessionTrack = (UCHAR) track;

    // We have to allocate a chunk of memory to make sure it is aligned on a sector base.
    UCHAR *data = (UCHAR *) VirtualAlloc(NULL, 2048*2, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (data != NULL) {
      DWORD iBytesReturned = 0;
      DeviceIoControl(hFile, IOCTL_CDROM_READ_TOC_EX, &input, sizeof(input), data, 2048*2, &iBytesReturned, NULL);
      if (!toc_dump.IsEmpty())
        toc_dump += "\r\n";
      csTemp.Format("Returned %i bytes: (format = %i, msf = %i, start track = %i)\r\n", iBytesReturned, format, msf, track);
      toc_dump += csTemp;
      DumpIt(toc_dump, data, 0, iBytesReturned, 1);
      VirtualFree(data, 0, MEM_RELEASE);
    } else
      toc_dump += "Could not allocate memory\r\n";
    SetDlgItemText(IDC_TOC_DUMP, toc_dump);
  } else {
    AfxMessageBox("Unable to 'open' drive");
  }
}

BOOL CReadTOC::OnInitDialog() {
  CDialogEx::OnInitDialog();

  // TODO:  Add extra initialization here
  CUltimateDlg *dlg = (CUltimateDlg *) AfxGetApp()->m_pMainWnd;
  GetDlgItem(IDC_TOC_DUMP)->SetFont(&dlg->m_DumpFont);

  // todo: get the cdroms from the system
  // For now, we hard code D:
  m_drive_letter.AddString("D:");

  m_drive_letter.SetCurSel(0);
  m_format.SetCurSel(0);
  m_msf = 0;
  m_starting_track = 1;
  toc_dump.Empty();

  UpdateData(FALSE);

  return TRUE;
}

void CReadTOC::OnClickedCopy() {
  if (!OpenClipboard())
    return;

  if (!EmptyClipboard())
    return;

  size_t size = toc_dump.GetLength();
  HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, size);
  if (hGlob != NULL) {
    memcpy(hGlob, toc_dump, size);
    ::SetClipboardData(CF_TEXT, hGlob);
    GlobalFree(hGlob);
  }

  CloseClipboard();
}

// The msf value is only valid with types 0 and 1
// The starting track is only valid with types 0 and 2
void CReadTOC::OnSelchangeFormat() {
  int cur = m_format.GetCurSel();
  GetDlgItem(IDC_MSF)->EnableWindow(cur <= 1);
  GetDlgItem(IDC_STARTING_TRACK)->EnableWindow((cur == 0) || (cur == 2));
}
