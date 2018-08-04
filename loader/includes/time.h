
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
