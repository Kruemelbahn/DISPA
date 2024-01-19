//=== LocoNetFrediSV for use with DISPA ===
#if defined FREDI_SV

#include <LocoNet.h>  // used to include ln_opc.h
#include <OLEDPanel.h>

uint8_t FrediSvToRead[] = { 2, 3, 4, 5, 6, 11, 12, 13, 14, 15, 16, 17, 43, 44, 255 };  // last one is 255, which indicates the end of this list

extern lnMsg *LnPacket;
extern uint8_t iyLineOffset;

uint8_t iCount(0);
unsigned long ul_WaitTelegram(0);

boolean isButtonStarPressed()
{
  uint8_t ui8_KPDi2cAddress(getKPDAdddress());
  if(ui8_KPDi2cAddress)
  {
    Wire.beginTransmission(ui8_KPDi2cAddress);
    Wire.write(0b11110111);  // listen to last row, containing keys '*0#D'
    Wire.endTransmission();

    Wire.beginTransmission(ui8_KPDi2cAddress);
    Wire.requestFrom((uint8_t)ui8_KPDi2cAddress, (uint8_t)1);
    uint8_t val(Wire.read());
    Wire.endTransmission();
    if(val == 0b11100111)
      return true;
  }
  return false;  
}

boolean CheckForCommunicationError(uint8_t ui8SecondsToWait)
{
  if((millis() - ul_WaitTelegram) > (iCount * 1000))
  {
#if defined DEBUG
    Serial.print('.');
#endif
    lcd_goto(iCount, 1);
    lcd_write(".");
    ++iCount;
  }

  if((millis() - ul_WaitTelegram) > (ui8SecondsToWait * 1000))
  {
#if defined DEBUG
    Serial.println("Communication error");
#endif
    lcd_clearLine(0);
    lcd_goto(0, 1);
#if defined LCD
    lcd_write("Communicat.error");
#else
    lcd_write("Communication error");
#endif
    // we will stay in 'setup()' until '*' is pressed
    while(true)
    {
      // nothing more to do except
      // light the Heartbeat LED
      oHeartbeat.beat();

      if (isButtonStarPressed()) // '*' BUTTON_STAR
        return true;
    }
  }
  return false;
}

void setupForFrediSV()
{
	if (!isButtonStarPressed()) // '*' BUTTON_STAR
    return;
  // '*' BUTTON_STAR has to be pressed during power on for more then 2 seconds to enter FREDI-diagnostic-mode
  delay(2000);
  if (!isButtonStarPressed())
    return;

  lcd_goto(0, 0);
  lcd_write("Dispatch FREDI");

  bShowFrediSV = true;
  SetCVsToDefault(); 

  iCount = 1;
  ul_WaitTelegram = millis(); // start controltime
 
  // we will stay in 'setup()' until we received ThrottleID or '*' is pressed
  while(!ui16_ThrottleId)
  {
    // light the Heartbeat LED
    oHeartbeat.beat();

    HandleLocoNetMessages();  // try to read current throttle-id (which is necessary for reading SVs)
    if(CheckForCommunicationError(15))
    {
      // abort, return to dispatch-mode
      returnToDispatchMode();
      return;
    }
  }
#if defined DEBUG
  Serial.print("Throttle-ID: 0x");
  Serial.println(ui16_ThrottleId, HEX);
#endif
  delay(100);

  // telegram for read one SV:
  // E5 10 01 02 02 10 <id_l> <id_h> <sv_l> <sv_h> 10 00       00 00 00 01
  // answer:
  // E5 10 01 42 02 10 <id_l> <id_h> <sv_l> <sv_h> 10 <sv_val> 00 00 00 01
  
  for(uint8_t ui8_CurrentIndexToFrediSvToRead = 0; ui8_CurrentIndexToFrediSvToRead < GetCVCount(); ui8_CurrentIndexToFrediSvToRead++)
  {
    // light the Heartbeat LED
    oHeartbeat.beat();

    uint8_t ui8_SvNoToRead(FrediSvToRead[ui8_CurrentIndexToFrediSvToRead]);
    if(ui8_SvNoToRead >= GetCVCount())
      break;  // end of list reached

    sendE5Telegram(0x01 /*src*/, 0x02 /*cmd*/, 0x00 /*svx1*/,
                    (uint8_t)(ui16_ThrottleId & 0xFF), (uint8_t)(ui16_ThrottleId >> 8),
                    (uint8_t)(ui8_SvNoToRead & 0xFF), (uint8_t)(ui8_SvNoToRead >> 8),
                    0 /*D1*/, 0/*D2*/, 0/*D3*/, 0/*D4*/);
    
    iCount = 1;
    ul_WaitTelegram = millis(); // start new controltime

    // we will stay in 'setup()' until we received SVs from FREDI or '*' is pressed
    // Check for any received LocoNet packets
    while(true)
    {
      // light the Heartbeat LED
      oHeartbeat.beat();

      LnPacket = LocoNet.receive();
      if(LnPacket)
      {
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL 
        Printout('R');
#endif
        if (LnPacket->data[0] == OPC_MOVE_SLOTS /*BA*/)
          MasterReplyLack(0x3A, 0x00);
        else if ((LnPacket->data[0] == OPC_PEER_XFER /*E5*/) &&
                 (LnPacket->data[1] == 0x10)
               )
               {
                 if(HandleE5MessageForFrediSv())
                   break;
               }
      }
      if(CheckForCommunicationError(5))
      {
        // abort, return to dispatch-mode
        returnToDispatchMode();
        return;
      }
    } // while(true)
  } // for(uint8_t CurrentIndexToFrediSvToRead = 0; CurrentIndexToFrediSvToRead < GetCVCount(); CurrentIndexToFrediSvToRead++)

  // and finally display them (initialy):
  lcd_SV_Part1();
}

#endif  