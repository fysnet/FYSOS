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


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

//lxr.free-electrons.com/source/include/linux/efi.h#L601

typedef enum {
  EfiCpuIoWidthUint8,
  EfiCpuIoWidthUint16,
  EfiCpuIoWidthUint32,
  EfiCpuIoWidthUint64,
  EfiCpuIoWidthFifoUint8,
  EfiCpuIoWidthFifoUint16,
  EfiCpuIoWidthFifoUint32,
  EfiCpuIoWidthFifoUint64,
  EfiCpuIoWidthFillUint8,
  EfiCpuIoWidthFillUint16,
  EfiCpuIoWidthFillUint32,
  EfiCpuIoWidthFillUint64,
  EfiCpuIoWidthMaximum
} EFI_CPU_IO_PROTOCOL_WIDTH;

typedef struct _EFI_CPU_IO_PROTOCOL EFI_CPU_IO_PROTOCOL;

typedef EFI_STATUS (*EFI_CPU_IO_PROTOCOL_IO_MEM) (EFI_CPU_IO_PROTOCOL *This, EFI_CPU_IO_PROTOCOL_WIDTH Width, bit32u AddressLo, bit32u AddressHi, int Count, void *Buffer);

struct EFI_CPU_IO_PROTOCOL_ACCESS {
  EFI_CPU_IO_PROTOCOL_IO_MEM Read;
  EFI_CPU_IO_PROTOCOL_IO_MEM Write;
};

struct _EFI_CPU_IO_PROTOCOL {
  struct EFI_CPU_IO_PROTOCOL_ACCESS Mem;
  struct EFI_CPU_IO_PROTOCOL_ACCESS Io;
};

// CPU I/O protocol
EFI_CPU_IO_PROTOCOL *cpu_io = NULL;

struct EFI_GUID CPUIOProtocolGUID = {
  0xB0732526, 
  0x38C8, 
  0x4B40, 
  {
    0x88, 0x77, 0x61, 0xC7, 0xB0, 0x6A, 0xAC, 0x45
  }
};

// initialize the protocol
EFI_STATUS cpu_init_protocol() {
  EFI_STATUS Status;
  EFI_HANDLE handles[100];
  int  bufferSize = (100 * sizeof(EFI_HANDLE));
  
#ifdef EFI_1_10_SYSTEM_TABLE_REVISION
  Status = gBS->LocateProtocol(&CPUIOProtocolGUID, NULL, (void **) &cpu_io);
#else
# error "We need version 1.1 compiled in"
  Status = EFI_UNSUPPORTED;
#endif

  //Status = gBS->LocateHandle(ByProtocol, &CPUIOProtocolGUID, NULL, &bufferSize, handles);
  //if (!EFI_ERROR(Status))
  //  Status = gBS->HandleProtocol(handles[0], &CPUIOProtocolGUID, (void **) &cpu_io);
  return Status;
}

#define MAX_PORT_ADDRESS 0xFFFF

bool is_port_addr(const bit32u io_addr) {
  return (io_addr <= MAX_PORT_ADDRESS);
}

/*
 * Determine EFI CPU I/O width code
 *
 * @v size		Size of value
 * @ret width		EFI width code
 *
 * Someone at Intel clearly gets paid by the number of lines of code
 * they write.  No-one should ever be able to make I/O this
 * convoluted.  The EFI_CPU_IO_PROTOCOL_WIDTH enum is my favourite
 * idiocy.
 */
EFI_CPU_IO_PROTOCOL_WIDTH efi_width(int size) {
	switch (size) {
	  case 1 :	return EfiCpuIoWidthFifoUint8;
	  case 2 :	return EfiCpuIoWidthFifoUint16;
	  case 4 :	return EfiCpuIoWidthFifoUint32;
	  case 8 :	return EfiCpuIoWidthFifoUint64;
	}
}

/*
 * Read from device
 *
 * @v io_addr		I/O address
 * @v size		Size of value
 * @ret data		Value read
 */
bit32u efi_ioread(void *io_addr, int size) {
	EFI_CPU_IO_PROTOCOL_IO_MEM read;
	bit32u data = 0;
	EFI_STATUS efirc;
  
  read = is_port_addr((bit32u) io_addr) ? cpu_io->Io.Read : cpu_io->Mem.Read;
	if ((efirc = read(cpu_io, efi_width(size), (bit32u) io_addr, 0, 1, (void *) &data)) != 0)
		return 0xFFFFFFFF;

	return data;
}

/**
 * Write to device
 *
 * @v data		Value to write
 * @v io_addr		I/O address
 * @v size		Size of value
 */
void efi_iowrite(void *io_addr, bit32u data, int size) {
	EFI_CPU_IO_PROTOCOL_IO_MEM write;
  
	write = is_port_addr((bit32u) io_addr) ?	cpu_io->Io.Write : cpu_io->Mem.Write;
	write(cpu_io, efi_width(size), (bit32u) io_addr, 0, 1, (void *) &data);
}
