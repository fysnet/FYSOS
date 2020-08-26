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

/*
 *  FILE_IO.H
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.CPP
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
  UINT64 Size;   // size of this structure including all of file name + NULL ending char
  UINT64 FileSize;  // bit64u filesize
  UINT64 PhysicalSize;
  struct EFI_TIME CreateTime;
  struct EFI_TIME LastAccessTime;
  struct EFI_TIME ModificationTime;
  UINT64 Attribute;
  CHAR16 FileName[1];
};


struct EFI_FILE_HANDLE {
  bit32u Revision[2];
  EFI_STATUS   (*Open)(struct EFI_FILE_HANDLE *File, struct EFI_FILE_HANDLE **NewHandle, CHAR16 *FileName, UINT64 OpenMode, UINT64 Attributes);
  EFI_STATUS   (*Close)(struct EFI_FILE_HANDLE *File);
  EFI_STATUS   (*Delete)(struct EFI_FILE_HANDLE *File);
  EFI_STATUS   (*Read)(struct EFI_FILE_HANDLE *File, UINTN *BufferSize, void *Buffer);
  EFI_STATUS   (*Write)(struct EFI_FILE_HANDLE *File, UINTN *BufferSize, void *Buffer);
  EFI_STATUS   (*GetPosition)(struct EFI_FILE_HANDLE *File, UINT64 *Position);
  EFI_STATUS   (*SetPosition)(struct EFI_FILE_HANDLE *File, UINT64 Position);
  EFI_STATUS   (*GetInfo)(struct EFI_FILE_HANDLE *File, struct EFI_GUID *InformationType, UINTN *BufferSize, void *Buffer);
  EFI_STATUS   (*SetInfo)(struct EFI_FILE_HANDLE *File, struct EFI_GUID *InformationType, UINTN BufferSize, void *Buffer);
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
  EFI_STATUS   (*LoadFile)(struct EFI_LOAD_FILE_INTERFACE *This, struct FILEPATH_DEVICE_PATH *FilePath, BOOLEAN BootPolicy, UINTN *BufferSize, void *Buffer);
};

struct EFI_FILE_IO_INTERFACE {
  UINT64 Revision;
  EFI_STATUS   (*OpenVolume)(struct EFI_FILE_IO_INTERFACE *This, struct EFI_FILE_HANDLE **Root);
};


EFI_STATUS OpenSimpleReadFile(BOOLEAN BootPolicy, void *SourceBuffer, UINTN SourceSize, struct EFI_DEVICE_PATH **FilePath, 
                              EFI_HANDLE *DeviceHandle, EFI_HANDLE *SimpleReadHandle);
EFI_STATUS OpenSimpleFileLen(EFI_HANDLE UserHandle, UINTN *RetSize);
void CloseSimpleReadFile(EFI_HANDLE UserHandle);
EFI_STATUS ReadSimpleReadFile(EFI_HANDLE UserHandle, UINTN Offset, UINTN *ReadSize, void *Buffer);

extern struct EFI_GUID FileSystemProtocol;

#endif // FILE_IO_H
