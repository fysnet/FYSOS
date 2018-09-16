The A.IMG and HD.IMG files in this folder are usually the latest bootable images, though are for a FAT12 Floppy and a FAT12 hard drive.  There is no GUI files installed.

This is the default bootable format for testing so that at the "prompt", I can type "wd<enter>" and it will write the console text to a file.  This is the file I would like if you will send it to me.

The LEANFS folder contains the same two files, though now are formatted with the LEAN FS and contain the files for the GUI.  The HD.IMG file contains all GUI system files while the A.IMG file is missing a few since it only has a whopping 1.44Meg of disk space.  When you get to the command prompt, type GUI<enter> and the GUI will begin to load.

However, even though the LeanFS files will still allow you to write the console text to the disk via "wd<enter>", there is no way to extract this file from the image/physical disk unless your host has support for the LEANFS.  Therefore, if you plan to send me this DEBUG.TXT file, please use the FAT12 images.

At the command prompt, type:

   wd&lt;enter&gt;
   
(The &lt;enter&gt; being the enter key)

This will write the console text to a file on the disk named: DEBUG.TXT.  I sure would like to see this text file:  fys [at] fysnet [dot] net.

Thanks,

Ben
