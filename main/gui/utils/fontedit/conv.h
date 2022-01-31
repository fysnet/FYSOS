
// old format

// type zero font matrix structure
struct __attribute__((__packed__)) FONT_INFO_OLD {
  bit16u index;   // Indicies in data of each character
  bit8u  width;   // Width of character
  bit8u  flags;   // bit 0 = drop 2 pixel(s) (ex: g,q,p,y, and other chars that need to be drawn 2 pixel(s) lower)
  char   deltax;  // +/- offset to print char 
  char   deltay;  // +/- offset to print char (allows for drop chars, etc)
  char   deltaw;  // +/- offset to combine with width above when moving to the next char
  bit8u  resv;    // reserved
};

struct __attribute__((__packed__)) FONT_OLD {
  bit8u  sig[4];       // 'FONT'
  bit8u  height;       // height of char set
  bit8u  max_width;    // width of widest char in set
  bit16u start;        // starting asciiz value (first entry in font == this ascii value)
  bit16u count;        // count of chars in set ( 0 < count <= 256 )
  bit32u datalen;      // len of the data section in bytes
  bit32u total_size;   // total size of this file in bytes
  bit32u flags;        // bit 0 = fixed width font, remaining bits are reserved
  bit8u  version;      // version of font file (4-bit major : 4-bit minor) (1.0 currently allowed)
  bit8u  type;         // type of font included in this file (0 = bitmap.  The only type allowed at this point)
  bit8u  type_vers;    // version of font type used (4-bit major : 4-bit minor) (1.0 currently allowed)
  bit8u  resv[11];     // reserved
  char   name[16];     // 15 chars, 1 null
  bit8u  gui_resv[44]; // reserved for the use of the GUI (three 64-bit pointers, plus other reserved room)
  //struct FONT_INFO info[];  // char info
  //bit8u data[];     // char data
};


// new format
// type zero font matrix structure
struct __attribute__((__packed__)) FONT_INFO_NEW {
  bit32u index;   // Indicies in data of each character
  bit8u  width;   // Width of character
  char   deltax;  // +/- offset to print char 
  char   deltay;  // +/- offset to print char (allows for drop chars, etc)
  char   deltaw;  // +/- offset to combine with width above when moving to the next char
  bit8u  resv[4]; // reserved
};

struct __attribute__((__packed__)) FONT_NEW {
  bit8u  sig[4];       // 'Font'      // new signature so we don't accidently read an old version of this font file
  bit8u  height;       // height of char set
  bit8u  max_width;    // width of widest char in set
  bit16u info_start;   // zero based offset to the first FONT_INFO block
  bit32s start;        // starting code point value (first entry in font == this value) (example: 65 = ascii 'A') (*** Signed ***)
  bit32s count;        // count of chars in set ( 0 < count <= 65535 )  (*** Signed ***)
  bit32u datalen;      // len of the data section in bytes
  bit32u total_size;   // total size of this file in bytes
  bit32u flags;        // bit 0 = fixed width font, remaining bits are reserved
  char   name[32];     // UTF-8 null terminated
  bit8u  resv[36];     // reserved
  //struct FONT_INFO info[];  // char info
  //bit8u data[];     // char data
};
