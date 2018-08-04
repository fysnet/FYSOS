#ifndef _PARAPORT_H
#define _PARAPORT_H

#define TIMEOUT_VAL  256

void para_putch(const bit8u);
bit16u para_printf(const char *str, ...);

#endif  // _PARAPORT_H
