

/**
   The MIT License (MIT)

   Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
   Copyright (c) 2018 by Fabrice Weinberg

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   ThingPulse invests considerable time and money to develop these open source libraries.
   Please support us by buying our products (and not the clones) from
   https://thingpulse.com

*/

// Main Include
#include <Arduino.h>  // Required for Arduino-specific types
#include "declares.h"

// Include watchdog timer library
#include <esp_system.h>
#include <esp_task_wdt.h>

// CPU Sleep stuff
#include <esp_sleep.h>

// Include the correct display library

// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// OR #include "SH1106Wire.h"   // legacy: #include "SH1106.h"

// For a connection via I2C using brzo_i2c (must be installed) include:
// #include <brzo_i2c.h>        // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// OR #include "SH1106Brzo.h"

// For a connection via SPI include:
// #include <SPI.h>             // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// OR #include "SH1106SPi.h"


// Optionally include custom images and fonts
#include "images.h"
#include "fontSuiGenerisRg.h"


// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA, SCL);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
// SSD1306Wire display(0x3c, D3, D5);  // ADDRESS, SDA, SCL  -  If not, they can be specified manually.
// SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);  // ADDRESS, SDA, SCL, OLEDDISPLAY_GEOMETRY  -  Extra param required for 128x32 displays.
// SH1106Wire display(0x3c, SDA, SCL);     // ADDRESS, SDA, SCL
//SSD1306Wire display(0x3c, 22, 20); //Dismo Config

// Initialize the OLED display using brzo_i2c:
// SSD1306Brzo display(0x3c, D3, D5);  // ADDRESS, SDA, SCL
// or
// SH1106Brzo display(0x3c, D3, D5);   // ADDRESS, SDA, SCL

#include "SparkFun_LIS2DH12.h" //Click here to get the library: http://librarymanager/All#SparkFun_LIS2DH12
SPARKFUN_LIS2DH12 accel;       //Create instance

/*
Fuel Gauge - Thanks Sparkfun and Paul Clark
*/ 
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> 
SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048

/*
Clock/RTC
*/
// Include libraries for time manipulation
#include <time.h>

/*
WiFi
*/

// Memory Management
/* 
Open Preferences with a namespace name.
Each application module, library, etc. has to use a namespace name
to prevent key name collisions.
*/
#include <Preferences.h>
Preferences preferencesMainApp;

// Logging Management
#include "esp_log.h"
static const char *TAG_MAIN = "mainApp";
//static const char *TAG_SENSOR = "Sensor";

/**
 * @file player-url-i2s.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-url-i2s/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */


#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"


const char *urls[] = {
  "http://stream.srg-ssr.ch/m/rsj/mp3_128",
  "http://stream.srg-ssr.ch/m/drs3/mp3_128",
  "http://stream.srg-ssr.ch/m/rr/mp3_128",
  "http://sunshineradio.ice.infomaniak.ch/sunshineradio-128.mp3",
  "http://streaming.swisstxt.ch/m/drsvirus/mp3_128"
};

// WiFi
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESPAsyncWebServer.h>
#include "credentials.h" 
WiFiManager wifiManager;
AsyncWebServer server(80); // Declare but don’t start the server yet

URLStream url(wifi_ssid, wifi_password);
//AudioSourceURL source(urlStream, urls, "audio/mp3");
I2SStream i2s;
//EncodedAudioStream dec(&i2s, new MP3DecoderHelix()); // Decoding stream
//StreamCopy copier(dec, url); // copy url to decoder

SineWaveGenerator<int16_t> sine;
GeneratedSoundStream<int16_t> in(sine); 
VolumeStream volume(in);
StreamCopy copier(i2s, volume); 
AudioActions action;


// additional controls
Debouncer nextButtonDebouncer(2000);


/*
Games
Easiest to include these libraries by adding them to the OS's Arduino Library Folder to avoid c_cpp_properties.json mods
*/

#include "ReactionTimeGame.h" 
ReactionTimeGame reactionGame(display, button_BottomRight); // Display class name and button to use

#include "SimonSaysGame.h"
SimonSaysGame* simonSaysGame = nullptr;  // Initialized later
bool simonSaysGameEnabled = false;




// Demo Config
typedef void (*Demo)(void);
Demo demos[] = {
  drawFontFaceDemo, // 0
  drawProgressBarDemo, 
  drawImageDemo_1,
  drawImageDemo_2,
  drawImageDemo_3,
  drawImageDemo_4,
  drawTextFlowDemo,
  drawTextAlignmentDemo,
  drawRectDemo,
  drawCircleDemo,
  drawBatteryProgressBar,
  drawAccelerometerScreen, // 11
  drawButtonCounters, // 12
  drawAudioPlayer, // 13
  drawFlashlight, // 14
  drawReactionTimeGame, // 15
  drawTimeOnCounter, // 16
  //startSimonSaysGame, // 
  drawSliderProgressBar, // 17
  drawClockDemo,          // 18 - Add the new screen for clock
  drawSerialDataScreenWrapper, // 19
  drawWifiConfig // 20
  };

int demoLength = (sizeof(demos) / sizeof(Demo));


// RGBW LEDs

#include <Adafruit_NeoPixel.h>
typedef void (*RGBW)(void);
RGBW colors[] = {red, green, blue, white, halfWHITE};

int colorLength = (sizeof(colors) / sizeof(RGBW));

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_RGBW + NEO_KHZ800);


// Function prototypes
void IRAM_ATTR debounceButton(int buttonIndex);

// Interrupt service routines for each button
void IRAM_ATTR handleButtonPress1() {
  debounceButton(0);
}

void IRAM_ATTR handleButtonPress2() {
  debounceButton(1);
}

void IRAM_ATTR handleButtonPress3() {
  debounceButton(2);
  //handleScrollUp();
}

void IRAM_ATTR handleButtonPress4() {
  debounceButton(3);
  //handleScrollDown();
}

void IRAM_ATTR handleButtonPress5() {
  debounceButton(4);
}

void IRAM_ATTR handleButtonPress6() {
  if(!reactionGameEnabled && !simonSaysGameEnabled){
    debounceButton(5);
    //buttonPressed = true;  // Just set the flag
  }
}

// Check if a button was pressed
// Based on counters, which isn't a great way to do it
// these types of events should be triggered via state handshake so holds and releases can be accomodated
bool compareButtonCounters(volatile int* counter1, volatile int* counter2, int length) {
  for (int i = 0; i < length; i++) {
    if (counter1[i] != counter2[i]) {
      return false;
    }
  }
  return true;
}

void buttonPressedTap(int buttonIndex){
  // Button 1 (Top Left)
  if (buttonIndex == 0){
    Serial.println("ButtonTap: " + String(buttonIndex));
    demoModePreviously = demoMode;
    demoMode = (demoMode - 1 + demoLength) % demoLength;    
  }
  // Button 2 (Top Right)
  if (buttonIndex == 1){
    demoModePreviously = demoMode;
    demoMode = (demoMode + 1)  % demoLength;
    Serial.println("ButtonTap: " + String(buttonIndex));
  }
  // // Button 6 (Bottom Right)
  // if (buttonIndex == 5){
  //   // if (demoMode == 10){ // Specific to Audio Player, fix/replace with enum
  //   //   player.stop();
  //   // }
  // }
}

// Debounce function to increment the button counter
void IRAM_ATTR debounceButton(int buttonIndex) {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime[buttonIndex]) > debounceDelay) {
    int currentState = digitalRead(buttonPins[buttonIndex]);

    // Check if the button state has changed
    if (currentState != buttonState[buttonIndex]) {
      lastDebounceTime[buttonIndex] = currentTime;

      // Only increment the counter if the button was pressed (LOW state)
      if (currentState == LOW && buttonState[buttonIndex] == HIGH) {
        buttonCounter[buttonIndex]++;
        millisLastInteraction = millisNow;
        buttonPressedTap(buttonIndex);
      }

      // Update the button state
      buttonState[buttonIndex] = currentState;
    }
  }
}

// Navigation Controls
void loadButtonCounters() {
  preferencesMainApp.begin("mainApp", true);

  // Retrieve each button counter value
  for (int i = 0; i < numButtons; i++) {
    buttonCounterSaved[i] = preferencesMainApp.getInt((String("button") + i).c_str(), 0); // Read the count back
    buttonCounter[i] = buttonCounterSaved[i]; // Keep the saved and lived counts synced
  }

  demoModeSaved = preferencesMainApp.getInt("demoMode", 0);
  demoMode = demoModeSaved;

  preferencesMainApp.end();
}

void saveButtonCounters() {
  // Compare button counters
  if (compareButtonCounters(buttonCounter, buttonCounterSaved, numButtons) == false){
    Serial.println("Button counters are not equal.");

    preferencesMainApp.begin("mainApp", false);

    // Store each button counter value
    for (int i = 0; i < numButtons; i++) {
      preferencesMainApp.putInt((String("button") + i).c_str(), buttonCounter[i]); // Write the live count
      buttonCounterSaved[i] = buttonCounter[i]; // Keep the saved and lived counts synced
    }

    preferencesMainApp.end();
  }

  if (demoMode != demoModeSaved){
    preferencesMainApp.begin("mainApp", false);
    preferencesMainApp.putInt("demoMode", demoMode);
    preferencesMainApp.end();
  }

}

void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: 
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1: 
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER: 
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: 
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP: 
      Serial.println("Wakeup caused by ULP program");
      break;
    default: 
      Serial.println("Wakeup was not caused by deep sleep");
      break;
  }
}


void disableWatchdog() {
    esp_task_wdt_delete(NULL); // Remove the current task from the watchdog
    Serial.println("Watchdog disabled");
}

void enableWatchdog() {
    esp_task_wdt_add(NULL); // Re-add the current task to the watchdog
    Serial.println("Watchdog enabled");
}

void setup() {
  // Initialize the task watchdog timer
  // esp_task_wdt_config_t wdt_config = {
  //     .timeout_ms = 15000,     // Timeout period in milliseconds (5 seconds)
  //     .idle_core_mask = 0,    // Idle core mask
  //     .trigger_panic = true,  // Trigger panic on timeout
  // };
  

  // Apply the watchdog timer configuration
  //esp_task_wdt_init(&wdt_config);
  esp_err_t result = esp_task_wdt_init(15, true); // Set a 15-second timeout

  // Add current task to the watchdog timer
  //esp_task_wdt_add(NULL);

  // Configure the wakeup pins
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW); // For Active LOW button, ext0 single pin lowest power



  // Set the global log level to INFO (can be ESP_LOG_NONE, ESP_LOG_ERROR, etc.)
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  
  // Alternatively, set log level for a specific tag
  esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
  //esp_log_level_set(TAG_SENSOR, ESP_LOG_DEBUG); // Debug level for Sensor

  // Example logging
  ESP_LOGI(TAG_MAIN, "Setup complete");
  //SP_LOGI(TAG_SENSOR, "Setup complete");
  //ESP_LOGD(TAG, "This is a debug message");



  // Configure the wakeup pins using ext1 and the required bitmask
  // uint64_t bitmask = 0;
  // for (int i = 0; i < numButtons; i++) {
  //   bitmask |= (1ULL << buttonPins[i]);
  // }
  // esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ALL_LOW); // Wake up on all LOW
  pinMode(OLED_RESET, OUTPUT);
  digitalWrite(OLED_RESET, LOW); 

  // Set up the power regulator pin
  pinMode(POWER_PIN_OLED, OUTPUT);
  digitalWrite(POWER_PIN_OLED, HIGH); // Turn on the OLED power regulator
  delay(200);
  digitalWrite(OLED_RESET, HIGH);
  delay(10);

  // Set up the power regulator pin
  pinMode(POWER_PIN_AUX, OUTPUT);
  digitalWrite(POWER_PIN_AUX, HIGH); // Turn on the aux power regulator

  Serial.begin(250000);
  Serial.println();
  Serial.println();

  // Initialising the UI will init the display too.
  display.init();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // RGBW LEDs
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Battery Voltage
  /*
  Battery voltage divider
  VBAT is plit through a 50/50 divider (2x 200k resistors) into VOLT_READ_PIN
  Circuit additional has 1uF cap to ground for filtering

  */
  pinMode(VOLT_READ_PIN, INPUT);

  // Set up the buttons as inputs with internal pull-up resistors if supported
  // Only GPIO 15 supports pull up, the other buttons have external pull-ups
  // for (int i = 0; i < numButtons; i++) {
  //   pinMode(buttonPins[i], INPUT_PULLUP);
  // }
  pinMode(button_TopLeft, INPUT);
  pinMode(button_TopRight, INPUT);
  pinMode(button_MiddleLeft, INPUT);
  pinMode(button_MiddleRight, INPUT);
  pinMode(button_BottomLeft, INPUT);
  pinMode(button_BottomRight, INPUT_PULLUP);

  // Attach interrupts to the buttons
  attachInterrupt(digitalPinToInterrupt(buttonPins[0]), handleButtonPress1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPins[1]), handleButtonPress2, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPins[2]), handleButtonPress3, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPins[3]), handleButtonPress4, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPins[4]), handleButtonPress5, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPins[5]), handleButtonPress6, FALLING);
  

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Check address jumper and wiring. Freezing...");
    while (1)
      ;
  }

  // Memory Management
  loadButtonCounters();

  // Print wakeup reason
  printWakeupReason();

  digitalWrite(OLED_RESET, HIGH); // Turn on the power regulator
  delay(200);
  // Initialising the UI will init the display too.
  display.init();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Speaker Setup
  //AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // Audio setup output
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.i2s_format = I2S_LSB_FORMAT; //I2S_LSB_FORMAT default vs STD?
  cfg.pin_ws = 27; // LRC pin
  cfg.pin_bck = 26; //BCLK pin
  cfg.pin_data = 14; //DIN pin
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  i2s.begin(cfg);
  in.begin(cfg); // Setup sound generation based on Audioi2s settings
  // set initial volume
  auto vcfg = volume.defaultConfig();
  vcfg.copyFrom(cfg);
  //vcfg.allow_boost = true; // activate amplification using linear control
  volume.begin(vcfg); // we need to provide the bits_per_sample and channels
  volume.setVolume(0.0);
  setupActions(); // activate sound keys

  // Fuel Gauge MAX17048
  lipo.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  if (lipo.begin() == false) // Connect to the MAX17048 using the default wire port
  {
    Serial.println(F("MAX17048 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

  // // Just because we can, let's reset the MAX17048
  // Serial.println(F("Resetting the MAX17048..."));
  // delay(1000); // Give it time to get its act back together

  // Read and print the reset indicator
  Serial.print(F("Reset Indicator was: "));
  bool RI = lipo.isReset(true); // Read the RI flag and clear it automatically if it is set
  Serial.println(RI); // Print the RI
  // If RI was set, check it is now clear
  if (RI)
  {
    Serial.print(F("Reset Indicator is now: "));
    RI = lipo.isReset(); // Read the RI flag
    Serial.println(RI); // Print the RI    
  }

  // To quick-start or not to quick-start? That is the question!
  // Read the following and then decide if you do want to quick-start the fuel gauge.
  // "Most systems should not use quick-start because the ICs handle most startup problems transparently,
  //  such as intermittent battery-terminal connection during insertion. If battery voltage stabilizes
  //  faster than 17ms then do not use quick-start. The quick-start command restarts fuel-gauge calculations
  //  in the same manner as initial power-up of the IC. If the system power-up sequence is so noisy that the
  //  initial estimate of SOC has unacceptable error, the system microcontroller might be able to reduce the
  //  error by using quick-start."
  // If you still want to try a quick-start then uncomment the next line:
	//lipo.quickStart();

  // Read and print the device ID
  Serial.print(F("Device ID: 0x"));
  uint8_t id = lipo.getID(); // Read the device ID
  if (id < 0x10) Serial.print(F("0")); // Print the leading zero if required
  Serial.println(id, HEX); // Print the ID as hexadecimal

  // Read and print the device version
  Serial.print(F("Device version: 0x"));
  uint8_t ver = lipo.getVersion(); // Read the device version
  if (ver < 0x10) Serial.print(F("0")); // Print the leading zero if required
  Serial.println(ver, HEX); // Print the version as hexadecimal

  // Read and print the battery threshold
  Serial.print(F("Battery empty threshold is currently: "));
  Serial.print(lipo.getThreshold());
  Serial.println(F("%"));

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% and 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.

  // Read and print the battery empty threshold
  Serial.print(F("Battery empty threshold is now: "));
  Serial.print(lipo.getThreshold());
  Serial.println(F("%"));

  // Read and print the high voltage threshold
  Serial.print(F("High voltage threshold is currently: "));
  float highVoltage = ((float)lipo.getVALRTMax()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
  Serial.print(highVoltage, 2);
  Serial.println(F("V"));

  // Set the high voltage threshold
  lipo.setVALRTMax((float)4.1); // Set high voltage threshold (Volts)

  // Read and print the high voltage threshold
  Serial.print(F("High voltage threshold is now: "));
  highVoltage = ((float)lipo.getVALRTMax()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
  Serial.print(highVoltage, 2);
  Serial.println(F("V"));

  // Read and print the low voltage threshold
  Serial.print(F("Low voltage threshold is currently: "));
  float lowVoltage = ((float)lipo.getVALRTMin()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
  Serial.print(lowVoltage, 2);
  Serial.println(F("V"));

  // Set the low voltage threshold
  lipo.setVALRTMin((float)3.9); // Set low voltage threshold (Volts)

  // Read and print the low voltage threshold
  Serial.print(F("Low voltage threshold is now: "));
  lowVoltage = ((float)lipo.getVALRTMin()) * 0.02; // 1 LSb is 20mV. Convert to Volts.
  Serial.print(lowVoltage, 2);
  Serial.println(F("V"));

  // Enable the State Of Change alert
  Serial.print(F("Enabling the 1% State Of Change alert: "));
  if (lipo.enableSOCAlert())
  {
    Serial.println(F("success."));
  }
  else
  {
    Serial.println(F("FAILED!"));
  }
  
  // Read and print the HIBRT Active Threshold
  Serial.print(F("Hibernate active threshold is: "));
  float actThr = ((float)lipo.getHIBRTActThr()) * 0.00125; // 1 LSb is 1.25mV. Convert to Volts.
  Serial.print(actThr, 5);
  Serial.println(F("V"));

  // Read and print the HIBRT Hibernate Threshold
  Serial.print(F("Hibernate hibernate threshold is: "));
  float hibThr = ((float)lipo.getHIBRTHibThr()) * 0.208; // 1 LSb is 0.208%/hr. Convert to %/hr.
  Serial.print(hibThr, 3);
  Serial.println(F("%/h"));

  // Initialize WiFi in low-power mode
  WiFi.mode(WIFI_STA); // Set to station mode by default

  esp_task_wdt_reset();
}

// Sets RGB LEDs pixels
void colorSet(uint32_t c, uint8_t wait) { // From NeoPixel Library
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
   strip.show();
   delay(wait);
}

void red() {
  colorSet(strip.Color(0, 25, 0, 0), 0); // Red
}

void green() {
  colorSet(strip.Color(25, 0, 0, 0), 0); // Green
}

void blue() {
  colorSet(strip.Color(0, 0, 25, 0), 0); // Blue
}

void white() {
  colorSet(strip.Color(0, 0, 0, 50), 0); // WHITE
}

void halfWHITE() {
  colorSet(strip.Color(0, 0, 0, 25), 0); // half WHITE
}

void setRandomColors() {
  uint8_t maxBrightness = 10;
  for(int i = 1; i < strip.numPixels(); i++) { //Set i=1 to skip over flashlight LED
    strip.setPixelColor(i, strip.Color(random(0, maxBrightness), random(0, maxBrightness), random(0, maxBrightness), 0));
  }
  strip.show();
}

void setDeterminedColors(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorW) {
  for(int i = 1; i < strip.numPixels(); i++) { //Set i=1 to skip over flashlight LED
    strip.setPixelColor(i, strip.Color(colorR, colorG, colorB, colorW));
  }
  strip.show();
}

void setColorsOff() {
  for(int i = 0; i < strip.numPixels(); i++) { //Set i=1 to skip over flashlight LED
    strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
  }
  strip.show();
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=1; i<strip.numPixels(); i++) { //Set i=1 to skip over flashlight LED
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

void accelerometer(){
    //Print accel values only if new data is available
  if (accel.available())
  {
    accelX = accel.getX();
    accelY = accel.getY();
    accelZ = accel.getZ();
    tempC = accel.getTemperature();

    // Serial.print("Acc [mg]: ");
    // Serial.print(accelX, 1);
    // Serial.print(" x, ");
    // Serial.print(accelY, 1);
    // Serial.print(" y, ");
    // Serial.print(accelZ, 1);
    // Serial.print(" z, ");
    // Serial.print(tempC, 1);
    // Serial.print("C");
    // Serial.println();

    //    int rawX = accel.getRawX();
    //    int rawY = accel.getRawY();
    //    int rawZ = accel.getRawZ();
    //
    //    Serial.print("Acc raw: ");
    //    Serial.print(rawX);
    //    Serial.print(" x, ");
    //    Serial.print(rawY);
    //    Serial.print(" y, ");
    //    Serial.print(rawZ);
    //    Serial.print(" z");
    //    Serial.println();
  }
}



void sliderPositionRead(){
  sliderPosition_Millivolts = analogReadMilliVolts(VOLT_READ_PIN); // Read through ADC with calibrated return
  sliderPosition_12Bits = analogRead(VOLT_READ_PIN); // Read through 12-bit ADC as raw bits into 16-bit var
  sliderPosition_12Bits_Inverted =  4095 - sliderPosition_12Bits;
  
  sliderPosition_8Bits =  255 - ((sliderPosition_12Bits * 255) / 4095); // Map 12-bit to 8-bit variable
  sliderPosition_8Bits_Inverted = (sliderPosition_12Bits * 255) / 4095; // Map 12-bit to 8-bit variable, inverted
    
  sliderPosition_Percentage = 100 - ((sliderPosition_12Bits * 100) / 4095); // Map 12-bit to percentage (0-100), inverted
}

void screenUpdate(){
  // clear the display
  //display.clear(); // Removed to support reaction time game

  // draw the current demo method
  demos[demoMode]();

  if (demoMode == 11) {
      accelerometerScreenEnabled = true;
  } else {
      accelerometerScreenEnabled = false;
  }

  if (demoMode == 15) {
      reactionGameEnabled = true;
  } else {
      reactionGameEnabled = false;
  }

  // if (demoMode == 17) {
  //     reactionGameEnabled = true;
  // } else {
  //     reactionGameEnabled = false;
  // }
  
  if (demoMode == 18) {
      clockScreenEnabled = true;
  } else {
      clockScreenEnabled = false;
  }

  if (demoMode != 13){
    audioPlayerRunning = false;
  }

  // // write the buffer to the display
  // display.display();
  updateScrollPositionFromSlider();
}

void serialPrinter(){
  // For debugging: print the button counters to the serial monitor
  // for (int i = 0; i < numButtons; i++) {
  //   Serial.print("Button ");
  //   Serial.print(i + 1);
  //   Serial.print(": ");
  //   Serial.println(buttonCounter[i]);
  // }
}

void ledSequencer(){
  if(ledSequencerEnabled){
    //colors[colorMode]();
    //rainbow(20);
    
    // Cycle through available screens
    //demoMode = (demoMode + 1)  % demoLength;
    
    
    //colorMode = (colorMode + 1) % colorLength;
    // Set NeoPixel colors
    setRandomColors(); // or rainbow(20);

    // Display the updated NeoPixel strip
    strip.show();
  }
}

void loop() {
  if(wifimanager_nonblocking) {
    wifiManager.process(); // avoid delays() in loop when non-blocking and other long running code 
  } 
  esp_task_wdt_reset();
  millisNow = millis();
  
  // Audio sounds
  copier.copy();
  action.processActions();

  if(reactionGameEnabled){
    Serial.println("reactionGameEnabled loop()");
    reactionGame.update(millisNow);
  }
  if (simonSaysGameEnabled && simonSaysGame != nullptr) {
      simonSaysGame->update(millis());
  }
  else{
    // Speaker Audio Handler thing
    //player.copy();

    // Megunolink Command Handler 
    // Check for new commands at the serial port
    //SerialCommandHandler.Process();
    
    if((millisNow - millisOld10) >= 10){
      //Calculate cycle time roughly from millis measurement
      //MyTable.SendData("millis", millis());
      //MyTable.SendData("10ms Task", (millisNow - millisOld10));
      millisOld10 = millisNow;

      if(demoMode != 14) {
        if(flashlightStatus){
          flashlightSwitch(false);
        }
      }

      // Reset LEDs
      if (demoModePreviously == 11 || demoModePreviously == 14 || demoModePreviously == 17) { 
        setColorsOff();    
      }

      // Turn WiFi back off
      if (demoModePreviously == 20) {
        stopWebServer();
      }
      
      // switchStateUpdate();
      // detectButtonState(buttonPosition, buttonState, buttonDirection);
      // screenMacroUnlockEarlyRelease();
      

    }

    if((millisNow - millisOld50) >= 50){
      //Calculate cycle time roughly from millis measurement
      //MyTable.SendData("50ms Task", (millisNow - millisOld50));
      millisOld50 = millisNow;

      accelerometer();
      sliderPositionRead();
      screenUpdate();
      
      // canTask();

      // voltageSenseRead();
      // homingManager();
      // screenMacroCoordinator();
      // screenFaultCoordinator();
      // screenTimeoutCoordinator();
      // nvmCANHandler();
      // masterWatchdogDeadSwitch();
    }

    if((millisNow - millisOld200) >= 200){
      //Calculate cycle time roughly from millis measurement
      //MyTable.SendData("200ms task", (millisNow - millisOld200));
      millisOld200 = millisNow;
      
      // updateScreenDebugValues();
      // updateScreenOverrideValues();
      // nvmPositionStorage();
      
      fuelGaugeUpdate();
      
      // Slow NVM write cycle, only check every
      if((millisNow - millisLastInteraction) >= 3000){
        Serial.println("demoMode: " + String(demoMode));
        //demoMode = 10;
        saveButtonCounters();
      }  
    }
  }

  if((millisNow - millisLastInteraction) >= 30000){
    // Go to deep sleep
    if(preventSleepWhileCharging){
      if(lipo.getChangeRate() < sleepChargingChangeThreshold){ // If discharging greater than 10% per hour, shut down
        Serial.println("Going to sleep now...");
        delay(1000);
        esp_deep_sleep_start();
      }
    } else {
      Serial.println("Going to sleep now...");
      delay(1000);
      esp_deep_sleep_start();
    }
  }

  if((millisNow - millisOldHeartbeat) >= 600000){
    //Calculate cycle time roughly from millis measurement
    //MyTable.SendData("Heartbeat task", (millisNow - millisOldHeartbeat)); //2mins = 120,000
    millisOldHeartbeat = millisNow;
    
    //dataLoggerHeartbeat();
  }

  if (DEMO_DURATION != 0){ // Disable demo mode if duration is set to 0
    if((millisNow - millisDemoMode) >= DEMO_DURATION){
      ledSequencer();
      millisDemoMode = millisNow;
    }
  }
  // particlePubRequestManager(); //Poll for publish requests
  // dataLoggerNvmRequestManager(); //Poll for NVM requests
}

void drawFontFaceDemo() {
  display.clear();
  // Font Demo1
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //display.setFont(ArialMT_Plain_10);
  //display.drawString(0, 0, "Hello world");
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 18, "Hello world");
  //display.setFont(ArialMT_Plain_24);
  //display.drawString(0, 26, "Hello world");
  display.display();
}

void drawTextFlowDemo() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 128,
  "Lorem ipsum confused the non-super nerds so here is some plain english filler to demo text wrapping" );
  display.display();
}

void drawTextAlignmentDemo() {
  display.clear();
  // Text alignment demo
  display.setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 33, "Right aligned (128,33)");
  display.display();
}

void drawRectDemo() {
  display.clear();
  // Draw a pixel at given position
  for (int i = 0; i < 10; i++) {
    display.setPixel(i, i);
    display.setPixel(10 - i, i);
  }
  display.drawRect(12, 12, 20, 20);

  // Fill the rectangle
  display.fillRect(14, 14, 17, 17);

  // Draw a line horizontally
  display.drawHorizontalLine(0, 40, 20);

  // Draw a line horizontally
  display.drawVerticalLine(40, 0, 20);
  display.display();
}

void drawCircleDemo() {
  display.clear();
  for (int i = 1; i < 8; i++) {
    display.setColor(WHITE);
    display.drawCircle(32, 32, i * 3);
    if (i % 2 == 0) {
      display.setColor(BLACK);
    }
    display.fillCircle(96, 32, 32 - i * 3);
  }
  display.display();
}

void drawProgressBarDemo() {
  display.clear();
  int progress = (buttonCounter[5] / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 30, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
  display.display();
}

void drawImageDemo_1() {
  display.clear();
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html or https://github.com/ThingPulse/esp8266-oled-ssd1306
  // on how to create xbm files
  //display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.drawXbm(40, 0, Logo_Round_Tilted_width, Logo_Round_Tilted_height, Logo_Round_Tilted_bits);
  //display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo_w_icon);
  display.display();
}

void drawImageDemo_2() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo_w_icon);
  display.display();
}

void drawImageDemo_3() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_cf_logo);
  display.display();
}

void drawImageDemo_4() {
  display.clear();
  display.drawXbm(0, 0, 128, 64, epd_bitmap_retro_car_sunset);
  display.display();
}

void drawBatteryProgressBar() {
  display.clear();
  // draw the progress bar
  display.drawProgressBar(0, 30, 120, 10, batteryVoltagePercentage);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, "Battery: " + String(batteryVoltagePercentage) + "%");

  // Battery Voltage
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(70, 40, "Battery (V): " + String(batteryVoltage));

  // // Battery Voltage Percentage
  // display.setFont(ArialMT_Plain_10);
  // display.setTextAlignment(TEXT_ALIGN_CENTER);
  // display.drawString(70, 30, "Bat %: " + String(pinVoltagePercentage));
  display.display();
}

// Function to map input (0 to 4095) to a rainbow progression (red -> orange -> yellow -> green t-> blue -> violet)
void mapToRainbow(int input, uint8_t dim, uint8_t &red, uint8_t &green, uint8_t &blue) {
    // Ensure input and dim are within bounds
    input = input < 0 ? 0 : (input > 4095 ? 4095 : input);
    dim = dim > 255 ? 255 : dim;

    // Normalize input to range 0.0–1.0
    float normalized = (float)input / 4095.0;

    // Calculate which segment of the rainbow we're in
    float x, y, z;
    if (normalized < 0.2) {  // Red → Orange
        float t = normalized / 0.2;
        x = 1.0;
        y = t;
        z = 0.0;
    } else if (normalized < 0.4) {  // Orange → Yellow
        float t = (normalized - 0.2) / 0.2;
        x = 1.0;
        y = 1.0;
        z = 0.0;
    } else if (normalized < 0.6) {  // Yellow → Green
        float t = (normalized - 0.4) / 0.2;
        x = 1.0 - t;
        y = 1.0;
        z = 0.0;
    } else if (normalized < 0.8) {  // Green → Blue
        float t = (normalized - 0.6) / 0.2;
        x = 0.0;
        y = 1.0 - t;
        z = t;
    } else {  // Blue → Violet
        float t = (normalized - 0.8) / 0.2;
        x = t;
        y = 0.0;
        z = 1.0;
    }

    // Scale RGB values by brightness (dim)
    red = (uint8_t)((y * dim));
    green = (uint8_t)((x * dim));
    blue = (uint8_t)((z * dim));
}

void drawSliderProgressBar() {
  display.clear();
  // draw the progress bar
  display.drawProgressBar(9, 28, 108, 10, sliderPosition_Percentage);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 14, "Slider: " + String(sliderPosition_Percentage) + "%");

  // Battery Voltage
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 40, "Slider (mV): " + String(sliderPosition_Millivolts));
  display.drawString(64, 50, "Slider (bits): " + String(sliderPosition_12Bits));
  display.display();

  uint8_t red, green, blue;
  mapToRainbow(sliderPosition_12Bits, 8, red, green, blue);
  strip.setPixelColor(pixel_Front_Top, strip.Color(red, green, blue, 0));
  strip.setPixelColor(pixel_Front_Middle, strip.Color(8 - red, 8 - green, 8 - blue, 0));
  strip.setPixelColor(pixel_Front_Bottom, strip.Color(red, green, blue, 0));

  strip.show();
}

void drawAccelerometerScreen() {
  if (accelerometerScreenEnabled) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 20, "X: " + String(accelX) + " Y: " + String(accelY) + " Z: " + String(accelZ));

    uint8_t redMap = map(accelX, -1030, 1030, 0, 255);
    uint8_t greenMap = map(accelY, -1030, 1030, 0, 255);
    uint8_t blueMap = map(accelZ, -1030, 1030, 0, 255);

    setDeterminedColors(redMap, greenMap, blueMap, 0); 
    ESP_LOGV(TAG_MAIN, "Accel RGB Values x=%d, y=%d z=%d", redMap, greenMap, blueMap);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "R: " + String(redMap) + " G: " + String(greenMap) + " B: " + String(blueMap));
    display.display();
  }
}

void drawButtonCounters() {
  display.clear();
  // Button Counters
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, 
    "Btns: 0="  + String(buttonCounter[0])
    + ", 1=" + String(buttonCounter[1])
    + ", 2=" + String(buttonCounter[2]));
  display.drawString(0, 20, 
    ", 3=" + String(buttonCounter[3])
    + ", 4=" + String(buttonCounter[4])
    + ", 5=" + String(buttonCounter[5]));

  // display.setFont(ArialMT_Plain_10);
  // display.setTextAlignment(TEXT_ALIGN_RIGHT);
  // // Print the button counters in a 3x2 grid
  // const int xOffset = 20; // Shift the grid to the right
  // for (int i = 0; i < numButtons; i++) {
  //   int x = xOffset + (i % 3) * 40; // Adjust the x position for each column
  //   int y = (i / 3) * 20; // Adjust the y position for each row
  //   display.drawString(x, y, "Btn " + String(i + 1) + ": " + String(buttonCounter[i]));
  // }
  display.display();
}

void drawTimeOnCounter() {
  display.clear();
  // Time On Counter
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 20, String(millis()));
  display.display();
}

void drawAudioPlayer() {
  float volumeSlider = float(sliderPosition_Percentage) / 100.0;
  volume.setVolume(volumeSlider);
  if(!audioPlayerRunning){
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(suiGenerisRg_20);
    display.drawString(64, 10, "Booper");
    display.display();
    audioPlayerRunning = true;
  }
  Serial.print("Volume: " + String(volume.volume()));
}

void flashlightSwitch(bool flashlightEnable){
  if(flashlightEnable){
    //setDeterminedColors(0, 0 , 0, 0);
    strip.setPixelColor(pixel_Back, strip.Color(sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits));
    strip.setPixelColor(pixel_Front_Top, strip.Color(sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits));
    strip.setPixelColor(pixel_Front_Middle, strip.Color(sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits));
    strip.setPixelColor(pixel_Front_Bottom, strip.Color(sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits, sliderPosition_8Bits));
    strip.show();
    flashlightStatus = true;
  }
  if(!flashlightEnable && flashlightStatus){
    //setDeterminedColors(0, 0 , 0, 0);
    strip.setPixelColor(pixel_Back, strip.Color(0, 0, 0, 0));
    strip.setPixelColor(pixel_Front_Top, strip.Color(0, 0, 0, 0));
    strip.setPixelColor(pixel_Front_Middle, strip.Color(0, 0, 0, 0));
    strip.setPixelColor(pixel_Front_Bottom, strip.Color(0, 0, 0, 0));
    strip.show();
    flashlightStatus = false;
  }
}

void drawFlashlight() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 18, "Flashlight");
  display.display();
  flashlightSwitch(true);
}

/*
Reaction Time Game
*/

void drawReactionTimeGame() {
  //display.clear();
  //reactionGame.update(millisNow);
  //display.display();
  if(!reactionGameEnabled){
    reactionGameEnabled = true;
    Serial.println("drawReactionTimeGame fired");
    detachInterrupt(digitalPinToInterrupt(button_BottomRight));
    reactionGame.enableISR();
  }
}

/*
Simon Says Game
*/
void drawSimonSaysGame(int numButtons) {
    int btnPins[4] = {button_TopLeft, button_TopRight, button_MiddleLeft, button_MiddleRight};
    
    if (simonSaysGame != nullptr) {
        delete simonSaysGame;  // Clean up the previous instance
    }

    simonSaysGame = new SimonSaysGame(display, btnPins, numButtons);
    simonSaysGameEnabled = true;
    simonSaysGame->enableISR();
}

void startSimonSaysGame() {
    int numButtons = 3;  // Example: Start the game with 3 buttons
    drawSimonSaysGame(numButtons);
}

void fuelGaugeUpdate() {
  batteryVoltagePercentage = lipo.getSOC();
  batteryVoltage = lipo.getVoltage();

  // Print the variables:
  Serial.print("Voltage: ");
  Serial.print(lipo.getVoltage());  // Print the battery voltage
  Serial.print("V");

  Serial.print(" Percentage: ");
  Serial.print(lipo.getSOC(), 2); // Print the battery state of charge with 2 decimal places
  Serial.print("%");

  Serial.print(" Change Rate: ");
  Serial.print(lipo.getChangeRate(), 2); // Print the battery change rate with 2 decimal places
  Serial.print("%/hr");

  Serial.print(" Alert: ");
  Serial.print(lipo.getAlert()); // Print the generic alert flag

  Serial.print(" Voltage High Alert: ");
  Serial.print(lipo.isVoltageHigh()); // Print the alert flag

  Serial.print(" Voltage Low Alert: ");
  Serial.print(lipo.isVoltageLow()); // Print the alert flag

  Serial.print(" Empty Alert: ");
  Serial.print(lipo.isLow()); // Print the alert flag

  Serial.print(" SOC 1% Change Alert: ");
  Serial.print(lipo.isChange()); // Print the alert flag
  
  Serial.print(" Hibernating: ");
  Serial.print(lipo.isHibernating()); // Print the alert flag
  
  Serial.println();

  // Logic to check for battery State of Charge and disable the LiPo charger as desired per the calibration
  // Requires Jumper on R64 to be soldered
  // Need to modify this so the LED isn't kept on when the device isn't charging
  if(enableBatterySOCCutoff) {
    if ((batteryVoltagePercentage > batterySOCCutoff) && (lipo.getChangeRate() > sleepChargingChangeThreshold)) {
      pinMode(CHRG_ENA, OUTPUT);
      digitalWrite(CHRG_ENA, HIGH);
      Serial.println("Charging Disabled, SOC Cutoff Reached");
    }
    else {
      pinMode(CHRG_ENA, OUTPUT);
      digitalWrite(CHRG_ENA, LOW);
      Serial.println("Charging Enabled, SOC Cutoff Not Reached");
    }
  }
}

// New clock drawing function
void drawClockDemo() {
  //struct tm currentTime;
  if (!clockScreenEnabled){
    clockScreenEnabled = true;
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(suiGenerisRg_20);
    display.drawString(64, 22, "--:--:--");
    display.display();
    
    // WiFi
    connectToWiFi();

    // Initialize RTC with the current time if needed
    // Set timezone if needed
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov");
    
    
    // Attempt to get time with multiple retries
    for (int i = 0; i < 5; i++) {  // Try up to 10 times
      if (getLocalTime(&currentTime)) {
        Serial.println("Successfully obtained time.");
        break;
      } else {
        Serial.println("Failed to obtain time, retrying...");
        delay(1000);  // Wait a bit before retrying
      }
    }

    // if (!getLocalTime(&currentTime)) {
    //   Serial.println("Failed to obtain time from NTP, setting default time.");
      
    //   // Manually set the default time (e.g., January 1, 2024, at 12:00 PM)
    //   currentTime.tm_year = 2024 - 1900; // Years since 1900
    //   currentTime.tm_mon = 0;            // January is 0
    //   currentTime.tm_mday = 1;           // First day of the month
    //   currentTime.tm_hour = 12;
    //   currentTime.tm_min = 0;
    //   currentTime.tm_sec = 0;
    //   time_t defaultTime = mktime(&currentTime);
    //   struct timeval tv = { .tv_sec = defaultTime };
    //   settimeofday(&tv, NULL);  // Set the ESP32's internal clock to this time

    //   Serial.println("Default time set.");
    // } else {
    //   Serial.println("Successfully obtained time.");
    // }
  }

  if (getLocalTime(&currentTime)) {
    char timeString[9];
    strftime(timeString, sizeof(timeString), "%I:%M:%S", &currentTime);

    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(suiGenerisRg_20);
    display.drawString(64, 22, String(timeString));
    display.display();
  }

}

// Function to set the time from structure
void updateTime(struct tm *currentTime) {
  time_t now = mktime(currentTime);
  timeval epoch = { .tv_sec = now, .tv_usec = 0 };
  settimeofday(&epoch, NULL);
}

// Increase time by 1 minute
void increaseMinute(struct tm *currentTime) {
  currentTime->tm_min += 1;
  updateTime(currentTime);
}

// Decrease time by 1 minute
void decreaseMinute(struct tm *currentTime) {
  currentTime->tm_min -= 1;
  updateTime(currentTime);
}

// if(clockScreenEnabled) {
//   if (getLocalTime(&currentTime)) {
//     updateTime(&currentTime);
// }

// if(buttonCounter[2]) { // Assuming button_MiddleLeft decrements time
//   decreaseMinute(&currentTime);
// }

// if(buttonCounter[3]) { // Assuming button_MiddleRight increments time
//   increaseMinute(&currentTime);
// }

// Function to connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  //WiFi.begin(wifi_ssid, wifi_password);
  WiFi.begin();
  // while (WiFi.status() != WL_CONNECTED) {
  //   //delay(500);
  //   Serial.print(".");
  // }
  Serial.println(" Connected!");
}

void startWebServer() {
  if (!isWebServerRunning) {
    disableWatchdog();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", "<h1>Cyber Fidget Config Portal</h1>");
    });

    wifiManager.setConfigPortalTimeout(60);
    wifiManager.setConfigPortalBlocking(wifimanager_nonblocking);
    server.begin(); // Start the web server
    isWebServerRunning = true;
    Serial.println("Web Server Started");
  }
}

void stopWebServer() {
  if (isWebServerRunning) {
    server.end(); // Stop the web server
    isWebServerRunning = false;
    Serial.println("Web Server Stopped");
    enableWatchdog();
  }
}

void startWiFiManager() {
  if (!wifiManager.startConfigPortal("CyberFidget_AP")) {
    Serial.println("Failed to connect to WiFi");
  } else {
    Serial.println("WiFi Connected");
    startWebServer(); // Start server after successful WiFi connection
  }
}

void drawWifiConfig() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10); // Use an appropriate font size
  display.drawString(64, 22, "Starting WiFi Portal...");
  display.display();
  startWiFiManager();
  //startWebServer();
}

void drawSerialDataScreenWrapper() {
    newDataReceived = false;

    while (Serial.available() > 0) {
        char incomingChar = Serial.read();
        newDataReceived = true;

        if (incomingChar == '\n') {
            // Parse incoming data and determine the number of lines
            String parsedData[10];
            int parsedLineCount = 0;
            int startIndex = 0;
            int commaIndex;
            while ((commaIndex = incomingData.indexOf(",", startIndex)) != -1 && parsedLineCount < 10) {
                String segment = incomingData.substring(startIndex, commaIndex); // Extract substring
                segment.trim(); // Trim whitespace in-place
                parsedData[parsedLineCount++] = segment; // Store trimmed segment in parsedData
                startIndex = commaIndex + 1;
            }
            String lastSegment = incomingData.substring(startIndex);
            lastSegment.trim();
            parsedData[parsedLineCount++] = lastSegment; 

            // Check if the number of lines has changed
            bool lineCountChanged = (parsedLineCount != lineCount);

            // Update dataLines and reset scroll position only if line count changes
            if (lineCountChanged) {
                lineCount = parsedLineCount;
                for (int i = 0; i < lineCount; i++) {
                    dataLines[i] = parsedData[i];
                }
                currentLine = 0; // Reset scroll position only if line count changes
                scrollOffset = 0; // Reset pixel offset
            }

            drawSerialDataScreen();
            isScreenUpdated = true;
            incomingData = ""; // Clear buffer for the next message
            lastDataTime = millis();
        } else {
            incomingData += incomingChar;
        }
    }

    // Handle the timeout scenario for default info screen
    if (millis() - lastDataTime > dataTimeout && !newDataReceived && lineCount == 0) {
        drawDefaultInfoScreen();
        isScreenUpdated = true;
    }
}

void drawSerialDataScreen() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    if (currentScrollMode == LINE_SCROLL) {
        // Line-based scrolling: Display lines based on currentLine
        for (int i = 0; i < maxLinesOnScreen && i + currentLine < lineCount; i++) {
            display.drawString(0, i * 10, dataLines[currentLine + i]);
        }
    } else if (currentScrollMode == PIXEL_SCROLL) {
        // Pixel-based scrolling: Apply pixel offset across all lines
        for (int i = 0; i < lineCount; i++) {
            int yPos = i * 10 - scrollOffset; // Calculate pixel offset for each line
            if (yPos >= -10 && yPos <= 64) {  // Only render lines within the display’s visible area
                display.drawString(0, yPos, dataLines[i]);
            }
        }
    }

    display.display();
}

void drawDefaultInfoScreen() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10); // Use an appropriate font size
  display.drawString(64, 22, "Waiting for Data...");
  display.display();
}

void handleScrollUp() {
  if (currentScrollMode == LINE_SCROLL) {
    // Line-based scrolling
    if (currentLine > 0) {
      currentLine--;
      drawSerialDataScreen();
    }
  } else if (currentScrollMode == PIXEL_SCROLL) {
    // Pixel-based scrolling
    if (scrollOffset > 0) {
      scrollOffset--;
      drawSerialDataScreen();
    } else if (currentLine > 0) {
      currentLine--;
      scrollOffset = maxScrollOffset;
      drawSerialDataScreen();
    }
  }
}

void handleScrollDown() {
    if (currentScrollMode == LINE_SCROLL) {
        // Line-based scrolling
        if (currentLine < lineCount - maxLinesOnScreen) {
            currentLine++;
            drawSerialDataScreen();
        }
    } else if (currentScrollMode == PIXEL_SCROLL) {
        // Pixel-based scrolling
        if (scrollOffset < maxScrollOffset) {
            scrollOffset++;
            drawSerialDataScreen();
        } else if (currentLine < lineCount - 1) {
            currentLine++;
            scrollOffset = 0;
            drawSerialDataScreen();
        }
    }
}

void updateScrollPositionFromSlider() {
    if (currentScrollMode == LINE_SCROLL) {
        // Line-based scrolling: Map slider to line count
        int newLine = map(sliderPosition_12Bits, 0, 4095, 0, max(lineCount - maxLinesOnScreen, 0));
        
        if (newLine != previousLine) { // Only update if there's a change
            currentLine = newLine;
            drawSerialDataScreen();
            isScreenUpdated = true;
            previousLine = newLine;
        }

    } else if (currentScrollMode == PIXEL_SCROLL) {
        // Pixel-based scrolling: Map slider to total scrollable height across all lines
        int totalScrollablePixels = max(0, (lineCount * 10) - 64); // 64 is the height of the display
        int scrollPosition = map(sliderPosition_12Bits, 0, 4095, 0, totalScrollablePixels);

        // Calculate new line and offset from scrollPosition
        int newLine = scrollPosition / 10;
        int newScrollOffset = scrollPosition % 10;

        // Update screen only if there's a change in line or offset
        if (newLine != previousLine || newScrollOffset != previousScrollOffset) {
            currentLine = newLine;
            scrollOffset = newScrollOffset;
            drawSerialDataScreen();
            isScreenUpdated = true;
            previousLine = newLine;
            previousScrollOffset = newScrollOffset;
        }
    }
}

void toggleScrollMode() {
    if (currentScrollMode == LINE_SCROLL) {
        currentScrollMode = PIXEL_SCROLL;
    } else {
        currentScrollMode = LINE_SCROLL;
        scrollOffset = 0; // Reset pixel offset when switching to line scroll mode
    }
    drawSerialDataScreen(); // Update screen to reflect new scroll mode
}


/*
Audio Boops with Buttons
*/
void actionKeyOn(bool active, int pin, void* ptr){
  if (audioPlayerRunning){
    float freq = *((float*)ptr);
    sine.setFrequency(freq);
    in.begin();
  }
}


void actionKeyOff(bool active, int pin, void* ptr){
  in.end();
}

// We want to play some notes on the Audioi2s keys 
void setupActions(){
  // assign buttons to notes
  auto act_low = AudioActions::ActiveLow;
  static float note[] = {N_C3, N_D3, N_E3, N_F3, N_G3, N_A3}; // frequencies
  action.add(button_TopLeft, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[0])); // C3
  action.add(button_TopRight, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[1])); // D3
  action.add(button_MiddleLeft, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[2])); // E3
  action.add(button_MiddleRight, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[3])); // F3
  action.add(button_BottomLeft, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[4])); // G3
  action.add(button_BottomRight, actionKeyOn, actionKeyOff, AudioActions::ActiveLow, &(note[5])); // A3
}