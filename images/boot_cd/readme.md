The bootcd_eltorito.iso image is a cd-rom image using the El Torito boot emulation.  However, no emulator that I know of, as well as any actual BIOS that I know of support the Emulate and Boot function (AH = 0x4C).

Specs are at: https://en.wikipedia.org/wiki/El_Torito_(CD-ROM_standard)

The file will boot, show my menu of a few images, allowing to select one of them, but when ready to boot that image, the emulator or BIOS errors...Function not supported (0x0100).
