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
 * Shared with ps2mouse.c, ps2key.c, busmouse.c, and serial.c
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
