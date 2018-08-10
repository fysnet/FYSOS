/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 *
 * Note:  Since this code uses wide chars (wchar_t), you *MUST* have my modified 
 *        version of SmallerC.  Contact me for more information.
 *        
 */

#ifndef PATH_H
#define PATH_H


#define EFI_DP_TYPE_MASK                    0x7F
#define MEDIA_DEVICE_PATH                   0x04
#define MEDIA_FILEPATH_DP                   0x04

#define END_INSTANCE_DEVICE_PATH_SUBTYPE    0x01
#define END_DEVICE_PATH_TYPE                0x7F
#define END_ENTIRE_DEVICE_PATH_SUBTYPE      0xFF
#define END_DEVICE_PATH_LENGTH  (sizeof(struct EFI_DEVICE_PATH))


struct FILEPATH_DEVICE_PATH {
  struct EFI_DEVICE_PATH Header;
  wchar_t PathName[1];
};

extern struct EFI_GUID DevicePathProtocol;


EFI_STATUS LibDevicePathToInterface(struct EFI_GUID *Protocol, struct EFI_DEVICE_PATH *FilePath, void **Interface);
struct EFI_DEVICE_PATH *DuplicateDevicePath(struct EFI_DEVICE_PATH *DevPath);
struct EFI_DEVICE_PATH *FileDevicePath(void *Device, wchar_t *FileName);

#endif // PATH_H
