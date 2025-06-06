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
 *  Shared with ps2mouse.c, ps2key.c, busmouse.c, and serial.c
 *
 *  Last updated: 12 Feb 2022
 *
 */

#ifndef FYSOS_PIC
#define FYSOS_PIC

#define PIC_MASTER  0x20
#define PIC_SLAVE   0xA0
#define EOI         0x20

#define PS2MOUSEIRQ   12
#define PS2KEYIRQ      1

void (interrupt far *old_isr)(void);   // holds old interrupt handler
// DOS's/BIOS' interrupt vector table
int ivt_num[16] = { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };

// mask off an IRQ.
//  irq = (15 - 0) irq to mask (set)
void picmask(const int irq) {
  // only do if 0-15
  if ((irq >= 0) && (irq <= 7))
    outpb(PIC_MASTER + 1, inpb(PIC_MASTER + 1) | (1 << irq));
  else if ((irq >= 8) && (irq <= 15))
    outpb(PIC_SLAVE + 1, inpb(PIC_SLAVE + 1) | (1 << (irq - 8)));
  else
    ;
}

// mask on an IRQ.
//  irq = (15 - 0) irq to unmask (clear)
void picunmask(const int irq) {
  // only do if 0-15
  if ((irq >= 0) && (irq <= 7))
    outpb(PIC_MASTER + 1, inpb(PIC_MASTER + 1) & ~(1 << irq));
  else if ((irq >= 8) && (irq <= 15))
    outpb(PIC_SLAVE + 1, inpb(PIC_SLAVE + 1) & ~(1 << (irq - 8)));
  else
    ;
}

#endif  // FYSOS_PIC
