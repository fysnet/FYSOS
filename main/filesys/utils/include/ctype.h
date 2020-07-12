/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2014
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: The Virtual File System, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef FYSOS_CTYPE
#define FYSOS_CTYPE

#define ENDIAN_16(x)   ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_32(x)   ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | \
                        (((x) & 0xFF0000) >> 8) | (((x) & 0xFF000000) >> 24))

// if (x) is a power of 2, return TRUE
#define POWERofTWO(x) (!((x) & ((x) - 1)) && (x))

// standard true and false
#define TRUE   1
#define FALSE  0

#if defined(DJGPP)
  typedef unsigned  char      bool;
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
#elif defined DJGPP
  typedef   signed  long long  bit64s;
  typedef unsigned  long long  bit64u;
#else
# error 64 bit types not defined
#endif

#endif // FYSOS_CTYPE
