
#include <conio.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

#include "../include/ctype.h"
#include "vid_info.h"


int main(int argc, char *argv[]) {
  int i, j, mode, major;
  bit16u modes[128], vbemodes[128];
  struct S_VIDEO_SVGA vid_info;
  
  if (vid_get_vid_info(&vid_info)) {
    i = vid_get_vid_modes(vid_info.svgamodesseg, vid_info.svgamodesptr, modes);
    printf("Found VESA v%i.%i compatible video card returning %i modes.\n", 
      major = (vid_info.VESAVersion >> 8), (vid_info.VESAVersion & 0xFF), i);
    
    printf("     Vesa Signature: %c%c%c%c\n", vid_info.VESASignature[0], vid_info.VESASignature[1],
                                         vid_info.VESASignature[2], vid_info.VESASignature[3]);
    printf("       Vesa Version: %i.%i\n", (vid_info.VESAVersion >> 8), (vid_info.VESAVersion & 0xFF));
    printf("      OEM String at: %04X:%04X \"%s\"\n", vid_info.oemseg, vid_info.oemptr, do_string(vid_info.oemseg, vid_info.oemptr));
    printf("       Capabilities: 0x%04X\n", vid_info.caps);
    printf("     DAC can be switched into 8-bit mode: %s\n", do_yesno(vid_info.caps & (1<<0)));
    printf("                      non-VGA controller: %s\n", do_yesno(vid_info.caps & (1<<1)));
    printf("           programmed DAC with blank bit: %s\n", do_yesno(vid_info.caps & (1<<2)));
    if (major >= 3) {
      printf("        supports stereoscopic signalling: %s ", do_yesno(vid_info.caps & (1<<3)));
      if (vid_info.caps & (1<<3))
        puts("via external VESA stereo connector");
      else
        puts("via VESA EVC connector");
    } else
      printf("        supports VBE/AF v1.0P extensions: %s\n", do_yesno(vid_info.caps & (1<<3)));
    printf("  must call EnableDirectAccess to access framebuffer: %s\n", do_yesno(vid_info.caps & (1<<4)));
    printf("          supports hardware mouse cursor: %s\n", do_yesno(vid_info.caps & (1<<5)));
    printf("              supports hardware clipping: %s\n", do_yesno(vid_info.caps & (1<<6)));
    printf("             supports transparent BitBLT: %s\n", do_yesno(vid_info.caps & (1<<7)));
    printf("       Mode List at: %04X:%04X \n", vid_info.svgamodesseg, vid_info.svgamodesptr);
    printf("         64k blocks: %i\n", vid_info.memory);
    printf("        OEM Version: %i.%i\n", (vid_info.oem_version >> 8), (vid_info.oem_version & 0xFF));
    printf("   Vendor String at: %04X:%04X \"%s\"\n", vid_info.vendorseg, vid_info.vendorptr, do_string(vid_info.vendorseg, vid_info.vendorptr));
    printf("  Product String at: %04X:%04X \"%s\"\n", vid_info.productseg, vid_info.productptr, do_string(vid_info.productseg, vid_info.productptr));
    printf(" Product Version at: %04X:%04X \"%s\"\n", vid_info.prodverseg, vid_info.prodverptr, do_string(vid_info.prodverseg, vid_info.prodverptr));
    if (vid_info.caps & (1<<3)) {
      printf("    AF Version: %i.%i\n", (vid_info.vbe_af_ver >> 8), (vid_info.vbe_af_ver & 0xFF));
      printf(" VBE Mode List: %04X:%04X: ", vid_info.vbemodesptr, vid_info.vbemodesptr);
      i = vid_get_vid_modes(vid_info.vbemodesptr, vid_info.vbemodesptr, vbemodes);
      for (j=0; j<i; j++)
        printf("0x%04X ", vbemodes[j]);
      puts("");
    }
    puts("");

    // now print each mode's information
    vesa_mode_enum(modes, major); // get the vesa mode info
  } else {
    printf("\n Failed to Get Video Information...");
    return -1;
  }
}

bool vid_get_vid_info(struct S_VIDEO_SVGA *info) {
  bool ret = FALSE;
  __dpmi_regs r;
  bit32u dosbuf;
  int i;
  
  dosbuf = __tb & 0xFFFFF;
  
  for (i=0; i<sizeof(struct S_VIDEO_SVGA); i++)
    _farpokeb(_dos_ds, dosbuf + i, 0);
  
  dosmemput("VBE2", 4, dosbuf);
  
  r.x.ax = 0x4F00;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  __dpmi_int(0x10, &r);
  
  if (r.h.ah == 0x00) {
    ret = TRUE;
    
    // store the info
    if (info) {
      dosmemget(dosbuf, sizeof(struct S_VIDEO_SVGA), info);
      if (strncmp(info->VESASignature, "VESA", 4) != 0)
        ret = FALSE;
    }
  }
  
  return ret;
}

int vid_get_vid_modes(const bit16u seg, const bit16u off, bit16u *modes) {
  
  int i = 0;
  
  bit16u mode;
  bit32u ptr = (seg << 4) + off;
  
  do {
    mode = _farpeekw(_dos_ds, ptr);
    ptr += 2;
    modes[i++] = mode;
  } while (mode < 0xFFFF);
  
  return i;
}

bool vid_get_mode_info(struct S_VIDEO_MODE_INFO *info, const bit16u mode) {
  __dpmi_regs r;
  
  // we don't do modes less than 0x100
  if (mode < 0x100)
    return FALSE;
  
  // clear it out for versions before 1.1
  memset(info, 0, sizeof(struct S_VIDEO_MODE_INFO));
  
  r.x.ax = 0x4F01;
  r.x.cx = mode;
  r.x.di = __tb & 0xF;
  r.x.es = __tb >> 4;
  __dpmi_int(0x10, &r);
  
  // Now check eax to see if supported.  If so, get next part.  etc....
  if (r.h.ah == 0) {
    // store the info
    if (info)
      dosmemget(__tb, sizeof(struct S_VIDEO_MODE_INFO), info);
    return TRUE;
  }
  
  return FALSE;
}

void vesa_mode_enum(bit16u *mode_list, const int major) {
  unsigned found = 0, mode_count = 0;
  struct S_VIDEO_MODE_INFO info;
  
  while (*mode_list < 0xFFFF) {
    printf("\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    printf("Checking found mode: 0x%04X\n", *mode_list);
    if (vid_get_mode_info(&info, *mode_list)) {
      // Mode Attributes:
      printf("       Mode Attributes: 0x%04X\n", info.mode_attrb);
      printf("     mode supported by present hardware configuration: %s\n", do_yesno(info.mode_attrb & (1<<0)));
      printf("                       optional information available: %s\n", do_yesno(info.mode_attrb & (1<<1)));
      printf("                                BIOS output supported: %s\n", do_yesno(info.mode_attrb & (1<<2)));
      printf("                                                color: %s\n", do_yesno(info.mode_attrb & (1<<3)));
      printf("                                        graphics mode: %s\n", do_yesno(info.mode_attrb & (1<<4)));
      if (major >= 2) {
        printf("                               mode is VGA-compatible: %s\n", do_yesno((info.mode_attrb & (1<<5)) ^ (1<<5)));
        printf("                         bank-switched mode supported: %s\n", do_yesno((info.mode_attrb & (1<<6)) ^ (1<<6)));
        printf("                    linear framebuffer mode supported: %s\n", do_yesno(info.mode_attrb & (1<<7)));
        printf("       double-scan mode available (320x200 & 320x240): %s\n", do_yesno(info.mode_attrb & (1<<8)));
        if (major >= 3) {
          printf("                            interlaced mode available: %s\n", do_yesno(info.mode_attrb & (1<<9)));
          printf("                   hardware supports triple buffering: %s\n", do_yesno(info.mode_attrb & (1<<10)));
          printf("               hardware supports stereoscopic display: %s\n", do_yesno(info.mode_attrb & (1<<11)));
          printf("                   dual display start address support: %s\n", do_yesno(info.mode_attrb & (1<<12)));
        }
      }
      printf("  must call EnableDirectAccess before calling bank-switching functions: %s\n", do_yesno(info.mode_attrb & (1<<9)));

      // WinA Attributes:
      printf("      Win A Attributes: 0x%04X\n", info.wina_attrb);
      printf("              exists: %s\n", do_yesno(info.wina_attrb & (1<<0)));
      printf("            readable: %s\n", do_yesno(info.wina_attrb & (1<<1)));
      printf("           writeable: %s\n", do_yesno(info.wina_attrb & (1<<2)));
      // WinB Attributes:
      printf("      Win B Attributes: 0x%04X\n", info.winb_attrb);
      printf("              exists: %s\n", do_yesno(info.winb_attrb & (1<<0)));
      printf("            readable: %s\n", do_yesno(info.winb_attrb & (1<<1)));
      printf("           writeable: %s\n", do_yesno(info.winb_attrb & (1<<2)));
      
      printf("       Win Granularity: %ik\n", info.win_granularity);
      printf("              Win Size: %ik\n", info.win_size);
      printf("           Win A Start: %04Xh\n", info.wina_segment);
      printf("           Win B Start: %04Xh\n", info.winb_segment);
      printf("       Win Function at: %04X:%04X\n", info.win_func_seg, info.win_func_ptr);
      printf("   Bytes per Scan Line: %i\n", info.bytes_scanline);
      printf("          X Resolution: %i\n", info.x_res);
      printf("          Y Resolution: %i\n", info.y_res);
      printf("           X Char Size: %i\n", info.x_char_size);
      printf("           Y Char Size: %i\n", info.y_char_size);
      printf("      Number of Planes: %i\n", info.num_planes);
      printf("        Bits per Pixel: %i\n", info.bits_pixel);
      printf("       Number of Banks: %i\n", info.num_banks);
      printf("          Memory Model: %i (%s)\n", info.memory_model, do_mem_model(info.memory_model));
      printf("             Bank Size: %ik\n", info.bank_size);
      printf(" Number of Image Pages: %i\n", info.num_image_pages + 1);
      printf("              reserved: %02X\n", info.resv1);
      printf("         Red Mask Size: %i\n", info.red_mask_size);
      printf("    Red Field Position: %i\n", info.red_field_pos);
      printf("       Green Mask Size: %i\n", info.green_mask_size);
      printf("  Green Field Position: %i\n", info.green_field_pos);
      printf("        Blue Mask Size: %i\n", info.blue_mask_size);
      printf("   Blue Field Position: %i\n", info.blue_field_pos);
      printf("        Resv Mask Size: %i\n", info.rsvd_mask_size);
      printf("   Resv Field Position: %i\n", info.rsvd_field_pos);
      printf("Direct Color Mode info: %i\n", info.direct_color_mode);
      
      if (major >= 2) {
        printf("   Linear Base Address: 0x%08X\n", info.linear_base);
        printf("    Off Screen Address: 0x%08X\n", info.offscreen);
        printf("       Off Screen Size: %ik\n", info.offscreen_size);
        
        if (major >= 3) {
          printf("       Bytes per Scan Line: %i\n", info.linear_b_scanline);
          printf(" Number of Images (Banked): %i\n", info.num_imgs_banked + 1);
          printf(" Number of Images (Linear): %i\n", info.num_imgs_linear + 1);
          printf("             Red Mask Size: %i\n", info.lm_red_mask_s);
          printf("        Red Field Position: %i\n", info.lm_red_mask_pos);
          printf("           Green Mask Size: %i\n", info.lm_grn_mask_s);
          printf("      Green Field Position: %i\n", info.lm_grn_mask_pos);
          printf("            Blue Mask Size: %i\n", info.lm_blue_mask_s);
          printf("       Blue Field Position: %i\n", info.lm_blue_mask_pos);
          printf("            Resv Mask Size: %i\n", info.lm_resv_mask_s);
          printf("       Resv Field Position: %i\n", info.lm_resv_mask_pos);
          printf("           Max Pixel Count: %i\n", info.max_pixel_cnt);
        }
      }
      mode_count++;
    }
    mode_list++;
    found++;
  }
  
  printf("Modes found: %i  Modes supported: %i\n", found, mode_count);
}

#define MAX_STR_SIZE  48

const char *yes_str = "yes";
const char *no_str = "no";
char temp[MAX_STR_SIZE + 1];
const char *null = "null";

const char *do_yesno(const bit32u yes) {
  return (yes != 0) ? yes_str : no_str;
}

const char *do_string(const bit16u seg, const bit16u off) {
  char ch;
  bit32u ptr = (seg << 4) + off;
  int i = 0;

  if ((seg == 0) && (off == 0))
    return null;
  
  while (i < MAX_STR_SIZE) {
    ch = _farpeekb(_dos_ds, ptr++);
    if (ch == 0)
      break;
    temp[i++] = ch;
  }
  temp[i] = 0;
  
  return temp;
}

char mem_model_str[][48] = {
  /* 00h  */  "text",
  /* 01h  */  "CGA graphics",
  /* 02h  */  "HGC graphics",
  /* 03h  */  "16-color (EGA) graphics",
  /* 04h  */  "packed pixel graphics",
  /* 05h  */  "'sequ 256' (non-chain 4) graphics",
  /* 06h  */  "direct color (HiColor, 24-bit color)",
  /* 07h  */  "YUV (luminance-chrominance, also called YIQ)",
  /* else */  "Unknown"
};

const char *do_mem_model(const int model) {
  if (model < 8)
    return mem_model_str[model];
  else
    return mem_model_str[8];
}
