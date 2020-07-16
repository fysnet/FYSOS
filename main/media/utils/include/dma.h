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
 *  Last updated: 15 July 2020
 */

#ifndef FYSOS_DMA
#define FYSOS_DMA

#pragma pack(push, 1)

//////////////////////////////////////////////////////////////////////////
// DMA constants

#define  CHANNEL_FDC       0x02  // FDC

#define  DMA_COMMAND       0x08
#define  DMA_STATUS        0x08
#define  DMA_REQUEST       0x09
#define  DMA_MASK_REG      0x0A
#define  DMA_MODE_REG      0x0B
#define  DMA_FLIP_FLOP     0x0C
#define  DMA_MASTER_CLR    0x0D
#define  DMA_CLR_MASK      0x0E  // any out to it enables all 4 channels
#define  DMA_MASTER_MASK   0x0F

#define  DMA16_MASK_REG    0xD4
#define  DMA16_MODE_REG    0xD6
#define  DMA16_MASTER_CLR  0xD8
#define  DMA16_FLIP_FLOP   0xD8

#define  DMA_PAGE_CH2      0x81     // fdc
#define  DMA_PAGE_CH3      0x82     // hdc
#define  DMA_PAGE_CH1      0x83     // user

// Mode Register Bits
#define  DMA_MODE_DEMAND     0x00  // bits 7:6
#define  DMA_MODE_SINGLE     0x40
#define  DMA_MODE_BLOCK      0x80
#define  DMA_MODE_CASCADE    0xC0

#define  DMA_MODE_DECREMENT  0x20  // bit 5
#define  DMA_MODE_INCREMENT  0x00

#define  DMA_MODE_AUTO_INIT  0x10  // bit 4
#define  DMA_MODE_SINGLE_CYC 0x00

#define  DMA_MODE_VERIFY     0x00  // bits 3:2
#define  DMA_MODE_WRITE      0x04
#define  DMA_MODE_READ       0x08

#define  DMA_MODE_CHANNEL0   0x00  // bits 1:0  // channel4
#define  DMA_MODE_CHANNEL1   0x01               // channel5
#define  DMA_MODE_CHANNEL2   0x02               // channel6
#define  DMA_MODE_CHANNEL3   0x03               // channel7

void dma_init_dma(const bit8u, const bit32u, bit16u);

#pragma pack(pop)


#endif // FYSOS_DMA
