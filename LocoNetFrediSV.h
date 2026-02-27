//=== LocoNetFrediSV for use with DISPA ===
#if defined FREDI_SV

  const uint8_t SV_READ_SINGLE_ANSWER(SV_CMD::SV_READ_SINGLE + PCMD_RW);  // 0x42
  const uint8_t SV_WRITE_QUAD_ANSWER(SV_CMD::SV_WRITE_QUAD + PCMD_RW);    // 0x45
  const uint8_t SV_DISCOVER_ANSWER(SV_CMD::SV_DISCOVER + PCMD_RW);        // 0x47

  extern uint16_t ui16_ThrottleId;
 
  //=== for compatibility with 'LocoNetE5ForDispa.ino'
  const uint8_t SV2_FORMAT_2(0x02);

  const uint8_t MAX_FREDISV(48);
  uint8_t FrediSvValues[MAX_FREDISV];

  void SetCVsToDefault()
  {
    for (uint8_t i = 0; i < MAX_FREDISV; i++)
      FrediSvValues[i] = 0;
  }
  
  void WriteCVtoEEPROM(uint8_t ui8_Index, uint16_t ui16_Value)
  {
    if (ui8_Index < MAX_FREDISV)
      FrediSvValues[ui8_Index] = (uint8_t)(ui16_Value);
  }

// ===end=== for compatibility with 'LocoNetE5ForDispa.ino'

  const uint8_t GetSV(uint8_t ui8_Index)
  {
    if (ui8_Index < MAX_FREDISV)
      return FrediSvValues[ui8_Index];
    return 0;
  }

#endif
