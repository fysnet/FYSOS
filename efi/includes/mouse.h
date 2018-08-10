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
 * Note:  Since this code uses wide chars (wchar_t), you *MUST* have my modified 
 *        version of SmallerC.  Contact me for more information.
 *        
 */

#ifndef MOUSE_H
#define MOUSE_H

#include "config.h"
#if UEFI_INCLUDE_MOUSE

int InitMouse(void);
void UpdateMouse();


struct EFI_SIMPLE_POINTER_STATE {
  INT32   RelativeMovementX; // The signed distance in counts that the pointer device has been moved along the x-axis.
  INT32   RelativeMovementY; // The signed distance in counts that the pointer device has been moved along the y-axis.
  INT32   RelativeMovementZ; // The signed distance in counts that the pointer device has been moved along the z-axis.
  BOOLEAN LeftButton;        // If TRUE, then the left button of the pointer device is being
  BOOLEAN RightButton;       // If TRUE, then the right button of the pointer device is being
};

struct EFI_SIMPLE_POINTER_MODE {
  bit32u  ResolutionX[2];  // The resolution of the pointer device on the x-axis in counts/mm.
  bit32u  ResolutionY[2];  // The resolution of the pointer device on the y-axis in counts/mm.
  bit32u  ResolutionZ[2];  // The resolution of the pointer device on the z-axis in counts/mm.
  BOOLEAN LeftButton;   // TRUE if a left button is present on the pointer device. Otherwise FALSE.
  BOOLEAN RightButton;  // TRUE if a right button is present on the pointer device. Otherwise FALSE.
};

struct EFI_SIMPLE_POINTER_PROTOCOL {
  EFI_STATUS (*Reset)(struct EFI_SIMPLE_POINTER_PROTOCOL *ssp, bool ExtendedVerification);
  EFI_STATUS (*GetState)(struct EFI_SIMPLE_POINTER_PROTOCOL *ssp, struct EFI_SIMPLE_POINTER_STATE *state);
  void  *WaitForInput;
  struct EFI_SIMPLE_POINTER_MODE *Mode;
};



int InitMouse(void);
void UpdateMouse();

#endif // UEFI_INCLUDE_MOUSE

#endif // MOUSE_H
