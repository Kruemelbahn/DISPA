# DISPA

was developed in FREMO to assign a vehicle to a LocoNET-throttle independently of a DCC command station. 

Version 2.0 (and higher) was developed for the usage on an Arduino-UNO or a compatible board.<br>
In Dispa.ino you can select whether the software should be compiled for an LCD or OLED.<br>

Version 2.1 supports the reading of Parameters from FREDI using the LNSV2-protocol (OLED is requested then).
In Dispa.ino you can select whether the software should be compiled with or without reading SVs.<br>
For usage of reading Parameters plese refer to chapter 2.3 "FREDI-Diagnose" in the manual [manual](Dispa.pdf)<br>

### Requested libraries
DISPA requires my libraries listed below in addition to various Arduino standard libraries:<br> 
- [HeartBeat](https://www.github.com/Kruemelbahn/HeartBeat)<br>
- [LCDPanel](https://www.github.com/Kruemelbahn/LCDPanel)<br>
- [OLEDPanel](https://www.github.com/Kruemelbahn/OLEDPanel)<br>

### original files
The original files developed by FREMO for DISPA can be found here:<br>
https://sourceforge.net/p/embeddedloconet/svn/HEAD/tree/old-cvs/trunk/apps/Dispa/<br>
the original schematics can be found here:<br>
https://sourceforge.net/p/fremodcc/svn/HEAD/tree/Dispa/trunk/
