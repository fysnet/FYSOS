/*
 *                             Copyright (c) 1984-2022
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             https://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  Last updated: 12 July 2022
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
