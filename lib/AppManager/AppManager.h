#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <Arduino.h>
#include "globals.h"

// Forward declare the “app” draw functions
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
void drawBooper();
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

// Function pointer type
typedef void (*App)(void);

// Define the X-Macro list for Apps
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

// Enum for app indexes
enum AppIndex {
    #define X(func, id, name) APP_##id,
    APP_LIST
    #undef X
    APP_COUNT
};

class AppManager {
public:
    static void setup();
    static void loop();
private:
    static void screenUpdate();
    static void processButtonEvents();
};

// Expose instance
extern AppManager appManager;

typedef void (*App)(void);
extern App apps[APP_COUNT];
extern const char* appNames[APP_COUNT];

#endif // APP_MANAGER_H
