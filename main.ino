/**************************************************************************************************************************************************
*                                                                                                                                                 *
*  DIY pressure monitoring system (suatble for air/gas/water/oil) - example of usage with an Air Compressor:                                      *
*  https://github.com/mikew777/KypruinoPressureMonitor/                                                                                           *
*                                                                                                                                                 *
*  This code is adapted for Kypruino UNO+ v0.6.x board with connected to it:                                                                      *
*  - analog pressure sensor (to Analog A2 pin)                                                                                                    *
*  - I2C external screen 128x32 (conected to special screen socket of Kypruino UNO+)                                                              *
*                                                                                                                                                 *
*  Kypruino UNO+ is an Arduino Uno R3 copmatible board helps quick prototyping and clean DIY projects with less wires and external components.    *
*                                                                                                                                                 *
*  Developed by Island Electronics team https://islandelectronics.com.cy                                                                          *
*                                                                                                                                                 *
**************************************************************************************************************************************************/

#include <Wire.h> // for I2C display
#include <Adafruit_GFX.h> // for I2C display
#include <Adafruit_SSD1306.h> // for I2C display
#include <Adafruit_NeoPixel.h> // to work with on-board NeoPixel LEDs
#include <EEPROM.h> // to access embedded EEPROM of Kypruino's MCU
#include <math.h>  // to use isnan() for new EEPROM

// ===== Hardware setup =====
// OLED-display 128×32 by I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET   -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NeoPixel (3 RGB LED) using pin 8
#define NEOPIXEL_PIN   8
#define NEOPIXEL_COUNT 3
Adafruit_NeoPixel strip(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Pressure sensor connected to A2
const int SENSOR_PIN = A2;

// on-board buttons for alarm settings
#define UPPER_INC_PIN 6
#define UPPER_DEC_PIN 2
#define LOWER_INC_PIN 4
#define LOWER_DEC_PIN 7

// Buzzer/piezo pin
#define BUZZER_PIN 9

// ===== Analog pressure sensor settings =====
const float V_MIN     = 0.5f;      // when 0 PSI
const float V_MAX     = 4.5f;      // when max 150 PSI
const float PSI_MIN   = 0.0f;
const float PSI_MAX   = 150.0f;
const float PSI_RANGE = PSI_MAX - PSI_MIN;
const float V_RANGE   = V_MAX   - V_MIN;
const float PSI_TO_BAR = 0.0689476f; // 1 PSI ≈ 0.0689476 bar

// ===== Timings =====
const unsigned long STATS_INTERVAL   = 5UL * 60UL * 1000UL; // 5 minutes
const unsigned long INSTANT_INTERVAL = 500;                // 0.5 sec

// ===== EEPROM =====
const int EEPROM_ADDR_LOWER = 0;
const int EEPROM_ADDR_UPPER = sizeof(float);
const float THRESH_MIN      = -99.9f;
const float THRESH_MAX      =  99.9f;
const float DEFAULT_LOWER   = -2.0f;
const float DEFAULT_UPPER   = 10.1f;

// ===== Global variables =====
// Alert thresholds (bottom and upper)
float lowerThreshold, upperThreshold;

// For statistics block of the screen
unsigned long nextStatsUpdate;
unsigned long lastInstantUpdate;
float sumBar = 0;
unsigned long countBar = 0;
float minBar = 1e6, maxBar = -1e6, avgBar = 0;

// Screen updates management
float prevAvgBar = NAN, prevMinBar = NAN, prevMaxBar = NAN;
int   prevRemMin = -1,  prevRemSec = -1;

// Alert thresholds settings mode function
void thresholdConfigMode();

void setup() {
  // on-board buttons initialization
  pinMode(UPPER_INC_PIN, INPUT_PULLUP);
  pinMode(UPPER_DEC_PIN, INPUT_PULLUP);
  pinMode(LOWER_INC_PIN, INPUT_PULLUP);
  pinMode(LOWER_DEC_PIN, INPUT_PULLUP);

  // Puzzer/piezo initialization
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // I2C display initialization
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // NeoPixels initialization
  strip.begin();
  strip.show();

  // Reading thresholds settings saved EEPROM
  EEPROM.get(EEPROM_ADDR_LOWER, lowerThreshold);
  EEPROM.get(EEPROM_ADDR_UPPER, upperThreshold);
  // If it is first time launch and EEPROM has no settings saved - writing default values DEFAULT_LOWER & DEFAULT_UPPER
  if ( isnan(lowerThreshold) || lowerThreshold < THRESH_MIN || lowerThreshold > THRESH_MAX ||
       isnan(upperThreshold) || upperThreshold < THRESH_MIN || upperThreshold > THRESH_MAX) {
    lowerThreshold = DEFAULT_LOWER;
    upperThreshold = DEFAULT_UPPER;
    EEPROM.put(EEPROM_ADDR_LOWER, lowerThreshold);
    EEPROM.put(EEPROM_ADDR_UPPER, upperThreshold);
  }


  // Timers
  unsigned long now = millis();
  nextStatsUpdate   = now + STATS_INTERVAL;
  lastInstantUpdate = now;
}

void loop() {
  unsigned long now = millis();

  // If button pressed - enter threshold settings mode
  if (digitalRead(UPPER_INC_PIN)==LOW ||
      digitalRead(UPPER_DEC_PIN)==LOW ||
      digitalRead(LOWER_INC_PIN)==LOW ||
      digitalRead(LOWER_DEC_PIN)==LOW) {
    thresholdConfigMode();
    // After threshold settings mode - setting flags to return main screen data
    prevAvgBar = prevMinBar = prevMaxBar = NAN;
    prevRemMin = prevRemSec = -1;
    lastInstantUpdate = 0;
  }

  // 1) Reading pressure sensor data
  int raw = analogRead(SENSOR_PIN);
  float voltage = raw * (5.0f / 1023.0f);
  float psi = (voltage - V_MIN) * (PSI_RANGE / V_RANGE);
  float bar = psi * PSI_TO_BAR;

  // 2) Statistics for data on screen
  sumBar += bar;
  countBar++;
  if (bar < minBar) minBar = bar;
  if (bar > maxBar) maxBar = bar;

  // 3) Reset statistics based on STATS_INTERVAL period setting
  if (now >= nextStatsUpdate) {
    avgBar = sumBar / countBar;
    sumBar = 0;
    countBar = 0;
    minBar = bar;
    maxBar = bar;
    nextStatsUpdate = now + STATS_INTERVAL;
  }

  // 4) Screen statistics update timer
  unsigned long remaining = (nextStatsUpdate > now) ? nextStatsUpdate - now : 0;
  int remMin = remaining / 60000;
  int remSec = (remaining % 60000) / 1000;

  // 5) Update of live data with frequency defined in INSTANT_INTERVAL for current pessure + NeoPixels + buzzer
  if (now - lastInstantUpdate >= INSTANT_INTERVAL) {
    lastInstantUpdate = now;

    // — Updating live pressure data screen block 0…15px
    display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK);

    // — using big font
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(bar, 1);
    display.print(" bar");

    // — reverting to standart font
    display.setTextSize(1);

    // NeoPixels: blue->yellow (for 0–4 bar range), yellow->red (for 4–8 bar range)
    float t; uint8_t R, G, B;
    if (bar <= 0.0f) {
      R = 0;   G = 0;   B = 255;
    } else if (bar < 4.0f) {
      t = bar / 4.0f;
      R = (uint8_t)(255 * t);
      G = (uint8_t)(255 * t);
      B = (uint8_t)(255 * (1.0f - t));
    } else if (bar < 8.0f) {
      t = (bar - 4.0f) / 4.0f;
      R = 255;
      G = (uint8_t)(255 * (1.0f - t));
      B = 0;
    } else {
      R = 255; G = 0; B = 0;
    }
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(R, G, B));
    }
    strip.show();

    // Buzzer: alert if threshold passed
    if (bar < lowerThreshold || bar > upperThreshold) {
      display.display();
      for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 1000);    // 1 kHz
        delay(500);
        noTone(BUZZER_PIN);
        if (i < 2) delay(100);
      }
    } else {
      noTone(BUZZER_PIN); 
    }
  }

  // 6) Statistics of pressure - it show for the past period Lowest pressure measured / M(average pressure calculated) / Highest pressure measured
  if (avgBar != prevAvgBar) {
    display.fillRect(0, 16, SCREEN_WIDTH, 8, BLACK);
    display.setCursor(0, 16);
    display.print("L:");
    display.print(minBar, 1);
    display.print(" M:");
    display.print(avgBar, 1);
    display.print(" H:");
    display.print(maxBar, 1);
    prevMinBar = minBar;
    prevAvgBar = avgBar;
    prevMaxBar = maxBar;
  }

  // 7) Show timer - when next statistics will be shown
  if (remMin != prevRemMin || remSec != prevRemSec) {
    display.fillRect(0, 24, SCREEN_WIDTH, 8, BLACK);
    display.setCursor(0, 24);
    display.print("Next:");
    if (remMin < 10) display.print('0');
    display.print(remMin);
    display.print(':');
    if (remSec < 10) display.print('0');
    display.print(remSec);
    prevRemMin = remMin;
    prevRemSec = remSec;
  }

  // 8) General display referesh execution
  display.display();
}

// ===== Thresholds settings mode funtion =====
void thresholdConfigMode() {
  unsigned long lastPress = millis();

  // Saving start thresholds to compare later to decide if to update EEPROM (to keep EEPROM healthy and not write to it if data not updated)
  float origLower = lowerThreshold;
  float origUpper = upperThreshold;

  // Drawing new thresholds settings screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Alarm levels:");
  display.setCursor(0, 12);
  display.print("Low:  ");
  display.print(lowerThreshold, 1);
  display.print(" bar");
  display.setCursor(0, 22);
  display.print("High: ");
  display.print(upperThreshold, 1);
  display.print(" bar");
  display.display();

  while (millis() - lastPress < 5000) {
    bool updated = false;

    if (digitalRead(UPPER_INC_PIN) == LOW) {
      upperThreshold = min(upperThreshold + 0.1f, THRESH_MAX);
      updated = true;
    }
    if (digitalRead(UPPER_DEC_PIN) == LOW) {
      upperThreshold = max(upperThreshold - 0.1f, THRESH_MIN);
      updated = true;
    }
    if (digitalRead(LOWER_INC_PIN) == LOW) {
      lowerThreshold = min(lowerThreshold + 0.1f, THRESH_MAX);
      updated = true;
    }
    if (digitalRead(LOWER_DEC_PIN) == LOW) {
      lowerThreshold = max(lowerThreshold - 0.1f, THRESH_MIN);
      updated = true;
    }

    if (updated) {
      // if any button pressed - reset timer to keep user in the settings menu for 5 sec after any interaction with this menu
      lastPress = millis();

      // if buttong pressed - means we need to update data on the screen to reflect changes
      display.fillRect(0, 12, SCREEN_WIDTH, 20, BLACK);
      display.setCursor(0, 12);
      display.print("Low:  ");
      display.print(lowerThreshold, 1);
      display.print(" bar");
      display.setCursor(0, 22);
      display.print("High: ");
      display.print(upperThreshold, 1);
      display.print(" bar");
      display.display();
    }

    delay(100); // to have smooth display operation
  }
  // Only if settings are changed when exiting threshold settings mode - writing new settings to EEPROM
  if (lowerThreshold != origLower) {
    EEPROM.put(EEPROM_ADDR_LOWER, lowerThreshold);
  }
  if (upperThreshold != origUpper) {
    EEPROM.put(EEPROM_ADDR_UPPER, upperThreshold);
  }
}

