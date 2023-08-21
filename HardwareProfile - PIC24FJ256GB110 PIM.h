/********************************************************************
 FileName:     	HardwareProfile - PIC24FJ256GB110 PIM.h
 Dependencies:  See INCLUDES section
 Processor:     PIC24FJ256GB110
 Hardware:      PIC24FJ256GB110 PIM
 Compiler:      Microchip C30
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
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
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
  2.3   09/15/2008   Broke out each hardware platform into its own
                     "HardwareProfile - xxx.h" file
********************************************************************/

#ifndef HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H
#define HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H

    /*******************************************************************/
    /******** USB stack hardware selection options *********************/
    /*******************************************************************/
    //This section is the set of definitions required by the MCHPFSUSB
    //  framework.  These definitions tell the firmware what mode it is
    //  running in, and where it can find the results to some information
    //  that the stack needs.
    //These definitions are required by every application developed with
    //  this revision of the MCHPFSUSB framework.  Please review each
    //  option carefully and determine which options are desired/required
    //  for your application.

    //#define USE_SELF_POWER_SENSE_IO
    #define tris_self_power     TRISAbits.TRISA2    // Input
    #define self_power          1

    //#define USE_USB_BUS_SENSE_IO
    #define tris_usb_bus_sense  TRISBbits.TRISB5    // Input
    #define USB_BUS_SENSE       1 
   
    //Uncomment this to make the output HEX of this project 
    //   to be able to be bootloaded using the HID bootloader
//    #define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER	

    //If the application is going to be used with the HID bootloader
    //  then this will provide a function for the application to 
    //  enter the bootloader from the application (optional)
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
        #define EnterBootloader() __asm__("goto 0x400")
    #endif   

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/

    /** Board definition ***********************************************/
    //These defintions will tell the main() function which board is
    //  currently selected.  This will allow the application to add
    //  the correct configuration bits as wells use the correct
    //  initialization functions for the board.  These defitions are only
    //  required in the stack provided demos.  They are not required in
    //  final application design.
    #define DEMO_BOARD PIC24FJ256GB110_PIM
    #define EXPLORER_16
    #define PIC24FJ256GB110_PIM
    #define CLOCK_FREQ 32000000
    
    /** LED ************************************************************/
//#define KTRONIC 1
#ifndef PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		// prove 2015 su demoboard
    #define mInitAllLEDs()      LATE &= 0xE0; TRISA &= 0xE0;
    
    #define mLED_1              LATAbits.LATA0			// numlock
    #define mLED_2              LATAbits.LATA1			// capslock
    #define mLED_3              LATAbits.LATA2			// scrolllock
    #define mLED_4              LATAbits.LATA3			// chiave
    #define mLED_0              LATAbits.LATA4			// test blinker
    #define m_LedChiaveBit			LATAbits.LATA5			// sistemare da qualche parte

    #define mGetLED_1()         mLED_1
    #define mGetLED_2()         mLED_2
    #define mGetLED_3()         mLED_3
    #define mGetLED_4()         mLED_4

    #define mLED_1_On()         mLED_1 = 0;
    #define mLED_2_On()         mLED_2 = 0;
    #define mLED_3_On()         mLED_3 = 0;
    #define mLED_4_On()         mLED_4 = 0;
    
    #define mLED_1_Off()        mLED_1 = 1;
    #define mLED_2_Off()        mLED_2 = 1;
    #define mLED_3_Off()        mLED_3 = 1;
    #define mLED_4_Off()        mLED_4 = 1;
    
    #define mLED_1_Toggle()     mLED_1 = !mLED_1;
    #define mLED_2_Toggle()     mLED_2 = !mLED_2;
    #define mLED_3_Toggle()     mLED_3 = !mLED_3;
    #define mLED_4_Toggle()     mLED_4 = !mLED_4;

#define m_Buzzer LATCbits.LATC2

#else
   
    #define mInitAllLEDs()      LATD &= 0xF0; TRISD &= 0xF0;
    
    #define mLED_1              LATDbits.LATD0
    #define mLED_2              LATDbits.LATD1
    #define mLED_3              LATDbits.LATD2
    #define mLED_4              LATDbits.LATD3
    #define mLED_0              LATDbits.LATD3			// test blinker ??
    #define m_LedChiaveBit			LATAbits.LATA5			// sistemare da qualche parte

    #define mGetLED_1()         mLED_1
    #define mGetLED_2()         mLED_2
    #define mGetLED_3()         mLED_3
    #define mGetLED_4()         mLED_4

    #define mLED_1_On()         mLED_1 = 1;
    #define mLED_2_On()         mLED_2 = 1;
    #define mLED_3_On()         mLED_3 = 1;
    #define mLED_4_On()         mLED_4 = 1;
    
    #define mLED_1_Off()        mLED_1 = 0;
    #define mLED_2_Off()        mLED_2 = 0;
    #define mLED_3_Off()        mLED_3 = 0;
    #define mLED_4_Off()        mLED_4 = 0;
    
    #define mLED_1_Toggle()     mLED_1 = !mLED_1;
    #define mLED_2_Toggle()     mLED_2 = !mLED_2;
    #define mLED_3_Toggle()     mLED_3 = !mLED_3;
    #define mLED_4_Toggle()     mLED_4 = !mLED_4;

#define m_Buzzer LATCbits.LATC2

#endif

#define AccendiLedScroll() mLED_3_On()
#define AccendiLedNum() 	 mLED_1_On()
#define AccendiLedCaps() 	 mLED_2_On()
#define AccendiLedChiave() m_LedChiaveBit=0

#define SpegniLedScroll() mLED_3_Off()
#define SpegniLedNum() 		mLED_1_Off()
#define SpegniLedCaps() 	mLED_2_Off()
#define SpegniLedChiave() m_LedChiaveBit=1

#define ToggleLedScroll() mLED_3^=1
#define ToggleLedNum() 		mLED_1^=1
#define ToggleLedCaps() 	mLED_2^=1
#define ToggleLedChiave() m_LedChiaveBit^=1

#define StatoLedNum()		 !mLED_1
#define StatoLedChiave() !m_LedChiaveBit


    
    /** SWITCH *********************************************************/
#ifndef PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		// prove 2015 su demoboard
    #define mInitAllSwitches()  {}
    #define mInitSwitch2()      {}
    #define mInitSwitch3()      {}
    #define sw2                 0
    #define sw3                 0
#else
    #define mInitAllSwitches()  TRISBbits.TRISB4=1;TRISBbits.TRISB5=1;
    #define mInitSwitch2()      TRISBbits.TRISB4=1;
    #define mInitSwitch3()      TRISBbits.TRISB5=1;
    #define sw2                 PORTBbits.RB4
    #define sw3                 PORTBbits.RB5
#endif
    
    /** USB external transceiver interface (optional) ******************/
    #define tris_usb_vpo        TRISBbits.TRISB3    // Output
    #define tris_usb_vmo        TRISBbits.TRISB2    // Output
    #define tris_usb_rcv        TRISAbits.TRISA4    // Input
    #define tris_usb_vp         TRISCbits.TRISC5    // Input
    #define tris_usb_vm         TRISCbits.TRISC4    // Input
    #define tris_usb_oe         TRISCbits.TRISC1    // Output
    
    #define tris_usb_suspnd     TRISAbits.TRISA3    // Output
    
    /** I/O pin definitions ********************************************/
    #define INPUT_PIN 1
    #define OUTPUT_PIN 0


    #define m_I2CClkBit         LATFbits.LATF0
    #define m_I2CDataBit        LATFbits.LATF1
    #define m_I2CDataBitI       PORTFbits.RF1
    #define m_I2CClkBitI        PORTFbits.RF0
    #define I2CDataTris      		TRISFbits.TRISF1
    #define I2CClkTris       		TRISFbits.TRISF0
		#define SPI_232_I 					PORTFbits.RF1
		#define SPI_232_IO 					LATFbits.LATF1
		#define SPI_232_I_TRIS 			TRISFbits.TRISF1
		#define SPI_232_O 					LATFbits.LATF0
		#define SPI_232_OI 					PORTFbits.RF0
		#define SPI_232_O_TRIS 			TRISFbits.TRISF0



	// PIC24F processor
	#define GetSystemClock()		(32000000ul)      // Hz
	#define GetInstructionClock()	(GetSystemClock()/2)
	#define GetPeripheralClock()	GetInstructionClock()

	#define FCY (GetSystemClock()/2)		// per LibPic30.h e delay

#endif  //HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H
