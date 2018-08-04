
#include "ctype.h"

#include "malloc.h"

#include "bz2.h"
#include "windows.h"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void makeMaps_d(struct S_DSTATE *s) {
  int i;
  s->nInUse = 0;
  for (i = 0; i < 256; i++)
    if (s->inUse[i]) {
      s->seqToUnseq[s->nInUse] = i;
      s->nInUse++;
    }
}

bool get_bits(struct S_DSTATE *s, bit32u *p, const int n) {
  while (1) {
    if (s->bsLive >= n) {
       *p = (s->bsBuff >> (s->bsLive - n)) & ((1 << n) - 1);
       s->bsLive -= n;
       break;
    }
    if (s->strm->avail_in == 0)
      return TRUE;
    s->bsBuff = (s->bsBuff << 8) | s->strm->next_in[0];
    s->bsLive += 8;
    s->strm->next_in++;
    s->strm->avail_in--;
    s->strm->total_in_lo32++;
    if (s->strm->total_in_lo32 == 0)
      s->strm->total_in_hi32++;
  }
  return FALSE;
}

void BZ2_hbCreateDecodeTables(int *limit, int *base, int *perm, bit8u *length, 
                              const int minLen, const int maxLen, const int alphaSize) {
  int pp = 0, i, vec;
  
  for (i = minLen; i <= maxLen; i++)
    for (int j=0; j<alphaSize; j++)
      if (length[j] == i) {
        perm[pp] = j;
        pp++;
      }
  for (i = 0; i < BZ_MAX_CODE_LEN; i++)
    base[i] = 0;
  for (i = 0; i < alphaSize; i++)
    base[length[i] + 1]++;
  
  for (i = 1; i < BZ_MAX_CODE_LEN; i++)
    base[i] += base[i-1];
  
  for (i = 0; i < BZ_MAX_CODE_LEN; i++)
    limit[i] = 0;
  
  vec = 0;
  for (i = minLen; i <= maxLen; i++) {
    vec += (base[i+1] - base[i]);
    limit[i] = vec-1;
    vec <<= 1;
  }
  for (i = minLen + 1; i <= maxLen; i++)
    base[i] = ((limit[i-1] + 1) << 1) - base[i];
}

bit32u BZ2_crc32Table[256] = {
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
  0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
  0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
  0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
  0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
  0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
  0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
  0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
  0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
  0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
  0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
  0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
  0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
  0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
  0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
  0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
  0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
  0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
  0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
  0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
  0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
  0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
  0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
  0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
  0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
  0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
  0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
  0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
  0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
  0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
  0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
  0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

int BZ2_rNums[512] = { 
   619, 720, 127, 481, 931, 816, 813, 233, 566, 247, 
   985, 724, 205, 454, 863, 491, 741, 242, 949, 214, 
   733, 859, 335, 708, 621, 574,  73, 654, 730, 472, 
   419, 436, 278, 496, 867, 210, 399, 680, 480,  51, 
   878, 465, 811, 169, 869, 675, 611, 697, 867, 561, 
   862, 687, 507, 283, 482, 129, 807, 591, 733, 623, 
   150, 238,  59, 379, 684, 877, 625, 169, 643, 105, 
   170, 607, 520, 932, 727, 476, 693, 425, 174, 647, 
    73, 122, 335, 530, 442, 853, 695, 249, 445, 515, 
   909, 545, 703, 919, 874, 474, 882, 500, 594, 612, 
   641, 801, 220, 162, 819, 984, 589, 513, 495, 799, 
   161, 604, 958, 533, 221, 400, 386, 867, 600, 782, 
   382, 596, 414, 171, 516, 375, 682, 485, 911, 276, 
    98, 553, 163, 354, 666, 933, 424, 341, 533, 870, 
   227, 730, 475, 186, 263, 647, 537, 686, 600, 224, 
   469,  68, 770, 919, 190, 373, 294, 822, 808, 206, 
   184, 943, 795, 384, 383, 461, 404, 758, 839, 887, 
   715,  67, 618, 276, 204, 918, 873, 777, 604, 560, 
   951, 160, 578, 722,  79, 804,  96, 409, 713, 940, 
   652, 934, 970, 447, 318, 353, 859, 672, 112, 785, 
   645, 863, 803, 350, 139,  93, 354,  99, 820, 908, 
   609, 772, 154, 274, 580, 184,  79, 626, 630, 742, 
   653, 282, 762, 623, 680,  81, 927, 626, 789, 125, 
   411, 521, 938, 300, 821,  78, 343, 175, 128, 250, 
   170, 774, 972, 275, 999, 639, 495,  78, 352, 126, 
   857, 956, 358, 619, 580, 124, 737, 594, 701, 612, 
   669, 112, 134, 694, 363, 992, 809, 743, 168, 974, 
   944, 375, 748,  52, 600, 747, 642, 182, 862,  81, 
   344, 805, 988, 739, 511, 655, 814, 334, 249, 515, 
   897, 955, 664, 981, 649, 113, 974, 459, 893, 228, 
   433, 837, 553, 268, 926, 240, 102, 654, 459,  51, 
   686, 754, 806, 760, 493, 403, 415, 394, 687, 700, 
   946, 670, 656, 610, 738, 392, 760, 799, 887, 653, 
   978, 321, 576, 617, 626, 502, 894, 679, 243, 440, 
   680, 879, 194, 572, 640, 724, 926,  56, 204, 700, 
   707, 151, 457, 449, 797, 195, 791, 558, 945, 679, 
   297,  59,  87, 824, 713, 663, 412, 693, 342, 606, 
   134, 108, 571, 364, 631, 212, 174, 643, 304, 329, 
   343,  97, 430, 751, 497, 314, 983, 374, 822, 928, 
   140, 206,  73, 263, 980, 736, 876, 478, 430, 305, 
   170, 514, 364, 692, 829,  82, 855, 953, 676, 246, 
   369, 970, 294, 750, 807, 827, 150, 790, 288, 923, 
   804, 378, 215, 828, 592, 281, 565, 555, 710,  82, 
   896, 831, 547, 261, 524, 462, 293, 465, 502,  56, 
   661, 821, 976, 991, 658, 869, 905, 758, 745, 193, 
   768, 550, 608, 933, 378, 286, 215, 979, 792, 961, 
    61, 688, 793, 644, 986, 403, 106, 366, 905, 644, 
   372, 567, 466, 434, 645, 210, 389, 550, 919, 135, 
   780, 773, 635, 389, 707, 100, 626, 958, 165, 504, 
   920, 176, 193, 713, 857, 265, 203,  50, 668, 108, 
   645, 990, 626, 197, 510, 357, 358, 850, 858, 364, 
   936, 638
};

//---------------------------------------------------
int BZ2_decompress(struct S_DSTATE *s) {
  bit32u   uc;
  int      retVal;
  int      minLen, maxLen;
  struct S_BZ_STREAM *strm = s->strm;
  
  // stuff that needs to be saved/restored
  int  i;
  int  j;
  int  t;
  int  alphaSize;
  int  nGroups;
  int  nSelectors;
  int  EOB;
  int  groupNo;
  int  groupPos;
  int  nextSym;
  int  nblockMAX;
  int  nblock;
  int  es;
  int  N;
  int  curr;
  int  zt;
  int  zn; 
  int  zvec;
  int  zj;
  int  gSel;
  int  gMinlen;
  int *gLimit;
  int *gBase;
  int *gPerm;
  
  if (s->state == BZ_X_MAGIC_1) {
    //initialise the save area
    s->save_i           = 0;
    s->save_j           = 0;
    s->save_t           = 0;
    s->save_alphaSize   = 0;
    s->save_nGroups     = 0;
    s->save_nSelectors  = 0;
    s->save_EOB         = 0;
    s->save_groupNo     = 0;
    s->save_groupPos    = 0;
    s->save_nextSym     = 0;
    s->save_nblockMAX   = 0;
    s->save_nblock      = 0;
    s->save_es          = 0;
    s->save_N           = 0;
    s->save_curr        = 0;
    s->save_zt          = 0;
    s->save_zn          = 0;
    s->save_zvec        = 0;
    s->save_zj          = 0;
    s->save_gSel        = 0;
    s->save_gMinlen     = 0;
    s->save_gLimit      = NULL;
    s->save_gBase       = NULL;
    s->save_gPerm       = NULL;
  }
  
  //restore from the save area
  i           = s->save_i;
  j           = s->save_j;
  t           = s->save_t;
  alphaSize   = s->save_alphaSize;
  nGroups     = s->save_nGroups;
  nSelectors  = s->save_nSelectors;
  EOB         = s->save_EOB;
  groupNo     = s->save_groupNo;
  groupPos    = s->save_groupPos;
  nextSym     = s->save_nextSym;
  nblockMAX   = s->save_nblockMAX;
  nblock      = s->save_nblock;
  es          = s->save_es;
  N           = s->save_N;
  curr        = s->save_curr;
  zt          = s->save_zt;
  zn          = s->save_zn; 
  zvec        = s->save_zvec;
  zj          = s->save_zj;
  gSel        = s->save_gSel;
  gMinlen     = s->save_gMinlen;
  gLimit      = s->save_gLimit;
  gBase       = s->save_gBase;
  gPerm       = s->save_gPerm;
  
  retVal = BZ_OK;
  switch (s->state) {
    case BZ_X_MAGIC_1: 
      s->state = BZ_X_MAGIC_1;
      if (get_bits(s, &uc, 8)) {
        retVal = BZ_OK;
        goto save_state_and_return;
      }
      if (uc != BZ_HDR_B) {
        retVal = BZ_DATA_ERROR_MAGIC;
        goto save_state_and_return;
      }
   case BZ_X_MAGIC_2:
     s->state = BZ_X_MAGIC_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != BZ_HDR_Z) {
       retVal = BZ_DATA_ERROR_MAGIC;
       goto save_state_and_return;
     }
   case BZ_X_MAGIC_3: 
     s->state = BZ_X_MAGIC_3;
     if (get_bits(s, &uc, 8)) {
        retVal = BZ_OK;
        goto save_state_and_return;
      }
     if (uc != BZ_HDR_h) {
       retVal = BZ_DATA_ERROR_MAGIC;
       goto save_state_and_return;
     }
   case BZ_X_MAGIC_4: 
     s->state = BZ_X_MAGIC_4;
     if (get_bits(s, &uc, 8)) {
       s->blockSize100k = uc; // ??????? (we have it below)
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->blockSize100k = uc;
     if (s->blockSize100k < (BZ_HDR_0 + 1) || 
         s->blockSize100k > (BZ_HDR_0 + 9)) {
       retVal = BZ_DATA_ERROR_MAGIC;
       goto save_state_and_return;
     }
     s->blockSize100k -= BZ_HDR_0;
     s->tt = (bit32u *) malloc(s->blockSize100k * 100000 * sizeof(int));
     if (s->tt == NULL) {
       retVal = BZ_MEM_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_1: 
     s->state = BZ_X_BLKHDR_1;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc == 0x17)
       goto endhdr_2;
     if (uc != 0x31) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_2:
     s->state = BZ_X_BLKHDR_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x41) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_3:
     s->state = BZ_X_BLKHDR_3;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x59) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_4:
     s->state = BZ_X_BLKHDR_4;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x26) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_5:
     s->state = BZ_X_BLKHDR_5;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x53) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_BLKHDR_6:
     s->state = BZ_X_BLKHDR_6;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x59) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
     s->currBlockNo++;
     s->storedBlockCRC = 0;
   case BZ_X_BCRC_1:
     s->state = BZ_X_BCRC_1;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedBlockCRC = (s->storedBlockCRC << 8) | ((bit32u) uc);
   case BZ_X_BCRC_2:
     s->state = BZ_X_BCRC_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedBlockCRC = (s->storedBlockCRC << 8) | ((bit32u) uc);
   case BZ_X_BCRC_3:
     s->state = BZ_X_BCRC_3;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedBlockCRC = (s->storedBlockCRC << 8) | ((bit32u) uc);
   case BZ_X_BCRC_4:
     s->state = BZ_X_BCRC_4;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedBlockCRC = (s->storedBlockCRC << 8) | ((bit32u) uc);
   case BZ_X_RANDBIT: 
     s->state = BZ_X_RANDBIT;
     if (get_bits(s, &uc, 1)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->blockRandomised = (bool) (uc & 1);
     s->origPtr = 0;
   case BZ_X_ORIGPTR_1:
     s->state = BZ_X_ORIGPTR_1;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->origPtr = (s->origPtr << 8) | ((int) uc);
   case BZ_X_ORIGPTR_2:
     s->state = BZ_X_ORIGPTR_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->origPtr = (s->origPtr << 8) | ((int) uc);
   case BZ_X_ORIGPTR_3:
     s->state = BZ_X_ORIGPTR_3;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->origPtr = (s->origPtr << 8) | ((int) uc);
     if (s->origPtr < 0) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
     if (s->origPtr > (10 + 100000 * s->blockSize100k)) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
     //--- Receive the mapping table ---
     for (i = 0; i < 16; i++) {
       case BZ_X_MAPPING_1:
         s->state = BZ_X_MAPPING_1;
         if (get_bits(s, &uc, 1)) {
           retVal = BZ_OK;
           goto save_state_and_return;
         }
         //s->inUse16[i] = (bool) uc;
         if (uc == 1) s->inUse16[i] = TRUE;
         else         s->inUse16[i] = FALSE;
     }
     
     for (i = 0; i < 256; i++)
       s->inUse[i] = FALSE;
     for (i = 0; i < 16; i++)
       if (s->inUse16[i])
         for (j = 0; j < 16; j++) {
           case BZ_X_MAPPING_2: 
             s->state = BZ_X_MAPPING_2;
             if (get_bits(s, &uc, 1)) {
               retVal = BZ_OK;
               goto save_state_and_return;
             }
             if (uc == 1) s->inUse[i * 16 + j] = TRUE;
         }

     makeMaps_d(s);
     if (s->nInUse == 0) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
     alphaSize = s->nInUse+2;
     
   //--- Now the selectors ---
   case BZ_X_SELECTOR_1: 
     s->state = BZ_X_SELECTOR_1;
     if (get_bits(s, &nGroups, 3)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (nGroups < 2 || nGroups > 6) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
   case BZ_X_SELECTOR_2: 
     s->state = BZ_X_SELECTOR_2;
     if (get_bits(s, &nSelectors, 15)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (nSelectors < 1) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return;
     }
     
     for (i = 0; i < nSelectors; i++) {
       j = 0;
       while (1) {
         case BZ_X_SELECTOR_3: 
         s->state = BZ_X_SELECTOR_3;
         if (get_bits(s, &uc, 1)) {
           retVal = BZ_OK;
           goto save_state_and_return;
         }
         if (uc == 0) break;
         j++;
         if (j >= nGroups) {
           retVal = BZ_DATA_ERROR;
           goto save_state_and_return;
         }
       }
       s->selectorMtf[i] = j;
     }
     
     //--- Undo the MTF values for the selectors. ---
     {
       bit8u pos[BZ_N_GROUPS], tmp, v;
       for (v = 0; v < nGroups; v++) pos[v] = v;
       for (i = 0; i < nSelectors; i++) {
         v = s->selectorMtf[i];
         tmp = pos[v];
         while (v > 0) { pos[v] = pos[v-1]; v--; }
         pos[0] = tmp;
         s->selector[i] = tmp;
       }
     }
     
     //--- Now the coding tables ---
     for (t = 0; t < nGroups; t++) {
       case BZ_X_CODING_1: 
         s->state = BZ_X_CODING_1;
         if (get_bits(s, &curr, 5)) {
           retVal = BZ_OK;
           goto save_state_and_return;
         }
         
         for (i = 0; i < alphaSize; i++) {
           while (1) {
             if (curr < 1 || curr > 20) { 
               retVal = BZ_DATA_ERROR; 
               goto save_state_and_return; 
             }
             case BZ_X_CODING_2: 
               s->state = BZ_X_CODING_2;
               if (get_bits(s, &uc, 1)) {
                 retVal = BZ_OK;
                 goto save_state_and_return;
               }
               if (uc == 0) break;
             case BZ_X_CODING_3: 
               s->state = BZ_X_CODING_3;
               if (get_bits(s, &uc, 1)) {
                 retVal = BZ_OK;
                 goto save_state_and_return;
               }
               if (uc == 0) curr++; 
               else         curr--;
           }
           s->len[t][i] = curr;
        }
      }
      
      //--- Create the Huffman decoding tables ---
      for (t = 0; t < nGroups; t++) {
        minLen = 32;
        maxLen = 0;
        for (i = 0; i < alphaSize; i++) {
          if (s->len[t][i] > maxLen) maxLen = s->len[t][i];
          if (s->len[t][i] < minLen) minLen = s->len[t][i];
        }
        BZ2_hbCreateDecodeTables(&s->limit[t][0], &s->base[t][0], &s->perm[t][0], &s->len[t][0], minLen, maxLen, alphaSize);
        s->minLens[t] = minLen;
      }
      
      //--- Now the MTF values ---
      EOB      = s->nInUse+1;
      nblockMAX = 100000 * s->blockSize100k;
      groupNo  = -1;
      groupPos = 0;
      
      for (i = 0; i <= 255; i++)
        s->unzftab[i] = 0;
      //-- MTF init --
      {
        int kk = MTFA_SIZE-1;
        for (int ii= 256 / MTFL_SIZE - 1; ii >= 0; ii--) {
          for (int jj= MTFL_SIZE-1; jj >= 0; jj--) {
            s->mtfa[kk] = (bit8u)(ii * MTFL_SIZE + jj);
            kk--;
          }
          s->mtfbase[ii] = kk + 1;
        }
      }
      //-- end MTF init --
      nblock = 0;
      
      if (groupPos == 0) {
        groupNo++;
        if (groupNo >= nSelectors) {
          retVal = BZ_DATA_ERROR; 
          goto save_state_and_return; 
        }
        groupPos = BZ_G_SIZE;
        gSel = s->selector[groupNo];
        gMinlen = s->minLens[gSel];
        gLimit = &s->limit[gSel][0];
        gPerm = &s->perm[gSel][0];
        gBase = &s->base[gSel][0];
      }
      groupPos--;
      zn = gMinlen;
    case BZ_X_MTF_1: 
      s->state = BZ_X_MTF_1;
      if (get_bits(s, &zvec, zn)) {
        retVal = BZ_OK;
        goto save_state_and_return;
      }
      
      while (1) {
        if (zn > 20) {  // 20 = the longest code ((? if 20 is the longest, should that be '>= 20'  ??? ))
          retVal = BZ_DATA_ERROR; 
          goto save_state_and_return; 
        }
        if (zvec <= gLimit[zn]) break;
        zn++;
        case BZ_X_MTF_2: 
          s->state = BZ_X_MTF_2;
          if (get_bits(s, &zj, 1)) {
            retVal = BZ_OK;
            goto save_state_and_return;
          }
          zvec = (zvec << 1) | zj;
      }
       
      if (((zvec - gBase[zn]) < 0) || ((zvec - gBase[zn]) >= BZ_MAX_ALPHA_SIZE)) {
         retVal = BZ_DATA_ERROR; 
         goto save_state_and_return;
      }
      nextSym = gPerm[zvec - gBase[zn]];
      
      while (1) {
        win_put_progress(strm->total_in_lo32, strm->total_in_hi32);
        if (nextSym == EOB) 
          break;
        if ((nextSym == BZ_RUNA) || (nextSym == BZ_RUNB)) {
          es = -1;
          N = 1;
          do {
            if (nextSym == BZ_RUNA) es = es + (0+1) * N; else
            if (nextSym == BZ_RUNB) es = es + (1+1) * N;
            N = N * 2;
            if (groupPos == 0) {
              groupNo++;
              if (groupNo >= nSelectors) {
                retVal = BZ_DATA_ERROR; 
                goto save_state_and_return; 
              }
              groupPos = BZ_G_SIZE;
              gSel = s->selector[groupNo];
              gMinlen = s->minLens[gSel];
              gLimit = &(s->limit[gSel][0]);
              gPerm = &(s->perm[gSel][0]);
              gBase = &(s->base[gSel][0]);
            }
            groupPos--;
            zn = gMinlen;
            case BZ_X_MTF_3: 
              s->state = BZ_X_MTF_3;
              if (get_bits(s, &zvec, zn)) {
                retVal = BZ_OK;
                goto save_state_and_return;
              }
              while (1) {
                if (zn > 20 ) { // the longest code
                  retVal = BZ_DATA_ERROR; 
                  goto save_state_and_return;
                }
                if (zvec <= gLimit[zn]) break;
                zn++;
                case BZ_X_MTF_4: 
                  s->state = BZ_X_MTF_4;
                  if (get_bits(s, &zj, 1)) {
                    retVal = BZ_OK;
                    goto save_state_and_return;
                  }
                  zvec = (zvec << 1) | zj;
              }
              if (zvec - gBase[zn] < 0
                || zvec - gBase[zn] >= BZ_MAX_ALPHA_SIZE) {
                 retVal = BZ_DATA_ERROR; 
                 goto save_state_and_return;
              }
              nextSym = gPerm[zvec - gBase[zn]];
          } while (nextSym == BZ_RUNA || nextSym == BZ_RUNB);

          es++;
          uc = s->seqToUnseq[ s->mtfa[s->mtfbase[0]] ];
          s->unzftab[uc] += es;

          while (es > 0) {
            if (nblock >= nblockMAX) {
              retVal = BZ_DATA_ERROR;
              goto save_state_and_return;
            }
            s->tt[nblock] = (bit32u)uc;
            nblock++;
            es--;
          }
          continue;
        } else {
           if (nblock >= nblockMAX) { 
             retVal = BZ_DATA_ERROR; 
             goto save_state_and_return; 
           }
           //-- uc = MTF ( nextSym-1 ) --
           {
             int ii, jj, kk, pp, lno, off;
             bit32u nn;
             nn = (bit32u)(nextSym - 1);
             if (nn < MTFL_SIZE) {
               // avoid general-case expense 
               pp = s->mtfbase[0];
               uc = s->mtfa[pp+nn];
               while (nn > 3) {
                 int z = pp+nn;
                 s->mtfa[(z)  ] = s->mtfa[(z)-1];
                 s->mtfa[(z)-1] = s->mtfa[(z)-2];
                 s->mtfa[(z)-2] = s->mtfa[(z)-3];
                 s->mtfa[(z)-3] = s->mtfa[(z)-4];
                 nn -= 4;
               }
               while (nn > 0) { 
                 s->mtfa[(pp+nn)] = s->mtfa[(pp+nn)-1]; nn--; 
               };
               s->mtfa[pp] = uc;
             } else { 
               // general case 
               lno = nn / MTFL_SIZE;
               off = nn % MTFL_SIZE;
               pp = s->mtfbase[lno] + off;
               uc = s->mtfa[pp];
               while (pp > s->mtfbase[lno]) { 
                 s->mtfa[pp] = s->mtfa[pp-1]; pp--; 
               }
               s->mtfbase[lno]++;
               while (lno > 0) {
                 s->mtfbase[lno]--;
                 s->mtfa[s->mtfbase[lno]] 
                    = s->mtfa[s->mtfbase[lno-1] + MTFL_SIZE - 1];
                 lno--;
               }
               s->mtfbase[0]--;
               s->mtfa[s->mtfbase[0]] = uc;
               if (s->mtfbase[0] == 0) {
                 kk = MTFA_SIZE-1;
                 for (ii = 256 / MTFL_SIZE-1; ii >= 0; ii--) {
                    for (jj = MTFL_SIZE-1; jj >= 0; jj--) {
                      s->mtfa[kk] = s->mtfa[s->mtfbase[ii] + jj];
                      kk--;
                    }
                    s->mtfbase[ii] = kk + 1;
                 }
               }
             }
           }
           //-- end uc = MTF ( nextSym-1 ) --
           s->unzftab[s->seqToUnseq[uc]]++;
           s->tt[nblock]   = (bit32u)(s->seqToUnseq[uc]);
           nblock++;
           if (groupPos == 0) {
             groupNo++;
             if (groupNo >= nSelectors) {
               retVal = BZ_DATA_ERROR; 
               goto save_state_and_return; 
             }
             groupPos = BZ_G_SIZE;
             gSel = s->selector[groupNo];
             gMinlen = s->minLens[gSel];
             gLimit = &(s->limit[gSel][0]);
             gPerm = &(s->perm[gSel][0]);
             gBase = &(s->base[gSel][0]);
           }
           groupPos--;
           zn = gMinlen;
         case BZ_X_MTF_5: 
           s->state = BZ_X_MTF_5;
           if (get_bits(s, &zvec, zn)) {
             retVal = BZ_OK;
             goto save_state_and_return;
           }
           while (1) {
             if (zn > 20) { // the longest code
               retVal = BZ_DATA_ERROR; 
               goto save_state_and_return; 
             }
             if (zvec <= gLimit[zn]) break;
             zn++;
           case BZ_X_MTF_6: 
             s->state = BZ_X_MTF_6;
             if (get_bits(s, &zj, 1)) {
               retVal = BZ_OK;
               goto save_state_and_return;
             }
             zvec = (zvec << 1) | zj;
           }
           if (zvec - gBase[zn] < 0
             || zvec - gBase[zn] >= BZ_MAX_ALPHA_SIZE) {
               retVal = BZ_DATA_ERROR; 
               goto save_state_and_return; 
           }
           nextSym = gPerm[zvec - gBase[zn]];
           continue;
         }
      }
      
      // Now we know what nblock is, we can do a better sanity check on s->origPtr. 
      if (s->origPtr < 0 || s->origPtr >= nblock) {
        retVal = BZ_DATA_ERROR; 
        goto save_state_and_return; 
      }
      
      //-- Set up cftab to facilitate generation of T^(-1) --
      s->cftab[0] = 0;
      for (i = 1; i <= 256; i++) s->cftab[i] = s->unzftab[i-1];
      for (i = 1; i <= 256; i++) s->cftab[i] += s->cftab[i-1];
      for (i = 0; i <= 256; i++) {
        if (s->cftab[i] < 0 || s->cftab[i] > nblock) {
           // s->cftab[i] can legitimately be == nblock 
           retVal = BZ_DATA_ERROR; goto save_state_and_return;
        }
      }
      
      s->state_out_len = 0;
      s->state_out_ch  = 0;
      s->calculatedBlockCRC = 0xFFFFFFFF;
      s->state = BZ_X_OUTPUT;
      
      //-- compute the T^(-1) vector --
      for (i = 0; i < nblock; i++) {
        uc = (bit8u)(s->tt[i] & 0xff);
        s->tt[s->cftab[uc]] |= (i << 8);
        s->cftab[uc]++;
      }

      s->tPos = s->tt[s->origPtr] >> 8;
      s->nblock_used = 0;
      if (s->blockRandomised) {
        s->rNToGo = 0;
        s->rTPos  = 0;
        s->tPos = s->tt[s->tPos];
        s->k0 = (bit8u)(s->tPos & 0xff);
        s->tPos >>= 8;
        s->nblock_used++;
        if (s->rNToGo == 0) {
          s->rNToGo = BZ2_rNums[s->rTPos];
          s->rTPos++;
          if (s->rTPos == 512) s->rTPos = 0;
        }
        s->rNToGo--;
        s->k0 ^= BZ_RAND_MASK; 
      } else {
        s->tPos = s->tt[s->tPos];
        s->k0 = (bit8u)(s->tPos & 0xff);
        s->tPos >>= 8;
        s->nblock_used++;
      }
      retVal = BZ_OK; 
      goto save_state_and_return;

endhdr_2:
   case BZ_X_ENDHDR_2:
     s->state = BZ_X_ENDHDR_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x72) { retVal = BZ_DATA_ERROR; goto save_state_and_return; }
   case BZ_X_ENDHDR_3:
     s->state = BZ_X_ENDHDR_3;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x45) { 
       retVal = BZ_DATA_ERROR; 
       goto save_state_and_return;
     }
   case BZ_X_ENDHDR_4:
     s->state = BZ_X_ENDHDR_4;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x38) { 
       retVal = BZ_DATA_ERROR; 
       goto save_state_and_return; 
     }
   case BZ_X_ENDHDR_5:
     s->state = BZ_X_ENDHDR_5;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x50) {
       retVal = BZ_DATA_ERROR; 
       goto save_state_and_return;
     }
   case BZ_X_ENDHDR_6:
     s->state = BZ_X_ENDHDR_6;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     if (uc != 0x90) {
       retVal = BZ_DATA_ERROR;
       goto save_state_and_return; 
     }
     s->storedCombinedCRC = 0;
   case BZ_X_CCRC_1:
     s->state = BZ_X_CCRC_1;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((bit32u)uc);
   case BZ_X_CCRC_2:
     s->state = BZ_X_CCRC_2;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((bit32u)uc);
   case BZ_X_CCRC_3:
     s->state = BZ_X_CCRC_3;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((bit32u)uc);
   case BZ_X_CCRC_4:
     s->state = BZ_X_CCRC_4;
     if (get_bits(s, &uc, 8)) {
       retVal = BZ_OK;
       goto save_state_and_return;
     }
     s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((bit32u)uc);
     s->state = BZ_X_IDLE;
     retVal = BZ_STREAM_END; 
     goto save_state_and_return;
   default: ; //AssertH ( False, 4001 );
   }

save_state_and_return:
   s->save_i           = i;
   s->save_j           = j;
   s->save_t           = t;
   s->save_alphaSize   = alphaSize;
   s->save_nGroups     = nGroups;
   s->save_nSelectors  = nSelectors;
   s->save_EOB         = EOB;
   s->save_groupNo     = groupNo;
   s->save_groupPos    = groupPos;
   s->save_nextSym     = nextSym;
   s->save_nblockMAX   = nblockMAX;
   s->save_nblock      = nblock;
   s->save_es          = es;
   s->save_N           = N;
   s->save_curr        = curr;
   s->save_zt          = zt;
   s->save_zn          = zn;
   s->save_zvec        = zvec;
   s->save_zj          = zj;
   s->save_gSel        = gSel;
   s->save_gMinlen     = gMinlen;
   s->save_gLimit      = gLimit;
   s->save_gBase       = gBase;
   s->save_gPerm       = gPerm;
   
   return retVal;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// initialize the strm structure
int bz2_bzdecompressinit(struct S_BZ_STREAM *strm) {
  struct S_DSTATE *state = NULL;
  
  if (strm == NULL)
    return BZ_PARAM_ERROR;
  
  strm->state = NULL;
  
  state = (struct S_DSTATE *) malloc(sizeof(struct S_DSTATE));
  if (state == NULL)
    return BZ_MEM_ERROR;
  
  state->strm = strm;
  state->state = BZ_X_MAGIC_1;
  state->bsLive = 0;
  state->bsBuff = 0;
  state->calculatedCombinedCRC = 0;
  state->tt = 0;
  state->currBlockNo = 0;
  
  strm->state = state;
  strm->total_in_lo32 = 0;
  strm->total_in_hi32 = 0;
  strm->total_out_lo32 = 0;
  strm->total_out_hi32 = 0;
  
  return BZ_OK;
}

// Return  TRUE if data corruption is discovered.
// Returns FALSE if there is no problem.
bool unrle_obuf_to_output_fast(struct S_DSTATE *s) {
  bit8u k1;
  
  if (s->blockRandomised) {
    while (1) {
      // try to finish existing run
      while (1) {
        if (s->strm->avail_out == 0) 
          return FALSE;
        if (s->state_out_len == 0) 
          break;
        s->strm->next_out[0] = s->state_out_ch;
        s->calculatedBlockCRC = (s->calculatedBlockCRC << 8) ^
          BZ2_crc32Table[(s->calculatedBlockCRC >> 24) ^ s->state_out_ch];
        s->state_out_len--;
        s->strm->next_out++;
        s->strm->avail_out--;
        s->strm->total_out_lo32++;
        if (s->strm->total_out_lo32 == 0)
          s->strm->total_out_hi32++;
      }
      // can a new run be started? 
      if (s->nblock_used == s->save_nblock + 1)
        return FALSE;
      
      // Only caused by corrupt data stream?
      if (s->nblock_used > (s->save_nblock + 1))
        return TRUE;
      
      s->state_out_len = 1;
      s->state_out_ch = s->k0;
      s->tPos = s->tt[s->tPos];
      k1 = (bit8u) (s->tPos & 0xFF);
      s->tPos >>= 8;
      if (s->rNToGo == 0) {
        s->rNToGo = BZ2_rNums[s->rTPos];
        s->rTPos++;
        if (s->rTPos == 512) 
          s->rTPos = 0;
      }
      s->rNToGo--;
      k1 ^= BZ_RAND_MASK; 
      s->nblock_used++;
      if (s->nblock_used == (s->save_nblock + 1))
        continue;
      if (k1 != s->k0) { 
        s->k0 = k1; 
        continue; 
      }      
      s->state_out_len = 2;
      s->tPos = s->tt[s->tPos];
      k1 = (bit8u) (s->tPos & 0xff);
      s->tPos >>= 8;
      if (s->rNToGo == 0) {
        s->rNToGo = BZ2_rNums[s->rTPos];
        s->rTPos++;
        if (s->rTPos == 512) s->rTPos = 0;
      }
      s->rNToGo--;
      k1 ^= BZ_RAND_MASK; 
      s->nblock_used++;
      if (s->nblock_used == (s->save_nblock + 1))
        continue;
      if (k1 != s->k0) {
        s->k0 = k1;
        continue;
      }
      
      s->state_out_len = 3;
      s->tPos = s->tt[s->tPos];
      k1 = (bit8u) (s->tPos & 0xFF);
      s->tPos >>= 8;
      if (s->rNToGo == 0) {
        s->rNToGo = BZ2_rNums[s->rTPos];
        s->rTPos++;
        if (s->rTPos == 512) s->rTPos = 0;
      }
      s->rNToGo--;
      k1 ^= BZ_RAND_MASK; 
      s->nblock_used++;
      if (s->nblock_used == (s->save_nblock + 1))
        continue;
      if (k1 != s->k0) {
        s->k0 = k1;
        continue;
      }
      s->tPos = s->tt[s->tPos];
      k1 = (bit8u) (s->tPos & 0xff);
      s->tPos >>= 8;
      if (s->rNToGo == 0) {
        s->rNToGo = BZ2_rNums[s->rTPos];
        s->rTPos++;
        if (s->rTPos == 512) 
          s->rTPos = 0;
      }
      s->rNToGo--;
      k1 ^= BZ_RAND_MASK; 
      s->nblock_used++;
      s->state_out_len = ((int)k1) + 4;
      s->tPos = s->tt[s->tPos];
      s->k0 = (bit8u) (s->tPos & 0xFF);
      s->tPos >>= 8;
      if (s->rNToGo == 0) {
        s->rNToGo = BZ2_rNums[s->rTPos];
        s->rTPos++;
        if (s->rTPos == 512)
          s->rTPos = 0;
      }
      s->rNToGo--;
      s->k0 ^= BZ_RAND_MASK; 
      s->nblock_used++;
    }
  } else {
    // restore 
    bit32u       c_calculatedBlockCRC = s->calculatedBlockCRC;
    bit8u        c_state_out_ch       = s->state_out_ch;
    int          c_state_out_len      = s->state_out_len;
    int          c_nblock_used        = s->nblock_used;
    int          c_k0                 = s->k0;
    bit32u      *c_tt                 = s->tt;
    bit32u       c_tPos               = s->tPos;
    bit8u       *cs_next_out          = s->strm->next_out;
    bit32u       cs_avail_out         = s->strm->avail_out;
    // end restore
    
    bit32u      avail_out_INIT = cs_avail_out;
    int         s_save_nblockPP = s->save_nblock + 1;
    bit32u      total_out_lo32_old;
    
    while (1) {
      // try to finish existing run
      if (c_state_out_len > 0) {
        while (1) {
          if (cs_avail_out == 0)
            goto return_notr;
          if (c_state_out_len == 1)
            break;
          *cs_next_out = c_state_out_ch;
          c_calculatedBlockCRC = (c_calculatedBlockCRC << 8) ^
            BZ2_crc32Table[(c_calculatedBlockCRC >> 24) ^ (bit8u) c_state_out_ch];
          c_state_out_len--;
          cs_next_out++;
          cs_avail_out--;
        }
s_state_out_len_eq_one:
        {
          if (cs_avail_out == 0) { 
            c_state_out_len = 1;
            goto return_notr;
          }
          *cs_next_out = c_state_out_ch;
          c_calculatedBlockCRC = (c_calculatedBlockCRC << 8) ^
            BZ2_crc32Table[(c_calculatedBlockCRC >> 24) ^ (bit8u) c_state_out_ch];
          cs_next_out++;
          cs_avail_out--;
        }
      }
      // Only caused by corrupt data stream? 
      if (c_nblock_used > s_save_nblockPP)
        return TRUE;
      
      // can a new run be started?
      if (c_nblock_used == s_save_nblockPP) {
        c_state_out_len = 0; 
        goto return_notr;
      }
      c_state_out_ch = c_k0;
      c_tPos = c_tt[c_tPos];
      k1 = (bit8u)(c_tPos & 0xff);
      c_tPos >>= 8;
      c_nblock_used++;
      if (k1 != c_k0) { 
        c_k0 = k1; 
        goto s_state_out_len_eq_one; 
      }
      if (c_nblock_used == s_save_nblockPP) 
        goto s_state_out_len_eq_one;

      c_state_out_len = 2;
      c_tPos = c_tt[c_tPos];
      k1 = (bit8u)(c_tPos & 0xff);
      c_tPos >>= 8;
      c_nblock_used++;
      if (c_nblock_used == s_save_nblockPP) continue;
      if (k1 != c_k0) { 
        c_k0 = k1; 
        continue;
      }
      
      c_state_out_len = 3;
      c_tPos = c_tt[c_tPos];
      k1 = (bit8u)(c_tPos & 0xff);
      c_tPos >>= 8;
      c_nblock_used++;
      if (c_nblock_used == s_save_nblockPP) 
        continue;
      if (k1 != c_k0) {
        c_k0 = k1;
        continue;
      }
      
      c_tPos = c_tt[c_tPos];
      k1 = (bit8u)(c_tPos & 0xff);
      c_tPos >>= 8;
      c_nblock_used++;
      c_state_out_len = ((int)k1) + 4;
      c_tPos = c_tt[c_tPos];
      c_k0 = (bit8u)(c_tPos & 0xff);
      c_tPos >>= 8;
      c_nblock_used++;
    }
    
return_notr:
    total_out_lo32_old = s->strm->total_out_lo32;
    s->strm->total_out_lo32 += (avail_out_INIT - cs_avail_out);
    if (s->strm->total_out_lo32 < total_out_lo32_old)
      s->strm->total_out_hi32++;
   
    // save 
    s->calculatedBlockCRC = c_calculatedBlockCRC;
    s->state_out_ch       = c_state_out_ch;
    s->state_out_len      = c_state_out_len;
    s->nblock_used        = c_nblock_used;
    s->k0                 = c_k0;
    s->tt                 = c_tt;
    s->tPos               = c_tPos;
    s->strm->next_out     = cs_next_out;
    s->strm->avail_out    = cs_avail_out;
    // end save
  }
  return FALSE;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
int bz2_bzdecompress(struct S_BZ_STREAM *strm) {
  struct S_DSTATE *s;
  
  if (strm == NULL)
    return BZ_PARAM_ERROR;
  s = strm->state;
  if (s == NULL) 
    return BZ_PARAM_ERROR;
  if (s->strm != strm)
    return BZ_PARAM_ERROR;
  
  while (1) {
    if (s->state == BZ_X_IDLE) 
      return BZ_SEQUENCE_ERROR;
    if (s->state == BZ_X_OUTPUT) {
      if (unrle_obuf_to_output_fast(s))
        return BZ_DATA_ERROR;
      if ((s->nblock_used == (s->save_nblock + 1)) && (s->state_out_len == 0)) {
        s->calculatedBlockCRC = ~s->calculatedBlockCRC;
        if (s->calculatedBlockCRC != s->storedBlockCRC)
          return BZ_DATA_ERROR;
        s->calculatedCombinedCRC = (s->calculatedCombinedCRC << 1) | (s->calculatedCombinedCRC >> 31);
        s->calculatedCombinedCRC ^= s->calculatedBlockCRC;
        s->state = BZ_X_BLKHDR_1;
      } else
        return BZ_OK;
    }
    
    if (s->state >= BZ_X_MAGIC_1) {
      int r = BZ2_decompress(s);
      if (r == BZ_STREAM_END) {
        if (s->calculatedCombinedCRC != s->storedCombinedCRC)
          return BZ_DATA_ERROR;
        return r;
      }
      if (s->state != BZ_X_OUTPUT)
        return r;
    }
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// decomrpess a buffer to a buffer
// this assumes all parameters are valid
// on entry:
//  fs:esi-> compressed data (will be on a dword boundary)
//  fs:edi-> location to store decompressed data
//  ebx    = size in bytes of compressed data.
//  ds:edx-> dword holding size in bytes of dest buffer
int bz2_bzbufftobuffdecompress(struct S_BZ_STREAM *strm, void *targ, const int *dest_buf_size, void *src, const int len) {
  int ret;
  
  if (targ == NULL || dest_buf_size == NULL || src == NULL)
    return BZ_PARAM_ERROR;
  
  ret = bz2_bzdecompressinit(strm);
  if (ret != BZ_OK) 
    return ret;
  
  strm->next_in = (bit8u *) src;
  strm->next_out = (bit8u *) targ;
  strm->avail_in = len;
  strm->avail_out = *dest_buf_size;
  
  ret = bz2_bzdecompress(strm);
  if (ret == BZ_OK)
    goto output_overflow_or_eof;
  if (ret != BZ_STREAM_END) 
    goto errhandler;
  
  // normal termination 
  *dest_buf_size -= strm->avail_out;
  ret = BZ_OK;
  goto errhandler;
  
output_overflow_or_eof:
  if (strm->avail_out > 0)
    ret = BZ_UNEXPECTED_EOF;
  else
    ret = BZ_OUTBUFF_FULL;
  
errhandler:
  mfree(strm->state->tt);
  mfree(strm->state);
  return ret; 
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  src-> compressed data (will be on a dword boundary)
//  targ-> location to store decompressed data
//  size   = size in bytes of compressed data.
//
//
int bz2_decompressor(void *targ, const void *src, const int *size) {
  struct S_BZ_STREAM strm;
  int targ_size = 0x01000000;  // 1 meg
  int ret = 0;
  
  // initialize the progress proc
  win_init_progress(*size);
  
  if ((ret = bz2_bzbufftobuffdecompress(&strm, targ, &targ_size, src, *size)) == BZ_OK)
    *size = strm.total_out_lo32;
  else
    *size = 0;
  
  return ret;
}
