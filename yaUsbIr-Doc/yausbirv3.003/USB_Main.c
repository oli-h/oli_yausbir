//**********************************************************************+*********************************************
// yaUsbIR V3
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
             
//**********************************************************************+*********************************************
//
//  MCU : 8051F387
//        32 KByte Flash
//        256 Byte RAM
//        2 KByte XRAM
//        1 KByte USB-RAM
//
//**********************************************************************+*********************************************

#define MAIN
#include "common.h"
#include "USB_Configuration.h"
#include "USB_Register.h"
#include "USB_Standard_Requests.h"
#include "USB_ISR.h"
#include "USB_Main.h"
#include "sio.h"
#include "io.h"
#include "ir.h"
#include "flash.h"
extern WORD wPowerTimeout;

//**********************************************************************+*********************************************
// Main Routine
//**********************************************************************+*********************************************
void main(void)
{
  BYTE bTemp;

  PCA0MD &= ~0x40;                                                      // Disable Watchdog timer temporarily
  RSTSRC = 0x00;

  vSysclkInit();                                                        // Initialize oscillator
  vPortInit();                                                          // Initialize crossbar and GPIO
  vUsb0Init();                                                          // Initialize USB0
  //vSetComBaud(19200);//only4Debug  
  vInitTimer0();

  tUsbCmd.bCmd = 0;

	tUsbCmd.bData[0] = SWITCH;

  if ((tUsbCmd.bData[0]==0)||
      (tFernb[0].wMainboardPowerOffTime>(99*(1000/10))) ) 
    // Lernspeicher definiert löschen nach MCU programmieren
    vWerkseinstellung();

  EIE1 |= 0x02;                                                         // Enable USB0 interrupt
  EA = 1;                                                               // Global Interrupt enable

  vLED(LONGBLINK | (tUsbCmd.bData[0]==0?2:1));
  vInitIr(IRMODE_RX);
 
  while (TRUE) {

    vIrSearch();

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

    if (yGetMenuSwitch()) { // -> Lern Taste gedrueckt
      tUsbCmd.bData[0] = 1;
      tUsbCmd.bData[1] = 0;
      tUsbCmd.bData[2] = 0;
      for (bTemp=0;bTemp<40;bTemp++) {
        vSleep(50);
        if (yGetMenuSwitch()) {
          tUsbCmd.bData[0]++;
          bTemp = 0;
        }
      }
      if (tUsbCmd.bData[0]>6) {
        vLED(ERRORBLINK | 3);// Error blinken
      } else {
        vLED(tUsbCmd.bData[0]);

        for (bTemp=0;bTemp<100;bTemp++) {
          vSleep(50);
          if (yGetMenuSwitch()) {
            tUsbCmd.bData[1]++;
            bTemp = 60;
          }
        }

        if ((tUsbCmd.bData[1]==0)||
            (tUsbCmd.bData[1]>IRMEM)||
            ((tUsbCmd.bData[0]==1)&&(tUsbCmd.bData[1]>2)) ){
          vLED(ERRORBLINK | 3);// Error blinken
        } else {
          vLED(tUsbCmd.bData[1]);

          if (tUsbCmd.bData[0]==6) {
            for (bTemp=0;bTemp<100;bTemp++) {
              vSleep(50);
              if (yGetMenuSwitch()) {
                tUsbCmd.bData[2]++;
                bTemp = 60;
              }
            }
            if ((tUsbCmd.bData[2]==0)||(tUsbCmd.bData[2]>MAX_IRTX_REPEAT)) {
              vLED(ERRORBLINK | 3);// Error blinken
              tUsbCmd.bData[0] = 0;
            } else {
              vLED(tUsbCmd.bData[2]);
            }
          }
          if (tUsbCmd.bData[0]>0)
            vIrOption();
        }
      }
    }

    if (wPowerTimeout>0) {// IR-Daten löschen
      if (yPacketToHost(TRUE)) 
        yPacketToHost(FALSE);
    }

    // Daten zum Computer ueber USB senden
    // task1: USB IN EP handling
    if ( Ep_StatusIN1 != EP_HALT ) {
      if (yPacketToHost(TRUE)) {// -> Sendedaten vorhanden
        if (_testbit_(IN_FIFO_empty)) {                                 // atomic access handling when FIFO is empty
          yPacketToHost(FALSE);// Sendedaten holen
                                                                        // To prevent conflict with USB interrupt
          bTemp = EIE1 & 0x02;                                          //   save USB interrupt enable bit
          EIE1 &= ~0x02;                                                //   disable USB interrupt temporarily
          POLL_WRITE_BYTE( INDEX, 1 );                                  // Load packet to FIFO
          Fifo_Write(FIFO_EP1, EP1_PACKET_SIZE, bInPacket);
          POLL_WRITE_BYTE(EINCSRL, rbInINPRDY);                         // set FIFO ready flag
          EIE1 |= bTemp;                                                // restore USB interrupt
        }
      }
    }

    // Daten vom Computer ueber USB empfangen
    // task2: USB OUT EP handling
    vDoUsbOut(FALSE);
/*
    if (Ep_StatusOUT1 != EP_HALT) {
      if (_testbit_(OUT_FIFO_loaded))    {                              // atomic access handling when FIFO is loaded
        // Daten aus USB-Puffer holen
                                                                        // To prevent conflict with USB interrupt
        bTemp = EIE1 & 0x02;                                            //   save USB interrupt enable bit
        EIE1 &= ~0x02;                                                  //   disable USB interrupt temporarily
        POLL_WRITE_BYTE( INDEX, 1 );                                    // unload packet from FIFO
        Fifo_Read(FIFO_EP1, EP1_PACKET_SIZE, bOutPacket);
        POLL_WRITE_BYTE(EOUTCSRL, 0);                                   // Clear Out Packet ready bit
        EIE1 |= bTemp;                                                  // restore USB interrupt

        vPacketFromHost();
      }
    }
*/
    if (tUsbCmd.bCmd & 0x80) {
      switch (tUsbCmd.bCmd & 0x7F) {
      case C_WATCHDOG:
        vSetWatchdog();
        break;        
      case C_OUTPUT:
        vSetOutput();
        break;        
      case C_INPUT:
        vSetInput();
        break;        
      case C_IR:
        if (tUsbCmd.bData[0]>C_6) {
          vIRConfig();
        } else {
          vLED(1);
          vIrOption();
        }
        break;        
      }
      tUsbCmd.bCmd = 0;
    }
  }
}

//**********************************************************************+*********************************************
// Daten vom Computer ueber USB empfangen
// task2: USB OUT EP handling
//**********************************************************************+*********************************************
void vDoUsbOut(BYTE yLircLearn)
{
  BYTE bTemp;

  if (Ep_StatusOUT1 != EP_HALT) {
    if (_testbit_(OUT_FIFO_loaded))    {                              // atomic access handling when FIFO is loaded
      // Daten aus USB-Puffer holen
                                                                      // To prevent conflict with USB interrupt
      bTemp = EIE1 & 0x02;                                            //   save USB interrupt enable bit
      EIE1 &= ~0x02;                                                  //   disable USB interrupt temporarily
      POLL_WRITE_BYTE( INDEX, 1 );                                    // unload packet from FIFO
      Fifo_Read(FIFO_EP1, EP1_PACKET_SIZE, bOutPacket);
      POLL_WRITE_BYTE(EOUTCSRL, 0);                                   // Clear Out Packet ready bit
      EIE1 |= bTemp;                                                  // restore USB interrupt

      vPacketFromHost(yLircLearn);
    }
  }
}

//**********************************************************************+*********************************************
// vUsbSuspend
// Enter suspend mode after suspend signalling is present on the bus
//      - called from USB ISR
//**********************************************************************+*********************************************
#ifdef ENABLE_SUSPEND_RESUME
void vUsbSuspend(void)
{           
//  OSCICN |= 0x20;                     // Put oscillator to suspend

  // When the device receives a non-idle USB event, it will resume execution
  // on the instruction that follows OSCICN |= 0x20.

  // Re-enable everything that was disabled when going into Suspend
}
#endif  // ENABLE_SUSPEND_RESUME

//**********************************************************************+*********************************************
// vUsbResume
// Resume normal USB operation
//      - called from USB ISR
//**********************************************************************+*********************************************
#ifdef ENABLE_SUSPEND_RESUME
// In this implementation, suspend/resume is supported by the oscillator suspend (OSCICN.5)
// Then, the resume process is also coded in vUsbSuspend(), just after "OSCICN |= 0x20"
void vUsbResume(void)
{
  vInitIr(IRMODE_RX);
}
#endif  // ENABLE_SUSPEND_RESUME
