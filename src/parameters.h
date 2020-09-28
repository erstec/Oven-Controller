#ifndef PARAMETERS_H
#define PARAMETERS_H

#define VERSION                     03          // v0.3
#define EEPROM_LAYOUT_VERSION       VERSION     // Increase together with version
#define AMOUNT_OF_INDEXES           2           // First idx just for take start of EEPROM
                                                // (because it was used before and it wear is high)
//#define INDEX_CONFIGURATION_VAR1  0           // unused
#define INDEX_CONFIGURATION_VAR2    1           // Store Setpoint here

#define RLY_OFF                 LOW
#define RLY_ON                  HIGH

#if defined(ARDUINO_AVR_UNO)

#define RELAY                   11      // GPIO PIN

#define BTTN_PLUS               2       // GPIO PIN
#define BTTN_MINUS              3       // GPIO PIN

#define THERM_DO                4       // GPIO PIN
#define THERM_CS                5       // GPIO PIN
#define THERM_CLK               6       // GPIO PIN

#define LED_CLK                 9       // GPIO PIN
#define LED_DIO                 10      // GPIO PIN

#define VCC_OUT                 7       // GPIO PIN
#define GND_OUT                 8       // GPIO PIN

#elif defined(ARDUINO_AVR_NANO)

#define RELAY                   11      // GPIO PIN

#define BTTN_PLUS               2       // GPIO PIN
#define BTTN_MINUS              3       // GPIO PIN

#define THERM_DO                4       // GPIO PIN
#define THERM_CS                5       // GPIO PIN
#define THERM_CLK               6       // GPIO PIN

#define LED_CLK                 9       // GPIO PIN
#define LED_DIO                 10      // GPIO PIN

#define VCC_OUT                 7       // GPIO PIN
#define GND_OUT                 8       // GPIO PIN

#endif

#define DEBOUNCE_TIME           5       // ms
#define LONG_PRESS_TIME         250     // ms

#define MAX6675PERIOD         250
#define FAILSAFE_TMO          5000
#define LEDPERIOD             500
#define SCREEN_UPDATE_PERIOD  100
#define SET_SCREEN_TMO        1000
#define STAT_SCREEN_PERIOD    5000
#define STAT_SCREEN_LEN       300

#define T_MIN           0
#define T_MAX           250
#define T_STEP          5

#endif
