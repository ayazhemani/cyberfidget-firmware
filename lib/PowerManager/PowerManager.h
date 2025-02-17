// PowerManager.h

#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <Arduino.h>
#include "ButtonManager.h"
#include "SSD1306Wire.h"
#include "ClockDisplay.h"

class PowerManager {
public:
    // Constructor
    PowerManager(SSD1306Wire& display, ButtonManager& buttonManager, ClockDisplay& clockDisplay);

    // Initialize the power manager
    void begin();

    // Function to handle shutdown screen updates
    void update();

    // Register button callback for shutdown
    void registerShutdownCallback();

    // Unregister button callback
    void unregisterShutdownCallback();

private:
    // The function to display the shutdown screen
    void drawShutdownScreen();
    
    // The callback function for button press event
    static void onButtonPressCallback(const ButtonEvent &event);

    // Reference to display and buttonManager
    SSD1306Wire& display;
    ButtonManager& buttonManager;
    ClockDisplay& clockDisplay;

    // Add a static instance reference
    static PowerManager* instance;

    // Double tap handling
    int bottomLeftButtonIndex;
    unsigned long lastTapTime;
    static const unsigned long DOUBLE_TAP_THRESHOLD_MS = 500; // Time window in milliseconds for double-tap
};

#endif  // POWERMANAGER_H