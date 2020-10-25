//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#ifndef FLASH_H
#define FLASH_H

void vFlashErasePage(WORD wAdr);
void vFlashWrite(WORD wAdr,BYTE bData[],WORD wDataLen);
void vFlashClearFF(WORD wAdr,WORD wDataLen);

#endif                                     
