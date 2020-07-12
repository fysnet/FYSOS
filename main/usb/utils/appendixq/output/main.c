
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  // for sei()

#include <util/delay.h>     // for _delay_ms()

#include "usbdrv.h"

// -------------------------------------------------------------------------

// this gets called when custom control message is received 
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) { 
  usbRequest_t *rq = (void *)data; // cast data to correct type 
  
  if ((rq->bRequest >= 0) && (rq->bRequest <= 7)) { // custom command is in the bRequest field 
    PORTB ^= (1 << rq->bRequest); // toggle LED on pin
    return 0; 
  }
  
  return 0;
}

int main(void) {
  wdt_enable(WDTO_1S);
  usbInit();
  
  // make all PORTB pins output
  DDRB = 0xFF;
  PORTB = 0;
  
  sei();
  while (1) {        // main event loop
    wdt_reset();
    usbPoll();
  }
  
  return 0;
}
