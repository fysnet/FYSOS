
#include "ctype.h"
#include "sys.h"

#include "conio.h"

bit8u inpb(int d) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  xor  eax,eax     \n"
    "  in   al,dx       \n"
  );
}

bit16u inpw(int d) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  xor  eax,eax     \n"
    "  in   ax,dx       \n"
  );
}

bit32u inpd(int d) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  in   eax,dx      \n"
  );
}

void outpb(int d, bit8u v) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  mov  eax,[ebp+12] \n"
    "  out  dx,al       \n"
  );
}

void outpw(int d, bit16u v) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  mov  eax,[ebp+12] \n"
    "  out  dx,ax       \n"
  );
}

void outpd(int d, bit32u v) {
  asm (
    "  mov  edx,[ebp+8] \n"
    "  mov  eax,[ebp+12] \n"
    "  out  dx,eax      \n"
  );
}

bool kbhit(void) {
  struct REGS regs;
  
  regs.eax = 0x00000100;
  intx(0x16, &regs);
  return (regs.eflags & (1<<6)) == 0;
}

bit16u getscancode(void) {
  struct REGS regs;
  
  regs.eax = 0x00000000;
  intx(0x16, &regs);
  return (bit16u) (regs.eax & 0x0000FFFF);
}

int isdigit(int c) {
  //return __chartype__[c + 1] & 0x08;
  return ((c >= '0') && (c <= '9'));
}

int toupper(int c) {
  //return c - (__chartype__[c + 1] & 0x20);
  if ((c >= 'a') && (c <= 'z'))
    return (c - ('a' - 'A'));
  return c;
}

int tolower(int c) {
  //return c + ((__chartype__[c + 1] & 0x10) << 1);
  if ((c >= 'A') && (c <= 'Z'))
    return (c + ('a' - 'A'));
  return c;
}
