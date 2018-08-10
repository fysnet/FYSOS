/*             Author: Benjamin David Lunt
 *                     Forever Young Software
 *                     Copyright (c) 1984-2018
 *  
 *  This code is donated to the Freeware communitee.  You have the
 *   right to use it for learning purposes only.  You may not modify it
 *   for redistribution for any other purpose unless you have written
 *   permission from the author.
 *
 *  You may modify and use it in your own projects as long as they are
 *   for non-profit only and not distributed.  Any project for profit that 
 *   uses this code must have written permission from the author.
 *
 *  For more information:
 *    http://www.fysnet.net/osdesign_book_series.htm
 *  Contact:
 *    fys [at] fysnet [dot] net
 *
 * Last update:  10 Aug 2018
 *
 * compile using SmallerC  (https://github.com/alexfru/SmallerC/)
 *  smlrcc @make.txt
 */

//  BZ2 can be found at: http://www.bzip.org/                               *
#ifndef _BZ2_H
#define _BZ2_H

// Header bytes
#define BZ_HDR_B 0x42   // 'B'
#define BZ_HDR_Z 0x5a   // 'Z'
#define BZ_HDR_h 0x68   // 'h'
#define BZ_HDR_0 0x30   // '0'

#define BZ_RUNA 0
#define BZ_RUNB 1

#define BZ_OK                0
//#define BZ_RUN_OK            1
//#define BZ_FLUSH_OK          2
//#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
//#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
//#define BZ_CONFIG_ERROR      (-9)

#define BZ_X_IDLE        1
#define BZ_X_OUTPUT      2

#define BZ_X_MAGIC_1     10
#define BZ_X_MAGIC_2     11
#define BZ_X_MAGIC_3     12
#define BZ_X_MAGIC_4     13
#define BZ_X_BLKHDR_1    14
#define BZ_X_BLKHDR_2    15
#define BZ_X_BLKHDR_3    16
#define BZ_X_BLKHDR_4    17
#define BZ_X_BLKHDR_5    18
#define BZ_X_BLKHDR_6    19
#define BZ_X_BCRC_1      20
#define BZ_X_BCRC_2      21
#define BZ_X_BCRC_3      22
#define BZ_X_BCRC_4      23
#define BZ_X_RANDBIT     24
#define BZ_X_ORIGPTR_1   25
#define BZ_X_ORIGPTR_2   26
#define BZ_X_ORIGPTR_3   27
#define BZ_X_MAPPING_1   28
#define BZ_X_MAPPING_2   29
#define BZ_X_SELECTOR_1  30
#define BZ_X_SELECTOR_2  31
#define BZ_X_SELECTOR_3  32
#define BZ_X_CODING_1    33
#define BZ_X_CODING_2    34
#define BZ_X_CODING_3    35
#define BZ_X_MTF_1       36
#define BZ_X_MTF_2       37
#define BZ_X_MTF_3       38
#define BZ_X_MTF_4       39
#define BZ_X_MTF_5       40
#define BZ_X_MTF_6       41
#define BZ_X_ENDHDR_2    42
#define BZ_X_ENDHDR_3    43
#define BZ_X_ENDHDR_4    44
#define BZ_X_ENDHDR_5    45
#define BZ_X_ENDHDR_6    46
#define BZ_X_CCRC_1      47
#define BZ_X_CCRC_2      48
#define BZ_X_CCRC_3      49
#define BZ_X_CCRC_4      50

#define MTFA_SIZE      4096
#define MTFL_SIZE        16

#define BZ_G_SIZE        50
#define BZ_N_GROUPS 6
//#define BZ_N_ITERS  4
#define BZ_MAX_SELECTORS (2 + (900000 / BZ_G_SIZE))

#define BZ_RAND_MASK ((s->rNToGo == 1) ? 1 : 0)

#define BZ_MAX_ALPHA_SIZE 258
#define BZ_MAX_CODE_LEN    23

#pragma pack(push, 1)

struct S_BZ_STREAM {
  bit8u  *next_in;
  bit32u avail_in;
  bit32u total_in_lo32;
  bit32u total_in_hi32;
  
  bit8u *next_out;
  bit32u avail_out;
  bit32u total_out_lo32;
  bit32u total_out_hi32;
  
  struct S_DSTATE *state;
};

/*-- Structure holding all the decompression-side stuff. --*/
struct S_DSTATE {
  /* pointer back to the struct bz_stream */
  struct S_BZ_STREAM *strm;
  
  /* state indicator for this stream */
  int    state;

  /* for doing the final run-length decoding */
  bit8u  state_out_ch;
  int    state_out_len;
  bool   blockRandomised;
  int    rNToGo;
  int    rTPos;

  /* the buffer for bit stream reading */
  bit32u bsBuff;
  int    bsLive;

  /* misc administratium */
  int    blockSize100k;
  int    currBlockNo;

  /* for undoing the Burrows-Wheeler transform */
  int    origPtr;
  bit32u tPos;
  int    k0;
  int    unzftab[256];
  int    nblock_used;
  int    cftab[257];
  int    cftabCopy[257];

  /* for undoing the Burrows-Wheeler transform (FAST) */
  bit32u *tt;

  /* stored and calculated CRCs */
  bit32u storedBlockCRC;
  bit32u storedCombinedCRC;
  bit32u calculatedBlockCRC;
  bit32u calculatedCombinedCRC;

  /* map of bytes used in block */
  int    nInUse;
  bool   inUse[256];
  bool   inUse16[16];
  bit8u  seqToUnseq[256];

  /* for decoding the MTF values */
  bit8u  mtfa[MTFA_SIZE];
  int    mtfbase[256 / MTFL_SIZE];
  bit8u  selector[BZ_MAX_SELECTORS];
  bit8u  selectorMtf[BZ_MAX_SELECTORS];
  bit8u  len[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];

  int    limit[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
  int    base[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
  int    perm[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
  int    minLens[BZ_N_GROUPS];
  
  /* save area for scalars in the main decompress code */
  /* The following 24 dwords must remain in this order */
  int    save_i;
  int    save_j;
  int    save_t;
  int    save_alphaSize;
  int    save_nGroups;
  int    save_nSelectors;
  int    save_EOB;
  int    save_groupNo;
  int    save_groupPos;
  int    save_nextSym;
  int    save_nblockMAX;
  int    save_nblock;
  int    save_es;
  int    save_N;
  int    save_curr;
  int    save_zt;
  int    save_zn; 
  int    save_zvec;
  int    save_zj;
  int    save_gSel;
  int    save_gMinlen;
  int   *save_gLimit;
  int   *save_gBase;
  int   *save_gPerm;
};

#pragma pack(pop)


int bz2_decompressor(void *, const void *, const int *);

#endif // _BZ2_H
