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
void sdelay(const bit32u s) {
  _delay((bit64u) s * cpu_hz); 
} 

#endif // MEDIA_TIMER
