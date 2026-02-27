//=== LocoNet support for E5-Telegram for DISPA ==================
//=== partially taken from:
//=== LocoNet support for E5-Telegram === usable for all =========
#include <LocoNet.h>

//=== functions ==================================================
boolean HandleE5MessageForFrediSv()
{
  if (LnPacket->data[4] == SV2_FORMAT_2)  // telegram with Message-Format '2'
  {
    // SV_ADRL
    uint8_t ui8_LSBAdr(((LnPacket->data[5] & 0x04) << 5) + (LnPacket->data[8] & 0x7F));
    if (LnPacket->data[3] == SV_READ_SINGLE_ANSWER)
    {
      // REPLY from SV read : store response D1 in SV-Table:
      uint8_t ui8_valueLSB(((LnPacket->data[10] & 0x01) << 7) + (LnPacket->data[11] & 0x7F));    // D1
      WriteCVtoEEPROM(ui8_LSBAdr, ui8_valueLSB);
      return true;
    } // if (LnPacket->data[3] == SV_READ_SINGLE_ANSWER)
  } // if (LnPacket->data[4] == SV2_FORMAT_2)  // telegram with Message-Format '2'
  return false;
}

boolean sendE5Telegram(uint8_t src, uint8_t cmd, uint8_t svx1, uint8_t dst_l, uint8_t dst_h, uint8_t ui8_sv_adrl, uint8_t ui8_sv_adrh, uint8_t ui8_D1, uint8_t ui8_D2, uint8_t ui8_D3, uint8_t ui8_D4)
{
  svx1 |= 0b00010000;
  uint8_t adrl(ui8_sv_adrl & 0x7F);
  if (ui8_sv_adrl > 0x7F)
    svx1 |= 0b00000100;
  uint8_t adrh(ui8_sv_adrh & 0x7F);
  if (ui8_sv_adrh > 0x7F)
    svx1 |= 0b00001000;

  uint8_t SVX2(0b00010000);

  uint8_t D1(ui8_D1 & 0x7F);
  if (ui8_D1 > 0x7F)
    SVX2 |= 0b00000001;
  uint8_t D2(ui8_D2 & 0x7F);
  if (ui8_D2 > 0x7F)
    SVX2 |= 0b00000010;
  uint8_t D3(ui8_D3 & 0x7F);
  if (ui8_D3 > 0x7F)
    SVX2 |= 0b00000100;
  uint8_t D4(ui8_D4 & 0x7F);
  if (ui8_D4 > 0x7F)
    SVX2 |= 0b00001000;

  // calculate checksum:
  uint8_t ui8_ChkSum(OPC_PEER_XFER ^ 16 ^ src ^ cmd ^ SV2_FORMAT_2 ^ svx1 ^ dst_l ^ dst_h ^ adrl ^ adrh ^ SVX2 ^ D1 ^ D2 ^ D3 ^ D4 ^ 0xFF);  //XOR
  bitWrite(ui8_ChkSum, 7, 0);     // set MSB zero

  addByteLnBuf(&LnTxBuffer, OPC_PEER_XFER);				// opcode E5
  addByteLnBuf(&LnTxBuffer, 16);									// length
  addByteLnBuf(&LnTxBuffer, src);		              // src
  addByteLnBuf(&LnTxBuffer, cmd);		              // cmd
  addByteLnBuf(&LnTxBuffer, SV2_FORMAT_2);				// sv-type
  addByteLnBuf(&LnTxBuffer, svx1);								// svx1
  addByteLnBuf(&LnTxBuffer, dst_l);		            // dst_l    
  addByteLnBuf(&LnTxBuffer, dst_h);		            // dst_h
  addByteLnBuf(&LnTxBuffer, adrl);					      // sv_adrl    
  addByteLnBuf(&LnTxBuffer, adrh);					      // sv_adrh
  addByteLnBuf(&LnTxBuffer, SVX2);								// svx2
  addByteLnBuf(&LnTxBuffer, D1);									// D1
  addByteLnBuf(&LnTxBuffer, D2);									// D2
  addByteLnBuf(&LnTxBuffer, D3);									// D3
  addByteLnBuf(&LnTxBuffer, D4);									// D4
  addByteLnBuf(&LnTxBuffer, ui8_ChkSum);					// Checksum
  addByteLnBuf(&LnTxBuffer, 0xFF);								// Limiter

  { // send packet
    // Check to see if we have build a complete packet yet
    LnPacket = recvLnMsg(&LnTxBuffer);    // Prepare to send
    if (LnPacket)
    { // check correctness
      LocoNet.send(LnPacket);  // Send the packet to the LocoNet
#if defined DEBUG || defined TELEGRAM_FROM_SERIAL 
      Printout('T');
#endif
    }
    return true;
  }
}
