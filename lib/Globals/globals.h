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

// Forward declarations for the “apps”
void drawFontFaceDemo();
void drawProgressBarDemo();
void drawImageDemo_1();
void drawImageDemo_2();
void drawImageDemo_3();
void drawImageDemo_4();
void drawTextFlowDemo();
void drawTextAlignmentDemo();
void drawRectDemo();
void drawCircleDemo();
void drawBatteryProgressBar();
void drawAccelerometerScreen();
void drawButtonCounters();
void drawFlashlight();
void drawReactionTimeGame();
void drawTimeOnCounter();
void drawSliderProgressBar();
void updateClockDisplay();
void drawSerialDataScreen();
void drawWifiConfig();
void drawSPHFluidGame();
void drawBreakoutGame();
void drawSimonSaysGame();
void drawMatrixScreensaver();
void drawDinoGame();
void drawPowerManager();
void drawBooper();

// The array of function pointers is set up via X-Macro:
typedef void (*App)(void);

// Step 2: X-Macro list of apps
#define APP_LIST \
    X(drawFontFaceDemo, FONT_FACE, "Font Face") \
    X(drawProgressBarDemo, PROGRESS_BAR, "Progress Bar") \
    X(drawImageDemo_1, IMAGE_1, "Image 1") \
    X(drawImageDemo_2, IMAGE_2, "Image 2") \
    X(drawImageDemo_3, IMAGE_3, "Image 3") \
    X(drawImageDemo_4, IMAGE_4, "Image 4") \
    X(drawTextFlowDemo, TEXT_FLOW, "Text Flow") \
    X(drawTextAlignmentDemo, TEXT_ALIGNMENT, "Text Alignment") \
    X(drawRectDemo, RECT, "Rectangle") \
    X(drawCircleDemo, CIRCLE, "Circle") \
    X(drawBatteryProgressBar, BATTERY, "Battery Progress") \
    X(drawAccelerometerScreen, ACCELEROMETER, "Accelerometer") \
    X(drawButtonCounters, BUTTON_COUNTERS, "Button Counters") \
    /* X(drawAudioPlayer, AUDIO_PLAYER, "Audio Player") */ \
    X(drawBooper, BOOPER, "Audio Booper") \
    X(drawFlashlight, FLASHLIGHT, "Flashlight") \
    X(drawReactionTimeGame, REACTION, "Reaction Time") \
    X(drawTimeOnCounter, TIME_ON_COUNTER, "Time Counter") \
    X(drawSliderProgressBar, SLIDER_PROGRESS_BAR, "Slider Progress") \
    X(updateClockDisplay, CLOCK_DISPLAY, "Clock Display") \
    X(drawSerialDataScreen, SERIAL_DATA, "Serial Data") \
    X(drawWifiConfig, WIFI_CONFIG, "WiFi Config") \
    X(drawSPHFluidGame, SPH_FLUID, "SPH Fluid") \
    X(drawBreakoutGame, BREAKOUT, "Breakout") \
    X(drawSimonSaysGame, SIMON_SAYS, "Simon Says") \
    X(drawMatrixScreensaver, MATRIX_SCREENSAVER, "Matrix Screensaver") \
    X(drawDinoGame, DINO_GAME, "Dino Game") \
    X(drawPowerManager, POWER_MANAGER, "Power Manager")

// Step 3: Enum
enum AppIndex {
    #define X(func, id, name) APP_##id,
    APP_LIST
    #undef X
    APP_COUNT
};

// Arrays of function pointers and names
extern App apps[APP_COUNT];
extern const char* appNames[APP_COUNT];

#endif // GLOBALS_H
