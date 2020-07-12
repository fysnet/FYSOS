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

/*
 * Shared with ps2mouse.c and ps2key.c
 */

#ifndef FYSOS_PS2SHARE
#define FYSOS_PS2SHARE

#define KEYBOARD_DATA  0x60
#define KEYBRD_STATUS  0x64
#define KEYBRD_CMND_60 0x60  // Commands go to the keyboard
#define KEYBRD_CMND_64 0x64  // Commands go to the keyboard controller

// These commands go to KEYBRD_CMND_64
#define KEY_CMD_READ       0x20   // Read Command byte
                   // 0x20-0x3F   // Read Controller RAM
#define KEY_CMD_WRITE      0x60   // Write Command byte
                   // 0x60-0x7F   // Write Controller RAM
#define KEY_CMD_FIRM_VER   0xA1   //?Controller firmware version
#define KEY_CMD_HAS_PASS   0xA4   // Check if Password installed
#define KEY_CMD_LOAD_PASS  0xA5   // Load Password
#define KEY_CMD_CHK_PASS   0xA6   // Check Password
#define KEY_CMD_DIS_SECOND 0xA7   // Disable second Port
#define KEY_CMD_EN_SECOND  0xA8   // Enable second Port
#define KEY_CMD_TEST_MP    0xA9   // Test second Port
#define KEY_CMD_SELFTEST   0xAA   // Controller self test
#define KEY_CMD_INT_TEST   0xAB   // Controller interface test
#define KEY_CMD_DIAG_DUMP  0xAC   // Diagnostic Dump
#define KEY_CMD_DIS_KEYB   0xAD   // Disable first port
#define KEY_CMD_EN_KEYB    0xAE   // Enable first port
#define KEY_CMD_VERSION    0xAF   //?Controller version
#define KEY_CMD_READ_INP   0xC0   // Read input port
#define KEY_CMD_CONT_INPL  0xC1   // Continuous input port poll, low
#define KEY_CMD_CONT_INPH  0xC2   // Continuous input port poll, high
#define KEY_CMD_GET_MODE   0xCA   //?Get the mode (bit 0 = 0 = ISA (AT) interface, 1 = PS/2 (MCA) interface
#define KEY_CMD_SET_MODE   0xCB   //?Set the mode (read with CA, then modify bit 0, and write it back)
#define KEY_CMD_READ_OUTP  0xD0   // Read output port
#define KEY_CMD_WRTE_OUTP  0xD1   // Write output port
#define KEY_CMD_WRTE_OUT   0xD2   // Write first output buffer
#define KEY_CMD_WRTE_OUTM  0xD3   // Write second output buffer
#define KEY_CMD_SECOND_PRE 0xD4   // 2nd port prefix (write to 2nd port)
#define KEY_CMD_DIS_A20    0xDD   // Disable A20 line
#define KEY_CMD_EN_A20     0xDF   // Enable A20 line
#define KEY_CMD_READ_TEST  0xE0   // Read Test Inputs
                   // 0xF0-0xFD   // Pulse output bits
#define KEY_CMD_RESET_SYS  0xFE   // System Reset
                                  //? = I have not confirmed this command

// These commands go to KEYBRD_CMND_60
#define KEY_SET_READ_EXT_ID   0xD0   // ?read extended ID (up to 256 bytes)
                      // 0xD1-0xDF   // ?Vendor Unique Commands
#define KEY_READ_2ND_ID       0xE1   // ?read secondary ID (two bytes)
#define KEY_SET_SCALE1        0xE6   //  set mouse scale 1:1
#define KEY_SET_SCALE2        0xE7   //  set mouse scale 2:1
#define KEY_SET_RES           0xE8   //  set mouse resolution
#define KEY_GET_MOUSE_INFO    0xE9   //  get current status information
#define KEY_GET_STREAM_MODE   0xEA   //  get stream mode
#define KEY_PACKET            0xEB   // ?packet (in remote mode)
#define KEY_RESET_WRAP_MODE   0xEC   //  reset wrap mode
#define KEY_LED_WRITE         0xED   //  set the state of the LED's
#define KEY_DIAG_ECHO         0xEE   //  diagnostic echo
#define KEY_GETSET_SCAN_CODE  0xF0   //  get/set Scan Code
#define KEY_CMD_ID            0xF2   //  ID
#define KEY_CMD_TPRATE        0xF3   //  set rate
#define KEY_CMD_ENABLE        0xF4   //  enable
#define KEY_CMD_RESDIS        0xF5   //  disable keyboard
#define KEY_CMD_RESDIS_MOUSE  0xF6   //  disable mouse
#define KEY_CMD_TYPEMATIC     0xF7   //  set all keys to typematic
#define KEY_CMD_MAKE_REL      0xF8   //  set all keys to make/release
#define KEY_CMD_MAKE          0xF9   //  set all keys to make
#define KEY_CMD_TYPE_MAKE_REL 0xFA   //  set all keys to typematic/make/release
#define KEY_CMD_ATYPEMATIC    0xFB   //  set a key to typematic
#define KEY_CMD_AMAKE_REL     0xFC   //  set a key to make/release
#define KEY_CMD_AMAKE         0xFD   //  set a key to make only
#define KEY_CMD_RESEND        0xFE   //  resend last scan code
#define KEY_CMD_RESET         0xFF   //  reset
                                     // ?= I have not confirmed this command

#define KEYBOARD_ACK       0xFA   // ACKnowledge
#define KEYBOARD_ERROR     0xFC   // ERROR

#define KEY_MODE_AT           0   // is older style (ISA) AT mode
#define KEY_MODE_PS2          1   // is newer style (MCA) PS/2 mode

int time_out = 8000;  // timeout delay count (default to 8000)
int ps2_type = 1;  // 1 = type1, 2 = type2

// This is a simple delay that uses no interrupts or
//  outside help.
void io_delay(const int max) {
  int i;

  for (i=INT_MIN; i<max; i++)
    ;
}

// read a byte from the keyboard (60h)
// waits for bit 0 in 64h to be 1 or timer to run out
bit8u keyboard_read(void) {
  int timer = time_out;
  
  while (timer) {
    if (inpb(KEYBRD_STATUS) & 1)
      return inpb(KEYBOARD_DATA);
    io_delay(INT_MAX);
    timer--;
  }
  
  return KEYBOARD_ERROR;
}

// clears any bytes "waiting" in the Output buffer
// will wait a short period of time before returning "all clear"
void keyboard_clear(void) {
  io_delay(INT_MAX);
  while (inpb(KEYBRD_STATUS) & 1) {
    inpb(KEYBOARD_DATA);
    io_delay(INT_MAX);
  }
}

// waits for the keyboard's Input buffer to be
//  ready for a write, returning TRUE when ready
//  or FALSE after timeout.
bool keyboard_write_ready(void) {
  int timer = time_out;
  
  while (timer) {
    if (!(inpb(KEYBRD_STATUS) & 2))
      return TRUE;
    io_delay(INT_MAX);
    timer--;
  }
  return FALSE;
}

// write a byte to the keyboard (60h or 64h)
bit8u keyboard_write(const bit8u port, const bit8u ch) {
  if (keyboard_write_ready()) {
    outpb(port, ch);
    if (keyboard_write_ready())
      return ch;
  }
  
  return KEYBOARD_ERROR;
}

// set is_second to TRUE if sending to the second channel
bool keyboard_cmnd60(const bit8u ch, const bool is_second) {
  if (is_second)
    if (keyboard_write(KEYBRD_CMND_64, KEY_CMD_SECOND_PRE) != KEY_CMD_SECOND_PRE)
      return FALSE;
  if (keyboard_write(KEYBRD_CMND_60, ch) == ch)
    return (keyboard_read() == KEYBOARD_ACK);
  return FALSE;
}

// detect how many ps/2 port is available
// returns in bit form:
//  bit 0 = 1 = first port found and available
//  bit 1 = 1 = second port found and available
// Note: This works on both (ISA) AT and (MCA) PS/2 style ports since
//  the bits toggled in the Config byte are the same for each.
bit8u det_ps2_port(void) {
  bit8u byte, byte1;
  bool first = 0, second = 0;
  
  // Disable each channel's clock line
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_DIS_KEYB);
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_DIS_SECOND);
  
  // make sure the Output buffer is clear
  inpb(KEYBOARD_DATA);
  
  // modify Config Byte
  if (keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ) != KEY_CMD_READ)
    return 0; // no ports found
  byte = keyboard_read();
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
  // disable interrupts on both ports, and disable translation
  keyboard_write(KEYBRD_CMND_60, (bit8u) (byte & ~((1<<6) | (1<<1) | (1<<0))));
  
  // test the controller with the SelfTest command
  // send 0xAA and controller should return 0x55
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_SELFTEST);
  if (keyboard_read() != 0x55)
    return 0;  // none
  
  // check to see if there are two ports (channels)
  // if bit 5 of the Config Byte was clear, we don't have a second port since
  //  it should now be disabled, which will set bit 5.
  if (!(byte & (1<<5)))
    second = 0; // second port is not available
  else {
    // else, enable the port and check bit 5 again.
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_SECOND);
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
    byte = keyboard_read();
    // if it is clear, we have a second port.
    if (!(byte & (1<<5))) {
      second = 1;  // second port is available
      keyboard_write(KEYBRD_CMND_64, KEY_CMD_DIS_SECOND);
    }
  }
  
  // interface tests
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_INT_TEST);
  byte = keyboard_read();
  if (byte > 0) {
    printf("Controller did not pass interface test for first port. (%i)\n", byte);
    first = 0;
  } else
    first = 1;
  if (second) {
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_TEST_MP);
    byte = keyboard_read();
    if (byte > 0) {
      printf("Controller did not pass interface test for second port. (%i)\n", byte);
      second = 0;
    }
  }
  
  // check to see if it is a type 1 or type 2 controller
  // Only type 1 controllers will allow us to set the
  //  translation bit (bit 6) in the Configuration byte
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
  byte = keyboard_read();
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
  keyboard_write(KEYBRD_CMND_60, (bit8u) (byte | (1<<6)));  // set translation bit
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
  byte1 = keyboard_read();
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
  keyboard_write(KEYBRD_CMND_60, byte);
  ps2_type = (byte1 & (1<<6)) ? 1 : 2;  // if bit is set, we have type 1, else type 2
  
  // Now re-enable the interrupts in the Config Byte
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
  byte = keyboard_read();
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
  // enable interrupts on both ports and re-enable translation 
  // (so that the keyboard remains available for our tests in FreeDOS,
  //   we will re-activate that IRQ as well. (bit 0))
  keyboard_write(KEYBRD_CMND_60, (bit8u) (byte | ((1<<6) | (second << 1) | (first << 0))));
  
  // Enable each channel's clock line
  if (first)
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_KEYB);
  if (second)
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_SECOND);
  
  return ((second << 1) | first);
}

#endif // FYSOS_PS2SHARE
