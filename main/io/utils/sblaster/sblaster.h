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

#define SB_LEFT_STATUS    0    //  left speaker status port  (R)
#define SB_LEFT_ADDRESS   0    //  left speaker address port (W)
#define SB_LEFT_DATA      1    //  left speaker data port    (W) (R\W on SB Pro 4)
#define SB_RIGHT_STATUS   2    // right speaker status port  (R)
#define SB_RIGHT_ADDRESS  2    // right speaker address port (W)
#define SB_RIGHT_DATA     3    // right speaker data port    (W) (R\W on SB Pro 4)
#define SB_MIXER_ADDRESS  4     // mixer register address port (index) (W) ( ** see table 1 **)
#define SB_MIXER_DATA     5     // mixer data port            (R/W)
#define SB_DSP_RESET         6  // DSP reset                  (W)
#define SB_FM_MUSIC_STATUS   8  // FM music status port       (R)
#define SB_FM_MUSIC_ADDRESS  8  // FM music address port      (W)
#define SB_FM_MUSIC_DATA     9  // FM music data port         (W)
#define SB_DSP_READ          0xA  // DSP read data (voice I/O and Midi) (R)
#define SB_DSP_WRITE         0xC  // DSP write data / write command     (W)
#define SB_DSP_WRITE_STATUS  0xC  // DSP write buffer status (bit 7)    (R)
#define SB_DSP_DATA_AVAIL    0xE  // DSP data available status (bit 7)  (R)

// DSP commands
#define SB_PAUSE_DMA        0xD0
#define SB_SPEAKER_ON       0xD1
#define SB_PAUSE_DMA16      0xD5
#define SB_DSP_ID           0xE0
#define SB_VERSION          0xE1
#define SB_COPY_STR         0xE3
#define SB_TEST_WRITE       0xE4
#define SB_TEST_READ        0xE8
#define SB_FIRE_IRQ8        0xF2
#define SB_FIRE_IRQ16       0xF3

// all chunks have a header of so.
struct WAV_CHUNK_HDR {
  bit32u id;      // chunk id
  bit32u size;    // size of chunk following this header
};

struct WAV_RIFF {
  struct WAV_CHUNK_HDR hdr; // "RIFF"  0x46464952 (size of whole file starting at next position)
  bit32u format;            // format.  Should be "WAVE" (0x45564157) for this header. 
};

struct WAV_FMT {
  struct WAV_RIFF riff;      // RIFF file
  struct WAV_CHUNK_HDR fmt;  // "fmt "  0x20746D66 // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
  bit16u format;             // PCM = 1  (i.e. Linear quantization) Values other than 1 indicate some form of compression.
  bit16u num_channels;       // Mono = 1, Stereo = 2, etc.
  bit32u sample_rate;        // 8000, 44100, etc.
  bit32u byte_rate;          // == SampleRate * NumChannels * BitsPerSample/8
  bit16u block_align;        // == NumChannels * BitsPerSample/8
  bit16u bits_per_sample;    // 8 bits = 8, 16 bits = 16, etc.
  struct WAV_CHUNK_HDR data; // "data"
};



#define SB_IO_TRYS   1024

bool get_parameters(int argc, char *arg[], char *filename, bool *auto_init);

bit16u detect_base(void);
bool reset_dsp(const bit16u base);
void dsp_stopall(const bit16u base);

bool get_dsp_id(const bit16u base, const bit8u test);
bool test_write_read(const bit16u base, const bit8u test);
bool get_dsp_version(const bit16u base, bit8u *major, bit8u *minor);
void get_copy_str(const bit16u base);

bit8u detect_dma(const bit16u base, const bool is8bit);
bit8u dma_req(void);
int detect_irq(const bit16u base, const int dma);
void dsp_transfer(const bit16u base, const int dma, const bit32u address, 
                  const bit16u len, const bit16u freq, const bit16u bsize,
                  const bool single);

void mixer_defaults(const bit16u base);
bool sb_write_byte(const bit16u base, const bit8u val);
bool sb_read_byte(const bit16u base, bit8u *val);

void interrupt irq_handler(void);
void play_wav_file(const char *filename, const bit16u base, const int dma, const bool auto_init);
void copy_mem_to_addr(const bit32u address, const bit8u *mem, const bit16u len);

void mdelay(const int ms);
