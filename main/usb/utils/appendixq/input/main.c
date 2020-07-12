
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
