#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <time.h> // For struct tm
#include "fontSuiGenerisRg.h" 

//
// 1) Compile-time constants
//

// If you want the demo duration globally available
constexpr unsigned long DEMO_DURATION = 3000;

// Pin assignments — changed from #define to constexpr for clarity
constexpr int POWER_PIN_OLED = 12;  // OLED VREG
constexpr int POWER_PIN_AUX  = 2;   // RGB VREG
//constexpr int LED_BUILTIN  = 13;  // Declared in esp32-hal
constexpr int CHRG_ENA       = 13;  // If truly the same as LED_BUILTIN, watch for conflicts
constexpr int PIN            = 0;   // NeoPixel or LED data pin
constexpr int OLED_RESET     = 7;   // Also RX pin
constexpr int VOLT_READ_PIN  = 35;  // Battery voltage divider pin
constexpr int RGB_COUNT      = 4;   // Number of RGB LEDs

// Debounce and hold thresholds
constexpr unsigned long BUTTON_DEBOUNCE_MS = 20;       // Debounce time in ms
constexpr unsigned long BUTTON_HOLD_THRESHOLD_MS = 1500; // Time in ms to trigger a "hold"

// Number of buttons
constexpr int numButtons = 6;

// Debounce delay
//constexpr unsigned long debounceDelay = 20;

//
// 2) Extern global variables (runtime values)
//   These are defined exactly once in globals.cpp
//

// Timers
extern unsigned long millisNow;
extern unsigned long millisOldHeartbeat;
extern unsigned long millisOld200;
extern unsigned long millisOld50;
extern unsigned long millisOld10;
extern unsigned long millisDemoMode;
extern unsigned long millisLastInteraction;

// // Memory Management
// extern Preferences preferencesMainApp;

// // Logging Management
// extern const char* TAG_MAIN;

// You had this variable but it’s unused in your sample main.cpp. Keep or remove as needed:
extern unsigned long masterWatchdogTimer; 

// Demo modes
extern int  demoMode;
extern int  demoModeSaved;
extern int  demoModePreviously;

// LED Sequencer & games
extern bool ledSequencerEnabled;
extern bool accelerometerScreenEnabled;
extern bool reactionGameEnabled;
extern volatile bool buttonPressed;

// Buttons
// extern const int buttonPins[numButtons];
// extern volatile unsigned long lastDebounceTime[numButtons];
// extern volatile int buttonState[numButtons];
extern int buttonCounter[numButtons];
extern int buttonCounterSaved[numButtons];
extern const int button_TopLeft;
extern const int button_TopRight;
extern const int button_MiddleLeft;
extern const int button_MiddleRight;
extern const int button_BottomLeft;
extern const int button_BottomRight;

extern const int button_TopLeftIndex;
extern const int button_TopRightIndex;
extern const int button_MiddleLeftIndex;
extern const int button_MiddleRightIndex;
extern const int button_BottomLeftIndex;
extern const int button_BottomRightIndex;

extern const int s_buttonPins[numButtons];    // Pin assignments for all buttons
extern const bool s_usePullups[numButtons];   // Whether each button uses internal pull-up resistors

// RGBW LEDS
extern const uint16_t pixel_Front_Top;
extern const uint16_t pixel_Front_Middle;
extern const uint16_t pixel_Front_Bottom;
extern const uint16_t pixel_Back;

// Accelerometer
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

// Slider
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
extern bool wifimanager_nonblocking;
extern bool isTryingToConnect;

// Clock
extern bool     clockScreenEnabled;
extern struct tm currentTime;

// Serial data / text scrolling
extern bool newDataReceived;
extern bool showingDefaultScreen;
extern int  lineCount;
extern int  currentLine;
extern bool isScreenUpdated;
extern int  previousLine;
extern int  scrollOffset;
extern int  previousScrollOffset;
extern String dataLines[];
extern String incomingData;
extern const int maxLinesOnScreen;
extern unsigned long lastDataTime;
extern const unsigned long dataTimeout;

// Scroll mode enumeration
enum ScrollMode {
    LINE_SCROLL,
    PIXEL_SCROLL
};
extern ScrollMode currentScrollMode;

// Audio beep logic
extern bool         beepActive;
extern unsigned long beepStartTime;
extern unsigned long beepDurationSimonSays;
extern float        beepFrequency;

// Buttons for SimonSays, etc.
extern const int buttonPinsSimonSays[4];

// Debounce struct for those 4 buttons
struct ButtonDebounce {
    bool stableState;
    bool lastReading;
    unsigned long lastChangeTime;
    bool wasPressed;
    static const unsigned long DEBOUNCE_MS = 100;
};
extern ButtonDebounce btns[4];

void beepOnBounce();
void loopAudio();
void connectToWiFi();
void beepOn();
void beepOff();
void updateBeep();
void startBeep();
void beepForSquareFn(int sq);
void beepOnUserPressFn(int sq);

// Step 1: Declare function prototypes (so globals.cpp knows they exist)
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
void drawAudioPlayer();
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

// Step 2: Define the X-Macro List
#define DEMO_LIST \
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

// Step 3: Define the Enum
enum DemoIndex {
    #define X(func, id, name) DEMO_##id,
        DEMO_LIST
    #undef X
    DEMO_COUNT
};

// Step 4: Declare function pointer and name arrays
typedef void (*Demo)(void);
extern Demo demos[DEMO_COUNT];
extern const char* demoNames[DEMO_COUNT];

#endif // GLOBALS_H
