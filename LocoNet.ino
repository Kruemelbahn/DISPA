//=== LocoNet for Dispa ===
#include <LocoNet.h>

//=== declaration of var's 
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
// ED is already used  and defined as OPC_IMM_PACKET
// EE is already used  and defined as OPC_IMM_PACKET_2

#define   TX_PIN   7

//=== keep informations for telegram-sequences ===================

//=== functions for receiving telegrams from SerialMonitor =======
// Eingabe Byteweise
// jedes Byte besteht aus zwei Hex-Zahlen
// Groß/Klein spielt keine Rolle
// Leerzeichen zwischen zwei Bytes erforderlich
// Prüfsumme wird am Ende automatisch ermittelt und hinzugefügt
#if defined TELEGRAM_FROM_SERIAL
static const int MAX_LEN_LNBUF = 64;
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

				case OPC_MOVE_SLOTS:	// 0xBA
#if defined DEBUG
          Serial.print("LnPacket->data[2]:");
          Serial.println(LnPacket->data[2]);
          Serial.print("slot:");
          Serial.println(slot);
#endif
					if (LnPacket->data[2] == slot)
					{
						if (slot != 0x00)
						{
							SlotTabelle[slot].ucSTAT |= (1 << 5) | (1 << 4);
							Slot_SL_RD(slot);
						}
						else
							Slot_SL_RD(2);
					}
					if ((slot != 0x00) && (LnPacket->data[2] == 0x00))
					{
						SlotTabelle[slot].ucSTAT &= ~(1 << 4);
						SlotTabelle[slot].ucSTAT |= (1 << 5);
					}					
					break;

				case OPC_WR_SL_DATA:	// 0xEF
					write_SL_Data(LnPacket->sd);
					break;

				case OPC_LONG_ACK:		// 0xB4
				case OPC_SL_RD_DATA:	// 0xE7
					break;  // ignore our own replies silently

				default:  // ignore packets that we don't want to handle
					break;
			} // switch (LnPacket->data[0])
		}

  } // if(LnPacket)
}

char MasterReplyLAck(uint8_t ucRequestOpc, uint8_t ucParam)
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
    return LocoNet.send(LnPacket);  // Send the packet to the LocoNet
  }
  return 0;
}

void Slot_SL_RD(uint8_t slot)
{
  uint16_t ui16LocoAddress(SlotTabelle[slot].ucADR  + (SlotTabelle[slot].ucADR2 >> 7));
  if((!bShowFrediSV && !ui16LocoAddress) || (slot >= SLOTMAX))
  {
    MasterReplyLAck(0x3A, 0x00);
    return;
  }

  uint8_t ui8_ChkSum(OPC_SL_RD_DATA ^ 0x0e ^ slot ^ SlotTabelle[slot].ucSTAT ^ SlotTabelle[slot].ucADR ^ SlotTabelle[slot].ucSPD ^ SlotTabelle[slot].ucDIRF ^ 0x05 ^ SlotTabelle[slot].ucSS2 ^ SlotTabelle[slot].ucADR2 ^ SlotTabelle[slot].ucSND ^ SlotTabelle[slot].ucID1 ^ SlotTabelle[slot].ucID2 ^ 0xFF);  //XOR
  bitWrite(ui8_ChkSum, 7, 0);     // set MSB zero

	addByteLnBuf( &LnTxBuffer, OPC_SL_RD_DATA); //0xE7
	addByteLnBuf( &LnTxBuffer, 0x0e);
	addByteLnBuf( &LnTxBuffer, slot);
	addByteLnBuf( &LnTxBuffer, SlotTabelle[slot].ucSTAT);
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

	MasterReplyLAck(0x6f, 0x7f);
	adr_lcd(slot);
}

#if defined DEBUG || defined TELEGRAM_FROM_SERIAL
void Printout(char ch)
{
  if(LnPacket)
  {
    // print out the packet in HEX
    Serial.print(ch);
    Serial.print("X: ");
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
