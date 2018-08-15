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

#ifndef _TIME_H
#define _TIME_H

#pragma pack(push, 1)

struct S_TIME {
  bit16u year;
  bit8u  month;
  bit8u  day;
  bit8u  hour;
  bit8u  min;
  bit8u  sec;
  bit8u  jiffy;
  bit16u msec;
  bit8u  d_savings;
  bit8u  weekday;
  bit16u yearday;
};

#pragma pack(pop)

void get_bios_time(struct S_TIME *);

#endif   // _TIME_H
