# DISPA

was developed in FREMO to assign a vehicle to a LocoNET-throttle independently of a DCC command station. 

Version 2.0 (and higher) was developed for the usage on an Arduino-UNO or a compatible board.<br>

Version 2.1 supports the reading of Parameters from FREDI using the LNSV2-protocol (OLED is requested then).

Version 2.2 supports the reading of Parameters from FREDI using the LNSV2-protocol either with OLED or LCD.

Version 2.3 supports the reading of Parameter SV8...10 from FREDI using the LNSV2-protocol either with OLED or LCD.
            new testpage for showing speed, direction and state of function F0...F16 of the connected FREDI

In Dispa.ino you can select whether the software should be compiled<br> 
- for usage with LCD or OLED.<br>
- with or without reading FREDI-SVs including a testpage.<br>
  For the meaning of SVs please refer to the footnote in chapter 2.3 "FREDI-Diagnose" in the [manual](Documentation/Dispa.pdf)<br>

### Requested libraries
DISPA requires my libraries listed below in addition to various Arduino standard libraries:<br> 
- [HeartBeat](https://www.github.com/Kruemelbahn/HeartBeat)<br>
- [LCDPanel](https://www.github.com/Kruemelbahn/LCDPanel)<br>
- [OLEDPanel](https://www.github.com/Kruemelbahn/OLEDPanel)<br>

### original files and schematic
The original files developed by FREMO for DISPA can be found here:<br>
https://sourceforge.net/p/embeddedloconet/svn/HEAD/tree/old-cvs/trunk/apps/Dispa/<br>
the original schematic developed by FREMO for DISPA can be found here:<br>
https://sourceforge.net/p/fremodcc/svn/HEAD/tree/Dispa/trunk/
