
#include "ctype.h"
#include "decompressor.h"

#include "malloc.h"
#include "string.h"
#include "windows.h"

#include "bz2.h"

bool do_decomp_flat(void *location, const void *src, const bit32u *size) {
  
  win_printf(main_win, "...moving");
  memcpy(location, src, *size);
  
  return TRUE;
}

bool do_decomp_bz2(void *location, const void *src, const bit32u *size) {
  int ret_size = *size;
  int ret;
  
  win_printf(main_win, "...decompressing(bz2)");
  
  if ((ret = bz2_decompressor(location, src, &ret_size)) != BZ_OK) {
    win_printf(main_win, "...Error decompressing file.  error: %i\n", ret);
    *size = 0;
    return FALSE;
  }
  
  *size = ret_size;
  return TRUE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// calculate the crc
bit32u calc_crc(void *location, const bit32u size) {
  bit8u *p = (bit8u *) location;
  bit8u octet;
  bit32u result = 0;
  int cnt = size;
  
  if (cnt > 4) {
    // initialize the progress proc
    win_init_progress(size);
    
    result  = *p++; result <<= 8;
    result |= *p++; result <<= 8;
    result |= *p++; result <<= 8; 
    result |= *p++;
    result  = ~result;
    
    cnt -= 4;
    
    for (int i=0; i<cnt; i++) {
      // display the progress
      win_put_progress(i, 0);
      
      octet = *p++;
      for (int j=0; j<8; j++) {
        if (result & 0x80000000) {
          result = (result << 1) ^ 0x04C11DB7 ^ (octet >> 7);
        } else {
          result = (result << 1) ^ (octet >> 7);
        }
        octet <<= 1;
      }
    }
    // make sure we get to 100% on the progress bar
    win_put_progress(size, 0);
    
    // The complement of the remainder
    result = ~result;
  }
  
  return result;
}

int decompressor(const void *source, const bit32u size) {
  struct S_LDR_HDR *ldr_hdr = (struct S_LDR_HDR *) source;
  bit8u *p = (bit8u *) source;
  bit32u ret_size = 0;
  bit8u crc = 0;
  bool ret;
  
  // see if the first 32 bytes of the file is a loader header
  // the first 32-bits will == 46595332h
  if (ldr_hdr->id != 0x46595332) {
    win_printf(main_win, "...Did not find load header id dword\n");
    return 0;
  }
  
  // now check the header's crc
  for (int i=0; i<sizeof(struct S_LDR_HDR); i++)
    crc += p[i];
  
  if (crc) {
    win_printf(main_win, "...Invalid header crc\n");
    return 0;
  }
  
  // now check for the compression type
  ret_size = size - sizeof(struct S_LDR_HDR);
  switch(ldr_hdr->comp_type) {
    case 0:
      ret = do_decomp_flat((void *) ldr_hdr->location, (void *) ((bit32u) source + sizeof(struct S_LDR_HDR)), &ret_size);
      break;
    case 1:
      ret = do_decomp_bz2((void *) ldr_hdr->location, (void *) ((bit32u) source + sizeof(struct S_LDR_HDR)), &ret_size);
      break;
    default:
      win_printf(main_win, "...Unknown decompression type found\n");
      ret = FALSE;
  }
  
  return (ret) ? ret_size : 0;
}
