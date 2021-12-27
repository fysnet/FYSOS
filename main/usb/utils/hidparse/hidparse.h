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
 *  Last updated: 26 Dec 2021
 */


int hid_convert_to_bin(bit8u *report, FILE *fp);
bool parse_cmnd_line(int argc, char *argv[]);

enum {
  USAGE_PAGE_GEN_DESKTOP = 1, USAGE_PAGE_BUTTON = 9,
};

enum {
  USAGE_POINTER = 1, USAGE_MOUSE,
  USAGE_POINTER_X = 0x30, USAGE_POINTER_Y, USAGE_POINTER_WHEEL = 0x38,
  
  USAGE_NOTHING = 0xFF
};

#define BUTTON_LEFT    0
#define BUTTON_RIGHT   1
#define BUTTON_MIDDLE  2


#define PATH_SIZE               10 // maximum depth for Path
#define USAGE_TAB_SIZE          50 // Size of usage stack
#define MAX_REPORT             300 // Including FEATURE, INPUT and OUTPUT

#define SIZE_0                0x00
#define SIZE_1                0x01
#define SIZE_2                0x02
#define SIZE_4                0x03
#define SIZE_MASK             0x03
                           
#define TYPE_MAIN             0x00
#define TYPE_GLOBAL           0x04
#define TYPE_LOCAL            0x08
#define TYPE_MASK             0xC0

#define ITEM_MASK             0xFC
#define ITEM_UPAGE            0x04
#define ITEM_USAGE            0x08  // local item
#define ITEM_LOG_MIN          0x14
#define ITEM_USAGE_MIN        0x18  // local item
#define ITEM_LOG_MAX          0x24
#define ITEM_USAGE_MAX        0x28  // local item
#define ITEM_PHY_MIN          0x34
#define ITEM_PHY_MAX          0x44
#define ITEM_UNIT_EXP         0x54
#define ITEM_UNIT             0x64
#define ITEM_REP_SIZE         0x74
#define ITEM_STRING           0x78  // local item?
#define ITEM_REP_ID           0x84
#define ITEM_REP_COUNT        0x94

#define ITEM_COLLECTION       0xA0
#define ITEM_END_COLLECTION   0xC0
#define ITEM_FEATURE          0xB0
#define ITEM_INPUT            0x80
#define ITEM_OUTPUT           0x90

// Attribute Flags
#define ATTR_DATA_CST         0x01
#define ATTR_NVOL_VOL         0x80

// Describe a HID Path point 
struct HID_NODE {
  int u_page;
  int usage;
};

// Describe a HID Path
struct HID_PATH {
  int    size;                      // HID Path size
  struct HID_NODE node[PATH_SIZE];  // HID Path
};

// Describe a HID Data with its location in report 
struct HID_DATA {
  int    value;            // HID Object Value
  struct HID_PATH path;    // HID Path
  
  bit8u  report_id;        // Report ID, (from incoming report) ???
  int    report_count;     // count of reports for this usage type
  int    offset;           // Offset of data in report
  int    size;             // Size of data in bits
                            
  bit8u  type;             // Type : FEATURE / INPUT / OUTPUT
  bit8u  attribute;        // Report field attribute (2 = (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position))
                           //                        (6 = (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position))
  bit32u unit;             // HID Unit
  bit8s  unit_exp;         // Unit exponent
  
  int    log_min;          // Logical Min
  int    log_max;          // Logical Max
  int    phy_min;          // Physical Min
  int    phy_max;          // Physical Max
};

struct HID_PARSER {
  const bit8u *report_desc;            // Store Report Descriptor
  int    report_desc_size;             // Size of Report Descriptor
  int    pos;                          // Store current pos in descriptor
  bit8u  item;                         // Store current Item
  int    value;                        // Store current Value
  
  struct HID_DATA data;                // Store current environment
  
  int    offset_table[MAX_REPORT][3];  // Store ID, type & offset of report
  int    report_count;                 // Store Report Count
  int    count;                        // Store local report count
  
  int    u_page;                       // Global UPage
  struct HID_NODE usage_table[USAGE_TAB_SIZE]; // Usage stack
  int    usage_size;                   // Design number of usage used
  int    usage_min;
  int    usage_max;
  
  int    cnt_object;                   // Count objects in Report Descriptor
  int    cnt_report;                   // Count reports in Report Descriptor
};


void hid_parse_report(const void *, const int);
bool hid_parse(struct HID_PARSER *, struct HID_DATA *);
void reset_parser(struct HID_PARSER *);
bool find_object(struct HID_PARSER *, struct HID_DATA *);

// helpers
struct S_USAGE_TYPES_STR {
  int value;
  const char *string;
};

struct S_USAGE_PAGES_STR {
  int value;
  struct S_USAGE_TYPES_STR *types;
  const char *string;
};

char *spaces(unsigned);
const char *hid_print_usage_page(const int);
const char *hid_print_usage_type(const int, const int);
const char *hid_print_collection(int);

