
#include "ctype.h"
#include "loader.h"

#include "paraport.h"
#include "disks.h"
#include "malloc.h"
#include "string.h"
#include "stdio.h"
#include "sys.h"
#include "windows.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// retrieve and store a little information about the disk
struct S_DISK_DATA disk_data;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// check to see if the BIOS supports large disks (hard drives)
bool large_disk = FALSE;
bit8u edd_version = 0;  // extended disk services version (01h = 1.0, 20h = 2.0 (edd v1.0), 21h = 2.1 (edd v1.1), 30h = edd v3.0)

bool large_disk_support(const int drv, bool *ret_flag) {
  struct REGS regs;
  bool ret = FALSE;
  
  if (spc_key_F2)
    para_printf("Checking for Large Disk support: ");
  
  // no need to call bios if not a hard drive
  if (drv >= 0x80) {
    regs.eax = 0x00004100;
    regs.ebx = 0x000055AA;
    regs.edx = drv;
    if (!intx(0x13, &regs) &&
         ((regs.ebx & 0x0000FFFF) == 0xAA55) &&
         (regs.ecx & (1<<0))) {
      edd_version = ((regs.eax & 0x0000FF00) >> 8);
      switch (edd_version) {
        case 0x01:
          edd_version = 0x10;
        case 0x20:
        case 0x21:
        case 0x30:
          if (spc_key_F2)
            para_printf("Found Extended Read Services version %i.%i\n", edd_version >> 4, edd_version & 0xF);
          if (ret_flag) *ret_flag = TRUE;
          return TRUE;
      }
    }
  }
  
  // else we must get a little information about the disk
  regs.eax = 0x00000800;
  regs.edx = drv;
  regs.edi = 0;
  regs.es = 0;
  if (!intx(0x13, &regs)) {
    disk_data.sec_per_track = regs.ecx & 0x3F;
    disk_data.num_heads = ((regs.edx >> 8) & 0xFF) + 1;
    ret = TRUE;
  }
  
  if (spc_key_F2)
    para_printf("Not supported.\n");
  
  if (ret && spc_key_F2)
    para_printf("Found %i sectors per track on %i heads.\n", disk_data.sec_per_track, disk_data.num_heads);
  
  if (ret_flag) *ret_flag = FALSE;
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This routine converts LBA to CHS
// Sector   = (LBA mod SPT) + 1
// Head     = (LBA  /  SPT) mod Heads
// Cylinder = (LBA  /  SPT)  /  Heads
//    (SPT = Sectors per Track)
void lba_to_chs(const bit32u lba, unsigned int *cyl, unsigned int *head, unsigned int *sect) {
  *sect = (lba % disk_data.sec_per_track) + 1;
  *head = (lba / disk_data.sec_per_track) % disk_data.num_heads;
  *cyl  = (lba / disk_data.sec_per_track) / disk_data.num_heads;
}

// some BIOSes need a short delay before/between calls for USB emulation
void delay(void) {
  bit32u *p = (bit16u *) 0x0000046C;
  bit32u c = *p;
  while (*p == c)
    ;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// This routine reads in 'cnt' sectors using the bios interrupt 13h.
//   Either the "short" form, using CHS or the "long" form, using
//   the BIOS Extentions service
// On entry:
//  lba = starting sector in LBA format (relative to volume)
//         (the code below adds the base LBA of the volume)
//  cnt = count of sectors to read
// On return:
//  count of sectors read
//
// (As of now, the compiler won't allow 64-bit numbers, so we assume
//  all LBA's are than 32-bit values)
// since the BIOS needs a buffer at <= 0x0000FFFF in address (offset)
//  we must make one, then copy it after the read.
// TODO: Pass a 64-bit LBA
int read_sectors(bit32u lba, int cnt, void *buffer) {
  struct S_READ_PACKET long_packet;
  struct REGS regs;
  int ret = 0, t;
  int count;
  bit8u *p = (bit8u *) buffer;
  bit8u *local = malloc(512 * 0x7F);
  bool b;
  
  if (spc_key_F2)
    para_printf("Reading %i sector(s) from LBA %i\n", count, lba);
  
  if (large_disk) {
    while (cnt) {
      // Some BIOSes using USB and emulation, fail on some reads, but
      //  a second (or third) read succeeds.
      count = (cnt > 0x7F) ? 0x7F : cnt;
      for (t=0; t<3; t++) {
        long_packet.size = sizeof(struct S_READ_PACKET);
        long_packet.resv = 0;
        long_packet.cnt = count;
        long_packet.buffer = (MK_SEG((bit32u) local) << 16) | MK_OFF((bit32u) local);
        long_packet.lba[0] = lba;
        long_packet.lba[1] = 0;
        add64(long_packet.lba, sys_block.boot_data.base_lba);
        regs.eax = 0x00004200;
        regs.edx = sys_block.boot_data.drive;
        regs.esi = MK_OFF((bit32u) &long_packet);
        regs.ds = MK_SEG((bit32u) &long_packet);
        delay();
        b = intx(0x13, &regs);
        if (!b) break;
      }
      // if we tried 3 times and failed, exit
      if (b)
        break;
      
      memcpy(p, local, count * 512);
      p += (count * 512);
      
      ret += count;
      cnt -= count;
      lba += count;
    }
  } else {
    /* It is unknown if the BIOS can span heads and cylinders, so we just read all (remaining) sectors
     * from the current track.  For example, if we start on sector 5, and there are 18 spt, we read
     * sectors 5 through 18 as long as count requires that many.  This is faster than reading a single
     *  sector at a time.
     */
    unsigned int cyl, head, sect, scnt;
    lba += sys_block.boot_data.base_lba[0];  // No need to 64-bit this.  Can't get a legal CHS value from a 64-bit LBA
    while (cnt) {
      lba_to_chs(lba, &cyl, &head, &sect);
      //scnt = (disk_data.sec_per_track - sect) + 1;
      //if (scnt > count)
      //  scnt = count;
      scnt = 1;
      regs.eax = 0x00000200 | scnt;
      regs.ecx = ((cyl & 0xFF) << 8) | ((cyl >> 8) & 3) | (sect & 0x3F);
      regs.edx = ((head & 0x0F) << 8) | sys_block.boot_data.drive;
      regs.ebx = MK_OFF((bit32u) local);
      regs.es = MK_SEG((bit32u) local);
      if (intx(0x13, &regs))
        break;
      
      memcpy(p, local, scnt * 512);
      p += (scnt * 512);
      
      cnt -= scnt;
      ret += scnt;
      lba += scnt;
    }
  }
  mfree(local);
  
  return ret;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// We need to move the current INT 1Eh table from ROM 
//  to RAM and set the Max SPT to 36.  Many BIOS's default
//  disk parameter tables will not recognize multi-sector
//  reads beyond the maximum sector number specified in
//  the default diskette parameter tables.  This may
//  mean 7 sectors in some cases.
// We need to also save the original INT 1Eh table
//  address so that we can restore it on (warm) reboot.
bit32u update_int1e(struct S_FLOPPY1E *targ) {
  bit32u org_1e = * (bit32u *) (0x1E * 4); // segmented address
  bit8u *s = (bit8u *) MK_FP(&org_1e);
  struct S_FLOPPY1E *new_1e = (struct S_FLOPPY1E *) 0x00522;
  
  // move from ROM to RAM
  // we choose 0x00000522 (0000:0522)
  memcpy(new_1e, s, SIZEOF_S_FLOPPY1E);
  new_1e->spt = 36; // set max spt to 36 (the most a 2.88 would have)
  
  // also copy to our loader buffer (unmodified)
  memcpy(targ, s, SIZEOF_S_FLOPPY1E);
  
  return org_1e;
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
  struct S_BIOS_DRV_PARAMS bios_params;
  
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
      if (spc_key_F2)
        para_printf("Getting drive parameters for drive 0x%02X.\n", cur_drive);
      // start to store some information
      // store the drive number
      p->drv_num = (bit8u) cur_drive;
      
      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      // check to see if the BIOS supports large disks for this drive
      if (large_disk_support(cur_drive, NULL)) {
        // flag that we did use extended parameters
        p->extended_info = TRUE;
        
        // get the extended parameters.
        regs.eax = 0x00004800;
        regs.edx = cur_drive;
        regs.esi = MK_OFF((bit32u) &bios_params);
        regs.ds = MK_SEG((bit32u) &bios_params);
        bios_params.ret_size = 0x0042;
        bios_params.info_flags = 0; // some DELL BIOS' error if this isn't zero
        if (!intx(0x13, &regs)) {
          // now copy from the local buffer to our passed buffer
          memcpy(&p->bios_params, &bios_params, sizeof(struct S_BIOS_DRV_PARAMS));
          // since the parameters at EDD_config_ptr may be at a temp buffer,
          //  we need to copy them now
          if ((bios_params.ret_size >= 0x1E) && (bios_params.EDD_config_ptr != 0xFFFFFFFF)) {
            void *dpte = MK_FP(&bios_params.EDD_config_ptr);
            memcpy(p->dpte, dpte, 16);
          }
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

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// make sure the floppy disk drive motor is turned off.
//  put a 1 at 0x00440 so that the next time the BIOS
//  decrements it, it will turn of the motor.
void floppy_off(void) {
  bit8u *running = (bit8u *) 0x0043F;
  bit8u *status = (bit8u *) 0x00440;
  
  if (spc_key_F2)
    para_printf("Ensuring that the floppy motors are off.\n");
  
  // first check to see that [0x0043F] > 0, if not, don't
  //  do this check code.  It seems that the Compaq BIOS 
  //  does not do the "decrement" if the drive is not turning.
  //  It may only hook the timer tick interrupt when the drive
  //  is turning, and when it counts to 0, it unhooks itself.
  if (*running) {
    // whatever it has in it, put a 1 in it to speed up the process
    *status = 1;
    
    // we need to wait for this to actually happen since
    //  the later 'cli' won't allow the timer tick interrupt to fire
    // make sure the interrupt flag is set or we will wait forever
    asm (
      "  pushfd \n"
      "  sti    \n"
    );
    
    // TODO: May want to do a timeout or something
    while (*status)
      ;
    
    asm (
      "  popf  \n"
    );
  }
}
