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

#ifndef FYSOS_PS2KEY
#define FYSOS_PS2KEY

// LED flags
#define KEYBOARD_LED_SCRL  0x01
#define KEYBOARD_LED_NUM   0x02
#define KEYBOARD_LED_CAPS  0x04

// shifted member
#define RSHIFTBIT      0x01   // bit in key_flags
#define LSHIFTBIT      0x02   // bit in key_flags
#define CTRLBIT        0x04   // bit in key_flags
#define ALTBIT         0x08   // bit in key_flags
#define SCROLLBIT      0x10   // bit in key_flags
#define NUMLOCKBIT     0x20   // bit in key_flags
#define CAPSLCKBIT     0x40   // bit in key_flags
#define INSERTBIT      0x80   // bit in key_flags

void interrupt ps2_key_irq();
bit16u key_translate(const bit8u key);
int key_scan_set(const bit8u *buff, const int set, const int len);
bool key_set_flags(const bit8u bit, const bool break_code);
void keyboard_toggle_led(const bit8u bit);


struct SCAN_CODE {
  bit8u  shift_flag;      // is this key a modifier? (shift, ctlr, alt, etc)
  bool   is_numpad;       // is a key effected by the num_lock state
	bit16u no_shift;        // scan code to keypress with no modifiers
	bit16u shifted;         // scan code to keypress with shift pressed
	bit16u control;         // scan code to keypress with ctrl pressed
	bit16u alt;             // scan code to keypress with alt pressed
  bit16u num_alt_code;    // used when alt is pressed and a numpad digit is used
};

#endif  // FYSOS_PS2KEY
