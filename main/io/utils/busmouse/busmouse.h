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
