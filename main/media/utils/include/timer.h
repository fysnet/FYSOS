
#ifndef MEDIA_TIMER
#define MEDIA_TIMER

// The gcc startup code should check to see if we are a 32-bit system.
// Since we loaded the DMPI already, it would have errored if not a 32-bit system.

bit64u cpu_hz;

bit64u rdtsc() {
  bit64u val; 
  __asm__ __volatile__ ("rdtsc" : "=A" (val)); 
  return val; 
}

unsigned idflag() {
  register unsigned long eax __asm__("ax");
  
  asm("pushfl\n"
      "popl  %eax\n"
      "orl   $0x200000, %eax\n"
      "pushl %eax\n"
      "popfl\n"
      "pushfl\n"
      "popl  %eax\n"
      "andl  $0x200000, %eax\n"
  );
  
  return (eax);
}

bool setup_timer() {
  
  bit32u eax, edx;
  bit64u start, end;
  
  if (!idflag())
    return FALSE;
  
  __asm__ __volatile__ (
    "movl  $0x000000001,%%eax\n"
    "cpuid\n"
    "movl  %%edx,%0"
	  : "=g" (edx)
    :
    : "eax", "ebx", "ecx", "edx"
  );
  
  if (edx & 0x10) {
    // disable the speaker, enable the T2 gate.
    outportb(0x61, (inportb(0x61) & ~0x02) | 0x01);
    
    // Set the PIT to Mode 0, counter 2, word access.
    outportb(0x43, 0xB0);
    
    // Load the counter with 0xFFFF
    outportb(0x42, 0xFF);
    outportb(0x42, 0xFF);
    
    // Read the number of ticks during the period.
    start = rdtsc();
    while (!(inportb(0x61) & 0x20))
      ;
    end = rdtsc();
    
    cpu_hz = (bit64u) ((end - start) * (bit64u) 1193180U / (bit64u) 0xFFFF);
    return TRUE;
  } else
    return FALSE;
}

// n = amount of rdtsc timer ticks to delay
void _delay(const bit64u n) {
  volatile const bit64u timeout = (rdtsc() + n);
  while (rdtsc() < timeout)
    ;
}

// delay for a specified number of nanoseconds. 
//  n = number of nanoseconds to delay.
void ndelay(const bit32u n) {
  _delay(((bit64u) n * cpu_hz) / (bit64u) 1000000000); 
} 
 
// delay for a specified number of microseconds. 
//  n = number of microseconds to delay. 
void udelay(const bit32u n) {
  _delay(((bit64u) n * cpu_hz) / (bit64u) 1000000); 
} 

// delay for a specified number of milliseconds. 
//  m = number of milliseconds to delay. 
void mdelay(const bit32u m) {
  _delay(((bit64u) m * cpu_hz) / (bit64u) 1000);
} 

// delay for a specified number of seconds. 
//  s = number of seconds to delay. 
void delay(const bit32u s) {
  _delay((bit64u) s * cpu_hz); 
} 

#endif // MEDIA_TIMER
