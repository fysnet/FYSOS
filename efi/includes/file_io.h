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

#ifndef FILE_IO_H
#define FILE_IO_H

#include "path.h"


// Open modes
#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

// File attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVIED      0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037


struct EFI_FILE_INFO {
  bit32u Size[2];   // size of this structure including all of file name + NULL ending char
  bit32u FileSize[2];  // bit64u filesize
  bit32u PhysicalSize[2];
  struct EFI_TIME CreateTime;
  struct EFI_TIME LastAccessTime;
  struct EFI_TIME ModificationTime;
  bit32u Attribute[2];
  CHAR16 FileName[1];
};


struct EFI_FILE_HANDLE {
  bit32u Revision[2];
  EFI_STATUS   (*Open)(struct EFI_FILE_HANDLE *File, struct EFI_FILE_HANDLE **NewHandle, wchar_t *FileName, bit32u OpenMode, bit32u Attributes);
  EFI_STATUS   (*Close)(struct EFI_FILE_HANDLE *File);
  EFI_STATUS   (*Delete)(struct EFI_FILE_HANDLE *File);
  EFI_STATUS   (*Read)(struct EFI_FILE_HANDLE *File, bit32u *BufferSize, void *Buffer);
  EFI_STATUS   (*Write)(struct EFI_FILE_HANDLE *File, bit32u *BufferSize, void *Buffer);
  EFI_STATUS   (*GetPosition)(struct EFI_FILE_HANDLE *File, bit32u *Position);  // Position is a 64-bit pointer???
  EFI_STATUS   (*SetPosition)(struct EFI_FILE_HANDLE *File, bit32u PositionL, bit32u PositionH);  // should be a single bit64u instead of two bit32u's
  EFI_STATUS   (*GetInfo)(struct EFI_FILE_HANDLE *File, struct EFI_GUID *InformationType, bit32u *BufferSize, void *Buffer);
  EFI_STATUS   (*SetInfo)(struct EFI_FILE_HANDLE *File, struct EFI_GUID *InformationType, bit32u BufferSize, void *Buffer);
  EFI_STATUS   (*Flush)(struct EFI_FILE_HANDLE *File);
};

struct SIMPLE_READ_HANDLE {
  UINTN   Signature;
  BOOLEAN FreeBuffer;
  VOID   *Source;
  UINTN  SourceSize;
  struct EFI_FILE_HANDLE *FileHandle;
};

struct EFI_LOAD_FILE_INTERFACE {
  EFI_STATUS   (*LoadFile)(struct EFI_LOAD_FILE_INTERFACE *This, struct FILEPATH_DEVICE_PATH *FilePath, bool BootPolicy, bit32u *BufferSize, void *Buffer);
};

struct EFI_FILE_IO_INTERFACE {
  bit32u Revision[2];
  EFI_STATUS   (*OpenVolume)(struct EFI_FILE_IO_INTERFACE *This, struct EFI_FILE_HANDLE **Root);
};




EFI_STATUS OpenSimpleReadFile(bool BootPolicy, void *SourceBuffer, bit32u SourceSize, struct EFI_DEVICE_PATH **FilePath, 
                              EFI_HANDLE *DeviceHandle, EFI_HANDLE *SimpleReadHandle);
EFI_STATUS OpenSimpleFileLen(EFI_HANDLE UserHandle, bit32u *RetSize);
void CloseSimpleReadFile(EFI_HANDLE UserHandle);
EFI_STATUS ReadSimpleReadFile(EFI_HANDLE UserHandle, bit32u Offset, bit32u *ReadSize, void *Buffer);



extern struct EFI_GUID FileSystemProtocol;

#endif // FILE_IO_H
