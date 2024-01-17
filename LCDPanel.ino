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
	lcd_write("FRANZ 2.1");
  lcd_goto(0, 1);
	lcd_write("(c)Kr");
  lcd.print(char(0xF5));  // ü
	lcd_write("melsoft");
  delay(2000);
}

#endif // defined LCD
