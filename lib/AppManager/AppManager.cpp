#include "AppManager.h"
#include "HAL.h"
#include "ReactionTimeGame.h"
#include "DinoGame.h"
#include "SimonSaysGame.h"
#include "MatrixScreensaver.h"
#include "Booper.h"
#include "SPHFluidGame.h"
#include "BreakoutGame.h"
#include "ClockDisplay.h"
#include "PowerManager.h"
#include "WiFiManagerCF.h"
#include "Flashlight.h"
#include "SliderPosition.h"
#include "SerialDisplay.h"
#include "RGBController.h"
#include "ExampleScreens.h"

// ALIASES for HAL references
static auto& display       = HAL::display();
static auto& buttonManager = HAL::buttonManager();
static auto& audioManager  = HAL::audioManager();
static auto& accelerometer = HAL::accelerometer();
static auto& wifiManager   = HAL::wifiManagerCF();
static auto& strip         = HAL::strip();  // NeoPixels

// Define the function pointer array
App apps[APP_COUNT] = {
    #define X(func, id, name) func,
        APP_LIST
    #undef X
};

// Define the string array for names
const char* appNames[APP_COUNT] = {
    #define X(func, id, name) name,
        APP_LIST
    #undef X
};

// Ensure everything is in sync
static_assert(sizeof(apps) / sizeof(apps[0]) == APP_COUNT, "Mismatch between app array size and enum count");

// Game instances
static ClockDisplay clockDisplay(display);
static PowerManager powerManager(display, buttonManager, clockDisplay);
static ReactionTimeGame reactionGame(display, button_BottomRightIndex, buttonManager);
static SPHFluidGame sphGame(display);
static BreakoutGame breakoutGame(display, buttonManager, audioManager);
static DinoGame dinoGame(display, buttonManager,
                         button_MiddleLeftIndex, button_MiddleRightIndex, button_BottomRightIndex);
static SimonSaysGame simonGame(display, buttonManager, audioManager);
static MatrixScreensaver matrixScreensaver(display);
static Booper booper(buttonManager, audioManager, display);

void AppManager::setup() {
    HAL::initHardware();
    HAL::configureWakeupPins();
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
    ESP_LOGI(TAG_MAIN, "AppManager setup complete");
    powerManager.begin();
}

void AppManager::loop() {
    HAL::loopHardware();
    millisNow = millis();
    processButtonEvents();

    if ((millisNow - millisOld10) >= 20) {
        millisOld10 = millisNow;
        sliderPositionRead();
        screenUpdate();
    }

    if ((millisNow - millisOld50) >= 50) {
        millisOld50 = millisNow;
        HAL::updateAccelerometer();
    }

    if ((millisNow - millisOld200) >= 200) {
        millisOld200 = millisNow;
        buttonManager.saveButtonCounters();
    }

    if ((millisNow - millisLastInteraction) >= 300000) {
        powerManager.deepSleep();
    }
}

void AppManager::processButtonEvents() {
    ButtonEvent ev;
    while (buttonManager.getNextEvent(ev)) {
        if (buttonManager.hasCallback(ev.buttonIndex)) {
            ButtonCallback cb = buttonManager.getCallback(ev.buttonIndex);
            if (cb) cb(ev);
        } else {
            switch (ev.eventType) {
                case ButtonEvent_Pressed:
                    if (ev.buttonIndex == button_TopLeftIndex) {
                        appPreviously = appActive;
                        appActive = (appActive - 1 + APP_COUNT) % APP_COUNT;
                    }
                    break;
                case ButtonEvent_Released:
                    buttonCounter[ev.buttonIndex]++;
                    if (ev.buttonIndex == button_TopRightIndex) {
                        appPreviously = appActive;
                        appActive = (appActive + 1) % APP_COUNT;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void AppManager::screenUpdate() {
    apps[appActive]();

    if (appActive != appPreviously) {
		if (appActive == APP_ACCELEROMETER) {
            accelerometerScreenEnabled = true;
        } else if (appPreviously == APP_ACCELEROMETER) {
            accelerometerScreenEnabled = false;
            setColorsOff();
        }
		
		if (appActive == APP_BOOPER) {
            booper.begin();
        } else if (appPreviously == APP_BOOPER) {
            booper.end();
        }
		
		if (appActive == APP_BREAKOUT) {
            breakoutGame.begin();
        } else if (appPreviously == APP_BREAKOUT) {
            breakoutGame.end();
        }

        if (appActive == APP_CLOCK_DISPLAY) {
            clockDisplay.begin();
        } else if (appPreviously == APP_CLOCK_DISPLAY) {
            clockDisplay.reset();
        }
		
        if (appActive == APP_DINO_GAME) {
            buttonManager.registerCallback(dinoGame.getJumpButtonIndex(), DinoGame::jumpButtonCallback);
            buttonManager.registerCallback(dinoGame.getDuckButtonIndex(), DinoGame::duckButtonCallback);
            buttonManager.registerCallback(dinoGame.getResetButtonIndex(), DinoGame::resetButtonCallback);
        } else if (appPreviously == APP_DINO_GAME) {
            buttonManager.unregisterCallback(dinoGame.getJumpButtonIndex());
            buttonManager.unregisterCallback(dinoGame.getDuckButtonIndex());
            buttonManager.unregisterCallback(dinoGame.getResetButtonIndex());
            dinoGame.resetGame();
        }

		if (appActive == APP_FLASHLIGHT) {
            flashlightStatus = true;
        } else if (appPreviously == APP_FLASHLIGHT) {
            flashlightStatus = false;
            setColorsOff();
        }
		
		if (appActive == APP_POWER_MANAGER) {
            powerManager.registerShutdownCallback();
        } else if (appPreviously == APP_POWER_MANAGER) {
            powerManager.unregisterShutdownCallback();
        }
		
		if (appActive == APP_REACTION) {
            buttonManager.registerCallback(reactionGame.getButtonIndex(), ReactionTimeGame::reactionButtonPressedCallback);
        } else if (appPreviously == APP_REACTION) {
            buttonManager.unregisterCallback(reactionGame.getButtonIndex());
            reactionGame.resetGame();
        }

        if (appActive == APP_SIMON_SAYS) {
            simonGame.begin();
        } else if (appPreviously == APP_SIMON_SAYS) {
            simonGame.end();
        }

        if (appActive == APP_MATRIX_SCREENSAVER) {
            matrixScreensaver.begin();
        }
		
		if (appActive == APP_WIFI_CONFIG) {
            buttonManager.registerCallback(
                button_BottomLeftIndex, 
                WiFiManagerCF::bottomLeftButtonCallback
            );
            buttonManager.registerCallback(
                button_BottomRightIndex,
                WiFiManagerCF::bottomRightButtonCallback
            );
        } else if (appPreviously == APP_WIFI_CONFIG) {
            buttonManager.unregisterCallback(button_BottomLeftIndex);
            buttonManager.unregisterCallback(button_BottomRightIndex);
        }

        appPreviously = appActive;
    }
}

// Draw functions
void drawReactionTimeGame() {
    reactionGame.update(millisNow);
}

void drawSPHFluidGame() {
    int targetCount = sliderPosition_Percentage_Inverted_Filtered;
    static int currentCount = 100;
    if (targetCount != currentCount) {
        sphGame.setParticleCount(targetCount);
        currentCount = targetCount;
    }
    sphGame.update(accelX, accelY);
}

void drawBreakoutGame() {
    breakoutGame.update(accelX);
    breakoutGame.draw();
}

void updateClockDisplay() {
    clockDisplay.update();
}

void drawDinoGame() {
    dinoGame.setSpeedBySlider(sliderPosition_Percentage_Inverted_Filtered);
    dinoGame.update();
    dinoGame.draw();
}

void drawPowerManager() {
    powerManager.update();
}

void drawWifiConfig() {
    wifiManager.process();  // using the local alias instead of HAL::wifiManagerCF()
}

void drawSimonSaysGame() {
    simonGame.update();
}

void drawMatrixScreensaver() {
    matrixScreensaver.update();
    matrixScreensaver.draw();
}

void drawBooper() {
    booper.update();
    booper.draw();
}
