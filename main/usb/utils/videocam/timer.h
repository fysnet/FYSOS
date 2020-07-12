
///////////////////////////////////////////////////////////////////////////////////////////////
// This is the timer delay code.
//
bit64u cpu_hz;   // clock ticks per second

// read the Time Stamp Count
bit64u rdtsc() {
  bit64u val; 
  __asm__ __volatile__ ("rdtsc" : "=A" (val)); 
  return val; 
}

// Check to see if the processor has the RDTSC instruction.
// This assumes the processor has the CPUID instruction.
bool setup_timer() {
  
  bit32u eax, edx;
  bit64u start, end;
  
  __asm__ __volatile__ (
    "movl  $0x000000001,%%eax\n"
    "cpuid\n"
    "movl  %%edx,%0"
	  : "=g" (edx)
    :
    : "eax", "ebx", "ecx", "edx"
  );
  
  if (edx & 0x10) {
    
    // Set up the PPC port - disable the speaker, enable the T2 gate.
    outb(0x61, (inb(0x61) & ~0x02) | 0x01);
    
    // Set the PIT to Mode 0, counter 2, word access.
    outb(0x43, 0xB0);
    
    // Load the counter with 0xFFFF
    outb(0x42, 0xFF);
    outb(0x42, 0xFF);
    
    // Read the number of ticks during the period.
    start = rdtsc();
    while (!(inb(0x61) & 0x20))
      ;
    end = rdtsc();
    
    cpu_hz = (bit64u) ((end - start) * (bit64u) 1193180 / (bit64u) 0xFFFF);
    
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


///////////////////////////////////////////////////////////////////////////////////////////////
// This is the interrupt handler code.
//

_go32_dpmi_seginfo old_handler, new_handler;

#define IRQ_MASK_BIT(irq) ((irq) < 8 ? 1 << (irq) : (1 << (irq)) >> 8) 
#define IRQ_VECTOR(irq) ((irq) < 8 ? (irq) + 0x08 : (irq) - 8 + 0x70) 

void mask_pic(int irq) {
  int pic = (irq < 8) ? 0x21 : 0xA1;
  int irq_bit = IRQ_MASK_BIT(irq);
  outportb(pic, (inportb(pic) | irq_bit));
}

void unmask_pic(int irq) {
  int pic = (irq < 8) ? 0x21 : 0xA1;
  int irq_bit = ~IRQ_MASK_BIT(irq);
  outportb(pic, (inportb(pic) & irq_bit));
}

int set_interrupt_handler(const int irq, const int handler) {
  
  int ret = _go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &old_handler);
  
  new_handler.pm_offset = handler;
  new_handler.pm_selector = _go32_my_cs();
  ret |= _go32_dpmi_allocate_iret_wrapper(&new_handler);
  ret |= _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_handler);
  
  if (ret == 0)
    unmask_pic(irq);
  
  return ret;
}

int stop_interrupt_handler(const int irq) {
  mask_pic(irq);
  
  int ret = _go32_dpmi_free_iret_wrapper(&new_handler);
  ret |=    _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &old_handler);
  
  return ret;
}
 