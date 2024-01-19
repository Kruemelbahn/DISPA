
  extern boolean bShowFrediSV;
  extern uint16_t ui16_ThrottleId;

#if defined FREDI_SV
  
  // for compatibility with 'LocoNetE5ForDispa.ino'
  #define ENABLE_LN_E5          (1)
  #define SV2_Format_2	        0x02

  #define ID_DEVICE   0
  #define SOFTWARE_ID 7

  const uint8_t MAX_FREDISV = 48;
  uint8_t FrediSvValues[MAX_FREDISV];

  const uint8_t GetCV(uint8_t ui8_Index)
  {
    if(ui8_Index == ID_DEVICE) // throttle-id low
      return (uint8_t)(ui16_ThrottleId & 0xFF); 
    if(ui8_Index == SOFTWARE_ID) // throttle-id high
      return (uint8_t)(ui16_ThrottleId >> 8);
    return 0;
  }

  const uint8_t GetCVCount() { return MAX_FREDISV; }
  void SetCVsToDefault()
  {
    for (uint8_t i = 0; i < GetCVCount(); i++)
      FrediSvValues[i] = 0;
  }
  
  void WriteCVtoEEPROM(uint8_t ui8_Index, uint16_t ui16_Value)
  {
    if (ui8_Index < GetCVCount())
      FrediSvValues[ui8_Index] = (uint8_t)(ui16_Value);
  }

// ===end=== for compatibility with 'LocoNetE5.ino'

  const uint8_t GetSV(uint8_t ui8_Index)
  {
    if (ui8_Index < GetCVCount())
      return FrediSvValues[ui8_Index];
    return 0;
  }

#endif
