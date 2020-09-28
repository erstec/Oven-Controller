# Oven-Controller
## **Arduino based Electric Oven Temperatre Controller with LED display**

## Idea was to make analog controlled electrical oven more convenient to use and add more precise digital temperature control to it

# Features
- Temperature range 0 - 250 C (step 5 C)
- Watchdog protection
- Runaway protection: if during 5 seconds there is no data from temperature sensor or sensor error present (sensor disconnected) - system turns relay off and halts, displaying `Err`
- Display periodically switches between current temperature (long time, witch `C` symbol) and setpoint (short time, with three-dashes symbol)
- Setpoint are stored in internal EEPROM with EEPROM Wear Leveling enabled

# Usage
- Single press - step up/down by 5 degrees
- Long press - fast run up/down until button released
- Double press - cycle between 180 - 200 - 220 degreees

# Compiling and Flashing
* Build with VS Code and PlatformIO extension
* Arduino Uno and Arduino Nano (with ATMega328) targets present
* Flash via standard Arduino board USB port (no need to use AVR programmer)
* All pins for connecting Buttons, LED display, Relay and Thermosensor interface are declared in `src/parameters.h`
* Currently code shows Celsius degrees, but can be easy changed to use Fahrenheits

# Used hardware
- Arduino Uno or Nano board
- Two NO (normal opened) push buttons, for controlling Temperature Setpoint (PLUS and MINUS) (https://www.aliexpress.com/w/wholesale-push-button-normally-opened.html)
- MAX6675 Thermocouple interface with K-Type Thermocouple (https://www.aliexpress.com/w/wholesale-max6675.html)
- TM1637 Segment LED Display (https://www.aliexpress.com/w/wholesale-tm1637.html)
- High Current Relay Module (https://www.aliexpress.com/w/wholesale-sla-05vdc-module.html)
- 230V AC to 5V DC power supply adapter with 5.5/2.1mm plug (for Arduino UNO). For Arduino Nano use powering via according pins.
- Connecting cables (https://www.aliexpress.com/w/wholesale-male-female-dupont-cable.html)

NOTE: Arduino PIN 7 is configured as VCC and PIN 8 as GND for MAX6675 module. This used just to simplify connection.

# Default Connections

| Signal Name | Arduino PIN |
| --- | --- |
| RELAY | 11 |
| PLUS Button | 2 |
| MINUS Button | 3 |
| MAX6675 DO | 4 |
| MAX6675 CS | 5 |
| MAX6675 CLK | 6 |
| MAX6675 VCC | 7 |
| MAX6675 GND | 8 |
| TM1637 LED DIO | 10 |
| TM1637 LED CLK | 9 |

# NOTES
- COM and NO terminals of Relay should be connected to stock temperature dial regulator wires, when stock regulator should be totally disconnected.
- Power supply can be connected to stock timer output terminals. It will work as default and used to power on oven and set time (mechanically).
- Thermocouple sensor should be placed/mounted to drilled hole on the side of oven chamber, approx 10-20% of height from top.
- LED Display and Buttons can be placed outside or mounted to stock front panel (make holes for them).

**WARNING:** **DO NOT** place Arduino, Relay or Power supply inside ovens case. Use custom case outside, 3D printed or universal. You can easily hide it behind back of oven. If stock wires is too short - use custom, connected with Wago connectors or use crimping tools. DO NOT SOLDER WIRES!
