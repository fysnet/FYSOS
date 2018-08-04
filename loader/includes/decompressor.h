
#ifndef _DECOMPRESSOR_H
#define _DECOMPRESSOR_H

#define LDR_HDR_FLAGS_HALT     (1<<0)
#define LDR_HDR_FLAGS_ISKERNEL (1<<1)

#pragma pack(push, 1)

struct S_LDR_HDR {
  bit32u id;             // 0x46595332 = 'FYS2'
  bit32u location;       // location to store the file
  bit32u flags;          // bit 0 = halt on error, bit 1 = is kernel file
  bit32u file_crc;       // uncompressed/moved files crc
  bit8u  comp_type;      // compression type (0=none, 1 = bz2)
  bit8u  hdr_crc;        // byte sum check sum of hdr
  bit32u file_size;      // size of uncompressed file
  bit8u  resv[10];       // reserved
};

#pragma pack(pop)

bit32u calc_crc(void *, const bit32u);
int decompressor(const void *, const bit32u);



#endif  // _DECOMPRESSOR_H
