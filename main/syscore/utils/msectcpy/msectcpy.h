
// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Sector Copy  v00.10.00    Forever Young Software 1984-2015\n";
char usagestr[] = "\n"
                  "\nUsage:"
                  "\n  MSECTCPY  targetfile.bin  sourcefile.bin  offset"
                  "\n"
                  "\n   targetfile.bin  - the target image file to write to"
                  "\n   sourcefile.bin  - the source file to read from"
                  "\n   offset          - zero based sector offset to write to"
                  "\n                     ex: 0 or 16 or 0x10 or 0x100 or 012."
                  "\n                      (is radix aware)"
                  "\n";

