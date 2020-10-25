//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#include "common.h"
#include "USB_Main.h"
#include "sio.h"

#define TBUF_SIZE 256 // DO NOT CHANGE
#define RBUF_SIZE 256 // DO NOT CHANGE

static xdata BYTE bTxBuf[TBUF_SIZE];
static xdata BYTE bRxBuf[RBUF_SIZE];

static xdata BYTE bTxIn = 0;
static xdata BYTE bTxOut = 0;
static BOOL yTxDisabled = FALSE;

static xdata BYTE bRxIn = 0;
static xdata BYTE bRxOut = 0;

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
BOOL yTxBufFull(void)
{
  EA = 0;                                                               // Disable Interrupts
  if ((TBUF_SIZE - (bTxIn - bTxOut)) <= 2) {
    EA = 1;                                                             // Enable Interrupts
    return TRUE;
  }
  EA = 1;                                                               // Enable Interrupts
  return FALSE;
}

//**********************************************************************+*********************************************
// ein Byte von der serielle Schnittstelle (UART1) empfangen oder 
// ein Byte gesendet
//**********************************************************************+*********************************************
static void vComIsr(void) interrupt 16 using 3
{
  // --- Received data interrupt ---
  if (SCON1 & 0x01) { // (RI1!=0) {
    if ((bRxIn + 1) != bRxOut) bRxBuf[bRxIn++] = SBUF1;
    SCON1 &= ~0x01;// RI1 = 0;
  }

  // --- Transmitted data interrupt ---
  if (SCON1 & 0x02) { // (TI0!=0) {
    SCON1 &= ~0x02;// TI1 = 0;
    if (bTxIn != bTxOut)
      SBUF1 = bTxBuf[bTxOut++];
    else
      yTxDisabled = TRUE;
  }
}

//**********************************************************************+*********************************************
// ein Byte ueber die serielle Schnittstelle (UART1) senden
//**********************************************************************+*********************************************
BOOL yTxByte(BYTE bChar)
{
  // --- If the buffer is full, return an error value ---
  if (yTxBufFull())
    return FALSE;

  // --- Add the data to the transmit buffer. If the transmit interrupt is disabled, then enable it ---
  EA = 0;                                                               // Disable Interrupts

  bTxBuf[bTxIn++] = bChar;

  if (yTxDisabled) {                                                    // if transmitter is disabled
    yTxDisabled = FALSE;
    SCON1 |= 0x02; // TI1 = 1;                                          // enable it
  }

  EA = 1;                                                               // Enable Interrupts

  return TRUE;
}

//**********************************************************************+*********************************************
// ein Byte von der serielle Schnittstelle (UART1) holen
//**********************************************************************+*********************************************
BOOL yRxByte(BYTE *pbChar)
{
  if ((bRxIn - bRxOut)==0) return FALSE;

  EA = 0;                                                               // Disable Interrupts
  *pbChar = bRxBuf[bRxOut++];
  EA = 1;                                                               // Enable Interrupts

  return TRUE;
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
BYTE bGetRxBufLen(void)
{
  return (bRxIn - bRxOut);
}

//**********************************************************************+*********************************************
// Configure the UART1 using Baudrate generator, for <BAUDRATE1> and 8-N-1.
//**********************************************************************+*********************************************
void vSetComBaud(DWORD zNewBaud)
{
  zBaud.z = zNewBaud;

  // mit RX1 und TX2

  P0MDOUT   = 0x41;
  P0SKIP    = 0x3F;// TX1 = P0.6, RX1 = P0.7
  XBR1      = 0x40;
  XBR2      = 0x01;

  SMOD1 = 0x0C;                                                         // set to disable parity, 8-data bit, disable extra bit, top bit 1 bit wide
  SCON1 = 0x10;                        // SCON1: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI1 and TI1 bits

  // Baud Rate = [BRG Clock / (65536 - (SBRLH1:SBRLL1))] x 1/2 x 1/Prescaler

  if (SYSCLK/zNewBaud/2/0xFFFF < 1) {
    SBRL1 = -(SYSCLK/zNewBaud/2);
    SBCON1 |= 0x03;                                                     // set prescaler to 1
  } else if (SYSCLK/zNewBaud/2/0xFFFF < 4) {
    SBRL1 = -(SYSCLK/zNewBaud/2/4);
    SBCON1 &= ~0x03;
    SBCON1 |= 0x01;                                                     // set prescaler to 4

  } else if (SYSCLK/zNewBaud/2/0xFFFF < 12) {
    SBRL1 = -(SYSCLK/zNewBaud/2/12);
    SBCON1 &= ~0x03;                                                    // set prescaler to 12
  } else {
    SBRL1 = -(SYSCLK/zNewBaud/2/48);
    SBCON1 &= ~0x03;
    SBCON1 |= 0x02;                                                     // set prescaler to 4
  }

  SCON1 |= 0x02;                                                        // indicate ready for TX
  SBCON1 |= 0x40;                                                       // enable baud rate generator

  yTxDisabled = TRUE;                                                   // Flag showing that UART can transmit
  EIE2 |= 0x02;                                                         // Enable UART1 interrupts
}