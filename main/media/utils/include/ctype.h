/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
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

#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))

#define ENDIAN_16U(x)  ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_32U(x)  ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8) | (((x) & 0xFF000000) >> 24))
#define ENDIAN_64U(x)  (                                              \
                        ((bit64u) ((x) & 0x00000000000000FF) << 56) | \
                        ((bit64u) ((x) & 0x000000000000FF00) << 40) | \
                        ((bit64u) ((x) & 0x0000000000FF0000) << 24) | \
                        ((bit64u) ((x) & 0x00000000FF000000) <<  8) | \
                        ((bit64u) ((x) & 0x000000FF00000000) >>  8) | \
                        ((bit64u) ((x) & 0x0000FF0000000000) >> 24) | \
                        ((bit64u) ((x) & 0x00FF000000000000) >> 40) | \
                        ((bit64u) ((x) & 0xFF00000000000000) >> 56)   \
                       )

// standard true and false
#define TRUE   1
#define FALSE  0

#if defined(DJGPP) 
  #ifndef __cplusplus
    typedef unsigned  char      bool;
  #endif
#endif

// size of memory operands
typedef   signed  char      bit8s;
typedef unsigned  char      bit8u;
typedef   signed short      bit16s;
typedef unsigned short      bit16u;
typedef   signed  long      bit32s;
typedef unsigned  long      bit32u;
#ifdef _MSC_VER
  typedef   signed  _int64    bit64s;  // Microsoft specific
  typedef unsigned  _int64    bit64u;  // Microsoft specific
  #define LL64BIT "I64"
#elif defined DJGPP
  typedef   signed  long long  bit64s;
  typedef unsigned  long long  bit64u;
  #define LL64BIT "ll"
#else
# error 64 bit types not defined
#endif

#endif // FYSOS_CTYPE
