This source code is to help me test a few things for my book as well as
show you what the book's GUI system is capable of.

* Requirements:
  Intel Compatible 386+ CPU
  At least 32Meg RAM
  Hard drive with a DOS bootable partition
    or
  1.44 floppy disk drive
  PS2 or Serial Mouse
  a DPMI is required (https://en.wikipedia.org/wiki/CWSDPMI)

* To install on Hard drive
  You will need a bootable DOS (FreeDOS, MS DOS, or other) partition.
  Copy the files from this source directory to a folder of your choosing within
   that partition.
  Compile using DJGPP (http://www.delorie.com/djgpp/)
  Create another directory and copy the .\RELEASE\*.* files along with this newly
   compiled .EXE file to that directory.
  Load the Mouse driver (CTMOUSE)
  Start the GUI (GUI.EXE) and follow directions.

* Starting the demo
  Once you have booted your DOS partition, simply enter GUI at the DOS prompt
   and press Enter.
    GUI <enter>
  The demo will check for a mouse driver installed, then display a list of
   available video modes. If more modes are available, press ENTER to continue
   to the next page, ESC to cancel.
  Choose a mode by pressing the corresponding letter for that mode, listed on
   the left-hand side.
  The demo will then load all files needed. (If you are using the floppy
   install method, this may take a few minutes.  You may use the /v parameter
   to display status as it is loaded.
    GUI /v<enter>
  Once the GUI Demo is loaded, you may use the mouse to manipulate the objects
   on the screen.
  Once you are finished, use the System Menu button at the top left of the
   screen, then select Exit, or press the Alt-Ctrl-Q key combination.
  Please read the notes below

* Notes:
  These tests will NOT work on anything but TRUE DOS or FreeDOS.  I have ran it 
  without (noticeable) errors on a FreeDOS 1.44m floppy boot as well as a Win98SE
   DOS only boot, both from the floppy and a hard drive partition.
  This GUI demo is simply to show you what a GUI is capable of.
  It does not function as a normal GUI due to the lack of a multi-tasking
   environment.  For example, all of the Message Boxes do nothing but close
   when you press a button.  If a single tasking environment (such as the
   one used here) was to wait for the user to press a button, nothing else
   would be displayed nor would the mouse and mouse events work.
  The text cursor within the demo Editor may not work correctly.  My intent is to
   show how to display the cursor and the text, not create a proper text editor.
  You may see errors or other items through out the demo.  This is expected
   and please send me your comments/results via an email (listed below).
  The intent of this GUI Demo is not to show how to create a fully functional
   operating system GUI, but to show how to build and communicate with the objects
   within the GUI.  The book, and this Demo, expect the user to create the
   fully functional aspect.  This Demo and the book's contents is just a guide
   in to how you would make a GUI.
  However, the source code to this demo is fully commented and explains what each 
   part does.  If you are ready to create a GUI, meaning you are somewhat advanced 
   in programming, you will be able to follow along in the source to better know 
   what each part does.

* Emulator Notes:
  This demo runs in VirtualBox, DOSBox, and Bochs just fine, though it may run in
   others as well.  However, VirtualBox does not handle the wheel mouse very well,
   and you must *not* use the /O parameter when starting the CTMOUSE driver.
  
Comments and/or error reports may be sent to:

                     fys [at] fysnet [dot] net

Please give details to the system you are using, the video mode selected, as
well as any other details you wish to give.
