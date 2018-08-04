
#include "ctype.h"

#include "conio.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys.h"
#include "windows.h"

#include "video.h"

bit16u vid_modes[VIDEO_MAX_MODES];

bit16u get_video_info(struct S_MODE_INFO *modeinfo) {
  int i, j;
  struct REGS regs;
  bit16u vesa_modes[VESA_MODE_SIZE];
  bit8u  buffer[512];
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // clear the buffer first
  memset(buffer, 0, 512);
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // call the BIOS, first setting the first 4 bytes to 'VBE2'
  // Since it returns 512 bytes, we need to use a spare buffer.
  memcpy(buffer, "VBE2", 4);
  regs.eax = 0x00004F00;
  regs.edi = MK_OFF((bit32u) buffer);
  regs.es = MK_SEG((bit32u) buffer);
  intx(0x10, &regs);
  if ((regs.eax & 0x0000FFFF) != 0x004F)
    return 0;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // now get the supported modes.  We have to do this incase
  //  the BIOS builds the list on the fly (Bochs BIOS does this)
  bit16u *p = (bit16u *) MK_FP(buffer + 0x0E);
  memcpy(vesa_modes, p, ((VESA_MODE_SIZE - 1) * 2));
  vesa_modes[(VESA_MODE_SIZE - 1)] = 0xFFFF;
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // Now scroll through the list of modes to see if we find any
  //  capable for our use.
  i = 0; j = 0;
  struct S_VIDEO_MODE_INFO *info = (struct S_VIDEO_MODE_INFO *) buffer;
  while (vesa_modes[i] < 0xFFFF) {
    memset(info, 0, sizeof(struct S_VIDEO_MODE_INFO));
    regs.eax = 0x00004F01;
    regs.ecx = vesa_modes[i];
    regs.edi = MK_OFF((bit32u) info);
    regs.es = MK_SEG((bit32u) info);
    intx(0x10, &regs);
    if ((regs.eax & 0x0000FFFF) == 0x004F) {
      if ((info->mode_attrb & 0x01) &&  // supported by the current hardware (video card and monitor)
          (info->bits_pixel >= 8) &&
         ((info->mode_attrb & (1<<7)) && (info->linear_base > 0)) &&  // bit 7 = 1 if LFB is available for this mode
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
        modeinfo[j].red = ((info->red_field_pos << 8) | info->red_mask_size);
        modeinfo[j].grn = ((info->green_field_pos << 8) | info->green_mask_size);
        modeinfo[j].blu = ((info->blue_field_pos << 8) | info->blue_mask_size);
        vid_modes[j] = vesa_modes[i];
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

void get_video_eedid(void) {
  struct REGS regs;
  bit8u crc, edid[128];
  struct S_EEDID *p = (struct S_EEDID *) edid;
  int i;
  bool b;
  
  // VESA VBE/DC (Display Data Channel) - INSTALLATION CHECK / CAPABILITIES
  regs.eax = 0x00004F15;
  regs.ebx = 0x00000000;
  b = intx(0x10, &regs);
  
  // successful?
  if (!b && ((regs.eax & 0x0000FFFF) == 0x004F)) {
    //win_printf(main_win, "Vesa Supported EEDID call...\n");
    regs.eax = 0x00004F15;
    regs.ebx = 0x00000001;
    regs.ecx = 0x00000000;
    regs.edx = 0x00000000;
    regs.edi = MK_OFF((bit32u) edid);
    regs.es = MK_SEG((bit32u) edid);
    b = intx(0x10, &regs);
    if (!b && ((regs.eax & 0x0000FFFF) == 0x004F)) {
      //win_printf(main_win, "EDID: b = %i, eax = 0x%08X\n\n", b, regs.eax);   // 0x0000004F = successfull
      fdebug(edid, 128);
      crc = 0;
      for (i=0; i<128; i++)
        crc += edid[i];
      //win_printf(main_win, "EDID crc = 0x%02X\n", crc);
      // if crc != 0, error
      if (crc == 0) {
        //printf("size of struct S_EEDID = %i\n", sizeof(struct S_EEDID));
        printf("0x%02X 0x%02X 0x%02X\n", p->est_timings1, p->est_timings2, p->est_timings_resv);
        
      }
    }
  }
}


