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

#ifndef FYSOS__UHCI
#define FYSOS__UHCI

#pragma pack(1)

#define UHCI_COMMAND     0x00
#define UHCI_STATUS      0x02

#define UHCI_INTERRUPT   0x04
#define UHCI_FRAME_NUM   0x06
#define UHCI_FRAME_BASE  0x08
#define UHCI_SOF_MOD     0x0C

#define UHCI_PORT_WRITE_MASK  0x124E    //  0001 0010 0100 1110

#define UHCI_HUB_RESET_TIMEOUT  50

#define TOKEN_OUT    0xE1
#define TOKEN_IN     0x69
#define TOKEN_SETUP  0x2D

#define BREADTH (0<<2)
#define DEPTH   (1<<2)

#define QUEUE_HEAD_PTR_MASK  0xFFFFFFF0
#define QUEUE_HEAD_Q         0x00000002
#define QUEUE_HEAD_T         0x00000001

struct UHCI_QUEUE_HEAD {
  bit32u   horz_ptr;
  bit32u   vert_ptr;
  bit32u   resv0[2];   // to make it 16 bytes in length
};


#define TD_PTR_MASK  0xFFFFFFF0
#define TD_VF        0x00000004
#define TD_Q         0x00000002
#define TD_T         0x00000001

#define TD_FLAGS_SPD      0x20000000
#define TD_FLAGS_CERR     0x18000000
#define TD_FLAGS_LS       0x04000000
#define TD_FLAGS_ISO      0x02000000
#define TD_FLAGS_IOC      0x01000000
#define TD_STATUS_ACTIVE  0x00800000
#define TD_STATUS_STALL   0x00400000
#define TD_STATUS_DBERR   0x00200000
#define TD_STATUS_BABBLE  0x00100000
#define TD_STATUS_NAK     0x00080000
#define TD_STATUS_CRC_TO  0x00040000
#define TD_STATUS_BSTUFF  0x00020000
#define TD_STATUS_MASK    0x00FF0000
#define TD_ACTLEN_MASK    0x000007FF

#define TD_INFO_MAXLEN_MASK   0xFFE00000
#define TD_INFO_MAXLEN_SHFT   21
#define TD_INFO_D             0x00080000
#define TD_INFO_ENDPT_MASK    0x00078000
#define TD_INFO_ENDPT_SHFT    15
#define TD_INFO_ADDR_MASK     0x00007F00
#define TD_INFO_ADDR_SHFT     8
#define TD_INFO_PID           0x000000FF

struct UHCI_TRANSFER_DESCRIPTOR { 
  bit32u   link_ptr;
  bit32u   reply;
  bit32u   info;
  bit32u   buff_ptr;
  bit32u   resv0[4];          // the last 4 dwords are reserved for software use.
};



#endif // FYSOS__UHCI
