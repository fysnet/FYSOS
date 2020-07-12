//////////////////////////////////////////////////////////////////////////
//  uhci.h  v1.00
//////////////////////////////////////////////////////////////////////////

#ifndef FYSOS__UHCI
#define FYSOS__UHCI

#pragma pack(1)

#define UHCI_COMMAND     0x00
#define UHCI_STATUS      0x02

#define UHCI_INTERRUPT   0x04
#define UHCI_FRAME_NUM   0x06
#define UHCI_FRAME_BASE  0x08
#define UHCI_SOF_MOD     0x0C

#define UHCI_PORT_WRITE_MASK  0x124E    //  0001 0010 0100 1110

#define UHCI_HUB_RESET_TIMEOUT  50

#define TOKEN_OUT    0xE1
#define TOKEN_IN     0x69
#define TOKEN_SETUP  0x2D

#define BREADTH (0<<2)
#define DEPTH   (1<<2)

#define QUEUE_HEAD_PTR_MASK  0xFFFFFFF0
#define QUEUE_HEAD_Q         0x00000002
#define QUEUE_HEAD_T         0x00000001

struct UHCI_QUEUE_HEAD {
	bit32u   horz_ptr;
	bit32u   vert_ptr;
  bit32u   resv0[2];   // to make it 16 bytes in length
};


#define TD_PTR_MASK  0xFFFFFFF0
#define TD_VF        0x00000004
#define TD_Q         0x00000002
#define TD_T         0x00000001

#define TD_FLAGS_SPD      0x20000000
#define TD_FLAGS_CERR     0x18000000
#define TD_FLAGS_LS       0x04000000
#define TD_FLAGS_ISO      0x02000000
#define TD_FLAGS_IOC      0x01000000
#define TD_STATUS_ACTIVE  0x00800000
#define TD_STATUS_STALL   0x00400000
#define TD_STATUS_DBERR   0x00200000
#define TD_STATUS_BABBLE  0x00100000
#define TD_STATUS_NAK     0x00080000
#define TD_STATUS_CRC_TO  0x00040000
#define TD_STATUS_BSTUFF  0x00020000
#define TD_STATUS_MASK    0x00FF0000
#define TD_ACTLEN_MASK    0x000007FF

#define TD_INFO_MAXLEN_MASK   0xFFE00000
#define TD_INFO_MAXLEN_SHFT   21
#define TD_INFO_D             0x00080000
#define TD_INFO_ENDPT_MASK    0x00078000
#define TD_INFO_ENDPT_SHFT    15
#define TD_INFO_ADDR_MASK     0x00007F00
#define TD_INFO_ADDR_SHFT     8
#define TD_INFO_PID           0x000000FF

struct UHCI_TRANSFER_DESCRIPTOR { 
  bit32u   link_ptr;
  bit32u   reply;
	bit32u   info;
	bit32u   buff_ptr;
	bit32u   resv0[4];          // the last 4 dwords are reserved for software use.
};



#endif // FYSOS__UHCI
