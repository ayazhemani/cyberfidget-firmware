#include "HAL.h"

// Include all low-level libraries and drivers:
#include <Wire.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "AudioManager.h"
#include "BatteryManager.h"
#include "RGBController.h"
#include "Flashlight.h"
#include "SliderPosition.h"
#include "SSD1306Wire.h"
#include "SparkFun_LIS2DH12.h"
#include "WiFiManagerCF.h"
#include "ButtonManager.h"
#include <Adafruit_NeoPixel.h>

// You probably still need your pin assignments, 
// either from globals.h or repeated here:
constexpr int POWER_PIN_OLED = 12;
constexpr int POWER_PIN_AUX  = 2;
constexpr int SDA = 21;  
constexpr int SCL = 22;
//constexpr int LED_BUILTIN  = 13;  // Declared in esp32-hal
constexpr int CHRG_ENA       = 13;  // If truly the same as LED_BUILTIN, watch for conflicts
constexpr int PIN            = 0;   // NeoPixel or LED data pin
constexpr int OLED_RESET     = 7;   // Also RX pin
constexpr int VOLT_READ_PIN  = 35;  // Battery voltage divider pin
constexpr int RGB_COUNT      = 4;   // Number of RGB LEDs

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
    static const int s_buttonPins[numButtons] = {36, 37, 38, 39, 34, 15};
    static const bool s_usePullups[numButtons] = {false, false, false, false, false, true};

    // Create one ButtonManager
    static ButtonManager s_buttonManager(
        s_buttonPins,
        s_usePullups,
        numButtons,
        20UL,    // Debounce ms
        1500UL   // Hold threshold ms
    );

    // The SSD1306 display
    static SSD1306Wire s_display(0x3c, SDA, SCL);

    // Accelerometer
    static SPARKFUN_LIS2DH12 s_accel;

    // If you have a global BatteryManager:
    static BatteryManager s_batteryManager; 

    // RGBW LEDs
    static Adafruit_NeoPixel s_rgbStrip = Adafruit_NeoPixel(RGB_COUNT, PIN, NEO_RGBW + NEO_KHZ800);

    // ... any other managers or singletons you want hidden behind HAL
}

namespace HAL
{
    //----------------------------------------------------------------------
    //  Initialize all hardware
    //----------------------------------------------------------------------
    void initHardware()
    {
        // Initialize serial, if desired
        Serial.begin(115200);

        // Turn on the OLED power regulator
        pinMode(POWER_PIN_OLED, OUTPUT);
        digitalWrite(POWER_PIN_OLED, HIGH);
        delay(100);

        // Turn on the AUX regulator (for RGB, etc.)
        pinMode(POWER_PIN_AUX, OUTPUT);
        digitalWrite(POWER_PIN_AUX, HIGH);

        // Possibly reset the display pin if needed
        // pinMode(OLED_RESET, OUTPUT);
        // digitalWrite(OLED_RESET, LOW);
        // delay(200);
        // digitalWrite(OLED_RESET, HIGH);

        // Initialize display
        s_display.init();
        s_display.setFont(ArialMT_Plain_10);

        // Initialize the I2C for accelerometer, etc.
        Wire.begin(SDA, SCL);
        if (!s_accel.begin())
        {
            Serial.println("Accel not detected. Freezing...");
            while (1);
        }

        // Initialize button manager
        s_buttonManager.begin();

        // Initialize audio manager
        s_audioManager.init();

        // Initialize battery manager
        s_batteryManager.init();

        // Initialize WiFi manager
        s_wifiManager.init();
        s_wifiManager.setDisplay(s_display);

        // Initialize RGB manager
        s_rgbStrip.begin();

        // Any other hardware inits ...
    }

    //----------------------------------------------------------------------
    //  Loop hardware (call once per main loop)
    //----------------------------------------------------------------------
    void loopHardware()
    {
        esp_task_wdt_reset();
        // e.g. update audio, or battery, or anything that must be polled
        s_audioManager.loop();
        s_buttonManager.update();
        s_batteryManager.update();
    }

    //----------------------------------------------------------------------
    // Provide references so other code can use these hardware objects
    //----------------------------------------------------------------------
    AudioManager& audioManager()       { return s_audioManager; }
    ButtonManager& buttonManager()     { return s_buttonManager; }
    SSD1306Wire& display()            { return s_display; }
    SPARKFUN_LIS2DH12& accelerometer(){ return s_accel; }
    WiFiManagerCF& wifiManagerCF()    { return s_wifiManager; }
    

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

    float readSliderPercentage()
    {
        // Read the raw slider position in millivolts and 12-bit ADC value
        sliderPosition_Millivolts = analogReadMilliVolts(VOLT_READ_PIN);
        sliderPosition_12Bits = analogRead(VOLT_READ_PIN);
    }

    void setOledPower(bool on)
    {
        digitalWrite(POWER_PIN_OLED, on ? HIGH : LOW);
    }

    void clearDisplay()
    {
        //if (!displayInitialized) return;
        s_display.clear();
    }

    void drawText(int x, int y, const char* text)
    {
        //if (!displayInitialized) return;
        s_display.drawString(x, y, text);
    }

    void updateDisplay()
    {
        //if (!displayInitialized) return;
        s_display.display();
    }

    void setRgbLed(int index, uint8_t r, uint8_t g, uint8_t b)
    {
        // If you have a NeoPixel library, you’d do:
        // pixels.setPixelColor(index, pixels.Color(r, g, b));
        // pixels.show();
    }
    
    void setRgbLedsOff()
    {
        // Turn them all off
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
}
