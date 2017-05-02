# Dumps of AT89S52
Orignal code of The AT89S52 read via avrdude and the avrdude-at89252.conf

avrdude -C avrdude-at89s52.conf -c arduino -p at89s52 -P COM3 -U flash:r:flash.ihex:i -F

* flash.bin - raw binary firmware
* flash.ihex - intel hex
* flash.hx - hex 0x, format
* flash.hxo - hex with spaces
* flash.d52 - disassembled with d52.exe (Don't know if this is a proper representation, but SPI addresses and calls seems to match)

