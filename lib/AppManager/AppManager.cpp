#include "AppManager.h"
#include "HAL.h"
#include "MenuManager.h"
#include "globals.h"
#include "PowerManager.h"
#include "ClockDisplay.h"

static auto& buttonManager = HAL::buttonManager();
static PowerManager powerManager(buttonManager, clockDisplay);

// Singleton instance
AppManager& AppManager::instance() {
    static AppManager singleton;
    return singleton;
}

// Private constructor
AppManager::AppManager() {
}

void AppManager::setup() {
    HAL::configureWakeupPins();
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set(TAG_MAIN, ESP_LOG_VERBOSE);
    HAL::initHardware();
    ESP_LOGI(TAG_MAIN, "AppManager setup complete");

    // Default to menu
    appActive     = APP_MENU;
    appPreviously = APP_MENU;

    // Start the menu
    appRegistry[appActive].beginFunc();
}

void AppManager::loop() {
    HAL::loopHardware();

    processButtonEvents();
    
    // // If the menu is active, let the menu handle everything:
    // if (MenuManager::instance().isMenuActive())
    // {
    //     MenuManager::instance().update();
    //     return; 
    // }

    if ((millis_NOW - millis_APP_TASK_20MS) >= TASK_20MS) {
        millis_APP_TASK_20MS = millis_NOW;
        runActiveApp();
    }

    // if ((millisNow - millis_HAL_TASK_50MS) >= TASK_50MS) {
    //     millis_HAL_TASK_50MS = millisNow;
    //     HAL::updateAccelerometer();
    // }

    if ((millis_NOW - millis_APP_TASK_200MS) >= TASK_200MS) {
        millis_APP_TASK_200MS = millis_NOW;
        buttonManager.saveButtonCounters();
    }

    if ((millis_NOW - millis_APP_LASTINTERACTION) >= TASK_LASTINTERACT) {
        powerManager.deepSleep();
    }
}

void AppManager::runActiveApp()
{
    // calls the runFunc for the currently active app
    appRegistry[appActive].runFunc();
}

void AppManager::processButtonEvents()
{
    ButtonEvent ev;
    while (HAL::buttonManager().getNextEvent(ev))
    {
        if (HAL::buttonManager().hasCallback(ev.buttonIndex)) {
            auto cb = HAL::buttonManager().getCallback(ev.buttonIndex);
            if (cb) cb(ev);
        } else {
            ESP_LOGD(TAG_MAIN, "Unhandled button event: %d %d", ev.buttonIndex, ev.eventType);
        }
    }
}

void AppManager::switchToApp(AppIndex newApp)
{
    ESP_LOGI(TAG_MAIN, "Switching to app %d", newApp);
    if (newApp == appActive) return;

    // end old
    appRegistry[appActive].endFunc();

    appPreviously = appActive;
    appActive     = newApp;

    // begin new
    appRegistry[appActive].beginFunc();
}