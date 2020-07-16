/*
 *                             Copyright (c) 1984-2020
 *                              Benjamin David Lunt
 *                             Forever Young Software
 *                            fys [at] fysnet [dot] net
 *                              All rights reserved
 * 
 * Redistribution and use in source or resulting in  compiled binary forms with or
 * without modification, are permitted provided that the  following conditions are
 * met.  Redistribution in printed form must first acquire written permission from
 * copyright holder.
 * 
 * 1. Redistributions of source  code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in printed form must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 3. Redistributions in  binary form must  reproduce the above copyright  notice,
 *    this list of  conditions and the following  disclaimer in the  documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE, DOCUMENTATION, BINARY FILES, OR OTHER ITEM, HEREBY FURTHER KNOWN
 * AS 'PRODUCT', IS  PROVIDED BY THE COPYRIGHT  HOLDER AND CONTRIBUTOR "AS IS" AND
 * ANY EXPRESS OR IMPLIED  WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED
 * WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  OWNER OR CONTRIBUTOR BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  CAUSED AND ON
 * ANY  THEORY OF  LIABILITY, WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR  TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  ANY WAY  OUT OF THE USE OF THIS
 * PRODUCT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  READER AND/OR USER
 * USES AS THEIR OWN RISK.
 * 
 * Any inaccuracy in source code, code comments, documentation, or other expressed
 * form within Product,  is unintentional and corresponding hardware specification
 * takes precedence.
 * 
 * Let it be known that  the purpose of this Product is to be used as supplemental
 * product for one or more of the following mentioned books.
 * 
 *   FYSOS: Operating System Design
 *    Volume 1:  The System Core
 *    Volume 2:  The Virtual File System
 *    Volume 3:  Media Storage Devices
 *    Volume 4:  Input and Output Devices
 *    Volume 5:  ** Not yet published **
 *    Volume 6:  The Graphical User Interface
 *    Volume 7:  ** Not yet published **
 *    Volume 8:  USB: The Universal Serial Bus
 * 
 * This Product is  included as a companion  to one or more of these  books and is
 * not intended to be self-sufficient.  Each item within this distribution is part
 * of a discussion within one or more of the books mentioned above.
 * 
 * For more information, please visit:
 *             http://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  SBLASTER.EXE
 *   Will play a given .wav file to the Sound Blaster
 *
 *  Assumptions/prerequisites:
 *  - This example is coded and compiled for 16-bit DOS using a 16-bit compiler.
 *  - There is no need for a DPMI driver.
 *  - All variables and numbers are 16-bit.
 *     char = bit8s = byte = 8-bits  (AL)
 *      int = bit16s = word = 16-bits (AX)
 *     long = bit32s = dword = 32-bits (DX:AX)
 *
 *  Last updated: 15 July 2020
 *
 *  Compile using MS QuickC 2.5
 *
 *  Usage:
 *    sblaster filename [-single | -auto ]
 */

#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/ctype.h"
#include "../include/pic.c"
#include "../include/bits.c"
#include "sblaster.h"

// leave these global so that we can check them through out the code
bit8u major, minor;

// global irq number
int irq_num;

int main(int argc, char *arg[]) {
  char filename[128];
  bool auto_init = 0;
  
  bit8u byte0, byte1, byte2, bit;
  bit8u esp_major, esp_minor;
  bit16u base;
  int dma, dma16;
  
  printf("Detect Sound Blaster.                v1.00.00\n"
         "Forever Young Software -- Copyright 1984-2020\n\n");
  
  base = detect_base();
  if (base == 0) {
    printf("No Sound Blaster detected...\n");
    return -1;
  }
  printf("Found Sound Blaster at io: 0x%03X\n", base);
  
  // get DSP ID
  if (get_dsp_id(base, 0x55))
    printf("DSP returned ID, so is at least 2.0\n");
  
  // test test commands
  if (test_write_read(base, 0x55))
    printf("Test commands returned okay.\n");
  
  // get DSP version
  if (get_dsp_version(base, &major, &minor)) {
    printf("Found DSP version %i.%i\n", major, minor);
    if (major == 2) {
      // the Media Vision ThunderBoard will return the ESP version
      //  if another Get Version command is issued immediately after
      //  the initial Get Version command.
      if (get_dsp_version(base, &esp_major, &esp_minor)) {
        // if they are the same version numbers, most likely it is
        //  not an Media Vision ThunderBoard and we just got the
        //  same version numbers again.
        if ((esp_major == major) && (esp_minor == minor)) {
          if (minor > 0)  // version 2.01+
            printf("High Speed Mono 8-bit board\n");
          else            // vesrion 2.00
            printf("Mono 8-bit board\n");
        } else
          printf("Media Vision Thunder Board with ESP version %i.%i\n", esp_major, esp_minor);
      }
      dma = detect_dma(base, 1);
      dma16 = detect_dma(base, 0);
      irq_num = detect_irq(base, dma);
    } else if (major == 3) {
      // see if we have a ESS1869
      // read the mixer address 0x40 register 4 times
      // if we have a 1869, the first two bytes will be 0x18 0x69
      outpb(base + SB_MIXER_ADDRESS, 0x40);
      if ((inpb(base + SB_MIXER_DATA) == 0x18) && (inpb(base + SB_MIXER_DATA) == 0x69))
        printf("Found ESS1869 board with configuration address of %04X\n", ((inpb(base + SB_MIXER_DATA) & 0xF) << 8) | inpb(base + SB_MIXER_DATA));
      outpb(base + SB_MIXER_ADDRESS, 0x00);  // zero all mixer controls
      outpb(base + SB_MIXER_DATA, 0x00);
      // There are two versions of the SB Pro.  The difference is in the FM chip used.  The earlier
      // version uses a two-operator FM chip, while the later version uses a four-operator FM chip.
      // To distinguished them, you can read the value from I/O port 388h, two-operator cards will 
      // return a value of 06h, and four-operator cards will return a value of 00h
      byte0 = inpb(0x388);
      if (byte0 == 6)
        printf("Sound Blaster Pro 1 Stereo 8-bit board. (OPL2)\n");
      else if (byte0 == 0)
        printf("Sound Blaster Pro 2 Stereo 8-bit board. (OPL3)\n");
      else
        printf("Sound Blaster Pro with unknown count of FM operators.\n");
      dma = detect_dma(base, 1);
      dma16 = detect_dma(base, 0);
      irq_num = detect_irq(base, dma);
    } else if (major == 4) {
      // Get IRQ setting
      outpb(base + SB_MIXER_ADDRESS, 0x80);
      byte0 = inpb(base + SB_MIXER_DATA);
      switch (byte0 & 0x0F) {
        case 1:  irq_num = 2; break;
        case 2:  irq_num = 5; break;
        case 4:  irq_num = 7; break;
        case 8:  irq_num = 10; break;
        default:
          printf("Unknown IRQ given: 0x%02X\n", byte0);
      }
      if (irq_num != detect_irq(base, dma))
        printf("Calculated IRQ doesn't equal found IRQ\n");
      
      outpb(base + SB_MIXER_ADDRESS, 0x81);
      byte0 = inpb(base + SB_MIXER_DATA);
      switch (byte0 & 0x0F) {
        case 2:  dma = 1; break;
        case 8:  dma = 3; break;
        default:
          dma = 0;
          printf("Unknown dma8 given: 0x%02X\n", byte0);
      }
      switch ((byte0 >> 4) & 0x0F) {
        case 0:  dma16 = 0; break;
        case 2:  dma16 = 5; break;
        case 4:  dma16 = 6; break;
        case 8:  dma16 = 7; break;
        default:
          dma16 = 0;
          printf("Unknown dma16 given: 0x%02X\n", byte0 >> 4);
      }
      
      outpb(base + SB_MIXER_ADDRESS, 0x94);
      byte0 = inpb(base + SB_MIXER_DATA);
      outpb(base + SB_MIXER_DATA, byte0 ^ 0x83);
      byte1 = inpb(base + SB_MIXER_DATA) ^ 0x83;
      outpb(base + SB_MIXER_DATA, byte0);
      if (byte0 == byte1)
        printf("Creative Sound Blaster 16\n");
      else
        printf("Sound Blaster 16/32/64 board: 16-bit ADC input, stereo 8-bit DAC outputs\n");
      
      // get copyright string
      get_copy_str(base);
    } else
      printf("Unknown Model version.\n");
  } else
    printf("Could not Get DSP version\n");
  
  printf("Found dma = %i (%i)\n", dma, dma16);
  printf("Found irq = %i\n", irq_num);
  
  if ((dma == 0) || (irq_num == -1))
    return -1;
  
  // get the command line parameters
  if (!get_parameters(argc, arg, filename, &auto_init)) {
    printf("Filename not given or unknown parameter...\n");
    return -1;
  }
  
  play_wav_file(filename, base, dma, auto_init);
  
  return 0;
}

// get the command line parameters
bool get_parameters(int argc, char *arg[], char *filename, bool *auto_init) {
  int i;
  bool file_name_given = 0;
  
  for (i=1; i<argc; i++) {
    if (!strcmp(arg[i], "-single"))
      *auto_init = 0;
    else if (!strcmp(arg[i], "-auto"))
      *auto_init = 1;
    else {
      strcpy(filename, arg[i]);
      file_name_given = 1;
    }
  }
  
  return file_name_given;
}

bit16u detect_base(void) {
  bit16u base;
  
  for (base=0x210; base<=0x280; base += 0x010) {
    // attempt to reset the DSP
    if (reset_dsp(base))
      return base;
  }
  
  // return 0 if not found
  return 0;
}

bool reset_dsp(const bit16u base) {
  bit8u val;
  
  outpb(base + SB_DSP_RESET, 1);
  mdelay(1);  // specs say wait for 3uS
  outp(base + SB_DSP_RESET, 0);
  
  return (sb_read_byte(base, &val) && (val == 0xAA));
}

void stop_dsp(const bit16u base) {
  reset_dsp(base);
  sb_write_byte(base, SB_PAUSE_DMA);
  sb_write_byte(base, SB_PAUSE_DMA16);
}

// if successful, controller will return the NOT of 'test'
//  A = (B XOR 0xFF)
//    is the same as
//  A = NOT B
bool get_dsp_id(const bit16u base, const bit8u test) {
  bit8u ret;
  
  sb_write_byte(base, SB_DSP_ID);
  sb_write_byte(base, test);
  sb_read_byte(base, &ret);
  return (ret == (test ^ 0xFF));
}

bool test_write_read(const bit16u base, const bit8u test) {
  bit8u ret;
  
  sb_write_byte(base, SB_TEST_WRITE);
  sb_write_byte(base, test);
  sb_write_byte(base, SB_TEST_READ);
  sb_read_byte(base, &ret);
  return (ret == test);
}

bool get_dsp_version(const bit16u base, bit8u *major, bit8u *minor) {
  return ((sb_write_byte(base, SB_VERSION)) &&
          (sb_read_byte(base, major)) &&
          (sb_read_byte(base, minor)));
}

void get_copy_str(const bit16u base) {
  char str[128];
  int i;
  bit8u ch;
  
  sb_write_byte(base, SB_COPY_STR);
  for (i=0; i<127; i++) {
    if (sb_read_byte(base, &ch)) {
      str[i] = ch;
      if (ch == 0)
        break;
    } else
      break;
  }
  // make sure it is null terminated
  str[i] = 0;
  
  printf("Copyright String: '%s'\n", str);
}

bit8u detect_dma(const bit16u base, const bool is8bit) {
  int i;
  
  // dma channels active when sound is not: can't be audio DMA channel
  bit8u dma_maskout = ~0x10;  // initially mask only DMA4 (cascade)
  
  // dma channels active during audio, minus those masked out
  bit8u dma_mask = 0;

  // stop all dsp activity
  stop_dsp(base);
  
  // poll to find out which DMA channels are currently in use (no sound yet)
  for (i=0; i<100; i++)
    dma_maskout &= ~dma_req();
  
  // program card to see what channel becomes active
  if (is8bit) {
    // 8-bit single-cycle DMA mode digitized sound output
    sb_write_byte(base, 0x14);
    sb_write_byte(base, 0x00);  // lo  one sample
    sb_write_byte(base, 0x00);  // hi
  } else {
    // 16-bit single-cycle DMA mode digitized sound output
    sb_write_byte(base, 0xB0); // 16-bit, D/A, S/C, FIFO off
    sb_write_byte(base, 0x10); // 16-bit mono signed PCM
    sb_write_byte(base, 0x00); // lo   one sample
    sb_write_byte(base, 0x00); // hi
  }
  
  // poll to find out which (unmasked) DMA channels are now in use (with sound)
  for (i=0; i<100; i++)
    dma_mask |= dma_req() & dma_maskout;
  
  // stop all dsp activity
  stop_dsp(base);
  
  if (bitcount(dma_mask) == 1)
    return bitpos(dma_mask);
  else
    return 0;
}

// returns dma_request for all channels (bit7->dma7..bit0->dma0)
bit8u dma_req(void) {
  return ((inpb(0xD0) & 0xF0) | (inpb(0x08) >> 4));
}

/*
 * This code "catches" all IRQ's and returns which IRQ's fired during 
 *  period we set.
 * This code is based on Ethan Brodsky's code: http://www.pobox.com/~ebrodsky/
 *  Thanks Ethan.
 *
 */
void (interrupt far *old_handler[16])(void);
int irq_hit[16];
int irq_mask[16];

void clear_irq_hit(void) {
  int i;
  
  for (i=0; i<16; i++)
    irq_hit[i] = 0;
}

void interrupt irq2_handler(void)  { irq_hit[2]  = 1;  _chain_intr(old_handler[2]);  }
void interrupt irq3_handler(void)  { irq_hit[3]  = 1;  _chain_intr(old_handler[3]);  }
void interrupt irq5_handler(void)  { irq_hit[5]  = 1;  _chain_intr(old_handler[5]);  }
void interrupt irq7_handler(void)  { irq_hit[7]  = 1;  _chain_intr(old_handler[7]);  }
void interrupt irq10_handler(void) { irq_hit[10] = 1;  _chain_intr(old_handler[10]); }

int detect_irq(const bit16u base, const int dma) {
  bit8u pic1_oldmask, pic2_oldmask;
  int i, irq = -1;
  
  // install our test handlers
  old_handler[2]  = _dos_getvect(ivt_num[2]);  _dos_setvect(ivt_num[2], irq2_handler);
  old_handler[3]  = _dos_getvect(ivt_num[3]);  _dos_setvect(ivt_num[3], irq3_handler);
  old_handler[5]  = _dos_getvect(ivt_num[5]);  _dos_setvect(ivt_num[5], irq5_handler);
  old_handler[7]  = _dos_getvect(ivt_num[7]);  _dos_setvect(ivt_num[7], irq7_handler);
  old_handler[10] = _dos_getvect(ivt_num[10]);  _dos_setvect(ivt_num[10], irq10_handler);
  
  // save and unmask IRQs
  pic1_oldmask = inp(PIC_MASTER + 1);
  outp(PIC_MASTER + 1, pic1_oldmask & 0x53);
  pic2_oldmask = inp(PIC_SLAVE + 1);
  outp(PIC_SLAVE + 1, pic2_oldmask & 0xFB);
  
  clear_irq_hit();
  
  // wait to see what interrupts are triggered without sound
  mdelay(100);
  
  // mask out any interrupts triggered without sound
  for (i = 0; i < 16; i++)
    irq_mask[i] = irq_hit[i];
  
  clear_irq_hit();
  
  // try to trigger an interrupt using DSP command F2
  sb_write_byte(base, SB_FIRE_IRQ8);
  
  mdelay(10);
  
  // detect any interrupts triggered
  for (i=0; i<16; i++)
    if (irq_hit[i] && !irq_mask[i])
      irq = i;
  
  // if F2 fails to trigger an interrupt, run a short transfer
  if (irq == -1) {
    reset_dsp(base);
    dsp_transfer(base, dma, 0x00000, 1, 11000, 0, 0);  // 1 byte, no address, no blocksize, single-cycle
    mdelay(10);
    inpb(base + SB_DSP_DATA_AVAIL);  // acknowledge interrupt
    // detect any interrupts triggered
    for (i=0; i<16; i++)
      if (irq_hit[i] && !irq_mask[i])
        irq = i;
  }
  
  // reset DSP
  reset_dsp(base);
  
  // restore IRQs
  outp(PIC_MASTER + 1, pic1_oldmask);
  outp(PIC_SLAVE + 1, pic2_oldmask);
  
  // uninstall handlers
  _dos_setvect(ivt_num[2], old_handler[2]);
  _dos_setvect(ivt_num[3], old_handler[3]);
  _dos_setvect(ivt_num[5], old_handler[5]);
  _dos_setvect(ivt_num[7], old_handler[7]);
  _dos_setvect(ivt_num[10], old_handler[10]);
  
  return irq;
}

// returns true if successful, false if timed-out
bool sb_write_byte(const bit16u base, const bit8u val) {
  int i;
  
  for (i=0; i<SB_IO_TRYS; i++)
    if ((inpb(base + SB_DSP_WRITE_STATUS) & 0x80) == 0) {
      outpb(base + SB_DSP_WRITE, val);
      return TRUE;
    }
  
  return FALSE;
}

// returns true if successful, false if timed-out
bool sb_read_byte(const bit16u base, bit8u *val) {
  int i;
  
  for (i=0; i<SB_IO_TRYS; i++)
    if ((inpb(base + SB_DSP_DATA_AVAIL) & 0x80)) {
      *val = inpb(base + SB_DSP_READ);
      return TRUE;
    }
  return FALSE;
}

int dma_pageports[] = { 0x87, 0x83, 0x81, 0x82, -1, 0x8B, 0x89, 0x8A };

void dsp_transfer(const bit16u base, const int dma, const bit32u address, 
                  const bit16u len, const bit16u freq, const bit16u bsize,
                  const bool auto_init) {
  bit16u length = len - 1;
  bit32u time_const;
  
  // mask DMA channel
  outp(0x0A, 0x04 | dma);
  
  // clear byte-pointer flip-flop
  outp(0x0C, 0x00);
  
  // address
  outp((0x00 + (2 * dma)), (bit8u) (((bit32u) address >> 0) & 0xFF)); // lo
  outp((0x00 + (2 * dma)), (bit8u) (((bit32u) address >> 8) & 0xFF)); // hi
  outp(dma_pageports[dma], (bit8u) (((bit32u) address >> 16) & 0x0F)); // page
  
  // clear byte-pointer flip-flop
  outp(0x0C, 0x00);
  
  // length
  outp((0x01 + (2 * dma)), (bit8u) ((length >> 0) & 0xFF)); // lo
  outp((0x01 + (2 * dma)), (bit8u) ((length >> 8) & 0xFF)); // hi
  
  if (auto_init)
    outp(0x0B, 0x58 | dma);  // write DMA mode: auto-init read transfer
  else  
    outp(0x0B, 0x48 | dma);  // write DMA mode: single-cycle read transfer
  
  // unmask DMA channel
  outp(0x0A, dma);
  
  // write frequency
  if (major < 4) {
    // time constant = (65536 - (256,000,000 / (channels * freq)))
    // channels = 1 for mono, 2 for stereo
    // (send high byte only)
    // freq of 11,000 = 42,263 time constant (0xA517)
    //////sb_write_byte(base, (bit8u) ((256 - 1000000) / freq));
    sb_write_byte(base, 0x40);
    time_const = ((bit32u) 65536 - ((bit32u) 256000000 / (bit32u) (1 * freq)));
    time_const = (time_const & 0x0000FF00) >> 8;
    sb_write_byte(base, (bit8u) time_const);
  } else {
    sb_write_byte(base, 0x41);
    sb_write_byte(base, (bit8u) ((freq >> 8) & 0xFF));
    sb_write_byte(base, (bit8u) ((freq >> 0) & 0xFF));
  }
  
  // set the block size
  if (auto_init && ((major >= 2) && (major < 4)) && bsize) {
    sb_write_byte(base, 0x48);
    sb_write_byte(base, (bit8u) (((bsize-1) >> 0) & 0xFF));    // lo
    sb_write_byte(base, (bit8u) (((bsize-1) >> 8) & 0xFF));    // hi
  }
  
  // start playback
  if (major < 4) {
    // 8-bit single-cycle/auto DMA mode digitized sound output
    if (auto_init && (major >= 2))
      sb_write_byte(base, 0x1C); // 8-bit auto_init dma mode
    else {
      sb_write_byte(base, 0x14); // 8-bit single cycle dma mode
      sb_write_byte(base, (bit8u) ((length >> 0) & 0xFF));    // lo
      sb_write_byte(base, (bit8u) ((length >> 8) & 0xFF));    // hi
    }
  } else {
    if (auto_init) {
      sb_write_byte(base, 0xC6); // 8-bit auto-init dma mode, FIFO on
      sb_write_byte(base, 0x00); // mode
      sb_write_byte(base, (bit8u) (((bsize-1) >> 0) & 0xFF));    // lo
      sb_write_byte(base, (bit8u) (((bsize-1) >> 8) & 0xFF));    // hi
    } else {
      sb_write_byte(base, 0xC2); // 8-bit single cycle dma mode, FIFO on
      sb_write_byte(base, 0x00); // mode
      sb_write_byte(base, (bit8u) ((length >> 0) & 0xFF));    // lo
      sb_write_byte(base, (bit8u) ((length >> 8) & 0xFF));    // hi
    }
  }
}

#define MEM_ALIGN   32
#define CHUNK_SIZE  8192
#define BLOCK_SIZE  (CHUNK_SIZE / 2)

// Our IRQ Handler
bool irq_fired = 0;
void interrupt irq_handler(void) {
  irq_fired  = 1;
  
  // end of interrupt on controller(s)
  if (irq_num > 7)
    outpb(PIC_SLAVE, EOI);
  outpb(PIC_MASTER, EOI);
}

/* Play a wav file
 *  Note to reader:
 *  Example 1:
 *   This code is in no way efficient.  In fact, this code reads from the disk
 *   after every CHUNK_SIZE is played, in turn pausing the playback to read
 *   from the disk.  However, this shows the reader how to program the SB card,
 *   therefore, I am leaving it up to the reader to create a routine that does
 *   not pause the playback.
 *   If you have a SB card with version 2.0 or higher, you can use AutoInit DMA
 *   and a multi-buffer section buffer to get around this effect.  See the description 
 *   in the book, and Example 2 below, on how to do this.
 *  Example 2:
 *   This code uses auto-initialize mode and multiple blocks to create uninterrupted
 *   audio.
 */
void play_wav_file(const char *filename, const bit16u base, const int dma, const bool auto_init) {
  FILE *fp;
  bit8u byte;
  bit16u len;
  struct WAV_FMT wav_fmt;
  void far *mem_block;
  int cur_block;
  bit32u address;  // physical address (must be less then 1meg) (< 0x000FFFFF)
  void *mem;
  
  // load file
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("Error opening file\n");
    return;
  }
  
  // read header
  len = (bit16u) fread(&wav_fmt, 1, (size_t) sizeof(struct WAV_FMT), fp);
  if (len != sizeof(struct WAV_FMT)) {
    printf("Could not read in header of WAV file...\n");
    fclose(fp);
    return;
  }

  if ((wav_fmt.riff.hdr.id  != 0x46464952) ||   // "RIFF"
      (wav_fmt.riff.format  != 0x45564157) ||   // "WAVE"
      (wav_fmt.fmt.id       != 0x20746D66) ||   // "fmt "
      (wav_fmt.data.id      != 0x61746164) ||   // "data"
      (wav_fmt.format       !=          1) ||   // 1 = PCM (non compression)
      (wav_fmt.num_channels !=          1) ||   // 1 = mono, 2 = stereo
      (wav_fmt.bits_per_sample !=       8)) {   // 8 bits per sample
    printf("Not a RIFF type WAV file we are expecting...\n");
    fclose(fp);
    return;
  }
  
  // get a physical address buffer under 1 Meg (0x000FFFFF)
  //  making sure it does not cross a 64k boundary
  mem_block = _fmalloc((CHUNK_SIZE * 2) + MEM_ALIGN);
  if (mem_block == NULL) {
    printf("Error allocating memory\n");
    fclose(fp);
    return;
  }
  
  address = ((bit32u) FP_SEG(mem_block) << 4) + (bit32u) FP_OFF(mem_block);
  address = (address + (MEM_ALIGN-1)) & ~(MEM_ALIGN-1);
  if (((address + CHUNK_SIZE - 1) & 0x0000FFFF) < CHUNK_SIZE)
    address = ((address + CHUNK_SIZE) & 0xFFFF0000);
  
  // allocate local mem buffer for fread()
  mem = malloc(CHUNK_SIZE);
  
  // save old handler and setup ours
  old_handler[irq_num]  = _dos_getvect(ivt_num[irq_num]);
  _dos_setvect(ivt_num[irq_num], irq_handler);
  
  outpb(base + SB_MIXER_ADDRESS, 0x22);  // set volume
  outpb(base + SB_MIXER_DATA, 0xDD);     // Left = HI Nibble, Right = LO nibble (0xFF is max)
  
  sb_write_byte(base, 0xD1); // turn the speaker on
  picunmask(irq_num);
  
  // TODO: If the length of the file is less than BLOCK_SIZE
  //  do single-cycle anyway.
  if (!auto_init) {
    do {
      // read a portion of the file
      len = (bit16u) fread(mem, 1, (size_t) CHUNK_SIZE, fp);
      if (len == 0)
        break;
      
      // copy it to the physical location of the block
      copy_mem_to_addr(address, mem, len);
      
      irq_fired = 0;
      dsp_transfer(base, dma, address, len, (bit16u) wav_fmt.sample_rate, 0, 0);
      
      // wait for interrupt
      while (irq_fired == 0)
        ;
      
      // acknowledge interrupt to DSP
      inpb(base + SB_DSP_DATA_AVAIL);
    } while (len == CHUNK_SIZE);
  } else {
    // read a portion of the file (two block sizes)
    len = (bit16u) fread(mem, 1, (size_t) BLOCK_SIZE * 2, fp);
    if (len > 0) {
      // copy it to the physical location of the block
      copy_mem_to_addr(address, mem, len);
      
      irq_fired = 0;
      dsp_transfer(base, dma, address, len, (bit16u) wav_fmt.sample_rate, BLOCK_SIZE, 1);
      
      cur_block = 0;
      while (1) {
        if (irq_fired) {
          // reset for next time
          irq_fired = 0;
          
          // if DSP version 4.00+, we need to check what triggered the interrupt
          if (major >= 4) {
            // check what interrupt it is
            outpb(base + SB_MIXER_ADDRESS, 0x82);  // interrupt status
            byte = inpb(base + SB_MIXER_DATA);     // bit 0 is 8-bit
            if ((byte & 1) == 0)                   // bit 1 is 16-bit
              continue;
          }
          
          // acknowledge interrupt to DSP
          inpb(base + SB_DSP_DATA_AVAIL);
          
          // read a BLOCK_SIZE from the file
          len = (bit16u) fread(mem, 1, (size_t) BLOCK_SIZE, fp);
          if (len > 0)
            // copy it to the physical location of the block
            copy_mem_to_addr(address + (cur_block * BLOCK_SIZE), mem, BLOCK_SIZE);
          
          // move to next block for next time
          // since there are only two blocks, we only need bit 0
          //  (much faster than incrementing and then checking > 1)
          cur_block ^= 1;
          
          // if we read less than BLOCK_SIZE, end after this one
          if (len < BLOCK_SIZE) {
            sb_write_byte(base, 0xDA); // exit after done with current block
            while (irq_fired == 0)     // wait for the block to finish
              ;
            goto play_done;
          }
        }
      }
    }
  }
  
play_done:  
  fclose(fp);
  
  sb_write_byte(base, 0xD3);  // turn speakers off
  
  picmask(irq_num);
  _dos_setvect(ivt_num[irq_num], old_handler[irq_num]);
  
  stop_dsp(base);
  
  free(mem);
  _ffree(mem_block);
  
  return;
}

#define MK_FP(seg, off) ((void far *) (((bit32u) (seg) << 16) | (bit16u) (off)))

void copy_mem_to_addr(const bit32u address, const bit8u *src, const bit16u len) {
  bit8u far *t = (bit8u far *) MK_FP((bit16u) (address >> 4), (address & 0xF));
  bit16u l = len;
  
  while (l--)
    *t++ = *src++;
  
  // fill remaining block with nulls
  if (len < BLOCK_SIZE) {
    l = BLOCK_SIZE - len;
    while (l--)
      *t++ = 0;
  }
}

// CLOCKS_PER_SEC must be at least 1 per mS
#if (CLOCKS_PER_SEC < 1000)
  #error "CLOCKS_PER_SEC < 1000"
#endif

// This is not a very accurate way to wait when calling
//  multiple times within a loop with a small 'ms' amount.
// It takes considerable amount of time to calculate the
//  time to wait each time it is called.
// However, for our purposes, it will be just fine.
void mdelay(const int ms) {
  clock_t pause;
  
// division is extremely slow, so if CLOCKS_PER_SEC
//  is already 1000 (default), why do the divide.
#if (CLOCKS_PER_SEC == 1000)
  pause = (clock_t) (ms + clock());
#else
  pause = (clock_t) ((ms * (CLOCKS_PER_SEC / 1000)) + clock());
#endif
  
  while (clock() < pause)
    ;
}
