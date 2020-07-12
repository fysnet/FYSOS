//////////////////////////////////////////////////////////////////////////
//  ctype.h  v1.00
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS_CTYPE
#define FYSOS_CTYPE

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
typedef   signed long long  bit64s;
typedef unsigned long long  bit64u;




#endif // FYSOS_CTYPE
