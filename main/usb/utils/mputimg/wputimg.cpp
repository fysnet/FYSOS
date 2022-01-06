/*
 * MGETIMG  Get Logical Disk Image                   v00.20.00
 * Forever Young Software                  Benjamin David Lunt
 * 
 * This code is for WinXP only...
 * 
 * Usage:
 *   Simply answer the questions given.  Hit <enter> for the
 *    default of any question that has a default value in []'s.
 *   That's it....
 *  
 */

#include <windows.h>
#include <stdio.h>

#include "..\\ctype.h"   // our types include
#include "mputimg.h"     // our include

 FILE  *fp;
HANDLE logical_drv;
 bit8u *buffer;          // a temp buffer
  char filename[80];     // filename
  char temp[128];
  char drv_letter[80];   // drive letter (A, B)
  char yesno[80];        // yes no
  bool verbose = TRUE;
struct DISK_TYPE *disk_info = NULL;
 char diff = ' ';

int main(int argc, char *argv[]) {
  
  // print start string
  printf(strtstr);
  
  // Make sure we are a version of Windows that allows direct disk access.
  // This code is for Version 5.x (WinNT/XP family)
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
  
  // defaults
  strcpy(drv_letter, "");
  strcpy(filename, "");
  
  // parse command line
  for (int i=1; i<argc; i++) {
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
  bit64u file_len = ftell(fp);
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
      // found a hard drive image?
      disk_info = &harddisk;
      // get sector count
      disk_info->total_sects = file_len / 512;
      disk_info->size = file_len;
      disk_info->cylinders = (bit32u) (disk_info->total_sects / (63 * 16)); // assuming 16 heads, 63 sectors per track
      disk_info->num_heads = 16;
      disk_info->sec_per_track = 63;
  }
  
  if (verbose) {
    printf("\n Found image with %i sectors.  Continue? [Y] ", file_len / 512);
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
      printf("Please choose a drive letter (A, B, C, etc.) [A]: ");
      gets(drv_letter);
      if (!strlen(drv_letter)) { strcpy(drv_letter, "A"); break; }
      drv_letter[0] = toupper(drv_letter[0]);
    } while ((drv_letter[0] < 'A') || (drv_letter[0] > 'Z'));
  }
  
  // if the user chose C:, warn again writing to the hard drive
  if (toupper(drv_letter[0]) == 'C') {
    printf("\n Are you sure you wanted C: (Must enter YES)? [No]");
    gets(yesno);
    if (!strlen(yesno)) strcpy(yesno, "No");
    if (strcmp(yesno, "YES") != 0) {
      printf("\nAborting...");
      return 0xFF;
    }
  }
  
  sprintf(drv_letter, "\\\\.\\%c:", toupper(drv_letter[0]));
  
  // TODO: make sure disk is in drive and ready (if floppy)
  //       check disk is ready (if hard drive)
  
  // print info   
  if (verbose) {
    if ((disk_info->cylinders * disk_info->num_heads * disk_info->sec_per_track) != disk_info->total_sects)
      diff = '*';
    printf("\n        Writing file:  %s"
         "\n           Cylinders:  %i%c"
         "\n               Sides:  %i"
         "\n       Sectors/Track:  %i"
         "\n       Total Sectors:  %" LL64BIT "i"
         "\n                Size:  %" LL64BIT "i",
         filename, disk_info->cylinders, diff, disk_info->num_heads, disk_info->sec_per_track,
         disk_info->total_sects, disk_info->size);
    if (diff == '*')
      printf("\n*Total Sectors doesn't match cylinder boundary.");
    
    // make sure
    printf("\n  Is this correct? (Y or N) [Y]: ");
    gets(yesno);
    if (!strlen(yesno)) strcpy(yesno, "Y");
    if (strcmp(yesno, "Y")) {
      printf("\nAborting...");
      return 0xFF;
    }
  }
  
  HANDLE logical_drv = CreateFile((char *) &drv_letter, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
  if (logical_drv == (void *) 0xFFFFFFFF) {
    printf("\n Error opening drive for write. (%i)", GetLastError());
    return -1;
  }
  
  // allocate mem
  if (!(buffer = (bit8u *) calloc(disk_info->sec_per_track * 512, sizeof(bit8u)))) {
    printf("\nError allocating buffer");
    return -8;
  }
  
  printf("\n");
  
  // do the write
  bit32u cnt;
  bit64u k = disk_info->total_sects;
  bit64u j = 0;
  while (k) {
    printf("\rWriting sector %" LL64BIT "i (of %" LL64BIT "i)     ", j, disk_info->total_sects);
    cnt = ((k >= (bit64u) disk_info->sec_per_track) ? disk_info->sec_per_track : (bit32u) k);
    if (fread(buffer, 512, cnt, fp) < cnt)
      break;
    if (!write_sectors(logical_drv, buffer, j, cnt))
      break;
    k -= cnt;
    j += cnt;
  }
  if (k == 0)
    printf("\rSuccessfully wrote %" LL64BIT "i sectors.      ", disk_info->total_sects);
  
  // close the file
  fclose(fp);
  CloseHandle(logical_drv);
  
  // free buffer
  free(buffer);
  
  return 0;
}

// write a "track" of sectors
bool write_sectors(HANDLE logical_drv, void *ptr, const bit64u lba, const bit32u cnt) {
  bit32u ntemp;
  unsigned t;  // try count (incase of floppies)
  
  for (t=0; t<3; t++) {
    WriteFile(logical_drv, ptr, (512 * cnt), (bit32u *) &ntemp, NULL);
    if (ntemp != (512 * cnt))
      printf("\n Try #%i: Error Writing to sector %" LL64BIT "i (%i)", t, lba, GetLastError());
    else
      return TRUE;
  }
  
  return FALSE;
}
