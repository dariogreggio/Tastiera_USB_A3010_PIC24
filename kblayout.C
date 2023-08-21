// VERSIONE CON PIC 18F4550 CON MATRICE PER tastiera KT-105-XXX     MONTATO SU KT-ED8-A         05-02-2013         

//------------------------------------------------------------------------------------------------------
//user.h


//ATTIVA      #define USA_BUZZER 1
//ATTIVA      #define MAX_LAYOUT 2			// usato per incr/toggle layout in CODE-248; obbligatorio 1,2 o 4!
//ATTIVA      #define MAX_TASTI_CONTEMPORANEI 4
//DISATTIVA   #define USA_CHIAVE 1
//DISATTIVA   #define USA_CHIAVE_TOGGLE 1		                      // usare INSIEME a USA_CHIAVE! e a USA_CHIAVE_TOGGLE_STATO_INIZIALE settandola come serve
//DISATTIVA   #define USA_CHIAVE_TOGGLE_STATO_INIZIALE 0	          // PROVATO OK //  // usare INSIEME a USA_CHIAVE_TOGGLE! se a 0 tastiera parte attiva se a 1 parte disattiva
//DISATTIVA   #define CONTROLLA_PHANTOM 1                         // NON PROVATO //
//DISATTIVA   #define CONTROLLA_PHANTOM_7 1                       // NON PROVATO // 
//DISATTIVA   #define NO_AUTOREPEAT_OLTRE_8_COLONNA 1             // NON PROVATO //
//DISATTIVA   #define USA_SLAVE 1                                 // NON PROVATO //
//DISATTIVA   #define LAYOUT_TEMP 1                               // v. fidia 11/05: la pressione di un tasto FUORI MATRICE cambia layout temporaneamente
//ATTIVA      #define USA_AUTOREPEAT 1
//DISATTIVA   #define NO_CTRLALTCANC_CTRLSHFTESC 1             // NON PROVATO //
//DISATTIVA   #define NO_ALTTAB_ALTESC_WIN 1
//DISATTIVA   #define ABILITA_WRITE 1
//DISATTIVA   #define CHIAVE_LED_IS_LAYOUT_LED 1

//DISATTIVA   #define TUTTI_CONTRO_TUTTI 1			// tastiera "a cazzo" 2010!

//DISATTIVA   #define LARIMART_NUMLOCK_LAYOUT 1			

//DISATTIVA   #define MODALITA_GILARDONI 1
//DISATTIVA   #define MODALITA_APROVADIPICIU 1


//DISATTIVA   #define NUM_CAPS_SCROLL_SCAMBIATI 1

//DISATTIVA   #define MULTIMEDIA_KEYBOARD 1 METTERLO in Build Options!







//------------------------------------------------------------------------------------------------------




/*********************************************************************
 *
 *                Microchip USB C18 Firmware - tastiera USB con Stack 2.0
 *
 *********************************************************************
 * FileName:        kblayout.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 3.47
 * Company:         Microchip Technology, Inc.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;    	Author:			Dario Greggio & Matteo Vallone
;		1/8/01: inizio
;		2/02: layout Mac
;		3/02: modificati nomi delle label tasti
;		6/05: versione C/Flash Pic18
;   09: aggiunte tasti cambia-layout al volo
;		14: versione Pic18 Stack 2.0, anche PIC24; secondo canale HID dati per riprogrammazione
;		4/15: gestione "poca" EEprom (interna)
;		11/15: gestione EEprom esterna
 ********************************************************************/

/** I N C L U D E S **********************************************************/
//#include <p18cxxx.h>
//#include <delays.h>
#include "usb.h"
#include "HardwareProfile.h"

//#include "user\user.h"
#ifdef __C30__
#include <libpic30.h>
#endif

#include "swi2c.h"



/** V A R I A B L E S ********************************************************/


#ifndef __C30__
#pragma udata
#endif

extern BYTE kbLayout;
const ROM BYTE idTastiera=1;			// CAMBIARE a OGNI MODELLO!


struct KB_LAYOUT_HEADER kbLayoutHeader;
#define MAX_BYTE_PER_TASTO 8
#ifdef USA_EEPROM

#ifdef USA_EEPROM_INTERNA
BYTE kbptr;
BYTE kbptr_prog;

#else
WORD kbptr,kblayoutptr[MAX_LAYOUT];
BYTE kbptr_prog,kbptr_prog_max;

#endif

#else

ROM BYTE *kbptr;
#endif

#pragma udata

#ifndef __C30__
#pragma romdata
#endif



//ROM BYTE tastoDummy=0;		// usato all'inizio (perche' ScanCode parte da 1) e per i tasti non-esistenti

// convenzione usata:

// PROVATO OK   //		da 1 a 239 e' un tasto semplice, che genera make/break code alla pressione/rilascio      *** AUTOREPEAT ***

// PROVATO OK   //		240 indica che si tratta di una combinazione di tasti, come se fossero premuti tutti insieme (press.1, press.2.. ril.2, ril.1).
                //            I tasti sono indicati uno dopo l'altro, fino allo 00h      *** NO AUTOREPEAT ***

// PROVATO  //		241 indica che si tratta di una combinazione di tasti, come se fossero digitati in sequenza (press/ril.1, press/ril.2, ..).
                //            I tasti sono indicati uno dopo l'altro, fino allo 00h   *** NO AUTOREPEAT***

// NON PROVATO  //		242	è come 241 ma con i codici ASCII anziché gli scan-code (spazio ROM permettendo!) 

// NON PROVATO  //		243 indica che si tratta di un tasto multiplo: un tasto più un eventuale modifier

// PROVATO OK   //		244 indica che il dovrà essere emesso il primo code che segue se NON è premuto ALT-GR,
                //            altrimenti il secondo (eventualmente preceduto da UN modificatore) 

// NON PROVATO  //		245 indica un tasto con codici Make/Break completamente diversi  
                //            (seguono il Make-Code, terminato con 00, e il Break-Code, terminato con 00; entrambi possono non esserci (v. tasto Pause))

// NON PROVATO  //		246 usare per le varianti a Pause/PrtSc quando premi insieme a CTRL/ALT ecc?? o estendere 245??

// PROVATO OK   //		247 = come 240 con autorepeat! (2005 - Fidia)             *** AUTOREPEAT ***

// PROVATO  //		248	cambia il layout nel #layout identificato dal BYTE che segue (0..3);
                //            eventualmente prevedere dei flag per avere il "cambia layout" solo con SHIFT o ALT.
// PROVATO  //	        se il codice che segue è 255 (TOGGLE), il layout viene incrementato e fatto modulo con  MAX_LAYOUT-1 
// 						//	        se il codice che segue è 254 (TEMP), il layout viene cambiato a 0 solo mentre il tasto è premuto
   						//	        se il codice che segue è 253 o 252 o 251, il layout viene cambiato a 2 (3, 4) solo mentre il tasto è premuto

// PROVATO  //		0		e' un tasto non usato.

// PROVATO  7.2015 //		250	manda i tasti che seguono al canale/HID report 2 o 3 - Multimedia
//http://www.microchip.com/forums/m618147.aspx
//http://www.microchip.com/forums/m391162.aspx

// NON PROVATO  //		255 e' riservato (tasto non usato)


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ROM BYTE tastoDummy=0;

ROM BYTE tastoLTOGLE[]={248,255};
ROM BYTE tastoLTEMP[]={248,254};
ROM BYTE tastoL0[]={248,0};
ROM BYTE tastoL1[]={248,1};
ROM BYTE tastoL2[]={248,2};
ROM BYTE tastoL3[]={248,3};
//matteo  16-09-2009
ROM BYTE tastoLTEMP1[]={248,253};
ROM BYTE tastoLTEMP2[]={248,252};
ROM BYTE tastoLTEMP3[]={248,251};
//matteo  16-09-2009


// TASTI SPECIALI DA VERIFICARE


//ROM BYTE tastoWWW[]={241,0x1D,0x1D,0x1D,0x49,0};

//ROM BYTE tastoCAC[]={240,0x14,0x11,0xE0,0x71,0};

//ROM BYTE tastoSAR[]={243,0xE0,0x11,0x52,0};

//ROM BYTE tastoSA[]={247,0xE0,0x11,0x4C,0};

//ROM BYTE tastoSAL[]={240,0xE0,0x11,0x52,0};

//ROM BYTE tastoMM[]={244,0x41,0xE0,0x11,0x24,0};


// TASTI SPECIALI QWERTY


//ROM BYTE tastoWWW[]={241,0x1D,0x1D,0x1D,0x49,0};

//ROM BYTE tastoIT[]={241,0x49,0x43,0x2c,0};

//ROM BYTE tastoCOM[]={241,0x49,0x21,0x44,0x3a,0};

//ROM BYTE tastoDE[]={241,0x49,0x23,0x24,0};

//ROM BYTE tastoES[]={241,0x49,0x24,0x1b,0};

//ROM BYTE tastoNET[]={241,0x49,0x31,0x24,0x2c,0};

//ROM BYTE tastoUK[]={241,0x49,0x3c,0x42,0};

//ROM BYTE tastoHPAGE[]={240,0x11,0xE0,0x6c,0};

//ROM BYTE tastoCHIOCCIOLA[]={240,0xe0,0x11,0x4c,0};



// TASTI SPECIALI AZERTY
// IL CARATTERE 0X41 IN QUESTI TASTI SPECIALI VA CON LO SHIFT-SX



ROM BYTE tastoWWW[]={241,0x1A,0x1A,0x1A,240,0x12,0x41,0,0};

ROM BYTE tastoIT[]={241,240,0x12,0x41,0,0x43,0x2C,0};

ROM BYTE tastoCOM[]={241,240,0x12,0x41,0,0x21,0x44,0x4C,0};

ROM BYTE tastoDE[]={241,240,0x12,0x41,0,0x23,0x24,0};

ROM BYTE tastoES[]={241,240,0x12,0x41,0,0x24,0x1b,0};

ROM BYTE tastoNET[]={241,240,0x12,0x41,0,0x31,0x24,0x2c,0};

ROM BYTE tastoUK[]={241,240,0x12,0x41,0,0x3c,0x42,0};

ROM BYTE tastoHPAGE[]={240,0x11,0xE0,0x6c,0};

ROM BYTE tastoCHIOCCIOLA[]={240,0xe0,0x11,0x45,0};

#ifdef MULTIMEDIA_KEYBOARD
ROM BYTE tastoVolumePiu[]=			{250,0x02,0x02,0x00,0};		//2015 NON Posso usare lo zero come terminatore.. quindi li prendo tutti e 3 sempre
ROM BYTE tastoVolumeMeno[]=			{250,0x02,0x04,0x00,0};
ROM BYTE tastoVolumeMute[]=			{250,0x02,0x01,0x00,0};
ROM BYTE tastoMediaPlayer[]=		{250,0x02,0x08,0x00,0};			// PROVARE! (sembra fare start/stop...)
ROM BYTE tastoStart[]=					{250,0x02,0x08,0x00,0};
ROM BYTE tastoStop[]=						{250,0x02,0x10,0x00,0};
ROM BYTE tastoScanNext[]=				{250,0x02,0x20,0x00,0};
ROM BYTE tastoScanPrev[]=				{250,0x02,0x40,0x00,0};
ROM BYTE tastoEMail[]=					{250,0x02,0x80,0x00,0};
ROM BYTE tastoCalcolatrice[]=		{250,0x02,0x00,0x01,0};
ROM BYTE tastoSearch[]=					{250,0x02,0x00,0x02,0};
ROM BYTE tastoBrowser[]=				{250,0x02,0x00,0x04,0};
ROM BYTE tastoFavorite[]=				{250,0x02,0x00,0x08,0};
ROM BYTE tastoRefresh[]=				{250,0x02,0x00,0x10,0};
ROM BYTE tastoBrowserStop[]=		{250,0x02,0x00,0x20,0};
ROM BYTE tastoBrowserForward[]=	{250,0x02,0x00,0x40,0};
ROM BYTE tastoBrowserBack[]=		{250,0x02,0x00,0x80,0};
ROM BYTE tastoPower[]=					{250,0x03,0x01,0x00,0};
ROM BYTE tastoSleep[]=					{250,0x03,0x02,0x00,0};
ROM BYTE tastoWakeup[]=					{250,0x03,0x04,0x00,0};
#endif

ROM BYTE tastoALT1[]={240,0xe2,0x1e,0};
ROM BYTE tastoALT2[]={240,0xe2,0x1f,0};
ROM BYTE tastoALT3[]={240,0xe2,0x20,0};
ROM BYTE tastoALT4[]={240,0xe2,0x21,0};
ROM BYTE tastoALT5[]={240,0xe2,0x22,0};
ROM BYTE tastoALT6[]={240,0xe2,0x23,0};
ROM BYTE tastoALT7[]={240,0xe2,0x24,0};
ROM BYTE tastoALT8[]={240,0xe2,0x25,0};
ROM BYTE tastoALT9[]={240,0xe2,0x26,0};
ROM BYTE tastoALT0[]={240,0xe2,0x27,0};

ROM BYTE tastoALT_FSU[]={247,0xe2,0x52,0};
ROM BYTE tastoALT_FGIU[]={247,0xe2,0x51,0};
ROM BYTE tastoALT_FSX[]={247,0xe2,0x50,0};
ROM BYTE tastoALT_FDX[]={247,0xe2,0x4f,0};




ROM BYTE tasto_L_SHIFT_F1[]={247,0xe1,0x3a,0};
ROM BYTE tasto_L_SHIFT_F2[]={247,0xe1,0x3b,0};
ROM BYTE tasto_L_SHIFT_F3[]={247,0xe1,0x3c,0};
ROM BYTE tasto_L_SHIFT_F4[]={247,0xe1,0x3d,0};
ROM BYTE tasto_L_SHIFT_F5[]={247,0xe1,0x3e,0};
ROM BYTE tasto_L_SHIFT_F6[]={247,0xe1,0x3f,0};
ROM BYTE tasto_L_SHIFT_F7[]={247,0xe1,0x40,0};
ROM BYTE tasto_L_SHIFT_F8[]={247,0xe1,0x41,0};
ROM BYTE tasto_L_SHIFT_F9[]={247,0xe1,0x42,0};
ROM BYTE tasto_L_SHIFT_F10[]={247,0xe1,0x43,0};
ROM BYTE tasto_L_SHIFT_F11[]={247,0xe1,0x44,0};
ROM BYTE tasto_L_SHIFT_F12[]={247,0xe1,0x45,0};


ROM BYTE tasto_L_CTRL_F1[]={247,0xe0,0x3a,0};
ROM BYTE tasto_L_CTRL_F2[]={247,0xe0,0x3b,0};
ROM BYTE tasto_L_CTRL_F3[]={247,0xe0,0x3c,0};
ROM BYTE tasto_L_CTRL_F4[]={247,0xe0,0x3d,0};
ROM BYTE tasto_L_CTRL_F5[]={247,0xe0,0x3e,0};
ROM BYTE tasto_L_CTRL_F6[]={247,0xe0,0x3f,0};
ROM BYTE tasto_L_CTRL_F7[]={247,0xe0,0x40,0};
ROM BYTE tasto_L_CTRL_F8[]={247,0xe0,0x41,0};
ROM BYTE tasto_L_CTRL_F9[]={247,0xe0,0x42,0};
ROM BYTE tasto_L_CTRL_F10[]={247,0xe0,0x43,0};
ROM BYTE tasto_L_CTRL_F11[]={247,0xe0,0x44,0};
ROM BYTE tasto_L_CTRL_F12[]={247,0xe0,0x45,0};




ROM BYTE tastoSHIFT_NOVE_USA[]={247,0xe1,0x26,0};
ROM BYTE tastoSHIFT_ZERO_USA[]={247,0xe1,0x27,0};
ROM BYTE tastoSHIFT_SETTE_USA[]={247,0xe1,0x24,0};
ROM BYTE tastoSHIFT_QUATTRO_USA[]={247,0xe1,0x21,0};
ROM BYTE tastoSHIFT_TRATTINI_USA[]={247,0xe1,0x38,0};
ROM BYTE tastoSHIFT_DUE_USA[]={247,0xe1,0x1f,0};
ROM BYTE tastoSHIFT_CINQUE_USA[]={247,0xe1,0x22,0};
ROM BYTE tastoSHIFT_UNO_USA[]={247,0xe1,0x1e,0};
ROM BYTE tastoSHIFT_EACC_USA[]={247,0xe1,0x2f,0};
ROM BYTE tastoSHIFT_PIU_USA[]={247,0xe1,0x30,0};
ROM BYTE tastoSHIFT_TRE_USA[]={247,0xe1,0x20,0};
ROM BYTE tastoSHIFT_AACC_USA[]={247,0xe1,0x34,0};
ROM BYTE tastoSHIFT_VIRGOLA_USA[]={247,0xe1,0x36,0};
ROM BYTE tastoSHIFT_PUNTO_USA[]={247,0xe1,0x37,0};
ROM BYTE tastoSHIFT_BACKSLASH_USA[]={247,0xe1,0x35,0};
ROM BYTE tastoSHIFT_SEI_USA[]={247,0xe1,0x23,0};
ROM BYTE tastoSHIFT_UACC_USA[]={247,0xe1,0x31,0};
ROM BYTE tastoSHIFT_OACC_USA[]={247,0xe1,0x33,0};
ROM BYTE tastoALTGR_CINQUE_USA[]={247,0xe6,0x22,0};
ROM BYTE tastoALTGR_SHIFT_OACC_USA[]={247,0xe6,0xe1,0x33,0};


ROM BYTE tastoCHIOCCIOLA_ITA[]={240,0xe6,0x33,0};
ROM BYTE tastoCANCELLETTO_ITA[]={240,0xe6,0x34,0};
ROM BYTE tastoPUNTO_VIRGOLA_ITA[]={240,0xe1,0x36,0};
ROM BYTE tastoAPRI_TONDA_ITA[]={240,0xe1,0x25,0};
ROM BYTE tastoCHIUDI_TONDA_ITA[]={240,0xe1,0x26,0};
ROM BYTE tastoDUE_PUNTI_ITA[]={240,0xe1,0x37,0};

ROM BYTE tasto_L_SHIFT_TAB[]={240,0xe1,0x2b,0};
// ROM BYTE tasto_L_SHIFT_TAB[]={247,0xe1,0x2b,0};


ROM BYTE tastoSHIFTVIRGOLA[]={247,0xe1,0x36,0};

ROM BYTE tastoSHIFTPUNTO[]={247,0xe1,0x37,0};

ROM BYTE tastoSHIFTTRATTINI[]={247,0xe1,0x38,0};

ROM BYTE tastoSHIFTZERO[]={247,0xe1,0x27,0};

ROM BYTE tastoMIN1[]={244,0x36,0x64,0};          // SU VIRGOLA
ROM BYTE tastoMAG1[]={244,0x37,0xE1,0x64,0};     // SU PUNTO

ROM BYTE tastoMIN2[]={244,0x37,0x64,0};          // SU PUNTO
ROM BYTE tastoMAG2[]={244,0x38,0xE1,0x64,0};     // SU TRATTINI





ROM BYTE tastoF1=0x3a;      //F1
ROM BYTE tastoF2=0x3b;      //F2
ROM BYTE tastoF3=0x3c;      //F3
ROM BYTE tastoF4=0x3d;      //F4
ROM BYTE tastoF5=0x3e;	    //F5
ROM BYTE tastoF6=0x3f;      //F6
ROM BYTE tastoF7=0x40;      //F7
ROM BYTE tastoF8=0x41;      //F8
ROM BYTE tastoF9=0x42;      //F9
ROM BYTE tastoF10=0x43;	    //F10
ROM BYTE tastoF11=0x44;     //F11
ROM BYTE tastoF12=0x45;     //F12


ROM BYTE tasto1=0x1e;	  	//1
ROM BYTE tasto2=0x1f;  		//2
ROM BYTE tasto3=0x20; 		//3
ROM BYTE tasto4=0x21; 		//4
ROM BYTE tasto5=0x22; 		//5
ROM BYTE tasto6=0x23; 		//6
ROM BYTE tasto7=0x24; 		//7
ROM BYTE tasto8=0x25; 		//8
ROM BYTE tasto9=0x26; 		//9
ROM BYTE tasto0=0x27; 		//0


ROM BYTE tastoA=0x04;	  	//A
ROM BYTE tastoB=0x05;	  	//B
ROM BYTE tastoC=0x06;	  	//C
ROM BYTE tastoD=0x07;	  	//D
ROM BYTE tastoE=0x08;			//E
ROM BYTE tastoF=0x09;	  	//F
ROM BYTE tastoG=0x0a;	  	//G
ROM BYTE tastoH=0x0b;	  	//H
ROM BYTE tastoI=0x0c;	  	//I
ROM BYTE tastoJ=0x0d;	  	//J
ROM BYTE tastoK=0x0e;	  	//K
ROM BYTE tastoL=0x0f;	  	//L
ROM BYTE tastoM=0x10;	  	//M
ROM BYTE tastoN=0x11;	  	//N
ROM BYTE tastoO=0x12;	  	//O
ROM BYTE tastoP=0x13;		//P
ROM BYTE tastoQ=0x14;		//Q
ROM BYTE tastoR=0x15;	  	//R
ROM BYTE tastoS=0x16;	  	//S
ROM BYTE tastoT=0x17;	  	//T
ROM BYTE tastoU=0x18;	  	//U
ROM BYTE tastoV=0x19;	  	//V
ROM BYTE tastoW=0x1a;		//W
ROM BYTE tastoX=0x1b;	  	//X
ROM BYTE tastoY=0x1c;	  	//Y
ROM BYTE tastoZ=0x1d;	  	//Z


ROM BYTE tastoESC=0x29;	    	//ESC
ROM BYTE tastoTAB=0x2b;	    	//TAB
ROM BYTE tastoSpazio=0x2c;		// SPACE
ROM BYTE tastoEnter=0x28;	  	//ENTER
ROM BYTE tastoBackSpace=0x2a;	//BKSPC


ROM BYTE tastoMinore=0x64;		// < >


ROM BYTE tastoCapsLock=0x39;    //CAPS LOCK
ROM BYTE tastoNumLock=0x53;	    //NUM LOCK
ROM BYTE tastoScrollLock=0x47;  //SCROLL


ROM BYTE tasto1Num=0x59;	            	//1 Tast
ROM BYTE tasto2Num=0x5a;            		//2 Tast
ROM BYTE tasto3Num=0x5b;            		//3 Tast
ROM BYTE tasto4Num=0x5c;	            	//4 Tast
ROM BYTE tasto5Num=0x5d;	            	//5 Tast
ROM BYTE tasto6Num=0x5e;	            	//6 Tast
ROM BYTE tasto7Num=0x5f;	            	//7 Tast
ROM BYTE tasto8Num=0x60;            		//8 Tast
ROM BYTE tasto9Num=0x61;	             	//9 Tast
ROM BYTE tasto0Num=0x62;                    //0 Tast 
ROM BYTE tastoPuntoNum=0x63;	        	//. Tast.
ROM BYTE tastoSlashNum=0x54;            	/// Tast
ROM BYTE tastoAsteriscoNum=0x55;    		//* Tast
ROM BYTE tastoMenoNum=0x56;	             	//- Tast
ROM BYTE tastoPiuNum=0x57;	             	//+ Tast
ROM BYTE tastoEnterNum=0x58;        		//ENTER Tast.


ROM BYTE tastoFrecciaSu=0x52;       		//U ARROW			
ROM BYTE tastoFrecciaDX=0x4f;      		//R ARROW
ROM BYTE tastoFrecciaGiu=0x51;      		//D ARROW
ROM BYTE tastoFrecciaSX=0x50;	        	//L ARROW


ROM BYTE tastoLCtrl=0xe0;		            //L CTRL
ROM BYTE tastoLShift=0xe1;	            	// L SHFT
ROM BYTE tastoLAlt=0xe2;	              	//L ALT
ROM BYTE tastoRCtrl=0xe4;             		//R CTRL
ROM BYTE tastoRShift=0xe5;            		//R SHIFT
ROM BYTE tastoAltGr=0xe6;         	     	//R ALT GR
ROM BYTE tastoLShiftCond=0xe8;           	// L SHFT condizionale basato su CAPS-LOCK (francia 2007)

ROM BYTE tastoPOWER=0x66;           	  	//POWER
ROM BYTE tastoLWindows=0xe3;        	  	//L WINDOWS
ROM BYTE tastoRWindows=0xe7;	          	// R WINDOWS
ROM BYTE tastoApps=0x65;	              	//tasto destro mouse NO su MAC (stesso di L-WIN = APPLICATIONS)


ROM BYTE tastoPrtSc=0x46;	               	//PrtSc
ROM BYTE tastoPause=0x48;             		//PAUSE

	
ROM BYTE tastoIns=0x49;          	  	//INS
ROM BYTE tastoHome=0x4a;	        	//HOME
ROM BYTE tastoPgUp=0x4b;      	    	//PGUP
ROM BYTE tastoCanc=0x4c;	            //CANC
ROM BYTE tastoEnd=0x4d;	             	//END
ROM BYTE tastoPgDn=0x4e;	            //PGDN
	
	
ROM BYTE tastoAACC=0x34;	        	// à ° ITALIANA
ROM BYTE tastoEACC=0x2f;	        	// è é ITALIANA 
ROM BYTE tastoIACC=0x2e;   	        	// ì	^ ITALIANA
ROM BYTE tastoOACC=0x33;        		// ò ç ITALIANA
ROM BYTE tastoUACC=0x31;	        	// ù § ITALIANA
ROM BYTE tastoAPOSTROFO=0x2d;   		// ' ? ITALIANA
ROM BYTE tastoPIU=0x30;	            	// + * ITALIANA
ROM BYTE tastoBACKSLASH=0x35;	    	// \ | ITALIANA  
ROM BYTE tastoVIRGOLA=0x36;		        // , ; ITALIANA
ROM BYTE tastoPUNTO=0x37;	       		// . : ITALIANA
ROM BYTE tastoTRATTINI=0x38;		    // - _ ITALIANA

//ROM BYTE tastoVOLPIU=0xAF;	       	// volume + multimedia NON VA! v. 2015
//ROM BYTE tastoVOLMENO=0xAE;	       	// volume - multimedia NON VA!




#ifndef __C30__
#pragma romdata KBLAYOUTS=0x6000
#else
// mettere eqv per PIC24 o anche no tanto non andiamo in Flash...
#endif


#ifndef MODALITA_A3010
const ROM BYTE * ROM KB_layout0[]= {
	&tastoDummy,			// serve xche' kbScanCode parte da 1...

		
// LAYOUT 2015, flf (...) 5.11.2015
// qua c'era bestemmione meritatosi dall'Essere che non esiste in data 4.9.15 e cancellato da umano ipocrita che morirà rapidamente
		
//colonna 0															  RB0
	&tasto0,                                                        //RD0
	&tasto9,  														//RD1
	&tasto8,  														//RD2
	&tasto7,														//RD3
	&tasto6,														//RD4
	&tasto5,														//RD5
	&tasto4,														//RD6
	&tasto3,														//RD7
	&tastoBACKSLASH, 										    	//RE0
	&tasto1,														//RE1
	&tasto2,          										        //RE2

//colonna 1													  		  RB1
	&tastoF10,                                                      //RD0
	&tastoF9,      											    	//RD1
	&tastoF8, 														//RD2
	&tastoF7,       											    //RD3
	&tastoF6,				  										//RD4
	&tastoF5, 														//RD5
	&tastoF4, 														//RD6
	&tastoF3,   													//RD7
	&tastoESC,		  										    	//RE0
	&tastoSleep /*2016 tastoF1*/, 														//RE1
	&tastoF2,     												    //RE2

//colonna 2											  				  RB2
	&tastoP,                                                        //RD0
	&tastoO,														//RD1
	&tastoI,														//RD2
	&tastoU,														//RD3
	&tastoY,		        								      	//RD4
	&tastoT,														//RD5
	&tastoR,														//RD6
	&tastoE,														//RD7
	&tastoTAB, 														//RE0
	&tastoPower /* 2016 tastoQ*/,			  											//RE1
	&tastoW,				  										//RE2

//colonna 3															  RB3
	&tastoOACC,                                                     //RD0
	&tastoL,	    												//RD1
	&tastoK,			        							    	//RD2
	&tastoJ,	    												//RD3
	&tastoH,	   													//RD4
	&tastoG,	  													//RD5
	&tastoF,	    												//RD6
	&tastoD,		      									     	//RD7
	&tastoCapsLock,  										    	//RE0
	&tastoA,	    												//RE1
	&tastoS,	    												//RE2

//colonna 4													  	    RB4
	&tastoPUNTO,                                                    //RD0
	&tastoVIRGOLA, 											    	//RD1
	&tastoM,		    											//RD2
	&tastoN,														//RD3
	&tastoB,														//RD4
	&tastoV,														//RD5
	&tastoC,														//RD6
	&tastoX,														//RD7
	&tastoLShift, 											    	//RE0
	&tastoMinore,													//RE1
	&tastoZ,			  											//RE2

//colonna 5							  							     RB5
	&tastoFrecciaDX,                                                //RD0
	&tastoFrecciaGiu,											   	//RD1
	&tastoFrecciaSX,     								         	//RD2
	&tastoRCtrl,		 											//RD3
	&tastoApps, 	    									    	//RD4
	&tastoRWindows,	   									         	//RD5
	&tastoAltGr,		   									     	//RD6
	&tastoSpazio,													//RD7
	&tastoLCtrl,  											    	//RE0
	&tastoLWindows, 										    	//RE1
	&tastoLAlt,	 											  		//RE2

//colonna 6															  RB6
	&tasto0Num,                                                     //RD0
	&tastoPuntoNum,  										      	//RD1
	&tastoEnterNum,    										       	//RD2
	&tasto3Num,     										      	//RD3
	&tasto2Num, 	     									    	//RD4
	&tasto1Num,	   													//RD5
	&tastoDummy,													//RD6
	&tastoFrecciaSu,											    //RD7
	&tastoTRATTINI, 										    	//RE0
	&tastoRShift,													//RE1
	&tastoDummy,													//RE2

//colonna 7									  						  RB7
	&tastoUACC,                                                     //RD0
	&tastoPIU,	    											    //RD1
	&tastoEACC,		   										    	//RD2
	&tasto6Num,  													//RD3
	&tasto5Num, 													//RD4
	&tasto4Num, 													//RD5
	&tastoDummy, 													//RD6
	&tastoTAB,   													//RD7
	&tastoAACC,  													//RE0
	&tastoEnter, 													//RE1
	&tastoDummy,	  											    //RE2

//colonna 8														      RC0
	&tastoPause,                                                    //RD0
	&tastoScrollLock,											   	//RD1
	&tastoPrtSc,      										      	//RD2
	&tastoF12,    												    //RD3
	&tastoF11,    												    //RD4
	&tastoPgUp,														//RD5
	&tastoHome, 													//RD6
	&tastoIns,														//RD7
	&tastoAPOSTROFO, 											    //RE0
	&tastoIACC,														//RE1
	&tastoBackSpace,  										        //RE2

//colonna 9														      RC1
	&tastoNumLock,                                                  //RD0
	&tastoSlashNum, 										    	//RD1
	&tastoAsteriscoNum,  								         	//RD2
	&tastoMenoNum,											    	//RD3
	&tastoPiuNum, 												   	//RD4
	&tasto9Num,														//RD5
	&tasto8Num, 	  											  	//RD6
	&tasto7Num,	    											   	//RD7
	&tastoCanc,	        									        //RE0
	&tastoEnd,   													//RE1
	&tastoPgDn,		      									        //RE2
	};

#if MAX_LAYOUT>1

ROM BYTE * ROM KB_layout1[]= {
	&tastoDummy, 			// serve xche' kbScanCode parte da 1...

		
// LAYOUT PER KT-105-XXX     MONTATO SU KT-ED8-A         05-02-2013	
//colonna 0															  RB0
	&tasto0,                                                        //RD0
	&tasto9,  														//RD1
	&tasto8,  														//RD2
	&tasto7,														//RD3
	&tasto6,														//RD4
	&tasto5,														//RD5
	&tasto4,														//RD6
	&tasto3,														//RD7
	&tastoBACKSLASH, 										    	//RE0
	&tasto1,														//RE1
	&tasto2,          										        //RE2

//colonna 1													  		  RB1
	&tastoF10,                                                      //RD0
	&tastoF9,      											    	//RD1
	&tastoF8, 														//RD2
	&tastoF7,       											    //RD3
	&tastoF6,				  										//RD4
	&tastoF5, 														//RD5
	&tastoF4, 														//RD6
	&tastoF3,   													//RD7
	&tastoESC,		  										    	//RE0
	&tastoF1, 														//RE1
	&tastoF2,     												    //RE2

//colonna 2											  				  RB2
	&tastoP,                                                        //RD0
	&tastoO,														//RD1
	&tastoI,														//RD2
	&tastoU,														//RD3
	&tastoY,		        								      	//RD4
	&tastoT,														//RD5
	&tastoR,														//RD6
	&tastoE,														//RD7
	&tastoTAB, 														//RE0
	&tastoQ,			  											//RE1
	&tastoW,				  										//RE2

//colonna 3															  RB3
	&tastoOACC,                                                     //RD0
	&tastoL,	    												//RD1
	&tastoK,			        							    	//RD2
	&tastoJ,	    												//RD3
	&tastoH,	   													//RD4
	&tastoG,	  													//RD5
	&tastoF,	    												//RD6
	&tastoD,		      									     	//RD7
	&tastoCapsLock,  										    	//RE0
	&tastoA,	    												//RE1
	&tastoS,	    												//RE2

//colonna 4													  	    RB4
	&tastoPUNTO,                                                    //RD0
	&tastoVIRGOLA, 											    	//RD1
	&tastoM,		    											//RD2
	&tastoN,														//RD3
	&tastoB,														//RD4
	&tastoV,														//RD5
	&tastoC,														//RD6
	&tastoX,														//RD7
	&tastoLShift, 											    	//RE0
	&tastoMinore,													//RE1
	&tastoZ,			  											//RE2

//colonna 5							  							     RB5
	&tastoFrecciaDX,                                                //RD0
	&tastoFrecciaGiu,											   	//RD1
	&tastoFrecciaSX,     								         	//RD2
	&tastoRCtrl,		 											//RD3
	&tastoApps, 	    									    	//RD4
	&tastoRWindows,	   									         	//RD5
	&tastoAltGr,		   									     	//RD6
	&tastoSpazio,													//RD7
	&tastoLCtrl,  											    	//RE0
	&tastoLWindows, 										    	//RE1
	&tastoLAlt,	 											  		//RE2

//colonna 6															  RB6
	&tasto0Num,                                                     //RD0
	&tastoPuntoNum,  										      	//RD1
	&tastoEnterNum,    										       	//RD2
	&tasto3Num,     										      	//RD3
	&tasto2Num, 	     									    	//RD4
	&tasto1Num,	   													//RD5
	&tastoDummy,													//RD6
	&tastoFrecciaSu,											    //RD7
	&tastoTRATTINI, 										    	//RE0
	&tastoRShift,													//RE1
	&tastoDummy,													//RE2

//colonna 7									  						  RB7
	&tastoUACC,                                                     //RD0
	&tastoPIU,	    											    //RD1
	&tastoEACC,		   										    	//RD2
	&tasto6Num,  													//RD3
	&tasto5Num, 													//RD4
	&tasto4Num, 													//RD5
	&tastoDummy, 													//RD6
	&tastoTAB,   													//RD7
	&tastoAACC,  													//RE0
	&tastoEnter, 													//RE1
	&tastoDummy,	  											    //RE2

//colonna 8														      RC0
	&tastoPause,                                                    //RD0
	&tastoScrollLock,											   	//RD1
	&tastoPrtSc,      										      	//RD2
	&tastoF12,    												    //RD3
	&tastoF11,    												    //RD4
	&tastoPgUp,														//RD5
	&tastoHome, 													//RD6
	&tastoIns,														//RD7
	&tastoAPOSTROFO, 											    //RE0
	&tastoIACC,														//RE1
	&tastoBackSpace,  										        //RE2

//colonna 9														      RC1
	&tastoNumLock,                                                  //RD0
	&tastoSlashNum, 										    	//RD1
	&tastoAsteriscoNum,  								         	//RD2
	&tastoMenoNum,											    	//RD3
	&tastoPiuNum, 												   	//RD4
	&tasto9Num,														//RD5
	&tasto8Num, 	  											  	//RD6
	&tasto7Num,	    											   	//RD7
	&tastoCanc,	        									        //RE0
	&tastoEnd,   													//RE1
	&tastoPgDn,		      									        //RE2


	};


#if MAX_LAYOUT>2

ROM BYTE * ROM KB_layout2[]= {
	&tastoDummy, 			// serve xche' kbScanCode parte da 1...

		
// LAYOUT PER KT-105-XXX     MONTATO SU KT-ED8-A         05-02-2013	
//colonna 0															  RB0
	&tasto0,                                                        //RD0
	&tasto9,  														//RD1
	&tasto8,  														//RD2
	&tasto7,														//RD3
	&tasto6,														//RD4
	&tasto5,														//RD5
	&tasto4,														//RD6
	&tasto3,														//RD7
	&tastoBACKSLASH, 										    	//RE0
	&tasto1,														//RE1
	&tasto2,          										        //RE2

//colonna 1													  		  RB1
	&tastoF10,                                                      //RD0
	&tastoF9,      											    	//RD1
	&tastoF8, 														//RD2
	&tastoF7,       											    //RD3
	&tastoF6,				  										//RD4
	&tastoF5, 														//RD5
	&tastoF4, 														//RD6
	&tastoF3,   													//RD7
	&tastoESC,		  										    	//RE0
	&tastoF1, 														//RE1
	&tastoF2,     												    //RE2

//colonna 2											  				  RB2
	&tastoP,                                                        //RD0
	&tastoO,														//RD1
	&tastoI,														//RD2
	&tastoU,														//RD3
	&tastoY,		        								      	//RD4
	&tastoT,														//RD5
	&tastoR,														//RD6
	&tastoE,														//RD7
	&tastoTAB, 														//RE0
	&tastoQ,			  											//RE1
	&tastoW,				  										//RE2

//colonna 3															  RB3
	&tastoOACC,                                                     //RD0
	&tastoL,	    												//RD1
	&tastoK,			        							    	//RD2
	&tastoJ,	    												//RD3
	&tastoH,	   													//RD4
	&tastoG,	  													//RD5
	&tastoF,	    												//RD6
	&tastoD,		      									     	//RD7
	&tastoCapsLock,  										    	//RE0
	&tastoA,	    												//RE1
	&tastoS,	    												//RE2

//colonna 4													  	    RB4
	&tastoPUNTO,                                                    //RD0
	&tastoVIRGOLA, 											    	//RD1
	&tastoM,		    											//RD2
	&tastoN,														//RD3
	&tastoB,														//RD4
	&tastoV,														//RD5
	&tastoC,														//RD6
	&tastoX,														//RD7
	&tastoLShift, 											    	//RE0
	&tastoMinore,													//RE1
	&tastoZ,			  											//RE2

//colonna 5							  							     RB5
	&tastoFrecciaDX,                                                //RD0
	&tastoFrecciaGiu,											   	//RD1
	&tastoFrecciaSX,     								         	//RD2
	&tastoRCtrl,		 											//RD3
	&tastoApps, 	    									    	//RD4
	&tastoRWindows,	   									         	//RD5
	&tastoAltGr,		   									     	//RD6
	&tastoSpazio,													//RD7
	&tastoLCtrl,  											    	//RE0
	&tastoLWindows, 										    	//RE1
	&tastoLAlt,	 											  		//RE2

//colonna 6															  RB6
	&tasto0Num,                                                     //RD0
	&tastoPuntoNum,  										      	//RD1
	&tastoEnterNum,    										       	//RD2
	&tasto3Num,     										      	//RD3
	&tasto2Num, 	     									    	//RD4
	&tasto1Num,	   													//RD5
	&tastoDummy,													//RD6
	&tastoFrecciaSu,											    //RD7
	&tastoTRATTINI, 										    	//RE0
	&tastoRShift,													//RE1
	&tastoDummy,													//RE2

//colonna 7									  						  RB7
	&tastoUACC,                                                     //RD0
	&tastoPIU,	    											    //RD1
	&tastoEACC,		   										    	//RD2
	&tasto6Num,  													//RD3
	&tasto5Num, 													//RD4
	&tasto4Num, 													//RD5
	&tastoDummy, 													//RD6
	&tastoTAB,   													//RD7
	&tastoAACC,  													//RE0
	&tastoEnter, 													//RE1
	&tastoDummy,	  											    //RE2

//colonna 8														      RC0
	&tastoPause,                                                    //RD0
	&tastoScrollLock,											   	//RD1
	&tastoPrtSc,      										      	//RD2
	&tastoF12,    												    //RD3
	&tastoF11,    												    //RD4
	&tastoPgUp,														//RD5
	&tastoHome, 													//RD6
	&tastoIns,														//RD7
	&tastoAPOSTROFO, 											    //RE0
	&tastoIACC,														//RE1
	&tastoBackSpace,  										        //RE2

//colonna 9														      RC1
	&tastoNumLock,                                                  //RD0
	&tastoSlashNum, 										    	//RD1
	&tastoAsteriscoNum,  								         	//RD2
	&tastoMenoNum,											    	//RD3
	&tastoPiuNum, 												   	//RD4
	&tasto9Num,														//RD5
	&tasto8Num, 	  											  	//RD6
	&tasto7Num,	    											   	//RD7
	&tastoCanc,	        									        //RE0
	&tastoEnd,   													//RE1
	&tastoPgDn,		      									        //RE2


	};


ROM BYTE * ROM KB_layout3[]= {
	&tastoDummy, 			// serve xche' kbScanCode parte da 1...

		
		
// LAYOUT PER KT-105-XXX     MONTATO SU KT-ED8-A         05-02-2013	
//colonna 0															  RB0
	&tasto0,                                                        //RD0
	&tasto9,  														//RD1
	&tasto8,  														//RD2
	&tasto7,														//RD3
	&tasto6,														//RD4
	&tasto5,														//RD5
	&tasto4,														//RD6
	&tasto3,														//RD7
	&tastoBACKSLASH, 										    	//RE0
	&tasto1,														//RE1
	&tasto2,          										        //RE2

//colonna 1													  		  RB1
	&tastoF10,                                                      //RD0
	&tastoF9,      											    	//RD1
	&tastoF8, 														//RD2
	&tastoF7,       											    //RD3
	&tastoF6,				  										//RD4
	&tastoF5, 														//RD5
	&tastoF4, 														//RD6
	&tastoF3,   													//RD7
	&tastoESC,		  										    	//RE0
	&tastoF1, 														//RE1
	&tastoF2,     												    //RE2

//colonna 2											  				  RB2
	&tastoP,                                                        //RD0
	&tastoO,														//RD1
	&tastoI,														//RD2
	&tastoU,														//RD3
	&tastoY,		        								      	//RD4
	&tastoT,														//RD5
	&tastoR,														//RD6
	&tastoE,														//RD7
	&tastoTAB, 														//RE0
	&tastoQ,			  											//RE1
	&tastoW,				  										//RE2

//colonna 3															  RB3
	&tastoOACC,                                                     //RD0
	&tastoL,	    												//RD1
	&tastoK,			        							    	//RD2
	&tastoJ,	    												//RD3
	&tastoH,	   													//RD4
	&tastoG,	  													//RD5
	&tastoF,	    												//RD6
	&tastoD,		      									     	//RD7
	&tastoCapsLock,  										    	//RE0
	&tastoA,	    												//RE1
	&tastoS,	    												//RE2

//colonna 4													  	    RB4
	&tastoPUNTO,                                                    //RD0
	&tastoVIRGOLA, 											    	//RD1
	&tastoM,		    											//RD2
	&tastoN,														//RD3
	&tastoB,														//RD4
	&tastoV,														//RD5
	&tastoC,														//RD6
	&tastoX,														//RD7
	&tastoLShift, 											    	//RE0
	&tastoMinore,													//RE1
	&tastoZ,			  											//RE2

//colonna 5							  							     RB5
	&tastoFrecciaDX,                                                //RD0
	&tastoFrecciaGiu,											   	//RD1
	&tastoFrecciaSX,     								         	//RD2
	&tastoRCtrl,		 											//RD3
	&tastoApps, 	    									    	//RD4
	&tastoRWindows,	   									         	//RD5
	&tastoAltGr,		   									     	//RD6
	&tastoSpazio,													//RD7
	&tastoLCtrl,  											    	//RE0
	&tastoLWindows, 										    	//RE1
	&tastoLAlt,	 											  		//RE2

//colonna 6															  RB6
	&tasto0Num,                                                     //RD0
	&tastoPuntoNum,  										      	//RD1
	&tastoEnterNum,    										       	//RD2
	&tasto3Num,     										      	//RD3
	&tasto2Num, 	     									    	//RD4
	&tasto1Num,	   													//RD5
	&tastoDummy,													//RD6
	&tastoFrecciaSu,											    //RD7
	&tastoTRATTINI, 										    	//RE0
	&tastoRShift,													//RE1
	&tastoDummy,													//RE2

//colonna 7									  						  RB7
	&tastoUACC,                                                     //RD0
	&tastoPIU,	    											    //RD1
	&tastoEACC,		   										    	//RD2
	&tasto6Num,  													//RD3
	&tasto5Num, 													//RD4
	&tasto4Num, 													//RD5
	&tastoDummy, 													//RD6
	&tastoTAB,   													//RD7
	&tastoAACC,  													//RE0
	&tastoEnter, 													//RE1
	&tastoDummy,	  											    //RE2

//colonna 8														      RC0
	&tastoPause,                                                    //RD0
	&tastoScrollLock,											   	//RD1
	&tastoPrtSc,      										      	//RD2
	&tastoF12,    												    //RD3
	&tastoF11,    												    //RD4
	&tastoPgUp,														//RD5
	&tastoHome, 													//RD6
	&tastoIns,														//RD7
	&tastoAPOSTROFO, 											    //RE0
	&tastoIACC,														//RE1
	&tastoBackSpace,  										        //RE2

//colonna 9														      RC1
	&tastoNumLock,                                                  //RD0
	&tastoSlashNum, 										    	//RD1
	&tastoAsteriscoNum,  								         	//RD2
	&tastoMenoNum,											    	//RD3
	&tastoPiuNum, 												   	//RD4
	&tasto9Num,														//RD5
	&tasto8Num, 	  											  	//RD6
	&tasto7Num,	    											   	//RD7
	&tastoCanc,	        									        //RE0
	&tastoEnd,   													//RE1
	&tastoPgDn,		      									        //RE2


	};


#endif
#endif

#else			// A3010

ROM BYTE * ROM KB_layout0[]= {
	&tastoDummy,			// serve xche' kbScanCode parte da 1...

//colonna 0															  RB0
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoAltGr,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 1													  		  RB1
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoRCtrl,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 2											  				  RB2
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoRShift,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 3															  RB3
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoPrtSc,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 4													  	    RB4
//    /* Row 4  pin 9  */ {KEY_SCROLL_LOCK, KEY_A, KEY_W, KEY_E, KEY_T, KEY_Y, 0, KEY_RETURN, KEY_UP, KEY_PAGE_DOWN, KEY_INSERT, KEY_RIGHT_BRACE, KEY_F12, KEY_F8},
	&tastoScrollLock,
	&tastoA,
	&tastoW,
	&tastoE,
	&tastoT,
	&tastoY,
	&tastoDummy,
	&tastoEnter,
	&tastoFrecciaSu,
	&tastoPgDn,
	&tastoIns,
	&tastoDummy,		// right brace...
	&tastoF12,
	&tastoF8,

//colonna 5							  							     RB5
//    /* Row 5  pin 10 */ {KEY_PAUSE, KEY_Q, KEY_2, KEY_3, KEY_R, KEY_U, 0, KEY_I, 0, KEY_END, KEY_BACKSPACE, KEY_O, KEY_F11, KEY_F9},
	&tastoPause,
	&tastoQ,
	&tasto2,
	&tasto3,
	&tastoR,
	&tastoU,
	&tastoDummy,
	&tastoI,
	&tastoDummy,
	&tastoEnd,
	&tastoBackSpace,
	&tastoO,
	&tastoF11,
	&tastoF9,

//colonna 6															  RB6
//    /* Row 6  pin 11 */ {0, 0, 0, 0, 0, 0, KEY_HOME, 0, 0, 0, 0, 0, 0, 0},
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoHome,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 7									  						  RB7
//    /* Row 7  pin 12 */ {0, KEY_1, 0, 0, KEY_5, KEY_6, 0, KEY_F5, 0, 0, KEY_EQUAL, KEY_F6, KEY_F10, KEY_9},
	&tastoDummy,
	&tasto1,
	&tastoDummy,
	&tastoDummy,
	&tasto5,
	&tasto6,
	&tastoDummy,
	&tastoF5,
	&tastoDummy,
	&tastoDummy,
	&tastoSHIFTZERO,		// uguale...
	&tastoF6,
	&tastoF10,
	&tasto9,

//colonna 8														      RC0
//    /* Row 8  pin 13 */ {0, 0, 0, 0, 0, 0, KEY_LEFT_SHIFT, 0, 0, 0, 0, 0, 0, 0},
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoLShift,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 9														      RC1
//    /* Row 9  pin 14 */ {0, 0, 0, 0, 0, 0, KEY_LEFT_CTRL, 0, 0, 0, 0, 0, 0, 0},
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoLCtrl,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 10														      RC1
//    /* Row 10 pin 15 */ {KEY_PAGE_UP, 0, KEY_F1, KEY_F4, KEY_4, KEY_7, 0, KEY_8, 0, KEY_DELETE, KEY_POUND, KEY_9, KEY_MINUS, 0},
	&tastoPgUp,
	&tastoDummy,
	&tastoF1,
	&tastoF4,
	&tasto4,
	&tasto7,
	&tastoDummy,
	&tasto8,
	&tastoDummy,
	&tastoCanc,
	&tastoDummy,			// pound...
	&tasto9,
	&tastoMenoNum,		// NO! trovare "meno" su tastiera
	&tastoDummy,

//colonna 11														      RC1
//    /* Row 11 pin 16 */ {KEY_NUM_LOCK, KEY_TAB, KEY_F2, KEY_F3, KEY_F, KEY_J, 0, KEY_BACKSLASH, KEYPAD_PERIOD, KEYPAD_HASH, KEYPAD_ASTERIX, KEY_L, KEYPAD_SLASH, KEY_0},
	&tastoNumLock,
	&tastoTAB,
	&tastoF2,
	&tastoF3,
	&tastoF,
	&tastoJ,
	&tastoDummy,
	&tastoBACKSLASH,
	&tastoPuntoNum,
	&tastoDummy,		// KEYPAD_HASH ??
	&tastoAsteriscoNum,
	&tastoL,
	&tastoSlashNum,
	&tasto0,

//colonna 12														      RC1
//    /* Row 12 pin 17 */ {KEYPAD_7, KEY_Z, KEY_S, KEY_D, KEY_G, KEY_H, 0, KEY_K, KEY_RIGHT, KEYPAD_MINUS, KEYPAD_9, KEY_QUOTE, KEYPAD_8, KEY_LEFT_BRACE},
	&tasto7Num,
	&tastoZ,
	&tastoS,
	&tastoD,
	&tastoG,
	&tastoH,
	&tastoDummy,
	&tastoK,
	&tastoFrecciaDX,
	&tastoMenoNum,
	&tasto9Num,
	&tastoDummy,		//"
	&tasto8Num,
	&tastoDummy,		// left brace

//colonna 13														      RC1
//    /* Row 13 pin 18 */ {KEYPAD_4, KEY_TILDE, KEY_X, KEY_C, KEY_V, KEY_M, 0, KEY_COMMA, KEY_DOWN, KEYPAD_PLUS, KEYPAD_6, KEY_PERIOD, KEYPAD_5, KEY_P},
	&tasto4Num,
	&tastoDummy,		// tilde
	&tastoX,
	&tastoC,
	&tastoV,
	&tastoM,
	&tastoDummy,
	&tastoVIRGOLA,
	&tastoPiuNum,
	&tasto6,
	&tastoPUNTO,
	&tasto5Num,
	&tastoP,
	&tastoDummy,

//colonna 14														      RC1
//    /* Row 14 pin 19 */ {0, 0, 0, 0, 0, 0, KEY_LEFT_ALT, 0, 0, 0, 0, 0, 0, 0},
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoLAlt,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,
	&tastoDummy,

//colonna 15														      RC1
//    /* Row 15 pin 20 */ {KEYPAD_1, KEY_ESC, KEY_CAPS_LOCK, KEY_SPACE, KEY_B, KEY_N, 0, KEY_LEFT, KEYPAD_0, KEYPAD_ENTER, KEYPAD_3, KEY_SLASH, KEYPAD_2, KEY_SEMICOLON}
	&tasto1Num,
	&tastoESC,
	&tastoCapsLock,
	&tastoSpazio,
	&tastoB,
	&tastoN,
	&tastoDummy,
	&tastoFrecciaSX,
	&tasto0Num,
	&tastoEnterNum,
	&tasto3Num,
	&tastoDummy,		// Slash...
	&tasto2Num,
	&tastoPUNTO_VIRGOLA_ITA,		// semicolon...
	};

#endif

#ifndef __C30__
#pragma romdata 
#endif


// in questo modo abbiamo un insieme di tasti usato da tutte e quattro i layout
// si possono definire altri 3 insiemi di tasti/stringhe e assegnarli nelle corrispondenti posizioni sopra

// i valori sopra provengono da un sorgente di Boagno e sono solo esemplificativi;
// altri esempi:
#define tasto1_1 240,0xE0,0xE2,0x4C,0		// CTRL+ALT+CANC

#define tasto2_1 241,0x1a,0x1a,0x1a,0x37,0	// WWW.

#define tasto3_1 242,"w","w","w",".",0				// www.



BYTE GetKBchar(BYTE n) {


#ifdef USA_EEPROM 

#ifdef USA_EEPROM_INTERNA
	if(n) {		// risparmio un /2 byte in eeprom interna... tanto per...
		kbptr=(n-1)*2+RESERVED_EEPROM+sizeof(struct KB_LAYOUT_HEADER);
		kbptr_prog=0;
		return EEleggi(kbptr);
		}
	else
		return 0;

#else
	WORD i;

	if(n) {		// 
		switch(kbLayout) {
			case 0:

				kbptr_prog=1;
				kbptr=kblayoutptr[0];
				i=0;
				while(i<110) {		// in effetti non può arrivare oltre 110... (si spera)
	// per ogni scancode c'è num.code/lunghezza/sequenza!
					I2CRead16Seq(kbptr,2);
					if(I2CBuffer[0] == n) {
						I2CRead16Seq(kbptr,3);			// 
						kbptr_prog_max=I2CBuffer[1];
						return I2CBuffer[2];	
						}
					else
						kbptr+=I2CBuffer[1]+2;
					i++;
					} 
				return 0;

#ifdef OLD
				kbptr=n-1;		// byte->word
				kbptr *= 3;
				kbptr+=kblayoutptr[0];
				kbptr_prog=0;
				I2CRead16Seq(kbptr+kbptr_prog,3);			// in effetti ********** non è così: bisogna cercare in tutto l'area il cod tasto,
// perché per ogni scancode c'è num.code/lunghezza/sequenza!
				kbptr_prog_max=I2CBuffer[1];

				return I2CBuffer[2];
#endif

				break;
	
#if MAX_LAYOUT>1
			case 1:
				kbptr_prog=1;
				kbptr=kblayoutptr[1];
				i=0;
				while(i<110) {		// in effetti non può arrivare oltre 110... (si spera)
	// per ogni scancode c'è num.code/lunghezza/sequenza!
					I2CRead16Seq(kbptr,2);
					if(I2CBuffer[0] == n) {
						I2CRead16Seq(kbptr,3);			// 
						kbptr_prog_max=I2CBuffer[1];
						return I2CBuffer[2];	
						}
					else
						kbptr+=I2CBuffer[1]+2;
					i++;
					} 
				return 0;
				break;
	
#if MAX_LAYOUT>2
			case 2:
			case 3:
				kbptr_prog=1;
				kbptr=kblayoutptr[kbLayout];
				i=0;
				while(i<110) {		// in effetti non può arrivare oltre 110... (si spera)
	// per ogni scancode c'è num.code/lunghezza/sequenza!
					I2CRead16Seq(kbptr,2);
					if(I2CBuffer[0] == n) {
						I2CRead16Seq(kbptr,3);			// 
						kbptr_prog_max=I2CBuffer[1];
						return I2CBuffer[2];	
						}
					else
						kbptr+=I2CBuffer[1]+2;
					i++;
					} 
				return 0;
				break;
#endif
#endif
			}
		}
	else
		return 0;

#endif

#else
	switch(kbLayout) {
		case 0:
			kbptr=KB_layout0[n];
			return *kbptr;
			break;

#if MAX_LAYOUT>1
		case 1:
			kbptr=KB_layout1[n];
			return *kbptr;
			break;

#if MAX_LAYOUT>2
		case 2:
			kbptr=KB_layout2[n];
			return *kbptr;
			break;
		case 3:
			kbptr=KB_layout3[n];
			return *kbptr;
			break;
#endif
#endif
		}
#endif

	}

BYTE GetKBcharNext(void) {

#ifdef USA_EEPROM 

#ifdef USA_EEPROM_INTERNA
	if(kbptr_prog<1) {
		kbptr_prog++;
		return EEleggi(kbptr+kbptr_prog);
		}
	else
		return 0;
#else
	if(kbptr_prog<kbptr_prog_max) {

		I2CRead16Seq(kbptr+2+kbptr_prog,1);			// salto i primi 2, sono dentro la sequenza
		kbptr_prog++;
		return I2CBuffer[0];
		}
	else
		return 0;
#endif

#else
	return *++kbptr;
#endif
	}

BYTE GetKBcharPrev(void) {

#ifdef USA_EEPROM 

#ifdef USA_EEPROM_INTERNA
	if(kbptr_prog>0) {			// meno utile ma va bene!
		kbptr_prog--;
		return EEleggi(kbptr+kbptr_prog);
		}
	else
		return 0;
#else
	if(kbptr_prog>0) {

		I2CRead16Seq(kbptr+2+kbptr_prog,1);			// salto i primi 2, sono dentro la sequenza
		kbptr_prog--;
		return I2CBuffer[0];
		}
	else
		return 0;
#endif

#else
	return *--kbptr;
#endif
	}


#ifdef USA_EEPROM 
signed char loadKBlayout(void) {

#ifdef USA_EEPROM_INTERNA
	BYTE addr,i,iter=0;

rifo:
	addr=RESERVED_EEPROM;
	{
	BYTE k;
	for(k=0; k<sizeof(struct KB_LAYOUT_HEADER); k++)
		I2CBuffer[k]=EEleggi(addr+k);
	}
	memcpy(&kbLayoutHeader,&I2CBuffer,sizeof(struct KB_LAYOUT_HEADER));
	if(kbLayoutHeader.signature==0x44  /*D,G*/) /* qua no && kbLayoutHeader.modello==idTastiera*/ {
		addr+=sizeof(struct KB_LAYOUT_HEADER);
// nulla da fare
		}
	else {
		presetKBlayout();
		iter++;
		if(iter<2)
			goto rifo;
		else {
			for(i=0; i<10; i++) {
				mLED_1 ^= 1;
				mLED_2 ^= 1;
				Delay_ms(200);
				ClrWdt();
				}
			}
		i=0;
		}

#else
	WORD addr,i,iter=0;
	BYTE ch,ch2,j,l,*k;

rifo:
	addr=RESERVED_EEPROM;
	I2CRead16Seq(addr,sizeof(struct KB_LAYOUT_HEADER));			// 
	memcpy(&kbLayoutHeader,&I2CBuffer,sizeof(struct KB_LAYOUT_HEADER));
	if(kbLayoutHeader.signature==0x44  /*D,G*/) /* qua no && kbLayoutHeader.modello==idTastiera*/ {
		addr+=sizeof(struct KB_LAYOUT_HEADER);

		I2CRead16Seq(addr,sizeof(kblayoutptr));			// 
		memcpy(&kblayoutptr,I2CBuffer,sizeof(kblayoutptr));
		addr+=sizeof(kblayoutptr);
// null'altro qua

		}
	else {
		presetKBlayout();
		iter++;
		if(iter<2)
			goto rifo;
		else {
			for(i=0; i<10; i++) {
				mLED_1 ^= 1;
				mLED_2 ^= 1;
				Delay_ms(200);
				ClrWdt();
				}
			}
		i=0;
		}
#endif

	return i;
	}

#else

signed char loadKBlayout(void) {			// giusto per avere i valori impostati... getconfig ecc...

	kbLayoutHeader.signature=0x44  /*D,G*/;
#ifdef USA_TASTIERA_21PIN
	kbLayoutHeader.numTasti=21;
#else
	kbLayoutHeader.numTasti=110;
#endif
	kbLayoutHeader.modello=0   /* qua no idTastiera*/ ;
	kbLayoutHeader.numLayout=1 /*MAX_LAYOUT tanto darebbe errore */;
	return 1;
	}
#endif


#ifdef USA_EEPROM 
signed char presetKBlayout(void) {

#ifdef USA_EEPROM_INTERNA
	BYTE addr,i,j;

	addr=RESERVED_EEPROM;
	kbLayoutHeader.signature=0x44  /*D,G*/;
#ifdef USA_TASTIERA_21PIN
	kbLayoutHeader.numTasti=21;
#else
	kbLayoutHeader.numTasti=110;
#endif
	kbLayoutHeader.modello=0   /* qua no idTastiera*/ ;
	kbLayoutHeader.numLayout=1 /*MAX_LAYOUT tanto darebbe errore */;

	memcpy(&I2CBuffer,&kbLayoutHeader,sizeof(struct KB_LAYOUT_HEADER));
	{
	BYTE k;
	for(k=0; k<sizeof(struct KB_LAYOUT_HEADER); k++)
		EEscrivi(addr+k,I2CBuffer[k]);
	}
	addr+=sizeof(struct KB_LAYOUT_HEADER);

	for(i=0; i<2; i++) {
		mLED_1 ^= 1;
		mLED_2 ^= 1;
		Delay_ms(100);
		ClrWdt();
		}


// la memoria è usata così: 2 byte per ogni tasto (e layout in ROM deve corrispondere, v. sopra!)
	for(i=0; i<110; i++) {
// USARE POI in definitiva! copia da ROM a EEprom ... perché no, peraltro? forse da consilium...
#define USA_ROM_EEPROM 1
#ifdef USA_ROM_EEPROM
		I2CBuffer[0]=*KB_layout0[i+1];		// codice... salto il dummy... se no sforo 256!
		I2CBuffer[1]=0;		// 0 fisso..
		{
		BYTE k;
		for(k=0; k<2; k++)
			EEscrivi(addr+k,I2CBuffer[k]);
		}
#endif
		addr += 2;
		}

#else			// mem. esterna

	WORD addr,i;

	addr=RESERVED_EEPROM;
	kbLayoutHeader.signature=0x44  /*D,G*/;
#ifdef USA_TASTIERA_21PIN
	kbLayoutHeader.numTasti=21;
#else
	kbLayoutHeader.numTasti=110;
#endif
	kbLayoutHeader.modello=0   /* qua no idTastiera*/ ;
	kbLayoutHeader.numLayout=MAX_LAYOUT;
	memcpy(&I2CBuffer,&kbLayoutHeader,sizeof(struct KB_LAYOUT_HEADER));
	I2CWritePage16(addr,sizeof(struct KB_LAYOUT_HEADER));			// 
	addr+=sizeof(struct KB_LAYOUT_HEADER);

	addr+=sizeof(kblayoutptr);

	kblayoutptr[0]=addr;

	for(i=0; i<2; i++) {
		mLED_1 ^= 1;
		mLED_2 ^= 1;
		Delay_ms(100);
		ClrWdt();
		}


// OCCHIO che a scrivere 128byte senza cambiare pagina, si sovrascrive quella corrente!
//2014 v. agie

//ripetere x tutti i layout...
	for(i=1; i<sizeof(KB_layout0)/sizeof(BYTE * ROM ); i++) {
		if(*KB_layout0[i] < 240) {
			I2CBuffer[0]=i;			// scancode a cui si riferisce il blocchetto...
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=1;			// lunghezza del blocchetto
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=*KB_layout0[i];		// cod. singolo
			I2CWritePage16(addr++,1);			// 
			}
		else {
			I2CBuffer[0]=i;
			I2CWritePage16(addr++,1);			// per non rischiare la pagina EEprom da 128 ..
			{
			BYTE i1,in241=0,in240;
			ROM BYTE *p2;
			p2=KB_layout0[i];
			switch(*p2) {			// casi particolari non zero-term
				case 248:
					i1=2;
					goto l0_248_255;
					break;
				case 250:
#ifdef MULTIMEDIA_KEYBOARD
					i1=5;
					goto l0_248_255;
#endif
					break;
				case 241:		// per gestire i 240 "embedded"
					in241=1;
				default:
					i1=0;
					in240=0;
					for(;;) {
						if(in241) {
							if(*p2==240) {			// 
								in240=1;
								}
							if(!*p2) {
								if(!in240)
									break;
								else
									in240=0;
								}
							}
						else {
							if(!*p2)
								break;
							}
						p2++;
						i1++;
						} 
l0_248_255:
					p2=KB_layout0[i];
					I2CBuffer[0]=i1;		// lunghezza del blocchetto
					I2CWritePage16(addr++,1);			// 
					while(i1--) {
						I2CBuffer[0]=*p2++;		// tasti composti
						I2CWritePage16(addr++,1);			// 
						}
					break;
				}
			}
			}
		}


#if MAX_LAYOUT>1
	kblayoutptr[1]=addr;

	for(i=0; i<2; i++) {
		mLED_1 ^= 1;
		mLED_2 ^= 1;
		Delay_ms(100);
		ClrWdt();
		}

	for(i=1; i<sizeof(KB_layout1)/sizeof(BYTE * ROM ); i++) {
		if(*KB_layout1[i] < 240) {
			I2CBuffer[0]=i;			// scancode a cui si riferisce il blocchetto...
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=1;			// lunghezza del blocchetto
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=*KB_layout1[i];		// cod. singolo
			I2CWritePage16(addr++,1);			// 
			}
		else {
			I2CBuffer[0]=i;
			I2CWritePage16(addr++,1);			// per non rischiare la pagina EEprom da 128 ..
			{
			BYTE i1,in241=0,in240;
			ROM BYTE *p2;
			p2=KB_layout1[i];
			switch(*p2) {			// casi particolari non zero-term
				case 248:
					i1=2;
					goto l1_248_255;
					break;
				case 250:
#ifdef MULTIMEDIA_KEYBOARD
					i1=5;
					goto l1_248_255;
#endif
					break;
				case 241:		// per gestire i 240 "embedded"
					in241=1;
				default:
					i1=0;
					in240=0;
					for(;;) {
						if(in241) {
							if(*p2==240) {			// 
								in240=1;
								}
							if(!*p2) {
								if(!in240)
									break;
								else
									in240=0;
								}
							}
						else {
							if(!*p2)
								break;
							}
						p2++;
						i1++;
						} 
l1_248_255:
					p2=KB_layout1[i];
					I2CBuffer[0]=i1;		// lunghezza del blocchetto
					I2CWritePage16(addr++,1);			// 
					while(i1--) {
						I2CBuffer[0]=*p2++;		// tasti composti
						I2CWritePage16(addr++,1);			// 
						}
// tendenzialmente non abbiam bisogno di sprecare lo 0 per il terminatore, perché la getCharNext ritorna 0 cmq...
// o forse no? lo mettiam lo stesso?
					break;
				}
			}
			}
		}

#if MAX_LAYOUT>2
	kblayoutptr[2]=addr;

	for(i=0; i<2; i++) {
		mLED_1 ^= 1;
		mLED_2 ^= 1;
		Delay_ms(100);
		ClrWdt();
		}

	for(i=1; i<sizeof(KB_layout2)/sizeof(BYTE * ROM ); i++) {
		if(*KB_layout2[i] < 240) {
			I2CBuffer[0]=i;			// scancode a cui si riferisce il blocchetto...
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=1;			// lunghezza del blocchetto
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=*KB_layout2[i];		// cod. singolo
			I2CWritePage16(addr++,1);			// 
			}
		else {
			I2CBuffer[0]=i;
			I2CWritePage16(addr++,1);			// per non rischiare la pagina EEprom da 128 ..
			{
			BYTE i1,in241=0,in240;
			ROM BYTE *p2;
			p2=KB_layout2[i];
			switch(*p2) {			// casi particolari non zero-term
				case 248:
					i1=2;
					goto l2_248_255;
					break;
				case 250:
#ifdef MULTIMEDIA_KEYBOARD
					i1=5;
					goto l2_248_255;
#endif
					break;
				case 241:		// per gestire i 240 "embedded"
					in241=1;
				default:
					i1=0;
					in240=0;
					for(;;) {
						if(in241) {
							if(*p2==240) {			// 
								in240=1;
								}
							if(!*p2) {
								if(!in240)
									break;
								else
									in240=0;
								}
							}
						else {
							if(!*p2)
								break;
							}
						p2++;
						i1++;
						} 
l2_248_255:
					p2=KB_layout2[i];
					I2CBuffer[0]=i1;		// lunghezza del blocchetto
					I2CWritePage16(addr++,1);			// 
					while(i1--) {
						I2CBuffer[0]=*p2++;		// tasti composti
						I2CWritePage16(addr++,1);			// 
						}
					break;
				}
			}
			}
		}

	kblayoutptr[3]=addr;
// OCCHIO SE SFORA 256!! controllare!

	for(i=1; i<sizeof(KB_layout3)/sizeof(BYTE * ROM ); i++) {
		if(*KB_layout3[i] < 240) {
			I2CBuffer[0]=i;			// scancode a cui si riferisce il blocchetto...
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=1;			// lunghezza del blocchetto
			I2CWritePage16(addr++,1);			// 
			I2CBuffer[0]=*KB_layout3[i];		// cod. singolo
			I2CWritePage16(addr++,1);			// 
			}
		else {
			I2CBuffer[0]=i;
			I2CWritePage16(addr++,1);			// per non rischiare la pagina EEprom da 128 ..
			{
			BYTE i1,in241=0,in240;
			ROM BYTE *p2;
			p2=KB_layout3[i];
			switch(*p2) {			// casi particolari non zero-term
				case 248:
					i1=2;
					goto l3_248_255;
					break;
				case 250:
#ifdef MULTIMEDIA_KEYBOARD
					i1=5;
					goto l3_248_255;
#endif
					break;
				case 241:		// per gestire i 240 "embedded"
					in241=1;
				default:
					i1=0;
					in240=0;
					for(;;) {
						if(in241) {
							if(*p2==240) {			// 
								in240=1;
								}
							if(!*p2) {
								if(!in240)
									break;
								else
									in240=0;
								}
							}
						else {
							if(!*p2)
								break;
							}
						p2++;
						i1++;
						} 
l3_248_255:
					p2=KB_layout3[i];
					I2CBuffer[0]=i1;		// lunghezza del blocchetto
					I2CWritePage16(addr++,1);			// 
					while(i1--) {
						I2CBuffer[0]=*p2++;		// tasti composti
						I2CWritePage16(addr++,1);			// 
						}
					break;
				}
			}
			}
		}
#endif

	addr=RESERVED_EEPROM+sizeof(struct KB_LAYOUT_HEADER);
	memcpy(I2CBuffer,&kblayoutptr,sizeof(kblayoutptr));
	I2CWritePage16(addr,sizeof(kblayoutptr));			// scrivere 2-4-8- byte qua, così, dovrebbe essere ok

#endif

#endif

	return i;
	}
#endif



const ROM BYTE ascii_2_usb[] = {			// parte da 32 => ' '; ovviamente vale per ITA ... (?)
	0x2c,			//SPACE
	0x16,	  	//!
	0x1e,  		//"
	0x26, 		//£
	0x25, 		//$
	0x2e, 		//%
	0x36, 		//&
	0x3d, 		///
	0x3e, 		//(
	0x46, 		//)
	0x45, 		//=
	0x30,			//+
	0x36,			//,
	0x38,			//-
	0x37,			//.
	0,				// / ??

	0x27, 		//0
	0x1e,	  	//1
	0x1f,  		//2
	0x20, 		//3
	0x21, 		//4
	0x22, 		//5
	0x23, 		//6
	0x24, 		//7
	0x25, 		//8
	0x26, 		//9
	0,
	0,
	0,
	0,
	0,
	0x4a,	  	// ?	

	0,				//@
	0x04,	  	//A
	0x05,	  	//B
	0x06,	  	//C
	0x07,	  	//D
	0x08,			//E
	0x09,	  	//F
	0x0a,	  	//G
	0x0b,	  	//H
	0x0c,	  	//I
	0x0d,	  	//J
	0x0e,	  	//K
	0x0f,	  	//L
	0x10,	  	//M
	0x11,	  	//N
	0x12,	  	//O

	0x13,		  //P
	0x14,			//Q
	0x15,	  	//R
	0x16,	  	//S
	0x17,	  	//T
	0x18,	  	//U
	0x19,	  	//V
	0x1a,		  //W
	0x1b,	  	//X
	0x1c,	  	//Y
	0x1d,	  	//Z

	0,0,0,0,0,0

	};



// dal demo della tastiera PS2-USB

#ifdef DARIO

table   code
        global  KeyboardTable

        extern  mtemp

KeyboardTable
    	movwf	mtemp
    	movlw	HIGH StartTable
    	movwf	PCLATH
	    movlw	low  StartTable
	    addwf	mtemp,w
	    btfsc	STATUS,C
	    incf	PCLATH,f
    	movwf	PCL
StartTable:

;entra in W lo scan code PS2/AT (quello con i MAKE CODE... E0...) ed esce il valore USB
        retlw   0x00    ; 00	  
        retlw   0x42    ; 01  F9
        retlw   0x00    ; 02
        retlw   0x3E    ; 03  F5
        retlw   0x3C    ; 04  F3
        retlw   0x3A    ; 05  F1
        retlw   0x3B    ; 06  F2
        retlw   0x45    ; 07  F12
        retlw   0x00    ; 08  
        retlw   0x43    ; 09  F10 
        retlw   0x41    ; 0A  F8
        retlw   0x3F    ; 0B  F6
        retlw   0x3D    ; 0C  F4
        retlw   0x2B    ; 0D  TAB
        retlw   0x35    ; 0E  ` opp. <
        retlw   0x00    ; 0F 
        retlw   0x00    ; 10
        retlw   0xE2    ; 11  L ALT
        retlw   0xE1    ; 12  L SHFT
        retlw   0x00    ; 13
        retlw   0xE0    ; 14  L CTRL
        retlw   0x14    ; 15  Q
        retlw   0x1E    ; 16  1
        retlw   0x00    ; 17  
        retlw   0x00    ; 18
        retlw   0x00    ; 19  
        retlw   0x1D    ; 1A  Z
        retlw   0x16    ; 1B  S
        retlw   0x04    ; 1C  A
        retlw   0x1A    ; 1D  W
        retlw   0x1F    ; 1E  2
        retlw   0x00    ; 1F  
        retlw   0x00    ; 20
        retlw   0x06    ; 21  C
        retlw   0x1B    ; 22  X
        retlw   0x07    ; 23  D
        retlw   0x08    ; 24  E
        retlw   0x21    ; 25  4
        retlw   0x20    ; 26  3
        retlw   0x00    ; 27  
        retlw   0x00    ; 28
        retlw   0x2C    ; 29  SPACE
        retlw   0x19    ; 2A  V
        retlw   0x09    ; 2B  F
        retlw   0x17    ; 2C  T
        retlw   0x15    ; 2D  R
        retlw   0x22    ; 2E  5
        retlw   0x00    ; 2F  
        retlw   0x00    ; 30
        retlw   0x11    ; 31  N
        retlw   0x05    ; 32  B
        retlw   0x0B    ; 33  H
        retlw   0x0A    ; 34  G
        retlw   0x1C    ; 35  Y
        retlw   0x23    ; 36  6
        retlw   0x00    ; 37 
        retlw   0x00    ; 38
        retlw   0x00    ; 39  
        retlw   0x10    ; 3A  M
        retlw   0x0D    ; 3B  J
        retlw   0x18    ; 3C  U
        retlw   0x24    ; 3D  7
        retlw   0x25    ; 3E  8
        retlw   0x00    ; 3F
        retlw   0x00    ; 40
        retlw   0x36    ; 41  ,
        retlw   0x0E    ; 42  K
        retlw   0x0C    ; 43  I
        retlw   0x12    ; 44  O
        retlw   0x27    ; 45  0
        retlw   0x26    ; 46  9
        retlw   0x00    ; 47
        retlw   0x00    ; 48
        retlw   0x37    ; 49  .
        retlw   0x38    ; 4A  /
        retlw   0x0F    ; 4B  L
        retlw   0x33    ; 4C  ;
        retlw   0x13    ; 4D  P
        retlw   0x2D    ; 4E  -
        retlw   0x00    ; 4F  
        retlw   0x00    ; 50
        retlw   0x00    ; 51
        retlw   0x34    ; 52  '
        retlw   0x00    ; 53
        retlw   0x2F    ; 54  [
        retlw   0x2E    ; 55  =
        retlw   0x00    ; 56
        retlw   0x00    ; 57
        retlw   0x39    ; 58  CAPS  
        retlw   0xE5    ; 59  R SHIFT
        retlw   0x28    ; 5A  ENTER
        retlw   0x30    ; 5B  ]
        retlw   0x00    ; 5C  
        retlw   0x31    ; 5D  \ opp. paragrafo$
        retlw   0x00    ; 5E  
        retlw   0x00    ; 5F  
        retlw   0x00    ; 60  
        retlw   0x00    ; 61  
        retlw   0x00    ; 62  
        retlw   0x00    ; 63  
        retlw   0x00    ; 64  
        retlw   0x00    ; 65  
        retlw   0x2A    ; 66  BKSP
        retlw   0x00    ; 67  
        retlw   0x00    ; 68
        retlw   0x59    ; 69  KP 1
        retlw   0x00    ; 6A  
        retlw   0x5C    ; 6B  KP 4
        retlw   0x5F    ; 6C  KP 7
        retlw   0x00    ; 6D  
        retlw   0x00    ; 6E  
        retlw   0x00    ; 6F  
        retlw   0x62    ; 70  KP 0
        retlw   0x63    ; 71  KP .
        retlw   0x5A    ; 72  KP 2
        retlw   0x5D    ; 73  KP 5
        retlw   0x5E    ; 74  KP 6
        retlw   0x60    ; 75  KP 8
        retlw   0x29    ; 76  ESC
        retlw   0x53    ; 77  NUM
        retlw   0x44    ; 78  F11
        retlw   0x57    ; 79  KP +
        retlw   0x5B    ; 7A  KP 3
        retlw   0x56    ; 7B  KP -
        retlw   0x55    ; 7C  KP *
        retlw   0x61    ; 7D  KP 9
        retlw   0x47    ; 7E  SCROLL
        retlw   0x00    ; 7F
        retlw   0x00    ; 80
        retlw   0xE6    ; 81  R ALT
        retlw   0x00    ; 82  
        retlw   0x40    ; 83  F7
        retlw   0xE4    ; 84  R CTRL  
        retlw   0x00    ; 85  
        retlw   0x00    ; 86  
        retlw   0x00    ; 87  
        retlw   0x00    ; 88  
        retlw   0x00    ; 89  
        retlw   0x00    ; 8A  
        retlw   0x00    ; 8B  
        retlw   0x00    ; 8C  
        retlw   0x00    ; 8D
        retlw   0x00    ; 8E  
        retlw   0xE3    ; 8F  L GUI  
        retlw   0x00    ; 90  
        retlw   0x00    ; 91  
        retlw   0x00    ; 92  
        retlw   0x00    ; 93  
        retlw   0x00    ; 94  
        retlw   0x00    ; 95  
        retlw   0x00    ; 96  
        retlw   0xE7    ; 97  R GUI
        retlw   0x00    ; 98  
        retlw   0x00    ; 99  
        retlw   0x00    ; 9A  
        retlw   0x00    ; 9B  
        retlw   0x00    ; 9C  
        retlw   0x00    ; 9D  
        retlw   0x00    ; 9E  
        retlw   0x65    ; 9F  APPS
        retlw   0x00    ; A0  
        retlw   0x00    ; A1  
        retlw   0x00    ; A2  
        retlw   0x00    ; A3  
        retlw   0x00    ; A4  
        retlw   0x00    ; A5  
        retlw   0x00    ; A6  
        retlw   0x00    ; A7  
        retlw   0x00    ; A8  
        retlw   0x00    ; A9  
        retlw   0x00    ; AA  
        retlw   0x00    ; AB  
        retlw   0x00    ; AC  
        retlw   0x00    ; AD  
        retlw   0x00    ; AE  
        retlw   0x00    ; AF  
        retlw   0x00    ; B0  
        retlw   0x00    ; B1 
        retlw   0x00    ; B2 
        retlw   0x00    ; B3 
        retlw   0x00    ; B4 
        retlw   0x00    ; B5 
        retlw   0x00    ; B6 
        retlw   0x00    ; B7 
        retlw   0x00    ; B8 
        retlw   0x00    ; B9 
        retlw   0x54    ; BA  KP /
        retlw   0x00    ; BB 
        retlw   0x00    ; BC 
        retlw   0x00    ; BD 
        retlw   0x00    ; BE 
        retlw   0x00    ; BF 
        retlw   0x00    ; C0  
        retlw   0x00    ; C1  
        retlw   0x00    ; C2  
        retlw   0x00    ; C3  
        retlw   0x00    ; C4  
        retlw   0x00    ; C5  
        retlw   0x00    ; C6  
        retlw   0x00    ; C7  
        retlw   0x00    ; C8  
        retlw   0x00    ; C9  
        retlw   0x58    ; CA  KP ENTER
        retlw   0x00    ; CB  
        retlw   0x00    ; CC  
        retlw   0x00    ; CD  
        retlw   0x00    ; CE  
        retlw   0x00    ; CF  
        retlw   0x00    ; D0  
        retlw   0x00    ; D1  
        retlw   0x00    ; D2  
        retlw   0x00    ; D3  
        retlw   0x00    ; D4  
        retlw   0x00    ; D5  
        retlw   0x00    ; D6  
        retlw   0x00    ; D7  
        retlw   0x00    ; D8  
        retlw   0x4D    ; D9  END
        retlw   0x00    ; DA  
        retlw   0x50    ; DB  L ARROW
        retlw   0x4A    ; DC  HOME
        retlw   0x00    ; DD  
        retlw   0x00    ; DE  
        retlw   0x00    ; DF  
        retlw   0x49    ; E0  INSERT
        retlw   0x4C    ; E1  DELETE
        retlw   0x51    ; E2  D ARROW
        retlw   0x00    ; E3  
        retlw   0x4F    ; E4  R ARROW
        retlw   0x52    ; E5  U ARROW
        retlw   0x00    ; E6  
        retlw   0x00    ; E7  
        retlw   0x00    ; E8  
        retlw   0x00    ; E9  
        retlw   0x4E    ; EA  PG DN
        retlw   0x00    ; EB  
        retlw   0x46    ; EC  PRNT SCRN
        dt   0x4B    ; ED  PG UP
        dt   0x00    ; EE  
        dt   0x00    ; EF  
				0x64 = @ 
				0x35 = <
				0x31 = (paragrafo$)
				0x48 = PAUSE (F15)

// nel primo BYTE del pacchetto (STATUS), abbiamo:
// 01=L CTRL
//	02=L SHFT
// 04=L ALT
// 08=L MELA
// 10=R CTRL
//	20=R SHFT
// 40=R ALT
// 80=R MELA

// se vengono premuti piu' di 6 tasti (qui, quattro ossia max gestiti), mandare 6 BYTE a 0x01 (ErrorRollOver, v.HID_USAGE_TABLE.PDF)

#endif


/** EOF kblayout.c *********************************************************/
