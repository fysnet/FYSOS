/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/fysnet/SmallerC)
 *  smlrcc @make.txt
 */

#include "ctype.h"
#include "loader.h"

#include "a20.h"
#include "apm.h"
#include "conio.h"
#include "crc32.h"
#include "disks.h"
#include "malloc.h"
#include "paraport.h"
#include "pci.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys.h"
#include "time.h"
#include "video.h"
#include "windows.h"

#include "decompressor.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// the actual GDT's we send to the kernel
struct S_GDT act_gdt[3] = {
  { 0, },
  {
    0xFFFF,    // -------------> limit 4gig (and byte below)
    0x0000,    // ______/------> base at 0x00000000
      0x00,    // /----/
      0x9A,    // |   Code(E/R), S=1, Priv = 00b, present = Yes
      0xCF,    // |   F (limit), avl = 0, 0, 32-bit, gran = 1
      0x00     // /
  },
  {
    0xFFFF,    // -------------> limit 4gig (and byte below)
    0x0000,    // ______/------> base at 0x00000000
      0x00,    // /----/
      0x92,    // |   Data(R/W), S=1, Priv = 00b, present = Yes
      0xCF,    // |   F (limit), avl = 0, 0, 32-bit, gran = 1
      0x00     // /
  }
};

// this is the list of files we need to load via this loader
// The can be any length but no paths, and must have valid characters
//  for all filesystems used.  However, since FAT only allows 8.3 format
//  filenames, this must remain 8.3 (unless we write code to get LFN's in FAT)
char system_files[3][11] = { 
  "kernel.sys",
  "system.sys",
  ""
};

bit32u kernel_base = 0;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// our system block we pass to the kernel
struct S_SYS_BLOCK sys_block;

int main(struct REGS *boot_regs) {
  int i, j;
  bool b;
  bit8u temp_buf[128];
  char  temp_str[128];
  struct REGS regs;
  int halt = 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // set to screen 03h and turn off the cursor
  // (if already screen mode 3 (or 7), don't re-set it.)
  //  This is so we can see what the BIOS/POST printed if we have an error
  //  before we clear the screen...)
  asm (
    "  mov  ax,0F00h           ; get current mode number\n"
    "  int  10h                ;\n"
    "  cmp  al,03h             ;\n"
    "  je   short no_reset     ;\n"
    "  cmp  al,07h             ;\n"
    "  je   short no_reset     ;\n"
    "  mov  ax,0003h           ; make sure we are in screen mode 03h\n"
    "  int  10h                ; 80x25\n"
    "no_reset:                 ;\n"
    "  mov  ax,0103h           ; al = current screen mode (bug on some BIOSs)\n"
    "  mov  ch,00100000b       ; bits 6:5 = 01b\n"
    "  int  10h \n"
  );
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Check for 386+ machine.  If not, give error and halt
  //   Returns processor type
  sys_block.has_cpuid = 0;
  sys_block.has_rdtsc = 0;
  puts("\nChecking for a 486+ processor with the RDTSC instruction...");
  i = chk_486();
  if (i < 6) {
    puts("A 486+ compatible processor w/the CPUID and RDTSC instructions is recommended.\n"
         " Processor detected is a: ");
    switch (i) {
      case 0:
        puts(" 8086 or compatible.");
        break;
      case 1:
        puts(" 80186 or compatible.");
        break;
      case 2:
        puts(" 80286 or compatible.");
        break;
      case 3:
        puts(" 80386 or compatible.");
        break;
      case 4:
        puts(" 80486 or compatible without CPUID.");
        break;
      case 5:
        puts(" 80486 or compatible with CPUID but without RDTSC.");
        sys_block.has_cpuid = 1;
        break;
    }
#if ALLOW_SMALL_MACHINE
    puts("\nPress a key to continue.");
    getscancode();
#else
    puts("\nPress a key to reboot machine.");
    getscancode();
    asm (
      "  mov  dl,80h             ; \n"
      "  int  18h                ; boot specs say that 'int 18h'\n"
    );
#endif
  } else {
    sys_block.has_cpuid = 1;
    sys_block.has_rdtsc = 1;
  }
  sys_block.is_small_machine = ALLOW_SMALL_MACHINE;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // save our boot_data code
  memcpy(&sys_block.boot_data, (void *) boot_regs->ebx, sizeof(struct S_BOOT_DATA));
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // initialize the crc32 stuff
  crc32_initialize();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now we need to make sure the a20 line is active
  //  and save the technique number used
  // (before the move to unreal mode)
  // (we don't have our memory manager up yet, so save in a temp var)
  sys_block.a20_tech = set_a20_line();
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// clear the keyboard (using a safety catch so we don't just loop endlessly
//  if there is something wrong with the keyboard
  i = 16;
  while (i-- && kbhit())
    getscancode();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // and hook the BIOS keyboard interrupt
  old_isr9 = hook_vector(9, &keyboard_isr);
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// initialize our memory allocation
  mem_initialize();
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// check to make sure we compiled our system data block correctly
  if (sizeof(struct S_SYS_BLOCK) != 0x1400) {
    printf("Size of our System Block was not correctly calculated...\n"
           " Found size is 0x%X and it should be 0x1400\n",
           sizeof(struct S_SYS_BLOCK));
    freeze();
  }
  
  // mark that we use a Legacy BIOS
  sys_block.bios_type = 0x42494F53;  // 'BIOS';
  
  // mark the four magic numbers
  sys_block.magic0 = SYS_B_MAGIC0;
  sys_block.magic1 = SYS_B_MAGIC1;
  sys_block.magic2 = SYS_B_MAGIC2;
  sys_block.magic3 = SYS_B_MAGIC3;
  
  // initialize the gdt and idt values
  sys_block.gdtoff = ((256*8)-1);  // Address of our GDT
  sys_block.gdtoffa = 0x00110000;  // KERN_GDT in memory.h
  sys_block.idtoff = ((256*8)-1);  // 256 = number of interrupts we allow
  sys_block.idtoffa = 0x00110800;  // KERN_IDT in memory.h
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// initialize our "windowing" system
  win_initialize();
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// create the main window
  main_win = win_create(NULL, " FYSOS v2.0 Loader", NULL, 0, 0, -1, -1, WIN_HAS_TITLE | WIN_HAS_STATUS | WIN_HAS_BORDER | WIN_HAS_VSCROLL);
  win_status_update(main_win, " Starting FYSOS...", FALSE);
  if (spc_key_F1) help_screen();  // check to see if the help screen has been requested
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Now we can request the allotted memory from the BIOS
  win_status_update(main_win, " Retrieving the memory map...", TRUE);
  get_memory(&sys_block.memory);
  sprintf(temp_buf, " Found %12u bytes", sys_block.memory.size[0]);
  win_status_update(main_win, temp_buf, TRUE);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Now get the PCI information. (Detection only)
  get_pci_info(&sys_block.bios_pci);
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Now load the system file(s)
//  Search for the files in root directory
  
  // check for large disk support
  if (!large_disk_support(sys_block.boot_data.drive, &large_disk)) {
    win_printf(main_win, "Error retrieving disk information...\n");
    freeze();
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // print the loading string
  if (spc_key_F1) help_screen();  // check to see if the help screen has been requested
  win_printf(main_win, "Loading system files...\n");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // allocate the compressed data buffer (used for each file found)
  void *buffer = malloc(DECOMP_BUFFER_SIZE);
  if (buffer == NULL) {
    win_printf(main_win, "Error allocating compressed data buffer...\n");
    freeze();
  }
  
  i = 0;
  while (system_files[i][0]) {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // print the status string
    win_printf(main_win, "Loading: %s", system_files[i]);
    if (spc_key_F1) help_screen();  // check to see if the help screen has been requested
    
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // we first try the root directory, then '\boot', then finally '\system\boot'
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    b = FALSE;
    for (j=0; j<3 && !b; j++) {
      switch (j) {
        case 0:
          strcpy(temp_str, system_files[i]);
          break;
        case 1:
          strcpy(temp_str, "boot\\");
          strcat(temp_str, system_files[i]);
          break;
        case 2:
          strcpy(temp_str, "system\\boot\\");
          strcat(temp_str, system_files[i]);
      }
      
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      // here is were we check the fs type for FAT12/16/32, etc
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      bit32u ret_size = 0;
      switch (sys_block.boot_data.file_system) {
#ifdef FS_FAT12
        case FS_FAT12:
#endif
#ifdef FS_FAT16
        case FS_FAT16:
#endif
#ifdef FS_FAT32
        case FS_FAT32:
#endif
#if defined(FS_FAT12) || defined(FS_FAT16) || defined(FS_FAT32)
          sprintf(temp_buf, "FYSOS v2.0 Loader - (FAT: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_fat(temp_str, buffer);
          break;
#endif
        
#ifdef FS_SFS
        case FS_SFS:
          sprintf(temp_buf, "FYSOS v2.0 Loader - (SFS: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_sfs(temp_str, buffer);
          break;
#endif        

#ifdef FS_LEAN
        case FS_LEAN:
          sprintf(temp_buf, "FYSOS v2.0 Loader - (LeanFS: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_leanfs(temp_str, buffer);
          break;
#endif        

#ifdef FS_EXT2
        case FS_EXT2:
          sprintf(temp_buf, "FYSOS v2.0 Loader - (Ext2: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_ext2(temp_str, buffer);
          break;
#endif        
        
#ifdef FS_FYSFS
        case FS_FYSFS:
          sprintf(temp_buf, "FYSOS v2.0 Loader - (FYSFS: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_fysfs(temp_str, buffer);
          break;
#endif
        
#ifdef FS_EXFAT
        case FS_EXFAT:
          sprintf(temp_buf, "FYSOS v2.0 Loader - (ExFAT: %s)", temp_str);
          win_title_update(main_win, temp_buf);
          ret_size = fs_exfat(temp_str, buffer);
          break;
#endif
          
        default:
          win_printf(main_win, "1st stage boot sent unknown file system identifier: (%i)\n", sys_block.boot_data.file_system);
          freeze();
      }
      
      if (ret_size) {
        struct S_LDR_HDR *ldr_hdr = (struct S_LDR_HDR *) buffer;
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // now that it is at 'buffer', call the decompressor and let
        // it place the decompressed file to the actual location
        if ((ret_size = decompressor(buffer, ret_size)) > 0) {
          // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          // Now do a CRC check on the file.
          win_printf(main_win, "...checking crc...");
          if (calc_crc((void *) ldr_hdr->location, ret_size) == ldr_hdr->file_crc) {
            win_printf(main_win, "(file crc passed)\n");
            // if this is the kernel, we need to update our pointer
            if (ldr_hdr->flags & LDR_HDR_FLAGS_ISKERNEL) {
              // if the kernel_base is not zero, we already have found a
              //  file with this flag set.  So give error.
              if (kernel_base != 0) {
                win_printf(main_win, "Already set 'kernel_base' to 0x%08X...\n", kernel_base);
                halt = 1;
              } else
                kernel_base = ldr_hdr->location;
            }
          } else {
            win_printf(main_win, "...Invalid file crc   \n");
            halt = (ldr_hdr->flags & LDR_HDR_FLAGS_HALT) > 0;  // halt if hdr states so
          }
        } else {
          // was an error decompressing the file.
          win_printf(main_win, "...Error decompressing file...\n");
          halt = (ldr_hdr->flags & LDR_HDR_FLAGS_HALT) > 0;  // halt if hdr states so
        }
        b = TRUE;
      }
    }
    if (!b)
      win_printf(main_win, "\n  There was an error loading the file\n");
    
    if (halt) {
      win_printf(main_win, "...halting...\n\n");
      freeze();
    }
    
    i++; // move to next file string
  }
  mfree(buffer);
  
  // update the title bar
  win_title_update(main_win, "FYSOS v2.0 Loader");
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Kernel file is loaded to 'kernel_base'
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if (kernel_base == 0) {
    void *panic = win_create(main_win, "Panic", "", 0, -6, 38, 4, WIN_HAS_TITLE | WIN_CNTR_TITLE | WIN_HAS_SHADDOW | WIN_IS_DIALOG | WIN_CENTER);
    win_printf(panic, " Did not load a valid kernel file.\n              Halting...\n");
    win_status_clear(main_win);
    freeze();
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if (spc_key_F2)
    para_printf("Updating the BIOS Floppy Parameter Block.\n");
  sys_block.org_int1e = update_int1e(&sys_block.floppy_1e);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Reset the diskette (floppy) drive so it loads the new int 1Eh table values
  regs.eax = 0x00000000;
  regs.edx = 0x00000000;
  intx(0x13, &regs);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Get the bios equipment list 16-bit word value
  // Get the keyboard status bits at 0040:0017
  get_bios_equ_list(&sys_block.bios_equip, &sys_block.kbd_bits);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // We run through all of the disk drives with a BIOS drive number of 80h
  //  or higher and store the (extended) disk parameters.
  get_drv_parameters(sys_block.drive_params);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // one of the last things we need to do before we jump,
  // is get the time from the bios.
  get_bios_time(&sys_block.time);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // see if we have and APM BIOS
  apm_bios_check(&sys_block.apm);
  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// clear the keyboard (using a safety catch so we don't just loop endlessly
//  if there is something wrong with the keyboard
  i = 16;
  while (i-- && kbhit())
    getscancode();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //  Now get the vesa video info from the BIOS.
  win_status_update(main_win, " Getting Video Information...", FALSE);
  //get_video_eedid();  // not implemented yet
  if (spc_key_F1) help_screen();  // check to see if the help screen has been requested
  sys_block.vid_mode_cnt = get_video_info(sys_block.mode_info);
  if (sys_block.vid_mode_cnt > 0) {
    // if we haven't selected to stay in text mode, ask for a mode to change to
    if (!spc_key_F9) {
      sys_block.cur_vid_index = get_video_mode(sys_block.mode_info, sys_block.vid_mode_cnt, 1024, 768, 16);
      if (sys_block.cur_vid_index == 0xFFFF)
        sys_block.cur_vid_index = get_video_mode(sys_block.mode_info, sys_block.vid_mode_cnt, 1024, 768, 15);
      if (sys_block.cur_vid_index == 0xFFFF)
        sys_block.cur_vid_index = get_video_mode(sys_block.mode_info, sys_block.vid_mode_cnt, 800, 600, 16);
      // if we didn't find a suitable mode, or the user has pressed the F8 key,
      // let the user select from a list.
      if ((sys_block.cur_vid_index == 0xFFFF) || spc_key_F8) {
        // print the available modes and ask for a selection of one of them.
        // if there are more modes than will fit on the screen, an 'enter' key
        //  will scroll to the next page. The ESC key will start the list over
#define LIST_HEIGHT  10
#define LIST_WIDTH   55
        allow_spc_keys = FALSE;
        win_status_update(main_win, "Please choose a screen mode", FALSE);
        void *list = win_create(main_win, "Video mode list", "", ((80 - LIST_WIDTH) / 2) - 1, 5, LIST_WIDTH, LIST_HEIGHT + 2,
          WIN_HAS_TITLE | WIN_HAS_STATUS | WIN_HAS_BORDER | WIN_HAS_SHADDOW | WIN_IS_DIALOG);
        int i = 0, j, k, avail[LIST_HEIGHT];
        bit16u ch;
next_entry:
        for (j=0; i<sys_block.vid_mode_cnt && j<LIST_HEIGHT; i++) {
          win_printf(list, " %c: %4i x %4i %2i bits per pixel\n", j + 'A', 
            sys_block.mode_info[i].xres, sys_block.mode_info[i].yres, sys_block.mode_info[i].bits_per_pixel);
          avail[j] = i;
          j++;
        }
        k = j;
        while (k++ < LIST_HEIGHT)
          win_printf(list, "\n");
        do {
          sprintf(temp_buf, "Select a video mode to use [A-%c], %s", j + 'A' - 1, (i < sys_block.vid_mode_cnt) ? "Enter=more" : "Esc=start over");
          win_status_update(list, temp_buf, FALSE);
          ch = getscancode();
          if (ch == 0x011B)  { // esc (start over)
            i = 0;
            goto next_entry;
          }
          if ((ch == 0x1C0D) && (i < sys_block.vid_mode_cnt))
            goto next_entry;
          if (ch == 0x4300) {  // F9 key
            spc_key_F9 = TRUE;
            ch = 0;
            break;
          }
          ch = toupper(ch & 0xFF) - 'A';
        } while ((ch < 0) || (ch >= j));
        sys_block.cur_vid_index = avail[ch];
        win_destroy(list);
        //win_printf(main_win, "You chose: %i\n", sys_block.cur_vid_index);
        allow_spc_keys = TRUE;
      }
    }
  } else {
    if (!spc_key_F9) {
      void *vesa_alert = win_create(main_win, "Video Screen Alert", "Press a key to continue", ((80 - 42) / 2) - 1, 5, 40, 8,
        WIN_HAS_TITLE | WIN_HAS_STATUS | WIN_HAS_BORDER | WIN_HAS_SHADDOW | WIN_IS_DIALOG);
      win_printf(vesa_alert, "\n"
                             "FYSOS recommends a VESA capable video\n"
                             "card with a Linear Base Frame Buffer.\n"
                             "\n"
                             "Setting Text Only...\n");
      getscancode();
      spc_key_F9 = TRUE;
      win_destroy(vesa_alert);
      win_status_update(main_win, " Setting Text Only Mode", FALSE);
    }
  }
  if (spc_key_F1) help_screen();  // check to see if the help screen has been requested
  
  if (spc_key_F2) {
    for (i=0; i<sys_block.vid_mode_cnt; i++) {
      para_printf("%c%2i: %ix%i %i %i 0x%08X\n", 
        (i == sys_block.cur_vid_index) ? '*' : ' ',
        i,
        sys_block.mode_info[i].xres, sys_block.mode_info[i].yres, 
        sys_block.mode_info[i].bits_per_pixel, sys_block.mode_info[i].memory_model,
        sys_block.mode_info[i].lfb);
    }
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // move the 1400h byte block of data to just before the kernel
  // kernel base should be on a meg boundary and since is a PE file,
  //  will have the first 0x400 bytes free for our use.  Therefore,
  //  we back up 0x1000 from the base and this is how we have 0x1400
  //  bytes of space for use.
  if (spc_key_F2)
    para_printf("Moving Data Block to 0x%08X.\n", kernel_base - 0x1000);
  sys_block.text_only = spc_key_F9;
  memcpy((void *) (kernel_base - 0x1000), &sys_block, 0x1400);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // move the actual GDT entries (first 3)
  // and clear out the remaining entries
  if (spc_key_F2)
    para_printf("Building/Moving GDT's to 0x%08X.\n", sys_block.gdtoffa);
  memcpy((void *) sys_block.gdtoffa, act_gdt, (3 * 8));
  memset((void *) (sys_block.gdtoffa + (3 * 8)), 0, ((256 - 3) * 8));
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // make sure the floppy disk drive motor is turned off.
  //  put a 1 at 0x00440 so that the next time the BIOS
  //  decrements it, it will turn off the motor.
  floppy_off();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Change to that screen mode (as long as F9 was not pressed)
  if (!spc_key_F9) {
    regs.eax = 0x00004F02;
    regs.ebx = vid_modes[sys_block.cur_vid_index] | (1<<14);
    intx(0x10, &regs);
    
    // if it is an 8-bit mode, we have to set the VGA palette
    //  we currently assume a VGA compatible
    if (sys_block.mode_info[sys_block.cur_vid_index].bits_per_pixel == 8)
      vid_set_256_palette(TRUE);
  } else {
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // we are already at screen mode 03, so don't "set" the screen mode
    //  since it will pause the display, making a noticable flash.
    // however, we do need to clear the screen turning the "cursor" off.
    // we use the "scroll screen" BIOS function to clear the screen.
    regs.eax = 0x00000600;  // ah = 6 = scroll up, al = 0 = entire window
    regs.ebx = 0x00000000;  // attribute
    regs.ecx = 0x00000000;  // row/column of upper-left corner
    regs.edx = 0x00001950;  // row/column of lower-right corner
    intx(0x10, &regs);
    // can't write anyting now, since the attribute is 0
  }
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // turn off the hardware cursor.
  regs.eax = 0x00000103;
  regs.ecx = 0x00002000;
  intx(0x10, &regs);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // from this point on, we must use the assembler instead of the compiler.
  // therefore, 'jump' to the finish code
  if (spc_key_F2)
    para_printf("Moving to PMODE.\n");
  
  finish();
  
} // end of _main()
