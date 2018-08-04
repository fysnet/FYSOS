
#ifndef _CTYPE_H
#define _CTYPE_H

// standard true and false
#define TRUE   1
#define FALSE  0
#define NULL   ((void *) 0)

// size of memory operands
typedef   signed  char      bool;
typedef   signed  char      bit8s;
typedef unsigned  char      bit8u;
typedef   signed short      bit16s;
typedef unsigned short      bit16u;
typedef   signed  long      bit32s;
typedef unsigned  long      bit32u;

int isdigit(int);
int tolower(int);
int toupper(int);

// SmallerC doesn't have 64-bit stuff yet
#define SIZEOFQUAD  8

#endif  // _CTYPE_H
