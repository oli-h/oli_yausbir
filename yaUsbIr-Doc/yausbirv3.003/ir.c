//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#include "string.h"
#include "common.h"
#include "USB_Main.h"
#include "USB_Configuration.h"
#include "ir.h"
#include "sio.h"
#include "io.h"
#include "flash.h"

WORD wPowerTimeout;
BYTE idata bRLedPowerTimeout = 0;
BOOL yIsSuspend = FALSE;

WORD wWatchdogTimeout;

//#define DUMP_PROZ // Abweichung % in den USB speichern

BYTE idata bRLedFlashCount = 0;

BYTE idata bSleep;
BYTE idata b10Ms;
WORD idata wPowerOutputTimeout;

BYTE idata bIrMode = 0xFF;// siehe IRMODE_xxx

// Vergleich gelernte Taste
BYTE xdata bIrPosA[IRMEM];
BYTE xdata bIrPosB[IRMEM];

// Kommandopuffer
BYTE bResponseCmd;
BYTE bResponseCmdData;

// IR-Sende/Empfangspuffer
BYTE bIrTxStep;//IR-TX Pollingraster in IRRX_F_POLL/bIrTxStep

BYTE bIrRingStart,bIrRingStartL;
BYTE bIrRingEnd;
WORD xdata wIrRing[MAX_PULSEDIS_IRFRAMESIZE];// Pulse/Pausen Daten, PULSE_BIT gibt an ob Puls oder Pause
WORD wIrTime;

BOOL yIrSig,yIrRxOld;
BOOL yIrRxSigTimeOverflow;

#ifdef DUMP_PROZ
  WORD wProz;//only4Debug
#endif

BYTE bIoSwitch;
BOOL ySwitch;

BYTE bSendRC5;

BYTE bInputs;
BYTE bInputsSend;
BYTE bInputPos;

BYTE bFrontSwitch = 0xFF;
BYTE bFrontSwitchRepeatTimeout = 0;
BYTE bFrontSwitchMatrixCount = 0;

BOOL yInternIr = FALSE;

BOOL yLedR;//LEDR

BYTE idata bLedPWMOff = 128;
BYTE idata bLedPWM;

#define FLASHTIME 15
//**********************************************************************+*********************************************
// (A*100)/B = %
//**********************************************************************+*********************************************
BYTE bAbweichung(WORD wA,WORD wB)
{
  #define MAXPROZVALUE (0xFFFF/100)// Max.Wert der 100 mal in WORD passt
  wA &= PULSE_MASK;
  wB &= PULSE_MASK;

  if ((wA<2)||(wB<2)) {// -> ungeultige Werte
    wB = 0;
  } else {

    while ((wA>MAXPROZVALUE)||(wB>MAXPROZVALUE)) {
      wA >>=1;//  /2
      wB >>=1;//  /2
    }

    if (wA==0) wA = 1;
    if (wB==0) wB = 1;

    if (wA>wB) {
      wB = (wB * 100) / wA;
    } else {
      wB = (wA * 100) / wB;
    }

    if (wB>100) wB = 100;
  }

  #ifdef DUMP_PROZ
    wProz = (BYTE) (100-wB);//only4Debug
    if (wA>wB) wProz |= 0x4000;//only4Debug Vorzeichen -%
  #endif
  return (BYTE) (100-wB);
}

//**********************************************************************+*********************************************
//
// This function configures the PCA time base, and sets up frequency output
// mode for Module 0 (CEX0 pin).
//
// The frequency generated at the CEX0 pin is equal to CEX0_FREQUENCY Hz,
// which is defined in the "Global Constants" section at the beginning of the
// file.
//
// The PCA time base in this example is configured to use SYSCLK / 12.
// The frequency range that can be generated using this example is ~2 kHz to
// ~500 kHz when the processor clock is 12 MHz.  Using different PCA clock
// sources or a different processor clock will generate different frequency
// ranges.
//
//**********************************************************************+*********************************************
//    How "Frequency Output Mode" Works:
//
//       The PCA's Frequency Output Mode works by toggling an output pin every
//    "N" PCA clock cycles.  The value of "N" should be loaded into the PCA0CPH
//    register for the module being used (in this case, module 0).  Whenever
//    the register PCA0L (PCA counter low byte) and the module's PCA0CPL value
//    are equivalent, two things happen:
//
//    1) The port pin associated with the PCA module toggles logic state
//    2) The value stored in PCA0CPH is added to PCA0CPL.
//
//    Using this mode, a square wave is produced at the output port pin of the
//    PCA module. The speed of the waveform can be changed by changing the
//    value of the PCA0CPH register.
//
//**********************************************************************+*********************************************
//    Configure frequency for CEX0
//    PCA0CPH0 = (SYSCLK/12)/(2*CEX0_FREQUENCY), where:
//    SYSCLK/12 = PCA time base
//    CEX0_FREQUENCY = desired frequency
//
//    PCA0CPH0 = (SYSCLK/12)/(2*CEX0_FREQUENCY);
//
//    CEX0_FREQUENCY = (SYSCLK/12)/(2*PCA0CPH0);
//
//
//
//    freq = (48000000/4) / (1*2)   = 6 MHz
//    freq = (48000000/4) / (255*2) = 23,529 KHz
//    freq = (48000000/1) / (1*2)   = 24 MHz
//    freq = (48000000/1) / (255*2) = 94,117 KHz
//
// 455KHz:
//
//    freq = (48000000/1) / (52*2)  =  461538,46 Hz  +6538,46
//    freq = (48000000/1) / (53*2)  =  452830,18 Hz  -2169,82
//
// 38KHz:
//
//    freq = (48000000/4) / (157*2)  =  38216,56 Hz  +216,56
//    freq = (48000000/4) / (158*2)  =  37974,68 Hz  -25,32
//
// xKHz
// 
//    freq = (48000000/4) / (170*2)  =  35294,11 Hz  
//    freq = (48000000/4) / (150*2)  =  40000 Hz
//
//
//**********************************************************************+*********************************************
void vSetIrRxUsStep(BYTE b)
{
#ifdef WITH_455KHZ
  CR = 0;// Stop PCA counter
  if (bIrMode==IRMODE_TX) {

    // Configure PCA time base; overflow interrupt disabled
    PCA0CN = 0x00; // Stop counter; clear all flags

    if (b==IRRX_STEP_455KHZ)// -> 455KHz
      PCA0MD = 0x08; // Use SYSCLK/1 as time base
    else
      PCA0MD = 0x02; // Use SYSCLK/4 as time base

    PCA0CPM0 = 0x46; // Module 0 = Frequency Output mode

    // Configure frequency for CEX0
    // PCA0CPH0 = (SYSCLK/1)/(2*wKhz*1000), where:
    // SYSCLK/1 = PCA time base
    // CEX0_FREQUENCY = desired frequency
    // PCA0CPH0 = (SYSCLK/1)/(2*CEX0_FREQUENCY);

    if (b==IRRX_STEP_455KHZ)// -> 455KHz
      PCA0CPH0 = 53;//(SYSCLK/1)/(2*wHz);
    else
      PCA0CPH0 = b*2;//(SYSCLK/8)/wHz;//(SYSCLK/4)/(2*wHz); (48000000/8)/38000 = 157
  }
#endif

  TH1 = -(b*2);// reload value in TH1  
}

//**********************************************************************+*********************************************
//         SYSCLK /4
//           TH1=
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 85;// 35,21 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 84;// 35,71 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 83;// 36,24 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 82;// 36,50 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 81;// 37,04 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 80;// 37,50 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 79;// 37,88 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 78;// 38,46 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 77;// 39,06 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 76;// 39,38 Khz
//      ya_usbir_txbuf[ya_usbir_txbufpos++] = 75;// 40Khz

// 32KHz     93,75
// 33        90,09
// 34        88,235
// 35        85,714
// 36        83,333
// 36,5      82,191
// 37        81,081
// 37,5      80
// 38        78,94
// 38,5      77,92
// 39
// 39,5
// 40        75
// 40,5
// 41        73,170
// 41,5      72,289
// 42KHz
// 
// Timer1 als 8Bit mit reload
// SYSCLK = 48 MHz
// 1us bei 1MHz
//
// 1/38KHz = 26,31579us
//
// SYSCLK /1 -> 20,83ns - 5,3125us
// SYSCLK /4 -> 83,33ns - 21,25us
//
// 6,57895us = 1/4 38KHz
//**********************************************************************+*********************************************
void vInitIr(BYTE bIrMod)
{
  BYTE bIrPosIndex;

  if ((bIrMod==IRMODE_TX) && (bIrMode==IRMODE_TX))
    return;// ist schon im Sendemodus!

  TR1 = 0;// Stop Timer1
  bIrMode = bIrMod;

  for (bIrPosIndex=0;bIrPosIndex<IRMEM;bIrPosIndex++) {
    bIrPosA[bIrPosIndex] = 0;
    bIrPosB[bIrPosIndex] = 0;
  }
  bResponseCmd = CMD_NONE;
  bResponseCmdData = CMD_NONE;

  if (bIrMode==IRMODE_LEARN_LIRC) {
    bIrRingStart = 0;
  } else {
    bIrRingStart = MAX_PULSEDIS_IRFRAMESIZE-1;
  }

  bIrRingStartL = bIrRingStart;
  bIrRingEnd = bIrRingStart;
  yIrRxSigTimeOverflow = TRUE;
  yIrRxOld = TRUE;

  memset(wIrRing,IRRX_NODATA,sizeof(wIrRing));
  wIrTime = 0;

  TMOD |= 0x20;// Timer1 Mode 2: 8-bit timer with auto-reload
  TL1 = 1;

  CKCON &= ~0x08;
  CKCON |= 0x01;// Timer1 System clock divided by 4 (48MHz /4), timer1-clock = 12Mhz

  if (bIrMode==IRMODE_TX) { // -> Frame-pause in den Sendepuffer legen
    vSetIrRxUsStep(bIrTxStep);
    yIrTxByte(IRTX_FRAME_REPEAT_PAUSE_TIME);
  } else {
    vSetIrRxUsStep(IRRX_STEP);
  }

  TR1 = 1;// Start Timer1
  ET1 = 1;// Enable interrupt requests generated by the TF1 flag
}

//**********************************************************************+*********************************************
// Sleeptimer, jede 1ms
//**********************************************************************+*********************************************
void vInitTimer0(void)
{
  TR0 = 0;// Stop Timer0
  CKCON &= ~0x04;
  CKCON |= 0x01;// Timer0 System clock divided by 4 (48MHz /4), timer0-clock = 12Mhz
  wPowerOutputTimeout = 0;
  wWatchdogTimeout = 0;
  wPowerTimeout = POWER_TIMEOUT;
  TMOD |= 0x01;// Timer0 Mode 1: 16-bit timer
  TR0 = 1;// Start Timer0
  ET0 = 1;// Enable interrupt requests generated by the TF0 flag
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
BOOL yGetMenuSwitch(void)
{
  static BOOL ySwitchOld = 1;

  if ((ySwitch==1)&&(ySwitchOld==0)) {//Taste losgelassen
    ySwitchOld = 1;
    return TRUE;
  }
  ySwitchOld = ySwitch;
  return FALSE;
}

//**********************************************************************+*********************************************
// wird aufgerufen bei Timer 0 overflow
// jede 1ms
//**********************************************************************+*********************************************
void vTimer0Isr(void) interrupt 1
{
  TR0 = 0;// Stop Timer0
  TH0 = 0xD1;// 0xFFFF-(SYSCLK/4/1000) = 0xFFFF - 12000 = D11F
  TL0 = 0x1F;
  TR0 = 1;// Start Timer0

  if (bSleep>0)// vSleep Zaehler
    bSleep--;

  if (SWITCH==0) {
    if (bIoSwitch>0) {
      bIoSwitch--;
      if (bIoSwitch==0) ySwitch = FALSE;
    }
  } else {
    if (bIoSwitch<20) {
      bIoSwitch++;
      if (bIoSwitch==20) ySwitch = TRUE;
    }
  }

  if (b10Ms<10) {// 10ms Zaehler
    b10Ms++;
  } else {
    // 10ms vergangen
    b10Ms = 0;
    if (wPowerOutputTimeout>0) {
      wPowerOutputTimeout--;
      if (wPowerOutputTimeout>0) {
        yLedR = FALSE;// LED rot aus
        MBSWITCH = 0;// = Powerswitch Taste des MBs gedrückt 
      } else {
        yLedR = TRUE;// LED rot an
        MBSWITCH = 1;// = Powerswitch Taste des MBs nicht gedrückt
      }
    }
		
		if (tFernb[0].bKeyBoardMatrix!=0) {// -> Inputs als 4x4 Tastaturmatrix abfragen, 16 Tasten
			if (bFrontSwitchRepeatTimeout>0) {
				bFrontSwitchRepeatTimeout--;
				if (bFrontSwitchRepeatTimeout==0) 
					bFrontSwitch = 0xFF;
			}

			bInputs = 0xFF;
			switch (ESWITCH & 0xF0) {
			case ~0x1F:
  			bInputs = bSendRC5;
				break;
			case ~0x2F:
				bInputs = bSendRC5+1;
				break;
			case ~0x4F:
				bInputs = bSendRC5+2;
				break;
			case ~0x8F:
  			bInputs = bSendRC5+3;
				break;
			}
			
			if (bInputs!=0xFF) {
        if (bInputs!=bFrontSwitch) {
  				bFrontSwitch = bInputs;
  				bFrontSwitchRepeatTimeout = tFernb[0].bKeyBoardRepeatTime;
  				vGenRC5(bFrontSwitch);
  				if (wPowerOutputTimeout==0)
  					bRLedFlashCount = FLASHTIME + (bRLedFlashCount % 4);

  				if (bIrMode==IRMODE_LEARN) {
  					bIrMode = IRMODE_LEARN_FRAMEEND;
  					yInternIr = TRUE;
  				}
				}

        bFrontSwitchMatrixCount = 5;
			}
      if ((bInputs==0xFF)&&(bFrontSwitchMatrixCount>0)) {
        bFrontSwitchMatrixCount--;
				if ((bFrontSwitchMatrixCount==0)&&(bFrontSwitchRepeatTimeout>0))
					bFrontSwitchRepeatTimeout = 1;
      } 
			
			bInputPos <<= 1;
			bSendRC5 += 4;
			if ((bInputPos!=0x01)&&(bInputPos!=0x02)&&(bInputPos!=0x04)&&(bInputPos!=0x08)) {
				bInputPos = 0x01;
				bSendRC5 = KEY_F1;
			}
			ESWITCH = ~bInputPos;
		} else {// -> Inputs als Tastatur oder normale Inputs abfragen
			if (bFrontSwitchRepeatTimeout>0) {
				bFrontSwitchRepeatTimeout--;
				if (bFrontSwitchRepeatTimeout==0) 
					bFrontSwitch = tFernb[0].bKeyBoardInputMask;
			}

			if (bFrontSwitch!=(ESWITCH | ~tFernb[0].bKeyBoardInputMask)) {
				bFrontSwitch = (ESWITCH | ~tFernb[0].bKeyBoardInputMask);
				bSendRC5 = 0xFF;
				switch (bFrontSwitch) {
				case ~0x01: bSendRC5 = 0; break;
				case ~0x02: bSendRC5 = 1; break;
				case ~0x04: bSendRC5 = 2; break;
				case ~0x08: bSendRC5 = 3; break;
				case ~0x10: bSendRC5 = 4; break;
				case ~0x20: bSendRC5 = 5; break;
				case ~0x40: bSendRC5 = 6; break;
				case ~0x80: bSendRC5 = 7; break;
				}      
				if (bSendRC5!=0xFF) {
					bFrontSwitchRepeatTimeout = tFernb[0].bKeyBoardRepeatTime;
					vGenRC5(bSendRC5);
					if (wPowerOutputTimeout==0)
						bRLedFlashCount = FLASHTIME + (bRLedFlashCount % 4);

					if (bIrMode==IRMODE_LEARN) {
						bIrMode = IRMODE_LEARN_FRAMEEND;
						yInternIr = TRUE;
					}
				} else {
					bFrontSwitchRepeatTimeout = 0; 
				}
			} else {
				bInputs = (ESWITCH & (~tFernb[0].bKeyBoardInputMask));
				if (bInputsSend != bInputs) {
					bSendRC5 = IN_1_L;
					bInputPos = 1;
					while (TRUE) {
						if ((bInputs & bInputPos)!=(bInputs & bInputPos)) {
							bInputs &= ~bInputPos;
							bInputs |= (bInputs & bInputPos);
							vGenRC5(((bInputs & bInputPos)!=0)?(bSendRC5+1):bSendRC5);// IN_n_L oder IN_n_H senden
							if (bIrMode==IRMODE_LEARN) {
								bIrMode = IRMODE_LEARN_FRAMEEND;
								yInternIr = TRUE;
							}
							break;
						}
						if (bInputPos==0x80) {//-> bInputsSend ist Fehlerhaft
							bInputsSend = bInputs;
							break;
						}
						bInputPos <<= 1; 
						bSendRC5 += 2;
					}
				}
			}	
		}

    if (bRLedFlashCount>0) {
      bRLedFlashCount--;
      yLedR = ((bRLedFlashCount % 4)>2)?TRUE:FALSE;
      if (bRLedFlashCount==0)
        yLedR = TRUE;// LED rot an
    } 

    if (wWatchdogTimeout>0) {
      wWatchdogTimeout--;
      if (wWatchdogTimeout==(tFernb[0].wMainboardPowerOffTime+tFernb[0].wMainboardPowerOffPauseTime)) {
        wPowerOutputTimeout = tFernb[0].wMainboardPowerOffTime;
      }      
      if (wWatchdogTimeout==0) {
        wPowerOutputTimeout = tFernb[0].wMainboardPowerOnTime;
      }      
    }

    if (yIsSuspend) {
      wPowerTimeout = POWER_TIMEOUT;

      switch (bRLedPowerTimeout) {
      case 0:
        if (bLedPWMOff>0)
          bLedPWMOff--;
        else
          bRLedPowerTimeout++;
        break;
      case 1:
        if (bLedPWMOff<255)
          bLedPWMOff++;
        else
          bRLedPowerTimeout++;
        break;
      default:
        bRLedPowerTimeout++;
        break;
      }
      /*
      if (bRLedPowerTimeout>0) {
        bRLedPowerTimeout--;
      } else {
        bRLedPowerTimeout = (1000/10);
        yLedR = ~yLedR;
      }
      */
    } else {
      if (wPowerTimeout>0) {
        wPowerTimeout--;
        if (wPowerTimeout==0)
          yLedR = TRUE;// LED rot an
      }
    }
  }

  LEDEX = yLedR;// externe LED gleich der internen LED setzen
  if (!yIsSuspend || (bRLedFlashCount!=0)) {
    if (tFernb[0].bSignalLEDMode==SIGNALLED_ON) 
      LEDR = ~yLedR;
    else 
      LEDR = 1;// LED aus
  }
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
void vSleep(BYTE b)
{
  bSleep = b;
  while (bSleep>0);
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
void vSleepSec(BYTE b)
{
  while (b-->0) {
    vSleep(250);vSleep(250);vSleep(250);vSleep(250);
  }
}

//**********************************************************************+*********************************************
// wird aufgerufen bei Timer 1 overflow
//
// Pulsweitenmessung:
// Am IRRX liegt der IR-Sensor an. Die Pulsweite (bzw.Pulspause) wird ueber den Timer1 gemessen.
// Timer1 zaehlt zwischen zwei Flankenwechsel die Signalweite.
//**********************************************************************+*********************************************
void vTimer1Isr(void) interrupt 3
{
  switch (bIrMode) {
  case IRMODE_TX:// Modus: IR-Signal senden
    if (wIrTime>0) {
      wIrTime--;
#ifndef WITH_455KHZ
      if (yIrSig) {//Pulse
        IRTX = !IRTX;// Sende-LED toggeln
      }
#endif// WITH_455KHZ
    } else {
#ifdef WITH_455KHZ
      PCA0CN = 0x00; // Stop PCA counter; clear all flags; CR = 0;
      PCA0L = 0xFF;
      PCA0H = 0xFF;
      XBR1 = 0x40; // dont Route CEX0 to P0.0
#endif// WITH_455KHZ
      IRTX = 0;// Sende-LED aus

      if (bIrRingStart==bIrRingEnd) {// -> Puffer leer
        bIrMode = IRMODE_RX;// umschalten auf empfangen
        vSetIrRxUsStep(IRRX_STEP);
      } else {
        bRLedFlashCount = FLASHTIME + (bRLedFlashCount % 4);
        wIrTime = wIrRing[bIrRingStart];
        wIrRing[bIrRingStart] = 0;
        if ((wIrTime&PULSE_BIT)==PULSE_BIT) {// -> Pulse
          wIrTime &= PULSE_MASK;
#ifdef WITH_455KHZ
          CR = 1;// Start PCA counter
          XBR1 = 0x41; // Route CEX0 to P0.0
#endif
          yIrSig = TRUE;//Pulse
        } else {
          yIrSig = FALSE;//Pause
        }

        bIrRingStart++;
        if (bIrRingStart>=MAX_PULSEDIS_IRFRAMESIZE)
          bIrRingStart = 0;
      }
    }
    break;

  case IRMODE_RX:// Modus: IR-Signal empfangen
  case IRMODE_LEARN:// Modus: IR-Signal empfangen zum lernen
  case IRMODE_LEARN_FRAMEEND:
#ifdef WITH_455KHZ
    XBR1 = 0x40;
#endif
    IRTX = 0;// Sende-LED aus
    yIrSig = IRRX;
    if (yIrSig!=yIrRxOld) { // -> Signalaenderung
      if (wPowerOutputTimeout==0)
        //bGLedFlashCount = FLASHTIME + (bGLedFlashCount % 4);
        bRLedFlashCount = FLASHTIME + (bRLedFlashCount % 4);

      if ((yIrRxSigTimeOverflow==FALSE)||
          (bIrMode==IRMODE_RX))            {// -> gueltiges Signal
        if (bIrMode!=IRMODE_LEARN_FRAMEEND) {// -> normaler Empfangsmodus
          bIrRingEnd++;
          if (bIrRingEnd>=MAX_PULSEDIS_IRFRAMESIZE)
            bIrRingEnd = 0;

          if (yIrRxOld==FALSE)
            wIrTime |= PULSE_BIT;

          wIrRing[bIrRingEnd] = wIrTime;

          if ((bIrMode==IRMODE_LEARN)&&(bIrRingEnd==MAX_PULSEDIS_IRFRAMESIZE-1))
            bIrMode = IRMODE_LEARN_FRAMEEND;
        }
      }

      yIrRxSigTimeOverflow = FALSE;
      wIrTime = IRRX_NODATA;
      yIrRxOld = yIrSig;
    }

    if (wIrTime<PULSE_MASK)
      wIrTime++;

    if (wIrTime>=IRRX_TIMEOUT_TIME) {
      if (yIrRxSigTimeOverflow==FALSE)
        yIrRxSigTimeOverflow = TRUE;

      if ((bIrMode==IRMODE_LEARN)&&(bIrRingEnd!=MAX_PULSEDIS_IRFRAMESIZE-1))// -> min. ein Signal vorhanden
        bIrMode = IRMODE_LEARN_FRAMEEND;
    }
    break;
  default:
    break;
  }

  if (yIsSuspend && (bRLedFlashCount==0)) {
    bLedPWM++;
    if (bLedPWM>64) bLedPWM++;
    if (bLedPWM>128) bLedPWM += 2;
    if (bLedPWM>196) bLedPWM += 4;
    LEDEX = (bLedPWM>bLedPWMOff)?FALSE:TRUE;
    if (tFernb[0].bSignalLEDMode==SIGNALLED_ON) 
      LEDR = LEDEX;// interne LED gleich der externen LED setzen
  }
}

//**********************************************************************+*********************************************
// FALSE zurueckgeben wenn mindestens noch zwei Signalwerte in den Puffer gespeichert werden koennen
//**********************************************************************+*********************************************
BOOL yIrTxBufFull(void)
{
  BYTE bFrei;
  // Ende > Start
  //  Frei = Pufferlaenge - (Ende - Start) -1
  // Start > Ende
  //  Frei = Pufferlaenge - ((Pufferlaenge - Start) + Ende +1)

  EA = 0;
  if (bIrRingStart==bIrRingEnd) {
    EA = 1;
    return FALSE;
  }
  if (bIrRingStart>bIrRingEnd)
    bFrei = MAX_PULSEDIS_IRFRAMESIZE - 1 - (bIrRingEnd-bIrRingStart);
  else
    bFrei = MAX_PULSEDIS_IRFRAMESIZE - ((MAX_PULSEDIS_IRFRAMESIZE-bIrRingStart) + bIrRingEnd + 1);
  EA = 1;

  return bFrei<2?TRUE:FALSE;
}

//**********************************************************************+*********************************************
// ein IR Puls/Pause Signal in den Sendepuffer legen
//**********************************************************************+*********************************************
BOOL yIrTxByte(WORD w)
{
  if (yIrTxBufFull()) return FALSE;
  if ((w & PULSE_BIT)==PULSE_BIT) {
    if ((w & PULSE_MASK)> IRTX_MAXPULSE_TIME)// -> zum Schutz der Sendedioden den Wert begrenzen!
      w = IRTX_MAXPULSE_TIME | PULSE_BIT; 
  }
  wIrRing[bIrRingEnd] = w;
  EA = 0;
  bIrRingEnd++;
  if (bIrRingEnd>=MAX_PULSEDIS_IRFRAMESIZE)
    bIrRingEnd = 0;
  EA = 1;
  return TRUE;
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
void vIrSearch(void)
{
  BYTE bIrPosIndex,bIrPosIndex2;
  WORD wSig,wSig2;
  BYTE bFoundFrame = 0xFF;
  BYTE bRep;

  if (bIrMode!=IRMODE_RX) return;

  if (bIrRingStartL==bIrRingEnd) return;
  bIrRingStartL++;
  if (bIrRingStartL>=MAX_PULSEDIS_IRFRAMESIZE)
    bIrRingStartL = 0;

  wSig = wIrRing[bIrRingStartL];

  // --- gelernte Tasten erkennen ---
  if ((wSig & PULSE_MASK)>=IRRX_TIMEOUT_TIME) {
    for (bIrPosIndex=0;bIrPosIndex<IRMEM;bIrPosIndex++) {
      bIrPosA[bIrPosIndex] = 0;
      bIrPosB[bIrPosIndex] = 0;
    }
  }

  for (bIrPosIndex=0;bIrPosIndex<IRMEM;bIrPosIndex++) {
    bIrPosIndex2 = bIrPosIndex<<1;
    if (tFernb[bIrPosIndex2].bIrCode==IRCODE_ERROR) continue;

    // Pause zwischen den Frames bei einer gelernten und gefundene Taste testen
    wSig2 = tFernb[bIrPosIndex2].tIr.wPulseDis[bIrPosA[bIrPosIndex]];
    // gelernte IR-Power-Taste erkennen
    if ((bAbweichung(wSig,wSig2)<IRRX_DIFF) && // -> mehr als 78% Signalweitenuebereinstimmung
        ((wSig & PULSE_BIT)==(wSig2 & PULSE_BIT)) ) {// -> und gleicher Puls/Pause
      // naechste Puls/Pause gefunden
      (bIrPosA[bIrPosIndex])++;
      if (bIrPosA[bIrPosIndex]>=((tFernb[bIrPosIndex2].yBiPhase==TRUE)?MAX_BIPHASE_IRFRAMESIZE:MAX_PULSEDIS_IRFRAMESIZE))// -> Frame groesser als gelerntes
        bIrPosA[bIrPosIndex] = 0;
      // Pause zwischen den Frames bei einer gelernten und gefundene Taste testen
      if (tFernb[bIrPosIndex2].tIr.wPulseDis[bIrPosA[bIrPosIndex]]==IRRX_ENDFRAME) { // -> gelernte Taste gefunden
        bFoundFrame = bIrPosIndex2;
        break;
      }
    } else {
      bIrPosA[bIrPosIndex] = 0;
    }

    // evtl. als BiPhase suchen
    if (tFernb[bIrPosIndex2].yBiPhase==TRUE) {
      wSig2 = tFernb[bIrPosIndex2].tIr.wBiPhase[1][bIrPosB[bIrPosIndex]];
      // gelernte IR-Power-Taste erkennen
      if ((bAbweichung(wSig,wSig2)<IRRX_DIFF) &&// -> mehr als 78% Signalweitenuebereinstimmung
          ((wSig & PULSE_BIT)==(wSig2 & PULSE_BIT)) ) {// -> und gleicher Puls/Pause
        // naechste Puls/Pause gefunden
        (bIrPosB[bIrPosIndex])++;
        if (bIrPosB[bIrPosIndex]>=MAX_BIPHASE_IRFRAMESIZE)// -> Frame groesser als gelerntes
          bIrPosB[bIrPosIndex] = 0;
        // Pause zwischen den Frames bei einer gelernten und gefundene Taste testen
        if (tFernb[bIrPosIndex2].tIr.wBiPhase[1][bIrPosB[bIrPosIndex]]==IRRX_ENDFRAME) { // -> gelernte Taste gefunden
          bFoundFrame = bIrPosIndex2;
          break;
        }
      } else {
        bIrPosB[bIrPosIndex] = 0;
      }
    }
  }

  if (bFoundFrame!=0xFF) {// -> Taste gefunden
    if (bFoundFrame==(IRMEM-1)*2) {
      wPowerOutputTimeout = OUTPUT_ON_TIME;// Output fuer 800ms einschalten
    } else {

      if (tFernb[bIrPosIndex2].bOption & IR_OPTION_SENDINDARK)
        // warten bis kein IR-Signal mehr empfangen wurde, erst dann senden
        while (yIrRxSigTimeOverflow==FALSE);

      bRep = tFernb[bIrPosIndex2].bRepeat;// Anzahl Wiederholungen    
      bIrTxStep = IRRX_STEP;

      bIrPosIndex2++;// Index auf Sendespeicher
      while (bRep>0) {
        bRep--;
        bIrPosIndex = 0;
        while (bIrPosIndex!=0xFF) {
          wSig = tFernb[bIrPosIndex2].tIr.wPulseDis[bIrPosIndex++];
          if (wSig==IRRX_ENDFRAME) {
            wSig = tFernb[bIrPosIndex2].tIr.wPulseDis[bIrPosIndex];//Frame-Pause hinzufuegen
            bIrPosIndex = 0xFF;
          }
          vInitIr(IRMODE_TX);// evtl. Umschalten auf senden
          while (!yIrTxByte(wSig));
        }
        while (bIrMode==IRMODE_TX);// warten bis senden beendet ist
      }
    }
    for (bIrPosIndex=0;bIrPosIndex<IRMEM;bIrPosIndex++) {
      bIrPosA[bIrPosIndex] = 0;
      bIrPosB[bIrPosIndex] = 0;
    }
  }
}

//**********************************************************************+*********************************************
// IR Puls/Pause Signal zurueckgeben
//**********************************************************************+*********************************************
BOOL yGetIrRxRing(WORD *pwSig)
{
  if (bIrRingStart==bIrRingEnd) return FALSE;
  bIrRingStart++;
  if (bIrRingStart>=MAX_PULSEDIS_IRFRAMESIZE)
    bIrRingStart = 0;

  *pwSig = wIrRing[bIrRingStart];

  return TRUE;
}

//**********************************************************************+*********************************************
// Daten zum Computer ueber USB senden
//**********************************************************************+*********************************************
BOOL yPacketToHost(BYTE yOnlyTest4NewPacket)
{
  tWORD w;
  BYTE b;
  BYTE bTemp2;
  BOOL yOk;

  if (yOnlyTest4NewPacket==TRUE) {
    if (bResponseCmd!=CMD_NONE) return TRUE;
    if ((bIrMode==IRMODE_RX)&&(bIrRingStart!=bIrRingEnd)) {
      //vSleep(255);//only4Debug
      return TRUE;// IR-Daten vorhanden
    }
    if (bGetRxBufLen()>0) return TRUE;// COM-Daten vorhanden
    return FALSE;
  }

  bInPacket[0] = CMD_NONE;

  switch (bResponseCmd) {
  case CMD_GETCOMBAUD:// COM-Baud zuruecksenden
    bInPacket[0] = CMD_GETCOMBAUD;
    // MSB-LSB
    bInPacket[1] = zBaud.b[0];
    bInPacket[2] = zBaud.b[1];
    bInPacket[3] = zBaud.b[2];
    bInPacket[4] = zBaud.b[3];
    break;
  default:
    if (bIrMode==IRMODE_RX) {
      // evtl. IR-Signal zum Computer senden
      yOk = yGetIrRxRing(&w.w);
      if (yOk) {// -> IR-Sig im Ringpuffer
        bInPacket[0] = CMD_IRDATA;
        bInPacket[1] = IRRX_STEP/6;//bIrRxUsStep;
        for (b=2;b<EP1_PACKET_SIZE;b++) {
          bInPacket[b++] = w.b[MSB];//MSB
          bInPacket[b] = w.b[LSB];//LSB
          if (yOk) {
            yOk = yGetIrRxRing(&w.w);
            if (!yOk) w.w = 0;// den Rest des Paketes loeschen
          }
        }
      } else {
        // evtl. COM-Daten zum Computer senden
        yOk = yRxByte(&bTemp2);
        if (yOk) {// -> RX-Byte im Ringpuffer
          bInPacket[0] = CMD_COMDATA;
          bInPacket[1] = 0;
          for (b=2;b<EP1_PACKET_SIZE;b++) {
            if (yOk) {
              bInPacket[b] = bTemp2;
              yOk = yRxByte(&bTemp2);
              if (!yOk)// -> letztes Zeichen
                bInPacket[1] = b-1;// Datenlaenge setzen
            } else {
              bInPacket[b] = 0;// den Rest des Paketes loeschen
            }
          }
        }
      }
    }
    break;
  }

  bResponseCmd = CMD_NONE;
  return bInPacket[0]!=CMD_NONE;
}

//**********************************************************************+*********************************************
// Daten vom Computer ueber USB empfangen
//**********************************************************************+*********************************************
void vPacketFromHost(BYTE yLircLearn)
{
  tWORD w;
  BYTE b;
  bResponseCmd = 0;
  switch (bOutPacket[0]) {
  //case CMD_NONE:// Kein Kommando
  //  break;
  case CMD_IRDATA:// IR-Daten senden
    bIrTxStep = bOutPacket[1];// IRRX_F_POLLstel
    if (bIrTxStep<50) 
      bIrTxStep = IRRX_STEP;

    if (yLircLearn) {
      for (b=2;b<EP1_PACKET_SIZE;b++) {
        w.b[MSB] = bOutPacket[b];
        b++;
        w.b[LSB] = bOutPacket[b];
        if (w.w>0) {// IR-Daten zum Lernen vorhanden
          yIrTxByte(w.w);
        } else {
          break;
        }
      }
      break;
    }

    if (bOutPacket[2]==CONTROLCMD) {//
      switch (bOutPacket[3]) {
      case C_END:
        // CMD in main() ausführen
        tUsbCmd.bCmd |= 0x80;
        break;
      case C_WATCHDOG:
      case C_OUTPUT:
      case C_INPUT:
      case C_IR:
        memset(&tUsbCmd,0,sizeof(tUsbCmd));
        tUsbCmd.bCmd = bOutPacket[3];
        break;
      default:
        if (tUsbCmd.bPos<sizeof(tUsbCmd.bData)) {
          tUsbCmd.bData[tUsbCmd.bPos] = bOutPacket[3];
          tUsbCmd.bPos++;
        }
        break;
      }
      break;
    }

    for (b=2;b<EP1_PACKET_SIZE;b++) {
      w.b[MSB] = bOutPacket[b];
      b++;
      w.b[LSB] = bOutPacket[b];
      if (w.w>0) {// IR-Daten zum Senden vorhanden
        vInitIr(IRMODE_TX);// evtl. Umschalten auf senden
        while (!yIrTxByte(w.w));
      } else {
        break;
      }
    }
    break;
  case CMD_COMDATA:// COM-Daten senden
    for (b=0;b<bOutPacket[1];b++) {
      while (yTxBufFull()) vSleep(1);
      yTxByte(bOutPacket[b+2]);
    }
    break;
  case CMD_SETCOMBAUD:// COM-Baud setzen
    zBaud.b[0] = bOutPacket[1];
    zBaud.b[1] = bOutPacket[2];
    zBaud.b[2] = bOutPacket[3];
    zBaud.b[3] = bOutPacket[4];
    vSetComBaud(zBaud.z);
    break;
  case CMD_GETCOMBAUD:// COM-Baud zuruecksenden
    // Cmd wird in yPacketToHost() verarbeitet
    bResponseCmd = bOutPacket[0];
    bResponseCmdData = bOutPacket[1];
    break;
  }
}

//**********************************************************************+*********************************************
//
//**********************************************************************+*********************************************
void vLED(BYTE bBlink)
{
  BOOL yLED;
  BOOL yLongBlink = (bBlink & LONGBLINK)>0?TRUE:FALSE;
  //BOOL yErrorBlink = (bBlink & ERRORBLINK)>0?TRUE:FALSE;
  bBlink &= ~(LONGBLINK | ERRORBLINK);

  while (bRLedFlashCount>0);

  bBlink <<= 1;
  yLED = 1;
  yLedR = yLED;
  vSleep(200);
  while (bBlink>0) {
    bBlink--;
    yLED = ~yLED;
    yLedR = yLED;

    if (bBlink>0) {
      vSleep(200);
      if (yLongBlink) {
        vSleep(250);
        vSleep(250);
        vSleep(250);
      }
    }
  }
  yLedR = TRUE;// LED an
}

//**********************************************************************+*********************************************
// Ein IR-Frame einer Fernbedienungstaste ermitteln
//**********************************************************************+*********************************************
BYTE bLearnIrCode(BYTE *pyBiPhase,BYTE yIRDataFromLirc)
{
  #define CALCMASK  0x3FFF
  #define CALC1     0x8000
  #define CALC2     0x4000
  BYTE bG;
  BYTE b;
  WORD wSig,wSig2,wFramePause;
  BOOL yIrCodeOk;
  BOOL yBiPhase;
  BYTE bIrCode = IRCODE_UNKNOWN;
  DWORD zSigD;
  BYTE bSigCount,n;

  yInternIr = FALSE;
  if (yIRDataFromLirc==FALSE) 
    vInitIr(IRMODE_LEARN);
  else
    vInitIr(IRMODE_LEARN_LIRC);
  yBiPhase = FALSE;

  yIrCodeOk = FALSE;

  // auf IR-Frame warten
  b = (10000/50);// max 10sec auf IR-Frame warten
  while ((b>0)&&((bIrMode==IRMODE_LEARN)||yIRDataFromLirc)) {
    vSleep(50);
    b--;

    if (yIRDataFromLirc) {
      vDoUsbOut(TRUE);
      if (bIrRingStart!=bIrRingEnd) {
        yIrCodeOk = TRUE;
        yIrTxByte(IRRX_ENDFRAME_MINTIME+10);// Frame ende
        bIrRingStart = bIrRingEnd;
        b = (300/50);
      }
    }
  }

  yIrCodeOk = (b==0)?yIrCodeOk:TRUE;

  // das IR-Frame liegt jetzt von 0 - bIrRingEnd in wIrRing
  // StartBit in wIrRing[0] (Pulse) und wIrRing[1] (Pause)

  wFramePause = IRTX_FRAME_REPEAT_PAUSE_TIME;

  for (bG=0;(bG<2) && yIrCodeOk;bG++) {
    // Werte glaetten
    // 1. wert[start-ende] PULSE_BIT loeschen
    // 2. wert[n] mit weiteren wert[n,start-ende] vergleichen:
    //     wenn wert[m] < 10% von wert[n] dann in Summe addieren, wert[m] mit PULSE_BIT setzen, count++
    // 3. summe /= count
    // 4. alle Werte mit PULSE_BIT gleich Summe setzen und PULSE_BIT loeschen
    // 5. wenn n<ende dann n++ und mit Punkt 2 weiter
    // 6. PULSE_BIT in 0,2,4 .. setzen

    // 1.
    for (b=0;b<=bIrRingEnd;b++) {
      if (wIrRing[b]==IRRX_NODATA) break;
      wIrRing[b] &= CALCMASK;
    }

    for (n=0;n<=bIrRingEnd;n++) {
      wSig = wIrRing[n];
      if (wSig==IRRX_NODATA) break;
      if (wSig & CALC2) continue;// -> wurde schon gemittelt
      wIrRing[n] |= CALC1;
      zSigD = wSig;
      bSigCount = 1;
      for (b=0;b<=bIrRingEnd;b++) {
        if (n==b) continue;
        wSig2 = wIrRing[b];
        if (wSig2==IRRX_NODATA) break;
        if (wSig2 & CALC2) continue;// -> wurde schon gemittelt
        if (bAbweichung(wSig,wSig2)<IRRX_DIFF) {// unter 22%
          zSigD += wSig2;
          bSigCount++; 
          wIrRing[b] |= CALC1;// in Berechnung aufnehmen
        }
      }

      // 3.
      zSigD /= bSigCount;
      wSig = (WORD)zSigD;
      // 4.
      for (b=0;b<=bIrRingEnd;b++) {
        wSig2 = wIrRing[b];
        if (wSig2 & CALC1) {
//          if (bAbweichung(wSig,wSig2)<IRRX_DIFF) { 
            wIrRing[b] = wSig | CALC2; 
//          } else {//-> zu groesse Abweichung
//            vLED(LONGBLINK | 2);
//            vSleep(250);
//            vSleep(250);
//            vSleep(250);
//            yIrCodeOk = FALSE;
//            n = bIrRingEnd;
//            break;
//          }
        }
      }
    }  
  }  

  if (yIrCodeOk) {
    // 6.
    for (b=0;b<=bIrRingEnd;b++) { 
      if (wIrRing[b]==IRRX_NODATA) break;
      wIrRing[b] |= PULSE_BIT;
      wIrRing[b] &= ~CALC2;
      b++;
      if (wIrRing[b]==IRRX_NODATA) break;
      wIrRing[b] &= ~CALC2;
    }

    // Grob die Pause zwischen zwei Frames ermitteln
    for (b=1;b<bIrRingEnd;b++) {
      wSig = wIrRing[b];     
      if (((wSig & PULSE_MASK)>IRRX_ENDFRAME_MINTIME)&&// Frame-Ende erkannt, min 50ms kein IR-Signal
          ((wSig & PULSE_BIT)==0)) {
        wFramePause = wSig;
        bIrRingEnd = b-1;
        for (;b<MAX_PULSEDIS_IRFRAMESIZE;b++)// den Rest nullen
          wIrRing[b] = IRRX_NODATA;
        break;
      }
    }
  }

  if (yIrCodeOk)
    if (bIrRingEnd<IRRX_MINSIGNALSIZE)// zu wenig Daten
      yIrCodeOk = FALSE;

  // Test auf Bi-Phase Protokolle
  if (yIrCodeOk) {
    // Test auf IRCODE_SIEMENS_PROTOCOL / IRCODE_RUWIDO_PROTOCOL
    if (!yBiPhase) {
      if ((bAbweichung(wIrRing[0],SIEMENS_OR_RUWIDO_START_BIT_PULSE_TIME)<=10)&&
          (bAbweichung(wIrRing[1],SIEMENS_OR_RUWIDO_START_BIT_PAUSE_TIME)<=10) ) {
        yBiPhase = TRUE;
        bIrCode = IRCODE_SIEMENS_PROTOCOL;
      }
    }

    // Test auf IRCODE_GRUNDIG_PROTOCOL / IRCODE_NOKIA_PROTOCOL / IRCODE_IR60_PROTOCOL
    if (!yBiPhase) {
      if ((bAbweichung(wIrRing[0],GRUNDIG_NOKIA_IR60_BIT_TIME)<=10)&&
          (bAbweichung(wIrRing[1],GRUNDIG_NOKIA_IR60_PRE_PAUSE_TIME)<=10) ) {
        yBiPhase = TRUE;
        bIrCode = IRCODE_GRUNDIG_PROTOCOL;
      }
    }

    // Test auf IRCODE_RC6_PROTOCOL / IRCODE_RC6A_PROTOCOL
    if (!yBiPhase) {
      if ((bAbweichung(wIrRing[0],RC6_START_BIT_PULSE_TIME)<=10)&&
          (bAbweichung(wIrRing[1],RC6_START_BIT_PAUSE_TIME)<=10) ) {
        yBiPhase = TRUE;
        bIrCode = IRCODE_RC6_PROTOCOL;
      }
    }
   
    // Test auf IRCODE_RC5_PROTOCOL
    if (!yBiPhase) {
      yBiPhase = TRUE;
      for (b=0;(b<=bIrRingEnd) && yBiPhase;b++) {
        wSig = wIrRing[b];
        if (wSig==IRRX_NODATA) break;

        if ((bAbweichung(wSig,RC5_BIT_TIME)>10)&&
            (bAbweichung(wSig,RC5_BIT_TIME*2)>10) )
          yBiPhase = FALSE;
      }
      if (yBiPhase) 
        bIrCode = IRCODE_RC5_PROTOCOL;
    }
  }

  // Test auf IR_DENON_PROTOCOL
  if (yIrCodeOk && (bIrCode==IRCODE_UNKNOWN)) {
    // Test auf Pulse 310us + Pause 1780us oder Test auf Pulse 310us + Pause 745us
    if (((bAbweichung(wIrRing[0],DENON_PULSE_TIME )<=10)&&
         (bAbweichung(wIrRing[1],DENON_0_PAUSE_TIME)<=10) ) ||
        ((bAbweichung(wIrRing[0],DENON_PULSE_TIME )<=10)&&
         (bAbweichung(wIrRing[1],DENON_1_PAUSE_TIME)<=10) )    )
      bIrCode = IRCODE_DENON_PROTOCOL;
  }

//Funtioniert nicht immer, da es einige Protokolle ohne Startbit gibt
  // weitere Frames nullen, ab 2. StartBit 
  // Achtung: Denon hat kein StartBit
  if (yIrCodeOk && !yBiPhase) {//-> Pulse Distance Protokolle
    if (bIrCode==IRCODE_DENON_PROTOCOL) {// -> Protokoll ohne StartBit
      //FixMe
    } else {// -> Protokoll mit StartBit
      // zweites StartBit suchen: erste Pulselaenge und erste Pausenlaenge ab 
      //  Position MINIRSIGNALCOUNT nochmal suchen, den Rest abschneiden
      for (b=IRRX_MINSIGNALSIZE;b<=bIrRingEnd;b++) {
        wSig = wIrRing[b];
        b++;
        wSig2 = wIrRing[b];
        if (wSig==IRRX_NODATA) break;
        if ((bAbweichung(wSig,wIrRing[0])<IRRX_DIFF)&&     // Pulse gegenueber Startbit
            (bAbweichung(wSig2,wIrRing[1])<IRRX_DIFF)   ) {// Pause gegenueber Startbit
          b -= 2;// weiteres StartBit nullen
          for (;b<=bIrRingEnd;b++)// den Rest nullen
            wIrRing[b] = IRRX_NODATA;
        }      
      }    
    }
  }

  if (yIrCodeOk) {
    // EndFrame Marke setzen
    for (b=0;b<MAX_PULSEDIS_IRFRAMESIZE;b++) {
      if (wIrRing[b]==IRRX_NODATA)  {
        wIrRing[b] = IRRX_ENDFRAME;
        b++;
        wIrRing[b] = wFramePause;
        break;
      }
    }
  }

  if (!yIrCodeOk) 
    bIrCode = IRCODE_ERROR;

  *pyBiPhase = yBiPhase;
  return bIrCode;
}

//**********************************************************************+*********************************************
//
// tFernb[ 0- 1],   bIndex =  0 -> Senden 1
// tFernb[ 2- 3],   bIndex =  2 -> Senden 2
// tFernb[ 4- 5],   bIndex =  4 -> Senden 3
// tFernb[ 6- 7],   bIndex =  6 -> Senden 4
// tFernb[ 8- 9],   bIndex =  8 -> Senden 5
// tFernb[10-11],   bIndex = 10 -> Senden 6
// tFernb[12-13],   bIndex = 12 -> Senden 7
// tFernb[14-15],   bIndex = 14 -> Senden 8
// tFernb[16],      bIndex = 16 -> Powertaste
//
//**********************************************************************+*********************************************
BOOL yLearnIrCode(BYTE bIndex,BYTE yIRDataFromLirc)
{
  BYTE b,bTemp2;
  BYTE bIrCodeA,bIrCodeB;
  BYTE yBiPhaseA,yBiPhaseB;
  BOOL yBiPhaseError;
  
  if (bIndex>=((2*(IRMEM-1))+1)) return FALSE;
   
  yBiPhaseError = FALSE;
 
  for (b=0;b<2;b++) {
    TR1 = 0;// IR-Empfang aus

    if (b>0) {
      vSleepSec(1);
      vLED(1);// OK blinken
    }

    bIrCodeA = bLearnIrCode(&yBiPhaseA,yIRDataFromLirc);
    while (!yIrRxSigTimeOverflow)// warten bis Fernbedienungstaste losgelassen wurde (=kein IR Signal)
      vSleep(10);
    TR1 = 0;// IR-Empfang aus

    if (bIrCodeA!=IRCODE_ERROR) {
      BYTE bOption = tFernb[bIndex].bOption;// Option sichern
      BYTE bRepeat = tFernb[bIndex].bRepeat;// Wiederholungen sichern
      if (yInternIr)
        bOption &= ~IR_OPTION_SENDINDARK;
      else
        bOption |= IR_OPTION_SENDINDARK;

      if (bRepeat>MAX_IRTX_REPEAT) bRepeat = 1;
      if (bRepeat==0) bRepeat = 1;
      vFlashClearFF((WORD)&(tFernb[bIndex]),T_IR_BREPEAT_SIZE);// von T_IR.yBiPhase bis einschliesslich T_IR.bRepeat löschen
      vFlashWrite((WORD)&(tFernb[bIndex].bOption),(BYTE*)&(bOption),sizeof(bOption));
      vFlashWrite((WORD)&(tFernb[bIndex].bRepeat),(BYTE*)&(bRepeat),sizeof(bRepeat));

      if (yBiPhaseA) 
        vFlashWrite((WORD)&(tFernb[bIndex].tIr.wBiPhase[0][0]),(BYTE*)&(wIrRing),2*MAX_BIPHASE_IRFRAMESIZE);
      else
        vFlashWrite((WORD)&(tFernb[bIndex].tIr.wPulseDis[0]),(BYTE*)&(wIrRing),2*MAX_PULSEDIS_IRFRAMESIZE);
      
      vSleepSec(1);
      vLED(1);// OK blinken

      bIrCodeB = bLearnIrCode(&yBiPhaseB,yIRDataFromLirc);
      while (!yIrRxSigTimeOverflow)// warten bis Taste losgelassen wurde (=kein IR Signal)
        vSleep(10);
      TR1 = 0;// IR-Empfang aus

      if ((bIrCodeA!=bIrCodeB)||
          (yBiPhaseA!=yBiPhaseB) )
        bIrCodeA = IRCODE_ERROR;

      // bei nicht BiPhase muessen beide Signalfolgen etwa gleich sein
      if ((bIrCodeA!=IRCODE_ERROR)&&!yBiPhaseA) {
        for (bTemp2=0;bTemp2<=bIrRingEnd;bTemp2++) { 
          if (wIrRing[bTemp2]==IRRX_NODATA) break;
          if (bAbweichung(tFernb[bIndex].tIr.wPulseDis[bTemp2],wIrRing[bTemp2])>=IRRX_DIFF) {// ueber 15% Abweichung
            bIrCodeA = IRCODE_ERROR;
            yBiPhaseError = TRUE;
            break;
          }
        }
      }
    }

    if (bIrCodeA!=IRCODE_ERROR) {
      vFlashWrite((WORD)&(tFernb[bIndex].yBiPhase),(BYTE*)&(yBiPhaseA),sizeof(yBiPhaseA));
      vFlashWrite((WORD)&(tFernb[bIndex].bIrCode),(BYTE*)&(bIrCodeA),sizeof(bIrCodeA));
      if (yBiPhaseA) 
        vFlashWrite((WORD)&(tFernb[bIndex].tIr.wBiPhase[1][0]),(BYTE*)&(wIrRing),2*MAX_BIPHASE_IRFRAMESIZE);
    } else {
      vLED(ERRORBLINK | (yBiPhaseError?4:3));
      vSleepSec(2);
      break;// -> Fehler beim lernen
    }

    if (bIndex==(IRMEM-1)*2) break;// -> Powertaste gelernt
    bIndex++;
  }

  return bIrCodeA!=IRCODE_ERROR?TRUE:FALSE;
}

//**********************************************************************+*********************************************
//bMenu bSubMenu
// 1x + 1x -> Powertaste anlernen 
// 1x + 2x -> Powertaste löschen

// 2x + 1x -> Lernbare Fernbedienung Speicher 1 (Vol+)
// 2x + 2x -> Lernbare Fernbedienung Speicher 2 (Vol-)
// 2x + 3x -> Lernbare Fernbedienung Speicher 3 (Mute)
// 2x + 4x -> Lernbare Fernbedienung Speicher 4 (Power TV)
// 2x + 5x -> Lernbare Fernbedienung Speicher 5
// 2x + 6x -> Lernbare Fernbedienung Speicher 6
// 2x + 7x -> Lernbare Fernbedienung Speicher 7
// 2x + 8x -> Lernbare Fernbedienung Speicher 8

// 3x + 1x -> Lernbare Fernbedienung 1 Senden erst bei losgelassener FB-Taste 
// 3x + 2x -> Lernbare Fernbedienung 2 Senden erst bei losgelassener FB-Taste 
// 3x + 3x -> Lernbare Fernbedienung 3 Senden erst bei losgelassener FB-Taste 
// 3x + 4x -> Lernbare Fernbedienung 4 Senden erst bei losgelassener FB-Taste 
// 3x + 5x -> Lernbare Fernbedienung 5 Senden erst bei losgelassener FB-Taste 
// 3x + 6x -> Lernbare Fernbedienung 6 Senden erst bei losgelassener FB-Taste 
// 3x + 7x -> Lernbare Fernbedienung 7 Senden erst bei losgelassener FB-Taste 
// 3x + 8x -> Lernbare Fernbedienung 8 Senden erst bei losgelassener FB-Taste 

// 4x + 1x -> Lernbare Fernbedienung 1 direkt Senden
// 4x + 2x -> Lernbare Fernbedienung 2 direkt Senden
// 4x + 3x -> Lernbare Fernbedienung 3 direkt Senden
// 4x + 4x -> Lernbare Fernbedienung 4 direkt Senden
// 4x + 5x -> Lernbare Fernbedienung 5 direkt Senden
// 4x + 6x -> Lernbare Fernbedienung 6 direkt Senden
// 4x + 7x -> Lernbare Fernbedienung 7 direkt Senden
// 4x + 8x -> Lernbare Fernbedienung 8 direkt Senden

// 5x + 1x -> Lernbare Fernbedienung Speicher 1 löschen
// 5x + 2x -> Lernbare Fernbedienung Speicher 2 löschen
// 5x + 3x -> Lernbare Fernbedienung Speicher 3 löschen
// 5x + 4x -> Lernbare Fernbedienung Speicher 4 löschen
// 5x + 5x -> Lernbare Fernbedienung Speicher 5 löschen
// 5x + 6x -> Lernbare Fernbedienung Speicher 6 löschen
// 5x + 7x -> Lernbare Fernbedienung Speicher 7 löschen
// 5x + 8x -> Lernbare Fernbedienung Speicher 8 löschen

// tFernb[ 0- 1],   bIndex =  0 -> Senden 1
// tFernb[ 2- 3],   bIndex =  2 -> Senden 2
// tFernb[ 4- 5],   bIndex =  4 -> Senden 3
// tFernb[ 6- 7],   bIndex =  6 -> Senden 4
// tFernb[ 8- 9],   bIndex =  8 -> Senden 5
// tFernb[10-11],   bIndex = 10 -> Senden 6
// tFernb[12-13],   bIndex = 12 -> Senden 7
// tFernb[14-15],   bIndex = 14 -> Senden 8
// tFernb[16],      bIndex = 16 -> Powertaste


// tUsbCmd.bCmd = C_IR
// tUsbCmd.bData[0] -> PosA -> bMenu
// tUsbCmd.bData[1] -> PosB -> bIndex
// tUsbCmd.bData[2] -> PosC -> bCount
//
// bCmd   PosA  PosB  PosC
// C_IR + C_1 + C_1 + C_0 + C_END -> Powertaste über IR-Empfänger anlernen 
// C_IR + C_1 + C_1 + C_1 + C_END -> Powertaste über irsend (lircd.conf) anlernen 
// C_IR + C_1 + C_2       + C_END -> Powertaste löschen
// C_IR + C_2 + n1  + C_0 + C_END -> Lernbare Fernbedienung, Speicher n1 über IR-Empfänger anlernen, n1 = C_1 bis C_8 
// C_IR + C_2 + n1  + C_1 + C_END -> Lernbare Fernbedienung, Speicher n1 über irsend (lircd.conf) anlernen, n1 = C_1 bis C_8 
// C_IR + C_3 + n1        + C_END -> Lernbare Fernbedienung n1, Senden erst bei losgelassener FB-Taste, n1 = C_1 bis C_8   
// C_IR + C_4 + n1        + C_END -> Lernbare Fernbedienung n1, direkt Senden, n1 = C_1 bis C_8
// C_IR + C_5 + n1        + C_END -> Lernbare Fernbedienung Speicher n1, löschen, n1 = C_1 bis C_8
// C_IR + C_6 + n1  + n2  + C_END -> Lernbare Fernbedienung n1 Sendewiederholung auf n2 einstellen, n2 = C_1 bis C_16
//
//**********************************************************************+*********************************************
void vIrOption(void)
{
  BOOL yOk = TRUE;
  BYTE bOption;
	BYTE bMenu = tUsbCmd.bData[0];
	BYTE bIndex = tUsbCmd.bData[1];
	BYTE bCount = tUsbCmd.bData[2];
	
  bIndex--;
  bIndex <<= 1;

  if ((bIndex>=((2*(IRMEM-1))+1))||(bCount>MAX_IRTX_REPEAT)) {
    vSleepSec(1);
    vLED(ERRORBLINK | 3);// Error blinken
  } else {
    switch (bMenu) {
    case 1: 
      if (bIndex==(2-1)*2) {// Powertaste loeschen  
        vFlashErasePage((WORD)&(tFernb[(IRMEM-1)*2]));
      } else { // Powertaste anlernen
        yOk = yLearnIrCode((IRMEM-1)*2,(bCount==C_1)?TRUE:FALSE);
      }
      break;
    case 2:
      yOk = yLearnIrCode(bIndex,(bCount==C_1)?TRUE:FALSE);
      break;
    case 3: // Lernbare Fernbedienung bIndex Senden erst bei losgelassener FB-Taste 
    case 4: // Lernbare Fernbedienung bIndex direkt Senden
      bOption = tFernb[bIndex].bOption;// Option sichern
      if (bMenu==3)
        bOption |= IR_OPTION_SENDINDARK;
      else
        bOption &= ~IR_OPTION_SENDINDARK;
      vFlashClearFF((WORD)&(tFernb[bIndex].bOption),sizeof(bOption));
      vFlashWrite((WORD)&(tFernb[bIndex].bOption),(BYTE*)&(bOption),sizeof(bOption));
      break;
    case 5: // Speicher bIndex loeschen
      vFlashClearFF((WORD)&(tFernb[bIndex++].bIrCode),1);// 0xFF = IRCODE_ERROR
      vFlashClearFF((WORD)&(tFernb[bIndex].bIrCode),1);// 0xFF = IRCODE_ERROR
      break;
    case 6: // Lernbare Fernbedienung bIndex: x Mal Wiederholungen setzen
      vFlashClearFF((WORD)&(tFernb[bIndex].bRepeat),sizeof(bCount));
      vFlashWrite((WORD)&(tFernb[bIndex].bRepeat),(BYTE*)&(bCount),sizeof(bCount));
      break;
    }

    if (yOk) {
      vSleepSec(bMenu<3?1:2);
      vLED(2); // ENDE blinken
    }
  }
  vInitIr(IRMODE_RX);
}

//**********************************************************************+*********************************************
// ein IR Puls/Pause Signal in den Empangspuffer legen
//**********************************************************************+*********************************************
void vIrRxByte(WORD wUs,BYTE yPulse)
{
  bIrRingEnd++;
  if (bIrRingEnd>=MAX_PULSEDIS_IRFRAMESIZE)
    bIrRingEnd = 0;

  if (yPulse) wUs |= PULSE_BIT;

  wIrRing[bIrRingEnd] = wUs;
}

//**********************************************************************+*********************************************
// RC5-Code in IR-Empfangspuffer hinterlegen
// Geraeteadresse ist immer 28dec
//**********************************************************************+*********************************************
void vGenRC5(BYTE bCmd)
{
  // 111 11100 bCmd     (bCmd = 000000-111111)
  WORD wRC5 = 0x3F00;
  WORD wBit = 0x2000;
  BOOL yLastBit;

  EA = 0;                                                               // Global Interrupt disable

  switch (bIrMode) {
  case IRMODE_LEARN:// Modus: IR-Signal empfangen zum lernen
  case IRMODE_RX:// Modus: IR-Signal empfangen
    wRC5 |= bCmd & 0x3F;

    if (bIrMode!=IRMODE_LEARN)
      vIrRxByte(PULSE_MASK-((WORD)(RC5_BIT_TIME)),FALSE);   
    yLastBit = FALSE;
    while (wBit!=0x0000) {
      if (wRC5 & wBit) {// 1 senden
        if (!((wBit==0x2000)&&(bIrMode==IRMODE_LEARN))) {
          if (yLastBit==FALSE) {
            wIrRing[bIrRingEnd] += RC5_BIT_TIME;
          } else {
            vIrRxByte(RC5_BIT_TIME,FALSE);      
          }
        }
        vIrRxByte(RC5_BIT_TIME,TRUE);
      } else {// 0 senden
        if (yLastBit==FALSE)
          vIrRxByte(RC5_BIT_TIME,TRUE);     
        else
          wIrRing[bIrRingEnd] += RC5_BIT_TIME;
        if (wBit!=0x0001)
          vIrRxByte(RC5_BIT_TIME,FALSE);
      }
      yLastBit = (wRC5 & wBit)?TRUE:FALSE;
      wBit >>= 1;
    }
    break;
  }

  EA = 1;                                                               // Global Interrupt enable
}
