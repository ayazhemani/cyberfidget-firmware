#ifndef HAL_H
#define HAL_H

#include <Arduino.h>

// Forward-declare any hardware-related classes you want to expose from the HAL:
class AudioManager;
class ButtonManager;
class SPARKFUN_LIS2DH12;
class WiFiManagerCF;
class Adafruit_NeoPixel;


// The “HAL” namespace wraps all hardware initialization + references
namespace HAL
{
    // Initializes all hardware (pins, I2C, display, accelerometer, etc.)
    void initHardware();

    // A “loop” method, if you want to update any hardware each pass
    void loopHardware();

    // Provide references to the hardware objects so higher-level code
    // (like AppManager) can access them without knowing the low-level details.
    AudioManager& audioManager();
    ButtonManager& buttonManager();
    SSD1306Wire& display();
    SPARKFUN_LIS2DH12& accelerometer();
    WiFiManagerCF& wifiManagerCF();

    // Class accessor
    Adafruit_NeoPixel& strip(); 
    
    // If you want to set wake pins, deep sleep, etc. directly from AppManager
    void configureWakeupPins();
    void enterDeepSleep();

    // Additional helper calls for controlling certain pins, turning hardware on/off, etc.
    void setOledPower(bool on);
    void setAuxPower(bool on);
    void chargingEnable();
    void chargingDisable();
    void updateAccelerometer();

    // ... add more hardware-related getters/setters as you see fit
};

#endif // HAL_H
