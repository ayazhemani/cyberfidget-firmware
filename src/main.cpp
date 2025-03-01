

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
WiFiManagerCF WiFiManagerCFObject;

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

#include <time.h> // Clock/RTC for time manipulation

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
ReactionTimeGame reactionGame(
  display, 
  button_BottomRightIndex /* button index */, 
  buttonManager
  );

#include "ClockDisplay.h"
ClockDisplay clockDisplay(display);

#include "SPHFluidGame.h"
SPHFluidGame sphGame(display);

#include "BreakoutGame.h"
BreakoutGame breakout(display);

#include "SimonSaysGame.h"
SimonSaysGame simonGame(display, buttonManager, beepForSquareFn, beepOnUserPressFn);

#include "MatrixScreensaver.h"
MatrixScreensaver matrixScreensaver(display);

#include "DinoGame.h"
DinoGame dinoGame(
  display,
  buttonManager, 
  /*jumpBtnIndex=*/button_MiddleLeftIndex, 
  /*duckBtnIndex=*/button_MiddleRightIndex,
  /*resetBtnIndex*/button_BottomRightIndex
  );

#include "PowerManager.h"  // Include the new PowerManager
PowerManager powerManager(display, buttonManager, clockDisplay);

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

  // Initialising the UI will init the display too.
  display.init();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // RGBW LEDs
  initRGB();

  // Slider
  pinMode(VOLT_READ_PIN, INPUT); // Slider Input
  sliderPositionFilterInit();

  // Buttons
   buttonManager.begin(); 

  if (accel.begin() == false)
  {
    ESP_LOGE(TAG_MAIN, "Accelerometer not detected. Check address config and power regulator state. Freezing...");
    while (1)
      ;
  }

  // Memory Management
  buttonManager.loadButtonCounters();

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

  WiFiManagerCFObject.init();
  WiFiManagerCFObject.setDisplay(display); // Pass the display reference if needed

  // Initially, the reaction game is not active, so ensure its callback is unregistered
  buttonManager.unregisterCallback(reactionGame.getButtonIndex());

  // Initialize the PowerManager
  powerManager.begin();




  // To be removed from setup()
  // Initialize the breakout game
  breakout.reset();
  breakout.setResetButton(button_BottomRight, /* activeLow = */ true); // Specify which pin to monitor for game resets
  breakout.setBounceCallback(beepOnBounce); // **Attach the bounce callback** so BreakoutGame calls beepOnBounce

  // Matrix Screensaver
  matrixScreensaver.begin();
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
    if (demoMode == DEMO_ACCELEROMETER){
        accelerometerScreenEnabled = true;
    } 
    else if (demoModePreviously == DEMO_ACCELEROMETER){
        accelerometerScreenEnabled = false;
        setColorsOff();
    }

    if (demoMode == DEMO_FLASHLIGHT) {
      flashlightStatus = true;
    } 
    else if (demoModePreviously == DEMO_FLASHLIGHT) {
      flashlightStatus = false;
      setColorsOff(); 
    }

    if (demoMode == DEMO_REACTION) {
      buttonManager.registerCallback(
        reactionGame.getButtonIndex(),
        ReactionTimeGame::reactionButtonPressedCallback
      );
    } 
    else if (demoModePreviously == DEMO_REACTION) {
      //reactionGameEnabled = false;
      buttonManager.unregisterCallback(reactionGame.getButtonIndex());
      reactionGame.resetGame(); // Reset the game state
    }
      
    if (demoMode == DEMO_CLOCK_DISPLAY) {
      clockDisplay.begin(); // Inside the module, begin() will only do initialization once.
    } 
    else if (demoModePreviously == DEMO_CLOCK_DISPLAY) {
      // If desired, you can call clockDisplay.reset() when leaving demo mode 18.
      clockDisplay.reset();
    }

    // TO BE ADDED WHEN REMOVED FROM SETUP()
    // if (demoMode == DEMO_BREAKOUT) {
    //   breakout.begin();
    // } 
    // else if (demoModePreviously == DEMO_BREAKOUT) {
    //   breakout.end();
    // }

    if (demoMode == DEMO_SIMON_SAYS) {
      simonGame.begin(); // Inside the module, begin() will only do initialization once.
    } 
    else if (demoModePreviously == DEMO_SIMON_SAYS) {
      simonGame.end(); // Unregister button callbacks
    }

    if (demoMode == DEMO_DINO_GAME) {
      //dinoGameEnabled = true;
      buttonManager.registerCallback(
        dinoGame.getJumpButtonIndex(),
        DinoGame::jumpButtonCallback
      );
      buttonManager.registerCallback(
        dinoGame.getDuckButtonIndex(),
        DinoGame::duckButtonCallback
      );
      buttonManager.registerCallback(
        dinoGame.getResetButtonIndex(), 
        DinoGame::resetButtonCallback
      );
    } 
    else if (demoModePreviously == DEMO_DINO_GAME) {
      //dinoGameEnabled = false;
      buttonManager.unregisterCallback(dinoGame.getJumpButtonIndex());
      buttonManager.unregisterCallback(dinoGame.getDuckButtonIndex());
      buttonManager.unregisterCallback(dinoGame.getResetButtonIndex());
      dinoGame.resetGame(); // Reset the game state
    }

    if (demoMode == DEMO_POWER_MANAGER) {
      powerManager.registerShutdownCallback();
    }
    else if (demoModePreviously == DEMO_POWER_MANAGER) {
      powerManager.unregisterShutdownCallback();
    }

    if (demoMode == DEMO_WIFI_CONFIG) {
      // Register WiFiManagerCF callbacks
      buttonManager.registerCallback(
        button_BottomLeftIndex,
        WiFiManagerCF::bottomLeftButtonCallback
      );
      buttonManager.registerCallback(
        button_BottomRightIndex,
        WiFiManagerCF::bottomRightButtonCallback
      );
    }
    else if (demoModePreviously == DEMO_WIFI_CONFIG) {
      buttonManager.unregisterCallback(button_BottomLeftIndex);
      buttonManager.unregisterCallback(button_BottomRightIndex);
    }

    demoModePreviously = demoMode; 
  }
}

void loop() {
  esp_task_wdt_reset();
  millisNow = millis();
  
  // Handle audio streaming + beep logic
  //loopAudio();

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
    ESP_LOGV(TAG_MAIN, "Button #%d => ", ev.buttonIndex);
    switch (ev.eventType) {
      case ButtonEvent_Pressed:
        ESP_LOGV(TAG_MAIN, "Pressed");
        if (ev.buttonIndex == 0) {
            demoModePreviously = demoMode;
            demoMode = (demoMode - 1 + DEMO_COUNT) % DEMO_COUNT;
        }
        break;

      case ButtonEvent_Released:
        ESP_LOGV(TAG_MAIN, "Released after %d ms", ev.duration);
        buttonCounter[ev.buttonIndex]++;
        if (ev.buttonIndex == 1) {
            demoModePreviously = demoMode;
            demoMode = (demoMode + 1) % DEMO_COUNT;
        }
        break;

      case ButtonEvent_Held:
        ESP_LOGV(TAG_MAIN, "Held for %d ms (and still pressed)!", ev.duration);
        break;

      default:
        break;
      }
    }
  }

  if((millisNow - millisOld10) >= 20){
    millisOld10 = millisNow;

    // Reset LEDs
    if (demoModePreviously == 11 || demoModePreviously == 17) { 
      setColorsOff();    
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
    buttonManager.saveButtonCounters();
    
    // // Slow NVM write cycle, only check every
    // if((millisNow - millisLastInteraction) >= 3000){
    //   saveButtonCounters();
    // }  
  }

  // if((millisNow - millisLastInteraction) >= 1800000){
  //   // Go to deep sleep
  //   if(preventSleepWhileCharging){
  //     if(batteryChangeRate < sleepChargingChangeThreshold){ // If discharging greater than 10% per hour, shut down
  //       clockDisplay.saveTime(); // Save the current clock time to Preferences so that it can be recovered later.
  //       Serial.println("Going to sleep now...");
  //       delay(1000);
  //       esp_deep_sleep_start();
  //     }
  //   } else {
  //     Serial.println("Going to sleep now...");
  //     delay(1000);
  //     esp_deep_sleep_start();
  //   }
  // }

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

void drawReactionTimeGame() {
  reactionGame.update(millisNow);
}

void drawSPHFluidGame(){
  // Map to 1–100
  int targetParticleCount = map(sliderPosition_12Bits_Inverted_Filtered, 0, 4095, 1, 100);

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

// Dino Game
void drawDinoGame() {
  dinoGame.setSpeedBySlider(sliderPosition_Percentage_Inverted_Filtered);
  dinoGame.update();
  dinoGame.draw();
}

// Power Manager
void drawPowerManager() {
  powerManager.update();
}

void drawWifiConfig() {
  WiFiManagerCFObject.process();
}

void drawSimonSaysGame(){
  simonGame.update();
}

void drawMatrixScreensaver(){
  // Update the screensaver
  matrixScreensaver.update();
  matrixScreensaver.draw();
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