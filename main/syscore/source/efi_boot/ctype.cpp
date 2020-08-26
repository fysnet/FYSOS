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
 *  CTYPE.CPP
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

#include "ctype.h"

bit8u _ctype[256] = {
/*   0 */  __C, __C, __C, __C, __C, __C, __C, __C, 
/*   8 */  __C, __C|__W, __C|__W, __C|__W, __C|__W, __C|__W, __C, __C, 
/*  16 */  __C, __C, __C, __C, __C, __C, __C, __C, 
/*  24 */  __C, __C, __C, __C, __C, __C, __C, __C, 
/*  32 */  __W|__S, __P, __P, __P, __P, __P, __P, __P, 
/*  40 */  __P, __P, __P, __P, __P, __P, __P, __P, 
/*  48 */  __D, __D, __D, __D, __D, __D, __D, __D, 
/*  56 */  __D, __D, __P, __P, __P, __P, __P, __P, 
/*  64 */  __P, __U|__H, __U|__H, __U|__H, __U|__H, __U|__H, __U|__H, __U, 
/*  72 */  __U, __U, __U, __U, __U, __U, __U, __U, 
/*  80 */  __U, __U, __U, __U, __U, __U, __U, __U, 
/*  88 */  __U, __U, __U, __P, __P, __P, __P, __P, 
/*  96 */  __P, __L|__H, __L|__H, __L|__H, __L|__H, __L|__H, __L|__H, __L, 
/* 104 */  __L, __L, __L, __L, __L, __L, __L, __L, 
/* 112 */  __L, __L, __L, __L, __L, __L, __L, __L, 
/* 120 */  __L, __L, __L, __P, __P, __P, __P, __C, 
/* 128 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 136 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 144 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 152 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 160 */  __X, __X, __X, __X, __X, __X, __X, __X, 
/* 168 */  __X, __X, __X, __X, __X, __X, __X, __X, 
/* 176 */  __X, __X, __X, __X, __X, __X, __X, __X, 
/* 184 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 192 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 200 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 208 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 216 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 224 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 232 */  __X, __X, __X, __X, __X, __X, __X, __X, 
/* 240 */  __X, __X, __X, __X, __X, __X, __X, __X,
/* 248 */  __X, __X, __X, __X, __X, __X, __X, __X
};

int tolower(int ch) { return (isupper(ch)) ? ch += 32 : ch; }
int toupper(int ch) { return (islower(ch)) ? ch -= 32 : ch; }
