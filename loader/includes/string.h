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

#ifndef STRING_H
#define STRING_H

char *strchr(char *, int);
unsigned strlen(char *);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, unsigned);
char *strcat(char *, const char *);

void *memset(void *, const bit8u, int);
void *memcpy(void *, const void *, int);
void *memmove(void *, void *, int);
int strcmp(const char *, const char *);
int stricmp(const char *, const char *);


#endif  // STRING_H
