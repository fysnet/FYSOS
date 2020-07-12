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
