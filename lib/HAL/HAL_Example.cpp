// HAL.cpp
#include "HAL.h"
#include "SSD1306Wire.h"
#include "SparkFun_LIS2DH12.h"
// #include "Adafruit_NeoPixel.h" or some other LED library

// Create static / file-scope objects if you want them hidden from the rest of the code:
static SSD1306Wire display(0x3c, PIN_ACCEL_SDA, PIN_ACCEL_SCL);
static SPARKFUN_LIS2DH12 accel; 
// … plus any other hardware objects

// Or wrap them in anonymous namespace if you prefer
namespace {

bool displayInitialized = false;

}

namespace HAL
{
    void initAll()
    {
        // Example: set pin modes
        pinMode(PIN_OLED_POWER, OUTPUT);
        pinMode(PIN_AUX_POWER, OUTPUT);
        digitalWrite(PIN_OLED_POWER, HIGH); // Turn on display
        // etc.
        
        // Initialize I2C or wire
        Wire.begin(PIN_ACCEL_SDA, PIN_ACCEL_SCL);
        
        // Accel
        if (!accel.begin()) {
            // handle error
            while(true) { /* freeze */ }
        }

        // Display
        display.init();
        display.setFont(ArialMT_Plain_10);
        display.flipScreenVertically();  // if desired
        display.clear();
        display.display();
        displayInitialized = true;

        // etc. for LED strip, battery, etc.
    }

    float getBatteryVoltage()
    {
        // example read from some battery manager or ADC
        // or do analogRead on a specific pin, do your conversion
        return 3.7f;  // placeholder
    }

    float getAccelerometerX() { return accel.getX(); }
    float getAccelerometerY() { return accel.getY(); }
    float getAccelerometerZ() { return accel.getZ(); }

    float readSliderPercentage()
    {
       int raw = analogRead(PIN_SLIDER);
       // Convert it to 0–100
       return (raw / 4095.0f) * 100.0f;
    }

    void setOledPower(bool on)
    {
        digitalWrite(PIN_OLED_POWER, on ? HIGH : LOW);
    }

    void clearDisplay()
    {
        if (!displayInitialized) return;
        display.clear();
    }

    void drawText(int x, int y, const char* text)
    {
        if (!displayInitialized) return;
        display.drawString(x, y, text);
    }

    void updateDisplay()
    {
        if (!displayInitialized) return;
        display.display();
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
    
    void enterDeepSleep(uint64_t sleepTimeUs)
    {
        // call esp_sleep_enable_timer_wakeup(sleepTimeUs);
        // etc.
        // esp_deep_sleep_start();
    }
};
