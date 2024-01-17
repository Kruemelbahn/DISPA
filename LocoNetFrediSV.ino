//=== LocoNetFrediSV for use with DISPA ===
#if defined FREDI_SV

#include <LocoNet.h>  // used to include ln_opc.h
#include <OLEDPanel.h>

uint8_t FrediSvToRead[] = { 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 43, 44, 255 };  // last one is 255, which indicates the end of this list

extern lnMsg *LnPacket;
extern uint8_t iyLineOffset;

uint8_t iCount(0);
unsigned long ul_WaitTelegram(0);

void CheckForCommunicationError(uint8_t ui8SecondsToWait)
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
    lcd_write("Communication error");
    while(true)
    {
      // nothing more to do except
      // light the Heartbeat LED
      oHeartbeat.beat();
    }
  }
}

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
    {
      return true;
    }
  }
  return false;  
}

void setupForFrediSV()
{
	if (!isButtonStarPressed()) // '*' BUTTON_STAR
    return;
  delay(2000);
  if (!isButtonStarPressed())
    return;

  lcd_goto(0, 0);
  lcd_write("Dispatch FREDI now...");

  bShowFrediSV = true;
  SetCVsToDefault(); 

  iCount = 1;
  ul_WaitTelegram = millis(); // start controltime
  while(!ui16_ThrottleId)
  {
    // light the Heartbeat LED
    oHeartbeat.beat();

    HandleLocoNetMessages();  // try to read current throttle-id (which is necessary for reading SVs)
    CheckForCommunicationError(15);
  }
#if defined DEBUG
  Serial.print("Throttle-ID: ");
  Serial.println(ui16_ThrottleId, HEX);
#endif
  delay(100);

  // telegram for read one SV, e.g. SV10:
  // E5 10 01 02 02 10 <id_h> <id_l> <sv_no> 00 10      00     00 00 00 01
  // answer:
  // E5 10 01 42 02 10 <id_h> <id_l> <sv_no> 00 1<sv_h> <sv_l> 00 00 00 01
  
  for(int ui8_CurrentIndexToFrediSvToRead = 0; ui8_CurrentIndexToFrediSvToRead < GetCVCount(); ui8_CurrentIndexToFrediSvToRead++)
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

    // Check for any received LocoNet packets
    while(true)
    {
      LnPacket = LocoNet.receive();
      if(LnPacket)
      {
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL 
        Printout('R');
#endif
        if (LnPacket->data[0] == OPC_MOVE_SLOTS /*BA*/)
          MasterReplyLAck(0x3A, 0x00);
        if ((LnPacket->data[0] == OPC_PEER_XFER /*E5*/) &&
            (LnPacket->data[1] == 0x10)
          )
          {
            if(HandleE5MessageForFrediSv())
              break;
          }
      }
      CheckForCommunicationError(5);
    } // while(true)
  } // for(int CurrentIndexToFrediSvToRead = 0; CurrentIndexToFrediSvToRead < GetCVCount(); CurrentIndexToFrediSvToRead++)

  // and finally display them (initialy):
  lcd_SV_Part1();
}

#endif  
