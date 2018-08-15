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
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */

#ifndef _APM_H
#define _APM_H

#pragma pack(push, 1)

struct S_APM {
  bit8u  present;
  bit8u  initialized;
  bit8u  maj_ver;
  bit8u  min_ver;
  bit16u flags;
  bit8u  batteries;
  bit16u cap_flags;
  bit16u error_code;
  bit8u  resv0[5];
  bit16u cs_segment32;
  bit32u entry_point;
  bit16u cs_segment;
  bit32u cs16_gdt_idx;
  bit32u cs32_gdt_idx;
  bit32u ds_gdt_idx;
  bit16u ds_segment;
  bit16u cs_length32;
  bit16u cs_length16;
  bit16u ds_length;
};

#pragma pack(pop)

void apm_bios_check(struct S_APM *);

#endif   // _APM_H
