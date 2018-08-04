
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
