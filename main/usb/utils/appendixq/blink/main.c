
#include <avr/io.h>
#include <avr/interrupt.h>  // for sei()

#include <util/delay.h>     // for _delay_ms()

int main(void) {
  
  // make bottom 2 PORTB pins output
  DDRB = 3;
  
  while (1) {        // main event loop
    PORTB = 1; // Turn LED on PB0 on and PB1 off
    _delay_ms(500); 
    PORTB = 2; // Turn LED on PB0 off and PB1 on
    _delay_ms(500); 
  }
  
  return 0;
}
