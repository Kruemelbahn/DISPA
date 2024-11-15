# DISPA

was developed in FREMO to assign a vehicle to a LocoNET-throttle independently of a DCC command station.

In Dispa.ino you can select whether the software should be compiled for usage with LCD or OLED.

DispaSetup is a small utility for testing OLED and keyboard.

## since Version 2.0
- developed for the usage on an Arduino-UNO or a compatible board.

## new in Version 2.1
- supports the reading of Parameters from FREDI using the LNSV2-protocol (OLED is requested then).

## new in Version 2.2
- supports the reading of Parameters from FREDI using the LNSV2-protocol either with OLED or LCD.

## new in Version 2.3
- supports the reading of Parameter SV8...10 from FREDI using the LNSV2-protocol either with OLED or LCD.
- New testpage for showing speed, direction and state of function F0...F16 of the connected FREDI.

## new in Version 2.4
- supports the reading of Parameter SV18...34 from FREDI using the LNSV2-protocol either with OLED or LCD.
- supports QR-Code-Reader and direct transfering QR-Code-data to the FREDI, please refer to chapter 2.4 "QR-Code-Leser" in the [manual](Documentation/Dispa.pdf).

## new in Version 2.5
- Bugfix for vehicleaddress reading from QR-Code.
- New: Transfer Decodersteps to FREDI without valid vehicleaddress.

## new in Version 2.6
- Bugfix for short vehicleaddress reading from QR-Code and transfer to FREDI.

### Requested libraries
DISPA requires my libraries listed below in addition to various Arduino standard libraries:<br> 
- [HeartBeat](https://www.github.com/Kruemelbahn/HeartBeat)<br>
- [LCDPanel](https://www.github.com/Kruemelbahn/LCDPanel)<br>
- [OLEDPanel](https://www.github.com/Kruemelbahn/OLEDPanel)<br>

### original files and schematic
The original files developed by FREMO for DISPA can be found here:<br>
https://sourceforge.net/p/embeddedloconet/svn/HEAD/tree/old-cvs/trunk/apps/Dispa/<br>
the original schematic developed by FREMO for DISPA can be found here:<br>
https://sourceforge.net/p/fremodcc/svn/HEAD/tree/Dispa/trunk/<br>
