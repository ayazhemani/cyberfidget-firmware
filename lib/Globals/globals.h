#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <time.h>
#include "esp_log.h"

// Global timers
extern unsigned long millisNow;
extern unsigned long millisOldHeartbeat;
extern unsigned long millisOld200;
extern unsigned long millisOld50;
extern unsigned long millisOld10;
extern unsigned long millisApp;
extern unsigned long millisLastInteraction;

extern const char *TAG_MAIN;

// App modes
extern int  appActive;
extern int  appActiveSaved;
extern int  appPreviously;

// Some booleans for states
extern bool accelerometerScreenEnabled;
extern bool reactionGameEnabled;
extern volatile bool buttonPressed;

// Button counters
constexpr int numButtons = 6;
extern int buttonCounter[numButtons];
extern int buttonCounterSaved[numButtons];

// For indexing the six buttons
extern const int button_TopLeftIndex;
extern const int button_TopRightIndex;
extern const int button_MiddleLeftIndex;
extern const int button_MiddleRightIndex;
extern const int button_BottomLeftIndex;
extern const int button_BottomRightIndex;

// Accelerometer readings
extern float accelX;
extern float accelY;
extern float accelZ;
extern float tempC;

// Battery
extern float batteryVoltage;
extern float batteryVoltagePercentage;
extern uint16_t batteryVoltageLowCutoff;
extern uint16_t batteryVoltageHighCutoff;
extern bool preventSleepWhileCharging;
extern bool enableBatterySOCCutoff;
extern float batterySOCCutoff;
extern float sleepChargingChangeThreshold;
extern float batteryChangeRate;

// Slider readings
extern int sliderPosition_Millivolts;
extern int sliderPosition_12Bits;
// Filtered slider positions
extern float sliderPosition_12Bits_Filtered;
extern float sliderPosition_12Bits_Inverted_Filtered;
extern int sliderPosition_8Bits_Filtered;
extern int sliderPosition_8Bits_Inverted_Filtered;
extern float sliderPosition_Percentage_Filtered;
extern float sliderPosition_Percentage_Inverted_Filtered;

// WiFi
extern char wifiAP_SSID[];
extern bool isTryingToConnect;

// Clock
extern bool clockScreenEnabled;
extern struct tm currentTime;

// Audio beep logic
extern bool beepActive;
extern float beepFrequency;

// Scroll mode
enum ScrollMode {
    LINE_SCROLL,
    PIXEL_SCROLL
};
extern ScrollMode currentScrollMode;

#endif // GLOBALS_H
