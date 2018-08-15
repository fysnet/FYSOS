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
