#### This is the source code to the [Font Editor](https://www.fysnet.net/fontedit/index.htm).

It is for Windows only, but can be compiled for 32-bit or 64-bit versions.
(Please let me know if you have compiled it for another platform)

#### A few notes:
This app now supports a new version of the Font File.  The old version is no longer supported.  Please see the changes for this new version at the link above.

This version of the Font file will break the [GUI Demo code](https://github.com/fysnet/FYSOS/tree/master/main/gui/source/gui_demo) included for this book.  If you use the GUI Demo code and this new Font file version, you will need to modify the GUI Demo code to use this new font version file.  I will see if I can get around to updating the source code to match.

If you load the resource.rc file into the Visual Studio IDE, the IDE will alter the file and break the build.  If you must edit the resource.rc file, make sure and use a text editor, not the Visual Studio IDE editor.

You can now load Linux PSF Font files, which get converted to this app's Font format.  You can save back to the PSF format if you wish.

If you had FNT files from the old format of my specs, use the conv.c/conv.h source files to convert them to the new format.

If anyone has used/is using this font specification in their work, I would like to know.  The FontEdit page linked to above lists some font files.  I will place yours there as well.

Thank you,
Ben
