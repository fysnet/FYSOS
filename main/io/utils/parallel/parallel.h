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
 *             https://www.fysnet.net/osdesign_book_series.htm
 */

/*
 *  Last updated: 12 Feb 2022
 */

#ifndef FYSOS_PARALLEL
#define FYSOS_PARALLEL

// SPP/EPP/ECP compatible cards
#define  PAR_DATA         0x000
#define  PAR_STATUS       0x001
#define  PAR_CONTROL      0x002
// EPP/ECP compatible cards
#define  PAR_EPP_ADDR     0x003
#define  PAR_EPP_DATA     0x004
#define  PAR_EPP_DATA_16  0x005
#define  PAR_EPP_DATA_24  0x006
#define  PAR_EPP_DATA_32  0x007
// ECP compatible cards
#define  PAR_ECP_FIFO     0x400
#define  PAR_ECP_CONFIG_A 0x400
#define  PAR_ECP_CONFIG_B 0x401
#define  PAR_ECP_CONTROL  0x402

// ECR modes of operation
#define PAR_ECR_SPP          0   // standard SPP mode
#define PAR_ECR_BYTE         1   // standard SPP mode with bi-directional
#define PAR_ECR_PPF          2   // SPP with FIFO
#define PAR_ECR_ECP          3   // ECP with FIFO (default for ECP)
#define PAR_ECR_EPP          4   // EPP mode (optional support)
#define PAR_ECR_RSVD         5   // reserved (no mode)
#define PAR_ECR_TST          6   // FIFO Test mode
#define PAR_ECR_CNF          7   // Configuration mode
#define PAR_ECR_MODE_MASK (7<<5) // mask out mode bits

#define PAR_ECR_NO_MASK   0xFF

#define PAR_ECR_FIFO_MT   (1<<0) // FIFO Empty
#define PAR_ECR_FIFO_FL   (1<<1) // FIFO Full
#define PAR_ECR_SERVICE   (1<<2) // ECR's service bit
#define PAR_ECR_DMA       (1<<3) // Enable DMA
#define PAR_ECR_INT       (1<<3) // Enable Interrupts

#define PAR_TYPE_SPP       0x00
#define PAR_TYPE_EPP       0x01
#define PAR_TYPE_EPPECP    0x02
#define PAR_TYPE_ECP       0x03

const char *par_type[] = {
  "SPP",
  "EPP",
  "EPP/ECP",
  "ECP"
};

bool par_detect_SPP(void);
bool par_detect_SPP_PS2(void);
bool par_detect_EPP(void);
bool par_detect_ECP(void);

bool par_detect_ECPEPP(void);
bool par_detect_ECR(void);
void par_ctrl_write(const bit8u mask, const bit8u val);
void par_ecr_write(const bit8u mask, const bit8u val);
void par_set_mode(const int mode, const bool interrupts, const bool dma, const bool service);

bool par_clear_epp_timeout(void);

void mdelay(const int ms);


// irq detection:  ECP only
bit8u par_ecp_prog_irq_support(void);
bit8u par_ecp_irq_test(void);
bit8u par_ecp_get_irq(void);

// dma detection:  ECP only
bit8u par_ecp_prog_dma_support(void);

#endif // FYSOS_PARALLEL
