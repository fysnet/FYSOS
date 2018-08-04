#ifndef CONIO_H
#define CONIO_H

 bit8u inpb(int);
bit16u inpw(int);
bit32u inpd(int);
  void outpb(int, bit8u);
  void outpw(int, bit16u);
  void outpd(int, bit32u);

  bool kbhit(void);
bit16u getscancode(void);

#endif  // CONIO_H
