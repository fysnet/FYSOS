# FYSOS
### [The FYSOS Operating System](http://www.fysnet.net/fysos.htm)

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
 
Your kernel file must then have an origin of **0x00800400** and have the first instruction ready to be executed at
 that location.
  
These files are released as is:
- [ ] Copyright (c) 1984-2021    Forever Young Software  Benjamin David Lunt
- [ ] This code is freeware, not public domain.  Please use respectfully.

You may:
- [X] use this code for learning purposes only.
- [X] use this code in your own Operating System development.
- [X] distribute any code that you produce pertaining to this code
     as long as it is for learning purposes only, not for profit,
     and you give credit where credit is due.

You may NOT:
- [X] distribute this code for any purpose other than listed above.
- [X] distribute this code for profit.

You MUST:
- [X] include this whole comment block at the top of this file.
- [X] include contact information to where the original source is located.

#### https://github.com/fysnet/FYSOS

The assembly files are built with the NewBasic Assembler

#### [NBASM ver 00.26.59](http://www.fysnet.net/newbasic.htm)

The C files are built with Alex's SmallerC

#### https://github.com/alexfru/SmallerC/

(Please see the note in the readme file for the EFI code)
    
You use these files at your own risk.
 
#### Latest binary or other questions
For the latest binary of my **complete OS in action**, or if you have any questions, please contact me at the email address shown below or visit:

#### http://www.fysnet.net/fysos.htm
 
Please note that there are other things I have forgotten to mention that you will need to take
 into consideration when using this code.  One thing is the 0x400 offset of the kernel file I use,
 though I don't think it matters here, but do check.  I will make note of things as I update the
 code and remember other things.

#### Errors, or it just flat out doesn't work
If you find any errors or have any questions, please feel free to send me an email.  It would
make it a lot easier on my part if you send me an URL to the image you tried.  If you don't
have an URL for the image, please contact me asking how you can send me the image.
If I have the image file you are using that produced the error, it will make it much easier
for me to fix the error.  Thank you.  You can contact me at:

#### fys [at] fysnet [dot] net

For more information on how to write an Operating System, have a look at the following book series:

#### http://www.fysnet.net/osdesign_book_series.htm
  
Ben
