#include "globals.h"

// --------------------------------------------------------------------
// 1) Define each extern variable exactly once here
// --------------------------------------------------------------------

// Timers
unsigned long millisNow             = 0;
unsigned long millisOldHeartbeat    = 0;
unsigned long millisOld200          = 0;
unsigned long millisOld50           = 0;
unsigned long millisOld10           = 0;
unsigned long millisDemoMode        = 0;
unsigned long millisLastInteraction = 0;

// Potentially unused
unsigned long masterWatchdogTimer   = 0; 

// Demo modes
int  demoMode          = 0;
int  demoModeSaved     = 0;
int  demoModePreviously= 0;

// LED Sequencer & games
bool ledSequencerEnabled       = false;
bool accelerometerScreenEnabled= false;
bool reactionGameEnabled       = false;
volatile bool buttonPressed    = false;

// Buttons
// const int buttonPins[numButtons] = {36, 37, 38, 39, 34, 15};
// volatile unsigned long lastDebounceTime[numButtons] = {0};
// volatile int buttonState[numButtons] = {0};
int buttonCounter[numButtons] = {0};
int buttonCounterSaved[numButtons] = {0};
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

// NOTE: The array positions must match your 6 buttons in the same order you want to handle them.
const int s_buttonPins[numButtons] = {
    button_TopLeft,
    button_TopRight,
    button_MiddleLeft,
    button_MiddleRight,
    button_BottomLeft,
    button_BottomRight
};
// For the internal/external pull-ups: 
// - If you have external pull-ups on the first 5, set false. 
// - If you want to rely on the MCU’s internal pull-up for the last one, set true.
// Only GPIO 15 supports pull up, the other buttons have external pull-ups
const bool s_usePullups[numButtons] = {
    false, false, false, false, false, true // Only BottomRight uses INPUT_PULLUP
};

// RGBW LEDS
const uint16_t pixel_Front_Top    = 1;
const uint16_t pixel_Front_Middle = 2;
const uint16_t pixel_Front_Bottom = 3;
const uint16_t pixel_Back         = 0;

// Accelerometer
float accelX = 0.0f;
float accelY = 0.0f;
float accelZ = 0.0f;
float tempC  = 0.0f;

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

// Slider
uint8_t  sliderPosition_Percentage     = 0;
uint16_t sliderPosition_Millivolts     = 0;
uint16_t sliderPosition_12Bits         = 0;
uint16_t sliderPosition_12Bits_Inverted= 0;
uint8_t  sliderPosition_8Bits          = 0;
uint8_t  sliderPosition_8Bits_Inverted = 0;

// WiFi
char wifiAP_SSID[] = "CyberFidget_AP";
bool wifimanager_nonblocking = false;

// Clock
bool      clockScreenEnabled = false;
struct tm currentTime        = {0};  // Zero-init

// Serial data / text scrolling
bool newDataReceived   = false;
bool showingDefaultScreen = true;
int  lineCount         = 0;
int  currentLine       = 0;
bool isScreenUpdated   = false;
int  previousLine      = 0;
int  scrollOffset      = 16;
int  previousScrollOffset = 0;
String dataLines[10];
String incomingData = ""; // Buffer for incoming data
const int maxLinesOnScreen = 6; // Number of lines that fit on the screen at once
unsigned long lastDataTime = 0; // Stores the last time data was received
const unsigned long dataTimeout = 5000; // Timeout period in milliseconds (e.g., 5 seconds)

// Scroll mode
ScrollMode currentScrollMode = PIXEL_SCROLL;

// Audio beep logic
bool         beepActive           = false;
unsigned long beepStartTime       = 0;
unsigned long beepDurationSimonSays= 200;
float        beepFrequency        = 440.0f;

// Buttons for SimonSays
const int buttonPinsSimonSays[4] = { 38, 39, 34, 15 }; // Adjust if they differ

// Debounce struct array
ButtonDebounce btns[4] = {
    { false, true, 0UL, false },
    { false, true, 0UL, false },
    { false, true, 0UL, false },
    { false, true, 0UL, false }
};

// PixelWaterfall config
float pixelGameInertia = 1.2f;
float pixelGameDamping = 0.95f;

// --------------------------------------------------------------------
// 2) (Optional) If you want to define any of the prototypes from
//    globals.h here, you can. Or you can define them in main.cpp.
//    For example, if you want disableWatchdog here:
//
// void disableWatchdog() {
//     // ...
// }
// --------------------------------------------------------------------
//
// But typically, you'd keep the actual function bodies in main.cpp
// or in a dedicated .cpp for each manager or class.
