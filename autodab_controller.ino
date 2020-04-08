// Required Libraries:
// Bounce2 - Version: Latest - https://github.com/thomasfredericks/Bounce2
// Chrono - Version: Latest - https://github.com/SofaPirate/Chrono
// FastLED - Version: Latest - https://github.com/FastLED/FastLED

// Solenoid config
#define SOLENOID_ENABLED true
#define SOLENOID_PIN  7

// LED config
#include <FastLED.h>
#define LED_FPS     60
#define LED_PIN     3
#define COLOR_ORDER RGB
#define CHIPSET     WS2811
#define NUM_LEDS    6
#define BRIGHTNESS  90
#define HUE_RED 0
#define HUE_YELLOW 64
#define HUE_GREEN 96
CRGB leds[NUM_LEDS];

// Rotary encoder config
#include "RotaryEncoder.h"
#define CLK_PIN         4
#define DT_PIN          5
RotaryEncoder encoder(CLK_PIN, DT_PIN); // initialize the rotary encoder

// Button config
#include <Bounce2.h>
#define MODE_BTN_PIN  6
#define GO_BTN_PIN    9
Bounce modeBtn = Bounce();
Bounce goBtn = Bounce();

// Timer conifg
#include <Chrono.h>
#include <LightChrono.h>
Chrono heatingTimer;
Chrono coolingTimer;
Chrono goodTimer;
#define HEATTIME_MIN 10     // Minimum heating time in seconds
#define HEATTIME_MAX 70     // Maximum heating time in seconds
#define HEATTIME_DEFAULT 30 // Default heating time in seconds
#define COOLTIME_DEFAULT 30 // Default cooling time in seconds
#define COOLTIME_MIN 1      // Minimum cooling time in seconds
#define COOLTIME_MAX 61     // Maximum cooling time in seconds
#define GOODTIME 10         // Length of time the nail is good to hit in seconds
uint8_t heattime = HEATTIME_DEFAULT;  // Torch on time in seconds.
uint8_t cooltime = COOLTIME_DEFAULT;  // Nail cooling time in seconds.

// Memory Config
#include <EEPROM.h>
#define HEATTIME_MEM_ADR 0 // Address in EEPROM for storing heattime
#define COOLTIME_MEM_ADR 1 // Address in EEPROM for storing cooltime

// State variables
enum states {
  IDLE_STATE,           // Everything is off.  Awaiting instructions.
  SET_HEAT_TIME_STATE,  // Adjust the heating time with the knob, with red LEDs as indicator.
  SET_COOL_TIME_STATE,  // Adjust the cooling time with knob, with yellow LEDs as indicator.
  HEATING_STATE,        // Solenoid is engaged.  Nail is heating up.  Red LEDs count up.
  COOLING_STATE,        // Solenoid is disengaged.  Nail is cooling down.  LEDs count down, fading from red to green.
  GOOD_STATE            // Nail is at perfect temperature.  Green LEDs dim.
};
enum states state = IDLE_STATE;
bool isTransitioning = true;

void setup() {
  delay(1000UL); // Give everything a second to power up

  // Setup Serial Monitor
  Serial.begin(9600);

  // Load saved values from EEPROM
  //EEPROM.begin(2);
  Serial.println("Reading stored values from EEPROM");
  uint8_t value = EEPROM.read(HEATTIME_MEM_ADR);
  Serial.print("Heattime (adr ");
  Serial.print(HEATTIME_MEM_ADR);
  Serial.print("): ");
  Serial.println(value);
  heattime = value ? value : HEATTIME_DEFAULT;
  value = EEPROM.read(COOLTIME_MEM_ADR);
  Serial.print("Cooltime (adr ");
  Serial.print(COOLTIME_MEM_ADR);
  Serial.print("): ");
  Serial.println(value);
  cooltime = value ? value : COOLTIME_DEFAULT;

  // Setup LEDs
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  // Setup Solenoid
  if (SOLENOID_ENABLED) {
    pinMode(SOLENOID_PIN, OUTPUT);
  }

  // Setup Buttons
  modeBtn.attach(MODE_BTN_PIN, INPUT_PULLUP);
  modeBtn.interval(50); // Use a debounce interval of 50 milliseconds
  goBtn.attach(GO_BTN_PIN, INPUT_PULLUP);
  goBtn.interval(50); // Use a debounce interval of 50 milliseconds
}

void loop() {
  // Update buttons and encoder
  modeBtn.update();
  goBtn.update();
  static Chrono encoderTimer;
  if (encoderTimer.hasPassed(1UL)) { // only check encoder every 1 millisecond 
    encoderTimer.restart();
    encoder.update();
  }

  // Update the LEDs
  static Chrono ledsTimer;
  if (ledsTimer.hasPassed(1000UL / LED_FPS)) {
    switch (state) {
      case IDLE_STATE:
        tickIdle();
        break;
      case SET_HEAT_TIME_STATE:
        tickSetHeatTime();
        break;
      case SET_COOL_TIME_STATE:
        tickSetCoolTime();
        break;
      case HEATING_STATE:
        tickHeating();
        break;
      case COOLING_STATE:
        tickCooling();
        break;
      case GOOD_STATE:
        tickGood();
        break;
    }

    ledsTimer.restart();
    FastLED.show();
  }
}



///////////////////////////////////////
// State methods
///////////////////////////////////////

// state: IDLE_STATE
// Everything is off.  Awaiting instructions.
void tickIdle() {
  if (isTransitioning) {
    isTransitioning = false;
    // Log the state transition
    Serial.println("READY FOR ACTION BOSS");

    // Clear all LEDs
    for(uint8_t i=0; i<NUM_LEDS; i++) {
      leds[i] = CHSV(0, 0, 0);
    }
  }

  // When mode button is pressed, change state to SET_HEAT_TIME_STATE
  if (modeBtn.fell()) {
    Serial.println("MODE button pressed");
    state = SET_HEAT_TIME_STATE;
    isTransitioning = true;
    return;
  }

  // When go button is pressed, change state to HEATING_STATE
  if (goBtn.fell()) {
    Serial.println("GO button pressed");
    state = HEATING_STATE;
    isTransitioning = true;
    return;
  }
}


// state: SET_HEAT_TIME_STATE
// Adjust the heating time with the knob, with red LEDs as indicator.
void tickSetHeatTime() {
  if (isTransitioning) {
    isTransitioning = false;
    // Log the state transition
    Serial.println("ADJUST HEATING TIME");
    Serial.println(" HEATING TIME :");
    Serial.println(heattime);
    // Reset the encoder
    encoder.reset();
  }

  // When mode button is pressed, store value and change state to SET_COOL_TIME_STATE
  if (modeBtn.fell()) {
    Serial.println("MODE button pressed");
    // Store heattime in memory
    EEPROM.write(HEATTIME_MEM_ADR, heattime);
    //EEPROM.commit();
    // Change state
    state = SET_COOL_TIME_STATE;
    isTransitioning = true;
    return;
  }

  // If rotary encoder is moved, adjust the amount of time for heating.
  int16_t encoderChange = encoder.getChange();
  if (encoderChange != 0) {
    // guard against integer overflow
    if (encoderChange < 0 && abs(encoderChange) > heattime) {
      heattime = HEATTIME_MIN;
    } else if (encoderChange > HEATTIME_MAX - heattime) {
      heattime = HEATTIME_MAX;
    } else {
      heattime = heattime + encoderChange;
    }
    heattime = constrain(heattime, HEATTIME_MIN, HEATTIME_MAX);

    Serial.print("Heating Time ");
    Serial.print(heattime);
    Serial.println(" seconds");
  }

  // Draw red LEDs to indicate how long heating will be.
  lightProgressive(HUE_RED, heattime, HEATTIME_MIN, HEATTIME_MAX);
}


// state: SET_COOL_TIME_STATE
// Adjust the cooling time with knob, with yellow LEDs as indicator.
void tickSetCoolTime() {
  // and reset the encoder
  if (isTransitioning) {
    isTransitioning = false;
    // Log the state transition
    Serial.println("ADJUST COOLING TIME");
    Serial.println("| COOLING TIME :");
    Serial.println(cooltime);
    // Reset the encoder
    encoder.reset();
  }

  // When mode button is pressed, store value and change state to IDLE_STATE
  if (modeBtn.fell()) {
    Serial.println("MODE button pressed");
    // Store cooltime in memory
    EEPROM.write(COOLTIME_MEM_ADR, cooltime);
    //EEPROM.commit();
    // Change state
    state = IDLE_STATE;
    isTransitioning = true;
    return;
  }

  // If rotary encoder is moved, adjust the amount of time for heating.
  int16_t encoderChange = encoder.getChange();
  if (encoderChange != 0) {
    // guard against integer overflow
    if (encoderChange < 0 && abs(encoderChange) > cooltime) {
      cooltime = COOLTIME_MIN;
    } else if (encoderChange > COOLTIME_MAX - cooltime) {
      cooltime = COOLTIME_MAX;
    } else {
      cooltime = cooltime + encoderChange;
    }
    cooltime = constrain(cooltime, COOLTIME_MIN, COOLTIME_MAX);

    Serial.print("Cooling Time ");
    Serial.print(cooltime);
    Serial.println(" seconds");
  }

  // Draw yellow LEDs to indicate how long cooling will be.
  lightProgressive(HUE_YELLOW, cooltime, COOLTIME_MIN, COOLTIME_MAX);
}


// state: HEATING_STATE
// Solenoid is engaged.  Nail is heating up.  Red LEDs count up.
void tickHeating() {
  if (isTransitioning) {
    isTransitioning = false;
    // Log state transition
    Serial.println("ENGAGING DAB");
    // Start heating timer
    heatingTimer.restart();
    // Engage solenoid to begin heating
    if (SOLENOID_ENABLED) {
      digitalWrite(SOLENOID_PIN, HIGH);
    }
  }

  // If go button is pressed, cancel heating by changing state to IDLE_STATE
  if (goBtn.fell()) {
    Serial.println("GO button pressed");
    if (SOLENOID_ENABLED) {
      digitalWrite(SOLENOID_PIN, LOW);
    }
    state = IDLE_STATE;
    isTransitioning = true;
    return;
  }
  
  // When the heating timer has elapsed, change state to COOLING_STATE
  if (heatingTimer.hasPassed(heattime * 1000UL)) {
    state = COOLING_STATE;
    isTransitioning = true;
    return;
  }

  // Light the LEDs red
  lightProgressive(HUE_RED, heatingTimer.elapsed(), 0UL, heattime * 1000UL);
}


// state: COOLING_STATE
// Solenoid is disengaged.  Nail is cooling down.  LEDs count down, fading from red to green.
void tickCooling() {
  if (isTransitioning) {
    isTransitioning = false;
    // Log the state transition
    Serial.println("DAB OFF; COOLING");
    // Disengage the solenoid, turning the heating off.
    if (SOLENOID_ENABLED) {
      digitalWrite(SOLENOID_PIN, LOW);
    }
    // Start cooling timer
    coolingTimer.restart();
  }

  // If go button is pressed, abort early by changing state to IDLE_STATE
  if (goBtn.fell()) {
    Serial.println("GO button pressed");
    state = IDLE_STATE;
    isTransitioning = true;
    return;
  }
  
  // When the cooling timer has elapsed, change state to GOOD_STATE
  if (coolingTimer.hasPassed(cooltime * 1000UL)) {
    state = GOOD_STATE;
    isTransitioning = true;
    return;
  }

  // Transition the LEDs from red to green
  uint8_t hue = HUE_GREEN * coolingTimer.elapsed() / (cooltime * 1000UL); // red happens to be 0, which is quite convenient for this.

  // Turn the leds off, progressively
  dimProgressive(hue, coolingTimer.elapsed(), 0UL, cooltime * 1000UL);
}


// state: GOOD_STATE
// Nail is at perfect temperature.  Green LEDs dim.
void tickGood() {
  if (isTransitioning) {
    isTransitioning = false;
    // Log the state transition
    Serial.println("IT'S ALL GOOD");
    // Start goodtime Timer
    goodTimer.restart();
  }

  // If go button is pressed, abort early by changing state to IDLE_STATE
  if (goBtn.fell()) {
    Serial.println("GO button pressed");
    state = IDLE_STATE;
    isTransitioning = true;
    return;
  }
  
  // When the goodtime timer has elapsed, change state to IDLE_STATE
  if (goodTimer.hasPassed(GOODTIME * 1000UL)) {
    state = IDLE_STATE;
    isTransitioning = true;
    return;
  }

  // Fade all LEDs from green to black
  uint8_t brightness = 255 - (255 * goodTimer.elapsed() / (GOODTIME * 1000UL));
  for (uint8_t i=0; i<NUM_LEDS; i++) {
    leds[i] = CHSV(HUE_GREEN, 255, brightness);
  }
}



///////////////////////////////////////
// LED utility methods
///////////////////////////////////////

void lightProgressive(uint8_t hue, unsigned long x, unsigned long min, unsigned long max ) {
  // Multiply the number of LEDs to light by 100 to so the decimal amount can be stored in an integer.
  // For example, if NUM_LEDS is 6, x is 53300, min is 0, and max is 60000,
  // then number of LEDs to light would be 5.33, and we'll store it as the integer 533.
  // This way we don't have to use any floating point math!
  unsigned long centiNumLedsToLight = 100 * NUM_LEDS * (x-min) / (max - min);
  // Dividing this by 100 gets us the number of fully lit LEDs
  uint8_t numFullyLit = centiNumLedsToLight / 100;
  // Using modulo 100 to find the remainder, then dividing that by 100,
  // gets us the percent that the next LED is partially lit.
  uint8_t partialBrightness = 255 * (centiNumLedsToLight % 100) / 100;

  // Light the fully bright LEDs
  for (uint8_t i=0; i<numFullyLit; i++) {
    leds[i] = CHSV( hue, 255, 255);
  }
  if (numFullyLit < NUM_LEDS) {
    // Partially light the next LED
    leds[numFullyLit] = CHSV( hue, 255, partialBrightness);
    // Clear any remaining LEDs after the partially-lit LED
    for (uint8_t i = numFullyLit+1; i<NUM_LEDS; i++) {
      leds[i] = CHSV( 0, 0, 0);
    }
  }
}

void dimProgressive(uint8_t hue, unsigned long x, unsigned long min, unsigned long max ) {
  // Multiply the number of dark LEDs by 100 to so the decimal amount can be stored in an integer.
  // For example, if NUM_LEDS is 6, x is 53300, min is 0, and max is 60000,
  // then number of dark LEDs would be 5.33, and we'll store it as the integer 533.
  // This way we don't have to use any floating point math!
  unsigned long centiNumLedsToDarken = 100 * NUM_LEDS * (x-min) / (max - min);
  // Dividing this by 100 gets us the number of dark LEDs
  uint8_t numFullyDark = centiNumLedsToDarken / 100;
  // Using modulo 100 to find the remainder, then dividing that by 100,
  // gets us the percent that the next LED is partially lit.
  uint8_t partialBrightness = 255 - (255 * (centiNumLedsToDarken % 100) / 100);
  
  // Clear the darkened LEDs
  for (uint8_t i=0; i<numFullyDark; i++) {
    leds[i] = CHSV( 0, 0, 0 );
  }
  if (numFullyDark < NUM_LEDS) {
    // Partially light the next LED
    leds[numFullyDark] = CHSV( hue, 255, partialBrightness);
    // Light the remaining LEDs
    for (uint8_t i = numFullyDark+1; i < NUM_LEDS; i++) {
      leds[i] = CHSV( hue, 255, 255);
    }
  }
}