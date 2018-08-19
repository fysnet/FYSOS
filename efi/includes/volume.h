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
 */

#ifndef VOLUME_H
#define VOLUME_H


#define MEDIA_HARDDRIVE_DP     0x01
#define MEDIA_CDROM_DP         0x02


#pragma pack(push, 1)

struct HARDDRIVE_DEVICE_PATH {
  struct EFI_DEVICE_PATH Header;
  /// Describes the entry in a partition table, starting with entry 1.
  /// Partition number zero represents the entire device. Valid
  /// partition numbers for a MBR partition are [1, 4]. Valid
  /// partition numbers for a GPT partition are [1, NumberOfPartitionEntries].
  ///
  UINT32                          PartitionNumber;
  /// Starting LBA of the partition on the hard drive.
  ///
  bit32u                          PartitionStart[2];
  /// Size of the partition in units of Logical Blocks.
  ///
  bit32u                          PartitionSize[2];
  /// Signature unique to this partition:
  /// If SignatureType is 0, this field has to be initialized with 16 zeros.
  /// If SignatureType is 1, the MBR signature is stored in the first 4 bytes of this field.
  ///   (The other 12 bytes are initialized with zeros)
  /// If SignatureType is 2, this field contains a 16 byte signature.
  UINT8                           Signature[16];
  /// Partition Format: (Unused values reserved).
  /// 0x01 - PC-AT compatible legacy MBR.
  /// 0x02 - GUID Partition Table.
  UINT8                           MBRType;
  /// Type of Disk Signature: (Unused values reserved).
  /// 0x00 - No Disk Signature.
  /// 0x01 - 32-bit signature from address 0x1b8 of the type 0x01 MBR.
  /// 0x02 - GUID signature.
  UINT8                           SignatureType;
};

struct EFI_BLOCK_IO_MEDIA {
  UINT32  MediaId;
  BOOLEAN RemovableMedia;
  BOOLEAN MediaPresent;
  BOOLEAN LogicalPartition;
  BOOLEAN ReadOnly;
  BOOLEAN WriteCaching;
  UINT32  BlockSize;
  UINT32  IoAlign;
  bit32u  LastBlock[2];
  bit32u  LowestAlignedLba[2];               // added after Revision 2.1
  UINT32  LogicalBlocksPerPhysicalBlock;     // added after Revision 2.1
  UINT32  OptimalTransferLengthGranularity;  // added after Revision 2.1
};
#pragma pack(pop)

struct EFI_BLOCK_IO_PROTOCOL {
  bit32u Revision[2];
  const struct EFI_BLOCK_IO_MEDIA *Media;
  EFI_STATUS   (*Reset)(struct EFI_BLOCK_IO_PROTOCOL *This, bool ExtendedVerification);
  EFI_STATUS   (*ReadBlocks)(struct EFI_BLOCK_IO_PROTOCOL *This, bit32u MeidaId, bit32u LbaLo, bit32u LbaHi, bit32u BufferSize, void *Buffer);
  EFI_STATUS   (*WriteBlocks)(struct EFI_BLOCK_IO_PROTOCOL *This, bit32u MeidaId, bit32u LbaLo, bit32u LbaHi, bit32u BufferSize, void *Buffer);
  EFI_STATUS   (*FlushBlocks)(struct EFI_BLOCK_IO_PROTOCOL *This);
};



EFI_STATUS GetVolumeInfo(struct S_DRV_PARAMS *drive_params, struct S_BOOT_DATA *boot_data);

#endif // VOLUME_H
