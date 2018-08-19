/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 *
 */


#include "config.h"
#include "ctype.h"
#include "efi_32.h"

#include "conin.h"

// ReadKeyStroke returns EFI_NOT_READY if no key available
// ReadKeyStroke returns EFI_SUCCESS if a key is available
//  It will not wait for a key to be available.
EFI_STATUS kbhit(struct EFI_INPUT_KEY *Key) {
  return gSystemTable->ConIn->ReadKeyStroke(gSystemTable->ConIn, Key);
}

// Wait for a key to be available, then read the key using ReadKeyStroke
EFI_STATUS getkeystroke(struct EFI_INPUT_KEY *Key) {
  WaitForSingleEvent(gSystemTable->ConIn->WaitForKey, 0);
  return gSystemTable->ConIn->ReadKeyStroke(gSystemTable->ConIn, Key);
}
