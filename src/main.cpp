#include <Arduino.h>
#include "../lib/MAX6675/max6675.h"
#include "../lib/TM1637/TM1637Display.h"
#include "../lib/Watchdog/Watchdog.h"
#include "parameters.h"

#include "elapsedMillis.h"
#include "EEPROMWearLevel.h"
#include "Button2.h"

// ------------- GPIO --------------------------
Button2 buttonPlus = Button2(BTTN_PLUS);
Button2 buttonMinus = Button2(BTTN_MINUS);

// ------------- WATCHDOG ----------------------
Watchdog watchdog;

// ------------- THERMOCOUPLE ------------------
MAX6675 thermocouple(THERM_CLK, THERM_CS, THERM_DO);

// ------------- LED SCREEN ---------------------
const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

const uint8_t not_available[] = {
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
  SEG_G,  // -
};

// Create degree Celsius symbol:
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Circle
  // SEG_A | SEG_D | SEG_E | SEG_F   // C
};

// Create SET symbol:
const uint8_t set_sym[] = {
  SEG_A | SEG_G | SEG_D,
};

// Create Err symbol:
const uint8_t err_sym[] = {
  SEG_A | SEG_F | SEG_G | SEG_E | SEG_D,
  SEG_G | SEG_E,
  SEG_G | SEG_E,
};

// Create uderscore '_' symbol:
const uint8_t underscore_sym[] = {
  SEG_D,
};

TM1637Display display(LED_CLK, LED_DIO);

// --------------------------------------------------

float Setpoint = 20.0;
uint8_t Output = RLY_OFF;

static double setP = Setpoint;

elapsedMillis ledTmo;
elapsedMillis max6675tmo;
elapsedMillis failsafeTmo;
elapsedMillis screenTmo;
elapsedMillis setScreenTmo;
elapsedMillis statScreenTmo;

// --------------------------------------------------

void showSetScreen() {
  setScreenTmo = 0;                 // show Set Screen
  screenTmo = SCREEN_UPDATE_PERIOD; // refresh screen now
  statScreenTmo = 0;                // don't show Stat Screen
  
  Serial.print("New SetPoint = ");
  Serial.println(setP);
}

void buttonPlusPressed() {
  setP += T_STEP;
  if (setP > T_MAX) setP = T_MAX;
}
void buttonMinusPressed() {
  setP -= T_STEP;
  if (setP < T_MIN) setP = T_MIN;
}

void click(Button2& btn) {
  if (btn == buttonPlus) {
    Serial.println("+");
    buttonPlusPressed();
  } else if (btn == buttonMinus) {
    Serial.println("-");
    buttonMinusPressed();
  }

  showSetScreen();
}

void longclick(Button2& btn) {
  if (btn == buttonPlus) {
    Serial.println("+ long");
    buttonPlusPressed();
  } else if (btn == buttonMinus) {
    Serial.println("- long");
    buttonMinusPressed();
  }

  showSetScreen();
}

#define TEMP_CNT  3
float targetTempGet(Button2& btn)
{
  const float tempArray[TEMP_CNT] = { 180.0, 200.0, 220.0 };

  if (btn == buttonPlus) {
    // Limits crossed
    // Upper
    if (Setpoint >= tempArray[TEMP_CNT - 1]) {
      return Setpoint;
    }
    // Lower
    if (Setpoint < tempArray[0]) {
      return tempArray[0];
    }
    // Between limits
    for (int i = 0; i < (TEMP_CNT - 1); i++) {
      if (Setpoint >= tempArray[i] && Setpoint < tempArray[i + 1]) {
        return tempArray[i + 1];
      }
    }
  } else if (btn == buttonMinus) {
    // Limits crossed
    // Lower
    if (Setpoint <= tempArray[0]) {
      return Setpoint;
    }
    // Upper
    if (Setpoint > tempArray[TEMP_CNT - 1]) {
      return tempArray[TEMP_CNT - 1];
    }
    // Between limits
    for (int i = 1; i < TEMP_CNT; i++) {
      if (Setpoint <= tempArray[i] && Setpoint > tempArray[i - 1]) {
        return tempArray[i - 1];
      }
    }
  }
  return Setpoint;
}

void doubleclick(Button2& btn)
{
    float targettemp = targetTempGet(btn);

    if (btn == buttonPlus) {
      Serial.println("+ double");
    } else if (btn == buttonMinus) {
      Serial.println("- double");
    }

    Serial.print("Selected Temp = ");
    Serial.println(targettemp);

    setP = targettemp;

    showSetScreen();
}

// --------------------------------------------------

void LoadSetPoint(void) {
  double _setP = -1;
  EEPROMwl.get(INDEX_CONFIGURATION_VAR2, _setP);
  Serial.print("Loaded EEPROM SetPoint = ");
  Serial.println(_setP);
  if (_setP > 0) {
    Setpoint = _setP;
    Serial.print("SetPoint = ");
    Serial.println(Setpoint);
    setP = Setpoint;
    showSetScreen();
  }
  EEPROMwl.printStatus(Serial);
}

void SaveSetpoint(double fSetPoint) {
  Setpoint = fSetPoint;
  EEPROMwl.put(INDEX_CONFIGURATION_VAR2, fSetPoint);
  Serial.print("Stored EEPROM SetPoint = ");
  Serial.println(fSetPoint);
  EEPROMwl.printStatus(Serial);
}

void Relay_Loop(float temperature) {
  static bool hystPassed = false;

  // let Setpoint=200
  if (!hystPassed) {
    if (temperature < Setpoint - 1) {   // till 198 RLY_ON and temperature rising
      Output = RLY_ON;
    } else {                            // >=199 RLY-OFF and hystPassed and temperature rising due to inertion then flling due to cooling
      Output = RLY_OFF;
      hystPassed = true;
    }
  } 
  // hystPassed
  else {
    if (temperature <= Setpoint - 4) {  // temp falling due to cooling and crosses 200-4=196 -> RLY_ON
      hystPassed = false;
    }
  }

  // Control Relay
  digitalWrite(RELAY, Output);
}

// ----------------------------------------------

typedef enum {
  None = 0,
  Celsius = 1,
  SetPoint = 2,
  Version = 3,
  FailSafe = 0xFE,
  All = 0xFF,
} eLEDmode;

void displayUpdate(uint8_t mode, int temperature) {
  uint8_t all[] = { 0xff, 0xff, 0xff, 0xff };

  switch (mode)
  {
  case All:
    // All segments on
    display.setSegments(all);
    break;

  case Celsius:
    display.showNumberDec(temperature, false, 3, 0);
    // display.setSegments(celsius, 2, 2);
    display.setSegments(celsius, 1, 3);
    break;
  
  case SetPoint:
    display.setSegments(set_sym, 1, 0);
    display.showNumberDec(temperature, false, 3, 1);
    break;

  case FailSafe:
    display.setSegments(err_sym, 4, 0);
    break;

  case Version:
    display.clear();
    display.showNumberDec(VERSION, true, 3, 1);
    display.setSegments(underscore_sym, 1, 2);
    break;
  
  default:
    display.setSegments(not_available);
    break;
  }
}
void displayUpdate(uint8_t mode) {
  displayUpdate(mode, 0);
}


// ---------------------------------------------

static float temperature;

void setup()
{
  pinMode(VCC_OUT, OUTPUT); // Vcc to MAX6675
  digitalWrite(VCC_OUT, HIGH);
  pinMode(GND_OUT, OUTPUT); // GND to MAX6675
  digitalWrite(GND_OUT, LOW);

  // LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Relay
  pinMode(RELAY, OUTPUT);

  display.setBrightness(3);
  displayUpdate(None);
  delay(500);
  displayUpdate(Version);
  delay(500);

  Serial.begin(115200);

  buttonPlus.setDebounceTime(DEBOUNCE_TIME);
  buttonPlus.setLongClickTime(LONG_PRESS_TIME);
  buttonMinus.setDebounceTime(DEBOUNCE_TIME);
  buttonMinus.setLongClickTime(LONG_PRESS_TIME);
  buttonPlus.setLongClickDetectedRetriggerable(true);
  buttonMinus.setLongClickDetectedRetriggerable(true);
  buttonPlus.setClickHandler(click);
  buttonMinus.setClickHandler(click);
  buttonPlus.setLongClickDetectedHandler(longclick);
  buttonMinus.setLongClickDetectedHandler(longclick);
  buttonPlus.setDoubleClickHandler(doubleclick);
  buttonMinus.setDoubleClickHandler(doubleclick);

  // Setup WDT
  watchdog.enable(Watchdog::TIMEOUT_4S);

  char versionStr[64];
  snprintf(versionStr, sizeof(versionStr), "Oven Temperature Controller v%d.%d - 2020-2021", VERSION / 10, VERSION % 10);
  Serial.println(versionStr);  

  // wait for MAX chip to stabilize
  delay(500);

  EEPROMwl.begin(EEPROM_LAYOUT_VERSION, AMOUNT_OF_INDEXES);

  LoadSetPoint();
}

void loop()
{
  buttonPlus.loop();
  buttonMinus.loop();

  if (ledTmo > LEDPERIOD)
  {
    // LED Blink
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    ledTmo = 0;
  }

  if (max6675tmo > MAX6675PERIOD)
  {
    temperature = thermocouple.readCelsius();
    
//    #warning Comment next line in Release
//    temperature = 180.0;

    Serial.print("C = "); 
    Serial.print(temperature);
    Serial.print(" / "); 
    Serial.print(Setpoint);
    Serial.print(", Relay = ");
    Serial.println(Output);

    // Serial.print("F = ");
    // Serial.println(thermocouple.readFahrenheit());


    if (temperature != -100 && temperature > 0) { // -100 -> K-Type error, 0 -> dataread error
      failsafeTmo = 0;
      Relay_Loop(temperature);
    } else {
      Serial.print("FailSafe TMO = ");
      Serial.println(failsafeTmo);
    }
    if (failsafeTmo >= FAILSAFE_TMO) {
      Serial.println("FailSafe! Halt");
      digitalWrite(RELAY, LOW); // heater off
      while (1) {
        displayUpdate(FailSafe);
        delay(250);
        display.clear();
        delay(250);
        watchdog.reset();
      }
    }

    max6675tmo = 0;
  }

  if (screenTmo > SCREEN_UPDATE_PERIOD)
  {
    // Update LED display
    if (failsafeTmo < FAILSAFE_TMO) {
      if (setScreenTmo < SET_SCREEN_TMO) { // show Set screen
        displayUpdate(SetPoint, setP);
      } else {
        if (statScreenTmo >= STAT_SCREEN_PERIOD) { // every 5 sec show SETPOINT for 300 ms
          if (statScreenTmo < (STAT_SCREEN_PERIOD + STAT_SCREEN_LEN)) {
            displayUpdate(SetPoint, Setpoint);
          } else {
            statScreenTmo = 0;
          }
        } else {
          displayUpdate(Celsius, temperature); 
        }
      }
    } else {
      displayUpdate(None);
    }
    
    screenTmo = 0;
  }

  // ---------------------------------------
  //  Terminal Input
  //if (cnt % SCREEN_UPDATE_PERIOD == 0) { // every 100 ms
  /*
    if (Serial.available()) {
        String s = Serial.readString();
        if (s.length() > 0) {
          double setPoint = s.toDouble();
          SaveSetpoint(setPoint);
          Serial.print("RCV SET = ");
          Serial.println(setPoint);
        }
        Serial.print("SetPoint = ");
        Serial.println(Setpoint);
    }
  */
  //}

  if (setScreenTmo >= SET_SCREEN_TMO) {
    if (Setpoint != setP) {
      Serial.print("New SetPoint SET = ");
      Serial.println(setP);
      SaveSetpoint(setP);
    }
  }

  watchdog.reset();
}