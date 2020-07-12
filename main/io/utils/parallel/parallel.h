/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2016
 *  
 *  This code is included on the disc that is included with the book
 *   FYSOS: Input and Output Devices, and is for that purpose only.  You have
 *   the right to use it for learning purposes only.  You may not modify it for
 *   redistribution for any other purpose unless you have written permission
 *   from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
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
