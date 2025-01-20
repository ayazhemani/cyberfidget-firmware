#pragma once
#include <Arduino.h>

class ButtonManager {
public:
    void init();
    void update(unsigned long now);
    // Accessors for counters, or pass callbacks for press events

private:
    static const int debounceDelay = 20;
    unsigned long lastDebounceTime[6];
    volatile int buttonCounter[6];
    // etc.
    void handleButtonPress(int index);
};
