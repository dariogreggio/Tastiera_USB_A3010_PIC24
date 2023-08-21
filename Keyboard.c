/********************************************************************
 FileName:		Keyboard.c
 Dependencies:  See INCLUDES section
 Processor:     PIC18, PIC24, dsPIC, and PIC32 USB Microcontrollers
 Hardware:      This demo is natively intended to be used on Microchip USB demo
                boards supported by the MCHPFSUSB stack.  See release notes for
                support matrix.  This demo can be modified for use on other hardware
                platforms.
 Complier:      Microchip C18 (for PIC18), XC16 (for PIC24/dsPIC), XC32 (for PIC32)
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

********************************************************************
 File Description:

 Change History:
  Rev   Description
  ----  -----------------------------------------
  1.0   Initial release
  2.1   Updated for simplicity and to use common
                     coding style
  2.8   Improvements to USBCBSendResume(), to make it easier to use.
  2.9   Added remote wakeup capability to the demo.
  2.9f  Adding new part support
********************************************************************/

#ifndef KEYBOARD_C
#define KEYBOARD_C

/** INCLUDES *******************************************************/
#include "compiler.h"
#include "usb.h"
#include "HardwareProfile.h"
#include "usb_function_hid.h"

#ifdef __C30__
#include <libpic30.h>
#endif

#include <ctype.h>

#include "swi2c.h"


/** CONFIGURATION **************************************************/
#if defined(PICDEM_FS_USB)      // Configuration bits for PICDEM FS USB Demo Board (based on PIC18F4550)
        #pragma config PLLDIV   = 1         // (4 MHz crystal on PICDEM FS USB board)
        #if (USB_SPEED_OPTION == USB_FULL_SPEED)
            #pragma config CPUDIV   = OSC1_PLL2  
        #else
            #pragma config CPUDIV   = OSC3_PLL4   
        #endif
        #pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
        #pragma config FOSC     = HSPLL_HS
        #pragma config FCMEN    = OFF
        #pragma config IESO     = OFF
        #pragma config PWRT     = OFF
        #pragma config BOR      = ON
        #pragma config BORV     = 3
        #pragma config VREGEN   = ON      //USB Voltage Regulator
        #pragma config WDT      = OFF
        #pragma config WDTPS    = 32768
        #pragma config MCLRE    = ON
        #pragma config LPT1OSC  = OFF
        #pragma config PBADEN   = OFF
//      #pragma config CCP2MX   = ON
        #pragma config STVREN   = ON
        #pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
        #pragma config XINST    = OFF       // Extended Instruction Set
        #pragma config CP0      = OFF
        #pragma config CP1      = OFF
//      #pragma config CP2      = OFF
//      #pragma config CP3      = OFF
        #pragma config CPB      = OFF
//      #pragma config CPD      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
//      #pragma config WRTD     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
        #pragma config EBTRB    = OFF

#elif defined(PICDEM_FS_USB_K50)
        #pragma config PLLSEL   = PLL3X     // 3X PLL multiplier selected
        #pragma config CFGPLLEN = OFF       // PLL turned on during execution
        #pragma config CPUDIV   = NOCLKDIV  // 1:1 mode (for 48MHz CPU)
        #pragma config LS48MHZ  = SYS48X8   // Clock div / 8 in Low Speed USB mode
        #pragma config FOSC     = INTOSCIO  // HFINTOSC selected at powerup, no clock out
        #pragma config PCLKEN   = OFF       // Primary oscillator driver
        #pragma config FCMEN    = OFF       // Fail safe clock monitor
        #pragma config IESO     = OFF       // Internal/external switchover (two speed startup)
        #pragma config nPWRTEN  = OFF       // Power up timer
        #pragma config BOREN    = SBORDIS   // BOR enabled
        #pragma config nLPBOR   = ON        // Low Power BOR
        #pragma config WDTEN    = SWON      // Watchdog Timer controlled by SWDTEN
        #pragma config WDTPS    = 32768     // WDT postscalar
        #pragma config PBADEN   = OFF       // Port B Digital/Analog Powerup Behavior
        #pragma config SDOMX    = RC7       // SDO function location
        #pragma config LVP      = OFF       // Low voltage programming
        #pragma config MCLRE    = ON        // MCLR function enabled (RE3 disabled)
        #pragma config STVREN   = ON        // Stack overflow reset
        //#pragma config ICPRT  = OFF       // Dedicated ICPORT program/debug pins enable
        #pragma config XINST    = OFF       // Extended instruction set

#elif defined(PIC18F87J50_PIM)				// Configuration bits for PIC18F87J50 FS USB Plug-In Module board
        #pragma config XINST    = OFF   	// Extended instruction set
        #pragma config STVREN   = ON      	// Stack overflow reset
        #pragma config PLLDIV   = 3         // (12 MHz crystal used on this board)
        #pragma config WDTEN    = OFF      	// Watch Dog Timer (WDT)
        #pragma config CP0      = OFF      	// Code protect
        #pragma config CPUDIV   = OSC1      // OSC1 = divide by 1 mode
        #pragma config IESO     = OFF      	// Internal External (clock) Switchover
        #pragma config FCMEN    = OFF      	// Fail Safe Clock Monitor
        #pragma config FOSC     = HSPLL     // Firmware must also set OSCTUNE<PLLEN> to start PLL!
        #pragma config WDTPS    = 32768
//      #pragma config WAIT     = OFF      	// Commented choices are
//      #pragma config BW       = 16      	// only available on the
//      #pragma config MODE     = MM      	// 80 pin devices in the 
//      #pragma config EASHFT   = OFF      	// family.
        #pragma config MSSPMSK  = MSK5
//      #pragma config PMPMX    = DEFAULT
//      #pragma config ECCPMX   = DEFAULT
        #pragma config CCP2MX   = DEFAULT   
        
// Configuration bits for PIC18F97J94 PIM and PIC18F87J94 PIM
#elif defined(PIC18F97J94_PIM) || defined(PIC18F87J94_PIM)
        #pragma config STVREN   = ON      	// Stack overflow reset
        #pragma config XINST    = OFF   	// Extended instruction set
        #pragma config BOREN    = ON        // BOR Enabled
        #pragma config BORV     = 0         // BOR Set to "2.0V" nominal setting
        #pragma config CP0      = OFF      	// Code protect disabled
        #pragma config FOSC     = FRCPLL    // Firmware should also enable active clock tuning for this setting
        #pragma config SOSCSEL  = LOW       // SOSC circuit configured for crystal driver mode
        #pragma config CLKOEN   = OFF       // Disable clock output on RA6
        #pragma config IESO     = OFF      	// Internal External (clock) Switchover
        #pragma config PLLDIV   = NODIV     // 4 MHz input (from 8MHz FRC / 2) provided to PLL circuit
        #pragma config POSCMD   = NONE      // Primary osc disabled, using FRC
        #pragma config FSCM     = CSECMD    // Clock switching enabled, fail safe clock monitor disabled
        #pragma config WPDIS    = WPDIS     // Program memory not write protected
        #pragma config WPCFG    = WPCFGDIS  // Config WORD page of program memory not write protected
        #pragma config IOL1WAY  = OFF       // IOLOCK can be set/cleared as needed with unlock sequence
        #pragma config LS48MHZ  = SYSX2     // Low Speed USB clock divider
        #pragma config WDTCLK   = LPRC      // WDT always uses INTOSC/LPRC oscillator
        #pragma config WDTEN    = ON        // WDT disabled; SWDTEN can control WDT
        #pragma config WINDIS   = WDTSTD    // Normal non-window mode WDT.
        #pragma config VBTBOR   = OFF       // VBAT BOR disabled
      
#elif defined(PIC18F46J50_PIM) || defined(PIC18F_STARTER_KIT_1) || defined(PIC18F47J53_PIM)
     #pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
     #pragma config STVREN = ON          //stack overflow/underflow reset enabled
     #pragma config XINST = OFF          //Extended instruction set disabled
     #pragma config CPUDIV = OSC1        //No CPU system clock divide
     #pragma config CP0 = OFF            //Program memory is not code-protected
     #pragma config OSC = HSPLL          //HS oscillator, PLL enabled, HSPLL used by USB
     #pragma config FCMEN = OFF          //Fail-Safe Clock Monitor disabled
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
     #pragma config WDTPS = 32768        //1:32768
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as clock
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as clock
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep
     #pragma config DSWDTEN = OFF        //Disabled
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
     #pragma config IOL1WAY = OFF        //IOLOCK bit can be set and cleared
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
     #pragma config WPEND = PAGE_0       //Start protection at page 0
     #pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored 
     #if defined(PIC18F47J53_PIM)
        #pragma config CFGPLLEN = OFF
     #else
        #pragma config T1DIG = ON           //Sec Osc clock source may be selected
        #pragma config LPT1OSC = OFF        //high power Timer1 mode
     #endif
#elif defined(LOW_PIN_COUNT_USB_DEVELOPMENT_KIT)
        #pragma config CPUDIV = NOCLKDIV
        #pragma config USBDIV = OFF
        #pragma config FOSC   = HS
        #pragma config PLLEN  = ON
        #pragma config FCMEN  = OFF
        #pragma config IESO   = OFF
        #pragma config PWRTEN = OFF
        #pragma config BOREN  = OFF
        #pragma config BORV   = 30
        #pragma config WDTEN  = OFF
        #pragma config WDTPS  = 32768
        #pragma config MCLRE  = OFF
        #pragma config HFOFST = OFF
        #pragma config STVREN = ON
        #pragma config LVP    = OFF
        #pragma config XINST  = OFF
        #pragma config BBSIZ  = OFF
        #pragma config CP0    = OFF
        #pragma config CP1    = OFF
        #pragma config CPB    = OFF
        #pragma config WRT0   = OFF
        #pragma config WRT1   = OFF
        #pragma config WRTB   = OFF
        #pragma config WRTC   = OFF
        #pragma config EBTR0  = OFF
        #pragma config EBTR1  = OFF
        #pragma config EBTRB  = OFF        

#elif	defined(PIC16F1_LPC_USB_DEVELOPMENT_KIT)
    // PIC 16F1459 fuse configuration:
    #if defined (USE_INTERNAL_OSC)  //Definition in the hardware profile
        __CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_ON & MCLRE_OFF & CP_OFF & BOREN_ON & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
        __CONFIG(WRT_OFF & CPUDIV_NOCLKDIV & USBLSCLK_48MHz & PLLMULT_3x & PLLEN_ENABLED & STVREN_ON &  BORV_LO & LPBOR_OFF & LVP_OFF);
    #else
        __CONFIG(FOSC_HS & WDTE_OFF & PWRTE_ON & MCLRE_OFF & CP_OFF & BOREN_ON & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
        __CONFIG(WRT_OFF & CPUDIV_NOCLKDIV & USBLSCLK_48MHz & PLLMULT_4x & PLLEN_ENABLED & STVREN_ON &  BORV_LO & LPBOR_OFF & LVP_OFF);
    #endif

#elif defined(EXPLORER_16)
    #if defined(__PIC24FJ256GB110__) || defined(__PIC24FJ256GB106__)
        _CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx2) 
        _CONFIG2( PLL_96MHZ_ON & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV2 & IOL1WAY_ON)
    #elif defined(PIC24FJ256GB210_PIM)
        _CONFIG1(FWDTEN_OFF & ICS_PGx2 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
        _CONFIG2(POSCMOD_HS & IOL1WAY_ON & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2 & IESO_OFF)
    #elif defined(__PIC24FJ64GB004__) || defined(__PIC24FJ64GB002__)
// quarzo        _CONFIG1(WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx1 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
//        _CONFIG2(POSCMOD_HS & I2C1SEL_PRI & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2 & IESO_OFF)
//        _CONFIG3(WPFP_WPFP0 & SOSCSEL_SOSC & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)
//        _CONFIG4(DSWDTPS_DSWDTPS3 & DSWDTOSC_LPRC & RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)

//senza quarzo
        _CONFIG1(WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx1 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
//        _CONFIG2(POSCMOD_HS & I2C1SEL_PRI & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_NODIV & IESO_ON)		// quarzo 4MHz
        _CONFIG2(POSCMOD_HS & I2C1SEL_PRI & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_FRCPLL & PLL96MHZ_ON & PLLDIV_NODIV & IESO_ON)		// osc int
//        _CONFIG3(WPFP_WPFP0 & SOSCSEL_SOSC & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)			SOSC??? secondari osc.. NO RB4!
        _CONFIG3(WPFP_WPFP0 & SOSCSEL_IO & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)
        _CONFIG4(DSWDTPS_DSWDTPS3 & DSWDTOSC_LPRC & RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)
    #elif defined(__32MX460F512L__) || defined(__32MX795F512L__)
        #pragma config UPLLEN   = ON        // USB PLL Enabled
        #pragma config FPLLMUL  = MUL_15        // PLL Multiplier
        #pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider
        #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
        #pragma config FPLLODIV = DIV_1         // PLL Output Divider
        #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
        #pragma config FWDTEN   = OFF           // Watchdog Timer
        #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
        //#pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
        #pragma config OSCIOFNC = OFF           // CLKO Enable
        #pragma config POSCMOD  = HS            // Primary Oscillator
        #pragma config IESO     = OFF           // Internal/External Switch-over
        #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
        #pragma config FNOSC    = PRIPLL        // Oscillator Selection
        #pragma config CP       = OFF           // Code Protect
        #pragma config BWP      = OFF           // Boot Flash Write Protect
        #pragma config PWP      = OFF           // Program Flash Write Protect
        #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
    #elif defined(__dsPIC33EP512MU810__)||defined(__PIC24EP512GU810__)
        _FOSCSEL(FNOSC_FRC);
        _FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT);
        _FWDT(FWDTEN_OFF);
    #else
        #error No hardware board defined, see "HardwareProfile.h" and __FILE__
    #endif
#elif defined(PIC24F_STARTER_KIT)
    _CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx2) 
    _CONFIG2( PLL_96MHZ_ON & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)
#elif defined(PIC24FJ256DA210_DEV_BOARD)
    _CONFIG1(FWDTEN_OFF & ICS_PGx2 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
    _CONFIG2(POSCMOD_HS & IOL1WAY_ON & OSCIOFNC_ON & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2 & IESO_OFF)
#elif defined(PIC32_USB_STARTER_KIT)
	//This covers both the PIC32 USB Starter Kit and the PIC32 USB Starter Kit II
    #pragma config UPLLEN   = ON        // USB PLL Enabled
    #pragma config FPLLMUL  = MUL_15        // PLL Multiplier
    #pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider
    #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
    #pragma config FPLLODIV = DIV_1         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer
    #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
    //#pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = HS            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
#elif defined(PIC24FJ64GB502_MICROSTICK)
    _CONFIG1(WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx1 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
    _CONFIG2(I2C1SEL_PRI & IOL1WAY_OFF & FCKSM_CSDCMD & FNOSC_PRIPLL & PLL96MHZ_ON & PLLDIV_DIV2 & IESO_OFF)
    _CONFIG3(WPFP_WPFP0 & SOSCSEL_SOSC & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)
    _CONFIG4(DSWDTPS_DSWDTPS3 & DSWDTOSC_LPRC & RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)
#else
    #error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif



/** VARIABLES ******************************************************/
#if defined(__18CXX)
    #pragma udata

    //The ReceivedDataBuffer[] and ToSendDataBuffer[] arrays are used as
    //USB packet buffers in this firmware.  Therefore, they must be located in
    //a USB module accessible portion of microcontroller RAM.
    #if defined(__18F14K50) || defined(__18F13K50) || defined(__18LF14K50) || defined(__18LF13K50)
        #pragma udata USB_VARIABLES=0x260
    #elif defined(__18F2455) || defined(__18F2550) || defined(__18F4455) || defined(__18F4550)\
        || defined(__18F2458) || defined(__18F2453) || defined(__18F4558) || defined(__18F4553)\
        || defined(__18LF24K50) || defined(__18F24K50) || defined(__18LF25K50)\
        || defined(__18F25K50) || defined(__18LF45K50) || defined(__18F45K50)
        #pragma udata USB_VARIABLES=0x500
    #elif defined(__18F4450) || defined(__18F2450)
        #pragma udata USB_VARIABLES=0x480
    #else
        #pragma udata
    #endif
#endif

#if defined(__XC8)
    #if defined(_18F14K50) || defined(_18F13K50) || defined(_18LF14K50) || defined(_18LF13K50)
        #define IN_DATA_BUFFER_ADDRESS 0x260
        #define OUT_DATA_BUFFER_ADDRESS (IN_DATA_BUFFER_ADDRESS + HID_INT_IN_EP_SIZE)
        #define IN_DATA_BUFFER_ADDRESS_TAG @IN_DATA_BUFFER_ADDRESS
        #define OUT_DATA_BUFFER_ADDRESS_TAG @OUT_DATA_BUFFER_ADDRESS
    #elif  defined(_18F2455)   || defined(_18F2550)   || defined(_18F4455)  || defined(_18F4550)\
        || defined(_18F2458)   || defined(_18F2453)   || defined(_18F4558)  || defined(_18F4553)\
        || defined(_18LF24K50) || defined(_18F24K50)  || defined(_18LF25K50)\
        || defined(_18F25K50)  || defined(_18LF45K50) || defined(_18F45K50)
        #define IN_DATA_BUFFER_ADDRESS 0x500
        #define OUT_DATA_BUFFER_ADDRESS (IN_DATA_BUFFER_ADDRESS + HID_INT_IN_EP_SIZE)
        #define IN_DATA_BUFFER_ADDRESS_TAG @IN_DATA_BUFFER_ADDRESS
        #define OUT_DATA_BUFFER_ADDRESS_TAG @OUT_DATA_BUFFER_ADDRESS
    #elif defined(_18F4450) || defined(_18F2450)
        #define IN_DATA_BUFFER_ADDRESS 0x480
        #define OUT_DATA_BUFFER_ADDRESS (IN_DATA_BUFFER_ADDRESS + HID_INT_IN_EP_SIZE)
        #define IN_DATA_BUFFER_ADDRESS_TAG @IN_DATA_BUFFER_ADDRESS
        #define OUT_DATA_BUFFER_ADDRESS_TAG @OUT_DATA_BUFFER_ADDRESS
    #elif defined(_16F1459)
        #define IN_DATA_BUFFER_ADDRESS 0x2050
        #define OUT_DATA_BUFFER_ADDRESS (IN_DATA_BUFFER_ADDRESS + HID_INT_IN_EP_SIZE)
        #define IN_DATA_BUFFER_ADDRESS_TAG @IN_DATA_BUFFER_ADDRESS
        #define OUT_DATA_BUFFER_ADDRESS_TAG @OUT_DATA_BUFFER_ADDRESS
    #else
        #define IN_DATA_BUFFER_ADDRESS_TAG
        #define OUT_DATA_BUFFER_ADDRESS_TAG
    #endif
#else
    #define IN_DATA_BUFFER_ADDRESS_TAG
    #define OUT_DATA_BUFFER_ADDRESS_TAG
#endif

//volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE] IN_DATA_BUFFER_ADDRESS_TAG;
//volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE] OUT_DATA_BUFFER_ADDRESS_TAG;

#ifndef __C30__
#pragma romdata
#endif
ROM static const char USB_MK_C[]="$Id:USB_K.C Ver 2.4.0 - 30/8/23";


#define TRIGM			0                //TRIGGER FLAG mouse update
#define TRIGK			1                //TRIGGER FLAG kb update
#define TRIGK2		2                //TRIGGER FLAG keyboard update vuoto (tutti zeri)

#if defined(__18CXX)
#pragma udata
#endif

#define MAX_ASSOLUTO_TASTI_CONTEMPORANEI 8

enum KB_USB_ERRORI {
	ERROR_NONE=0,
	ERROR_ROLLOVER=1,
	ERROR_POSTFAIL=2,
	ERROR_UNDEFINED=3
	};

#if defined(__18CXX)
    #pragma udata
#endif
BYTE old_sw2,old_sw3;
struct CONFIG_TASTIERA configTastiera;
extern const ROM BYTE idTastiera;			// v. kblayout.c
extern struct KB_LAYOUT_HEADER kbLayoutHeader;
BYTE kbLayout,kbLayoutTemp;
BYTE /*kbBufferO[9],*/ kbKeys[8 /*MAX_TASTI_CONTEMPORANEI*/];
#warning ovviamente se in config ci metti di più va tutto a troie...
#define kbBufferO hid_report_in
#ifdef MULTIMEDIA_KEYBOARD
//BYTE kbBufferO2[2];
#endif
#ifdef TUTTI_CONTRO_TUTTI 
WORD rowState[9];			//9x9 slots
#else
WORD rowState[10];
#endif
BYTE kbScanCode,kbScanCodeSaved;
BYTE FLAGA;        //GENERAL PURPOSE FLAG

USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;
//unsigned char ReceivedDataBuffer[64];	//
//unsigned char ToSendDataBuffer[64];
#define ReceivedDataBuffer hid_report_out2
#define ToSendDataBuffer hid_report_in2
USB_HANDLE lastINTransmission;
//USB_HANDLE lastOUTTransmission=0; usiamo control report per i led!
#ifdef KEYBOARD_WITH_DATA
BOOL keyMode=0;
#endif
BYTE scanIdleCnt;
BOOL BlinkStatusValid;
DWORD CountdownTimerToShowUSBStatusOnLEDs;
extern BYTE idle_rate;		// v. hid.c
#ifdef USE_SW_SUSPEND
BYTE inSuspend=FALSE;
void USBCBWakeFromSuspend(void);
#endif

/** PRIVATE PROTOTYPES *********************************************/
void BlinkUSBStatus(void);
BOOL Switch2IsPressed(void);
BOOL Switch3IsPressed(void);
static void InitializeSystem(void);
void ProcessIO(void);
void UserInit(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);
void Keyboard(void);
BYTE count0Bit(WORD);
BYTE convertModifCode(BYTE);
void scanKBD(void);
void checkKey(BYTE);
void scanDoError(BYTE);
BYTE *flushCurrentBuffer(void);
void presetBuffer(BYTE );
BYTE GetKBchar(BYTE);
BYTE GetKBcharNext(void);

BYTE *Handle240(BYTE *);

void USBHIDCBSetReportComplete(void);

/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	//On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
	//the reset, high priority interrupt, and low priority interrupt
	//vectors.  However, the current Microchip USB bootloader 
	//examples are intended to occupy addresses 0x00-0x7FF or
	//0x00-0xFFF depending on which bootloader is used.  Therefore,
	//the bootloader code remaps these vectors to new locations
	//as indicated below.  This remapping is only necessary if you
	//wish to program the hex file generated from this project with
	//the USB bootloader.  If no bootloader is used, edit the
	//usb_config.h file and comment out the following defines:
	//#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
	//#define PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
#warning RICORDARSI DEL LINKER FILE rm...
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
	#else	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
	#endif
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);        // See c018i.c in your C18 compiler dir
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
	    _asm goto _startup _endasm
	}
	#endif
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
	     _asm goto YourHighPriorityISRCode _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto YourLowPriorityISRCode _endasm
	}
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	//Note: If this project is built while one of the bootloaders has
	//been defined, but then the output hex file is not programmed with
	//the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
	//As a result, if an actual interrupt was enabled and occured, the PC would jump
	//to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
	//executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
	//(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
	//would effective reset the application.
	
	//To fix this situation, we should always deliberately place a 
	//"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
	//"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
	//hex file of this project is programmed with the bootloader, these sections do not
	//get bootloaded (as they overlap the bootloader space).  If the output hex file is not
	//programmed using the bootloader, then the below goto instructions do get programmed,
	//and the hex file still works like normal.  The below section is only required to fix this
	//scenario.
	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
	     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
	     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

	#pragma code
	
	
	//These are your actual interrupt handling routines.
	#pragma interrupt YourHighPriorityISRCode
	void YourHighPriorityISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
        #if defined(USB_INTERRUPT)
	        USBDeviceTasks();
        #endif
	
	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
	
	}	//This return will be a "retfie", since this is in a #pragma interruptlow section 

#elif defined(__C30__) || defined __XC16__
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
        /*
         *	ISR JUMP TABLE
         *
         *	It is necessary to define jump table as a function because C30 will
         *	not store 24-bit wide values in program memory as variables.
         *
         *	This function should be stored at an address where the goto instructions 
         *	line up with the remapped vectors from the bootloader's linker script.
         *  
         *  For more information about how to remap the interrupt vectors,
         *  please refer to AN1157.  An example is provided below for the T2
         *  interrupt with a bootloader ending at address 0x1400
         */
//        void __attribute__ ((address(0x1404))) ISRTable(){
//        
//        	asm("reset"); //reset instruction to prevent runaway code
//        	asm("goto %0"::"i"(&_T2Interrupt));  //T2Interrupt's address
//        }
    #endif
#elif defined(_PIC14E)
    	//These are your actual interrupt handling routines.
	void interrupt ISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
        #if defined(USB_INTERRUPT)

	        USBDeviceTasks();
        #endif
	}
#endif





/** DECLARATIONS ***************************************************/
#if defined(__18CXX)
    #pragma code
#endif

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
  InitializeSystem();


  #if defined(USB_INTERRUPT)
    USBDeviceAttach();
  #endif


  while(1) {

		ClrWdt();

    #if defined(USB_POLLING)
		// Check bus status and service USB interrupts.
      USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        				  // this function periodically.  This function will take care
        				  // of processing and responding to SETUP transactions 
        				  // (such as during the enumeration process when you first
        				  // plug in).  USB hosts require that USB devices should accept
        				  // and process SETUP packets in a timely fashion.  Therefore,
        				  // when using polling, this function should be called 
        				  // regularly (such as once every 1.8ms or faster** [see 
        				  // inline code comments in usb_device.c for explanation when
        				  // "or faster" applies])  In most cases, the USBDeviceTasks() 
        				  // function does not take very long to execute (ex: <100 
        				  // instruction cycles) before it returns.
    #endif
    				  
#ifdef USE_SW_SUSPEND
	if(inSuspend) {
		}
#endif

		// Application-specific tasks.
		// Application related code may be added here, or in the ProcessIO() function.
    ProcessIO();        
    }	//end while
	}	//end main


/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void) {

  #if defined(_PIC14E)
      //Configure all pins for digital mode, except RB4, which has a POT on it
      ANSELA = 0x00;
      #if defined(_16F1459) || defined(_16LF1459)
          ANSELB = 0x10;  //RB4 has a POT on it, on the Low Pin Count USB Dev Kit board
      #endif
      ANSELC = 0x00;
      #if defined (USE_INTERNAL_OSC)
          OSCTUNE = 0;
          OSCCON = 0xFC;          //16MHz HFINTOSC with 3x PLL enabled (48MHz operation)
          ACTCON = 0x90;          //Enable active clock tuning with USB
      #endif
  #endif

  #if (defined(__18CXX) & !defined(PIC18F87J50_PIM) & !defined(PIC18F97J94_FAMILY))
      ADCON1 |= 0x0F;                 // Default all pins to digital
  #elif defined(__C30__) || defined __XC16__
      #if defined(__PIC24FJ256GB110__) || defined(__PIC24FJ256GB106__)
          AD1PCFGL = 0xFFFF;
       #elif defined(__dsPIC33EP512MU810__)||defined(__PIC24EP512GU810__)
      	ANSELA = 0x0000;
  		ANSELB = 0x0000;
  		ANSELC = 0x0000;
  		ANSELD = 0x0000;
  		ANSELE = 0x0000;
  		ANSELG = 0x0000;
          
          // The dsPIC33EP512MU810 features Peripheral Pin
          // select. The following statements map UART2 to 
          // device pins which would connect to the the 
          // RX232 transciever on the Explorer 16 board.

           RPINR19 = 0;
           RPINR19 = 0x64;
           RPOR9bits.RP101R = 0x3;

      #endif
  #elif defined(__C32__)
      AD1PCFG = 0xFFFF;
  #endif

  #if defined(PIC18F87J50_PIM) || defined(PIC18F46J50_PIM) || defined(PIC18F_STARTER_KIT_1) || defined(PIC18F47J53_PIM)
  	//On the PIC18F87J50 Family of USB microcontrollers, the PLL will not power up and be enabled
  	//by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
  	//This allows the device to power up at a lower initial operating frequency, which can be
  	//advantageous when powered from a source which is not gauranteed to be adequate for 48MHz
  	//operation.  On these devices, user firmware needs to manually set the OSCTUNE<PLLEN> bit to
  	//power up the PLL.
      {
          unsigned int pll_startup_counter = 600;
          OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
          while(pll_startup_counter--);
      }
      //Device switches over automatically to PLL output after PLL is locked and ready.
  #endif

  #if defined(PIC18F87J50_PIM)
  	//Configure all I/O pins to use digital input buffers.  The PIC18F87J50 Family devices
  	//use the ANCONx registers to control this, which is different from other devices which
  	//use the ADCON1 register for this purpose.
      WDTCONbits.ADSHR = 1;			// Select alternate SFR location to access ANCONx registers
      ANCON0 = 0xFF;                  // Default all pins to digital
      ANCON1 = 0xFF;                  // Default all pins to digital
      WDTCONbits.ADSHR = 0;			// Select normal SFR locations
  #endif

	#if defined(PIC18F97J94_FAMILY)
	    //Configure I/O pins for digital input mode.
	    ANCON1 = 0xFF;
	    ANCON2 = 0xFF;
	    ANCON3 = 0xFF;
	    #if(USB_SPEED_OPTION == USB_FULL_SPEED)
	        //Enable INTOSC active clock tuning if full speed
	        ACTCON = 0x90; //Enable active clock self tuning for USB operation
	        while(OSCCON2bits.LOCK == 0);   //Make sure PLL is locked/frequency is compatible
	                                        //with USB operation (ex: if using two speed 
	                                        //startup or otherwise performing clock switching)
	    #endif
	#endif
	
	#if defined(PIC18F45K50_FAMILY)
	    //Configure oscillator settings for clock settings compatible with USB 
	    //operation.  Note: Proper settings depends on USB speed (full or low).
	    #if(USB_SPEED_OPTION == USB_FULL_SPEED)
	        OSCTUNE = 0x80; //3X PLL ratio mode selected
	        OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
	        OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
	        while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
	        *((unsigned char*)0xFB5) = 0x90;  //Enable active clock tuning for USB operation
	    #endif
	#endif
	
	#if defined(PIC18F46J50_PIM) || defined(PIC18F_STARTER_KIT_1) || defined(PIC18F47J53_PIM)
	//Configure all I/O pins to use digital input buffers.  The PIC18F87J50 Family devices
	//use the ANCONx registers to control this, which is different from other devices which
	//use the ADCON1 register for this purpose.
	ANCON0 = 0x7F;                  // All pins to digital except AN7 (temp sensor)
	ANCON1 = 0xBF;                  // Default all pins to digital.  Bandgap on.
	#endif
    
  #if defined(PIC24FJ64GB004_PIM) || defined(PIC24FJ256DA210_DEV_BOARD)
	//On the PIC24FJ64GB004 Family of USB microcontrollers, the PLL will not power up and be enabled
	//by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
	//This allows the device to power up at a lower initial operating frequency, which can be
	//advantageous when powered from a source which is not gauranteed to be adequate for 32MHz
	//operation.  On these devices, user firmware needs to manually set the CLKDIV<PLLEN> bit to
	//power up the PLL.
    {
        unsigned int pll_startup_counter = 600;
        CLKDIVbits.PLLEN = 1;
        while(pll_startup_counter--);
    }

    //Device switches over automatically to PLL output after PLL is locked and ready.
    #endif
    
    #if defined(__32MX460F512L__)|| defined(__32MX795F512L__)
    // Configure the PIC32 core for the best performance
    // at the operating frequency. The operating frequency is already set to 
    // 60MHz through Device Config Registers
    SYSTEMConfigPerformance(60000000);
	#endif

#if defined(__dsPIC33EP512MU810__)||defined(__PIC24EP512GU810__)

    // Configure the device PLL to obtain 60 MIPS operation. The crystal
    // frequency is 8MHz. Divide 8MHz by 2, multiply by 60 and divide by
    // 2. This results in Fosc of 120MHz. The CPU clock frequency is
    // Fcy = Fosc/2 = 60MHz. Wait for the Primary PLL to lock and then
    // configure the auxilliary PLL to provide 48MHz needed for USB 
    // Operation.

	PLLFBD = 58;				/* M  = 60	*/
	CLKDIVbits.PLLPOST = 0;		/* N1 = 2	*/
	CLKDIVbits.PLLPRE = 0;		/* N2 = 2	*/
	OSCTUN = 0;			

    /*	Initiate Clock Switch to Primary
     *	Oscillator with PLL (NOSC= 0x3)*/
	
    __builtin_write_OSCCONH(0x03);		
	__builtin_write_OSCCONL(0x01);
	
	while (OSCCONbits.COSC != 0x3);       

  // Configuring the auxiliary PLL, since the primary
  // oscillator provides the source clock to the auxiliary
  // PLL, the auxiliary oscillator is disabled. Note that
  // the AUX PLL is enabled. The input 8MHz clock is divided
  // by 2, multiplied by 24 and then divided by 2. Wait till 
  // the AUX PLL locks.

  ACLKCON3 = 0x24C1;   
  ACLKDIV3 = 0x7;
  
  ACLKCON3bits.ENAPLL = 1;
  while(ACLKCON3bits.APLLCK != 1); 

#endif


//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.  Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
// 	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
// 	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.  This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	HardwareProfile.h file.    
  #if defined(USE_USB_BUS_SENSE_IO)
  tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
  #endif
    
//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.  If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2" 
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in HardwareProfile - (platform).h, and that an appropriate I/O pin 
//  has been mapped	to it.
  #if defined(USE_SELF_POWER_SENSE_IO)
  tris_self_power = INPUT_PIN;	// See HardwareProfile.h
  #endif
    
  UserInit();

  USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
  			  					//variables to known states.
	}	//end InitializeSystem



/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the demo code
 *                  initialization that is required.
 *
 * Note:            
 *
 *****************************************************************************/
void UserInit(void) {

#if !defined(__PIC24FJ256GB106__)
	TRISA=0;
#endif
	TRISB=0;
	TRISC=0b00000000;
	TRISD=0b11111111;
	TRISE=0b00000111;

	LATC=0;			// 
	LATD=0;
	LATE=0;

  //Initialize all of the LED pins
  mInitAllLEDs();
  BlinkStatusValid = TRUE;
  
  //Initialize all of the push buttons
  mInitAllSwitches();
  old_sw2 = sw2;
  old_sw3 = sw3;

	Delay_ms(100);			// wait to settle up
	ClrWdt();
	Delay_ms(150);			// wait to settle up 2015 
	Delay_ms(150);			// 
	ClrWdt();
//	Delay_ms(150);			// wait to settle up 2015 
//	Delay_ms(150);			// 
//	ClrWdt();

 	SpegniLedNum();
	SpegniLedCaps();
	SpegniLedScroll();
	SpegniLedChiave();

	loadKBdefaults();
	loadKBlayout();

#ifdef USA_CHIAVE							// (mettere pullup su PORTA!)
#ifdef USA_CHIAVE_TOGGLE
#ifdef USA_CHIAVE_TOGGLE_STATO_INIZIALE 
	if(!USA_CHIAVE_TOGGLE_STATO_INIZIALE)
		AccendiLedChiave();
	else
		SpegniLedChiave();
#else
	AccendiLedChiave();
#endif
#endif
#else			// se no-chiave, fisso acceso! (tranne se layout-led, 2015)
#ifndef CHIAVE_LED_IS_LAYOUT_LED
	AccendiLedChiave();
#else
	SpegniLedChiave();
#endif
#endif


  //initialize the variable holding the handle for the last transmission
  lastINTransmission = 0;
//  lastOUTTransmission = 0;
	USBInHandle=0;
	USBOutHandle=0;

#ifdef KEYBOARD_WITH_DATA
	keyMode=0;
#endif
	scanIdleCnt=0;
	idle_rate=SCANCNT_EVERY_2_IDLE_DEFAULT;

#ifdef USE_SW_SUSPEND
	inSuspend=FALSE;
#endif
	}	//end UserInit


/********************************************************************
 * Function:        void ProcessIO(void)
 *	
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user
 *                  routines. It is a mixture of both USB and
 *                  non-USB tasks.
 *
 * Note:            None
 *******************************************************************/
void ProcessIO(void) {   
	int i; 

    //Blink the LEDs according to the USB device status
    //However, the LEDs are also used temporarily for showing the Num Lock 
    //keyboard LED status.  If the host sends an LED state update interrupt
    //out report, or sends it by a SET_REPORT control transfer, then
    //the demo board LEDs are temporarily taken over to show the Num Lock
    //LED state info.  After a countdown timout, the firmware will switch
    //back to showing the USB state on the LEDs, instead of the num lock status.
  if(BlinkStatusValid == TRUE) {
	  BlinkUSBStatus();
		}
	else {
		CountdownTimerToShowUSBStatusOnLEDs--;
		if(CountdownTimerToShowUSBStatusOnLEDs == 0) {
			BlinkStatusValid = TRUE;
			}	
		}	 
	
	//Check if we should assert a remote wakeup request to the USB host, when
	//the user presses the pushbutton.
#ifdef DARIO
    if(sw2 == 0) {
      USBCBSendResume(); //Does nothing unless we are in USB suspend with remote wakeup armed.
    	} 
#endif


	//Call the function that behaves like a keyboard  
  Keyboard();        
     
  // User Application USB tasks
  if((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl==1)) 
		return;

#ifdef KEYBOARD_WITH_DATA
	if(!HIDRxHandleBusy(USBOutHandle)) {   
	  switch(ReceivedDataBuffer[1]) {			// ReceivedDataBuffer[0] contiene 2 = OUTPUT_REPORT, se ESPLICITAMENTE DEFINITO

			case CMD_NOP:
				break;
			
			case CMD_GETID:						// restituisce ID ASCII ()
				i=0;						// puntatore alla stringa Copyright
			
				do {
					prepOutBuffer(CMD_GETID);					// indicatore
#if defined(__18CXX)
					memcpypgm2ram((void *)(ToSendDataBuffer+2),(rom far void *)(((rom far char *)USB_MK_C)+i),HID_INT_IN_EP2_SIZE-2);
#else
					memcpy((void *)(ToSendDataBuffer+2),(void *)(((char *)USB_MK_C)+i),HID_INT_IN_EP2_SIZE-2);
#endif
			 	  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			    while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			      #if defined(USB_POLLING)
							 USBDeviceTasks()
						#endif
						; //USBDeviceTasks();
						//sendEP2();  USARE!
					i+=HID_INT_IN_EP2_SIZE-2;
					} while(!memchr((void *)(ToSendDataBuffer+2),0,HID_INT_IN_EP2_SIZE-2));
			
				break;
			
			case CMD_GETCAPABILITIES:								// restituisce config. 
				prepOutBuffer(CMD_GETCAPABILITIES);					// indicatore
				ToSendDataBuffer[2]=VERNUMH;
				ToSendDataBuffer[3]=VERNUML;

				ToSendDataBuffer[53]=configTastiera.memorySize;
				ToSendDataBuffer[56]=kbLayoutHeader.numTasti;

	      USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
		    while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
						//sendEP2();  USARE!
				break;

			case CMD_GETCONFIG:											// restituisce config. 
				prepOutBuffer(CMD_GETCONFIG);					// indicatore
				ToSendDataBuffer[15]=RESERVED_EEPROM;
				ToSendDataBuffer[33]=idTastiera;
				ToSendDataBuffer[34]=configTastiera.autorepeat;
				ToSendDataBuffer[35]=configTastiera.buzzer;
				ToSendDataBuffer[36]=configTastiera.maxLayout;
				ToSendDataBuffer[37]=configTastiera.maxTastiContemporanei;
				ToSendDataBuffer[38]=configTastiera.noCtrlAltCanc;
				ToSendDataBuffer[39]=configTastiera.noCtrlShiftEsc;
				ToSendDataBuffer[40]=configTastiera.noAltTab;
				ToSendDataBuffer[41]=configTastiera.noAltEsc;
				ToSendDataBuffer[42]=configTastiera.gestisciPhantom;
				ToSendDataBuffer[43]=configTastiera.numlockIsLayout;
				ToSendDataBuffer[44]=configTastiera.noWrite;
				ToSendDataBuffer[45]=configTastiera.isMultimedia;
				ToSendDataBuffer[46]=configTastiera.chiave;
				ToSendDataBuffer[47]=configTastiera.chiaveLed;
				ToSendDataBuffer[48]=configTastiera.scanK;		// i mSec!
				ToSendDataBuffer[49]=configTastiera.retroIlluminazione;		// 
				ToSendDataBuffer[50]=configTastiera.NumCapsScrollScambiati;

				ToSendDataBuffer[53]=configTastiera.memorySize;
				ToSendDataBuffer[56]=kbLayoutHeader.numTasti;
				ToSendDataBuffer[57]=kbLayoutHeader.modello;
				ToSendDataBuffer[58]=kbLayoutHeader.numLayout;


	      USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
		    while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;
			
			case CMD_GETSTATUS:											// boh, lascio tutto per ora
				prepStatusBuffer(1);					// indicatore
				ToSendDataBuffer[16]=idle_rate;
				ToSendDataBuffer[17]=configTastiera.scanK;
				ToSendDataBuffer[18]=0 /*Leds*/;

				ToSendDataBuffer[53]=configTastiera.memorySize;
				
	      USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
		    while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_SETCONFIG:											// restituisce config. 
				prepOutBuffer(CMD_SETCONFIG);					// indicatore
				configTastiera.autorepeat=ReceivedDataBuffer[34];
				configTastiera.buzzer=ReceivedDataBuffer[35];
				configTastiera.maxLayout=ReceivedDataBuffer[36];		//
				if(configTastiera.maxLayout>MAX_LAYOUT)
					configTastiera.maxLayout=MAX_LAYOUT;
				configTastiera.maxTastiContemporanei=min(MAX_ASSOLUTO_TASTI_CONTEMPORANEI,ReceivedDataBuffer[37]);
				configTastiera.noCtrlAltCanc=ReceivedDataBuffer[38];
				configTastiera.noCtrlShiftEsc=ReceivedDataBuffer[39];
				configTastiera.noAltTab=ReceivedDataBuffer[40];
				configTastiera.noAltEsc=ReceivedDataBuffer[41];
				configTastiera.gestisciPhantom=ReceivedDataBuffer[42];
				configTastiera.numlockIsLayout=ReceivedDataBuffer[43];
				configTastiera.noWrite=ReceivedDataBuffer[44];
				configTastiera.isMultimedia=ReceivedDataBuffer[45];
				configTastiera.chiave=ReceivedDataBuffer[46];
				configTastiera.chiaveLed=ReceivedDataBuffer[47];
				configTastiera.scanK=ReceivedDataBuffer[48];			// i mSec!
				configTastiera.retroIlluminazione=ReceivedDataBuffer[49];
				configTastiera.NumCapsScrollScambiati=ReceivedDataBuffer[50];
				ToSendDataBuffer[2]=saveKBdefaults();
			
	      USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
		    while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;


			case CMD_PROGMEM:					// programmazione EEprom (ESCLUSA PRIMA PARTE, ovvero layout) (blocchi SEMPRE da 32 BYTE, con USB nuovo/2005):
				prepOutBuffer(CMD_PROGMEM);			// indicatore

#ifdef USA_EEPROM 
#ifdef USA_EEPROM_INTERNA

				memcpy((void *)I2CBuffer,(void *)ReceivedDataBuffer+8,32);
				ToSendDataBuffer[2]=0;
				{
				BYTE k;
				BYTE addr=RESERVED_EEPROM+ReceivedDataBuffer[3];
				if(addr<RESERVED_EEPROM)			// che non sfori o faccia il giro!
					addr=RESERVED_EEPROM;
				for(k=0; k<(ReceivedDataBuffer[2] & 0x3f); k++)
					EEscrivi(addr+k,I2CBuffer[k]);
				ToSendDataBuffer[2]=k;
				}
				//if(!(ReceivedDataBuffer[2] & 0x80)) 			// addr a 8 opp. 16? (b6:0 = # BYTEs)
					//qua sempre solo 8bit

#else
				memcpy((void *)I2CBuffer,(void *)ReceivedDataBuffer+8,32);
				ToSendDataBuffer[2]=0;
/* BAH INUTILE 2015
				if(!(ReceivedDataBuffer[2] & 0x80)) {			// addr a 8 opp. 16? (b6:0 = # BYTEs)
					// lo_addr 
					ToSendDataBuffer[2]=I2CWritePage(RESERVED_EEPROM+ReceivedDataBuffer[3],min(ReceivedDataBuffer[2] & 0x3f,32));					// gestire ev. errore?
					}

				else {
*/
					// lo_addr & hi_addr
					ReceivedDataBuffer[2] &= 0x7f;	// safety pre-2015
					ToSendDataBuffer[2]=I2CWritePage16(RESERVED_EEPROM+MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4]),min(ReceivedDataBuffer[2] & 0x3f,32));				// gestire ev. errore?
/*					}
*/
#endif
#else
				ToSendDataBuffer[2]=ToSendDataBuffer[3]=ToSendDataBuffer[4]=0;
#endif
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_READMEM:			// lettura EEprom (ESCLUSA PRIMA PARTE, ovvero layout): 2 BYTE lo/hi addr. (blocchi SEMPRE da 32 BYTE QUA, con USB nuovo/2005):
				prepOutBuffer(CMD_READMEM);			// indicatore

#ifdef USA_EEPROM 
#ifdef USA_EEPROM_INTERNA
				{
				BYTE k;
				BYTE addr=RESERVED_EEPROM+ReceivedDataBuffer[3];
				if(addr<RESERVED_EEPROM)			// che non sfori o faccia il giro!
					addr=RESERVED_EEPROM;
				for(k=0; k<(ReceivedDataBuffer[2] & 0x3f); k++)
					I2CBuffer[k]=EEleggi(addr+k);
				}
				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=ReceivedDataBuffer[3];
				ToSendDataBuffer[4]=ReceivedDataBuffer[4];
				memcpy((void *)ToSendDataBuffer+8,(void *)I2CBuffer,32);
#else
/* BAH INUTILE! 2015
				if(!(ReceivedDataBuffer[2] & 0x80)) {			// addr a 8 opp. 16? (b6:0 = # BYTEs)
					// lo_addr
					I2CRead8Seq(RESERVED_EEPROM+ReceivedDataBuffer[3],min(ReceivedDataBuffer[2] & 0x3f,32));
					}
				else {
*/
					ReceivedDataBuffer[2] &= 0x7f;	// safety pre-2015
					I2CRead16Seq(RESERVED_EEPROM+MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4]),min(ReceivedDataBuffer[2] & 0x3f,32));			// lo_addr


//					I2CRead16Seq(MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4]),min(ReceivedDataBuffer[2] & 0x3f,32));			// lo_addr
// test bios distruttore 2015*************


/*					}
*/

				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=ReceivedDataBuffer[3];
				ToSendDataBuffer[4]=ReceivedDataBuffer[4];
				memcpy((void *)ToSendDataBuffer+8,(void *)I2CBuffer,32);
#endif
#else
				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=ReceivedDataBuffer[3];
				ToSendDataBuffer[4]=ReceivedDataBuffer[4];
				for(i=0; i<32; i++) {
					ToSendDataBuffer[8+i]=*KB_layout0[i+MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4])];	// tanto per...
// dovrebbe passare automaticamente ai layout 1..2..3
					}
#endif
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_WRITEEEPROM:					// programmazione EEprom (blocchi SEMPRE da 32 BYTE, con USB nuovo/2005):
				prepOutBuffer(CMD_WRITEEEPROM);			// indicatore

#ifdef USA_EEPROM 
#ifdef USA_EEPROM_INTERNA

				memcpy((void *)I2CBuffer,(void *)ReceivedDataBuffer+8,32);
				ToSendDataBuffer[2]=0;
				{
				BYTE k;
				BYTE addr=RESERVED_EEPROM+ReceivedDataBuffer[3];
				if(addr<RESERVED_EEPROM)			// che non sfori o faccia il giro!
					addr=RESERVED_EEPROM;
				for(k=0; k<(ReceivedDataBuffer[2] & 0x3f); k++)
					EEscrivi(addr+k,I2CBuffer[k]);
				ToSendDataBuffer[2]=k;
				}
				//if(!(ReceivedDataBuffer[2] & 0x80)) 			// addr a 8 opp. 16? (b6:0 = # BYTEs)
					//qua sempre solo 8bit

#else
				memcpy((void *)I2CBuffer,(void *)ReceivedDataBuffer+8,32);
				ToSendDataBuffer[2]=0;
					// lo_addr & hi_addr
					ReceivedDataBuffer[2] &= 0x7f;	// safety pre-2015
					ToSendDataBuffer[2]=I2CWritePage16(MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4]),min(ReceivedDataBuffer[2] & 0x3f,32));				// gestire ev. errore?
#endif
#else
				ToSendDataBuffer[2]=ToSendDataBuffer[3]=ToSendDataBuffer[4]=0;
#endif
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_READEEPROM:			// lettura EEprom: 2 BYTE lo/hi addr. (blocchi SEMPRE da 32 BYTE QUA, con USB nuovo/2005):
				prepOutBuffer(CMD_READEEPROM);			// indicatore

#ifdef USA_EEPROM 
#ifdef USA_EEPROM_INTERNA
				{
				BYTE k;
				BYTE addr=RESERVED_EEPROM+ReceivedDataBuffer[3];
				if(addr<RESERVED_EEPROM)			// che non sfori o faccia il giro!
					addr=RESERVED_EEPROM;
				for(k=0; k<(ReceivedDataBuffer[2] & 0x3f); k++)
					I2CBuffer[k]=EEleggi(addr+k);
				}
				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=ReceivedDataBuffer[3];
				ToSendDataBuffer[4]=ReceivedDataBuffer[4];
				memcpy((void *)ToSendDataBuffer+8,(void *)I2CBuffer,32);
#else
				ReceivedDataBuffer[2] &= 0x7f;	// safety pre-2015
				I2CRead16Seq(MAKEWORD(ReceivedDataBuffer[3],ReceivedDataBuffer[4]),min(ReceivedDataBuffer[2] & 0x3f,32));			// lo_addr

				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=ReceivedDataBuffer[3];
				ToSendDataBuffer[4]=ReceivedDataBuffer[4];
				memcpy((void *)ToSendDataBuffer+8,(void *)I2CBuffer,32);
#endif
#else
				ToSendDataBuffer[2]=ToSendDataBuffer[3]=ToSendDataBuffer[4]=0;
#endif
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_RESETEEPROM:			// preset EEprom
				prepOutBuffer(CMD_RESETEEPROM);			// indicatore

				if(ReceivedDataBuffer[2]) {			// se "tutta"...
					resetKBdefaults();
					saveKBdefaults();
					}
#ifdef USA_EEPROM 
				presetKBlayout();
#endif
				ToSendDataBuffer[2]=ReceivedDataBuffer[2];
				ToSendDataBuffer[3]=idTastiera;

			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_ZAPEEPROM:			// debug 2015
				prepOutBuffer(CMD_ZAPEEPROM);			// indicatore

#ifdef USA_EEPROM 
#ifdef USA_EEPROM_INTERNA
				{
				WORD k;
				for(k=0; k<256; k+=64) {
					for(i=0; i<32; i++)
						I2CBuffer[i]=rand();
					for(i=0; i<32; i++)
						EEscrivi(k+i,I2CBuffer[i]);
					}
				for(i=0; i<32; i++)
					I2CBuffer[i]=rand();
				for(i=0; i<32; i++)
					EEscrivi(32+i,I2CBuffer[i]);
				}
				ToSendDataBuffer[2]=1;
#else

				{
				WORD k;
				for(k=0; k<4096; k+=64) {
					for(i=0; i<32; i++)
						I2CBuffer[i]=rand();
					I2CWritePage16(k,32);				// 
					}
				for(i=0; i<32; i++)
					I2CBuffer[i]=rand();
				I2CWritePage16(32,32);				// esplicitamente 32 per layout!
				}
				ToSendDataBuffer[2]=1;
#endif
#else
				ToSendDataBuffer[2]=0;
#endif

			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;


			case CMD_KEYMODE:
				keyMode=ReceivedDataBuffer[2] & 1;
				prepOutBuffer(CMD_KEYMODE);
			
				ToSendDataBuffer[2]=0;
				ToSendDataBuffer[3]=keyMode;
				ToSendDataBuffer[4]=kbScanCodeSaved;		// tanto per
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;

			case CMD_RESET_IF:						// RESET (occhio a Enum. USB)
				Reset();
				break;	
						
			case CMD_DEBUG_ECHO:						// ECHO
				prepOutBuffer(CMD_DEBUG_ECHO);
				memcpy((void *)(ToSendDataBuffer+2),(void *)(ReceivedDataBuffer+2),
			//						min(HID_INT_IN_EP2_SIZE,HID_INT_OUT_EP2_SIZE)-2);
					HID_INT_OUT_EP2_SIZE-1);
			
			  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
			  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();
				//sendEP2();  USARE!
				break;
			}
	  USBOutHandle = HIDRxPacket(HID_EP2,(BYTE*)&ReceivedDataBuffer,HID_INT_OUT_EP2_SIZE);
 		}
#endif

	}	//end ProcessIO


void Keyboard(void) {
	static WORD divider=0;

	//Check if the IN endpoint is not busy, and if it isn't check if we want to send 
	//keystroke data to the host.
#ifdef PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		// prove 2015 su demoboard
//  if(!HIDTxHandleBusy(lastINTransmission)) {
    if(Switch3IsPressed()) {
     	//Load the HID buffer
     	hid_report_in[0] = 1;
     	hid_report_in[1] = 0;
     	hid_report_in[2] = 0;
     	hid_report_in[3] = key++;		// finire/togliere! 2023
     	hid_report_in[4] = 0;
     	hid_report_in[5] = 0;
     	hid_report_in[6] = 0;
     	hid_report_in[7] = 0;
     	hid_report_in[8] = 0;

     	//Send the 9 BYTE packet over USB to the host.
     	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x09);
		  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
		    #if defined(USB_POLLING)
			  USBDeviceTasks()
				#endif
				; //USBDeviceTasks();
			//sendEP1();  USARE!

     	hid_report_in[3] = 0;

     	//clear...
     	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x09);
		  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
		    #if defined(USB_POLLING)
			  USBDeviceTasks()
				#endif
				; //USBDeviceTasks();
			//sendEP1();  USARE!

      if(key == 40) {
        key = 4;
        }
      }
#ifdef MULTIMEDIA_KEYBOARD
    else if(Switch2IsPressed()) {
     	//Load the HID buffer
     	hid_report_in[0] = 2;
	    if(Switch3IsPressed()) {		// così non va ma ok
     		hid_report_in[1] = 0x2;
     		hid_report_in[2] = 0x0;
				}
			else {
     		hid_report_in[1] = 0x0;
     		hid_report_in[2] = 0x1;
				}

     	hid_report_in[0] = 3;			// power/sleep
	    if(Switch3IsPressed()) 
     		hid_report_in[1] = 0x2;
			else
     		hid_report_in[1] = 0x1;
     	hid_report_in[2] = 0x0;


     	//Send the 3 BYTE packet over USB to the host.
     	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x03);
		  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
		    #if defined(USB_POLLING)
			  USBDeviceTasks()
				#endif
						//sendEP1_mmedia();  USARE!
				; //USBDeviceTasks();
    /*
     	hid_report_in[1] = 0;		// boh no non serve

     	//clear...
     	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x02);
		  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
		    #if defined(USB_POLLING)
			  USBDeviceTasks()
				#endif
				; //USBDeviceTasks();
						//sendEP1();  USARE!
*/
			}
#endif
    else {
#endif

	divider++;
	if(divider >= (configTastiera.scanK*300) /*7000*/) {		// ???' 40-50mS usare scanFreq!!
// ~60mSec con default, 4.9.15 porcamadonna le borse e/o gli italiani
//		rowState=0;

		mLED_0 ^= 1;


#ifndef PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		// prove 2015 su demoboard
		scanKBD();
#endif



		if(configTastiera.buzzer) {
			if(FLAGA & (1 << TRIGK))	{ 		// solo alla pressione!
	
		//	btfss	FLAGA,TRIGK2		; solo alla pressione!
		//	goto  fineBuzzer
	
//			if(!kbBufferO[0]) {				// solo se tasto non-modifier!	hmmm QUA CONSILIUM CMQ no!
	
#ifdef BUZZER_PWM 
// mettere?	      OC2CON1 |= 0x0006;   // on
//				__delay_ms(20);				// ritardo almeno 20mSec
//	      OC2CON1 &= ~0x0006;   // off
#else
					m_Buzzer=1;   // on
					Delay_ms(20);				// ritardo almeno 20mSec
		      m_Buzzer=0;   // off
#endif
				}
//			}
			}			// buzzer

		if(FLAGA & ((1 << TRIGK) | (1 << TRIGK2))) {
			goto sendKB;
			}

		if(idle_rate) {
			if(configTastiera.autorepeat) {
				scanIdleCnt++;
				}
			if(scanIdleCnt >= idle_rate) {
				scanIdleCnt=0;

sendKB:
//hid_report_in[0]=1;
//hid_report_in[1]=0;
//hid_report_in[3]=5;
//mLED_2_Toggle();
#ifdef KEYBOARD_WITH_DATA
				if(!keyMode) {
#endif


#ifdef USE_SW_SUSPEND
					if(inSuspend) {		// c'è anche vicino a Scan... verificare...

//	mLED_1_Toggle();
//	mLED_2_Toggle();
//RemoteWakeup=TRUE;		// FORZO 2016

//					usb_stat.RemoteWakeup=1; //FORZARE? no se il pc sa che sei remote-wakeup capable
//  UCONbits.SUSPND = 1;                    // 
						Delay_ms(10);
    	//USBRemoteWakeup();                  // attempt RWU old stack
						USBCBSendResume();
						Delay_ms(10);
//  UCONbits.SUSPND = 0;                    // 
//	Delay_ms(10);
						inSuspend=FALSE;
/*				kbBufferO[0]=1;		 a un certo punto uscivano dei tasti costanti dopo il wakeup
				kbBufferO[1]=0;
				kbBufferO[2]=0;
				kbBufferO[3]=0;
				kbBufferO[4]=0;
				kbBufferO[5]=0;
				kbBufferO[6]=0;
				kbBufferO[7]=0;
			  kbBufferO[8]=0;*/
						}

#endif

					sendEP1();  

#ifdef KEYBOARD_WITH_DATA
					}
				else {
					if(FLAGA & (1 << TRIGK)) {			// solo pressione...
						static BYTE oldKbScanCodeSaved;

// no #ifdef USE_3_HID_ENDPOINT
						prepOutBuffer(CMD_KEYMODE);
						ToSendDataBuffer[2]=1;
						ToSendDataBuffer[3]=/*kbKeys[0]*/ kbScanCodeSaved;		//boh? o scancode?? solo il primo? !
// mah, no, e mando pure flag di ripetuto!						kbScanCodeSaved=0;
						if(kbScanCodeSaved && kbScanCodeSaved==oldKbScanCodeSaved)
							ToSendDataBuffer[3] |= 0x80;
						oldKbScanCodeSaved=kbScanCodeSaved;
	
			     	//Send the 8 BYTE packet over USB to the host.
/*FINIRE! v. consilium*/
			 			USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
						while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
							#if defined(USB_POLLING)
								 USBDeviceTasks()
							#endif
							; //USBDeviceTasks();
						//sendEP2();  USARE!


/*			     	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x08);
					  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
					    #if defined(USB_POLLING)
						  USBDeviceTasks()
							#endif
							; //USBDeviceTasks();
							*/
// #endif
						}
					}
#endif

				FLAGA &= ~((1 << TRIGK) | (1 << TRIGK2));
				}
			
nosendKB:	;
			}		// idle_rate
		divider=0;
		}			// divider
	
#ifdef PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		// prove 2015 su demoboard
	}		// switch/test
#endif


//    }
    
    
#ifdef DARIO
    //Check if any data was sent from the PC to the keyboard device.  Report descriptor allows
    //host to send 1 BYTE of data.  Bits 0-4 are LED states, bits 5-7 are unused pad bits.
    //The host can potentially send this OUT report data through the HID OUT endpoint (EP1 OUT),
    //or, alternatively, the host may try to send LED state information by sending a
    //SET_REPORT control transfer on EP0.  See the USBHIDCBSetReportHandler() function.
    if(!HIDRxHandleBusy(lastOUTTransmission)) {
		//Do something useful with the data now.  Data is in the OutBuffer[0].
		//Num Lock LED state is in Bit0.
		if(hid_report_out[0] & 0x01) //Make LED1 and LED2 match Num Lock state.
		{
			mLED_1_On();
			mLED_2_On();
			}
		else {
			mLED_1_Off();
			mLED_2_Off();			
			}
		
		//Stop toggling the LEDs, so you can temporily see the Num lock LED state instead.
		//Once the CountdownTimerToShowUSBStatusOnLEDs reaches 0, the LEDs will go back to showing USB state instead.
		BlinkStatusValid = FALSE;
		CountdownTimerToShowUSBStatusOnLEDs = 140000;
	
		lastOUTTransmission = HIDRxPacket(HID_EP,(BYTE*)&hid_report_out,1);
		} 
#endif
    

	}	//end keyboard()



// v. anche TAST101
void scanKBD(void) {
	static /*overlay*/ BYTE *p,*kbSrcPtr,*kbDstPtr;
	static /*overlay*/ WORD *rowStatePtr,*rowStatePtr2;
	static /*overlay*/ BYTE i,mtemp;
	static WORD kbtemp;
	static WORD kbScanY;
#ifdef TUTTI_CONTRO_TUTTI 
	static WORD colMask;
#endif
#ifdef MULTIMEDIA_KEYBOARD
	static BYTE divider_250;
#endif

//	clrf kbKeys			; PROVA!!


#ifdef TUTTI_CONTRO_TUTTI 


	kbScanY=0x01fe;
	kbScanCode=1;			// parto da 1!! (se no lo zero=non valido taglierebbe via ROW0:COL0)
	colMask=0x0001;		// questa filtra via "i riflessi di col. input o in precedenza usati come out"

	LATD=0;						// prima, riscrivo PORTD e PORTE
	LATE &= 0b11111110;

// faccio la scansione, scrivendo nelle colonne e leggendo dalle righe;
//		poi salvo i valori nell'array da 10 WORD (20 BYTE) rowState

	p=(BYTE *)rowState;

	do {
		TRISD=kbScanY & 0xff;
		TRISE= (TRISE & 0xfe) | ((HIBYTE(kbScanY)) & 0x01);

		Delay10TCYx(3);									// ritardino...

		*p++=PORTD | (colMask & 0xff);
		*p++=(PORTE | 0xfe) | (HIBYTE(colMask));		// rimuovo bit inutilizzati

		TRISE |= 0x01;				// 1=input 
		TRISD = 255;			// 1=input

		Delay10TCYx(3);									// ritardino...

		kbScanY <<= 1;
		kbScanY |= 1;								// entra 1
		colMask <<= 1;
		colMask |= 1;
		} while(kbScanY & 0x200);


// risistemiamo la situaz. uscite
// fisso uscite a 1 x non lasciare appese (anche se matteo mette pullup)
	LATD=0xff;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATE |= 0b00000001;
	TRISE &= 0b11111110;				// output
	TRISD = 0;			// output

#else

#ifdef MODALITA_GILARDONI

	kbScanY=0x03fe;
	kbScanCode=1;			// parto da 1!! (se no lo zero=non valido taglierebbe via ROW0:COL0)


	LATB=0;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATC &= 0x3f;

// faccio la scansione, scrivendo nelle colonne e leggendo dalle righe;
//		poi salvo i valori nell'array da 10 WORD (20 BYTE) rowState

	p=(BYTE *)rowState;

	do {
		TRISB=kbScanY & 0xff;
		TRISC= (TRISC & 0x3f) | ((kbScanY >> 2) & 0xC0);

		Delay10TCY();									// ritardino...

		*p++=PORTD;
		*p++=PORTE | 0xf8;		// rimuovo bit inutilizzati

		TRISC |= 0x03;				// 1=input (e cmq. per RS232 se usata), 0=output per PWM!
		TRISB = 255;			// 1=input

		Delay10TCYx(3);									// ritardino...

		kbScanY <<= 1;
		kbScanY |= 1;								// entra 1
		} while(kbScanY & 0x400);


// risistemiamo la situaz. uscite
//	TRISC |= 0xC0;				// 1=input (e cmq. per RS232 se usata), 0=output per PWM!
//	TRISB = 255;			// 1=input
// fisso uscite a 1 x non lasciare appese (anche se matteo mette pullup)
	LATB=0xff;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATC |= 0x03;
	TRISC &= 0xfc;				// output
	TRISB = 0;			// output


#else

#ifdef USA_TASTIERA_21PIN

/* tutti verso massa allegramente */

	TRISB |= 0b11111111;						// setto input
	TRISC |= 0b00000011;						// setto input
	TRISD |= 0b11111111;						// setto input
	TRISE |= 0b00000111;						// setto input

	Delay_uS(50);

	kbScanCode=1;	

	memset(rowState,0xff,11*2);
	p=(BYTE *)rowState;
	*p=0xff;
  for(i=0, kbtemp=0b11111110; i<8; i++) {
    if(!(PORTB & ~kbtemp))
			*p &= kbtemp;
		kbtemp <<= 1;
		kbtemp |= 1;
		}

	p++;
	*p &= PORTC & 0b00000001 ? 0b11111111 : 0b11111110;
	*p &= PORTC & 0b00000010 ? 0b11111111 : 0b11111101;

	p++;

  for(i=0, kbtemp=0b11111110; i<8; i++) {
    if(!(PORTD & ~kbtemp))
			*p &= kbtemp;
		kbtemp <<= 1;
		kbtemp |= 1;
		}

	p++;
	*p &= PORTE & 0b00000001 ? 0b11111111 : 0b11111110;
	*p &= PORTE & 0b00000010 ? 0b11111111 : 0b11111101;
	*p &= PORTE & 0b00000100 ? 0b11111111 : 0b11111011;

#else

#ifdef MODALITA_A3010

	kbScanY=0xfffe;
	kbScanCode=1;			// parto da 1!! (se no lo zero=non valido taglierebbe via ROW0:COL0)


	TRISB=0xff;
	TRISC=0xff;
	LATB= 0x00;						// SERVONO 16x16 qua!!! sistemare, e occhio al PIC scelto
	LATC =0x00;

// faccio la scansione, scrivendo nelle colonne e leggendo dalle righe;
//		poi salvo i valori nell'array da 10 WORD (20 BYTE) rowState

	p=(BYTE *)rowState;

	do {
		TRISB=LOBYTE(kbScanY);
		TRISC=HIBYTE(kbScanY);

#if defined(__18CXX)
		Delay10TCYx(3);									// ritardino...
#else
		__delay_us(1);
#endif

		*p++=LATD;
		*p++=LATE;		// rimuovo bit inutilizzati

		TRISC= 255;			// 1=input 
		TRISB= 255;			// 1=input

#if defined(__18CXX)
		Delay10TCYx(3);									// ritardino...
#else
		__delay_us(1);
#endif

		kbScanY <<= 1;
		kbScanY |= 1;								// entra 1
		} while(kbScanY & 0x8000);


// risistemiamo la situaz. uscite
// fisso uscite a 1 x non lasciare appese (anche se matteo mette pullup)
	LATB=0xff;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATC=0xff;
	TRISC=0x00;				// output
	TRISB=0x00;			// output

#else

	kbScanY=0x03fe;
	kbScanCode=1;			// parto da 1!! (se no lo zero=non valido taglierebbe via ROW0:COL0)


	TRISB=0xff;
	TRISC |= 0x03;
	LATB=0;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATC &= 0xfc;

// faccio la scansione, scrivendo nelle colonne e leggendo dalle righe;
//		poi salvo i valori nell'array da 10 WORD (20 BYTE) rowState

	p=(BYTE *)rowState;

	do {
		TRISB=kbScanY & 0xff;
		TRISC= (TRISC & 0xfc) | (HIBYTE(kbScanY) & 0x03);

#if defined(__18CXX)
		Delay10TCYx(3);									// ritardino...
#else
		__delay_us(1);
#endif

		*p++=PORTD;
		*p++=PORTE | 0xf8;		// rimuovo bit inutilizzati

		TRISC |= 0x03;				// 1=input (e cmq. per RS232 se usata), 0=output per PWM!
		TRISB = 255;			// 1=input

#if defined(__18CXX)
		Delay10TCYx(3);									// ritardino...
#else
		__delay_us(1);
#endif

		kbScanY <<= 1;
		kbScanY |= 1;								// entra 1
		} while(kbScanY & 0x400);


// risistemiamo la situaz. uscite
// SI POTREBBE TOGLIERE (2010)
//	TRISC |= 0x03;				// 1=input (e cmq. per RS232 se usata), 0=output per PWM!
//	TRISB = 255;			// 1=input
// fisso uscite a 1 x non lasciare appese (anche se matteo mette pullup)
	LATB=0xff;						// prima, riscrivo PORTC (soprattutto, essendo mista) e PORTD
	LATC |= 0x03;
	TRISC &= 0xfc;				// output
	TRISB = 0;			// output

#endif			//A3010

#endif			//21 tasti

#endif			//gilardoni

#endif			//tuttixtutti

//	return;







// ora vado a controllare i tasti premuti, facendo anche attenzione ai tasti ripetuti:
//		non devono essere piu' di 2 nella colonna 7 (modifier)
//		 e in generale non devono esserci tasti fantasma (quelli "a quadrato")

	for(rowStatePtr=rowState; rowStatePtr < rowState+(sizeof(rowState)/sizeof(WORD)); rowStatePtr++) {	// puntatore alle letture fatte prima

		ClrWdt();


#ifdef CONTROLLA_PHANTOM
#ifdef CONTROLLA_PHANTOM_7
		if(rowStatePtr == (rowState+7)) {			// quando sono sulla COL7...

			i=count0Bit(*rowStatePtr);				// ...controllo che non ci siano + di 2 tasti premuti
			if(i>2) {
				scanDoError(ERROR_ROLLOVER /* o ERROR_UNDEFINED */);

      	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x09);
			  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
			    #if defined(USB_POLLING)
				  USBDeviceTasks()
					#endif
					; //USBDeviceTasks();

				return;
				}
			}

		else {																// altrimenti controllo normale:
#endif

			i=count0Bit(*rowStatePtr);				// se in questa colonna ci sono almeno 2 tasti premuti...
			if(i>=2) {												// ...altrimenti salto il controllo!


//	movlw rowState						; per tutte le WORD nell'array di scansione...
//	movwf rowStatePtr2
	// per tutte le WORD che RIMANGONO nell'array di scansione...
//				rowStatePtr2=rowStatePtr+1;
//				if(rowStatePtr2 < rowState) {
	
					for(rowStatePtr2=rowStatePtr+1; rowStatePtr2 < rowState+(sizeof(rowState)/sizeof(WORD)); rowStatePtr2++) {
						ClrWdt();
	
						if(*rowStatePtr2 != 0xffff) {
							if(*rowStatePtr2 == *rowStatePtr) {
										// guardo se in qualche colonna c'e' premuto lo stesso tasto di QUESTA colonna...
								scanDoError(ERROR_ROLLOVER /* o ERROR_UNDEFINED */);
				      	sendEP1();
	
								return;
								}
							}
	
						} // UNA colonna IN MENO ?? (PER LE PERMUTAZIONI)
//					}
				}

#ifdef CONTROLLA_PHANTOM_7
			}
#endif

#endif

	
#ifdef TUTTI_CONTRO_TUTTI 
		for(kbtemp=1; !(kbtemp & 0x200); kbtemp <<= 1 /* entra 0 */) {					// analizzo le 9 righe... o forse 8??
#else
		for(kbtemp=1; !(kbtemp & 0x800); kbtemp <<= 1 /* entra 0 */) {					// analizzo le righe...
#endif
			ClrWdt();

			if(!(kbtemp & *rowStatePtr))
				checkKey(kbScanCode);

#ifdef USE_SW_SUSPEND
//			if(inSuspend) {		no, qua no
//				inSuspend=0;
//				USBWakeFromSuspend();
//				USBCBWakeFromSuspend();

//				}
#endif

			kbScanCode++;
			} // 11 righe

		}			// for(rowStatePtr...


	presetBuffer(1);

#ifdef MULTIMEDIA_KEYBOARD
//	kbBufferO2[0]=0;
//	kbBufferO2[1]=0;
#endif

	if(FLAGA & (1 << TRIGK2)) {		// se al giro prima ho mandato un pacchetto con dei tasti...
		FLAGA |= (1 << TRIGK);				// ...mi preparo a mandarne uno di pulizia...
		FLAGA &= ~(1 << TRIGK2);				// ...e pulisco la condizione
		}


	kbSrcPtr=kbKeys;
	kbDstPtr=kbBufferO+3;
	mtemp=configTastiera.maxTastiContemporanei;

	do {

		ClrWdt();
		kbScanCode=*kbSrcPtr & 0x7f;		// c'e' un tasto?
																	//  (lo pulisco e lo salvo qui che va bene)
		
		if(!kbScanCode)
			goto gestKeys3;						// no!

//		if(!kbScanCodeSaved)		no perché non viene polled da pc - mandiamo noi (v.sopra)
			kbScanCodeSaved=kbScanCode;

		if(*kbSrcPtr & 0x80) {						//	si', e' premuto?

			*kbSrcPtr &= 0x7f;					//   il tasto e' stato usato
			kbScanCode=GetKBchar(kbScanCode);				// look-up
			if(!kbScanCode)							// (nessuna delle 2 op. prec. tocca i flag!)
				goto gestKeys3;						// se indica zero, nessun codice

			if(kbScanCode >= 240) {			// se tasto normale...
gestKeys4:
				switch(kbScanCode) {
					case 247:								// se combinazione di tasti... SENZA autorepeat
//					case 247:								// se combinazione di tasti... CON autorepeat

						kbDstPtr=Handle240(kbDstPtr);

		      	sendEP1();
						presetBuffer(1);			// rilascio tutti		// (compreso modifier)
		      	sendEP1();
//usare?									kbDstPtr=flushCurrentBuffer();

						Delay_S_(12);		// 3/4 sec di ritardo, xche' qua andiamo "in continuo"!

//							goto gestKeys3;
						break;


					case 241:					// se sequenza di tasti...
						presetBuffer(1);			// pulisco tutto il buffer
						
						do {
gestKeys44_:
							kbScanCode=GetKBcharNext();				// ...li leggo uno dopo l'altro...
	
							if(kbScanCode == 240) {
								kbDstPtr=flushCurrentBuffer();		// mando fuori quanto visto finora..
								kbDstPtr=Handle240(kbDstPtr);			// ..gestisco il 240 da solo..
								kbDstPtr=flushCurrentBuffer();		// ..e riparto
								goto gestKeys44_;
//								continue;			//no? no, va a "while" (giustamente)
								}
							else if(kbScanCode >= 224) {			// se modifier (shift, ctrl, ecc)...
								i=convertModifCode(kbScanCode);			// ...lo gestisco
								kbBufferO[1] |= i;
								continue;		//...quindi qua è inutile...
								}
							else {
								*kbDstPtr++=kbScanCode;				// altrimenti uso tasto normale
																							// ...e li piazzo nel buffer x USB
								}

							} while(kbScanCode && (kbDstPtr<(kbBufferO+9)));			// ...finche' non finiscono o sono + di 6

						if(kbBufferO[3]) {
							kbDstPtr=flushCurrentBuffer();
							if(kbScanCode)
								goto gestKeys44_;
//							else
//								goto gestKeys3;						// ...serve cmq?? no...
							}
						Delay_S_(8);		// 1/2 sec di ritardo, per stringhe lunghe
						break;

					case 242:					// se sequenza di tasti (dato il codice ASCII)...
						presetBuffer(1);			// pulisco tutto il buffer

						do {
gestKeys54_:
							kbScanCode=GetKBcharNext();				// ...li leggo uno dopo l'altro...

							switch(kbScanCode) {		// 
								case 0:
									break;
								case '@':			// se servono altri, vanno aggiunti...
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b01000000;		// R-ALT
									kbBufferO[3]=0x33;			// v. tastoCHIOCCIOLA_ITA in kblayout, ecc.
									kbDstPtr=flushCurrentBuffer();
									break;
								case '#':			// 
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b01000000;		// R-ALT
									kbBufferO[3]=0x34;			// 
									kbDstPtr=flushCurrentBuffer();
									break;
/*								case ';':			// 
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b00000010;		// L-SHIFT
									kbBufferO[3]=0x36;			// 
									kbDstPtr=flushCurrentBuffer();
									break;
								case ':':			// 
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b00000010;		// L-SHIFT
									kbBufferO[3]=0x37;			// 
									kbDstPtr=flushCurrentBuffer();
									break;
								case '(':			// 
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b00000010;		// L-SHIFT
									kbBufferO[3]=0x25;			// 
									kbDstPtr=flushCurrentBuffer();
									break;
								case ')':			// 
									kbDstPtr=flushCurrentBuffer();
									kbBufferO[0]=1;
									kbBufferO[1]=0b00000010;		// L-SHIFT
									kbBufferO[3]=0x26;			// 
									kbDstPtr=flushCurrentBuffer();
									break;
serve davvero, o sarebbe solo per altri tipi di tastiera (USA ecc)? */
								default:
									*kbDstPtr++=ascii_2_usb[toupper(kbScanCode)-' '];	// solo maiuscole
																										// ...e li piazzo nel buffer x USB
									break;
								}
							} while(kbScanCode && (kbDstPtr<(kbBufferO+9)));			// ...finche' non finiscono o sono + di 6

						if(kbBufferO[3]) {
							kbDstPtr=flushCurrentBuffer();
							if(kbScanCode)
								goto gestKeys54_;
//							else
//								goto gestKeys3;						// ...serve cmq?? no...
							}
						Delay_S_(8);		// 1/2 sec di ritardo, per stringhe lunghe
						break;

					case 243:						// se tasto "multiplo"...
						kbScanCode=GetKBcharNext();				// ...leggo il prossimo codice...

						if(kbScanCode >= 224) {			// se modifier (shift, ctrl, ecc)...
							i=convertModifCode(kbScanCode);			// ...lo gestisco
							kbBufferO[1] |= i;
										// (prima ho ELIMINATO quel che c'era prima nei modifier)
							kbScanCode=GetKBcharNext();				// ...leggo il prossimo codice...
							}

						goto  gestKeys4_4;					// ...uso codice-normale e lo vado a mettere al solito posto

						break;

					case 244:					// se tasto modificabile con ALT-GR...
						kbScanCode=GetKBcharNext();				// leggo il primo codice (codice-normale)

						if((kbBufferO[1] & (1 << 6))) {					// se è premuto ALT-GR...
							kbBufferO[1] &= ~(1 << 6);					// ...ELIMINO ALT-GR dai modifier che c'erano prima
	
							kbScanCode=GetKBcharNext();				// ...leggo il prossimo codice...
	
							if(kbScanCode >= 224) {			// se modifier (shift, ctrl, ecc)...
								i=convertModifCode(kbScanCode);			// ...lo gestisco
								kbBufferO[1] |= i;
												// (prima ho ELIMINATO quel che c'era prima nei modifier)
								kbScanCode=GetKBcharNext();				// ...leggo il prossimo codice...
								}
							}

						// se invece NON è premuto ALT-GR... e quindi ora in tutti i casi...
						goto gestKeys4_4;					// ...uso codice-normale e lo vado a mettere al solito posto

//RIMETTERLO DOPO??									kbBufferO[1] |= (1 << 6);					// ...ELIMINO ALT-GR dai modifier che c'erano prima

						break;

					case 245:						// se tasto diverso tra Make e Break...
//						goto	gestKeys3;					// qui non dovrebbe esistere (v. PS/2)
						break;

					case 248:
//						if(!kbKeys[0]) {			// solo se non c'è nessun tasto premuto (2008); no, gestito con kbLayoutTemp (2015)
							i=GetKBcharNext();
							if(configTastiera.maxLayout>1) {
								switch(i) {
									case 255:
	 									kbLayout++;
										kbLayout &= (configTastiera.maxLayout-1);
			//							goto gestKeys3;
										break;
									case 254:
	 									kbLayout=0;
										kbLayoutTemp=0;
										break;
										//matteo  16-09-2009
									case 253:
	 									kbLayout=1;
										kbLayoutTemp=1;
										break;
									case 252:
										if(configTastiera.maxLayout>2) {
		 									kbLayout=2;
											kbLayoutTemp=2;
											}
										break;
									case 251:
										if(configTastiera.maxLayout>2) {
		 									kbLayout=3;
											kbLayoutTemp=3;
											//matteo  16-09-2009
											}
										break;

									default:
										i &= (configTastiera.maxLayout-1);
	 									kbLayout=i;
		//							goto gestKeys3;
										break;
									}
								}
							if(configTastiera.chiaveLed) {
								if(kbLayout & 1)
									m_LedChiaveBit=1;
								else
									m_LedChiaveBit=0;
								}
							if(i!=254 && i!=253 && i!=252 && i!=251) {
								Delay_S_(12);		// 3/4 sec di ritardo, xche' qua andiamo "in continuo"!
// evita ripetersi, 2013; spostato dopo led, 2015
// ..messo solo su tasti "a rotazione"/"non temporanei"
								}
//							}


						break;


					case 240:								// se combinazione di tasti... CON autorepeat
						kbDstPtr=Handle240(kbDstPtr);
						sendEP1(); 
						break;


#ifdef MULTIMEDIA_KEYBOARD
					case 250:
						{
#ifdef USE_SW_SUSPEND
						if(inSuspend) {		// mettere anche su tasti normali? meglio di no

//RemoteWakeup=TRUE;		// FORZO 2016

// fare solo per tasto power e wakeup, e non per gli altri, specie Sleep? (!)

//							usb_stat.RemoteWakeup=1; //FORZARE?
//  UCONbits.SUSPND = 1;                    // 
							Delay_ms(10);
				    	//USBRemoteWakeup();                  // attempt RWU old stack
							USBCBSendResume();
							Delay_ms(10);
//  UCONbits.SUSPND = 0;                    // 
//	Delay_ms(10);
	inSuspend=FALSE;

	presetBuffer(1);		// sarebbe 2 in effetti, ma tanto viene messo sotto

							}
#endif

							if(!divider_250) {
								i=GetKBcharNext();			// devono essere sempre 4 (incluso il '2')!
								kbBufferO[0]=i;
								i=GetKBcharNext();
								kbBufferO[1]=i;
								i=GetKBcharNext();
								kbBufferO[2]=i;
								i=GetKBcharNext();
								kbBufferO[3]=i;

								sendEP1_mmedia();		// 

								kbBufferO[1]=0;			// rilasci
								kbBufferO[2]=0;
								kbBufferO[3]=0;

								sendEP1_mmedia();		// 
						
								kbBufferO[0]=1;			// ripristino tanto per il futuro
								kbBufferO[1]=0;
								kbBufferO[2]=0;
								kbBufferO[3]=0;

								}

#ifdef USA_AUTOREPEAT_250 	//USA_AUTOREPEAT			// o toglierlo del tutto? in fondo un one-shot qua è ok!
							divider_250++;
							if(divider_250 > 25) {
								divider_250=0;
								}
#else
							divider_250=1;
#endif
//[vecchio modo divider2>2] ma solo sul volume su e volume giù, negli altri fa casino e cambia stato in continuazione fdf 2015 ... che fare?
						}

#ifdef OLD
					case 250:
//						{
//							static BYTE divider2;

//							if(!divider2) {
						i=GetKBcharNext();
						kbBufferO2[0]=i;
						if(i) {
							i=GetKBcharNext();
							kbBufferO2[1]=i;
							}

		      	lastINTransmission2 = HIDTxPacket(HID_EP2 3!, (BYTE*)hid_report_in2, 0x08);
					  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
					    #if defined(USB_POLLING)
						  USBDeviceTasks()
							#endif
							; //USBDeviceTasks();

						kbBufferO2[0]=1;
						kbBufferO2[1]=0;

		      	lastINTransmission2 = HIDTxPacket(HID_EP2 3!, (BYTE*)hid_report_in2, 0x08);
					  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
					    #if defined(USB_POLLING)
						  USBDeviceTasks()
							#endif
							; //USBDeviceTasks();

//							}

//							divider2++;

//							if(divider2 > 7) {
//								divider2=0;
//								}

//						}
						break;
#endif
#endif

					default:									// non deve succedere!
//						goto  gestKeys3;
						break;

					}			// switch
				}

			else if(kbScanCode >= 224) {			// se modifier (shift, ctrl, ecc)...
				i=convertModifCode(kbScanCode);
				kbBufferO[1] |= i;
				goto gestKeysSet;
				}
			else {

gestKeys4_4:

				if(configTastiera.noCtrlAltCanc) {
					if((((kbBufferO[1] & 0b00000101) == 0b00000101) && (kbScanCode==0x4c || kbScanCode==0x63)) ||			// CANC & Numpad-punto
						(((kbBufferO[1] & 0b00010100) == 0b00010100) && (kbScanCode==0x4c || kbScanCode==0x63)) ||
						(((kbBufferO[1] & 0b01000001) == 0b01000001) && (kbScanCode==0x4c || kbScanCode==0x63)) ||
						(((kbBufferO[1] & 0b01010000) == 0b01010000) && (kbScanCode==0x4c || kbScanCode==0x63)))
						goto skip_ctrlaltcanc;
					}
				if(configTastiera.noCtrlShiftEsc) {
					if((kbScanCode==0x29) &&		// ESC
						( ((kbBufferO[1] & 0b00000011) == 0b00000011) ||
						((kbBufferO[1] & 0b00010010) == 0b00010010) ||
						((kbBufferO[1] & 0b00100001) == 0b00100001) ||
						((kbBufferO[1] & 0b00110000) == 0b00110000) ))
						goto skip_ctrlaltcanc;
					}

				if(configTastiera.noAltTab) {
					if((((kbBufferO[1] & 0b00000100) == 0b00000100) && (kbScanCode==0x2b)) ||			// TAB
						(((kbBufferO[1] & 0b01000000) == 0b01000000) && (kbScanCode==0x2b))
						)
						goto skip_AltWin;
					}
				if(configTastiera.noAltEsc) {
					if((((kbBufferO[1] & 0b00000100) == 0b00000100) && (kbScanCode==0x29)) ||			// ESC
						(((kbBufferO[1] & 0b01000000) == 0b01000000) && (kbScanCode==0x29))
						)
						goto skip_AltWin;
					}
				if(configTastiera.noAltEsc) {		// sì??
					if( ((kbBufferO[1] & 0b10001000) ))		// WIN
						goto skip_AltWin;
					}

				*kbDstPtr++=kbScanCode;			//   lo piazzo nel buffer x USB

skip_ctrlaltcanc:
skip_AltWin:

gestKeysSet:
		//	bsf		FLAGA,TRIGK
		//	bsf		FLAGA,TRIGK2
//				goto gestKeys3;
	;
				}

			}		// *kbSrcPtr & 0x80

		else {

			*kbSrcPtr=0;							// no, non e' + premuto dunque lo elimino
			FLAGA |= (1 << TRIGK2);			// i dati sono cambiati (in rilascio), spedire!
	//	clrf scanFreqK						; il contatore riparte (evita che il pross. pacch. sia troppo svelto!)
	// NO, cagata!
			}

gestKeys3:
		kbSrcPtr++;
		} while(--mtemp);

	if(kbLayoutTemp && !kbBufferO[3]) {
		kbLayout=0;
		kbLayoutTemp=0;
		}
#ifdef MULTIMEDIA_KEYBOARD
	if(!kbKeys[0]) {			// sì!
		divider_250=0;
		}
#endif
	}


BYTE *Handle240(BYTE *kbDstPtr) {
	BYTE i;

	do {
		kbScanCode=GetKBcharNext();				// ...li leggo uno dopo l'altro...

		if(kbScanCode >= 224) {						// se modifier (shift, ctrl, ecc)...
			i=convertModifCode(kbScanCode);			// ...lo gestisco
			kbBufferO[1] |= i;
			}
		else {
			*kbDstPtr++=kbScanCode;						// altrimenti uso tasto normale
																				// ...e li piazzo nel buffer x USB
			}
		} while(kbScanCode);				// ...finche' non finiscono

	return kbDstPtr;
	}

void checkKey(BYTE kbScanCode) {
	BYTE *p;
	BYTE mtemp;

	p=kbKeys;
	mtemp=configTastiera.maxTastiContemporanei;
	do {
		ClrWdt();
		if((*p & 0x7f) == kbScanCode)			//	se, tolto il flag...
																			// ...trovo questo tasto...
			goto checkKeys3;
		p++;
		} while(--mtemp);

	p=kbKeys;							// ...altrimenti il tasto non era premuto!
	mtemp=configTastiera.maxTastiContemporanei;
	do {
		ClrWdt();
		if(!*p)					//	cerco un posto libero...
			goto checkKeys5;
		p++;
		} while(--mtemp);

	scanDoError(ERROR_ROLLOVER);
	return;


checkKeys5:
	*p=kbScanCode;				// ...e ce lo metto
	FLAGA |= (1 << TRIGK);			// i dati sono cambiati (in pressione), spedire! (e occhio ai tasti multipli... FINIRE!)
//	clrf scanFreqK						; il contatore riparte (evita che il pross. pacch. sia troppo svelto!)
// no, cagata!

// QUI, se si e' aggiunto un tasto, fare BEEP!
checkKeys3:
	*p |= 0x80;						// ...il tasto e' tuttora premuto
	}


BYTE convertModifCode(BYTE n) {				// entra in kbScanCode il cod. modifier (0xe0..0xef) e converte 0..7 in bit-field 1..128
	BYTE temp=1;

	n &= 0xf;									// ...lo gestisco
	while(n--) 								// converto 0..7 in bit-field 1..128
		temp<<=1;
	return temp;
	}

BYTE count0Bit(WORD t) {
	BYTE i=0;
	WORD j;

	for(j=1; j; j<<=1) {
		if(!(t & j))
			i++;
		}
	return i;
	}
	

void scanDoError(BYTE w) {		// non c'e' piu' posto (max 4 tasti contemporanei!) DO ERRORE

	kbBufferO[0]=w;
	kbBufferO[3]=w;
	kbBufferO[4]=w;
	kbBufferO[5]=w;
	kbBufferO[6]=w;
	kbBufferO[7]=w;
	kbBufferO[8]=w;

/* altri codici disponibili:  https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2
0 00 Reserved (no event indicated)
1 01 Keyboard ErrorRollOver 
2 02 Keyboard POSTFail
3 03 Keyboard ErrorUndefined
	*/
	}

BYTE *flushCurrentBuffer(void) {

  sendEP1();
	presetBuffer(1);
  sendEP1();

	return kbBufferO+3;			// ripristino
	}

void presetBuffer(BYTE n) {

	kbBufferO[0]=n;				// tanto per
	kbBufferO[1]=0;				// lo rilascio (pulisco tutto il buffer)
	kbBufferO[2]=0;				// 
	kbBufferO[3]=0;				// 
	kbBufferO[4]=0;
	kbBufferO[5]=0;
	kbBufferO[6]=0;
	kbBufferO[7]=0;
	kbBufferO[8]=0;
	}


/******************************************************************************
 * Function:        BOOL Switch2IsPressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - pressed, FALSE - not pressed
 *
 * Side Effects:    None
 *
 * Overview:        Indicates if the switch is pressed.  
 *
 * Note:            
 *
 *****************************************************************************/
BOOL Switch2IsPressed(void) {

  if(sw2 != old_sw2) {
    old_sw2 = sw2;                  // Save new value
    if(sw2 == 0)                    // If pressed
      return TRUE;                // Was pressed
    }	//end if

  return FALSE;                       // Was not pressed
	}	//end Switch2IsPressed

/******************************************************************************
 * Function:        BOOL Switch3IsPressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - pressed, FALSE - not pressed
 *
 * Side Effects:    None
 *
 * Overview:        Indicates if the switch is pressed.  
 *
 * Note:            
 *
 *****************************************************************************/
BOOL Switch3IsPressed(void) {

  if(sw3 != old_sw3) {
    old_sw3 = sw3;                  // Save new value
    if(sw3 == 0)                    // If pressed
      return TRUE;                // Was pressed
    } //end if

  return FALSE;                       // Was not pressed
	}	//end Switch3IsPressed


#if defined(__18CXX)
//Delays approx W milliseconds
void Delay_ms(BYTE mSec) {
	
	while(mSec--) {

		// usare Delay10KTCY();			// TARARE!! @20MHz??

		Delay_uS(200);
		Delay_uS(200);
		Delay_uS(200);
		Delay_uS(200);
		Delay_uS(200);
/*		Delay_uS(200); tolto 2015
		Delay_uS(200);
		Delay_uS(200);
		Delay_uS(200);
		Delay_uS(200);
*/
		}

//	return;
	}



//Delays W microseconds (includes movlw, call, and return) @ 48MHz
void Delay_uS(BYTE uSec) {

	do {
//		Delay100TCYx(2);			// TARARE!! @20MHz??
		Delay1TCY();			// TARATO @48MHz 7-9-09			// ERA 100... corretto 24/7/06 ma da TESTARE!!
		Nop();
		Nop();
		Nop();
		ClrWdt();						// 1; Clear the WDT
		Nop();
		Nop();
		Nop();
		}	while(--uSec);

  //return             ; 1

	}


#define Delay_S() Delay_S_(16)				// circa 1s @ 48MHz !! 
  

void Delay_S_(BYTE i) {				// circa ?s @ 20MHz !! modificare!!
	BYTE i2;

	do {
					
		i2=0;										// 256x256uS= ~66mS

		do {
			Delay100TCYx(31);			// 256000 cicli a 48MHz = 3100*.083=257uS, * 256 = ~66.666mS

			} while(--i2);

		} while(i--);

  //return             ; 1

	}
#endif



// ----------------------------------------------------------------------------------------
#ifdef KEYBOARD_WITH_DATA
void prepOutBuffer(BYTE t) {					// entra W=IDENTificatore

	clearOutBuffer();
	ToSendDataBuffer[0]=1 /*SKYNET_REPORT_ID_INPUT*/;					// dev'essere per forza 1! (ossia ID report di input)
											 													// .. contiene xxx_REPORT_ID_INPUT, se metto esplicitamente un REPORT_ID
	ToSendDataBuffer[1]=t;
	}

void clearOutBuffer(void) {

	memset((void *)&ToSendDataBuffer,0,64 /*BUF_SIZE_USB*/);
	}

void prepStatusBuffer(BYTE t) {

	prepOutBuffer(CMD_GETSTATUS);				// indicatore
	ToSendDataBuffer[2]=t;
	}
#endif


void sendEP1(void) {

 	//Send the 8 BYTE packet over USB to the host.
	kbBufferO[0]=1;			// tanto per!
  if((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl==1)) 
		return;
#ifndef __DEBUG
 	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x09);
  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
    #if defined(USB_POLLING)
	  USBDeviceTasks()
		#endif
		; //USBDeviceTasks();
#endif
	}

void sendEP2(void) {

#ifndef USE_2_HID_ENDPOINT
#ifndef __DEBUG
  USBInHandle = HIDTxPacket(HID_EP2,(BYTE*)ToSendDataBuffer,HID_INT_IN_EP2_SIZE);
  while(HIDTxHandleBusy(USBInHandle))		// QUA DOPO! (xché il buffer è usato "live")
    #if defined(USB_POLLING)
		 USBDeviceTasks()
	#endif
	; //USBDeviceTasks();
#endif
#endif
	}

#ifdef MULTIMEDIA_KEYBOARD
void sendEP1_mmedia(void) {

	// boh qua kbBufferO[0]=2;			// tanto per!
  if((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl==1)) 
		return;
#ifndef __DEBUG
 	lastINTransmission = HIDTxPacket(HID_EP, (BYTE*)hid_report_in, 0x03);
  while(HIDTxHandleBusy(lastINTransmission))		// QUA DOPO! (xché il buffer è usato "live")
    #if defined(USB_POLLING)
	  USBDeviceTasks()
		#endif
		; //USBDeviceTasks();
#endif
	}
#endif



void resetKBdefaults(void) {

	configTastiera.signature=0x47;
	configTastiera.Options_Mode1=0;
#ifdef USA_EEPROM
#ifdef USA_EEPROM_INTERNA 
	configTastiera.memorySize=1;
#else
	configTastiera.memorySize=255;			// supponiamo 64KB anche se sarà una 25xx1024!
#endif
#else
	configTastiera.memorySize=0;			// kein
#endif
#ifdef USA_AUTOREPEAT 
	configTastiera.autorepeat=1;
#else
	configTastiera.autorepeat=0;
#endif
#ifdef USA_BUZZER
	configTastiera.buzzer=1;
#else
	configTastiera.buzzer=0;
#endif
	configTastiera.maxLayout=MAX_LAYOUT;								
	configTastiera.maxTastiContemporanei=MAX_TASTI_CONTEMPORANEI;  //2018
	configTastiera.gestisciPhantom=0;
#ifdef NO_CTRLALTCANC_CTRLSHFTESC 
	configTastiera.noCtrlAltCanc=1;
	configTastiera.noCtrlShiftEsc=1;
#else
	configTastiera.noCtrlAltCanc=0;
	configTastiera.noCtrlShiftEsc=0;
#endif
#ifdef NO_ALTTAB_ALTESC_WIN 
	configTastiera.noAltTab=1;
	configTastiera.noAltEsc=1;
#else
	configTastiera.noAltTab=0;
	configTastiera.noAltEsc=0;
#endif
#ifdef USA_CHIAVE
	configTastiera.chiave=1;
#else
	configTastiera.chiave=0;
#endif
#ifdef CHIAVE_LED_IS_LAYOUT_LED 
	configTastiera.chiaveLed=1;
#else
	configTastiera.chiaveLed=0;
#endif
#ifdef LARIMART_NUMLOCK_LAYOUT		
	configTastiera.numlockIsLayout=1;
#else
	configTastiera.numlockIsLayout=0;
#endif
#ifdef USA_CHIAVE_TOGGLE
	configTastiera.chiaveToggle=1;
#else
	configTastiera.chiaveToggle=0;
#endif
#ifdef USA_CHIAVE_TOGGLE_STATO_INIZIALE
	configTastiera.chiaveToggleIniziale=1;
#else
	configTastiera.chiaveToggleIniziale=0;
#endif
	configTastiera.scanK=20;			//SCANCNT_K_DEFAULT 50			// default rate scan/sec tastiera
	configTastiera.noWrite=0;
#ifdef NUM_CAPS_SCROLL_SCAMBIATI
	configTastiera.NumCapsScrollScambiati=1;
#else
	configTastiera.NumCapsScrollScambiati=0;
#endif
#ifdef MULTIMEDIA_KEYBOARD
	configTastiera.isMultimedia=1;
#else
	configTastiera.isMultimedia=0;
#endif
	configTastiera.retroIlluminazione=0x88;		// qui 2 pack-ati
	configTastiera.retroIlluminazione2=0;		// non usati qua, v.consilium
	configTastiera.zonaMortaTrackball[0]=configTastiera.zonaMortaTrackball[1]=0;
	}

signed char loadKBdefaults(void) {		// queste le lascio comunque anche se senza EEprom... ovviamente con valori fissi!
	signed char i=sizeof(struct CONFIG_TASTIERA),j;

	Delay_ms(200);
	ClrWdt();

#ifdef USA_EEPROM

#ifdef USA_EEPROM_INTERNA

	j=0;
	do {
		{
		BYTE k;
		for(k=0; k<min(i,32); k++)			// sizeof < 32 !
			I2CBuffer[k]=EEleggi(j+k);
		}
		memcpy(&configTastiera /* +j */,&I2CBuffer,min(i,32));
		i -= min(i,32);
		j += min(i,32);
		} while(i>0);
	if(configTastiera.signature != 0x47) {
		resetKBdefaults();
		saveKBdefaults();
		}

#else

#if !defined(__DEBUG) 		// in simulatore non ho i2c...
	j=0;
	do {
		I2CRead16Seq(j,32);			// sizeof < 32 !
		memcpy(&configTastiera /*+j */,&I2CBuffer,min(i,32));
		i -= min(i,32);
		j += min(i,32);
		} while(i>0);
	if(configTastiera.signature != 0x47) {
		resetKBdefaults();
		saveKBdefaults();
		}
#else
	resetKBdefaults();
#endif

#endif

#else
	resetKBdefaults();
#endif

	return 1;
	}

signed char saveKBdefaults(void) {
	signed char i=sizeof(struct CONFIG_TASTIERA),j;

#ifdef USA_EEPROM

#ifdef USA_EEPROM_INTERNA

	j=0;
	do {
		memcpy(&I2CBuffer,&configTastiera,min(i,32));
		{
		BYTE k;
		for(k=0; k<min(i,32); k++)		// sizeof < 32 !
			EEscrivi(j+k,I2CBuffer[k]);
		}
		i -= min(i,32);
		j += min(i,32);
		} while(i>0);

#else

#if !defined(__DEBUG) 		// in simulatore non ho i2c...
	j=0;
	do {
		memcpy(&I2CBuffer,&configTastiera /* +j */,min(i,32));
		I2CWritePage16(j,32);			// sizeof < 32 !
		i -= min(i,32);
		j += min(i,32);
		} while(i>0);
#endif

#endif
	return 1;

#else
	return 0;
#endif

	}


// EEprom *******************************************************************
#ifdef USA_EEPROM_INTERNA 
void EEscrivi(BYTE addr,BYTE n) {		// usare void * ?

	EEADR = (BYTE)addr;
	EEDATA=n;

	EECON1bits.EEPGD=0;		// Point to Data Memory
	EECON1bits.CFGS=0;		// Access EEPROM
	EECON1bits.WREN=1;

	INTCONbits.GIE = 0;			// disattiva interrupt globali... e USB?
	EECON2=0x55;		 // Write 55h
	EECON2=0xAA;		 // Write AAh
	EECON1bits.WR=1;									// abilita write.
	INTCONbits.GIE = 1;			// attiva interrupt globali
	do {
		ClrWdt();
		} while(EECON1bits.WR);							// occupato ? 


	EECON1bits.WREN=0;								// disabilita write.
  }

BYTE EEleggi(BYTE addr) {			// usare void * ?

	EEADR=(BYTE)addr;			// Address to read
	EECON1bits.EEPGD=0;		// Point to Data Memory
	EECON1bits.CFGS=0;		// Access EEPROM
	EECON1bits.RD=1;		// EE Read
	return EEDATA;				// W = EEDATA
	}
#endif

/********************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BlinkUSBStatus turns on and off LEDs 
 *                  corresponding to the USB device state.
 *
 * Note:            mLED macros can be found in HardwareProfile.h
 *                  USBDeviceState is declared and updated in
 *                  usb_device.c.
 *******************************************************************/
void BlinkUSBStatus(void) {
  static WORD led_count=0;
    
	#if !defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
return;
#endif

  if(led_count == 0)
		led_count = 10000U;
  led_count--;

  #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
  #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
  #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
  #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

  if(USBSuspendControl == 1) {
        if(led_count==0) {
            mLED_1_Toggle();
            if(mGetLED_1()) {
              mLED_2_On();
            	}
            else {
              mLED_2_Off();
            	}
        	}	//end if
    	}
    else {
      if(USBDeviceState == DETACHED_STATE) {
        mLED_Both_Off();
        }
      else if(USBDeviceState == ATTACHED_STATE) {
        mLED_Both_On();
        }
      else if(USBDeviceState == POWERED_STATE) {
        mLED_Only_1_On();
        }
      else if(USBDeviceState == DEFAULT_STATE) {
        mLED_Only_2_On();
        }
      else if(USBDeviceState == ADDRESS_STATE) {
        if(led_count == 0) {
          mLED_1_Toggle();
          mLED_2_Off();
          }//end if
        }
      else if(USBDeviceState == CONFIGURED_STATE) {
        if(led_count==0) {
          mLED_1_Toggle();
          if(mGetLED_1()) {
            mLED_2_Off();
          	}
          else {
            mLED_2_On();
          }
        }	//end if
      }	//end if(...)
    }	//end if(UCONbits.SUSPND...)

	}	//end BlinkUSBStatus





// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

// Note *: The "usb_20.pdf" specs indicate 500uA or 2.5mA, depending upon device classification. However,
// the USB-IF has officially issued an ECN (engineering change notice) changing this to 2.5mA for all 
// devices.  Make sure to re-download the latest specifications to get all of the newest ECNs.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Callback that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void USBCBSuspend(void) {
	//Example power saving code.  Insert appropriate code here for the desired
	//application behavior.  If the microcontroller will be put to sleep, a
	//process similar to that shown below may be used:
	
	//ConfigureIOPinsForLowPower();
	//SaveStateOfAllInterruptEnableBits();
	//DisableAllInterruptEnableBits();
	//EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
	//Sleep();
	//RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
	//RestoreIOPinsToNormal();									//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

	//Alternatively, the microcontroller may use clock switching to reduce current consumption.
//  	mLED_1_Toggle();
//   	mLED_2_Toggle();

#ifdef USE_SW_SUSPEND
#warning SW_SUSPEND in use!
	if(!inSuspend) {				// pare che passi di qua in continuazione, quando è in suspend (forse perché non lo gestiamo/non andiamo davvero in sleep).
													// allora facciam così
		inSuspend=TRUE;
		{
		BYTE i;
	//	for(i=0; i<50; i++)	{			// 3sec?
	    WORD delay_count = 50000U;                // 60ms ca. @48MHz
	    do {
	      delay_count--;
				ClrWdt();
	      } while(delay_count);        
		}
	}
//	}
#else
	#if defined(__18CXX)
    	//Configure device for low power consumption
    	mLED_1_Off();
    	mLED_2_Off();
    	//Should also configure all other I/O pins for lowest power consumption.
    	//Typically this is done by driving unused I/O pins as outputs and driving them high or low.
    	//In this example, this is not done however, in case the user is expecting the I/O pins
    	//to remain tri-state and has hooked something up to them.
    	//Leaving the I/O pins floating will waste power and should not be done in a
    	//real application.  

        //Note: The clock switching code needed is processor specific, as the 
        //clock trees and registers aren't identical accross all PIC18 USB device
        //families.
    	#if defined(PIC18F97J94_FAMILY)
          OSCCON = 0x06;  //FRC / 16 = 500kHz selected.
    	#else
        	OSCCON = 0x13;	//Sleep on sleep, 125kHz selected as microcontroller clock source
    	#endif
	#endif
//  Sleep();                                // Goto sleep
#endif


	//IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is 
	//cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause 
	//things to not work as intended.	
	
	}

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *					
 *					This call back is invoked when a wakeup from USB suspend 
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void) {
	// If clock switching or other power savings measures were taken when
	// executing the USBCBSuspend() function, now would be a good time to
	// switch back to normal full power run mode conditions.  The host allows
	// 10+ milliseconds of wakeup time, after which the device must be 
	// fully back to normal, and capable of receiving and processing USB
	// packets.  In order to do this, the USB module must receive proper
	// clocking (IE: 48MHz clock must be available to SIE for full speed USB
	// operation).  
	// Make sure the selected oscillator settings are consistant with USB operation 
	// before returning from this function.

	//Switch clock back to main clock source necessary for USB operation
	//Previous clock source was something low frequency as set in the 
	//USBCBSuspend() function.
	#if defined(__18CXX)
        #if defined(PIC18F97J94_FAMILY)
            OSCCON3 = 0x01; //8MHz FRC / 2 = 4MHz output
            OSCCON = 0x01;  //FRC with PLL selected 
            while(OSCCON2bits.LOCK == 0);   //Wait for PLL lock       
    	#elif defined(PIC18F45K50_FAMILY)
            OSCCON = 0x70;  //Switch to 16MHz HFINTOSC (+ PLL)
            while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
        #else
        	OSCCON = 0x60;		//Primary clock source selected.
            //Adding a software start up delay will ensure
            //that the primary oscillator and PLL are running before executing any other
            //code.  If the PLL isn't being used, (ex: primary osc = 48MHz externally applied EC)
            //then this code adds a small unnecessary delay, but it is harmless to execute anyway.
        	{
            unsigned int pll_startup_counter = 800;	//Long delay at 31kHz, but ~0.8ms at 48MHz
            while(pll_startup_counter--);			//Clock will switch over while executing this delay loop
            }
        #endif
	#endif		

#ifdef USE_SW_SUSPEND
//	inSuspend=0;
#endif

	}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void) {
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
	}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void) {
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

	// Typically, user firmware does not need to do anything special
	// if a USB error occurs.  For example, if the host sends an OUT
	// packet to your device, but the packet gets corrupted (ex:
	// because of a bad connection, or the user unplugs the
	// USB cable during the transmission) this will typically set
	// one or more USB error interrupt flags.  Nothing specific
	// needs to be done however, since the SIE will automatically
	// send a "NAK" packet to the host.  In response to this, the
	// host will normally retry to send the packet again, and no
	// data loss occurs.  The system will typically recover
	// automatically, without the need for application firmware
	// intervention.
	
	// Nevertheless, this callback function is provided, such as
	// for debugging purposes.
	}


/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and 
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific 
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void) {
  
	USBCheckHIDRequest();
	}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void USBCBStdSetDscHandler(void) {
  // Must claim session ownership if supporting this request
	
	}	//end


/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This 
 *					callback function should initialize the endpoints 
 *					for the device's usage according to the current 
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void) {

    //enable the HID endpoint
  USBEnableEndpoint(HID_EP,USB_IN_ENABLED /*| USB_OUT_ENABLED non serve...*/ | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);

#ifdef KEYBOARD_WITH_DATA
  USBEnableEndpoint(HID_EP2,USB_IN_ENABLED | USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
  USBOutHandle = HIDRxPacket(HID_EP2,(BYTE*)&ReceivedDataBuffer,HID_INT_OUT_EP2_SIZE);   //canale dati!!
#endif
    
//Arm OUT endpoint so we can receive caps lock, num lock, etc. info from host
// boh
//	lastOUTTransmission = HIDRxPacket(HID_EP,(BYTE*)&hid_report_out,1);

	}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void) {
  static WORD delay_count;
    
    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager 
    //properties page for the USB device, power management tab, the 
    //"Allow this device to bring the computer out of standby." checkbox 
    //should be checked).
  if(USBGetRemoteWakeupStatus() == TRUE) {
      //Verify that the USB bus is in fact suspended, before we send remote wakeup signalling.
    if(USBIsBusSuspended() == TRUE) {
      USBMaskInterrupts();
          
      //Clock switch to settings consistent with normal USB operation.
      USBCBWakeFromSuspend();
      USBSuspendControl = 0; 
      USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                  //until a new suspend condition is detected.

      //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
      //device must continuously see 5ms+ of idle on the bus, before it sends
      //remote wakeup signalling.  One way to be certain that this parameter
      //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
      //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
      //5ms+ total delay since start of idle).
      delay_count = 3600U;        
      do {
        delay_count--;
      	} while(delay_count);
      
      //Now drive the resume K-state signalling onto the USB bus.
      USBResumeControl = 1;       // Start RESUME signaling
      delay_count = 1800U;        // Set RESUME line for 1-13 ms
      do {
        delay_count--;
        } while(delay_count);
      USBResumeControl = 0;       //Finished driving resume signalling

      USBUnmaskInterrupts();
      }
    }
	}


/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        int event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           int event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size) {

  switch(event) {
    case EVENT_TRANSFER:
      //Add application specific callback task or callback function here if desired.
      break;
    case EVENT_SOF:
      USBCB_SOF_Handler();
      break;
    case EVENT_SUSPEND:
      USBCBSuspend();
      break;
    case EVENT_RESUME:
      USBCBWakeFromSuspend();
      break;
    case EVENT_CONFIGURED: 
      USBCBInitEP();
      break;
    case EVENT_SET_DESCRIPTOR:
      USBCBStdSetDscHandler();
      break;
    case EVENT_EP0_REQUEST:
      USBCBCheckOtherReq();
      break;
    case EVENT_BUS_ERROR:
      USBCBErrorHandler();
      break;
    case EVENT_TRANSFER_TERMINATED:
        //Add application specific callback task or callback function here if desired.
        //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
        //FEATURE (endpoint halt) request on an application endpoint which was 
        //previously armed (UOWN was = 1).  Here would be a good place to:
        //1.  Determine which endpoint the transaction that just got terminated was 
        //      on, by checking the handle value in the *pdata.
        //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
        //      endpoints).
      break;
    default:
      break;
    }      

  return TRUE; 
	}


// *****************************************************************************
// ************** USB Class Specific Callback Function(s) **********************
// *****************************************************************************

/********************************************************************
 * Function:        void USBHIDCBSetReportHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        USBHIDCBSetReportHandler() is used to respond to
 *					the HID device class specific SET_REPORT control
 *					transfer request (starts with SETUP packet on EP0 OUT).  
 * Note:            
 *******************************************************************/
void USBHIDCBSetReportHandler(void) {

	//Prepare to receive the keyboard LED state data through a SET_REPORT
	//control transfer on endpoint 0.  The host should only send 1 BYTE,
	//since this is all that the report descriptor allows it to send.
	USBEP0Receive((BYTE*)&CtrlTrfData, USB_EP0_BUFF_SIZE, USBHIDCBSetReportComplete);
	}

//Secondary callback function that gets called when the above
//control transfer completes for the USBHIDCBSetReportHandler()
void USBHIDCBSetReportComplete(void) {

	//1 BYTE of LED state data should now be in the CtrlTrfData buffer.

	//Num Lock LED state is in Bit0.
	if(CtrlTrfData[1 /*era 0 diventa 1 con il multimedia*/ ] & 0x01)	{		//Make LED1 and LED2 match Num Lock state.
		if(!configTastiera.NumCapsScrollScambiati) {
			AccendiLedNum();
//			mLED_1_On();
			}
		else {
			AccendiLedScroll();
//			mLED_3_On();
			}
		}
	else {
		if(!configTastiera.NumCapsScrollScambiati) {
			SpegniLedNum();
//			mLED_1_Off();
			}
		else {
			SpegniLedScroll();
//			mLED_3_Off();
			}
		}
	if(CtrlTrfData[1] & 0x02)	{		//Make LED2 match Caps Lock state.
		AccendiLedCaps();
//		mLED_2_On();
		}
	else {
		SpegniLedCaps();
//		mLED_2_Off();			
		}
	if(CtrlTrfData[1] & 0x04)	{		//Make LED3 match Scroll Lock state.
		if(!configTastiera.NumCapsScrollScambiati) {
			AccendiLedScroll();
//			mLED_3_On();
			}
		else {
			AccendiLedNum();
//			mLED_1_On();
			}
		}
	else {
		if(!configTastiera.NumCapsScrollScambiati) {
			SpegniLedScroll();
//			mLED_3_Off();
			}
		else {
			SpegniLedNum();
//			mLED_1_Off();
			}
		}

	if(configTastiera.numlockIsLayout) {	// il Num-Lock trasforma parte della tastiera in tast. numerico
		if(!configTastiera.NumCapsScrollScambiati) {
			if(mGetLED_1())
				kbLayout=1;
			else
				kbLayout=0;
			}
		else {
			if(mGetLED_3())
				kbLayout=1;
			else
				kbLayout=0;
			}
		}
	
	//Stop toggling the LEDs, so you can temporily see the Num lock LED state instead.
	//Once the CountdownTimerToShowUSBStatusOnLEDs reaches 0, the LEDs will go back to showing USB state instead.
	BlinkStatusValid = FALSE;	
	CountdownTimerToShowUSBStatusOnLEDs = 140000; 
	}	

/** EOF Keyboard.c **********************************************/
#endif
