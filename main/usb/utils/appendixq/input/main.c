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

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  // for sei()

#include <util/delay.h>     // for _delay_ms()

#include <avr/eeprom.h>
#include <avr/pgmspace.h>   // required by usbdrv.h

#include "usbdrv.h"

static struct {
  uint8_t pins[8];
} state_report;

PROGMEM char usbHidReportDescriptor[42] = { // USB report descriptor
  0x06, 0x00, 0xFF,     // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,           // USAGE (Vendor Usage 1)
  0xA1, 0x01,           // COLLECTION (Application)
  0x15, 0x00,           // LOGICAL_MINIMUM (0)
  0x26, 0xFF, 0x00,     // LOGICAL_MAXIMUM (255)
  0x75, 0x08,           // REPORT_SIZE (8)
  
  // read state
  0x85, 0x00,           // REPORT_ID (0)
  0x95, sizeof(state_report), // REPORT_COUNT
  0x09, 0x00,           // USAGE (Undefined)
  0xB2, 0x02, 0x01,     // FEATURE (Data,Var,Abs,Buf)
  
  0xC0                  // END_COLLECTION
};

// -------------------------------------------------------------------------

#define CONFIG_TOP ((uint8_t *) 0)
#define CONFIG_LEN 128

static uchar currentAddress;
static uchar bytesRemaining;
static uchar report_id;

void buildStateReport() {
  uchar pins;
  
  pins = PINB;
  state_report.pins[0] = (pins & _BV(PB0)) ? 1 : 0;
  state_report.pins[1] = (pins & _BV(PB1)) ? 1 : 0;
  state_report.pins[2] = (pins & _BV(PB2)) ? 1 : 0;
  state_report.pins[3] = (pins & _BV(PB3)) ? 1 : 0;
  state_report.pins[4] = (pins & _BV(PB4)) ? 1 : 0;
  state_report.pins[5] = (pins & _BV(PB5)) ? 1 : 0;
  state_report.pins[6] = (pins & _BV(PB6)) ? 1 : 0;
  state_report.pins[7] = (pins & _BV(PB7)) ? 1 : 0;
}

uchar usbFunctionRead(uchar *data, uchar len) {
    if (len > bytesRemaining) len = bytesRemaining;
    eeprom_read_block(data, CONFIG_TOP + currentAddress, len);
    currentAddress += len;
    bytesRemaining -= len;
    return len;
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (void *)data;
  
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {  // HID class request
    report_id = rq->wValue.bytes[0];
    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      if (report_id == 0) {
        buildStateReport();
        usbMsgPtr = (void *) &state_report;
        return sizeof(state_report);
      }
    }
  }
  
  return 0;
}

int main(void) {
  wdt_enable(WDTO_1S);
  usbInit();
  
  // make all PORTB pins input
  DDRB = 0;
  
  sei();
  while (1) {        // main event loop
    wdt_reset();
    usbPoll();
  }
  
  return 0;
}
