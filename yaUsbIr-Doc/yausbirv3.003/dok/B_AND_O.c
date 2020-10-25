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
//**********************************************************************+*********************************************

void vSetIrOn(Byte yOn)
{
  if (yOn) {
    CR = 1;// Start PCA counter
    //IRTX = 1;// Sende-LED an
  } else {
    PCA0CN = 0x00; // Stop PCA counter; clear all flags; CR = 0;
    IRTX = 0;// Sende-LED aus
    PCA0L = 0xFF;
    PCA0H = 0xFF;
  }
}

void vSetIrFrequency(Word wHz)
{
  CR = 0;// Stop PCA counter

  XBR1 = 0x41; // Route CEX0 to P0.0

  // Configure PCA time base; overflow interrupt disabled
  PCA0CN = 0x00; // Stop counter; clear all flags

  if (wHz==0xFFFF)// -> 455KHz
    PCA0MD = 0x08; // Use SYSCLK/1 as time base
  else
    PCA0MD = 0x02; // Use SYSCLK/4 as time base

  PCA0CPM0 = 0x46; // Module 0 = Frequency Output mode

  // Configure frequency for CEX0
  // PCA0CPH0 = (SYSCLK/1)/(2*wKhz*1000), where:
  // SYSCLK/1 = PCA time base
  // CEX0_FREQUENCY = desired frequency
  // PCA0CPH0 = (SYSCLK/1)/(2*CEX0_FREQUENCY);

  if (wHz==0xFFFF)// -> 455KHz
    PCA0CPH0 = 53;//(SYSCLK/1)/(2*wHz);
  else
    PCA0CPH0 = (SYSCLK/8)/wHz;//(SYSCLK/4)/(2*wHz); (48000000/8)/38000 = 157
}




void vTimer1Isr(void) interrupt 3
{
  switch (bIrMode) {
  case IRMODE_TX:// Modus: IR-Signal senden
    if (wIrTime>0) {
      wIrTime--;
//      if (yIrSig) {//Pulse
//        IRTX = !IRTX;// Sende-LED toggeln
//      }
    } else {
//      IRTX = 0;// Sende-LED aus

      PCA0CN = 0x00; // Stop PCA counter; clear all flags; CR = 0;
      IRTX = 0;// Sende-LED aus
      PCA0L = 0xFF;
      PCA0H = 0xFF;

      if (bIrRingStart==bIrRingEnd) {// -> Puffer leer
        bIrMode = IRMODE_RX;// umschalten auf empfangen
        bIrRxUsStep = IRRX_STEP/6;
        TH1 = -(IRRX_STEP*2);// reload value in TH1
      } else {
        bRLedFlashCount = FLASHTIME + (bRLedFlashCount % 4);
        wIrTime = wIrRing[bIrRingStart];
        wIrRing[bIrRingStart] = 0;
        if ((wIrTime&PULSE_BIT)==PULSE_BIT) {
          wIrTime &= PULSE_MASK;
//          yIrSig = TRUE;//Pulse
          CR = 1;// Start PCA counter
        } //else {
//          yIrSig = FALSE;//Pause
//        }

        bIrRingStart++;
        if (bIrRingStart>=MAX_PULSEDIS_IRFRAMESIZE)
          bIrRingStart = 0;
      }
    }
    break;
