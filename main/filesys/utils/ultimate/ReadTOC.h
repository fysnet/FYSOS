#pragma once

#define BX_CD_FRAMESIZE 2048
#define CD_FRAMESIZE    2048

// READ_TOC_EX structure(s) and #defines

#define CDROM_READ_TOC_EX_FORMAT_TOC      0x00
#define CDROM_READ_TOC_EX_FORMAT_SESSION  0x01
#define CDROM_READ_TOC_EX_FORMAT_FULL_TOC 0x02
#define CDROM_READ_TOC_EX_FORMAT_PMA      0x03
#define CDROM_READ_TOC_EX_FORMAT_ATIP     0x04
#define CDROM_READ_TOC_EX_FORMAT_CDTEXT   0x05

#define IOCTL_CDROM_BASE              FILE_DEVICE_CD_ROM
#define IOCTL_CDROM_READ_TOC_EX       CTL_CODE(IOCTL_CDROM_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS)
#ifndef IOCTL_DISK_GET_LENGTH_INFO
#define IOCTL_DISK_GET_LENGTH_INFO    CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
#endif

typedef struct _CDROM_READ_TOC_EX {
    UCHAR Format    : 4;
    UCHAR Reserved1 : 3; // future expansion
    UCHAR Msf       : 1;
    UCHAR SessionTrack;
    UCHAR Reserved2;     // future expansion
    UCHAR Reserved3;     // future expansion
} CDROM_READ_TOC_EX, *PCDROM_READ_TOC_EX;

typedef struct _TRACK_DATA {
    UCHAR Reserved;
    UCHAR Control : 4;
    UCHAR Adr : 4;
    UCHAR TrackNumber;
    UCHAR Reserved1;
    UCHAR Address[4];
} TRACK_DATA, *PTRACK_DATA;

typedef struct _CDROM_TOC_SESSION_DATA {
    // Header
    UCHAR Length[2];  // add two bytes for this field
    UCHAR FirstCompleteSession;
    UCHAR LastCompleteSession;
    // One track, representing the first track
    // of the last finished session
    TRACK_DATA TrackData[1];
} CDROM_TOC_SESSION_DATA, *PCDROM_TOC_SESSION_DATA;

// CReadTOC dialog
class CReadTOC : public CDialogEx
{
	DECLARE_DYNAMIC(CReadTOC)

public:
	CReadTOC(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CReadTOC();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_READ_TOC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString toc_dump;
	virtual void OnOK();
  virtual BOOL OnInitDialog();
  afx_msg void OnClickedCopy();
  BOOL m_msf;
  CComboBox m_drive_letter;
  CComboBox m_format;
  int m_starting_track;

	afx_msg void OnSelchangeFormat();
};
