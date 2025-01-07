// Main Loop timers
void disableWatchdog();
void enableWatchdog();
unsigned long millisNow = 0;
unsigned long millisOldHeartbeat = 0;
unsigned long millisOld200 = 0;
unsigned long millisOld50 = 0;
unsigned long millisOld10 = 0;
unsigned long masterWatchdogTimer = 0; // Time (millis) when the watchdog loop was last polled
unsigned long millisDemoMode = 0;
unsigned long millisLastInteraction = 0; // Keeps track of when the user last interacted so long execution things can happen

const int nextButtonPin = 15; 
bool audioPlayerRunning = false;

void connectToWiFi();
void startWebServer();
void stopWebServer();
void startWiFiManager();
bool isWebServerRunning = false;
bool wifimanager_nonblocking = false; 

int demoMode = 0; // Default screen
int demoModeSaved = 0;
int demoModePreviously = 0;
int colorMode = 0;

#define DEMO_DURATION 3000 // Typically 3000 or 0

// Function prototypes
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
void startSimonSaysGame();
void drawSimonSaysGame();
void drawSliderProgressBar();
void drawWifiConfig();
void drawPixelWaterfallGame();

// ECU Peripherals
#define POWER_PIN_OLED 12 //OLED VREG
#define POWER_PIN_AUX 2 //RGB VREG
#define LED_BUILTIN 13
#define CHRG_ENA 13
#define PIN 0
#define LED_COUNT 4
#define OLED_RESET 7 // also RX



bool ledSequencerEnabled = false;
uint16_t pixel_Back = 0;
uint16_t pixel_Front_Top = 1; 
uint16_t pixel_Front_Middle = 2; 
uint16_t pixel_Front_Bottom = 3;  

void colorSet();
void red();
void green();
void blue();
void white();
void halfWHITE();
void mapToRainbow(int input, uint8_t dim, uint8_t &red, uint8_t &green, uint8_t &blue);


// Define the GPIO pins for the buttons
void buttonPressedTap();
bool compareButtonCounters();
const int buttonPins[] = {36, 37, 38, 39, 34, 15};
const int numButtons = 6;

const int button_TopLeft = 36;
const int button_TopRight = 37;
const int button_MiddleLeft = 38;
const int button_MiddleRight = 39;
const int button_BottomLeft = 34;
const int button_BottomRight = 15;

bool accelerometerScreenEnabled = false; 

volatile bool inReactionGame = false; // Global flag to track if the reaction game is active
volatile bool buttonPressed = false;  // Flag to track button presses
bool reactionGameEnabled = false; // Let the game be main priority

// Configuration variables for PixelWaterfallGame
float pixelGameInertia = 1.2;
float pixelGameDamping = 0.95;

void flashlightSwitch(bool flashlightEnable);
bool flashlightStatus = false;

// Variables to store the last debounce time and state of the buttons
volatile unsigned long lastDebounceTime[numButtons] = {0};
volatile int buttonState[numButtons] = {0};
volatile int buttonCounter[numButtons] = {0};
volatile int buttonCounterSaved[numButtons] = {0};

// Debounce delay in milliseconds
const unsigned long debounceDelay = 20;

float accelX = 0;
float accelY = 0;
float accelZ = 0;
float tempC = 0;

// Battery Voltage
float batteryVoltage = 0;
uint16_t batteryVoltageLowCutoff = 3200;
uint16_t batteryVoltageHighCutoff = 4200;
float batteryVoltagePercentage = 0;
void fuelGaugeUpdate();
bool preventSleepWhileCharging = true;
#define VOLT_READ_PIN 35 // Battery voltage divider pin
bool enableBatterySOCCutoff = true; // Requires jumper on R64 solder bridge to connect ESPP32 IO13 to MCP73831 PROG
float batterySOCCutoff = 80.0; // Percentage to cut-off battery charging 
float sleepChargingChangeThreshold = -10.0;

// Slider Pin Voltage
void sliderPositionRead();
uint8_t sliderPosition_Percentage = 0;
uint16_t sliderPosition_Millivolts = 0;
uint16_t sliderPosition_12Bits = 0;
uint16_t sliderPosition_12Bits_Inverted = 0;
uint8_t sliderPosition_8Bits = 0;
uint8_t sliderPosition_8Bits_Inverted = 0;

// Clock / Time
void drawClockDemo();
void updateTime(struct tm *currentTime);
void increaseMinute(struct tm *currentTime);
void decreaseMinute(struct tm *currentTime);

bool clockScreenEnabled = false;
struct tm currentTime;

// Serial printing data streams
void drawSerialDataScreenWrapper();
void drawSerialDataScreen();
void drawDefaultInfoScreen();

String incomingData = ""; // Buffer for incoming data
unsigned long lastDataTime = 0;    // Stores the last time data was received
const unsigned long dataTimeout = 5000; // Timeout period in milliseconds (e.g., 5 seconds)
bool newDataReceived = false;
bool showingDefaultScreen = true; // Track whether the default screen is displayed to be more CPU efficient with redraws
String dataLines[10]; // Array to hold parsed lines of data
int lineCount = 0; // Number of lines in the current data
int currentLine = 0; // The first line to display on the screen
const int maxLinesOnScreen = 6; // Number of lines that fit on the screen at once
void handleScrollUp();
void handleScrollDown();
void updateScrollPositionFromSlider();
bool isScreenUpdated = false; // Flag to track if the screen is up-to-date
int previousLine = 0;         // Track the last displayed scroll position
int scrollOffset = 0; // Pixel-level scroll offset
int previousScrollOffset = 0;  // Track the last pixel offset in pixel scroll mode
const int maxScrollOffset = 10; // Set the maximum scroll offset for smooth scrolling (adjust as needed)
enum ScrollMode {
    LINE_SCROLL,
    PIXEL_SCROLL
};
ScrollMode currentScrollMode = PIXEL_SCROLL; // Default to line-based scrolling
void toggleScrollMode();

void actionKeyOn(bool active, int pin, void* ptr);
void actionKeyOff(bool active, int pin, void* ptr);
void setupActions();