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

/* write one character to the LCD */
void lcd_putc(char c)
{
  displayPanel.print(c);
}

/* Go to the specified position */
void lcd_goto(uint8_t x, uint8_t y)
{
  displayPanel.setCursor(x, y + iyLineOffset);
}
  
void lcd_clrxy(uint8_t x, uint8_t y, uint8_t count)
{
  displayPanel.setCursor(x, y + iyLineOffset);
  while(count--)
    displayPanel.print(' ');
  displayPanel.setCursor(x, y + iyLineOffset);
}

void lcd_word(uint16_t ui16_Out)
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
	lcd_write("FRANZ 2.0 (aka DISPA)");
  lcd_goto(1, 2);
	lcd_write("(c)Kr");
	lcd_putc(char(0xFC));  // ü
	lcd_write("melsoft 2019");

  iyLineOffset = 4;
}

#endif // defined OLED
