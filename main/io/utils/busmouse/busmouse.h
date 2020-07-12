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

#define ABS_BASE  0x023C

// Standard Bus Mouse Adapter
//  Both MS and Logitech use this adapter
#define LOGM_DATA_PORT   (ABS_BASE + 0)
#define LOGM_SIG_PORT    (ABS_BASE + 1)
#define LOGM_CTRL_PORT   (ABS_BASE + 2)
#define LOGM_CNFIG_PORT  (ABS_BASE + 3)

#define	LOGM_ALLOW_COUNTER (0 << 7)
#define	LOGM_HOLD_COUNTER  (1 << 7)
#define	LOGM_READ_X        (0 << 6)
#define	LOGM_READ_Y        (1 << 6)
#define	LOGM_READ_LOW      (0 << 5)
#define	LOGM_READ_HIGH     (1 << 5)
#define	LOGM_ENABLE_IRQ	   (0 << 4)
#define	LOGM_DISABLE_IRQ   (1 << 4)

#define	LOGM_READ_X_LOW	  (LOGM_READ_X | LOGM_READ_LOW)
#define	LOGM_READ_X_HIGH  (LOGM_READ_X | LOGM_READ_HIGH)
#define	LOGM_READ_Y_LOW	  (LOGM_READ_Y | LOGM_READ_LOW)
#define	LOGM_READ_Y_HIGH  (LOGM_READ_Y | LOGM_READ_HIGH)

#define LOGM_BUTTON_LEFT  (1 << 7)
#define LOGM_BUTTON_MID   (1 << 6)
#define LOGM_BUTTON_RIGHT (1 << 5)

// MS Inport mouse
#define INPORT_CTRL_PORT   (ABS_BASE + 0)
#define INPORT_DATA_PORT   (ABS_BASE + 1)
#define INPORT_SIG_PORT    (ABS_BASE + 2)

#define INPORT_REG_BTNS         0x00
#define INPORT_REG_X            0x01
#define INPORT_REG_Y            0x02
#define INPORT_REG_MODE         0x07
#define INPORT_RAISE_IRQ        0x16
#define INPORT_RESET            0x80

#define INPORT_MODE_HOLD        0x20
#define INPORT_MODE_BASE        0x10
#define INPORT_MODE_IRQ         0x01

#define INPORT_BUTTON_LEFT   (1 << 2)
#define INPORT_BUTTON_MID    (1 << 1)
#define INPORT_BUTTON_RIGHT  (1 << 0)
#define INPORT_BUTTON_LEFTx  (1 << 5)
#define INPORT_BUTTON_MIDx   (1 << 4)
#define INPORT_BUTTON_RIGHTx (1 << 3)

// the two types of cards we know
enum {
  CARD_ID_INPORT = 0,
  CARD_ID_LOGITECH,
};

int bus_identify(void);
int bus_find_irq(const int id);

void interrupt log_mouse_irq();
void interrupt inport_mouse_irq();
