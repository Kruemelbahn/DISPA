//=== OLEDPanel for Dispa ===
// include the library code:
#if defined OLED

#include <OLEDPanel.h>

//=== declaration of var's =======================================
//--- for use of OLED

OLEDPanel displayPanel;
uint8_t iyLineOffset = 0;

//--- end OLED

void lcd_init()
{
  displayPanel.begin();  // no function-keys used
}
	
void lcd_clear(void)
{
  /* do nothing:
   * function is used for clearing lcd-panel which has only two lines...
   */
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
	displayPanel.print("DISPA ");
  displayPanel.print(SW_VERSION);
  displayPanel.print(" (fka FRANZ)");
  lcd_goto(1, 2);
	displayPanel.print("(c)Kr");
	displayPanel.print(char(0xFC));  // ü
	displayPanel.print("melsoft ");
  displayPanel.print(SW_YEAR);

  iyLineOffset = 4;
}

#if defined FREDI_SV
void lcd_binary(uint8_t ui8_Out)
{
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
    case 1: displayPanel.print("ATmega8"); break;
    case 2: displayPanel.print("ATmega88"); break;
    case 3: displayPanel.print("ATmega168"); break;
    case 4: displayPanel.print("ATmega328P"); break;
    default: displayPanel.print(iType); break;
  }
}

void lcd_Handle_SVPages(uint8_t taste)
{
  if(taste == 1)
    lcd_SV_Part1();
  else if(taste == 2)
    lcd_SV_Part2();
  else if(taste == 3)
    lcd_SV_Part3();
}

void lcd_SV_Part1()
{
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  iyLineOffset = 0;
  lcd_goto(0, 0);
  displayPanel.print("FREDI-SV - Part 1");
  lcd_goto(1, 2);
  displayPanel.print("SV 2   : "); // softwareversion from build
  displayPanel.print(GetSV(2), DEC);

  lcd_goto(1, 3);
  displayPanel.print("SV 3, 4: 0x"); // throttle-id
  lcd_wordAsHex((GetSV(4) << 8) + GetSV(3));
  
  lcd_goto(1, 4);
  displayPanel.print("SV 5   : "); // hardwareversion from makefile
  lcd_Processor(GetSV(5));
  
  lcd_goto(1, 5);
  displayPanel.print("SV 6   : "); // hardwareversion from flash
  lcd_Processor(GetSV(6));
  
  lcd_goto(1, 6);
  displayPanel.print("SV11   : "); // operating mode
  if(GetSV(11) == 0x55)
    displayPanel.print("operating");
  else if(GetSV(11) == 0xFF)
    displayPanel.print("selftest");
  else  
    displayPanel.print(GetSV(11));
}

void lcd_SV_Part2()
{
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  lcd_goto(0, 0);
  displayPanel.print("FREDI-SV - Part 2");
  lcd_goto(1, 2);
  displayPanel.print("SV12   : "); // hardwareversion detected after selftest
  lcd_binary(GetSV(12));
  // und jetzt aufgeschlüsselt:
  uint8_t ui8Line(3);
  lcd_goto(1, ui8Line);
  // line 1:
  if(GetSV(2) <= 21) // softwareversion from build
  {
    switch(GetSV(12) & 0x03)
    {
      case 0: displayPanel.print("unknown Hardware"); ++ui8Line; break;
      case 1: displayPanel.print("Incr.-Encoder"); ++ui8Line; break;
      case 2: displayPanel.print("Incr.-Encoder Switch"); ++ui8Line; break;
      case 3: displayPanel.print("Analog.-Poti"); ++ui8Line; break;
      default: break;
    } // switch(GetSV(12) & 0x03)
  } // if(GetSV(2) <= 21)
  else
  {
    switch(GetSV(12) & 0x0F)
    {
      case 0: 
      case 15:displayPanel.print("unknown Hardware"); ++ui8Line; break;
      case 1: displayPanel.print("Incr.-Encoder F1-F4"); ++ui8Line; break;
      case 2:
      case 3: displayPanel.print("Analog.-Poti F1-F4"); ++ui8Line; break;
      case 4: displayPanel.print("Matrix-FREDI"); ++ui8Line; break;
      case 6: displayPanel.print("Matrix-FREDI (SWD)"); ++ui8Line; break;
      case 14:displayPanel.print("FREDI w. BRAKE-Button"); ++ui8Line; break;
      default: break;
    } // switch(GetSV(12) & 0x0F)
  } // else if(GetSV(2) <= 21)
  lcd_goto(1, ui8Line);
  // line 2:
  if(GetSV(12) & 0x40) // Bit 6
    displayPanel.print("Two Shift buttons");
  else
    displayPanel.print("One Shift button");
   ++ui8Line;

  lcd_goto(1, ui8Line);
  // line 3:
  if(GetSV(12) & 0x80) // Bit 7
    displayPanel.print("Forw-Off-Rev-Switch");
  else
    displayPanel.print("Forw-Rev-Switch");
}

void lcd_SV_Part3()
{
  for(uint8_t y = 1; y < 8; y++)
    displayPanel.clearLine(y);

  lcd_goto(0, 0);
  displayPanel.print("FREDI-SV - Part 3");
  lcd_goto(1, 2);
  displayPanel.print("SV13,14: "); // softwareindex
  displayPanel.print(GetSV(13), DEC);
  displayPanel.print('.');
  displayPanel.print(GetSV(14), DEC);
  
  lcd_goto(1, 3);
  displayPanel.print("SV15-17: "); // date of compilation
  lcd_HexAsDec(GetSV(15));
  displayPanel.print('.');
  lcd_HexAsDec(GetSV(16));
  displayPanel.print('.');
  lcd_HexAsDec(GetSV(17));

  lcd_goto(1, 4);
  displayPanel.print("SV43   : "); // Using OPC-IMM (Digitrax compatible) for functions above F12
  if(GetSV(43) == 0xFF)
    displayPanel.print("no OPC_IMM");
  else
  {
    if(GetSV(43) == 0x00)
      displayPanel.print("OPC_IMM act");
    else
      displayPanel.print(GetSV(43), DEC);
  }      

  lcd_goto(1, 5);
  displayPanel.print("SV44   : "); // brake-function, range = F1...F19
  if(GetSV(44) == 0xFF)
    displayPanel.print("no BRAKE");
  else
  {
    displayPanel.print("BRAKE: F");
    displayPanel.print(GetSV(44), DEC);
  }      
}
#endif // defined FREDI_SV

#endif // defined OLED
