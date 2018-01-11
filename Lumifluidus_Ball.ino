// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// LightBall_V1
// 	uses Hope RFM69 Module to transmit data.
// 	receives color Information for LED-Display.
// 	Answers 'How Is There' Packages.
// 	debugout on usbserial interface: 115200baud
//
// hardware:
// 	Board:
// 		Arduino compatible
// 		with RFM69 module
// 		for example: Mega1284RFM69_PCB
//
// libraries used:
// 	~ Wire.h  arduino TWI-lib
// 	~ SPI.h arduino SPI-lib
// 	~ EEPROMEx.h
// 		Extended EEPROM library
// 		Copyright (c) 2012 Thijs Elenbaas.  All right reserved.
// 		GNU Lesser General Public  License, version 2.1
// 		http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino/
// 	~ RFM69.h
// 		RFM69 Library
// 		for Hope RF RFM69W, RFM69HW, RFM69CW, RFM69HCW (semtech SX1231, SX1231H)
// 		Copyright Felix Rusu (2014), felix@lowpowerlab.com
// 		http://lowpowerlab.com/
// 		GNU General Public License 3 (http://www.gnu.org/licenses/gpl-3.0.txt)
// 		https://github.com/LowPowerLab/
// 	~ Adafruit_TLC59711.h
// 		Adafruit Library
// 		for Texas Instruments TLC59711 12ch 16bit LED constant sink driver
// 			This is a library for our Adafruit 12-channel PWM/LED drive
// 			Pick one up today in the adafruit shop!
// 			------> http://www.adafruit.com/products/1455
// 			These drivers uses SPI to communicate, 2 pins are required to
// 			interface: Data and Clock. The boards are chainable.
// 			Adafruit invests time and resources providing this open source code,
// 			please support Adafruit and open-source hardware by purchasing products from Adafruit!
// 			Written by Limor Fried/Ladyada for Adafruit Industries.
// 			BSD license, all text above must be included in any redistribution
// 		https://github.com/adafruit/Adafruit_TLC59711
// 	~ slight_FaderLin
// 	~ slight_ButtonInput
// 		written by stefan krueger (s-light),
// 			github@s-light.eu, http://s-light.eu, https://github.com/s-light/
// 			MIT
// 	~
//
//
// written by stefan krueger (s-light),
// 	stefan@s-light.eu, http://s-light.eu, https://github.com/s-light/
//
//
// changelog / history
// 	28.10.2014 09:24 created (based on __Template_SLIGHT.ino)
// 	28.10.2014 09:24 added EEPROMEx & DeviceIdentity.h (not finished)
// 	08.02.2015 13:47 merge things from 'HWTest_RFM69.ino'
// 	08.02.2015 23:00 basic RFM69 test works.
// 	08.02.2015 23:00 EEPROM configuration works.
// 	10.02.2015 22:55 add LED-Driver Chip test
// 	10.02.2015 22:55 extend LED test with FaderLin and ButtonInput
// 	14.03.2015 20:10 added receive Color function
// 	11.01.2018 17:02 converted to git repository
//
//
// TO DO:
// 	~
// 	~
// 	~ check RFM69 lib  license?!
// 		if i understand this right - if i use the RFM69 lib i have to license my own work also as GPL. ??
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// license
//
// GNU GENERAL PUBLIC LICENSE (Version 3, 29 June 2007 or later)
// 	details see LICENSE file
//
// Apache License Version 2.0
// 	Copyright 2015-2018 Stefan Krueger
//
// 	Licensed under the Apache License, Version 2.0 (the "License");
// 	you may not use this file except in compliance with the License.
// 	You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// 	Unless required by applicable law or agreed to in writing, software
// 	distributed under the License is distributed on an "AS IS" BASIS,
// 	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// 	See the License for the specific language governing permissions and
// 	limitations under the License.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Includes:  (must be at the beginning of the file.)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// use "" for files in same directory as .ino
// #include "file.h"

// device identity structure definitions
#include "DeviceIdentity.h"

// arduino SPI
#include <SPI.h>

// arduino TWI / I2C lib
// #include <Wire.h>

// edit EEPROMEx.h so that EEPROM.h is not include seams to work.. ?!
// #include <EEPROM.h>
#include <EEPROMEx.h>

#include <RFM69.h>

#include <Adafruit_TLC59711.h>

#include <slight_FaderLin.h>
#include <slight_ButtonInput.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// info
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void print_info(Print &pOut) {
	pOut.println();
	//             "|~~~~~~~~~|~~~~~~~~~|~~~..~~~|~~~~~~~~~|~~~~~~~~~|"
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("|                       ^ ^                      |"));
	pOut.println(F("|                      (0,0)                     |"));
	pOut.println(F("|                      ( _ )                     |"));
	pOut.println(F("|                       \" \"                      |"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("| Lumifluidus_Ball.ino"));
	pOut.println(F("|  Light Ball Test V1 ..."));
	pOut.println(F("|"));
	pOut.println(F("| This Sketch has a debug-menu:"));
	pOut.println(F("| send '?'+Return for help"));
	pOut.println(F("|"));
	pOut.println(F("| dream on & have fun :-)"));
	pOut.println(F("|"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println(F("|"));
	//pOut.println(F("| Version: Nov 11 2013  20:35:04"));
	pOut.print(F("| version: "));
	pOut.print(F(__DATE__));
	pOut.print(F("  "));
	pOut.print(F(__TIME__));
	pOut.println();
	pOut.println(F("|"));
	pOut.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pOut.println();

	//pOut.println(__DATE__); Nov 11 2013
	//pOut.println(__TIME__); 20:35:04
}


/** Serial.print to Flash: Notepad++ Replace RegEx
	Find what:		Serial.print(.*)\("(.*)"\);
	Replace with:	Serial.print\1\(F\("\2"\)\);
**/


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// definitions (gloabl)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DebugOut
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

boolean bLEDState = 0;
// const uint8_t cbID_LED_Info = 9;
#if defined(__AVR_ATmega1284P__)
	// use LED_Info on 'Mega1284RFM69_PCB'
	const uint8_t cbID_LED_Info = 1;
#else
	// default
	const uint8_t cbID_LED_Info = 9;
#endif

unsigned long ulDebugOut_LiveSign_TimeStamp_LastAction	= 0;
const uint16_t cwDebugOut_LiveSign_UpdateInterval			= 1000; //ms

boolean bDebugOut_LiveSign_Serial_Enabled	= 0;
boolean bDebugOut_LiveSign_LED_Enabled		= 1;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EEPROMEx
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int eeprom_Address_DeviceHardware = 0;
unsigned int eeprom_Address_DeviceConfig = eeprom_Address_DeviceHardware + sizeof(tDeviceHardware);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Device Information
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#include "DeviceInfo.h"

// store this in EEPROM
// This Device --> must be updated in setup to use EEPROM values
tDeviceHardware dhwDefault = {
	  1,			// uint16_t ID
	RF69_433MHZ	// uint8_t radio_Frequency;
};

tDeviceHardware dhwThisBall = {
	  1,			// uint16_t ID
	RF69_433MHZ	// uint8_t radio_Frequency;
};

tDeviceConfig dconfDefault = {
	     1,		// uint8_t bBallID;
	    42,		// uint8_t bNetworkID;
	     0,		// uint8_t bMasterID;
	     1,		// uint8_t bBallStartAddress;
	  1005,		// uint16_t uiFadeTime; (ms)
};

tDeviceConfig dconfThisBall = {
	     1,		// uint8_t bBallID;
	    42,		// uint8_t bNetworkID;
	     0,		// uint8_t bMasterID;
	     1,		// uint8_t bBallStartAddress;
	  1001,		// uint16_t uiFadeTime; (ms)
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// RFM69
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RFM69 radio;
// other config values are stored in eeprom.

const uint8_t radio_ACKTime =			  30; // ms

//radio_PromiscuosMode set to 'true' to sniff all packets on the same network
const bool radio_PromiscuosMode =	false;

//radio_KEY has to be same 16 characters/uint8_ts on all nodes, not more not less!
//									'1234567890123456'
// #define radio_KEY 				"thisIsEncryptKey"
#define radio_KEY 					"!Light_Ball_Art!"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Board
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Infrared LED on PD7 OC2A = D15
const uint8_t pinIRLED = 15;

// TLC59711
	// How many boards do you have chained?
	#define NUM_TLC59711 1
	#define data   12
	#define clock  13
	Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clock, data);

uint8_t uiLEDDriverTestStep = 0;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// slight FaderLin
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
slight_FaderLin myFader(
	0, // uint8_t cbID_New
	4*3, // uint8_t cbChannelCount_New
	myFader_callback_OutputChanged, // tCbfuncValuesChanged cbfuncValuesChanged_New
	myFader_callback_onEvent // tCbfuncStateChanged cbfuncStateChanged_New
);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// slight ButtonInput
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

slight_ButtonInput myButtonSwitchColor(
	42, // uint8_t cbID_New
	14, // uint8_t cbPin_New,
	myInput_callback_GetInput, // tCbfuncGetInput cbfuncGetInput_New,
	myCallback_onEvent, // tcbfOnEvent cbfCallbackOnEvent_New,
	  30, // const uint16_t cwDuration_Debounce_New = 30,
	1000, // const uint16_t cwDuration_HoldingDown_New = 1000,
	  50, // const uint16_t cwDuration_ClickSingle_New =   50,
	3000, // const uint16_t cwDuration_ClickLong_New =   3000,
	 100  // const uint16_t cwDuration_ClickDouble_New = 1000
);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu Input
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// a string to hold new data
char  sMenu_Input_New[]				= "x:TestValue";
// flag if string is complete
bool bMenu_Input_Flag_BF		= false; // BufferFull
bool bMenu_Input_Flag_EOL		= false;
bool bMenu_Input_Flag_CR		= false;
bool bMenu_Input_Flag_LF		= false;
bool bMenu_Input_Flag_LongLine	= false;
bool bMenu_Input_Flag_SkipRest	= false;

// string for Currently to process Command
char  sMenu_Command_Current[]		= "x:TestValue ";


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Output system
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DualWrite from pYro_65 read more at: http://forum.arduino.cc/index.php?topic=200975.0
class DualWriter : public Print{
	public:
		DualWriter( Print &p_Out1, Print &p_Out2 ) : OutputA( p_Out1 ), OutputB( p_Out2 ){}

		size_t write( uint8_t u_Data ) {
			OutputA.write( u_Data );
			OutputB.write( u_Data );
			return 0x01;
		}
	protected:
		Print &OutputA;
		Print &OutputB;
};

//DualWriter dwOUT( Serial, Serial1);



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// other things...
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug things
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// http://forum.arduino.cc/index.php?topic=183790.msg1362282#msg1362282
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void printBinary8(uint8_t bIn)  {

	for (unsigned int mask = 0b10000000; mask; mask >>= 1) {
		// check if this bit is set
		if (mask & bIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}

void printBinary12(uint16_t bIn)  {
	//                       B12345678   B12345678
	//for (unsigned int mask = 0x8000; mask; mask >>= 1) {
	for (unsigned int mask = 0b100000000000; mask; mask >>= 1) {
		// check if this bit is set
		if (mask & bIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}

void printBinary16(uint16_t wIn)  {
	for (unsigned int mask = 0b1000000000000000; mask; mask >>= 1) {
	// check if this bit is set
		if (mask & wIn) {
			Serial.print('1');
		}
		else {
			Serial.print('0');
		}
	}
}


void printArray(uint16_t *array, uint8_t bCount) {
	Serial.print(F(" "));
	uint8_t bIndex = 0;
	printuint8_tAlignRight(array[bIndex]);
	for(bIndex = 1; bIndex < bCount; bIndex++){
		Serial.print(F(", "));
		printuint8_tAlignRight(array[bIndex]);
	}
}

void printuint8_tAlignRight(uint8_t bValue) {
	//uint8_t bOffset = 0;
	if (bValue < 100) {
		if (bValue < 10) {
			//bOffset = 2;
			Serial.print(F("  "));
		} else {
			//bOffset = 1;
			Serial.print(F(" "));
		}
	}
	Serial.print(bValue);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu System
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Modes for Menu Switcher
const uint8_t cbMenuMode_Main	= 1;
const uint8_t cbMenuMode_Config	= 2;
uint8_t bMenuMode = cbMenuMode_Main;


// SubMenu Config
bool bUpdateEEPROMAllowed = false;
void handleMenu_Config(Print &pOut, char *caCommand) {
	// pOut.println(F("SubMenu Config:"));
	// pOut.println(F("\t nothing here."));
	// pOut.println(F("\t finished."));
	// exit submenu
	// reset state manschine of submenu
	// jump to main
	// bMenuMode = cbMenuMode_Main;
	// bMenu_Input_Flag_EOL = true;

	/**
		// Individual Hardware Addresses
		struct tDeviceHardware {
			uint16_t ID;
			uint8_t radio_Frequency;
		};

		// Device Configuration
		struct tDeviceConfig {
			uint8_t bBallID;
			uint8_t bNetworkID;
			uint8_t bMasterID;
			uint8_t bBallStartAddress;
			uint16_t uiFadeTime;
		};
	/**/

	// format: '_:65535'
	char *caValue = &caCommand[2];

	// default interpretation is 8bit and 16bit unsigned int.
	uint8_t  ui8ValueTemp  = atoi(caValue);
	uint16_t ui16ValueTemp = atoi(caValue);

	switch (caCommand[0]) {
		case '?': {
			// help:
			pOut.println(F("config your Ball:"));
			pOut.println(F("\t '?': this help"));
			pOut.println();
			pOut.println(F("\t 'I': print current values."));
			pOut.println(F("\t 'D': load default values."));
			pOut.println();
			pOut.println(F("\t 'i': Hardware ID        'i:255'"));
			pOut.println(F("\t 'b': bBallID            'b:255'"));
			pOut.println(F("\t 'n': bNetworkID         'n:255'"));
			pOut.println(F("\t 'm': bMasterID          'm:255'"));
			pOut.println(F("\t 'a': bBallStartAddress  'a:255'"));
			pOut.println(F("\t 'f': uiFadeTime         'f:65535'"));
			pOut.println();
			pOut.println(F("\t 'U': update EEPROM"));
			pOut.println();
		} break;
		case 'I': {
			Serial.println(F("\t current values: "));
			eeprom_DeviceHW_print(Serial, &dhwThisBall);
			eeprom_DeviceConfig_print(Serial, &dconfThisBall);
		} break;
		case 'D': {
			Serial.print(F("\t loading default values...."));
			// load default
			memcpy(&dconfThisBall, &dconfDefault, sizeof(tDeviceConfig));
			memcpy(&dhwThisBall, &dhwDefault, sizeof(tDeviceHardware));
			Serial.println(F("\t new values: "));
			eeprom_DeviceHW_print(Serial, &dhwThisBall);
			eeprom_DeviceConfig_print(Serial, &dconfThisBall);
		} break;
		//--------------------------------------------------------------------
		case 'i': {
			Serial.print(F("\t set Hardware ID: "));
			dhwThisBall.ID = ui8ValueTemp;
			Serial.println(dhwThisBall.ID);
		} break;
		case 'b': {
			Serial.print(F("\t set bBallID: "));
			dconfThisBall.bBallID = ui8ValueTemp;
			Serial.println(dconfThisBall.bBallID);
		} break;
		case 'n': {
			Serial.print(F("\t set bNetworkID: "));
			dconfThisBall.bNetworkID = ui8ValueTemp;
			Serial.println(dconfThisBall.bNetworkID);
		} break;
		case 'm': {
			Serial.print(F("\t set bMasterID: "));
			dconfThisBall.bMasterID = ui8ValueTemp;
			Serial.println(dconfThisBall.bMasterID);
		} break;
		case 'a': {
			Serial.print(F("\t set bBallStartAddress: "));
			dconfThisBall.bBallStartAddress = ui8ValueTemp;
			Serial.println(dconfThisBall.bBallStartAddress);
		} break;
		case 'f': {
			Serial.print(F("\t set uiFadeTime: "));
			dconfThisBall.uiFadeTime = ui16ValueTemp;
			Serial.println(dconfThisBall.uiFadeTime);
		} break;
		//--------------------------------------------------------------------
		case 'U': {
			Serial.println(F("\t do you really want to update the EEPROM?"));
			Serial.println(F("\t current values: "));
			eeprom_DeviceHW_print(Serial, &dhwThisBall);
			eeprom_DeviceConfig_print(Serial, &dconfThisBall);
			// allow update process
			bUpdateEEPROMAllowed = true;
			Serial.println(F("\t   than confirm this with '!:!'"));
		} break;
		case '!': {
			Serial.println(F("\t updating EEPROM:"));
			if (*caValue == '!') {
				// update EEPROM
				Serial.println(F("\t ----- updating DeviceHW:"));
				eeprom_DeviceHW_update(Serial, &dhwThisBall);
				Serial.println(F("\t ----- updating DeviceConfig:"));
				eeprom_DeviceConfig_update(Serial, &dconfThisBall);
				pOut.println(F("\t done."));
			} else {
				pOut.println(F("\t error"));
			}
			// reset update process
			bUpdateEEPROMAllowed = false;
			// jump to main
			bMenuMode = cbMenuMode_Main;
			bMenu_Input_Flag_EOL = true;
		} break;
		default: {
			// exit submenu
			// reset state manschine of submenu
			// reset update process
			bUpdateEEPROMAllowed = false;
			// jump to main
			bMenuMode = cbMenuMode_Main;
			bMenu_Input_Flag_EOL = true;
		}
	}// end switch

}


// Main Menu
void handleMenu_Main(Print &pOut, char *caCommand) {
	/* pOut.print("sCommand: '");
	pOut.print(sCommand);
	pOut.println("'"); */
	switch (caCommand[0]) {
		case 'h':
		case 'H':
		case '?': {
			// help
			pOut.println(F("____________________________________________________________"));
			pOut.println();
			pOut.println(F("Help for Commands:"));
			pOut.println();
			pOut.println(F("\t '?': this help"));
			pOut.println(F("\t 'i': sketch info"));
			pOut.println(F("\t 'y': toggle DebugOut livesign print"));
			pOut.println(F("\t 'Y': toggle DebugOut livesign LED"));
			pOut.println(F("\t 'x': tests"));
			pOut.println();
			pOut.println(F("\t 't': send runtime "));
			pOut.println(F("\t 'l': test led-driver steps"));
			pOut.println(F("\t 'L': test led-driver static"));
			// pOut.println(F("\t 'f': DemoFadeTo(ID, value) 'f1:65535'"));
			pOut.println();
			pOut.println(F("\t 'set:' enter SubMenu1"));
			pOut.println();
			pOut.println(F("____________________________________________________________"));
		} break;
		case 'i': {
			print_info(pOut);
			printRFM69Info(pOut);
		} break;
		case 'y': {
			pOut.println(F("\t toggle DebugOut livesign Serial:"));
			bDebugOut_LiveSign_Serial_Enabled = !bDebugOut_LiveSign_Serial_Enabled;
			pOut.print(F("\t bDebugOut_LiveSign_Serial_Enabled:"));
			pOut.println(bDebugOut_LiveSign_Serial_Enabled);
		} break;
		case 'Y': {
			pOut.println(F("\t toggle DebugOut livesign LED:"));
			bDebugOut_LiveSign_LED_Enabled = !bDebugOut_LiveSign_LED_Enabled;
			pOut.print(F("\t bDebugOut_LiveSign_LED_Enabled:"));
			pOut.println(bDebugOut_LiveSign_LED_Enabled);
		} break;
		case 'x': {
			// get state
			pOut.println(F("__________"));
			pOut.println(F("Tests:"));

			pOut.println(F("   "));
			printRFM69Info(pOut);

			pOut.println();

			pOut.println(F("__________"));
		} break;
		//--------------------------------------------------------------------------------
		case 't': {
			pOut.println(F("\t sendRunTime"));
			sendRunTime();
		} break;

		case 'l': {
			pOut.println(F("\t test LED-Driver"));
			ledboard_test_nextColor();
		} break;
		case 'L': {
			pOut.println(F("\t test LED-Driver static"));
			tlc.setLED(0, 200,   0,   0);
			tlc.setLED(1,   0, 200,   0);
			tlc.setLED(2,   0,   0, 200);
			tlc.setLED(3, 200, 200, 200);
			tlc.write();
		} break;

		//--------------------------------------------------------------------------------
		case 's': {
			// SubMenu1
			if ( (caCommand[1] == 'e') && (caCommand[2] == 't') && (caCommand[3] == ':') ) {
				//if full command is 'set:' enter submenu
				bMenuMode = cbMenuMode_Config;
				if ( caCommand[4] != '\0' ) {
					//full length command
				} else {
					// display help
					sMenu_Input_New[0] = '?';
					bMenu_Input_Flag_EOL = true;
				}
			}
		} break;
		//--------------------------------------------------------------------------------
		default: {
			pOut.print(F("command '"));
			pOut.print(caCommand);
			pOut.println(F("' not recognized. try again."));
			sMenu_Input_New[0] = '?';
			bMenu_Input_Flag_EOL = true;
		}
	} //end switch

	//end Command Parser
}


// Menu Switcher
void menuSwitcher(Print &pOut, char *caCommand) {
	switch (bMenuMode) {
			case cbMenuMode_Main: {
				handleMenu_Main(pOut, caCommand);
			} break;
			case cbMenuMode_Config: {
				handleMenu_Config(pOut, caCommand);
			} break;
			default: {
				// something went wronge - so reset and show MainMenu
				bMenuMode = cbMenuMode_Main;
			}
		} // end switch bMenuMode
}

// Check for NewLineComplete and enter menuSwitcher
// sets Menu Output channel (pOut)
void check_NewLineComplete() {
	// complete line found:
		if (bMenu_Input_Flag_EOL) {
			// Serial.println(F("check_NewLineComplete"));
			// Serial.println(F("  bMenu_Input_Flag_EOL is set. "));
			// Serial.print  (F("    sMenu_Input_New: '"));
			// Serial.print(sMenu_Input_New);
			// Serial.println(F("'"));

			// Serial.println(F("  Flags:"));
			// Serial.print  (F("    bMenu_Input_Flag_BF: '"));
			// Serial.println(bMenu_Input_Flag_BF);
			// Serial.print  (F("    bMenu_Input_Flag_CR: '"));
			// Serial.println(bMenu_Input_Flag_CR);
			// Serial.print  (F("    bMenu_Input_Flag_LF: '"));
			// Serial.println(bMenu_Input_Flag_LF);
			// Serial.print  (F("    bMenu_Input_Flag_EOL: '"));
			// Serial.println(bMenu_Input_Flag_EOL);


			// Serial.println(F("  copy sMenu_Input_New to sMenu_Command_Current."));
			// copy to current buffer
			strcpy(sMenu_Command_Current, sMenu_Input_New);

			// Serial.println(F("  clear sMenu_Input_New"));
			// reset memory
			memset(sMenu_Input_New,'\0',sizeof(sMenu_Input_New)-1);

			// Serial.println(F("  clear bMenu_Input_Flag_EOL"));
			// reset flag
			bMenu_Input_Flag_EOL = false;
			bMenu_Input_Flag_LF = false;

			// print info if things were skipped.
			if (bMenu_Input_Flag_BF) {
				Serial.println(F("input was to long. used first part - skipped rest."));
				bMenu_Input_Flag_BF = false;
			}

			// parse line / run command
			menuSwitcher(Serial, sMenu_Command_Current);


			// Serial.print  (F("    sMenu_Input_New: '"));
			// Serial.print(sMenu_Input_New);
			// Serial.println(F("'"));
			// Serial.print  (F("    sMenu_Command_Current: '"));
			// Serial.print(sMenu_Command_Current);
			// Serial.println(F("'"));


			// Serial.println(F("  check bMenu_Input_Flag_SkipRest"));
			// if ( !bMenu_Input_Flag_SkipRest) {
				// Serial.println(F("   parse Line:"));

				// if (bMenu_Input_Flag_BF) {
					// Serial.println(F("input was to long. used first part - skipped rest."));
					// bMenu_Input_Flag_BF = false;
				// }

				// parse line / run command
				// menuSwitcher(Serial, sMenu_Command_Current);

				// if(bMenu_Input_Flag_LongLine) {
					// bMenu_Input_Flag_SkipRest = true;
					// bMenu_Input_Flag_LongLine = false;
				// }
			// } else {
				// Serial.println(F("   skip rest of Line"));
				// bMenu_Input_Flag_SkipRest = false;
			// }

		}// if Flag complete
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Serial Receive Handling
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void handle_SerialReceive() {
	// collect next input text
	while ((!bMenu_Input_Flag_EOL) && (Serial.available())) {
	// while (Serial.available()) {
		// get the new uint8_t:
		char charNew = (char)Serial.read();
		/*Serial.print(F("charNew '"));
		Serial.print(charNew);
		Serial.print(F("' : "));
		Serial.println(charNew, DEC);*/

		// collect next full line
		/* http://forums.codeguru.com/showthread.php?253826-C-String-What-is-the-difference-between-n-and-r-n
			'\n' == 10 == LineFeed == LF
			'\r' == 13 == Carriage Return == CR
			Windows: '\r\n'
			Linux: '\n'
			Apple: '\r'
		*/
		// check for line end
		switch (charNew) {
			case '\r': {
				// Serial.println(F("incoming char is \\r: set EOL"));
				bMenu_Input_Flag_EOL = true;
				bMenu_Input_Flag_CR = true;
				// bMenu_Input_Flag_LF = false;
			} break;
			case '\n': {
				// Serial.println(F("incoming char is \\n: set EOL"));
				// Serial.println(F("  Flags:"));
					// Serial.print  (F("    bMenu_Input_Flag_BF: '"));
					// Serial.println(bMenu_Input_Flag_BF);
					// Serial.print  (F("    bMenu_Input_Flag_CR: '"));
					// Serial.println(bMenu_Input_Flag_CR);
					// Serial.print  (F("    bMenu_Input_Flag_LF: '"));
					// Serial.println(bMenu_Input_Flag_LF);
					// Serial.print  (F("    bMenu_Input_Flag_EOL: '"));
					// Serial.println(bMenu_Input_Flag_EOL);



				bMenu_Input_Flag_LF = true;

				// Serial.println(F("  check for CR"));
				// check if last char was not CR - if this is true set EOL - else ignore.
				if(!bMenu_Input_Flag_CR) {
					bMenu_Input_Flag_EOL = true;
				} else {
					bMenu_Input_Flag_CR = false;
				}

				// Serial.println(F("  Flags:"));
					// Serial.print  (F("    bMenu_Input_Flag_BF: '"));
					// Serial.println(bMenu_Input_Flag_BF);
					// Serial.print  (F("    bMenu_Input_Flag_CR: '"));
					// Serial.println(bMenu_Input_Flag_CR);
					// Serial.print  (F("    bMenu_Input_Flag_LF: '"));
					// Serial.println(bMenu_Input_Flag_LF);
					// Serial.print  (F("    bMenu_Input_Flag_EOL: '"));
					// Serial.println(bMenu_Input_Flag_EOL);


				// this check also works for windows double line ending
				//if (strlen(sMenu_Input_New) > 0) {
					// bMenu_Input_Flag_EOL = true;
				// }
			} break;
			default: {
				// normal char -
				// add it to the sMenu_Input_New:
				//check for length
				if (strlen(sMenu_Input_New) < (sizeof(sMenu_Input_New)-1) ) {
					sMenu_Input_New[strlen(sMenu_Input_New)] = charNew;
				} else {
					//Serial.println(F(" line to long! ignoring rest of line"));
					// set complete flag so line will be parsed
					// Serial.println(F("Buffer is full: set EOL; set LongLine"));
					//bMenu_Input_Flag_EOL = true;
					bMenu_Input_Flag_BF = true;
					// skip rest of line
					bMenu_Input_Flag_LongLine = true;
				}
			}// default
		}// switch charNew

		//check_NewLineComplete();
	}
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EEPROMEx functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/* print Helper
void print_HEX(Print &pOut, uint8_t bValue) {
	if( bValue < 0x10) {
		pOut.print("0");
	}
	pOut.print(bValue,HEX);
}

void print_MAC(Print &pOut, uint8_t *array) {
	//pOut.print(F(" "));
	uint8_t bIndex = 0;
	print_HEX(pOut, array[bIndex]);
	for(bIndex = 1; bIndex < 6; bIndex++){
		pOut.print(F(", "));
		print_HEX(pOut, array[bIndex]);
	}
}


uint8_t print_AlignRight_uint16_t(Print &pOut, uint16_t wValue) {
	uint8_t bLeadingZeros = 0;
	if (wValue < 10000) {
		bLeadingZeros = bLeadingZeros + 1;
		pOut.print(F(" "));
		if (wValue < 1000) {
			bLeadingZeros = bLeadingZeros + 1;
			Serial.print(F(" "));
			if (wValue < 100) {
				bLeadingZeros = bLeadingZeros + 1;
				Serial.print(F(" "));
				if (wValue < 10) {
					bLeadingZeros = bLeadingZeros + 1;
					Serial.print(F(" "));
				}
			}
		}
	}
	pOut.print(wValue);
	return bLeadingZeros;
}

*/

void eeprom_DeviceHW_print(Print &pOut, tDeviceHardware *dhwTemp){
	/* *
	// Individual Hardware Addresses
	struct tDeviceHardware {
		uint16_t ID;
	};
	* */
	pOut.println(F("\t deviceHW values:"));
	pOut.print  (F("\t   ID: "));
	pOut.print  ((*dhwTemp).ID);
	pOut.println();
	pOut.print  (F("\t   radio_Frequency: "));
	printRFM69Frequence(pOut, (*dhwTemp).radio_Frequency);
	pOut.println();
}

// return:  >0 = success
uint8_t eeprom_DeviceHW_read(tDeviceHardware *dhwNew){
	// read EEPROM and update dpThisDevice.
	uint8_t bResultFlag = 0;
	Serial.println(F("reading DeviceHW values from EEPROM:"));
	Serial.print(F("\t eeprom_Address_DeviceHardware:"));
	Serial.println(eeprom_Address_DeviceHardware);
	Serial.print(F("\t sizeof(tDeviceHardware):"));
	Serial.print(sizeof(tDeviceHardware));
	Serial.println(F(" uint8_ts"));
	tDeviceHardware dhwTemp;
	uint8_t bReaduint8_t = 0;
	bReaduint8_t = EEPROM.readBlock(eeprom_Address_DeviceHardware, dhwTemp);
	if ( bReaduint8_t == sizeof(tDeviceHardware) ) {
		bResultFlag = 1;
		Serial.print(F("\t   read "));
		Serial.print(bReaduint8_t);
		Serial.println(F(" uint8_ts"));
		// copy data to local struct.
		// (*dhwNew).ID = dhwTemp.ID;
		memcpy(dhwNew, &dhwTemp, sizeof(tDeviceHardware));
	} else {
		// something went wrong.
			bResultFlag = 0;
		Serial.println(F("\t there occurred an error while reading EEPROM."));
		Serial.print(F("\t   read "));
		Serial.print(bReaduint8_t);
		Serial.println(F(" uint8_ts"));
	}
	return bResultFlag;
};

// return:  >0 = success
uint8_t eeprom_DeviceHW_update(Print &pOut, tDeviceHardware *dhwNew){
	uint8_t bResultFlag = 0;
	// update EEPROM with dhwNew.
	pOut.println(F("update Device Info in EEPROM:"));
	//pOut.print(F("  free RAM = "));
	//pOut.println(freeRam());
	pOut.print(F("\t eeprom_Address_DeviceHardware: "));
	pOut.println(eeprom_Address_DeviceHardware);
	pOut.print(F("\t sizeof(tDeviceHardware): "));
	pOut.print(sizeof(tDeviceHardware));
	pOut.println(F(" uint8_ts"));
	uint8_t bWrittenuint8_t = 0;
	bWrittenuint8_t = EEPROM.updateBlock(eeprom_Address_DeviceHardware, (*dhwNew));
	//pOut.print(F("  free RAM = "));
	//pOut.println(freeRam());
	if ( bWrittenuint8_t > 0 ) {
		pOut.print(F("\t   updated "));
		pOut.print(bWrittenuint8_t);
		pOut.println(F(" uint8_ts"));
		//pOut.println(F(" is it a timing thing?"));
		pOut.println(F("\t succesfull updated values."));
	} else {
		pOut.println(F("\t nothing to update. values are equal."));
	}
	return bResultFlag;
};



void eeprom_DeviceConfig_print(Print &pOut, tDeviceConfig *dconfTemp){
	/*
	// Device Configuration
	struct tDeviceConfig {
		uint8_t bBallID;
		uint8_t bNetworkID;
		uint8_t bMasterID;
		uint8_t bBallStartAddress;
		uint16_t uiFadeTime;
	};
	*/

	pOut.println(F("\t tDeviceConfig:"));
	pOut.print  (F("\t   bBallID : "));
	pOut.print((*dconfTemp).bBallID);
	pOut.println();
	pOut.print  (F("\t   bNetworkID : "));
	pOut.print((*dconfTemp).bNetworkID);
	pOut.println();
	pOut.print  (F("\t   bMasterID : "));
	pOut.print((*dconfTemp).bMasterID);
	pOut.println();
	pOut.print  (F("\t   bBallStartAddress : "));
	pOut.print((*dconfTemp).bBallStartAddress);
	pOut.println();
	pOut.print  (F("\t   uiFadeTime : "));
	pOut.print((*dconfTemp).uiFadeTime);
	pOut.println();

}

// return:  >0 = success
uint8_t eeprom_DeviceConfig_read(tDeviceConfig *dconfNew){
	// read EEPROM and update dpThisDevice.
	uint8_t bResultFlag = 0;
	Serial.println(F("reading DeviceConf values from EEPROM:"));
	Serial.print(F("\t eeprom_Address_DeviceConfig:"));
	Serial.println(eeprom_Address_DeviceConfig);
	Serial.print(F("\t sizeof(tDeviceConfig):"));
	Serial.print(sizeof(tDeviceConfig));
	Serial.println(F(" uint8_ts"));
	tDeviceConfig dconfTemp;
	uint8_t bReaduint8_t = 0;
	bReaduint8_t = EEPROM.readBlock(eeprom_Address_DeviceConfig, dconfTemp);
	if ( bReaduint8_t == sizeof(tDeviceConfig) ) {
		Serial.print(F("\t   read "));
		Serial.print(bReaduint8_t);
		Serial.println(F(" uint8_ts"));
		//copy data to local struct.
		// --> is it possible to use memcpy ??
		// (*dconfNew).uiFadeTime = dconfTemp.uiFadeTime;
		memcpy(dconfNew, &dconfTemp, sizeof(tDeviceConfig));
		bResultFlag = 1;
	} else {
		// something went wrong.
		Serial.println(F("\t there occurred an error while reading EEPROM."));
		Serial.print(F("\t   read "));
		Serial.print(bReaduint8_t);
		Serial.println(F(" uint8_ts"));
		bResultFlag = 0;
	}
	return bResultFlag;
};

// return:  >0 = success
uint8_t eeprom_DeviceConfig_update(Print &pOut, tDeviceConfig *dconfNew){
	// update EEPROM with dconfNew.
	uint8_t bResultFlag = 0;
	pOut.println(F("update Device Info in EEPROM:"));
	pOut.print(F("\t eeprom_Address_DeviceConfig: "));
	pOut.println(eeprom_Address_DeviceConfig);
	pOut.print(F("\t sizeof(tDeviceConfig): "));
	pOut.print(sizeof(tDeviceConfig));
	pOut.println(F(" uint8_ts"));
	uint8_t bWritenuint8_t = 0;
	bWritenuint8_t = EEPROM.updateBlock(eeprom_Address_DeviceConfig, (*dconfNew));
	if ( bWritenuint8_t > 0 ) {
		pOut.print(F("\t   updated "));
		pOut.print(bWritenuint8_t);
		pOut.println(F(" uint8_ts"));
		pOut.println(F("\t successful updated values."));
		bResultFlag = 2;
	} else {
		pOut.println(F("\t nothing to update. values are equal."));
		bResultFlag = 1;
	}
	return bResultFlag;
};



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// RFM69
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void printRFM69Frequence(Print &pOut, const uint8_t cbFrequence) {
	switch(cbFrequence) {
		case RF69_433MHZ: {
			pOut.print(F("433MHz"));
		} break;
		case RF69_868MHZ: {
			pOut.print(F("868MHz"));
		} break;
		case RF69_915MHZ: {
			pOut.print(F("915MHz"));
		} break;
	}
}

void printRFM69Info(Print &pOut) {
	// get local things
	tDeviceConfig *dconfThis = &dconfThisBall;
	tDeviceHardware *dhwThis = &dhwThisBall;

	pOut.println(F("RFM69 Info:"));
	pOut.print(F("\t radio_Frequency: "));
	printRFM69Frequence(pOut, (*dhwThis).radio_Frequency);
	pOut.println();
	pOut.print(F("\t NetworkID: "));
	pOut.println((*dconfThis).bNetworkID);
	pOut.print(F("\t BallID: "));
	pOut.println((*dconfThis).bBallID);
	pOut.print(F("\t MasterID: "));
	pOut.println((*dconfThis).bMasterID);

	pOut.println(F("\t sw fixed values: "));
	pOut.print(F("\t   radio_ACKTime: "));
	pOut.println(radio_ACKTime);
	pOut.print(F("\t   radio_PromiscuosMode: "));
	pOut.println(radio_PromiscuosMode);
	pOut.print(F("\t   radio_KEY: "));
	pOut.println(radio_KEY);
}


void handle_RFM69Receive() {
	if ( radio.receiveDone() ) {
		// new packet received

		// oh boy! We've got data!!! Let's save it to my own buffer!
		// https://lowpowerlab.com/forum/index.php/topic,776.msg4990.html#msg4990
		char tempData[RF69_MAX_DATA_LEN];
		// uint8_t tempData[RF69_MAX_DATA_LEN];
		uint8_t datalen = radio.DATALEN;
		uint8_t sender = radio.SENDERID;
		uint8_t target = radio.TARGETID;
		int16_t rssi = radio.RSSI;
		memcpy(tempData, (const void *)radio.DATA, datalen);

		bool bACKreq = radio.ACKRequested();

		// first send ACK.
		if (bACKreq) {
			radio.sendACK();
		}

		// now output data:
		Serial.print("received from '");
		Serial.print(sender, DEC);
		Serial.print("' [RX_RSSI:");
		Serial.print(rssi);
		Serial.print("] : '");
		for (uint8_t i = 0; i < datalen; i++) {
			Serial.print( (char)tempData[i] );
		}
		Serial.print("' ");

		if (bACKreq) {
			Serial.print("--> ACK sent");
		}

		// check for color command:
		if (tempData[0] == 'c') {
			// set color
			parseColor(Serial, tempData);
		}

		Serial.println();
	}
}

void sendRunTime() {
	Serial.println("sendRunTime:");
	char tempData[] = "Hi i am running 000000000000000000ms";
	sprintf(tempData, "Hi i am running %0u ms", millis());

	Serial.print("\t tempData: ");
	Serial.println(tempData);

	Serial.print("\t target: ");
	Serial.println(dconfThisBall.bMasterID);


	Serial.print("\t sendWithRetry: ");
	if (  radio.sendWithRetry(dconfThisBall.bMasterID, tempData, strlen(tempData) )   ) {
		Serial.print(" ok!");
	} else {
		Serial.print(" nothing...");
	}

	Serial.println();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// remote
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void parseColor(Print &pOut, char *caCommand) {
	pOut.println(F("parsing Color:"));

	pOut.print(F("\t   caCommand: '"));
	pOut.print(caCommand);
	pOut.println(F("'"));

	// color:255,255,255
	char *caTempPos = &caCommand[1];

	// find ':'
	// init
	caTempPos = strtok(caTempPos, ": ");
	// get first position
	caTempPos = strtok(NULL, ": ");

	uint8_t temp_red = atoi(caTempPos);

	// find ','
	// init
	caTempPos = strtok(caTempPos, ", ");
	// get first position
	caTempPos = strtok(NULL, ", ");
	uint8_t temp_green = atoi(caTempPos);

	// get second position
	caTempPos = strtok(NULL, ", ");
	uint8_t temp_blue = atoi(caTempPos);

	uint16_t iColor[] = {
		temp_red*255,
		temp_green*255,
		temp_blue*255
	};
	ledboard_fadeToColor(iColor);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Board
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void ledboard_fadeToColor(uint16_t *iColor){
	const uint8_t iChannelCount = myFader.getChannelCount();
	uint16_t waTemp[iChannelCount];
	for (uint16_t uiIndex = 0; uiIndex < iChannelCount; uiIndex = uiIndex + 3) {
		waTemp[uiIndex+0] = iColor[0];
		waTemp[uiIndex+1] = iColor[1];
		waTemp[uiIndex+2] = iColor[2];
	}
	myFader.startFadeTo(200, waTemp);
}

void ledboard_test_nextColor() {
	Print &pOut = Serial;
	pOut.print(F("\t uiLEDDriverTestStep:"));
	pOut.println(uiLEDDriverTestStep);

	//  switch IR LED OFF
	digitalWrite(pinIRLED, LOW);

	uint16_t iColor[] = {0,0,0};

	switch (uiLEDDriverTestStep) {
		case 0: {
			pOut.println(F("\t    color: test1"));
			iColor[0] =   100;
			iColor[1] =     0;
			iColor[2] =     0;
		} break;
		case 1: {
			pOut.println(F("\t    color: test2"));
			iColor[0] =     0;
			iColor[1] =   100;
			iColor[2] =     0;
		} break;
		case 2: {
			pOut.println(F("\t    color: test3"));
			iColor[0] =     0;
			iColor[1] =     0;
			iColor[2] =   100;
		} break;
		case 3: {
			pOut.println(F("\t    color: IR-LED"));
			iColor[0] =     1;
			iColor[1] =     1;
			iColor[2] =     1;
			digitalWrite(pinIRLED, HIGH);
		} break;
		case 4: {
			pOut.println(F("\t    color: blueish"));
			iColor[0] =     0;
			iColor[1] =  5000;
			iColor[2] = 20000;
		} break;
		case 5: {
			pOut.println(F("\t    color: sun"));
			iColor[0] = 20000;
			iColor[1] =  5000;
			iColor[2] =     0;
		} break;
		case 6: {
			pOut.println(F("\t    color: dark lounge"));
			iColor[0] =   100;
			iColor[1] =     0;
			iColor[2] =   100;
		} break;


		default: {
			pOut.println(F("\t    color: OFF"));
			iColor[0] = 0;
			iColor[1] = 0;
			iColor[2] = 0;
		}
	} //end switch


	ledboard_fadeToColor(iColor);

	uiLEDDriverTestStep = uiLEDDriverTestStep +1 ;
	if (uiLEDDriverTestStep > 8) {
		uiLEDDriverTestStep = 0;
	}
}




uint16_t sequencer_CurrentStep = 0;

uint16_t sequencer_StepData[][3] = {
	{ // Off
		  0,   0,   0,
	},
	{ // White 100%
		255, 255, 255,
	},
	{ // White 50%
		127, 127, 127,
	},
	{ // White 10%
		 25,  25,  25,
	},
	{ // Red
		255,   0,   0,
	},
	{ // Green
		  0, 255,   0,
	},
	{ // Blue
		  0,   0, 255,
	},
	// { // Welcome
		  // 0,   0, 255,
	// },
};

const uint8_t sequencer_StepCount = sizeof(sequencer_StepData) / 3;

void sequencer_NextStep() {
	sequencer_CurrentStep = sequencer_CurrentStep +1;
	if ( sequencer_CurrentStep >= sequencer_StepCount) {
		sequencer_CurrentStep = 0;
	}
	const uint8_t iChannelCount = myFader.getChannelCount();
	uint16_t waTemp[iChannelCount];
	for (uint16_t uiIndex = 0; uiIndex < iChannelCount; uiIndex = uiIndex + 3) {
		waTemp[uiIndex+0] = sequencer_StepData[sequencer_CurrentStep][0];
		waTemp[uiIndex+1] = sequencer_StepData[sequencer_CurrentStep][1];
		waTemp[uiIndex+2] = sequencer_StepData[sequencer_CurrentStep][2];
	}
	myFader.startFadeTo(500, waTemp);
}



void setLEDs_blue_front() {
	uint16_t temp[myFader.getChannelCount()] = {
    //  r,     g,     b
		    0,     0, 65535,
		    0,     0,     0,
		    0,     0,     0,
		    0,     0, 65535,
	};
	myFader.startFadeTo(500, temp);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// input handler
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// slight_ButtonInput things
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

boolean myInput_callback_GetInput(uint8_t bID, uint8_t bPin) {
	// read input invert reading - button closes to GND.
	// check HWB
	// return ! (PINE & B00000100);
	return ! digitalRead(bPin);
}


void myCallback_onEvent(slight_ButtonInput *pInstance, uint8_t bEvent) {

	// Serial.print(F("Instance ID:"));
	// Serial.println((*pInstance).getID());

	// Serial.print(F("Event: "));
	// (*pInstance).printEvent(Serial, bEvent);
	// Serial.println();

	// show event additional infos:
	switch (bEvent) {
		/*case slight_ButtonInput::event_StateChanged : {
			Serial.print(F("\t state: "));
			(*pInstance).printState(Serial);
			Serial.println();
		} break;
		// click
		/*case slight_ButtonInput::event_Down : {
			Serial.print(F("the button is pressed down! do something.."));
		} break;*/
		// case slight_ButtonInput::event_HoldingDown : {
			// Serial.print(F("duration active: "));
			// Serial.println((*pInstance).getDurationActive());
		// } break;
		/*case slight_ButtonInput::event_Up : {
			Serial.print(F("up"));
		} break;
		*/
		case slight_ButtonInput::event_Click : {
			Serial.print(F("click"));
			ledboard_test_nextColor();
		} break;
		// case slight_ButtonInput::event_ClickLong : {
			// Serial.print(F("click long"));
		// } break;
		// case slight_ButtonInput::event_ClickDouble : {
			// Serial.print(F("click double"));
		// } break;
		// case slight_ButtonInput::event_ClickTriple : {
			// Serial.print(F("click triple"));
		// } break;
		// case slight_ButtonInput::event_ClickMulti : {
			// Serial.print(F("click count: "));
			// Serial.println((*pInstance).getClickCount());
		// } break;
	} //end switch

}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// my Fader things
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void myFader_callback_OutputChanged(uint8_t bID, uint16_t *wValues, uint8_t bCount) {

	for (uint8_t i = 0; i<4; i++) {
		uint8_t indexValue = i*3;
		tlc.setLED(i, wValues[indexValue+0], wValues[indexValue+1], wValues[indexValue+2]);
	}
	tlc.write();
}

void myFader_callback_onEvent(slight_FaderLin *pInstance, uint8_t bEvent) {

	// Serial.print(F("Instance ID:"));
	// Serial.println((*pInstance).getID());

	// Serial.print(F("Event: "));
	// (*pInstance).printEvent(Serial, bEvent);
	// Serial.println();

	// react on events:
	switch (bEvent) {
		case slight_FaderLin::event_StateChanged : {
			// Serial.print(F("myMotorFader"));
			// Serial.print((*pInstance).getID());
			// Serial.println(F(" : "));
			// Serial.print(F("\t state: "));
			// (*pInstance).printState(Serial);
			// Serial.println();

			/* *
			switch (bState) {
				case slight_FaderLin::state_Standby : {
						//
					} break;
				case slight_FaderLin::state_Fading : {
						//
					} break;
				case slight_FaderLin::state_Finished : {
						//
					} break;
			} //end switch
			/* */

		} break;

		case slight_FaderLin::event_fading_Finished : {
			// Serial.print(F("\t fading Finished."));
			// effectSystem_bMode = effectSys_Standby;
		} break;
	} //end switch

}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// some things
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialise PINs
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//LiveSign
		pinMode(cbID_LED_Info, OUTPUT);
		digitalWrite(cbID_LED_Info, HIGH);

		// as of arduino 1.0.1 you can use INPUT_PULLUP

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// init serial
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		// for ATmega32U4 devices:
		#if defined (__AVR_ATmega32U4__)
			//wait for arduino IDE to release all serial ports after upload.
			delay(2000);
		#endif

		Serial.begin(115200);

		// for ATmega32U4 devices:
		#if defined (__AVR_ATmega32U4__)
			// Wait for Serial Connection to be Opend from Host or 6second timeout
			unsigned long ulTimeStamp_Start = millis();
			while( (! Serial) && ( (millis() - ulTimeStamp_Start) < 6000 ) ) {
				1;
			}
		#endif

		Serial.println();

		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Welcom
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		print_info(Serial);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// read device identity
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		Serial.println(F("Device Identity:")); {
			bool bEEPROM_Fine =  eeprom_DeviceHW_read(&dhwThisBall);
			if ( !bEEPROM_Fine ) {
				Serial.println(F("\t EEPROM-HW information are not present. pleas set."));
				//bMenuMode = cbMenuMode_HWID;
			} else {
				eeprom_DeviceHW_print(Serial, &dhwThisBall);
			}

			eeprom_DeviceConfig_read(&dconfThisBall);
			eeprom_DeviceConfig_print(Serial, &dconfThisBall);
		}
		Serial.println(F("\t finished."));

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// setup RFM69
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());

		// print all global parameters
		printRFM69Info(Serial);
		Serial.println(F("setup RFM69:")); {

			Serial.println(F("\t --> initialize"));
			radio.initialize(
				dhwThisBall.radio_Frequency,
				dconfThisBall.bBallID,
				dconfThisBall.bNetworkID);

			Serial.println(F("\t --> set HighPower"));
			//uncomment only for RFM69HW!
			radio.setHighPower();

			Serial.println(F("\t --> encrypt"));
			radio.encrypt(radio_KEY);

			// Serial.println(F("\t --> set Frequency:"));
			// set frequency to some custom frequency
			// radio.setFrequency(919000000);

		}
		Serial.println(F("\t --> finished."));

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// setup LED Board
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());

		Serial.println(F("setup LED Board:")); {

			Serial.println(F("\t init Adafruit_TLC59711"));
			tlc.begin();
			tlc.write();

			Serial.println(F("\t setup IR-LED"));
			pinMode(pinIRLED, OUTPUT);
			digitalWrite(pinIRLED, LOW);
		}
		Serial.println(F("\t finished."));


	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// start my Fader
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());

		Serial.println(F("slight_FaderLin:")); {
			Serial.println(F("\t myFader.begin();"));
			myFader.begin();
		}
		Serial.println(F("\t finished."));

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// start slight_ButtonInput
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		Serial.print(F("# Free RAM = "));
		Serial.println(freeRam());

		Serial.println(F("slight_ButtonInput:"));
		{
			Serial.println(F("\t pinMode INPUT_PULLUP"));
			// pinMode(myButtonSwitchColor.getPin(), INPUT_PULLUP);
			pinMode(myButtonSwitchColor.getPin(), INPUT);
			digitalWrite(myButtonSwitchColor.getPin(), HIGH);

			Serial.println(F("\t myButtonSwitchColor.begin();"));
			myButtonSwitchColor.begin();

		}
		Serial.println(F("\t finished."));



	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// show Serial Commands
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		// reset Serial Debug Input
		memset(sMenu_Input_New, '\0', sizeof(sMenu_Input_New)-1);
		//print Serial Options
		sMenu_Input_New[0] = '?';
		bMenu_Input_Flag_EOL = true;


	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// start default mode
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	setLEDs_blue_front();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// GO
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		Serial.println(F("Loop:"));

} // setup


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main Loop
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Menu Input
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// Serial
		handle_SerialReceive();
		check_NewLineComplete();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// RFM69
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		handle_RFM69Receive();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// my fader
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		myFader.update();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// my Button
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		myButtonSwitchColor.update();

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Timed things
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		/*
		// every XXXXms
		if ( ( millis() - ulTimeStamp_LastAction ) > cwUpdateInterval) {
			ulTimeStamp_LastAction =  millis();
			//
		}
		*/


	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Debug Out
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		if ( (millis() - ulDebugOut_LiveSign_TimeStamp_LastAction) > cwDebugOut_LiveSign_UpdateInterval) {
			ulDebugOut_LiveSign_TimeStamp_LastAction = millis();

			if ( bDebugOut_LiveSign_Serial_Enabled ) {
				Serial.print(millis());
				Serial.print(F("ms;"));
				Serial.print(F("  free RAM = "));
				Serial.println(freeRam());
			}

			if ( bDebugOut_LiveSign_LED_Enabled ) {
				bLEDState = ! bLEDState;
				if (bLEDState) {
					//set LED to HIGH
					digitalWrite(cbID_LED_Info, HIGH);
				} else {
					//set LED to LOW
					digitalWrite(cbID_LED_Info, LOW);
				}
			}

		}

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // loop


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
