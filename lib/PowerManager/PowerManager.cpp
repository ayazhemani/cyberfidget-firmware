// PowerManager.cpp
#include "PowerManager.h"
#include "globals.h"
#include "CFHAL.h"

// Initialize the static instance reference
PowerManager* PowerManager::instance = nullptr;

PowerManager::PowerManager(ButtonManager& btnMgr, ClockDisplay& clkDsply)
    : display(HAL::displayProxy()), buttonManager(btnMgr), clockDisplay(clkDsply), lastTapTime(0) {
    // Set the static instance to this object
    instance = this;
}

void PowerManager::begin() {
    registerShutdownCallback();
}

void PowerManager::update() {
    drawShutdownScreen();
}

void PowerManager::registerShutdownCallback() {
    buttonManager.registerCallback(button_BottomLeftIndex, onButtonPressCallback);
}

void PowerManager::unregisterShutdownCallback() {
    buttonManager.unregisterCallback(button_BottomLeftIndex);
}

void PowerManager::drawShutdownScreen() {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 10, "Shutdown");
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 30, "Double-tap Bottom Left");
    display.drawString(64, 40, "to power off");
    display.display();
}

void PowerManager::onButtonPressCallback(const ButtonEvent &event) {
    if (instance) {
        if (event.eventType == ButtonEvent_Pressed) {
            unsigned long currentTime = millis();
            if (currentTime - instance->lastTapTime <= DOUBLE_TAP_THRESHOLD_MS) {
                // Detected a double-tap
                instance->display.clear();
                instance->display.setTextAlignment(TEXT_ALIGN_CENTER);
                instance->display.setFont(ArialMT_Plain_10);
                instance->display.drawString(64, 30, "Powering off...");
                instance->display.display();
                
                // Optional: save state before sleep
                instance->clockDisplay.saveTime();  // Commented out if using external display manager call

                delay(500);
                ESP_LOGV(TAG_MAIN, "Entering deep sleep...");
                esp_deep_sleep_start();
            } else {
                // Update last tap time
                instance->lastTapTime = currentTime;
            }
        }
    }
}

void PowerManager::deepSleep() {
    // Go to deep sleep
    if(preventSleepWhileCharging){
        if(batteryChangeRate < sleepChargingChangeThreshold){ // If discharging greater than 10% per hour, shut down
            clockDisplay.saveTime(); // Save the current clock time to Preferences so that it can be recovered later.
            ESP_LOGI(TAG_MAIN, "Going to sleep now...");
            
            display.clear();
            display.setFont(ArialMT_Plain_10);
            display.drawString(64, 20, "Going to sleep now...");
            display.display();
            
            delay(3000);
            esp_deep_sleep_start();
        }
        } else {
        ESP_LOGI(TAG_MAIN, "Going to sleep now...");
    
        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.drawString(64, 20, "Going to sleep now...");
        display.display();
    
        delay(3000);
        esp_deep_sleep_start();
        }
}