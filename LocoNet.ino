//=== LocoNet for Dispa ===
#include <LocoNet.h>

//=== declaration of var's
const uint8_t DEFAULT_LOCO_ADDRESS(3);
const uint8_t SRC_E5(0);

extern const uint8_t SLOTMAX;
extern SSlotData_t SlotTabelle[SLOTMAX];

static  lnMsg   *LnPacket;
static  LnBuf   LnTxBuffer;

/*  
 *  http://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf
 *  http://wiki.rocrail.net/doku.php?id=loconet:ln-pe-de
 */
// default OPC's are defined in 'utility\ln_opc.h'
// A3 is already used  and defined as OPC_LOCO_F912
// A8 is already used by FRED  and defined as OPC_DEBUG
// A8 is already used by FREDI and defined as OPC_FRED_BUTTON
// AF is already used by FREDI and defined as OPC_SELFTEST resp. as OPC_FRED_ADC
// D4 is already used  and defined as OPC_UHLI_FUN
// ED is already used  and defined as OPC_IMM_PACKET
// EE is already used  and defined as OPC_IMM_PACKET_2

const uint8_t TX_PIN(7);

//=== keep informations for telegram-sequences ===================

#if defined FREDI_SV
  #if not defined OPC_LOCO_F912
    #define OPC_LOCO_F912       0xA3
  #endif
  #if not defined OPC_UHLI_FUN
    #define OPC_UHLI_FUN        0xD4
  #endif
#endif

//=== functions for receiving telegrams from SerialMonitor =======
// Eingabe Byteweise
// jedes Byte besteht aus zwei Hex-Zahlen
// Groß/Klein spielt keine Rolle
// Leerzeichen zwischen zwei Bytes erforderlich
// Prüfsumme wird am Ende automatisch ermittelt und hinzugefügt
#if defined TELEGRAM_FROM_SERIAL
static const uint8_t MAX_LEN_LNBUF = 64;
uint8_t ui8_PointToBuffer = 0;

uint8_t ui8a_receiveBuffer[MAX_LEN_LNBUF];
uint8_t ui8a_resultBuffer[MAX_LEN_LNBUF];

void ClearReceiveBuffer()
{
  ui8_PointToBuffer = 0;
  for(uint8_t i = 0; i < MAX_LEN_LNBUF; i++)
    ui8a_receiveBuffer[i] = 0;
}

uint8_t Adjust(uint8_t ui8_In)
{
  uint8_t i = ui8_In;
  if((i >= 48) && (i <= 57))
    i -= 48;
  else
    if((i >= 65) && (i <= 70))
      i -= 55;
    else
      if((i >= 97) && (i <= 102))
        i -= 87;
      else
        i = 0;
  return i;
}

uint8_t AnalyzeBuffer()
{
  uint8_t i = 0;
  uint8_t ui8_resBufSize = 0;
  while(uint8_t j = ui8a_receiveBuffer[i++])
  {
    if(j != ' ')
    {
      j = Adjust(j);
      uint8_t k = ui8a_receiveBuffer[i++];
      if(k != ' ')
        ui8a_resultBuffer[ui8_resBufSize++] = j * 16 + Adjust(k);
      else
        ui8a_resultBuffer[ui8_resBufSize++] = j;
    }
  }

  // add checksum:
  uint8_t ui8_checkSum(0xFF);
  for(i = 0; i < ui8_resBufSize; i++)
    ui8_checkSum ^= ui8a_resultBuffer[i]; 
  bitWrite(ui8_checkSum, 7, 0);     // set MSB zero
  ui8a_resultBuffer[ui8_resBufSize++] = ui8_checkSum;

  return ui8_resBufSize;
}

#endif

//=== functions ==================================================
void InitLocoNet()
{
  // First initialize the LocoNet interface, specifying the TX Pin
  LocoNet.init(TX_PIN);

#if defined TELEGRAM_FROM_SERIAL
  ClearReceiveBuffer();
#endif
}

void HandleLocoNetMessages()
{
#if defined TELEGRAM_FROM_SERIAL
  LnPacket = NULL;
  if (Serial.available())
  {
    byte byte_Read = Serial.read();
    if(byte_Read == '\n')
    {
      if(AnalyzeBuffer() > 0)
        LnPacket = (lnMsg*) (&ui8a_resultBuffer);
      ClearReceiveBuffer();
    }
    else
    {
      if(ui8_PointToBuffer < MAX_LEN_LNBUF)
        ui8a_receiveBuffer[ui8_PointToBuffer++] = byte_Read;
    }
  }
#else

  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive();
#endif
  if(LnPacket)
  {
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
		Printout('R');
#endif
#if defined FREDI_SV
    /*
      FREDI-Diagnostic:
      receive speed- and function-telegrams and keep their values for display - neither more nor less
      thus the values 'slot', 'locoaddress' and 'status' are not used and therefore ignored
      messages below are without any response
    */
    switch (LnPacket->data[0])
    {
      case OPC_LOCO_SPD:  // 0xA0
        ui8_currentLocoSpeed = LnPacket->data[2];
        break;
      case OPC_LOCO_DIRF: // 0xA1
        ui8_currentFunctions[0] = LnPacket->data[2];
        break;
      case OPC_LOCO_SND:  // 0xA2
        ui8_currentFunctions[1] = LnPacket->data[2]; 
         break;
      case OPC_LOCO_F912: // 0xA3
        ui8_currentFunctions[2] = LnPacket->data[2]; 
        break;
      case OPC_UHLI_FUN:  // 0xD4
        if((LnPacket->data[1] == 0x20) && (LnPacket->data[3] == 0x08))
          ui8_currentFunctions[3] = LnPacket->data[4]; // Arg4
        break;
      case OPC_IMM_PACKET:  // 0xED
        if((LnPacket->data[1] == 0x0B) && (LnPacket->data[3] == 0x7F))
        {
          if((LnPacket->data[3] == 0x34) && (LnPacket->data[6] == 0x5E)) // short address
            ui8_currentFunctions[3] = LnPacket->data[7]; // IM3
          if((LnPacket->data[3] == 0x44) && (LnPacket->data[7] == 0x5E)) // long address
            ui8_currentFunctions[3] = LnPacket->data[8]; // IM4
        }
        break;
      default:  // ignore packets that we don't want to handle
        break;
    }
#endif
#if defined QRCODE
    /*
      FREDI will answer E5-telegram, initiated by QRCode-Reader:
    */
    switch (LnPacket->data[0])
    {
      case OPC_PEER_XFER:  // 0xE5
        if (LnPacket->data[4] == SV2_Format_2)  // telegram with Message-Format '2'
        {
          if (LnPacket->data[3] == 0x45)
          {
            // REPLY from SV write : compare D1...D4:
            if((LnPacket->data[8] == 8) && (LnPacket->data[9] == 0))
            { // answer contains current values SV8, 9, 10, 11
              const uint8_t ui8_value_D1(((LnPacket->data[10] & 0x01) << 7) + (LnPacket->data[11] & 0x7F));    // D1 = SV8
              const uint8_t ui8_value_D2(((LnPacket->data[10] & 0x02) << 6) + (LnPacket->data[12] & 0x7F));    // D2 = SV9
              const uint8_t ui8_value_D3(((LnPacket->data[10] & 0x04) << 5) + (LnPacket->data[13] & 0x7F));    // D3 = SV10
              const uint8_t ui8_value_D4(((LnPacket->data[10] & 0x08) << 4) + (LnPacket->data[14] & 0x7F));    // D4 = SV11
              if(    (ui8_value_D1 != SlotTabelle[2].ucADR)   // adrLow
                  || (ui8_value_D2 != SlotTabelle[2].ucADR2)  // adrHigh
                  || (ui8_value_D3 != statTable[fahrstufen].stat1Val)
                  || (ui8_value_D4 != SKIP_SELF_TEST) )
                ui8FlagSendingDisptach = 0x80; // error!
              else
                ui8FlagSendingDisptach = 0x03; // success
            } // if((LnPacket->data[8] == 8) && (LnPacket->data[9] == 0))
          } // if (LnPacket->data[3] == 0x45)
        } // if (LnPacket->data[4] == SV2_Format_2)  // telegram with Message-Format '2'
        break;
      default:  // ignore packets that we don't want to handle
        break;
    }
#endif
		uint8_t slot(LnPacket->data[1]);
		if (slot < SLOTMAX)
		{
			switch (LnPacket->data[0])
			{
				case OPC_LOCO_ADR:		// 0xBF
					SlotManager(LnPacket->data[2], slot);
					break;

				case OPC_RQ_SL_DATA:	// 0xBB
					Slot_SL_RD(slot);
					break;

				case OPC_MOVE_SLOTS:	// 0xBA = 'Dispatch Get' vom FRED
#if defined DEBUG
          Serial.print(F("LnPacket->data[2]:"));
          Serial.println(LnPacket->data[2]);
          Serial.print(F("slot:"));
          Serial.println(slot);
#endif
					if (LnPacket->data[2] == slot)
					{
						if (slot != 0x00)
						{
							SlotTabelle[slot].ucSTAT |= (1 << 5) | (1 << 4); // set busy and active
							Slot_SL_RD(slot);
						}
						else
							Slot_SL_RD(2);  // 'BA 00 00 45' received -> dispatch = sendet 'Slot Read' als Antwort der Zentrale
					}
					if ((slot != 0x00) && (LnPacket->data[2] == 0x00))
					{
            // 'BA xx 00 <chk>' received -> undispatch
						SlotTabelle[slot].ucSTAT &= ~(1 << 4);  // set inactive
						SlotTabelle[slot].ucSTAT |= (1 << 5);   // set busy
					}					
					break;

        case OPC_PEER_XFER:  // 0xE5
          if (LnPacket->data[4] == SV2_Format_2)  // telegram with Message-Format '2'
          {
            if (LnPacket->data[3] == 0x47)
            {
              // REPLY from Discover :
              const uint8_t ui8_value_D3(((LnPacket->data[10] & 0x04) << 5) + (LnPacket->data[13] & 0x7F));    // D3 = ThrotlleId-low
              const uint8_t ui8_value_D4(((LnPacket->data[10] & 0x08) << 4) + (LnPacket->data[14] & 0x7F));    // D4 = ThrotlleId-high
              ui16_ThrottleId = (ui8_value_D4 << 8) + ui8_value_D3;
            } // if (LnPacket->data[3] == 0x47)
          } // if (LnPacket->data[4] == SV2_Format_2)  // telegram with Message-Format '2'
          break;

				case OPC_WR_SL_DATA:	// 0xEF = 'Write Slot' vom FRED als Antwort auf 'Slot Read' der Zentrale
					write_SL_Data(LnPacket->sd);  // speichert die Daten vom FRED und sendet ein LACK (B4 6F 00 5B)
					break;

				case OPC_LONG_ACK:		// 0xB4
				case OPC_SL_RD_DATA:	// 0xE7
					break;  // ignore our own replies silently
				default:  // ignore packets that we don't want to handle
					break;
			} // switch (LnPacket->data[0])
		} // if (slot < SLOTMAX)

  } // if(LnPacket)
}

void MasterReplyLack(uint8_t ucRequestOpc, uint8_t ucParam)
{
	ucRequestOpc &= 0x7F;
	ucParam      &= 0x7F;

  // calculate checksum:
  uint8_t ui8_ChkSum(OPC_LONG_ACK ^ ucRequestOpc ^ ucParam ^ 0xFF);  //XOR
  bitWrite(ui8_ChkSum, 7, 0);     // set MSB zero

  addByteLnBuf( &LnTxBuffer, OPC_LONG_ACK); //0xB4
  addByteLnBuf( &LnTxBuffer, ucRequestOpc); //1. Data Byte
  addByteLnBuf( &LnTxBuffer, ucParam);      //2. Data Byte
  addByteLnBuf( &LnTxBuffer, ui8_ChkSum);   //Checksum
  addByteLnBuf( &LnTxBuffer, 0xFF);         //Limiter

  // Check to see if we have received a complete packet yet
  LnPacket = recvLnMsg( &LnTxBuffer );    //Prepare to send
  if(LnPacket)
  {
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
    Printout('T');
#endif
    LocoNet.send(LnPacket);  // Send the packet to the LocoNet
  }
}

void Slot_SL_RD(uint8_t slot)
{
  uint16_t ui16LocoAddress(SlotTabelle[slot].ucADR  + (SlotTabelle[slot].ucADR2 >> 7));
  if((!bShowFrediSV && !ui16LocoAddress) || (slot >= SLOTMAX))
  {
    MasterReplyLack(0x3A, 0x00);
    return;
  }
  if(bShowFrediSV && !ui16LocoAddress)
    SlotTabelle[slot].ucADR = DEFAULT_LOCO_ADDRESS;  // to have a valid locoaddress in case of dispatch

  uint8_t ui8_ChkSum(OPC_SL_RD_DATA ^ 0x0e ^ slot ^ SlotTabelle[slot].ucSTAT ^ SlotTabelle[slot].ucADR ^ SlotTabelle[slot].ucSPD ^ SlotTabelle[slot].ucDIRF ^ 0x05 ^ SlotTabelle[slot].ucSS2 ^ SlotTabelle[slot].ucADR2 ^ SlotTabelle[slot].ucSND ^ SlotTabelle[slot].ucID1 ^ SlotTabelle[slot].ucID2 ^ 0xFF);  //XOR
  bitWrite(ui8_ChkSum, 7, 0);     // set MSB zero

	addByteLnBuf( &LnTxBuffer, OPC_SL_RD_DATA); //0xE7
	addByteLnBuf( &LnTxBuffer, 0x0e);
	addByteLnBuf( &LnTxBuffer, slot);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucSTAT); // includes status and decodertype in lower 3 bits (.... .XXX)
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucADR);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucSPD);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucDIRF);
	addByteLnBuf( &LnTxBuffer, 0x05); //trk
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucSS2);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucADR2);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucSND);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucID1);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucID2);
	addByteLnBuf( &LnTxBuffer, ui8_ChkSum);   //Checksum
	addByteLnBuf( &LnTxBuffer, 0xFF);         //Limiter

	// Check to see if we have received a complete packet yet
  LnPacket = recvLnMsg( &LnTxBuffer );    //Prepare to send
  if(LnPacket)
  {
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
    Printout('T');
#endif
    LocoNet.send(LnPacket);  // Send the packet to the LocoNet
  }
}

void write_SL_Data(rwSlotDataMsg SL_Data)
{
	uint8_t slot(SL_Data.slot);
	if (slot >= SLOTMAX)
		return;

	SlotTabelle[slot].ucSTAT = SL_Data.stat;
	SlotTabelle[slot].ucADR  = SL_Data.adr;
	SlotTabelle[slot].ucSPD  = SL_Data.spd;
	SlotTabelle[slot].ucDIRF = SL_Data.dirf;
	SlotTabelle[slot].ucSS2  = SL_Data.ss2;
	SlotTabelle[slot].ucADR2 = SL_Data.adr2;
	SlotTabelle[slot].ucSND  = SL_Data.snd;
	SlotTabelle[slot].ucID1  = SL_Data.id1;
	SlotTabelle[slot].ucID2  = SL_Data.id2;

	MasterReplyLack(0x6f, 0x7f);
	adr_lcd(slot);
}

void send_request_for_ThrottleId()
{
  sendE5Telegram(SRC_E5 /*src*/, 0x07 /*cmd*/, 0x00 /*svx1*/,
                  0, 0,
                  0 /*ui8_sv_adrl*/, 0 /*ui8_sv_adrh*/,
                  0 /*D1*/, 0/*D2*/, 0/*D3*/, 0/*D4*/);
}

#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
void Printout(char ch)
{
  if(LnPacket)
  {
    // print out the packet in HEX
    Serial.print(ch);
    Serial.print(F("X: "));
    uint8_t ui8_msgLen = getLnMsgSize(LnPacket); 
    for (uint8_t i = 0; i < ui8_msgLen; i++)
    {
      uint8_t ui8_val = LnPacket->data[i];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if(ui8_val < 16)
        Serial.print('0');
      Serial.print(ui8_val, HEX);
      Serial.print(' ');
    }
    Serial.println();
  } // if(LnPacket)
}
#endif
