The fdc_boot files, when assembled, simply makes a 1.44Meg floppy image ready for emulation in Bochs or QEMU.

It simply shows that it is indeed capable of loading sectors from the floppy without using the BIOS.
i.e.: Only using direct programming of the FDC and DMA to load additional sectors from the floppy disk.

This was simply an experiment to see if it was capable within the first 512 bytes that the BIOS already
loads for us.  It also must not assume very much and does have error handling.

It needs a little more work, but does actually show that it can be done.

Ben