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

#ifndef _STDIO_H
#define _STDIO_H


#pragma pack(push, 1)


#pragma pack(pop)


typedef char *va_list;

bit16u vsprintf(const char *, const char *, va_list);
bit16u sprintf(char *, const char *, ...);

bit16u printf(const char *, ...);
int putchar(const int);
int puts(const char *);

void set_vector(const int, const bit32u);



#endif  // _STDIO_H
