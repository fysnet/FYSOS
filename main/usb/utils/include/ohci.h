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

#ifndef FYSOS__OHCI
#define FYSOS__OHCI

#pragma pack(1)

#define OHCRevision          0x00
#define OHCControl           0x04
#define OHCCommandStatus     0x08
#define OHCInterruptStatus   0x0C
#define OHCInterruptEnable   0x10
#define OHCInterruptDisable  0x14
#define OHCHCCA              0x18
#define OHCPeriodCurrentED   0x1C
#define OHCControlHeadED     0x20
#define OHCControlCurrentED  0x24
#define OHCBulkHeadED        0x28
#define OHCBulkCurrentED     0x2C
#define OHCDoneHead          0x30
#define OHCFmInterval        0x34
#define OHCFmRemaining       0x38
#define OHCFmNumber          0x3C
#define OHCPeriodicStart     0x40
#define OHCLSThreshold       0x44
#define OHCRhDescriptorA     0x48
#define OHCRhDescriptorB     0x4C
#define OHCRhStatus          0x50
#define OHCRhPortStatus      0x54

#define OHCRhDescriptorA_MASK 0xFFFFFBF0

struct OHCI_ED {
  bit32u flags;
  bit32u tailp;
  bit32u headp;
  bit32u nexted;
};

struct OHCI_TD {
  bit32u flags;
  bit32u cbp;
  bit32u nexttd;
  bit32u be;
};

struct OHCI_HCCA {
  bit32u HccaInterruptTable[32];
  bit16u HccaFrameNumber;
  bit16u HccaPad1;
  bit32u HccaDoneHead;
  bit8u  reserved[116];
  bit32u unknown;
};


#define TD_DP_SETUP  0
#define TD_DP_OUT    1
#define TD_DP_IN     2
#define TD_DP_RESV   3

// we have to place all data in a single struct so that we can pass it back
//  from local access to physcal memory.
struct OHCI_FRAME {
  struct OHCI_HCCA hcca;            // 256 bytes
  bit8u  reserved0[16];             //  16
  struct OHCI_ED   ed_table[31];    // 496
  struct OHCI_ED   control_ed[16];  // 256
  struct OHCI_ED   bulk_ed[16];     // 256
  struct OHCI_TD   our_tds[32];     // 32 * 4 * 4 
  bit8u  setup[8];                  // 8
  bit8u  packet[32];                // return packet data space
};



#endif // FYSOS__OHCI
