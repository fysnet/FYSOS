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

#ifndef FYSOS_VIDEOCAM
#define FYSOS_VIDEOCAM


#pragma pack(1)


#define E_NO_RDTSC -2   // no rdtsc instruction found

// we are looking for this specific camera
//  change it to the camera you have
#define VENDOR_ID   0x18EC
#define PRODUCT_ID  0x3399


#define SUCCESS                   0
#define ERROR_STALLED            -1
#define ERROR_DATA_BUFFER_ERROR  -2
#define ERROR_BABBLE_DETECTED    -3
#define ERROR_NAK                -4
#define ERROR_TIME_OUT          254
#define ERROR_UNKNOWN           255


enum {
  VS_PROBE_CONTROL = 1,
  VS_COMMIT_CONTROL = 2
};

// Video-Class Specific Request Codes
// Table A.8 (page 114)
enum {
  VID_UNUSED   = 0x00,
  VID_SET_CUR  = 0x01,
  VID_GET_CUR  = 0x81,
  VID_GET_MIN  = 0x82,
  VID_GET_MAX  = 0x83,
  VID_GET_RES  = 0x84,
  VID_GET_LEN  = 0x85,
  VID_GET_INFO = 0x86,
  VID_GET_DEF  = 0x87
};

// Processing Unit Controls
enum {
  PU_CONTROL_UNDEFINED = 0,
  PU_BACKLIGHT_COMP_CONTROL,
  PU_BRIGHTNESS_CONTROL,
  PU_CONTRAST_CONTROL,
  PU_GAIN_CONTROL,
  PU_POWER_LINE_FREQ_CONTROL,
  PU_HUE_CONTROL,
  PU_SATURATION_CONTROL,
  PU_SHARPNESS_CONTROL,
  PU_GAMMA_CONTROL,
  PU_WHITE_BALANCE_TEMP_CONTROL,
  PU_WHITE_BALANCE_TEMP_AUTO_CONTROL,
  PU_WHITE_BALANCE_COMP_CONTROL,
  PU_WHITE_BALANCE_COMP_AUTO_CONTROL,
  PU_DIGITAL_MULT_CONTROL,
  PU_DIGITAL_MULT_LIMIT_CONTROL,
  PU_HUE_AUTO_CONTROL
};

struct VS_PROBE_COMMIT_CONTROL {
  bit16u hint;
  bit8u  format_index;
  bit8u  frame_index;
  bit32u frame_interval;
  bit16u key_frame_rate;
  bit16u pframe_rate;
  bit16u comp_quality;
  bit16u comp_window_size;
  bit16u delay;
  bit32u max_video_frame_size;
  bit32u max_payload_transer_size;
};

// these descriptors are for version 1.00 of the video class specification
struct VIDEO_CONTROL_FUNC_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit16u version;
  bit16u total_len;
  bit32u clock;
  bit8u  collection_count;
  bit8u  interface_num;
};

struct VIDEO_INPUT_TERM_FUNC_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  term_id;
  bit16u term_type;
  bit8u  output_term_id;
  bit8u  term_str;
  bit16u focal_len_min;
  bit16u focal_len_max;
  bit16u ocular_focal;
  bit8u  next_field_size;
  bit8u  bitmap[16];
};

struct VIDEO_OUTPUT_TERM_FUNC_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  term_id;
  bit16u term_type;
  bit8u  output_term_id;
  bit8u  source;
  bit8u  term_str;
};

struct VIDEO_PROC_FUNC_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  unit_addr;
  bit8u  source;
  bit16u max_mult;
  bit8u  cntrl_size;
  bit16u control_bitmap;  // size is from cntrl_size above
  bit8u  unit_str;
};

// control information returned
struct PROC_UNIT_ITEM_VALUES {
  bit8u  info;
  bit16u len;
  bit16u min;
  bit16u max;
  bit16u res;
  bit16u def;
};

struct VIDEO_PROC_EPFUNC_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit16u max_size;
};

struct VIDEO_STREAM_HEADER_DESC {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  num_formats;
  bit16u total_len;
  bit8u  endpt;
  bit8u  caps_bitmap;
  bit8u  term_id;
  bit8u  still_cap_method;
  bit8u  trigger_support;
  bit8u  trigger_usage;
  bit8u  control_size;
  bit8u  control_bitmap[1];
};

struct VS_FORMAT {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  index;
  bit8u  count;
  bit8u  guid[16];
  bit8u  bits_pixel;
  bit8u  def_frame_index;
  bit8u  x_ratio;
  bit8u  y_ratio;
  bit8u  interlace_flags;
  bit8u  copy_protect;
};

struct VS_FRAME {
  bit8u  len;
  bit8u  type;
  bit8u  sub_type;
  bit8u  index;
  bit8u  caps;
  bit16u width;
  bit16u height;
  bit32u min_bit_rate;
  bit32u max_bit_rate;
  bit32u max_buffer_size;
  bit32u def_frame_interval;
  bit8u  interval_type;
  union {
    struct {
      bit32u shortest;
      bit32u longest;
      bit32u granularity;
    } continuous;
    bit32u interval[16];   // up to 16, could be more
  };
};

// This is our information structure
struct VIDCAM_INFO_S {
  // from interface association desc
  int  interface_assoc_index; // index of iad
  int  first_int_num;     // first interface number associated with this function
  int  last_int_num;      // last " " " " " 
  int  control_int;       // index of control interface
  int  stream_int;        // index of streaming interface
  bool video_cntrl;       // if not FALSE, we found the video control descriptor
  bool video_in;          // if not FALSE, we found the video contol in descriptor
  bool video_out;         // if not FALSE,  """"
  bool video_proc;        // if not FALSE,  """"
  bool video_stream_hdr;  // if not FALSE,  """"
  
  // interfaces
  int  interface_count;   // count of interfaces
  struct {
    int  int_count;
    int  ep_count;
    struct S_PAIRS {
      struct INTERFACE_DESC intr_desc;
      struct ENDPOINT_DESC endpt_desc;
    } pairs[16];   // the first, plus 15 alternatives
  } interfaces[2];
  
  // remember that this assumes the format and frames are "uncompressed" formats
  int format_cnt;         // count of payload formats
  struct {
    int frame_cnt;
    struct VS_FORMAT format;
    struct VS_FRAME frame[16];
  } formats[8];
  
  struct VIDEO_CONTROL_FUNC_DESC control_func;
  struct VIDEO_INPUT_TERM_FUNC_DESC vc_input_func;
  struct VIDEO_OUTPUT_TERM_FUNC_DESC vc_output_func;
  struct VIDEO_PROC_FUNC_DESC vc_proc_func;

  struct VIDEO_PROC_EPFUNC_DESC vc_proc_epfunc;
  
  struct VIDEO_STREAM_HEADER_DESC vs_header;

};

bool process_ehci(struct PCI_DEV *, struct PCI_POS *);
bit32u heap_alloc(const bit32u);
void heap_free(const bit32u);
void heap_init();

void ehci_init_stack_frame(const bit32u);
void ehci_init_periodic(const bit32u);
bool ehci_stop_legacy(const struct PCI_POS *, const bit32u);

bool ehci_enable_list(const bool, const int);
bool ehci_handshake(const bit32u, const bit32u, const bit32u, unsigned);
bool get_next_cntrlr(struct PCI_DEV *, struct PCI_POS *);

bool ehci_reset_port(const int);

void usb_request_packet(struct REQUEST_PACKET *packet, const bit8u, const bit8u, const bit8u, const bit8u, const bit16u, const bit16u);
bool ehci_get_descriptor(const int, struct DEVICE_DESC *, const int);
bool ehci_get_string(char *, const int, const int, const int);
bool ehci_get_config(struct CONFIG_DESC *, const int, const int);

void ehci_do_video(struct DEVICE_DESC *, const int);

void ehci_clear_phy_mem(const bit32u, const int);
void ehci_copy_to_phy_mem(const bit32u, void *, const int);
void ehci_copy_from_phy_mem(const void *, const bit32u, const int);
void ehci_write_phy_mem(const bit32u, bit32u);
bit32u ehci_read_phy_mem(const bit32u);

bool ehci_set_address(const bit8u, const bit8u);
bool ehci_control_in(const void *, struct REQUEST_PACKET *, const int, const int, const bit8u);
bool ehci_control_out(void *, struct REQUEST_PACKET *, const int, const int, const bit8u);
int  ehci_iso_in(const void *, const struct ENDPOINT_DESC *, const int, const bit8u);
void ehci_queue(bit32u, const bit32u, const bit8u, const bit16u, const bit8u);
int ehci_setup_packet(const bit32u, bit32u);
int ehci_packet(bit32u, const bit32u, bit32u, const bit32u, const bool, bit8u, const bit8u, const bit16u);
void ehci_iso_packet(const bit32u, const bit32u, const struct ENDPOINT_DESC *, const int, const bit8u, const bit8u);
void ehci_insert_queue(bit32u, const bit8u);
bool ehci_remove_queue(bit32u);
void ehci_insert_periodic(const bit32u);
int ehci_wait_interrupt(bit32u, const bit32u, bool *);
int ehci_wait_iso(bit32u, const bit32u, int *);

void ehci_write_cap_reg(const bit32u, const bit32u);
void ehci_write_op_reg(const bit32u, const bit32u);
bit32u ehci_read_cap_reg(const bit32u);
bit32u ehci_read_op_reg(const bit32u);

void get_control_info(struct PROC_UNIT_ITEM_VALUES *, const int, const int, const bit8u, const int);

#endif  // FYSOS_VIDEOCAM
