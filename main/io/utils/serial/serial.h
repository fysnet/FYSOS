/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Input and Output Devices, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#define  SER_NUM     4  // we check for 4 serial ports

#define  SER_DIVISOR_LSB  0x000
#define  SER_DIVISOR_MSB  0x001
#define  SER_RECEIVE      0x000
#define  SER_TRANSMIT     0x000
#define  SER_INT_ENABLE   0x001
#define  SER_INT_ID       0x002
#define  SER_ALT_FUNC     0x002
#define  SER_LINE_CNTRL   0x003
#define  SER_FIFO         0x003
#define  SER_MODEM_CNTRL  0x004
#define  SER_LINE_STATUS  0x005
#define  SER_MODEM_STATUS 0x006
#define  SER_SCRATCH      0x007
#define  SER_SPECIAL      0x007

#define SER_DIV_LATCH_ON  0x80  // used to turn reg 0,1 into divisor latch

#define SER_BAUD_1200  96   // baud rate divisors for 1200 baud - 19200
#define SER_BAUD_2400  48
#define SER_BAUD_9600  12
#define SER_BAUD_19200  6

#define SER_STOP_1      0     // 1 stop bit per character
#define SER_STOP_2      4     // 2 stop bits per character

#define SER_BITS_5      0     // send 5 bit characters
#define SER_BITS_6      1     // send 6 bit characters
#define SER_BITS_7      2     // send 7 bit characters
#define SER_BITS_8      3     // send 8 bit characters

#define SER_PARITY_NONE 0     // no parity
#define SER_PARITY_ODD  8     // odd parity
#define SER_PARITY_EVEN 24    // even parity

#define STR_LEN 64

// must be same as "char types[][]"
enum {
  SER_TYPE_NONE = 0,
  SER_TYPE_8250,
  SER_TYPE_8250A_16450,
  SER_TYPE_16C1450,
  SER_TYPE_16550_BAD_FIFO,
  SER_TYPE_16550AF_C_CF,
  SER_TYPE_16C1550,
  SER_TYPE_16552_DUAL,
  SER_TYPE_82510,
};

enum {
  MOUSE_SERIAL_0 = 0,
  MOUSE_SERIAL_1,
  MOUSE_SERIAL_2,
};

struct S_MOUSE_INFO {
  int    type;  // controller type
  int    cur_byte;
  int    buttons, packet_type;
  bool   wheel;
  bit16u base;
  int    irq_num;
  bit8u  ser_x, ser_y, ser_z, ser_buttons;
  
  bool   left;                 // 1 if pressed, 0 if not
  bool   mid;                  // 1 if pressed, 0 if not
  bool   right;                // 1 if pressed, 0 if not
  bit16s x;                    // value sent from int handler
  bit16s y;                    // value sent from int handler
  bit16s z;                    // value sent from int handler
};

void interrupt ser_mouse_irq();

void ser_set_dlab(const bit16u base, const bool set);
bool ser_iotest(const bit16u port, const bit8u val);
bool ser_can_pwrdown(const bit16u base);

bool find_serial_mouse(bit16u base, char *str);
void ser_activate(const bit16u base, const bool activate);

void mdelay(const int ms);

