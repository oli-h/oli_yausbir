//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
#ifndef SIO_H
#define SIO_H

void vSetComBaud(DWORD zNewBaud);
BYTE bGetRxBufLen(void);
BOOL yTxBufFull(void);
BOOL yRxByte(BYTE *pbChar);
BOOL yTxByte(BYTE bChar);

#ifdef MAIN
  tDWORD zBaud;
#else
  extern tDWORD zBaud;
#endif

#endif // SIO_H
