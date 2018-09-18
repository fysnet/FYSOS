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
 * Last update:  18 Sept 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

#include "ctype.h"

#include "conio.h"
#include "windows.h"
#include "sys.h"

#include "stdio.h"

void *MK_FP(const void *pair) {
	return (void *) (((* (bit32u *) pair & 0xFFFF0000) >> 12) + (* (bit32u *) pair & 0x0000FFFF));
}

bit16u MK_SEG(const bit32u addr) {
  return (bit16u) ((addr & 0x000FFFF0) >> 4);
}

bit16u MK_OFF(const bit32u addr) {
  return (bit16u) (addr & 0xF);
}

void freeze(void) {
  asm (
    "halt: \n"
    "  hlt\n"
    "  jmp short halt\n"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Call an interrupt
// Return TRUE = carry set
// Note: On 80486's and before, for Self-modifying code, we need to do a serialization instruction
//  after the write and before the instruction that is written to.  One of the instructions that can
//  do this is the CPUID instruction.  However, we have not checked to see if this machine has this
//  instruction yet, so we must do something else.  An unconditional jump should work just fine.
bool intx(int i, struct REGS *regs) {
  asm (
    "  push ds                ; \n"  // save ds
    "  push es                ; \n"  // save es
    "  mov  eax,[ebp+8]       ; self modify code to interrupt number\n"
    "  mov [dword .intx1 + 1], al\n"
    "  jmp .next_instruct     ; \n"  // self mod code needs serialization...
    ".next_instruct:          ; \n"
    "  mov  esi,[ebp+12]      ; \n"
    "  push ebp               ; \n"  // save ebp
    "  push esi               ; \n"  // save the pointer to our regs data
    "  mov  eax,[esi+ 0]      ; \n"
    "  mov  ebx,[esi+ 4]      ; \n"
    "  mov  ecx,[esi+ 8]      ; \n"
    "  mov  edx,[esi+12]      ; \n"
    "  mov  edi,[esi+20]      ; \n"
    "  mov  ebp,[esi+24]      ; \n"
    "  mov  es,[esi+34]       ; \n"
    "  mov  ds,[esi+32]       ; \n"
    "  mov  esi,[fs:esi+16]   ; \n"  // fs should already be zero (flat at 0x00000000)
    ".intx1:                  ; \n"
    "  int  0                 ; zero will be replaced with value above \n"
    "  xchg esi,[esp]         ; swap the esi values (before and after values) \n"
    "  mov  [fs:esi+ 0],eax   ; \n"
    "  mov  [fs:esi+ 4],ebx   ; \n"
    "  mov  [fs:esi+ 8],ecx   ; \n"
    "  mov  [fs:esi+12],edx   ; \n"
    "  pop  eax               ; save the new esi value\n"
    "  mov  [fs:esi+16],eax   ; \n"  // esi
    "  mov  [fs:esi+20],edi   ; \n"
    "  mov  [fs:esi+24],ebp   ; \n"
    "  mov  [fs:esi+32],ds    ; \n"
    "  mov  [fs:esi+34],es    ; \n"
    "  pushfd                 ; retrieve eflags \n"
    "  pop  eax               ;  into eax \n"
    "  mov  [fs:esi+28],eax   ; then into regs->eflags \n"
    "  pop  ebp               ; \n"
    "  pop  es                ; \n"
    "  pop  ds                ; \n"
  );
  
  return (bool) (regs->eflags & 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Checks for a 486+ machine with the CPUID and RDTSC instructions.
//  on entry: nothing
//  on exit:
//    returns
//         0 = 8086, 1 = 186, 2 = 286, (see note)
//         3 = 386+
//         4 = 486+ without CPUID, 
//         5 = 486+ with CPUID but not RDTSC,
//         6 = 486+ with CPUID and RDTSC
//  Note: we have already checked for at least an 80x386 via the boot code
//   that loaded this loader.sys file.  i.e.: it won't return less than '3'
//   since if it was, wouldn't have gotten this far anyway.
int chk_486(void) {
  asm (
    "  pushf                   ; save the original flags value\n"
    "  \n"
    "  mov  ax,00h             ; Assume an 8086\n"
    "  mov  cx,0121h           ; If CH can be shifted by 21h,\n"
    "  shl  ch,cl              ; then it's an 8086, because\n"
    "  jz   short chk486done   ; a 186+ limits shift counts.\n"
    "  push sp                 ; If SP is pushed as its\n"
    "  pop  ax                 ; original value, then\n"
    "  cmp  ax,sp              ; it's a 286+.\n"
    "  mov  ax,01h             ; is 186\n"
    "  jne  short chk486done   ;\n"
    "  mov  ax,7000h           ; if bits 12,13,14 are still set\n"
    "  push ax                 ; after pushing/poping to/from\n"
    "  popf                    ; the flags register then we have\n"
    "  pushf                   ; a 386+\n"
    "  pop  ax                 ;\n"
    "  and  ax,7000h           ;\n"
    "  cmp  ax,7000h           ;\n"
    "  mov  ax,02h             ; is 286\n"
    "  jne  short chk486done   ; it's a 386\n"
    "  \n"
    "  ; =-=-=-=-=- test for .486\n"
    "  ; if we can toggle bit 18 in EFLAGS (AC bit) we have a\n"
    "  ;  486+.  The 386 doesn't have the AC bit.\n"
    "  cli\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  mov  ebx,eax\n"
    "  xor  eax,00040000h      ; bit 18\n"
    "  push eax\n"
    "  popfd\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  push ebx\n"
    "  popfd\n"
    "  sti\n"       
    "  xor  eax,ebx\n"
    "  mov  ax,03              ; is 386\n"
    "  jz   short chk486done   ; else it's a 486+\n"
    "  \n"
    "  ; =-=-=-=-=- test for CPUID\n"
    "  ; if we can toggle bit 21 in EFLAGS (ID bit) we have a\n"
    "  ;  486+ with the CPUID instruction\n"
    "  cli\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  mov  ebx,eax\n"
    "  xor  eax,00200000h      ; bit 21\n"
    "  push eax\n"
    "  popfd\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  push ebx\n"
    "  popfd\n"
    "  sti\n"       
    "  xor  eax,ebx\n"
    "  mov  ax,04              ; is 486+ without CPUID\n"
    "  jz   short chk486done   ; else it's a 486+ with CPUID\n"
    "  \n"
    "  ; =-=-=-=-=- test for RDTSC\n"
    "  ; do a CPUID with function 1.  If bit 4 of EDX on return,\n"
    "  ;  we have the RDTSC instruciton\n"
    "  mov  eax,1\n"
    "  cpuid\n"
    "  test edx,10h\n"
    "  mov  ax,05              ; is 486+ with CPUID but without RDTSC\n"
    "  jz   short chk486done\n"
    "  \n"
    "  ; =-=-=-=-=- We got a 486+ with the CPUID and RDTSC instructions\n"
    "  mov  ax,06              ; is 486+ with CPUID and RDTSC\n"
    "  \n"
    "chk486done: \n"
    "  movzx eax,ax \n" 
    "  popf                    ; restore the original flags value\n"
  );
}

// adds two 64-bit values storing the sum in the targ location
void add64(void *targ, void *src) {
  asm (
    "  mov  edi,[ebp+8]     \n"
    "  mov  esi,[ebp+12]    \n"
    "  mov  eax,[esi + 0]   \n"
    "  add  [edi + 0],eax   \n"
    "  mov  eax,[esi + 4]   \n"
    "  adc  [edi + 4],eax   \n"
  );  
}

void get_bios_equ_list(bit16u *bios_equip, bit8u *kbd_bits) {
  *bios_equip = * (bit16u *) 0x00410;
  *kbd_bits = * (bit8u *) 0x00417;
}

// reads the current value of the Time Stamp Counter
bit32u read_tsc(void) {
#if ALLOW_SMALL_MACHINE
  return 0;
#else
  asm (" rdtsc ");
  // at this point, edx contains the high order dword
  // however, we have a 32-bit compiler, so it will be
  //  ignored on return.
#endif
}

// disable interrupts  (using 'cli')
bool disable_ints(void) {
  asm (
    " pushfd      \n"
    " cli         \n"
    " pop  eax    \n"
    " and  eax,0x00000200 \n"
    " shr  eax,9  \n"
  );
}

// enable interrupts  (using 'sti')
bool enable_ints(void) {
  asm (
    " pushfd      \n"
    " pop  eax    \n"
    " and  eax,0x00000200 \n"
    " shr  eax,9  \n"
    " sti         \n"
  );
}

// restore interrupts  (using 'sti')
bool restore_ints(const bool state) {
  if (state) 
    return enable_ints();
  return disable_ints();
}

// hook a BIOS vector
void *hook_vector(const int i, const void *addr) {
  void *old_isr;
  bool ints;
  
  ints = disable_ints();
  
  old_isr = (void *) * (bit32u *) (i * sizeof(bit32u));
  * (bit32u *) (i * sizeof(bit32u)) = (((bit32u) addr & 0x000FFFF0) << 12) | ((bit32u) addr & 0xF);
  
  restore_ints(ints);
  
  return old_isr;
}

void *old_isr9 = NULL;
bool spc_key_F1 = FALSE,  // help screen
     spc_key_F2 = FALSE,  // verbose
     spc_key_F3 = FALSE,  // nothing at this time
     spc_key_F4 = FALSE,  // nothing at this time
     spc_key_F5 = FALSE,  // nothing at this time
     spc_key_F6 = FALSE,  // nothing at this time
     spc_key_F7 = FALSE,  // nothing at this time
     spc_key_F8 = FALSE,  // choose screen mode
     spc_key_F9 = FALSE;  // stay in text mode

struct S_SPC_KEYS {
  bit8u keycode;
  bool *flag;
  bool toggle;  // TRUE = toggle it, else just set it
} spc_keys[] = {
  { 0x3B, &spc_key_F1, FALSE },
  { 0x3C, &spc_key_F2, TRUE },
  { 0x3D, &spc_key_F3, FALSE },
  { 0x3E, &spc_key_F4, FALSE },
  { 0x3F, &spc_key_F5, FALSE },
  { 0x40, &spc_key_F6, FALSE },
  { 0x41, &spc_key_F7, FALSE },
  { 0x42, &spc_key_F8, FALSE },
  { 0x43, &spc_key_F9, FALSE },  // this one should not be able to toggle it... Leave at FALSE
  { 0, NULL }
};

bool allow_spc_keys = TRUE;

void keyboard_isr(void) {
  // save all registers used
  asm (
    " pushad       \n"
    " push  ds     \n"
    " xor  ax,ax   \n"
    " mov  ds,ax   \n"
  );
  
  // if a special key is pressed (F8 for example), set its flag
  // this assumes we are using scan code set 2 ????
  int i = 0;
  if (allow_spc_keys) {
    bit8u keycode = inpb(0x60);  // catch all key releases too. (bit 7 set)
    while (spc_keys[i].flag) {
      if (spc_keys[i].keycode == (keycode & 0x7F)) {
        // only change the flag on the release of the key
        // ignore the press, but return, not allowing the BIOS to capture the key
        if (keycode & 0x80) {
          if (spc_keys[i].flag) {
            if (help_scr) {
              // we ignore the F1 key press in the help screen
              if (spc_keys[i].keycode != 0x3B) {
                // if showing help screen, we toggle the function
                *spc_keys[i].flag ^= TRUE;
                win_status_update(main_win, "Press a key to return", FALSE);
                win_title_update(main_win, ((struct S_WINDOW *) main_win)->title);
              }
            } else {
              // outside of help screen, we just set it or toggle it
              if (spc_keys[i].toggle)
                *spc_keys[i].flag ^= TRUE;
              else
                *spc_keys[i].flag = TRUE;
            }
          }
        }
        
        // don't let the BIOS handler have this one.  Return to code...
        asm (
          "  mov  al,20h       \n"  // end of interrupt
          "  out  20h,al       \n"  //  ...
          "  pop  ds           \n"  // 
          "  popad             \n"  // restore all registers used
          "  add  sp,8         \n"  // remove the 'keycode' and 'i' local parameters
          "  pop  ebp          \n"  // restore ebp
          "  iret              \n"  // return from handler
        );
      }
      i++;
    }
  }
  
  // else, it was any other key, so pass it on to the BIOS' handler
  asm (
    "  pop  ds           \n"  //
    "  popad             \n"  // restore all registers used
    "  add  sp,8         \n"  // remove the 'keycode' and 'i' local parameters
    "  pop  ebp          \n"  // restore ebp
    "  sub  sp,4         \n"  // make room for the seg:off
    "  push eax          \n"  // save eax
    "  mov  eax,[dword fs:_old_isr9]  \n"  // must have the 'dword' operand or SmallerC won't create a relocation for it
    "  mov  [esp+4],eax  \n"  // put the seg:eax in the room we allocated
    "  pop  eax          \n"  // restore eax
    "  retf              \n"  // jump (removing 4 bytes from the stack)
  );
}
