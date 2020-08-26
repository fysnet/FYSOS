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
 *  CTYPE.H
 *  This is a helper source file for a demo bootable image for UEFI.
 *
 *  Assumptions/prerequisites:
 *    32-bit or 64-bit
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   See BOOT.CPP
 */

#ifndef CTYPE_H
#define CTYPE_H


#include "../include/ctype.h"

// CTYPE items such as isalpha, etc.
//  These are just #defines instead of function calls
//  Adds a slight bit more code to each calling file, but is much faster.
#define __X    0x00    // unknown
#define __U    0x01    // upper
#define __L    0x02    // lower
#define __D    0x04    // digit
#define __C    0x08    // cntrl
#define __P    0x10    // punct
#define __W    0x20    // white space (space/lf/tab)
#define __H    0x40    // hex digit
#define __S    0x80    // hard space (0x20)

extern bit8u _ctype[256];

#define isalnum(ch)  ((_ctype[(int)(bit8u)(ch)] & (__U|__L|__D)) != 0)       // 'a' - 'z' || 'A' - 'Z' || '0' - '9'
#define isalpha(ch)  ((_ctype[(int)(bit8u)(ch)] & (__U|__L)) != 0)           // 'a' - 'z' || 'A' - 'Z'
#define iscntrl(ch)  ((_ctype[(int)(bit8u)(ch)] & (__C)) != 0)               //  0 -> 1Fh, 7Fh
#define isdigit(ch)  ((_ctype[(int)(bit8u)(ch)] & (__D)) != 0)               // '0' -> '9'
#define isgraph(ch)  ((_ctype[(int)(bit8u)(ch)] & (__P|__U|__L|__D)) != 0)   // all printable except whitespace chars
#define islower(ch)  ((_ctype[(int)(bit8u)(ch)] & (__L)) != 0)               // 'a' -> 'z'
#define isprint(ch)  ((_ctype[(int)(bit8u)(ch)] & (__C)) == 0)               // all chars that are not control chars
#define ispunct(ch)  ((_ctype[(int)(bit8u)(ch)] & (__P)) != 0)               // 
#define isspace(ch)  ((_ctype[(int)(bit8u)(ch)] & (__W|__S)) != 0)           // 09h,0Ah,0Bh,0Ch,0Dh,20h
#define isupper(ch)  ((_ctype[(int)(bit8u)(ch)] & (__U)) != 0)               // 'A' -> 'Z'
#define isxdigit(ch) ((_ctype[(int)(bit8u)(ch)] & (__D|__H)) != 0)           // '0' - '9' || 'a' - 'f' || 'A' - 'F'

#define isascii(ch)  ((bit8u)(ch) <= 0x7F)
#define toascii(ch)  ((bit8u)(ch) & 0x7F)

int toupper(int c);
int tolower(int c);

#endif // CTYPE_H
