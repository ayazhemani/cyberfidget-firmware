// AppManager.cpp
#include "AppManager.h"
#include "globals.h"

// Include necessary headers
#include <Arduino.h>
#include "AudioManager.h"
#include "BatteryManager.h"
#include "RGBController.h"
#include "Flashlight.h"
#include "SliderPosition.h"
#include "SerialDisplay.h"
#include "ExampleScreens.h"
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>
#include <Wire.h>
#include <time.h>

#include "ButtonManager.h"
#include "SSD1306Wire.h"
#include "SparkFun_LIS2DH12.h"
#include "WiFiManagerCF.h"
#include "PowerManager.h"

// Define global objects (if not already defined in globals.cpp)
AudioManager audioManager;
ButtonManager buttonManager(s_buttonPins, s_usePullups, numButtons /* #buttons */, 
    BUTTON_DEBOUNCE_MS /* debounce ms */, 
    BUTTON_HOLD_THRESHOLD_MS /* hold threshold ms, for example */);
SSD1306Wire display(0x3c, SDA, SCL); 
SPARKFUN_LIS2DH12 accel; 
WiFiManagerCF WiFiManagerCFObject;

// Other extern declarations...

#include "ClockDisplay.h"
ClockDisplay clockDisplay(display);

PowerManager powerManager(display, buttonManager, clockDisplay);

#include "ReactionTimeGame.h"
ReactionTimeGame reactionGame(
    display, 
    button_BottomRightIndex /* button index */, 
    buttonManager
    );

#include "SPHFluidGame.h"
SPHFluidGame sphGame(display);

#include "BreakoutGame.h"
BreakoutGame breakoutGame(display, buttonManager, audioManager);

#include "DinoGame.h"
DinoGame dinoGame(
  display,
  buttonManager, 
  /*jumpBtnIndex=*/button_MiddleLeftIndex, 
  /*duckBtnIndex=*/button_MiddleRightIndex,
  /*resetBtnIndex*/button_BottomRightIndex
  );

#include "SimonSaysGame.h"
SimonSaysGame simonGame(display, buttonManager, audioManager);

#include "MatrixScreensaver.h"
MatrixScreensaver matrixScreensaver(display);

#include "Booper.h"
Booper booper(buttonManager, audioManager, display);

// Include other headers as needed

// Function declarations (if needed)
void configureWakeupPins();
void configureLogging();
void initDisplay();
void initSerial();
void initRGB();
void initSlider();
void initAccelerometer();
void initAuxPowerRegulator();
void printWakeupReason();
void screenUpdate();
void accelerometer();
// Other function declarations...

void AppManagerSetup() {
    configureWakeupPins();
    configureLogging();
    initDisplay();
    initAuxPowerRegulator();
    initSerial();
    initRGB();
    initSlider();
    buttonManager.begin();
    initAccelerometer();
    buttonManager.loadButtonCounters();
    printWakeupReason();

    // Initialize AudioManager
    audioManager.init();

    batteryManager.init(); // Initialize fuel gauge and battery management

    WiFiManagerCFObject.init();
    WiFiManagerCFObject.setDisplay(display); // Pass the display reference if needed

    // Initialize the PowerManager
    powerManager.begin();
}

void AppManagerLoop() {
    esp_task_wdt_reset();
    millisNow = millis();

    // Update AudioManager
    audioManager.loop();

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
                appPreviously = appActive;
                appActive = (appActive - 1 + APP_COUNT) % APP_COUNT;
            }
            break;

        case ButtonEvent_Released:
            ESP_LOGV(TAG_MAIN, "Released after %d ms", ev.duration);
            buttonCounter[ev.buttonIndex]++;
            if (ev.buttonIndex == 1) {
                appPreviously = appActive;
                appActive = (appActive + 1) % APP_COUNT;
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
    }

    if((millisNow - millisLastInteraction) >= 300000){
        powerManager.deepSleep();
    }

    if((millisNow - millisOldHeartbeat) >= 600000){
        millisOldHeartbeat = millisNow;
    }
}

// Now define the helper functions used in AppManagerSetup()
void configureWakeupPins() {
    // Configure the wakeup pins
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW); // For Active LOW button, ext0 single pin lowest power
}

void configureLogging() {
    // Set the global log level to INFO (can be ESP_LOG_NONE, ESP_LOG_ERROR, etc.)
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    
    // Alternatively, set log level for a specific tag
    esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);

    // ESP_LOGE - error (lowest)
    // ESP_LOGW - warning
    // ESP_LOGI - info
    // ESP_LOGD - debug
    // ESP_LOGV - verbose (highest)
    ESP_LOGI(TAG_MAIN, "Logging configured");
}

void initDisplay() {
    // Initialize display reset pin
    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW); 

    // Set up the power regulator pin for OLED
    pinMode(POWER_PIN_OLED, OUTPUT);
    digitalWrite(POWER_PIN_OLED, HIGH); // Turn on the OLED power regulator
    delay(200);
    digitalWrite(OLED_RESET, HIGH);
    delay(10);

    // Initialize the display
    display.init();
    // display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void initAuxPowerRegulator() {
    // Set up the power regulator pin
    pinMode(POWER_PIN_AUX, OUTPUT);
    digitalWrite(POWER_PIN_AUX, HIGH); // Turn on the aux power regulator
}

void initSerial() {
    Serial.begin(115200);
}

void initSlider() {
    // Initialize the slider
    pinMode(VOLT_READ_PIN, INPUT); // Slider Input
    sliderPositionFilterInit(); // Assuming this function is defined elsewhere
}

void initAccelerometer() {
    if (accel.begin() == false) {
        ESP_LOGE(TAG_MAIN, "Accelerometer not detected. Check address config and power regulator state. Freezing...");
        while (1);
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
    // draw the current app method
    apps[appActive]();

    if (appActive != appPreviously){
        if (appActive == APP_ACCELEROMETER){
            accelerometerScreenEnabled = true;
        } 
        else if (appPreviously == APP_ACCELEROMETER){
            accelerometerScreenEnabled = false;
            setColorsOff();
        }

        if (appActive == APP_FLASHLIGHT) {
        flashlightStatus = true;
        } 
        else if (appPreviously == APP_FLASHLIGHT) {
        flashlightStatus = false;
        setColorsOff(); 
        }

        if (appActive == APP_SLIDER_PROGRESS_BAR) {
        ESP_LOGV(TAG_MAIN, "Enter Slider Progress Bar");
        } 
        else if (appPreviously == APP_SLIDER_PROGRESS_BAR) {
        setColorsOff(); 
        }

        if (appActive == APP_ACCELEROMETER) {
        ESP_LOGV(TAG_MAIN, "Enter Accelerometer");
        } 
        else if (appPreviously == APP_ACCELEROMETER) {
        setColorsOff(); 
        }

        if (appActive == APP_REACTION) {
        buttonManager.registerCallback(
            reactionGame.getButtonIndex(),
            ReactionTimeGame::reactionButtonPressedCallback
        );
        } 
        else if (appPreviously == APP_REACTION) {
        //reactionGameEnabled = false;
        buttonManager.unregisterCallback(reactionGame.getButtonIndex());
        reactionGame.resetGame(); // Reset the game state
        }
        
        if (appActive == APP_CLOCK_DISPLAY) {
        clockDisplay.begin(); // Inside the module, begin() will only do initialization once.
        } 
        else if (appPreviously == APP_CLOCK_DISPLAY) {
        clockDisplay.reset();
        }

        if (appActive == APP_BREAKOUT) {
        breakoutGame.begin();    } 
        else if (appPreviously == APP_BREAKOUT) {
        breakoutGame.end();
        }

        if (appActive == APP_SIMON_SAYS) {
        simonGame.begin(); // Inside the module, begin() will only do initialization once.
        } 
        else if (appPreviously == APP_SIMON_SAYS) {
        simonGame.end(); // Unregister button callbacks
        }

        if (appActive == APP_MATRIX_SCREENSAVER) {
        matrixScreensaver.begin();
        } 
        else if (appPreviously == APP_MATRIX_SCREENSAVER) {
        ESP_LOGV(TAG_MAIN, "Exit Matrix Screensaver");
        }

        if (appActive == APP_DINO_GAME) {
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
        else if (appPreviously == APP_DINO_GAME) {
        //dinoGameEnabled = false;
        buttonManager.unregisterCallback(dinoGame.getJumpButtonIndex());
        buttonManager.unregisterCallback(dinoGame.getDuckButtonIndex());
        buttonManager.unregisterCallback(dinoGame.getResetButtonIndex());
        dinoGame.resetGame(); // Reset the game state
        }

        if (appActive == APP_BOOPER) {
        booper.begin();
        }
        else if (appPreviously == APP_BOOPER) {
        booper.end();
        }

        if (appActive == APP_POWER_MANAGER) {
        powerManager.registerShutdownCallback();
        }
        else if (appPreviously == APP_POWER_MANAGER) {
        powerManager.unregisterShutdownCallback();
        }

        if (appActive == APP_WIFI_CONFIG) {
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
        else if (appPreviously == APP_WIFI_CONFIG) {
        buttonManager.unregisterCallback(button_BottomLeftIndex);
        buttonManager.unregisterCallback(button_BottomRightIndex);
        }

        appPreviously = appActive; 
    }
}

void drawReactionTimeGame() {
    reactionGame.update(millisNow);
}

void drawSPHFluidGame(){
    // Map to 1–100
    int targetParticleCount = sliderPosition_Percentage_Inverted_Filtered;

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
    breakoutGame.update(accelX);
    breakoutGame.draw();
}


void updateClockDisplay() {
    clockDisplay.update();
}

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

void drawBooper(){
    // Update the screensaver
    booper.update();
    booper.draw();
}