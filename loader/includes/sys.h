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

#ifndef _SYS_H
#define _SYS_H

#pragma pack(push, 1)

// ex: 3F2504E0-4F89-11D3-9A0C-0305E82C3301
//       data1    2    3    4     5[6]
struct S_GUID {
  bit32u data1;
  bit16u data2;
  bit16u data3;
  bit16u data4;
  bit8u  data5[6];
};

struct REGS {
  bit32u eax;    //  0
  bit32u ebx;    //  4
  bit32u ecx;    //  8
  bit32u edx;    // 12
  bit32u esi;    // 16
  bit32u edi;    // 20
  bit32u ebp;    // 24
  bit32u eflags; // 28
  bit16u ds;     // 32
  bit16u es;     // 34
};

struct S_GDT {
  bit16u limitlow;             // low word of limit
  bit16u baselow;              // low word of base
  bit8u  basemid;              // mid byte of base
  bit8u  flags0;               // 
  bit8u  flags1;               // 
  bit8u  basehigh;             // high byte of base
};

#pragma pack(pop)


void *MK_FP(void *);
bit16u MK_SEG(const bit32u);
bit16u MK_OFF(const bit32u);

void freeze(void);
bool intx(const int, struct REGS *);
int chk_486(void);

void add64(void *targ, void *src);

void get_bios_equ_list(bit16u *, bit8u *);

bool disable_ints(void);
bool enable_ints(void);
bool restore_ints(const bool);

void *hook_vector(const int, const void *);

extern bool allow_spc_keys;
extern bool spc_key_F1, spc_key_F2, spc_key_F3,
            spc_key_F4, spc_key_F5, spc_key_F6,
            spc_key_F7, spc_key_F8, spc_key_F9;
extern void *old_isr9;
void keyboard_isr(void);

#endif  // _SYS_H
