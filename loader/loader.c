/***************************************************************************
*  Copyright (c) 1984-2016    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: loader.c                                                           *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
* DESCRIPTION:                                                             *
*  Loader code for the FYS OS version 2.0 operating system.                *
*                                                                          *
* BUILT WITH:   NewBasic Compiler and Assembler                            *
*                 http://www.fysnet/newbasic.htm                           *
*               NBC   ver 00.20.25                                         *
*          Command line: nbc loader<enter>                                 *
*               NBASM ver 00.26.59                                         *
*          Command line: nbasm loader loader.sys -d<enter>                 *
*                                                                          *
* Last Updated: 10 Aug 2016                                                *
*                                                                          *
****************************************************************************
* Notes:                                                                   *
*                                                                          *
* The loader is for the FAT, exFAT, FYSFS, LEAN, and EXT2 file systems.    *
*                                                                          *
* Loading of files:                                                        *
*  The kernel file can be placed anywhere on disk, and does not need to    *
*   be continuous on the disk, as long as it is in the ROOT directory.     *
*   (Same with any other pre-kernel files)                                 *
*                                                                          *
* This loader program has been loaded via the BOOT.BIN file (boot sector)  *
*  and resides at 3000:0000h (0x30000).  It can use any memory above       *
*  this position.                                                          *
*                                                                          *
* The bootsector passed the address of a data block in the DS:SI register  *
*  pair.  Each FS boot, if needed, set these values for use.  See the      *
*  S_BOOT_DATA structure for more information.                             *
*  We need to save the respective values for later.                        *
*  ROOT of current disk is at:  ROOTSEG                                    *
*   FAT of current disk is at:  FATSEG                                     *
*   (see boot.inc)                                                         *
*                                                                          *
* Boot has also left us a stack at ss=07C0h which is 16k in size.          *
*  We later set it to just less than a size of 4 meg                       *
*                                                                          *
* With this loader, we do the following in the following order:            *
*  Check for 486+ code making sure not to destroy es:di                    *
*  Load system files                                                       *
*  Sets our GDT and IDT                                                    *
*  Move to PMODE                                                           *
*  Jump to kernel                                                          *
*                                                                          *
****************************************************************************
*                                                                          *
* TODO:                                                                    *
*  - I am sure there are lots to do....                                    *
*                                                                          *
***************************************************************************/

// locations to set our stack to
#define STACK_BASE   0x01000000

// these equates are where our memory buffers are for the file loading and
//  decompressing.  They are set at locations 128meg and 112meg.  If we don't detect
//  that we have this much memory, we adjust these two to be just less than
//  available memory.
#define DECOMP_BUF_MIN    0x03000000  // minimum amount of memory we need
#define DECOMP_BUF_SIZE   0x01000000  // 16 meg memory buffer(s) size
#define DECOMP_BUF        0x08000000  // default to 128 meg
// this is the memory that is reserved for the decompressors allocation code
// it is just below the memory above and is DECOMP_BUF_SIZE in size
#define DECOMP_MEM  (DECOMP_BUF - DECOMP_BUF_SIZE)

//.optoff                        // we don't want anything changed unknowingly
#pragma optimizer(off)

// turn off diagnostic reporting
#pragma diag(off)

#outfile "loader.sys"            // create 'loader.sys' file

#include "./includes/ctype.h"    // ctype stuff
#include "./includes/boot.h"     // boot include file

#include "loader.h"              // main include file

//.rmode                         // bios starts with (un)real mode
#pragma ptype(rmode)

//.186                           // only allow 80x86 code at start
#pragma proc(186)
#pragma proc(short)              // we want 16-bit offsets

// include some #defines (equates) for the asm file
_asm (
  "PARAM0 equ [ebp+ 6]   ; first parameter in the list (left to right)\n"
  "PARAM1 equ [ebp+10]   ; second parameter \n"
  "PARAM2 equ [ebp+14]   ; ... \n"
  "PARAM3 equ [ebp+18]   ; .. \n"
  "PARAM4 equ [ebp+22]   ; . \n"
  "\n"
  "nPARAM0 equ [esp+ 2]  ; first parameter in the list of a _naked function \n"
  "nPARAM1 equ [esp+ 6]  ; second parameter \n"
  "nPARAM2 equ [esp+10]  ; ... \n"
  "nPARAM3 equ [esp+14]  ; .. \n"
  "nPARAM4 equ [esp+18]  ; . \n"
);

//           org  00h            // flat binary
#pragma org(0x00)

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// start of our loader code
//  ( do not put any code or data above this point!!! )
//  ( this must be at 0x0000 in the binary file )
_naked int main(void) {
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // the boot code passes the address of boot_data in
  //  the ds:si register pair.  Let's copy it to our
  //  static buffer
  // no other information was passed other than what
  //  is in this boot_data block.
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  setup the seg registers (real mode)
  _asm (
    "  mov  ax," #LOADSEG  "   ; set ds and es to LOADSEG }\n"
    "  mov  es,ax              ; \n"
    "  push ax                 ; save ax for ds later\n"
    "  mov  di,offset _boot_data ; ds:si = 07C0:xxxxh\n"
    "  mov  cx," #SIZEOF_S_BOOT_DATA "\n"
    "  cld                     ; \n"
    "  rep                     ; store BOOT BPB from passed BOOT\n"
    "    movsb                 ; \n"
    "  pop  ds                 ; ds now = LOADSEG from above\n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // change the position of the stack so that ss = ds = es
  _asm (
    "  cli                     ; don't allow an interrupt while we do this \n"
    "  mov  ax,ds              ; we want ss = ds so this C code works \n"
    "  mov  ss,ax              ; \n"
    "  mov  sp,0FFFEh          ; top of segment\n"
    "  sti                     ; \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // set to screen 03h
  _asm (
    "  mov  ax,0003h           ; make sure we are in screen mode 03h\n"
    "  int  10h                ; 80x25\n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // turn off the cursor.
  // FYSOS turns it back on after keyboard initialization.
  _asm (
    "  mov  ax,0103h           ; al = current screen mode (bug on some BIOSs)\n"
    "  mov  ch,00100000b       ; bits 6:5 = 01\n"
    "  int  10h \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Check for 386+ machine.  If not, give error and halt
  //   Returns processor type
  puts16("\nChecking for a 486+ processor with the RDTSC instruction...");
  main_i = chk_486();
  if (main_i < 6) {
    puts16("\n A 486+ compatible processor with the CPUID and RDTSC instructions is required."
           "\n Processor detected was a ");
    switch (main_i) {
      case 0:
        puts16("8086 or compatible.");
        break;
      case 1:
        puts16("80186 or compatible.");
        break;
      case 2:
        puts16("80286 or compatible.");
        break;
      case 3:
        puts16("80386 or compatible.");
        break;
      case 4:
        puts16("80486 or compatible without CPUID.");
        break;
      case 5:
        puts16("80486 or compatible with CPUID but without RDTSC.");
        break;
    }
    puts16("\nPress a key to reboot system.");
    
    _asm (
      "  xor  ah,ah              ; Wait for keypress\n"
      "  int  16h                ; \n"
      "  int  18h                ; boot specs say that 'int 18h'\n"
      "                          ;  should be issued here.\n"
    );
  }
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// We now can use 386+ code (still in real mode though)
//
//.486       // allow processor specific code for the 486
#pragma proc(486)
#pragma proc(long)               // we want 32-bit offsets
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // check to see if we are a 64-bit machine.  No reason other than just
  //  for curiosity's sake.
  if (is_64bit_cpu())
    puts16(" (Found 64-bit CPU...)");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now that we are 32-bit aware, clear the high order part of esp
  // we don't use it yet, but soon will (when we set up unreal mode)
  _asm ("  and  esp,0000FFFFh      ; now that we are 32-bit aware, clear the high order part of esp\n");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // initialize the crc32 stuff
  crc32_initialize();
  
  // mark that we use a Legacy BIOS
  bios_type = 'BIOS';
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now we need to make sure the a20 line is active
  //  and save the technique number used
  // (before the move to unreal mode)
  a20_tech = set_a20_line();
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Setup of Un-Real mode and point gs and fs to a flat address space.
//  we also set ds, es, and ss to a base of 0x30000, but with a 4-gig limit
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // load a temp GDT
  _asm (
    "  cli  \n"
    "  lgdtf [gdtoff_temp]\n"
    "  \n"    
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Switch to protected mode
    "  mov  eax,CR0\n"
    "  or   al,01h\n"
    "  mov  CR0,eax\n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Here is the jump.  Intel recommends we do a jump.
    "  db  0EAh\n"
    "  dw  offset unreal_mode     ; not in pmode yet, so word sized\n"
    "  dw  " #LOADCODE16 "\n"
    "  \n"
    "unreal_mode: \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // load selector with a base of 0x00000000
    "  mov  eax," #FLATDATA16 " ; Selector for 4Gb data seg\n"
    "  mov  fs,ax               ; \n"
    "  mov  gs,ax               ; \n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // make the stack segment use all of ESP with a base of LOADSEG
    //  and since we don't have anything at the current stack location
    //  (that we care of anyway), move the stack pointer up aways
    //  (this will give us a stack size of 4meg - (LOADSEG * 4))
    // we also set ds and es to use all of the 4 gig space, with a
    //  base of 0x30000
    "  mov  eax," #LOADDATA16 "  ; Selector for 4Gb data seg\n"
    "  mov  ds,ax                ; \n"
    "  mov  es,ax                ; \n"
    "  mov  eax," #LOADSTACK16 " ; Selector for 64k stack seg\n"
    "  mov  ss,ax                ; \n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Switch back to real mode
    "  mov  eax,CR0\n"
    "  and  eax,7FFFFFFEh\n"
    "  mov  CR0,eax\n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Here is the jump.  Intel recommends we do a jump.
    "  db  0EAh\n"
    "  dw  offset real_mode     ; in rmode, so word sized\n"
    "  dw  " #LOADSEG "\n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    "real_mode:\n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // reload fs and gs with a 16-bit segment of 0x0000
    // the limit is still set at 4gig.
    "  xor  ax,ax\n"
    "  mov  fs,ax\n"
    "  mov  gs,ax\n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // reload ds, es, and ss with a 16-bit segment of LOADSEG
    // the limit is still set at 4gig.
    "  mov  ax," #LOADSEG " \n"
    "  mov  ds,ax\n"
    "  mov  es,ax\n"
    "  mov  ss,ax\n"
    "  mov  esp,(00400000h - (" #LOADSEG " << 4)) ; start at 4meg and come down \n"
    "  \n"
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // allow interrupts again
    "  sti\n"
  );
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Now we can request the allotted memory from the BIOS
  get_memory(&memory);
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Now get the PCI information. (Detection only)
  get_pci_info(&bios_pci);
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Now load the system file(s)
//  Search for the files in root directory
  
  // check for large disk support
  large_disk = large_disk_support(boot_data.drive);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // print the loading string
  printf("\nLoading system files...");
  
  main_i = 0;
  while (system_files[main_i][0]) {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // print the status string
    printf("\nLoading: %s", system_files[main_i]);
    
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // here is were we check the fs type for FAT12/16/32
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ret_size = 0;
    switch (boot_data.file_system) {
      case FS_FAT12:
      case FS_FAT16:
      case FS_FAT32:
        ret_size = fs_fat(&boot_data, system_files[main_i], decomp_buf_loc);
        break;
        
      case FS_LEAN:
        ret_size = fs_lean(&boot_data, system_files[main_i], decomp_buf_loc);
        break;
        
//      case FS_EXT2:
//        ret_size = fs_ext2(&boot_data, system_files[main_i], decomp_buf_loc);
//        break;
        
      case FS_FYSFS:
        ret_size = fs_fysfs(&boot_data, system_files[main_i], decomp_buf_loc);
        break;
        
//      case FS_EXFAT:
//        ret_size = fs_exfat(&boot_data, system_files[main_i], decomp_buf_loc);
//        break;
        
      default:
        printf("\n1st stage boot sent unknown file system identifier: (%i)", boot_data.file_system);
        freeze();
    }
    
    if (ret_size) {
      ldr_hdr = decomp_buf_loc;
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      // now that it is at 'decomp_buf_loc', call the decompressor and let
      // it place the decompressed file to the actual location
      if ((ret_size = decompressor(decomp_buf_loc, ret_size)) > 0) {
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Now do a CRC check on the file.
        printf("...checking crc");
        if (calc_crc(ldr_hdr->location, ret_size) == ldr_hdr->file_crc) {
          puts("(file crc passed)     ");
          // if this is the kernel, we need to update our pointer
          if (ldr_hdr->flags & LDR_HDR_FLAGS_ISKERNEL) {
            // if the kernel_base is not zero, we already have found a
            //  file with this flag set.  So give error.
            if (kernel_base != 0) {
              printf("\nAlready set 'kernel_base' to 0x%08X...\n...halting...", kernel_base);
              freeze();
            } else
              kernel_base = ldr_hdr->location;
          }
        } else {
          puts("...Invalid file crc   ");
        }
      } else {
        // was an error decompressing the file.
        puts("...Error decompressing file...");
        if (ldr_hdr->flags & LDR_HDR_FLAGS_HALT) {
          puts("...halting...");
          freeze();
        }
      }
    }
    
    main_i++; // move to next file string
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Kernel file is loaded to 'kernel_base'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  update_int1e();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Reset the diskette (floopy) drive so it loads the new int 1Eh table values
  // Note: We can no longer simply use INT 13h, since our
  //  stack is a BIG stack.  We must call our interrupter.
  main_regs.eax = 0x00000000;
  main_regs.edx = 0x00000000;
  intx(0x13, &main_regs);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Get the bios equipment list 16-bit word value
  // Get the keyboard status bits at 0040:0017
  get_bios_equ_list(&bios_equip, &kbd_bits);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We run through all of the disk drives with a BIOS drive number of 80h
  //  or higher and store the (extended) disk parameters.
  get_drv_parameters(drive_params);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // one of the last things we need to do before we jump,
  // is get the time from the bios.
  get_bios_time(&time);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // see if we have and APM BIOS
  apm_bios_check(&apm);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Now get the vesa video info from the BIOS.
  vid_mode_cnt = get_video_info(mode_info);
  if (vid_mode_cnt > 0) {
    // see if a key was pressed (F8 maybe) and if so,
    //  let the user choose a mode, else choose one
    //  for them
    cur_vid_index = get_video_mode(mode_info, vid_mode_cnt, 1024, 768, 16);
    if (cur_vid_index == 0xFFFF)
      cur_vid_index = get_video_mode(mode_info, vid_mode_cnt, 1024, 768, 15);
    if (cur_vid_index == 0xFFFF)
      cur_vid_index = get_video_mode(mode_info, vid_mode_cnt, 800, 600, 16);
    if ((cur_vid_index == 0xFFFF) || (kbhit() && getscancode() == 0x4200)) {
      // print the available modes and ask for a selection of one of them.
      // if there are more modes than will fit on the screen, an 'enter' key
      //  will scroll to the next page.
      // the ESC key will exit
      main_i = 0;
next_entry:
      printf("\n");
      for (main_j=0; main_i<vid_mode_cnt && main_j<23; main_i++) {
        printf(" %c: %4i x %4i %2i bits\n", main_j + 'A', mode_info[main_i].xres, mode_info[main_i].yres, mode_info[main_i].bits_per_pixel);
        main_avail[main_j] = main_i;
        main_j++;
      }
      if (main_i < vid_mode_cnt)
        printf(" ** 'Enter' for more\n");
      do {
        printf("Please select a video mode to use [A-%c]: ", main_j + 'A' - 1);
        main_ch = getscancode();
        if (main_ch == 0x011B)  { // esc (start over)
          main_i = 0;
          goto next_entry;
        }
        if ((main_ch == 0x1C0D) && (main_i < vid_mode_cnt)) {
          printf("\n");
          goto next_entry;
        }
        main_ch = toupper(main_ch & 0xFF);
        printf("%c\n", main_ch);
        main_ch -= 'A';
      } while ((main_ch < 0) || (main_ch >= main_j));
      cur_vid_index = main_avail[main_ch];
    }
  } else {
    puts("FYSOS requires a VESA capable video card with a Linear Base Frame Buffer...\n");
    freeze();
  }
  
  //printf(" (%i) %ix%i %i %i 0x%08X\n", 
  //  cur_vid_index,
  //  mode_info[cur_vid_index].xres, mode_info[cur_vid_index].yres, 
  //  mode_info[cur_vid_index].bits_per_pixel, mode_info[cur_vid_index].memory_model,
  //  mode_info[cur_vid_index].lfb);
  //freeze();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // move the 1400h byte block of data to just before the kernel
  // kernel base should be on a meg boundary and since is a PE file,
  //  will have the first 0x400 bytes free for our use.  Therefore,
  //  we back up 0x1000 from the base and this is how we have 0x1400
  //  bytes of space for use.
  _asm (
    "  cld                     \n"
    "  mov  esi,offset _gdtoff \n"
    "  mov  edi,_kernel_base   \n"
    "  sub  edi,1000h          \n"
    "  mov  ecx,(1400h>>2)     \n"
    "@@: lodsd                 \n"
    "  mov  fs:[edi],eax       \n"
    "  add  edi,4              \n"
    "  dec  ecx                \n"
    "  jnz  short @b           \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // move the actual GDT entries (first 3)
  _asm (
    "  mov  esi,offset act_gdt \n"
    "  mov  edi,[_gdtoff+2]    \n"
    "  mov  ecx,((3 * 8) >> 2) \n"
    "@@: lodsd                 \n"
    "  mov  fs:[edi],eax       \n"
    "  add  edi,4              \n"
    "  dec  ecx                \n"
    "  jnz  short @b           \n"
  );
  
  // **
  // don't put any code here unless you save edi
  // **
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear out the remaining entries
  _asm (
    "  xor  eax,eax                    \n"
    "  mov  ecx,(((256 - 3) * 8) >> 2) \n"
    "@@:                               \n"
    "  mov  fs:[edi],eax               \n"
    "  add  edi,4                      \n"
    "  dec  ecx                        \n"
    "  jnz  short @b                   \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // make sure the floppy disk drive motor is turned off.
  //  put a 1 at 0x00440 so that the next time the BIOS
  //  decrements it, it will turn of the motor.
  floppy_off();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Change to that screen mode
  // Note: We can no longer simply use INT 10h, since our
  //  stack is a BIG stack.  We must call our interrupter.
  main_regs.eax = 0x00004F02;
  main_regs.ebx = mode_info[cur_vid_index].bios_mode_num | (1<<14);
  intx(0x10, &main_regs);
  
  // if it is 8-bit, we have to set the VGA palette
  //  we currently assume a VGA compatible
  if (mode_info[cur_vid_index].bits_per_pixel == 8)
    vid_set_256_palette(TRUE);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // turn off the hardware cursor.
  main_regs.eax = 0x00000103;
  main_regs.ecx = 0x00002000;
  intx(0x10, &main_regs);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // load the actual GDT and IDT
  _asm (
    "  cli             \n"
    "  lgdtf [_gdtoff]  \n"
    "  lidtf [idtoff]  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Switch to protected mode
  _asm (
    "  mov  eax,CR0   \n"
    "                 \n"
    "  and  eax,(~60000000h)  ; clear CD and NW bits  \n"
    "                 \n"
    "  or   al,21h     ; and set the NE bit \n"
    "  mov  CR0,eax   \n"
    "                 \n"
    "  ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  \n"
    "  ; Here is the jump.  Intel recommends we do a jump. \n"
    "  db  66h        \n"
    "  db  0EAh       \n"
    "  dd  (prot_mode + (" #LOADSEG " << 4)) \n"
    "  dw  " #FLATCODE "\n"
    "                 \n"
    "still_in_16bit:  \n"
  );

  // screen is in graphics mode...
  //printf("\nWe didn't make it into pmode...");
  freeze();

#pragma ptype(pmode)  //.pmode
  
  // let's make sure we made it here.
  // if still in 16bit mode,
  //  the below will look like this ---->
  _asm (
    "prot_mode:                     ; xor  ax,ax     \n"
    "  xor eax,eax                  ; xor  ax,0000h  \n"
    "  xor eax,0C08B0000h           ; mov  ax,ax     \n"
    "  jz  short still_in_16bit     ; jz   short still_in_16bit \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // we need to make sure that CR4.TSD = 0 (bit 2)
  // i.e.: allows the RDTSC instruction
  // we need to make sure that CR4.VME = 0 (bit 0)
  _asm (
    "  mov  eax,cr4  \n"
    "  and  al,(~5)  \n"
    "  mov  cr4,eax  \n"
    "  ; we need to add to _kernel_base now, before we\n"
    "  ; change the ds register/selector below.\n"
    "  add  dword _kernel_base,400h  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  set up the segment descriptors
  //  ds, es, fs, and ss have a base of 00000000h
  _asm (
    "  mov  ax," #FLATDATA " ; Selector for 4Gb data seg \n"
    "  mov  ds,ax         ;  ds  \n"
    "  mov  es,ax         ;  es  \n"
    "  mov  fs,ax         ;  fs  \n"
    "  mov  gs,ax         ;  gs  \n"
    "  mov  ss,ax         ;  ss  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  set up a stack at STACK_BASE (physical) of 4 meg size
  _asm ("  mov  esp,((" #STACK_BASE " + 00400000h) - 4) \n");
            
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We now have PMODE setup and all our segment selectors correct.
  // CS              = 0x00000000
  // SS & remaining  = 0x00000000
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Here is the jump.
  // We will jump to physical address 'kernel_base' + 400h
  _asm (
    "             db  0EAh           \n"
    "_kernel_base dd  ?  ; *in* pmode, so *dword* sized \n"
    "             dw  " #FLATCODE "  \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Kernel file should have taken over from here
  //   but to be sure
  // screen is in graphics mode...
  //printf("\n We didn't make it dude...");
  freeze();
} // end of _main()

// remain global
int main_i, main_j, main_avail[24], ret_size;
bit16u main_ch;
struct REGS main_regs;
struct S_LDR_HDR farF *ldr_hdr;

//.rmode
#pragma ptype(rmode)

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Checks for a 486+ machine with the CPUID and RDTSC instructions.
//  on entry: nothing
//  on exit:
//    returns
//         0 = 8086, 1 = 186, 2 = 286, 3 = 386,
//         4 = 486+ without CPUID, 
//         5 = 486+ with CPUID but not RDTSC,
//         6 = 486+ with CPUID and RDTSC
#pragma proc(short)  // we want 16-bit offsets and registers (we might still be a 8086)
int chk_486(void) {
  _asm (
    "  pushf                   ; save the original flags value\n"
    "  \n"
    "  mov  ax,00h             ; Assume an 8086\n"
    "  mov  cx,0121h           ; If CH can be shifted by 21h,\n"
    "  shl  ch,cl              ; then it's an 8086, because\n"
    "  jz   short @f           ; a 186+ limits shift counts.\n"
    "  push sp                 ; If SP is pushed as its\n"
    "  pop  ax                 ; original value, then\n"
    "  cmp  ax,sp              ; it's a 286+.\n"
    "  mov  ax,01h             ; is 186\n"
    "  jne  short @f           ;\n"
    "  mov  ax,7000h           ; if bits 12,13,14 are still set\n"
    "  push ax                 ; after pushing/poping to/from\n"
    "  popf                    ; the flags register then we have\n"
    "  pushf                   ; a 386+\n"
    "  pop  ax                 ;\n"
    "  and  ax,7000h           ;\n"
    "  cmp  ax,7000h           ;\n"
    "  mov  ax,02h             ; is 286\n"
    "  jne  short @f           ; it's a 386\n"
    "  \n"
    "  ; =-=-=-=-=- test for .486\n"
    "  ; if we can toggle bit 18 in EFLAGS (AC bit) we have a\n"
    "  ;  486+.  The 386 doesn't have the AC bit.\n"
    "  cli\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  mov  ebx,eax\n"
    "  xor  eax,00040000h      ; bit 18\n"
    "  push eax\n"
    "  popfd\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  push ebx\n"
    "  popfd\n"
    "  sti\n"       
    "  xor  eax,ebx\n"
    "  mov  ax,03              ; is 386\n"
    "  jz   short @f           ; else it's a 486+\n"
    "  \n"
    "  ; =-=-=-=-=- test for CPUID\n"
    "  ; if we can toggle bit 21 in EFLAGS (ID bit) we have a\n"
    "  ;  486+ with the CPUID instruction\n"
    "  cli\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  mov  ebx,eax\n"
    "  xor  eax,00200000h      ; bit 21\n"
    "  push eax\n"
    "  popfd\n"
    "  pushfd\n"
    "  pop  eax\n"
    "  push ebx\n"
    "  popfd\n"
    "  sti\n"       
    "  xor  eax,ebx\n"
    "  mov  ax,04              ; is 486+ without CPUID\n"
    "  jz   short @f           ; else it's a 486+ with CPUID\n"
    "  \n"
    "  ; =-=-=-=-=- test for RDTSC\n"
    "  ; do a CPUID with function 1.  If bit 4 of EDX on return,\n"
    "  ;  we have the RDTSC instruciton\n"
    "  mov  eax,1\n"
    "  cpuid\n"
    "  test edx,10h\n"
    "  mov  ax,05              ; is 486+ with CPUID but without RDTSC\n"
    "  jz   short @f\n"
    "  \n"
    "  ; =-=-=-=-=- We got a 486+ with the CPUID and RDTSC instructions\n"
    "  mov  ax,06              ; is 486+ with CPUID and RDTSC\n"
    "  \n"
    "@@: popf                    ; restore the original flags value\n"
  );
}
#pragma proc(long)              // we can go back to long now (32-bit offsets and registers)

// reference: 
//  AMD manual (24593-Rev. 3.25-June 2015)
//    http://support.amd.com/TechDocs/24593.pdf
//  Intel manual Vol 3C, page 34-6, section 34.4.1.1 (325384-055US, June 2015)
//    "Support for Intel 64 architecture is reported by CPUID.80000001:EDX[29] = 1"
//  Intel manual Vol 2A, page 3-186, Table 3-17 (253666-055US, June 2015)
// Use CPUID to determine if the processor supports long mode
_naked bool is_64bit_cpu() {
  _asm (
    "  mov  eax,80000000h       ; Get highest extended function\n"
    "  cpuid                    ; \n"
    "  cmp  eax,80000001h       ; is at least 80000001h allowed?\n"
    "  jb   short @f            ; If not, long mode not support\n"
    "  mov  eax,80000001h       ; Get extended function 8000001h flags\n"
    "  cpuid                    ; EDX = extended-features flags\n"
    "  bt   edx,29              ; If bit 29 is set, long mode is supported\n"
    "  jnc  short @f            ; \n"
    "  mov  eax,1               ; return TRUE\n"
    "  ret                      ; we're done\n"
    "@@:                        ; \n"
    "  xor  eax,eax             ; return FALSE\n"
    "  ret                      ; we're done\n"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Call an interrupt
// Return TRUE = carry set
// Since we use a "large stack", we must setup a stack that only uses the first
//  16-bits of esp so that the BIOS will work correctly.
// (The AX = 4800h service uses atleast ~512 bytes...)
// TODO: use a cs: override on saved_esp incase we need to pass ds to a service.
#define SMALL_STACK_SIZE 1024  // INT 1Ah/B101h may require up to 1024 bytes
bit32u saved_esp;
_asm ( ".align 4" );
bit8u  small_stack[SMALL_STACK_SIZE];
bool intx(int i, struct REGS *regs) {
  _asm (
    "  push ebp                ; save stack frame \n"
    "  mov  eax,PARAM0         ; self modify code to interrupt number\n"
    "  mov  [int_num + 1],al   ; \n"
    "  mov  ebp,PARAM1         ; \n"
    "  push ebp                ; \n"
    "  mov  eax,[ebp+ 0]       ; \n"
    "  mov  ebx,[ebp+ 4]       ; \n"
    "  mov  ecx,[ebp+ 8]       ; \n"
    "  mov  edx,[ebp+12]       ; \n"
    "  mov  esi,[ebp+16]       ; \n"
    "  mov  edi,[ebp+20]       ; \n"
    "  mov  _saved_esp,esp     ; save the esp value \n"
    "  mov  esp,(_small_stack + " #SMALL_STACK_SIZE ") \n"
    "int_num: int  0           ; zero will be replaced with value above \n"
    "  mov  esp,_saved_esp     ; restore the esp value \n"
    "  pop  ebp                ; restore pointer to regs\n"
    "  mov  [ebp+ 0],eax       ; \n"
    "  mov  [ebp+ 4],ebx       ; \n"
    "  mov  [ebp+ 8],ecx       ; \n"
    "  mov  [ebp+12],edx       ; \n"
    "  mov  [ebp+16],esi       ; \n"
    "  mov  [ebp+20],edi       ; \n"
    "  pushfd                  ; retrieve eflags \n"
    "  pop  eax                ;  into eax \n"
    "  mov  [ebp+28],eax       ; then into regs->eflags \n"
    "  pop  ebp                ; restore stack frame \n"
  );
  
  return (bool) (regs->eflags & 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// 
// get the memory size from the bios (int 15h/e820/e801/88 or cmos)
bool get_memory(struct S_MEMORY *memory) {
  bit32u temp[2];
  int  i;
  bool r, present;
  struct REGS regs;
  struct S_BIOS_MEME820 *bios_memE820 = (struct S_BIOS_MEME820 *) local_buffer;
  
  // clear it out
  memset(memory, 0, sizeof(struct S_MEMORY));
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now try the most recent/best service
  present = FALSE;
  regs.ebx = 0;
  do {
    regs.eax = 0x0000E820;
    regs.edx = 0x534D4150;  // 'SMAP'
    regs.ecx = sizeof(struct S_BIOS_MEME820);
    regs.edi = local_buffer;  // es:buffer
    if (intx(0x15, &regs) || (regs.eax != 0x534D4150))
      break;
    
    if ((regs.ecx < 20) || (regs.ecx > sizeof(struct S_BIOS_MEME820)))
      // bug in bios - all returned descriptors must be
      // at least 20 bytes long, and cannot be larger then
      // the input buffer size sent in ecx.
      break;

//      printf("\n AAA %-12u %-12u %-12u %-12u %i %i BBB",
//        bios_memE820->base[0], bios_memE820->base[1],
//        bios_memE820->size[0], bios_memE820->size[1],
//        bios_memE820->type,
//        dword);
//      _asm ( " xchg bx,bx \n");
    
    present = TRUE;
    
    memory->block[memory->blocks].base[0] = bios_memE820->base[0];
    memory->block[memory->blocks].base[1] = bios_memE820->base[1];
    memory->block[memory->blocks].size[0] = bios_memE820->size[0];
    memory->block[memory->blocks].size[1] = bios_memE820->size[1];
    memory->block[memory->blocks].type = bios_memE820->type;
    memory->block[memory->blocks].attrib[0] = 0;
    memory->block[memory->blocks].attrib[1] = 0;
    
    // add to the accumulator
    add64(memory->size, memory->block[memory->blocks].size);
    
    memory->word = 1; // (our) type goes here
    memory->blocks++;
  } while (regs.ebx != 0);
  
  if (present && (memory->blocks > 0))
    goto check_mem_limits;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // if service E820 didn't work, try service E881 and E801
  // first, clear it out (again).
  memset(memory, 0, sizeof(struct S_MEMORY));
  
  // service E881 is identical to E801 except that E881 returns
  //  the values as 32-bits while E801 returns them as 16-bits
  regs.eax = 0x00000E881;
  if (r = intx(0x15, &regs)) {
    regs.eax = 0x00000E801;
    if (!(r = intx(0x15, &regs))) {
      // clear out high word for the code below
      regs.eax &= 0xFFFF;
      regs.ebx &= 0xFFFF;
      regs.ecx &= 0xFFFF;
      regs.edx &= 0xFFFF;
    }
  }
  
  if (!r) {
    regs.eax <<= 10;  // in K's
    regs.ebx <<= 16;  // in 64K's
    if (!regs.eax & !regs.ebx) {
      regs.eax = regs.ecx << 10; // in K's
      regs.ebx = regs.edx << 16; // in 64K's
    }
    
    temp[0] = regs.eax + (1 << 20);  // add a meg for the first meg of the address space
    temp[1] = 0;
    add64(memory->size, temp);
    temp[0] = regs.ebx;
    add64(memory->size, temp);
    
    memory->word = 2; // type goes here
    
    goto check_mem_limits;
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now try service 88h
  //  service 88h only returns up to 15 meg.
  //  we need to try service C7h for the rest.
  memset(memory, 0, sizeof(struct S_MEMORY));
  
  regs.eax = 0x000008800;
  if (!intx(0x15, &regs)) {
    temp[0] = (regs.eax & 0x0000FFFF) << 10;
    temp[1] = 0;
    add64(memory->size, temp);
    
    // now see if service C7h is available
    // on return:
    //  carry clear and ah = 0
    //   es:bx->rom table
    _asm ( "  push es  \n" );
    regs.eax = 0x00000C000;
    if (!intx(0x15, &regs)) {
      if ((regs.eax & 0x0000FF00) == 0) {
        // service C7h is available if bit 4 in feature_byte 2 (offset 06h) is set.
        if (* (bit8u farE *) (regs.ebx & 0x0000FFFF) & 0x10) {
          regs.eax = 0x0000C700;
          regs.esi = local_buffer;
          if (!intx(0x15, &regs)) {
            // dword at 0Eh = system memory between 16M and 4G, in 1K blocks
            // max value return = 0x003FC000 so shifting left by 10 won't overflow
            temp[0] = (* (bit32u *) &local_buffer[0x0E]) << 10;
            temp[1] = 0;
            add64(memory->size, temp);
            memory->word = 3; // type goes here
            _asm ( "  pop  es  \n" );
            goto check_mem_limits;
          }
        }
      }
    }
    _asm ( "  pop  es  \n" );
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now try the cmos value
  //  reg 17h = low byte
  //  reg 18h = high byte  (in kb's)
  outpb(0x70, 0x18);
  memory->size[0] = (inpb(0x71) << (8 + 10));
  outpb(0x70, 0x17);
  memory->size[0] |= (inpb(0x71) << (0 + 10));
  memory->size[1] = 0;
  memory->word = 4; // type goes here
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now we need to set up our limits.
  // if anything in the hi dword of memory.size, we have plenty :)
check_mem_limits:
  if (memory->size[1] == 0) {  // if high dword > 0, we have enough memory, else
    if (memory->size[0] < (DECOMP_BUF + DECOMP_BUF_SIZE)) {
      // not enough memory for preset limits, so adjust to compensate.
      // first, we need to make sure we have at least the min allowed.
      if (memory->size[0] < DECOMP_BUF_MIN) {
        printf("\n  *** Not enough memory ***"
               "\n        Size in megabytes found: %i", memory->size[0] >> 20);
        printf("\n Size of extended memory needed: %i", DECOMP_BUF_MIN >> 20);
        freeze();
      }
      
      // don't use the top meg incase there is bios data there
      decomp_buf_loc = memory->size[0] - (0x00100000 + DECOMP_BUF_SIZE);
      decomp_mem_loc = decomp_buf_loc - DECOMP_BUF_SIZE;
    }
  }
  
  return TRUE;
}

bool kbhit(void) {
  struct REGS regs;
  
  regs.eax = 0x00000100;
  intx(0x16, &regs);
  return (regs.eflags & (1<<6)) == 0;
}

bit16u getscancode(void) {
  struct REGS regs;
  
  regs.eax = 0x00000000;
  intx(0x16, &regs);
  return (bit16u) (regs.eax & 0x0000FFFF);
}

/*  vsync()
 *     no parameters
 * 
 *   wait for the vertical sync to start
 */
void vsync(void) {
  // wait until any previous retrace has ended
  while (inpb(0x3DA) & (1<<3));
  
  // wait until a new retrace has just begun
  while (!(inpb(0x3DA) & (1<<3)));
}

/*  vid_set_256_palette()
 *         port_io = use port I/O
 *
 *  this sets the 256 bpp mode palette
 *  if the card isn't VGA compatible, we have
 *   to use the VESA BIOS to set the palette
 *
 *  The RGB components can be 0 - 63 each, with 256 entries
 *   within the table to choose from.
 */
void vid_set_256_palette(const bool port_io) {
  int i;
  //bit8u palette256_info[256 * 4], *p;
  //struct REGS regs;
  
  // some controllers are not VGA compatible
  //if (port_io) {
    // wait for the retrace to start (older video cards require this)
    vsync();
    // mode is VGA compatible, so use the VGA regs
    for (i=0; i<255; i++) {
      outpb(0x3C8, (bit8u) i);
      outpb(0x3C9, (bit8u) (i & 0xE0) >> 2);
      outpb(0x3C9, (bit8u) (i & 0x1C) << 1);
      outpb(0x3C9, (bit8u) (i & 0x03) << 4);
    }
    // white
    outpb(0x3C8, (bit8u) 255);
    outpb(0x3C9, 0xFF);
    outpb(0x3C9, 0xFF);
    outpb(0x3C9, 0xFF);
  //} else {
    /*
    // must use VBE to set the DAC (palette)
    
    // ax = 0x4F09
    // bl = 0x80  set palette info while vertical retrace
    // cx = number of registers (256)
    // dx = start (0)
    // es:di -> address to buffer
    //  buffer is:
    //    {
    //     db  blue
    //     db  green
    //     db  red
    //     db  resv
    //    } [256];
    
    p = palette256_info;
    for (i=0; i<255; i++) {
      *p++ = (bit8u) (i & 0xE0) >> 2;
      *p++ = (bit8u) (i & 0x1C) << 1;
      *p++ = (bit8u) (i & 0x03) << 4;
      *p++ = 0;
    }
    // white
    *p++ = 0xFF;
    *p++ = 0xFF;
    *p++ = 0xFF;
    *p++ = 0;
    
    // call the service
    regs.edi = palette256_info;
    regs.eax = 0x00004F09;
    regs.ebx = 0x00000080;
    regs.ecx = 256;
    regs.edx = 0;
    intx(0x10, &regs);
    */
  //}
}

bit16u get_video_info(struct S_MODE_INFO *modeinfo) {
  int i, j;
  bit16u *p;
  struct REGS regs;
  bit16u vesa_modes[VESA_MODE_SIZE];
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear the buffer first
  memset(local_buffer, 0, 256);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // call the BIOS, first setting the first 4 bytes to 'VBE2'
  // Since it returns 512 bytes, we need to use a spare buffer.
  memcpy(local_buffer, "VBE2", 4);
  regs.edi = local_buffer;
  regs.eax = 0x00004F00;
  intx(0x10, &regs);
  if ((regs.eax & 0x0000FFFF) != 0x004F)
    return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now get the supported modes.  We have to do this incase
  //  the BIOS builds the list on the fly (Bochs BIOS does this)
  p = (bit16u *) (local_buffer + 0x0E);
  memcpy_ds(vesa_modes, (void *) p[0], p[1], ((VESA_MODE_SIZE - 1) * 2));
  vesa_modes[(VESA_MODE_SIZE - 1)] = 0xFFFF;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now scroll through the list of modes to see if we find any
  //  capable for our use.
  i = 0; j = 0;
  struct S_VIDEO_MODE_INFO *info = (struct S_VIDEO_MODE_INFO *) local_buffer;
  while (vesa_modes[i] < 0xFFFF) {
    memset(info, 0, sizeof(struct S_VIDEO_MODE_INFO));
    regs.eax = 0x00004F01;
    regs.ecx = vesa_modes[i];
    regs.edi = info;
    intx(0x10, &regs);
    if ((regs.eax & 0x0000FFFF) == 0x004F) {
      if ((info->mode_attrb & 0x01) &&  // supported by the current hardware (video card and monitor)
          (info->bits_pixel >= 8) &&
         ((info->mode_attrb & (1<<7)) && (info->linear_base > 0)) &&
        (
          (info->memory_model == 4) || // model = 4 = packed pixel
          (info->memory_model == 6)    // model = 6 = direct color
        )
       ) {
        modeinfo[j].lfb = info->linear_base;
        modeinfo[j].xres = info->x_res;
        modeinfo[j].yres = info->y_res;
        modeinfo[j].bytes_per_scanline = info->bytes_scanline;
        modeinfo[j].bits_per_pixel = info->bits_pixel;
        modeinfo[j].bios_mode_num = vesa_modes[i];
        modeinfo[j].memory_model = info->memory_model;
        if (++j == VIDEO_MAX_MODES)
          break;
      }
    }
    i++;
  }
  return j;
}

// see if there is a mode in the list of modes that matches the resolution indicated
//  and return that index if so.
bit16u get_video_mode(struct S_MODE_INFO *modeinfo, bit16u cnt, int x, int y, int bits) {
  for (bit16u i=0; i<cnt; i++)
    if ((modeinfo[i].xres == x) && (modeinfo[i].yres == y) && (modeinfo[i].bits_per_pixel == bits))
      return i;
  return 0xFFFF;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// check to see if the BIOS shows that we have a PCI Bus
void get_pci_info(struct S_BIOS_PCI *info) {
  struct REGS regs;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear the buffer first
  memset(info, 0, sizeof(struct S_BIOS_PCI));
  
  // call the service
  regs.eax = 0x0000B101;
  regs.edi = 0x00000000;
  if (!intx(0x1A, &regs) && !(regs.eax & 0x0000FF00)) {
    info->sig = regs.edx;
    info->flags = (bit8u) ((regs.eax >> 0) & 0xFF);
    info->major = (bit8u) ((regs.ebx >> 8) & 0xFF);
    info->minor = (bit8u) ((regs.ebx >> 0) & 0xFF);
    info->last  = (bit8u) ((regs.ecx >> 0) & 0xFF);
  }  
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// check to see if the BIOS supports large disks (hard drives)
bool large_disk = FALSE;

bool large_disk_support(const int drv) {
  struct REGS regs;
  
  // no need to call bios if not a hard drive
  if (drv >= 0x80) {
    regs.eax = 0x00004100;
    regs.ebx = 0x000055AA;
    regs.edx = drv;
    if (!intx(0x13, &regs) &&
         ((regs.ebx & 0x0000FFFF) == 0xAA55) &&
         (regs.ecx & (1<<0)))
       return TRUE;
  }
  
  return FALSE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get the number of fixed disk drives from the BIOS
//
/*
_naked int get_fixed_drvs(void) {
  _asm (
    "  push es      \n"
    "  xor  eax,eax \n"
    "  mov  es,ax   \n"
    "  mov  al,es:[0475h] \n"  // eax = 0x000000XX from above
    "  pop  es      \n"
    "  ret          \n"
  );
}
*/

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This routine scans through all drives 80h or higher, using the byte at
//  0040:0075 as a count of installed hard drives.  If a drive is found at
//  that drive number, it stores the drives parameters.  It checks to see
//  if the extended services are available for that drive.  If so, it gets
//  the extended parameters.  If not, it gets the standard parameters.
// Some BIOS' incorrectly return type in AX instead of AH, so we test for
//  this too.
void get_drv_parameters(struct S_DRV_PARAMS *drive_params) {
  int cur_drive = 0x80; //, count;
  struct S_DRV_PARAMS *p = drive_params;
  struct REGS regs;
  struct S_BIOS_DRV_PARAMS *bios_params = (struct S_BIOS_DRV_PARAMS *) local_buffer;
  
  // clear out the struct first
  memset(drive_params, 0, sizeof(struct S_DRV_PARAMS) * 10);
  //count = get_fixed_drvs();
  
  //for (int i=0; i<count && i<10; i++) {
  for (int i=0; i<10; i++) {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // check to see if there is a disk present at drive number
    regs.eax = 0x000015FF;
    regs.edx = cur_drive;
    if (!intx(0x13, &regs) && (((regs.eax & 0xFFFF) == 0x0003) || ((regs.eax & 0xFF00) == 0x0300))) {
      // start to store some information
      // store the drive number
      p->drv_num = (bit8u) cur_drive;
      
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      // check to see if the BIOS supports large disks for this drive
      if (large_disk_support(cur_drive)) {
        // flag that we did use extended parameters
        p->extended_info = TRUE;
        
        // get the extended parameters.
        regs.eax = 0x00004800;
        regs.edx = cur_drive;
        regs.esi = (bit32u) bios_params;
        bios_params->ret_size = 0x0042;
        bios_params->info_flags = 0; // some DELL BIOS' error if this isn't zero
        if (!intx(0x13, &regs)) {
          // now copy from the local buffer to our passed buffer
          memcpy(&p->bios_params, bios_params, sizeof(struct S_BIOS_DRV_PARAMS));
          // since the parameters at EDD_config_ptr may be at a temp buffer,
          //  we need to copy them now
          if ((bios_params->ret_size >= 0x1E) && (bios_params->EDD_config_ptr != 0xFFFFFFFF))
            memmove(p->dpte, (bios_params->EDD_config_ptr >> 16), (void *) (bios_params->EDD_config_ptr & 0xFFFF), 16);
        }
      } else {
        // get the standard parameters.
        regs.eax = 0x00000800;
        regs.edx = cur_drive;
        if (!intx(0x13, &regs)) {
          // CH = low eight bits of maximum cylinder number
          // CL = maximum sector number (bits 5-0)
          //     high two bits of maximum cylinder number (bits 7-6)
          // DH = maximum head number
          // DL = number of drives
          p->bios_params.cylinders = (((regs.ecx & 0x000000C0) << 2) | ((regs.ecx & 0x0000FF00) >> 8));
          p->bios_params.spt = (regs.ecx & 0x0000003F) + 1;
          p->bios_params.heads = ((regs.edx & 0x0000FF00) >> 8);
        }
      }
      p++;  // okay to move to next one
    }
    
    // reset the bus after an ah=15h call
    regs.eax = 0x00000100;
    regs.edx = cur_drive;
    intx(0x13, &regs);
    
    cur_drive++;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This routine converts LBA to CHS
// Sector   = (LBA mod SPT) + 1
// Head     = (LBA  /  SPT) mod Heads
// Cylinder = (LBA  /  SPT)  /  Heads
//    (SPT = Sectors per Track)
void lba_to_chs(const bit32u lba, unsigned int *cyl, unsigned int *head, unsigned int *sect) {
  *sect = (lba % boot_data.SecPerTrack) + 1;
  *head = (lba / boot_data.SecPerTrack) % boot_data.Heads;
  *cyl  = (lba / boot_data.SecPerTrack) / boot_data.Heads;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This routine reads in CX sectors using the bios interrupt 13h.
//   Either the "short" form, using CHS or the "long" form, using
//   the BIOS Extentions service
// On entry:
//  edx:eax = starting sector in LBA format
//      ecx = count of sectors to read
// (As of now, the compiler won't allow 64-bit numbers, so we assume
//  all LBA's are than 32-bit values)
// since the BIOS needs a buffer that <= 0x0000FFFF in address (offset)
//  we must make one, then copy it after the read.
// TODO: Pass a 64-bit LBA
struct S_READ_PACKET long_packet;
int read_sectors(bit32u lba, const int cnt, const void *buffer) {
  struct REGS regs;
  int ret = 0;
  int count = cnt;
  bit8u *p = (bit8u *) buffer;
  
  _asm (
    "  push es              \n"
    "  mov  ax," #LOADSEG  "\n"
    "  mov  es,ax           \n"
  );
  
  if (large_disk) {
    while (count) {
      long_packet.size = sizeof(struct S_READ_PACKET);
      long_packet.resv = 0;
      long_packet.cnt = 1;
      long_packet.buffer = (LOADSEG << 16) | (bit32u) local_buffer;
      long_packet.lba[0] = lba;
      long_packet.lba[1] = 0;
      add64(long_packet.lba, boot_data.base_lba);
      regs.eax = 0x00004200;
      regs.edx = boot_data.drive;
      regs.esi = &long_packet;
      if (intx(0x13, &regs))
        break;
      
      memcpy(p, local_buffer, 512);
      p += 512;
      
      ret++;
      count--;
      lba++;
    }
  } else {
    /* It is unknown if the BIOS can span heads and cylinders, so we just read all (remaining) sectors
     * from the current track.  For example, if we start on sector 5, and there are 18 spt, we read
     * sectors 5 through 18 as long as count requires that many.  This is faster than reading a single
     *  sector at a time.
     */
    unsigned int cyl, head, sect, scnt;
    lba += boot_data.base_lba[0];  // No need to 64-bit this.  Can't get a legal CHS value from a 64-bit LBA
    while (count) {
      lba_to_chs(lba, &cyl, &head, &sect);
      scnt = (boot_data.SecPerTrack - sect) + 1;
      if (scnt > LOCAL_BUFF_SECT_SIZE)
        scnt = LOCAL_BUFF_SECT_SIZE;
      if (scnt > count)
        scnt = count;
      regs.eax = 0x00000200 | scnt;
      regs.ebx = local_buffer;
      regs.ecx = ((cyl & 0xFF) << 8) | ((cyl >> 8) & 3) | (sect & 0x3F);
      regs.edx = ((head & 0x0F) << 8) | boot_data.drive;
      if (intx(0x13, &regs))
        break;
      
      memcpy(p, local_buffer, (scnt * 512));
      p += (scnt * 512);
      
      count -= scnt;
      ret += scnt;
      lba += scnt;
    }
  }
  
  _asm ("  pop  es \n");
  
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// the a20 code:
//
//   http://git.etherboot.org/?p=wraplinux.git;a=blob_plain;f=reloc/a20.S
//
// 1. out 64,D1 |wait for bit 1 = 0 (in 64) or TimeOut,
//    out 60,DF ;out 60,DD will turn A20 off.
//
// 2. set bit 1 of port 92h.
//
// 3. set a bit in PCI-config (host<->SB-bridge, Vendor specific).
//

// Note: VirtualBox doesn't like the int bit off for this long.
//  therefore, I have commented out the cli and sti instructions.

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// tests and activates the a20 line.
//  returns in al, the number of the technique used.
//  if can not set the a20 line, this functions prints an error and freezes.
int set_a20_line(void) {
  bit8u byte;
  struct REGS regs;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // make sure no interrupts bother us
  //_asm ("  cli  \n");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // first check to see if it is already active
  if (test_a20()) {
    //_asm ("  sti  \n");
    return 0;
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // it was not already set, so try
  // Method 1: keyboard controller
  if (!wait_kbd_status(1)) {    // waits while keyboard status bit 1 = 1
    outpb(0x64, 0xD0);
    if (!wait_kbd_status(0)) {  // waits while keyboard status bit 0 = 0
      byte = inpb(0x60);
      if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
        outpb(0x64, 0xD1);
        if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
          outpb(0x60, byte | 2);
          // Wait for the A20 line to settle down (up to 20usecs)
          outpb(0x64, 0xFF);  // Send FFh (Pulse Output Port NULL)
          if (!wait_kbd_status(1)) {  // waits while keyboard status bit 1 = 1
            // now test the a20 line
            if (test_a20()) {
//              _asm ("  sti  \n");
              return 1;        // keyboard method
            }
          }
        }
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 2: fast a20 (port 0x92)
  byte = inpb(0x92);
  //byte &= 0xFE;  // make sure we don't do a reset
  outpb(0x92, byte | 2);
  
  // now test the a20 line
  if (test_a20()) {
//    _asm ("  sti  \n");
    return 2;        // fast port 92h
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 3: Keyboard controller: Command DFh
  if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
    outpb(0x60, 0xDF);       // Send command DFh
    // read in the acknowledge
    if (!wait_kbd_status(0)) {   // waits while keyboard status bit 0 = 0
      if (inpb(0x60) == 0xFA) {  // if not ACK, then error
        // Wait for the A20 line to settle down (up to 20usecs)
        // Some UHCI controllers when using legacy mode, need the FF (null command)
        //  sent after the above DF command.
        outpb(0x64, 0xFF);    // Send FFh (Pulse Output Port NULL)
        if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
          // now test the a20 line
          if (test_a20()) {
//            _asm ("  sti  \n");
            return 3;        // keyboard method: command DFh
          }
        }
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 4: Brute force of Method 1
  outpb(0x64, 0xD1);
  if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
    outpb(0x60, 0xDF);
    if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
      outpb(0x64, 0xFF);
      if (!wait_kbd_status(1)) { // waits while keyboard status bit 1 = 1
        // now test the a20 line
        if (test_a20()) {
//          _asm ("  sti  \n");
          return 4;        // keyboard method: brute force of #1
        }
      }
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 5: BIOS a20 enable service (later PS/2's)
  regs.eax = 0x00002401;
  if (!intx(0x15, &regs) && !(regs.eax & 0x0000FF00)) {
    // now test the a20 line
    if (test_a20()) {
//      _asm ("  sti  \n");
      return 5;        // BIOS a20 service 2401h
    }
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Method 6: 


/*
! For the HP Vectra
        call    empty_8042
        jnz     err
        mov     al,#0xdf
        out     #0x64,al
        call    empty_8042
        jnz     err
        mov     al,#0xdf        ! Do it again
        out     #0x64,al
        call    empty_8042
        jnz     err
! Success

also look at
  c:\temp\fysos\new\himem\himem.asm for multiple ways to enable
  the a20 line for multiple machines.
    starts at 'AT_A20Handler'
*/
 
 // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 // if we make it here, then we didn't set the a20 line
 // say so and freeze.
 printf("\nUnable to set the a20 line.  Halting..."
        "\nPlease report this to me at fys@fysnet.net"
        "\nInclude as much information about your computer"
        "\n along with brand and version of BIOS."
        "\n"
        "\nThank you.");
 freeze();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// tests whether the a20 line is set.
//  we do this by writing a value to 0x00100900, and then check the value
//  at 0x00000900 to see if they are the same.  i.e.: 0x00100900 will wrap
//  around to 0x00000900 if the a20 line is not active.
// we use 0x00100900 since the point of wrap around, 0x00000900, does not
//  contain anything valid already and can be overwritten.
//  return TRUE if active
_naked bool test_a20() {

  _asm (
    "  push ds                                    \n"
    "  push es                                    \n"
    "                                             \n"
    "  mov  ax,0FF00h  ; FF00:1900h = 0x00100900  \n"
    "  mov  ds,ax                                 \n"
    "  mov  si,1900h                              \n"
    "                                             \n"
    "  xor  ax,ax      ; 0000:0900h = 0x00000900  \n"
    "  mov  es,ax                                 \n"
    "  mov  di,0900h                              \n"
    "                                             \n"
    "  ; read and store the original dword at ds:[si] \n"
    "  mov  eax,[si]                              \n"
    "  push eax        ; save the current value   \n"
    "                                             \n"
    "  xor  eax,eax    ; assume not set (return 0) \n"
    "                                             \n"
    "  ; loop 32 times to make sure               \n"
    "  mov  ecx,32                                \n"
    "@@: add  dword [si],01010101h                \n"
    "  mov  ebx,[si]                              \n"
    "  call io_delay                              \n"
    "  cmp  ebx,es:[di]                           \n"
    "  loope @b                                   \n"
    "                                             \n"
    "  ; if equal jmp over (return 1)             \n"
    "  je  short @f                               \n"
    "  inc  eax     ; eax = 1, A20 is set         \n"
    "@@:                                          \n"
    "                                             \n"
    "  ; if the values are still the same, the two pointers \n"
    "  ;  point to the same location in memory.             \n"
    "  ; i.e.: the a20 line is not set                      \n"
    "                                             \n"
    "  pop  ebx     ; restore the original value  \n"
    "  mov  [si],ebx                              \n"
    "                                             \n"
    "  pop  es                                    \n"
    "  pop  ds                                    \n"
    "                                             \n"
    "  ret                                        \n"
    "                                             \n"
    " ; =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=   \n"
    " ; a slight delay on older machines          \n"
    "io_delay   proc near uses ax                 \n"
    "  xor  al,al                                 \n"
    "  out  80h,al                                \n"
    "  out  80h,al                                \n"
    "  ret                                        \n"
    "io_delay   endp                              \n"
    "                                             \n"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// waits while keyboard status bit 1 = 1
// returns TRUE if timed-out, else FALSE if okay
// (used in a20 above)
bool wait_kbd_status(const int bit) {
  for (int i=0; i<65535; i++)
    if (!(inpb(0x64) & (1 << bit)))
      return FALSE;
  
  return TRUE;
}

// include the bz2 unzip stuff
#include "loaderbz.h"

// this is the list of files we need to load via this loader
// The can be any length but no paths, and must have valid characters
//  for all filesystems used.  However, since FAT only allows 8.3 format
//  filenames, this must remain 8.3 (unless we write code to get LFN's in FAT)
char system_files[3][11] = { 
  "kernel.sys", 
  "system.sys", 
  ""
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// the file decompressor.
// on entry:
//   compressed data (will be on a dword boundary)
//   size in bytes of file.
//  - *will* be on a dword boundary
//  - we can also destroy the last 3 bytes at the end of each buffer if needed.
//    (i.e.: no need to check for an odd dword length)
// on return:
//  size of uncompressed/moved data in bytes
//    or zero if error
//  uncompressed/moved data
int decompressor(const bit32u decomp_buf_loc, const bit32u size) {
  struct S_LDR_HDR farF *ldr_hdr = (struct S_LDR_HDR farF *) decomp_buf_loc;
  bit8u farF *p = (bit8u farF *) decomp_buf_loc;
  bit32u ret_size = 0;
  bit8u crc = 0;
  bool ret;
  
  // see if the first 32 bytes of the file is a loader header
  // the first 32-bits will == 46595332h
  if (ldr_hdr->id != 0x46595332) {
    puts("...Did not find load header id dword");
    return 0;
  }
  
  // now check the header's crc
  for (int i=0; i<sizeof(struct S_LDR_HDR); i++)
    crc += p[i];
  
  if (crc) {
    puts("...Invalid header crc");
    return 0;
  }
  
  // now check for the compression type
  ret_size = size - sizeof(struct S_LDR_HDR);
  switch(ldr_hdr->comp_type) {
    case 0:
      ret = do_decomp_flat((void *) ldr_hdr->location, decomp_buf_loc + sizeof(struct S_LDR_HDR), &ret_size);
      break;
    case 1:
      ret = do_decomp_bz2((void *) ldr_hdr->location, decomp_buf_loc + sizeof(struct S_LDR_HDR), &ret_size);
      break;
    default:
      puts("...Unknown decompression type found");
      ret = FALSE;
  }
  
  if (!ret)
    return 0;
  else
    return ret_size;
}

bool do_decomp_flat(void *location, const bit32u src, const int *size) {
  bit8u farF *t = (bit8u farF *) location;
  bit8u farF *s = (bit8u farF *) src;
  int cnt = *size;
  
  puts("...moving");
  
  while (cnt--)
    *t++ = *s++;
  
  return TRUE;
}

bool do_decomp_bz2(void *location, const bit32u src, const int *size) {
  int ret_size = *size;
  int ret;
  
  puts("...decompressing(bz2)");
  
  if ((ret = bz2_decompressor((void *) location, (void *) src, &ret_size)) != BZ_OK) {
    printf("...Error decompressing file.  error: %i", ret);
    *size = 0;
    return FALSE;
  }
  
  *size = ret_size;
  return TRUE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// calculate the crc
// on entry:
//  ecx = length
//  fs:edi-> data
// on return:
//  eax = resulting crc
bit32u calc_crc(void *location, const int size) {
  bit8u farF *p = (bit8u farF *) location;
  bit8u octet;
  bit32u result = 0;
  int cnt = size;
  
  if (cnt > 4) {
    // initialize the progress proc
    init_progress(size);
    
    result  = *p++; result <<= 8;
    result |= *p++; result <<= 8;
    result |= *p++; result <<= 8; 
    result |= *p++;
    result  = ~result;
    
    cnt -= 4;
    
    for (int i=0; i<cnt; i++) {
      // display the progress
      put_progress(i, 0);
      
      octet = *p++;
      for (int j=0; j<8; j++) {
        if (result & 0x80000000) {
          result = (result << 1) ^ 0x04C11DB7 ^ (octet >> 7);
        } else {
          result = (result << 1) ^ (octet >> 7);
        }
        octet <<= 1;
      }
    }
    
    // The complement of the remainder
    result = ~result;
  }
  
  return result;
}

// include the file system code
#include "fat.c"
#include "lean.c"
//#include "ext2.c"
#include "fysfs.c"
//#include "exfat.c"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// We need to move the current INT 1Eh table to RAM
//  and set the Max SPT to 36.  Many BIOS's default
//  disk parameter tables will not recognize multi-sector
//  reads beyond the maximum sector number specified in
//  the default diskette parameter tables.  This may
//  mean 7 sectors in some cases.
// We need to also save the original INT 1Eh table
//  address so that we can restore it on (warm) reboot.
_naked void update_int1e(void) {
  _asm (
    "  push ds                     \n"
    "  push es                     \n"
    "  xor  ax,ax                  \n"
    "  mov  es,ax                  \n"
    "  mov  ax,es:[(1Eh*4)+2]      \n"
    "  mov  si,es:[(1Eh*4)]        \n"
    "  mov  [_org_int1e+2],ax      \n"
    "  mov  [_org_int1e+0],si      \n"
    "  mov  ds,ax                  \n"
    "  mov  di,0522h               \n"
    "  mov  es:[(1Eh*4)+2],es      \n"
    "  mov  es:[(1Eh*4)+0],di      \n"
    "  mov  cx," #SIZEOF_S_FLOPPY1E "\n"
    "  cld                         \n"
    "  rep                         \n"
    "    movsb                     \n"
    "  mov  byte es:[di-7],36  ; set max spt to 36 (the most a 2.88 would have) \n"
    "  xor  ax,ax                  \n"
    "  mov  ds,ax                  \n"
    "  mov  si,0522h               \n"
    "  mov  ax," #LOADSEG "        \n"
    "  mov  es,ax                  \n"
    "  mov  di,offset _floppy_1e   \n"
    "  mov  cx," #SIZEOF_S_FLOPPY1E "\n"
    "  rep                         \n"
    "    movsb                     \n"
    "  pop  es                     \n"
    "  pop  ds                     \n"
    "  ret                         \n"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get the bios equipment list 16-bit word value
// Get the keyboard status bits at 0040:0017
_naked void get_bios_equ_list(bit16u *bios_equip, bit8u *kbd_bits) {
  _asm (
    "  push es            \n"
    "  mov  ax,40h        \n"
    "  mov  es,ax         \n"
    "  mov  ax,es:[0010h] \n"
    "  mov  ebx,nPARAM0   \n"
    "  mov  [ebx],ax      \n"
    "  mov  al,es:[0017h] \n"
    "  mov  ebx,nPARAM1   \n"
    "  mov  [ebx],al      \n"
    "  pop  es            \n"
    "  ret                \n"
  );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// converts a bcd numeral to an integer
//
_inline int bcd2dec(const int bcd) {
  return (((bcd & 0xF0) >> 4) * 10) + (bcd & 0x0F);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// get time of day from the BIOS
//
void get_bios_time(struct S_TIME *time) {
  struct REGS regs;
  
  // clear it out
  memset(time, 0, sizeof(struct S_TIME));
  
  regs.eax = 0x00000400;
  intx(0x1A, &regs);
  time->year = (bcd2dec((regs.ecx & 0x0000FF00) >> 8) * 100) + 
                bcd2dec(regs.ecx & 0x000000FF);
  time->month = bcd2dec((regs.edx & 0x0000FF00) >> 8);
  time->day   = bcd2dec(regs.edx & 0x000000FF);
  
  regs.eax = 0x00000200;
  intx(0x1A, &regs);
  time->hour = bcd2dec((regs.ecx & 0x0000FF00) >> 8);
  time->min = bcd2dec(regs.ecx & 0x000000FF);
  time->sec = bcd2dec((regs.edx & 0x0000FF00) >> 8);
  time->d_savings = (regs.edx & 0x000000FF);
  
  time->jiffy = 0;  // todo
  time->weekday = 0; // todo
}

void apm_bios_check(struct S_APM *apm) {
  struct REGS regs;
  
  // clear it out
  memset(apm, 0, sizeof(struct S_APM));
  
  regs.eax = 0x00005300;
  regs.ebx = 0x00000000;
  if (!intx(0x15, &regs) && ((regs.ebx & 0xFFFF) == 0x504D)) {  // 'PM' == 0x504D
    apm->present = TRUE;
    apm->maj_ver = (bit8u) ((regs.eax & 0xFF00) >> 8);
    apm->maj_ver = (bit8u) ((regs.eax & 0x00FF) >> 0);
    apm->flags = (bit16u) (regs.ecx & 0xFFFF);
    
    // get capabilities
    regs.eax = 0x00005310;
    regs.ebx = 0x00000000;
    if (!intx(0x15, &regs)) {
      apm->batteries = (bit8u) (regs.ebx & 0xFF);
      apm->cap_flags = (bit16u) (regs.ecx & 0xFFFF);
      
      // now connect to the 32bit interface
      regs.eax = 0x00005303;
      regs.ebx = 0x00000000;
      if (!intx(0x15, &regs)) {
        apm->cs_segment32 = (bit16u) (regs.eax & 0xFFFF);
        apm->entry_point = regs.ebx;
        apm->cs_segment = (bit16u) (regs.ecx & 0xFFFF);
        apm->ds_segment = (bit16u) (regs.edx & 0xFFFF);
        apm->cs_length32 = (bit16u) (regs.esi & 0xFFFF);
        apm->cs_length16 = (bit16u) ((regs.esi >> 16) & 0xFFFF);
        apm->ds_length = (bit16u) (regs.edi & 0xFFFF);
      } else {
        apm->present = FALSE;
        apm->error_code = (bit16u) (regs.eax & 0xFFFF);   // ah = error_code, al = function
      }
    } else {
      apm->present = FALSE;
      apm->error_code = (bit16u) (regs.eax & 0xFFFF);   // ah = error_code, al = function
    }
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// make sure the floppy disk drive motor is turned off.
//  put a 1 at 0x00440 so that the next time the BIOS
//  decrements it, it will turn of the motor.
_naked void floppy_off(void) {
  
  _asm (
    "  push es      \n"
    "  mov  ax,40h  \n"
    "  mov  es,ax   \n"

    // first check to see that [0x00440] > 0, if not, don't
    //  do check this code.  It seems that the Compaq BIOS 
    //  does not do the "decrement" if the drive is not turning.
    //  It may only hook the timer tick interrupt when the drive
    //  is turning, and when it counts to 0, it unhooks itself.
    "  cmp  byte es:[40h],0      \n"
    "  je   short drive_no_wait  \n"
    "  mov  byte es:[40h],1      \n"
    
    // we need to wait for this to actually happen since
    //  the later 'cli' won't allow the timer tick interrupt to fire
    // make sure the interrupt flag is set or we will wait forever
    "  sti                       \n"
    "@@: cmp  byte es:[40h],0    \n"
    "  jne  short @b             \n"
    "\n"
    "drive_no_wait:              \n"
    "  pop  es                   \n"
    "  ret                       \n"
  );
}

_naked bit8u inpb(int d) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  in   al,dx       \n"
    "  ret              \n"
  );
}

_naked bit16u inpw(int d) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  in   ax,dx       \n"
    "  ret              \n"
  );
}

_naked bit32u inpd(int d) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  in   eax,dx      \n"
    "  ret              \n"
  );
}

_naked void outpb(int d, bit8u v) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  out  dx,al       \n"
    "  ret              \n"
  );
}

_naked void outpw(int d, bit16u v) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  out  dx,ax       \n"
    "  ret              \n"
  );
}

_naked void outpd(int d, bit32u v) {
  _asm (
    "  mov  edx,nPARAM0 \n"
    "  out  dx,eax      \n"
    "  ret              \n"
  );
}

// see http://www.cplusplus.com/reference/cstdio/printf/ for parameters
//
int printf(char *fmt, ...) {
  va_list vl = (va_list) ((char *) &fmt + sizeof(char *));
  
  int c, sign, width, precision, lmodifier;
  bool ljust, alt, lzeroes;
  
  while ((c = (bit8u) *fmt++) != '\0') {
    if (c != '%' || *fmt == '%') {
      putchar(c);
      fmt += (c == '%');
      continue;
    }
    if ((c = (bit8u) *fmt++) == '\0')
      return -1;
    
    ljust = alt = lzeroes = FALSE;
    sign = 0;
    for (;;) {
      if (c == '-') {
        ljust = TRUE;
        lzeroes = FALSE;
      } else if (c == '+')
        sign = '+';
      else if (c == ' ') {
        if (!sign)
          sign = ' ';
      } else if (c == '#')
        alt = TRUE;
      else if (c == '0') {
        if (!ljust)
          lzeroes = TRUE;
      } else
        break;
      
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    }
    
    width = -1;
    if (isdigit(c)) {
      width = 0;
      while (isdigit(c)) {
        width = width * 10 + (c - '0'); // TBD??? overflow check???
        if ((c = (bit8u) *fmt++) == '\0')
          return -1;
      }
    } else if (c == '*') {
      width = *(int *) vl; vl += sizeof(int);
      if (width < 0) {
        ljust = TRUE;
        lzeroes = FALSE;
        width = -width; // TBD??? overflow check???
      }
      if ((c = *fmt++) == '\0')
        return -1;
    }
    
    precision = -1;
    if (c == '.') {
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
      precision = 0;
      lzeroes = FALSE;
      if (isdigit(c)) {
        while (isdigit(c)) {
          precision = precision * 10 + (c - '0'); // TBD??? overflow check???
          if ((c = (bit8u) *fmt++) == '\0')
            return -1;
        }
      }  else if (c == '*') {
        precision = *(int *) vl; vl += sizeof(int);
        if ((c = *fmt++) == '\0')
          return -1;
      }
    }
    
    lmodifier = 0;
    if (c == 'h') {
      if (*fmt == 'h') {
        fmt++;
        lmodifier = 'H';
      } else
        lmodifier = c;
    } else if (strchr("jzt", c))
      lmodifier = c;
    if (lmodifier)
      if ((c = (bit8u) *fmt++) == '\0')
        return -1;
    
    if (c == 'i')
      c = 'd';
    if (!strchr("douxXcsp", c))
      return -1;
    
    if (c == 'c') {
      int ch = (bit8u) *(int *) vl; vl += sizeof(int);
      if (!ljust)
        while (width > 1) {
          putchar(' ');
          width--;
        }
      putchar(ch);
      
      if (ljust)
        while (width > 1) {
          putchar(' ');
          width--;
        }
      continue;
    } else if (c == 's') {
      int len, i;
      char *s = *(char **) vl; vl += sizeof(char *);
      
      if (precision < 0)
        len = strlen(s); // TBD??? overflow check???
      else {
        len = 0;
        while (len < precision)
          if (s[len]) len++;
          else        break;
      }
      
      if (!ljust)
        while (width > len) {
          putchar(' ');
          width--;
        }
      
      i = len;
      while (i--)
        putchar(*s++);
       
      if (ljust)
        while (width > len) {
          putchar(' ');
          width--;
        }
      continue;
    } else {
      unsigned v = *(unsigned *) vl, tmp;
      char s[11]; // up to 11 octal digits in 32-bit numbers
      char *p = s + sizeof(s);
      unsigned base = (c == 'p') ? 16 : 10;
      char *digits = "0123456789abcdef";
      char *hexpfx = NULL;
      int dcnt;
      int len;
      vl += sizeof(unsigned);
      
      if (c == 'o')
        base = 8;
      else if (toupper(c) == 'X') {
        base = 16;
        if (c == 'X')
          digits = "0123456789ABCDEF";
        if (alt && v)
          hexpfx = (c == 'X') ? "0X" : "0x";
      }
      
      if (c != 'd') {
        if (lmodifier == 'H')
          v = (bit8u) v;
        else if (lmodifier == 'h')
          v = (unsigned short) v;
        sign = 0;
      } else {
        if (lmodifier == 'H')
          v = (signed char) v;
        else if (lmodifier == 'h')
          v = (short) v;
        if ((int) v < 0) {
          v = -v;
          sign = '-';
        }
      }
      
      tmp = v;
      do {
        *--p = digits[tmp % base];
        tmp /= base;
      } while (tmp);
      dcnt = s + sizeof(s) - p;
      
      if (precision < 0)
        precision = 1;
      else if ((v == 0) && (precision == 0))
        dcnt = 0;
      
      if (alt && (c == 'o'))
        if (((v == 0) && (precision == 0)) || (v && (precision <= dcnt)))
          precision = dcnt + 1;
      
      if (precision < dcnt)
        precision = dcnt;
      
      // width padding:
      // - left/right
      // - spaces/zeroes (zeroes are to appear after sign/base prefix)
      // sign:
      // - '-' if negative
      // - '+' or '-' always
      // - space if non-negative or empty
      // alt:
      // - octal: prefix 0 to conversion if non-zero or empty
      // - hex: prefix "0x"/"0X" to conversion if non-zero
      // precision:
      // - prefix conversion digits with zeroes to precision
      // - special case: 0 with precision=0 results in empty conversion
      // [leading spaces] [sign/hex prefix] [leading zeroes] [(precision-dcnt) zeroes] [dcnt digits] [trailing spaces]
      len = (sign != 0) + (hexpfx != NULL) * 2 + precision;
      
      if (!ljust && !lzeroes)
        while (width > len) {
          putchar(' ');
          width--;
        }
      
      if (sign)
        putchar(sign);
      else if (hexpfx) {
        putchar(hexpfx[0]);
        putchar(hexpfx[1]);
      }
      
      if (!ljust && lzeroes)
        while (width > len) {
          putchar('0');
          width--;
        }
      
      while (precision-- > dcnt)
        putchar('0');
      
      while (dcnt--)
        putchar(*p++);
      
      if (ljust)
        while (width > len) {
          putchar(' ');
          width--;
        }
      
      continue;
    }
  }
  
  return 0;
}

int putchar(const int ch) {
  struct REGS regs;
  
  regs.eax = (0x0E00 | (ch & 0xFF));
  regs.ebx = 0;
  intx(0x10, &regs);
  
  if (ch == 10)
    putchar(13);
  
  return ch;
}

int puts(const char *s) {
  bit8u ch;
  while (*s != '\0')
    putchar(ch = *s++);
  return ch;
}

void freeze(void) {
  _asm (
    "halt: \n"
    "  hlt\n"
    "  jmp short halt\n"
  );
}

_naked void *memset(void *targ, bit8u val, const int cnt) {
  _asm (
    "  mov  edi,nPARAM0  ; destination \n"
    "  mov  eax,nPARAM1  ; value \n"
    "  mov  ah,al        ; put in ah for words \n"
    "  mov  ecx,nPARAM2  ; count \n"
    "  shr  ecx,1        ; div by 2 (save the carry) \n"
    "  cld               ; make sure we're going forward \n"
    "  rep               ; \n"
    "    stosw           ; \n"
    "  rcl  ecx,1        ; bring the carry back into cx \n"
    "  rep               ; \n"
    "    stosb           ; \n"
    "  mov  eax,nPARAM0  ; return value \n"
    "  ret               ; \n"
  );
}

// This assumes that the memory does not overlap each other
_naked void *memmove(void *targ, const bit16u src_ds, void *src, const unsigned int cnt) {
  _asm (
    "  push ds          \n"
    "  mov  edi,nPARAM0 \n"
    "  mov  eax,nPARAM1 \n"
    "  mov  ds,ax       \n"
    "  mov  esi,nPARAM2 \n"
    "  mov  ecx,nPARAM3 \n"
    "  push ecx         \n"
    "  shr  ecx,2       \n"
    "  cld              \n"
    ".adsize            \n"
    "  rep              \n"
    "    movsd          \n"
    "  pop  ecx         \n"
    "  and  ecx,3       \n"
    ".adsize            \n"
    "  rep              \n"
    "    movsb          \n"
    "  pop  ds          \n"
    "  ret              \n"
  );
}

// must assume DS: for source and ES: for target
_naked void *memcpy(void *targ, const void *src, const int cnt) {
  _asm (
    "  mov  edi,nPARAM0  ; \n"
    "  mov  esi,nPARAM1  ; \n"
    "  mov  ecx,nPARAM2  ; \n"
    "  push ecx          ; \n"
    "  shr  ecx,2        ; \n"
    "  cld               ; make sure we're going forward \n"
    ".adsize             ; \n"
    "  rep               ; \n"
    "    movsd           ; \n"
    "  pop  ecx          ; \n"
    "  and  ecx,3        ; \n"
    ".adsize             ; \n"
    "  rep               ; \n"
    "    movsb           ; \n"
    "  mov  eax,nPARAM0  ; return value \n"
    "  ret               ; \n"
  );
}

// allows the change of DS:, but assumes ES: for target
void *memcpy_ds(void *targ, const void *src, const int ds, const int cnt) {
  _asm (
    "  push ds           ; save original ds \n"
    "  mov  edi,PARAM0   ; target \n"
    "  mov  esi,PARAM1   ; source \n"
    "  mov  eax,PARAM2   ; new ds \n"
    "  mov  ds,ax        ; \n"
    "  mov  ecx,PARAM3   ; count \n"
    "  push ecx          ; \n"
    "  shr  ecx,2        ; \n"
    "  cld               ; make sure we're going forward \n"
    ".adsize             ; \n"
    "  rep               ; \n"
    "    movsd           ; \n"
    "  pop  ecx          ; \n"
    "  and  ecx,3        ; \n"
    ".adsize             ; \n"
    "  rep               ; \n"
    "    movsb           ; \n"
    "  pop  ds           ; restore original ds \n"
  );
  
  return targ;
}

// return -1 if targ < than src
// return  0 if targ == than src
// return  1 if targ > than src
int stricmp(const char *targ, const char *src) {
	const bit8u *t = (const bit8u *) targ;
  const bit8u *s = (const bit8u *) src;
  
  while (tolower(*t) == tolower(*s)) {
		if (*t++ == '\0')
			return 0;
    s++;
  }
  
  if (tolower(*t) < tolower(*s))
    return -1;
  else
    return 1;
}
/*
unsigned char __chartype__[1 + 256] = {
  0x00, // for EOF=-1
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01, 0x01,0x03,0x03,0x03,0x03,0x03,0x01,0x01,
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
  0x02,0x04,0x04,0x04,0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
  0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08, 0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04,
  0x04,0x50,0x50,0x50,0x50,0x50,0x50,0x10, 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
  0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x04,0x04,0x04,0x04,0x04,
  0x04,0x60,0x60,0x60,0x60,0x60,0x60,0x20, 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20, 0x20,0x20,0x20,0x04,0x04,0x04,0x04,0x01
};
*/
int isdigit(int c) {
  //return __chartype__[c + 1] & 0x08;
  return ((c >= '0') && (c <= '9'));
}

int toupper(int c) {
  //return c - (__chartype__[c + 1] & 0x20);
  if ((c >= 'a') && (c <= 'z'))
    return (c - ('a' - 'A'));
  return c;
}

int tolower(int c) {
  //return c + ((__chartype__[c + 1] & 0x10) << 1);
  if ((c >= 'A') && (c <= 'Z'))
    return (c + ('a' - 'A'));
  return c;
}

_naked char *strchr(char *s, int c) {
  _asm (
    "  mov  esi,nPARAM0 ; source string    \n"
    "  mov  ebx,nPARAM1 ; char to look for \n"
    "  xor  eax,eax     ; assume NULL      \n"
    "  cld              ; make sure we go forward \n"
    "@@:                ; \n"
    "  lodsb            ; get the first char \n"
    "  cmp  al,bl       ; \n"
    "  je   short @f    ; found it \n"
    "  or   al,al       ; are we at the eos? \n"
    "  jnz  short @b    ;  \n"
    "  ret              ; \n"
    "@@:                ; \n"
    "  lea  eax,[esi-1] ; return offset \n"
    "  ret              ; \n"
  );
}

_naked unsigned strlen(char *str) {
  _asm (
    "  mov  edi,nPARAM0 ; source string  \n"
    "  mov  ecx,-1      ; \n"
    "  xor  al,al       ; \n"
    "  cld              ; make sure we go forward \n"
    "  repnz            ; \n"
    "    scasb          ; \n"
    "  mov  eax,ecx     ; \n"
    "  not  eax         ; \n"
    "  dec  eax         ; \n"
    "  ret              ; \n"
  );
}

_naked void add64(void *targ, void *src) {
  _asm (
    "  mov  edi,nPARAM0     \n"
    "  mov  esi,nPARAM1     \n"
    "  mov  eax,[esi + 0]   \n"
    "  add  [edi + 0],eax   \n"
    "  mov  eax,[esi + 4]   \n"
    "  adc  [edi + 4],eax   \n"
    "  ret                  \n"
  );  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Progress bar
bit32u progress_tot_size  = 0;  // set to total size at start of progress
int    progress_last      = 0;  // last percent printed
bit8u  progress_x, progress_y;  // cursor start location

// this will not work for numbers greater than 42,949,671.
// for example, the calculation of
//   val = ((lo + 1) * 100);
// when lo = 42,949,672 would be
//   val = ((42,949,672 + 1) * 100) = 4,294,967,300 = 0x1_0000_0004
// and 0x1_0000_0004 is a 33 bit number
// so, until we can use 64-bit numbers, 42,949,671 is the limit
void init_progress(bit32u limit) {
  struct REGS regs;
  
  limit--;  // zero based
  
  if (limit > 42949671)
    limit = 42949671;
  if (limit < 0)
    limit = 0;
  
  progress_tot_size = limit;
  progress_last = -1;
  
  // get and save the current cursor position
  regs.eax = 0x00000300;
  regs.ebx = 0x00000000;
  intx(0x10, &regs);
  progress_x = regs.edx & 0xFF;
  progress_y = (regs.edx >> 8) & 0xFF;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Progress counter.  Displays (n%)
// on entry
//  current progress (64-bit)
#define PP_INDICIES 12
void put_progress(bit32u lo, bit32u hi) {
  struct REGS regs;
  bit32u val, cnt, i, j;
  
  // watch the limit
  if (lo > 42949671)
    lo = 42949671;
  
  // don't allow it to be more than the set total size
  //  from init_progress()
  if (lo > progress_tot_size)
    lo = progress_tot_size;
  
  // increment it to make it print 100% when done
  //  (1 based) (1  to 100, not 0 to 99)
  // this line is the limit maker.  If we could do
  //  this calculation in 64-bit, there would be a 
  //  much larger limit.
  val = ((lo + 1) * 100) / progress_tot_size;
  
  // don't let val be more than 100%
  // may happen on numbers less than PP_INDICIES
  if (val > 100)
    val = 100;
  
  if (val != progress_last) {
    progress_last = val;
    
    // print so many bars  ( ascii 221, then 219)
    putchar(' ');
    putchar(179);
    cnt = ((PP_INDICIES * 2 * (val + 1)) + 99) / 100;
    j = cnt / 2;
    for (i=0; i<j; i++)
      if (i < (j - 1))
        putchar(219);
      else {
        if (cnt & 1) putchar(219);
        else         putchar(221);
      }
    for (; i<PP_INDICIES; i++)
      putchar('_'); // or do 176
    putchar(179);
    
    printf(" (%i%%)", val);
    
    // set the location of the cursor to the saved position
    // either for the next call, or to clear the bar when we
    //  are done with the progress.
    regs.eax = 0x00000200;
    regs.edx = (progress_y << 8) | progress_x;
    regs.ebx = 0x00000000;
    intx(0x10, &regs);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC32
bit32u crc32_table[256]; // CRC lookup table array.

void crc32_initialize(void) {
  memset(crc32_table, 0, sizeof(crc32_table));
  
  // 256 values representing ASCII character codes.
  for (int i=0; i<=0xFF; i++) {
    crc32_table[i] = crc32_reflect(i, 8) << 24;
    
    for (int j=0; j<8; j++)
      crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? CRC32_POLYNOMIAL : 0);
    
    crc32_table[i] = crc32_reflect(crc32_table[i], 32);
  }
}

// Reflection is a requirement for the official CRC-32 standard.
//  You can create CRCs without it, but they won't conform to the standard.
bit32u crc32_reflect(bit32u reflect, char ch) {
  bit32u ret = 0;
  
  // Swap bit 0 for bit 7 bit 1 For bit 6, etc....
  for (int i=1; i<(ch + 1); i++) {
    if (reflect & 1)
      ret |= 1 << (ch - i);
    reflect >>= 1;
  }
  
  return ret;
}

bit32u crc32(void *data, bit32u len) {
  bit32u crc = 0xFFFFFFFF;
  crc32_partial(&crc, data, len);
  return (crc ^ 0xFFFFFFFF);
}

void crc32_partial(bit32u *crc, void *ptr, bit32u len) {
  bit8u *data = (bit8u *) ptr;
  while (len--)
    *crc = (*crc >> 8) ^ crc32_table[(*crc & 0xFF) ^ *data++];
}

_asm (
  " .para  ; make sure to align on a 16-byte boundary  \n"
  "gdtoff_temp dw ((8*5)-1)                            \n"
  "            dd  ((" #LOADSEG "<<4) + gdtoff_temp)   \n"
  "            dw  0   ; filler                        \n"
  " \n"
  "            ; code16 desc                           \n"
  "            dw  0FFFFh     ; -------------> limit 0xFFFF + (byte below)  \n"
  "            dw   0000h     ; ______/------> base at LOADSEG              \n"
  "            db     03h     ; /----/                                         \n"
  "            db     9Ah     ; |   Code(E/R), S=1, Priv = 00b, present = Yes  \n"
  "            db     00h     ; |   0 (limit), avl = 0, 0, 16-bit, gran = 0    \n"
  "            db     00h     ; /                                           \n"
  " \n"
  "            ; flatdata16 desc                      \n"
  "            dw  0FFFFh     ; -------------> limit 4gig + (byte below)   \n"
  "            dw   0000h     ; ______/------> base at 0x00000000          \n"
  "            db     00h     ; /----/                                     \n"
  "            db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes  \n"
  "            db     8Fh     ; |   F (limit), avl = 0, 0, 16-bit, gran = 1    \n"
  "            db     00h     ; /                                           \n"
  " \n"
  "            ; stack16 desc  ; LOADSTACK16  \n"
  "            dw  0FFFFh     ; -------------> limit 4gig + (byte below)   \n"
  "            dw   0000h     ; ______/------> base at LOADSEG             \n"
  "            db     03h     ; /----/                                     \n"
  "            db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes  \n"
  "            db    0CFh     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1    \n"
  "            db     00h     ; /                                           \n"
  " \n"
  "            ; loaddata16 desc  ; LOADDATA16  \n"
  "            dw  0FFFFh     ; -------------> limit 4gig + (byte below)   \n"
  "            dw   0000h     ; ______/------> base at LOADSEG             \n"
  "            db     03h     ; /----/                                     \n"
  "            db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes  \n"
  "            db     8Fh     ; |   F (limit), avl = 0, 0, 16-bit, gran = 1    \n"
  "            db     00h     ; /                                           \n"
  " \n"
);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  This is the 4096 byte block that we will move to ('kernel_base'-0C00h) for
//  the kernel.sys file *after we load the kernel.sys file*
// ***** don't forget to update S_LOADER in fysos.h ************
#pragma pack(push, 1)

_asm (
  // this is the two code and data segment entries for the new GDT at 0x00110000
  // code desc
  "act_gdt    dup 8,0        ; first is always null \n"
  
  "  dw  0FFFFh     ; -------------> limit 4gig + (byte below)\n"
  "  dw   0000h     ; ______/------> base at 0x00000000\n"
  "  db     00h     ; /----/\n"
  "  db     9Ah     ; |   Code(E/R), S=1, Priv = 00b, present = Yes\n"
  "  db    0CFh     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1\n"
  "  db     00h     ; /\n"
  
  // data desc (matches cs (mostly))
  "  dw  0FFFFh     ; -------------> limit 4gig + (byte below)\n"
  "  dw   0000h     ; ______/------> base at 0x00000000\n"
  "  db     00h     ; /----/                            \n"
  "  db     92h     ; |   Data(R/W), S=1, Priv = 00b, present = Yes\n"
  "  db    0CFh     ; |   F (limit), avl = 0, 0, 32-bit, gran = 1\n"
  "  db     00h     ;/  \n"

  " .para  ; make sure to align on a 16-byte boundary  \n"
  "_gdtoff    dw  ((256*8)-1)         ; Address of our GDT    \n"
  "           dd  00110000h           ;  KERN_GDT in memory.h \n"
  " \n"
  "idtoff     dw  ((256*8)-1)         ; 256 = number of interrupts we allow  \n"
  "           dd  00110800h           ;  KERN_IDT in memory.h                \n"
  " \n"
);

bit32u bios_type;             // 'BIOS' = legacy BIOS, 'UEFI' = UEFI booted          //    4
bit32u uefi_image_handle;     // UEFI Image Handle                                   //    4
bit32u uefi_system_table;     // UEFI System Table Pointer                           //    4
struct S_BOOT_DATA boot_data; // booted data                                         //   48
bit8u  resv0[32];             // reserved                                            //   32
struct S_BIOS_PCI bios_pci;   // PCI information from the BIOS                       //    8
bit32u org_int1e;             // original INT1Eh address                             //    4
struct S_FLOPPY1E floppy_1e;  // floppies status                                     //   11
struct S_TIME time;         // current time passed to kernel                         //   14
struct S_APM apm;           // Advanced Power Management                             //   44
bit8u  resv1[3];            // dword alignment                                       //    3
bit16u bios_equip;          // bios equipment list at INT 11h (or 0040:0010h)        //    2
bit8u  kbd_bits;            // bits at 0x00417                                       //    1
struct S_MEMORY memory;     // memory blocks returned by INT 15h memory services     // 1164
bit8u  a20_tech;            // the technique number used to enable the a20 line      //    1
bit16u vid_mode_cnt;        // count of video mode info blocks found                 //    2
bit16u cur_vid_index;       // index into mode_info[] of chosen/default/current mode //    2
struct S_MODE_INFO mode_info[VIDEO_MAX_MODES]; // video modes information for kernel             //   VIDEO_MAX_MODES * 16
bit8u  resv2[29];           // reserved                                              //   29
struct S_DRV_PARAMS drive_params[10];  // up to 10 hard drive parameter tables       //  960
bit8u  padding[2067];       // padding to 4096 bytes                                 // 

#pragma pack(pop)

_asm (
  ".ifne ($ - _gdtoff) 1400h                          \n"
  " %error 1 'transfer block not 1400h bytes in size' \n"
  " %print ($ - _gdtoff)                              \n"
  ".endif                                             \n"
  " \n"
);

/*
// must be called while in "32-bit" mode
void debugit(int n) {
  printf("here %i\n", n);
  _asm (
      "  xor  ah,ah              ; Wait for keypress\n"
      "  int  16h                ; \n"
  );
}
*/

///////////////////////////////////////////////////////////////////
//  We need to go back to 186 for this code.  We haven't moved to
//  32-bit code when we print the 486 check stuff, so the SizeOfWord
//  still needs to be 2 for the caller *and* the callee
//.186                          
#pragma proc(186)
#pragma proc(short)              // we want 16-bit offsets

// we haven't set up a "large" stack yet, so we can call the interrupt directly
int putchar16(const int ch) {
  _asm (
    "  mov  ax,[bp+4] ; \n"
    "  mov  ah,0Eh    ; print char service \n"
    "  xor  bx,bx     ; \n"
    "  int  10h       ; output the character \n"
  );
  
  if (ch == 10)
    putchar16(13);
  
  return ch;
}

int puts16(const char *s) {
  bit8u ch;
  while (*s != '\0')
    putchar16(ch = *s++);
  return ch;
}

// A local buffer used so that the offset is 0000xxxxh
//  we make it large enough to hold a single track of sectors from a floppy disk
//  (18 sectors per track is a 1.44meg)
// (however, we are near the 64k limit here, so we limit it to LOCAL_BUFF_SECT_SIZE sectors)
bit8u local_buffer[LOCAL_BUFF_SECT_SIZE * 512];

