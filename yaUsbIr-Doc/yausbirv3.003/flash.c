//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#include "common.h"
#include "USB_Main.h"
#include "flash.h"

//**********************************************************************+*********************************************
// Flash-Page löschen
//**********************************************************************+*********************************************
void vFlashErasePage(WORD wAdr)
{
  BOOL yEA = EA;
  xdata BYTE *pWrite;

  EA = 0;
  RSTSRC |= 0x02;                                                       // Enable VDD-Monitor für FLASH-Schreiben !

  pWrite = (BYTE xdata *)wAdr;

  PSCTL = 0x03;                                                         // MOVX writes erase FLASH page
  FLKEY = 0xA5;                                                         // FLASH lock and key sequence 1
  FLKEY = 0xF1;                                                         // FLASH lock and key sequence 2
  *pWrite = 0;                                                          // initiate page erase
  PSCTL = 0;                                                            // MOVX writes target XRAM

  EA = yEA;
  RSTSRC &= ~0x02;                                                      // Disable VDD-Monitor
}

//**********************************************************************+*********************************************
// Byte-Array im Flash speichern (Achtung es werden nur 0-Bit's geschrieben)
//**********************************************************************+*********************************************
void vFlashWrite(WORD wAdr,BYTE bData[],WORD wDataLen)
{
  BOOL yEA = EA;
  xdata BYTE *pWrite;
  xdata WORD wPos;
  idata BYTE bWriteData;

  EA = 0;
  RSTSRC |= 0x02;                                                       // Enable VDD-Monitor für FLASH-Schreiben !

  pWrite = (BYTE xdata *)wAdr;

  for (wPos=0;wPos<wDataLen;wPos++) {
    bWriteData = bData[wPos];
    PSCTL = 0x01;                                                       // MOVX writes target FLASH memory
    FLKEY = 0xA5;                                                       // FLASH lock and key sequence 1
    FLKEY = 0xF1;                                                       // FLASH lock and key sequence 2
    *pWrite = bWriteData;                                               // copy byte
    PSCTL = 0x00;                                                       // MOVX writes target XRAM
    pWrite++;
  }

  EA = yEA;
  RSTSRC &= ~0x02;                                                      // Disable VDD-Monitor
}

//**********************************************************************+*********************************************
// mit Hilfe von einer Temp-Page (512 Byte) wDataLen Bytes ab wAdr auf 0xFF setzen (löschen)
//**********************************************************************+*********************************************
void vFlashClearFF(WORD wAdr,WORD wDataLen)
{
  BOOL yEA = EA;
  WORD wSrcCopy = wAdr & 0xFE00;
  WORD wDestCopy = (WORD)&tTEMP;
  BYTE bData;

  EA = 0;

  // Temp-Page löschen
  vFlashErasePage(wDestCopy);

  // alles bis kurz vor wAdr kopieren
  while (wSrcCopy<wAdr) {
    bData = *((BYTE code *)wSrcCopy);
    if (bData!=0xFF)
      vFlashWrite(wDestCopy,(BYTE*)&bData,1);
    wSrcCopy++;
    wDestCopy++;
  }

  // wDataLen Lücke (=Erase) ab wAdr
  while (wDataLen>0) {
    wSrcCopy++;
    wDestCopy++;
    wDataLen--;
  }

  // ab wAdr+wDataLen weiter alles kopieren bis Pageende
  while (wSrcCopy<=(wAdr | 0x1FF)) {
    bData = *((BYTE code *)wSrcCopy);
    if (bData!=0xFF)
      vFlashWrite(wDestCopy,(BYTE*)&bData,1);
    wSrcCopy++;
    wDestCopy++;
  }

  // Jetzt die Daten wieder zurückkopieren
  vFlashErasePage(wAdr & 0xFE00);
  wDestCopy = wAdr & 0xFE00;
  wSrcCopy = (WORD)&tTEMP;
  while (wSrcCopy<=(((WORD)&tTEMP) | 0x1FF)) {
    bData = *((BYTE code *)wSrcCopy);
    if (bData!=0xFF)
      vFlashWrite(wDestCopy,(BYTE*)&bData,1);
    wSrcCopy++;
    wDestCopy++;
  }
  EA = yEA;
}

