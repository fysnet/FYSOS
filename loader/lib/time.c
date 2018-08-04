
#include "ctype.h"

#include "paraport.h"
#include "string.h"
#include "sys.h"
#include "time.h"
#include "windows.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// converts a bcd numeral to an integer
//
int bcd2dec(const int bcd) {
  return (((bcd & 0xF0) >> 4) * 10) + (bcd & 0x0F);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// get the BIOS time of day
void get_bios_time(struct S_TIME *time) {
  struct REGS regs;
  
  // clear it out
  memset(time, 0, sizeof(struct S_TIME));
  
  regs.eax = 0x00000400;
  intx(0x1A, &regs);
  time->year = (bcd2dec((regs.ecx & 0x0000FF00) >> 8) * 100) + 
                bcd2dec(regs.ecx & 0x000000FF);
  time->month = bcd2dec((regs.edx & 0x0000FF00) >> 8);
  time->day   = bcd2dec(regs.edx & 0x000000FF);
  
  regs.eax = 0x00000200;
  intx(0x1A, &regs);
  time->hour = bcd2dec((regs.ecx & 0x0000FF00) >> 8);
  time->min = bcd2dec(regs.ecx & 0x000000FF);
  time->sec = bcd2dec((regs.edx & 0x0000FF00) >> 8);
  time->d_savings = (regs.edx & 0x000000FF);
  
  time->jiffy = 0;  // todo
  time->weekday = 0; // todo
  
  if (spc_key_F2)
    para_printf("Time and Date: %04i/%02i/%02i  %02i:%02i:%02i.%02i\n",
      time->year, time->month, time->day,
      time->hour, time->min, time->sec, time->jiffy);
}
