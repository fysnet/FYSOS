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
 *  BOOT.C
 *  This is the main C source file for a demo bootable image for UEFI.
 *  This code will simply print a few chars to the screen.
 *
 *  To use:
 *   You need a GPT formatted disk image with at least one partition entry
 *   formatted to FAT32 (FAT16 works with most EFI systems), with the following
 *    files in the \EFI\BOOT\ directory:
 *     BOOTIA32.EFI
 *     startup.nsh
 *   Then boot the image using an EFI compatible emulator such as Oracle VM VirtualBox
 *    or QEMU
 *
 *  Assumptions/prerequisites:
 *    32-bit only
 *
 *  Last updated: 23 Aug 2020
 *
 *  To Build:
 *   You need the NewBasic C Compiler found at:  http://www.fysnet.net/newbasic.htm
 *    and the Flat Assembler found at: https://flatassembler.net/
 *   Then use the following command lines to build 'BOOTIA32.EFI'
 *
 *   nbc boot.c -fasm -efi
 *   fasm boot.asm
 *   ren boot.efi BOOTIA32.EFI
 */

#pragma proc(486)     // allow atleast 486 instructions
#pragma proc(long)    // we want 32-bit offsets
#pragma ptype(pmode)  //.pmode (all EFI code is in pmode)

#include "../include/ctype.h"
#include "efi_32.h"

/*
 * include the remaining parts of the library
 */
#include "efi_32.c"
#include "conout.c"


/*
 * efi_main()
 * this is what gets called by the EFI boot services
 */
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable) {
  
  // initialize our library code
  if (!InitializeLib(ImageHandle, SystemTable))
    return 1;
  
  // clear the screen
  cls();
  
  // print the Hello World string
  puts(L"Hello, World!");
  
  // freeze
  while (1)
    _asm ("hlt \n");
  
  // done
  return EFI_SUCCESS;
}
