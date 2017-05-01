# Logic Dumps of running EMT707CTL

These where made using [Sigrok](https://sigrok.org/)s PulseView and 16Channel Cypress 2 board. It took quite some time to figure out that settings for SPI needed to be Mode2 to get correct registyr values - so for a while I started looking at other Datasheets instead of the ADE7753.

* [EMT707CTL.sr](EMT707CTL.sr) Is normal startup
  * ![First EEPROM READ](../Images/EMT707CTL_FirstEEPROM_Read.png)
  * ![ADE7753 Init](../Images/EMT707CTL_ADE7753_Init.png)
  * ![ADE7753 ZX Data](../Images/EMT707CTL_ADE7753_Data.png)
  * ![ADE7753 LCD](../Images/EMT707CTL_ADE7753_LCD.png)
  * ![ADE7753 EEPROM](../Images/EMT707CTL_ADE7753_EEPROM2.png)
* [EMT707CTL_reset.sr](EMT707CTL_reset.sr) Is when pressing reset a couple of times
* [EMT707CTL_Menu.sr](EMT707CTL_Menu.sr) Is when pressing menu button
  * ![Image Menu logic](../Images/EMT707CTL_Menu.png) 
  * The goal with this dump is to try and figure out how the LCD protocol works

The actuall values in some of these dumps seems to have been affected by logic probe, They should not affect relative between SPI read and LCD output, but who knows.