/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
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
 *
 *  compile using gcc (DJGPP)
 *   gcc -Os hdc_type.cpp -o hdc_type.exe -s
 *
 *  usage:
 *    hdc_type [parameters]
 *
 *  parameters:
 *    -v         indicates verbose output
 *    -ide       if SATA found with multiple mode support, use Legacy IDE mode
 *    -ahci      if SATA found with multiple mode support, use AHCI mode
 *    -do_isa    detect controllers on the ISA bus too
 *    -pio_only  do pio only I/O
 *
 *  Notes:
 *   - This code is mainly for the Intel PIIX and ICHx controllers.
 *
 *   IDE:
 *     - This code assumes all I/O access is via Port I/O, not memory mapped I/O
 *        addresses.  When reading the PCI BARs, be sure to check bit 0.  This
 *        code will check for, but skip any memory mapped controllers.
 *        Read Chapter 2 for more information about memory mapped controllers.
 *   AHCI:
 *     - This code assumes all memory mapped I/O addresses.
 *
 */

#include <ctype.h>
#include <conio.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MDELAY(x)   mdelay(x)  // use our mS delay
//#define MDELAY(x) delay(x)  // use DJGPP's mS delay

#include <crt0.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>

#include "..\include\ctype.h"
#include "..\include\timer.h"
#include "..\include\pci.h"
#include "..\include\misc.h"
#include "..\include\common.h"

#include "sata_type.h"
#include "hdc_type.h"

// lock all memory, to prevent it being swapped or paged out
int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY;
bool verbose = FALSE;
bool do_isa = FALSE;
bool pio_only = FALSE;
int  sata_mode = -1;

struct S_ATA_CNTRLR s_hdc[CNTRLR_COUNT] = {
  { 0, 0x01F0, ATA_ADPT_CNTRL1, 0x0000, 0, 14, CNTRL_TYPE_UNKNOWN, 0, FALSE, },
  { 0, 0x0170, ATA_ADPT_CNTRL2, 0x0000, 0, 15, CNTRL_TYPE_UNKNOWN, 0, FALSE, },
//{ 0, 0x01E8, 0x3ED,           0x0000, 0, 11, CNTRL_TYPE_UNKNOWN, 0, FALSE, },  // uses irq 11 (alternate 12 or 9)
//{ 0, 0x0168, 0x36D,           0x0000, 0, 10, CNTRL_TYPE_UNKNOWN, 0, FALSE, },  // uses irq 10 (alternate 12 or 9)
//{ 0, 0x01E0, 0x3ED,           0x0000, 0,  8, CNTRL_TYPE_UNKNOWN, 0, FALSE, },  // uses irq 8
//{ 0, 0x0160, 0x36D,           0x0000, 0, 12, CNTRL_TYPE_UNKNOWN, 0, FALSE, },  // uses irq 12
};

int main(int argc, char *argv[]) {
  int i, j;
  bit32u type;
  bit8u pci_bus = 0, pci_dev = 0, pci_func = 0;
  
  struct S_ATA_CNTRLR hdc;
  char str[33];
  
  // parse the command line parameters
  if (!get_parameters(argc, argv))
    return -1;
  
  // check for the RDTSC instruction, and calculate a timer
  if (!setup_timer()) {
    printf("\n Sorry.  This utility requires a 486+ with the RDTSC instruction.");
    return -3;
  }
  
  // first detect all PCI(e) controllers
  while (1) {
    // find the base address of the controller
    type = PCI_CLASS_ATA;
    if (!pci_get_base(&pci_bus, &pci_dev, &pci_func, &type))
      break;
    
    if (verbose) {
      printf(" Found PCI ATA device:\n");
      printf("  * Bus = %i, device = %i, function = %i, type = 0x%06X\n", pci_bus, pci_dev, pci_func, type);
      printf("  * Vendor = 0x%04X  Device = 0x%04X  rev = 0x%02X\n",
        read_pci(pci_bus, pci_dev, pci_func, 0, sizeof(bit16u)),
        read_pci(pci_bus, pci_dev, pci_func, 2, sizeof(bit16u)),
        read_pci(pci_bus, pci_dev, pci_func, 8, sizeof(bit8u)));
    }
    
    // make sure the device is powered
    if (!pci_set_D0_state(pci_bus, pci_dev, pci_func))
      break;
    
    // get type of controller
    hdc.type = ata_controller_type(read_pci(pci_bus, pci_dev, pci_func, 0, sizeof(bit16u)),
                                   read_pci(pci_bus, pci_dev, pci_func, 2, sizeof(bit16u)), str);
    
    switch ((type & 0x0000FF00) >> 8) {
      case PCI_ATA_SUB_SCSI:  //   SCSI
        
        break;
      case PCI_ATA_SUB_IDE:   //   IDE 
        ata_ide_detect(&hdc, pci_bus, pci_dev, pci_func, str);
        break;
      case PCI_ATA_SUB_FDC:   //   FDC
        
        break;
      case PCI_ATA_SUB_IPI:   //   IPI
        
        break;
      case PCI_ATA_SUB_RAID:  //   RAID
        
        break;
      case PCI_ATA_SUB_ATA:   //   ATA controller w/single DMA (20h) w/chained DMA (30h)
        
        break;
      case PCI_ATA_SUB_SATA:   //   Serial ATA
        ata_sata_detect(&hdc, pci_bus, pci_dev, pci_func, str);
        break;
      case PCI_ATA_SUB_SAS:    //   Serial Attached SCSI
        
        break;
      case PCI_ATA_SUB_SSS:    //   Solid State Storage
        
        break;
      case PCI_ATA_SUB_OTHER: //   Other
        // try to see if it is a standard IDE type
        // One controller I have does this
        ata_ide_detect(&hdc, pci_bus, pci_dev, pci_func, str);
        break;
      default:
        ;
    }
    
    // if bit 7 of the header type (of the first function of the device) is set,
    //  then this is a multi-function device.
    //  else, skip checking the rest of the functions.
    if (pci_func == 0)
      if ((read_pci(pci_bus, pci_dev, pci_func, 0x0E, sizeof(bit8u)) & 0x80) == 0)
        pci_func = PCI_MAX_FUNC;
    
    // increment to next function for next loop
    pci_func++;
  }
  
  // ISA controllers
  if (do_isa) {
    for (j=0; j<CNTRLR_COUNT; j++) {
      if (verbose) printf("\nDetecting ATA Disk Controller at 0x%04X\n", s_hdc[j].base);
      if (s_hdc[j].is_pci_dev) {
        if (verbose) printf("Found this controller as PCI device.  Skipping...\n");
      } else {
        if (det_ata_controller(&s_hdc[j])) {
          printf("\n*** Found ATA Disk Controller at 0x%04X (0x%04X) (irq = %i)", s_hdc[j].base, s_hdc[j].alt_base, s_hdc[j].irq);
          
          for (i=0; i<2; i++) {
            s_hdc[j].drive[i].cntrlr = &s_hdc[j];
            s_hdc[j].drive[i].drv = i;
            if (verbose) printf("\nDetecting ATA Disk at index %i\n", i);
            if (det_ata_drive(&s_hdc[j].drive[i])) {
              printf("*** Found Drive: '%s' 0x%03X  drv: %i\n",
                s_hdc[j].drive[i].info.model_num, 
                s_hdc[j].drive[i].cntrlr->base,
                s_hdc[j].drive[i].drv
              );
            }
          }
        }
      }
    }
  }
  
  return 0;
}

bool ata_ide_detect(struct S_ATA_CNTRLR *cntrlr, const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func, const char *str) {
  int i, channel, p_interface;
  bit16u hdc_base, hdc_altbase, bus_master;
  bit32u type;
  
  // If bit 1 is set, bit 0 can be set or cleared.
  // If bit 0 is clear, this controller is in Compatibility mode and must use the legacy
  //  port values of 0x1F0, and the BAR's may be zeros. (The PIIX3 assumes this)
  // If bit 0 is set, this controller is in Native mode and uses the port values in
  //  the BAR's.
  type = read_pci(pci_bus, pci_dev, pci_func, (2<<2), sizeof(bit32u)) >> 8;
  p_interface = (type & 0xFF);
  hdc_base = 0x1F0;  // assume primary base (for the first controller)
  hdc_altbase = ATA_ADPT_CNTRL1;
  for (channel=ATA_CHANNEL_PRIMARY; channel<=ATA_CHANNEL_SECONDARY; channel++) {
    if ((cntrlr->type >= CNTRL_TYPE_STD_SATA) || (p_interface & (1 << (channel * 2)))) {
      // native mode
      cntrlr->mode = HDC_MODE_NATIVE;
      if (channel == ATA_CHANNEL_PRIMARY) {
        hdc_base = read_pci(pci_bus, pci_dev, pci_func, 0x10, sizeof(bit16u));
        hdc_altbase = read_pci(pci_bus, pci_dev, pci_func, 0x14, sizeof(bit16u));
      } else {
        hdc_base = read_pci(pci_bus, pci_dev, pci_func, 0x18, sizeof(bit16u));
        hdc_altbase = read_pci(pci_bus, pci_dev, pci_func, 0x1C, sizeof(bit16u));
      }
      if (!(hdc_base & 1) || !(hdc_altbase & 1)) {
        printf("One or more of the controller's BAR registers use memory mapped I/O...\n");
        goto next_ide_controller;
      } else {
        hdc_base &= 0xFFFC;
        hdc_altbase &= 0xFFFC;
      }
    } else
      // Legacy Mode
      cntrlr->mode = HDC_MODE_LEGACY;
    
    if ((cntrlr->type >= CNTRL_TYPE_STD_SATA) || (p_interface & 0x80)) {
      bus_master = read_pci(pci_bus, pci_dev, pci_func, 0x20, sizeof(bit16u));
      if (!(bus_master & 1)) {
        printf("Bus Master BAR register uses memory mapped I/O...\n");
        goto next_ide_controller;
      } else
        bus_master &= 0xFFFC;
      
      // I/O access enable and bus master enable
      write_pci(pci_bus, pci_dev, pci_func, 0x04, sizeof(bit16u), (1 << 2) | (1 << 0));
      
      switch (cntrlr->type) {
        case CNTRL_TYPE_PIIX:
        case CNTRL_TYPE_PIIX3:
        case CNTRL_TYPE_PIIX4:
        case CNTRL_TYPE_ICH5:
        case CNTRL_TYPE_ICH6:
          // Make sure the PCI Latency Timer is at least 48 and at most 64 (in increments of 8).
          // On the PIIX/ICH, the MIN_LAT and MAX_LAT fields are reserved, so we simply set it to 64.
          write_pci(pci_bus, pci_dev, pci_func, 0x0D, sizeof(bit8u), 64);
          
          /********************************************************************\
          *  set the timing of the PCI transfers
          *   (when in legacy mode, we must set bit 15)
          * You would actually need to call a function and calcuate the timing
          *  of the current device, then set it here.  There are way too many
          *  different drives to show what timings to set, hence most OS's
          *  have a different driver for just about any device.
          * Therefore, see Appendix F of the book this code came with and you can
          *  get an idea of what to set it too.
          * For now, we set the timings to some default/well-known values.
          \********************************************************************/
          write_pci(pci_bus, pci_dev, pci_func, 0x40, sizeof(bit16u), 0xA344);
          write_pci(pci_bus, pci_dev, pci_func, 0x42, sizeof(bit16u), 0xA344);
          
          // TODO: What is this one?
          //write_pci(pci_bus, pci_dev, pci_func, 0x48, sizeof(bit8u), 0x00);
          
          // TODO:
          //write_pci(pci_bus, pci_dev, pci_func, 0x54, sizeof(bit32u), 
          //  read_pci(pci_bus, pci_dev, pci_func, 0x54, sizeof(bit32u)) | 0x0400);
          break;
        case CNTRL_TYPE_0571:
          // Offset 48->4B - Drive Timing Control (R/W)
          // The following fields define the Active Pulse Width and Recovery Time for the IDE DIOR# and DIOW# signals:
          //  31-28 Primary Drive 0 Active Pulse Width........ def=1010b
          //  27-24 Primary Drive 0 Recovery Time............. def=1000b
          //  23-20 Primary Drive 1 Active Pulse Width........ def=1010b
          //  19-16 Primary Drive 1 Recovery Time............. def=1000b
          //  15-12 Secondary Drive 0 Active Pulse Width ..... def=1010b
          //  11- 8 Secondary Drive 0 Recovery Time .......... def=1000b
          //   7- 4 Secondary Drive 1 Active Pulse Width ..... def=1010b
          //   3- 0 Secondary Drive 1 Recovery Time .......... def=1000b
          // The actual value for each field is the encoded value in the field plus one and indicates the number of PCI clocks.
          break;
        case CNTRL_TYPE_SATA_SIL:
          // Silicon Image, Inc (3512)
          break;
        default:
          // Unknown if this is the same port and timing
          ;
      }
    } else {
      write_pci(pci_bus, pci_dev, pci_func, 0x04, sizeof(bit16u), 0x0001);      // I/O access enable and bus master disable
      bus_master = 0x0000;
    }
    
    // get the IRQ number
    switch (cntrlr->type) {
      // The PIIX3 specs say that the PCI config space from offset 0x24 to 0x3f 
      // is "reserved", so there is no PCI IRQ to be set. The PIIX and PIIX3 both 
      // have 2 IDE channels and they use IRQ 14 and 15.
      case CNTRL_TYPE_PIIX:
      case CNTRL_TYPE_PIIX3:
      case CNTRL_TYPE_PIIX4:
      case CNTRL_TYPE_SATA_SIL:
        cntrlr->irq = (channel == ATA_CHANNEL_PRIMARY) ? 14 : 15;
        break;
      case CNTRL_TYPE_ICH5:
      case CNTRL_TYPE_ICH6:
      case CNTRL_TYPE_0571:
      default:
        // In Legacy (non-Native) mode, 14 and 15 are used no matter
        //  the IRQ field in the PCI config space.
        // In Native mode, the PCI config space value is used.
        if (cntrlr->mode == HDC_MODE_NATIVE) {
          // if in Native Mode, we need to write the IRQ to the PCI Config Space.
          cntrlr->irq = 14; // this is were you would choose an available IRQ number from your kernel code
          write_pci(pci_bus, pci_dev, pci_func, 0x3C, sizeof(bit8u), cntrlr->irq);
        } else
          cntrlr->irq = (channel == ATA_CHANNEL_PRIMARY) ? 14 : 15;
    }
    
    printf("\n*** Found ATA Disk Controller (%s) at %04Xh (%04Xh) (%04Xh) (irq %i)", 
      str, hdc_base, hdc_altbase, bus_master, cntrlr->irq);
    if (!verbose) puts("");
    cntrlr->base = hdc_base;
    cntrlr->alt_base = hdc_altbase;
    cntrlr->bus_master = bus_master;
    cntrlr->channel = channel;
    
    // mark this as a PCI device
    cntrlr->is_pci_dev = TRUE;
    cntrlr->pci_dev.bus = pci_bus;
    cntrlr->pci_dev.dev = pci_dev;
    cntrlr->pci_dev.func = pci_func;
    
    // mark this device in the main list so that we
    //  don't try to detect it again later.
    for (i=0; i<CNTRLR_COUNT; i++)
      if (s_hdc[i].base == cntrlr->base)
        s_hdc[i].is_pci_dev = TRUE;
    
    for (i=0; i<2; i++) {
      cntrlr->drive[i].cntrlr = cntrlr;
      cntrlr->drive[i].drv = i;
      if (verbose) printf("\nDetecting ATA Disk at index %i\n", i);
      if (det_ata_drive(&cntrlr->drive[i])) {
        printf("*** Found Drive: '%s' 0x%03X  drv: %i\n",
          cntrlr->drive[i].info.model_num, 
          cntrlr->drive[i].cntrlr->base,
          cntrlr->drive[i].drv
        );
      }
    }
next_ide_controller:
    hdc_base = 0x170;  // set for assumption of second controller.
    hdc_altbase = ATA_ADPT_CNTRL2;
  }
  
  return TRUE;
}

// return controller type, via the vendor and device id of the PCI configuration space.
int ata_controller_type(const bit16u vendor_id, const bit16u device_id, char *str) {
  int type = CNTRL_TYPE_UNKNOWN;
  
  switch (vendor_id) {
    case 0x1095:  // Silicon Image, Inc
      switch (device_id) {
        case 0x3512:
          type = CNTRL_TYPE_SATA_SIL;
          strcpy(str, "SATA Silicon Image 3512");
          break;
        default:
          ;
      }
      break;
    case 0x1106:  // Via Tech
      switch (device_id) {
        case 0x0571:  // Bus Master IDE Controller
          type = CNTRL_TYPE_0571;
          strcpy(str, "0571");
          break;
        default:
          ;
      }
      break;
    case 0x8086:  // Intel
      switch (device_id) {
        case 0x1230:
          type = CNTRL_TYPE_PIIX;
          strcpy(str, "PIIX");
          break;
        case 0x2411:
          type = CNTRL_TYPE_ICH;
          strcpy(str, "ICH");
          break;
        case 0x2421:
          type = CNTRL_TYPE_ICH0;
          strcpy(str, "ICH0");
          break;
        case 0x244A:
        case 0x244B:
          type = CNTRL_TYPE_ICH2;
          strcpy(str, "ICH2");
          break;
        case 0x24DB:  // EIDE Controller
          type = CNTRL_TYPE_ICH5;
          strcpy(str, "ICH5");
          break;
        case 0x266F:  // PATA100 Controller - 266F
          type = CNTRL_TYPE_ICH6;
          strcpy(str, "ICH6");
          break;
        case 0x248A:
        case 0x248B:
          type = CNTRL_TYPE_ICH3;
          strcpy(str, "ICH3");
          break;
        case 0x24CA:
        case 0x24CB:
          type = CNTRL_TYPE_ICH4;
          strcpy(str, "ICH4");
          break;
        case 0x2922:
          type = CNTRL_TYPE_ICH9R;
          strcpy(str, "ICH9R");
          break;
        case 0x7010:  // PIIX3 IDE Interface (Triton II)
          type = CNTRL_TYPE_PIIX3;
          strcpy(str, "PIIX3");
          break;
        case 0x7111:
          type = CNTRL_TYPE_PIIX4;
          strcpy(str, "PIIX4");
          break;
        default:
          ;
      }
      break;
    default:
      ;
  }
  
  return type;
}

// Some controllers don't like us to write values to the registers until after
//  we have done a specific sequence of writes.  We do this in the detect_ata()
//  function where we detect a drive.   Therefore, let's just return TRUE here
//  if we read anything other than 0xFF from the status register.
bool det_ata_controller(struct S_ATA_CNTRLR *cntrlr) {
  
  if (inportb(cntrlr->base + ATA_STATUS) == 0xFF) {
    if (verbose) printf("ATA_STATUS returned 0xFF.  No controller attached.\n");
    return FALSE;
  }
  
  return TRUE;
}

/* Select a drive on the given base
 * Before you can read the status, and then send all the other values to the other IO ports,
 *  many drives require a little time to respond to a "select", and push their status onto 
 *  the bus. The suggestion is to read the Status register four times before reading it for
 *  the fifth time to see if the controller is ready for the command.
 * The controller wants about 400nS after selecting the drive, and reading from the same I/O
 *  port numerous times takes about 100nS each time.  Therefore, if you read it four times,
 *  the fifth time (usually done after calling this function and just before sending a command),
 *  will be at least 400nS later.  This allows the drive time to push the correct voltages onto 
 *  the bus. 
 * However, reading IO ports to create delays wastes a lot of CPU cycles. So, it is actually 
 *  better to have your driver remember the last value sent to each Drive Select IO port, to avoid 
 *  doing unneeded drive selections, if the value did not change.  This way, if you do not need
 *  to select the drive due to that drive already selected, you can just return, saving about
 *  500nS of time.
 */
bit8u cur_selected = 0xFF;  // save the current selected device so we don't have to select it each time.
bool ata_select_drv(const bit16u base, const bit8u drv, const bit8u flags, const bit8u lba24_head) {
  bit8u select = (flags & ATA_DH_ISLBA) ? ((ATA_DH_SET_BITS | ATA_DH_ISLBA | (drv << 4) | (lba24_head & 0x0F)))
                                        : ((ATA_DH_SET_BITS |    0         | (drv << 4) | (lba24_head & 0x0F)));
  
  // if the drive is already selected with these parameters,
  //  no need to do it again, so just return
  if (select == cur_selected)
    return TRUE;
  
  if (inportb(base + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) {
    if (verbose) printf("0: Could not select drive...(%02X)\n", inportb(base + ATA_STATUS));
    return FALSE;
  }
  
  // select the drive
  outportb(base + ATA_DRV_HEAD, select);
  mdelay(2); // pause for at least 1ms
  inportb(base + ATA_STATUS);  // pause and clear any pending interrupt
  
  if (inportb(base + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) {
    if (verbose) printf("1: Could not select drive...(%02X)\n", inportb(base + ATA_STATUS));
    return FALSE;
  }
  
  // save the new select value
  cur_selected = select;
  
  return TRUE;
}

// reset a ATA drive
bool ata_device_reset(struct S_ATA_CNTRLR *cntrlr, const int drv, bit16u *type_id) {
  bit8u r0, r1;
  
  if (verbose) printf("Drive reset: 0x%04X (%i)\n", cntrlr->base, drv);
  
  // we should not reset the device if the drive is busy.
  // however, we don't care if it is busy, we need the reset.
  
  // turn off interrupts
  // (4heads is a thing of the past, but we do it anyway)
  outportb(cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_4HEADS | ATA_DEV_CNTRL_nINT);
  mdelay(10);
  
  // select the drive 
  //  (do not call ata_select_drv() since it reads from the status register)
  outportb(cntrlr->base + ATA_DRV_HEAD, 0xA0 | ATA_DH_ISLBA | (drv << 4));
  mdelay(2);
  
  // check to see if there is a drive here
  outportb(cntrlr->base + ATA_SECTOR_COUNT, 0x55);
  outportb(cntrlr->base + ATA_SECTOR_NUMBER, 0xAA);
  outportb(cntrlr->base + ATA_SECTOR_COUNT, 0xAA);
  outportb(cntrlr->base + ATA_SECTOR_NUMBER, 0x55);
  outportb(cntrlr->base + ATA_SECTOR_COUNT, 0x55);
  outportb(cntrlr->base + ATA_SECTOR_NUMBER, 0xAA);
  r0 = inportb(cntrlr->base + ATA_SECTOR_COUNT);
  r1 = inportb(cntrlr->base + ATA_SECTOR_NUMBER);
  if (verbose) printf("id bytes:  0x%02X and 0x%02X\n", r0, r1);
  
  // if no drive, no need to try to reset it
  if ((r0 != 0x55) || (r1 != 0xAA))
    return FALSE;
  
  // select the drive to reset
  // (it resets both drives on this controller anyway)
  cur_selected = 0xFF;  // force select
  ata_select_drv(cntrlr->base, drv, ATA_DH_ISLBA, 0);
  
  // after setting the SRST bit, the controller may take 2us to come out of sleep mode,
  //  and 400ns to set the BSY bit.  We should wait a minimum of 5us before clearing it anyway.
  if (verbose) puts("Setting SRST bit");
  outportb(cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_RESET | ATA_DEV_CNTRL_nINT);
  udelay(5);         // hold for a min of 5 us
  
  // clear the SRST bit, disabling interrupts too
  if (verbose) puts("Clearing SRST bit");
  outportb(cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_nINT);  // clear the bit(s), no interrupts
  mdelay(2);         // hold for a min of 2+ms
  
  // wait for the controller to not be busy (for a reset, could take up to 30 seconds)
  if (!ata_wait_busy(cntrlr->base, ATA_WAIT_RDY)) {      // 30000
    if (verbose) puts("Controller returned busy after reset.");
    return FALSE;
  }
  mdelay(5);
  
  // specs say to read the ERROR register just after reset
  bit8u error = inportb(cntrlr->base + ATA_ERROR);
  if (verbose) printf("Error Register returned: 0x%02X\n", error);
  
  // force a select next time since we reset the controller above
  cur_selected = 0xFF;
  ata_select_drv(cntrlr->base, drv, 0, 0);
  
  bit8u count = inportb(cntrlr->base + ATA_SECTOR_COUNT);
  bit8u number = inportb(cntrlr->base + ATA_SECTOR_NUMBER);
  if ((count == 1) && (number == 1)) {
    bit8u cyl_low = inportb(cntrlr->base + ATA_CYL_LOW);
    bit8u cyl_high = inportb(cntrlr->base + ATA_CYL_HIGH);
    if (type_id) *type_id = (cyl_high << 8) | cyl_low;
    if (verbose) printf("Detect controller bytes: (%i) 0x%02X  0x%02X\n", drv, cyl_low, cyl_high);
    return TRUE;
  } else
    if (verbose) printf("Detect controller bytes not 1 and 1: (%i) 0x%02X  0x%02X\n", drv, count, number);
  
  return FALSE;
}

// detect ata drives one at a time
bool det_ata_drive(struct S_ATA *ata) {
  
  int i, sel;
  bool ret = FALSE;
  bit16u type_id;
  bit32u phy_address;
  
  // make sure we set this flag as false before we begin
  ata->atapi = FALSE;
  
  if (verbose) printf("Detect ATA drive: cntlr base = 0x%04X  drive = %i\n", ata->cntrlr->base, ata->drv);
  
  // reset the device.
  if (!ata_device_reset(ata->cntrlr, ata->drv, &type_id)) {
    if (verbose) printf("Did not reset the controller or drive not found on that port.\n");
    return FALSE;
  }
  
  // The ata specs say that after a reset, the cyl_high register and cyl_low register
  //  will = 0xEB and 0x14 respecively for an ATAPI compatible drive.  (ata3 -> 8.2.1.g) (ata4 -> 9.1)
  //  (ata5 -> 9.12) (ata6 -> 9.12) (ata8 -> page 225)
  switch (type_id) {
    case 0xEB14:
      ata->atapi_type = ATA_TYPE_PATAPI;
      ata->atapi = TRUE;
      break;
    case 0xC33C:
      ata->atapi_type = ATA_TYPE_SATA;
      ata->atapi = FALSE;
      break;
    case 0x9669:
      ata->atapi_type = ATA_TYPE_SATAPI;
      ata->atapi = TRUE;
      break;
    case 0x0000:
    default:
      ata->atapi_type = ATA_TYPE_PATA;
      ata->atapi = FALSE;
  }
  if (verbose) printf("Type_id returned 0x%04X  atapi = %i  atapi_type = %i\n", type_id, ata->atapi, ata->atapi_type);
  
  // get ready and send identify command(s)
  if (verbose) printf("Sending Identify Command\n");
  
  // get the ATA(PI) Identify (Packet) Device Information Block
  ret = ata_get_identify(ata, &ata->info);
  if (verbose) printf("Identify command returned = %i\n", ret);
  if (ret) {
    //printf("Dump Identify:\n");
    //dump(&ata->info, 512);
    
    // =-=-=-=-=-=-=-=-=-=
    // For the reader:
    // If this is an ATAPI device, you should send the Request Sense command here.
    //  twice..
    
    // check to see if dword IO is allowed
    bit8u temp_buf[512];
    bool temp_verbose = verbose;
    verbose = FALSE;  // don't print any verbose items when getting the IDENTIFY again
    ata->dword_io = TRUE;
    if (ata_get_identify(ata, temp_buf) && (memcmp(temp_buf, &ata->info, 512) == 0)) {
      if (temp_verbose) printf("32-bit PIO is allowed...(0x%04X)\n", ata->info.double_words);
    } else {
      if (temp_verbose) printf("32-bit PIO is NOT allowed...(0x%04X)\n", ata->info.double_words);
      ata->dword_io = FALSE;
      // =-=-=-=-=-=-=-=-=-=
      // For the reader:
      // ATAPI devices will probably return a Sense of 04h, Hardware error, here.  You will
      //  need to reset the device.  Since it is an ATAPI device, you can send the ATA_CMD_DEVICE_RESET
      //  command.  However, after sending this command, you will need to get the IDENTIFY DEVICE 
      //  block again, since most drives will expect it and won't do anything until it is received.
    }
    verbose = temp_verbose;
    
    ata->removable = (ata->info.get_conf & (1<<7)) ? TRUE : FALSE;
    ata->command_set = (ata->info.get_conf & (0x1F << 8)) >> 8;
    
    if (verbose) printf("ata->atapi = %i, ata->removable = %i\n", ata->atapi, ata->removable);
    
    // check to make sure Identify Block is valid
    if (!ata_check_identify(&ata->info))
      return FALSE;
    
    // get highest ata(pi) version
    ata->version = ata_highest_ata_version(ata->info.major_ver, ata->atapi);
    if (verbose) printf("Highest ATA version = %i\n", ata->version);
    
    // note if word 83, bits 6:5 are set, we need to spin up the device
    if ((ata->version >= 7) && ((ata->info.command_set2 & (3 << 5)) == 3)) {
      printf("Word 83, bits 6:5 == 11b.  Must send a Set Features Subcommand to drive before it will spin up...\n");
      ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
      outportb(ata->cntrlr->base + ATA_FEATURE_CODE, ATA_FEATURE_DEVICE_SPINUP);
      outportb(ata->cntrlr->base + ATA_COMMAND, CMD_SET_FEATURES); // set feature command
      if (!ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY))
        return FALSE;
      // if bit 2 of WORD 0 is set, we need to get the IDENTIFY block again.
      if (ata->info.get_conf & (1<<2)) {
        // get the ATA(PI) Identify (Packet) Device Information Block
        if (!ata_get_identify(ata, &ata->info)) {
          printf("Failed to retrieve Identify (Packet) Device Information Block...\n");
          return FALSE;
        }
      }
    }
    
    // if bit 2 of WORD 0 is still set, give error
    if (ata->info.get_conf & (1<<2))
      printf("Word 0, bit 2, \"Response Incomplete\" still set...");
    
    // "cleanup" some of the info
    fix_ide_string(ata->info.model_num, 40);
    fix_ide_string(ata->info.serial_num, 20);
    fix_ide_string(ata->info.firmware_rev, 8);
    
    // does the drive support DMA transfers
    ata->dma_supported = (ata->info.caps & (1 << 8)) ? TRUE : FALSE;
    if (!ata->dma_supported && verbose)
      printf("Drive does not support DMA transfers...\n");
    
    // does the drive support LBA addressing
    ata->lba_supported = (ata->info.caps & (1 << 9)) ? TRUE : FALSE;
    if (!ata->lba_supported && verbose)
      printf("Drive does not support LBA addressing...\n");
    
    // verify the ata/atapi status with word 85, bit 4.
    if (ata->version >= 6) {
      if (ata->atapi && !(ata->info.command_set4 & (1 << 4))) {
        printf("Error: Reset returned ATAPI, but word 85, bit 4 == 0\n");
        return FALSE;
      }
      if (!ata->atapi && (ata->info.command_set4 & (1 << 4))) {
        printf("Error: Reset returned ATA, but word 85, bit 4 == 1\n");
        return FALSE;
      }
    }
    
    // see if 48-bit capable
    if (ata->lba_supported) {
      ata->large_cap = ((ata->version >= 6) && (ata->info.command_set2 & (1<<10)) && (ata->info.command_set5 & (1<<10)) && ata->info.lba_capacity2);
      if (verbose) printf("Is 48-bit capable = %i\n", ata->large_cap);
    } else
      ata->large_cap = FALSE;
    
    if (!ata->atapi) {
      // update CHS parameters
      if (ata->version <= 2) {
        ata->cylinders = ata->info.cylinders;
        ata->heads = (bit8u) ata->info.heads;
        ata->sect_track = (bit8u) ata->info.sects_track;
      } else if (ata->info.rest_valid & 1) {
        ata->cylinders = ata->info.num_cur_cylinders;
        ata->heads = (bit8u) ata->info.num_cur_heads;
        ata->sect_track = (bit8u) ata->info.num_cur_sect_track;
      } else {
        // ata6+ doesn't show the CHS values.
        // Calculate Cyls from capacity and assume h=16 & spt=63
        ata->cylinders = (bit32u) (ata->capacity / 1008);
        ata->heads = 16;
        ata->sect_track = 63;
      }
    }
    
    // determine the capacity of the drive
    ata_capacity(ata, &ata->capacity);
    if (verbose) printf("Returned capaticy: %" LL64BIT "i sectors\n", ata->capacity);
    
    // for an example, send the CMD_READ_NATIVE_MAX(_EXT) command
    if (ata->info.command_set1 & (1<<10)) {
      bit64s native_size = -1;
      ata_select_drv(ata->cntrlr->base, ata->drv, ATA_DH_ISLBA, 0);
      if (!ata->large_cap) {
        outportb(ata->cntrlr->base + ATA_COMMAND, CMD_READ_NATIVE_MAX);
        if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {  // wait for drive ready.
          native_size = ((inportb(ata->cntrlr->base + ATA_DRV_HEAD) & 0x0F) << 24) |
                         (inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE) << 16)    |
                         (inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE) <<  8)     |
                         (inportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE) <<  0);
        }
      } else {
        outportb(ata->cntrlr->base + ATA_COMMAND, CMD_READ_NATIVE_MAX_EXT);
        if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {  // wait for drive ready.
          // write HOB bit
          outportb(ata->cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_HOB | ATA_DEV_CNTRL_nINT);
          native_size = ((bit64u) inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE) << 40) |
                        ((bit64u) inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE) << 32)  |
                        ((bit64u) inportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE) << 24);
          // clear HOB bit
          outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, 0);
          native_size |= (inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE) << 16) |
                         (inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE) <<  8)  |
                         (inportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE) <<  0);
        }
      }
      if (native_size != -1) {
        if (verbose) printf("READ Native Max (Ext) returned: %" LL64BIT "i\n", native_size);
        if ((native_size + 1) != ata->capacity)
          printf("Error: READ Native Max (Ext) returned: %" LL64BIT "i while ata->capacity = %" LL64BIT "i\n", native_size + 1, ata->capacity);
      }
    }
    
    // the IDENTIFY will only return a maximum of 0x3FFF cylinders.
    // if capacity > 0x3FFF * heads * spt, update ata->cylinders
    if (!ata->atapi)
      if (ata->capacity > (0x3FFF * ata->heads * ata->sect_track))
        ata->cylinders = (bit32u) (ata->capacity / 1008);
    if (verbose) printf("Calculated (total) cylinders (not valid on Optical Disks): %i\n", ata->cylinders);
    
    // figure out how many words per sector this drive has
    ata->words_sect = words_per_sector(ata, ata->atapi);
    if (verbose) printf("Words per sector: %i\n", ata->words_sect);
    
    // detect the DMA transfer mode
    // If PCI Device and no Bus Master register, we can't use busmaster DMA
    if (!ata->dma_supported || !ata->cntrlr->is_pci_dev || !ata->cntrlr->bus_master) {
      ata->transfer_type = DMA_TYPE_NONE;
      // SET_FEATURES: set mode to a PIO mode (0 to 7)      
      ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
      outportb(ata->cntrlr->base + ATA_FEATURE_CODE, ATA_FEATURE_SET_TRANSFER_MODE);
      outportb(ata->cntrlr->base + ATA_FEATURE_CODE_1, XFER_PIO_0);
      outportb(ata->cntrlr->base + ATA_COMMAND, CMD_SET_FEATURES); // set feature command
      if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {  // wait for drive ready.
        if (inportb(ata->cntrlr->base + ATA_FEATURE_RESULT) & ATA_ERROR_ABRT)
          printf("Error setting PIO type\n");
        else
          printf("Successfully set PIO type\n");
      } else
        printf("Error setting PIO type\n");
    } else {
      if (ata_get_dma_mode(ata)) {
        if (verbose) printf("Successfully set DMA to type %i, mode %i\n", ata->transfer_type, ata->transfer_mode);
      } else
        ata->dma_supported = FALSE;
    }
    
    // get type of cable attached (only on IDE devices)
    if (ata->cntrlr->type < CNTRL_TYPE_STD_SATA) {
      ata->cable_type = ata_get_cable_reporting(ata);
      if (verbose) {
        switch (ata->cable_type) {
          case CABLE_TYPE_40WIRE:
            printf("Cable Reporting returned 40 wire cable.\n");
            break;
          case CABLE_TYPE_40WIRE_SHORT:
            printf("Cable Reporting returned 40 short wire cable.\n");
            break;
          case CABLE_TYPE_80WIRE:
            printf("Cable Reporting returned 80 wire cable.\n");
            break;
          default:
            printf("Cable Reporting: Unknown or not a PCI Device.\n");
        }
      }
    }
    
    // is the "Indication of Media Status Notification" supported
    ata->stat_notif_enabled = FALSE;
    ata->stat_notif_support = ((ata->info.word127 & 0x0003) == 0x01);
    if (verbose) printf("ata->stat_notif_support = %i, ata->stat_notif_enabled = %i\n", ata->stat_notif_support, ata->stat_notif_enabled);
    
    // if it is a CDROM, lba = ATA_DEV_TYPE_OPTICAL_BASE, else lba = 0...
    bit64u lba = 0;
    struct S_BLOCK_STATUS status;
    ata_get_status(ata, &status);
    if (status.is_optical_disc)
      lba = ATA_DEV_TYPE_OPTICAL_BASE;
    
    if (status.inserted) {
      // Let's try to read LBA 0 (or 16) using DMA mode
      bit8u sector_buff[4096];
      bool read_okay = FALSE;
      bit8u packet[ATAPI_MAX_PACKET_SIZE];
      if (ata->dma_supported && !pio_only) {
        if (verbose) printf("Attempt to read a sector %i from LBA 0 using DMA\n", (bit32u) lba);
        init_ext_int(ata->cntrlr->irq); // Initialize and allow interrupts for the ATA
        
        outportb(ata->cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_eINT); // allow interrupts on the controller
        
        // Allocate DOS memory for the read (4096 * 2 * 2)
        // (We actually allocate 4 pages incase the first 2 cross a 64k boundary)
        //  First page will be for the DMA's Physical Region Descriptor(s)
        //  Second page will be the actual physical memory to read to
        if ((__dpmi_allocate_dos_memory((4096 * 2 * 2) / 16, &sel) == -1) ||
            (__dpmi_get_segment_base_address(sel, &phy_address) == -1)) {
          printf("Error allocating DOS memory\n");
          return FALSE;
        }
        
        // check to see we don't cross a 64k boundary, and move to the
        //  second half if we do.  (We can modify phy_address since we
        //  use 'sel' to free the memory and not 'phy_address')
        // (this also moves it if we occupy exactly the last (4096 * 2) bytes, 
        //   but so what, this is a demonstration...)
        if (((phy_address + (4096 * 2)) & 0xFFFF) < (4096 * 2)) {
          printf("Physical Address crosses a 64k boundary (0x%08X).\n"
                 "Moving to: 0x%08X\n", phy_address, (phy_address + (4096 * 2)));
          phy_address += (4096 * 2);
        }
        
        if (!ata->atapi) {
          // ATA only, send READ_DMA command
          if (!ata_tx_rx_data(ata, ATA_TRNS_TYPE_DMA, ATA_DIR_RECV, CMD_READ_DMA, ATA_WAIT_RDY, 0, lba, NULL, ata->words_sect * 2, phy_address)) {
            // error
            printf("Error in ATA only READ_DMA (status = 0x%02X, error = 0x%02X)\n", 
              inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS), inportb(ata->cntrlr->base + ATA_ERROR));
          } else
            read_okay = TRUE;
        } else {
          // ATAPI only, send PACKET command with read packet
          ata_create_rw_packet(ata->command_set, packet, ATA_DIR_RECV, lba, 1);
          if (!atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_DMA, ATA_DIR_RECV, packet, NULL, ata->words_sect * 2, phy_address)) {
            // error
            printf("Error in ATAPI only Read(12) (status = 0x%02X, error = 0x%02X)\n", 
              inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS), inportb(ata->cntrlr->base + ATA_ERROR));
          } else
            read_okay = TRUE;
        }
        
        if (read_okay) {
          read_okay = FALSE;
          
          // wait for interrupt
          if (ata_wait_int(ata, 2000)) {
            dma_stop_dma(ata);
            inportb(ata->cntrlr->base + ATA_STATUS); // clear the interrupt pending flag
            dosmemget(phy_address + 4096, ata->words_sect * 2, sector_buff);
            read_okay = TRUE;
          } else {
            dma_stop_dma(ata);
          }
        }
        
        outportb(ata->cntrlr->alt_base + ATA_DEV_CONTROL, ATA_DEV_CNTRL_nINT); // disallow interrupts on the controller
        exit_ext_int(ata->cntrlr->irq);
        __dpmi_free_dos_memory(sel);  // free the memory
      } else {
        // read a sector using PIO
        if (verbose) printf("Attempt to read a sector %i from LBA 0 using PIO\n", (bit32u) lba);
        if (!ata->atapi) {
          if (ata_tx_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, ATA_CMD_READ, ATA_WAIT_RDY, 0, lba, sector_buff, ata->words_sect * 2, (bit32u) NULL))
            read_okay = TRUE;
        } else {
          ata_create_rw_packet(ata->command_set, packet, ATA_DIR_RECV, lba, 1);
          if (atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, sector_buff, ata->words_sect * 2, (bit32u) NULL))
            read_okay = TRUE;
        }
      }
      
      // dump the sector to the screen
      if (read_okay) {
        if (verbose) {
          printf("Dump of Read Sector:\n");
          dump(sector_buff, ata->words_sect * 2);
        }
      } else
        printf("Error reading LBA 0 using PIO or DMA transfer...\n");
    }
  }
  
  return ret;
}

// Get Identify Block
bool ata_get_identify(struct S_ATA *ata, void *buffer) {
  bool ret = FALSE;
  
  // If we send an ATA Identify Device to an ATAPI device, the ATAPI device will purposely fail to indicate
  //  that we have an ATAPI device.  So first send the ATA Identify command.  If we have an ATA only device,
  //  it will succeed and we will move on.  If it fails, we will check the contents of the parameter registers
  //  and send the ATAPI Identify command.
  if (ata_tx_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, ATA_CMD_ID_DEVICE, ATA_WAIT_RDY, 0, 0, buffer, 512, (bit32u) NULL)) {
    // we have an ATA device.
    if (ata->atapi)
      printf("ID Device command insists we have an ATA device, but the signature after reset, says otherwise...\n");
    else
      ret = TRUE;
  } else {
    // The ATAPI, after receiving an ATA Identify command will set the parameter registers to the After-Reset
    //  values indicating what type of device it is.  
    if ((inportb(ata->cntrlr->base + ATA_SECTOR_COUNT) == 1) &&
        (inportb(ata->cntrlr->base + ATA_SECTOR_NUMBER) == 1) &&
        (inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE) == 0x14) &&
        (inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE) == 0xEB)) {
      if (ata_tx_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, ATA_CMD_ID_PACKET_DEVICE, ATAPI_WAIT_RDY, 0, 0, buffer, 512, (bit32u) NULL)) {
        // we have an ATAPI device
        if (!ata->atapi)
          printf("ID Device command insists we have an ATAPI device, but the signature after reset, says otherwise...\n");
        else {
          struct S_ATA_INFO *info = (struct S_ATA_INFO *) buffer;
          if (verbose) printf("Packet size = %i\n", ((info->get_conf & 0x3) == 0) ? 12 : 16);
          ret = TRUE;
        }
      }
    }
  }

  return ret;
}

// Some devices have short 40 wire cables.
const struct SHORT_CABLES short_cables[] = {
  { 0x27DF, 0x1025, 0x0102 }, // ICH7 on Acer 5602aWLMi
  { 0x27DF, 0x0005, 0x0280 }, // ICH7 on Acer 5602WLMi
  { 0x27DF, 0x1025, 0x0110 }, // ICH7 on Acer 3682WLMi
  { 0x27DF, 0x1043, 0x1267 }, // ICH7 on Asus W5F
  { 0x27DF, 0x103C, 0x30A1 }, // ICH7 on HP Compaq nc2400
  { 0x27DF, 0x1071, 0xD221 }, // ICH7 on Hercules EC-900
  { 0x24CA, 0x1025, 0x0061 }, // ICH4 on Acer Aspire 2023WLMi
  { 0x24CA, 0x1025, 0x003d }, // ICH4 on ACER TM290
  { 0x266F, 0x1025, 0x0066 }, // ICH6 on ACER Aspire 1694WLMi
  { 0x2653, 0x1043, 0x82D8 }, // ICH6M on Asus Eee 701
  { 0x27DF, 0x104d, 0x900E }, // ICH7 on Sony TZ-90
  { 0, }
};

// Returns the type of cable attached
bit8u ata_get_cable_reporting(const struct S_ATA *ata) {
  if (ata->cntrlr->is_pci_dev) {
    const bit16u device = read_pci(ata->cntrlr->pci_dev.bus, ata->cntrlr->pci_dev.dev, ata->cntrlr->pci_dev.func, 0x02, sizeof(bit16u));
    const bit16u sub_vendor = read_pci(ata->cntrlr->pci_dev.bus, ata->cntrlr->pci_dev.dev, ata->cntrlr->pci_dev.func, 0x2C, sizeof(bit16u));
    const bit16u sub_device = read_pci(ata->cntrlr->pci_dev.bus, ata->cntrlr->pci_dev.dev, ata->cntrlr->pci_dev.func, 0x2E, sizeof(bit16u));
    int i = 0;
    
    while (short_cables[i].device) {
      if ((device == short_cables[i].device) &&
          (sub_vendor == short_cables[i].sub_vendor) &&
          (sub_device == short_cables[i].sub_device))
             return CABLE_TYPE_40WIRE_SHORT;
      i++;
    }
    
    // if we got here, the device must have a regular 40 wire or 80 wire cable
    const bit8u mask = (ata->cntrlr->channel == ATA_CHANNEL_PRIMARY) ? 0x30 : 0xC0;
    const bit8u byte = read_pci(ata->cntrlr->pci_dev.bus, ata->cntrlr->pci_dev.dev, ata->cntrlr->pci_dev.func, 0x54, sizeof(bit8u));
    
    return (byte & mask) ? CABLE_TYPE_80WIRE : CABLE_TYPE_40WIRE;
  } else
    return CABLE_TYPE_UNKNOWN;
}

// currently, this only does read and write.  You will have to modify it for others.
void ata_create_rw_packet(const bit8u command_set, bit8u *packet, const bool dir, const bit64u lba, const bit32u count) {
  
  // make sure it is cleared out first
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  
  switch (command_set) {
    case 5:  // MMC-5 commands (CDROMs)
      packet[0] = (dir == ATA_DIR_SEND) ? ATAPI_CMD_WRITE_12 : ATAPI_CMD_READ_12;
      packet[1] = (0 << 5) | (0 << 4) | (1 << 3) | (0<<0);  // reserved, DPO, FUA = 1, and reserved/obsolete = 0
      packet[2] = (bit8u) ((bit64u) (lba & (bit64u) 0x00000000FF000000) >> 24);  // Big-endian LBA (byte 3)
      packet[3] = (bit8u) ((bit64u) (lba & (bit64u) 0x0000000000FF0000) >> 16);  // Big-endian LBA (byte 2)
      packet[4] = (bit8u) ((bit64u) (lba & (bit64u) 0x000000000000FF00) >>  8);  // Big-endian LBA (byte 1)
      packet[5] = (bit8u) ((bit64u) (lba & (bit64u) 0x00000000000000FF) >>  0);  // Big-endian LBA (byte 0)
      packet[6] = (bit8u) ((count & 0xFF000000) >> 24);  // Big-endian LEN (byte 3)
      packet[7] = (bit8u) ((count & 0x00FF0000) >> 16);  // Big-endian LEN (byte 2)
      packet[8] = (bit8u) ((count & 0x0000FF00) >>  8);  // Big-endian LEN (byte 1)
      packet[9] = (bit8u) ((count & 0x000000FF) >>  0);  // Big-endian LEN (byte 0)
      packet[10] = (0 << 7) | 0;     // streaming, reserved
      packet[11] = 0;                // control
      break;
    case 0:  // SBC-3 commands
      break;
    case 4:  // SBC commands (Write Once Device)
    case 7:  // SBC commands (Optical Memory Device)
      break;
    case 1:  // SSC-3 commands
      break;
    case 2:  // SSC command (printer device)
      break;
    case 3:  // SPC-2 commands (processor device)
      break;
    case 8:  // SMC-3 commands (Media Changer)
      break;
    case 12: // SCC-2 commands (Storage Array Controller device (RAID))
      break;
    case 13: // SES commands (Enclosure services)
      break;
    case 14: // RBC commands (Magnetic Disks (and USB media))
      break;
    case 6:  // (Scanner Device) (Obsolete)
    case 9:  // (Communications Device) (Obsolete)
    default: // many other values and devices...
      ;
  }
}

// If bits (7:0) of this word contain the signature A5h, bits (15:8) contain the data structure checksum.
// The data structure checksum is the two’s complement of the sum of all bytes in words (254:0) and the 
// byte consisting of bits (7:0) in word 255. Each byte shall be added with unsigned arithmetic, and 
// overflow shall be ignored. The sum of all 512 bytes is zero when the checksum is correct
bool ata_check_identify(void *block) {
  bit8u *p = (bit8u *) block;
  bit8u crc = 0;
  int i;
  
  if (p[510] == 0xA5) {
    for (i=0; i<511; i++)
      crc += p[i];
    
    /* Note:
     * We should also check that this value is not zero.
     *  if it is zero, all bytes could possibly be zero too.
     */
    
    if ((bit8u) (-crc) == p[511]) {
      if (verbose) printf("Identify Block checksum varified\n", p[511]);
      return TRUE;
    } else {
      if (verbose) printf("Identify Block did not checksum...calculated crc %02X, stored crc %02X\n", (bit8u) (-crc), p[511]);
    }
  } else if (p[510] != 0) {
      if (verbose) printf("Identify Block signature field not 0xA5...(%02X)\n", p[510]);
  } else
    // return TRUE because it could possibly be a version below ATAPI-5
    return TRUE;
  
  return FALSE;
}

/* 
 * The characters in the strings returned by the IDENTIFY command are 
 *  stored within the block as little-endian 16-bit words.
 *  e.g 'eGenir c2143' == 'Generic 2143' 
 * Therefore, we need to byte-swap them
 */  
void fix_ide_string(char *s, int len) {
  char c, *p = s, *end = s + (len & ~1);
  
  // Swap characters
  while (p != end) {
    c = *p;  
    *p = *(p + 1);  
    *(p + 1) = c;  
    p += 2;  
  }  
  
  // Make sure we have a NULL byte at the end
  // Delete trailing spaces/invalid chars
  p = end - 1;
  *p-- = '\0';
  while (p-- != s) {
    c = *p;
    if ((c > 32) && (c < 127))
      break;
    *p = '\0';
  }
}

// wait for the controller to return a 'not busy'
bool ata_wait_busy(const bit16u base, const bit16u time) {
  int timeout = time;
  while (timeout) {
    if (!(inportb(base + ATA_STATUS) & 0x80))
      return TRUE;
    mdelay(1);
    timeout--;
  }
  
  if (verbose) printf("ata_wait_busy returned FALSE\n");
  return FALSE;
}

// wait for the controller to return a 'not busy'
// if bit 7 is set, bits 6-0 are undefined
bool ata_wait(const struct S_ATA *ata, const bit8u ch, int timeout) {
  
  // we should always delay at least 400nS after sending a COMMAND and reading the (alt)Status register
  // therefore, read it once, ignoring the return
  inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS);
  
  while (timeout) {
    bit8u status = inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS);
    if (!(status & ATA_STATUS_BSY)) {
      if (status & (ATA_STATUS_ERR | ATA_STATUS_DF))
        return FALSE;
      // can be any bit of specified in ch
      if (status & ch) return TRUE;
    }
    mdelay(1);
    timeout--;
  }
  
  if (verbose) printf("ata_wait returned FALSE\n");
  return FALSE;
}

bool atapi_wait(const struct S_ATA *ata, const bit8u ch, int timeout) {
  // we should always delay at least 400nS after sending a COMMAND and reading the (alt)Status register
  // the specs say that an ignored read from the alt_status register is sufficient
  inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS);
  
  while (timeout) {
    bit8u status = inportb(ata->cntrlr->base + ATA_STATUS);
    if (status & (ATA_STATUS_ERR | ATA_STATUS_DF))
      return FALSE;
    // must be all bits specified in ch
    if ((status & ch) == ch) return TRUE;
    mdelay(1);
    timeout--;
  }
  
  if (verbose) printf("atapi_wait returned FALSE (%02X) (%02X)", inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS), inportb(ata->cntrlr->alt_base + ATA_ERROR));
  return FALSE;
}

// gets highest ATA(PI) version from identify data
// The major version bitfield is at word 80 in both ATA and ATAPI IDENTIFY buffer
bit8u ata_highest_ata_version(const bit16u major_ver, const bool atapi) {
  bit8u version = 0;
  
  if ((major_ver > 0) && (major_ver < 0xFFFF)) {
    for (version=14; version>0; version--) {
      if (major_ver & (1<<version))
        break;
    }
  } else {
    if (!atapi) {
      printf("\nWarning.  ATA device did not report ata(pi) version.  Assume version 3+ (Y,N)?");
      if (toupper(getche()) == 'Y')
        version = 3;
    } else
      version = 4; // first version with ATAPI (packet interface)
  }
  
  return version;
}

// This function returns TRUE if the given version is supported by the device.
// The major version bitfield is at word 80 in both ATA and ATAPI IDENTIFY buffer
bool ata_version_supported(const bit16u major_ver, const int vers) {
  if ((vers > 0) && (vers <= 14))
    if ((major_ver > 0) && (major_ver < 0xFFFF))
      return ((major_ver & (1 << vers)) > 0);
  return FALSE;
}

bool ata_capacity(struct S_ATA *ata, bit64u *lbas) {
  
  struct S_BLOCK_STATUS status;
  struct CAPS caps;
  
  if ((ata == NULL) || (lbas == NULL))
    return FALSE;
  
  if (!ata->atapi) {
    // Even though ATA1&2 have the words at 60-61, most drives below ATA3 didn't use it
    if (ata->version <= 2)
      *lbas = (ata->cylinders * ata->sect_track * ata->heads);
    else if (ata->large_cap)
      *lbas = ata->info.lba_capacity2;
    else
      *lbas = ata->info.lba_capacity;
  } else {
    ata_get_status(ata, &status);
    if (status.inserted) {
      atapi_read_capacity(ata, &caps, 8);
      *lbas = ENDIAN_32U(caps.lba);
      if (verbose) printf("Sector Size returned: %i\n", ENDIAN_32U(caps.size & 0x00FFFFFF));
    } else
      *lbas = 0;
  }
  
  return TRUE;
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool atapi_read_capacity(struct S_ATA *ata, void *buf, bit16u buflen) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[0] = ATAPI_CMD_READ_CAPACITY;
  packet[1] = (0 << 5) | 0 | (0<<0);  // LUN and RelAddr = 0
  
  return atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, buf, buflen, (bit32u) NULL);
}

// Sector   = (LBA mod SPT) + 1       (SPT = Sectors per Track)
// Head     = (LBA  /  SPT) mod Heads
// Cylinder = (LBA  /  SPT)  /  Heads
void ata_lba_to_chs(bit32u lba, bit8u spt, bit8u heads, bit16u *cyl, bit8u *head, bit8u *sector) {
  *sector = (bit8u)  ((lba % spt) + 1);
  *head   = (bit8u)  ((lba / spt) % heads);
  *cyl    = (bit16u) ((lba / spt) / heads);
}

// All ATA transfers assume 512 byte sectors
bool ata_tx_rx_data(const struct S_ATA *ata, bool ttype, const bool dir, const bit8u command, 
                    const int wait, const bit16u features, const bit64u lba, void *buf,
                    int buflen, const bit32u phy_address) {
  
  int i, j;
  
  // identify commands don't pass any information
  // (we don't have the information to pass with out it anyway)
  if ((command == ATA_CMD_ID_DEVICE) || (command == ATA_CMD_ID_PACKET_DEVICE)) {
    // make sure we use PIO for these two commands
    ttype = ATA_TRNS_TYPE_PIO;
    
    // select drive
    // (reads the status register to clear any pending interrupts)
    ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
    
    // The ATA IDENTIFY we must wait for the drive to be ready.
    // The ATAPI IDENTIFY we do not.
    if (command == ATA_CMD_ID_DEVICE) 
      if (!ata_wait(ata, ATA_STATUS_RDY, wait))
        return FALSE;
  } else {
    // initialize the dma?
    if (ttype == ATA_TRNS_TYPE_DMA)
      dma_init_dma(ata, phy_address, buflen);
    
    if (!ata->lba_supported) {
      // Do CHS style
      
      // convert to CHS
      bit16u cyl;
      bit8u head, sector;
      ata_lba_to_chs((bit32u) lba, (bit8u) ata->sect_track, (bit8u) ata->heads, &cyl, &head, &sector);
      
      // select drive
      // (reads the status register to clear any pending interrupts)
      ata_select_drv(ata->cntrlr->base, ata->drv, 0, head);
      
      // wait for the controller to not be busy (i.e: wait for it to be ready)
      if (ata_wait(ata, ATA_STATUS_RDY, wait)) {
        outportb(ata->cntrlr->base + ATA_FEATURES, (features & 0x00FF) >> 0);
        outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, ((buflen / 512) & 0x00FF) >> 0);
        outportb(ata->cntrlr->base + ATA_SECTOR_NUMBER, (bit8u) sector);
        outportb(ata->cntrlr->base + ATA_CYL_LOW, (bit8u) ((cyl & 0x00FF) >> 0));
        outportb(ata->cntrlr->base + ATA_CYL_HIGH, (bit8u) ((cyl & 0xFF00) >> 8));
      } else
        return FALSE;
    } else {
      if (ata->large_cap) {
        // Do 48-bit style
        
        // select drive
        // (reads the status register to clear any pending interrupts)
        ata_select_drv(ata->cntrlr->base, ata->drv, ATA_DH_ISLBA, 0);
        // wait for the controller to not be busy (i.e: wait for it to be ready)
        if (ata_wait(ata, ATA_STATUS_RDY, wait)) {
          // HOB's first
          outportb(ata->cntrlr->base + ATA_FEATURES, (features & 0xFF00) >> 8);
          outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, ((buflen / 512) & 0xFF00) >> 8);
          outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE,  (bit8u) ((bit64u) (lba & (bit64u) 0x0000FF000000) >> 24));
          outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE,  (bit8u) ((bit64u) (lba & (bit64u) 0x00FF00000000) >> 32));
          outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, (bit8u) ((bit64u) (lba & (bit64u) 0xFF0000000000) >> 40));
          // Low order last
          outportb(ata->cntrlr->base + ATA_FEATURES, (features & 0x00FF) >> 0);
          outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, ((buflen / 512) & 0x00FF) >> 0);
          outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE,  (bit8u) ((bit64u) (lba & (bit64u) 0x0000000000FF) >>  0));
          outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE,  (bit8u) ((bit64u) (lba & (bit64u) 0x00000000FF00) >>  8));
          outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, (bit8u) ((bit64u) (lba & (bit64u) 0x000000FF0000) >> 16));
        } else 
          return FALSE;
      } else {
        // Do 28-bit style
        
        // select drive
        // (reads the status register to clear any pending interrupts)
        ata_select_drv(ata->cntrlr->base, ata->drv, ATA_DH_ISLBA, (bit8u) ((bit64u) (lba & (bit64u) 0x00000F000000) >> 24));
        
        // wait for the controller to not be busy (i.e: wait for it to be ready)
        if (ata_wait(ata, ATA_STATUS_RDY, wait)) {
          outportb(ata->cntrlr->base + ATA_FEATURES, (features & 0xFF));
          outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, ((buflen / 512) & 0xFF));
          outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE,  (bit8u) (((bit32u) lba & 0x000000FF) >>  0));
          outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE,  (bit8u) (((bit32u) lba & 0x0000FF00) >>  8));
          outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, (bit8u) (((bit32u) lba & 0x00FF0000) >> 16));
        } else
          return FALSE;
      }
    }
  }
  
  // send the command
  outportb(ata->cntrlr->base + ATA_COMMAND, command);
  // wait for the drive to be ready for the transfer
  if (ata_wait(ata, ATA_STATUS_DRQ, wait)) {
    if (ttype == ATA_TRNS_TYPE_PIO) {
      // All sector transfers on an ATA call are 512 byte sectors...
      bit32u addr = (bit32u) buf;
      while (buflen > 0) {
        if (ata->dword_io) {
          bit32u *ptr = (bit32u *) addr;
          j = ((buflen > 512) ? 512 : buflen) / sizeof(bit32u);
          for (i=0; i<j; i++) {
            if (dir == ATA_DIR_RECV)
              *ptr++ = inportl(ata->cntrlr->base + ATA_DATA);
            else
              outportl(ata->cntrlr->base + ATA_DATA, *ptr++);
          }
        } else {
          bit16u *ptr = (bit16u *) addr;
          j = ((buflen > 512) ? 512 : buflen) / sizeof(bit16u);
          for (i=0; i<j; i++) {
            if (dir == ATA_DIR_RECV)
              *ptr++ = inportw(ata->cntrlr->base + ATA_DATA);
            else
              outportw(ata->cntrlr->base + ATA_DATA, *ptr++);
          }
        }
        buflen -= 512;
        addr += 512;
        // wait for the drive to be ready for the next transfer
        if ((buflen > 0) && !ata_wait(ata, ATA_STATUS_DRQ, wait))
          return FALSE;
      }
    } else
      dma_start_dma(ata, dir);
    return TRUE;
  }
  
  return FALSE;
}

bool atapi_tx_packet_rx_data(struct S_ATA *ata, const bool ttype, const bool dir, bit8u *packet, void *buf, 
                             int buflen, const bit32u phy_address) {
  int i, j, count, sect_size, bytes_per_drq;
  const int packet_size = ((ata->info.get_conf & 0x3) == 0) ? 12 : 16;
  void *buffer;
  
  // if we haven't figured out words_per_sect yet, assume 512
  sect_size = (ata->words_sect == 0) ? (512 / 2) : ata->words_sect;
  
  // initialize the dma
  if (ttype == ATA_TRNS_TYPE_DMA)
    dma_init_dma(ata, phy_address, buflen);
  
  // select drive
  ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
  
  // wait for the controller to not be busy (i.e: wait for it to be ready)
  if (ata_wait(ata, ATA_STATUS_RDY, ATAPI_WAIT_RDY)) {
    // now for the parameters
    outportb(ata->cntrlr->base + ATA_FEATURES, (0 << 1) | (ttype << 0));     // OVR = 0, DMA = (PIO mode (0) or DMA mode (1))
    outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, 0);  // (Tag N/A)
    outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE, 0);  // N/A
    outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE, (((sect_size * 2) >> 0) & 0xFF));  // low byte of limit  (size of all bytes to transfer...)
    outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, (((sect_size * 2) >> 8) & 0xFF)); // high byte of limit (...not counting command packet sent)
    outportb(ata->cntrlr->base + ATA_COMMAND, ATA_CMD_PACKET);  // send packet
    // The hardware is usually instantly ready for the packet, but
    //  we wait for it anyway.
    if (!ata_wait(ata, ATA_STATUS_DRQ, ATAPI_WAIT_RDY))
      return FALSE;
    
    // send the 12- or 16-byte packet
    for (i=0; i<packet_size; i+=2)
      outportw(ata->cntrlr->base + ATA_DATA, packet[i] | (packet[i+1] << 8));
    
    if (buflen > 0) {
      if (ttype == ATA_TRNS_TYPE_PIO) {
        // wait for the drive to be ready for the transfer
        if (atapi_wait(ata, ATA_STATUS_DRQ, ATAPI_WAIT_RDY)) {
          bytes_per_drq = (inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE) << 8) | inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE);
          buffer = malloc(bytes_per_drq);
          
          // On an ATAPI transfer, the tranfer length can and usually will be less
          //  than a multiple of a sector size.  Therefore, we code it so that we
          //  only transfer the amount requested (rounded up).
          if (ata->dword_io)
            count = ((bytes_per_drq + (sizeof(bit32u) - 1)) / sizeof(bit32u));
          else
            count = ((bytes_per_drq + (sizeof(bit16u) - 1)) / sizeof(bit16u));
          do {
            // if it is a transfer out (write), we need to copy to our temp buffer
            if ((buf != NULL) && (buflen > 0) && (dir == ATA_DIR_SEND))
              memcpy(buffer, buf, MIN(buflen, bytes_per_drq));
            
            if (ata->dword_io) {
              bit32u *ptr = (bit32u *) buffer;
              for (i=0; i<count; i++) {
                if (dir == ATA_DIR_RECV)
                  *ptr++ = inportl(ata->cntrlr->base + ATA_DATA);
                else
                  outportl(ata->cntrlr->base + ATA_DATA, *ptr++);
              }
            } else {
              bit16u *ptr = (bit16u *) buffer;
              for (i=0; i<count; i++) {
                if (dir == ATA_DIR_RECV)
                  *ptr++ = inportw(ata->cntrlr->base + ATA_DATA);
                else
                  outportw(ata->cntrlr->base + ATA_DATA, *ptr++);
              }
            }
            // read the status register to acknowledge command
            inportb(ata->cntrlr->base + ATA_STATUS);
            
            // move the data to our passed buffer
            if ((buf != NULL) && (buflen > 0)) {
              int c = MIN(buflen, bytes_per_drq);
              if (dir == ATA_DIR_RECV)
                memcpy(buf, buffer, c);
              buf = (void *) ((bit32u) buf + c);
              buflen -= c;
            }
            // wait for the drive to be ready for the next transfer
          } while (atapi_wait(ata, ATA_STATUS_DRQ, 50));  // don't wait very long to see if there are more bytes to transfer (50ms)
          
          // free the memory
          free(buffer);
          
          // return successful transfer
          return TRUE;
        }
      } else
        dma_start_dma(ata, dir);
      return TRUE;
    }
  }
  
  return FALSE;
}

int words_per_sector(struct S_ATA *ata, const bool atapi) {
  
  int i, cnt = 0;
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  const int packet_size = ((ata->info.get_conf & 0x3) == 0) ? 12 : 16;
  if (atapi) {
    memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
    packet[ 0] = ATAPI_CMD_READ_12;
    packet[ 5] = 16;
    packet[ 9] = 1;
  }
  
  if (ata_inserted(ata)) {
    // select drive
    ata_select_drv(ata->cntrlr->base, ata->drv, ATA_DH_ISLBA, 0);
    
    // wait for the controller to not be busy
    if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {
      // now for the parameters
      if (atapi) {
        outportb(ata->cntrlr->base + ATA_FEATURES, 0);      // OVR = 0, DMA = 0 (PIO mode)
        outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, 0);  // (Tag N/A)
        outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE, 0);  // N/A
        outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE, ((2048 >> 0) & 0xFF));  // low byte of limit  (size of all bytes to transfer...)
        outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, ((2048 >> 8) & 0xFF)); // high byte of limit (...not counting command packet sent)
      } else {
        outportb(ata->cntrlr->base + ATA_FEATURES, 0);
        outportb(ata->cntrlr->base + ATA_SECTOR_COUNT, 1);
        outportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE, 0);
        outportb(ata->cntrlr->base + ATA_LBA_MID_BYTE, 0);
        outportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE, 0);
      }
      
      if (atapi) {
        outportb(ata->cntrlr->base + ATA_COMMAND, ATA_CMD_PACKET);
        for (i=0; i<packet_size; i+=2)
          outportw(ata->cntrlr->base + ATA_DATA, packet[i] | (packet[i+1] << 8));
      } else      
        outportb(ata->cntrlr->base + ATA_COMMAND, ATA_CMD_READ);
      // This assumes that all sectors will be a multiple of 512 bytes.
      // We could easily check for smaller sectors down to a multiple of 2 bytes
      //  by checking the DRQ bit after every word read.  This will give us a multiple of 2 bytes.
      // However, every sector should be a multiple of 512-bytes...
      while (ata_wait(ata, ATA_STATUS_DRQ, (atapi) ? ATAPI_WAIT_RDY : ATA_WAIT_RDY)) {
        for (i=0; i<(512>>1); i++)
          inportw(ata->cntrlr->base + ATA_DATA);
        cnt += 256;
      }
    } else
      // This is not an error.  We want this one to happen.
      puts("ATA Controller: wait failed.");
    
    // if cnt == 0, then assume 256
    if (cnt == 0) cnt = 256;
    
  } else
    cnt = 0;
  
  return cnt;
}

// Is there a disk in the drive.
bool ata_inserted(struct S_ATA *ata) {
  
  // if it is a non removable drive, don't bother checking
  if (!ata->removable) return TRUE;
  
  struct S_BLOCK_STATUS status;
  ata_get_status(ata, &status);
  
  if (verbose) printf("ata_inserted returning: %i\n", status.inserted);
  return status.inserted;
}

bool ata_get_dma_mode(struct S_ATA *ata) {
  int i;
  bool ret = FALSE;
  
  ata->transfer_type = DMA_TYPE_NONE;  // assume no DMA
  ata->transfer_mode = 0;
  
  // first start with ultra-dma
  // Check bit 2 in Word 53 to see if Word 88 is valid
  if (ata->info.rest_valid & (1 << 2)) {
    i = 0;
    while ((i < 7) && (ata->info.dma_ultra & (1 << i)))
      i++;
    if (i < 7) {
      ata->transfer_type = DMA_TYPE_ULTRA;
      ata->transfer_mode = i - 1;
    }
  } else {
    // Ultra DMA must not be valid, so try multiword
    i = 0;
    while ((i < 4) && (ata->info.multiword_dma & (1 << i)))
      i++;
    if (i < 4) {
      ata->transfer_type = DMA_TYPE_MULTIWORD;
      ata->transfer_mode = i - 1;
    }
  }  
  
  /*
   * The specs say that you should use the following type of DMA unless you have
   *  specific code to handle Ultra-DMA, or a different mode of MultiWord, mode 2.
   * For the purpose of this book, I showed above how to retrieve the modes, and
   *  show below how to select these modes, but will now set it to the default 
   *  of Multiword, mode 2 and return.
   * Please be sure and read the note about this in Chapter 10
   */
  ata->transfer_type = DMA_TYPE_MULTIWORD;
  ata->transfer_mode = 2;
  ret = TRUE;
  
  /*
  // if we found a type, let's set the controller to that type and mode
  if (ata->transfer_type != DMA_TYPE_NONE) {
    ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
    outportb(ata->cntrlr->base + ATA_FEATURE_CODE, ATA_FEATURE_SET_TRANSFER_MODE);
    outportb(ata->cntrlr->base + ATA_FEATURE_CODE_1, ((ata->transfer_type << 3) | ata->transfer_mode));
    outportb(ata->cntrlr->base + ATA_COMMAND, CMD_SET_FEATURES); // set feature command
    if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {  // wait for drive ready.
      if (inportb(ata->cntrlr->base + ATA_FEATURE_RESULT) & ATA_ERROR_ABRT)
        printf("Error setting DMA to type %i, mode %i\n", ata->transfer_type, ata->transfer_mode);
      else
        ret = TRUE;
    }
  }
  */
  
  return ret;
}

bool ata_get_status(struct S_ATA *ata, struct S_BLOCK_STATUS *status) {
  
  bit8u error, buffer[512];
  bit8u req_sense[16];
  bool ret = FALSE;
  int t;
  
  // if status = null, return error
  if (!status)
    return FALSE;
  memset(status, 0, sizeof(struct S_BLOCK_STATUS));
  
  if (ata->removable) {
    // first check the status via "Get Media Status" command, but only if the
    //  "Removable Media Status Notification" feature set is active.
    if (ata->atapi && (ata->info.command_set1 & (1 << 2))) {
      ata_select_drv(ata->cntrlr->base, ata->drv, 0, 0);
      outportb(ata->cntrlr->base + ATA_COMMAND, ATA_CMD_GET_MEDIA_STATUS);
      if (ata_wait(ata, ATA_STATUS_RDY, ATA_WAIT_RDY)) {  // wait for drive ready.
        error = inportb(ata->cntrlr->base + ATA_ERROR);
        if (!(error & ATA_ERROR_ABRT)) {
          status->write_prot = ((error & ATA_ERROR_WP)  > 0);
          status->changed    = ((error & ATA_ERROR_MC)  > 0);
          status->inserted   = ((error & ATA_ERROR_NM) == 0);
          status->removable  = TRUE;
          status->medium_type = MEDIA_PROFILE_0008; // assume a CD-ROM
          return TRUE;
        }
      }
    }
    
    // if ATA device, or ATA_CMD_GET_MEDIA_STATUS not available or failed above,
    //  try the Request Sense command.
    for (t=0; t<4; t++) {
      if (atapi_request_sense(ata, req_sense, 16))
        break;
    }
    if (t == 4)
      return FALSE;
    
    if (verbose) printf("Request Sense returned %02X %02X\n", req_sense[2], req_sense[12]);
    status->changed = (((req_sense[2] & 0x0F) == 6) && (req_sense[12] == 0x28));
    
    if (atapi_get_config(ata, buffer, 512, MEDIA_PROFILE_0008)) {
      struct S_GET_CONFIG_HDR *config = (struct S_GET_CONFIG_HDR *) buffer;
      struct S_GET_CONFIG_DESC *desc = (struct S_GET_CONFIG_DESC *) (buffer + sizeof(struct S_GET_CONFIG_HDR));
      if (verbose) printf("atapi_get_config() returned TRUE (%02Xh)\n", ENDIAN_16U(config->current));
      while (ENDIAN_16U(desc->code) < 0x0111) {
        switch (ENDIAN_16U(desc->code)) {
          case 3:   // The medium may be removed from the device
            status->removable = TRUE;
            break;
          case 30:  // CD Read-The ability to read CD specific structures
            break;
          case 31:  // DVD Read-The ability to read DVD specific structures
            status->dvd_capable = TRUE;
            break;
          case 32:  // Random Writable-Write support for randomly addressed writes
            status->writable = TRUE;
            break;
          case 0x0108: // drive has unique identifier
            // identifier following descriptor header
            break;
          case 0x0109: // has ability to return unique media serial number
            // able to read serial number via command ABh sub-command 01h.
            status->read_serial_num = TRUE;
            break;
          default:
            ;
        }
        desc = (struct S_GET_CONFIG_DESC *) ((bit8u *) desc + 4 + desc->add_len);
      }
      status->medium_type = ENDIAN_16U(config->current);
      status->inserted = (status->medium_type > MEDIA_PROFILE_0000);
      ret = TRUE;
      status->is_optical_disc = (((status->medium_type >= MEDIA_PROFILE_0008) && (status->medium_type <= MEDIA_PROFILE_000A)) ||
                                 ((status->medium_type >= MEDIA_PROFILE_0010) && (status->medium_type <= MEDIA_PROFILE_0018)) ||
                                 ((status->medium_type >= MEDIA_PROFILE_001A) && (status->medium_type <= MEDIA_PROFILE_001B)) ||
                                 ((status->medium_type >= MEDIA_PROFILE_002A) && (status->medium_type <= MEDIA_PROFILE_002B)) ||
                                 ((status->medium_type >= MEDIA_PROFILE_0040) && (status->medium_type <= MEDIA_PROFILE_0043)) ||
                                 ((status->medium_type >= MEDIA_PROFILE_0050) && (status->medium_type <= MEDIA_PROFILE_0053)) ||
                                  (status->medium_type == MEDIA_PROFILE_0058) ||
                                  (status->medium_type == MEDIA_PROFILE_005A));
      // double check removable
      //  A lot of devices don't have the removable feature entry...
      // However, if we got here, we know it is removable since this whole
      //  block of code is only executed if ata->removable is set...
      if (ata->removable != status->removable) {
        if (verbose) printf("Error: ata->removable != status.removable\n");
        status->removable = ata->removable;
      }
    } else {
      status->inserted = FALSE;
      ret = FALSE;
    }
  } else {
    status->inserted = TRUE;
    status->changed = FALSE;
    ret = TRUE;
  }
  
  if (verbose) printf("ata_get_status returning: inserted = %i, changed = %i, write_prot = %i\n",
    status->inserted, status->changed, status->write_prot);
  return ret;
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 *
 * Always request 252 bytes of request data (section 6.27 of SPC-3)
 */
bool atapi_request_sense(struct S_ATA *ata, void *buf, bit16u buflen) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  bit8u request[252];
  memset(request, 0, 252);
  
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[ 0] = ATAPI_CMD_REQUEST_SENSE;
  packet[ 4] = 252;
  
  if (atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, request, 252, (bit32u) NULL)) {
    memcpy(buf, request, buflen);
    return TRUE;
  }
  if (verbose) printf("atapi_request_sense() failed.\n");
  
  return FALSE;
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool atapi_mode_sense(struct S_ATA *ata, void *buf, bit16u buflen, bit8u pc, bit8u pagecode) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[ 0] = ATAPI_CMD_MODE_SENSE;
  packet[ 2] = (bit8u) (pc << 6) | pagecode;
  packet[ 7] = (bit8u) ((buflen >>  8) & 0xFF);
  packet[ 8] = (bit8u) ((buflen >>  0) & 0xFF);
  
  return atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, buf, buflen, (bit32u) NULL);
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool atapi_inquiry(struct S_ATA *ata, void *buf, bit16u buflen, bit8u pagecode) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[ 0] = ATAPI_CMD_INQUIRY;
  packet[ 2] = pagecode;
  packet[ 3] = 0;
  packet[ 4] = 255;
  return atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, buf, buflen, (bit32u) NULL);
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool atapi_get_config(struct S_ATA *ata, void *buf, bit16u buflen, bit16u feature_num) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[ 0] = ATAPI_CMD_GET_CONFIGURATION;
  packet[ 1] = 1;
  packet[ 2] = ((feature_num >> 8) & 0xFF);
  packet[ 3] = ((feature_num >> 0) & 0xFF);
  packet[ 7] = ((buflen >> 8) & 0xFF);
  packet[ 8] = ((buflen >> 0) & 0xFF);
  return atapi_tx_packet_rx_data(ata, ATA_TRNS_TYPE_PIO, ATA_DIR_RECV, packet, buf, buflen, (bit32u) NULL);
}

volatile bool ata_drv_stats = FALSE;

void ata_irq_master(void) {
  ata_drv_stats = TRUE;
  outportb(0x20, 0x20); // EOI
}

void ata_irq_slave(void) {
  ata_drv_stats = TRUE;
  outportb(0xA0, 0x20); // EOI
  outportb(0x20, 0x20); // EOI
}

// wait for completion interrupt
//  returns FALSE if takes more than given time
bool ata_wait_int(const struct S_ATA *ata, int timeout) {
  while (timeout) {
    if (ata_drv_stats) {
      ata_drv_stats = FALSE;
      // If you are running this in VirtualBox, this will return FALSE
      // VirtualBox does not set the BM_STATUS_INTR bit correctly.
      // Comment out this line and return TRUE with the second line
      //  if you are using VirtualBox.
      if (inportb(ata->cntrlr->bus_master + BM0_STATUS) & BM_STATUS_INTR)
        return TRUE;
    }
    mdelay(1); // hold for 1 millisecond
    timeout--;
  }
  return FALSE;
}

const int irq_to_vect[] = {
/* IRQ 0- 7 */	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
/* IRQ 8-15 */	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
};
_go32_dpmi_seginfo ata_old, ata_new;
void init_ext_int(const int irq_num) {
  
  if (irq_num >= 16)
		return;
  
  int vector = irq_to_vect[irq_num];
  
  if (verbose) printf("Hooking IRQ %i (vector 0x%02X)\n", irq_num, vector);
  
  // get the old handler info
  _go32_dpmi_get_protected_mode_interrupt_vector(vector, &ata_old);
  
  // set new handler info
  if (irq_num >= 8)
    ata_new.pm_offset = (bit32u) ata_irq_slave;
  else
    ata_new.pm_offset = (bit32u) ata_irq_master;
  ata_new.pm_selector = _go32_my_cs();
  
  _go32_dpmi_allocate_iret_wrapper(&ata_new);
  _go32_dpmi_set_protected_mode_interrupt_vector(vector, &ata_new);
  enable_irq_at_8259(irq_num);
}

void exit_ext_int(const int irq_num) {
  
	if (irq_num >= 16)
		return;
  
  int vector = irq_to_vect[irq_num];
  
  // ata free
  disable_irq_at_8259(irq_num);
  _go32_dpmi_set_protected_mode_interrupt_vector(vector, &ata_old);
  _go32_dpmi_free_iret_wrapper(&ata_new);
}

void enable_irq_at_8259(int irq_num) {
  if (irq_num >= 16)
    return;
  
  if (irq_num >= 8) {
    outportb(0x21, inportb(0x21) & ~0x04);
    irq_num = 1 << (irq_num - 8);
    outportb(0xA1, inportb(0xA1) & ~irq_num);
  } else {
    irq_num = 1 << irq_num;
    outportb(0x21, inportb(0x21) & ~irq_num);
  }
}

void disable_irq_at_8259(int irq_num) {
  if (irq_num >= 16)
    return;
  
  if (irq_num >= 8) {
    irq_num = 1 << (irq_num - 8);
    outportb(0xA1, inportb(0xA1) | irq_num);
  } else {
    irq_num = 1 << irq_num;
    outportb(0x21, inportb(0x21) | irq_num);
  }
}

// setup the dma for a transfer
void dma_init_dma(const struct S_ATA *ata, const bit32u address, const int size) {
  
  if (ata->cntrlr->is_pci_dev) {
    if (ata->cntrlr->bus_master) {
      // create descriptor table
      bit32u table[2];
      table[0] = address + 4096;    // address to store sector
      table[1] = 0x80000000 | size; // EOT plus size of sector
      dosmemput((const void *) table, sizeof(bit32u) * 2, address);
      if (ata->cntrlr->channel == ATA_CHANNEL_PRIMARY) {
        outportb(ata->cntrlr->bus_master + BM0_COMMAND, 0);
        outportb(ata->cntrlr->bus_master + BM0_STATUS, (1 << 2) | (1 << 1));
        outportl(ata->cntrlr->bus_master + BM0_ADDRESS, address);
      } else {
        outportb(ata->cntrlr->bus_master + BM1_COMMAND, 0);
        outportb(ata->cntrlr->bus_master + BM1_STATUS, (1 << 2) | (1 << 1));
        outportl(ata->cntrlr->bus_master + BM1_ADDRESS, address);
      }
    } else {
      // Is pci device, but no bus_master...
      //  Then must use the ISA type DMA (see note below)
    }
  } else {
    // Do ISA type DMA
    //  On 32-bit machines, it is faster to read 128 32-bit dwords using
    //  PIO transfers than it is to set up the ISA DMA and use it.
    //  Therefore, you might want to return FALSE here and default back
    //   to PIO transfers.
  }
}

void dma_start_dma(const struct S_ATA *ata, const bool dir) {
  if (ata->cntrlr->is_pci_dev) {
    if (ata->cntrlr->bus_master) {
      if (ata->cntrlr->channel == ATA_CHANNEL_PRIMARY)
        outportb(ata->cntrlr->bus_master + BM0_COMMAND, (dir << 3) | (1 << 0));
      else
        outportb(ata->cntrlr->bus_master + BM1_COMMAND, (dir << 3) | (1 << 0));
    } else {
      // is pci device, but no bus_master...
    }
  } else {
    // do ISA type DMA
  }
}

bit8u dma_stop_dma(const struct S_ATA *ata) {
  bit8u status;
  
  if (ata->cntrlr->is_pci_dev) {
    if (ata->cntrlr->bus_master) {
      if (ata->cntrlr->channel == ATA_CHANNEL_PRIMARY) {
                 inportb(ata->cntrlr->bus_master + BM0_STATUS);
        status = inportb(ata->cntrlr->bus_master + BM0_STATUS);
        outportb(ata->cntrlr->bus_master + BM0_COMMAND, (0 << 0));
        outportb(ata->cntrlr->bus_master + BM0_STATUS, status);
        return status;
      } else {
                 inportb(ata->cntrlr->bus_master + BM1_STATUS);
        status = inportb(ata->cntrlr->bus_master + BM1_STATUS);
        outportb(ata->cntrlr->bus_master + BM1_COMMAND, (0 << 0));
        outportb(ata->cntrlr->bus_master + BM1_STATUS, status);
        return status;
      }
    } else {
      // is pci device, but no bus_master...
    }
  } else {
    // do ISA type DMA
  }
  
  return 0;
}

// simply parses the command line parameters for specific values
bool get_parameters(int argc, char *argv[]) {
  
  for (int i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(&argv[i][1], "v") == 0)
        verbose = TRUE;
      else if (strcmp(&argv[i][1], "ide") == 0)
        sata_mode = SATA_MODE_IDE;
      else if (strcmp(&argv[i][1], "ahci") == 0)
        sata_mode = SATA_MODE_AHCI;
      else if (strcmp(&argv[i][1], "do_isa") == 0)
        do_isa = TRUE;
      else if (strcmp(&argv[i][1], "pio_only") == 0)
        pio_only = TRUE;
      else {
        printf("Unknown parameter: %s\n", argv[i]);
        return FALSE;
      }
    } else {
      printf("Unknown parameter: %s\n", argv[i]);
      return FALSE;
    }
  }
  
  return TRUE;
}

void dump_regs(const struct S_ATA *ata) {
  printf("Controller Base = 0x%04X, drv = %i\n", ata->cntrlr->base, ata->drv);
  printf("  Error = 0x%02X\n", inportb(ata->cntrlr->base + ATA_ERROR));
  printf("  Count = 0x%02X\n", inportb(ata->cntrlr->base + ATA_SECTOR_COUNT));
  printf("    Low = 0x%02X\n", inportb(ata->cntrlr->base + ATA_LBA_LOW_BYTE));
  printf("    Mid = 0x%02X\n", inportb(ata->cntrlr->base + ATA_LBA_MID_BYTE));
  printf("   High = 0x%02X\n", inportb(ata->cntrlr->base + ATA_LBA_HIGH_BYTE));
  printf(" Drv_Hd = 0x%02X\n", inportb(ata->cntrlr->base + ATA_DRV_HEAD));
  printf(" Status = 0x%02X\n", inportb(ata->cntrlr->alt_base + ATA_ALT_STATUS));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SATA AHCI stuff starts here
///
///
///
const char modestr[4][16] = {
  "Legacy IDE Mode",   // SATA_MODE_IDE
  "AHCI Mode",         // SATA_MODE_AHCI
  "RAID Mode",         // SATA_MODE_RAID
  "Reserved????"       // SATA_MODE_RESV
};

bool ata_sata_detect(struct S_ATA_CNTRLR *cntrlr, const bit8u pci_bus, const bit8u pci_dev, const bit8u pci_func, const char *str) {
  bit16u addr_map;
  int mode, num_drives, i, drive;
  bit8u *buffer;
  bit32u dword;
  
  // I only demonstrate the ICH9R controller here, since all SATA controllers
  //  are vendor/device specific.
  // If you know that your controller has the same map register address, you 
  //  may comment this block out so that the code continues on.
  if (cntrlr->type != CNTRL_TYPE_ICH9R) {
    printf("\nSATA controllers are vendor/device specific.  I only demonstrate the\n"
           "Intel ICH9R SATA (IDE/AHCI/RAID) controller here.  Other controllers\n"
           "should be very similar.\n");
    return FALSE;
  }
  
  //
  // The way to switch modes is device specific and usually in the PCI Config Space.
  // It is not in the AHCI specs or the SATA specs...
  // *** It is recommended that you do not modify these bits as an OS. ***
  // *** It is recommended that you let the BIOS modify them for your with
  //       its boot up sequence and menu options. ***

  // if the controller is in IDE mode, we can either call the IDE code,
  //  or change it to AHCI and do this.
  // if it is currently in RAID mode, we can give error and return,
  //  or change it to AHCI and do this.

  //
  //  *** This code is for the Intel ICH9 SATA AHCI controller. ***
  //  *** You will need to check the controller you are using and change
  //       this to allow that controller.  ***
  //  *** This code is for example only. ***
  //
  addr_map = read_pci(pci_bus, pci_dev, pci_func, 0x90, sizeof(bit16u));
  mode = (addr_map & 0x00C0) >> 6;
  printf("\nThis controller is currently in %s\n", modestr[mode]);
  
  bit32u base = read_pci(pci_bus, pci_dev, pci_func, 0x24, sizeof(bit32u));
  // mem I/O access enable and bus master enable
  write_pci(pci_bus, pci_dev, pci_func, 0x04, sizeof(bit16u), (1 << 2) | (1 << 1));
  
  // set up the memmapped I/O access
  // The AHCI controller uses the dword at base5 and is memmapped access
  cntrlr->base_mi.address = (base & 0xFFFFF800);
  cntrlr->base_mi.size = 0x1100;
  if (!get_physical_mapping(&cntrlr->base_mi, &cntrlr->base_selector)) {
    printf("Error 'allocating' physical memory for HBA.\n");
    return FALSE;
  }
  
  switch (mode) {
    case SATA_MODE_IDE:
      switch (sata_mode) {
        case -1:
        case SATA_MODE_IDE:
          _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x00000000); // make sure the EA bit is clear
          return ata_ide_detect(cntrlr, pci_bus, pci_dev, pci_func, str);
        case SATA_MODE_AHCI:
          // switch to AHCI mode and fall through
          if ((addr_map & 3) == 0) {
            write_pci(pci_bus, pci_dev, pci_func, 0x90, sizeof(bit16u), ((addr_map & ~0x00C0) | (SATA_MODE_AHCI << 6)));
            _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x80000000); // make sure the EA bit is set
            printf("Switching to AHCI mode...\n");
            // and fall through
          } else {
            printf("ADDR_MAP:MV != 0.  Not switching to AHCI mode...\n");
            return FALSE;
          }
          break;
        default:
          return FALSE; // error, shouldn't have gotten here
      }
      break;
    case SATA_MODE_AHCI:
      switch (sata_mode) {
        case -1:
        case SATA_MODE_AHCI:
          break; // fall through
        case SATA_MODE_IDE:
          // switch to IDE mode
          if ((addr_map & 3) == 0) {
            write_pci(pci_bus, pci_dev, pci_func, 0x90, sizeof(bit16u), ((addr_map & ~0x00C0) | (SATA_MODE_IDE << 6)));
            _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x00000000); // make sure the EA bit is clear
            printf("Switching to IDE mode...\n");
            return ata_ide_detect(cntrlr, pci_bus, pci_dev, pci_func, str);
          } else {
            printf("ADDR_MAP:MV != 0.  Not switching to IDE mode...\n");
            return FALSE;
          }
        default:
          return FALSE; // error, shouldn't have gotten here
      }
      break;
    case SATA_MODE_RAID:
      switch (sata_mode) {
        case SATA_MODE_IDE:
          // switch to IDE mode
          if ((addr_map & 3) == 0) {
            write_pci(pci_bus, pci_dev, pci_func, 0x90, sizeof(bit16u), ((addr_map & ~0x00C0) | (SATA_MODE_IDE << 6)));
            _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x00000000); // make sure the EA bit is clear
            printf("Switching to IDE mode...\n");
            return ata_ide_detect(cntrlr, pci_bus, pci_dev, pci_func, str);
          } else {
            printf("ADDR_MAP:MV != 0.  Not switching to IDE mode...\n");
            return FALSE;
          }
        case SATA_MODE_AHCI:
          // switch to AHCI mode
          if ((addr_map & 3) == 0) {
            printf("Switching to AHCI mode...\n");
            write_pci(pci_bus, pci_dev, pci_func, 0x90, sizeof(bit16u), ((addr_map & ~0x00C0) | (SATA_MODE_AHCI << 6)));
            _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x80000000); // make sure the EA bit is set
          } else {
            printf("ADDR_MAP:MV != 0.  Not switching to AHCI mode...\n");
            return FALSE;
          }
          // fall through
          break;
        default:
          printf("Found RAID mode.  Ignoring and returning...\n");
          return FALSE;
      }
      break;
    default:
      printf("Found reserved/unknown mode.  Ignoring and returning...\n");
      return FALSE;
  }
  
  /************************************************************************************
  * we either found an SATA controller already in AHCI mode, or we switched           *
  *  it to AHCI mode due to a command line of -ahci                                   *
  ************************************************************************************/
  
  // Gain ownership of the controller
  if (!sata_gain_ownership(cntrlr)) {
    printf("There was an error trying to gain ownership of the controller...\n");
    return FALSE;
  }
  
  /*
   * To place the AHCI HBA into a minimally initialized state, system software shall:
   *  1. Indicate that system software is AHCI aware by setting GHC.AE to ‘1’.
   * (Note: We don't neccassarily follow the specs in order.  We do 1., 4., 2., etc.)
   */
  i = 5;
  while ((_farpeekl(cntrlr->base_selector, HBA_HC_Glob_host_cntrl) & 0x80000000) == 0) {
    if (--i == 0) {
      printf("Could not set the EA bit in the Global Host Controller register after 5 tries...\n");
      return FALSE;
    }
    _farpokel(cntrlr->base_selector, HBA_HC_Glob_host_cntrl, 0x80000000);
    mdelay(100);
  }
  
  if (verbose) {
    printf("     Caps: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Capabilities));
    printf("      GHC: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Glob_host_cntrl));
    printf("       IS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Interrupt_status));
    printf("       PI: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Ports_implemented));
    printf("       VS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Version));
    printf("  CCC_CTL: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Ccc_ctl));
    printf("CCC_PORTS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Ccc_ports));
    printf("   EM_LOC: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Em_location));
    printf("   EM_CTL: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Em_control));
    printf("    CAPS2: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Capabilities_ext));
    printf("     BOHC: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_HC_Bohc));
  }
  
  /*
   * 4. Determine how many command slots the HBA supports, by reading CAP.NCS.
   *    (We also get the count of allocated drives (ports) it has)
   */
  cntrlr->command_slots = ((_farpeekl(cntrlr->base_selector, HBA_HC_Capabilities) & 0x00000F00) >> 8);
  num_drives = (_farpeekl(cntrlr->base_selector, HBA_HC_Capabilities) & 0x1F) + 1;
  
  /* 
   * 2. Determine which ports are implemented by the HBA, by reading the PI register. 
   *    This bit map value will aid software in determining how many ports are available 
   *    and which port registers need to be initialized.
   */
  dword = _farpeekl(cntrlr->base_selector, HBA_HC_Ports_implemented);
  drive = 0;
  for (i=0; i<HBA_MAX_PORTS; i++) {
    if (dword & (1 << i)) {
      memset(&cntrlr->drive[drive], 0, sizeof(struct S_ATA));
      cntrlr->drive[drive].cntrlr = cntrlr;
      cntrlr->drive[drive].drv = i;
      drive++;
    }
  }
  
  // if drive > num_drives, we have a problem
  if (drive > num_drives)
    printf("There were more bits set in the PI register than the controller said was available...\n");
  
  // we calculated the number of implemented ports in the loop above
  num_drives = drive;
  
  // loop through each port to see if anything is available
  for (drive=0; drive<num_drives; drive++) {
    struct S_ATA *ata = &cntrlr->drive[drive];
    
    printf("\n Drive #%i (on port #%i)\n", drive, ata->drv);
    
    /* 
     * 3. Ensure that the controller is not in the running state by reading and examining 
     *    each implemented port’s PxCMD register. If PxCMD.ST, PxCMD.CR, PxCMD.FRE and 
     *    PxCMD.FR are all cleared, the port is in an idle state. Otherwise, the port is 
     *    not idle and should be placed in the idle state prior to manipulating HBA and port
     *    specific registers.
     */
    dword = _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD));
    if (HBA_PORT_CMD_ST(dword) || HBA_PORT_CMD_CR(dword) || HBA_PORT_CMD_FRE(dword) || HBA_PORT_CMD_FR(dword)) {
      if (!sata_stop_cmd(ata)) {
        // if the port did not successfully stop, reset the port.
        if (verbose) printf("Sending COMRESET to port.\n");
        if (!sata_port_reset(ata)) {
          printf("Port never became functional...\n");
          continue;
        }
      }
    }
    
    if (verbose) {
      printf("Initial Port values:\n");
      printf("     CLB: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCLB)));
      printf("    CLBU: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCLBU)));
      printf("      FB: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxFB)));
      printf("     FBU: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxFBU)));
      printf("      IS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIS)));
      printf("      IE: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIE)));
      printf("     CMD: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)));
      printf("     TFD: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxTFD)));
      printf("     SIG: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSIG)));
      printf("    SSTS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSSTS)));
      printf("    SCTL: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSCTL)));
      printf("    SERR: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSERR)));
      printf("    SACT: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSACT)));
      printf("      CI: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCI)));
      printf("    SNTF: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSNTF)));
      printf("     FBS: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxFBS)));
      printf("  DEVSLP: 0x%08X\n", _farpeekl(cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxDEVSLP)));
    }
    
    switch (sata_get_port_type(ata)) {
      case SATA_SIG_NONE:
        printf("No Device Attached\n");
        continue;  // move to next port (loop to for())
      case SATA_SIG_NON_ACT:
        printf("Found Non-Active Device\n");
        continue;  // move to next port (loop to for())
      case SATA_SIG_ATA:
        printf("Found SATA device\n");
        ata->atapi_type = ATA_TYPE_SATA;
        ata->atapi = FALSE;
        break;
      case SATA_SIG_ATAPI:
        printf("Found SATAPI device\n");
        ata->atapi_type = ATA_TYPE_SATAPI;
        ata->atapi = TRUE;
        break;
      case SATA_SIG_SEMB:
        printf("Found Enclosure Management Bridge\n");
        continue;  // move to next port (loop to for())
      case SATA_SIG_PM:
        printf("Found Port Multiplier\n");
        continue;  // move to next port (loop to for())
      default:
        printf("Unknown Device Attached\n");
        continue;  // move to next port (loop to for())
    }
    if (!sata_port_initialize(ata))
      continue;
    
    if (verbose) printf("atapi = %i  atapi_type = %i\n", ata->atapi, ata->atapi_type);
    
    // get ready and send identify command
    if (verbose) printf("Sending Identify Command\n");
    
    // send the identify device command (ATA(PI))
    if (sata_tx_rx_data(ata, ATA_DIR_RECV, (ata->atapi) ? ATA_CMD_ID_PACKET_DEVICE : ATA_CMD_ID_DEVICE, FALSE, 0, 512, 0, NULL, (void *) &ata->info)) {
      //printf("Dump SATA Identify:\n");
      //dump(&ata->info, 512);
      
      ata->removable = (ata->info.get_conf & (1<<7)) ? TRUE : FALSE;
      ata->command_set = (ata->info.get_conf & (0x1F << 8)) >> 8;
      
      if (verbose) printf("ata->atapi = %i, ata->removable = %i\n", ata->atapi, ata->removable);
      
      // check to make sure Identify Block is valid
      if (!ata_check_identify(&ata->info))
        return FALSE;
      
      // if bit 2 of WORD 0 is still set, give error
      if (ata->info.get_conf & (1<<2))
        printf("Word 0, bit 2, \"Response Incomplete\" still set...");
      
      // "cleanup" some of the info
      fix_ide_string(ata->info.model_num, 40);
      fix_ide_string(ata->info.serial_num, 20);
      fix_ide_string(ata->info.firmware_rev, 8);
      
      // get highest ata(pi) version
      ata->version = ata_highest_ata_version(ata->info.major_ver, ata->atapi);
      if (verbose) printf("Highest ATA version = %i\n", ata->version);
      
      // does the drive support DMA transfers
      ata->dma_supported = (ata->info.caps & (1 << 8)) ? TRUE : FALSE;
      if (!ata->dma_supported && verbose)
        printf("Drive does not support DMA transfers...\n");
      
      // does the drive support LBA addressing
      ata->lba_supported = (ata->info.caps & (1 << 9)) ? TRUE : FALSE;
      if (!ata->lba_supported && verbose)
        printf("Drive does not support LBA addressing...\n");
      
      // verify the ata/atapi status with word 85, bit 4.
      if (ata->version >= 6) {
        if (ata->atapi && !(ata->info.command_set4 & (1 << 4))) {
          printf("Error: Reset returned ATAPI, but word 85, bit 4 == 0\n");
          return FALSE;
        }
        if (!ata->atapi && (ata->info.command_set4 & (1 << 4))) {
          printf("Error: Reset returned ATA, but word 85, bit 4 == 1\n");
          return FALSE;
        }
      }
      
      // see if 48-bit capable
      if (ata->lba_supported) {
        ata->large_cap = ((ata->version >= 6) && (ata->info.command_set2 & (1<<10)) && (ata->info.command_set5 & (1<<10)) && ata->info.lba_capacity2);
        if (verbose) printf("Is 48-bit capable = %i\n", ata->large_cap);
      } else
        ata->large_cap = FALSE;
      
      if (!ata->atapi) {
        // update CHS parameters
        if (ata->version <= 2) {
          ata->cylinders = ata->info.cylinders;
          ata->heads = (bit8u) ata->info.heads;
          ata->sect_track = (bit8u) ata->info.sects_track;
        } else if (ata->info.rest_valid & 1) {
          ata->cylinders = ata->info.num_cur_cylinders;
          ata->heads = (bit8u) ata->info.num_cur_heads;
          ata->sect_track = (bit8u) ata->info.num_cur_sect_track;
        } else {
          // ata6+ doesn't show the CHS values.
          // Calculate Cyls from capacity and assume h=16 & spt=63
          ata->cylinders = (bit32u) (ata->capacity / 1008);
          ata->heads = 16;
          ata->sect_track = 63;
        }
      }
      
      struct S_BLOCK_STATUS status;
      sata_get_status(ata, &status);
      if (status.inserted) {
        // determine the capacity of the drive
        sata_capacity(ata, &ata->capacity);
        if (verbose) printf("Returned capaticy: %" LL64BIT "i sectors\n", ata->capacity);
        
        // Now demonstrate how to read a sector from the drive
        bool success = FALSE;
        int cnt = 1;  // how many sectors to read
        bit64u lba = 0; // starting LBA
        if (status.is_optical_disc)
          lba = ATA_DEV_TYPE_OPTICAL_BASE;
        int bytes_per_sector = (ata->atapi) ? 4096 : 512;  // for this example, we assume if atapi it has 4096-byte sectors
        buffer = (bit8u *) calloc(cnt * bytes_per_sector, 1);
        if (ata->atapi) {
          bit8u packet[ATAPI_MAX_PACKET_SIZE];
          ata_create_rw_packet(ata->command_set, packet, ATA_DIR_RECV, lba, cnt);
          success = sata_tx_rx_data(ata, ATA_DIR_RECV, 0, TRUE, lba, bytes_per_sector * cnt, 0, packet, buffer);
        } else
          success = sata_tx_rx_data(ata, ATA_DIR_RECV, CMD_READ_DMA_EXT, FALSE, lba, bytes_per_sector * cnt, cnt, NULL, buffer);
        
        if (success) {
          if (verbose) {
            printf("Dump of Read Sector(s):\n");
            dump(buffer, bytes_per_sector * cnt);
          }
        } else
          printf("Didn't read sector(s):\n");
        free(buffer);
      }
    } else
      printf("Didn't Identify drive:\n");
    
    // we're done with the port, so free it
    sata_free_port(ata);
  }
  
  return TRUE;
}

/*
 * With version 1.2.0 and higher, the controller will have bit 0 set in the CAP2 register if BIOS/OS Handoff is supported.
 *  If this bit is zero, or the version is less than 1.2.0, you must assume that you already have ownership and may 
 *  skip this check.
 * If the CAP2.BOH bit is set, the BOHC register at offset 28h is valid and you may use it to gain ownership of the controller.
 * 
 * To gain ownership of the controller, set the BOHC.OOS bit and wait for the BOHC.BOS bit to be clear.
 * 
 * So that you don’t wait indefinitely for a faulty controller, after setting the BOHC.OOS bit wait 25mS.
 *  If the BOHC.BB bit has become set, you must wait a minimum of 2,000mS for the BIOS to be done.  
 *  If after this 2,000mS wait, the BOHC.BOS and BOHC.BB bits are not clear, you have a faulty controller.
 *  If they are both clear, you may assume ownership and continue.
 *
 * If the BOHC.BB bit is not set after 25mS, and the BOHC.BOS bit is clear, you may assume ownership and continue.
 *
 * Physical Page 108 in the AHCI 1.3.1 specs (pdf page 116)
 *
 * Returns TRUE if we gained ownership of the controller (or BOH is not supported)
 */
bool sata_gain_ownership(struct S_ATA_CNTRLR *cntrlr) {
  bit32u version, cap2, bohc;
  
  // only supported in 1.2.0 and above
  version = _farpeekl(cntrlr->base_selector, HBA_HC_Version);
  if (version >= 0x00010200) {
    // only supported if CAP2.BOH is set
    cap2 = _farpeekl(cntrlr->base_selector, HBA_HC_Capabilities_ext);
    if (cap2 & (1 << 0)) {
      // set the OOS bit
      bohc = _farpeekl(cntrlr->base_selector, HBA_HC_Bohc);
      _farpokel(cntrlr->base_selector, HBA_HC_Bohc, bohc | (1 << 1));
      // wait 25ms
      mdelay(25);
      // if after 25ms the BB bit is set, we need to wait 2,000ms (2 seconds)
      bohc = _farpeekl(cntrlr->base_selector, HBA_HC_Bohc);
      if (bohc & (1 << 4))
        mdelay(2000);
      // if after the wait (25ms or 2000ms), if the BB and BOS bits are clear,
      //  we have ownership
      bohc = _farpeekl(cntrlr->base_selector, HBA_HC_Bohc);
      if ((bohc & ((1 << 4) | (1 << 1) | (1 << 0))) != (1 << 1))
        return FALSE;
      
      // clear the OOC bit
      _farpokel(cntrlr->base_selector, HBA_HC_Bohc, bohc);
    }
  }
  
  return TRUE;
}

// Check device type
bit32u sata_get_port_type(struct S_ATA *ata) {
  bit32u ssts = _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSSTS));
  
	if (HBA_PORT_SSTS_DET(ssts) != HBA_PORT_PxSSTS_DET_PRES_PHY)
		return SATA_SIG_NONE;
	if (HBA_PORT_SSTS_IPM(ssts) != HBA_PORT_PxSSTS_IPM_ACTIVE)
		return SATA_SIG_NON_ACT;
  
  return _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSIG));
}

/*
 * 5. For each implemented port, system software shall allocate memory for and program:
 *     PxCLB (and PxCLBU if CAP.S64A is set to ‘1’)
 *     PxFB (and PxFBU if CAP.S64A is set to ‘1’)
 *    It is good practice for system software to ‘zero-out’ the memory allocated and referenced 
 *    by PxCLB and PxFB. After setting PxFB and PxFBU to the physical address of the FIS receive
 *    area, system software shall set PxCMD.FRE to ‘1’.
 */
bool sata_port_initialize(struct S_ATA *ata) {
  int i, slot, size;
  
  ata->cntrlr->drive[ata->drv].selector = -1;
  
  size = (HBA_MAX_CMD_SLOTS * sizeof(struct S_HBA_CMD_LIST)) +   // Command list is 32 entries of 32 bytes each
         (HBA_MAX_CMD_SLOTS * sizeof(struct S_HBA_CMD_TABLE)) +  // Command Table is 32 entries of 256 bytes each (256 allows 8 prdt entries) (hardware allows up to 65535 entries)
         sizeof(struct S_HBA_RCV_FIS) +                          // FIS - 1 entry of 256 bytes
         (CMD_LIST_ALIGNMENT - 1);                               // 1023 extra bytes so we can align on a 1024 byte alignment
  
  // Allocate memory for the Command List, Command Table, and Recv FIS
  // *** Please note that this example only allocates enough memory for 8 entries in the PRD Table ***
  if ((__dpmi_allocate_dos_memory((size + 15) / 16, &ata->cntrlr->drive[ata->drv].selector) == -1) ||
      (__dpmi_get_segment_base_address(ata->cntrlr->drive[ata->drv].selector, &ata->cntrlr->drive[ata->drv].phy_address) == -1)) {
    printf("Error allocating DOS memory\n");
    return FALSE;
  }
  
  // calculate addresses
  //  cmd_list_addr must be 1024 aligned
  //  cmd_table_addr must be 128 byte aligned
  //  fis_add must be 256 byte aligned
  ata->cntrlr->drive[ata->drv].cmd_list_addr  = ((ata->cntrlr->drive[ata->drv].phy_address + (CMD_LIST_ALIGNMENT - 1)) & ~(CMD_LIST_ALIGNMENT - 1));
  ata->cntrlr->drive[ata->drv].cmd_table_addr =  (ata->cntrlr->drive[ata->drv].cmd_list_addr + (HBA_MAX_CMD_SLOTS * sizeof(struct S_HBA_CMD_LIST)));
  ata->cntrlr->drive[ata->drv].fis_add        =  (ata->cntrlr->drive[ata->drv].cmd_table_addr + (HBA_MAX_CMD_SLOTS * sizeof(struct S_HBA_CMD_TABLE)));
  
  // create the command list's table pointers
  for (slot=0; slot<HBA_MAX_CMD_SLOTS; slot++) {
    // write the address of each table to the physical memory we use for the controller
    ata->cntrlr->drive[ata->drv].cmd_list[slot].dword0 = CMD_LIST_prdtl(8);  // default to 8
    ata->cntrlr->drive[ata->drv].cmd_list[slot].prdbc = 0;
    ata->cntrlr->drive[ata->drv].cmd_list[slot].ctba = ata->cntrlr->drive[ata->drv].cmd_table_addr + (slot * sizeof(struct S_HBA_CMD_TABLE));
    ata->cntrlr->drive[ata->drv].cmd_list[slot].ctbau = 0;
  }
  
  // clear the memory
  for (i=0; i<size; i++)
    _farpokeb(ata->cntrlr->drive[ata->drv].selector, i, 0);
  
  // make sure the command engine is stopped
  sata_stop_cmd(ata);
  
  // write the (physical) address of the command list to the controller
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCLB), ata->cntrlr->drive[ata->drv].cmd_list_addr);
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCLBU), 0);	
  
  // write the address of the FIS to the controller
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxFB), ata->cntrlr->drive[ata->drv].fis_add);
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxFBU), 0);	
  
  sata_start_cmd(ata); // Start command engine
  
  return TRUE;
}

void sata_free_port(struct S_ATA *ata) {
  
  sata_stop_cmd(ata);
  
  if (ata->cntrlr->drive[ata->drv].selector != -1)
    __dpmi_free_dos_memory(ata->cntrlr->drive[ata->drv].selector);  // free the memory
}

/* SATA: Start command engine
 * 6. For each implemented port, clear the PxSERR register, by writing ‘1s’ to each implemented 
 *    bit location.
 * 7. Determine which events should cause an interrupt, and set each implemented port’s PxIE 
 *    register with the appropriate enables. To enable the HBA to generate interrupts, system
 *    software must also set GHC.IE to a ‘1’.
 * Note: Due to the multi-tiered nature of the AHCI HBA’s interrupt architecture, system software
 *    must always ensure that the PxIS (clear this first) and IS.IPS (clear this second) registers 
 *    are cleared to ‘0’ before programming the PxIE and GHC.IE registers. This will prevent any 
 *    residual bits set in these registers from causing an interrupt to be asserted.
 */
void sata_start_cmd(struct S_ATA *ata) {
  int timer;
  bit32u dword;
  
  // we need to clear the DRQ and BSY bits before we start the engine
  if (_farpeekl(ata->cntrlr->base_selector, HBA_HC_Capabilities) & (1 << 24)) {
    dword = _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD));
    _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD), dword | (1 << 3));
    
    // wait for bit 3 to be come clear
    timer = 500;
    while (_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)) & (1 << 3)) {
      mdelay(1);
      if (--timer == 0) {
        printf("*** The controller never cleared bit 3 for us...\n");
        break;
      }
    }
  }
  
	// Set FRE (bit4) and ST (bit0)
  dword = _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD));
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD),
    dword | HBA_PORT_CMD_fre(1) | HBA_PORT_CMD_st(1));
  
  // clear SERR register
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSERR), 0x07FF0F03);
  
  // clear the IS register and set the IE register, setting GHC.IE if we want an interrupt to occur.
  // Since I don't use interrupts for this utility, I will make sure they are all turned off
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIS), 0xFFFFFFFF);
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIE), 0x00000000);
  _farpokel(ata->cntrlr->base_selector, HBA_HC_Glob_host_cntrl,
    _farpeekl(ata->cntrlr->base_selector, HBA_HC_Glob_host_cntrl) & ~(1 << 1));
}
 
/* SATA: Stop command engine
 *  System software places a port into the idle state by clearing PxCMD.ST and waiting for 
 *  PxCMD.CR to return ‘0’ when read. Software should wait at least 500 milliseconds for 
 *  this to occur. If PxCMD.FRE is set to ‘1’, software should clear it to ‘0’ and wait 
 *  at least 500 milliseconds for PxCMD.FR to return ‘0’ when read.  If PxCMD.CR or PxCMD.FR
 *  do not clear to ‘0’ correctly, then software may attempt a port reset or a full HBA reset
 *  to recover.
 */
bool sata_stop_cmd(struct S_ATA *ata) {
  int timer;
  
  // Clear ST (bit0)
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD),
    _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)) & ~HBA_PORT_CMD_st(1));
  
  // Wait until CR (bit15) is cleared or 500 milliseconds
  timer = 500;
  while (_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)) & HBA_PORT_CMD_cr(1)) {
    mdelay(1);
    if (--timer == 0)
      return FALSE;
  }
  
	// If FRE (bit4) is set, clear it 
  if (HBA_PORT_CMD_FRE(_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)))) {
    _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD),
      _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)) & ~HBA_PORT_CMD_fre(1));
    
	  // Wait until FR (bit14) is cleared or 500 milliseconds
    timer = 500;
    while (HBA_PORT_CMD_FR(_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCMD)))) {
      mdelay(1);
      if (--timer == 0)
        return FALSE;
    }
  }
  
  return TRUE;
}

/* Reset a port
 *  Cause a port reset (COMRESET) by writing 1 to the PxSCTL.DET field to invoke a COMRESET on the 
 *  interface and start a re-establishment of Phy layer communications. Software shall wait at least 
 *  1 millisecond before clearing PxSCTL.DET to 0.  This ensures that at least one COMRESET signal is 
 *  sent over the interface. After clearing PxSCTL.DET to 0, software should wait for communication to 
 *  be re-established as indicated by PxSSTS.DET being set to 3. Then software should write all 1s to the 
 *  PxSERR register to clear any bits that were set as part of the port reset.
 */
bool sata_port_reset(struct S_ATA *ata) {
  int timer;
  
  // HBA_PORT_PxSCTL.DET = 1
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSCTL),
    (_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSCTL)) & 0xFFFFFFF0) | 0x01);
  
  // wait at least 1 ms
  mdelay(1);
  
  // HBA_PORT_PxSCTL.DET = 0
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSCTL),
    _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSCTL)) & 0xFFFFFFF0);
  
  timer = 500;
  while (HBA_PORT_SSTS_DET(_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSSTS))) != HBA_PORT_PxSSTS_DET_PRES_PHY) {
    mdelay(1);
    if (timer-- == 0)
      return FALSE;
  }
  
  // clear SERR register
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSERR), 0xFFFFFFFF);  // 0x07FF0F03  ??
  
  return TRUE;
}

bool sata_tx_rx_data(const struct S_ATA *ata, const bool dir, const bit8u command, const bool is_atapi,
                     bit64u lba, const int buflen, int count, bit8u *packet, const void *buffer) {
  int spin, slot, i, sel, prdts;
  struct S_HBA_CMD_TABLE cmd_table;
  bit32u phy_address;
  bool ret = FALSE;
  
  // calculate the count of PRD Table Entries
  //  each entry can do up to 4Meg bytes per transfer
  // This assumes that the memory is physically consecutive, one page
  //  right after the other.  You will need to modify your driver
  //  to calculate this as 4k pages or what ever length you have. 
  prdts = (buflen / (4 * 1024 * 1024)) + 1;
  
  // we only allow 8 PRDT entries with this demonstration.
  // You would need to change 
  //   struct S_HBA_PRDT_ENTRY prdt_entry[8];
  // to
  //   struct S_HBA_PRDT_ENTRY prdt_entry[x];
  // where 'x' is the count you wish to allow (up to 65535)
  // or you need to allocate the memory instead.
  if (prdts > 8) {
    printf("This demonstration only allows for 8 PRDT entries.\n");
    return FALSE;
  }
  
  // Clear pending interrupt bits
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIS), 0xFFFFFFFF);
  
  slot = sata_find_cmdslot(ata);
  if (slot == -1)
    return FALSE;
  
  // need to allocate at least buflen bytes of physical memory
  if ((__dpmi_allocate_dos_memory((buflen + 15) / 16, &sel) == -1) ||
      (__dpmi_get_segment_base_address(sel, &phy_address) == -1)) {
    printf("Error allocating DOS memory\n");
    return FALSE;
  }
  
	// command list entry
  ata->cntrlr->drive[ata->drv].cmd_list[slot].prdbc = 0;
  ata->cntrlr->drive[ata->drv].cmd_list[slot].dword0 = CMD_LIST_prdtl(prdts)  |    // PRDT entries count
                                              CMD_LIST_a(is_atapi)            |    // ATAPI command ?
                                              CMD_LIST_w(dir == ATA_DIR_SEND) |    // Write bit  (0 = Read from device, 1 = write to device)
                                              (sizeof(struct S_FIS_REG_H2D) / sizeof(bit32u));  // Command FIS size
  // store it to physical memory
  dosmemput(&ata->cntrlr->drive[ata->drv].cmd_list[slot], sizeof(struct S_HBA_CMD_LIST), 
    ata->cntrlr->drive[ata->drv].cmd_list_addr + (slot * sizeof(struct S_HBA_CMD_LIST)));
  
  // command table for this slot
  memset(&cmd_table, 0, sizeof(struct S_HBA_CMD_TABLE));
  
	struct S_FIS_REG_H2D *cmd_fis = (struct S_FIS_REG_H2D *) &cmd_table.cfis;
  cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->flags = FIS_REG_H2D_c(1);   // Command
  if (is_atapi) {
    // is ATAPI command, so initialize Command Table area
    memcpy(cmd_table.acmd, packet, 16); 
    cmd_fis->command = ATA_CMD_PACKET;
    cmd_fis->features = 0x01;  // DMA
    // make sure LBA and count == 0
    lba = 0;
    count = 0;
  } else {
    cmd_fis->command = command;
    cmd_fis->features = 0x00;
  }
  
  /* If we are using 28-bit LBA's, we use LBA0, LBA1, LBA2, and the
   *  lower half of dev_head.
   * If we are using 48-bit LBA's, we use LBA0, 1, 2, 3, 4, and 5.
   * It won't matter that we set the lower half of Dev_Head when
   *  using 48-bit LBA's, so there is no need for an if() statement
   *  here, or some other way to determine if we are using 48-bit LBA's.
   *  A 28-bit command will only use LBA0, 1, 2, and half of dev_head.
   *  A 48-bit command will only use LBA0, 1, 2, 3, 4, and 5, not dev_head.
   */
	cmd_fis->lba_0 = (bit8u) ((lba & (bit64u) 0x0000000000FF) >>  0);
	cmd_fis->lba_1 = (bit8u) ((lba & (bit64u) 0x00000000FF00) >>  8);
	cmd_fis->lba_2 = (bit8u) ((lba & (bit64u) 0x000000FF0000) >> 16);
  
  cmd_fis->dev_head = 0xA0
    | ATA_DH_ISLBA
    | ((bit8u) ((lba & (bit64u) 0x00000F000000) >> 24));
  
	cmd_fis->lba_3 = (bit8u) ((lba & (bit64u) 0x0000FF000000) >> 24);
	cmd_fis->lba_4 = (bit8u) ((lba & (bit64u) 0x00FF00000000) >> 32);
	cmd_fis->lba_5 = (bit8u) ((lba & (bit64u) 0xFF0000000000) >> 40);
  cmd_fis->features_exp = 0x00;
  
	cmd_fis->sect_count_low  = (bit8u) ((count & 0x00FF) >> 0);
	cmd_fis->sect_count_high = (bit8u) ((count & 0xFF00) >> 8);
  cmd_fis->reserved = 0x00;
  cmd_fis->control = 0x08; // setting bit 3 ensures that this FIS gets sent since the Device Control
                           //  register will have bit 3 cleared.  A FIS will only be sent if the
                           //  Command register and command field are different, or the Device Control
                           //  register and the control field are different.
  
  cmd_fis->resv = 0x00000000;
  
  // 4Meg bytes per PRDT Entry
  int cnt = buflen;
  for (i=0; i<prdts; i++) {
    cmd_table.prdt_entry[i].dba = phy_address;
    cmd_table.prdt_entry[i].dbau = 0;
    cmd_table.prdt_entry[i].resv = 0;
    if (cnt > (4 * 1024 * 1024)) {
      cmd_table.prdt_entry[i].dword3 = PRDT_ENTRY_int(0) | PRDT_ENTRY_dbc(4 * 1024 * 1024); // 4Meg bytes
      phy_address += (4 * 1024 * 1024);
      cnt -= (4 * 1024 * 1024);
    } else {
      cmd_table.prdt_entry[i].dword3 = PRDT_ENTRY_int(0) | PRDT_ENTRY_dbc(cnt);
      phy_address += cnt;
      cnt = 0;
    }
	}
  
  // store it to physical memory
  dosmemput(&cmd_table, sizeof(struct S_HBA_CMD_TABLE), ata->cntrlr->drive[ata->drv].cmd_list[slot].ctba);
  
	// wait until the port is no longer busy before issuing a new command
  spin = 0;
  while ((_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxTFD)) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) && (spin < 1000000))
    spin++;
  
  if (spin == 1000000) {
    printf("Port is hung\n");
    return FALSE;
  }
  
  // Issue command
  _farpokel(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCI), (1 << slot));
  
  // assume good transfer
  ret = TRUE;
  
  // Wait for completion
  while (1)	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCI)) & (1 << slot)) == 0) 
			break;
		// Task file error
    if (HBA_PORT_IS_TFES(_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIS)))) {
      printf("Disk error\n");
      ret = FALSE;
      break;
    }
  }
  
	// Check again
  if (HBA_PORT_IS_TFES(_farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxIS)))) {
    printf("Disk error\n");
    ret = FALSE;
  }
  
  // if no errors, copy the data from physical memory to our buffer
  if (ret) {
    // store it to physical memory
    for (i=0; i<prdts; i++)
      dosmemget(cmd_table.prdt_entry[i].dba, PRDT_ENTRY_DBC(cmd_table.prdt_entry[i].dword3), (void *) buffer);
  }
  
  __dpmi_free_dos_memory(sel);  // free the memory
  
  return ret;
}

// Find an empty command slot by reading the PxCI and PxSACT registers for the port.
// An empty command slot has its respective bit cleared to ‘0’ in both the PxCI 
// and PxSACT registers.
//  (The PxSACT register is used for Native Queue Commands, but still shown here for completeness)
int sata_find_cmdslot(const struct S_ATA *ata) {
  int i;
  
  // If not set in SACT and CI, the slot is free
  bit32u slots = _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxSACT)) |
    _farpeekl(ata->cntrlr->base_selector, HBA_PORT_ADDR(ata->drv, HBA_PORT_PxCI));
  
  for (i=0; i<ata->cntrlr->command_slots; i++) {
    if ((slots & 1) == 0)
      return i;
    slots >>= 1;
  }
  
  return -1;
}

bool sata_capacity(struct S_ATA *ata, bit64u *lbas) {
  
  struct S_BLOCK_STATUS status;
  struct CAPS caps;
  
  if ((ata == NULL) || (lbas == NULL))
    return FALSE;
  
  if (!ata->atapi) {
    // Even though ATA1&2 have the words at 60-61, most drives below ATA3 didn't use it
    if (ata->version <= 2)
      *lbas = (ata->cylinders * ata->sect_track * ata->heads);
    else if (ata->large_cap)
      *lbas = ata->info.lba_capacity2;
    else
      *lbas = ata->info.lba_capacity;
  } else {
    sata_get_status(ata, &status);
    if (status.inserted) {
      satapi_read_capacity(ata, &caps, 8);
      *lbas = ENDIAN_32U(caps.lba);
      if (verbose) printf("Sector Size returned: %i\n", ENDIAN_32U(caps.size & 0x00FFFFFF));
    } else
      *lbas = 0;
  }
  
  return TRUE;
}

bool sata_get_status(struct S_ATA *ata, struct S_BLOCK_STATUS *status) {
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  bit8u error, buffer[512];
  bool ret = FALSE;
  int t;
  
  // if status = null, return error
  if (!status)
    return FALSE;
  memset(status, 0, sizeof(struct S_BLOCK_STATUS));
  
  if (ata->removable) {
    memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
    packet[0] = ATAPI_CMD_READ_DISC_INFO;
    packet[1] = (0 << 3) | (0 << 0);  // data type = 0
    packet[7] = 0;
    packet[8] = 32;  // at most 32 bytes
    if (sata_tx_rx_data(ata, ATA_DIR_RECV, 0, TRUE, 0, 32, 0, packet, buffer)) {
      status->inserted   = TRUE;
      status->write_prot = ((buffer[2] & (1 << 4)) == 0);
      if (satapi_get_config(ata, buffer, 512, MEDIA_PROFILE_0008)) {
        struct S_GET_CONFIG_HDR *config = (struct S_GET_CONFIG_HDR *) buffer;
        struct S_GET_CONFIG_DESC *desc = (struct S_GET_CONFIG_DESC *) (buffer + sizeof(struct S_GET_CONFIG_HDR));
        if (verbose) printf("atapi_get_config() returned TRUE (%02Xh)\n", ENDIAN_16U(config->current));
        while (ENDIAN_16U(desc->code) < 0x0111) {
          switch (ENDIAN_16U(desc->code)) {
            case 3:   // The medium may be removed from the device
              status->removable = TRUE;
              break;
            case 30:  // CD Read-The ability to read CD specific structures
              break;
            case 31:  // DVD Read-The ability to read DVD specific structures
              status->dvd_capable = TRUE;
              break;
            case 32:  // Random Writable-Write support for randomly addressed writes
              status->writable = TRUE;
              break;
            case 0x0108: // drive has unique identifier
              // identifier following descriptor header
              break;
            case 0x0109: // has ability to return unique media serial number
              // able to read serial number via command ABh sub-command 01h.
              status->read_serial_num = TRUE;
              break;
            default:
              ;
          }
          desc = (struct S_GET_CONFIG_DESC *) ((bit8u *) desc + 4 + desc->add_len);
        }
        status->medium_type = ENDIAN_16U(config->current);
        status->inserted = (status->medium_type > MEDIA_PROFILE_0000);
        ret = TRUE;
        status->is_optical_disc = (((status->medium_type >= MEDIA_PROFILE_0008) && (status->medium_type <= MEDIA_PROFILE_000A)) ||
                                   ((status->medium_type >= MEDIA_PROFILE_0010) && (status->medium_type <= MEDIA_PROFILE_0018)) ||
                                   ((status->medium_type >= MEDIA_PROFILE_001A) && (status->medium_type <= MEDIA_PROFILE_001B)) ||
                                   ((status->medium_type >= MEDIA_PROFILE_002A) && (status->medium_type <= MEDIA_PROFILE_002B)) ||
                                   ((status->medium_type >= MEDIA_PROFILE_0040) && (status->medium_type <= MEDIA_PROFILE_0043)) ||
                                   ((status->medium_type >= MEDIA_PROFILE_0050) && (status->medium_type <= MEDIA_PROFILE_0053)) ||
                                    (status->medium_type == MEDIA_PROFILE_0058) ||
                                    (status->medium_type == MEDIA_PROFILE_005A));
        // double check removable
        //  A lot of devices don't have the removable feature entry...
        // However, if we got here, we know it is removable since this whole
        //  block of code is only executed if ata->removable is set...
        if (ata->removable != status->removable) {
          if (verbose) printf("Error: ata->removable != status.removable\n");
          status->removable = ata->removable;
        }
      }
    } else {
      status->inserted = FALSE;
      ret = FALSE;
    }
  } else {
    status->inserted = TRUE;
    status->changed = FALSE;
    ret = TRUE;
  }
  
  if (verbose) printf("sata_get_status returning: inserted = %i, changed = %i, write_prot = %i\n",
    status->inserted, status->changed, status->write_prot);
  
  return ret;
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool satapi_read_capacity(struct S_ATA *ata, void *buf, bit16u buflen) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[0] = ATAPI_CMD_READ_CAPACITY;
  packet[1] = (0 << 5) | 0 | (0<<0);  // LUN and RelAddr = 0
  
  return sata_tx_rx_data(ata, ATA_DIR_RECV, 0, TRUE, 0, buflen, 0, packet, buf);
}

/*
 * This function assumes command set = 5.  We need to call a function passing ata->command_set.
 *   For an example, see ata_create_rw_packet()
 */
bool satapi_get_config(struct S_ATA *ata, void *buf, bit16u buflen, bit16u feature_num) {
  
  bit8u packet[ATAPI_MAX_PACKET_SIZE];
  memset(packet, 0, ATAPI_MAX_PACKET_SIZE);
  packet[ 0] = ATAPI_CMD_GET_CONFIGURATION;
  packet[ 1] = 1;
  packet[ 2] = ((feature_num >> 8) & 0xFF);
  packet[ 3] = ((feature_num >> 0) & 0xFF);
  packet[ 7] = ((buflen >> 8) & 0xFF);
  packet[ 8] = ((buflen >> 0) & 0xFF);
  return sata_tx_rx_data(ata, ATA_DIR_RECV, 0, TRUE, 0, buflen, 0, packet, buf);
}
