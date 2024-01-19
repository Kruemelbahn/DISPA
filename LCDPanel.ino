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
	lcd_write("DISPA ");
	lcd_write(SW_VERSION);
	lcd_write(" / ");
	lcd_write(SW_YEAR);
  lcd_goto(0, 1);
	lcd_write("(c)Kr");
  lcd.print(char(0xF5));  // ü
	lcd_write("melsoft");
  delay(2000);
  lcd_clear();
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
  lcd.print(ui8_Out, BIN);
}

void lcd_Processor(uint8_t iType)
{
  switch(iType)
  {
    case 1: lcd.print("ATmega8"); break;
    case 2: lcd.print("ATmega88"); break;
    case 3: lcd.print("ATmega168"); break;
    case 4: lcd.print("ATmega328P"); break;
    default: lcd.print(iType); break;
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
  else if(taste == 4)
    lcd_SV_Part4();
  else if(taste == 5)
    lcd_SV_Part5();
  else if(taste == 6)
    lcd_SV_Part6();
  else if(taste == 7)
    lcd_SV_Part7();
}

void lcd_SV_Part1()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print("SV 2:"); // softwareversion from build
  lcd.print(GetSV(2), DEC);

  lcd_goto(0, 1);
  lcd.print("SV 3,4:0x"); // throttle-id
  lcd_wordAsHex((GetSV(4) << 8) + GetSV(3));
}

void lcd_SV_Part2()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print("SV 5:"); // hardwareversion from makefile
  lcd_Processor(GetSV(5));
  
  lcd_goto(0, 1);
  lcd.print("SV 6:"); // hardwareversion from flash
  lcd_Processor(GetSV(6));
}

void lcd_SV_Part3()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print("SV11:"); // operating mode
  if(GetSV(11) == 0x55)
    lcd.print("operating");
  else if(GetSV(11) == 0xFF)
    lcd.print("selftest");
  else  
    lcd.print(GetSV(11));
  
  lcd_goto(0, 1);
  lcd.print("SV12:"); // hardwareversion detected after selftest
  lcd_binary(GetSV(12));
}

void lcd_SV_Part4()
{
  lcd_clear();
  lcd_goto(0, 0);
  // line 1:
  if(GetSV(2) <= 21) // softwareversion from build
  {
    switch(GetSV(12) & 0x03)
    {
      case 0: lcd.print("unknown Hardware"); break;
      case 1: lcd.print("Incr.-Encoder"); break;
      case 2: lcd.print("Incr.Enc. Switch"); break;
      case 3: lcd.print("Analog.-Poti"); break;
      default: break;
    } // switch(GetSV(12) & 0x03)
  } // if(GetSV(2) <= 21)
  else
  {
    switch(GetSV(12) & 0x0F)
    {
      case 0: 
      case 15:lcd.print("unknown Hardware"); break;
      case 1: lcd.print("Incr.-Enc. F1-F4"); break;
      case 2:
      case 3: lcd.print("Ana.-Poti F1-F4"); break;
      case 4: lcd.print("Matrix-FREDI"); break;
      case 6: lcd.print("SWD-FREDI"); break;
      case 14:lcd.print("FREDI w. BRAKE"); break;
      default: break;
    } // switch(GetSV(12) & 0x0F)
  } // else if(GetSV(2) <= 21)

  lcd_goto(0, 1);
  // line 2:
  if(GetSV(12) & 0x40) // Bit 6
    lcd.print("Two Shift button");
  else
    lcd.print("One Shift button");
}

void lcd_SV_Part5()
{
  lcd_clear();
  lcd_goto(0, 0);
  // line 3:
  if(GetSV(12) & 0x80) // Bit 7
    lcd.print("Forw-Off-Rev-Sw");
  else
    lcd.print("Forw-Rev-Sw");
}

void lcd_SV_Part6()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print("SV13,14:"); // softwareindex
  lcd.print(GetSV(13), DEC);
  lcd.print('.');
  lcd.print(GetSV(14), DEC);
  
  lcd_goto(0, 1);
  lcd.print("SV15-17:"); // date of compilation
  lcd_HexAsDec(GetSV(15));
  lcd.print('.');
  lcd_HexAsDec(GetSV(16));
  lcd.print('.');
  lcd_HexAsDec(GetSV(17));
}

void lcd_SV_Part7()
{
  lcd_clear();
  lcd_goto(0, 0);
  lcd.print("SV43:"); // Using OPC-IMM (Digitrax compatible) for functions above F12
  if(GetSV(43) == 0xFF)
    lcd.print("no OPC_IMM");
  else
  {
    if(GetSV(43) == 0x00)
      lcd.print("OPC_IMM act");
    else
      lcd.print(GetSV(43), DEC);
  }      

  lcd_goto(0, 1);
  lcd.print("SV44:"); // brake-function, range = F1...F19
  if(GetSV(44) == 0xFF)
    lcd.print("no BRAKE");
  else
  {
    lcd.print("BRAKE: F");
    lcd.print(GetSV(44), DEC);
  }      
}

#endif // defined FREDI_SV

#endif // defined LCD
