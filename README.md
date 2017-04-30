# This is my documentation and projects for reverse enginering
[Coltech EMT707CTL From Clas Ohlson artid: 36-2897](https://www.clasohlson.com/se/Elenergim%C3%A4tare/36-2897)

[Some discussions I don't understand in Finnish](http://lampopumput.info/foorumi/index.php?topic=15290.15)

To open this one up you need a 3 pointed "Nintendo" screwdriver

### Chips
* LM7805 5Volt standard regulator
* 0053 Rebranded [ADE7753](http://www.analog.com/media/en/technical-documentation/data-sheets/ADE7753.pdf) with 3.579545MHz crystal
* [Atmel AT89S52 24AU](http://www.atmel.com/images/doc1919.pdf) with 12MHz crystal
  * Reads from EEPROM
  * communicates with ADE7753
  * Reads buttons
  * Sends LCD updates to PIC1621B
* 24LC03 I2C EEPROM for storing cost, power usage and probably calibration data
* PIC1621B - Only Used as LCD driver
  * Uses separate Reset, Clock and Data lines (10 bit?) from AT89S52

### Headers
#### ISP for AT89S52
Name | Description
---- | -----------
CLK | Program Clock
GND |
MISO | Program MISO
VCC | 5 Volt
RST | Reset
MOSI | Program MOSI

#### CON Between CPU board(A) and Meter board(B)

Pin | Name | Description
--- | ---- | -----------
1 | VCC | 5Volt from LM7805
2 | GND |
3 | PWCHK | Goes high when power is stable
4 | CF | ADE7753 CF Pin
5 | ZERO | Toggles with line freq. see ADE7753 ZX Pin
6 | DIN | SPI MOSI
7 | DOUT | SPI MISO
8 | SCLK | SPI Clock
9 | CS | SPI ADE7753 Select
