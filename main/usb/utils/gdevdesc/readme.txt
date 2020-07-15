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

Notes:

1. These utilities will try to retrieve the Device Descriptor of any attached
   USB device.

2. These utilities will only see a device attached to the root hub.  
   i.e.:  It will not detect a device plugged in to an external hub.

3. Three of the four utilities, UHCI, OHCI, and XHCI, will see any device attached
   as a full- or low-speed device or a super-speed device (XHCI), simply because 
   of the way the USB is designed. However, the EHCI utility will only work with
   high-speed devices.  Because of the way the USB is designed, any low- or
   full-speed device will be handed off to the companion controller(s).  When
   using the EHCI utility, be sure to test with high-speed devices only.

4. If your machine has a Rate Matching Hub attached to the root port, these
   utilities may not see anything attached, since they will not extend past
   the hub.


****** Important ******
   Unfortunately, modern hardware no longer has 1.44m floppy drives and expects
   you to boot from a USB thumb drive.  If this is the case, the System Management
   part of the BIOS/Firmware may keep control of the USB and not allow any of
   these utilities to see an attached device. I have experienced this on many
   newer model machines.

   Unfortunately, I can only get these to work on a machine that I can completely 
   turn off all BIOS/Firmware USB emulation.  However, by doing so, you can no 
   longer boot the USB thumb drive to get to the utilities to test them.
   
   If you have an older machine with a 1.44m floppy drive, it is best to turn
   off all USB emulation, boot from the floppy, and run these utilities from
   that floppy.  Each utility is confirmed to work when ran from a floppy with
   USB emulation turned off.

   It is ironic that the USB has become so integrated into our systems that
   the BIOS/Firmware no longer allows you to program the USB with a simple
   utility such as these included here.  However, without this integration,
   the USB would not be what it is today.
   
   However, I hope the source code for these utilities will show you how to 
   program your system drivers to be able to retrieve Device Descriptors.


Notes common to all four utilities.

1. The heap_alloc() function does not have a corresponding free() function.
   The intent of heap_alloc() is only to allocate from our heap, memory
   with a physical address.  It is assumed that there will be enough memory
   in the heap and the allocated memory does not need to be free'd except
   when the heap itself is free'd.

   It was not my intent to have a robust allocation scheme.  Just a simple
   allocation scheme to show the examples.  You will need to create your
   own allocation scheme if you plan to do more memory allocation.

   Since there is no free() function, if you call the control_in() funtions
   multiple times, you will quickly run out of memory.

2. These examples do not have as much error checking as they probably should.
   It is not my intent to show how to catch common errors.  It is my intent
   to show how to retrieve the Device Descriptor of an attached device.

3. These examples assume that there is only one device plugged in on any
   given downstream port.  i.e.:  These utilities will not retrieve nor
   communicate with any devices plugged into an external hub.  These utilities
   will only retrieve the Device Descriptor from the device plugged in to
   the root hub port.  However, you may have multiple root hub ports occupied.

4. These examples are intended to run on DOS.  They will not run on Linux or
   Windows.  These examples are intended to show how to communicate with the
   USB without relying on any underlining operating system, except for the
   actual memory allocation used.

