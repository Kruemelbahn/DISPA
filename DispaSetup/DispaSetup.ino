/*
 * DispaSetup - used for checking OLED and Keypad
 
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
  - 10 
  - 11 Out LCD-Panel Enable 
  - 12 Out LCD-Panel R/W
  - 13
  - 14
  - 15
  - 16
  - 17
  - 18     (used by I²C: SDA)
  - 19     (used by I²C: SCL)

 *************************************************** 
 *  Copyright (c) 2018 Michael Zimmermann <http://www.kruemelsoft.privat.t-online.de>
 *  All rights reserved.
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************
 */
    
#include <Wire.h>
#include <i2ckeypad.h>

#include <OLEDPanel.h>

#include <HeartBeat.h>
HeartBeat oHeartbeat;

//========================================================
#define PCF8574_ADDR 0x39

#define ROWS 4
#define COLS 4

i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);

OLEDPanel displayPanel;

void setup()
{
  Serial.begin(57600);

  Wire.begin();

  kpd.init();

  displayPanel.begin();  // no function-keys used

  displayPanel.setCursor(0,0);
  displayPanel.print(F("Dispa-Setup"));
}

void loop()
{
  // light the Heartbeat LED
  oHeartbeat.beat();
  
  char key = kpd.get_key();

  if(key != '\0')
  {
    displayPanel.setCursor(0,2);
    displayPanel.print(key);
  }
}
