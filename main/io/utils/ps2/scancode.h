/*
 *                             Copyright (c) 1984-2020
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
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  Last updated: 15 July 2020
 */

#ifndef FYSOS_SCANCODES
#define FYSOS_SCANCODES

#define SET_TOT_ITEMS   128

bit8u scan_sequence[3][SET_TOT_ITEMS][12] = {
  { // set 1
    //  key code sequence
    { 0x1E,0                        }, // A
    { 0x30,0                        }, // B
    { 0x2E,0                        }, // C
    { 0x20,0                        }, // D
    { 0x12,0                        }, // E
    { 0x21,0                        }, // F
    { 0x22,0                        }, // G
    { 0x23,0                        }, // H
    { 0x17,0                        }, // I
    { 0x24,0                        }, // J
    { 0x25,0                        }, // K
    { 0x26,0                        }, // L
    { 0x32,0                        }, // M
    { 0x31,0                        }, // N
    { 0x18,0                        }, // O
    { 0x19,0                        }, // P
    { 0x10,0                        }, // Q
    { 0x13,0                        }, // R
    { 0x1F,0                        }, // S
    { 0x14,0                        }, // T
    { 0x16,0                        }, // U
    { 0x2F,0                        }, // V
    { 0x11,0                        }, // W
    { 0x2D,0                        }, // X
    { 0x15,0                        }, // Y
    { 0x2C,0                        }, // Z
    { 0x0B,0                        }, // 0)
    { 0x02,0                        }, // 1!
    { 0x03,0                        }, // 2@
    { 0x04,0                        }, // 3#
    { 0x05,0                        }, // 4$
    { 0x06,0                        }, // 5%
    { 0x07,0                        }, // 6^
    { 0x08,0                        }, // 7&
    { 0x09,0                        }, // 8*
    { 0x0A,0                        }, // 9(
    { 0x29,0                        }, // `~
    { 0x0C,0                        }, // -_
    { 0x0D,0                        }, // =+
    { 0x2B,0                        }, // \|
    { 0x0E,0                        }, // backspace
    { 0x39,0                        }, // Space
    { 0x0F,0                        }, // tab
    { 0x3A,0                        }, // Caps-Lock
    { 0x2A,0                        }, // L-Shift
    { 0xE0,0x2A,0                   }, // Fake L-Shift
    { 0x1D,0                        }, // L-CTRL
    { 0xE0,0x5B,0                   }, // L-GUI
    { 0x38,0                        }, // L-Alt
    { 0x36,0                        }, // R-Shift
    { 0xE0,0x36,0                   }, // Fake R-Shift
    { 0xE0,0x1D,0                   }, // R-CTRL
    { 0xE0,0x5C,0                   }, // R-GUI
    { 0xE0,0x38,0                   }, // R-Alt
    { 0xE0,0x5D,0                   }, // Apps (right of Right GUI)
    { 0x1C,0                        }, // Enter (Main)
    { 0x01,0                        }, // ESC
    { 0x3B,0                        }, // F1
    { 0x3C,0                        }, // F2
    { 0x3D,0                        }, // F3
    { 0x3E,0                        }, // F4
    { 0x3F,0                        }, // F5
    { 0x40,0                        }, // F6
    { 0x41,0                        }, // F7
    { 0x42,0                        }, // F8
    { 0x43,0                        }, // F9
    { 0x44,0                        }, // F10
    { 0x57,0                        }, // F11
    { 0x58,0                        }, // F12
    { 0xE0,0x2A,0xE0,0x37,0         }, // Prnt Screen
    { 0xE0,0x37,0xE0,0x2A,0         }, // Prnt Screen (break) (ignore it)
    { 0x46,0                        }, // Scroll-Lock
    { 0xE1,0x1D,0x45,0xE1,0x9D,0xC5,0}, // Pause
    { 0x1A,0                        }, // [{
    { 0xE0,0x52,0                   }, // Insert
    { 0xE0,0x47,0                   }, // Home
    { 0xE0,0x49,0                   }, // Page Up
    { 0xE0,0x53,0                   }, // Delete
    { 0xE0,0x4F,0                   }, // End
    { 0xE0,0x51,0                   }, // Page Down
    { 0xE0,0x48,0                   }, // Up Arrow
    { 0xE0,0x4B,0                   }, // Left Arrow
    { 0xE0,0x50,0                   }, // Down Arrow
    { 0xE0,0x4D,0                   }, // Right Arrow
    { 0x45,0                        }, // Nums-Lock
    { 0xE0,0x35,0                   }, // Keypad /
    { 0x37,0                        }, // Gray * (old prtscrn)
    { 0x4A,0                        }, // Gray-
    { 0x4E,0                        }, // Gray+
    { 0xE0,0x1C,0                   }, // Keypad Enter
    { 0x53,0                        }, // Del/.
    { 0x52,0                        }, // 0/Insert
    { 0x4F,0                        }, // 1/End
    { 0x50,0                        }, // 2/Down
    { 0x51,0                        }, // 3/PgDn
    { 0x4B,0                        }, // 4/Left
    { 0x4C,0                        }, // 5/Center
    { 0x4D,0                        }, // 6/Right
    { 0x47,0                        }, // 7/Home
    { 0x48,0                        }, // 8/Up
    { 0x49,0                        }, // 9/PgUp
    { 0x1B,0                        }, // ]}
    { 0x27,0                        }, // ;:
    { 0x28,0                        }, // '"
    { 0x33,0                        }, // ,<
    { 0x34,0                        }, // .>
    { 0x35,0                        }, // /?
    { 0xE0,0x5E,0                   }, // Power
    { 0xE0,0x5F,0                   }, // Sleep
    { 0xE0,0x63,0                   }, // Wake
    { 0xE0,0x19,0                   }, // Next Track
    { 0xE0,0x10,0                   }, // Previous Track
    { 0xE0,0x24,0                   }, // Stop
    { 0xE0,0x22,0                   }, // Play/Pause
    { 0xE0,0x20,0                   }, // Mute
    { 0xE0,0x30,0                   }, // Volume Up
    { 0xE0,0x2E,0                   }, // Volume Down
    { 0xE0,0x6D,0                   }, // Media Select
    { 0xE0,0x6C,0                   }, // E-mail
    { 0xE0,0x21,0                   }, // Calculator
    { 0xE0,0x6B,0                   }, // My Computer
    { 0xE0,0x65,0                   }, // WWW Search
    { 0xE0,0x32,0                   }, // WWW Home
    { 0xE0,0x6A,0                   }, // WWW Back
    { 0xE0,0x69,0                   }, // WWW Forward
    { 0xE0,0x68,0                   }, // WWW Stop
    { 0xE0,0x67,0                   }, // WWW Refresh
    { 0xE0,0x66,0                   }, // WWW Favorites
  },
  { // set 2
    { 0x1C,0                        }, // A
    { 0x32,0                        }, // B
    { 0x21,0                        }, // C
    { 0x23,0                        }, // D
    { 0x24,0                        }, // E
    { 0x2B,0                        }, // F
    { 0x34,0                        }, // G
    { 0x33,0                        }, // H
    { 0x43,0                        }, // I
    { 0x3B,0                        }, // J
    { 0x42,0                        }, // K
    { 0x4B,0                        }, // L
    { 0x3A,0                        }, // M
    { 0x31,0                        }, // N
    { 0x44,0                        }, // O
    { 0x4D,0                        }, // P
    { 0x15,0                        }, // Q
    { 0x2D,0                        }, // R
    { 0x1B,0                        }, // S
    { 0x2C,0                        }, // T
    { 0x3C,0                        }, // U
    { 0x2A,0                        }, // V
    { 0x1D,0                        }, // W
    { 0x22,0                        }, // X
    { 0x35,0                        }, // Y
    { 0x1A,0                        }, // Z
    { 0x45,0                        }, // 0)
    { 0x16,0                        }, // 1!
    { 0x1E,0                        }, // 2@
    { 0x26,0                        }, // 3#
    { 0x25,0                        }, // 4$
    { 0x2E,0                        }, // 5%
    { 0x36,0                        }, // 6^
    { 0x3D,0                        }, // 7&
    { 0x3E,0                        }, // 8*
    { 0x46,0                        }, // 9(
    { 0x0E,0                        }, // `~
    { 0x4E,0                        }, // -_
    { 0x55,0                        }, // =+
    { 0x5D,0                        }, // \|
    { 0x66,0                        }, // backspace
    { 0x29,0                        }, // Space
    { 0x0D,0                        }, // tab
    { 0x58,0                        }, // Caps-Lock
    { 0x12,0                        }, // L-Shift
    { 0xE0,0x12,0                   }, // Fake L-Shift
    { 0x14,0                        }, // L-CTRL
    { 0xE0,0x1F,0                   }, // L-GUI
    { 0x11,0                        }, // L-Alt
    { 0x59,0                        }, // R-Shift
    { 0xE0,0x59,0                   }, // Fake R-Shift
    { 0xE0,0x14,0                   }, // R-CTRL
    { 0xE0,0x27,0                   }, // R-GUI
    { 0xE0,0x11,0                   }, // R-Alt
    { 0xE0,0x2F,0                   }, // Apps (right of Right GUI)
    { 0x5A,0                        }, // Enter (Main)
    { 0x76,0                        }, // ESC
    { 0x05,0                        }, // F1
    { 0x06,0                        }, // F2
    { 0x04,0                        }, // F3
    { 0x0C,0                        }, // F4
    { 0x03,0                        }, // F5
    { 0x0B,0                        }, // F6
    { 0x83,0                        }, // F7
    { 0x0A,0                        }, // F8
    { 0x01,0                        }, // F9
    { 0x09,0                        }, // F10
    { 0x78,0                        }, // F11
    { 0x07,0                        }, // F12
    { 0xE0,0x12,0xE0,0x7C,0         }, // Prnt Screen
    { 0xE0,0x7C,0xE0,0x12,0         }, // Prnt Screen (break) (ignore it)
    { 0x7E,0                        }, // Scroll-Lock
    { 0xE1,0x14,0x77,0xE1,0xF0,0x14,0xF0,0x77,0}, // Pause
    { 0x54,0                        }, // [{
    { 0xE0,0x70,0                   }, // Insert
    { 0xE0,0x6C,0                   }, // Home
    { 0xE0,0x7D,0                   }, // Page Up
    { 0xE0,0x71,0                   }, // Delete
    { 0xE0,0x69,0                   }, // End
    { 0xE0,0x7A,0                   }, // Page Down
    { 0xE0,0x75,0                   }, // Up Arrow
    { 0xE0,0x6B,0                   }, // Left Arrow
    { 0xE0,0x72,0                   }, // Down Arrow
    { 0xE0,0x74,0                   }, // Right Arrow
    { 0x77,0                        }, // Nums-Lock
    { 0xE0,0x4A,0                   }, // Keypad /
    { 0x7C,0                        }, // Gray * (old prtscrn)
    { 0x7B,0                        }, // Gray-
    { 0x79,0                        }, // Gray+
    { 0xE0,0x5A,0                   }, // Keypad Enter
    { 0x71,0                        }, // Del/.
    { 0x70,0                        }, // 0/Insert
    { 0x69,0                        }, // 1/End
    { 0x72,0                        }, // 2/Down
    { 0x7A,0                        }, // 3/PgDn
    { 0x6B,0                        }, // 4/Left
    { 0x73,0                        }, // 5/Center
    { 0x74,0                        }, // 6/Right
    { 0x6C,0                        }, // 7/Home
    { 0x75,0                        }, // 8/Up
    { 0x7D,0                        }, // 9/PgUp
    { 0x5B,0                        }, // ]}
    { 0x4C,0                        }, // ;:
    { 0x52,0                        }, // '"
    { 0x41,0                        }, // ,<
    { 0x49,0                        }, // .>
    { 0x4A,0                        }, // /?
    { 0xE0,0x37,0                   }, // Power
    { 0xE0,0x3F,0                   }, // Sleep
    { 0xE0,0x5E,0                   }, // Wake
    { 0xE0,0x4D,0                   }, // Next Track
    { 0xE0,0x15,0                   }, // Previous Track
    { 0xE0,0x3B,0                   }, // Stop
    { 0xE0,0x34,0                   }, // Play/Pause
    { 0xE0,0x23,0                   }, // Mute
    { 0xE0,0x32,0                   }, // Volume Up
    { 0xE0,0x21,0                   }, // Volume Down
    { 0xE0,0x50,0                   }, // Media Select
    { 0xE0,0x48,0                   }, // E-mail
    { 0xE0,0x2B,0                   }, // Calculator
    { 0xE0,0x40,0                   }, // My Computer
    { 0xE0,0x10,0                   }, // WWW Search
    { 0xE0,0x3A,0                   }, // WWW Home
    { 0xE0,0x38,0                   }, // WWW Back
    { 0xE0,0x30,0                   }, // WWW Forward
    { 0xE0,0x28,0                   }, // WWW Stop
    { 0xE0,0x20,0                   }, // WWW Refresh
    { 0xE0,0x18,0                   }, // WWW Favorites
  },
  { // set 3
    { 0x1C,0                        }, // A
    { 0x32,0                        }, // B
    { 0x21,0                        }, // C
    { 0x23,0                        }, // D
    { 0x24,0                        }, // E
    { 0x2B,0                        }, // F
    { 0x34,0                        }, // G
    { 0x33,0                        }, // H
    { 0x43,0                        }, // I
    { 0x3B,0                        }, // J
    { 0x42,0                        }, // K
    { 0x4B,0                        }, // L
    { 0x3A,0                        }, // M
    { 0x31,0                        }, // N
    { 0x44,0                        }, // O
    { 0x4D,0                        }, // P
    { 0x15,0                        }, // Q
    { 0x2D,0                        }, // R
    { 0x1B,0                        }, // S
    { 0x2C,0                        }, // T
    { 0x3C,0                        }, // U
    { 0x2A,0                        }, // V
    { 0x1D,0                        }, // W
    { 0x22,0                        }, // X
    { 0x35,0                        }, // Y
    { 0x1A,0                        }, // Z
    { 0x45,0                        }, // 0)
    { 0x16,0                        }, // 1!
    { 0x1E,0                        }, // 2@
    { 0x26,0                        }, // 3#
    { 0x25,0                        }, // 4$
    { 0x2E,0                        }, // 5%
    { 0x36,0                        }, // 6^
    { 0x3D,0                        }, // 7&
    { 0x3E,0                        }, // 8*
    { 0x46,0                        }, // 9(
    { 0x0E,0                        }, // `~
    { 0x4E,0                        }, // -_
    { 0x55,0                        }, // =+
    { 0x5C,0                        }, // \|
    { 0x66,0                        }, // backspace
    { 0x29,0                        }, // Space
    { 0x0D,0                        }, // tab
    { 0x14,0                        }, // Caps-Lock
    { 0x12,0                        }, // L-Shift
    { 0x00,0                        }, // Fake L-Shift (Set 3 doesn't do this)
    { 0x11,0                        }, // L-CTRL
    { 0x8B,0                        }, // L-GUI
    { 0x19,0                        }, // L-Alt
    { 0x59,0                        }, // R-Shift
    { 0x00,0                        }, // Fake R-Shift (Set 3 doesn't do this)
    { 0x58,0                        }, // R-CTRL
    { 0x8C,0                        }, // R-GUI
    { 0x39,0                        }, // R-Alt
    { 0x8D,0                        }, // Apps (right of Right GUI)
    { 0x5A,0                        }, // Enter (Main)
    { 0x08,0                        }, // ESC
    { 0x07,0                        }, // F1
    { 0x0F,0                        }, // F2
    { 0x17,0                        }, // F3
    { 0x1F,0                        }, // F4
    { 0x27,0                        }, // F5
    { 0x2F,0                        }, // F6
    { 0x37,0                        }, // F7
    { 0x3F,0                        }, // F8
    { 0x47,0                        }, // F9
    { 0x4F,0                        }, // F10
    { 0x56,0                        }, // F11
    { 0x5E,0                        }, // F12
    { 0x57,0                        }, // Prnt Screen
    { 0x00,0                        }, // Prnt Screen (break) (ignore it)
    { 0x5F,0                        }, // Scroll-Lock
    { 0x62,0                        }, // Pause
    { 0x54,0                        }, // [{
    { 0x67,0                        }, // Insert
    { 0x6E,0                        }, // Home
    { 0x6F,0                        }, // Page Up
    { 0x64,0                        }, // Delete
    { 0x65,0                        }, // End
    { 0x6D,0                        }, // Page Down
    { 0x63,0                        }, // Up Arrow
    { 0x61,0                        }, // Left Arrow
    { 0x60,0                        }, // Down Arrow
    { 0x6A,0                        }, // Right Arrow
    { 0x76,0                        }, // Nums-Lock
    { 0x77,0                        }, // Keypad /
    { 0x7E,0                        }, // Gray * (old prtscrn)
    { 0x84,0                        }, // Gray-
    { 0x7C,0                        }, // Gray+
    { 0x79,0                        }, // Keypad Enter
    { 0x71,0                        }, // Del/.
    { 0x70,0                        }, // 0/Insert
    { 0x69,0                        }, // 1/End
    { 0x72,0                        }, // 2/Down
    { 0x7A,0                        }, // 3/PgDn
    { 0x6B,0                        }, // 4/Left
    { 0x73,0                        }, // 5/Center
    { 0x74,0                        }, // 6/Right
    { 0x6C,0                        }, // 7/Home
    { 0x75,0                        }, // 8/Up
    { 0x7D,0                        }, // 9/PgUp
    { 0x5B,0                        }, // ]}
    { 0x4C,0                        }, // ;:
    { 0x52,0                        }, // '"
    { 0x41,0                        }, // ,<
    { 0x49,0                        }, // .>
    { 0x4A,0                        }, // /?
  }
};

struct SCAN_CODE scan_code[SET_TOT_ITEMS] = {
 // shift       is_     no_    shifted  control  alt  num_alt  command
 // flag        numpad  shift                           _code
  { 0x00,       FALSE, 0x1E61, 0x1E41, 0x1E01, 0x1E00, 0x0000 }, // A
  { 0x00,       FALSE, 0x3062, 0x3042, 0x3002, 0x3000, 0x0000 }, // B
  { 0x00,       FALSE, 0x2E63, 0x2E43, 0x2E03, 0x2E00, 0x0000 }, // C
  { 0x00,       FALSE, 0x2064, 0x2044, 0x2004, 0x2000, 0x0000 }, // D
  { 0x00,       FALSE, 0x1265, 0x1245, 0x1205, 0x1200, 0x0000 }, // E
  { 0x00,       FALSE, 0x2166, 0x2146, 0x2106, 0x2100, 0x0000 }, // F
  { 0x00,       FALSE, 0x2267, 0x2247, 0x2207, 0x2200, 0x0000 }, // G
  { 0x00,       FALSE, 0x2368, 0x2348, 0x2308, 0x2300, 0x0000 }, // H
  { 0x00,       FALSE, 0x1769, 0x1749, 0x1709, 0x1700, 0x0000 }, // I
  { 0x00,       FALSE, 0x246A, 0x244A, 0x240A, 0x2400, 0x0000 }, // J
  { 0x00,       FALSE, 0x256B, 0x254B, 0x250B, 0x2500, 0x0000 }, // K
  { 0x00,       FALSE, 0x266C, 0x264C, 0x260C, 0x2600, 0x0000 }, // L
  { 0x00,       FALSE, 0x326D, 0x324D, 0x320D, 0x3200, 0x0000 }, // M
  { 0x00,       FALSE, 0x316E, 0x314E, 0x310E, 0x3100, 0x0000 }, // N
  { 0x00,       FALSE, 0x186F, 0x184F, 0x180F, 0x1800, 0x0000 }, // O
  { 0x00,       FALSE, 0x1970, 0x1950, 0x1910, 0x1900, 0x0000 }, // P
  { 0x00,       FALSE, 0x1071, 0x1051, 0x1011, 0x1000, 0x0000 }, // Q
  { 0x00,       FALSE, 0x1372, 0x1352, 0x1312, 0x1300, 0x0000 }, // R
  { 0x00,       FALSE, 0x1F73, 0x1F53, 0x1F13, 0x1F00, 0x0000 }, // S
  { 0x00,       FALSE, 0x1474, 0x1454, 0x1414, 0x1400, 0x0000 }, // T
  { 0x00,       FALSE, 0x1675, 0x1655, 0x1615, 0x1600, 0x0000 }, // U
  { 0x00,       FALSE, 0x2F76, 0x2F56, 0x2F16, 0x2F00, 0x0000 }, // V
  { 0x00,       FALSE, 0x1177, 0x1157, 0x1117, 0x1100, 0x0000 }, // W
  { 0x00,       FALSE, 0x2D78, 0x2D58, 0x2D18, 0x2D00, 0x0000 }, // X
  { 0x00,       FALSE, 0x1579, 0x1559, 0x1519, 0x1500, 0x0000 }, // Y
  { 0x00,       FALSE, 0x2C7A, 0x2C5A, 0x2C1A, 0x2C00, 0x0000 }, // Z
  { 0x00,       FALSE, 0x0B30, 0x0B29, 0x0000, 0x8100, 0x0000 }, // 0)
  { 0x00,       FALSE, 0x0231, 0x0221, 0x0000, 0x7800, 0x0000 }, // 1!
  { 0x00,       FALSE, 0x0332, 0x0340, 0x0300, 0x7900, 0x0000 }, // 2@
  { 0x00,       FALSE, 0x0433, 0x0423, 0x0000, 0x7A00, 0x0000 }, // 3#
  { 0x00,       FALSE, 0x0534, 0x0524, 0x0000, 0x7B00, 0x0000 }, // 4$
  { 0x00,       FALSE, 0x0635, 0x0625, 0x0000, 0x7C00, 0x0000 }, // 5%
  { 0x00,       FALSE, 0x0736, 0x075E, 0x0000, 0x7D00, 0x0000 }, // 6^
  { 0x00,       FALSE, 0x0837, 0x0826, 0x071E, 0x7E00, 0x0000 }, // 7&
  { 0x00,       FALSE, 0x0938, 0x092A, 0x0000, 0x7F00, 0x0000 }, // 8*
  { 0x00,       FALSE, 0x0A39, 0x0A28, 0x0000, 0x8000, 0x0000 }, // 9(
  { 0x00,       FALSE, 0x2960, 0x297E, 0x0000, 0x2900, 0x0000 }, // `~
  { 0x00,       FALSE, 0x0C2D, 0x0C5F, 0x0C1F, 0x8200, 0x0000 }, // -_
  { 0x00,       FALSE, 0x0D3D, 0x0D2B, 0x0000, 0x8300, 0x0000 }, // =+
  { 0x00,       FALSE, 0x2B5C, 0x2B7C, 0x2B1C, 0x2B00, 0x0000 }, // \|
  { 0x00,       FALSE, 0x0E08, 0x0E08, 0x0E7F, 0x0E00, 0x0000 }, // backspace
  { 0x00,       FALSE, 0x3920, 0x3920, 0x3920, 0x3920, 0x0000 }, // Space
  { 0x00,       FALSE, 0x0F09, 0x0F00, 0x9400, 0xA500, 0x0000 }, // tab
  { CAPSLCKBIT, FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Caps-Lock
  { LSHIFTBIT,  FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // L-Shift
  { 0xFF,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Fake L-Shift
  { CTRLBIT,    FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // L-CTRL
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // L-GUI
  { ALTBIT,     FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // L-Alt
  { RSHIFTBIT,  FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // R-Shift
  { 0xFF,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Fake R-Shift
  { CTRLBIT,    FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // R-CTRL
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // R-GUI
  { ALTBIT,     FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // R-Alt
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Apps (right of Right GUI)
  { 0x00,       FALSE, 0x1C0D, 0x1C0D, 0x1C0A, 0x1C00, 0x0000 }, // Enter (Main)
  { 0x00,       FALSE, 0x011B, 0x011B, 0x011B, 0x0100, 0x0000 }, // ESC
  { 0x00,       FALSE, 0x3B00, 0x5400, 0x5E00, 0x6800, 0x0000 }, // F1
  { 0x00,       FALSE, 0x3C00, 0x5500, 0x5F00, 0x6900, 0x0000 }, // F2
  { 0x00,       FALSE, 0x3D00, 0x5600, 0x6000, 0x6A00, 0x0000 }, // F3
  { 0x00,       FALSE, 0x3E00, 0x5700, 0x6100, 0x6B00, 0x0000 }, // F4
  { 0x00,       FALSE, 0x3F00, 0x5800, 0x6200, 0x6C00, 0x0000 }, // F5
  { 0x00,       FALSE, 0x4000, 0x5900, 0x6300, 0x6D00, 0x0000 }, // F6
  { 0x00,       FALSE, 0x4100, 0x5A00, 0x6400, 0x6E00, 0x0000 }, // F7
  { 0x00,       FALSE, 0x4200, 0x5B00, 0x6500, 0x6F00, 0x0000 }, // F8
  { 0x00,       FALSE, 0x4300, 0x5C00, 0x6600, 0x7000, 0x0000 }, // F9
  { 0x00,       FALSE, 0x4400, 0x5D00, 0x6700, 0x7100, 0x0000 }, // F10
  { 0x00,       FALSE, 0x8500, 0x8700, 0x8900, 0x8B00, 0x0000 }, // F11
  { 0x00,       FALSE, 0x8600, 0x8800, 0x8A00, 0x8C00, 0x0000 }, // F12
  { 0x00,       FALSE, 0x8600, 0x8800, 0x8A00, 0x8C00, 0x0000 }, // Prnt Screen
  { 0xFF,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Prnt Screen (break) (ignore it)
  { SCROLLBIT,  FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Scroll-Lock
  { 0x00,       FALSE, 0x8600, 0x8800, 0x8A00, 0x8C00, 0x0000 }, // Pause
  { 0x00,       FALSE, 0x1A5B, 0x1A7B, 0x1A1B, 0x1A00, 0x0000 }, // [{
  { INSERTBIT,  FALSE, 0x5200, 0x5230, 0x9200, 0xA200, 0x0000 }, // Insert
  { 0x00,       FALSE, 0x4700, 0x4737, 0x7700, 0x9700, 0x0000 }, // Home
  { 0x00,       FALSE, 0x4900, 0x4939, 0x8400, 0x9900, 0x0000 }, // Page Up
  { 0x00,       FALSE, 0x5300, 0x532E, 0x9300, 0xA300, 0x0000 }, // Delete
  { 0x00,       FALSE, 0x4F00, 0x4F31, 0x7500, 0x9F00, 0x0000 }, // End
  { 0x00,       FALSE, 0x5100, 0x5133, 0x7600, 0xA100, 0x0000 }, // Page Down
  { 0x00,       FALSE, 0x4800, 0x4838, 0x8D00, 0x9800, 0x0000 }, // Up Arrow
  { 0x00,       FALSE, 0x4B00, 0x4B34, 0x7300, 0x9800, 0x0000 }, // Left Arrow
  { 0x00,       FALSE, 0x5000, 0x5032, 0x9100, 0xA000, 0x0000 }, // Down Arrow
  { 0x00,       FALSE, 0x4D00, 0x4D36, 0x7400, 0x9D00, 0x0000 }, // Right Arrow
  { NUMLOCKBIT, FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Num-Lock
  { 0x00,       FALSE, 0x352F, 0x352F, 0x9500, 0xA400, 0x0000 }, // Keypad /
  { 0x00,       FALSE, 0x372A, 0x372A, 0x9600, 0x3700, 0x0000 }, // Gray * (old prtscrn)
  { 0x00,       FALSE, 0x4A2D, 0x4A2D, 0x8E00, 0x4A00, 0x0000 }, // Gray-
  { 0x00,       FALSE, 0x4E2B, 0x4E2B, 0x9000, 0x4E00, 0x0000 }, // Gray+
  { 0x00,       FALSE, 0x1C0D, 0x1C0D, 0x1C0A, 0x1C00, 0x0000 }, // Keypad Enter
  { 0x00,       TRUE,  0x342E, 0x532E, 0x0000, 0x0000, 0x5300 }, // Del/.
  { 0x00,       TRUE,  0x0B30, 0x0B29, 0x0000, 0x0000, 0x5200 }, // 0/Insert
  { 0x00,       TRUE,  0x0231, 0x0221, 0x0000, 0x0000, 0x4F00 }, // 1/End
  { 0x00,       TRUE,  0x0332, 0x0340, 0x0000, 0x0000, 0x5000 }, // 2/Down
  { 0x00,       TRUE,  0x0433, 0x0423, 0x0000, 0x0000, 0x5100 }, // 3/PgDn
  { 0x00,       TRUE,  0x0534, 0x0524, 0x0000, 0x0000, 0x4B00 }, // 4/Left
  { 0x00,       TRUE,  0x0635, 0x0625, 0x0000, 0x0000, 0x4C00 }, // 5/Center
  { 0x00,       TRUE,  0x0736, 0x075E, 0x0000, 0x0000, 0x4D00 }, // 6/Right
  { 0x00,       TRUE,  0x0837, 0x0826, 0x0000, 0x0000, 0x4700 }, // 7/Home
  { 0x00,       TRUE,  0x0938, 0x092A, 0x0000, 0x0000, 0x4800 }, // 8/Up
  { 0x00,       TRUE,  0x0A39, 0x0A28, 0x0000, 0x0000, 0x4900 }, // 9/PgUp
  { 0x00,       FALSE, 0x1B5D, 0x1B7D, 0x1B1D, 0x1B00, 0x0000 }, // ]}
  { 0x00,       FALSE, 0x273B, 0x273A, 0x0000, 0x2700, 0x0000 }, // ;:
  { 0x00,       FALSE, 0x2827, 0x2822, 0x0000, 0x2800, 0x0000 }, // '"
  { 0x00,       FALSE, 0x332C, 0x333C, 0x0000, 0x3300, 0x0000 }, // ,<
  { 0x00,       FALSE, 0x342E, 0x343E, 0x0000, 0x3400, 0x0000 }, // .>
  { 0x00,       FALSE, 0x352F, 0x353F, 0x0000, 0x3500, 0x0000 }, // /?
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Power
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Sleep
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Wake
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Next Track
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Previous Track
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Stop
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Play/Pause
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Mute
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Volume Up
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Volume Down
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Media Select
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // E-mail
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // Calculator
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // My Computer
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Search
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Home
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Back
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Forward
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Stop
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Refresh
  { 0x00,       FALSE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, // WWW Favorites
};

#endif  // FYSOS_SCANCODES
