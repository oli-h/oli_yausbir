//-----------------------------------------------------------------------------
// USB_Register.h
//-----------------------------------------------------------------------------

#ifndef USB_REGISTER_H
#define USB_REGISTER_H

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

// USB Core Registers
#define BASE            0x00
#define FADDR           BASE
#define POWER           BASE + 0x01
#define IN1INT          BASE + 0x02
#define OUT1INT         BASE + 0x04
#define CMINT           BASE + 0x06
#define IN1IE           BASE + 0x07
#define OUT1IE          BASE + 0x09
#define CMIE            BASE + 0x0B
#define FRAMEL          BASE + 0x0C
#define FRAMEH          BASE + 0x0D
#define INDEX           BASE + 0x0E
#define CLKREC          BASE + 0x0F
#define INMAX           BASE + 0x10
#define E0CSR           BASE + 0x11
#define EINCSRL         BASE + 0x11
#define EINCSRH         BASE + 0x12
#define OUTMAX          BASE + 0x13
#define EOUTCSRL        BASE + 0x14
#define EOUTCSRH        BASE + 0x15
#define E0CNT           BASE + 0x16
#define EOUTCNTL        BASE + 0x16
#define EOUTCNTH        BASE + 0x17
#define FIFO_EP0        BASE + 0x20
#define FIFO_EP1        BASE + 0x21
#define FIFO_EP2        BASE + 0x22
#define FIFO_EP3        BASE + 0x23

// USB Core Register Bits

// POWER
#define rbISOUD         0x80
#define rbSPEED         0x40
#define rbUSBRST        0x08
#define rbRESUME        0x04
#define rbSUSMD         0x02
#define rbSUSEN         0x01

// IN1INT
#define rbIN3           0x08
#define rbIN2           0x04
#define rbIN1           0x02
#define rbEP0           0x01

// OUT1INT
#define rbOUT3          0x08
#define rbOUT2          0x04
#define rbOUT1          0x02

// CMINT
#define rbSOF           0x08
#define rbRSTINT        0x04
#define rbRSUINT        0x02
#define rbSUSINT        0x01

// IN1IE
#define rbIN3E          0x08
#define rbIN2E          0x04
#define rbIN1E          0x02
#define rbEP0E          0x01

// OUT1IE
#define rbOUT3E         0x08
#define rbOUT2E         0x04
#define rbOUT1E         0x02

// CMIE
#define rbSOFE          0x08
#define rbRSTINTE       0x04
#define rbRSUINTE       0x02
#define rbSUSINTE       0x01

// E0CSR
#define rbSSUEND        0x80
#define rbSOPRDY        0x40
#define rbSDSTL         0x20
#define rbSUEND         0x10
#define rbDATAEND       0x08
#define rbSTSTL         0x04
#define rbINPRDY        0x02
#define rbOPRDY         0x01

// EINCSRL
#define rbInCLRDT       0x40
#define rbInSTSTL       0x20
#define rbInSDSTL       0x10
#define rbInFLUSH       0x08
#define rbInUNDRUN      0x04
#define rbInFIFONE      0x02
#define rbInINPRDY      0x01

// EINCSRH
#define rbInDBIEN       0x80
#define rbInISO         0x40
#define rbInDIRSEL      0x20
#define rbInFCDT        0x08
#define rbInSPLIT       0x04

// EOUTCSRL
#define rbOutCLRDT      0x80
#define rbOutSTSTL      0x40
#define rbOutSDSTL      0x20
#define rbOutFLUSH      0x10
#define rbOutDATERR     0x08
#define rbOutOVRUN      0x04
#define rbOutFIFOFUL    0x02
#define rbOutOPRDY      0x01

// EOUTCSRH
#define rbOutDBOEN      0x80
#define rbOutISO        0x40

//-----------------------------------------------------------------------------
// Register Read/Write Macros
//-----------------------------------------------------------------------------

// These first two macros do not poll for busy, and can be used to increase
// program speed where necessary, but should never be used for successive
// reads or writes.

#define READ_BYTE(addr, target) { USB0ADR = (0x80 | addr);  \
                                    while(USB0ADR & 0x80);  \
                                    target = USB0DAT; }

#define WRITE_BYTE(addr, data) {    USB0ADR = (addr);       \
                                    USB0DAT = data; }

// These two macros are polling versions of the above macros, and can be
// used for successive reads writes without taking the chance that the
// Interface Engine is busy from the last register access.

#ifdef ENABLE_INLINE_POLL_READ_WRITE

#define POLL_READ_BYTE(addr, target)    { while(USB0ADR & 0x80);    \
                                          READ_BYTE(addr, target); }

#define POLL_WRITE_BYTE(addr, dt)       { while(USB0ADR & 0x80);    \
                                          WRITE_BYTE(addr, dt);    }
#else
    #define POLL_READ_BYTE(addr, target)    { target = POLL_READ_BYTE_FUNC( addr ); }
    #define POLL_READ_BYTE_DEF
    extern BYTE POLL_READ_BYTE_FUNC( BYTE addr );
    extern void POLL_WRITE_BYTE( BYTE addr, BYTE dt );
#endif


#endif  // USB_REGISTER_H
