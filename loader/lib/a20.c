
#include "ctype.h"
#include "conio.h"
#include "stdio.h"
#include "sys.h"

#include "a20.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// waits while keyboard status bit 1 = 1
// returns TRUE if timed-out, else FALSE if okay
// (used in a20 above)
bool wait_kbd_status(const int bit) {
  for (int i=0; i<65535; i++)
    if (!(inpb(0x64) & (1 << bit)))
      return FALSE;
  
  return TRUE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// tests and activates the a20 line.
//  returns in al, the number of the technique used.
//  if can not set the a20 line, this functions prints an error and freezes.
bit8u set_a20_line(void) {
  bit8u byte;
  struct REGS regs;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // make sure no interrupts bother us
  //_asm ("  cli  \n");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // first check to see if it is already active
  if (test_a20()) {
    //_asm ("  sti  \n");
    return 0;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 1: BIOS a20 enable service (later PS/2's)
  regs.eax = 0x00002401;
  if (!intx(0x15, &regs) && !(regs.eax & 0x0000FF00)) {
    // now test the a20 line
    if (test_a20())
      return 1;        // BIOS a20 service 2401h
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // it was not already set, so try
  // Method 2: keyboard controller
  if (!wait_kbd_status(1)) {    // waits while keyboard status bit 1 = 1
    outpb(0x64, 0xD0);
    if (!wait_kbd_status(0)) {  // waits while keyboard status bit 0 = 0
      byte = inpb(0x60);
      if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
        outpb(0x64, 0xD1);
        if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
          outpb(0x60, byte | 2);
          // Wait for the A20 line to settle down (up to 20usecs)
          outpb(0x64, 0xFF);  // Send FFh (Pulse Output Port NULL)
          if (!wait_kbd_status(1)) {  // waits while keyboard status bit 1 = 1
            // now test the a20 line
            if (test_a20())
              return 2;        // keyboard method
          }
        }
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 3: fast a20 (port 0x92)
  byte = inpb(0x92);
  //byte &= 0xFE;  // make sure we don't do a reset
  outpb(0x92, byte | 2);
  
  // now test the a20 line
  if (test_a20())
    return 3;        // fast port 92h
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 4: Keyboard controller: Command DFh
  if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
    outpb(0x60, 0xDF);       // Send command DFh
    // read in the acknowledge
    if (!wait_kbd_status(0)) {   // waits while keyboard status bit 0 = 0
      if (inpb(0x60) == 0xFA) {  // if not ACK, then error
        // Wait for the A20 line to settle down (up to 20usecs)
        // Some UHCI controllers when using legacy mode, need the FF (null command)
        //  sent after the above DF command.
        outpb(0x64, 0xFF);    // Send FFh (Pulse Output Port NULL)
        if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
          // now test the a20 line
          if (test_a20())
            return 4;        // keyboard method: command DFh
        }
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 5: Brute force of Method 1
  outpb(0x64, 0xD1);
  if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
    outpb(0x60, 0xDF);
    if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
      outpb(0x64, 0xFF);
      if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
        // now test the a20 line
        if (test_a20())
          return 5;        // keyboard method: brute force of #1
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 6: 


/*
! For the HP Vectra
        call    empty_8042
        jnz     err
        mov     al,#0xdf
        out     #0x64,al
        call    empty_8042
        jnz     err
        mov     al,#0xdf        ! Do it again
        out     #0x64,al
        call    empty_8042
        jnz     err
! Success

also look at
  c:\temp\fysos\new\himem\himem.asm for multiple ways to enable
  the a20 line for multiple machines.
    starts at 'AT_A20Handler'
*/
 
 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 // if we make it here, then we didn't set the a20 line
 // say so and freeze.
 printf("\nUnable to set the a20 line.  Halting..."
        "\nPlease report this to me at fys@fysnet.net"
        "\nInclude as much information about your computer"
        "\n along with brand and version of BIOS."
        "\n"
        "\nThank you.");
 freeze();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// tests whether the a20 line is set.
//  we do this by writing a value to 0x00100900, and then check the value
//  at 0x00000900 to see if they are the same.  i.e.: 0x00100900 will wrap
//  around to 0x00000900 if the a20 line is not active.
// we use 0x00100900 since the point of wrap around, 0x00000900, does not
//  contain anything valid already and can be overwritten.
//  return TRUE if active
bool test_a20() {
  asm (
    "  mov  edi,00100900h                         \n"
    "  mov  esi,00000900h                         \n"
    "                                             \n"
    "  ; read and store the original dword at [edi] \n"
    "  mov  eax,[edi]                             \n"
    "  push eax        ; save the current value   \n"
    "                                             \n"
    "  xor  eax,eax    ; assume not set (return 0) \n"
    "                                             \n"
    "  ; loop 32 times to make sure               \n"
    "  mov  ecx,32                                \n"
    ".1: add  dword [edi],01010101h               \n"
    "  mov  ebx,[edi]                             \n"
    "  cmp  ebx,[esi]                             \n"
    "  loope .1                                   \n"
    "                                             \n"
    "  ; if equal jmp over (return 1)             \n"
    "  je  short .2                               \n"
    "  inc  eax     ; eax = 1, A20 is set         \n"
    ".2:                                          \n"
    "                                             \n"
    "  ; if the values are still the same, the two pointers \n"
    "  ;  point to the same location in memory.             \n"
    "  ; i.e.: the a20 line is not set                      \n"
    "                                             \n"
    "  pop  ebx     ; restore the original value  \n"
    "  mov  [edi],ebx                             \n"
  );
}
