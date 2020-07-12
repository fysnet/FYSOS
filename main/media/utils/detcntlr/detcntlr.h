/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2015
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Media Storage Devices, and is for that purpose only.  You have the
 *   right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 */

#ifndef FYSOS_DETCNTLR
#define FYSOS_DETCNTLR

bit32u pci_mem_range(bit8u bus, bit8u dev, bit8u func, bit8u port);
bool get_parameters(int argc, char *argv[]);


#endif // FYSOS_DETCNTLR
