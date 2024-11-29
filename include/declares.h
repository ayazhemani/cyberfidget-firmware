// Main Loop timers
unsigned long millisNow = 0;
unsigned long millisOldHeartbeat = 0;
unsigned long millisOld200 = 0;
unsigned long millisOld50 = 0;
unsigned long millisOld10 = 0;
unsigned long masterWatchdogTimer = 0; // Time (millis) when the watchdog loop was last polled
unsigned long millisDemoMode = 0;
unsigned long millisLastInteraction = 0; // Keeps track of when the user last interacted so long execution things can happen

const int nextButtonPin = 15; 
bool audioPlayerRunning = 0;

void connectToWiFi();

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

// ECU Peripherals
#define POWER_PIN_OLED 12 //OLED VREG
#define POWER_PIN_AUX 2 //RGB VREG
#define LED_BUILTIN 13
#define PIN 0
#define LED_COUNT 4
#define OLED_RESET 7 // also RX

// Battery Voltage Monitor
#define VOLT_READ_PIN 35 // Battery voltage divider pin

bool ledSequencerEnabled = false;

void colorSet();
void red();
void green();
void blue();
void white();
void halfWHITE();


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
void fuelGaugeReport();
bool preventSleepWhileCharging = true;

// Slider Pin Voltage
uint8_t pinVoltagePercentage = 0;
uint16_t pinVoltage = 0;
uint16_t pinVoltageBits = 0;

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
