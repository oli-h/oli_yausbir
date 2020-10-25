//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
#ifndef IO_H
#define IO_H

sbit IRRX     = P0^1;                       // IR-Empfänger, =0 IRPulse
sbit SWITCH   = P2^7;                       // Schliesser, =0 wenn Taste gedrückt
sbit LEDEX    = P2^0;                       // externe grüne LED, =0 dann LED eingeschaltet
sbit LEDR     = P2^1;                       // rote LED, =0 dann LED eingeschaltet
sbit MBSWITCH = P2^2;                       // Optokoppler, =0 Powerswitch Taste des MBs gedrückt
sbit IRTX     = P0^0;                       // IR-Sender, =1 dann Ausgang nach GND geschaltet und IR-LED ein
#define ESWITCH P1                          // 8x externe Schliesser, =0 wenn Taste gedrückt

sbit OUT1 = P0^4;
sbit OUT2 = P0^5;
sbit OUT3 = P0^6;
sbit OUT4 = P0^7;

void vIRConfig(void);
void vSysclkInit(void);
void vPortInit(void);
void vSetOutput(void);
void vSetWatchdog(void);
void vSetInput(void);
void vWerkseinstellung(void);

#endif // IO_H
