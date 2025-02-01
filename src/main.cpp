

/**
License Placeholder

*/

// Main Include
#include <Arduino.h>  // Required for Arduino-specific types
#include "globals.h"


#include "AudioManager.h"
#include "BatteryManager.h"
#include "ButtonManager.h"
//displayermanager
#include "RGBController.h"
#include "SliderPosition.h"
#include "WiFiManagerCF.h"

#include "Flashlight.h"
#include "SerialDisplay.h"
#include "ExampleScreens.h"


// Include watchdog timer library
#include <esp_system.h>
#include <esp_task_wdt.h>

// CPU Sleep stuff
#include <esp_sleep.h>

// Create one ButtonManager
ButtonManager buttonManager(s_buttonPins, s_usePullups, 6 /* #buttons */, 
                            20 /* debounce ms */, 
                            1500 /* hold threshold ms, for example */);

// Include the correct display library
// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"


// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, SDA, SCL);  

#include "SparkFun_LIS2DH12.h" //Click here to get the library: http://librarymanager/All#SparkFun_LIS2DH12
SPARKFUN_LIS2DH12 accel;       //Create instance

/*
Clock/RTC
*/
// Include libraries for time manipulation
#include <time.h>

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


/*
Games
Easiest to include these libraries by adding them to the OS's Arduino Library Folder to avoid c_cpp_properties.json mods
*/

// ReactionTimeGame instance
#include "ReactionTimeGame.h"
ReactionTimeGame reactionGame(display, button_BottomRightIndex /* button index */, buttonManager);

#include "ClockDisplay.h"
ClockDisplay clockDisplay(display);

#include "PixelWaterFallGame.h"
PixelWaterfallGame pixelGame(display);

#include "SPHFluidGame.h"
SPHFluidGame sphGame(display);

#include "BreakoutGame.h"
BreakoutGame breakout(display);

#include "SimonSaysGame.h"
SimonSaysGame simonGame(display, beepForSquareFn, beepOnUserPressFn);

#include "MatrixScreensaver.h"
MatrixScreensaver matrixScreensaver(display);

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
  updateClockDisplay,          // 18 - Add the new screen for clock
  drawSerialDataScreen, // 19
  drawWifiConfig, // 20
  drawPixelWaterfallGame, // 21
  drawSPHFluidGame, // 22
  drawBreakoutGame, // 23
  drawSimonSaysGame2, // 24
  drawMatrixScreensaver // 25
  };

int demoLength = (sizeof(demos) / sizeof(Demo));

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

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Initialising the UI will init the display too.
  display.init();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // RGBW LEDs
  initRGB();
  pinMode(VOLT_READ_PIN, INPUT); // Slider Input

  // Buttons
   buttonManager.begin(); 

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

  initAudio();  // Initialize the audio system
  batteryManager.init(); // Initialize fuel gauge and battery management

  // Initialize WiFi in low-power mode
  WiFiManagerCFObject.init();

  // Initially, the reaction game is not active, so ensure its callback is unregistered
  buttonManager.unregisterCallback(reactionGame.getButtonIndex());

  // Set inertia and damping using global variables
  pixelGame.setInertia(pixelGameInertia);
  pixelGame.setDamping(pixelGameInertia);

  // Reset pixel positions
  pixelGame.resetPixels();

  // Initialize the breakout game
  breakout.reset();
  breakout.setResetButton(button_BottomRight, /* activeLow = */ true); // Specify which pin to monitor for game resets
  breakout.setBounceCallback(beepOnBounce); // **Attach the bounce callback** so BreakoutGame calls beepOnBounce

  // Start the Simon game
  simonGame.begin();
  for (int i = 0; i < 4; i++) { // Initialize button states
    //pinMode(buttonPins[i], INPUT_PULLUP); // Done Already
    btns[i].stableState    = false;
    btns[i].lastReading    = true;  
    btns[i].lastChangeTime = 0;
  }
  // randomSeed(analogRead(A0)); // If you want truly random patterns each reset

  // Matrix Screensaver
  matrixScreensaver.begin();

  esp_task_wdt_reset();
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




void screenUpdate(){
  // draw the current demo method
  demos[demoMode]();

  if (demoMode != demoModePreviously){
    if (demoMode == 11){
        accelerometerScreenEnabled = true;
    } 
    else if (demoModePreviously == 11){
        accelerometerScreenEnabled = false;
    }

    if (demoMode == 14) {
      flashlightStatus = true;
    } 
    else if (demoModePreviously == 14) {
      flashlightStatus = false;
      setColorsOff(); 
    }

    if (demoMode == 15) {
      //reactionGameEnabled = true;
      buttonManager.registerCallback(
        reactionGame.getButtonIndex(),
        ReactionTimeGame::reactionButtonPressedCallback
      );
      Serial.println("ReactionTimeGame callback registered: " + String(reactionGame.getButtonIndex()));
    } 
    else if (demoModePreviously == 15) {
      //reactionGameEnabled = false;
      buttonManager.unregisterCallback(reactionGame.getButtonIndex());
      Serial.println("ReactionTimeGame callback unregistered.");
      reactionGame.resetGame(); // Reset the game state
    }
    
  if (demoMode == 18) {
    clockDisplay.begin(); // Inside the module, begin() will only do initialization once.
  } 
  else if (demoModePreviously == 18) {
    // If desired, you can call clockDisplay.reset() when leaving demo mode 18.
    clockDisplay.reset();
  }

    // if (demoMode != 13){
    //   audioPlayerRunning = false;
    // }

    demoModePreviously = demoMode; 
  }
}

void loop() {
  esp_task_wdt_reset();
  millisNow = millis();
  
  // Handle audio streaming + beep logic
  loopAudio();

  // 1) Update the button states
  buttonManager.update();

  // Fetch and handle events
  ButtonEvent ev;
  while (buttonManager.getNextEvent(ev)) {
    // Check if the event is for a button with a callback
    if (buttonManager.hasCallback(ev.buttonIndex)) {
      // Retrieve and invoke the callback
      ButtonCallback cb = buttonManager.getCallback(ev.buttonIndex);
      if (cb != nullptr) {
          cb(ev);  // Invoke the callback with the event
      }
    } 
    else {
    // Global event handling for buttons without callbacks
    Serial.print("Button #");
    Serial.print(ev.buttonIndex);
    Serial.print(" => ");

    switch (ev.eventType) {
      case ButtonEvent_Pressed:
        Serial.println("Pressed");
        if (ev.buttonIndex == 0) {
            demoModePreviously = demoMode;
            demoMode = (demoMode - 1 + demoLength) % demoLength;
        }
        break;

      case ButtonEvent_Released:
        Serial.print("Released after ");
        Serial.print(ev.duration);
        Serial.println(" ms");
        buttonCounter[ev.buttonIndex]++;
        if (ev.buttonIndex == 1) {
            demoModePreviously = demoMode;
            demoMode = (demoMode + 1) % demoLength;
        }
        break;

      case ButtonEvent_Held:
        Serial.print("Held for ");
        Serial.print(ev.duration);
        Serial.println(" ms (and still pressed)!");
        break;

      default:
        break;
      }
    }
  }

  WiFiManagerCFObject.process();  // Non-blocking WiFi processing if enabled

  if((millisNow - millisOld10) >= 20){
    millisOld10 = millisNow;

    // Reset LEDs
    if (demoModePreviously == 11 || demoModePreviously == 17) { 
      setColorsOff();    
    }

    // Turn WiFi back off
    if (demoModePreviously == 20) {
      WiFiManagerCFObject.stopWebServer();
    }

    sliderPositionRead();
    screenUpdate();
  }

  if((millisNow - millisOld50) >= 50){
    millisOld50 = millisNow;
    accelerometer();
  }

  if((millisNow - millisOld200) >= 200){
    millisOld200 = millisNow;
    batteryManager.update();
    
    // Slow NVM write cycle, only check every
    if((millisNow - millisLastInteraction) >= 3000){
      saveButtonCounters();
    }  
  }

  if((millisNow - millisLastInteraction) >= 30000){
    // Go to deep sleep
    if(preventSleepWhileCharging){
      if(batteryChangeRate < sleepChargingChangeThreshold){ // If discharging greater than 10% per hour, shut down
        clockDisplay.saveTime(); // Save the current clock time to Preferences so that it can be recovered later.
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
    millisOldHeartbeat = millisNow;
  }

  if (DEMO_DURATION != 0){ // Disable demo mode if duration is set to 0
    if((millisNow - millisDemoMode) >= DEMO_DURATION){
      ledSequencer();
      millisDemoMode = millisNow;
    }
  }
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

void drawSliderProgressBar() {
  display.clear();
  // draw the progress bar
  display.setFont(ArialMT_Plain_10);
  display.drawProgressBar(9, 28, 108, 10, sliderPosition_Percentage);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 14, "Slider: " + String(sliderPosition_Percentage) + "%");

  // Battery Voltage
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 40, "Slider (mV): " + String(sliderPosition_Millivolts));
  display.drawString(64, 50, "Slider (bits): " + String(sliderPosition_12Bits));
  display.display();

  uint8_t red, green, blue;
  mapToRainbow(sliderPosition_12Bits, 8, red, green, blue);
  strip.setPixelColor(pixel_Front_Top, strip.Color(red, green, blue, 0));
  strip.setPixelColor(pixel_Front_Middle, strip.Color(8 - red, 8 - green, 8 - blue, 0));
  strip.setPixelColor(pixel_Front_Bottom, strip.Color(red, green, blue, 0));

  updateStrip();
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

    setDeterminedColorsFront(redMap, greenMap, blueMap, 0); 
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

/*
Reaction Time Game
*/

void drawReactionTimeGame() {
  reactionGame.update(millisNow);
}

void drawPixelWaterfallGame(){
    // Update the game with new accelerometer data
    pixelGame.update(accelX, accelY);
}

void drawSPHFluidGame(){

    // Map to 1–100
    int targetParticleCount = map(sliderPosition_12Bits_Inverted, 0, 4095, 1, 100);

    // If you want to prevent re-randomizing the fluid on every tiny pot change,
    // check if the count has actually changed.
    static int currentCount = 100; // or whatever you start with
    if (targetParticleCount != currentCount) {
        sphGame.setParticleCount(targetParticleCount);
        currentCount = targetParticleCount;
    }
    // Update the game with new accelerometer data
    sphGame.update(accelX, accelY);
}

void drawBreakoutGame(){
    // Update the game with new accelerometer data
    breakout.update(accelX);
}

// New clock drawing function
void updateClockDisplay() {
  clockDisplay.update();
}

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


void drawWifiConfig() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10); // Use an appropriate font size
  display.drawString(64, 12, "Press Button to Start");
  display.drawString(64, 22, "WiFi Portal");
  display.display();

  // Check if button_BottomRight is pressed
  if (digitalRead(button_BottomRight) == LOW) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10); // Use an appropriate font size
    display.drawString(64, 12, wifiAP_SSID);
    display.drawString(64, 22, "Started...192.168.4.1");
    display.display();
    WiFiManagerCFObject.startWiFiPortal();
  }
}


void drawDefaultInfoScreen() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10); // Use an appropriate font size
  display.drawString(64, 22, "Waiting for Data...");
  display.display();
}

/*
Audio Boops with Buttons
*/

int readWhichButton() {
  int pressedIndex = -1;
  for (int i = 0; i < 4; i++) {
    bool rawReading = (digitalRead(buttonPinsSimonSays[i]) == LOW); 
    if (rawReading != btns[i].lastReading) {
      btns[i].lastChangeTime = millis();
      btns[i].lastReading    = rawReading;
    } else {
      // if stable for > DEBOUNCE_MS, accept new state
      if ((millis() - btns[i].lastChangeTime) > ButtonDebounce::DEBOUNCE_MS) {
        btns[i].stableState = rawReading;
      }
    }
    // if stableState is true => button i is pressed
    if (btns[i].stableState && !btns[i].wasPressed) {
      pressedIndex = i;
      btns[i].wasPressed = true;
      break; 
    }
    // Reset wasPressed if button is released
    if (!btns[i].stableState) {
      btns[i].wasPressed = false;
    }
  }
  return pressedIndex;
}

void drawSimonSaysGame2(){
  // 1) Read debounced button
  int pressed = readWhichButton();

  // 2) Update the Simon game
  simonGame.update(pressed);
  // 3) Manage beep timing (non-blocking)
  updateBeep();

  // If using AudioTools, call your copier.copy() here
}

void drawMatrixScreensaver(){
  // Update the screensaver
  matrixScreensaver.update();
  matrixScreensaver.draw();
}

void beepOn(float freq) {
  Serial.print("BEEP ON @ "); Serial.println(freq);
  // e.g. sine.setFrequency(freq); in.begin();
}

void beepOff() {
  Serial.println("BEEP OFF");
  // e.g. in.end();
}

void updateBeep() {
  if (beepActive && (millis() - beepStartTime >= beepDurationSimonSays)) {
    beepOff();
    beepActive = false;
  }
}

void startBeep(float freq) {
  beepFrequency = freq;
  beepOn(freq);
  beepActive = true;
  beepStartTime = millis();
}

// additional audio controls
Debouncer nextButtonDebouncer(2000);

// ------------------- 4) Callback Functions for the Simon Game -------------------
void beepForSquareFn(int sq) {
  // Use `sq` to pick a beep frequency, for example:
  float freq = 220.0f;
  switch (sq) {
    case 0: freq = 220.0f; break;
    case 1: freq = 330.0f; break;
    case 2: freq = 440.0f; break;
    case 3: freq = 550.0f; break;
  }
  // Then your code to start the beep
  startBeep(freq);
}

// Called when the user presses a correct button (must take int sq)
void beepOnUserPressFn(int sq) {
  // Possibly a different freq set:
  float freq = 660.0f;
  switch (sq) {
    case 0: freq = 660.0f; break;
    case 1: freq = 770.0f; break;
    case 2: freq = 880.0f; break;
    case 3: freq = 990.0f; break;
  }
  startBeep(freq);
}