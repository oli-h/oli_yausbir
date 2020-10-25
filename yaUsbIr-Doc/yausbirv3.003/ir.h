//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
#ifndef IR_H
#define IR_H

#define WITH_455KHZ

#define IRRX_F_POLL 6000000
#define IRRX_STEP 78  // = 38461,5384615 KHz
#ifdef WITH_455KHZ
  #define IRRX_STEP_455KHZ 71
#endif

#define F_INT (IRRX_F_POLL/IRRX_STEP)

#define IRRX_DIFF 22                                  // in %, Signalabweichung gegenüber der gelernten Puls/Pausenweite
#define IRRX_ENDFRAME 0xFFFF
#define IRRX_NODATA 0x0000

#define IRRX_ENDFRAME_MINTIME (50.0e-3*F_INT)         // Pause > 50ms 
#define IRRX_TIMEOUT_TIME (200.0e-3*F_INT)            // Pause > 200ms
#define IRRX_MINSIGNALSIZE 10
#define IRTX_MAXPULSE_TIME (WORD)(20.0e-3*F_INT)      // max. Pulsesendezeit der IR-Sendedioden 20 ms
#define IRTX_FRAME_REPEAT_PAUSE_TIME (45.0e-3*F_INT)  // default, 45 ms Pausesendezeit zwischen zwei Frames 

#define OUTPUT_ON_TIME (800/10)                       // Output für 800ms einschalten

#define SIGNALLED_OFF 0
#define SIGNALLED_ON 1

#define MAX_IRTX_REPEAT 16

#define POWER_TIMEOUT (5000/10)

//**********************************************************************+*********************************************
// Infos IR Protokolle von der Seite: 
// http://www.sbprojects.com/knowledge/ir/index.php
// http://perso.netplus.ch/FCorthay/IR/index.html
//**********************************************************************+*********************************************
                                          //        |  kein  |FAIL|    |
                                          // BiPhase|StartBit|PASS|CODE|
                                          // -------+--------+----+----+----------------------------------------------
#define IRCODE_UNKNOWN                  0 //        |        |    |    | Unbekannt
#define IRCODE_RC5_PROTOCOL             1 //   x    |   x    |    |    | Philips etc
#define IRCODE_RC6_PROTOCOL             2 //   x    |        |    |    | Philips etc
#define IRCODE_GRUNDIG_PROTOCOL         3 //   x    |        |pass|0117| Grundig
#define IRCODE_NOKIA_PROTOCOL           4 //   x    |        |pass|0061| Nokia
#define IRCODE_SIEMENS_PROTOCOL         5 //   x    |        |pass|0200| Siemens, e.g. Gigaset
#define IRCODE_RC6A_PROTOCOL            6 //   x    |        |    |    | RC6A, e.g. Kathrein, XBOX
#define IRCODE_RUWIDO_PROTOCOL          7 //   x    |        |    |    | Ruwido, e.g. T-Home Mediareceiver
#define IRCODE_IR60_PROTOCOL            8 //   x    |        |    |    | IR60 (SDA2008)
#define IRCODE_DENON_PROTOCOL           9 //        |   x    |pass|0364| Denon, Sharp
#define IRCODE_SIRCS_PROTOCOL          10 //        |        |pass|0076| Sony (40KHz!)
#define IRCODE_NEC_PROTOCOL            11 //        |        |pass|0056| NEC, Pioneer, JVC, Toshiba, NoName etc.
#define IRCODE_SAMSUNG_PROTOCOL        12 //        |        |pass|0160| Samsung
#define IRCODE_MATSUSHITA_PROTOCOL     13 //        |        |pass|0065| Matsushita
#define IRCODE_KASEIKYO_PROTOCOL       14 //        |        |pass|0197| Kaseikyo (Panasonic etc)
#define IRCODE_RECS80_PROTOCOL         15 //        |        |pass|1394| Philips, Thomson, Nordmende, Telefunken, Saba
#define IRCODE_SAMSUNG32_PROTOCOL      16 //        |        |    |    | Samsung32: no sync pulse at bit 16, length 32 instead of 37
#define IRCODE_RECS80EXT_PROTOCOL      17 //        |        |pass|0099| Philips, Technisat, Thomson, Nordmende, Telefunken, Saba
#define IRCODE_NUBERT_PROTOCOL         18 //        |        |    |    | Nubert
#define IRCODE_LEGO_PROTOCOL           19 //        |        |    |    | LEGO Power Functions RC
#define IRCODE_THOMSON_PROTOCOL        20 //        |        |    |    | Thomson
#define IRCODE_NEC42_PROTOCOL          21 //        |        |    |    | NEC with 42 bits
#define IRCODE_NEC16_PROTOCOL          22 //        |        |    |    | NEC with 16 bits (incl. sync)
#define IRCODE_JVC_PROTOCOL            23 //        |        |pass|0051| JVC (NEC with 16 bits)
#define IRCODE_KATHREIN_PROTOCOL       24 //        |        |    |    | Kathrein
#define IRCODE_SKYMASTER_PROTOCOL      25 //        |        |pass|0016| Skymaster

#define IRCODE_ERROR                 0xFF // Fehler beim lernen oder keine Taste angelernt

// Bitlängen der IR-Protokolle

#define SIRCS_START_BIT_PULSE_TIME                 ( 2400.0e-6*F_INT) // 2400 usec pulse
#define SIRCS_START_BIT_PAUSE_TIME                 (  600.0e-6*F_INT) //  600 usec pause
#define SIRCS_1_PULSE_TIME                         ( 1200.0e-6*F_INT) // 1200 usec pulse
#define SIRCS_0_PULSE_TIME                         (  600.0e-6*F_INT) //  600 usec pulse
#define SIRCS_PAUSE_TIME                           (  600.0e-6*F_INT) //  600 usec pause
#define SIRCS_AUTO_REPETITION_PAUSE_TIME           (   25.0e-3*F_INT) // auto repetition after 25ms
#define SIRCS_FRAME_REPEAT_PAUSE_TIME              (   25.0e-3*F_INT) // frame repeat after 25ms

#define NEC_START_BIT_PULSE_TIME                   ( 9000.0e-6*F_INT) // 9000 usec pulse
#define NEC_START_BIT_PAUSE_TIME                   ( 4500.0e-6*F_INT) // 4500 usec pause
#define NEC_REPEAT_START_BIT_PAUSE_TIME            ( 2250.0e-6*F_INT) // 2250 usec pause
#define NEC_PULSE_TIME                             (  560.0e-6*F_INT) //  560 usec pulse
#define NEC_1_PAUSE_TIME                           ( 1690.0e-6*F_INT) // 1690 usec pause
#define NEC_0_PAUSE_TIME                           (  560.0e-6*F_INT) //  560 usec pause
#define NEC_FRAME_REPEAT_PAUSE_TIME                (   40.0e-3*F_INT) // frame repeat after 40ms

#define SAMSUNG_START_BIT_PULSE_TIME               ( 4500.0e-6*F_INT) // 4500 usec pulse
#define SAMSUNG_START_BIT_PAUSE_TIME               ( 4500.0e-6*F_INT) // 4500 usec pause
#define SAMSUNG_PULSE_TIME                         (  550.0e-6*F_INT) //  550 usec pulse
#define SAMSUNG_1_PAUSE_TIME                       ( 1650.0e-6*F_INT) // 1650 usec pause
#define SAMSUNG_0_PAUSE_TIME                       (  550.0e-6*F_INT) //  550 usec pause
#define SAMSUNG_FRAME_REPEAT_PAUSE_TIME            (   25.0e-3*F_INT) // frame repeat after 25ms
#define SAMSUNG32_AUTO_REPETITION_PAUSE_TIME       (   47.0e-3*F_INT) // repetition after 47 ms
#define SAMSUNG32_FRAME_REPEAT_PAUSE_TIME          (   47.0e-3*F_INT) // frame repeat after 47ms

#define MATSUSHITA_START_BIT_PULSE_TIME            ( 3488.0e-6*F_INT) // 3488 usec pulse
#define MATSUSHITA_START_BIT_PAUSE_TIME            ( 3488.0e-6*F_INT) // 3488 usec pause
#define MATSUSHITA_PULSE_TIME                      (  872.0e-6*F_INT) //  872 usec pulse
#define MATSUSHITA_1_PAUSE_TIME                    ( 2616.0e-6*F_INT) // 2616 usec pause
#define MATSUSHITA_0_PAUSE_TIME                    (  872.0e-6*F_INT) //  872 usec pause
#define MATSUSHITA_FRAME_REPEAT_PAUSE_TIME         (   45.0e-3*F_INT) // frame repeat after 45ms

#define KASEIKYO_START_BIT_PULSE_TIME              ( 3380.0e-6*F_INT) // 3380 usec pulse
#define KASEIKYO_START_BIT_PAUSE_TIME              ( 1690.0e-6*F_INT) // 1690 usec pause
#define KASEIKYO_PULSE_TIME                        (  423.0e-6*F_INT) //  525 usec pulse
#define KASEIKYO_1_PAUSE_TIME                      ( 1269.0e-6*F_INT) //  525 usec pause
#define KASEIKYO_0_PAUSE_TIME                      (  423.0e-6*F_INT) // 1690 usec pause
#define KASEIKYO_AUTO_REPETITION_PAUSE_TIME        (   74.0e-3*F_INT) // repetition after 74 ms
#define KASEIKYO_FRAME_REPEAT_PAUSE_TIME           (   74.0e-3*F_INT) // frame repeat after 74 ms

#define RECS80_START_BIT_PULSE_TIME                (  158.0e-6*F_INT) //  158 usec pulse
#define RECS80_START_BIT_PAUSE_TIME                ( 7432.0e-6*F_INT) // 7432 usec pause
#define RECS80_PULSE_TIME                          (  158.0e-6*F_INT) //  158 usec pulse
#define RECS80_1_PAUSE_TIME                        ( 7432.0e-6*F_INT) // 7432 usec pause
#define RECS80_0_PAUSE_TIME                        ( 4902.0e-6*F_INT) // 4902 usec pause
#define RECS80_FRAME_REPEAT_PAUSE_TIME             (   45.0e-3*F_INT) // frame repeat after 45ms

#define RC5_BIT_TIME                               (  889.0e-6*F_INT) // 889 usec pulse/pause
#define RC5_FRAME_REPEAT_PAUSE_TIME                (   45.0e-3*F_INT) // frame repeat after 45ms

#define DENON_PULSE_TIME                           (  310.0e-6*F_INT) //  310 usec pulse in practice,  275 in theory
#define DENON_1_PAUSE_TIME                         ( 1780.0e-6*F_INT) // 1780 usec pause in practice, 1900 in theory
#define DENON_0_PAUSE_TIME                         (  745.0e-6*F_INT) //  745 usec pause in practice,  775 in theory
#define DENON_AUTO_REPETITION_PAUSE_TIME           (   65.0e-3*F_INT) // inverted repetition after 65ms
#define DENON_FRAME_REPEAT_PAUSE_TIME              (   65.0e-3*F_INT) // frame repeat after 65ms

#define RC6_START_BIT_PULSE_TIME                   ( 2666.0e-6*F_INT) // 2.666 msec pulse
#define RC6_START_BIT_PAUSE_TIME                   (  889.0e-6*F_INT) // 889 usec pause
#define RC6_TOGGLE_BIT_TIME                        (  889.0e-6*F_INT) // 889 msec pulse/pause
#define RC6_BIT_TIME                               (  444.0e-6*F_INT) // 889 usec pulse/pause
#define RC6_FRAME_REPEAT_PAUSE_TIME                (   45.0e-3*F_INT) // frame repeat after 45ms

#define RECS80EXT_START_BIT_PULSE_TIME             (  158.0e-6*F_INT) //  158 usec pulse
#define RECS80EXT_START_BIT_PAUSE_TIME             ( 3637.0e-6*F_INT) // 3637 usec pause
#define RECS80EXT_PULSE_TIME                       (  158.0e-6*F_INT) //  158 usec pulse
#define RECS80EXT_1_PAUSE_TIME                     ( 7432.0e-6*F_INT) // 7432 usec pause
#define RECS80EXT_0_PAUSE_TIME                     ( 4902.0e-6*F_INT) // 4902 usec pause
#define RECS80EXT_FRAME_REPEAT_PAUSE_TIME          (   45.0e-3*F_INT) // frame repeat after 45ms

#define NUBERT_START_BIT_PULSE_TIME                ( 1340.0e-6*F_INT) // 1340 usec pulse
#define NUBERT_START_BIT_PAUSE_TIME                (  340.0e-6*F_INT) //  340 usec pause
#define NUBERT_1_PULSE_TIME                        ( 1340.0e-6*F_INT) // 1340 usec pulse
#define NUBERT_1_PAUSE_TIME                        (  340.0e-6*F_INT) //  340 usec pause
#define NUBERT_0_PULSE_TIME                        (  500.0e-6*F_INT) //  500 usec pulse
#define NUBERT_0_PAUSE_TIME                        ( 1300.0e-6*F_INT) // 1300 usec pause
#define NUBERT_AUTO_REPETITION_PAUSE_TIME          (   35.0e-3*F_INT) // auto repetition after 35ms
#define NUBERT_FRAME_REPEAT_PAUSE_TIME             (   35.0e-3*F_INT) // frame repeat after 45ms

#define BANG_OLUFSEN_START_BIT1_PULSE_TIME         (  200.0e-6*F_INT) //   200 usec pulse
#define BANG_OLUFSEN_START_BIT1_PAUSE_TIME         ( 3125.0e-6*F_INT) //  3125 usec pause
#define BANG_OLUFSEN_START_BIT2_PULSE_TIME         (  200.0e-6*F_INT) //   200 usec pulse
#define BANG_OLUFSEN_START_BIT2_PAUSE_TIME         ( 3125.0e-6*F_INT) //  3125 usec pause
#define BANG_OLUFSEN_START_BIT3_PULSE_TIME         (  200.0e-6*F_INT) //   200 usec pulse
#define BANG_OLUFSEN_START_BIT3_PAUSE_TIME         (15625.0e-6*F_INT) // 15625 usec pause
#define BANG_OLUFSEN_START_BIT4_PULSE_TIME         (  200.0e-6*F_INT) //   200 usec pulse
#define BANG_OLUFSEN_START_BIT4_PAUSE_TIME         ( 3125.0e-6*F_INT) //  3125 usec pause
#define BANG_OLUFSEN_PULSE_TIME                    (  200.0e-6*F_INT) //   200 usec pulse
#define BANG_OLUFSEN_1_PAUSE_TIME                  ( 9375.0e-6*F_INT) //  9375 usec pause
#define BANG_OLUFSEN_0_PAUSE_TIME                  ( 3125.0e-6*F_INT) //  3125 usec pause
#define BANG_OLUFSEN_R_PAUSE_TIME                  ( 6250.0e-6*F_INT) //  6250 usec pause (repeat last bit)
#define BANG_OLUFSEN_TRAILER_BIT_PAUSE_TIME        (12500.0e-6*F_INT) // 12500 usec pause (trailer bit)
#define BANG_OLUFSEN_FRAME_REPEAT_PAUSE_TIME       (   45.0e-3*F_INT) // frame repeat after 45ms

#define GRUNDIG_NOKIA_IR60_BIT_TIME                (  528.0e-6*F_INT) // 528 usec pulse/pause
#define GRUNDIG_NOKIA_IR60_PRE_PAUSE_TIME          ( 2639.0e-6*F_INT) // 2639 usec pause after pre bit
#define GRUNDIG_NOKIA_IR60_FRAME_REPEAT_PAUSE_TIME ( 117.76e-3*F_INT) // info frame repeat after 117.76 ms
#define GRUNDIG_AUTO_REPETITION_PAUSE_TIME         (   20.0e-3*F_INT) // repetition after 20ms

#define NOKIA_AUTO_REPETITION_PAUSE_TIME           (   20.0e-3*F_INT) // repetition after 20ms

#define IR60_AUTO_REPETITION_PAUSE_TIME            (   22.2e-3*F_INT) // repetition after 22.2ms

#define SIEMENS_OR_RUWIDO_START_BIT_PULSE_TIME     (  275.0e-6*F_INT) //  275 usec pulse
#define SIEMENS_OR_RUWIDO_START_BIT_PAUSE_TIME     (  550.0e-6*F_INT) //  550 usec pause
#define SIEMENS_OR_RUWIDO_BIT_PULSE_TIME           (  275.0e-6*F_INT) //  275 usec short pulse
#define SIEMENS_OR_RUWIDO_BIT_PULSE_TIME_2         (  550.0e-6*F_INT) //  550 usec long pulse
#define SIEMENS_OR_RUWIDO_BIT_PAUSE_TIME           (  275.0e-6*F_INT) //  275 usec short pause
#define SIEMENS_OR_RUWIDO_BIT_PAUSE_TIME_2         (  550.0e-6*F_INT) //  550 usec long pause
#define SIEMENS_OR_RUWIDO_FRAME_REPEAT_PAUSE_TIME  (   45.0e-3*F_INT) // frame repeat after 45ms

#define FDC_START_BIT_PULSE_TIME                   ( 2085.0e-6*F_INT) // 2085 usec pulse
#define FDC_START_BIT_PAUSE_TIME                   (  966.0e-6*F_INT) //  966 usec pause
#define FDC_PULSE_TIME                             (  300.0e-6*F_INT) //  300 usec pulse
#define FDC_1_PAUSE_TIME                           (  715.0e-6*F_INT) //  715 usec pause
#define FDC_0_PAUSE_TIME                           (  220.0e-6*F_INT) //  220 usec pause
#define FDC_FRAME_REPEAT_PAUSE_TIME                (   60.0e-3*F_INT) // frame repeat after 60ms

#define RCCAR_START_BIT_PULSE_TIME                 ( 2000.0e-6*F_INT) // 2000 usec pulse
#define RCCAR_START_BIT_PAUSE_TIME                 ( 2000.0e-6*F_INT) // 2000 usec pause
#define RCCAR_PULSE_TIME                           (  600.0e-6*F_INT) //  360 usec pulse
#define RCCAR_1_PAUSE_TIME                         (  450.0e-6*F_INT) //  650 usec pause
#define RCCAR_0_PAUSE_TIME                         (  900.0e-6*F_INT) //  180 usec pause
#define RCCAR_FRAME_REPEAT_PAUSE_TIME              (   40.0e-3*F_INT) // frame repeat after 40ms

#define JVC_START_BIT_PULSE_TIME                   ( 9000.0e-6*F_INT) // 9000 usec pulse
#define JVC_START_BIT_PAUSE_TIME                   ( 4500.0e-6*F_INT) // 4500 usec pause
#define JVC_PULSE_TIME                             (  560.0e-6*F_INT) //  560 usec pulse
#define JVC_1_PAUSE_TIME                           ( 1690.0e-6*F_INT) // 1690 usec pause
#define JVC_0_PAUSE_TIME                           (  560.0e-6*F_INT) //  560 usec pause
#define JVC_FRAME_REPEAT_PAUSE_TIME                (   22.0e-3*F_INT) // frame repeat after 22ms

#define NIKON_START_BIT_PULSE_TIME                 ( 2200.0e-6*F_INT) //  2200 usec pulse
#define NIKON_START_BIT_PAUSE_TIME                 (27100.0e-6*F_INT) // 27100 usec pause
#define NIKON_PULSE_TIME                           (  500.0e-6*F_INT) //   500 usec pulse
#define NIKON_1_PAUSE_TIME                         ( 3500.0e-6*F_INT) //  3500 usec pause
#define NIKON_0_PAUSE_TIME                         ( 1500.0e-6*F_INT) //  1500 usec pause
#define NIKON_FRAME_REPEAT_PAUSE_TIME              (   60.0e-3*F_INT) // frame repeat after 60ms

#define KATHREIN_START_BIT_PULSE_TIME              (  210.0e-6*F_INT) // 1340 usec pulse
#define KATHREIN_START_BIT_PAUSE_TIME              ( 6218.0e-6*F_INT) //  340 usec pause
#define KATHREIN_1_PULSE_TIME                      (  210.0e-6*F_INT) // 1340 usec pulse
#define KATHREIN_1_PAUSE_TIME                      ( 3000.0e-6*F_INT) //  340 usec pause
#define KATHREIN_0_PULSE_TIME                      (  210.0e-6*F_INT) //  500 usec pulse
#define KATHREIN_0_PAUSE_TIME                      ( 1400.0e-6*F_INT) // 1300 usec pause
#define KATHREIN_SYNC_BIT_PAUSE_LEN_TIME           ( 4600.0e-6*F_INT) // 4600 usec sync (on 6th and/or 8th bit)
#define KATHREIN_AUTO_REPETITION_PAUSE_TIME        (   35.0e-3*F_INT) // auto repetition after 35ms
#define KATHREIN_FRAME_REPEAT_PAUSE_TIME           (   35.0e-3*F_INT) // frame repeat after 35ms

#define NETBOX_START_BIT_PULSE_TIME                ( 2400.0e-6*F_INT) // 2400 usec pulse
#define NETBOX_START_BIT_PAUSE_TIME                (  800.0e-6*F_INT) //  800 usec pause
#define NETBOX_PULSE_TIME                          (  800.0e-6*F_INT) //  800 usec pulse
#define NETBOX_PAUSE_TIME                          (  800.0e-6*F_INT) //  800 usec pause
#define NETBOX_AUTO_REPETITION_PAUSE_TIME          (   35.0e-3*F_INT) // auto repetition after 35ms
#define NETBOX_FRAME_REPEAT_PAUSE_TIME             (   35.0e-3*F_INT) // frame repeat after 35ms

#define LEGO_START_BIT_PULSE_TIME                  (  158.0e-6*F_INT) //  158 usec pulse ( 6 x 1/38kHz)
#define LEGO_START_BIT_PAUSE_TIME                  ( 1026.0e-6*F_INT) // 1026 usec pause (39 x 1/38kHz)
#define LEGO_PULSE_TIME                            (  158.0e-6*F_INT) //  158 usec pulse ( 6 x 1/38kHz)
#define LEGO_1_PAUSE_TIME                          (  553.0e-6*F_INT) //  553 usec pause (21 x 1/38kHz)
#define LEGO_0_PAUSE_TIME                          (  263.0e-6*F_INT) //  263 usec pause (10 x 1/38kHz)
#define LEGO_FRAME_REPEAT_PAUSE_TIME               (   40.0e-3*F_INT) // frame repeat after 40ms

#define THOMSON_PULSE_TIME                         (  550.0e-6*F_INT) //  550 usec pulse
#define THOMSON_1_PAUSE_TIME                       ( 4500.0e-6*F_INT) // 4500 usec pause
#define THOMSON_0_PAUSE_TIME                       ( 2000.0e-6*F_INT) // 2000 usec pause
#define THOMSON_AUTO_REPETITION_PAUSE_TIME         (   65.0e-3*F_INT) // repetition after 65ms
#define THOMSON_FRAME_REPEAT_PAUSE_TIME            (   65.0e-3*F_INT) // frame repeat after 65ms

#define LONGBLINK 0x80
#define ERRORBLINK 0x40

#define CMD_NONE               0x00 // Kommando = 0 -> Keine Kommando, nur Dummydaten
#define CMD_IRDATA             0x01 // Kommando = 1 -> IR-Daten senden/ empfangen
#define CMD_COMDATA            0x02 // Kommando = 2 -> serielle Daten vom CommPort senden/ empfangen
#define CMD_SETCOMBAUD         0x03 // Kommando = 3 -> Baudrate CommPort setzen
#define CMD_GETCOMBAUD         0x04 // Kommando = 4 -> Baudrate CommPort abfragen

#define IRMODE_OFF                0 // IR senden und empfangen aus
#define IRMODE_RX                 1 // IR empfangen
#define IRMODE_TX                 2 // IR senden
#define IRMODE_LEARN              3 // ein IR Frame empfangen zum lernen
#define IRMODE_LEARN_FRAMEEND     4 // es wurde ein IR Frame empfangen zum lernen
#define IRMODE_LEARN_LIRC         5 // Daten von lirc übernehmen

#define PULSE_BIT       0x8000
#define PULSE_MASK      0x7FFF

#define CONTROLCMD  (((29952/13)/256)|0x80)
#define C_0         ((29952/13)&0xFF)
#define C_1         ((29965/13)&0xFF)
#define C_2         ((29978/13)&0xFF)
#define C_3         ((29991/13)&0xFF)
#define C_4         ((30004/13)&0xFF)
#define C_5         ((30017/13)&0xFF)
#define C_6         ((30030/13)&0xFF)
#define C_7         ((30043/13)&0xFF)
#define C_8         ((30056/13)&0xFF)
#define C_9         ((30069/13)&0xFF)
#define C_10        ((30082/13)&0xFF)
#define C_11        ((30095/13)&0xFF)
#define C_12        ((30108/13)&0xFF)
#define C_13        ((30121/13)&0xFF)
#define C_14        ((30134/13)&0xFF)
#define C_15        ((30147/13)&0xFF)
#define C_16        ((30160/13)&0xFF)
#define C_END       ((30173/13)&0xFF)
#define C_WATCHDOG  ((30186/13)&0xFF)
#define C_OUTPUT    ((30199/13)&0xFF)
#define C_INPUT     ((30212/13)&0xFF)
#define C_IR        ((30225/13)&0xFF)

// RC5-Codes für Frontswitch und Inputs
#define KEY_F1 0x00
#define KEY_F2 0x01
#define KEY_F3 0x02
#define KEY_F4 0x03
#define KEY_F5 0x04
#define KEY_F6 0x05
#define KEY_F7 0x06
#define KEY_F8 0x07
#define KEY_F9 0x08
#define KEY_F10 0x09
#define KEY_F11 0x0A
#define KEY_F12 0x0B
#define KEY_F13 0x0C
#define KEY_F14 0x0D
#define KEY_F15 0x0E
#define KEY_F16 0x0F
#define IN_1_L 0x10
#define IN_1_H 0x11
#define IN_2_L 0x12
#define IN_2_H 0x13
#define IN_3_L 0x14
#define IN_3_H 0x15
#define IN_4_L 0x16
#define IN_4_H 0x17
#define IN_5_L 0x18
#define IN_5_H 0x19
#define IN_6_L 0x1A
#define IN_6_H 0x1B
#define IN_7_L 0x1C
#define IN_7_H 0x1D
#define IN_8_L 0x1E
#define IN_8_H 0x1F

void vInitIr(BYTE bIrMod);
void vInitTimer0(void);
BOOL yPacketToHost(BYTE yOnlyTest4NewPacket);
void vPacketFromHost(BYTE yLearn);
void vSleep(BYTE b);
void vSleepSec(BYTE b);
void vLED(BYTE bBlink);
BOOL yGetMenuSwitch(void);
void vIrSearch(void);
void vIrOption(void);
BOOL yIrTxByte(WORD w);
void vGenRC5(BYTE bCmd);

#endif // IR_H
