/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2017
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
 *
 *  compile using MS QuickC 2.5
 *
 *  usage:
 *    ps2key [-veryslow | -slow | -fast | -veryfast | -1 | -2 | -3 | -c]
 *    
 *    This app uses a very simple form of delay which is extremely different
 *     on slower and faster machines.  Therefore, indicate on the command line
 *      -veryslow for older very slow machines (50Mhz)
 *      -slow for older slow machines (250Mhz)
 *      -fast for newer somewhat faster machines (1,500Mhz)
 *      -veryfast for newer faster machines (2,000Mhz +)
 *     You can simply give no parameter for a machine in the range of 1,000Mhz
 *      -1 (try to) use scan code set 1
 *      -2 (try to) use scan code set 2
 *      -3 (try to) use scan code set 3
 *     Be default, this util will print our translated scan code, make, and break codes.
 *     To print keyboard's scan code only, use
 *      -c
 *
 * Notes:
 *  - This example is coded and compiled for 16-bit DOS using a 16-bit compiler.
 *  - There is no need for a DPMI driver.
 *  - All variables and numbers are 16-bit.
 *     char = bit8s = byte = 8-bits  (AL)
 *      int = bit16s = word = 16-bits (AX)
 *     long = bit32s = dword = 32-bits (DX:AX)
 *
 */

#include <conio.h>
#include <dos.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ctype.h"
#include "ps2key.h"

#include "../include/pic.c"
#include "ps2share.c"

bool is_second = FALSE;

// Since our translator may take longer to translate
//  than the keyboard will send chars, we need a revolving
//  buffer to store the scan codes in.
#define KEY_BUFFER_SIZE 16
bit8u key_buffer[KEY_BUFFER_SIZE];
volatile int key_buff_us = 0;   // our current position
volatile int key_buff_irq = 0;  // the IRQ's current position

int scan_set = 0;     // 1 = set 1, 2 = set 2, etc.

int main(int argc, char *argv[]) {
  bit8u ports, byte, org_byte;
  bool mode = KEY_MODE_PS2;
  int  i, id0 = -1, id1 = -1;
  bit16u scancode = 0;
  int try_scan_set = 1; // will use scan set 1 by default
  int org_scan_set;
  bool codes_only = FALSE;  // default to no
  
  printf("Detect PS/2 Controller and Keyboard.   v1.00.00\n"
         "Forever Young Software  --  Copyright 1984-2016\n\n");
  
  // for slow machines, don't count so high
  // for fast machines, delay needs to be larger counter
  // larger number needed for faster machines.
  // (cheap way to delay)
  for (i=1; i<argc; i++) {
    if (!strcmp(argv[i], "-veryslow"))
      time_out = 100;
    else if (!strcmp(argv[i], "-slow"))
      time_out = 2000;
    else if (!strcmp(argv[i], "-fast"))
      time_out = 12000;
    else if (!strcmp(argv[i], "-veryfast"))
      time_out = 20000;
    else if (!strcmp(argv[i], "-1"))
      try_scan_set = 1;
    else if (!strcmp(argv[i], "-2"))
      try_scan_set = 2;
    else if (!strcmp(argv[i], "-3"))
      try_scan_set = 3;
    else if (!strcmp(argv[i], "-c"))
      codes_only = 1;
    else {
      printf("Unknown Parameter...\n");
      return -1;
    }
  }
  
  // when using FreeDOS, we must disallowed its handler since
  //  it may already be active, hence reading in bytes not allowing
  //  us to read in command ACK's
  picmask(PS2MOUSEIRQ);  // make sure the IRQ is off
  picmask(PS2KEYIRQ);
  
  // detect the ports (channels)
  ports = det_ps2_port();
  if (ports == 0) {
    printf("No PS/2 Port(s) detected...\n");
    return -1;
  }
  printf("Found Type%i (MCA) PS/2 port...\n", ps2_type);
  if (ports & 1)
    printf("Found first PS/2 Port and is available...\n");
  if (ports & 2)
    printf("Found second PS/2 Port and is available...\n");
  
  // we will assume the keyboard is on the first channel
  is_second = FALSE;
  
  // Get current interface.  Should return 0x00 or 0x01.
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_GET_MODE);
  byte = keyboard_read();
  if (byte & 1) {
    printf("Current interface is (MCA) PS/2\n");
    mode = KEY_MODE_PS2;
  } else {
    printf("Current interface is (ISA) AT\n");
    mode = KEY_MODE_AT;
  }
  
  // get current scan code set
  // if we have a Type1 controller, scan code set could be
  //  set 1, 2, or 3.  If we have a Type2 controller, scan
  //  code set can only be set 2 or 3
  // (After a reset, the current scan code will be 2, so we do this now,  )
  // ( so that we can set it back for FreeDOS.                            )
  // (I am placing it here to show you how to get it.                     )
  if (ps2_type == 1) {
    // first turn off translation
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
    org_byte = keyboard_read();
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
    keyboard_write(KEYBRD_CMND_60, (bit8u) (org_byte & ~(1<<6)));  // clear translation bit
  }
  if (!keyboard_cmnd60(KEY_GETSET_SCAN_CODE, is_second) || !keyboard_cmnd60(0, is_second))
    printf("Error getting current scan code set...\n");
  else
    scan_set = keyboard_read();
  // restore translation bit
  if (ps2_type == 1) {
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
    keyboard_write(KEYBRD_CMND_60, org_byte);
  }
  org_scan_set = scan_set;
  printf("Scan Code Set before reset is set #%i\n", scan_set);
  
  // reset the device on the channel
  // after reset, we should then recieve a KEY_CMD_SELFTEST byte
  if (!keyboard_cmnd60(KEY_CMD_RESET, is_second)) {
    printf("Error sending Reset Command...\n");
    goto done;
  }
  //mdelay(500); // must wait 500mS to 750mS for command to complete
  // since we use a cheap delay, let's hope this is enough.
  // your code will need a better way of delaying more accurately.
  io_delay(INT_MAX);
  if (keyboard_read() == KEY_CMD_SELFTEST) {
    // reset command might return an ID too...
    // If you wish to wait for it, un-comment these next lines.
    // However, most keyboard may not and your code will wait for the 
    //  timeout, showing a noticable pause in execution time.
    /* io_delay(INT_MAX);  // must wait on slower machines
     * id0 = keyboard_read();
     * if (id0 != KEYBOARD_ERROR)
     *  printf("Found a PS/2 Device with an initial id of %i\n", id0);
     */
  } else {
    printf("Did not receive KEY_CMD_SELFTEST byte\n");
    goto done;
  }
  
  // turn off all LED's
  if (keyboard_cmnd60(KEY_LED_WRITE, is_second))
    keyboard_cmnd60(0, is_second);
  
  // do a keyboard echo and see if we get 0xEE back
  printf("Checking Keyboard Echo function... ");
  if ((keyboard_write(KEYBRD_CMND_60, KEY_DIAG_ECHO) == KEY_DIAG_ECHO) && (keyboard_read() == KEY_DIAG_ECHO))
    puts("success");
  else
    puts("fail");
  
  // Get Keyboard ID (not available on all machines)
  // XT sends nothing, AT sends ACK only
  // MFII with translation sends ACK + ABh + 41h
  // MFII without translation sends ACK + ABh + 83h
  if (keyboard_cmnd60(KEY_CMD_ID, is_second)) {
    id0 = keyboard_read();
    if (id0 == 0xAB)
      id1 = keyboard_read();
    printf("Found a PS/2 Device with an id of %02Xh %02Xh\n", id0, id1);
  } else {
    printf("Did not receive ID byte\n");
    goto done;
  }
  
  // turn on all LED's
  if (keyboard_cmnd60(KEY_LED_WRITE, is_second))
    keyboard_cmnd60(KEYBOARD_LED_CAPS | KEYBOARD_LED_NUM | KEYBOARD_LED_SCRL, is_second);
  
  // set typematic rate and delay
  if (keyboard_cmnd60(KEY_CMD_TPRATE, is_second))
    keyboard_cmnd60((1 << 5) | (0 << 0), is_second);  // 500 ms / 30.0 reports/sec
  
  // if we can change it and it isn't already set to try_scan_set, change it to try_scan_set
  if (scan_set != try_scan_set) {
    if ((ps2_type == 1) || ((ps2_type == 2) && (try_scan_set > 1))) {
      if (!keyboard_cmnd60(KEY_GETSET_SCAN_CODE, is_second) || !keyboard_cmnd60((bit8u) try_scan_set, is_second))
        printf("Error setting scan set code to set %i...\n", try_scan_set);
      else {
        printf("Succeeded setting scan set code to set %i...\n", try_scan_set);
        scan_set = try_scan_set;
        // if we chose and successfully changed to set 3,
        //  we need to set the break codes too.
        if (scan_set == 3)
          keyboard_cmnd60(KEY_CMD_TYPE_MAKE_REL, is_second);
      }
    } else {
      printf("Type1 controllers cannot use Scan Code Set 1.\n");
      goto done;
    }
  }
  
  // no matter which set we use, turn off translation
  // (only need to for Type1 controller.  Type2 can't have it set anyway)
  if (ps2_type == 1) {
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_READ);
    byte = keyboard_read();
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
    keyboard_write(KEYBRD_CMND_60, (bit8u) (byte & ~(1<<6)));  // clear translation bit
  }
  
  // set up the IRQ and start catching keys
  old_isr = _dos_getvect(ivt_num[PS2KEYIRQ]);
  _dos_setvect(ivt_num[PS2KEYIRQ], ps2_key_irq);
  
  inpb(KEYBOARD_DATA);  // clear any byte that may be in the buffer
  keyboard_cmnd60(KEY_CMD_ENABLE, is_second);  // enable the keyboard
  picunmask(PS2KEYIRQ);  // make sure the IRQ is on
  
  // our loop
  puts("");
  do {
    if (key_buff_us != key_buff_irq) {
      if (codes_only)
        printf(" %02X", key_buffer[key_buff_us]);
      else
        scancode = key_translate(key_buffer[key_buff_us]);
      if (++key_buff_us == KEY_BUFFER_SIZE)
        key_buff_us = 0;
    }
  } while (scancode != 0x011B);
  
  // disable keyboard while we reset it for FreeDOS
  picmask(PS2KEYIRQ);  // make sure the IRQ is off
  keyboard_cmnd60(KEY_CMD_RESDIS, is_second);  // disable the keyboard
  
  // reset the LED's to what they should be
  if (keyboard_cmnd60(KEY_LED_WRITE, is_second))
    keyboard_cmnd60(KEYBOARD_LED_NUM, is_second);
  
  // if we changed the scan_set, restore it
  // (No need to check to see if we can change it,
  //  because we obviously already did...)
  if (scan_set != org_scan_set) {
    keyboard_cmnd60(KEY_GETSET_SCAN_CODE, is_second);
    keyboard_cmnd60((bit8u) org_scan_set, is_second);
  }
  // restore translation bit
  if (ps2_type == 1) {
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
    keyboard_write(KEYBRD_CMND_60, org_byte);
  }
  // re-enable the hosts (FreeDOS) interrupt handler
  _dos_setvect(ivt_num[PS2KEYIRQ], old_isr);
  keyboard_cmnd60(KEY_CMD_ENABLE, is_second);  // enable the keyboard
  picunmask(PS2KEYIRQ);
  return 0;
  
done:
  // if we changed the scan_set, restore it
  // (No need to check to see if we can change it,
  //  because we obviously already did...)
  if (scan_set != org_scan_set) {
    keyboard_cmnd60(KEY_GETSET_SCAN_CODE, is_second);
    keyboard_cmnd60((bit8u) org_scan_set, is_second);
  }
  // restore translation bit
  if (ps2_type == 1) {
    keyboard_write(KEYBRD_CMND_64, KEY_CMD_WRITE);
    keyboard_write(KEYBRD_CMND_60, org_byte);
  }
  printf("Error...\n");
  keyboard_write(KEYBRD_CMND_64, KEY_CMD_EN_KEYB);  // re-enable the keyboard
  picunmask(PS2KEYIRQ);
  return -1;
}

// an IRQ to simply read in the byte and 
//  place it into our rotating buffer
// (this routine does not check for buffer overflow/overwrite)
void interrupt ps2_key_irq() {
  
  // place a scan code byte into the buffer.
  key_buffer[key_buff_irq] = inpb(KEYBOARD_DATA);
  if (++key_buff_irq == KEY_BUFFER_SIZE)
    key_buff_irq = 0;
  
  // acknowledge IRQ(s)
  outpb(PIC_MASTER, EOI);      // end of interrupt on controller
}

// Include the scan code tables
#include "scancode.h"

bit8u break_buffer[16] = { 0, };
int sb_b = 0;
bit8u scan_buffer[16] = { 0, };
int sb_i = 0;
bool  is_break = FALSE;
bit8u key_flags = 0x00;
bit8u ext_key_flags = 0x00;

bit16u key_translate(const bit8u key) {
  bit16u scancode = 0;
  int index;
  
  switch (scan_set) {
    case 1:  // set 1
      if ((key != 0xE0) && (key != 0xE1) && (key & 0x80)) {
        scan_buffer[sb_i++] = key & 0x7F;
        is_break = TRUE;
      } else
        scan_buffer[sb_i++] = key;
      scan_buffer[sb_i] = 0;
      break_buffer[sb_b++] = key;
      break_buffer[sb_b] = 0;
      break;
    case 2:  // set 2
    case 3:  // set 3
      break_buffer[sb_b++] = key;
      break_buffer[sb_b] = 0;
      if (key == 0xF0) {
        is_break = TRUE;
        return 0;
      }
      scan_buffer[sb_i++] = key;
      scan_buffer[sb_i] = 0;
      break;
  }
  
  index = key_scan_set(scan_buffer, scan_set, sb_i);
  if (index > -1) {
    if (scan_code[index].shift_flag != 0xFF) {  // we ignore the two FAKE Shift scan codes
      if (scan_code[index].shift_flag)
        key_set_flags(scan_code[index].shift_flag, is_break);
      if (!is_break) {
        if (scan_code[index].is_numpad && !(key_flags & NUMLOCKBIT))
          scancode = scan_code[index].num_alt_code;
        else if ((key_flags & (LSHIFTBIT | RSHIFTBIT)) && (key_flags & CAPSLCKBIT))
          scancode = scan_code[index].no_shift;
        else if ((key_flags & (LSHIFTBIT | RSHIFTBIT)) || (key_flags & CAPSLCKBIT))
          scancode = scan_code[index].shifted;
        else if ((key_flags & CTRLBIT))
          scancode = scan_code[index].control;
        else if ((key_flags & ALTBIT))
          scancode = scan_code[index].alt;
        else 
          scancode = scan_code[index].no_shift;
        
        // We now have a valid scan code in 'scancode'.
        // This is where you would add the scancode to your 
        // user keyboard buffer.
        printf(" ScanCode: %04X  Make:", scancode);
        index = 0;
        while (scan_buffer[index])
          printf(" %02X", scan_buffer[index++]);
      } else {
        printf(" Break:");
        index=0;
        while (break_buffer[index])
          printf(" %02X", break_buffer[index++]);
        puts("");
      }
    }
  } else
    return scancode;
  
  // clear buffer for next time
  is_break = FALSE;
  scan_buffer[0] = 0;
  sb_i = 0;
  break_buffer[0] = 0;
  sb_b = 0;
  return scancode;
}

void keyboard_toggle_led(const bit8u bit) {
  bit8u byte;
  bit8u cur_bits = ((key_flags & SCROLLBIT)  ? KEYBOARD_LED_SCRL : 0) |
                   ((key_flags & NUMLOCKBIT) ? KEYBOARD_LED_NUM  : 0) |
                   ((key_flags & CAPSLCKBIT) ? KEYBOARD_LED_CAPS : 0);
  if (bit & SCROLLBIT)
    cur_bits ^= KEYBOARD_LED_SCRL;
  if (bit & NUMLOCKBIT)
    cur_bits ^= KEYBOARD_LED_NUM;
  if (bit & CAPSLCKBIT)
    cur_bits ^= KEYBOARD_LED_CAPS;
  
  // disable the keyboard
  picmask(PS2KEYIRQ);  // make sure the IRQ is off
  keyboard_write(KEYBRD_CMND_64, (bit8u) ((is_second) ? KEY_CMD_DIS_SECOND : KEY_CMD_DIS_KEYB));
  
  if (keyboard_cmnd60(KEY_LED_WRITE, is_second))
    keyboard_cmnd60(cur_bits, is_second);
  
  // enable the keyboard
  keyboard_write(KEYBRD_CMND_64, (bit8u) ((is_second) ? KEY_CMD_EN_SECOND : KEY_CMD_EN_KEYB));
  picunmask(PS2KEYIRQ);  // make sure the IRQ is on
}

// set = 1 = set 1, etc...
int key_scan_set(const bit8u *buff, const int set, const int len) {
  int i;
  
  if (len > 0) {
    for (i=0; i<SET_TOT_ITEMS; i++)
      if (memcmp(buff, scan_sequence[set-1][i], len + 1) == 0)
        return i;
  }
  
  return -1;
}

bool key_set_flags(const bit8u bit, const bool break_code) {
  switch (bit) {
    case LSHIFTBIT:
      if (!break_code)
        key_flags |= LSHIFTBIT;
      else
        key_flags &= ~(LSHIFTBIT | RSHIFTBIT);
      break;
    case RSHIFTBIT:
      if (!break_code)
        key_flags |= RSHIFTBIT;
      else
        key_flags &= ~(LSHIFTBIT | RSHIFTBIT);
      break;
    case CAPSLCKBIT:
      if (!break_code) {
        keyboard_toggle_led(CAPSLCKBIT);
        key_flags ^= CAPSLCKBIT;
      }
      break;
    case CTRLBIT:
      if (!break_code)
        key_flags |= CTRLBIT;
      else
        key_flags &= ~CTRLBIT;
      break;
    case ALTBIT:
      if (!break_code)
        key_flags |= ALTBIT;
      else
        key_flags &= ~ALTBIT;
      break;
    case SCROLLBIT:
      if (break_code) {
        keyboard_toggle_led(SCROLLBIT);
        key_flags ^= SCROLLBIT;
      }
      break;
    case INSERTBIT:
      // TODO: only toggle the bit if no modifying keys are pressed.
      //  i.e: Don't toggle if shift+ins, ctrl+ins, etc.
      if (break_code)
        key_flags ^= INSERTBIT;
      break;
    case NUMLOCKBIT:
      if (break_code) {
        keyboard_toggle_led(NUMLOCKBIT);
        key_flags ^= NUMLOCKBIT;
      }
      break;
    default:
      return FALSE;
  }
  return TRUE;
}
