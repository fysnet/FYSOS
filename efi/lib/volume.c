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


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

#include "boot.h"

#include "conout.h"
#include "memory.h"

#include "path.h"
#include "volume.h"


struct EFI_GUID BlockIOProtocolGUID = {
  0x964E5B21, 
  0x6459, 
  0x11D2, 
  {
    0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B
  }
};

// return a handle to the current code's Device Handle (Block Device)
EFI_HANDLE EFIGetSelfDeviceHandle(void) {
  struct EFI_LOADED_IMAGE *SelfLoadedImage;
  EFI_STATUS Status;
    
  Status = gBS->HandleProtocol(gImageHandle, &LoadedImageProtocolGUID, (void **) &SelfLoadedImage);
  if (!EFI_ERROR(Status))
    return SelfLoadedImage->DeviceHandle;
  return NULL;
}

// https://github.com/tqh/efi-example/blob/master/disk_example.c

EFI_STATUS GetVolumeInfo(struct S_DRV_PARAMS *drive_params, struct S_BOOT_DATA *boot_data) {
  EFI_HANDLE *handles;
  struct EFI_DEVICE_PATH *devicePath;
  struct EFI_BLOCK_IO_PROTOCOL *blockIOProtocol;
  int  bufferSize = 0;
  UINT32 i, HandleCount;
  EFI_STATUS Status, ioStatus;
  
  // get the Device Handle of the block device we booted from
  //  (at least the one that we loaded this code from)
  const EFI_HANDLE Self = EFIGetSelfDeviceHandle();

#ifdef EFI_1_10_SYSTEM_TABLE_REVISION
  Status = gBS->LocateHandleBuffer(ByProtocol, &BlockIOProtocolGUID, NULL, &HandleCount, &handles);
#elif defined(EFI_1_02_SYSTEM_TABLE_REVISION)
  Status = gBS->LocateHandle(ByProtocol, &BlockIOProtocolGUID, NULL, &bufferSize, NULL);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    handles = (EFI_HANDLE *) AllocatePool(bufferSize * 3);
    Status = gBS->LocateHandle(ByProtocol, &BlockIOProtocolGUID, NULL, &bufferSize, handles);
    if (EFI_ERROR(Status) || (handles == NULL))
      FreePool(handles);
  }
  HandleCount = bufferSize / sizeof(EFI_HANDLE);
#endif
  
  if (EFI_ERROR(Status) || (handles == NULL)) {
    printf(L"Volume: %[Failed to Locate Handles!%]\r\n", ERROR_COLOR);
    return Status;
  }
  
  for (i=0; i<HandleCount; i++) {
    Status = gBS->HandleProtocol(handles[i], &DevicePathProtocol, (void **) &devicePath);
    if (EFI_ERROR(Status) || (devicePath == NULL))
      continue;
    
    while (((devicePath->Type & EFI_DP_TYPE_MASK) != END_DEVICE_PATH_TYPE) && (devicePath->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE)) {
      if ((devicePath->Type == MEDIA_DEVICE_PATH) && (devicePath->SubType == MEDIA_HARDDRIVE_DP)) {
        struct HARDDRIVE_DEVICE_PATH *hdPath = (struct HARDDRIVE_DEVICE_PATH *) devicePath;
        ioStatus = gBS->HandleProtocol(handles[i], &BlockIOProtocolGUID, (void**) &blockIOProtocol);
        
        /*
        printf(L"*****************\r\n");
        printf(L"     Partition num: %i %s\r\n",  hdPath->PartitionNumber, (Self == handles[i]) ? L"(boot partition)" : L"");
        printf(L"   Partition start: %i\r\n",  hdPath->PartitionStart[0]);
        printf(L"    Partition size: %i\r\n",  hdPath->PartitionSize[0]);
        printf(L"          MBR type: %i",  hdPath->MBRType);
        if (hdPath->MBRType == 0x01)
          printf(L" (PC-AT compatible legacy MBR)\r\n");
        else if (hdPath->MBRType == 0x02)
          printf(L" (GUID Partition Table)\r\n");
        else
          printf(L" (Unknown)\r\n");
        printf(L"    Signature type: %i",  hdPath->SignatureType);
        switch (hdPath->SignatureType) {
          case 0:
            printf(L"\r\n");
            break;
          case 1:
            printf(L" MBR = %08X\r\n", * (bit32u *) &hdPath->Signature[0]);
            break;
          case 2:
            printf(L" Is 16-byte GUID: %08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X\r\n", 
              * (bit32u *) &hdPath->Signature[0],
              * (bit16u *) &hdPath->Signature[4],
              * (bit16u *) &hdPath->Signature[6],
              * (bit16u *) &hdPath->Signature[8],
              hdPath->Signature[10], hdPath->Signature[11], hdPath->Signature[12], 
              hdPath->Signature[13], hdPath->Signature[14], hdPath->Signature[15]);
            break;
          default:
            printf(L"Unknown sig type\r\n");
        }
        
        if (!EFI_ERROR(ioStatus) && (blockIOProtocol != NULL)) {
          printf(L"MediaID = 0x%08X\r\n", blockIOProtocol->Media->MediaId);
          printf(L"Removable = %i, ", blockIOProtocol->Media->RemovableMedia > 0);
          printf(L"Present = %i, ", blockIOProtocol->Media->MediaPresent > 0);
          printf(L"Log Part = %i, ", blockIOProtocol->Media->LogicalPartition > 0);
          printf(L"Read Only = %i, ", blockIOProtocol->Media->ReadOnly > 0);
          printf(L"WriteCaching = %i\r\n", blockIOProtocol->Media->WriteCaching > 0);
          printf(L"Block Size = %i\r\n", blockIOProtocol->Media->BlockSize);
          printf(L"  IO Align = %i\r\n", blockIOProtocol->Media->IoAlign);
          printf(L"Last Block = %i\r\n", blockIOProtocol->Media->LastBlock[0]);
          printf(L"   Lowest Aligned LBA = %i\r\n", blockIOProtocol->Media->LowestAlignedLba[0]);
          printf(L" Log Block/Phys Block = %i\r\n", blockIOProtocol->Media->LogicalBlocksPerPhysicalBlock);
          printf(L"    Optimal Tx Length = %i\r\n", blockIOProtocol->Media->OptimalTransferLengthGranularity);
        }
        */
        
        // if this is the block device we booted from, update the base_lba
        if (Self == handles[i]) {
          // we need to update the signature field
          // we read the base lba and get it offset 0x01B8
          boot_data->signature = 0;
          if (!EFI_ERROR(ioStatus) && (blockIOProtocol != NULL)) {
            bit8u *Buffer = AllocatePool(512);
            if (Buffer != NULL) {
              Status = blockIOProtocol->ReadBlocks(blockIOProtocol, blockIOProtocol->Media->MediaId, 0, 0, 512, Buffer);
              if (!EFI_ERROR(Status))
                boot_data->signature = * (bit32u *) &Buffer[0x01B8];
              FreePool(Buffer);
            }
          }
          // update the base_lba
          boot_data->base_lba[0] = hdPath->PartitionStart[0];
          boot_data->base_lba[1] = hdPath->PartitionStart[1];
          // mark the loader_base as zero
          boot_data->loader_base = 0;
          // the file_system is either FAT12, FAT16 or FAT32
          // (we don't use this field past the loader (this code), but
          //  we might later.  The only way I know how to detect the
          //  FAT size used in EFI is to manually detect it by the BPB.
          //  we have read the BPB above, so maybe just do that.)
          boot_data->file_system = 0; ///////
          // mark the drive number as 0
          boot_data->drive = 0;
        }

      }
      // move to next one
      devicePath = (struct EFI_DEVICE_PATH *) ((bit8u *) devicePath + devicePath->Length);
    }
    
    // update our saved fields
    /*  // I don't know why this won't return the LastBlock and other fields...????
    Status = gBS->HandleProtocol(handles[i], &BlockIOProtocolGUID, (void **) &blockIOProtocol);
    if (!EFI_ERROR(Status) && (blockIOProtocol != NULL)) {
      printf(L" Revision %08X.%08X\r\n", blockIOProtocol->Revision[1], blockIOProtocol->Revision[0]);
      printf(L"Total Sectors: %i (%i)\r\n", blockIOProtocol->Media->LastBlock[0], blockIOProtocol->Media->BlockSize);
      //printf(L" **** %i %i\r\n", blockIOProtocol->Media->MediaPresent, blockIOProtocol->Media->LogicalPartition);
      drive_params[i].bios_params.tot_sectors[0] = blockIOProtocol->Media->LastBlock[0];
      drive_params[i].bios_params.tot_sectors[1] = blockIOProtocol->Media->LastBlock[1];
      drive_params[i].bios_params.bytes_per_sector = (bit16u) blockIOProtocol->Media->BlockSize;
    }
    */
  }
  FreePool(handles);
  
  return EFI_SUCCESS;
}
