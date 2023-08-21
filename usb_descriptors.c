/********************************************************************
 FileName:     	usb_descriptors.c
 Dependencies:  See INCLUDES section
 Processor:	    PIC18, PIC24, dsPIC, and PIC32 USB Microcontrollers
 Hardware:      This demo is natively intended to be used on Microchip USB demo
                boards supported by the MCHPFSUSB stack.  See release notes for
                support matrix.  This demo can be modified for use on other 
                hardware platforms.
 Complier:  	Microchip C18 (for PIC18), C30 (for PIC24/33), C32 (for PIC32)
 Company:		Microchip Technology, Inc.

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


*********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.  
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a BYTE array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were 
changed in v2.x of the stack from a structure to a BYTE array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/
 
/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/
#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_function_hid.h"
#include "HardwareProfile.h"

/** CONSTANTS ******************************************************/
#if defined(__18CXX)
#pragma romdata
#endif

/* Device Descriptor */
ROM USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,    // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,                // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,          // Max packet size for EP0, see usb_config.h
	  0x2C88 /*0x115F*/,                 // Vendor ID KTronic 2015 (ADPM)
#ifdef KEYBOARD_WITH_DATA
#ifndef USE_2_HID_ENDPOINT
#error Servono 2 endpoint!!
#endif
  0x0107,                 // Product ID: tastiera KTronic con canale dati per riprogrammazione ecc.
#else
#ifdef MULTIMEDIA_KEYBOARD
  0x0106,                 // Product ID: tastiera "Logitech" ADPM Ktronic 
#else
  0x0101,                 // Product ID: tastiera "Mac" ADPM Ktronic 
#endif
#endif
  MAKEWORD(VERNUML,VERNUMH),                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
ROM BYTE configDescriptor1[]={
  /* Configuration Descriptor */
  0x09,    // Size of this descriptor in bytes
  USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
#ifdef USE_2_HID_ENDPOINT
  DESC_CONFIG_WORD((9+SIZEOF_IF0+SIZEOF_IF1)),      // Total length of data for this cfg; v ANCHE USB_FUNCTION_HID!
  2,                      // Number of interfaces in this cfg
#else
  DESC_CONFIG_WORD((9+SIZEOF_IF0)),      // Total length of data for this cfg; v ANCHE USB_FUNCTION_HID!
  1,                      // Number of interfaces in this cfg
#endif
  1,                      // Index value of this configuration
  3,                      // Configuration string index
  _DEFAULT /*|  _SELF */ | _RWU,          // Attributes, see usbdefs_std_dsc.h
	70,	// MaxPower (in 2mA units); self-powered draws 0 mA from the bus.
	// calcolo un maggior consumo ... ANCHE QUA! 2012

  /* Interface Descriptor #1 */
  0x09 /*sizeof(USB_INTF_DSC)*/,   // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
  HID_INTF_ID,                      // Interface Number
  0,                      // Alternate Setting Number
  1 /*2 v. asm vecchio*/, // Number of endpoints in this intf
  HID_INTF,               // Class code
  BOOT_INTF_SUBCLASS,     // Subclass code
  HID_PROTOCOL_KEYBOARD,     // Protocol code
  4,                      // Interface string index
    
  /* HID Class-Specific Descriptor */
  sizeof(USB_HID_DSC)+3,    // Size of this descriptor in bytes
  DSC_HID,                // HID descriptor type
  DESC_CONFIG_WORD(0x111 /*0x0200*/),     // HID Spec Release Number in BCD format
  0x0d,                   // Country Code (0x00 for Not supported)
  HID_NUM_OF_DSC,         // Number of class descriptors, see usbcfg.h
  DSC_RPT,                // Report descriptor type
  DESC_CONFIG_WORD(HID_RPT01_SIZE),      // Size of the report descriptor

    /* Endpoint Descriptor */
  0x07,	USB_DESCRIPTOR_ENDPOINT, HID_EP | _EP_IN, _INTERRUPT, DESC_CONFIG_WORD(HID_INT_IN_EP_SIZE), 10			/// INTERVAL + veloce per encoder ECC!! boh
,
//http://www.microchip.com/forums/m658374-print.aspx
#ifdef USE_2_HID_ENDPOINT
	/* Interface Descriptor #2 */
  0x09,   // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
  HID_INTF_ID2,                      // Interface Number
  0,                      // Alternate Setting Number
  2,                     // Number of endpoints in this intf
 /*255*/  HID_INTF,              // Class code (anche qui Dallas mette 0xff, a questo e i due segg.)
  0,							// Subclass code
  HID_PROTOCOL_NONE /*0*/,              // Protocol code
  5,                   // Interface string index

  /* HID Class-Specific Descriptor */
  sizeof(USB_HID_DSC)+3,    // Size of this descriptor in bytes
  DSC_HID,                // HID descriptor type
  DESC_CONFIG_WORD(0x111 /*0x0200*/),                 // HID Spec Release Number in BCD format
  0x0d,                   // Country Code (0x00 for Not supported)
  HID_NUM_OF_DSC,         // Number of class descriptors, see usbcfg.h
  DSC_RPT,                // Report descriptor type
  DESC_CONFIG_WORD(HID_RPT02_SIZE),      // Size of the report descriptor

  0x07,USB_DESCRIPTOR_ENDPOINT,HID_EP2 | _EP_IN,_INTERRUPT,DESC_CONFIG_WORD(HID_INT_IN_EP2_SIZE),0x01,
  0x07,USB_DESCRIPTOR_ENDPOINT,HID_EP2 | _EP_OUT,_INTERRUPT,DESC_CONFIG_WORD(HID_INT_OUT_EP2_SIZE),0x01
#endif
	};


//Language code string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[1];} sd000={
	sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409
	}};

//Manufacturer string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[23];} sd001={
	sizeof(sd001),USB_DESCRIPTOR_STRING,
	{'O','A','(','C','y','b','e','r','d','y','n','e',' ','(','e','x','-','A','D','P','M',')',')'}};

//Product string descriptor
#ifdef KEYBOARD_WITH_DATA
ROM struct { BYTE bLength; BYTE bDscType; 
#ifdef MULTIMEDIA_KEYBOARD
	WORD string[58]; 
#else
	WORD string[47]; 
#endif
	} sd002 = {
	sizeof(sd002),USB_DESCRIPTOR_STRING,
	{'U','S','B',' ','K','e','y','b','o','a','r','d',' ','K','-','t','r','o','n','i','c',' ','w','i','t','h',' ','d','a','t','a',' ','c','h','a','n','n','e','l'
#ifdef MULTIMEDIA_KEYBOARD
	,'M','u','l','t','i','m','e','d','i','a',' '
#endif
#ifdef KTRONIC
	,'K','-','t','r','o','n','i','c'}};
#else
	,'G','.','D','a','r','*','*','*'}};
#endif
#else
ROM struct { BYTE bLength; BYTE bDscType; 
#ifdef MULTIMEDIA_KEYBOARD
	WORD string[32]; 
#else
	WORD string[21]; 
#endif
	} sd002 = {
	sizeof(sd002),USB_DESCRIPTOR_STRING,
	{'U','S','B',' ','K','e','y','b','o','a','r','d',' '
#ifdef MULTIMEDIA_KEYBOARD
	,'M','u','l','t','i','m','e','d','i','a',' '
#endif
#ifdef KTRONIC
	,'K','-','t','r','o','n','i','c'}};
#else
	,'G','.','D','a','r','*','*','*'}};
#endif
#endif

ROM struct{BYTE bLength; BYTE bDscType; WORD string[5];} sd003={
	sizeof(sd003),USB_DESCRIPTOR_STRING,
	{'V','2','.','4','0'}};
ROM struct{BYTE bLength; BYTE bDscType; WORD string[8];} sd004={
	sizeof(sd004),USB_DESCRIPTOR_STRING,
	{'K','e','y','b','o','a','r','d'}};
ROM struct { BYTE bLength; BYTE bDscType; WORD string[17]; } sd005 = {
	sizeof(sd005),USB_DESCRIPTOR_STRING,
	{'K','e','y','b','o','a','r','d',' ','D','a','t','a','P','i','p','e'}};

//Class specific descriptor - HID Keyboard
ROM struct{BYTE report[HID_RPT01_SIZE];} hid_rpt01={
{   0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,  /*   Report ID  2015 usato per poter poi inviare pure multimedia... quindi serve prefisso 1 su tasti normali...*/
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                          // End Collection
#ifdef MULTIMEDIA_KEYBOARD
	,
	0x05, 0x0C, /*		Usage Page (Consumer Devices)		*/
	0x09, 0x01, /*		Usage (Consumer Control)			*/
	0xA1, 0x01, /*		Collection (Application)			*/
	0x85, 0x02,	/*		Report ID=2							*/
	0x05, 0x0C, /*		Usage Page (Consumer Devices)		*/
	0x15, 0x00, /*		Logical Minimum (0)					*/
	0x25, 0x01, /*		Logical Maximum (1)					*/
	0x75, 0x01, /*		Report Size (1)						*/
	0x95, 0x10, /*		Report Count (16)					*/
	0x09, 0xe2, // USAGE (Mute) 0x01
	0x09, 0xe9, // USAGE (Volume Up) 0x02
	0x09, 0xea, // USAGE (Volume Down) 0x03
	0x09, 0xcd, // USAGE (Play/Pause) 0x04
	0x09, 0xb7, // USAGE (Stop) 0x05
	0x09, 0xb6, // USAGE (Scan Previous Track) 0x06
	0x09, 0xb5, // USAGE (Scan Next Track) 0x07
	0x0a, 0x8a, 0x01, // USAGE (Mail) 0x08
	0x0a, 0x92, 0x01, // USAGE (Calculator) 0x09
	0x0a, 0x21, 0x02, // USAGE (www search) 0x0a
	0x0a, 0x23, 0x02, // USAGE (www home) 0x0b
	0x0a, 0x2a, 0x02, // USAGE (www favorites) 0x0c
	0x0a, 0x27, 0x02, // USAGE (www refresh) 0x0d
	0x0a, 0x26, 0x02, // USAGE (www stop) 0x0e
	0x0a, 0x25, 0x02, // USAGE (www forward) 0x0f
	0x0a, 0x24, 0x02, // USAGE (www back) 0x10
	0x81, 0x62, // INPUT (Data,Var,Abs,NPrf,Null)	
	0xC0,		/*		End Collection						*/

0x05, 0x01, /* Usage Page (Generic Desktop) */
0x09, 0x80, /* Usage (System Control) */
0xA1, 0x01, /* Collection (Application) */
0x85, 0x03, /* Report ID 0x03 [SYSTEM CTRL] */
0x19, 0x81, /* Usage minimum (System POWER (era Sleep=82)) */
0x29, 0x83, /* Usage maximum (System Wake up) */
0x95, 0x02, /* Report count (2) */
0x81, 0x06, /*Input (data, variable, relative, Preferred) */
0x95, 0x06, /* Report count (6) */
0x81, 0x01, /*Input (Constant) */
0xC0 /*End Collection */

#endif
	}
};

ROM struct { BYTE report[HID_RPT02_SIZE]; } hid_rpt02={ 
{
    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
                                   // -------- common global items ---------
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
                                   // -------- input report ---------
    0x85, 1, /*      Report ID (1)												*/
    0x95, HID_INT_IN_EP2_SIZE-1,                    //   REPORT_COUNT (64)
    0x19, 0x01,                    //   USAGE_MINIMUM (Vendor Usage 1)
    0x29, HID_INT_IN_EP2_SIZE,                    //   USAGE_MAXIMUM (Vendor Usage 64)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
                                   // -------- output report ---------
    0x85, 2, /*      Report ID (2)												*/
    0x95, HID_INT_OUT_EP2_SIZE-1,                    //   REPORT_COUNT (64)
    0x19, 0x01,                    //   USAGE_MINIMUM (Vendor Usage 1)
    0x29, HID_INT_OUT_EP2_SIZE,                    //   USAGE_MAXIMUM (Vendor Usage 64)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
 
    0xc0                           // END_COLLECTION
	}	};                   // End Collection};	

//Array of configuration descriptors
ROM BYTE *ROM USB_CD_Ptr[]=
{
    (ROM BYTE *ROM)&configDescriptor1
};

//Array of string descriptors
ROM BYTE *ROM USB_SD_Ptr[]=
{
    (ROM BYTE *ROM)&sd000,
    (ROM BYTE *ROM)&sd001,
    (ROM BYTE *ROM)&sd002,
    (ROM BYTE *ROM)&sd003,
    (ROM BYTE *ROM)&sd004,
    (ROM BYTE *ROM)&sd005
};

/** EOF usb_descriptors.c ***************************************************/

#endif
