# FYSOS
The FYSOS Operating System<br />
  http://www.fysnet.net/fysos.htm<br />
<br />
The included files are the boot process for the FYSOS Operating System for a UEFI system.<br />
<br />
This loader will find the files specified (see system_files[]), and load them to the locations specified
 within their respected headers.<br />
<br />
For example, after making the kernel.sys file (currently not included here, but you may specify your own), use
 the following command line:<br />
   makehdr kernel.sys /l0x00800000 /c1 /e /okernel0.sys /x /k<br />
 This will add the correct header to the kernel.sys file so that the loader loads the file to the
 correct place, in this case, 0x00800000.<br />
 See the makehdr.c file for more information on what the command line switches are.<br />
<br />
Your kernel file must then have an origin of 0x00800400 and have the first instruction ready to be executed at
 that location.<br />
<br />
* This code is a work in progress and is not complete and may be inaccurate.  Please use at your own risk<br />
* Place the BOOT.efi file in the \efi\boot\BOOT.efi folder of a properly formated UEFI GUID partition.<br />
<br />
These files are released as is:<br />
  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt<br />
  This code is freeware, not public domain.  Please use respectfully.<br />
<br />
You may:<br />
   - use this code for learning purposes only.<br />
   - use this code in your own Operating System development.<br />
   - distribute any code that you produce pertaining to this code<br />
     as long as it is for learning purposes only, not for profit,<br />
     and you give credit where credit is due.<br />
<br />
You may NOT:<br />
   - distribute this code for any purpose other than listed above.<br />
   - distribute this code for profit.<br />
<br />
You MUST:<br />
   - include this whole comment block at the top of this file.<br />
   - include contact information to where the original source is located.<br />
             https://github.com/fysnet/FYSOS<br />
<br />
The C files are built with Alex's SmallerC<br />
    https://github.com/alexfru/SmallerC<br />
Please note that since this code uses Wide Chars (wchar_t), you *MUST* have the modified version
 of SmallerC I have just for this purpose.  Contact me for more information on this modified version.<br />
<br />
You use these files at your own risk.<br />
<br />
If you have any questions, please contact me at the email address shown at the top of:<br />
  http://www.fysnet.net/fysos.htm<br />
<br />
Please note that there are other things I have forgotten to mention that you will need to take
 into consideration when using this code.  One thing is the 0x400 offset of the kernel file I use,
 though I don't think it matters here, but do check.  I will make note of things as I update the
 code and remember other things.<br />
<br />
If you find any errors or have any questions, please feel free to send me an email at:<br />
  fys [at] fysnet [dot] net<br />
<br />
For more information on how to write an Operating System, have a look at the following book series:<br />
  http://www.fysnet.net/osdesign_book_series.htm<br />
<br />
Ben
