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
 */

#include "ctype.h"

#include "apm.h"
#include "paraport.h"
#include "string.h"
#include "sys.h"
#include "windows.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// 
void apm_bios_check(struct S_APM *apm) {
  struct REGS regs;
  
  // clear it out
  memset(apm, 0, sizeof(struct S_APM));
  
  regs.eax = 0x00005300;
  regs.ebx = 0x00000000;
  if (!intx(0x15, &regs) && ((regs.ebx & 0xFFFF) == 0x504D)) {  // 'PM' == 0x504D
    apm->present = TRUE;
    apm->maj_ver = (bit8u) ((regs.eax & 0xFF00) >> 8);
    apm->maj_ver = (bit8u) ((regs.eax & 0x00FF) >> 0);
    apm->flags = (bit16u) (regs.ecx & 0xFFFF);
    
    // get capabilities
    regs.eax = 0x00005310;
    regs.ebx = 0x00000000;
    if (!intx(0x15, &regs)) {
      apm->batteries = (bit8u) (regs.ebx & 0xFF);
      apm->cap_flags = (bit16u) (regs.ecx & 0xFFFF);
      
      // now connect to the 32bit interface
      regs.eax = 0x00005303;
      regs.ebx = 0x00000000;
      if (!intx(0x15, &regs)) {
        apm->cs_segment32 = (bit16u) (regs.eax & 0xFFFF);
        apm->entry_point = regs.ebx;
        apm->cs_segment = (bit16u) (regs.ecx & 0xFFFF);
        apm->ds_segment = (bit16u) (regs.edx & 0xFFFF);
        apm->cs_length32 = (bit16u) (regs.esi & 0xFFFF);
        apm->cs_length16 = (bit16u) ((regs.esi >> 16) & 0xFFFF);
        apm->ds_length = (bit16u) (regs.edi & 0xFFFF);
      } else {
        apm->present = FALSE;
        apm->error_code = (bit16u) (regs.eax & 0xFFFF);   // ah = error_code, al = function
      }
    } else {
      apm->present = FALSE;
      apm->error_code = (bit16u) (regs.eax & 0xFFFF);   // ah = error_code, al = function
    }
  }
  
  if (spc_key_F2 && apm->present)
    para_printf("Found BIOS APM support.\n");
}
