/****************************************************************************
	Copyright (C) 2010 Stefan Bormann, Olaf Funke, Heiko Herholz, Martin Pischky

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************

	IMPORTANT:

	Note: The sale any LocoNet device hardware (including bare PCB's) that
	uses this or any other LocoNet software, requires testing and certification
	by Digitrax Inc. and will be subject to a licensing agreement.

	Please contact Digitrax Inc. for details.

*****************************************************************************

	Title :   Main module of "Dispa" project.
	Author:   Heiko Herholz <heiko@modellbahnfrokler.de>
	Date:     04-September-2010
	Software: AVR-GCC
	Target:   AtMega168 (Prototype on "Watti", newer Hardware intended)

	DESCRIPTION
	This module implements the main loop, lcd access and loconet handling.
	"Dispa" is intended for setting the loco address and mode that is
	controlled by a simple loconet throttle as FRED, FREDI via the "dispatch"
	protocol of loconet.

*****************************************************************************

  Adapted:  2019-10-27 by Michael Zimmermann
            - adapted and ported for use on Arduino UNO or compatible
            - with OLED (or LCD)
            - and display ThrottleID on OLED
            - DEBUG-support added
            2023-11-17 V2.2 by Michael Zimmermann
            - added functionality for read and show of most interesting FrediSVs
            2024-08-11 V2.3 by Michael Zimmermann
            - added functionality for read and show of SV8...10, FREDI-Test added
            2024-08-29 V2.4 by Michael Zimmermann
            - added functionality for QRCode-Reader
  
used I²C-Addresses:
  - 0x78  OLED-Panel ggf. mit
  - 0x23  Button am OLED-Panel
  
  - 0x39  Keypad 4x4
  
discrete In/Outs used for functionalities:
  -  0     (used  by USB)
  -  1     (used  by USB)
  -  2
  -  3 Out LCD-Panel D7
  -  4 Out LCD-Panel D6
  -  5 Out LCD-Panel D5
  -  6 Out used   by HeartBeat
  -  7 Out used   by LocoNet [TxD]
  -  8 In  used   by LocoNet [RxD]
  -  9 Out LCD-Panel D4
  - 10 In  RxD (SofwareSerial, TxD von QR-Code-Scanner)
  - 11 Out LCD-Panel Enable 
  - 12 Out LCD-Panel R/W
  - 13 Out TxD (SofwareSerial, RxD von QR-Code-Scanner)
  - 14 In  Button (QR-Code-Scanner)
  - 15
  - 16 Out LED (QR-Code-Scanner)
  - 17
  - 18     (used by I²C: SDA)
  - 19     (used by I²C: SCL)

*****************************************************************************/

//#define DEBUG

//#define LCD
#define OLED

#if defined LCD && defined OLED
  #error LCD and OLED defined
#endif
#if !defined OLED && !defined LCD
    #error Neither LCD nor OLED defined
#endif
    
// keep following defines - unless there are problems with them:
#define FREDI_SV
#define QRCODE
#define SEND_QRCODE_DATA

#include <HeartBeat.h>
HeartBeat oHeartbeat;

//========================================================
#define SW_VERSION F("2.6")
#define SW_YEAR    F("2024")

#include <LocoNet.h>  // used to include ln_opc.h
#include "LocoNetFrediSV.h"

#if not defined BUTTON_SELECT
  #define BUTTON_SELECT 0x01
  #define BUTTON_KEYPAD 0x80
  #define BUTTON_STAR   0x40
#endif

enum SPECIAL_BUTTON { NONE = 66, HASH = 98, STAR = 99 };

typedef struct
{
	unsigned char ucSTAT;   // slot status
	unsigned char ucADR;    // least significant 7 bits or short address
	unsigned char ucSPD;    // Speed
	unsigned char ucDIRF;   // direction & lower functions
	unsigned char ucSS2;    // slot status 2
	unsigned char ucADR2;   // most significant 7 bit or zero for short address
	unsigned char ucSND;    // the good old "upper" function group
	unsigned char ucID1;
	unsigned char ucID2;
} SSlotData_t;

const uint8_t DEC_STEPS(1);
const uint8_t SKIP_SELF_TEST(0x55);
const uint8_t EMERGENCY_STOP(0x01);
const uint8_t SLOTMAX(28);
static SSlotData_t SlotTabelle[SLOTMAX];
static uint8_t trk(0x05);
uint16_t ui16_ThrottleId(0);
uint16_t zahl(0);
uint8_t stelle(0);
uint8_t fahrstufen(DEC_STEPS);
uint8_t iyLineOffset(0);

// 0x01 = E5 can be send
// 0x02 = E5 sended, waits for answer
// 0x03 = E5 answer is ok
// 0x04 = E5 sends function states
// 0x80 = E5 answer is faulty
uint8_t ui8FlagSendingDisptach(0);
bool bFREDITestActive(false);
bool bReadFromQRCode(false);
boolean bShowFrediSV(false);
#ifdef FREDI_SV
  const uint8_t FCT_GROUPS(4);
  uint8_t ui8_currentLocoSpeed(0);
  uint8_t ui8_currentFunctions[FCT_GROUPS] {0};
  uint8_t ui8_mirrorLocoSpeed(0);
  uint8_t ui8_mirrorFunctions[FCT_GROUPS] {0};
#endif

uint8_t ret_slot(uint8_t adr, uint8_t adr2)
{
  for (uint8_t n = 1; n < SLOTMAX; n++)
	{
		if ((SlotTabelle[n].ucADR == adr) && (SlotTabelle[n].ucADR2 == adr2))
      return n;
	}
	return 0;
}

uint8_t get_slot(uint8_t adr, uint8_t adr2)
{
	for (uint8_t n = 1; n < SLOTMAX; n++)
	{
		if (((!(SlotTabelle[n].ucSTAT & (1 << 5)))) && (!(SlotTabelle[n].ucSTAT & (1 << 4))))
		{		
			SlotTabelle[n].ucADR = adr;
			SlotTabelle[n].ucADR2 = adr2;
      return n;
		}
	}
	return 0;
}

typedef struct 
{
	unsigned char stat1Val;
	char stringVal[5];
} StatTable_t;

const uint8_t COUNT(6);
static const StatTable_t statTable[COUNT] =
{
	{ DEC_MODE_128A,  "128A" }, // 0000 0111 (STAT1_SL_SPDEX | STAT1_SL_SPD14 | STAT1_SL_SPD28)
	{ DEC_MODE_28A,   "28A " }, // 0000 0100 (STAT1_SL_SPDEX)
	{ DEC_MODE_128,   "128 " }, // 0000 0011 (STAT1_SL_SPD14 | STAT1_SL_SPD28)
	{ DEC_MODE_28,    "28  " }, // 0000 0010 (STAT1_SL_SPD14)
	{ DEC_MODE_14,    "14  " }, // 0000 0001 (STAT1_SL_SPD28)
	{ DEC_MODE_28TRI, "28M " }  // 0000 0000
};

static const char* Stat1ValToString(unsigned char s) 
{
  for(uint8_t i = 0; i < COUNT; i++) 
		if( ( s & DEC_MODE_MASK ) == statTable[i].stat1Val ) 
			return statTable[i].stringVal;
 	return "?   ";
}

static void lcd_ThrottleId()
{
	lcd_clrxy(0, 3, 21); // is line 7 with iyLineOffset = 4
	lcd_write(F("ThrottleID: 0x"));
  lcd_wordAsHex(ui16_ThrottleId);
}

static void adr_lcd(uint8_t slot)
{
	if (slot < SLOTMAX)
		ui16_ThrottleId = (SlotTabelle[slot].ucID2 << 8) + SlotTabelle[slot].ucID1;
  ui8FlagSendingDisptach = 0;
  if(bShowFrediSV)
    return;

	lcd_clearLine(0);
	lcd_goto(0, 0);
  lcd_write(F("Alt: "));
  if (slot < SLOTMAX)
  {
    if (SlotTabelle[slot].ucADR2 > 0)
      lcd_wordAsDec((SlotTabelle[slot].ucADR2 * 128 + SlotTabelle[slot].ucADR));
    else
      lcd_wordAsDec(SlotTabelle[slot].ucADR);

    lcd_goto(12, 0);
    lcd_write(Stat1ValToString(SlotTabelle[slot].ucSTAT));
  }

#if defined OLED
  lcd_ThrottleId();
#endif
}

static void SlotManager(uint8_t adr, uint8_t adr2)
{
	uint8_t slot(ret_slot(adr, adr2));
	if (slot == 0)
	  slot = get_slot(adr, adr2);
	if (slot > 0)
	  Slot_SL_RD(slot);
}

static void disp_put()
{
  uint16_t ui16High(zahl / 128);
  uint16_t ui16Low(zahl - (ui16High * 128));
	SlotTabelle[2].ucADR = ui16Low;
	SlotTabelle[2].ucADR2 = ui16High;
	SlotTabelle[2].ucSPD = EMERGENCY_STOP;
}

static void AdresseSet()
{
	lcd_clrxy(5, 1, 4);
	lcd_goto(5, 1);
	lcd_wordAsDec(zahl);
	disp_put();
}

static void readzahl(uint8_t aktwert)
{
  if (stelle == 0)
  	zahl = aktwert;
	else
    if (stelle < 4)
      zahl = (zahl * 10) + aktwert;
	stelle++;
  AdresseSet();
}

uint8_t get_Tastatur()
{
	uint16_t ui16_EditValue(0);
	uint8_t ui8_buttons(0);
  // returns non-zero-value in ui8_buttons when key is released after pressing him
  getEditValueFromKeypad(true, 9, &ui16_EditValue, &ui8_buttons);

  if(ui8_buttons)
  {
  	lcd_clrxy(0, 2, 21); // is line 6 with iyLineOffset = 4
    bReadFromQRCode = false;
    ui8FlagSendingDisptach = 0;
#if defined OLED
    lcd_clearLine(6);
#endif
  }

  if(ui8_buttons & BUTTON_SELECT)  // '#'
		return SPECIAL_BUTTON::HASH;

  if(ui8_buttons & BUTTON_STAR)  // '*'
		return SPECIAL_BUTTON::STAR;

  if(ui8_buttons & BUTTON_KEYPAD)  // '0'...'9'
		return (uint8_t)(ui16_EditValue);

  return SPECIAL_BUTTON::NONE;
}

static void FahrstufenSet()
{
  SlotTabelle[2].ucSTAT &= ~DEC_MODE_MASK;
  SlotTabelle[2].ucSTAT |= statTable[fahrstufen].stat1Val;
  
  lcd_goto(12, 1);
	lcd_write(Stat1ValToString(SlotTabelle[2].ucSTAT));
}

void fahrstufenCHG()
{
  fahrstufen++;
  if (fahrstufen == COUNT)
    fahrstufen = 0;

  FahrstufenSet();
}

void returnToDispatchMode()
{
  bShowFrediSV = false;
  bFREDITestActive = false;
  
#if defined OLED  
  iyLineOffset = 0;
#endif
  lcd_title();
  show_new();
}

void show_new()
{
  lcd_goto(0, 1);
  lcd_write(F("Neu: "));
  fahrstufen = DEC_STEPS;
  fahrstufenCHG();
}

void HandleTastatur()
{
#if defined FREDI_SV
  if(bShowFrediSV)
    lcd_Handle_Dynamic_TestPage(); // dynamic page, handle always when requested
#endif

	uint8_t taste(get_Tastatur());
  if (taste == SPECIAL_BUTTON::NONE)
    return;
  if(!bShowFrediSV)
  {
    if (taste == SPECIAL_BUTTON::STAR)  // '*'
    {
      stelle = 0;
      zahl = 0;
      disp_put();
      lcd_clrxy(5, 1, 7);
    }
    else if (taste == SPECIAL_BUTTON::HASH)  // '#'
      fahrstufenCHG();
    else
      readzahl(taste);
  }    
#if defined FREDI_SV
  else
  {
    if (taste == SPECIAL_BUTTON::STAR)  // '*'
      // switch from SV-Mode to dispatch-mode:
      returnToDispatchMode();
    else
      lcd_Handle_Static_SVPages(taste);
  }
#endif
}

void setup()
{
#if defined DEBUG
  Serial.begin(57600);
#endif

  for (uint8_t lauf = 0; lauf < SLOTMAX; lauf++)
    SlotTabelle[lauf].ucSTAT=0x00;
  
  trk = 0x07;
  SlotTabelle[2].ucSTAT = 0x33;
  
  //---- init LocoNet
  InitLocoNet();
  
  //---- init Keypad
  CheckAndInitKeypad();

  //---- init LCD
  lcd_init();

  lcd_title();

#if defined QRCODE
  //---- init serial / QRCode-Reader
  serial_init();
#endif

#if defined FREDI_SV
  setupForFrediSV();
  if(!bShowFrediSV)
#endif
    show_new();
}

void loop()
{
  // light the Heartbeat LED
  oHeartbeat.beat();

  HandleLocoNetMessages();
	HandleTastatur();
#if defined QRCODE
  HandleSerial();
#endif
}

// to keep heartbeat alive during delay
void yield(void) { oHeartbeat.beat(); }
