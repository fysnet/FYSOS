
/************************************************************************ 
  MPUTIMG  Save Disk Image to Floppy      v00.10.00
  Forever Young Software        Benjamin David Lunt
  
  This utility was desinged for use with Bochs to install
    a disk image to a floppy disk.
  
  Bochs is located at:
    http://bochs.sourceforge.net
  
  I designed this program to be used for testing my own OS,
   though you are welcome to use it any way you wish.
  
  Please note that I release it and it's code for others to
   use and do with as they want.  You may copy it, modify it,
   do what ever you want with it as long as you release the
   source code and display this entire comment block in your
   source or documentation file.
   (you may add to this comment block if you so desire)
  
  Please use at your own risk.  I do not specify that this
   code is correct and unharmful.  No warranty of any kind
   is given for its release.
  
  I take no blame for what may or may not happen by using
   this code with your purposes.
  
  'nuff of that!  You may modify this to your liking and if you
   see that it will help others with their use of Bochs, please
   send the revised code to fys@fysnet.net.  I will then
   release it as I have this one.
  
  You may get the latest and greatest at:
    http://www.fysnet.net/fysos.htm
  
  Thanks, and thanks to those who contribute(d) to Bochs....
  
  ********************************************************
  
  Things to know:
  - Currently, this app only allows the floppy disk sizes
    specified.
  - This app reads full tracks at a time to make it quicker.
  
  ********************************************************
  
  To compile using DJGPP:  (http://www.delorie.com/djgpp/)
    gcc -Os mputimg.c -o mputimg.exe -s  (DOS .EXE requiring DPMI)
  
  Compiles as is with MS VC++ 6.x         (WinNT .EXE file)
    **** Requires WinNT machine ***
  
  ********************************************************
  
  Usage:
    Simply answer the questions given.  Hit <enter> for the
     default of any question that has a default value in []'s.
    That's it....
  
************************************************************************/


//#define WINNT    // to compile for WINNT
//#define WIN95    // to compile for WIN95
#define DOS      // to compile for DOS (using a DPMI)

#if (defined(WINNT) && (defined(WIN95) || defined(DOS))) || \
    (defined(WIN95) && (defined(WINNT) || defined(DOS))) || \
    (defined(DOS) && (defined(WINNT) || defined(WIN95)))
  #error Must not define multiple targets
#endif

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
//#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
//#include <stddef.h>
//#include <dos.h>

#include "mputimg.h"   // our include

#if defined(WINNT) | defined(WIN95)
  #include <windows.h>
  HANDLE logical_drv;
#elif defined(DOS)
  #if defined(DJGPP)
    #include <dpmi.h>
    #include <go32.h>
  #endif
  #define TRUE    1
  #define FALSE   0
  #define bool char
  unsigned  char *fbuffer;
#else
  #error Must define a target platform
#endif

FILE *fp;

unsigned  char *buffer;          // a temp buffer
          char filename[80];     // filename
          char temp[128];
      char drv_letter[80];       // drive letter (A, B)
      char yesno[80];            // yes no
      bool verbose = TRUE;

struct DISK_TYPE *disk_info = NULL;

#if defined(WINNT) | defined(WIN95)
  bool write_track(char *drv_letter, int cyl, int side, unsigned char *ptr, int spt);
#else
  bool write_track(char drv_letter, int cyl, int side, unsigned char *ptr, int spt);
#endif  
void read_sectors(FILE *fp, unsigned char *ptr, unsigned long cnt);

int main(int argc, char *argv[]) {

  int i, j;
  unsigned long file_len;

  // print start string
  printf(strtstr);

#if defined(WINNT)
  // Make sure we are a version of Windows that allows direct disk access.
  OSVERSIONINFO os_ver_info;
  os_ver_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&os_ver_info);
  if (os_ver_info.dwMajorVersion < 5) {
    printf("\nThis utility only works with WinXP and possibly earlier versions of WinNT");
    if (os_ver_info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
      printf("\nDid not find WinXP, but did find an NT version of Windows.  Continue? (Y|N)");
      gets(temp);
      if (strcmp(temp, "Y") && strcmp(temp, "Yes") && strcmp(temp, "YES"))
        return 0xFF;
    } else
      return 0xFF;
  }
#elif defined(WIN95)
  printf("\n *** Warning.  This currently only works on Win95 machines ***");
  printf("\n ***           If you are using a WinNT machine, recompile. ***");
  printf("\n   Continue (Yes or No): ");
  gets(temp);
  if (strcmp(temp, "Y") && strcmp(temp, "Yes") && strcmp(temp, "YES"))
    return 0xFF;
#endif
  
  // defaults
  strcpy(drv_letter, "");
  strcpy(filename, "");
  
  // parse command line
  for (i=1; i<argc; i++) {
    if (argv[i][0] == '/') {
      if (argv[i][1] == 'V')
        verbose = TRUE;
      else if (argv[i][1] == 'v')
        verbose = FALSE;
    } else if (argv[i][1] == ':') {
      drv_letter[0] = toupper(argv[i][0]);
    } else {
      strncpy(filename, argv[i], 80);
    }
  }
  
	// if no image file was given on command line, get one
  if (!strlen(filename)) {
	  printf("\n Filename [floppy.img]: ");
	  gets(filename);
	  if (!strlen(filename)) strcpy(filename, "floppy.img");
  }
  
  // open image file
  if ((fp = fopen(filename, "rb")) == NULL) {
    printf("\n Error opening image file: %s", filename);
    return 0x01;
  }
  
  fseek(fp, 0, SEEK_END);
  file_len = ftell(fp);
  rewind(fp);
  
  if (file_len % 512) {
    printf("\n File is not a multiple of 512 byte sectors.");
    return 0x02;
  }
  
  switch (file_len / 512) {
    case 320:
      disk_info = &disk160;
      break;
    case 360:
      disk_info = &disk180;
      break;
    case 640:
      disk_info = &disk320;
      break;
    case 720:
      disk_info = &disk360;
      break;
    case 2400:
      disk_info = &disk1220;
      break;
    case 1440:
      disk_info = &disk720;
      break;
    case 2880:
      disk_info = &disk1440;
      break;
    case 3360:
      disk_info = &disk1720;
      break;
    case 5760:
      disk_info = &disk2880;
      break;
    default:
      printf("\n Image is of unknown size.");
      return 0x02;
  }
  
  if (verbose) {
    printf("\n Found %s image with %i sectors.  Continue? [Y]", disk_info->name, file_len / 512);
    gets(yesno);
    if (!strlen(yesno)) strcpy(yesno, "Y");
    if (strcmp(yesno, "Y")) {
      printf("\nAborting...");
      return 0xFF;
    }
  }
  
  // if user did not specify a driver letter on the command line, then ask for one
  if (!strlen(drv_letter)) {  
    do {
      printf("Please choose a drive letter (A, B, C, D, ...) [A]: ");
      gets(drv_letter);
      if (!strlen(drv_letter)) { strcpy(drv_letter, "A"); break; }
      drv_letter[0] = toupper(drv_letter[0]);
    } while ((drv_letter[0] < 'A') || (drv_letter[0] > 'Z'));
  }
  
#if defined(WINNT)
  sprintf(drv_letter, "\\\\.\\%c:", toupper(drv_letter[0]));
#else
  drv_letter[0] = toupper(drv_letter[0]) - 'A';
#endif
  
  // TODO: make sure disk is in drive and ready (if floppy)
  //       check disk is ready (if hard drive)
  
  // print info   
  if (verbose) {
    printf("\n        Writing file:  %s"
         "\n           Cylinders:  %i"
         "\n               Sides:  %i"
         "\n       Sectors/Track:  %i"
         "\n       Total Sectors:  %i"
         "\n                Size:  %s",
         filename, disk_info->cylinders, disk_info->num_heads, disk_info->sec_per_track,
         disk_info->total_sects, disk_info->name);
    
    // make sure
    printf("\n  Is this correct? (Y or N) [Y]: ");
    gets(yesno);
    if (!strlen(yesno)) strcpy(yesno, "Y");
    if (strcmp(yesno, "Y")) {
      printf("\nAborting...");
      return 0xFF;
    }
  }
  
#if defined(WINNT)
  logical_drv = CreateFile((char *) &drv_letter, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
#elif defined(WIN95)
  logical_drv = CreateFile("\\\\.\\vwin32", 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL);
#endif
  
#if defined(WINNT) || defined(WIN95)
  if (logical_drv == (void *) 0xFFFFFFFF) {
    printf("\n Error opening drive for write. (%i)", GetLastError());
    return 0x03;
  }
#endif
  
  // allocate mem
  if (!(buffer = (unsigned char *) calloc(disk_info->sec_per_track * 512, sizeof(unsigned char)))) {
    printf("\nError allocating buffer");
    return 0x08;
  }
  
  printf("\n");
  
  // do it
  for (i=0; i<disk_info->cylinders; i++) {
    for (j=0; j<disk_info->num_heads; j++) {
      read_sectors(fp, buffer, disk_info->sec_per_track);
      
#if defined(WINNT) | defined(WIN95)
      printf("\rWriting cyl %i (of %i), side %i", i + 1, disk_info->cylinders, j);
      if (!write_track((char *) &drv_letter, i, j, buffer, disk_info->sec_per_track)) {
        i = disk_info->cylinders;
        break;
      }
#else
      printf("\rWriting cyl %i (of %i)", i + 1, disk_info->cylinders);
      if (!write_track(drv_letter[0], i, j, buffer, disk_info->sec_per_track)) {
        i = disk_info->cylinders;
        break;
      }
#endif

    }
  }
  
  // close the file
  fclose(fp);
  
#if defined(WINNT) | defined(WIN95)
  CloseHandle(logical_drv);
#endif
  
  // free buffer
  free(buffer);
  
  return 0x00;
}

// Write a track
#if defined(WINNT) | defined(WIN95)
  bool write_track(char *drv_letter, int cyl, int side, unsigned char *ptr, int spt) {
#else
  bool write_track(char drv_letter, int cyl, int side, unsigned char *ptr, int spt) {
#endif  
  
  unsigned short status;
  unsigned t;
  
#if defined(WINNT)
  unsigned long ntemp;
#elif defined(WIN95)
  DIOC_REGISTERS reg;
  unsigned long cb;
  static unsigned long win95_start = 0;
#else
  #if defined(DJGPP)
    __dpmi_regs regs;
  #endif
#endif
  
  for (t=0; t<3; t++) {
#if defined(WINNT)
    WriteFile(logical_drv, (void *) ptr, (512 * spt), (unsigned long *) &ntemp, NULL);
    if (ntemp == 0) {
      printf("\n Error: %i", GetLastError());
      break;
    } else
      status = FALSE;
#elif defined(WIN95)
    reg.reg_ECX = spt;
    reg.reg_EDX = win95_start;
    reg.reg_EBX = (unsigned long) ptr;
    reg.reg_EAX = drv_letter[0];
    reg.reg_Flags = 0x0001;     // assume error (carry flag is set) 
    status = (DeviceIoControl(logical_drv, VWIN32_DIOC_DOS_INT26,
       &reg, sizeof(reg), &reg, sizeof(reg), &cb, NULL) == 0);
    //if (!status & (reg.reg_Flags & 0x0001)) status = TRUE;
    win95_start += spt;
#else
    int ret_sel;
    int seg = __dpmi_allocate_dos_memory(((512*spt)+15)>>4, &ret_sel);
    dosmemput(ptr, (512*spt), seg << 4);
    
    regs.h.ah = 0x03;
    regs.h.al = spt;
    regs.h.ch = (cyl & 0xFF);
    regs.h.cl = ((cyl & 0x0300) >> 2) | 1;
    regs.h.dh = side;
    regs.h.dl = drv_letter;
    regs.x.bx = 0x0000;
    regs.x.es = seg;
    __dpmi_int(0x13, &regs);
    status = regs.x.flags;
    
    __dpmi_free_dos_memory(ret_sel);
#endif
    
    if (!(status & 0x01)) return TRUE;
  }
  printf("\nError writing to drive.  Continue? [N]\n");
  gets(yesno);
  if (!strlen(yesno)) strcpy(yesno, "N");
  if (strcmp(yesno, "N") == 0)
    return FALSE;
  else
    return TRUE;
}

// Read sector(s)
void read_sectors(FILE *fp, unsigned char *ptr, unsigned long cnt) {
  if (fread(ptr, 512, cnt, fp) < cnt) {
    printf("\n **** Error reading from file ****");
    exit(0xFF);
  }
}
