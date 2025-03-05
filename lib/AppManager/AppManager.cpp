#include "AppManager.h"
#include "globals.h"
#include "HAL.h"

// Include your game screens
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

static ClockDisplay clockDisplay( HAL::display() );
static PowerManager powerManager( HAL::display(), HAL::buttonManager(), clockDisplay );

// Example: ReactionTimeGame wants references to display + button manager
static ReactionTimeGame reactionGame( HAL::display(), button_BottomRightIndex, HAL::buttonManager() );
static SPHFluidGame    sphGame( HAL::display() );
static BreakoutGame    breakoutGame( HAL::display(), HAL::buttonManager(), HAL::audioManager() );
static DinoGame        dinoGame( HAL::display(), HAL::buttonManager(),
                                 button_MiddleLeftIndex, button_MiddleRightIndex, button_BottomRightIndex );
static SimonSaysGame   simonGame( HAL::display(), HAL::buttonManager(), HAL::audioManager() );
static MatrixScreensaver matrixScreensaver( HAL::display() );
static Booper          booper( HAL::buttonManager(), HAL::audioManager(), HAL::display() );

void AppManagerSetup()
{
    // All the hardware-level stuff is now in HAL:
    HAL::initHardware();
    HAL::configureWakeupPins();

    // If you have other logging config calls, do them here or in HAL
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
    ESP_LOGI(TAG_MAIN, "AppManagerSetup complete");

    // If you have any additional setup steps for your managers:
    powerManager.begin();
}

void AppManagerLoop()
{
    // Let the HAL do any hardware-level updates (watchdog, buttonManager, audio, etc.)
    HAL::loopHardware();

    // Keep track of time
    millisNow = millis();

    // 1) Buttons are updated by HAL::loopHardware(). 
    //    If you still want to handle button events here:
    ButtonEvent ev;
    while ( HAL::buttonManager().getNextEvent(ev) ) 
    {
        if ( HAL::buttonManager().hasCallback(ev.buttonIndex) ) {
            // If there's a button callback
            ButtonCallback cb = HAL::buttonManager().getCallback(ev.buttonIndex);
            if (cb) cb(ev);
        }
        else {
            // Global event handling
            ESP_LOGV(TAG_MAIN, "Button #%d => ", ev.buttonIndex);
            switch (ev.eventType) {
                case ButtonEvent_Pressed:
                    ESP_LOGV(TAG_MAIN, "Pressed");
                    if (ev.buttonIndex == button_TopLeftIndex) {
                        appPreviously = appActive;
                        appActive = (appActive - 1 + APP_COUNT) % APP_COUNT;
                    }
                    break;
                case ButtonEvent_Released:
                    ESP_LOGV(TAG_MAIN, "Released after %d ms", ev.duration);
                    buttonCounter[ev.buttonIndex]++;
                    if (ev.buttonIndex == button_TopRightIndex) {
                        appPreviously = appActive;
                        appActive = (appActive + 1) % APP_COUNT;
                    }
                    break;
                case ButtonEvent_Held:
                    ESP_LOGV(TAG_MAIN, "Held for %d ms", ev.duration);
                    break;
                default:
                    break;
            }
        }
    }

    // Example housekeeping intervals
    if ((millisNow - millisOld10) >= 20) {
        millisOld10 = millisNow;
        sliderPositionRead();   // Reads and filters slider
        screenUpdate();
    }

    if ((millisNow - millisOld50) >= 50) {
        millisOld50 = millisNow;
        // If you want updated accelerometer data each 50ms:
        if (HAL::accelerometer().available()) {
            accelX = HAL::accelerometer().getX();
            accelY = HAL::accelerometer().getY();
            accelZ = HAL::accelerometer().getZ();
            tempC  = HAL::accelerometer().getTemperature();
        }
    }

    if ((millisNow - millisOld200) >= 200) {
        millisOld200 = millisNow;
        // Possibly do battery update here if you want
        // Save button counters, etc.
        HAL::buttonManager().saveButtonCounters();
    }

    // Sleep management
    if ((millisNow - millisLastInteraction) >= 300000) { // 5 minutes
        powerManager.deepSleep();
    }

    if ((millisNow - millisOldHeartbeat) >= 600000) { // 10 minutes
        millisOldHeartbeat = millisNow;
        // Optional heartbeat
    }
}

// This is the function that draws the “current app” each 20 ms
void screenUpdate()
{
    // Call the function pointer from the array
    apps[appActive]();

    // If we changed apps, handle transitions
    if (appActive != appPreviously)
    {
        // Example: if appActive is accelerometer, turn on something; if we left it, turn off, etc.
        if (appActive == APP_ACCELEROMETER) {
            accelerometerScreenEnabled = true;
        } else if (appPreviously == APP_ACCELEROMETER) {
            accelerometerScreenEnabled = false;
            setColorsOff(); 
        }

        if (appActive == APP_FLASHLIGHT) {
            flashlightStatus = true;
        } else if (appPreviously == APP_FLASHLIGHT) {
            flashlightStatus = false;
            setColorsOff();
        }

        // Reaction game
        if (appActive == APP_REACTION) {
            HAL::buttonManager().registerCallback(
                reactionGame.getButtonIndex(),
                ReactionTimeGame::reactionButtonPressedCallback
            );
        } else if (appPreviously == APP_REACTION) {
            HAL::buttonManager().unregisterCallback(reactionGame.getButtonIndex());
            reactionGame.resetGame();
        }

        // Clock display
        if (appActive == APP_CLOCK_DISPLAY) {
            clockDisplay.begin();
        } else if (appPreviously == APP_CLOCK_DISPLAY) {
            clockDisplay.reset();
        }

        // Breakout
        if (appActive == APP_BREAKOUT) {
            breakoutGame.begin();
        } else if (appPreviously == APP_BREAKOUT) {
            breakoutGame.end();
        }

        // Simon Says
        if (appActive == APP_SIMON_SAYS) {
            simonGame.begin();
        } else if (appPreviously == APP_SIMON_SAYS) {
            simonGame.end();
        }

        // Matrix
        if (appActive == APP_MATRIX_SCREENSAVER) {
            matrixScreensaver.begin();
        } else if (appPreviously == APP_MATRIX_SCREENSAVER) {
            // ...
        }

        // Dino
        if (appActive == APP_DINO_GAME) {
            HAL::buttonManager().registerCallback(
                dinoGame.getJumpButtonIndex(),
                DinoGame::jumpButtonCallback
            );
            HAL::buttonManager().registerCallback(
                dinoGame.getDuckButtonIndex(),
                DinoGame::duckButtonCallback
            );
            HAL::buttonManager().registerCallback(
                dinoGame.getResetButtonIndex(),
                DinoGame::resetButtonCallback
            );
        } else if (appPreviously == APP_DINO_GAME) {
            HAL::buttonManager().unregisterCallback(dinoGame.getJumpButtonIndex());
            HAL::buttonManager().unregisterCallback(dinoGame.getDuckButtonIndex());
            HAL::buttonManager().unregisterCallback(dinoGame.getResetButtonIndex());
            dinoGame.resetGame();
        }

        // Booper
        if (appActive == APP_BOOPER) {
            booper.begin();
        } else if (appPreviously == APP_BOOPER) {
            booper.end();
        }

        // Power Manager
        if (appActive == APP_POWER_MANAGER) {
            powerManager.registerShutdownCallback();
        } else if (appPreviously == APP_POWER_MANAGER) {
            powerManager.unregisterShutdownCallback();
        }

        // WiFi Config
        if (appActive == APP_WIFI_CONFIG) {
            HAL::buttonManager().registerCallback(
                button_BottomLeftIndex, 
                WiFiManagerCF::bottomLeftButtonCallback
            );
            HAL::buttonManager().registerCallback(
                button_BottomRightIndex,
                WiFiManagerCF::bottomRightButtonCallback
            );
        } else if (appPreviously == APP_WIFI_CONFIG) {
            HAL::buttonManager().unregisterCallback(button_BottomLeftIndex);
            HAL::buttonManager().unregisterCallback(button_BottomRightIndex);
        }

        appPreviously = appActive;
    }
}

// Example “draw” functions for certain apps:
void drawReactionTimeGame() {
    // Reaction game’s logic
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
    HAL::wifiManagerCF().process();
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

// etc. for the rest of your “draw” functions...
