//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
#ifndef USB_MAIN_H
#define USB_MAIN_H

// Global Constants
#include "common.h"

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK        0x00            // Select 4x clock multiplier, for USB Full Speed
#define USB_INT_OSC_DIV_2   0x10            // See Oscillators in Datasheet
#define USB_EXT_OSC         0x20
#define USB_EXT_OSC_DIV_2   0x30
#define USB_EXT_OSC_DIV_3   0x40
#define USB_EXT_OSC_DIV_4   0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC         0x00            // Select to use internal osc.
#define SYS_EXT_OSC         0x01            // Select to use external osc.
#define SYS_4X_DIV_2        0x02
#define SYS_4X              0x03
#define SYS_LOW_FREQ_OSC    0x04            // Low frequency OSC

#define SYSCLK 48000000L                    // SYSCLK frequency in Hz

#define MAX_PULSEDIS_IRFRAMESIZE (3*64)     // max. Speicherlänge für Pulse Distance 
#define MAX_BIPHASE_IRFRAMESIZE (MAX_PULSEDIS_IRFRAMESIZE/2)// max. Speicherlänge für BiPhase

#define IR_OPTION_SENDINDARK 0x01

typedef struct {                            // Konfig-Daten im FLASH, max eine Page (512 bytes):
  BYTE yBiPhase;
  BYTE bIrCode;
  union {
    // gelerntes IR-Signal für Power ON/OFF
    WORD wPulseDis[MAX_PULSEDIS_IRFRAMESIZE];       
    WORD wBiPhase[2][MAX_BIPHASE_IRFRAMESIZE];
  } tIr;
  BYTE bOption;// siehe IR_OPTION_xxx
  BYTE bRepeat;// Anzahl der Sendewiederholung, 1..MAX_IRTX_REPEAT
  WORD wMainboardPowerOffTime;// Zeit bis Mainboard über den Powerbutton ausschaltet
  WORD wMainboardPowerOffPauseTime;//Pausenzeit nach abschalten über Powerbutton
  WORD wMainboardPowerOnTime;// Zeit zum Einschalten über Powerbutton
  BYTE bKeyBoardMatrix;// 0= Inputs als normale 8 Inputs lesen, 1= Inputs für eine Tastaturmatrix verwenden
  BYTE bKeyBoardInputMask;// Dx=1 -> Input als Tastaturinput, Dx=0 -> Input nur bei Flankenwechsel oder per Anfrage
  BYTE bKeyBoardRepeatTime;// Zeitabstand Tastaturwiederholung
  BYTE bSignalLEDMode;// 0= Signal-LED normal, 1= Signal-LED dimmen
  BYTE bDummy[512-(1+1+(2*MAX_PULSEDIS_IRFRAMESIZE)+1+1+2+2+2+1+1+1+1)];
} T_IR;

#define T_IR_BREPEAT_SIZE (1+1+sizeof(tFernb[0].tIr)+1+1)

typedef struct {
  BYTE bCmd;
  BYTE bData[4];
  BYTE bPos;
} T_CONFCMD;

#define IRMEM 9

// Global variables
#ifdef MAIN
  BYTE idata bOutPacket[ EP1_PACKET_SIZE ]; // Last packet received from host
  BYTE idata bInPacket[ EP1_PACKET_SIZE ];  // Next packet to sent to host
  //BYTE xdata bInPacket[ EP1_PACKET_SIZE ];// Next packet to sent to host

  // !!! Locked Flash Pages = 0x7E00-0x7FFF !!!
  // Unlocked Flash Pages = 0x0000-0x7DFF
  code T_IR tTEMP _at_ 0x7C00;      // 512 Bytes Temp FlashData-Speicher für Löschaktionen
  code T_IR tFernb[(2*(IRMEM-1))+1] _at_ 0x7C00-(((2*(IRMEM-1))+1)*0x200);
  T_CONFCMD xdata tUsbCmd;  
#else
  extern BYTE idata bOutPacket[];           // Last packet received from host
  extern BYTE idata bInPacket[];            // Next packet to sent to host
  extern code T_IR tTEMP;
  extern code T_IR tFernb[];
  extern T_CONFCMD xdata tUsbCmd;
#endif

#define USBOUT_MAIN 0
#define USBOUT_LEARN 1

void vDoUsbOut(BYTE yLircLearn);

#endif // USB_MAIN_H
