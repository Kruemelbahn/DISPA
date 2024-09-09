//=== OLEDPanel for Dispa ===
// include the library code:
#if defined OLED

#include <OLEDPanel.h>

//=== declaration of var's =======================================
//--- for use of OLED

OLEDPanel displayPanel;

//--- end OLED

void lcd_init()
{
  displayPanel.begin();  // no function-keys used
}
	
void lcd_write(const __FlashStringHelper *pText)
{
  displayPanel.print(pText);
}

void lcd_write(const char *s)
{
  displayPanel.print(s);
}

/* Go to the specified position */
void lcd_goto(uint8_t x, uint8_t y)
{
  displayPanel.setCursor(x, y + iyLineOffset);
}

void lcd_clearLine(uint8_t y)
{
  displayPanel.clearLine(y);
} 

void lcd_clrxy(uint8_t x, uint8_t y, uint8_t count)
{
  displayPanel.setCursor(x, y + iyLineOffset);
  while(count--)
    displayPanel.print(' ');
  displayPanel.setCursor(x, y + iyLineOffset);
}

void lcd_wordAsDec(uint16_t ui16_Out)
{
  // 4 digits, 0..9999
  if(ui16_Out < 1000)
    displayPanel.print('0');
  if(ui16_Out < 100)
    displayPanel.print('0');
  if(ui16_Out < 10)
    displayPanel.print('0');
  displayPanel.print(ui16_Out, DEC);
}

void lcd_wordAsHex(uint16_t ui16_Out)
{
	// 4 digits, 0..FFFF
	if (ui16_Out < 0x1000)
		displayPanel.print('0');
	if (ui16_Out < 0x100)
		displayPanel.print('0');
	if (ui16_Out < 0x10)
		displayPanel.print('0');
	displayPanel.print(ui16_Out, HEX);
}

void lcd_title()
{
  displayPanel.clear();
  lcd_goto(0, 1);
	displayPanel.print(F("DISPA "));
  displayPanel.print(SW_VERSION);
  displayPanel.print(F(" (fka FRANZ)"));
  lcd_goto(1, 2);
	displayPanel.print(F("(c)Kr"));
	displayPanel.print(char(0xFC));  // ü
	displayPanel.print(F("melsoft "));
  displayPanel.print(SW_YEAR);

  iyLineOffset = 4;
}

void lcd_owner_name(const char *sOwner, const char *sName)
{
  lcd_clrxy(0, 2, 21);  // is line 6 (iyLineOffset = 4 is active)
  if(sOwner && (strlen(sOwner) > 0))
    lcd_write(sOwner);
  if(sName && (strlen(sName) > 0))
  {
    if(sOwner && (strlen(sOwner) > 0))
      lcd_write("=");
    lcd_write(sName);
  }
}

#if defined FREDI_SV

void lcd_binary(uint8_t ui8_Out)
{
  if(ui8_Out < 128)
    displayPanel.print('0');
  if(ui8_Out < 64)
    displayPanel.print('0');
  if(ui8_Out < 32)
    displayPanel.print('0');
  if(ui8_Out < 16)
    displayPanel.print('0');
  if(ui8_Out < 8)
    displayPanel.print('0');
  if(ui8_Out < 4)
    displayPanel.print('0');
  if(ui8_Out < 2)
    displayPanel.print('0');
  displayPanel.print(ui8_Out, BIN);
}

void lcd_HexAsDec(uint8_t ui8_Out)
{
  displayPanel.print((uint8_t)(ui8_Out / 16), DEC);
  displayPanel.print((uint8_t)(ui8_Out % 16), DEC);
}

void lcd_Processor(uint8_t iType)
{
  switch(iType)
  {
    case 1: displayPanel.print(F("ATmega8")); break;
    case 2: displayPanel.print(F("ATmega88")); break;
    case 3: displayPanel.print(F("ATmega168")); break;
    case 4: displayPanel.print(F("ATmega328P")); break;
    default: displayPanel.print(iType); break;
  }
}

// function is called cyclic by 'void HandleTastatur()' when 'bShowFrediSV' is set
void lcd_Handle_Static_SVPages(uint8_t taste)
{
  if(taste == 1)
    lcd_SV_Part1();
  else if(taste == 2)
    lcd_SV_Part2();
  else if(taste == 3)
    lcd_SV_Part3();
  else if(taste == 4)
    lcd_SV_Part4();
  else if(taste == 5)
    lcd_SV_Part5();
  else if (!bFREDITestActive && (taste == SPECIAL_BUTTON::HASH))  // '#'
  {
    const uint16_t ui16LocoAddress((GetSV(8) << 7) + GetSV(9));
    if((ui16LocoAddress > 0)|| (ui16LocoAddress < 10239))
      lcd_FREDI_InitTest();
  }
}

void lcd_SV_Part1()
{
  bFREDITestActive = false;
  
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  iyLineOffset = 0;
  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Part 1"));
  lcd_goto(1, 2);
  displayPanel.print(F("SV 2   : ")); // softwareversion from build
  displayPanel.print(GetSV(2), DEC);

  lcd_goto(1, 3);
  displayPanel.print(F("SV 3, 4: 0x")); // throttle-id
  lcd_wordAsHex((GetSV(4) << 8) + GetSV(3));
  
  lcd_goto(1, 4);
  displayPanel.print(F("SV 5   : ")); // hardwareversion from makefile
  lcd_Processor(GetSV(5));
  
  lcd_goto(1, 5);
  displayPanel.print(F("SV 6   : ")); // hardwareversion from flash
  lcd_Processor(GetSV(6));
}

void lcd_SV_Part2()
{
  bFREDITestActive = false;
  
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  iyLineOffset = 0;
  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Part 2"));
  lcd_goto(1, 2);
  displayPanel.print(F("SV 8, 9: ")); // Lokadresse
  const uint16_t ui16LocoAddress((GetSV(8) << 7) + GetSV(9));
  if(!ui16LocoAddress || ui16LocoAddress >= 10239)
    displayPanel.print(F("---"));
  else
    displayPanel.print(ui16LocoAddress, DEC);

  lcd_goto(1, 3);
  displayPanel.print(F("SV10   : ")); // Fahrstufen
  // aufgeschlüsselt:
  switch(GetSV(10) & 0x07)
  {
    case  0: displayPanel.print(F("28 FS")); break;
    case  1: displayPanel.print(F("28M FS")); break;
    case  2: displayPanel.print(F("14 FS")); break;
    case  3: displayPanel.print(F("128 FS")); break;
    case  4: displayPanel.print(F("28A FS")); break;
    case  7: displayPanel.print(F("128A FS")); break;
    default: lcd_binary(GetSV(10)); break;
  }

  lcd_goto(1, 4);
  displayPanel.print(F("SV11   : ")); // operating mode
  if(GetSV(11) == SKIP_SELF_TEST)
    displayPanel.print(F("operating"));
  else if(GetSV(11) == 0xFF)
    displayPanel.print(F("selftest"));
  else  
    displayPanel.print(GetSV(11));
}

void lcd_SV_Part3()
{
  bFREDITestActive = false;
  
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Part 3"));
  lcd_goto(1, 2);
  displayPanel.print(F("SV12   : ")); // hardwareversion detected after selftest
  lcd_binary(GetSV(12));
  // und jetzt aufgeschlüsselt:
  uint8_t ui8Line(3);
  lcd_goto(1, ui8Line);
  // line 1:
  if(GetSV(2) <= 21) // softwareversion from build
  {
    switch(GetSV(12) & 0x03)
    {
      case 0: displayPanel.print(F("unknown Hardware")); ++ui8Line; break;
      case 1: displayPanel.print(F("Incr.-Encoder")); ++ui8Line; break;
      case 2: displayPanel.print(F("Incr.-Encoder Switch")); ++ui8Line; break;
      case 3: displayPanel.print(F("Analog.-Poti")); ++ui8Line; break;
      default: break;
    } // switch(GetSV(12) & 0x03)
  } // if(GetSV(2) <= 21)
  else
  {
    switch(GetSV(12) & 0x0F)
    {
      case 0: 
      case 15:displayPanel.print(F("unknown Hardware")); ++ui8Line; break;
      case 1: displayPanel.print(F("Incr.-Encoder F1-F4")); ++ui8Line; break;
      case 2:
      case 3: displayPanel.print(F("Analog.-Poti F1-F4")); ++ui8Line; break;
      case 4: displayPanel.print(F("Matrix-FREDI")); ++ui8Line; break;
      case 6: displayPanel.print(F("Matrix-FREDI (SWD)")); ++ui8Line; break;
      case 14:displayPanel.print(F("FREDI w. BRAKE-Button")); ++ui8Line; break;
      default: break;
    } // switch(GetSV(12) & 0x0F)
  } // else if(GetSV(2) <= 21)
  lcd_goto(1, ui8Line);
  // line 2:
  if(GetSV(12) & 0x40) // Bit 6
    displayPanel.print(F("Two Shift buttons"));
  else
    displayPanel.print(F("One Shift button"));
   ++ui8Line;

  lcd_goto(1, ui8Line);
  // line 3:
  if(GetSV(12) & 0x80) // Bit 7
    displayPanel.print(F("Forw-Off-Rev-Switch"));
  else
    displayPanel.print(F("Forw-Rev-Switch"));
}

void lcd_SV_Part4()
{
  bFREDITestActive = false;
  
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Part 4"));
  lcd_goto(1, 2);
  displayPanel.print(F("SV13,14: ")); // softwareindex
  displayPanel.print(GetSV(13), DEC);
  displayPanel.print('.');
  displayPanel.print(GetSV(14), DEC);
  
  lcd_goto(1, 3);
  displayPanel.print(F("SV15-17: ")); // date of compilation
  lcd_HexAsDec(GetSV(15));
  displayPanel.print('.');
  lcd_HexAsDec(GetSV(16));
  displayPanel.print('.');
  lcd_HexAsDec(GetSV(17));

  lcd_goto(1, 4);
  displayPanel.print(F("SV43   : ")); // Using OPC-IMM (Digitrax compatible) for functions above F12
  if(GetSV(43) == 0xFF)
    displayPanel.print(F("no OPC_IMM"));
  else
  {
    if(GetSV(43) == 0x00)
      displayPanel.print(F("OPC_IMM act"));
    else
      displayPanel.print(GetSV(43), DEC);
  }      

  lcd_goto(1, 5);
  displayPanel.print(F("SV44   : ")); // brake-function, range = F1...F19
  if(GetSV(44) == 0xFF)
    displayPanel.print(F("no BRAKE"));
  else
  {
    displayPanel.print(F("BRAKE: F"));
    displayPanel.print(GetSV(44), DEC);
  }      
}

#define FCT_ROW 5
void lcd_SV_Part5()
{
  bFREDITestActive = false;
  
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Part 5"));
  lcd_goto(1, 2);
  displayPanel.print(F("SV18...34")); // F-status
  
  lcd_goto(11, 3);
  displayPanel.print(F("1111111"));
  lcd_goto(0, 4);
  displayPanel.print(F("F01234567890123456"));
  lcd_goto(1, FCT_ROW);

  for(uint8_t iSvNo = 18; iSvNo < 35; iSvNo++)
    displayPanel.print(!GetSV(iSvNo) ? '0' : '1');
}

void lcd_FREDI_InitTest()
{
  bFREDITestActive = true;

  for(uint8_t y = 0; y < 8; y++)
    displayPanel.clearLine(y);

  iyLineOffset = 0;
  lcd_goto(0, 0);
  displayPanel.print(F("FREDI-SV - Test"));
  lcd_goto(6, 2);
  displayPanel.print(F("< 000 >")); // speed and direction, will be overwritten
  lcd_goto(11, 3);
  displayPanel.print(F("1111111"));
  lcd_goto(0, 4);
  displayPanel.print(F("F01234567890123456"));
  lcd_goto(1, FCT_ROW);
  displayPanel.print(F(".................")); // F0...F16, will be overwritten
}

void lcd_Handle_Dynamic_TestPage()
{
  if(!bFREDITestActive)
    return;

  // show values from FREDI depending on telegrams A0...A3, D4 and ED
  // speed and direction
  if(ui8_currentLocoSpeed != ui8_mirrorLocoSpeed)
  {
    lcd_goto(8, 2);
    if(ui8_currentLocoSpeed < 100)
      displayPanel.print(' ');
    if(ui8_currentLocoSpeed < 10)
      displayPanel.print(' ');
    displayPanel.print(ui8_currentLocoSpeed, DEC);

    ui8_mirrorLocoSpeed = ui8_currentLocoSpeed;
  }

  // functions
  for (uint8_t iIndex = 0; iIndex < FCT_GROUPS; iIndex++ )
  {
    if (ui8_mirrorFunctions[iIndex] == ui8_currentFunctions[iIndex])
      continue;
      
    if(!iIndex)
    {
      lcd_goto(6, 2); // reverse direction
      displayPanel.print(ui8_currentFunctions[iIndex] & 0x20 ? '<' : ' ');
      lcd_goto(12, 2); // forward direction
      displayPanel.print(ui8_currentFunctions[iIndex] & 0x20 ? ' ' : '>');

      lcd_goto(1, FCT_ROW);       // F0
      displayPanel.print(ui8_currentFunctions[iIndex] & 0x10 ? 'I' : '.');
    } // if(!iIndex)

    const uint8_t iXPos(iIndex * 4 + 2);
    lcd_goto(iXPos, FCT_ROW);     // F1, F5, F9,  F13
    displayPanel.print(ui8_currentFunctions[iIndex] & 0x01 ? 'I' : '.');
    lcd_goto(iXPos + 1, FCT_ROW); // F2, F6, F10, F14
    displayPanel.print(ui8_currentFunctions[iIndex] & 0x02 ? 'I' : '.');
    lcd_goto(iXPos + 2, FCT_ROW); // F3, F7, F11, F15
    displayPanel.print(ui8_currentFunctions[iIndex] & 0x04 ? 'I' : '.');
    lcd_goto(iXPos + 3, FCT_ROW); // F4, F8, F12, F16
    displayPanel.print(ui8_currentFunctions[iIndex] & 0x08 ? 'I' : '.');

    ui8_mirrorFunctions[iIndex] = ui8_currentFunctions[iIndex];
  } // for (uint8_t iIndex = 0; iIndex < FCT_GROUPS; iIndex++ )
}

#endif // defined FREDI_SV

#endif // defined OLED
