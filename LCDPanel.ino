//=== LCDPanel for Dispa ===
// include the library code:
#if defined LCD

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 9, 5, 4, 3);

void lcd_init()
{
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
}
	
void lcd_clear(void)
{
  lcd.clear();
}

void lcd_write(const __FlashStringHelper *pText)
{
  lcd.print(pText);
}

void lcd_write(const char *s)
{
  lcd.print(s);
}

/* Go to the specified position */
void lcd_goto(uint8_t x, uint8_t y)
{
  lcd.setCursor(x, y);
}
  
void lcd_clrxy(uint8_t x, uint8_t y, uint8_t count)
{
  lcd.setCursor(x, y);
  while(count--)
    lcd.print(' ');
  lcd.setCursor(x, y);
}

void lcd_clearLine(uint8_t y)
{
  lcd_clrxy(0, y, 16);
} 

void lcd_wordAsDec(uint16_t ui16_Out)
{
  // 4 digits, 0..9999
  if(ui16_Out < 1000)
    lcd.print('0');
  if(ui16_Out < 100)
    lcd.print('0');
  if(ui16_Out < 10)
    lcd.print('0');
  lcd.print(ui16_Out, DEC);
}

void lcd_title()
{
  lcd_clear();
  lcd_goto(0, 0);
	lcd_write(F("DISPA "));
	lcd_write(SW_VERSION);
	lcd_write(F(" / "));
	lcd_write(SW_YEAR);
  lcd_goto(0, 1);
	lcd_write(F("(c)Kr"));
  lcd.print(char(0xF5));  // ü
	lcd_write(F("melsoft"));
  delay(2000);
  lcd_clear();
}

void lcd_owner_name(const char *sOwner, const char *sName)
{
  lcd_clrxy(0, 0, 16);
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

void lcd_HexAsDec(uint8_t ui8_Out)
{
  lcd.print((uint8_t)(ui8_Out / 16), DEC);
  lcd.print((uint8_t)(ui8_Out % 16), DEC);
}

void lcd_wordAsHex(uint16_t ui16_Out)
{
	// 4 digits, 0..FFFF
	if (ui16_Out < 0x1000)
		lcd.print('0');
	if (ui16_Out < 0x100)
		lcd.print('0');
	if (ui16_Out < 0x10)
		lcd.print('0');
	lcd.print(ui16_Out, HEX);
}

void lcd_binary(uint8_t ui8_Out)
{
  if(ui8_Out < 128)
    lcd.print('0');
  if(ui8_Out < 64)
    lcd.print('0');
  if(ui8_Out < 32)
    lcd.print('0');
  if(ui8_Out < 16)
    lcd.print('0');
  if(ui8_Out < 8)
    lcd.print('0');
  if(ui8_Out < 4)
    lcd.print('0');
  if(ui8_Out < 2)
    lcd.print('0');
  lcd.print(ui8_Out, BIN);
}

void lcd_Processor(uint8_t iType)
{
  switch(iType)
  {
    case 1: lcd.print(F("ATmega8")); break;
    case 2: lcd.print(F("ATmega88")); break;
    case 3: lcd.print(F("ATmega168")); break;
    case 4: lcd.print(F("ATmega328P")); break;
    default: lcd.print(iType); break;
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
  else if(taste == 6)
    lcd_SV_Part6();
  else if(taste == 7)
    lcd_SV_Part7();
  else if(taste == 8)
    lcd_SV_Part8();
  else if(taste == 9)
    lcd_SV_Part9();
  else if (taste == SPECIAL_BUTTON::HASH)  // '#'
  {
    const uint16_t ui16LocoAddress((GetSV(8) << 7) + GetSV(9));
    if((ui16LocoAddress > 0)|| (ui16LocoAddress < 10239))
      lcd_FREDI_InitTest();
  }
}

void lcd_SV_Part1()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV 2:")); // softwareversion from build
  lcd.print(GetSV(2), DEC);

  lcd_goto(0, 1);
  lcd.print(F("SV 3,4:0x")); // throttle-id
  lcd_wordAsHex((GetSV(4) << 8) + GetSV(3));
}

void lcd_SV_Part2()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV 5:")); // hardwareversion from makefile
  lcd_Processor(GetSV(5));
  
  lcd_goto(0, 1);
  lcd.print(F("SV 6:")); // hardwareversion from flash
  lcd_Processor(GetSV(6));
}

void lcd_SV_Part3()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV 8,9:")); // Lokadresse
  const uint16_t ui16LocoAddress((GetSV(8) << 7) + GetSV(9));
  if(!ui16LocoAddress || ui16LocoAddress >= 10239)
    lcd.print(F("---"));
  else
    lcd.print(ui16LocoAddress, DEC);
  
  lcd_goto(0, 1);
  lcd.print(F("SV10:")); // Fahrstufen
  // aufgeschlüsselt:
  switch(GetSV(10) & 0x07)
  {
    case  0: lcd.print(F("28 FS")); break;
    case  1: lcd.print(F("28M FS")); break;
    case  2: lcd.print(F("14 FS")); break;
    case  3: lcd.print(F("128 FS")); break;
    case  4: lcd.print(F("28A FS")); break;
    case  7: lcd.print(F("128A FS")); break;
    default: lcd.print(F("unknown FS")); break;
  }
}

void lcd_SV_Part4()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV11:")); // operating mode
  if(GetSV(11) == SKIP_SELF_TEST)
    lcd.print(F("operating"));
  else if(GetSV(11) == 0xFF)
    lcd.print(F("selftest"));
  else  
    lcd.print(GetSV(11));
  
  lcd_goto(0, 1);
  lcd.print(F("SV12:")); // hardwareversion detected after selftest
  lcd_binary(GetSV(12));
}

void lcd_SV_Part5()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  // line 1:
  if(GetSV(2) <= 21) // softwareversion from build
  {
    switch(GetSV(12) & 0x03)
    {
      case 0: lcd.print(F("unknown Hardware")); break;
      case 1: lcd.print(F("Incr.-Encoder")); break;
      case 2: lcd.print(F("Incr.Enc. Switch")); break;
      case 3: lcd.print(F("Analog.-Poti")); break;
      default: break;
    } // switch(GetSV(12) & 0x03)
  } // if(GetSV(2) <= 21)
  else
  {
    switch(GetSV(12) & 0x0F)
    {
      case 0: 
      case 15:lcd.print(F("unknown Hardware")); break;
      case 1: lcd.print(F("Incr.-Enc. F1-F4")); break;
      case 2:
      case 3: lcd.print(F("Ana.-Poti F1-F4")); break;
      case 4: lcd.print(F("Matrix-FREDI")); break;
      case 6: lcd.print(F("SWD-FREDI")); break;
      case 14:lcd.print(F("FREDI w. BRAKE")); break;
      default: break;
    } // switch(GetSV(12) & 0x0F)
  } // else if(GetSV(2) <= 21)

  lcd_goto(0, 1);
  // line 2:
  if(GetSV(12) & 0x40) // Bit 6
    lcd.print(F("Two Shift button"));
  else
    lcd.print(F("One Shift button"));
}

void lcd_SV_Part6()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  // line 3:
  if(GetSV(12) & 0x80) // Bit 7
    lcd.print(F("Forw-Off-Rev-Sw"));
  else
    lcd.print(F("Forw-Rev-Sw"));
}

void lcd_SV_Part7()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV13,14:")); // softwareindex
  lcd.print(GetSV(13), DEC);
  lcd.print('.');
  lcd.print(GetSV(14), DEC);
  
  lcd_goto(0, 1);
  lcd.print(F("SV15-17:")); // date of compilation
  lcd_HexAsDec(GetSV(15));
  lcd.print('.');
  lcd_HexAsDec(GetSV(16));
  lcd.print('.');
  lcd_HexAsDec(GetSV(17));
}

void lcd_SV_Part8()
{
  bFREDITestActive = false;
  
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV43:")); // Using OPC-IMM (Digitrax compatible) for functions above F12
  if(GetSV(43) == 0xFF)
    lcd.print(F("no OPC_IMM"));
  else
  {
    if(GetSV(43) == 0x00)
      lcd.print(F("OPC_IMM act"));
    else
      lcd.print(GetSV(43), DEC);
  }      

  lcd_goto(0, 1);
  lcd.print(F("SV44:")); // brake-function, range = F1...F19
  if(GetSV(44) == 0xFF)
    lcd.print(F("no BRAKE"));
  else
  {
    lcd.print(F("BRAKE: F"));
    lcd.print(GetSV(44), DEC);
  }      
}

void lcd_SV_Part9()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("SV18...34"));

  lcd_goto(15, 0);
  lcd.print(!GetSV(18) ? '0' : '1');
  lcd_goto(0, 1);
  for(uint8_t iSvNo = 19; iSvNo < 35; iSvNo++)
    lcd.print(!GetSV(iSvNo) ? '0' : '1');
}

void lcd_FREDI_InitTest()
{
  bFREDITestActive = true;

  lcd_clear();
  lcd_goto(0, 0);
  lcd.print(F("< 000 >        .")); // speed, direction and F0, will be overwritten
  lcd_goto(0, 1);
  lcd.print(F("................")); // F1...F16, will be overwritten
}

void lcd_Handle_Dynamic_TestPage()
{
  if(!bFREDITestActive)
    return;

  // show valuse from FREDI depending on telegrams A0...A3, D4 and ED
  // speed an direction
  if(ui8_currentLocoSpeed != ui8_mirrorLocoSpeed)
  {
    lcd_goto(2, 0);
    if(ui8_currentLocoSpeed < 100)
      lcd.print(' ');
    if(ui8_currentLocoSpeed < 10)
      lcd.print(' ');
    lcd.print(ui8_currentLocoSpeed, DEC);

    ui8_mirrorLocoSpeed = ui8_currentLocoSpeed;
  }
  // functions
  for (uint8_t iIndex = 0; iIndex < FCT_GROUPS; iIndex++ )
  {
    if (ui8_mirrorFunctions[iIndex] == ui8_currentFunctions[iIndex])
      continue;

    if(!iIndex)
    {
      lcd_goto(0, 0); // reverse direction
      lcd.print(ui8_currentFunctions[iIndex] & 0x20 ? '<' : ' ');
      lcd_goto(6, 0); // forward direction
      lcd.print(ui8_currentFunctions[iIndex] & 0x20 ? ' ' : '>');

      lcd_goto(15, 0);      // F0
      lcd.print(ui8_currentFunctions[iIndex] & 0x10 ? '|' : '.');
    } // if(!iIndex)

    const uint8_t iXPos(iIndex * 4);
    lcd_goto(iXPos, 3);     // F1, F5, F9,  F13
    lcd.print(ui8_currentFunctions[iIndex] & 0x01 ? '|' : '.');
    lcd_goto(iXPos + 1, 3); // F2, F6, F10, F14
    lcd.print(ui8_currentFunctions[iIndex] & 0x02 ? '|' : '.');
    lcd_goto(iXPos + 2, 3); // F3, F7, F11, F15
    lcd.print(ui8_currentFunctions[iIndex] & 0x04 ? '|' : '.');
    lcd_goto(iXPos + 3, 3); // F4, F8, F12, F16
    lcd.print(ui8_currentFunctions[iIndex] & 0x08 ? '|' : '.');

    ui8_mirrorFunctions[iIndex] = ui8_currentFunctions[iIndex];
  } // for (uint8_t iIndex = 0; iIndex < FCT_GROUPS; iIndex++ )
}

#endif // defined FREDI_SV

#endif // defined LCD
