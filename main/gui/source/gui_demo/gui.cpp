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
 *  gui.cpp
 *
 * compile using gcc (djgpp) for DOS, using the included 'makefile'
 *    D:\DGJPP\BIN\make             to build
 *    D:\DGJPP\BIN\make clean       to clean
 *
 *  usage:
 *    gui
 *      /ppath_to_use  (full path to find remaining files)
 *      /t             (truncate debug.txt file)
 *      /v             (verbose)
 *    
 *  
 *  Last updated: 17 July 2020
 */

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "../include/ctype.h"
#include "gui.h"

#include "images.h"
#include "examples.h"
#include "palette.h"
#include "gui_mb.h"
#include "filesel.h"
#include "grfx.h"
#include "mouse.h"
#include "video.h"

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <pc.h>
#include <unistd.h>

extern struct S_SYS_VIDEO *sys_video;

struct S_VIDEO_MODE_INFO *video_mode_info = NULL;

extern int mouse_x, mouse_y;

// Publically accessable static data
struct EVENTSTACK eventstack;

// Private static data
struct ROOT_ROOT root;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Desktop Image object
struct IMAGE desktop_img = { { 0, }, 0, NULL };

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Screen Buffer Access
 *  We need to set up a descriptor pointing to the linear buffer of the screen
 *   memory so that we can write to it.
 * (In linear access, this will be a physical buffer at least X * Y * bpp)
 * (In Bank Switching, this will be at 0x00A0000 * win_ganularity)
 *
 * This is only needed for this demo since we use FreeDOS and DPMI to get to
 *  physical memory.  Your OS will already have access to physical memory.
 */
__dpmi_meminfo base_mi;
int base_selector;

bool get_physical_mapping(__dpmi_meminfo *mi, int *selector) {
  int sel;
  
  if (__dpmi_physical_address_mapping(mi) == -1)
    return FALSE;
  sel = __dpmi_allocate_ldt_descriptors(1);
  if (sel == -1)
    return FALSE;
  if (__dpmi_set_segment_base_address(sel, mi->address) == -1)
    return FALSE;
  if (__dpmi_set_segment_limit(sel, mi->size - 1))
    return FALSE;
  
  *selector = sel;
  return TRUE;
}

/* gfx_show_focus
 *   show =  0 = don't show the focus dotted box
 *           1 = show the focus dotted box
 *
 *   this is called by the GUI system to turn on or off the show focus flag
 */
void gfx_show_focus(const bool show) {
  if (grx_args.showfocus && !show && obj_get_focus())
    obj_dirty(obj_get_focus(), FALSE);
  grx_args.showfocus = show;
}

// Since we are still debugging our code, we can't really use printf()'s to see
//  error code and the such.  Therefore, we print all data, errors, info, etc.,
//  to a file for view later.
FILE *dfp;

time_t timestamp;
char default_path[512];

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Main
 *  This is were we start
 */
int main(int argc, char *argv[]) {
  int i, j, ch, mode;
  bit16u modes[MAX_MODE_COUNT];
  struct S_VIDEO_SVGA video_info;
  bool verbose = FALSE, truncate = FALSE;
  bool linear_addressing;

  // we need to parse the command line and get the parameters found
  strcpy(default_path, "");
  parse_command(argc, argv, &verbose, &truncate);
  
#ifdef DO_DEBUG
  // open the debug file
  if (truncate)
    dfp = fopen("debug.txt", "w");
  else
    dfp = fopen("debug.txt", "a");
  if (dfp == NULL) {
    puts("\nError opening t.txt file");
    return -1;
  }
  
  if (verbose)
    DEBUG((dfp, "CLOCKS_PER_SEC = %i", CLOCKS_PER_SEC));
#endif
  
  clrscr();
  printf("GUI.EXE  An example Graphical User Interface for the book:\n"
         "   GUI: The Graphical User Interface, Benjamin David Lunt.\n"
         "(C)opyright 1984-2020              Forever Young Software \n\n");
  
  sys_video = (struct S_SYS_VIDEO *) calloc(sizeof(struct S_SYS_VIDEO), 1);
  sys_video->mode_count = 0;
  
  if (vid_get_vid_info(&video_info)) {
    if (verbose) {
      printf("\nFound VESA v%i.%i compatible video card:  ", 
        (video_info.VESAVersion >> 8), (video_info.VESAVersion & 0xFF));
    }
    vid_get_vid_modes(&video_info, modes);
    vesa_mode_enum(sys_video, modes, verbose); // get the vesa mode info
    sys_video->current_mode_index = 0x03;
    if (verbose) {
      printf("\nPress a key to continue...\n");
      getch();
    }
  } else {
    printf("Failed to Get Video Information...\n");
    fclose(dfp);
    return -1;
  }
  
  // make sure keyboard is empty
  while (kbhit())
    getch();
  

  // print the available modes and ask for a selection of one of them.
  // if there are more modes than will fit on the screen, an 'enter' key
  //  will scroll to the next page.
  // the ESC key will exit
  int avail[24];
  static char access_str[2][12] = { "bank switch", "linear" };
  i = 0;
next_entry:
  for (j=0; i<sys_video->mode_count && j<23; i++) {
    if (sys_video->modes[i].bits_pixel >= 8) {
      printf(" %c: mode: 0x%03X: %4i x %4i %2i bits (%s)\n", j + 'A', sys_video->modes[i].mode, 
        sys_video->modes[i].width, sys_video->modes[i].height, sys_video->modes[i].bits_pixel,
        access_str[sys_video->modes[i].linear]);
      avail[j] = i;
      j++;
    }
  }
  if (i < sys_video->mode_count)
    printf(" ** 'Enter' for more\n");
  do {
    printf("Please select a video mode to use [A-%c]: ", j + 'A' - 1);
    ch = getch();
    if (ch == 0x1B)  { // esc
      fclose(dfp);
      return -1;
    }
    if ((ch == 0x0D) && (i < sys_video->mode_count)) {
      puts("");
      goto next_entry;
    }
    ch = toupper(ch);
    printf("%c\n", ch);
    ch -= 'A';
  } while ((ch < 0) || (ch >= j));
  mode = sys_video->modes[avail[ch]].mode;
  
  // save the current mode index
  sys_video->current_mode_index = avail[ch];
  video_mode_info = sys_video->modes[sys_video->current_mode_index].mode_info;
  
  if (verbose) {
    DEBUG((dfp, "\n\n[0x%08X]", video_mode_info->linear_base));
    DEBUG((dfp, "\n bits per pixel: %i", video_mode_info->bits_pixel));
    DEBUG((dfp, "\n          width: %i", video_mode_info->x_res));
    DEBUG((dfp, "\n         height: %i", video_mode_info->y_res));
  }
  
  // check the bit in the video_mode_attribute word that states if
  //   we support linear addressing on this mode.
  if (sys_video->modes[sys_video->current_mode_index].linear) {
    // setup flat descriptor to the linear base
    // Note: This only needs to be done for this demo since we are using FreeDOS and DPMI
    base_mi.address = video_mode_info->linear_base;
    base_mi.size = video_mode_info->y_res * video_mode_info->bytes_scanline;
    if (!get_physical_mapping(&base_mi, &base_selector)) {
      printf("\n Linear: Error 'allocating' physical memory for video.");
      DEBUG((dfp, "\n Linear: Error 'allocating' physical memory for video."));
      fclose(dfp);
      return -1;
    }
    linear_addressing = TRUE;  // use linear addressing
    printf("\n Using Linear Addressing.");
    DEBUG((dfp, "\n Using Linear Addressing."));
    
  // we might get to use bank-switching then.
  } else if (sys_video->modes[sys_video->current_mode_index].bankswitch) {
    vesa_bankswitch_init();
    linear_addressing = FALSE;  // use bank switching
    
  } else {
    // else, we don't have a way to write to the screen
    printf("\n Error: mode_attrb:7 = 0 or Linear_base = 0.");
    printf("\n   or");
    printf("\n        mode_attrb:6 = 1 or WinA Segment = 0.");
    fclose(dfp);
    return -1;
  }
  
  // make the root object
  memset(&root, 0, sizeof(root));
  object(DESKTOPOBJ, root_class, sizeof(root.root), NULL, 0);
  DESKTOPOBJ->win = DESKTOP;
  
  // Set some startup values
  memset(&grx_args, 0, sizeof(struct GFX_ARGS));
  grx_args.w = video_mode_info->x_res;
  grx_args.h = video_mode_info->y_res;
  grx_args.c = video_mode_info->bits_pixel;
  grx_args.buffer = obj_bitmap(grx_args.w, grx_args.h, 1);
  grx_args.showpointer = TRUE;
  grx_args.showfocus = FALSE;
  // if VBE < 3.0, use video_mode_info->bytes_scanline
  // if VBE >= 3.0 use video_mode_info->linear_b_scanline
  if (linear_addressing)
    grx_args.bytes_scanline = (video_info.VESAVersion >= 0x0300) ? video_mode_info->linear_b_scanline : video_mode_info->bytes_scanline;
  
  // if we are an 8-bit mode, by setting the palette below, we will change
  //  the color of the text that is still on the screen.  Therefore, we need
  //  to either clear the screen, or display a splash screen, or something else.
  // for now, we will just clear the screen
  //  however, let's be a bit creative.
  /*
  for (j=0; j<25; j++)
    for (i=0; i<(80*25*2); i++) {
      ch = _farpeekb(_dos_ds, 0xB8000 + i);
      if (ch > 0)
        _farpokeb(_dos_ds, 0xB8000 + i, ch >> 1);
    }
  */
  
  switch (grx_args.c) {
    case 8:
      grx_args.screen_out = (linear_addressing) ? vesa_linear8 : vesa_bankswitch8;
      // if we are an 8-bits per pixel (256 color) mode, we need to set the palette
      vid_set_256_palette((video_mode_info->mode_attrb & (1<<5)) == 0);
      break;
    case 15:
      grx_args.screen_out = (linear_addressing) ? vesa_linear15 : vesa_bankswitch15;
      break;
    case 16:
      grx_args.screen_out = (linear_addressing) ? vesa_linear16 : vesa_bankswitch16;
      break;
    case 24:
      grx_args.screen_out = (linear_addressing) ? vesa_linear24 : vesa_bankswitch24;
      break;
    case 32:
      grx_args.screen_out = (linear_addressing) ? vesa_linear32 : vesa_bankswitch32;
      break;
    default:
      vid_set_text_mode(0x03);
      printf("\n Unknown or unsupported bits per pixel value: %i", video_mode_info->bits_pixel);
      fclose(dfp);
      if (linear_addressing) __dpmi_free_physical_address_mapping(&base_mi);
      return FALSE;
  }
  
  // Reset and initialize the mouse driver
  mouse_x = grx_args.w-1;
  mouse_y = grx_args.h-1;
  if (!mouse_init(0, 0, grx_args.w-1, grx_args.h-1, grx_args.w>>1, grx_args.h>>1, 1, 1)) {
    puts("\n Error initializing Mouse.  Need to install mouse driver?");
    DEBUG((dfp, "\n Error initializing Mouse.  Need to install mouse driver?"));
    fclose(dfp);
    if (linear_addressing) __dpmi_free_physical_address_mapping(&base_mi);
    vid_set_text_mode(0x03);
    return -1;
  }
  
  // set the drawing area
  DEBUG((dfp, "\n Setting the drawing area: %i x %i", grx_args.w, grx_args.h));
  drs_area(grx_args.w - 1, grx_args.h - 1);
  
  // Setup some helper data
  grx_args.screen.left = 0;
  grx_args.screen.top = 0;
  grx_args.screen.right = grx_args.w - 1;
  grx_args.screen.bottom = grx_args.h - 1;
  
  // Load the default Font and any remaining fonts we desire
  if (font_load("System.fnt", TRUE) == NULL) {  // 8x14
    printf("\n Error loading default font...");
    DEBUG((dfp, "\n Error loading default font..."));
    fclose(dfp);
    remove_mouse_handler();
    if (linear_addressing) __dpmi_free_physical_address_mapping(&base_mi);
    return -1;
  }
  if (verbose) puts("Loading font: LucidiaC");
  font_load("LucidaC.fnt", FALSE);  //  x19 Lucida Caligraphy
  if (verbose) puts("Loading font: SansSerf");
  font_load("SansSerf.fnt", FALSE); // 8x12 Sans Sarif
  if (verbose) puts("Loading font: Simple");
  font_load("Simple.fnt", FALSE);   // 8x12 Simple Block
  // we currently don't load the "arial" font here so that
  //  we can show how to do it later when the font is needed
  //  but not loaded.
  //font_load("Arial.fnt", FALSE);    //      Narrow Arial
  
  //  Did we create the graphics buffer yet?
  if (!grx_args.buffer)
    grx_args.buffer = obj_bitmap(grx_args.w, grx_args.h, 1);
  atom_lock(&grx_args.buffer->base.atom);
  
  grx_args.redraw = gui_redraw;
  
  //////////////////////////////////////////////////////////////////////
  root.root.base.rectatom.rect.left = 0;
  root.root.base.rectatom.rect.top = 0;
  root.root.base.rectatom.rect.right = grx_args.w - 1;
  root.root.base.rectatom.rect.bottom = grx_args.h - 1;
  
  // load static images
  if (verbose) puts("Loading static image file.");
  if (!load_static_images("images.sys")) {
    fclose(dfp);
    remove_mouse_handler();
    if (linear_addressing) __dpmi_free_physical_address_mapping(&base_mi);
    return -1;
  }
  
  // Load desktop image
  if (verbose) puts("Loading desktop image file.");
  desktop_img.bitmap = get_bitmap("hallway.gif", TRUE);
  // If we wish to have this image take up the whole background, set the flag below
#if STRETCH_DESKTOP
  desktop_img.flags = IMAGE_FLAGS_STRETCH;
#endif
  // if we didn't find the image file, or there was an error with it,
  //  build a random image to use.
  if (desktop_img.bitmap == NULL) {
    desktop_img.bitmap = make_random_image(grx_args.w, grx_args.h);
    desktop_img.flags = 0;  // do not stretch image
  }
  
  // load "external" mouse cursor image
  load_mouse_pointer(POINTER_ID_BUSY, ID_ICON_BUSY, -1, -1);
  
  // Set video mode
  if (!vid_set_mode(sys_video->current_mode_index, linear_addressing, TRUE)) {
    printf("\n Video Mode 0x%03X not supported.", mode);
    DEBUG((dfp, "\n Video Mode 0x%03X not supported.", mode));
    fclose(dfp);
    remove_mouse_handler();
    if (linear_addressing) __dpmi_free_physical_address_mapping(&base_mi);
    return -1;
  }
  
  //////////////////////////////////////////////////////////////////////
  // At this point, we are no longer in text mode.  Any printf()'s we
  //  use will not be seen.  Therefore, all remaining must be DEBUG()'s
  //////////////////////////////////////////////////////////////////////
  
  // do the "start" button and clock and "task bar"
  struct TASKBAR *taskbar = obj_taskbar(NULL, 0, DESKTOPOBJ, 0);
  obj_geometry(GUIOBJ(taskbar));
  obj_dirty(GUIOBJ(taskbar), TRUE);
  
  //////////////////////////////////////////////////////////////////////
  // example windows and objects
  //////////////////////////////////////////////////////////////////////
  
  
  //////////////////////////////////////////////////////////////////////
  // Image example
  struct WIN *win0;
  win0 = obj_win(NULL, DESKTOP, 0, NULL, 0, BORDER_NORMAL);
  obj_position(GUIOBJ(win0), 50, 50, 200, 250);
  textual_set(GUITEXTUAL(win0), "Test Image", -1, FALSE);
  obj_geometry(GUIOBJ(win0));
  
  struct IMAGE win0_img;
  obj_image(&win0_img, sizeof(struct IMAGE), GUIOBJ(win0), 123);
  obj_defaultrect(GUIOBJ(&win0_img), NULL);
  win0_img.bitmap = get_bitmap("snow.bmp", FALSE);
  
  if (win0_img.bitmap == NULL)
    win0_img.bitmap = obj_bitmap(20, 20, 1);
  obj_resize(GUIOBJ(&win0_img), gui_w(win0_img.bitmap), gui_h(win0_img.bitmap));
  obj_layout(GUIOBJ(&win0_img), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(win0), 3, 3);
  
  //////////////////////////////////////////////////////////////////////
  // message box example
  int r = gui_message_box(NULL, "Message Box", "This is a message box\nHere is a line\nanother\nanother\nanother\nthe last\n\nMan does this need work!!!",
    GUI_MB_OKCANCEL);
  
  /// Numberous message boxes to test and show the ability of a message box
  gui_message_box(NULL, "Message Box", "A message Box with 2 lines\nand the okay button only", GUI_MB_OK);
  gui_message_box(NULL, "Message Box", "A message Box with 2 lines\nand the okay and cancel buttons", GUI_MB_OKCANCEL);
  gui_message_box(NULL, "Message Box", "A message Box with 3 lines,\nthe icon,\nand the retry and cancel buttons", GUI_MB_RETRYCANCEL);
  gui_message_box(NULL, "Message Box", "1 line and the icon", GUI_MB_RETRYCANCEL);
  gui_message_box(NULL, "Message Box", "1 line and no icon", GUI_MB_OK);
  gui_message_box(NULL, "Message Box", "A message Box with 2 lines\nand the yes and no buttons", GUI_MB_YESNO);
  gui_message_box(NULL, "Save File?", "Would you like to save this\nfile before you exit?", GUI_MB_YESNOCANCEL);
  gui_message_box(NULL, "Message Box", "lots\nlines\nand\nthe\nicon\nsafd\nsdf\nasdf", GUI_MB_RETRYCANCEL);
  gui_message_box(NULL, "Message Box", "This would be an\nAbort message", GUI_MB_ABORT);
  gui_message_box(NULL, "Font Check", " !\"#$%&'()*+,-./0123456789:;<=>?@\nABCDEFGHIJKLMNOPQRSTUVWXYZ\n[\\]^_'\nabcdefghijklmnopqrstuvwxyz\n{|}~", GUI_MB_OK);
#if USE_TRANPARENT_TITLES
  gui_message_box(NULL, "A Transparent Titlebar", "Transparency is easy as adding a value\nto the T portion of the color.\n"
    "Also notice we can stretch an image to\nfit a desired size as in the desktop image.", GUI_MB_OK);
#endif
  
  //////////////////////////////////////////////////////////////////////
  // list box example
  struct APP0 *app0;
  app0 = newapp0();
  obj_position(GUIOBJ(app0), 50, 275, 200, 225);
  textual_set(GUITEXTUAL(app0), "A list example", -1, FALSE);
  obj_geometry(GUIOBJ(app0));
  
  //////////////////////////////////////////////////////////////////////
  // progress bar, slider bars, and updown example
  struct APP1 *app1;
  app1 = newapp1();
  app1->progress.percent = 45;  // start the percent at 45
  app1->progress.show = TRUE;   // show the percentage as text also.
  app1->toggle[0].checked = TRUE;  // show the percentage as text also.
  
  sliderbar_setlimits(&app1->hslider, -10, 10);
  sliderbar_setcur(&app1->hslider, 0);
  sliderbar_setratio(&app1->hslider, 2);
  sliderbar_setflags(&app1->hslider, SLIDERBAR_HORZ | SLIDERBAR_TICKS | SLIDERBAR_WIDE | SLIDERBAR_NUMS | SLIDERBAR_POINTED);
  
  sliderbar_setlimits(&app1->vslider, -20, 20);
  sliderbar_setcur(&app1->vslider, 0);
  sliderbar_setratio(&app1->vslider, 4);
  sliderbar_setflags(&app1->vslider, SLIDERBAR_VERT | SLIDERBAR_TICKS | SLIDERBAR_WIDE | SLIDERBAR_NUMS | SLIDERBAR_POINTED);
  
  updown_setcur(&app1->updown, -999);
  updown_setflags(&app1->updown, UPDOWN_LEFT_TEXT);
  //updown_setflags(&app1->updown, UPDOWN_LEFT_TEXT | UPDOWN_FIT_TO_TEXT);
  //updown_setflags(&app1->updown, UPDOWN_RIGHT_TEXT);
  
  obj_position(GUIOBJ(app1), 275, 50, 485, 275);
  textual_set(GUITEXTUAL(app1), "Object Examples", -1, FALSE);
  obj_geometry(GUIOBJ(app1));

  //////////////////////////////////////////////////////////////////////
  // An Edit example
  struct APP2 *app2;
  app2 = newapp2();
  obj_position(GUIOBJ(app2), 400, 275, 535, 325);
  textual_set(GUITEXTUAL(app2), "An Edit Example", -1, FALSE);
  obj_geometry(GUIOBJ(app2));
  
  //////////////////////////////////////////////////////////////////////
  // show some icons example
  struct WIN *win_icons;
  win_icons = obj_win(NULL, DESKTOP, 0, NULL, 0, BORDER_NORMAL);
  obj_position(GUIOBJ(win_icons), 100, 650, 500, 36);
  textual_set(GUITEXTUAL(win_icons), "Icons", -1, FALSE);
  obj_geometry(GUIOBJ(win_icons));
  
  struct IMAGE image[13];
  static int image_ids[13] = { 
    ID_ICON_USER_ADD, 
    ID_ICON_CAUTION,
    ID_ANIMATE_DEMO,  // this one has already been uncommented for you
    ID_ICON_BUSY,     //   (even though the book explaines to do it)
    ID_ICON_ARROW_LEFT,
    ID_ICON_FOLDER_OPEN,
    ID_ICON_FOLDER_CLOSED,
    ID_ICON_COPY,
    ID_ICON_DELETE,
    ID_ICON_DOCUMENT,
    ID_ICON_MAIL,
    ID_ICON_PROMPT,
    ID_ICON_TRASH
  };
  
  for (i=0; i<13; i++) {
    obj_image(&image[i], sizeof(struct IMAGE), GUIOBJ(win_icons), 0);
    obj_defaultrect(GUIOBJ(&image[i]), NULL);
    image_ownerdraw(&image[i], image_ids[i], 32, 32);
    if (i == 0)
      obj_layout(GUIOBJ(&image[0]), (LAYOUT) (LAYOUT_X1 | LAYOUT_Y1), GUIOBJ(win_icons), 3, 3);
    else
      obj_layout(GUIOBJ(&image[i]), (LAYOUT) (LAYOUT_RIGHT | LAYOUT_Y1), GUIOBJ(&image[i-1]), 5, 0);
  }
  
  //////////////////////////////////////////////////////////////////////
  // A File Select example
  struct FILESEL *filesel = fileselwin(NULL, DESKTOP, 0, NULL, 0, 1);
  textual_set(GUITEXTUAL(filesel), "File select", -1, FALSE);
  textual_set(&filesel->dir, getcwd(filesel->current, PATH_MAX - 1), -1, FALSE);
  obj_defaultrect(GUIOBJ(filesel), NULL);
  obj_layout(GUIOBJ(filesel), LAYOUT_CENTER, 0, 0, 0);
  obj_geometry(GUIOBJ(filesel));
  win_activate(GUIWIN(filesel));
  win_dirty(GUIWIN(filesel));
  
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  // Start the windowing system.
  // Once we do this, we don't come back until we exit with the
  //  intent of returning to DOS
  gui_execute();
  
  // restore the original mouse handler
  remove_mouse_handler();
  
  // free some memory objects  
  free_static_images();
  if (desktop_img.bitmap)
    free(desktop_img.bitmap);
  
  // free the screen buffer selector
  if (linear_addressing)
    __dpmi_free_physical_address_mapping(&base_mi);
  
  // return back to screen mode 3
  vid_set_text_mode(0x03);
  
  fclose(dfp);
  return 0;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * helper functions
 *  
 */

/* event_default()
 *       no parameters
 *
 * do an event
 *   
 */
void event_default(void) {
  if (GUIOBJ(eventstack.object)->win->handler)
    GUIOBJ(eventstack.object)->win->handler(GUIOBJ(eventstack.object)->win);
  else
    GUIOBJ(eventstack.object)->_class();
}

/* obj_event()
 *        obj = pointer to object to send event to
 *      event = the event enum to send to the object
 *       data = pointer to optional data to send to the object
 *
 * sends an event to the object specified
 *   
 */
void *obj_event(struct OBJECT *obj, EVENT event, const void *data) {
  void *ret = NULL;
  const struct EVENTSTACK prev = eventstack;
  struct WIN *win;
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  win = obj->win;
  
  if (event != DESTRUCT) {
    atom_lock(&win->base.atom);
    atom_lock(&obj->base.atom);
  }
  
  // Before we try to send a message, filter out interactive messages when an app has a modal window open
  if (win->modallock && (event > INTERACT_BEGIN) && (event < INTERACT_END))
    goto done;
  
  // A (little) dirty hack to make the eventstack.object a derived class of OBJECT
  //  which makes the nice macros usable even for eventstack.object.  The cast is safe
  //  becasue DERIVED is only a wrapper of OBJECT and has (must have!) no 
  //  data of its own
  eventstack.object = (struct DERIVED *) obj;
  eventstack.event = event;
  eventstack.data = data;
  eventstack.answer = NULL;
  
  gui_event();
  
  ret = eventstack.answer;
  eventstack = prev;
  
done:
  if (event != DESTRUCT) {
    atom_unlock(&obj->base.atom);
    atom_unlock(&win->base.atom);
  }
  
  return ret;
}

/* emit()
 *      event = the event enum to send to the object
 *       data = pointer to optional data to send to the object
 *
 * send an event to the current object's direct parent
 *   
 */
void *emit(EVENT event, const void *data) {
  if (GUIOBJ(eventstack.object)->parent)
    return obj_event(GUIOBJ(eventstack.object)->parent, event, data);
  
  return NULL;
}

/* inform()
 *      event = the event enum to send to the object
 *       data = pointer to optional data to send to the object
 *
 * send an event to the current object's direct parent window
 *   
 */
void *inform(EVENT event, const void *data) {
  if (GUIOBJ(eventstack.object)->win->parent)
    return obj_event(GUIOBJ(GUIOBJ(eventstack.object)->win->parent), event, data);
  
  return NULL;
}

/* answer()
 *     answer = pointer to object that is answering
 *
 * used in want_focus(), for example, and returns pointer if true
 *   or NULL if not true
 *   
 */
void answer(void *answer) {
  eventstack.answer = answer;
}

/* pointer_info()
 *     no parameters
 *
 * 
 * returns a pointer to the mouse cursor info
 *   
 */
const struct POINTER_INFO *pointer_info(void) {
  return &root.pointer;
}

/* pointer_hold()
 *     no parameters
 *
 * 
 * are we moving the object with the pointer button held
 *   
 */
bool pointer_hold(void) {
  if (GUIOBJ(eventstack.object) == root.pointed) {
    root.pointerhold = root.pointed;
    return TRUE;
  }
  return FALSE;
}

/* pointer_release()
 *     no parameters
 *
 * 
 * we released the item
 *   
 */
bool pointer_release(void) {
  if (GUIOBJ(eventstack.object) == root.pointerhold) {
    root.pointerhold = NULL;
    return TRUE;
  }
  return FALSE;
}

/* obj_x()
 *     obj = the object in question
 *
 * 
 * return the x position of the object relative to its parent
 *   
 */
int obj_x(struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  if (obj->parent)
    return gui_left(obj) - gui_left(obj->parent);
  
  return gui_left(obj);
}

/* obj_y()
 *     obj = the object in question
 *
 * 
 * return the y position of the object relative to its parent
 *   
 */
int obj_y(struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  if (obj->parent)
    return gui_top(obj) - gui_top(obj->parent);
  
  return gui_top(obj);
}

/* obj__move()
 *     obj = the object in question
 *      dx = x coordinate to move to
 *      dy = y coordinate to move to
 *
 * 
 * move object to these coordinates *including all children*
 *   
 */
void obj__move(struct OBJECT *obj, const int dx, const int dy) {
  struct RECT rect;
  rect = *GUIRECT(obj);
  
  rect.left -= dx;
  rect.top -= dy;
  rect.right -= dx;
  rect.bottom -= dy;
  rectatom_place(GUIRECTATOM(obj), &rect);
  
  obj = obj->last;
  while (obj) {
    obj__move(obj, dx, dy);
    obj = obj->prev;
  }
}

/* obj_place()
 *     obj = the object in question
 *    rect = a rect holding coordinates and size
 *
 * 
 * place object to these coordinates *including all children*
 *   
 */
void obj_place(struct OBJECT *obj, const struct RECT *rect) {
  struct OBJECT *ptr;
  int dx, dy;
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  ptr = obj->last;
  dx = gui_left(obj);
  dy = gui_top(obj);
  
  rectatom_place(GUIRECTATOM(obj), rect);
  
  dx -= gui_left(obj);
  dy -= gui_top(obj);
  
  while (ptr) {
    obj__move(ptr, dx, dy);
    ptr = ptr->prev;
  }
}

/* obj_position()
 *     obj = the object in question
 *       x = x coordinate of new position (relative to parent)
 *       y = y coordinate of new position
 *       w = new width of object
 *       h = new height of object
 *
 *  may pass GUIDEF in parameters to use existing location/size
 *  including moving of all children
 *   
 */
void obj_position(struct OBJECT *obj, int x, int y, int w, int h) {
  struct RECT rect;
  memset(&rect, 0, sizeof(struct RECT));
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  rect = *GUIRECT(obj);
  if (x != GUIDEF)
    rect.left = x + gui_left(obj->parent);
  if (y != GUIDEF)
    rect.top = y + gui_top(obj->parent);
  if (w == GUIDEF)
    w = gui_w(obj);
  if (h == GUIDEF)
    h = gui_h(obj);
  rect.right = rect.left + w - 1;
  rect.bottom = rect.top + h - 1;
  
  obj_place(obj, &rect);
}


/* obj_move()
 *     obj = the object in question
 *       x = x coordinate to move to
 *       y = y coordinate to move to
 * 
 * move object to these coordinates
 *   
 */
void obj_move(struct OBJECT *obj, int x, int y) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj_position(obj, x, y, gui_w(obj), gui_h(obj));
}

/* obj_resize()
 *     obj = the object in question
 *       w = new width
 *       h = new height
 * 
 *  may pass GUIDEF in parameters to use existing location/size
 * resize object to this size
 *   
 */
void obj_resize(struct OBJECT *obj, int w, int h) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj_position(obj, obj_x(obj), obj_y(obj), w, h);
}

/* obj_geometry()
 *     obj = the object in question
 * 
 *  sends a GEOMETRY event to object specified
 *   
 */
void obj_geometry(struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj_event(obj, GEOMETRY, 0);
}

/* defaultrect_data()
 *     no parameters
 * 
 *  while in an event handler, if event is DEFAULTRECT
 *   this will return RECT of object
 *   
 */
struct RECT *defaultrect_data(void) {
  if (eventstack.event == DEFAULTRECT)
    return (struct RECT *) eventstack.data;
  
  return NULL;
}

/* obj_defaultrect()
 *     obj = the object in question
 *    rect = pointer to rect for return data
 * 
 *  sends a default rect event to object
 *   sending current size data in rect
 *   
 */
void obj_defaultrect(struct OBJECT *obj, struct RECT *rect) {
  struct RECT data;
  memset(&data, 0, sizeof(struct RECT));
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  data = *GUIRECT(obj);
  
  obj_event(obj, DEFAULTRECT, &data);
  
  if (rect)
    *rect = data;
  else
    obj_place(obj, &data);
}

/* obj_layout()
 *     obj = the object in question
 *   flags = layout flags
 *   other = the object to use for reference
 *       x = delta reference
 *       y = delta reference
 *   
 *  places obj relative to other using flags and x and y
 *
 */
void obj_layout(struct OBJECT *obj, LAYOUT flags, const struct OBJECT *other, int x, int y) {
  int w, h;
  struct RECT rect, objrect;
  
  // if obj == NULL, use object specified in event
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  // if other == NULL, use root
  if (!other)
    other = DESKTOPOBJ;
  
  // get current rects for both
  objrect = *GUIRECT(obj);
  rect = *GUIRECT(other);
  
  // get width and height of target rect
  w = objrect.right - objrect.left;
  h = objrect.bottom - objrect.top;
  
  // if LAYOUT_W used, use width of referenced object
  if (flags & LAYOUT_W)
    w = rect.right - rect.left;
  
  // if LAYOUT_H used, use height of referenced object
  if (flags & LAYOUT_H)
    h = rect.bottom - rect.top;
  
  // place to the left of referenced object
  //  positive x moves as a delta to the left
  //  negative x moves as a delta to the right
  if (flags & LAYOUT_LEFT)
    objrect.left = rect.left - w - x;
  
  // place to the right of referenced object
  //  positive x moves as a delta to the right
  //  negative x moves as a delta to the left
  else if (flags & LAYOUT_RIGHT)
    objrect.left = rect.right + x;
  
  // place to the right of left coordinate of referenced object
  //  positive x moves as a delta to the right
  //  negative x moves as a delta to the left
  else if (flags & LAYOUT_X1)
    objrect.left = rect.left + x;
  
  // place to the left of right coordinate of referenced object
  //  positive x moves as a delta to the right
  //  negative x moves as a delta to the left
  else if (flags & LAYOUT_X2)
    objrect.left = rect.right - w + x;
  
  // horz center of referenced object
  //  positive x moves as a delta to the right
  //  negative x moves as a delta to the left
  else if (flags & LAYOUT_HCENTER)
    objrect.left = rect.left + ((rect.right - rect.left - w) / 2) + x;
  
  // place on top of referenced object
  //  positive y moves as a delta up
  //  negative y moves as a delta down
  if (flags & LAYOUT_TOP)
    objrect.top = rect.top - h - y;
  
  // place on bottom of referenced object
  //  positive y moves as a delta down
  //  negative y moves as a delta up
  else if (flags & LAYOUT_BOTTOM)
    objrect.top = rect.bottom + y;
  
  // place top of target in reference to top coordinate of referenced object
  //  positive y moves as a delta down
  //  negative y moves as a delta up
  else if (flags & LAYOUT_Y1)
    objrect.top = rect.top + y;
  
  // place top of target in reference to bottom coordinate of referenced object
  //  positive y moves as a delta down
  //  negative y moves as a delta up
  else if (flags & LAYOUT_Y2)
    objrect.top = rect.bottom - h + y;
  
  // vert centers target in reference to refreenced object
  //  positive y moves as a delta down
  //  negative y moves as a delta up
  else if (flags & LAYOUT_VCENTER)
    objrect.top = rect.top + ((rect.bottom - rect.top - h) / 2) + y;
  
  // update size
  objrect.right = objrect.left + w;
  objrect.bottom = objrect.top + h;
  
  // place the object
  obj_place(obj, &objrect);
}

/* obj_wantmove()
 *         obj = the object in question
 *    wantmove = 0 = don't allow object to capture mouse movements and move
 *               1 = allow object to capture mouse movements and move (title bar for example)
 *   
 *  gives (or takes) the ability for the object to be grabbed and moved
 *
 */
void obj_wantmove(struct OBJECT *obj, bool wantmove) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->wantmove = wantmove;
}

/* obj_armed()
 *         obj = the object in question
 *   
 *  returns true if objected is armed
 *
 */
bool obj_armed(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  return obj->armed;
}

/* obj_arm()
 *         obj = the object in question
 *         arm = 0 = mark as not armed
 *               1 = mark as armed
 *   
 *  if armable, mark as (not) armed
 *
 */
void *obj_arm(struct OBJECT *obj, bool arm) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  if (!obj->armable)
      return NULL;
  
  if (obj->disabled)
      return NULL;
  
  if (obj->armed && arm)
      return NULL;
  
  if (!obj->armed && !arm)
      return NULL;
  
  return obj_event(obj, ARM, arm ? obj : NULL);
}

/* obj_armable()
 *         obj = the object in question
 *     armable = 0 = object is not to be armed
 *               1 = object can be armed
 *
 *  mark object as able (or not) to be armed
 *
 */
void obj_armable(struct OBJECT *obj, bool armable) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->armable = armable;
}

/* obj_is_armable()
 *         obj = the object in question
 *
 *  returns true if obj can be armed
 *
 */
bool obj_is_armable(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  return obj->armable;
}

/* obj_selected()
 *         obj = the object in question
 *   
 *  returns true if objected is selected
 *
 */
bool obj_selected(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  return obj->selected;
}

/* obj_select()
 *         obj = the object in question
 *      select = 0 = mark as not selected
 *               1 = mark as selected
 *   
 *  if selectable, mark as (not) selected
 *
 */
void *obj_select(struct OBJECT *obj, bool select) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  if (!obj->selectable)
    return NULL;
  
  if (obj->disabled)
    return NULL;
  
  if (obj->selected && select)
    return NULL;
    
  if (!obj->selected && !select)
    return NULL;
  
  return obj_event(obj, SELECT, select ? obj : NULL);
}

/* obj_selectable()
 *         obj = the object in question
 *  selectable = 0 = object is not to be selected
 *               1 = object can be selected
 *
 *  mark object as able (or not) to be selected
 *
 */
void obj_selectable(struct OBJECT *obj, bool selectable) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->selectable = selectable;
}

/* obj_is_selectable()
 *         obj = the object in question
 *
 *  returns true if obj can be selected
 *
 */
bool obj_is_selectable(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  return obj->selectable;
}

/* obj_set_mnemonic()
 *         obj = the object in question
 *         set = 0 = object does not allow mnemonics
 *               1 = object allows mnemonics
 *   
 *  sets an object to allow (or disallow) mnemonics
 *  (a button may have mnemonic (&Cancel))
 *
 */
bool obj_set_mnemonic(struct OBJECT *obj, const bool set) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->has_mnemonic = set;

  return 0;
}

/* obj_has_mnemonic()
 *         obj = the object in question
 *   
 *  returns true if object may have a mnemonic
 *   * does not check for the mnemonic, just that it may 
 *     or may not have one *
 *
 */
bool obj_has_mnemonic(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  return obj->has_mnemonic;
}

/* obj_disable()
 *         obj = the object in question
 *     disable = 0 = object is not to be disabled
 *               1 = object can be disabled
 *   
 *  mark object as able (or not) to be disabled
 *
 */
void obj_disable(struct OBJECT *obj, bool disable) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->disabled = disable;
}

/* obj_disabled()
 *         obj = the object in question
 *   
 *  returns true if objected is disabled
 *
 */
bool obj_disabled(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  // The object might be blocked because of a modal window
  //  so return disabled.
  if (obj->win->modallock)
    return TRUE;
  
  // The object might just be disabled
  return obj->disabled;
}

/* obj_visible()
 *         obj = the object in question
 *     visible = 0 = object is not to be visible
 *               1 = object can be visible
 *   
 *  mark object as able (or not) to be visible
 *
 */
void obj_visible(struct OBJECT *obj, bool visible) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  obj->visible = visible;
}

/* obj_isvisible()
 *         obj = the object in question
 *   
 *  returns true if objected is visible
 *   * doesn't mean that object isn't hidden by other window
 *     just means that the object can be drawn *
 *
 */
bool obj_isvisible(const struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  // The object might be blocked because of a modal window
  if (obj->win->modallock)
    return TRUE;
  
  return obj->visible;
}

/* obj_top()
 *         obj = the object in question
 *   
 *  moves obj to the top of the z-order
 *   (only a WIN has this event handler)
 *
 */
void *obj_top(struct OBJECT *obj) {
  struct OBJECT *parent;
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  parent = obj->parent;
  if (!parent)
    return obj;
  
  if (!obj->next)
    return obj;
  
  node_remove(obj);
  node_insert(obj, parent);
  
  return obj_event(obj, TOP, 0);
}

/* obj_get_focus()
 *     no parameters
 *   
 *  returns the obj that has the current focus
 *
 */
struct OBJECT *obj_get_focus() {
  return root.focus;
}

/* obj_focus()
 *         obj = the object in question
 *   
 *  sets the focus to the object specified
 *  (only if object answers that it can receive focus)
 *
 */
void obj_focus(struct OBJECT *obj) {
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  if (obj == root.focus)
    return;
  
  if (obj_event(obj, FOCUS_WANT, root.focus)) {
    if (root.focus)
      obj_event(root.focus, FOCUS_LOST, obj);
    
    root.focus = obj;
    
    if (root.focus)
      obj_event(root.focus, FOCUS_GOT, NULL);
  }
}

/* cur_active_win()
 *     no parameters
 *   
 *  returns the the active window
 *
 */
struct WIN *cur_active_win(void) {
  return root.active;
}

/* win_get_menu()
 *        win = the window in question
 *   
 *  returns the pointer to the menu of the window in question
 *   (could return it as NULL if menu never created)
 *
 */
struct MENU *win_get_menu(struct WIN *win) {
  if (win == NULL)
    win = cur_active_win();
  return win->menu;
}

/* win_get_button_bar()
 *        win = the window in question
 *   
 *  returns the pointer to the button_bar of the window in question
 *   (could return it as NULL if menu never created)
 *
 */
struct BUTTON_BAR *win_get_button_bar(struct WIN *win) {
  if (win == NULL)
    win = cur_active_win();
  return win->button_bar;
}

/* win_active()
 *        win = the window in question
 *   
 *  returns true if this window is the active window
 *
 */
bool win_active(const struct WIN *win) {
  if (win == root.active)
    return TRUE;
  
  return FALSE;
}

/* win_activate()
 *        win = the window in question
 *   
 *  try to active this window
 *  (will deactivate other window)
 *
 */
bool win_activate(struct WIN *win) {
  struct WIN *oldactive = root.active;
  
  if (win == oldactive)
    return FALSE;
  
  root.active = win;
  
  if (oldactive)
    obj_event(GUIOBJ(oldactive), ACTIVE, NULL);
  
  if (win)
    obj_event(GUIOBJ(win), ACTIVE, win);
  
  return TRUE;
}

/* obj_event_recurse()
 *        obj = this object and all child objects of this object
 *      event = event to send
 *       data = pointer to optional data
 *   
 *  sends 'event' to all objects that contain a class() call
 *
 */
void obj_event_recurse(struct OBJECT *obj, EVENT event, const void *data) {
  struct OBJECT *ptr = obj->last;
  
  while (ptr) {
    if (ptr->_class) obj_event(ptr, event, data);
    obj_event_recurse(ptr, event, data);
    ptr = ptr->prev;
  }
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Object
 *  
 */

/* obj_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 *  most objects should have already caught all events, but this
 *   will be the last event handler if no.
 *  however, a destruct event will come clear to here so that
 *   we can free the memory
 *  other (general) events will come to here too
 *  
 */
void obj_class(void) {
  struct OBJECT *obj = GUIOBJ(eventstack.object);
  
  switch (eventstack.event) {
    // kill the object
    case DESTRUCT:
      while (obj->last) {
        struct OBJECT *ptr = obj->last;
        node_remove(ptr);
        atom_delete(&ptr->base.atom);
      }
      break;
      
    // most objects should have an expose handler, but just
    //  incase, we draw a rectangle background
    case EXPOSE:
      gui_obj(obj);
      break;
      
    // arm the event if .data != NULL
    case ARM:
      if (obj->armable)
        obj->armed = (eventstack.data) ? TRUE : FALSE;
      break;
      
    // select the event if .data != NULL
    case SELECT:
      if (obj->selectable)
        obj->selected = (eventstack.data) ? TRUE : FALSE;
      break;
      
    case FOCUS_WANT:
      // only want focus if not disabled
      if (obj_disabled(obj))
        answer(NULL);
      else {
        answer(obj);
        obj_dirty(obj, FALSE);
      }
      return;
      
    case FOCUS_LOST:
      // since we might "decorate" the obj when it has the focus,
      //  we need to undecorate it when we lose the focus
      obj_dirty(obj, FALSE);
      return;
      
    // if object is armable, it will be armed
    // if object can gain focus, it will gain the focus
    case LPOINTER_PRESS:
      obj_arm(GUIOBJ(eventstack.object), TRUE);
      obj_focus(GUIOBJ(eventstack.object));
      break;
      
    // if object was armed, it will be unarmed
    case LPOINTER_RELEASE:
      if (obj->armed) {
        obj_event(GUIOBJ(eventstack.object), POINTER_CLICK, 0);
        obj_arm(GUIOBJ(eventstack.object), FALSE);
      }
      break;
      
    // if mouse cursor left object, make sure it is un armed
    // (valuable incase mouse cursor leave movable object while
    //   trying to move it)
    case POINTER_LEAVE:
      if (obj->armed)
        obj_arm(GUIOBJ(eventstack.object), FALSE);
      break;
  }
}

/* destroy()
 *       atom = the atom to destroy
 *   
 *  will remove atom of parent, will free memory, and destroy atom
 *
 */
void destroy(void *atom) {
  struct OBJECT *obj = (struct OBJECT *) atom;
  
  if (obj->parent)
    gfx_dirty(GUIRECT(obj));
  
  obj_event(obj, DESTRUCT, 0);
  
  node_remove(obj);
  
  if (obj == root.pointed) {
    root.pointed = 0;
    root.pointer.x = -1;
  }
  
  if (obj == root.pointerhold) {
    root.pointerhold = NULL;
    root.pointer.x = -1;
  }
  
  if (obj == GUIOBJ(root.active))
    root.active = NULL;
  
  if (obj == root.focus)
    root.focus = NULL;
}

/*  object()
 *          obj = pointer to the object
 *       _class = class handler to use
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the object
 *
 *  creates an object
 *    all other objects call this to create themselves first
 *   
 */
void *object(struct OBJECT *obj, CLASS _class, bit32u size, struct OBJECT *parent, int theid) {
  struct RECT rect;
  
  if (!parent)
    parent = DESKTOPOBJ;
  
  MINSIZE(size, struct OBJECT);
  obj = (struct OBJECT *) obj_rectatom(GUIRECTATOM(obj), destroy, size);
  
  obj->_class = (_class) ? _class : obj_class;
  obj->win = (obj == DESKTOPOBJ) ? DESKTOP : parent->win;
  obj->armed = FALSE;
  obj->selected = FALSE;
  obj->id = theid;
  obj->visible = TRUE;
  obj->has_mnemonic = FALSE;
  
  rect.left = -1;
  rect.top = -1;
  rect.right = -1;
  rect.bottom = -1;
  rectatom_place(GUIRECTATOM(obj), &rect);
  
  if (obj != parent)
    node_insert(obj, parent);
  
  return obj;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Image Object
 *  
 */

/* image_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 * NOTE:
 *  to make a BITMAP animate, it must be created as
 *   an image object
 *  
 */
void image_class(void) {
  struct IMAGE *image = (IMAGE *) eventstack.object;
  
  switch (eventstack.event) {
    // draw image to screen
    case EXPOSE:
      gui_image(image);
      return;

    // called everytime there is a clock() tick
    case SEC_ELAPSED:
      if (image->bitmap->count > 1) {
        if (--image->bitmap->count_down <= 0) {
          if (++image->bitmap->current == image->bitmap->count)
            image->bitmap->current = 0;
          image->bitmap->count_down = image->bitmap->delay_array[image->bitmap->current];
          obj_dirty(GUIOBJ(image), TRUE);
        }
      }
      return;
      
    // give it a default size of 20x20
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + 20;
      rect->bottom = rect->top + 20;
    } return;
  }
  
  // call the object class
  obj_class();
}

/*  obj_image()
 *        image = pointer to the image object
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the object
 *
 *  creates an image object
 *   
 */
struct IMAGE *obj_image(struct IMAGE *image, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct IMAGE);
  
  image = (struct IMAGE *) object(GUIOBJ(image), image_class, size, parent, theid);
  if (image) {
    image->bitmap = NULL;
    image->flags = 0;
  }
  
  // return image object
  return image;
}

/*  image_ownerdraw()
 *        image = pointer to the image object
 *           id = id to retrieve from static images
 *            w = width of object (or 0 for any)
 *            h = width of object (or 0 for any)
 *
 *  creates an image object from the static images file
 *   
 */
void image_ownerdraw(struct IMAGE *image, const int id, int w, int h) {
  image->bitmap = get_static_bitmap(id, w, h);
  if (image->bitmap)
    obj_resize(GUIOBJ(image), gui_w(image->bitmap), gui_h(image->bitmap));
  else {
    if (!w && !h)
      w = h = 16;
    image->bitmap = obj_bitmap(w, h, 1);
    obj_resize(GUIOBJ(image), w, h);
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Button Object
 *  
 */

/* button_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 */
void button_class(void) {
  struct BUTTON *button = (struct BUTTON *) eventstack.object;
  
  switch (eventstack.event) {
    // draw button to screen
    case EXPOSE:
      gui_button(button);
      return;
      
    // mark button as dirty if armed or selected
    case ARM:
    case SELECT:
      obj_dirty(GUIOBJ(button), TRUE);
      break;
      
    // if we click on the button, tell parent and select button
    case POINTER_CLICK:
      if (!obj_selected(GUIOBJ(button)))
        emit((EVENT) GUIOBJ(button)->id, button);
      obj_select(GUIOBJ(button), !obj_selected(GUIOBJ(button)));
      break;
      
    // give the button a default size of if has bitmap, size to bitmap
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      if (button->bitmap) {
        rect->right = rect->left + gui_w(button->bitmap);
        rect->bottom = rect->top + gui_h(button->bitmap);
      } else {
        rect->right = rect->left + textual_width(GUITEXTUAL(button)) + 8;
        rect->bottom = rect->top + textual_height(GUITEXTUAL(button)) + 4;
      }
    } return;
      
    case KEY: {
      const struct KEY_INFO *key_info = (const struct KEY_INFO *) eventstack.data;
      if (key_info->code == VK_ENTER)
        obj_event(GUIOBJ(button), POINTER_CLICK, NULL);
    } break;
  }
  
  // call the textual class to draw text
  textual_class();
}

/*  obj_button()
 *       button = pointer to the button object
 *         size = size of memory to allocate
 *       parent = parent object (usually a window)
 *        theid = id of the object
 *        flags = flags for the button
 *
 *  creates a button object
 *   
 */
struct BUTTON *obj_button(struct BUTTON *button, bit32u size, struct OBJECT *parent, int theid, const bit32u flags) {
  MINSIZE(size, struct BUTTON);
  
  // button objects are textual objects with a border drawn around them
  button = (struct BUTTON *) obj_textual(GUITEXTUAL(button), size, parent, theid);
  if (button) {
    GUIOBJ(button)->_class = button_class;
    button->bitmap = NULL;
    button->flags = flags;
    GUITEXTUAL(button)->flags |= TEXTUAL_FLAGS_MNEMONIC;  // allow mnemonics
    GUITEXTUAL(button)->align = (ALIGN) ALIGN_CENTER;     // center the text in the button
    
    // armable and selectable
    obj_armable(GUIOBJ(button), TRUE);
    obj_selectable(GUIOBJ(button), TRUE);
    GUIOBJ(button)->has_mnemonic = TRUE;
  }
  
  // return the button object
  return button;
}

/*  button_ownerdraw()
 *       button = pointer to the button object
 *           id = id to retrieve from static images
 *            w = width of object (or 0 for any)
 *            h = width of object (or 0 for any)
 *
 *  gets a bitmap for the button
 *   
 */
void button_ownerdraw(struct BUTTON *button, const int id, int w, int h) {
  button->bitmap = get_static_bitmap(id, w, h);
  if (button->bitmap)
    obj_resize(GUIOBJ(button), gui_w(button->bitmap), gui_h(button->bitmap));
  else {
    if (!w && !h)
      w = h = 16;
    button->bitmap = obj_bitmap(w, h, 1);
    obj_resize(GUIOBJ(button), w, h);
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Window Object
 *  
 */

/*  win_dirty()
 *          win = pointer to the window
 *
 *  marks the win as dirty
 *   
 */
void win_dirty(struct WIN *win) {
  if (win) {
    obj_dirty(GUIOBJ(win), TRUE);
  
    if (win->border)
      obj_dirty(GUIOBJ(win->border), TRUE);
  }
}

/* win_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 */
void win_class(void) {
  struct WIN *win = (struct WIN *) eventstack.object;
  
  switch (eventstack.event) {
    // kill the window and everything in it.
    case DESTRUCT:
      if (win->ismodal && win->parent) {
        win->parent->modallock--;
        win_dirty(win->parent);
      }
      
      if (win->border)
        atom_delete(&win->border->base.atom);
      
      if (win->menu)
        atom_delete(&win->menu->base.atom);
      
      if (win->button_bar)
        atom_delete(&win->button_bar->base.atom);
      
      if (win->icon)
        atom_delete(&win->icon->base.atom);
      
      while (win->last) {
        struct WIN *ptr = win->last;
        node_remove(ptr);
        atom_delete(&ptr->base.atom);
      }
      
      node_remove(win);
      break;
      
    // draw the window
    //  (draws only the background)
    case EXPOSE:
      gui_win(win);
      return;
      
    // move window to top
    case TOP:
      if (win->border) {
        struct OBJECT *parent = GUIOBJ(eventstack.object)->parent;
        
        node_remove(GUIOBJ(win->border));
        node_insert(GUIOBJ(win->border), parent);
        
        node_remove(GUIOBJ(eventstack.object));
        node_insert(GUIOBJ(eventstack.object), parent);
      }
      
      win_dirty(win);
      break;

    // activate window
    case ACTIVE:
      if (win->border)
        obj_dirty(GUIOBJ(win->border), TRUE);
      return;
      
    // do the geometry for the window
    case GEOMETRY:
      if (win->border) {
        obj_defaultrect(GUIOBJ(win->border), NULL);
        obj_geometry(GUIOBJ(win->border));
      }
      return;
  }
  
  // call the textual class for the title bar
  textual_class();
}

/*  win_handler()
 *          win = pointer to the window
 *
 *  activates the window, bringing it to the top, then
 *   calls the class for the object clicked/hovered/left/whatever
 *  then call the callback for the border class
 *   
 */
void win_handler(struct WIN *win) {
  // we need to activate the window and bring it
  //  to the top of an object within this window
  //  was clicked
  if (eventstack.event == LPOINTER_PRESS) {
    obj_top(GUIOBJ(win));
    win_activate(win);
  }
  
  // call the objects class
  GUIOBJ(eventstack.object)->_class();
  
  // call the callback routine for the border
  if (win->border && win->border->callback)
    win->border->callback(win);
}

/*  obj_win()
 *          win = pointer to the window object
 *   parent_win = parent window of this new window
 *         size = size of memory to allocate
 *      handler = pointer to the handler for this window
 *        theid = id of the object
 *        flags = flags for the win
 *
 *  creates a window object
 *   
 */
struct WIN *obj_win(struct WIN *win, struct WIN *parent_win, bit32u size, HANDLER handler, const int theid, const bit32u flags) {
  MINSIZE(size, struct WIN);
  
  // if handler not specified, use default handler above
  // (this allows the user to overide the handler and call their own)
  if (!handler)
    handler = win_handler;
  
  // create as a textual object since the title is a text object
  win = (struct WIN *) obj_textual(GUITEXTUAL(win), size, GUIOBJ(parent_win), theid);
  if (win) {
    GUIOBJ(win)->_class = win_class;
    GUIOBJ(win)->win = win;
    win->handler = handler;
    win->flags = flags;
    win->icon = NULL;
    
    obj_armable(GUIOBJ(win), TRUE);
    
    win->border = winborder(NULL, 0, win, ID_BORDER_WIN, flags);
    win->menu = NULL;
    win->button_bar = NULL;
    // if you would like a specific font for the title bar, uncomment this line
    //textual_set_font(GUITEXTUAL(win), "Simple");
    
    node_remove(GUIOBJ(win));
    node_insert(GUIOBJ(win), DESKTOPOBJ);
  }
  
  // return window object
  return win;
}

/*  win_set_icon_id()
 *          win = pointer to the window
 *           id = id to use from static images
 *
 *  set the icon for the window (id)
 *  (must be 16x14)
 *
 *  (TODO: what if font is set in function above with smaller font)
 *         may want to get size of font used)
 *  (TODO: with this in mind, may want to allow get_static_bitmap to
 *         get a smaller image than specified if > 0, rather than exactly
 *         that size.  then if smaller found, change (or stretch) to size
 *         specified.)
 *   
 */
void win_set_icon_id(struct WIN *win, const bit32u id) {
  win->icon = get_static_bitmap(id, 16, 14);
}

/*  win_set_icon_id()
 *          win = pointer to the window
 *     filename = string holding filename to get
 *
 * set the icon for the window (from filename)
 * filename is an image file to retrieve
 *   
 */
void win_set_icon_name(struct WIN *win, const char *filename) {
  win->icon = get_bitmap(filename, FALSE);
}

/*  win_child()
 *          win = pointer to the window
 *       target = pointer to child window to add
 *
 * insert a window into the list of windows
 *   
 */
void win_child(struct WIN *win, struct WIN *target) {
  node_remove(win);
  node_insert(win, target);
}

/*  win_child()
 *          win = pointer to the window
 *       target = pointer to child window
 *
 * insert a window into the list of windows making child a modal of
 *  (don't allow parent to catch events until child is destroyed)
 *   
 */
void win_modal(struct WIN *win, struct WIN *target) {
  win_child(win, target);
  
  if (target) {
    win->ismodal = TRUE;
    target->modallock++;
    
    win_dirty(target);
  }
}

/*  win_set_status()
 *          win = pointer to the window
 *       status = string to set status line to
 *
 * set the window status line text (if status present)
 *   
 */
void win_set_status(struct WIN *win, const char *status) {
  if (win->border->flags & BORDER_HAS_STATUS) {
    textual_set(&win->border->status, status, -1, FALSE);
    obj_dirty(GUIOBJ(&win->border->status), FALSE);
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * Window Border Object
 *  
 */


/* winborder_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 */
void winborder_class(void) {
  struct WINBORDER *border = (struct WINBORDER *) eventstack.object;
  struct WIN *owner = GUIOBJ(border)->win;
  
  switch (eventstack.event) {
    // kill border?
    case DESTRUCT:
      if (owner)
        owner->border = 0;
      if (owner->icon)
        atom_delete((struct ATOM *) owner->icon);
      break;
      
    // draw border
    case EXPOSE:
      gui_winborder(border);
      return;
      
    // arm border
    case ARM: {
      const struct POINTER_INFO *info = pointer_info();
      // only move it if we click on the top part, not the sides, or the bottom
      // (remove this single if statement (not the body of) to allow movement from any point on the border)
      if (owner && ((info->y - obj_y(GUIOBJ(border))) <= textual_height(GUITEXTUAL(owner)))) {
        obj_class();
        obj_wantmove(GUIOBJ(border), obj_armed(GUIOBJ(border)));
      }
    } return;
      
    // are we moving the window?
    case POINTER_MOVE:
      // moving the window
      if (obj_armed(GUIOBJ(border))) {
        const struct POINTER_INFO *info = pointer_info();
        if (info->dx || info->dy) {
          obj_dirty(GUIOBJ(border), TRUE);
          obj_move(GUIOBJ(border), obj_x(GUIOBJ(border)) + info->dx, obj_y(GUIOBJ(border)) + info->dy);
          obj_dirty(GUIOBJ(border), TRUE);
         
          if (owner) {
            obj_dirty(GUIOBJ(owner), TRUE);
            obj_move(GUIOBJ(owner), obj_x(GUIOBJ(owner)) + info->dx, obj_y(GUIOBJ(owner)) + info->dy);
            obj_dirty(GUIOBJ(owner), TRUE);
          }
        }
      }
      break;
      
    // give a default size to the border (window)
    case DEFAULTRECT: {
      int sh = 0, th = 0;
      struct RECT *rect = defaultrect_data();
      
      th = MAX(textual_height(GUITEXTUAL(owner)), gui_h(&border->close));
      if (border->flags & (BORDER_HAS_RESIZE | BORDER_HAS_STATUS))
        sh = MAX(textual_height(&border->status), gui_h(&border->resize));
      
      if (owner) {
         const struct RECT *ownerrect = GUIRECT(owner);
         rect->left = ownerrect->left - 1;
         rect->top = ownerrect->top - th - 3;
         rect->right = ownerrect->right + 1;
         rect->bottom = ownerrect->bottom + sh + 2;
      }
      return;
    }
  }
  
  // call the object class
  obj_class();
}

/*  winborder_callback()
 *          win = pointer to the window
 *
 * this is the winborder call back that gets called after anything happens
 *  to the border (window and children of too).
 *   
 */
void winborder_callback(struct WIN *win) {
  struct WINBORDER *border = win->border;
  
  // Resize window if necessary
  if ((GUIOBJ(eventstack.object) == GUIOBJ(&border->resize)) && (eventstack.event == POINTER_MOVE) && obj_armed(GUIOBJ(&border->resize))) {
    const struct POINTER_INFO *info = pointer_info();
    if (info->dx || info->dy) {
      if (win) {
        win_dirty(win);
        obj_resize(GUIOBJ(win), MAX((gui_w(win) + info->dx), 40), MAX((gui_h(win) + info->dy), 0));
        obj_geometry(GUIOBJ(win));
        win_dirty(win);
      } else {
        obj_dirty(GUIOBJ(border), TRUE);
        obj_resize(GUIOBJ(border), MAX((gui_w(border) + info->dx), 40), MAX((gui_h(border) + info->dy), 0));
        obj_geometry(GUIOBJ(border));
        obj_dirty(GUIOBJ(border), TRUE);
      }
    }
  }
  
  // If the min button pressed, minimize the window
  if ((GUIOBJ(eventstack.object) == GUIOBJ(&border->min)) && obj_armed(GUIOBJ(&border->min))) {
    if (!(border->flags & BORDER_IS_MINIMIZED)) {
      obj_arm(GUIOBJ(&border->min), FALSE);
      int w = textual_width(GUITEXTUAL(border->base.obj.win)) + gui_w(&border->close) + 6;
      if (border->flags & BORDER_HAS_MIN)
        w += gui_w(&border->min);
      if (border->flags & BORDER_HAS_MAX)
        w += gui_w(&border->max);
      if (win) {
        win->border->org_size = *GUIRECT(GUIOBJ(win));
        win_dirty(win);
        obj_resize(GUIOBJ(win), w + 5, 1);
        obj_geometry(GUIOBJ(win));
        win_dirty(win);
      } else {
        border->org_size = *GUIRECT(GUIOBJ(border));
        obj_dirty(GUIOBJ(border), TRUE);
        obj_resize(GUIOBJ(border), w + 5, 1);
        obj_geometry(GUIOBJ(border));
        obj_dirty(GUIOBJ(border), TRUE);
      }
      border->flags |= BORDER_IS_MINIMIZED;
      border->flags &= ~BORDER_IS_MAXIMIZED;
      if (border->flags & BORDER_HAS_MAX)
        obj_disable(GUIOBJ(&border->max), FALSE);
      obj_disable(GUIOBJ(&border->min), TRUE);
    }
  }
  
  // If the max button pressed, maximize the window
  if ((GUIOBJ(eventstack.object) == GUIOBJ(&border->max)) && obj_armed(GUIOBJ(&border->max))) {
    if (!(border->flags & BORDER_IS_MAXIMIZED)) {
      obj_arm(GUIOBJ(&border->max), FALSE);
      int w, h;
      if (win) {
        w = win->border->org_size.right - win->border->org_size.left;
        h = win->border->org_size.bottom - win->border->org_size.top;
        if ((w > 0) && (h > 0)) {
          win_dirty(win);
          obj_resize(GUIOBJ(win), w, h);
          obj_geometry(GUIOBJ(win));
          win_dirty(win);
        }
      } else {
        w = border->org_size.right - border->org_size.left;
        h = border->org_size.bottom - border->org_size.top;
        if ((w > 0) && (h > 0)) {
          obj_dirty(GUIOBJ(border), TRUE);
          obj_resize(GUIOBJ(border), border->org_size.right, border->org_size.bottom);
          obj_geometry(GUIOBJ(border));
          obj_dirty(GUIOBJ(border), TRUE);
        }
      }
      border->flags |= BORDER_IS_MAXIMIZED;
      border->flags &= ~BORDER_IS_MINIMIZED;
      if (border->flags & BORDER_HAS_MIN)
        obj_disable(GUIOBJ(&border->min), FALSE);
      obj_disable(GUIOBJ(&border->max), TRUE);
    }
  }
  
  // Destroy window if close button is selected
  if (obj_selected(GUIOBJ(&border->close)))
    atom_delete(&win->base.atom);
}

/*  winborder()
 *    winborder = pointer to the border object
 *         size = size of memory to allocate
 *        owner = pointer to the window that owns this border
 *        theid = id of the object
 *        flags = flags for the border
 *
 *  creates a border object
 *   
 */
struct WINBORDER *winborder(struct WINBORDER *border, bit32u size, struct WIN *owner, const int theid, const bit32u flags) {
  MINSIZE(size, struct WINBORDER);
  
  border = (struct WINBORDER *) object(GUIOBJ(border), winborder_class, size, GUIOBJ(owner)->parent, theid);
  if (border) {
    border->callback = winborder_callback;
    GUIOBJ(border)->win = owner;
    border->flags = flags;
    
    obj_armable(GUIOBJ(border), TRUE);
    
    obj_button(&border->close, 0, GUIOBJ(border), ID_BORDER_CLOSE, 0);
    button_ownerdraw(&border->close, ID_BORDER_CLOSE, 0, 0);
    if (flags & BORDER_CLOSE_DISABLED)
      obj_disable(GUIOBJ(&border->close), TRUE);
    else
      obj_defaultrect(GUIOBJ(&border->close), 0);
    
    // TODO:
    //  Since we don't maximize a window to full screen, like other GUI's do, we could
    //   only display the MAX or the MIN, depending on what state the window was in.
    //   Also, if you have the MAX, you must have a MIN and visa-versa.
    //  Therefore, we could get rid of the BORDER_HAS_MIN and _MAX and just use
    //   BORDER_IS_MIN_CAPABLE.  Then display only the current state's "other" button.
    if (flags & BORDER_HAS_MIN) {
      obj_button(&border->min, 0, GUIOBJ(border), ID_BORDER_MIN, 0);
      button_ownerdraw(&border->min, ID_BORDER_MIN, 0, 0);
      obj_selectable(GUIOBJ(&border->min), FALSE);
      obj_defaultrect(GUIOBJ(&border->min), 0);
    }
    
    if (flags & BORDER_HAS_MAX) {
      obj_button(&border->max, 0, GUIOBJ(border), ID_BORDER_MAX, 0);
      button_ownerdraw(&border->max, ID_BORDER_MAX, 0, 0);
      obj_selectable(GUIOBJ(&border->max), FALSE);
      obj_defaultrect(GUIOBJ(&border->max), 0);
      obj_disable(GUIOBJ(&border->max), TRUE);  // windows start out maximized
    }
    
    if (flags & BORDER_HAS_RESIZE) {
      obj_button(&border->resize, 0, GUIOBJ(border), ID_BORDER_RESIZE, 0);
      button_ownerdraw(&border->resize, ID_BORDER_RESIZE, 0, 0);
      obj_selectable(GUIOBJ(&border->resize), FALSE);
      obj_wantmove(GUIOBJ(&border->resize), TRUE);
      obj_defaultrect(GUIOBJ(&border->resize), 0);
    }
    
    // create the status line object
    if (flags & BORDER_HAS_STATUS) {
      obj_textual(&border->status, sizeof(struct TEXTUAL), GUIOBJ(border), 0);
      textual_set_font(&border->status, "Arial");
      textual_set_flags(&border->status, TEXTUAL_FLAGS_READONLY | TEXTUAL_FLAGS_NOCARET);
      text_obj_color(&border->status, GUICOLOR_gray19, GUICOLOR_transparent);  // color must be after setting of flags
    }
    
    // initially zero in size
    border->org_size.left = 0;
    border->org_size.right = 0;
    border->org_size.top = 0;
    border->org_size.bottom = 0;
  }
  
  // return the border object
  return border;
}

/* root_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 *
 */
void root_class(void) {
  switch (eventstack.event) {
    // draw desktop to screen
    case EXPOSE:
      gui_root(DESKTOP);
      return;
  }
  
  // call the object class
  obj_class();
}

/* expose_rect()
 *   no parameters
 *
 *  returns the rect to expose
 *
 */
const struct RECT *expose_rect(void) {
  if (eventstack.event == EXPOSE)
    return (const struct RECT *) eventstack.data;
  
  return NULL;
}

struct EXPOSEDATA {
   struct OBJECT *target;
   bool children;
} exposedata = { NULL, FALSE };

/* is_exposing()
 *   no parameters
 *
 *  returns true if in EXPOSE event, and has target to expose
 *  i.e.: don't want to call the expose code while already exposing
 *
 */
bool is_exposing(void) {
  return ((eventstack.event == EXPOSE) && exposedata.target) ? FALSE : TRUE;
}

/* is_exposing()
 *         obj = object in question
 *        rect = area to expose
 *
 *  expose area (draw to screen buffer)
 *
 */
void expose(struct OBJECT *obj, const struct RECT *rect) {
  const struct EVENTSTACK prev = eventstack;
  
  if (!RECT_VALID(*rect))
    return;
  
  // A (little) dirty hack to make the eventstack.object a derived class of OBJECT
  //  which makes the nice macros usable even for eventstack.object.  The cast is safe
  //  becasue DERIVED is only a wrapper of OBJECT and has (must have!) no 
  //  data of its own
  eventstack.object = (struct DERIVED *) obj;
  eventstack.event = EXPOSE;
  eventstack.data = rect;
  eventstack.answer = NULL;
  
  if (exposedata.target) {
    static struct RECT dummy = { -1, -1, -2, -2 };
    
    if (exposedata.target == obj)
      gfx_dirty(rect);
    
    root.clip = dummy;
    bitmap_clip(GUISCREEN, &dummy);
    GUIOBJ(eventstack.object)->_class();
  } else {
    root.clip = *rect;
    
    if (bitmap_clip(GUISCREEN, rect))
      GUIOBJ(eventstack.object)->_class();
  }
  eventstack = prev;
}

/* drawrecurse()
 *       obj = object in question
 *    parent = parent of object
 *      rect = area to expose
 *
 *  this is were we only draw what needs to be drawn
 *  for example if only the right hand part of a window is exposed,
 *   we only draw that part rather than drawing all of the window, then
 *   drawing over it with another window.
 *
 */
void drawrecurse(struct OBJECT *obj, struct OBJECT *parent, const struct RECT *rect) {
  if (!RECT_VALID(*rect))
    return;
  
  while (obj) {
    struct RECT subrect;
    memset(&subrect, 0, sizeof(struct RECT));
    
    if (RECTS_OVERLAP(*rect, *GUIRECT(obj))) {
      RECT_INTERSECT(*rect, *GUIRECT(obj), subrect);
      
      if ((exposedata.target == obj) && exposedata.children) {
        expose(obj, &subrect);
        return;
      } else
        drawrecurse(obj->last, obj, &subrect);
      
      TOP_RECTS(*rect, *GUIRECT(obj), subrect);
      drawrecurse(obj->prev, parent, &subrect);
      
      LEFT_RECTS(*rect, *GUIRECT(obj), subrect);
      drawrecurse(obj->prev, parent, &subrect);
      
      RIGHT_RECTS(*rect, *GUIRECT(obj), subrect);
      drawrecurse(obj->prev, parent, &subrect);
      
      BOT_RECTS(*rect, *GUIRECT(obj), subrect);
      drawrecurse(obj->prev, parent, &subrect);
      return;
    }
    obj = obj->prev;
  }
  expose(parent, rect);
}

/* gui_redraw()
 *      rect = area to expose
 *
 */
void gui_redraw(const struct RECT *rect) {
  struct EXPOSEDATA old = exposedata;
  
  bool oldexposing = root.exposing;
  root.exposing = TRUE;

  exposedata.target = 0;
  exposedata.children = FALSE;
  drawrecurse(DESKTOPOBJ->last, DESKTOPOBJ, rect);
  exposedata = old;
  root.exposing = oldexposing;
}

/* obj_dirty()
 *       obj = object in question
 *  children = do children to?
 *
 *  marks area of object and if children set, all of them too
 *
 */
void obj_dirty(struct OBJECT *obj, const bool children) {
  EXPOSEDATA old = exposedata;
  
  bool oldexposing = root.exposing;
  root.exposing = TRUE;
  
  if (!obj)
    obj = GUIOBJ(eventstack.object);
  
  exposedata.target = obj;
  exposedata.children = children;
  drawrecurse(DESKTOPOBJ->last, DESKTOPOBJ, GUIRECT(obj));
  exposedata = old;
  
  root.exposing = oldexposing;
}

/* expose_background()
 *      rect = area to expose
 *
 *  exposes the background (the underneith) of where this object will
 *   be drawn.  This is incase we have transparent pixels in the object
 *   about to be drawn.  for example, a button is only drawn as the border
 *   and the text.  if we didn't expose the background the body of the
 *   button would be non-sense pixels.
 *
 */
void expose_background(const struct RECT *rect) {
  struct RECT area;
  memset(&area, 0, sizeof(struct RECT));
  
  if (eventstack.event != EXPOSE)
    return;
  
  if (!rect)
    rect = expose_rect();
  
  RECT_INTERSECT(*expose_rect(), *rect, area);
  
  if (!RECT_VALID(area))
    return;
  
  if (GUIOBJ(eventstack.object)->parent) {
    struct RECT oldclip = root.clip;
    drawrecurse(GUIOBJ(eventstack.object)->prev, GUIOBJ(eventstack.object)->parent, &area);
    bitmap_clip(GUISCREEN, &oldclip);
  }
}

/* pointer_event()
 *      event = event to send
 *
 *  Send a pointer event to the current window or root
 *
 */
void pointer_event(EVENT event) {
  // get current target, either the one we are holding, or the one we are pointed to
  struct OBJECT *target = (root.pointerhold) ? root.pointerhold : root.pointed;
  
  // if not found, don't send it an event
  if (!target)
    return;
  
  // make sure and turn off the focus show
  switch (event) {
    case LPOINTER_PRESS:
    case MPOINTER_PRESS:
    case RPOINTER_PRESS:
      gfx_show_focus(FALSE);
  }
  
  // if the event is a move, but target doesn't move, don't send the event
  if ((event == POINTER_MOVE) && !target->wantmove)
    return;
  
  // we can now send the event
  obj_event(target, event, NULL);
}

/* pointer_event()
 *        obj = current object that may have the pointer
 *
 *  return object that has the pointer
 *
 */
struct OBJECT *ppointed(const struct OBJECT *obj) {
  struct OBJECT *child;
  
  if (!RECT_CONTAINS(*GUIRECT(obj), root.pointer.x, root.pointer.y))
    return NULL;
  
  child = obj->last;
  
  while (child) {
    if (RECT_CONTAINS(*GUIRECT(child), root.pointer.x, root.pointer.y))
      return ppointed(child);
    
    child = child->prev;
  }

  // return the object
  return (struct OBJECT *) obj;
}

/* pointer_refuse()
 *        obj = current object that may want the pointer
 *
 *  return a pointer to the object that accepts it
 *
 */
struct OBJECT *pointer_refuse(struct OBJECT * obj) {
  struct OBJECT *ptr;
  
  if (!obj)
    return NULL;
  
  if (obj_event(obj, POINTER_WANT, NULL) == 0)
    return obj;
  
  ptr = obj->prev;
  while (ptr) {
    if (RECT_CONTAINS(*GUIRECT(ptr), root.pointer.x, root.pointer.y))
      return pointer_refuse(ppointed(ptr));
    
    ptr = ptr->prev;
  }
  
  return pointer_refuse(obj->parent);
}

/* gui_stop()
 *      no parameters
 *
 *  stop the gui on the next loop of the "schedule" below
 *
 */
void gui_stop(void) {
  root.running = FALSE;
}

/* gui_execute()
 *      no parameters
 *
 * This is called once we have set up the environment and are ready to start
 *  creating obects and placing them on the screen.  We only exit once we
 *  are ready to return to DOS.
 *
 */
int gui_execute(void) {
  int px, py, pz, pb;
  struct KEY_INFO key;
  bool moved;
  bool lpressed, mpressed, rpressed;
  bool lrelease, mrelease, rrelease;
  bool lheld, mheld, rheld;
  bool lhold = FALSE, mhold = FALSE, rhold = FALSE;
  struct RECT *rootrect;
  struct RECT gfxrect;
  time_t time_current;
  
  clock_t cur_tick, elapsd_tick;
  elapsd_tick = cur_tick = clock();
  
  root.running = TRUE;
  while (root.running) {
    rootrect = (struct RECT *) GUIRECT(DESKTOP);
    gfxrect = grx_args.screen;
    
    atom_lock(DESKTOP.base.atom);
    gfx_poll();
    
    if (!DESKTOPOBJ->last)
      atom_delete(DESKTOP.base.atom);
    
    if ((gfxrect.left != rootrect->left) || (gfxrect.top != rootrect->top) || (gfxrect.right != rootrect->right) || (gfxrect.bottom != rootrect->bottom)) {
      obj_place(GUIOBJ(DESKTOP), &gfxrect);
      gfx_dirty(&gfxrect);
    }

    if (gfx_pointer(&px, &py, &pz, &pb)) {
      struct OBJECT *pointed = root.pointed;
      moved = ((px != root.pointer.x) || (py != root.pointer.y))        ? TRUE : FALSE;
      lrelease = (!(pb & MOUSE_LBUT) &&  (root.pointer.b & MOUSE_LBUT)) ? TRUE : FALSE;
      mrelease = (!(pb & MOUSE_MBUT) &&  (root.pointer.b & MOUSE_MBUT)) ? TRUE : FALSE;
      rrelease = (!(pb & MOUSE_RBUT) &&  (root.pointer.b & MOUSE_RBUT)) ? TRUE : FALSE;
      lpressed = ( (pb & MOUSE_LBUT) && !(root.pointer.b & MOUSE_LBUT)) ? TRUE : FALSE;
      mpressed = ( (pb & MOUSE_MBUT) && !(root.pointer.b & MOUSE_MBUT)) ? TRUE : FALSE;
      rpressed = ( (pb & MOUSE_RBUT) && !(root.pointer.b & MOUSE_RBUT)) ? TRUE : FALSE;
      lheld    = ( (pb & MOUSE_LBUT) &&  (root.pointer.b & MOUSE_LBUT)) ? TRUE : FALSE;
      mheld    = ( (pb & MOUSE_MBUT) &&  (root.pointer.b & MOUSE_MBUT)) ? TRUE : FALSE;
      rheld    = ( (pb & MOUSE_RBUT) &&  (root.pointer.b & MOUSE_RBUT)) ? TRUE : FALSE;
      
      if (moved || lpressed || mpressed || rpressed || lrelease || mrelease || rrelease || (pz != 0)) {
        root.pointer.dx = px - root.pointer.x;
        root.pointer.dy = py - root.pointer.y;
        root.pointer.dz = pz;
        root.pointer.x = px;
        root.pointer.y = py;
        root.pointer.b = pb;
      }
      
      if (moved)
        pointer_event(POINTER_MOVE);
      
      if (pz != 0)
        pointer_event(MPOINTER_MOVE);
      
      if (!pointed || moved) {
        pointed = ppointed(DESKTOPOBJ);
        
        if (pointed && (pointed != root.pointed) && !root.pointerhold)
          pointed = pointer_refuse(pointed);
      }
      
      if ((pointed != root.pointed) && !root.pointerhold) {
        pointer_event(POINTER_LEAVE);
        
        root.pointed = pointed;
        pointer_event(POINTER_ENTER);
      }
      
      if (lpressed)
        pointer_event(LPOINTER_PRESS);
      if (mpressed)
        pointer_event(MPOINTER_PRESS);
      if (rpressed)
        pointer_event(RPOINTER_PRESS);
      
      if (lrelease) {
        pointer_event(LPOINTER_RELEASE);
        lhold = FALSE;
      }
      if (mrelease) {
        pointer_event(MPOINTER_RELEASE);
        mhold = FALSE;
      }
      if (rrelease) {
        pointer_event(RPOINTER_RELEASE);
        rhold = FALSE;
      }
      
      if (lheld) {
        if (moved) lhold = TRUE;
        if (!lhold) pointer_event(LPOINTER_HELD);
      }
      if (mheld) {
        if (moved) mhold = TRUE;
        if (!mhold) pointer_event(MPOINTER_HELD);
      }
      if (rheld) {
        if (moved) rhold = TRUE;
        if (!rhold) pointer_event(RPOINTER_HELD);
      }
    }
    
    if (gfx_key(&key.code, &key.ascii, &key.shift)) {
      //DEBUG((dfp, "Key pressed: %02X %02X %02X", key.code, key.ascii, key.shift));
      
      // watch for global exit key sequence (alt ctrl Q)
      if ((key.code == 0x1000) && KEY_ALT(key.shift) && KEY_CTRL(key.shift)) {
        root.running = FALSE;
        break;
      }
      
      struct WIN *w = cur_active_win();
      if (w) {
        const void *ret = obj_event(GUIOBJ(w), KEY, &key);
      //  if ((ret == &key) && root.focus && root.focus->win)
      //    obj_event(GUIOBJ(root.focus->win), KEY_UNUSED, &key);
      }
    }
    
    // this is a crude way of sending timed intervals to all
    //  windows.  In your actual OS, which will have more accurate
    //  timing, you will need to make this much more accurate.
    cur_tick = clock();
    if (cur_tick != elapsd_tick) {
      elapsd_tick = cur_tick;
      time_current = time(NULL);
      obj_event_recurse(DESKTOPOBJ, SEC_ELAPSED, &time_current);
    }
    
    if (atom_unlock(DESKTOP.base.atom))
      root.running = FALSE;
  }
  
  gui_exit();
  
  return -1; //EXIT_SUCCESS;
}

/* file_size()
 *      fp = file pointer of file in question
 *
 * you would think that long ago, someone what have added "get_file_size()"
 *  to the list of functions supported in the C Library...
 *
 */
bit32u file_size(FILE *fp) {
  bit32u fpos, fsz = 0;
  
  fpos = ftell(fp);
  
  fseek(fp, 0, SEEK_END);
  fsz = ftell(fp);
  
  fseek(fp, fpos, SEEK_SET);
  
  return fsz;
}

/* Parse command line.  We are looking for the following items
 *  /v         - display some information as we go
 *  /t         - truncate debug file
 *  /p         - default path to use
 */
void parse_command(int argc, char *argv[], bool *info, bool *truncate) {
  
  int i;
  const char *s;
  
  for (i=1; i<argc; i++) {
    s = argv[i];
    if ((*s == '/') || (*s == '-')) {
      s++;
      if ((strcmp(s, "V") == 0) ||
          (strcmp(s, "v") == 0))
        *info = TRUE;
      else if ((strcmp(s, "T") == 0) ||
               (strcmp(s, "t") == 0))
        *truncate = TRUE;
      else if (tolower(*s) == 'p') {
        strcpy(default_path, s + 1);
        if ((strlen(default_path) > 0) && (default_path[strlen(default_path)-1] != '\\'))
          strcat(default_path, "\\");
      } else
        printf("\n Unknown switch parameter: /%s", s);
    }
  }
}

/* prefix_default_path()
 *       target = char pointer to string to hold new filename now with default path
 *     filename = filename passed to prefix to pathname
 *
 * this will possibly prepend the filename given with the default path given
 *  on the command line (if any)
 * if the filename given starts with '\', '.', or 'd:', we do not prefix
 *  the filename with the path
 * if the pathname does not contain anything, we don't prefix it
 *
 */
void prefix_default_path(char *target, const char *filename) {
  
  if ((strlen(default_path) == 0) || // if nothing to prefix
      (filename[0] == '\\') ||       // if '\'
      (filename[0] == '.') ||        // if '.'
      (isalpha(filename[0]) && (filename[1] == ':'))) {  // if 'a:'
    strcpy(target, filename);        // just copy and be done
    return;
  }
  
  strcpy(target, default_path);
  strcat(target, filename);  
}
