//**********************************************************************+*********************************************
// yaUsbIR V3
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
             
#include "common.h"
#include "io.h"
#include "USB_Main.h"
#include "ir.h"
#include "flash.h"

extern WORD wWatchdogTimeout;

//**********************************************************************+*********************************************
// vSysclkInit
//
// Initialize the system clock and USB clock
//**********************************************************************+*********************************************
void vSysclkInit(void)
{
  CLKSEL  =  0x00;                                                      // Set SYSCLK to 12 / 8 MHz
  OSCICN &= ~0x03;
  CLKMUL  =  0x00;                                                      // Select internal oscillator as
                                                                        // input to clock multiplier
  CLKMUL |=  0x80;                                                      // Enable clock multiplier
                                                                        // delay about 5 us (8 clock)
  _nop_();  _nop_();  _nop_();  _nop_();
  _nop_();  _nop_();  _nop_();  _nop_();

  CLKMUL |=  0xC0;                                                      // Initialize the clock multiplier
  while(!(CLKMUL & 0x20));                                              // Wait for multiplier to lock

  FLSCL  |= 0x10;                                                       // set FLRT for 48MHz SYSCLK
  CLKSEL  = USB_4X_CLOCK | SYS_4X;                                      // Select system clock and USB clock
}

//**********************************************************************+*********************************************
// vPortInit
//
// This function configures the crossbar and GPIO ports.
//
// P0.0  -  Unassigned,  Push-Pull,  Digital,out,IR-Sender       , =1 -> IR-LED ein
// P0.1  -  Unassigned,  Open-Drain, Digital,in, IR-Empfaenger
// P0.2  -  Unassigned,  Open-Drain, Digital,NC
// P0.3  -  Unassigned,  Open-Drain, Digital,NC
// P0.4  -  Unassigned,  Open-Drain, Digital,out,Schaltoutput A
// P0.5  -  Unassigned,  Open-Drain, Digital,out,Schaltoutput B
// P0.6  -  Unassigned,  Open-Drain, Digital,out,Schaltoutput C
// P0.7  -  Unassigned,  Open-Drain, Digital,out,Schaltoutput D
//
// P1.0  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 8
// P1.1  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 7
// P1.2  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 6
// P1.3  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 5
// P1.4  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 4
// P1.5  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 3
// P1.6  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 2
// P1.7  -  Unassigned,  Open-Drain, Digital,in, extern Taster/Input 1
//
// P2.0  -  Unassigned,  Open-Drain, Digital,out,extern LED-gruen, =0 -> LED ein
// P2.1  -  Unassigned,  Open-Drain, Digital,out,LED-rot         , =0 -> LED ein
// P2.2  -  Unassigned,  Open-Drain, Digital,out,MB Switch       , =0 -> Switch überbrückt
// P2.3  -  Unassigned,  Open-Drain, Digital,NC
// P2.4  -  Unassigned,  Open-Drain, Digital,NC
// P2.5  -  Unassigned,  Open-Drain, Digital,NC
// P2.6  -  Unassigned,  Open-Drain, Digital,NC
// P2.7  -  Unassigned,  Open-Drain, Digital,in ,Lern-Taster
//
// P3.0  -  Unassigned,  Open-Drain, Digital,
//
//**********************************************************************+*********************************************
void vPortInit(void)
{
  // Portpin als Output dann PortBit=1
  P0MDOUT   = 0x01;
  P2MDOUT   = 0x00;
  XBR0      = 0x00;
  XBR1      = 0x40;// Enable Crossbar

  IRTX = 0;// Sende-LED aus
}

//**********************************************************************+*********************************************
// tUsbCmd.bCmd = C_IR
// tUsbCmd.bData[0] -> PosA
// tUsbCmd.bData[1] -> PosB
//
// bCmd   PosA  PosB 
// C_IR + C_7             + C_END -> Werkseinstellung
// C_IR + C_8 + C_0       + C_END -> rote Signal-LED aus
// C_IR + C_8 + C_1       + C_END -> rote Signal-LED ein (Werkseinstellung)
//**********************************************************************+*********************************************
void vIRConfig(void)
{
  EA = 0;

  switch (tUsbCmd.bData[0]) {
  case C_7:
    vWerkseinstellung();
    break;
  case C_8:
    vFlashClearFF((WORD)&(tFernb[0].bSignalLEDMode),1);
    vFlashWrite((WORD)&(tFernb[0].bSignalLEDMode),(BYTE*)&(tUsbCmd.bData[1]),1);
    break;
  }

  EA = 1;
}

//**********************************************************************+*********************************************
// tUsbCmd.bCmd = C_OUTPUT
// tUsbCmd.bData[0] -> PosA
// tUsbCmd.bData[1] -> PosB
//
// bCmd       PosA  PosB 
// C_OUTPUT + n1 +  C_0 + C_END -> Output n1 auf 0 (GND, low) setzen, n1 = C_1 bis C_4
// C_OUTPUT + n1 +  C_1 + C_END -> Output n1 auf 1 (open collector, high) setzen, n1 = C_1 bis C_4
//**********************************************************************+*********************************************
void vSetOutput(void)
{
  switch (tUsbCmd.bData[0]) {
  case C_1:
    OUT1 = tUsbCmd.bData[1];
    break;
  case C_2:
    OUT2 = tUsbCmd.bData[1];
    break;
  case C_3:
    OUT3 = tUsbCmd.bData[1];
    break;
  case C_4:
    OUT4 = tUsbCmd.bData[1];
    break;
  }
}

//**********************************************************************+*********************************************
// tUsbCmd.bCmd = C_WATCHDOG
// tUsbCmd.bData[0] -> PosA
// tUsbCmd.bData[1] -> PosB
// tUsbCmd.bData[2] -> PosC
// tUsbCmd.bData[3] -> PosD
//
// bCmd         PosA  PosB  PosC  PosD
// C_WATCHDOG + C_1 + n1  + n2  + n3 + C_END -> Watchdog retriggern und nach (n1*10000)+(n2*1000)+(n3*100) Millisekunden auslösen
// C_WATCHDOG + C_2 + n1  + n2  + n3 + C_END -> Zeit bis Mainboard über den Powerbutton ausschaltet
// C_WATCHDOG + C_3 + n1  + n2  + n3 + C_END -> Pausenzeit nach abschalten über Powerbutton
// C_WATCHDOG + C_4 + n1  + n2  + n3 + C_END -> Zeit zum Einschalten über Powerbutton
// C_WATCHDOG + C_0 + C_END -> Watchdog ausschalten
//
//**********************************************************************+*********************************************
void vSetWatchdog(void)
{
  WORD wTime = ((tUsbCmd.bData[1]*100)+(tUsbCmd.bData[2]*10)+tUsbCmd.bData[3]);
  wTime *= (100/10);

  EA = 0;
	
	switch (tUsbCmd.bData[0]) {
	case C_0:
    wWatchdogTimeout = 0;
		break;
	case C_1:
    wWatchdogTimeout = wTime + tFernb[0].wMainboardPowerOffTime + tFernb[0].wMainboardPowerOffPauseTime;
		break;
	case C_2:
		wWatchdogTimeout = 0;
    vFlashClearFF((WORD)&(tFernb[0].wMainboardPowerOffTime),sizeof(wTime));
    vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOffTime),(BYTE*)&(wTime),sizeof(wTime));
		break;
	case C_3:
		wWatchdogTimeout = 0;
    vFlashClearFF((WORD)&(tFernb[0].wMainboardPowerOffPauseTime),sizeof(wTime));
    vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOffPauseTime),(BYTE*)&(wTime),sizeof(wTime));
		break;
	case C_4:
		wWatchdogTimeout = 0;
    vFlashClearFF((WORD)&(tFernb[0].wMainboardPowerOnTime),sizeof(wTime));
    vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOnTime),(BYTE*)&(wTime),sizeof(wTime));
		break;
	}

  EA = 1;
}

//**********************************************************************+*********************************************
// tUsbCmd.bCmd = C_INPUT
// tUsbCmd.bData[0] -> PosA
// tUsbCmd.bData[1] -> PosB
//
// bCmd      PosA  PosB
// C_INPUT + C_0 + C_0 + C_END -> Inputs als Pegelinput löschen und alle Inputs als 8x Tastatur abfragen (default)
// C_INPUT + C_0 + C_1 + C_END -> Inputs als Pegelinput löschen und alle Inputs als 4x4 Tastaturmatrix abfragen, 16 Tasten
// C_INPUT + C_1 + n1  + C_END -> Input n1 (n1=C_1..C_8) als Pegelinput abfragen, Statusabfrage über "C_INPUT + C_3 + n1 + C_END"
// C_INPUT + C_2 + n1  + C_END -> Tastaurwiederholungszeit setzen, Zeit in Millisekunden = n1*100 (n1=C_1..C_9)
// C_INPUT + C_3 + n1  + C_END -> Pegel an Input n1 (n1=C_1..C_8) abfragen und als RC5-Code (IN_1_L bis IN_8_H) zurückliefern
//**********************************************************************+*********************************************
void vSetInput(void)
{
	BYTE bTemp = 1<<(tUsbCmd.bData[1]-C_1);
	
  EA = 0;

  switch (tUsbCmd.bData[0]) {
  case C_0:
		if (tFernb[0].bKeyBoardInputMask!=0xFF)
			vFlashClearFF((WORD)&(tFernb[0].bKeyBoardInputMask),1);// Inputs als Pegelinput
		if (tFernb[0].bKeyBoardMatrix!=tUsbCmd.bData[1]) {
			vFlashClearFF((WORD)&(tFernb[0].bKeyBoardMatrix),1);
			vFlashWrite((WORD)&(tFernb[0].bKeyBoardMatrix),(BYTE*)&(tUsbCmd.bData[1]),1);// Tastaturmatrix aus = 0 / an = 1
		}
    break;
  case C_1:
    if ((tUsbCmd.bData[1]>0)&&(tUsbCmd.bData[1]<9)) {
  		bTemp = tFernb[0].bKeyBoardInputMask & (~bTemp);
  		if (tFernb[0].bKeyBoardInputMask!=bTemp) {
        //vFlashClearFF((WORD)&(tFernb[0].bKeyBoardInputMask),1);// Tastaturmatrix aus
  			vFlashWrite((WORD)&(tFernb[0].bKeyBoardInputMask),(BYTE*)&(bTemp),sizeof(bTemp));// Dx=0 dann Pegelinput
  		}
    }
    break;
  case C_2:
    if (tUsbCmd.bData[1]>0) {
      tUsbCmd.bData[1] *= 10;
	  	if (tFernb[0].bKeyBoardRepeatTime!=tUsbCmd.bData[1]) {
		  	vFlashClearFF((WORD)&(tFernb[0].bKeyBoardRepeatTime),1);
			  vFlashWrite((WORD)&(tFernb[0].bKeyBoardRepeatTime),(BYTE*)&(tUsbCmd.bData[1]),1);// Tastaturwiederholung speichern
  		}
		}
    break;
  case C_3:
    EA = 1;
    if ((tUsbCmd.bData[1]>0)&&(tUsbCmd.bData[1]<9)) {
  		if ((~tFernb[0].bKeyBoardInputMask & bTemp)!=0)// -> Input ist als Pegelinput
  			vGenRC5(((ESWITCH & bTemp)?IN_1_H:IN_1_L) + ((tUsbCmd.bData[1]-C_1)*2));
    }
    break;
  }

  EA = 1;
}

//**********************************************************************+*********************************************
// Werkseinstellung
//**********************************************************************+*********************************************
void vWerkseinstellung(void)
{
  BYTE bTemp;
  code WORD wCONFIG[2] = {5*(1000/10),OUTPUT_ON_TIME};

  // Lernspeicher definiert löschen 
  for (bTemp=0;bTemp<(2*(IRMEM-1))+1;bTemp++) 
    vFlashErasePage((WORD)&(tFernb[bTemp]));

  vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOffTime),(BYTE*)&(wCONFIG[0]),2);// Zeit bis Mainboard über den Powerbutton ausschaltet
  vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOffPauseTime),(BYTE*)&(wCONFIG[0]),2);// Pausenzeit nach abschalten über Powerbutton
  vFlashWrite((WORD)&(tFernb[0].wMainboardPowerOnTime),(BYTE*)&(wCONFIG[1]),2);// Zeit zum Einschalten über Powerbutton
  bTemp = 0;// Matrix aus = 0
  vFlashWrite((WORD)&(tFernb[0].bKeyBoardMatrix),(BYTE*)&(bTemp),sizeof(bTemp));
  bTemp = (100/10);// 100ms Zeitabstand Tastaturwiederholung
  vFlashWrite((WORD)&(tFernb[0].bKeyBoardRepeatTime),(BYTE*)&(bTemp),sizeof(bTemp));
  bTemp = SIGNALLED_ON;
  vFlashWrite((WORD)&(tFernb[0].bSignalLEDMode),(BYTE*)&(bTemp),sizeof(bTemp));
}

