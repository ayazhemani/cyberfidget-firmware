#include "globals.h"

// Timers
unsigned long millisNow             = 0;
unsigned long millisOldHeartbeat    = 0;
unsigned long millisOld200          = 0;
unsigned long millisOld50           = 0;
unsigned long millisOld10           = 0;
unsigned long millisApp             = 0;
unsigned long millisLastInteraction = 0;

// Logging
const char* TAG_MAIN = "mainApp";

// App modes
int appActive      = 0;
int appActiveSaved = 0;
int appPreviously  = 0;

// States
bool accelerometerScreenEnabled = false;
bool reactionGameEnabled        = false;
volatile bool buttonPressed     = false;

// Buttons
int buttonCounter[numButtons]       = {0};
int buttonCounterSaved[numButtons]  = {0};

const int button_TopLeftIndex    = 0;
const int button_TopRightIndex   = 1;
const int button_MiddleLeftIndex = 2;
const int button_MiddleRightIndex= 3;
const int button_BottomLeftIndex = 4;
const int button_BottomRightIndex= 5;

// Accelerometer
float accelX = 0;
float accelY = 0;
float accelZ = 0;
float tempC  = 0;

// Battery
float batteryVoltage            = 0.0f;
float batteryVoltagePercentage  = 0.0f;

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

// WiFi
char wifiAP_SSID[]   = "CyberFidget_AP";
bool isTryingToConnect= false;

// Clock
bool clockScreenEnabled = false;
struct tm currentTime   = {0};

// Audio beep logic
bool beepActive       = false;
float beepFrequency   = 440.0f;

// Scroll mode
ScrollMode currentScrollMode = PIXEL_SCROLL;

// Now define the function pointer array & app names:
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
// void drawAudioPlayer();  // Commented out per your X-Macro
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

App apps[APP_COUNT] = {
  #define X(func, id, name) func,
  APP_LIST
  #undef X
};

const char* appNames[APP_COUNT] = {
  #define X(func, id, name) name,
  APP_LIST
  #undef X
};
