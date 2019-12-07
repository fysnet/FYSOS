comment |*******************************************************************
*  Copyright (c) 1984-2019    Forever Young Software  Benjamin David Lunt  *
*                                                                          *
*                            FYS OS version 2.0                            *
* FILE: fdc_boot.inc                                                       *
*                                                                          *
* This code is freeware, not public domain.  Please use respectfully.      *
*                                                                          *
* You may:                                                                 *
*  - use this code for learning purposes only.                             *
*  - use this code in your own Operating System development.               *
*  - distribute any code that you produce pertaining to this code          *
*    as long as it is for learning purposes only, not for profit,          *
*    and you give credit where credit is due.                              *
*                                                                          *
* You may NOT:                                                             *
*  - distribute this code for any purpose other than listed above.         *
*  - distribute this code for profit.                                      *
*                                                                          *
* You MUST:                                                                *
*  - include this whole comment block at the top of this file.             *
*  - include contact information to where the original source is located.  *
*            https://github.com/fysnet/FYSOS                               *
*                                                                          *
***************************************************************************|

QEMU_EMULATOR       equ  0

STACK_OFFSET        equ  4000h

; assumptions
DRIVE               equ  0      ; we assume the first drive (this can be 0, 1, 2, or 3)
CNTRLR              equ  0x3F0  ; we assume the first controller (this can be 0x3F0 or 0x370)
IRQ_NUM             equ  0x0E   ; Interrupt Vector Table index for the floppy disk

; FDC constants
FDC_SRA             equ  (CNTRLR + 0x000)      ; FDC Diskette Status Register A at 3?0h (PS/2)
FDC_SRB             equ  (CNTRLR + 0x001)      ; FDC Diskette Status Register B at 3?1h (PS/2)
FDC_DOR             equ  (CNTRLR + 0x002)      ; FDC Digital Output Register at 3?2h  (all systems)
FDC_MSR             equ  (CNTRLR + 0x004)      ; FDC Main Status Register at 3?4h  (all systems)
FDC_CSR             equ  (CNTRLR + 0x005)      ; FDC Command Status Register 0 at 3?5h  (all systems)
FDC_DIR             equ  (CNTRLR + 0x007)      ; FDC Digital Input Register at 3?7h (PS/2)
  FDC_DIR_CHNG_LINE    equ  0x80
FDC_CCR             equ  (CNTRLR + 0x007)      ; FDC Configuration Control Register at 3?7h (PS/2)

; Standard FDC driver services
FDC_CMD_RESET       equ  0x01  ; Reset (Intel 8271)
FDC_CMD_MODE        equ  0x01  ; National Semiconductor DP8473
FDC_CMD_READ_TRK    equ  0x02
FDC_CMD_SPECIFY     equ  0x03
FDC_CMD_STATUS      equ  0x04
FDC_CMD_WRITE       equ  0x05
FDC_CMD_READ        equ  0x06
FDC_CMD_RECAL       equ  0x07
FDC_CMD_SENSE_INT   equ  0x08
FDC_CMD_WRITE_DEL   equ  0x09
FDC_CMD_READ_ID     equ  0x0A
FDC_CMD_MOTOR_ON    equ  0x0B  ; Intel 82072
FDC_CMD_READ_DEL    equ  0x0C
FDC_CMD_FORMAT      equ  0x0D
FDC_CMD_DUMP_REGS   equ  0x0E
FDC_CMD_SEEK        equ  0x0F
FDC_CMD_VERSION     equ  0x10
FDC_CMD_SCAN_EQ     equ  0x11
FDC_CMD_PERP288     equ  0x12
FDC_CMD_CONFIGURE   equ  0x13
FDC_CMD_UNLOCK      equ  0x14  ; bit 7 + 0x14 (bit 7 = 1 = lock)
FDC_CMD_VERIFY      equ  0x16
FDC_CMD_POWERDOWN   equ  0x17
FDC_CMD_PARTID      equ  0x18
FDC_CMD_SCAN_LEQ    equ  0x19
FDC_CMD_SCAN_HEQ    equ  0x1D
FDC_CMD_SET_TRK     equ  0x21  ; National Semiconductor DP8473
.if QEMU_EMULATOR
  FDC_CMD_SAVE        equ  0x2C
.else
  FDC_CMD_SAVE        equ  0x2E
.endif
FDC_CMD_OPTION      equ  0x33
FDC_CMD_EXIT_STND   equ  0x34  ; uPD72065A/66 (among others)
FDC_CMD_STNDBY      equ  0x35  ; uPD72065A/66 (among others)
FDC_CMD_H_RESET     equ  0x36  ; uPD72065A/66 (among others)
.if QEMU_EMULATOR
  FDC_CMD_RESTORE     equ  0x4C
.else
  FDC_CMD_RESTORE     equ  0x4E
.endif
FDC_CMD_DRV_SPEC    equ  0x8E  ; Intel 82078
FDC_CMD_REL_SEEK    equ  0x8F
FDC_CMD_FOR_WRITE   equ  0xAD

FDC_MT              equ  (1<<7)
FDC_MFM             equ  (1<<6)
FDC_SKIP            equ  (1<<5)

FDC_CMD_VERIFY_SK   equ  (FDC_CMD_READ | FDC_SKIP)


FDC_IMPLIED_SEEK    equ  0x40
FDC_DISABLE_FIFO    equ  0x20
FDC_DISABLE_POLL    equ  0x10

; code sent to FDC_CCR register
FDC_500KBPS         equ  0x00
FDC_300KBPS         equ  0x01
FDC_250KBPS         equ  0x02
FDC_1000KBPS        equ  0x03


FDC_TRY_COUNT       equ  128
FDC_INT_WAIT        equ  32768   ; ~500ns per tick



; Mode Register Bits
DMA_MODE_DEMAND     equ  0x00  ; bits 7:6
DMA_MODE_SINGLE     equ  0x40
DMA_MODE_BLOCK      equ  0x80
DMA_MODE_CASCADE    equ  0xC0

DMA_MODE_DECREMENT  equ  0x20  ; bit 5
DMA_MODE_INCREMENT  equ  0x00

DMA_MODE_AUTO_INIT  equ  0x10  ; bit 4
DMA_MODE_SINGLE_CYC equ  0x00

DMA_MODE_VERIFY     equ  0x00  ; bits 3:2
DMA_MODE_WRITE      equ  0x04
DMA_MODE_READ       equ  0x08

DMA_MODE_CHANNEL0   equ  0x00  ; bits 1:0  ; channel4
DMA_MODE_CHANNEL1   equ  0x01              ; channel5
DMA_MODE_CHANNEL2   equ  0x02              ; channel6
DMA_MODE_CHANNEL3   equ  0x03              ; channel7

DMA_CMD_WRITE       equ  (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_WRITE)  ; read from a disk, write to memory
DMA_CMD_READ        equ  (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_READ)   ; write to a disk, read from memory
DMA_CMD_VERIFY      equ  (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_VERIFY)
DMA_CMD_FORMAT      equ  (DMA_MODE_SINGLE | DMA_MODE_INCREMENT | DMA_MODE_SINGLE_CYC | DMA_MODE_READ)

DMA_MASK_REG        equ 0x0A
DMA_MODE_REG        equ 0x0B
DMA_FLIP_FLOP       equ 0x0C


ERROR_NOT_READY     equ  '10'  ; little endian '01'
ERROR_NO_INTERRUPT  equ  '20'  ;
ERROR_NOT_TWO       equ  '30'
ERROR_NO_MATCH      equ  '40'
ERROR_SPECIFY       equ  '50'
ERROR_SEEK          equ  '60'
ERROR_READ          equ  '70'
ERROR_NOT_SEVEN     equ  '80'



.end
