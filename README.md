# DISPA

was developed in FREMO to assign a vehicle to a LocoNET-throttle independently of a DCC command station. 

For the next development steps on my side I copied the essential files and worked on them.
<hr>
Step 1 (v2.0, done): modified the source for usage with Arduino-UNO or a compatible board. In Dispa.ino you can select whether the software should be compiled for an LCD or OLED.<br>
Step 2 (v2.1, in progress): support of the reading of parameters from FREDI using the LNSV2-protocol (OLED is required then).

### Requested libraries
DISPA requires my libraries listed below in addition to various Arduino standard libraries:<br> 
- [HeartBeat](https://www.github.com/Kruemelbahn/HeartBeat)
- [LCDPanel](https://www.github.com/Kruemelbahn/LCDPanel)
- [OLEDPanel](https://www.github.com/Kruemelbahn/OLEDPanel)

### original files
- the original files developed by FREMO for DISPA can be found here: https://sourceforge.net/p/embeddedloconet/svn/HEAD/tree/old-cvs/trunk/apps/Dispa/
- the original schematics can be found here: https://sourceforge.net/p/fremodcc/svn/HEAD/tree/Dispa/trunk/
