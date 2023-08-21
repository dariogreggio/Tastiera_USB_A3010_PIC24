/********************************************************************
 FileName:      HardwareProfile.h
 Dependencies:  See INCLUDES section
 Processor:     PIC18, PIC24, dsPIC33, or PIC32 USB Microcontrollers
 Hardware:      This demo is natively intended to be used on Microchip USB demo
                boards supported by the MCHPFSUSB stack.  See release notes for
                support matrix.  The firmware may be modified for use on
                other USB platforms by editing this file (HardwareProfile.h)
                and adding a new hardware specific 
                HardwareProfile - [platform name].h file.
 Compiler:      Microchip C18 (for PIC18), C30 (for PIC24/dsPIC33), or C32 (for PIC32)
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC® Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

********************************************************************/

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

//#define DEMO_BOARD USER_DEFINED_BOARD

#if !defined(DEMO_BOARD)
    #if defined(__C32__)
        #if defined(__32MX460F512L__)
            #if defined(PIC32MX460F512L_PIM)		//If this is defined, it is saved in the project and can be found under: Project --> Build Options... --> Project --> MPLAB PIC32 C Compiler --> Preprocessor Macros
                #include "HardwareProfile - PIC32MX460F512L PIM.h"
            #elif defined(PIC32_USB_STARTER_KIT)	//If this is defined, it is saved in the project and can be found under: Project --> Build Options... --> Project --> MPLAB PIC32 C Compiler --> Preprocessor Macros
                #include "HardwareProfile - PIC32 USB Starter Kit.h"
            #endif
        #elif defined(__32MX795F512L__)
            #if defined(PIC32MX795F512L_PIM)
                #include "HardwareProfile - PIC32MX795F512L PIM.h"
            #elif defined(PIC32_USB_STARTER_KIT)
                //PIC32 USB Starter Kit II
                #include "HardwareProfile - PIC32 USB Starter Kit.h"
            #endif
        #endif
    #endif

    #if defined(__C30__) || defined __XC16__
        #if defined(__PIC24FJ256GB110__) || defined(__PIC24FJ256GB106__)
            #include "HardwareProfile - PIC24FJ256GB110 PIM.h"
        #elif defined(__PIC24FJ256GB210__)
            #include "HardwareProfile - PIC24FJ256GB210 PIM.h"
        #elif defined(__PIC24FJ256GB106__)
            #include "HardwareProfile - PIC24F Starter Kit.h"
        #elif defined(__PIC24FJ64GB004__) || defined(__PIC24FJ64GB002__)
            #include "HardwareProfile - PIC24FJ64GB004 PIM.h"
        #elif defined(__PIC24FJ64GB502__)
            #include "HardwareProfile - PIC24FJ64GB502 Microstick.h"
        #elif defined(__PIC24FJ256DA210__)
            #include "HardwareProfile - PIC24FJ256DA210 Development Board.h"
        #elif defined(__dsPIC33EP512MU810__)
            #if defined(DSPIC33EP512MU810_PIM)
                #include "HardwareProfile - dsPIC33EP512MU810 PIM.h"
            #elif defined(DSPIC33E_STARTER_KIT)
                #include "HardwareProfile - dsPIC33E Starter Kit.h"
            #endif
        #elif defined(__PIC24EP512GU810__)
            #if defined(PIC24EP512GU810_PIM)
                #include "HardwareProfile - PIC24EP512GU810 PIM.h"
            #endif
        #endif
    #endif

    #if defined(__18CXX)
        #if defined(__18F4550) || defined(__18F45K50)
            #include "HardwareProfile - PICDEM FSUSB.h"
//            #include "HardwareProfile - PICDEM FSUSB K50.h"
        #elif defined(__18F87J50)
            #include "HardwareProfile - PIC18F87J50 PIM.h"
        #elif defined(__18F97J94)
            #include "HardwareProfile - PIC18F97J94 PIM.h"
        #elif defined(__18F87J94)
            #include "HardwareProfile - PIC18F87J94 PIM.h"
        #elif defined(__18F14K50)
            #include "HardwareProfile - LPC USB Development Kit - PIC18F14K50.h"
        #elif defined(__18F46J50)
            #if defined(PIC18F_STARTER_KIT_1)
                #include "HardwareProfile - PIC18F Starter Kit 1.h"
            #else
                #include "HardwareProfile - PIC18F46J50 PIM.h"
            #endif
        #elif defined(__18F47J53)
            #include "HardwareProfile - PIC18F47J53 PIM.h"
        #endif
    #endif

    #if defined(_PIC14E)
        #if defined (_16F1459)
    		#include "HardwareProfile - LPC USB Development Kit - PIC16F1459.h"
        #endif
    #endif
#endif

#if !defined(DEMO_BOARD)
    #error "Demo board not defined.  Either define DEMO_BOARD for a custom board or select the correct processor for the demo board."
#endif

#include "GenericTypedefs.h"
#include "stdlib.h"


#ifdef __C30__
#define ROM const
#else
#define ROM rom
#endif


#define SERNUM      1000
#define VERNUMH     2
#define VERNUML     4


//#define KTRONIC			// cambia stringa Ditta ;) 2023; v. anche vecchi pcb e debug in giro
#define USA_BUZZER 1
#define MAX_LAYOUT 4			// usato per incr/toggle layout in CODE-248; obbligatorio 1,2 o 4!
#define MAX_TASTI_CONTEMPORANEI 4
//#define USA_CHIAVE 1
//#define USA_CHIAVE_TOGGLE 1		                      // usare INSIEME a USA_CHIAVE! e a USA_CHIAVE_TOGGLE_STATO_INIZIALE settandola come serve
//#define USA_CHIAVE_TOGGLE_STATO_INIZIALE 0	          // PROVATO OK //  // usare INSIEME a USA_CHIAVE_TOGGLE! se a 0 tastiera parte attiva se a 1 parte disattiva ; LED ATTIVI BASSI!
//#define CONTROLLA_PHANTOM 1                         // NON PROVATO //
//#define CONTROLLA_PHANTOM_7 1                       // NON PROVATO // 
//#define NO_AUTOREPEAT_OLTRE_8_COLONNA 1             // NON PROVATO //
//#define USA_SLAVE 1                                 // NON PROVATO //
//#define LAYOUT_TEMP 1                               // v. fidia 11/05: la pressione di un tasto FUORI MATRICE cambia layout temporaneamente
#define USA_AUTOREPEAT 1
//#define NO_CTRLALTCANC_CTRLSHFTESC 1             // NON PROVATO //
//#define NO_ALTTAB_ALTESC_WIN 1
//#define ABILITA_WRITE 1
//#define CHIAVE_LED_IS_LAYOUT_LED 1

//#define TUTTI_CONTRO_TUTTI 1			// tastiera "a cazzo" 2010!
//#define USA_TASTIERA_21PIN 1

//#define LARIMART_NUMLOCK_LAYOUT 1			

//#define MODALITA_GILARDONI 1
//#define MODALITA_APROVADIPICIU 1
#define MODALITA_A3010 1			// Acorn A3010, 2023 da facebook; v. file arduino; https://retrorepairsandrefurbs.com/2021/09/16/acorn-archimedes-3000-repair-restoration/

//#define TEST_LED 1

//#define SECCO_MATRICE 1

#ifndef __C30__
//#define USA_EEPROM 1 						//altrimenti funziona con layout fisso, come tastiere vecchie
//#define USA_EEPROM_INTERNA 1 				//NO su 24 "normali"!!
#endif



//#define NUM_CAPS_SCROLL_SCAMBIATI 1

// #define MULTIMEDIA_KEYBOARD 1 METTERLO in Build Options!

#ifdef MODALITA_APROVADIPICIU 
#define NO_CTRLALTCANC_CTRLSHFTESC 1             
#define NO_ALTTAB_ALTESC_WIN 1
#endif

#ifdef TUTTI_CONTRO_TUTTI
#if defined(CONTROLLA_PHANTOM) || defined(CONTROLLA_PHANTOM_7) || defined(NO_AUTOREPEAT_OLTRE_8_COLONNA) || defined (NO_CTRLALTCANC_CTRLSHFTESC) || defined(NO_ALTTAB_ALTESC_WIN)
#error Impossibile definire Blocchi in modalità TUTTI
#endif
#endif

#ifdef MODALITA_A3010
#define MAX_LAYOUT 1
#endif


#ifdef USA_EEPROM_INTERNA 
#if MAX_LAYOUT>1
#undef MAX_LAYOUT 
#define MAX_LAYOUT 1
#warning Su EEprom interna, è disponibile un solo layout
#endif
#endif


#if !defined(USA_FULLSPEED) && !defined(USA_LOWSPEED)
//#error #definire USA_FULLSPEED o USA_LOWSPEED nel progetto!
//tolto per ora...
#warning #definire USA_FULLSPEED o USA_LOWSPEED nel progetto!
#endif

extern const ROM BYTE ascii_2_usb[];

extern const ROM BYTE * ROM KB_layout0[];


#ifdef __C30__
#define Delay_ms(n) __delay_ms(n)
#define Delay_uS(n) __delay_us(n)
#define Delay_S_(n) __delay_ms(n*500L)
#else
void Delay_ms(unsigned char);
void Delay_uS(unsigned char);
void Delay_S_(unsigned char);
#endif
#define Delay_SPI() Delay_uS(2)
#define Delay_1uS() Delay_uS(1)

#ifndef __C30__			// dovrebbe essere in stdlib come su C30 ma invece...
#define min(a,b) ((a)>=(b) ? (a) : (b))
#endif

void prepOutBuffer(BYTE );
void clearOutBuffer(void);
void prepStatusBuffer(BYTE);
void sendEP1(void);
void sendEP2(void);
void sendEP1_mmedia(void);

signed char loadKBlayout(void);
signed char saveKBlayout(void);
signed char presetKBlayout(void);
signed char loadKBdefaults(void);
signed char saveKBdefaults(void);
void resetKBdefaults(void);

void EEscrivi(BYTE ,BYTE );
BYTE EEleggi(BYTE );

//#define SCANCNT_K_DEFAULT 50			// default rate scan/sec tastiera
#define SCANCNT_EVERY_2_IDLE_DEFAULT 8	// ora fa una scan tastiera ogni 70mS, l'idle dev'essere ogni 500mSec


struct CONFIG_TASTIERA {
	BYTE signature;		// 0x47
	BYTE Options_Mode1;
	BYTE autorepeat;
	BYTE buzzer;
	BYTE maxLayout;
	BYTE maxTastiContemporanei;
	BYTE gestisciPhantom;
	BYTE noCtrlAltCanc,noCtrlShiftEsc;
	BYTE noAltTab,noAltEsc;
	BYTE chiave,chiaveLed,chiaveToggle,chiaveToggleIniziale;
	BYTE numlockIsLayout;
	BYTE scanK;
	BYTE noWrite;
	BYTE isMultimedia; // ah ah :)
	BYTE NumCapsScrollScambiati;
	BYTE retroIlluminazione,retroIlluminazione2;
	BYTE zonaMortaTrackball[2];
	BYTE memorySize;			// in 256bytes pages
	};		
/*La lettura dell'indirizzo 0:

0x01 0x09 0x20 0x00 0x00 0x00 0x00 0x00 

0x47 0x00 0x01 0x01 0x04 0x04 0x00 0x00 
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 
0x14 0x00 0x00 0x00 0x88 0x00 0x00 0x00 
0xFF 0x00 0x00 0x00 0x00 0x00 0x00 0xFF

Lettura get_config
0x01 0x03 0x00 0x00 0x00 0x00 0x00 0x00 
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x20 
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 
0x00 0x01 0xDF 0x40 0xFE 0x84 0x92 0x25 
0x00 0x42 0x37 0x8A 0x82 0x84 0x94 0x8C 
0x25 0x42 0x2B 0x00 0x00 0x14 0x00 0x00 
0x40 0x96 0x80 0x00 0x00 0x00 0x00 0x00
*/

struct KB_LAYOUT_HEADER {
	BYTE signature;		// 'GD
	BYTE modello;			// v.kblayout, numero ID
	BYTE numTasti;
	BYTE numLayout;		// riportare a consilium...
	};
#define RESERVED_EEPROM 32			// su EEprom interna, 256, vogliamo 2 layout da 110 + 32... 252 + header=4...
//#if RESERVED_EEPROM<(sizeof(struct CONFIG_TASTIERA))		dà errore...
//#error manca spazio per valori default tastiera!
//#endif


//USB cmd #
enum {
	CMD_NOP=0,
	CMD_GETID=1,
	CMD_GETCAPABILITIES=2,
	CMD_GETCONFIG=3,
	CMD_SETCONFIG=4,
	CMD_GETSTATUS,
	CMD_SETSTATUS,
	CMD_PROGMEM=8,
	CMD_READMEM,
	CMD_BEEP=11,
	CMD_SETLED=15,
	CMD_TEST=16,
	CMD_KEYMODE,
	CMD_WRITEEEPROM=24,
	CMD_READEEPROM,
	CMD_RESETEEPROM,
	CMD_ZAPEEPROM,
	CMD_WRITERTC=28,
	CMD_READRTC,

	CMD_RESET_IF=30,
	CMD_DEBUG_ECHO=31,
	CMD_SETLEDEXT=32,
	CMD_READ_IDLOCS=63
	};

/*
; MANUFACTURER_ID=0x115f (ADPM) = 4447; GC = 10Fb

; PRODUCT_IDs
// 0x101=tastiera Mac-Ktronic
// 0x102=tastiera/mouse Ktronic
// 0x103=mouse Interlink (anche PS/2 autosensing) Ktronic
// 0x104=tastiera 6 tasti Ktronic e anche Remote Keyboard ADPM; e Consilium
// 0x105=Terminale USB Ktronic 
// 0x106=tastiera Logitech-Ktronic Multimediale
// 0x107=tastiera Ktronic con canale dati per riconfigurazione ecc.
// 0x108=Composite device (MSC+CDC)KT-ED12 Boagno
// 0x109=Composite device (MSC+CDC)KT-ED11 Boagno
// 0x10a=Tester generico KTronic
// 0x10b=tastiera capacitiva 
// 0x10c=tastiera IR

// 0x54=demo MSD HID (geco)

// 0x201=Skynet USB RS485
// 0x202=Skynet USB Onde convogliate
// 0x203=Skynet USB Wireless CC900
// 0x204=Skynet USB Wireless Aurel
// 0x20a=Skynet USB simil-ethernet
// 0x210=SkyPIC USB per Eprom tradizionali/parallele/PIC
// 0x220=USB_RELE scheda I/O USB
// 0x230=PELUCHEUSB Giocattolo con MP3/musica, e anche GECO
// 0x231=USBSTEP per pannelli solari
// 0x240=Analizzatore logico USB
// 0x241=USBPower Alimentatore; anche BTAGallerini
// 0x250=VGA clock
// 0x251=TettoRGB e led rgb Rudi
// 0x252=LivioPizzuti's device
// 0x253=USBTime
// 0x254=USBRadio
// 0x255=Oriol's device
// 0x256=Geco tuning device
// 0x257=Manjeevan's device
// 0x258=joystick cockpit auto
// 0x259=joystick + keyboard nicola mogicato insta 2021
// 0x260=GSMDevice
// 0x261=Geiger (MSD + HID)
// 0x262=USBthermo (MSD + HID); anche bluetooth ktronic
// 0x263=Caricabatteria Pb(MSD + HID)

*/

#endif  //HARDWARE_PROFILE_H
