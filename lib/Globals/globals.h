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
extern uint8_t  sliderPosition_Percentage;
extern uint16_t sliderPosition_Millivolts;
extern uint16_t sliderPosition_12Bits;
extern uint16_t sliderPosition_12Bits_Inverted;
extern uint8_t  sliderPosition_8Bits;
extern uint8_t  sliderPosition_8Bits_Inverted;

// WiFi
extern char wifiAP_SSID[];
extern bool wifimanager_nonblocking;

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

//
// 3) Additional shared variables for PixelWaterfallGame (if needed):
//
extern float pixelGameInertia;
extern float pixelGameDamping;

//
// 4) Global function prototypes that main.cpp references
//    (If you have them defined in main.cpp or elsewhere.)
//

void disableWatchdog();
void enableWatchdog();
void beepOnBounce();
void loopAudio();
void connectToWiFi();

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
void drawWifiConfig();
void drawPixelWaterfallGame();
void drawSPHFluidGame();
void drawBreakoutGame();
void drawSimonSaysGame2();
void drawMatrixScreensaver();
void beepOn();
void beepOff();
void updateBeep();
void startBeep();
void beepForSquareFn(int sq);
void beepOnUserPressFn(int sq);
void updateClockDisplay();
void drawDinoGame();

#endif // GLOBALS_H
