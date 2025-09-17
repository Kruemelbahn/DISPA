//=== ReadFromSerial for DISPA ===
#if defined QRCODE

#include <SoftwareSerial.h>
#include <Bounce.h>

const uint8_t DISPA_RXD(10);  // PB2
const uint8_t DISPA_TXD(13);  // PB5
const uint8_t PIN_BUTTON(14); // PC0
const uint8_t PIN_LED(16);    // PC2

const uint8_t DEFAULT_SPEEDSTEP(2);
const uint16_t RECV_BUF_COUNT(300);

#define FZ_QRCODE
#if defined FZ_QRCODE
  uint8_t ui8FzOwnerPos(0);   // optional
  uint8_t ui8FzNumberPos(0);  // optional
#endif
uint8_t ui8DecAddressPos(0);
uint8_t ui8DecStepsPos(0);    // optional, default taken from manual input
char recv_buf[RECV_BUF_COUNT];

const uint8_t SV_FCT_NR_MAX(17); // 0...16
// 0 = change nothing
// 1 = set to 0x00
// 2 = set to 0xFF
uint8_t ui8FctArray[SV_FCT_NR_MAX] { 0 };

SoftwareSerial dispaSerial(DISPA_RXD, DISPA_TXD); // RX, TX

// instanciate Bounce-Object for 20ms
Bounce bounceBtn(Bounce(PIN_BUTTON, 20));

//--- 'compareTo' is taken from WString.cpp
int compareTo(const char *s1, const char *s2)
{
	if (!s1 || !s2)
  {
		if (s2 && (strlen(s2) > 0))
      return 0 - *(unsigned char *)(s2);
		if (s1 && (strlen(s1) > 0))
      return *(unsigned char *)(s1);
		return 0;
	}
	return strcmp(s1, s2);
}

static const uint8_t StringToStat1ValCount(unsigned char *s) 
{
  if (s && (strlen(s) > 0))
  {
    for(uint8_t i = 0; i < COUNT; i++) 
    {
      // remove spaces at the end before comparison:
      char tmpStringVal[5] { 0 };
      for(int j = 0; j < 5; j++)
        tmpStringVal[j] = (statTable[i].stringVal[j] == ' ' ? 0x00 : statTable[i].stringVal[j]);
      if(compareTo(s, tmpStringVal) == 0) 
        return i;
    } // for(uint8_t i = 0; i < COUNT; i++) 
  } // if (s && (strlen(s) > 0))
 	return fahrstufen; // keep unchanged if 's' is empty
}

void clearAllBuf()
{
#if defined FZ_QRCODE
  ui8FzOwnerPos = 0;
  ui8FzNumberPos = 0;
#endif
  ui8DecAddressPos = 0;
  ui8DecStepsPos = 0;

  for (uint16_t i = 0; i < RECV_BUF_COUNT; i++)
    recv_buf[i] = 0;

  for (uint8_t i = 0; i < SV_FCT_NR_MAX; i++)
    ui8FctArray[i] = 0;
}

void serial_init()
{  
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  clearAllBuf();

  dispaSerial.begin (9600);
}

void HandleSerial()
{
  if(bShowFrediSV)
    return;

  if (ui8FlagSendingDisptach == 0x01)
  {
#if defined SEND_QRCODE_DATA
    ul_WaitTelegram = millis(); // start controltime
    while(!ui16_ThrottleId)
    {
      // light the Heartbeat LED
      oHeartbeat.beat();

      send_request_for_ThrottleId();
      delay(1000);

      HandleLocoNetMessages();  // try to read current throttle-id
      if((millis() - ul_WaitTelegram) > 15000)
      {
        ui8FlagSendingDisptach = 0x80;
    #if defined DEBUG
        Serial.println(F("Communication error"));
    #endif
      } // if((millis() - ul_WaitTelegram) > 15000)
    } // while(!ui16_ThrottleId)
#endif
    if (ui16_ThrottleId)
    {
      lcd_ThrottleId();
  #if defined DEBUG 
      Serial.print(F("Throttle-ID: 0x"));
      Serial.println(ui16_ThrottleId, HEX);
  #endif
    } // if (ui16_ThrottleId)
  } // if (ui8FlagSendingDisptach == 0x01)
#if defined SEND_QRCODE_DATA
  if (ui16_ThrottleId && (ui8FlagSendingDisptach == 0x01))
  {
    ui8FlagSendingDisptach = 0x02;
    if(SlotTabelle[2].ucADR /*adrLow*/ || SlotTabelle[2].ucADR2 /*adrHigh*/)
    {
      // cmd == 0x05 => write four Bytes: SV8, 9, 10, 11 (where SV11 = mode, assumed it is SKIP_SELF_TEST which is send again to keep this state)
      sendE5Telegram(SRC_E5 /*src*/, 0x05 /*cmd*/, 0x00 /*svx1*/,
        (uint8_t)(ui16_ThrottleId & 0xFF), (uint8_t)(ui16_ThrottleId >> 8),
        8, 0,
        SlotTabelle[2].ucADR2 /*D1*/, SlotTabelle[2].ucADR/*D2*/, statTable[fahrstufen].stat1Val/*D3*/, SKIP_SELF_TEST/*D4*/);
    } // if(SlotTabelle[2].ucADR /*adrLow*/ || SlotTabelle[2].ucADR2 /*adrHigh*/)
    else
    {
      // cmd == 0x01 => write one Byte: SV10 (DecoderSteps)
      sendE5Telegram(SRC_E5 /*src*/, 0x01 /*cmd*/, 0x00 /*svx1*/,
        (uint8_t)(ui16_ThrottleId & 0xFF), (uint8_t)(ui16_ThrottleId >> 8),
        10, 0,
        statTable[fahrstufen].stat1Val /*D1*/, 0/*D2*/, 0/*D3*/, 0/*D4*/);
    } // else if(SlotTabelle[2].ucADR /*adrLow*/ && SlotTabelle[2].ucADR2 /*adrHigh*/)
    delay(250); // give throttle a change for reply

    // send function types if neccessary:
    for(uint8_t iFctNo = 0; iFctNo < SV_FCT_NR_MAX; iFctNo++)
    {
      if(!ui8FctArray[iFctNo])
        continue;
        
      // cmd == 0x01 => write one Byte
      sendE5Telegram(SRC_E5 /*src*/, 0x01 /*cmd*/, 0x00 /*svx1*/,
        (uint8_t)(ui16_ThrottleId & 0xFF), (uint8_t)(ui16_ThrottleId >> 8),
        iFctNo + 18, 0,
        ui8FctArray[iFctNo] == 1 ? 0x00 : 0xFF /*D1*/, 0/*D2*/, 0/*D3*/, 0/*D4*/);
        delay(250); // give throttle a change for reply...which is ignored
    } // for(uint8_t iFctNo = 0; iFctNo < SV_FCT_NR_MAX; iFctNo++)
  } // if (ui16_ThrottleId && (ui8FlagSendingDisptach == 0x01))
#endif // #if defined SEND_QRCODE_DATA

  // check the answer, whether values are set correct:
  if(ui8FlagSendingDisptach > 0x03)
  { 
    // handle error on writing SV8, 9, 10, 11
    ui8FlagSendingDisptach = 0;
  }

  if (!dispaSerial.available())
    return;

  bReadFromQRCode = false;

  clearAllBuf();

  int chRead(0);
  int iRecvMaxBufPos(0);
#if defined DEBUG
  Serial.print("START:");
#endif
  while(true)
  {
    chRead = dispaSerial.read();
    if((chRead == '\n') || (chRead == '\r')) // end char '\n' automatically send from WaveShare-Scanner (p.35)
      break;
    if((chRead < 0x20) || (chRead > 0x7F))   // only uses basic ASCII-Range
      continue;
    recv_buf[iRecvMaxBufPos++] = (char)(chRead);
    if(iRecvMaxBufPos >= (RECV_BUF_COUNT - 1))  // buffer voll!
      break;
  }
  recv_buf[RECV_BUF_COUNT - 1] = 0; // nur zur Sicherheit...

#if defined DEBUG
  Serial.println();
  Serial.print("QRCODE:");
  Serial.println(recv_buf);
#endif

  // Empfangspuffer analysieren:
  //    loco.owner=MZ&loco.name=ETA 180 015 a+b, ETA 180 016 a+b&loco.address=680&loco.steps=128
  //    http://192.168.4.1/index.html?loco.address=680&loco.direction=2&loco.longAddress=on&loco=1
  char *p(strstr(recv_buf, "o.s")); // part of 'loco.steps='
  if(p)
  {
    p += 7; // points to '='
    ui8DecStepsPos = (p - recv_buf);
  } // if(p)
  p = strstr(recv_buf, "o.a"); // part of 'loco.address='
  if(p)
  {
    p += 9; // points to '='
    ui8DecAddressPos = (p - recv_buf);
  } // if(p)
  p = strstr(recv_buf, "o.f");
  while(p) // part of 'loco.fxxx='
  {
    p += 3; // points to first number
    uint8_t ui8pos(p - recv_buf);
    uint8_t ui8fNr(0);
    while (true)
    {
      chRead = recv_buf[ui8pos++];
      if(chRead == '=')
      {
        if(ui8fNr < SV_FCT_NR_MAX)
        {
          chRead = recv_buf[ui8pos];
          if(chRead == '0')
            ui8FctArray[ui8fNr] = 1;  // set SV to 0x00
          if(chRead == '1')
            ui8FctArray[ui8fNr] = 2;  // set SV to 0xFF
#if defined DEBUG
          Serial.print('F');
          Serial.print(ui8fNr);
          Serial.print('=');
          Serial.println(ui8FctArray[ui8fNr]);
#endif        
          break;
        } // if(ui8fNr < SV_FCT_NR_MAX)
      } // if(chRead == '=')
      if((chRead >= '0') && (chRead <= '9'))
        ui8fNr = (ui8fNr * 10) + (chRead - '0');
    } // while (true)
    p = strstr(p, "o.f");
  } // while(p) // part of 'loco.fxxx='
#if defined FZ_QRCODE
  p = strstr(recv_buf, "o.n"); // part of 'loco.name='
  if(p)
  {
    p += 6; // points to '='
    ui8FzNumberPos = (p - recv_buf);
  } // if(p)
  p = strstr(recv_buf, "o.o"); // part of 'loco.owner='
  if(p)
  {
    p += 7; // points to '='
    ui8FzOwnerPos = (p - recv_buf);
  } // if(p)
  if(ui8FzOwnerPos)
    recv_buf[ui8FzOwnerPos++] = 0;
  if(ui8FzNumberPos)
    recv_buf[ui8FzNumberPos++] = 0;
#endif

  for (uint8_t i = 0; i < iRecvMaxBufPos; i++)
    if(recv_buf[i] == '&')
      recv_buf[i] = 0;

#if defined FZ_QRCODE
  lcd_owner_name(ui8FzOwnerPos ? recv_buf + ui8FzOwnerPos : NULL, ui8FzNumberPos ? recv_buf + ui8FzNumberPos : NULL);
#endif

  if(ui8DecAddressPos)
  {
    recv_buf[ui8DecAddressPos++] = 0;
#if defined DEBUG
    Serial.print("ADR:");
    Serial.println(recv_buf + ui8DecAddressPos);
#endif
    uint8_t ui8Zahl(0);
    for (uint8_t i = ui8DecAddressPos; i < iRecvMaxBufPos; i++)
    {
      chRead = recv_buf[i];
      if(!chRead)
        break;
      ui8Zahl = (ui8Zahl * 10) + (chRead - '0');
    }
    if((ui8Zahl > 0) && (ui8Zahl < 9999))
    {
      stelle = strlen(recv_buf + ui8DecAddressPos);
      zahl = ui8Zahl;
      AdresseSet();
    } // if((ui8Zahl > 0) && (ui8Zahl < 9999))
  } // if(ui8DecAddressPos)

  if(ui8DecStepsPos)
  {
    recv_buf[ui8DecStepsPos++] = 0;
#if defined DEBUG
    Serial.print("FS:");
    Serial.println(recv_buf + ui8DecStepsPos);
#endif
    fahrstufen = StringToStat1ValCount(recv_buf + ui8DecStepsPos);
    FahrstufenSet();
  } // if(ui8DecStepsPos)

  ui8FlagSendingDisptach = 0x01;
  bReadFromQRCode = true;
} // void HandleSerial()

#endif