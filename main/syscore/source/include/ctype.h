/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2019
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The System Core, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef FYSOS_CTYPE
#define FYSOS_CTYPE

// standard true and false
#define TRUE   1
#define FALSE  0
#define NULL   ((void *) 0)

#if defined(DJGPP)
  typedef unsigned  char      bool;
#elif defined(_NBC_)
  typedef unsigned  char      bool;
#endif

// size of memory operands
typedef   signed  char      bit8s;     // 8 bit signed char
typedef unsigned  char      bit8u;     // 8 bit unsigned char
typedef   signed short      bit16s;    // 16 bit signed word
typedef unsigned short      bit16u;    // 16 bit unsigned word
typedef   signed  long      bit32s;    // 32 bit signed double word
typedef unsigned  long      bit32u;    // 32 bit unsigned double word
typedef   signed  _int64    bit64s;  // Microsoft specific
typedef unsigned  _int64    bit64u;  // Microsoft specific

typedef unsigned short      wchar_t;

#endif // FYSOS_CTYPE
