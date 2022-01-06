
// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Put Image  v00.10.00    Forever Young Software 1984-2022\n";

struct DISK_TYPE {
	 int total_sects;
	 int cylinders;
	 int sec_per_track;
	 int num_heads;
	 int size;
  char name[16];
};

struct DISK_TYPE disk160  = {  320, 40,  8, 1,  160, "160K"};
struct DISK_TYPE disk180  = {  360, 40,  9, 1,  180, "180K"};
struct DISK_TYPE disk320  = {  640, 40,  8, 2,  320, "320K"};
struct DISK_TYPE disk360  = {  720, 40,  9, 2,  360, "360K"};
struct DISK_TYPE disk1220 = { 2400, 80, 15, 2, 2400, "1.22M"};
struct DISK_TYPE disk720  = { 1440, 80,  9, 2,  720, "720K"};
struct DISK_TYPE disk1440 = { 2880, 80, 18, 2, 1440, "1.44M"};
struct DISK_TYPE disk1720 = { 3360, 80, 21, 2, 1720, "1.72M"};
struct DISK_TYPE disk2880 = { 5760, 80, 36, 2, 2880, "2.88M"};

// used for direct floppy access in Win95
#define  VWIN32_DIOC_DOS_INT25      2
#define  VWIN32_DIOC_DOS_INT26      3
#define  VWIN32_DIOC_DOS_DRIVEINFO  6

typedef struct _DIOC_REGISTERS {
    unsigned long reg_EBX;
    unsigned long reg_EDX;
    unsigned long reg_ECX;
    unsigned long reg_EAX;
    unsigned long reg_EDI;
    unsigned long reg_ESI;
    unsigned long reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;

