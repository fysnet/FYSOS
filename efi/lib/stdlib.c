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
 */


#include "config.h"
#include "ctype.h"


void add64(void *targ, void *src) {
  asm (
    "  mov  edi,[ebp+ 8]    \n"
    "  mov  esi,[ebp+12]    \n"
    "  mov  eax,[esi + 0]   \n"
    "  add  [edi + 0],eax   \n"
    "  mov  eax,[esi + 4]   \n"
    "  adc  [edi + 4],eax   \n"
  );  
}

// Left shift 64bit by 32bit and get a 64bit result
void shl64(void *targ, int count) {
  asm (
    "  mov  esi,[ebp+ 8]    \n"
    "  mov  ecx,[ebp+12]    \n"
    "  mov  eax,[esi + 0]   \n"
    "  mov  edx,[esi + 4]   \n"
    "  and  ecx, 63         \n"
    "  shld edx,eax,cl      \n"
    "  shl  eax,cl          \n"
    "  cmp  ecx,32          \n"
    "  jb   short shl64_d   \n"
    "  mov  edx,eax         \n"
    "  xor  eax,eax         \n"
    "shl64_d:               \n"
    "  mov  [esi + 0],eax   \n"
    "  mov  [esi + 4],edx   \n"
  );  
}

void freeze(void) {
  while (1)
    asm (" hlt \n");
}
