/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
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

#define ENDIAN_16(x)   ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))

// I like the 'b' added
#define inpb  inp
#define outpb outp

// standard true and false
#define TRUE   1
#define FALSE  0

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
#ifdef _MSC_VER
  #if _MSC_VER > 600  // QC2.5 is 600 and doesn't support 64 bits :-)
    typedef   signed  _int64    bit64s;  // 64 bit signed quad word
    typedef unsigned  _int64    bit64u;  // 64 bit unsigned quad word
    #define LL64BIT "I64"
  #endif
#elif defined DJGPP
  typedef   signed  long long  bit64s;  // 64 bit signed quad word
  typedef unsigned  long long  bit64u;  // 64 bit unsigned quad word
  #define LL64BIT "ll"
#else
# error 64 bit types not defined
#endif

#endif // FYSOS_CTYPE
