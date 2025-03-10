#include "HAL.h"

// Include all low-level libraries and drivers:
#include <Wire.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <Adafruit_NeoPixel.h>
#include <SSD1306Wire.h>

#include "AudioManager.h"
#include "BatteryManager.h"
#include "RGBController.h"
#include "Flashlight.h"
#include "SliderPosition.h"

#include "SparkFun_LIS2DH12.h"
#include "WiFiManagerCF.h"
#include "ButtonManager.h"


// You probably still need your pin assignments, 
// either from globals.h or repeated here:
constexpr int POWER_PIN_OLED = 12;
constexpr int POWER_PIN_AUX  = 2;
//constexpr int SDA = 21;  
//constexpr int SCL = 22;
//constexpr int LED_BUILTIN  = 13;  // Declared in esp32-hal
constexpr int CHRG_ENA       = 13;  // If truly the same as LED_BUILTIN, watch for conflicts
constexpr int PIN            = 0;   // NeoPixel or LED data pin
constexpr int OLED_RESET     = 7;   // Also RX pin
constexpr int VOLT_READ_PIN  = 35;  // Battery voltage divider pin

// RGBW LEDS
constexpr int RGB_COUNT      = 4;   // Number of RGB LEDs
const uint16_t pixel_Front_Top    = 1;
const uint16_t pixel_Front_Middle = 2;
const uint16_t pixel_Front_Bottom = 3;
const uint16_t pixel_Back         = 0;

// Buttons
const int button_TopLeft = 36;
const int button_TopRight = 37;
const int button_MiddleLeft = 38;
const int button_MiddleRight = 39;
const int button_BottomLeft = 34;
const int button_BottomRight = 15;

const int button_TopLeftIndex = 0;
const int button_TopRightIndex = 1;
const int button_MiddleLeftIndex = 2;
const int button_MiddleRightIndex = 3;
const int button_BottomLeftIndex = 4;
const int button_BottomRightIndex = 5;

// Accelerometer
float accelX = 0;
float accelY = 0;
float accelZ = 0;
float tempC  = 0;

// Slider
int sliderPosition_Millivolts     = 0;
int sliderPosition_12Bits         = 0;
uint16_t sliderPosition_12Bits_Inverted= 0;
uint8_t  sliderPosition_8Bits          = 0;
uint8_t  sliderPosition_8Bits_Inverted = 0;
// Filtered slider positions
float sliderPosition_12Bits_Filtered = 0.0f;
float sliderPosition_12Bits_Inverted_Filtered = 0.0f;
int sliderPosition_8Bits_Filtered = 0;
int sliderPosition_8Bits_Inverted_Filtered = 0;
float sliderPosition_Percentage_Filtered = 0.0f;
float sliderPosition_Percentage_Inverted_Filtered = 0.0f;

// Battery
float    batteryVoltage             = 0.0f;
float    batteryVoltagePercentage   = 0.0f;
uint16_t batteryVoltageLowCutoff    = 3200;
uint16_t batteryVoltageHighCutoff   = 4200;
bool     preventSleepWhileCharging  = true;
bool     enableBatterySOCCutoff     = true;
float    batterySOCCutoff           = 80.0f;
float    sleepChargingChangeThreshold = -10.0f;
float    batteryChangeRate          = 0.0f;


// Here we create the “internal” (file-scope) single instances 
// of the hardware classes. We’ll return references to these
// from the HAL namespace.
namespace {
    // For example:
    static AudioManager       s_audioManager;
    static WiFiManagerCF      s_wifiManager;
    
    // If you have 6 buttons, pass them in accordingly:
    constexpr int numButtons = 6;
    // Example button pins (if not from globals):
    //   { 36, 37, 38, 39, 34, 15 };
    static const int s_buttonPins[numButtons] = {
        button_TopLeft, 
        button_TopRight, 
        button_MiddleLeft, 
        button_MiddleRight, 
        button_BottomLeft, 
        button_BottomRight
    };
    static const bool s_usePullups[numButtons] = {false, false, false, false, false, true};

    // Create one ButtonManager
    static ButtonManager s_buttonManager(
        s_buttonPins,
        s_usePullups,
        numButtons,
        20UL,    // Debounce ms
        1500UL   // Hold threshold ms
    );

    // Accelerometer
    static SPARKFUN_LIS2DH12 s_accel;

    // If you have a global BatteryManager:
    static BatteryManager s_batteryManager; 

    // RGBW LEDs
    static Adafruit_NeoPixel s_rgbStrip = Adafruit_NeoPixel(RGB_COUNT, PIN, NEO_GRBW + NEO_KHZ800);

    // Real display
    static SSD1306Wire s_realDisplay(0x3C, SDA, SCL);

    // Display proxy
    static DisplayProxy s_displayProxy(s_realDisplay);  

    // ... any other managers or singletons you want hidden behind HAL
}

namespace HAL
{
    //----------------------------------------------------------------------
    //  Initialize all hardware
    //----------------------------------------------------------------------
    void initHardware()
    {
        pinMode(OLED_RESET, OUTPUT);
        digitalWrite(OLED_RESET, LOW);

  

        // Turn on the OLED power regulator
        pinMode(POWER_PIN_OLED, OUTPUT);
        digitalWrite(POWER_PIN_OLED, HIGH);
        delay(200);
        digitalWrite(OLED_RESET, HIGH);
        delay(10);

        // Turn on the AUX regulator (for RGB, etc.)
        pinMode(POWER_PIN_AUX, OUTPUT);
        digitalWrite(POWER_PIN_AUX, HIGH);

        // Initialize serial
        Serial.begin(115200);
        Serial.println();
        Serial.println();

        // Initialize display
        s_realDisplay.init();
        s_realDisplay.setFont(ArialMT_Plain_10);

        // Initialize the I2C for accelerometer, etc.
        Wire.begin(SDA, SCL);
        if (!s_accel.begin())
        {
            Serial.println("Accelerometer not detected. Halting...");
            while (1);
        }

        // Initialize button manager
        s_buttonManager.init();

        // Initialize audio manager
        s_audioManager.init();

        // Initialize battery manager
        s_batteryManager.init();

        // Initialize WiFi manager
        s_wifiManager.init();

        // Initalize slider
        pinMode(VOLT_READ_PIN, INPUT); // Slider Input

        // Initialize RGB manager
        s_rgbStrip.begin();

        // Any other hardware inits ...

        // Print wakeup reason
        printWakeupReason();
    }

    //----------------------------------------------------------------------
    //  Initialize all the things, helpful for standalone apps
    //----------------------------------------------------------------------
    void initEasyEverything()
    {
        configureWakeupPins();
        esp_log_level_set("*", ESP_LOG_VERBOSE);
        esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
        initHardware();
        ESP_LOGI(TAG_MAIN, "Setup() complete");
    }

    //----------------------------------------------------------------------
    //  Loop hardware (call once per main loop)
    //----------------------------------------------------------------------
    void loopHardware()
    {
        esp_task_wdt_reset();
        millis_NOW = millis();

        // e.g. update audio, or battery, or anything that must be polled
        s_audioManager.loop();
        s_buttonManager.update();
        

        if ((millis_NOW - millis_HAL_TASK_20MS) >= TASK_20MS) {
            millis_HAL_TASK_20MS = millis_NOW;
            sliderPositionRead(VOLT_READ_PIN);       
        }
    
        if ((millis_NOW - millis_HAL_TASK_50MS) >= TASK_50MS) {
            millis_HAL_TASK_50MS = millis_NOW;
            HAL::updateAccelerometer();
        }
    
        if ((millis_NOW - millis_HAL_TASK_200MS) >= TASK_200MS) {
            millis_HAL_TASK_200MS = millis_NOW;
            s_batteryManager.update();
        }
    }

    //----------------------------------------------------------------------
    // Provide references so other code can use these hardware objects
    //----------------------------------------------------------------------
    AudioManager& audioManager()        { return s_audioManager; }
    ButtonManager& buttonManager()      { return s_buttonManager; }
    SSD1306Wire& realDisplay()          { return s_realDisplay; }
    SPARKFUN_LIS2DH12& accelerometer()  { return s_accel; }
    WiFiManagerCF& wifiManagerCF()      { return s_wifiManager; }
    Adafruit_NeoPixel& strip()          { return s_rgbStrip; }
    

    //----------------------------------------------------------------------
    // Example power/wakeup calls
    //----------------------------------------------------------------------
    void configureWakeupPins()
    {
        // Example: external wake on GPIO 15
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
    }

    void enterDeepSleep()
    {
        esp_deep_sleep_start();
    }

    //----------------------------------------------------------------------
    // Helpers
    //----------------------------------------------------------------------
    float getBatteryVoltage()
    {
        // example read from some battery manager or ADC
        // or do analogRead on a specific pin, do your conversion
        return 3.7f;  // placeholder
    }

    float getAccelerometerX() { return s_accel.getX(); }
    float getAccelerometerY() { return s_accel.getY(); }
    float getAccelerometerZ() { return s_accel.getZ(); }

    void setOledPower(bool on)
    {
        digitalWrite(POWER_PIN_OLED, on ? HIGH : LOW);
    }

    DisplayProxy& displayProxy() {
        return s_displayProxy;
    }

    // SSD1306Wire& realDisplay() {
    //     return s_realDisplay;
    // }

    void clearDisplay()
    {
        //if (!displayInitialized) return;
        s_realDisplay.clear();
    }

    void drawString(int16_t x, int16_t y, const String &text)
    {
        //if (!displayInitialized) return;
        s_realDisplay.drawString(x, y, text);
    }

    void updateDisplay()
    {
        //if (!displayInitialized) return;
        s_realDisplay.display();
    }

    void setRgbLed(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        // If you have a NeoPixel library, you’d do:
        s_rgbStrip.setPixelColor(index, s_rgbStrip.Color(r, g, b, w));
        s_rgbStrip.show();
    }
    
    void setRgbLedsOff()
    {
        for (int i = 0; i < s_rgbStrip.numPixels(); i++) {
            s_rgbStrip.setPixelColor(i, s_rgbStrip.Color(0,0,0,0));
          }
          updateStrip();
    }

    void chargingEnable()
    {
        pinMode(CHRG_ENA, OUTPUT);
        digitalWrite(CHRG_ENA, HIGH);
    }

    void chargingDisable()
    {
        pinMode(CHRG_ENA, OUTPUT);
        digitalWrite(CHRG_ENA, LOW);
    }

    void updateAccelerometer()
    {
            accelX = s_accel.getX();
            accelY = s_accel.getY();
            accelZ = s_accel.getZ();
            tempC  = s_accel.getTemperature();
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
}
