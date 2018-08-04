# FYSOS
The FYSOS Operating System
  http://www.fysnet.net/fysos.htm
  
The included files are the boot process for the FYSOS Operating System.

Within the /boot directory are the boot sector source code files for various techniques and file systems.
Within the /loader directory are the loader files for various file systems.

The loader will find the files specified (see system_files[]), and load them to the locations specified
 within their respected headers.
 
For example, after making the kernel.sys file (currently not included here, but you may specify your own), use
 the following command line:
   makehdr kernel.sys /l0x00800000 /c1 /e /okernel0.sys /x /k
 This will add the correct header to the kernel.sys file so that the loader loads the file to the
 correct place, in this case, 0x00800000.
 See the makehdr.c file for more information on what the command line switches are.
 
Your kernel file must then have an origin of 0x00800000 and have the first instruction ready to be executed at
 that location.
  
These files are released as is:
  Copyright (c) 1984-2018    Forever Young Software  Benjamin David Lunt
  This code is freeware, not public domain.  Please use respectfully.

You may:
   - use this code for learning purposes only.
   - use this code in your own Operating System development.
   - distribute any code that you produce pertaining to this code
     as long as it is for learning purposes only, not for profit,
     and you give credit where credit is due.

You may NOT:
   - distribute this code for any purpose other than listed above.
   - distribute this code for profit.

You MUST:
   - include this whole comment block at the top of this file.
   - include contact information to where the original source is located.
             https://github.com/fysnet/FYSOS

The assembly files are built with the NewBasic Assembler
    http://www.fysnet/newbasic.htm
         NBASM ver 00.26.59

The C files are built with Alex's SmallerC
    https://github.com/alexfru/SmallerC
    
You use these files at your own risk.
 
If you have any questions, please contact me at the email address shown at the top of:
  http://www.fysnet.net/fysos.htm
 
At this time, the kernel.sys and system.sys files are not included.  However, this boot and loader
 code is written to allow any kernel and system file to be loaded.  Create your own  :-)
  
Please note that there are other things I have forgotten to mention that you will need to take
 into consideration when using this code.  One thing is the 0x400 offset of the kernel file I use,
 though I don't think it matters here, but do check.  I will make note of things as I update the
 code and remember other things.
  
If you find any errors or have any questions, please feel free to send me an email at:
  fys [at] fysnet [dot] net

For more information on how to write an Operating System, have a look at the following book series:
  http://www.fysnet.net/osdesign_book_series.htm
  
Ben
