/*
 *                             Copyright (c) 1984-2022
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
 *  HIDPARSE.EXE
 *   This code will take a USB HID discriptor report, type 0x22, as a filename on the 
 *    command line, and parse it to display the items within it.
 *  
 *   This code is heavily based on the code found at www.usb.org under the HID
 *    developers section.
 *
 *  Assumptions/prerequisites:
 *   - Must be ran via a TRUE DOS environment, either real hardware or emulated.
 *   - Must have a pre-installed 32-bit DPMI.
 *   - Will produce unknown behavior if ran under existing operating system other
 *     than mentioned here.
 *   - Must have full access to said hardware.
 *   - This code assumes the attached device is a high-speed device.  If a full-
 *     or low-speed device is attached, the device will not be found by this code.
 *     Use GD_UHCI or GD_OHCI for full- and low-speed devices.
 *
 *  Last updated: 5 Jan 2022
 *
 *  Compiled using (DJGPP v2.05 gcc v9.3.0) (http://www.delorie.com/djgpp/)
 *   gcc -Os hidparse.c -o hidparse.exe -s
 *
 *  Usage:
 *    HIDParse filename.ext
 *
 *  Where filename.ext is a binary file of the HID Report Descriptor returned by a device.
 *  The example reports are included as:
 *    Report0.bin  -- Micro Innovations three button Scroll Mouse
 *    Report1.bin  -- Micro Innovations Keypad
 *    Report2.bin  -- Cypress three button mouse
 *    Report3.bin  -- Dell USB QWERTY Keyboard
 *   Any remaining reports are found via the internet or another form of media
 *    Report4.bin  -- "I Don't know"
 *   Text input example file
 *    Report5.txt  -- MOSART Semiconductor, 6 button mouse
 *    Report6.txt  -- Intelli-mouse wireless 3 button mouse
 *    Report7.txt  -- Joy-stick
 *  
 */

#include "..\include\ctype.h"

#include "ctype.h"
#include "conio.h"
#include "stdio.h"
#include "string.h"

#include "hidparse.h"

char filename[128];
bool is_ascii = FALSE;

bool HID_PARSE_OUTPUT = TRUE;

int main(int argc, char *argv[]) {
  
  FILE *fp;
  bit8u hid_report[4096];
  int hid_report_len;
  
  printf("\nHIDParser -- Parse a HID Report Descriptor  v1.50.00\n"
           "Forever Young Software           Copyright 1984-2022\n");
  
  // parse the command line
  if (!parse_cmnd_line(argc, argv))
    return -1;
  
  if (is_ascii) {
    fp = fopen(filename, "r");
    if (fp == NULL) {
      printf(" Error opening file: %s\n", filename);
      return -2;
    }
    
    hid_report_len = hid_convert_to_bin(hid_report, fp);
  } else {
    fp = fopen(filename, "rb");
    if (fp == NULL) {
      printf(" Error opening file: %s\n", filename);
      return -2;
    }
    
    hid_report_len = fread(hid_report, 1, 4096, fp);
  }
  
  fclose(fp);
  if (hid_report_len < 1) {
    printf(" Error reading from file.\n");
    return -3;
  }
  
  hid_parse_report(hid_report, hid_report_len);
  puts("");
  
  HID_PARSE_OUTPUT = FALSE;  // Don't print the items from here on out
  
  // as an example, we will try to get the X Coordinate offset and size
  // (if the HID report isn't a mouse report, this will print "not found")
  struct HID_PARSER parser;
  struct HID_DATA data;
  
  memset(&data, 0, sizeof(struct HID_DATA));
  data.type = ITEM_INPUT;
  data.path.node[0].u_page = USAGE_PAGE_GEN_DESKTOP;
  data.path.node[0].usage = USAGE_MOUSE;
  data.path.node[1].u_page = USAGE_PAGE_GEN_DESKTOP;
  data.path.node[1].usage = USAGE_POINTER;
  data.path.node[2].u_page = USAGE_PAGE_GEN_DESKTOP;
  data.path.node[2].usage = USAGE_POINTER_X;
  //data.path.node[2].usage = USAGE_POINTER_Y;     // to get the Y Coordinate, comment X above and uncomment this line
  //data.path.node[2].usage = USAGE_POINTER_WHEEL; // to get the Wheel Coordinate, comment X above and uncomment this line
  data.path.size = 3;
  
  parser.report_desc = (const bit8u *) hid_report;
  parser.report_desc_size = hid_report_len;
  
  puts("\nFinding Coordinate value:");
  if (find_object(&parser, &data))
    printf("    size: %i (in bits)\n"
           "  offset: %i (in bits)\n"
           "     min: %i\n"
           "     max: %i\n"
           "  attrib: 0x%02X (input, output, or feature, etc.)\n",
      data.size, data.offset, data.log_min, data.log_max, data.attribute);
  else
    puts("  Did not find Coordinate value.  Could be keyboard HID report...");
  
  return 0;
}

void hid_parse_report(const void *rep, const int len) {
  struct HID_PARSER parser;
  struct HID_DATA data;

  reset_parser(&parser);
  parser.report_desc = (const bit8u *) rep;
  parser.report_desc_size = len;

  HID_PARSE_OUTPUT = TRUE;
  while (hid_parse(&parser, &data))
    ;
}

void reset_parser(struct HID_PARSER *parser) {
  parser->pos = 0;
  parser->count = 0;
  parser->cnt_object = 0;
  parser->cnt_report = 0;
  
  parser->usage_size = 0;
  //parser->usage_min = -1;
  //parser->usage_max = -1;
  memset(parser->usage_table, 0, sizeof(struct HID_NODE) * USAGE_TAB_SIZE);
  
  memset(parser->offset_table, 0, MAX_REPORT * 3 * sizeof(int));
  memset(&parser->data, 0, sizeof(struct HID_DATA));
  
  parser->data.report_id = 1; // we must give it a non-zero value or the parser doesn't work
}

int *get_report_offset(struct HID_PARSER *parser, const bit8u report_id, const bit8u report_type) {
  
  int pos = 0;
  while ((pos < MAX_REPORT) && (parser->offset_table[pos][0] != 0)) {
    if ((parser->offset_table[pos][0] == report_id) && (parser->offset_table[pos][1] == report_type))
      return &parser->offset_table[pos][2];
    pos++;
  }
  if (pos < MAX_REPORT) {
    // Increment Report count
    parser->cnt_report++;
    parser->offset_table[pos][0] = report_id;
    parser->offset_table[pos][1] = report_type;
    parser->offset_table[pos][2] = 0;
    return &parser->offset_table[pos][2];
  }
  return NULL;
}

bit32s format_value(bit32s value, bit8u size) {
  if (size == 1) 
    value = (bit32s) (bit8s) value;
  else if (size == 2) 
    value = (bit32s) (bit16s) value;
  return value;
}

bool hid_parse(struct HID_PARSER *parser, struct HID_DATA *data) {
  bool found = FALSE;
  static unsigned space_cnt = 0;
  static bool did_collection = FALSE;
  static int item_size[4] = { 0, 1, 2, 4 };
  
  while (!found && (parser->pos < parser->report_desc_size)) {
    // Get new parser->item if current parser->count is empty 
    if (parser->count == 0) {
      if (HID_PARSE_OUTPUT) printf("\n %02X ", parser->report_desc[parser->pos]);
      parser->item = parser->report_desc[parser->pos++];
      parser->value = 0;
      memcpy(&parser->value, &parser->report_desc[parser->pos], item_size[parser->item & SIZE_MASK]);
      if (HID_PARSE_OUTPUT) {
        for (int t=0; t<4; t++) {
          if (t < item_size[parser->item & SIZE_MASK])
            printf("%02X ", parser->report_desc[parser->pos + t]);
          else
            printf("   ");
        }
      }
      
      // Pos on next item
      parser->pos += item_size[parser->item & SIZE_MASK];
    }
    
    //printf("\n (parser->item & ITEM_MASK) = 0x%04X", (parser->item & ITEM_MASK));
    switch (parser->item & ITEM_MASK) {
      case ITEM_UPAGE:
        // Copy upage in usage stack
        parser->u_page = (int) parser->value;
        if (HID_PARSE_OUTPUT) printf("%sUsage Page (%s)", spaces(space_cnt), hid_print_usage_page(parser->u_page));
        
        // copy to the usage table, but do not increment the counter incase there is a USAGE entry
        parser->usage_table[parser->usage_size].u_page = parser->u_page;
        parser->usage_table[parser->usage_size].usage = 0xFF;
        break;
        
      case ITEM_USAGE:
        // Copy global or local u_page if any, in usage stack
        if ((parser->item & SIZE_MASK) > 2)
          parser->usage_table[parser->usage_size].u_page = (int) (parser->value >> 16);
        else
          parser->usage_table[parser->usage_size].u_page = parser->u_page;
        
        // Copy Usage in Usage stack
        parser->usage_table[parser->usage_size].usage = parser->value & 0xFFFF;
        if (HID_PARSE_OUTPUT) printf("%sUsage (%s)", spaces(space_cnt), hid_print_usage_type(parser->u_page, parser->value & 0xFFFF));
        
        // Increment Usage stack size
        parser->usage_size++;
        
        break;
        
      case ITEM_USAGE_MIN:
        //parser->usage_min = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        //if (HID_PARSE_OUTPUT) printf("%sUsage min (%i)", spaces(space_cnt), parser->usage_min);
        if (HID_PARSE_OUTPUT) printf("%sUsage min (%i=%s)", spaces(space_cnt), parser->value, hid_print_usage_type(parser->u_page, parser->value));
        break;
        
      case ITEM_USAGE_MAX:
        //parser->usage_max = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        //if (HID_PARSE_OUTPUT) printf("%sUsage max (%i)", spaces(space_cnt), parser->usage_max);
        if (HID_PARSE_OUTPUT) printf("%sUsage max (%i=%s)", spaces(space_cnt), parser->value, hid_print_usage_type(parser->u_page, parser->value));
        break;
        
      case ITEM_COLLECTION:
        // Get UPage/Usage from usage_table and store them in parser->Data.Path
        parser->data.path.node[parser->data.path.size].u_page = parser->usage_table[0].u_page;
        parser->data.path.node[parser->data.path.size].usage = parser->usage_table[0].usage;
        parser->data.path.size++;
        
        // Unstack u_page/Usage from usage_table (never remove the last)
        if (parser->usage_size > 0) {
          bit8u ii=0;
          while (ii < parser->usage_size) {
            parser->usage_table[ii].usage = parser->usage_table[ii+1].usage;
            parser->usage_table[ii].u_page = parser->usage_table[ii+1].u_page;
            ii++;
          }
          // Remove Usage
          parser->usage_size--;
        }
        
        // Get Index if any
        if (parser->value >= 0x80) {
          parser->data.path.node[parser->data.path.size].u_page = 0xFF;
          parser->data.path.node[parser->data.path.size].usage = parser->value & 0x7F;
          parser->data.path.size++;
        }
        
        if (HID_PARSE_OUTPUT) {
          printf("%sCollection (%s)", spaces(space_cnt), hid_print_collection(parser->value));
          space_cnt += 2;
        }
        break;
        
      case ITEM_END_COLLECTION:
        parser->data.path.size--;
        // Remove Index if any
        if (parser->data.path.node[parser->data.path.size].u_page == 0xFF)
          parser->data.path.size--;

        if (HID_PARSE_OUTPUT) {
          if (space_cnt >= 2) space_cnt -= 2;
          printf("%sEnd Collection", spaces(space_cnt));
        }
        break;
        
      case ITEM_FEATURE:
      case ITEM_INPUT:
      case ITEM_OUTPUT:
        // An object was found
        found = TRUE;
        
        // Increment object count
        parser->cnt_object++;
        
        // Get new parser->Count from global value
        if (parser->count == 0)
          parser->count = parser->report_count;
        
        // Get u_page/Usage from usage_table and store them in parser->Data.Path
        parser->data.path.node[parser->data.path.size].u_page = parser->usage_table[0].u_page;
        parser->data.path.node[parser->data.path.size].usage = parser->usage_table[0].usage;
        parser->data.path.size++;
        
        // Unstack u_page/Usage from usage_table (never remove the last)
        if (parser->usage_size > 0) {
          bit8u ii = 0;
          while (ii < parser->usage_size) {
            parser->usage_table[ii].u_page = parser->usage_table[ii+1].u_page;
            parser->usage_table[ii].usage = parser->usage_table[ii+1].usage;
            ii++;
          }
          // Remove Usage
          parser->usage_size--;
        }
        
        // Copy data type
        parser->data.type = (bit8u) (parser->item & ITEM_MASK);
        //printf("\n parser->data.type = %i", parser->data.type);
        
        // Copy data attribute
        parser->data.attribute = (bit8u) parser->value;
        //printf("\n parser->data.attribute = %i", parser->data.attribute);
        
        // Store offset
        parser->data.offset = *get_report_offset(parser, parser->data.report_id, (bit8u) (parser->item & ITEM_MASK));
        //printf("\n parser->data.offset = %i", parser->data.offset);
        
        // Get Object in pData
        memcpy(data, &parser->data, sizeof(struct HID_DATA));
        
        // Increment Report Offset
        *get_report_offset(parser, parser->data.report_id, (bit8u) (parser->item & ITEM_MASK)) += parser->data.size;
        
        // Remove path last node
        parser->data.path.size--;
        
        // Decrement count
        if (parser->count > 0)
          parser->count--;
        
        if (!did_collection) {
          if (HID_PARSE_OUTPUT) {
            if ((parser->item & ITEM_MASK) == ITEM_FEATURE)
              printf("%sFeature ", spaces(space_cnt));
            if ((parser->item & ITEM_MASK) == ITEM_INPUT)
              printf("%sInput ", spaces(space_cnt));
            if ((parser->item & ITEM_MASK) == ITEM_OUTPUT)
              printf("%sOutput ", spaces(space_cnt));
            printf("(%s,%s,%s" /* ",%s,%s,%s,%s" */ ")",
              !(parser->value & (1<<0)) ? "Data"     : "Constant",
              !(parser->value & (1<<1)) ? "Array"    : "Variable",
              !(parser->value & (1<<2)) ? "Absolute" : "Relative"/*,
              !(parser->value & (1<<3)) ? "No Wrap"  : "Wrap",
              !(parser->value & (1<<4)) ? "Linear"   : "Non Linear",
              !(parser->value & (1<<5)) ? "Preferred State" : "No Preferred",
              !(parser->value & (1<<6)) ? "No Null"  : "Null State",
              //!(parser->value & (1<<8)) ? "Bit Fueld" : "Buffered Bytes"
              */
            );
          }
          did_collection = TRUE;
        }
        break;
        
      case ITEM_REP_ID:
        parser->data.report_id = (bit8u) parser->value;
        if (HID_PARSE_OUTPUT) printf("%sReport ID: %i", spaces(space_cnt), parser->data.report_id);
        break;
        
      case ITEM_REP_SIZE:
        parser->data.size = parser->value;
        if (HID_PARSE_OUTPUT) printf("%sReport size (%i)", spaces(space_cnt), parser->data.size);
        break;
        
      case ITEM_REP_COUNT:
        parser->report_count = parser->value;
        if (HID_PARSE_OUTPUT) printf("%sReport count (%i)", spaces(space_cnt), parser->report_count);
        did_collection = FALSE;
        break;
        
      case ITEM_UNIT_EXP:
        parser->data.unit_exp = (bit8s) parser->value;
	      // convert 4 bits signed value to 8 bits signed value
      	if (parser->data.unit_exp > 7)
          parser->data.unit_exp |= 0xF0;
        if (HID_PARSE_OUTPUT) printf("%sUnit Exp (%i)", spaces(space_cnt), parser->data.unit_exp);
        break;
        
      case ITEM_UNIT:
        parser->data.unit = parser->value;
        if (HID_PARSE_OUTPUT) printf("%sUnit (%i)", spaces(space_cnt), parser->data.unit);
        break;
        
      case ITEM_LOG_MIN:
        parser->data.log_min = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        if (HID_PARSE_OUTPUT) printf("%sLogical Min (%i)", spaces(space_cnt), parser->data.log_min);
        break;
        
      case ITEM_LOG_MAX:
        parser->data.log_max = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        if (HID_PARSE_OUTPUT) printf("%sLogical Max (%i)", spaces(space_cnt), parser->data.log_max);
        break;
        
      case ITEM_PHY_MIN:
        parser->data.phy_min = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        if (HID_PARSE_OUTPUT) printf("%sPhysical Min (%i)", spaces(space_cnt), parser->data.phy_min);
        break;
        
      case ITEM_PHY_MAX:
        parser->data.phy_max = format_value(parser->value, item_size[parser->item & SIZE_MASK]);
        if (HID_PARSE_OUTPUT) printf("%sPhysical Max (%i)", spaces(space_cnt), parser->data.phy_max);
        break;
        
      default:
        printf("\n Found unknown item 0x%02X", (parser->item & ITEM_MASK));
    }
  }
  
  return found;
}

bool find_object(struct HID_PARSER *parser, struct HID_DATA *data) {
  struct HID_DATA found_data;
  reset_parser(parser);
  while (hid_parse(parser, &found_data)) {
    if ((data->path.size > 0) && (found_data.type == data->type) &&
      memcmp(found_data.path.node, data->path.node, data->path.size * sizeof(struct HID_NODE)) == 0) {
        memcpy(data, &found_data, sizeof(struct HID_DATA));
        data->report_count = parser->report_count;
        return TRUE;
    }
    // Found by ReportID/Offset
    else if ((found_data.report_id == data->report_id) && (found_data.type == data->type) && (found_data.offset == data->offset)) {
      memcpy(data, &found_data, sizeof(struct HID_DATA));
      data->report_count = parser->report_count;
      return TRUE;
    }
  }
  return FALSE;
}

void get_value(const bit8u *buf, struct HID_DATA *data) {
  
  int bit = data->offset + 8;  // First byte of report indicate report ID
  int weight = 0;
  
  data->value = 0;
  
  while (weight < data->size) {
    int state = buf[bit >> 3] & (1 << (bit % 8));
    if (state)
      data->value += (1 << weight);
    weight++;
    bit++;
  }
  if (data->value > data->log_max)
    data->value |= ~data->log_max;
}

void set_value(const struct HID_DATA *data, bit8u* buf) {
  
  int bit = data->offset + 8; // First byte of report indicate report ID
  int weight = 0;
  
  while (weight < data->size) {
    int state = data->value & (1 << weight);
    if ((bit % 8) == 0)
      buf[bit / 8] = 0;
    
    if (state)
      buf[bit / 8] += (1 << (weight % 8));
    
    weight++;
    bit++;
  }
}

// helpers

char spaces_buff[33];
char *spaces(unsigned cnt) {
  if (cnt > 32)
    return "**";

  memset(spaces_buff, ' ', 32);
  spaces_buff[cnt] = 0;
  return spaces_buff;
}

char usage_page_str[128];

// Generic Desktop Devices
struct S_USAGE_TYPES_STR usage_type001[] = {
  { 0x0000, "Undefined" },                 
  { 0x0001, "Pointer" },                   
  { 0x0002, "Mouse" },                     
  { 0x0003, "Reserved" },                  
  { 0x0004, "Joystick" },                  
  { 0x0005, "Game Pad" },                  
  { 0x0006, "Keyboard" },                  
  { 0x0007, "Keypad" },                    
  { 0x0008, "Multi-axis Controller" },     
  { 0x0009, "Tablet PC System Controls" }, 
  { 0x0030, "X" },                         
  { 0x0031, "Y" },                         
  { 0x0032, "Z" },                         
  { 0x0033, "Rx" },                        
  { 0x0034, "Ry" },                        
  { 0x0035, "Rz" },                        
  { 0x0036, "Slider" },                    
  { 0x0037, "Dial" },                      
  { 0x0038, "Wheel" },                     
  { 0x0039, "Hat Switch" },                
  { 0x003A, "Counted Buffer" },            
  { 0x003B, "Byte Count" },                
  { 0x003C, "Motion Wake-up" },            
  { 0x003D, "Start" },                     
  { 0x003E, "Select" },                    
  { 0x003F, "Reserved" },                  
  { 0x0040, "Vx" },                        
  { 0x0041, "Vy" },                        
  { 0x0042, "Vz" },                        
  { 0x0043, "Vbrx" },                      
  { 0x0044, "Vbry" },                      
  { 0x0045, "Vbrz" },                      
  { 0x0046, "Vno" },                       
  { 0x0047, "Feature Notification" },      
  { 0x0048, "Resolution Mutliplier" },     
  { 0x0080, "System Control" },            
  { 0x0081, "System Power Down" },         
  { 0x0082, "System Sleep" },              
  { 0x0083, "System Wake-up" },            
  { 0x0084, "System Context Menu" },       
  { 0x0085, "System Main Menu" },          
  { 0x0086, "System App Menu" },           
  { 0x0087, "System Menu Help" },          
  { 0x0088, "System Menu Exit" },          
  { 0x0089, "System Menu Select" },        
  { 0x008A, "System Menu Right" },         
  { 0x008B, "System Menu Left" },          
  { 0x008C, "System Menu Up" },            
  { 0x008D, "System Menu Down" },          
  { 0x008E, "System Cold Restart " },      
  { 0x008F, "System Warm Restart" },       
  { 0x0090, "D-Pan Up" },                  
  { 0x0091, "D-Pan Down" },                
  { 0x0092, "D-Pan Right" },               
  { 0x0093, "D-Pan Left" },                
  { 0x00A0, "System Dock" },               
  { 0x00A1, "System unDock" },             
  { 0x00A2, "System Setup" },              
  { 0x00A3, "System Break" },              
  { 0x00A4, "System Debugger Break" },     
  { 0x00A5, "Application Break" },         
  { 0x00A6, "Application Debugger Break" }, 
  { 0x00A7, "System Speaker Mute" },        
  { 0x00A8, "System Hibernate" },           
  { 0x00B0, "System Display Invert" },      
  { 0x00B1, "System Display Internal" },    
  { 0x00B2, "System Display External" },    
  { 0x00B3, "System Display Both" },        
  { 0x00B4, "System Display Dual" },        
  { 0x00B5, "System Display Toggle Int/Ext" },  
  { 0x00B6, "System Display Swap Prim/Sec" },   
  { 0x00B7, "System Display Display Auto Scale" }, 
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type002[] = {
  { 0x0000, "Undefined" }, 
  { 0x0001, "Flight Simulation Device" }, 
  { 0x0002, "Automobile Simulation Device" }, 
  { 0x0003, "Tank Simulation Device" }, 
  { 0x0004, "Spaceship Simulation Device" }, 
  { 0x0005, "Submarine Simulation Device" }, 
  { 0x0006, "Sailing Simulation Device" }, 
  { 0x0007, "Motorcycle Simulation Device" }, 
  { 0x0008, "Sports Simulation Device" }, 
  { 0x0009, "Airplane Simulation Device" }, 
  { 0x000A, "Helicopter Simulation Device" }, 
  { 0x000B, "Magic Carpet Simulation Device" }, 
  { 0x000C, "Bicycle" }, 
  { 0x0020, "Flight Control Stick" }, 
  { 0x0021, "Flight Stick" }, 
  { 0x0022, "Cyclic Control" }, 
  { 0x0023, "Cyclic Trim" }, 
  { 0x0024, "Flight Yoke" }, 
  { 0x0025, "Track Control" }, 
  { 0x0026, "Driving Control" }, 
  { 0x00B0, "Aileron" }, 
  { 0x00B1, "Aileron Trim" }, 
  { 0x00B2, "Anti-Torque Control" }, 
  { 0x00B3, "Auto-pilot Enable" }, 
  { 0x00B4, "Chaff Release" }, 
  { 0x00B5, "Collective Control" }, 
  { 0x00B6, "Dive Brake" }, 
  { 0x00B7, "Electronic Counter Measures" }, 
  { 0x00B8, "Elevator" }, 
  { 0x00B9, "Elevator Trim" }, 
  { 0x00BA, "Rudder" }, 
  { 0x00BB, "Throttle" }, 
  { 0x00BC, "Flight Communication" }, 
  { 0x00BD, "Flare Release" }, 
  { 0x00BE, "Landing Gear" }, 
  { 0x00BF, "Toe Brake" }, 
  { 0x00C0, "Trigger" }, 
  { 0x00C1, "Weapons Arm" }, 
  { 0x00C2, "Weapons Select" }, 
  { 0x00C3, "Wing Flaps" }, 
  { 0x00C4, "Accelerator" }, 
  { 0x00C5, "Brake" }, 
  { 0x00C6, "Clutch" }, 
  { 0x00C7, "Shifter" }, 
  { 0x00C8, "Steering" }, 
  { 0x00C9, "Turret Direction" }, 
  { 0x00CA, "Barrel Elevation" }, 
  { 0x00CB, "Dive Plane" }, 
  { 0x00CC, "Ballast" }, 
  { 0x00CD, "Bicycle Crank" }, 
  { 0x00CE, "Handle Bars" }, 
  { 0x00CF, "Front Brake" }, 
  { 0x00D0, "Rear Brake" }, 

  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type003[] = {
  { 0x0000, "Unidentified" }, 
  { 0x0001, "Belt" }, 
  { 0x0002, "Body Suit" }, 
  { 0x0003, "Flexor" }, 
  { 0x0004, "Glove" }, 
  { 0x0005, "Head Tracker" }, 
  { 0x0006, "Head Mounted Display" }, 
  { 0x0007, "Hand Tracker" }, 
  { 0x0008, "Oculometer" }, 
  { 0x0009, "Vest" }, 
  { 0x000A, "Animatronic Device" }, 
  { 0x0020, "Stereo Enable" }, 
  { 0x0021, "Display Enable" }, 
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type004[] = {
  { 0x0000, "Unidentified" }, 
  { 0x0001, "Baseball Bat" }, 
  { 0x0002, "Golf Club" }, 
  { 0x0003, "Rowing Machine" }, 
  { 0x0004, "Treadmill" }, 
  { 0x0030, "Oar" }, 
  { 0x0031, "Slope" }, 
  { 0x0032, "Rate" }, 
  { 0x0033, "Stick Speed" }, 
  { 0x0034, "Stick Face Angle" }, 
  { 0x0035, "Stick Heel/Toe" }, 
  { 0x0036, "Stick Follow Through" }, 
  { 0x0037, "Stick Tempo" }, 
  { 0x0038, "Stick Type" }, 
  { 0x0039, "Stick Height" }, 
  { 0x0050, "Putter" }, 
  { 0x0051, "1 Iron" }, 
  { 0x0052, "2 Iron" }, 
  { 0x0053, "3 Iron" }, 
  { 0x0054, "4 Iron" }, 
  { 0x0055, "5 Iron" }, 
  { 0x0056, "6 Iron" }, 
  { 0x0057, "7 Iron" }, 
  { 0x0058, "8 Iron" }, 
  { 0x0059, "9 Iron" }, 
  { 0x005A, "10 Iron" }, 
  { 0x005B, "11 Iron" }, 
  { 0x005C, "Sand Wedge" }, 
  { 0x005D, "Loft Wedge" }, 
  { 0x005E, "Power Wedge" }, 
  { 0x005F, "1 Wood" }, 
  { 0x0060, "3 Wood" }, 
  { 0x0061, "5 Wood" }, 
  { 0x0062, "7 Wood" }, 
  { 0x0063, "9 Wood" }, 

  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type005[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "3D Game Controller" },
  { 0x0002, "Pinball Device" },
  { 0x0003, "Gun Device" },
  { 0x0020, "Point of View" },
  { 0x0021, "Turn Right/Left" },
  { 0x0022, "Pitch Right/Left" },
  { 0x0023, "Roll Forward/Backward" },
  { 0x0024, "Move Right/Left" },
  { 0x0025, "Move Forward/Backward" },
  { 0x0026, "Move Up/Down" },
  { 0x0027, "Lean Right/Left" },
  { 0x0028, "Lean Forward/Backward" },
  { 0x0029, "Height of POV" },
  { 0x002A, "Flipper" },
  { 0x002B, "Secondary Flipper" },
  { 0x002C, "Bump" },
  { 0x002D, "New Game" },
  { 0x002E, "Shoot Ball" },
  { 0x002F, "Player" },
  { 0x0030, "Gun Bolt" },
  { 0x0031, "Gun Clip" },
  { 0x0032, "Gun Selector" },
  { 0x0033, "Gun Single Shot" },
  { 0x0034, "Gun Burst" },
  { 0x0035, "Gun Automatic" },
  { 0x0036, "Gun Safety" },
  { 0x0037, "Gamepad Fire/Jump" },
  { 0x0039, "Gamepad Trigger" },

  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type006[] = {
  { 0x0000, "Unidentified" },
  { 0x0020, " Battery Strength" },
  { 0x0021, " Wireless Channel" },
  { 0x0022, " Wireless ID" },
  { 0x0023, " Discover Wireless Control" },
  { 0x0024, " Security Code Character Entered" },
  { 0x0025, " Security Code Character Erased" },
  { 0x0026, " Security Code Cleared" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type007[] = {
  { 0x0000, "Reserved (no event indicated)" },
  { 0x0001, "Keyboard ErrorRollOver" },
  { 0x0002, "Keyboard POSTFail" },
  { 0x0003, "Keyboard ErrorUndefined" },
  { 0x0004, "Keyboard a and A" },
  { 0x0005, "Keyboard b and B" },
  { 0x0006, "Keyboard c and C" },
  { 0x0007, "Keyboard d and D" },
  { 0x0008, "Keyboard e and E" },
  { 0x0009, "Keyboard f and F" },
  { 0x000A, "Keyboard g and G" },
  { 0x000B, "Keyboard h and H" },
  { 0x000C, "Keyboard i and I" },
  { 0x000D, "Keyboard j and J" },
  { 0x000E, "Keyboard k and K" },
  { 0x000F, "Keyboard l and L" },
  { 0x0010, "Keyboard m and M" },
  { 0x0011, "Keyboard n and N" },
  { 0x0012, "Keyboard o and O" },
  { 0x0013, "Keyboard p and P" },
  { 0x0014, "Keyboard q and Q" },
  { 0x0015, "Keyboard r and R" },
  { 0x0016, "Keyboard s and S" },
  { 0x0017, "Keyboard t and T" },
  { 0x0018, "Keyboard u and U" },
  { 0x0019, "Keyboard v and V" },
  { 0x001A, "Keyboard w and W" },
  { 0x001B, "Keyboard x and X" },
  { 0x001C, "Keyboard y and Y" },
  { 0x001D, "Keyboard z and Z" },
  { 0x001E, "Keyboard 1 and !" },
  { 0x001F, "Keyboard 2 and @" },
  { 0x0020, "Keyboard 3 and #" },
  { 0x0021, "Keyboard 4 and $" },
  { 0x0022, "Keyboard 5 and %" },
  { 0x0023, "Keyboard 6 and ^" },
  { 0x0024, "Keyboard 7 and &" },
  { 0x0025, "Keyboard 8 and *" },
  { 0x0026, "Keyboard 9 and (" },
  { 0x0027, "Keyboard 0 and )" },
  { 0x0028, "Keyboard Return (ENTER)" },
  { 0x0029, "Keyboard ESCAPE" },
  { 0x002A, "Keyboard DELETE (Backspace)" },
  { 0x002B, "Keyboard Tab" },
  { 0x002C, "Keyboard Spacebar" },
  { 0x002D, "Keyboard - and (underscore)" },
  { 0x002E, "Keyboard = and +" },
  { 0x002F, "Keyboard [ and {" },
  { 0x0030, "Keyboard ] and }" },
  { 0x0031, "Keyboard \\ and |" },
  { 0x0032, "Keyboard Non-US # and ~" },
  { 0x0033, "Keyboard ; and :" },
  { 0x0034, "Keyboard ' and \"" },
  { 0x0035, "Keyboard Grave Accent and Tilde" },
  { 0x0036, "Keyboard, and <" },
  { 0x0037, "Keyboard . and >" },
  { 0x0038, "Keyboard / and ?" },
  { 0x0039, "Keyboard Caps Lock" },
  { 0x003A, "Keyboard F1" },
  { 0x003B, "Keyboard F2" },
  { 0x003C, "Keyboard F3" },
  { 0x003D, "Keyboard F4" },
  { 0x003E, "Keyboard F5" },
  { 0x003F, "Keyboard F6" },
  { 0x0040, "Keyboard F7" },
  { 0x0041, "Keyboard F8" },
  { 0x0042, "Keyboard F9" },
  { 0x0043, "Keyboard F10" },
  { 0x0044, "Keyboard F11" },
  { 0x0045, "Keyboard F12" },
  { 0x0046, "Keyboard PrintScreen" },
  { 0x0047, "Keyboard Scroll Lock" },
  { 0x0048, "Keyboard Pause" },
  { 0x0049, "Keyboard Insert" },
  { 0x004A, "Keyboard Home" },
  { 0x004B, "Keyboard PageUp" },
  { 0x004C, "Keyboard Delete Forward" },
  { 0x004D, "Keyboard End" },
  { 0x004E, "Keyboard PageDown" },
  { 0x004F, "Keyboard RightArrow" },
  { 0x0050, "Keyboard LeftArrow" },
  { 0x0051, "Keyboard DownArrow" },
  { 0x0052, "Keyboard UpArrow" },
  { 0x0053, "Keypad Num Lock and Clear" },
  { 0x0054, "Keypad /" },
  { 0x0055, "Keypad *" },
  { 0x0056, "Keypad -" },
  { 0x0057, "Keypad +" },
  { 0x0058, "Keypad ENTER" },
  { 0x0059, "Keypad 1 and End" },
  { 0x005A, "Keypad 2 and Down Arrow" },
  { 0x005B, "Keypad 3 and PageDn" },
  { 0x005C, "Keypad 4 and Left Arrow" },
  { 0x005D, "Keypad 5" },
  { 0x005E, "Keypad 6 and Right Arrow" },
  { 0x005F, "Keypad 7 and Home" },
  { 0x0060, "Keypad 8 and Up Arrow" },
  { 0x0061, "Keypad 9 and PageUp" },
  { 0x0062, "Keypad 0 and Insert" },
  { 0x0063, "Keypad . and Delete" },
  { 0x0064, "Keyboard Non-US \\ and |" },
  { 0x0065, "Keyboard Application" },
  { 0x0066, "Keyboard Power" },
  { 0x0067, "Keypad =" },
  { 0x0068, "Keyboard F13" },
  { 0x0069, "Keyboard F14" },
  { 0x006A, "Keyboard F15" },
  { 0x006B, "Keyboard F16" },
  { 0x006C, "Keyboard F17" },
  { 0x006D, "Keyboard F18" },
  { 0x006E, "Keyboard F19" },
  { 0x006F, "Keyboard F20" },
  { 0x0070, "Keyboard F21" },
  { 0x0071, "Keyboard F22" },
  { 0x0072, "Keyboard F23" },
  { 0x0073, "Keyboard F24" },
  { 0x0074, "Keyboard Execute" },
  { 0x0075, "Keyboard Help" },
  { 0x0076, "Keyboard Menu" },
  { 0x0077, "Keyboard Select" },
  { 0x0078, "Keyboard Stop" },
  { 0x0079, "Keyboard Again" },
  { 0x007A, "Keyboard Undo" },
  { 0x007B, "Keyboard Cut" },
  { 0x007C, "Keyboard Copy" },
  { 0x007D, "Keyboard Paste" },
  { 0x007E, "Keyboard Find" },
  { 0x007F, "Keyboard Mute" },
  { 0x0080, "Keyboard Volume Up" },
  { 0x0081, "Keyboard Volume Down" },
  { 0x0082, "Keyboard Locking Caps Lock" },
  { 0x0083, "Keyboard Locking Num Lock" },
  { 0x0084, "Keyboard Locking Scroll Lock" },
  { 0x0085, "Keypad Comma" },
  { 0x0086, "Keypad Equal Sign" },
  { 0x0087, "Keyboard International1" },
  { 0x0088, "Keyboard International2" },
  { 0x0089, "Keyboard International3" },
  { 0x008A, "Keyboard International4" },
  { 0x008B, "Keyboard International5" },
  { 0x008C, "Keyboard International6" },
  { 0x008D, "Keyboard International7" },
  { 0x008E, "Keyboard International8" },
  { 0x008F, "Keyboard International9" },
  { 0x0090, "Keyboard LANG1" },
  { 0x0091, "Keyboard LANG2" },
  { 0x0092, "Keyboard LANG3" },
  { 0x0093, "Keyboard LANG4" },
  { 0x0094, "Keyboard LANG5" },
  { 0x0095, "Keyboard LANG6" },
  { 0x0096, "Keyboard LANG7" },
  { 0x0097, "Keyboard LANG8" },
  { 0x0098, "Keyboard LANG9" },
  { 0x0099, "Keyboard Alternate Erase" },
  { 0x009A, "Keyboard SysReq/Attention" },
  { 0x009B, "Keyboard Cancel" },
  { 0x009C, "Keyboard Clear" },
  { 0x009D, "Keyboard Prior" },
  { 0x009E, "Keyboard Return" },
  { 0x009F, "Keyboard Separator" },
  { 0x00A0, "Keyboard Out" },
  { 0x00A1, "Keyboard Oper" },
  { 0x00A2, "Keyboard Clear/Again" },
  { 0x00A3, "Keyboard CrSel/Props" },
  { 0x00A4, "Keyboard ExSel" },
  { 0x00E0, "Keyboard LeftControl" },
  { 0x00E1, "Keyboard LeftShift" },
  { 0x00E2, "Keyboard LeftAlt" },
  { 0x00E3, "Keyboard Left GUI" },
  { 0x00E4, "Keyboard RightControl" },
  { 0x00E5, "Keyboard RightShift" },
  { 0x00E6, "Keyboard RightAlt" },
  { 0x00E7, "Keyboard Right GUI" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type008[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Num Lock" },
  { 0x0002, "Caps Lock" },
  { 0x0003, "Scroll Lock" },
  { 0x0004, "Compose" },
  { 0x0005, "Kana" },
  { 0x0006, "Power" },
  { 0x0007, "Shift" },
  { 0x0008, "Do Not Disturb" },
  { 0x0009, "Mute" },
  { 0x000A, "Tone Enable" },
  { 0x000B, "High Cut Filter" },
  { 0x000C, "Low Cut Filter" },
  { 0x000D, "Equalizer Enable" },
  { 0x000E, "Sound Field On" },
  { 0x000F, "Surround Field On" },
  { 0x0010, "Repeat" },
  { 0x0011, "Stereo" },
  { 0x0012, "Sampling Rate Detect" },
  { 0x0013, "Spinning" },
  { 0x0014, "CAV" },
  { 0x0015, "CLV" },
  { 0x0016, "Recording Format Detect" },
  { 0x0017, "Off-Hook" },
  { 0x0018, "Ring" },
  { 0x0019, "Message Waiting" },
  { 0x001A, "Data Mode" },
  { 0x001B, "Battery Operation" },
  { 0x001C, "Battery OK" },
  { 0x001D, "Battery Low" },
  { 0x001E, "Speaker" },
  { 0x001F, "Head Set" },
  { 0x0020, "Hold" },
  { 0x0021, "Microphone" },
  { 0x0022, "Coverage" },
  { 0x0023, "Night Mode" },
  { 0x0024, "Send Calls" },
  { 0x0025, "Call Pickup" },
  { 0x0026, "Conference" },
  { 0x0027, "Stand-by" },
  { 0x0028, "Camera On" },
  { 0x0029, "Camera Off" },
  { 0x002A, "On-Line" },
  { 0x002B, "Off-Line" },
  { 0x002C, "Busy" },
  { 0x002D, "Ready" },
  { 0x002E, "Paper-Out" },
  { 0x002F, "Paper-Jam" },
  { 0x0030, "Remote" },
  { 0x0031, "Forward" },
  { 0x0032, "Reverse" },
  { 0x0033, "Stop" },
  { 0x0034, "Rewind" },
  { 0x0035, "Fast Forward" },
  { 0x0036, "Play" },
  { 0x0037, "Pause" },
  { 0x0038, "Record" },
  { 0x0039, "Error" },
  { 0x003A, "Usage Selected Indicator" },
  { 0x003B, "Usage In Use Indicator" },
  { 0x003C, "Usage Multi Mode Indicator" },
  { 0x003D, "Indicator On" },
  { 0x003E, "Indicator Flash" },
  { 0x003F, "Indicator Slow Blink" },
  { 0x0040, "Indicator Fast Blink" },
  { 0x0041, "Indicator Off" },
  { 0x0042, "Flash On Time" },
  { 0x0043, "Slow Blink On Time" },
  { 0x0044, "Slow Blink Off Time" },
  { 0x0045, "Fast Blink On Time" },
  { 0x0046, "Fast Blink Off Time" },
  { 0x0047, "Usage Indicator Color" },
  { 0x0048, "Red" },
  { 0x0049, "Green" },
  { 0x004A, "Amber" },
  { 0x004B, "Generic Indicator" },
  { 0x004C, "System Suspend" },
  { 0x004D, "External Power Connected" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type00B[] = {
  { 0x0000, "Unassigned" },
  { 0x0001, "Phone" },
  { 0x0002, "Answering Machine" },
  { 0x0003, "Message Controls" },
  { 0x0004, "Handset" },
  { 0x0005, "Headset" },
  { 0x0006, "Telephony Key Pad" },
  { 0x0007, "Programmable Button" },
  { 0x0020, "Hook Switch" },
  { 0x0021, "Flash" },
  { 0x0022, "Feature" },
  { 0x0023, "Hold" },
  { 0x0024, "Redial" },
  { 0x0025, "Transfer" },
  { 0x0026, "Drop" },
  { 0x0027, "Park" },
  { 0x0028, "Forward Calls" },
  { 0x0029, "Alternate Function" },
  { 0x002A, "Line" },
  { 0x002B, "Speaker Phone" },
  { 0x002C, "Conference" },
  { 0x002D, "Ring Enable" },
  { 0x002E, "Ring Select" },
  { 0x002F, "Phone Mute" },
  { 0x0030, "Caller ID" },
  { 0x0050, "Speed Dial" },
  { 0x0051, "Store Number" },
  { 0x0052, "Recall Number" },
  { 0x0053, "Phone Directory" },
  { 0x0070, "Voice Mail" },
  { 0x0071, "Screen Calls" },
  { 0x0072, "Do Not Disturb" },
  { 0x0073, "Message" },
  { 0x0074, "Answer On/Off" },
  { 0x0090, "Inside Dial Tone" },
  { 0x0091, "Outside Dial Tone" },
  { 0x0092, "Inside Ring Tone" },
  { 0x0093, "Outside Ring Tone" },
  { 0x0094, "Priority Ring Tone" },
  { 0x0095, "Inside Ringback" },
  { 0x0096, "Priority Ringback" },
  { 0x0097, "Line Busy Tone" },
  { 0x0098, "Reorder Tone" },
  { 0x0099, "Call Waiting Tone" },
  { 0x009A, "Confirmation Tone 1" },
  { 0x009B, "Confirmation Tone 2" },
  { 0x009C, "Tones Off" },
  { 0x00B0, "Phone Key 0" },
  { 0x00B1, "Phone Key 1" },
  { 0x00B2, "Phone Key 2" },
  { 0x00B3, "Phone Key 3" },
  { 0x00B4, "Phone Key 4" },
  { 0x00B5, "Phone Key 5" },
  { 0x00B6, "Phone Key 6" },
  { 0x00B7, "Phone Key 7" },
  { 0x00B8, "Phone Key 8" },
  { 0x00B9, "Phone Key 9" },
  { 0x00BA, "Phone Key Star" },
  { 0x00BB, "Phone Key Pound" },
  { 0x00BC, "Phone Key A" },
  { 0x00BD, "Phone Key B" },
  { 0x00BE, "Phone Key C" },
  { 0x00BF, "Phone Key D" },

  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type00C[] = {
  { 0x0000, "Undefined" },                
  { 0x0001, "Consumer Control" },                
  { 0x0002, "Numeric Key Pad" },                
  { 0x0003, "Programmable Buttons" },                
  { 0x0004, "Microphone" },                
  { 0x0005, "Headphone" },                
  { 0x0006, "Graphic Equalizer" },                
  { 0x0020, "+10" },                
  { 0x0021, "+100" },                
  { 0x0022, "AM/PM" },                
  { 0x0030, "Power" },                
  { 0x0031, "Reset" },                
  { 0x0032, "Sleep" },                
  { 0x0033, "Sleep After" },                
  { 0x0034, "Sleep Mode" },                
  { 0x0035, "Illumination" },                
  { 0x0036, "Function Buttons" },                
  { 0x0040, "Menu" },                
  { 0x0041, "Menu Pick" },                
  { 0x0042, "Menu Up" },                
  { 0x0043, "Menu Down" },                
  { 0x0044, "Menu Left" },                
  { 0x0045, "Menu Right" },                
  { 0x0046, "Menu Esc" },                
  { 0x0047, "Menu Value Increase" },                
  { 0x0048, "Menu Value Decrease" },                
  { 0x0060, "Data On Screen" },                
  { 0x0061, "Closed Caption" },                
  { 0x0062, "Closed Caption Select" },                
  { 0x0063, "VCR/TV" },                
  { 0x0064, "Broadcast Mode" },                
  { 0x0065, "Snap shot" },                
  { 0x0066, "Still" },                
  { 0x0080, "Selection" },                
  { 0x0081, "Assign Selection" },                
  { 0x0082, "Mode Setup" },                
  { 0x0083, "Recal Last" },                
  { 0x0084, "Enter Channel" },                
  { 0x0085, "Order Movie" },                
  { 0x0086, "Channel" },                
  { 0x0087, "Media Selection" },                
  { 0x0088, "Media Select Computer" },                
  { 0x0089, "Media Select TV" },                
  { 0x008A, "Media Select WWW" },                
  { 0x008B, "Media Select DVD" },                
  { 0x008C, "Media Select Telephone" },                
  { 0x008D, "Media Select Program Guide" },                
  { 0x008E, "Media Select Video Phone" },                
  { 0x008F, "Media Select Games" },                
  { 0x0090, "Media Select Messages" },                
  { 0x0091, "Media Select CD" },                
  { 0x0092, "Media Select VCR" },                
  { 0x0093, "Media Select Tuner" },                
  { 0x0094, "Quit" },                
  { 0x0095, "Help" },                
  { 0x0096, "Media Select Tape" },                
  { 0x0097, "Media Select Cable" },                
  { 0x0098, "Media Select Satelite" },                
  { 0x0099, "Media Select Security" },                
  { 0x009A, "Media Select Home" },                
  { 0x009B, "Media Select Call" },                
  { 0x009C, "Channel Increment" },                
  { 0x009E, "Channel Decrement" },                
  { 0x009D, "Media Select SAP" },                
  { 0x009F, "Reserved" },                
  { 0x00A0, "VCR Plus" },                
  { 0x00A1, "Once" },                
  { 0x00A2, "Daily" },                
  { 0x00A3, "Weekly" },                
  { 0x00A4, "Monthly" },                
  { 0x00B0, "Play" },                
  { 0x00B1, "Pause" },                
  { 0x00B2, "Record" },                
  { 0x00B3, "Fast Foward" },                
  { 0x00B4, "Rewind" },                
  { 0x00B5, "Scan Next Track" },                
  { 0x00B6, "Scan Previous Track" },                
  { 0x00B7, "Stop" },                
  { 0x00B8, "Eject" },                
  { 0x00B9, "Random Play" },                
  { 0x00BA, "Select Disc" },                
  { 0x00BB, "Enter Disk" },                
  { 0x00BC, "Repeat" },                
  { 0x00BD, "Tracking" },                
  { 0x00BE, "Track Normal" },                
  { 0x00BF, "Slow Tracking" },                
  { 0x00C0, "Frame Forward" },                
  { 0x00C1, "Frame Back" },                
  { 0x00C2, "Mark" },                
  { 0x00C3, "Clear Mark" },                
  { 0x00C4, "Repeat from Mark" },                
  { 0x00C5, "Return to Mark" },                
  { 0x00C6, "Search Mark Forward" },                
  { 0x00C7, "Search Mark Backwards" },                
  { 0x00C8, "Counter Reset" },                
  { 0x00C9, "Show Counter" },                
  { 0x00CA, "Tracking Increment" },                
  { 0x00CB, "Tracking Decrement" },                
  { 0x00CC, "Stop/Eject" },                
  { 0x00CD, "Play/Pause" },                
  { 0x00CE, "Play/Skip" },                
  { 0x00E0, "Volume" },                
  { 0x00E1, "Balance" },                
  { 0x00E2, "Mute" },                
  { 0x00E3, "Bass" },                
  { 0x00E4, "Treble" },                
  { 0x00E5, "Bass Boost" },                
  { 0x00E6, "Surround Mode" },                
  { 0x00E7, "Loudness" },                
  { 0x00E8, "MPX" },                
  { 0x00E9, "Volume Increment" },                
  { 0x00EA, "Volume Decrement" },                
  { 0x00F0, "Speed Select" },                
  { 0x00F1, "Playback Speed" },                
  { 0x00F2, "Standard Play" },                
  { 0x00F3, "Long Play" },                
  { 0x00F4, "Extended Play" },                
  { 0x00F5, "Slow" },                
  { 0x0100, "Fan Enable" },                
  { 0x0101, "Fan Speed" },                
  { 0x0102, "Light Enable" },                
  { 0x0103, "Light Illumination Level" },                
  { 0x0104, "Climate Control Enable" },                
  { 0x0105, "Room Temperature" },                
  { 0x0106, "Security Enable" },                
  { 0x0107, "Fire Alarm" },                
  { 0x0108, "Police Alarm" },                
  { 0x0109, "Proximity" },                
  { 0x010A, "Motion" },                
  { 0x010B, "Duress Alarm" },                
  { 0x010C, "Holdup Alarm" },                
  { 0x010D, "Medical Alarm" },                
  { 0x0150, "Balance Right" },                
  { 0x0151, "Balance Left" },                
  { 0x0152, "Bass Increment" },                
  { 0x0153, "Bass Decrement" },                
  { 0x0154, "Treble Increment" },                
  { 0x0155, "Trebel Decrement" },                
  { 0x0160, "Speaker System" },                
  { 0x0161, "Channel Left" },                
  { 0x0162, "Channel Right" },                
  { 0x0163, "Channel Center" },                
  { 0x0164, "Channel Front" },                
  { 0x0165, "Channel Center Front" },                
  { 0x0166, "Channel Side" },                
  { 0x0167, "Channel Surround" },                
  { 0x0168, "Channel Low Freq Enhancement" },                
  { 0x0169, "Channel Top" },                
  { 0x016A, "Channel Unknown" },                
  { 0x0170, "Sub-channel" },                
  { 0x0171, "Sub-channel Increment" },                
  { 0x0172, "Sub-channel Decrement" },                
  { 0x0173, "Alternate Audio Increment" },                
  { 0x0174, "Alternate Audio Decrement" },                
  { 0x0180, "Application Launch" },                
  { 0x0180, "App Launch: Config Tool" },                
  { 0x0181, "App Launch: Launch Button Config" },                
  { 0x0182, "App Launch: Programmable Button" },                
  { 0x0183, "App Launch: Consumer Control" },                
  { 0x0184, "App Launch: Word Processor" },                
  { 0x0185, "App Launch: Text Editor" },                
  { 0x0186, "App Launch: Spread Sheet" },                
  { 0x0187, "App Launch: Graphics Editor" },                
  { 0x0188, "App Launch: Presentation App" },                
  { 0x0189, "App Launch: Database App" },                
  { 0x018A, "App Launch: Email Reader" },                
  { 0x018B, "App Launch: News Reader" },                
  { 0x018C, "App Launch: Voice Mail" },                
  { 0x018D, "App Launch: Contacts/Address Book" },                
  { 0x018E, "App Launch: Calendar/Schedule Book" },                
  { 0x018F, "App Launch: Task/Project Manager" },                
  { 0x0190, "App Launch: Log/Journal/Time card" },                
  { 0x0191, "App Launch: Checkbook/Finance" },                
  { 0x0192, "App Launch: Calculator" },                
  { 0x0193, "App Launch: A/V Capture/Playback" },                
  { 0x0194, "App Launch: Local Machine Browser" },                
  { 0x0195, "App Launch: LAN/WAN Browser" },                
  { 0x0196, "App Launch: Internet Browser" },                
  { 0x0197, "App Launch: Remote Networking" },                
  { 0x0198, "App Launch: Network Conference" },                
  { 0x0199, "App Launch: Network Chat" },                
  { 0x019A, "App Launch: Telephone Dialer" },                
  { 0x019B, "App Launch: Logon" },                
  { 0x019C, "App Launch: Logoff" },                
  { 0x019D, "App Launch: Logon/Logoff" },                
  { 0x019E, "App Launch: Term Lock/Screensaver" },                
  { 0x019F, "App Launch: Control Panel" },                
  { 0x01A0, "App Launch: Command Line" },                
  { 0x01A1, "App Launch: Process Task Manager" },                
  { 0x01A2, "App Launch: Select Task/App" },                
  { 0x01A3, "App Launch: Next Task/App" },                
  { 0x01A4, "App Launch: Prev Task/App" },                
  { 0x01A5, "App Launch: Premptive Halt" },                
  { 0x01A6, "App Launch: Integrated Help Center" },                
  { 0x01A7, "App Launch: Documents" },                
  { 0x01A8, "App Launch: Thesarus" },                
  { 0x01A9, "App Launch: Dictionary" },                
  { 0x01AA, "App Launch: Desktop" },                
  { 0x01AB, "App Launch: Spell Check" },                
  { 0x01AC, "App Launch: Grammer Check" },                
  { 0x01AD, "App Launch: Wireless Status" },                
  { 0x01AE, "App Launch: Keyboard Layout" },                
  { 0x01AF, "App Launch: Virus Protect" },                
  { 0x01B0, "App Launch: Encryption" },                
  { 0x01B1, "App Launch: Screen Saver" },                
  { 0x01B2, "App Launch: Alarms" },                
  { 0x01B3, "App Launch: Clock" },                
  { 0x01B4, "App Launch: File Browser" },                
  { 0x01B5, "App Launch: Power Status" },                
  { 0x01B6, "App Launch: Image Browser" },                
  { 0x01B7, "App Launch: Audio Browser" },                
  { 0x01B8, "App Launch: Movie Browser" },                
  { 0x01B9, "App Launch: Digital Rights Manager" },                
  { 0x01BA, "App Launch: Digital Wallet" },                
  { 0x01BB, "App Launch: Reserved" },                
  { 0x01BC, "App Launch: Instant Messaging" },                
  { 0x01BD, "App Launch: OEM Tools" },                
  { 0x01BE, "App Launch: OEM Help" },                
  { 0x01BF, "App Launch: Online Community" },                
  { 0x01C0, "App Launch: Entertainment Content Browser" },                
  { 0x01C1, "App Launch: Online Shopping Browser" },                
  { 0x01C2, "App Launch: SmartCard Info" },                
  { 0x01C3, "App Launch: Market Monitor" },                
  { 0x01C4, "App Launch: Customized News" },                
  { 0x01C5, "App Launch: Online Activity Browser" },                
  { 0x01C6, "App Launch: Research/Search Browser" },                
  { 0x01C7, "App Launch: Audio Player" },                
  { 0x0200, "GUI Controls" },                
  { 0x0201, "GUI: New" },                
  { 0x0202, "GUI: Open" },                
  { 0x0203, "GUI: Close" },                
  { 0x0204, "GUI: Exit" },                
  { 0x0205, "GUI: Maximize" },                
  { 0x0206, "GUI: Minimize" },                
  { 0x0207, "GUI: Save" },                
  { 0x0208, "GUI: Print" },                
  { 0x0209, "GUI: Properties" },                
  { 0x021A, "GUI: Undo" },                
  { 0x021B, "GUI: Copy" },                
  { 0x021C, "GUI: Cut" },                
  { 0x021D, "GUI: Paste" },                
  { 0x021E, "GUI: Select All" },                
  { 0x021F, "GUI: Find" },                
  { 0x0220, "GUI: Find/Replace" },                
  { 0x0221, "GUI: Search" },                
  { 0x0222, "GUI: Goto" },                
  { 0x0223, "GUI: Home" },                
  { 0x0224, "GUI: Back" },                
  { 0x0225, "GUI: Foward" },                
  { 0x0226, "GUI: Stop" },                
  { 0x0227, "GUI: Refresh" },                
  { 0x0228, "GUI: Prev Link" },                
  { 0x0229, "GUI: Next Link" },                
  { 0x022A, "GUI: Bookmarks" },                
  { 0x022B, "GUI: History" },                
  { 0x022C, "GUI: Subscriptions" },                
  { 0x022D, "GUI: Zoom In" },                
  { 0x022E, "GUI: Zoom Out" },                
  { 0x022F, "GUI: Zoom" },                
  { 0x0230, "GUI: Full Screen" },                
  { 0x0231, "GUI: Normal View" },                
  { 0x0232, "GUI: View Toggle" },                
  { 0x0233, "GUI: Scroll Up" },                
  { 0x0234, "GUI: Scroll Down" },                
  { 0x0235, "GUI: Scroll" },                
  { 0x0236, "GUI: Pan Left" },                
  { 0x0237, "GUI: Pan Right" },                
  { 0x0238, "GUI: Pan" },                
  { 0x0239, "GUI: New Window" },                
  { 0x023A, "GUI: Tile Horz" },                
  { 0x023B, "GUI: Tile Vert" },                
  { 0x023C, "GUI: Format" },                
  { 0x023D, "GUI: Edit" },                
  { 0x023E, "GUI: Bold" },                
  { 0x023F, "GUI: Italics" },                
  { 0x0240, "GUI: Underline" },                
  { 0x0241, "GUI: StrikeThrough" },                
  { 0x0242, "GUI: SubScript" },                
  { 0x0243, "GUI: SuperScript" },                
  { 0x0244, "GUI: All Caps" },                
  { 0x0245, "GUI: Rotate" },                
  { 0x0246, "GUI: Resize" },                
  { 0x0247, "GUI: Flip Horz" },                
  { 0x0248, "GUI: Flip Vert" },                
  { 0x0249, "GUI: Mirror Horz" },                
  { 0x024A, "GUI: Mirror Vert" },                
  { 0x024B, "GUI: Font Select" },                
  { 0x024C, "GUI: Font Color" },                
  { 0x024D, "GUI: Font Size" },                
  { 0x024E, "GUI: Justify Left" },                
  { 0x024F, "GUI: Justify Center H" },                
  { 0x0250, "GUI: Justify Right" },                
  { 0x0251, "GUI: Justify Block H" },                
  { 0x0252, "GUI: Justify Top" },                
  { 0x0253, "GUI: Justify Center V" },                
  { 0x0254, "GUI: Justify Bottom" },                
  { 0x0255, "GUI: Justify Block V" },                
  { 0x0256, "GUI: Indent Decrease" },                
  { 0x0257, "GUI: Indent Increase" },                
  { 0x0258, "GUI: Numbered List" },                
  { 0x0259, "GUI: Restart Numbering" },                
  { 0x025A, "GUI: Bulleted List" },                
  { 0x025B, "GUI: Promote" },                
  { 0x025C, "GUI: Demote" },                
  { 0x025D, "GUI: Yes" },                
  { 0x025E, "GUI: No" },                
  { 0x025F, "GUI: Cancel" },                
  { 0x0260, "GUI: Catalog" },                
  { 0x0261, "GUI: Buy/Checkout" },                
  { 0x0262, "GUI: Add to Cart" },                
  { 0x0263, "GUI: Expand" },                
  { 0x0264, "GUI: Expand All" },                
  { 0x0265, "GUI: Collapse" },                
  { 0x0266, "GUI: Collapse All" },                
  { 0x0267, "GUI: Print Preview" },                
  { 0x0268, "GUI: Paste Special" },                
  { 0x0269, "GUI: Insert Mode" },                
  { 0x026A, "GUI: Delete" },                
  { 0x026B, "GUI: Lock" },                
  { 0x026C, "GUI: Unlock" },                
  { 0x026D, "GUI: Protect" },                
  { 0x026E, "GUI: Unprotect" },                
  { 0x026F, "GUI: Attache Comment" },                
  { 0x0270, "GUI: Delete Comment" },                
  { 0x0271, "GUI: View Comment" },                
  { 0x0272, "GUI: Select Word" },                
  { 0x0273, "GUI: Select Sentence" },                
  { 0x0274, "GUI: Select Paragraph" },                
  { 0x0275, "GUI: Select Column" },                
  { 0x0276, "GUI: Select Row" },                
  { 0x0277, "GUI: Select Table" },                
  { 0x0278, "GUI: Select Object" },                
  { 0x0279, "GUI: Redo/Repeat" },                
  { 0x027A, "GUI: Sort" },                
  { 0x027B, "GUI: Sort Ascending" },                
  { 0x027C, "GUI: Sort Descending" },                
  { 0x027D, "GUI: Filter" },                
  { 0x027E, "GUI: Set Clock" },                
  { 0x027F, "GUI: View Clock" },                
  { 0x0280, "GUI: Select Time Zone" },                
  { 0x0281, "GUI: Edit Time Zone" },                
  { 0x0282, "GUI: Set Alarm" },                
  { 0x0283, "GUI: Clear Alarm" },                
  { 0x0284, "GUI: Snooze Alarm" },                
  { 0x0285, "GUI: Reset Alarm" },                
  { 0x0286, "GUI: Synchronize" },                
  { 0x0287, "GUI: Send/Receive" },                
  { 0x0288, "GUI: Send To" },                
  { 0x0289, "GUI: Reply" },                
  { 0x028A, "GUI: Reply All" },                
  { 0x028B, "GUI: Forward Message" },                
  { 0x028C, "GUI: Send" },                
  { 0x028D, "GUI: Attach File" },                
  { 0x028E, "GUI: Upload" },                
  { 0x028F, "GUI: Download" },                
  { 0x0290, "GUI: Set Boards" },                
  { 0x0291, "GUI: Insert Row" },                
  { 0x0292, "GUI: Insert Column" },                
  { 0x0293, "GUI: insert File" },                
  { 0x0294, "GUI: Insert Picture" },                
  { 0x0295, "GUI: Insert Object" },                
  { 0x0296, "GUI: Insert Symbol" },                
  { 0x0297, "GUI: Save and Close" },                
  { 0x0298, "GUI: Rename" },                
  { 0x0299, "GUI: Merge" },                
  { 0x029A, "GUI: Split" },                
  { 0x029B, "GUI: Distribute Horz" },                
  { 0x029C, "GUI: Distribute Vert" },

  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type00D[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Digitizer" },
  { 0x0002, "Pen" },
  { 0x0003, "Light Pen" },
  { 0x0004, "Touch Screen" },
  { 0x0005, "Touch Pad" },
  { 0x0006, "White Board" },
  { 0x0007, "Coordinate Measuring Machine" },
  { 0x0008, "3-D Digitizer" },
  { 0x0009, "Stereo Plotter" },
  { 0x000A, "Articulated Arm" },
  { 0x000B, "Armature" },
  { 0x000C, "Multiple Point Digitizer" },
  { 0x000D, "Free Space Wand" },
  { 0x0020, "Stylus" },
  { 0x0021, "Puck" },
  { 0x0022, "Finger" },
  { 0x0030, "Tip Pressure" },
  { 0x0031, "Barrel Pressure" },
  { 0x0032, "In Range" },
  { 0x0033, "Touch" },
  { 0x0034, "Untouch" },
  { 0x0035, "Tap" },
  { 0x0036, "Quality" },
  { 0x0037, "Data Valid" },
  { 0x0038, "Transducer Index" },
  { 0x0039, "Tablet Function Keys" },
  { 0x003A, "Program Change Keys" },
  { 0x003B, "Battery Strength" },
  { 0x003C, "Invert" },
  { 0x003D, "X Tilt" },
  { 0x003E, "Y Tilt" },
  { 0x003F, "Azimuth" },
  { 0x0040, "Altitude" },
  { 0x0041, "Twist" },
  { 0x0042, "Tip Switch" },
  { 0x0043, "Secondary Tip Switch" },
  { 0x0044, "Barrel Switch" },
  { 0x0045, "Eraser" },
  { 0x0046, "Tablet Pick" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type00F[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type014[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Alphanumeric Display" },
  { 0x0020, "Display Attributes Report" },
  { 0x0021, "ASCII Character Set" },
  { 0x0022, "Data Read Back" },
  { 0x0023, "Font Read Back" },
  { 0x0024, "Display Control Report" },
  { 0x0025, "Clear Display" },
  { 0x0026, "Display Enable" },
  { 0x0027, "Screen Saver Delay" },
  { 0x0028, "Screen Saver Enable" },
  { 0x0029, "Vertical Scroll" },
  { 0x002A, "Horizontal Scroll" },
  { 0x002B, "Character Report" },
  { 0x002C, "Display Data" },
  { 0x002D, "Display Status" },
  { 0x002E, "Stat Not Ready" },
  { 0x002F, "Stat Ready" },
  { 0x0030, "Err Not a loadable character" },
  { 0x0031, "Err Font data cannot be read" },
  { 0x0032, "Cursor Position Report" },
  { 0x0033, "Row" },
  { 0x0034, "Column" },
  { 0x0035, "Rows" },
  { 0x0036, "Columns" },
  { 0x0037, "Cursor Pixel Positioning" },
  { 0x0038, "Cursor Mode" },
  { 0x0039, "Cursor Enable" },
  { 0x003A, "Cursor Blink" },
  { 0x003B, "Font Report" },
  { 0x003C, "Font Data" },
  { 0x003D, "Character Width" },
  { 0x003E, "Character Height" },
  { 0x003F, "Character Spacing Horizontal" },
  { 0x0040, "Character Spacing Vertical" },
  { 0x0041, "Unicode Character Set" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type040[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Medical Ultrasound" },
  { 0x0020, "VCR/Acquisition" },
  { 0x0021, "Freeze/Thaw" },
  { 0x0022, "Clip Store" },
  { 0x0023, "Update" },
  { 0x0024, "Next" },
  { 0x0025, "Save" },
  { 0x0026, "Print" },
  { 0x0027, "Microphone Enable" },
  { 0x0040, "Cine" },
  { 0x0041, "Transmit Power" },
  { 0x0042, "Volume" },
  { 0x0043, "Focus" },
  { 0x0044, "Depth" },
  { 0x0060, "Soft Step - Primary" },
  { 0x0061, "Soft Step - Secondary" },
  { 0x0070, "Depth Gain Compensation" },
  { 0x0080, "Zoom Select" },
  { 0x0081, "Zoom Adjust" },
  { 0x0082, "Spectral Doppler Mode Select" },
  { 0x0083, "Spectral Doppler Adjust" },
  { 0x0084, "Color Doppler Mode Select" },
  { 0x0085, "Color Doppler Adjust" },
  { 0x0086, "Motion Mode Select" },
  { 0x0087, "Motion Mode Adjust" },
  { 0x0088, "2-D Mode Select" },
  { 0x0089, "2-D Mode Adjust" },
  { 0x00A0, "Soft Control Select" },
  { 0x00A1, "Soft Control Adjust" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type080[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Monitor Control" },
  { 0x0002, "EDID Information" },
  { 0x0003, "VDIF Information" },
  { 0x0004, "VESA Version" },
  { 0x0005, "On Screen Display" },
  { 0x0006, "Auto Size Center" },
  { 0x0007, "Polarity Horz Synch" },
  { 0x0008, "Polarity Vert Synch" },
  { 0x0009, "Sync Type" },
  { 0x000A, "Screen Position" },
  { 0x000B, "Horizontal Frequency" },
  { 0x000C, "Vertical Frequency" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type082[] = {
  { 0x0010, "Brightness" },
  { 0x0012, "Contrast" },
  { 0x0016, "Video Gain Red" },
  { 0x0018, "Video Gain Green" },
  { 0x001A, "Video Gain Blue" },
  { 0x001C, "Focus" },
  { 0x0020, "Horizontal Position" },
  { 0x0022, "Horizontal Size" },
  { 0x0024, "Horizontal Pincushion" },
  { 0x0026, "Horizontal Pincushion Balance" },
  { 0x0028, "Horizontal Misconvergence" },
  { 0x002A, "Horizontal Linearity" },
  { 0x002C, "Horizontal Linearity Balance" },
  { 0x0030, "Vertical Position" },
  { 0x0032, "Vertical Size" },
  { 0x0034, "Vertical Pincushion" },
  { 0x0036, "Vertical Pincushion Balance" },
  { 0x0038, "Vertical Misconvergence" },
  { 0x003A, "Vertical Linearity" },
  { 0x003C, "Vertical Linearity Balance" },
  { 0x0040, "Parallelogram Distortion" },
  { 0x0042, "Trapezoidal Distortion" },
  { 0x0044, "Tilt" },
  { 0x0046, "Top Corner Distortion Control" },
  { 0x0048, "Top Corner Distortion Balance" },
  { 0x004A, "Bottom Corner Distortion Control" },
  { 0x004C, "Bottom Corner Distortion Balance" },
  { 0x0056, "Moir Horizontal" },
  { 0x0058, "Moir Vertical" },
  { 0x005E, "Input Level Select" },
  { 0x0060, "Input Source Select" },
  { 0x0062, "Stereo Mode" },
  { 0x006C, "Video Black Level Red" },
  { 0x006E, "Video Black Level Green" },
  { 0x0070, "Video Black Level Blue" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type083[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "Settings" },
  { 0x0002, "Degauss" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type084[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "iName" },
  { 0x0002, "PresentStatus" },
  { 0x0003, "ChangedStatus" },
  { 0x0004, "UPS" },
  { 0x0005, "PowerSupply" },
  { 0x0010, "BatterySystem" },
  { 0x0011, "BatterySystemID" },
  { 0x0012, "Battery" },
  { 0x0013, "BatteryID" },
  { 0x0014, "Charger" },
  { 0x0015, "ChargerID" },
  { 0x0016, "PowerConverter" },
  { 0x0017, "PowerConverterID" },
  { 0x0018, "OutletSystem" },
  { 0x0019, "OutletSystemID" },
  { 0x001A, "Input" },
  { 0x001B, "InputID" },
  { 0x001C, "Output" },
  { 0x001D, "OutputID" },
  { 0x001E, "Flow" },
  { 0x001F, "FlowID" },
  { 0x0020, "Outlet" },
  { 0x0021, "OutletID" },
  { 0x0022, "Gang" },
  { 0x0023, "GangID" },
  { 0x0024, "Sink" },
  { 0x0025, "SinkID" },
  { 0x0030, "Voltage" },
  { 0x0031, "Current" },
  { 0x0032, "Frequency" },
  { 0x0033, "ApparentPower" },
  { 0x0034, "ActivePower" },
  { 0x0035, "PercentLoad" },
  { 0x0036, "Temperature" },
  { 0x0037, "Humidity" },
  { 0x0040, "ConfigVoltage" },
  { 0x0041, "ConfigCurrent" },
  { 0x0042, "ConfigFrequency" },
  { 0x0043, "ConfigApparentPower" },
  { 0x0044, "ConfigActivePower" },
  { 0x0045, "ConfigPercentLoad" },
  { 0x0046, "ConfigTemperature" },
  { 0x0047, "ConfigHumidity" },
  { 0x0050, "SwitchOnControl" },
  { 0x0051, "SwitchOffControl" },
  { 0x0052, "ToggleControl" },
  { 0x0053, "LowVoltageTransfer" },
  { 0x0054, "HighVoltageTransfer" },
  { 0x0055, "DelayBeforeReboot" },
  { 0x0056, "DelayBeforeStartup" },
  { 0x0057, "DelayBeforeShutdown" },
  { 0x0058, "Test" },
  { 0x0059, "Vendorspecificcommand" },
  { 0x0060, "Present" },
  { 0x0061, "Good" },
  { 0x0062, "InternalFailure" },
  { 0x0063, "VoltageOutOfRange" },
  { 0x0064, "FrequencyOutOfRange" },
  { 0x0065, "Overload" },
  { 0x0066, "OverCharged" },
  { 0x0067, "OverTemperature" },
  { 0x0068, "ShutdownRequested" },
  { 0x0069, "ShutdownImminent" },
  { 0x006A, "VendorSpecificAnswerValid" },
  { 0x006B, "SwitchOn/Off" },
  { 0x006C, "Switcheble" },
  { 0x006D, "Used" },
  { 0x006E, "Boost" },
  { 0x006F, "Buck" },
  { 0x0070, "Initialized" },
  { 0x0071, "Tested" },
  
{ 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type085[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "SMBBatteryMode" },
  { 0x0002, "SMBBatteryStatus" },
  { 0x0003, "SMBAlarmWarning" },
  { 0x0004, "SMBChargerMode" },
  { 0x0005, "SMBChargerStatus" },
  { 0x0006, "SMBChargerSpecInfo" },
  { 0x0007, "SMBSelectorState" },
  { 0x0008, "SMBSelectorPreset" },
  { 0x0009, "SMBSelectorInfo" },
  { 0x0010, "OptionalMfgFunction1" },
  { 0x0011, "OptionalMfgFunction2" },
  { 0x0012, "OptionalMfgFunction3" },
  { 0x0013, "OptionalMfgFunction4" },
  { 0x0014, "OptionalMfgFunction5" },
  { 0x0015, "ConnectionToSMBus" },
  { 0x0016, "OutputConnection" },
  { 0x0017, "ChargerConnection" },
  { 0x0018, "BatteryInsertion" },
  { 0x0019, "Usenext" },
  { 0x001A, "OKToUse" },
  { 0x0028, "ManufacturerAccess" },
  { 0x0029, "RemainingCapacityLimit" },
  { 0x002A, "RemainingTimeLimit" },
  { 0x002B, "AtRate" },
  { 0x002C, "CapacityMode" },
  { 0x002D, "BroadcastToCharger" },
  { 0x002E, "PrimaryBattery" },
  { 0x002F, "ChargeController" },
  { 0x0040, "TerminateCharge" },
  { 0x0041, "TermminateDischarge" },
  { 0x0042, "BelowRemainingCapacityLimit" },
  { 0x0043, "RemainingTimeLimitExpired" },
  { 0x0044, "Charging" },
  { 0x0045, "Discharging" },
  { 0x0046, "FullyCharged" },
  { 0x0047, "FullyDischarged" },
  { 0x0048, "ConditionningFlag" },
  { 0x0049, "AtRateOK" },
  { 0x004A, "SMBErrorCode" },
  { 0x004B, "NeedReplacement" },
  { 0x0060, "AtRateTimeToFull" },
  { 0x0061, "AtRateTimeToEmpty" },
  { 0x0062, "AverageCurrent" },
  { 0x0063, "Maxerror" },
  { 0x0064, "RelativeStateOfCharge" },
  { 0x0065, "AbsoluteStateOfCharge" },
  { 0x0066, "RemainingCapacity" },
  { 0x0067, "FullChargeCapacity" },
  { 0x0068, "RunTimeToEmpty" },
  { 0x0069, "AverageTimeToEmpty" },
  { 0x006A, "AverageTimeToFull" },
  { 0x006B, "CycleCount" },
  { 0x0080, "BattPackModelLevel" },
  { 0x0081, "InternalChargeController" },
  { 0x0082, "PrimaryBatterySupport" },
  { 0x0083, "DesignCapacity" },
  { 0x0084, "SpecificationInfo" },
  { 0x0085, "ManufacturerDate" },
  { 0x0086, "SerialNumber" },
  { 0x0087, "iManufacturerName" },
  { 0x0088, "iDevicename" },
  { 0x0089, "iDeviceChemistery" },
  { 0x008A, "iManufacturerData" },
  { 0x008B, "Rechargeable" },
  { 0x008C, "WarningCapacityLimit" },
  { 0x008D, "CapacityGranularity1" },
  { 0x008E, "CapacityGranularity2" },
  { 0x00C0, "InhibitCharge" },
  { 0x00C1, "EnablePolling" },
  { 0x00C2, "ResetToZero" },
  { 0x00D0, "ACPresent" },
  { 0x00D1, "BatteryPresent" },
  { 0x00D2, "PowerFail" },
  { 0x00D3, "AlarmInhibited" },
  { 0x00D4, "ThermistorUnderRange" },
  { 0x00D5, "ThermistorHot" },
  { 0x00D6, "ThermistorCold" },
  { 0x00D7, "ThermistorOverRange" },
  { 0x00D8, "VoltageOutOfRange" },
  { 0x00D9, "CurrentOutOfRange" },
  { 0x00DA, "CurrentNotRegulated" },
  { 0x00DB, "VoltageNotRegulated" },
  { 0x00DC, "MasterMode" },
  { 0x00DD, "ChargerBattery/HostControlled" },
  { 0x00F0, "ChargerSpecInfo" },
  { 0x00F1, "ChargerSpecRef" },
  { 0x00F2, "Level2" },
  { 0x00F3, "Level3" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type086[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type087[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type08C[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type08D[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type08E[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type08F[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type090[] = {
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_type091[] = {
  
  { 0xFFFF, "" }
};

/*
struct S_USAGE_TYPES_STR usage_typeF1F2[] = {
  { 0x0000, "?????0000" },
  { 0x0001, "?????1111" },
  
  { 0xFFFF, "" }
};
*/

struct S_USAGE_TYPES_STR usage_typeFF00[] = {
  { 0x00E9, "Base Up" },
  { 0x00EA, "Base Down" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_typeFF84[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "iName" },
  { 0x0002, "PresentStatus" },
  { 0x0003, "ChangedStatus" },
  { 0x0004, "UPS" },
  { 0x0005, "PowerSupply" },
  { 0x0010, "BatterySystem" },
  { 0x0011, "BatterySystemID" },
  { 0x0012, "Battery" },
  { 0x0013, "BatteryID" },
  { 0x0014, "Charger" },
  { 0x0015, "ChargerID" },
  { 0x0016, "PowerConverter" },
  { 0x0017, "PowerConverterID" },
  { 0x0018, "OutletSystem" },
  { 0x0019, "OutletSystemID" },
  { 0x001A, "Input" },
  { 0x001B, "InputID" },
  { 0x001C, "Output" },
  { 0x001D, "OutputID" },
  { 0x001E, "Flow" },
  { 0x001F, "FlowID" },
  { 0x0020, "Outlet" },
  { 0x0021, "OutletID" },
  { 0x0022, "Gang" },
  { 0x0023, "GangID" },
  { 0x0024, "Sink" },
  { 0x0025, "SinkID" },
  { 0x0030, "Voltage" },
  { 0x0031, "Current" },
  { 0x0032, "Frequency" },
  { 0x0033, "ApparentPower" },
  { 0x0034, "ActivePower" },
  { 0x0035, "PercentLoad" },
  { 0x0036, "Temperature" },
  { 0x0037, "Humidity" },
  { 0x0040, "ConfigVoltage" },
  { 0x0041, "ConfigCurrent" },
  { 0x0042, "ConfigFrequency" },
  { 0x0043, "ConfigApparentPower" },
  { 0x0044, "ConfigActivePower" },
  { 0x0045, "ConfigPercentLoad" },
  { 0x0046, "ConfigTemperature" },
  { 0x0047, "ConfigHumidity" },
  { 0x0050, "SwitchOnControl" },
  { 0x0051, "SwitchOffControl" },
  { 0x0052, "ToggleControl" },
  { 0x0053, "LowVoltageTransfer" },
  { 0x0054, "HighVoltageTransfer" },
  { 0x0055, "DelayBeforeReboot" },
  { 0x0056, "DelayBeforeStartup" },
  { 0x0057, "DelayBeforeShutdown" },
  { 0x0058, "Test" },
  { 0x0059, "Vendorspecificcommand" },
  { 0x0060, "Present" },
  { 0x0061, "Good" },
  { 0x0062, "InternalFailure" },
  { 0x0063, "VoltageOutOfRange" },
  { 0x0064, "FrequencyOutOfRange" },
  { 0x0065, "Overload" },
  { 0x0066, "OverCharged" },
  { 0x0067, "OverTemperature" },
  { 0x0068, "ShutdownRequested" },
  { 0x0069, "ShutdownImminent" },
  { 0x006A, "VendorSpecificAnswerValid" },
  { 0x006B, "SwitchOn/Off" },
  { 0x006C, "Switcheble" },
  { 0x006D, "Used" },
  { 0x006E, "Boost" },
  { 0x006F, "Buck" },
  { 0x0070, "Initialized" },
  { 0x0071, "Tested" },
  
  { 0xFFFF, "" }
};

struct S_USAGE_TYPES_STR usage_typeFF85[] = {
  { 0x0000, "Undefined" },
  { 0x0001, "SMBBatteryMode" },
  { 0x0002, "SMBBatteryStatus" },
  { 0x0003, "SMBAlarmWarning" },
  { 0x0004, "SMBChargerMode" },
  { 0x0005, "SMBChargerStatus" },
  { 0x0006, "SMBChargerSpecInfo" },
  { 0x0007, "SMBSelectorState" },
  { 0x0008, "SMBSelectorPreset" },
  { 0x0009, "SMBSelectorInfo" },
  { 0x0010, "OptionalMfgFunction1" },
  { 0x0011, "OptionalMfgFunction2" },
  { 0x0012, "OptionalMfgFunction3" },
  { 0x0013, "OptionalMfgFunction4" },
  { 0x0014, "OptionalMfgFunction5" },
  { 0x0015, "ConnectionToSMBus" },
  { 0x0016, "OutputConnection" },
  { 0x0017, "ChargerConnection" },
  { 0x0018, "BatteryInsertion" },
  { 0x0019, "Usenext" },
  { 0x001A, "OKToUse" },
  { 0x0028, "ManufacturerAccess" },
  { 0x0029, "RemainingCapacityLimit" },
  { 0x002A, "RemainingTimeLimit" },
  { 0x002B, "AtRate" },
  { 0x002C, "CapacityMode" },
  { 0x002D, "BroadcastToCharger" },
  { 0x002E, "PrimaryBattery" },
  { 0x002F, "ChargeController" },
  { 0x0040, "TerminateCharge" },
  { 0x0041, "TermminateDischarge" },
  { 0x0042, "BelowRemainingCapacityLimit" },
  { 0x0043, "RemainingTimeLimitExpired" },
  { 0x0044, "Charging" },
  { 0x0045, "Discharging" },
  { 0x0046, "FullyCharged" },
  { 0x0047, "FullyDischarged" },
  { 0x0048, "ConditionningFlag" },
  { 0x0049, "AtRateOK" },
  { 0x004A, "SMBErrorCode" },
  { 0x004B, "NeedReplacement" },
  { 0x0060, "AtRateTimeToFull" },
  { 0x0061, "AtRateTimeToEmpty" },
  { 0x0062, "AverageCurrent" },
  { 0x0063, "Maxerror" },
  { 0x0064, "RelativeStateOfCharge" },
  { 0x0065, "AbsoluteStateOfCharge" },
  { 0x0066, "RemainingCapacity" },
  { 0x0067, "FullChargeCapacity" },
  { 0x0068, "RunTimeToEmpty" },
  { 0x0069, "AverageTimeToEmpty" },
  { 0x006A, "AverageTimeToFull" },
  { 0x006B, "CycleCount" },
  { 0x0080, "BattPackModelLevel" },
  { 0x0081, "InternalChargeController" },
  { 0x0082, "PrimaryBatterySupport" },
  { 0x0083, "DesignCapacity" },
  { 0x0084, "SpecificationInfo" },
  { 0x0085, "ManufacturerDate" },
  { 0x0086, "SerialNumber" },
  { 0x0087, "iManufacturerName" },
  { 0x0088, "iDevicename" },
  { 0x0089, "iDeviceChemistery" },
  { 0x008A, "iManufacturerData" },
  { 0x008B, "Rechargeable" },
  { 0x008C, "WarningCapacityLimit" },
  { 0x008D, "CapacityGranularity1" },
  { 0x008E, "CapacityGranularity2" },
  { 0x00C0, "InhibitCharge" },
  { 0x00C1, "EnablePolling" },
  { 0x00C2, "ResetToZero" },
  { 0x00D0, "ACPresent" },
  { 0x00D1, "BatteryPresent" },
  { 0x00D2, "PowerFail" },
  { 0x00D3, "AlarmInhibited" },
  { 0x00D4, "ThermistorUnderRange" },
  { 0x00D5, "ThermistorHot" },
  { 0x00D6, "ThermistorCold" },
  { 0x00D7, "ThermistorOverRange" },
  { 0x00D8, "VoltageOutOfRange" },
  { 0x00D9, "CurrentOutOfRange" },
  { 0x00DA, "CurrentNotRegulated" },
  { 0x00DB, "VoltageNotRegulated" },
  { 0x00DC, "MasterMode" },
  { 0x00DD, "ChargerBattery/HostControlled" },
  { 0x00F0, "ChargerSpecInfo" },
  { 0x00F1, "ChargerSpecRef" },
  { 0x00F2, "Level2" },
  { 0x00F3, "Level3" },
  
  { 0xFFFF, "" }
};

// Usage Pages
struct S_USAGE_PAGES_STR usage_page_strings[] = {
  { 0x0000, NULL,          "Undefined" },            
  { 0x0001, usage_type001, "Generic Desktop" },      
  { 0x0002, usage_type002, "Simulation" },           
  { 0x0003, usage_type003, "Virtual Reality" },      
  { 0x0004, usage_type004, "Sport" },                
  { 0x0005, usage_type005, "Game" },                 
  { 0x0006, usage_type006, "Generic Device" },       
  { 0x0007, usage_type007, "Keyboard/Keypad" },      
  { 0x0008, usage_type008, "LEDs" },                 
  { 0x0009, NULL,          "Button" },               
  { 0x000A, NULL,          "Ordinal" },              
  { 0x000B, usage_type00B, "Telephony" },            
  { 0x000C, usage_type00C, "Consumer" },             
  { 0x000D, usage_type00D, "Digitizer" },            
  { 0x000F, usage_type00F, "PID Page" },
  { 0x0010, NULL,          "Unicode" },              
  { 0x0014, usage_type014, "Alphanumeric Display" }, 
  { 0x0040, usage_type040, "Meidcal Insturments" },  
  { 0x0080, usage_type080, "Monitor Pages" },        
  { 0x0081, NULL,          "Monitor Pages" },        
  { 0x0082, usage_type082, "Monitor Pages" },        
  { 0x0083, usage_type083, "Monitor Pages" },        
  { 0x0084, usage_type084, "Power Pages" },          
  { 0x0085, usage_type085, "Power Pages" },          
  { 0x0086, usage_type086, "Power Pages" },          
  { 0x0087, usage_type087, "Power Pages" },          
  { 0x008C, usage_type08C, "Barcode Scanner" },      
  { 0x008D, usage_type08D, "Scale" },                
  { 0x008E, usage_type08E, "Magnetic Stripe Reading (MSR) Devices" }, 
  { 0x008F, usage_type08F, "Reserved Point of Sale" }, 
  { 0x0090, usage_type090, "Camera Control" },       
  { 0x0091, usage_type091, "Arcade" },               
  //{ 0xF1F2, usage_typeF1F2, "??????" },  // unknown:  I have a MOSART Semi Wireless Mouse that uses this Upage
  { 0xFF00, usage_typeFF00, "MS Non-Standard" },               
  { 0xFF84, usage_typeFF84, "APC Non-Standard" },               
  { 0xFF85, usage_typeFF85, "APC Non-Standard" },               

  { 0xFFFF, NULL, "" }
};

const char *hid_print_usage_page(const int page) {
  int i = 0;
  
  while ((usage_page_strings[i].value < page) && (usage_page_strings[i].value < 0xFFFF))
    i++;
  
  if ((usage_page_strings[i].value != page) || (usage_page_strings[i].value == 0xFFFF)) {
    sprintf(usage_page_str, "Unknown Usage Page: 0x%04X", page);
    return usage_page_str;
  }
  
  return usage_page_strings[i].string;
}

const char *hid_print_usage_type(const int page, const int type) {
  int i = 0;
  
  while ((usage_page_strings[i].value < page) && (usage_page_strings[i].value < 0xFFFF))
    i++;
  
  if ((usage_page_strings[i].value != page) || (usage_page_strings[i].value == 0xFFFF)) {
    sprintf(usage_page_str, "Unknown Usage Page: 0x%04X, with Type: 0x%04X", page, type);
    return usage_page_str;
  }
  
  // button press, ordinal, or UTC
  if (page == 0x0009) {
    sprintf(usage_page_str, "Button number %i", type);
    return usage_page_str;
  } else if (page == 0x000A) {
    sprintf(usage_page_str, "Ordinal %i", type);
    return usage_page_str;
  } else if (page == 0x0010) {
    sprintf(usage_page_str, "UTC 0x%04X", type);
    return usage_page_str;
  }
  
  if (usage_page_strings[i].types == NULL)
    return "Oops";
  
  int j = 0;
  struct S_USAGE_TYPES_STR *types = usage_page_strings[i].types;
  while ((types[j].value < type) && (types[j].value < 0xFFFF))
    j++;
  
  if ((types[j].value != type) || (types[j].value == 0xFFFF)) {
    sprintf(usage_page_str, "Usage Page: %s, with Unknown Type: 0x%04X", usage_page_strings[i].string, type);
    return usage_page_str;
  }
  
  return types[j].string;
}

char collection_str[][64] = {
  "Physical", "Application", "Logical", "Report", "Named Array", "Usage Switch", "Usage Modifier"
};

const char *hid_print_collection(const int val) {
  if (val <= 0x06)
    return collection_str[val];
  else if (val <= 0x7F)
    return "Reserved";
  else if (val <= 0xFF)
    return "Vendor-defined";
  else
    return "Error: val > 0xFF";
}

// read in the file as ascii bytes:
//  example:
//   05 01 09 02 A1 01 85 03-09 01 A1 00 05 09 19 01
//   ...
// and convert to binary bytes, storing in report.
// Note:
//  - all numbers are assumed hex and must be two digits wide
//  - this is simple code and does not check for the 0x prefix
//  - no checking for C style comments, etc....
int hid_convert_to_bin(bit8u *report, FILE *fp) {
  int len = 0, ch = 0;
  bit8u value = 0;
  
  while (1) {
    while (1) {
      ch = fgetc(fp);
      if (ch == EOF)
        return len;
      if (isxdigit(ch))
        break;
    }
  
    while (isxdigit(ch)) {
      ch = toupper(ch);
      value <<= 4;
      if (isdigit(ch))
        value += (ch - '0');
      else
        value += (ch - 'A' + 10);
      ch = fgetc(fp);
    }
    report[len++] = value;
  }
}

// parse the command line
// usage:
//   /a        = file is ascii bytes (optional)
//   filename  = file name of file to open
//
// example: (binary file)
//    hidparse example0.bin
// example: (ascii file)
//    hidparse example1.txt /a
bool parse_cmnd_line(int argc, char *argv[]) {
  int i = 1;
  
  while (i<argc) {
    if (!strcmp(argv[i], "/a")) {
      is_ascii = TRUE;
    } else 
      strcpy(filename, argv[i]);
    i++;
  }
  
  if (strlen(filename) == 0) {
    printf("Usage:\n"
           " D:\\HIDParse filename.bin\n"
           "  or\n"
           " D:\\HIDParse filename.txt /a\n");
    return FALSE;
  }
  
  // we got all the info we needed
  return TRUE;
}
